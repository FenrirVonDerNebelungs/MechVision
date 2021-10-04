#include "LineFinder.h"

LineFinder::LineFinder() : m_minTrigo(0.f), m_mino(0.f), m_minLineSegPts(0), m_dLine(0),
m_loopBackDist(0.f), m_nloopBackScan(0), m_numHex(0),
m_in_line(NULL), m_covered(NULL),
m_n(0),
m_lineSegR(NULL), m_numLineSegR(0), m_lineSegL(NULL), m_numLineSegL(0),
m_plateLayer(NULL)
{
	m_scratchLine.n = 0;
	m_scratchLine.pts = NULL;
	for (int i = 0; i < LINEFINDERMAXLINES; i++) {
		m_lines[i].n = 0;
		m_lines[i].pts = NULL;
		m_lines[i].f = NULL;
	}
	for (int i = 0; i < 6; i++) {
		utilStruct::zero2pt(m_lunaVecs[i]);
		utilStruct::zero2pt(m_lunaVecPerp[i]);
	}
}
LineFinder::~LineFinder() { ; }

unsigned char LineFinder::init(
	s_PlateLayer* plateLayer,
	float minTrigo,
	float mino,
	int minLineSegPts,
	int dLine,
	float loopBackDist,
	int nloopBackScan
) {
	m_minTrigo = minTrigo;
	m_mino = mino;
	m_minLineSegPts = minLineSegPts;
	m_dLine = dLine;
	m_loopBackDist = loopBackDist;
	m_nloopBackScan = nloopBackScan;
	if (plateLayer == NULL)
		return ECODE_FAIL;
	m_plateLayer = plateLayer;
	m_numHex = m_plateLayer->p[0].m_nHex;

	/*owned*/
	m_covered = new bool[m_numHex];
	m_in_line = new bool[m_numHex];
	for (int i = 0; i < m_numHex; i++) {
		m_covered[i] = false;
		m_in_line[i] = false;
	}
	m_n = 0;
	for (int i = 0; i < LINEFINDERMAXLINES; i++) {
		m_lines[i].n = 0;
		m_lines[i].pts = new s_linePoint[m_numHex];
	}
	m_lineSegR = new s_linePoint[m_numHex];
	m_numLineSegR = 0;
	m_lineSegL = new s_linePoint[m_numHex];
	m_numLineSegL = 0;
	m_scratchLine.pts = new s_linePoint[m_numHex];
	m_scratchLine.n = 0;

	setConstVectors();
	return ECODE_OK;
}
void LineFinder::release() {
	if (m_scratchLine.pts != NULL)
		delete[] m_scratchLine.pts;
	m_scratchLine.pts = NULL;
	if (m_lineSegL != NULL)
		delete[] m_lineSegL;
	m_lineSegL = NULL;
	if (m_lineSegR != NULL)
		delete[] m_lineSegR;
	m_lineSegR = NULL;

	for (int i = 0; i < LINEFINDERMAXLINES; i++) {
		if (m_lines[i].pts != NULL) {
			delete[] m_lines[i].pts;
		}
		m_lines[i].pts = NULL;
		m_lines[i].n = 0;
	}
	m_n = 0;
	if (m_in_line != NULL)
		delete[] m_in_line;
	m_in_line = NULL;
	if (m_covered != NULL)
		delete[] m_covered;
	m_covered = NULL;
}
void LineFinder::reset() {
	for (int i = 0; i < m_numHex; i++) {
		m_covered[i] = false;
		m_in_line[i] = false;
	}
	for (int i = 0; i < m_n; i++) {
		m_lines[i].n = 0;
	}
	m_n = 0;
	m_numLineSegR = 0;
	m_numLineSegL = 0;
	m_scratchLine.n = 0;
}
unsigned char LineFinder::spawn() {
	unsigned char retCode = ECODE_ABORT;
	reset();
	int curhexi = 6000;
	do {
		int nextHexi;
		retCode = spawnLine(curhexi, nextHexi);
		curhexi = nextHexi;
	} while (RetOk(retCode));
	return ECODE_OK;
}

