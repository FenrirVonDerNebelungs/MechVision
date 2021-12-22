#include "LineFinder.h"

namespace n_line {
	void copyPt(const s_linePoint& p1, s_linePoint& p2)
	{
		p2.o = p1.o;
		p2.lunai = p1.lunai;
		p2.hexi = p1.hexi;
		p2.linei = p1.linei;
		p2.v = p1.v;
		p2.perp = p1.perp;
		p2.loc = p1.loc;
	}
	void copyLines(const s_line& l1, s_line& l2) {
		for (long i = 0; i < l1.n; i++) {
			copyPt(l1.pts[i], l2.pts[i]);
		}
	}
}

LineFinder::LineFinder() : m_spawn_start_hexi(0), m_minTrigo(0.f), m_mino(0.f), m_minLineSegPts(0), m_dLine(0),
m_maxNebDist(0.f), m_minMergeOverlap(0), m_mergeOverlap(0.f), m_numHex(0),
m_in_line(NULL), m_covered(NULL),
m_n(0), m_n_lines(0),
m_lineSegR(NULL), m_numLineSegR(0), m_lineSegL(NULL), m_numLineSegL(0),
m_plateLayer(NULL)
{
	m_scratchLine.n = 0;
	m_scratchLine.pts = NULL;
	m_scratchLine.f = NULL;
	m_scratchLine.blacked = false;
	m_scratchLine.lowerHalf = false;
	m_scratchLine1.n = 0;
	m_scratchLine1.pts = NULL;
	m_scratchLine1.f = NULL;
	m_scratchLine1.blacked = false;
	m_scratchLine1.lowerHalf = false;
	for (int i = 0; i < LINEFINDERMAXLINES; i++) {
		m_singLunaLines[i].n = 0;
		m_singLunaLines[i].pts = NULL;
		m_singLunaLines[i].f = NULL;
		m_singLunaLines[i].blacked = false;
		m_singLunaLines[i].lowerHalf = false;
		m_mergedLines[i].n = 0;
		m_mergedLines[i].pts = NULL;
		m_mergedLines[i].f = NULL;
		m_mergedLines[i].blacked = false;
		m_mergedLines[i].lowerHalf = false;
		
		m_lines[i] = NULL;
	}
	for (int i = 0; i < 6; i++) {
		utilStruct::zero2pt(m_lunaVecs[i]);
		utilStruct::zero2pt(m_lunaVecPerp[i]);
	}
}
LineFinder::~LineFinder() { ; }

