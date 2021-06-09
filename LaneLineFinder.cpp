#include "stdafx.h"
#include "LaneLineFinder.h"
LaneLineFinder::LaneLineFinder() : m_rawLineFinder(NULL), m_rawlines(NULL), m_nraw(0),
m_maxImgDim(0.f), m_ZoneTopFrac(0.f), m_minNumLaneLinePts(0), m_fracForMostlyOut(0.f),
m_numLinesFound(0), m_nZippedLines(0),
m_maxPtsToSearchForZipStart(0), m_maxZipStartLineWidth(0.f), m_maxZipLineWidth(0.f),
m_minNumZipMatched(0.f), m_fracForZipMatch(0.f)
{
	m_laneCandZoneTopMid.x0 = 0.f;
	m_laneCandZoneTopMid.x1 = 0.f;
	for (int i = 0; i < LINEFINDERMAXLINES; i++) {
		m_linesFoundIs[i] = -1;
		m_lineMostlyOut[i] = false;
		m_ForwardLine[i] = false;
		m_zippedLines[i].n = 0;
		m_zippedLines[i].pts = NULL;
	}
}
LaneLineFinder::~LaneLineFinder() {
	;
}
unsigned char LaneLineFinder::init
(
	LineFinder* lineFinder,
	Img* img,
	float ZoneTopFrac,
	int minNumLaneLinePts,
	float fracForMostlyOut,
	int   maxPtsToSearchForZipStart,
	float maxZipStartLineWidth,
	float maxZipLineWidth,
	float minNumZipMatched,
	float fracForZipMatch
)
{
	m_rawLineFinder = lineFinder;
	s_2pt imageWH = { (float)img->getWidth(), (float)img->getHeight() };
	m_maxImgDim = imageWH.x1;
	m_ZoneTopFrac = ZoneTopFrac;
	m_minNumLaneLinePts = minNumLaneLinePts;
	m_fracForMostlyOut = fracForMostlyOut;

	m_laneCandZoneTopMid.x0 = imageWH.x0 / 2.f;
	m_laneCandZoneTopMid.x1 = imageWH.x1 * m_ZoneTopFrac;

	for (int i = 0; i < LINEFINDERMAXLINES; i++) {
		m_linesFoundIs[i] = -1;
		m_lineMostlyOut[i] = false;
		m_ForwardLine[i] = false;
	}
	m_numLinesFound = 0;

	for (int i = 0; i < LINEFINDERMAXLINES; i++) {
		m_zippedLines[i].n = 0;
		m_zippedLines[i].pts = NULL;
	}
	m_nZippedLines = 0;

	m_maxPtsToSearchForZipStart = maxPtsToSearchForZipStart;
	m_maxZipStartLineWidth = maxZipStartLineWidth;
	m_maxZipLineWidth = maxZipLineWidth;
	m_minNumZipMatched = minNumZipMatched;
	m_fracForZipMatch = fracForZipMatch;
	return ECODE_OK;
}
void LaneLineFinder::release() {
	reset();
}
unsigned char LaneLineFinder::spawn() {
	reset();
	m_rawlines = m_rawLineFinder->getLines();
	m_nraw = m_rawLineFinder->getNLines();
	findZoneLines();
	zipLines();
	return ECODE_OK;
}
void LaneLineFinder::reset() {
	m_numLinesFound = 0;
	for (int i = 0; i < m_nZippedLines; i++) {
		if (m_zippedLines[i].pts != NULL)
			delete [] m_zippedLines[i].pts;
		m_zippedLines[i].pts = NULL;
		m_zippedLines[i].n = 0;
	}
	m_nZippedLines = 0;
}
unsigned char LaneLineFinder::findZoneLines() {
	for (int i_line = 0; i_line < m_nraw; i_line++) {
		bool mostlyOut, forwardDir;
		bool foundLine = zoneLine(m_rawlines[i_line], mostlyOut, forwardDir);
		if (foundLine) {
			m_linesFoundIs[m_numLinesFound] = i_line;
			m_lineMostlyOut[m_numLinesFound] = mostlyOut;
			m_ForwardLine[m_numLinesFound] = forwardDir;
			m_numLinesFound++;
		}
	}
	return (m_numLinesFound > 0) ? ECODE_OK : ECODE_ABORT;
}
bool LaneLineFinder::zoneLine(s_line& rawLine, bool& mostlyOut, bool& forwardDir) {
	int closestPt = -1;/*line point with lowest y*/
	float lowesty = 2.f * m_laneCandZoneTopMid.x1;
	bool found = false;
	float countOut = 0.f;
	float countTot = 0.f;
	for (int i = 0; i < rawLine.n; i++) {
		float cury = rawLine.pts[i].loc.x1;
		if (cury < m_laneCandZoneTopMid.x1) {
			countTot++;
			if (isWhiteOut(rawLine.pts[i]))
				countOut++;
			if (cury < lowesty) {
				closestPt = i;
				lowesty = cury;
			}
		}
	}
	mostlyOut = false;
	forwardDir = true;
	if (countTot > 0.f) {
		found = true;
		float fracOut = countOut / countTot;
		if (fracOut > m_fracForMostlyOut)
			mostlyOut = true;
		if (closestPt > ((float)rawLine.n) / 2.f)
			forwardDir = false;
	}
	return found;
}
bool LaneLineFinder::isWhiteOut(s_linePoint& pt) {
	s_2pt centerBottom = { m_laneCandZoneTopMid.x0, 0.f };
	s_2pt vecToPt = vecMath::v12(centerBottom, pt.loc);
	float dotprod = vecMath::dot(vecToPt, pt.perp);
	return (dotprod > 0);
}

