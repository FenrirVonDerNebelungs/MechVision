#include "HexEye.h"

HexEye::HexEye() :m_r(0.f), m_rs(0.f), m_R(0.f), m_Rs(0.f), 
m_imgWidth(0), m_imgHeight(0), m_N_levels(0),
m_N_eyes(0)
{
	for (int i = 0; i < 6; i++) {
		m_hexU[i].x0 = 0.f;
		m_hexU[i].x1 = 0.f;
	}
	for (int i = 0; i < HEXEYE_MAXEYES; i++) {
		m_eye[i].n = 0;
		for (int j = 0; j < HEXEYE_MAXLEVELS; j++) {
			PatStruct::zeroHexPlate(m_eye[i].lev[j]);
		}
	}
}
HexEye::~HexEye() {
	;
}
unsigned char HexEye::init(HexBase* lowHexes) {
	m_N_eyes = 0;
	setHexUs();
	m_r = lowHexes->getRhex();
	m_rs = lowHexes->getRShex();
	m_imgWidth = lowHexes->getImg()->getWidth();
	m_imgHeight = lowHexes->getImg()->getHeight();

	return ECODE_OK;
}
unsigned char HexEye::init(float r) {
	m_N_eyes = 0;
	m_r = r;
	m_rs = r * sqrt(3.f) / 2.f;
	setHexUs();
}
unsigned char HexEye::init(float r, int NLevels) {
	HexEye::init(r);
	if (RetOk(spawn(NLevels))) {
		m_imgHeight = (long)ceilf(m_R * 2.f);
		m_imgWidth = m_imgHeight;
	}
	else
		return ECODE_FAIL;
}
unsigned char HexEye::spawn() {
	int NLevels = setRfromImg();
	return spawn(NLevels);
}
unsigned char HexEye::spawn(int NLevels) {
	m_N_levels = NLevels;
	if (m_N_levels >= HEXEYE_MAXLEVELS || m_N_levels<1)
		return ECODE_FAIL;
	float fac = Math::power(2.f, m_N_levels - 1);
	m_R = fac * m_r;
	m_Rs = fac * m_rs;

	return spawnEye();
}
void HexEye::release() {
	releaseEyes();
}
void HexEye::releaseEyes() {
	for (int i = 0; i < HEXEYE_MAXEYES; i++) {
		releaseLevels(m_eye[i]);
	}
}

unsigned char HexEye::spawnEye() {
	if (m_N_eyes >= HEXEYE_MAXEYES)
		return ECODE_ABORT;
	if (RetOk(initLevels())) {
		if (RetOk(genPatternsForLevels(m_eye[m_N_eyes].lev)))
			m_eye[m_N_eyes].n = m_N_levels;
		else
			return ECODE_FAIL;
		m_N_eyes++;
	}
	else
		return ECODE_FAIL;
	return ECODE_OK;
}
void HexEye::setHexUs()
{
	float longs = sqrtf(3.f) / 2.f;
	float shorts = 0.5f;
	/*start with to the right*/
	m_hexU[0].x0 = 1.f;
	m_hexU[0].x1 = 0.f;

	m_hexU[1].x0 = shorts;
	m_hexU[1].x1 = longs;

	m_hexU[2].x0 = -shorts;
	m_hexU[2].x1 = longs;

	m_hexU[3].x0 = -1.f;
	m_hexU[3].x1 = 0.f;

	m_hexU[4].x0 = -shorts;
	m_hexU[4].x1 = -longs;

	m_hexU[5].x0 = shorts;
	m_hexU[5].x1 = -longs;
}
int HexEye::setRfromImg() {
	if (m_imgWidth <= 0.f || m_imgHeight <= 0.f)
		return ECODE_FAIL;
	float minDim = (float)m_imgHeight;
	if (m_imgWidth < m_imgHeight) {
		minDim = (float)m_imgWidth;
	}
	minDim /= 2.f;
	float Rmax = minDim / m_rs;
	float n = logf(Rmax) / logf(2.0f);
	if (n < 1)
		return ECODE_FAIL;
	int N = (int)floorf(n);
	m_N_levels = N + 1;
	if (m_N_levels >= HEXEYE_MAXLEVELS)
		return -1;
	float mulFac = Math::power(2.f, N);
	m_Rs = mulFac * m_rs;
	m_R = mulFac * m_r;
	return m_N_levels;
}