unsigned char LineFinder::init(
	s_PlateLayer* plateLayer,
	long spawn_start_hexi,
	float minTrigo,
	float mino,
	int minLineSegPts,
	int dLine,
	float maxNebDistFac,
	long minMergeOverlap,
	float mergeOverlap
) {
	m_spawn_start_hexi = spawn_start_hexi;
	m_minTrigo = minTrigo;
	m_mino = mino;
	m_minLineSegPts = minLineSegPts;
	m_dLine = dLine;

	m_minMergeOverlap = minMergeOverlap;
	m_mergeOverlap = mergeOverlap;

	if (plateLayer == NULL)
		return ECODE_FAIL;
	m_plateLayer = plateLayer;
	m_numHex = m_plateLayer->p[0].m_nHex;

	/*all plate layers should have same structure as far as web*/
	s_hexPlate& plate = m_plateLayer->p[0];
	float Rs = plate.m_RShex;
	m_maxNebDist = maxNebDistFac * Rs;

	/*owned*/
	m_covered = new bool[m_numHex];
	m_in_line = new bool[m_numHex];
	for (int i = 0; i < m_numHex; i++) {
		m_covered[i] = false;
		m_in_line[i] = false;
	}
	m_n = 0;
	m_n_lines = 0;
	for (int i = 0; i < LINEFINDERMAXLINES; i++) {
		m_singLunaLines[i].n = 0;
		m_singLunaLines[i].pts = new s_linePoint[m_numHex];
		m_mergedLines[i].n = 0;
		m_mergedLines[i].pts = new s_linePoint[m_numHex];
	}
	m_lineSegR = new s_linePoint[m_numHex];
	m_numLineSegR = 0;
	m_lineSegL = new s_linePoint[m_numHex];
	m_numLineSegL = 0;
	m_scratchLine.pts = new s_linePoint[m_numHex];
	m_scratchLine.n = 0;
	m_scratchLine1.pts = new s_linePoint[m_numHex];
	m_scratchLine1.n = 0;

	setConstVectors();
	return ECODE_OK;
}
void LineFinder::release() {
	if (m_scratchLine1.pts != NULL)
		delete[] m_scratchLine1.pts;
	m_scratchLine1.pts = NULL;
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
		if (m_singLunaLines[i].pts != NULL) {
			delete[] m_singLunaLines[i].pts;
		}
		m_singLunaLines[i].pts = NULL;
		m_singLunaLines[i].n = 0;
		if (m_mergedLines[i].pts != NULL) {
			delete[] m_mergedLines[i].pts;
		}
		m_mergedLines[i].pts = NULL;
		m_mergedLines[i].n = 0;

		m_lines[i] = NULL;
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
		m_singLunaLines[i].n = 0;
		m_singLunaLines[i].blacked = false;
		m_mergedLines[i].n = 0;
		m_mergedLines[i].blacked = false;
		m_mergedLines[i].lowerHalf = false;
		m_lines[i] = NULL;
	}
	m_n = 0;
	m_n_lines = 0;
	m_numLineSegR = 0;
	m_numLineSegL = 0;
	m_scratchLine.n = 0;
	m_scratchLine1.n = 0;
}
unsigned char LineFinder::spawn() {
	unsigned char retCode = ECODE_ABORT;
	reset();
	int curhexi = m_spawn_start_hexi;
	do {
		int nextHexi;
		retCode = spawnLine(curhexi, nextHexi);
		curhexi = nextHexi;
	} while (RetOk(retCode));
	return mergeLunaLines();
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
	float o=0.f;
	int lineStartHexi = findLineStart(start_hexi, lunaHighest, o);
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
		addLinePoint(lunai, currentPti, o, m_lineSegR, m_numLineSegR);
		int webCircleStart_i = lunai + 2;
		retCode = scanNextLink(webCircleStart_i, currentPti, lunai, newhexi, newlunai, o);
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
		addLinePoint(lunai, currentPti, o, m_lineSegL, m_numLineSegL);
		int webCircleStart_i = lunai + 5;
		retCode = scanNextLink(webCircleStart_i, currentPti, lunai, newhexi, newlunai, o);
		if (currentPti < m_numHex && currentPti >= 0) {
			blackout(currentPti);
			blackoutSurrounding(currentPti);
		}
		currentPti = newhexi;
		lunai = newlunai;
	} while (RetOk(retCode));

	if (m_numLineSegR + m_numLineSegL < m_minLineSegPts)
		return ECODE_OK;/*ok line attempted was too short*/

	/*
	if (RetOk(mergeSegs(m_lineSegR, m_numLineSegR, m_lineSegL, m_numLineSegL, m_scratchLine))) {
		retCode = formLine(m_scratchLine, m_lines[m_n]);
		m_n++;
	}
		else
		retCode = ECODE_FAIL;this shouldn't happen
	*/
	if (Err(mergeSegs(m_lineSegR, m_numLineSegR, m_lineSegL, m_numLineSegL, m_singLunaLines[m_n])))
		retCode = ECODE_FAIL;
	m_n++;
	return ECODE_OK;
}
int LineFinder::findLineStart(int start_hexi, int& lunaHighest, float& o) {
	lunaHighest = 0;
	for (int i = start_hexi; i < m_numHex; i++) {
		if (!(m_covered[i])) {
			float maxo = -99.0;
			for (int j = 0; j < 6; j++) {
				maxo = m_plateLayer->p[j].m_fhex[i].o;
				if (maxo > m_minTrigo) {
					o = maxo;
					lunaHighest = j;
					return i;
				}
			}
		}
	}
	return -1;
}
unsigned char LineFinder::addLinePoint(int lunai, int hexi, float o, s_linePoint linePts[], int& nPts) {
	if (nPts >= m_numHex)
		return ECODE_ABORT;
	linePts[nPts].o = o;
	linePts[nPts].hexi = hexi;
	linePts[nPts].lunai = lunai;
	linePts[nPts].loc.x0 = m_plateLayer->p[lunai].m_fhex[hexi].x;
	linePts[nPts].loc.x1 = m_plateLayer->p[lunai].m_fhex[hexi].y;
	nPts++;
	return ECODE_OK;
}
unsigned char LineFinder::scanNextLink(int webCircleStart_i, long hex_i, int lunai, int& retHexi, int& retlunai, float& o) 
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
				o = hio;
				retlunai = lunai;
				retHexi = cur_lunaNode->thislink;
			}
		}
	}

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
/*
unsigned char LineFinder::formLine(s_line& denseLine, s_line& newLine) {
	newLine.n = 0;
	for (int i = 1; i < (denseLine.n - 1); i += m_dLine) {
		setVectors(denseLine.pts[i - 1], denseLine.pts[i + 1], denseLine.pts[i]);
		newLine.pts[newLine.n].o = denseLine.pts[i].o;
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
*/
unsigned char LineFinder::setVectorsForLine(s_line* lineptr) {
	for (int i = 0; i < lineptr->n; i++)
		setVectors(lineptr->pts[i]);
	return ECODE_OK;
}
unsigned char LineFinder::setVectors(s_linePoint& pt) {
	pt.perp = m_lunaVecPerp[pt.lunai];
	pt.v = vecMath::perpUL(pt.perp);
	return ECODE_OK;
}
unsigned char LineFinder::mergeLunaLines() {
	for (int i = 0; i < m_n; i++) {
		bool wasMerged = false;
		s_line* firstLine = &(m_singLunaLines[i]);
		for (int j = i + 1; j < m_n; j++) {
			s_line* secondLine = (m_singLunaLines[j].blacked) ? &(m_mergedLines[j]) :&(m_singLunaLines[j]);
			long cur_l_i = 0;
			long cur_c_i = 0;
			if (doMergeLunaLines(*firstLine, *secondLine, cur_l_i, cur_c_i)) {
				bool selFirst = false;
				unsigned char err = mergeLunaLinesForward(cur_l_i, cur_c_i, *firstLine, *secondLine, m_scratchLine, selFirst);
				err = mergeLunaLineToTail(m_scratchLine, *firstLine, cur_l_i, m_mergedLines[j]);/*really this should merge which ever line is stronger instead of always selecting the 1st line in the array, however this is faster since it avoids a copy of the line*/
				m_singLunaLines[j].blacked = true;
				wasMerged = true;
				firstLine = &(m_mergedLines[j]);
			}
		}
		if(!wasMerged && m_mergedLines[i].n<=0)
			n_line::copyLines(m_singLunaLines[i], m_mergedLines[i]);
	}
	m_n_lines = 0;
	for (int i = 0; i < m_n; i++) {
		if (m_mergedLines[i].n>0) {
			m_lines[m_n_lines] = &(m_mergedLines[i]);
			setVectorsForLine(m_lines[m_n_lines]);
			m_n_lines++;
		}
	}
	return ECODE_OK;
}
bool LineFinder::doMergeLunaLines(const s_line& l, const s_line& c, long& cur_l_i, long& cur_c_i) {
	int cnt_found = 0;
	bool inc_prev = false;
	bool cur_found=false;
	cur_c_i = -1;
	cur_l_i = -1;
	bool isFound = false;
	/*lines have only one luna*/
	if (!lunaNeb(l.pts[0], c.pts[0]))/*check to make sure that merging line to line with similar luna*/
		return false;
	for (long l_i = 0; l_i < l.n-m_minMergeOverlap; l_i++) {
		int c_i = 0;
		for (; c_i < c.n-m_minMergeOverlap; c_i++) {
			if (overlapPts(l.pts[l_i], c.pts[c_i])) {
				/*check if a few more overlap*/
				bool cur_found = true;
				for (long i_cnt = 0; i_cnt < m_minMergeOverlap; i_cnt++) {
					if (!overlapPts(l.pts[l_i + i_cnt], c.pts[c_i + i_cnt]))
						cur_found = false;
				}
				if (cur_found) {
					cur_c_i = c_i;
					cur_l_i = l_i;
					isFound = true;
					break;
				}
			}
		}
		if (isFound) 
			break;
	}
	return isFound;
}
unsigned char LineFinder::mergeLunaLinesForward(int l_i, int c_i, const s_line& l, const s_line& c, s_line& m, bool& selFirst) {
	int cur_c_i = c_i;
	int cur_m_i = 0;
	float c_val_sum = 0.f;
	float l_val_sum = 0.f;
	int i=0;
	for (i = l_i; i < l.n; i++) {
		c_val_sum += c.pts[cur_c_i].o;
		l_val_sum += l.pts[l_i].o;
		/*since lines have only one lunai per line it is not necessary to check again for luna match*/
		if (neb(l.pts[i], c.pts[cur_c_i])) {
			if (l.pts[i].o >= c.pts[cur_c_i].o) {
				n_line::copyPt(l.pts[i], m.pts[cur_m_i]);
			}
			else
				n_line::copyPt(c.pts[cur_c_i], m.pts[cur_m_i]);
			cur_m_i++;
			cur_c_i++;
			if (cur_c_i >= c.n)
				break;
		}
		else {
			break;
		}
	}
	/*neb will be less restricted than mergeOverlap, therefore at least the min number for a merge overlap have been selected*/
	selFirst = (l_val_sum >= c_val_sum);
	if (cur_c_i >= c.n && cur_m_i >= l.n) {
		/*went all the way to the end for both lines*/
		m.n = cur_m_i;
		return ECODE_OK;
	}
	if (selFirst) {
		/*complete the merge line with points from the first 'l' line*/
		/*for loop ends before inc*/
		for (; i < l.n; i++) {
			n_line::copyPt(l.pts[i], m.pts[cur_m_i]);
			cur_m_i++;
		}
		i++;
	}
	else {
		for (; cur_c_i < c.n; cur_c_i++) {
			n_line::copyPt(c.pts[cur_c_i], m.pts[cur_m_i]);
			cur_m_i++;
		}
	}
	m.n = cur_m_i;
	return ECODE_OK;
}
unsigned char LineFinder::mergeLunaLineToTail(const s_line& m, const s_line& c, long c_i, s_line& mm) {
	long cur_mm_i = 0;
	for (long i = 0; i < c_i; i++) {
		n_line::copyPt(c.pts[i], mm.pts[cur_mm_i]);
		cur_mm_i++;
	}
	for (long i = 0; i < m.n; i++) {
		n_line::copyPt(m.pts[i], mm.pts[cur_mm_i]);
		cur_mm_i++;
	}
	mm.n = cur_mm_i;
	return ECODE_OK;
}
bool LineFinder::lunaNeb(const s_linePoint& p1, const s_linePoint& p2) {
	int lunaDiff = abs(p1.lunai - p2.lunai);
	if (lunaDiff > 3)
		lunaDiff = 6 - lunaDiff;
	return lunaDiff <= LINEFINDERMAXLUNADIFF;
}
bool LineFinder::neb(const s_linePoint& p1, const s_linePoint& p2) {
	return n_line::isIn(p1, p2, m_maxNebDist);
}
bool LineFinder::overlapPts(const s_linePoint& p1, const s_linePoint& p2) {
	float Rs = m_plateLayer->p[0].m_RShex;
	return n_line::isIn(p1, p2, Rs);
}