unsigned char LaneLineFinder::zipLines() {
	for (int i = 0; i < m_numLinesFound; i++) {
		if (m_lineMostlyOut[i]) {
			int i_side, i_zip;
			int indexZippedTo = isZipped(m_rawlines[m_linesFoundIs[i]], m_ForwardLine[i], i_zip, i_side);
			if (indexZippedTo >= 0)
				addZippedLine(m_rawlines[m_linesFoundIs[i]], m_ForwardLine[i], i_side);
		}
	}
	return ECODE_OK;
}
int LaneLineFinder::isZipped(s_line& lineSide, bool lineForward, int& i_zip, int& i_side) {
	for (int i = 0; i < m_numLinesFound; i++) {
		if (!(m_lineMostlyOut[i])) {
			/*find closest point*/
			if (closestPoints(m_rawlines[m_linesFoundIs[i]], lineSide, m_ForwardLine[i], lineForward, i_zip, i_side)) {
				getZipStartPoints(m_rawlines[m_linesFoundIs[i]], lineSide, i_zip, i_side, m_ForwardLine[i], lineForward);
				if (doZip(i_zip, i_side, m_rawlines[m_linesFoundIs[i]], lineSide, m_ForwardLine[i], lineForward))
					return m_linesFoundIs[i];
			}
		}
	}
	return -1;
}
bool LaneLineFinder::getZipStartPoints(s_line& lineZip, s_line& lineSide, int& i_zip, int& i_side, bool zipForward, bool lineForward)
{
	/*go backwards to find start */
	int i_inc = (lineForward) ? -1 : 1;
	int j_inc = (zipForward) ? -1 : 1;
	int i = i_side;
	int j = i_zip;
	while (i < lineSide.n && i >= 0 && j < lineZip.n && j >= 0) {
		float ptsDist = vecMath::dist(lineSide.pts[i].loc, lineZip.pts[j].loc);
		i += i_inc;
		j += j_inc;
		if (ptsDist > m_maxZipStartLineWidth)
			break;
	}
	i -= i_inc;
	j -= j_inc;
	i_side = i;
	i_zip = j;
	return true;
}
bool LaneLineFinder::closestPoints(s_line& lineZip, s_line& lineSide, bool zipForward, bool lineForward, int& i_zip, int& i_side) {
	float distPts = m_maxImgDim;
	i_side = -1;
	i_zip = -1;
	int maxNSide = (lineSide.n > m_maxPtsToSearchForZipStart) ? m_maxPtsToSearchForZipStart : lineSide.n;
	int maxNZip = (lineZip.n > m_maxPtsToSearchForZipStart) ? m_maxPtsToSearchForZipStart : lineZip.n;
	int zip_index_start = zipForward ? 0 : lineZip.n-1;
	int zip_index = zip_index_start;
	int line_index = lineForward ? 0 : lineSide.n-1;
	int zip_inc = zipForward ? 1 : -1;
	int line_inc = lineForward ? 1 : -1;
	for (int i = 0; i < maxNSide; i++) {
		for (int j = 0; j < maxNZip; j++) {
			float newDist = vecMath::dist(lineSide.pts[line_index].loc, lineZip.pts[zip_index].loc);
			if (newDist < distPts) {
				i_side = line_index;
				i_zip = zip_index;
				distPts = newDist;
			}
			zip_index += zip_inc;
		}
		zip_index = zip_index_start;
		line_index += line_inc;
	}

	return (distPts < m_maxZipStartLineWidth);
}

bool LaneLineFinder::doZip(int i_zip, int i_side, s_line& lineZip, s_line& lineSide, bool zipForward, bool lineForward) {
	int side_inc = (lineForward) ? 1 : -1;
	int zip_inc = (zipForward) ? 1 : -1;
	float numZipped = 0.f;

	int side_i = i_side;
	int zip_i = i_zip;
	int sideCnt = (lineForward) ? lineSide.n - i_side : i_side + 1;
	int zipCnt = (zipForward) ? lineZip.n - i_zip : i_zip + 1;
	int maxCnt = (sideCnt <= zipCnt) ?  sideCnt: zipCnt;
	float numTot = (float)maxCnt;

	for(int i=0; i<maxCnt; i++)
	{
		s_2pt ptsV = vecMath::v12(lineSide.pts[side_i].loc, lineZip.pts[zip_i].loc);
		float ptsDist = vecMath::len(ptsV);
		if(ptsDist < m_maxZipLineWidth) {
			float perpVecDot = vecMath::dot(lineSide.pts[side_i].perp, lineZip.pts[zip_i].perp);
			float perpIntoLineDot = vecMath::dot(ptsV, lineSide.pts[side_i].perp);
			if (perpVecDot < 0.f  && perpIntoLineDot>=0.f) 
				numZipped += 1.f;
		}
		side_i += side_inc;
		zip_i += zip_inc;
	}
	if (numTot <= 0.f)
		return false;
	float fracZipMatched = numZipped / numTot;
	return (numZipped > m_minNumZipMatched && fracZipMatched > m_fracForZipMatch);
}
unsigned char LaneLineFinder::addZippedLine(s_line& rawLine, bool isForward, int& i_side) {
	m_zippedLines[m_nZippedLines].pts = new s_linePoint[rawLine.n];
	int n = 0;
	if (isForward) {
		for (int i = i_side; i < rawLine.n; i++) {
			m_zippedLines[m_nZippedLines].pts[n] = rawLine.pts[i];
			n++;
		}
	}
	else {
		for (int i = i_side; i >= 0; i--) {
			m_zippedLines[m_nZippedLines].pts[n] = rawLine.pts[i];
			n++;
		}
	}
	m_zippedLines[m_nZippedLines].n = n;
	m_nZippedLines++;
	return ECODE_OK;
}