unsigned char HexEye::initLevels() {
	/*roughly calculated the number of hexes in a level from the total area*/
	float radius = 0.f;
	float cur_R = m_R;
	for (int i = 0; i < m_N_levels; i++) {
		m_eye[m_N_eyes].lev[i].m_Rhex = cur_R;
		m_eye[m_N_eyes].lev[i].m_RShex = cur_R * sqrtf(3.f)/2.f;
		m_eye[m_N_eyes].lev[i].m_Shex = cur_R;
		for (int U_i = 0; U_i < 6; U_i++)
			m_eye[m_N_eyes].lev[i].m_hexU[U_i] = m_hexU[U_i];

		float i_ = (float)i;
		radius += cur_R;
		float rato = radius / cur_R;
		float N_in_tri = rato * rato;
		float N_min = 6.f * N_in_tri;
		int N = (int)ceilf(N_min);
		m_eye[m_N_eyes].lev[i].m_fhex = new s_fNode[N];
		for (int j = 0; j < N; j++) {
			initNode(m_eye[m_N_eyes].lev[i].m_fhex[j], j);
		}
		m_eye[m_N_eyes].lev[i].m_nHex = 0;

		cur_R /= 2.f;
	}
	return ECODE_OK;
}
void HexEye::releaseLevels(s_hexEye& eye) {
	for (int i = 0; i < HEXEYE_MAXLEVELS; i++) {
		if (eye.lev[i].m_fhex != NULL) {
			for (int nd_i = 0; nd_i < eye.lev[i].m_nHex; nd_i++) {
				eye.lev[i].m_fhex[nd_i].releaseWebPtrs();
				eye.lev[i].m_fhex[nd_i].releaseNodePtrs();
			}
			delete[] eye.lev[i].m_fhex;
		}
		eye.lev[i].m_fhex = NULL;
		eye.lev[i].m_nHex = 0;
	}
	eye.n = 0;
}
void HexEye::initNode(s_fNode& h, int nd_i) {
	h.x = 0.f;
	h.y = 0.f;
	h.thislink = nd_i;
	h.initNodePtrs(7);/*7th node is center node*/
	h.initWebPtrs(6);
}
unsigned char HexEye::genPatternsForLevels(s_hexPlate* levels) {
	levels[0].m_fhex[0].x = 0.f;
	levels[0].m_fhex[0].y = 0.f;
	levels[0].m_fhex[0].thislink = 0;
	levels[0].m_nHex = 1;
	float curRs = levels[0].m_RShex;
	int numToGen = m_N_levels - 1;
	for (int i = 0; i < numToGen; i++) {
		curRs /= 2.f;
		int indx = 0;
		/*loop over all the hexes in the current level and generate the hexes for the next lower level*/
		for (int i_top = 0; i_top < levels[i].m_nHex; i_top++) {
			int sub_hex_start_indx = indx;
			s_2pt loc = { levels[i].m_fhex[i_top].x, levels[i].m_fhex[i_top].y };
			s_2pt_i nebi[HEXEYE_MAXNEBINDXS];
			int num_neb = collectNebIndexes(levels[i], i_top, nebi);
			genLowerPattern(levels[i + 1].m_fhex, curRs, loc, nebi, num_neb, indx);/*index advances to final pattern filled but not beyond*/
			indx++;/*get ready to fill the next one*/
			levels[i].m_fhex[i_top].nodes[6] = (s_bNode*)&(levels[i+1].m_fhex[sub_hex_start_indx]);
			for (int down_i = 0; down_i < 6; down_i++) {
				levels[i].m_fhex[i_top].nodes[down_i] = levels[i + 1].m_fhex[sub_hex_start_indx].web[down_i];
			}
		}
		levels[i + 1].m_nHex = indx;
	}
	return ECODE_OK;
}
int HexEye::collectNebIndexes(s_hexPlate& topLevel, int curTopIndex, s_2pt_i nebi[]) {
	int numi = 0;
	for (int i_webtop = 0; i_webtop < 6; i_webtop++) {
		s_fNode* TopNd = (s_fNode*)topLevel.m_fhex[curTopIndex].web[i_webtop];
		if(TopNd!=NULL){
			int rev_webtopi = Math::loop(i_webtop + 3, 6);
			/*check if down links for this top hex have been generated*/
			if (TopNd->nodes[6]!=NULL){
				/*if the centerlink has been generated then so to have the web links,
				which means the shared hex is already filled*/
				nebi[numi].x0 = i_webtop;//index direction from new top to half overlaped hex below
				nebi[numi].x1 = TopNd->nodes[rev_webtopi]->thislink;//this is the index of the node in the bottom that is shared half in the new top
				numi++;
			}
		}
	}
	return numi;
}
int HexEye::getNebLevIndex(s_2pt_i nebi[], int num_neb, int web_i) {
	int indxFound = foundInAr(nebi, num_neb, web_i);
	if (indxFound < 0)
		return -1;
	return nebi[indxFound].x1;
}
unsigned char HexEye::genLowerPattern(s_fNode* lhxs, float Rs, s_2pt& loc, s_2pt_i nebi[], int num_neb,  int& indx) {
	/*
	lhxs is this level of hexes,
	Rs is the current hex short radius
	loc is the center of this 7 pack pattern
	indx is the index of the current center in its level
	*/
	// lhxs=m_levels[m_N_levels].m_hexes;
	lhxs[indx].x = loc.x0;
	lhxs[indx].y = loc.x1;
	lhxs[indx].thislink = indx;
	int cindx = indx;
	for (int i = 0; i < 6; i++) {
		int nebLev_i = getNebLevIndex(nebi, num_neb, i);
		int oplink = Math::loop(i + 3, 6);
		if (nebLev_i>=0 ){
			/*this neighbor is alredy generated*/
			lhxs[nebLev_i].web[oplink] = &(lhxs[cindx]);
			lhxs[cindx].web[i] = &(lhxs[nebLev_i]);
		}
		else {/*only fill if these have not previously been filled*/
			indx++;/*add new hex*/
			lhxs[indx].x = m_hexU[i].x0 * Rs + loc.x0;
			lhxs[indx].y = m_hexU[i].x1 * Rs + loc.x1;
			lhxs[indx].thislink = indx;

			lhxs[indx].web[oplink] = &(lhxs[cindx]);
			lhxs[cindx].web[i] = &(lhxs[indx]);
		}
	}
	weaveRound(lhxs[cindx]);
	for (int i = 0; i < 6; i++) {
		int nebLev_i = getNebLevIndex(nebi, num_neb, i);
		if (nebLev_i >= 0)
			weaveRound(lhxs[nebLev_i]);
	}
	return ECODE_OK;
}
unsigned char HexEye::weaveRound(s_fNode& lhx) {
	for (int i = 0; i < 6; i++) {
		s_bNode* prev_nd = lhx.web[Math::loop(i - 1, 6)];
		s_fNode* cur_nd = (s_fNode*)lhx.web[i];
		s_bNode* next_nd = lhx.web[Math::loop(i + 1, 6)];
		cur_nd->web[Math::loop(4 + i, 6)] = prev_nd;
		cur_nd->web[Math::loop(2 + i, 6)] = next_nd;
	}
	return ECODE_OK;
}

