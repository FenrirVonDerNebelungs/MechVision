#include "stdafx.h"
#include "LineFinder.h"
LineFinder::LineFinder() : m_stackLevel(0), m_minTrigf(0.f),
m_minf(0.f), m_minLineSegPts(0), m_dLine(0), m_loopBackDist(0.f), m_nloopBackScan(0),
m_numHex(0),
m_in_line(NULL), m_covered(NULL), m_n(0),
m_stack(NULL), m_hexes(NULL), m_luna4s(NULL)
{
	for (int i = 0; i < LINEFINDERMAXLINES; i++) {
		m_lines[i].n = 0;
		m_lines[i].pts = NULL;
	}
	for (int i = 0; i < 6; i++) {
		m_lunaVecs[i].x0 = 0;
		m_lunaVecs[i].x1 = 0;
		m_lunaVecPerp[i].x0 = 0;
		m_lunaVecPerp[i].x1 = 0;
	}
	m_v60U.x0 = 0; 
	m_v60U.x1 = 0;
}
LineFinder::~LineFinder() {
	;
}
unsigned char LineFinder::init(
	HexStack* stack,
	PatternScan* patScan,
	int stackLevel,
	float minTrigf,
	float minf,
	int minLineSegPts,
	int dLine,
	float loopBackDist,
	int nloopBackScan
) {
	m_stackLevel = stackLevel;
	m_minTrigf = minTrigf;
	m_minf = minf;
	m_minLineSegPts = minLineSegPts;
	m_dLine = dLine;
	m_loopBackDist = loopBackDist;
	m_nloopBackScan = nloopBackScan;
	if (stack == NULL || patScan==NULL)
		return ECODE_FAIL;
	m_stack = stack;
	HexBase* hbase = stack->getLayer(m_stackLevel);
	m_hexes = hbase->getHexes();
	m_numHex = hbase->getNHex();
	m_luna4s = patScan->getEdgefs();

	/*owned*/
	m_covered = new bool[m_numHex];
	m_in_line = new bool[m_numHex];
	for (int i = 0; i < m_numHex; i++) {
		m_covered[i] = false;
		m_in_line[i] = false;
	}
	m_n = 0;

	setConstVectors();
	return ECODE_OK;
}
void LineFinder::release() {
	for (int i = 0; i < m_n; i++) {
		if (m_lines[i].pts != NULL) {
			delete[] m_lines[i].pts;
		}
		m_lines[i].pts = NULL;
		m_lines[i].n = 0;
	}
	m_n = 0;
	if (m_in_line != NULL)
		delete[] m_in_line;
	if (m_covered != NULL)
		delete[] m_covered;
	m_covered = NULL;
}
unsigned char LineFinder::spawn() {
	unsigned char retCode = ECODE_ABORT;
	reset();
	int curhexi = 0;
	do {
		int nextHexi;
		retCode = spawnLine(curhexi, nextHexi);
		curhexi = nextHexi;
	} while (RetOk(retCode));
	return ECODE_OK;
}
void LineFinder::setConstVectors() {
	s_2pt vUs[6];
	genConsUsides(vUs);
	for (int i = 0; i < 6; i++) {
		m_lunaVecPerp[i].x0 = vUs[i].x0;
		m_lunaVecPerp[i].x1 = vUs[i].x1;
		m_lunaVecs[i] = vecMath::perpUL(m_lunaVecPerp[i]);
	}
	m_v60U.x0 = 0.5f;
	m_v60U.x1 = sqrtf(3.f) / 2.f;
}
void LineFinder::genConsUsides(s_2pt vecs[]) {
	float longs = sqrtf(3.f) / 2.f;
	float shorts = 0.5f;
	float s2 = sqrt(2.f);
	/*start with to the right up*/

	vecs[0].x0 = 0.f;
	vecs[0].x1 = -1.f;

	vecs[1].x0 = -longs;
	vecs[1].x1 = -shorts;

	vecs[2].x0 = -longs;
	vecs[2].x1 = shorts;

	vecs[3].x0 = 0.f;
	vecs[3].x1 = 1.f;

	vecs[4].x0 = longs;
	vecs[4].x1 = shorts;

	vecs[5].x0 = longs;
	vecs[5].x1 = -shorts;
}
void LineFinder::reset() {
	for (int i = 0; i < m_numHex; i++) {
		m_covered[i] = false;
		m_in_line[i] = false;
	}
	for (int i = 0; i < m_n; i++) {
		if (m_lines->pts != NULL)
			delete[] m_lines->pts;
		m_lines->pts = NULL;
		m_lines->n = 0;
	}
	m_n = 0;
}
unsigned char LineFinder::spawnLine(int start_hexi, int& ret_hexi) {
	if (m_n >= LINEFINDERMAXLINES - 1)/*allow for 2 lines to be spawned in case of split*/
		return ECODE_ABORT;
	/*holding lines*/
	s_linePoint* lineSegR = new s_linePoint[m_numHex];
	int numLineSegR = 0;
	s_linePoint* lineSegL = new s_linePoint[m_numHex];
	int numLineSegL = 0;
	/*             */

	int lineStartHexi = findLineStart(start_hexi);
	ret_hexi = lineStartHexi;
	if (lineStartHexi < 0)
		return ECODE_ABORT;
	/*find highest luna*/
	int highestLunai = 0;
	float maxf = -100;
	for (int i = 0; i < 6; i++) {
		if (m_luna4s[lineStartHexi].f[i] > maxf) {
			maxf = m_luna4s[lineStartHexi].f[i];
			highestLunai = i;
		}
	}
	int currentPti = lineStartHexi;
	/*travel in the RRot direction along the line, or forward*/
	int lunai = highestLunai;
	unsigned char retCode = ECODE_ABORT;
	int newhexi, newlunai;
	do {
		addLinePoint(lunai, currentPti, lineSegR, numLineSegR);
		int webCircleStart_i = lunai + 2;
		retCode = scanNextLink(webCircleStart_i, m_hexes[currentPti], lunai, newhexi, newlunai);
		blackout(m_hexes[currentPti]);
		blackoutSurrounding(m_hexes[currentPti]);
		currentPti = newhexi;
		lunai = newlunai;
	} while (RetOk(retCode));
	lunai = highestLunai;/*go back to start and go in the opposite direction along the line*/
	currentPti = lineStartHexi;
	do {
		addLinePoint(lunai, currentPti, lineSegL, numLineSegL);
		int webCircleStart_i = lunai + 5;
		retCode = scanNextLink(webCircleStart_i, m_hexes[currentPti], lunai, newhexi, newlunai);
		blackout(m_hexes[currentPti]);
		blackoutSurrounding(m_hexes[currentPti]);
		currentPti = newhexi;
		lunai = newlunai;
	} while (RetOk(retCode));
	if (numLineSegR + numLineSegL < m_minLineSegPts)
		return ECODE_OK;/*ok line attempted was too short*/
	s_line fullSegLine;
	fullSegLine.n = 0;
	fullSegLine.pts = new s_linePoint[m_numHex];
	if (RetOk(mergeSegs(lineSegR, numLineSegR, lineSegL, numLineSegL, fullSegLine))) {
		retCode = formLine(fullSegLine, m_lines[m_n]);
		int i_corner = needsSplit(m_lines[m_n]);/*check for loop back on itself*/
		m_n++;
		if (i_corner > 0) {
			splitLine(i_corner, m_lines[m_n - 1], m_lines[m_n]);/*if line looped back then split*/
			m_n++;
		}
	}
	else
		retCode= ECODE_FAIL;/*this shouldn't happen*/
	delete[] fullSegLine.pts;
	delete[] lineSegL;
	delete[] lineSegR;
	return retCode;
}
int LineFinder::findLineStart(int start_hexi) {
	for (int i = start_hexi; i < m_numHex; i++) {
		if (!(m_covered[i])) {
			float maxf = m_luna4s[i].edgef;
			if (maxf > m_minTrigf)
				return i;
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
unsigned char LineFinder::blackoutSurrounding(s_hex& pthex) {
	m_covered[pthex.thisLink] = true;
	for (int i = 0; i < 6; i++) {
		if(pthex.web[i]>=0)
			m_covered[pthex.web[i]] = true;
	}
	return ECODE_OK;
}
unsigned char LineFinder::scanNextLink(int webCircleStart_i, s_hex& pthex, int lunai, int& retHexi, int& retlunai) {
	//int webCircleStart_i = lunai + 2;

	float hif = -100.f;
	retlunai = -1;
	retHexi = -1;

	for (int i = 0; i < 3; i++) {
		int webi = Math::loop(webCircleStart_i + i, 6);
		s_luna4fs& curluna = m_luna4s[pthex.web[webi]];
 		for (int j = -1; j <= 1; j++) {
			int cur_lunai = Math::loop(lunai + j, 6);
			if (curluna.f[cur_lunai] > hif && !(m_in_line[pthex.web[webi]]) ) {
				hif = curluna.f[cur_lunai];
				retlunai = cur_lunai;
				retHexi = pthex.web[webi];
			}
		}
	}
	return (hif >= m_minf) ? ECODE_OK : ECODE_ABORT;
}
unsigned char LineFinder::mergeSegs(s_linePoint lineR[], int nlineR, s_linePoint lineL[], int nlineL, s_line& newLine) {
	if (nlineR + nlineL >= m_numHex)
		return ECODE_FAIL;
	for (int i = (nlineL-1); i >=1; i--) {
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
	float estNumPts = ((float)denseLine.n) / ((float)m_dLine);
	int estNumPts_ = (int)ceil(estNumPts)+1;
	newLine.pts = new s_linePoint[estNumPts_];
	newLine.n = 0;
	for (int i = 1; i < (denseLine.n-1); i += m_dLine) {
		setVectors(denseLine.pts[i - 1], denseLine.pts[i + 1], denseLine.pts[i]);
		newLine.pts[newLine.n].lunai = denseLine.pts[i].lunai;
		newLine.pts[newLine.n].hexi = denseLine.pts[i].hexi;
		newLine.pts[newLine.n].linei = m_n;
		newLine.pts[newLine.n].loc.x0 = m_hexes[denseLine.pts[i].hexi].x;
		newLine.pts[newLine.n].loc.x1 = m_hexes[denseLine.pts[i].hexi].y;
		newLine.pts[newLine.n].perp = denseLine.pts[i].perp;
		newLine.pts[newLine.n].v = denseLine.pts[i].v;
		(newLine.n)++;
	}
	return ECODE_OK;
}
int LineFinder::needsSplit(s_line& newLine) {
	int i_corner = -1;

	int halfn = newLine.n / 2;
	int nScan = (halfn > m_nloopBackScan) ? m_nloopBackScan : halfn;

	float minDist = m_loopBackDist;
	bool isLoopBack = false;
	for (int j = newLine.n - 1; j >= newLine.n - nScan; j--) {
		for (int i = 0;  i < nScan; i++) {
			float ptsDist = vecMath::dist(newLine.pts[j].loc, newLine.pts[i].loc);
			if (ptsDist < minDist) {
				isLoopBack = true;
				break;
			}
		}
		if (isLoopBack)
			break;
	}
	if (isLoopBack) {
		/*find where the corner should be, right now will just use the halfway point*/
		i_corner = halfn - 1;
	}
	return i_corner;
}
unsigned char LineFinder::splitLine(int i_corner, s_line& fullline, s_line& newLine) {
	int estNumPts = fullline.n - i_corner + 1;
	newLine.pts = new s_linePoint[estNumPts];
	newLine.n = 0;
	for (int full_i = i_corner+1; full_i < fullline.n; full_i++) {
		newLine.pts[newLine.n].lunai = fullline.pts[i_corner].lunai;
		newLine.pts[newLine.n].hexi = fullline.pts[i_corner].hexi;
		newLine.pts[newLine.n].linei = m_n;
		newLine.pts[newLine.n].loc.x0 = fullline.pts[i_corner].loc.x0;
		newLine.pts[newLine.n].loc.x1 = fullline.pts[i_corner].loc.x1;
		newLine.pts[newLine.n].perp = fullline.pts[i_corner].perp;
		newLine.pts[newLine.n].v = fullline.pts[i_corner].v;
		(newLine.n)++;
	}
	fullline.n = i_corner;
	return ECODE_OK;
}
unsigned char LineFinder::setVectors(s_linePoint& prePt, s_linePoint& postPt, s_linePoint& pt) {
	pt.perp = m_lunaVecPerp[pt.lunai];
	/*
	int lunarotPost = pt.lunai - postPt.lunai;
	int lunarotPre = pt.lunai - prePt.lunai;
	s_2pt perpVecInLunaU = getVectorInLunaU(lunarotPre, lunarotPost);
	pt.perp = vecMath::convBasis(m_lunaVecs[pt.lunai], m_lunaVecPerp[pt.lunai], perpVecInLunaU);
	*/
	pt.v = vecMath::perpUL(pt.perp);
	return ECODE_OK;
}

s_2pt LineFinder::getVectorInLunaU(int reli0, int reli1) {
	s_2pt Uret;
	if ((reli0 * reli1) < 0) {
		/*two directions cancel out*/
		Uret.x0 = 0.f;
		Uret.x1 = 1.f;
		return Uret;
	}
	s_2pt vi0 = { (float)reli0 * m_v60U.x0, m_v60U.x1 };
	s_2pt vi1 = { -(float)reli1 * m_v60U.x0, m_v60U.x1 };
	if (reli0 == 0)
		vi0.x1 = 1.f;
	if (reli1 == 0)
		vi1.x1 = 1.f;
	Uret.x0 = vi0.x0 + vi1.x0;
	Uret.x1 = vi0.x1 + vi1.x1 + 1.f;
	float mag = vecMath::len(Uret);
	if (mag >= 0) {
		float invmag = 1.f / mag;
		Uret.x0 *= invmag;
		Uret.x1 *= invmag;
	}
	else {
		Uret.x0 = 0.f;
		Uret.x1 = 1.f;
	}
	return Uret;
}