void LineFinder::setConstVectors() {
	for (int i = 0; i < 6; i++) {
		m_lunaVecPerp[i].x0 = m_plateLayer->p[0].m_hexU[i].x0;
		m_lunaVecPerp[i].x1 = m_plateLayer->p[0].m_hexU[i].x1;
		m_lunaVecs[i] = vecMath::perpUL(m_lunaVecPerp[i]);
	}
}

unsigned char LineFinder::spawnLine(int start_hexi, int& ret_hexi) {
	if (m_n >= LINEFINDERMAXLINES - 1)/*allow for 2 lines to be spawned in case of split*/
		return ECODE_ABORT;
	m_numLineSegL = 0;
	m_numLineSegR = 0;
	m_scratchLine.n = 0;

	int lunaHighest = -1;
	int lineStartHexi = findLineStart(start_hexi, lunaHighest);
	ret_hexi = lineStartHexi;
	if (lineStartHexi < 0)
		return ECODE_ABORT;

	int currentPti = lineStartHexi;
	/*travel in the RRot direction along the line, or forward*/
	int lunai = lunaHighest;
	unsigned char retCode = ECODE_ABORT;
	int newhexi = 0;
	int newlunai = 0;

	do {
		addLinePoint(lunai, currentPti, m_lineSegR, m_numLineSegR);
		int webCircleStart_i = lunai + 2;
		retCode = scanNextLink(webCircleStart_i, currentPti, lunai, newhexi, newlunai);
		if (currentPti < m_numHex && currentPti >= 0) {
			blackout(currentPti);
			blackoutSurrounding(currentPti);
		}
		currentPti = newhexi;
		lunai = newlunai;
	} while (RetOk(retCode));
	lunai = lunaHighest;/*go back to start and go in the opposite direction along the line*/
	currentPti = lineStartHexi;
	do {
		addLinePoint(lunai, currentPti, m_lineSegL, m_numLineSegL);
		int webCircleStart_i = lunai + 5;
		retCode = scanNextLink(webCircleStart_i, currentPti, lunai, newhexi, newlunai);
		if (currentPti < m_numHex && currentPti >= 0) {
			blackout(currentPti);
			blackoutSurrounding(currentPti);
		}
		currentPti = newhexi;
		lunai = newlunai;
	} while (RetOk(retCode));

	if (m_numLineSegR + m_numLineSegL < m_minLineSegPts)
		return ECODE_OK;/*ok line attempted was too short*/

	if (RetOk(mergeSegs(m_lineSegR, m_numLineSegR, m_lineSegL, m_numLineSegL, m_scratchLine))) {
		retCode = formLine(m_scratchLine, m_lines[m_n]);
		m_n++;
	}
	else
		retCode = ECODE_FAIL;/*this shouldn't happen*/
	return retCode;
}
int LineFinder::findLineStart(int start_hexi, int& lunaHighest) {
	lunaHighest = 0;
	for (int i = start_hexi; i < m_numHex; i++) {
		if (!(m_covered[i])) {
			float maxo = -99.0;
			for (int j = 0; j < 6; j++) {
				maxo = m_plateLayer->p[j].m_fhex[i].o;
				if (maxo > m_minTrigo) {
					lunaHighest = j;
					return i;
				}
			}
		}
	}
	return -1;
}
unsigned char LineFinder::addLinePoint(int lunai, int hexi, s_linePoint linePts[], int& nPts) {
	if (nPts >= m_numHex)
		return ECODE_ABORT;
	linePts[nPts].hexi = hexi;
	linePts[nPts].lunai = lunai;
	nPts++;
	return ECODE_OK;
}
unsigned char LineFinder::scanNextLink(int webCircleStart_i, long hex_i, int lunai, int& retHexi, int& retlunai) 
{
	float hio = -100.f;
	retlunai = -1;
	retHexi = -1;
	for (int i = 0; i < 3; i++) {
		int webi = Math::loop(webCircleStart_i + i, 6);
		s_fNode* cur_lunaNode = (s_fNode*)m_plateLayer->p[lunai].m_fhex[hex_i].nodes[webi];
		if (cur_lunaNode != NULL) {
			if (cur_lunaNode->o > hio && !(m_in_line[cur_lunaNode->thislink])) {
				hio = cur_lunaNode->o;
				retlunai = lunai;
				retHexi = cur_lunaNode->thislink;
			}
		}
	}
	/*
	for (int i = 0; i < 3; i++) {
		int webi = Math::loop(webCircleStart_i + i, 6);
		for (int j = -1; j <= 1; j++) {
			int cur_lunai = Math::loop(lunai + j, 6);
			s_fNode* cur_lunaNode = (s_fNode*)m_plateLayer->p[cur_lunai].m_fhex[hex_i].nodes[webi];
			if (cur_lunaNode != NULL) {
				if (cur_lunaNode->o > hio && !(m_in_line[cur_lunaNode->thislink])) {
					hio = cur_lunaNode->o;
					retlunai = cur_lunai;
					retHexi = cur_lunaNode->thislink;
				}
			}
		}
	}
	*/
	return (hio >= m_mino) ? ECODE_OK : ECODE_ABORT;
}
unsigned char LineFinder::blackoutSurrounding(long hex_i) {
	m_covered[hex_i] = true;
	/*all plates should have the same structure*/
	for (int i = 0; i < 6; i++) {
		s_fNode* sur_nd = (s_fNode*)m_plateLayer->p[0].m_fhex[hex_i].nodes[i];
		if (sur_nd != NULL) {
			long toblack_i = sur_nd->thislink;
			m_covered[toblack_i] = true;
		}
	}
	return ECODE_OK;
}