unsigned char HexEye::rootOn(s_hexEye& eye, s_hexPlate& lowHexes, long center_i) {
	/*assume that center_i is inside of hex map*/
	s_hexPlate* levels = eye.lev;
	s_fNode* ehx = levels[m_N_levels - 1].m_fhex;
	s_fNode* fhx = lowHexes.m_fhex;

	int next_web_i = 3;
	s_fNode* pat_nd = &(ehx[0]);
	s_fNode* low_nd = &(fhx[center_i]);
	do {
		pat_nd = runLine(pat_nd, low_nd, next_web_i);
		if (pat_nd != NULL)
			next_web_i = turnCorner(&pat_nd, &low_nd, 3, 0);
		else
			break;
	} while (next_web_i >= 0);

	next_web_i = 0;
	pat_nd = &(ehx[0]);
	low_nd = &(fhx[center_i]);
	do {
		pat_nd = runLine(pat_nd, low_nd, next_web_i);
		if (pat_nd != NULL)
			turnCorner(&pat_nd, &low_nd, 0, 3);
		else
			break;
	} while (next_web_i >= 0);

	return ECODE_OK;
}
int HexEye::rotateCLK(s_fNode* pat_hex, int strt_i) {
	int web_i = -1;
	s_fNode* ndPtr=NULL;
	bool found = false;
	/*rotate at end to swap back*/
	for (int i = 0; i < 3; i++) {
		web_i = strt_i - i;
		if (web_i < 0)
			web_i += 6;
		ndPtr = (s_fNode*)pat_hex->web[web_i];
		if (ndPtr !=NULL ) {
			found = true;
			break;
		}
	}
	return (found) ? web_i : -1;
}
int HexEye::rotateCCLK(s_fNode* pat_hex, int strt_i) {
	int web_i = -1;
	s_fNode* ndPtr = NULL;
	bool found = false;
	/*rotate at end to swap back*/
	for (int cnt = 0; cnt < 3; cnt++) {
		web_i = strt_i + cnt;
		/*only values for strt i will be 0 and 3 so don't need a check for >=6*/
		ndPtr = (s_fNode*)pat_hex->thislink;
		if (ndPtr !=NULL) {
			found = true;
			break;
		}
	}
	return (found) ? web_i : -1;
}
s_fNode* HexEye::runLine(s_fNode* pat_hex, s_fNode* low_hex, int next_web_i) {
	int hex_i = -1;
	s_fNode* next_hex_nd = pat_hex;
	s_fNode* next_low_nd = low_hex;
	do {
		pat_hex = next_hex_nd;
		pat_hex->nodes[0] = (s_bNode*)low_hex; 
		pat_hex->N = 1;
		/*advance*/
		next_hex_nd = (s_fNode*)pat_hex->web[next_web_i];
		next_low_nd = (s_fNode*)low_hex->web[next_web_i];
	} while (next_hex_nd != NULL && next_low_nd != NULL);
	if (next_hex_nd != NULL && next_low_nd == NULL)
		return NULL;
	return pat_hex;
}
int HexEye::turnCorner(s_fNode** pat_node, s_fNode** low_hex, int fwd_web_i, int rev_web_i) {
	int next_web_i = -1;
	if (next_web_i == fwd_web_i) {
		next_web_i = rotateCLK(*pat_node, fwd_web_i);
		if (next_web_i >= 0) {
			*pat_node = (s_fNode*)(*pat_node)->nodes[next_web_i];
			*low_hex = (s_fNode*)(*low_hex)->nodes[next_web_i];
			next_web_i = ((*low_hex)!=NULL) ? 0 : -1;
		}
	}
	if (next_web_i == rev_web_i) {
		next_web_i = rotateCCLK(*pat_node, rev_web_i);
		if (next_web_i >= 0) {
			*pat_node = (s_fNode*)(*pat_node)->nodes[next_web_i];
			*low_hex = (s_fNode*)(*low_hex)->nodes[next_web_i];
			next_web_i = ((*low_hex)!=NULL) ? 3 : -1;
		}
	}
	return next_web_i;
}
int HexEye::foundInAr(s_2pt_i ar[], int n, int val) {
	for (int i = 0; i < n; i++) {
		if (ar[i].x0 == val)
			return i;
	}
	return -1;
}