unsigned char LineFinder::mergeSegs(s_linePoint lineR[], int nlineR, s_linePoint lineL[], int nlineL, s_line& newLine) {
	if (nlineR + nlineL >= m_numHex)
		return ECODE_FAIL;
	for (int i = (nlineL - 1); i >= 1; i--) {
		newLine.pts[newLine.n] = lineL[i];
		(newLine.n)++;
	}
	for (int i = 0; i < nlineR; i++) {
		newLine.pts[newLine.n] = lineR[i];
		(newLine.n)++;
	}
	return ECODE_OK;
}
unsigned char LineFinder::formLine(s_line& denseLine, s_line& newLine) {
	newLine.n = 0;
	for (int i = 1; i < (denseLine.n - 1); i += m_dLine) {
		setVectors(denseLine.pts[i - 1], denseLine.pts[i + 1], denseLine.pts[i]);
		newLine.pts[newLine.n].lunai = denseLine.pts[i].lunai;
		newLine.pts[newLine.n].hexi = denseLine.pts[i].hexi;
		newLine.pts[newLine.n].linei = m_n;
		newLine.pts[newLine.n].loc.x0 = m_plateLayer->p[0].m_fhex[denseLine.pts[i].hexi].x;//m_hexes[denseLine.pts[i].hexi].x;
		newLine.pts[newLine.n].loc.x1 = m_plateLayer->p[0].m_fhex[denseLine.pts[i].hexi].y;// m_hexes[denseLine.pts[i].hexi].y;
		newLine.pts[newLine.n].perp = denseLine.pts[i].perp;
		newLine.pts[newLine.n].v = denseLine.pts[i].v;
		(newLine.n)++;
	}
	return ECODE_OK;
}

unsigned char LineFinder::setVectors(s_linePoint& prePt, s_linePoint& postPt, s_linePoint& pt) {
	pt.perp = m_lunaVecPerp[pt.lunai];
	pt.v = vecMath::perpUL(pt.perp);
	return ECODE_OK;
}