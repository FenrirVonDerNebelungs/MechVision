#include "stdafx.h"
#include "NNetTree.h"
NNetTreeTest::NNetTreeTest():m_topHexi(0), m_nnetTestGen(NULL), m_hexMaster(NULL), 
m_colWheel(NULL), m_wrapperNodes(NULL), m_patternNodes(NULL), m_midNodes(NULL), 
m_N_wrapperNodes(0), m_N_patternNodes(0), m_N_midNodes(0)
{
	;
}
NNetTreeTest::~NNetTreeTest() {
	;
}

void NNetTreeTest::run() {
	evalLevel0();
	evalhigherLevels();
}
unsigned char NNetTreeTest::Init(HexStack* hexMaster, int topHexi) {
	m_hexMaster = hexMaster;
	m_nnetTestGen = new NNetTestGen;
	m_colWheel = new NNetTestColwheel;
	m_wrapperNodes = new s_nWrapperNode[49];
	m_patternNodes = new s_nNode[140];
	m_midNodes = new s_nNode[NUMMIDNODES];
	m_N_wrapperNodes = 49; 
	m_N_patternNodes = 140;
	m_N_midNodes = 1;
	for (int i = 0; i < 49; i++) {
		m_wrapperNodes[i].nodes = NULL;
		m_wrapperNodes[i].w = NULL;
		m_wrapperNodes[i].N = 0;
		m_wrapperNodes[i].f = 0.5f;
		m_wrapperNodes[i].hnodes = new s_nNode * [NUMPATNODES];/*each wrapper node has 19 high  nodes*/
		m_wrapperNodes[i].hN = NUMPATNODES;
	}
	for (int i = 0; i < 140; i++) {
		m_patternNodes[i].nodes = new s_nNode * [7];
		m_patternNodes[i].w = new float[7];
		m_patternNodes[i].N = 7;
		m_patternNodes[i].f = 0.5f;
		m_patternNodes[i].hnodes = new s_nNode * [NUMMIDNODES];
		m_patternNodes[i].hN = NUMMIDNODES;
	}
	for (int i = 0; i < NUMMIDNODES; i++) {
		m_midNodes[i].nodes = new s_nNode * [140];
		m_midNodes[i].w = new float[140];
		m_midNodes[i].N = 140;
		m_midNodes[i].f = 0.5f;
	}
	m_nnetTestGen->fillUnattachedTree(m_midNodes, m_patternNodes, m_wrapperNodes);
	m_colWheel->Init();
	AttachTreeToHexImg();
	return ECODE_OK;
}
void NNetTreeTest::Release() {
	if (m_nnetTestGen != NULL) {
		m_nnetTestGen->releaseUnattachedTree();
		delete m_nnetTestGen;
	}
	m_nnetTestGen = NULL;
	if (m_colWheel != NULL) {
		m_colWheel->Release();
		delete m_colWheel;
	}
	m_colWheel = NULL;

	if (m_midNodes != NULL) {
		for (int i = 0; i < NUMMIDNODES; i++) {
			if (m_midNodes[i].nodes != NULL)
				delete [] (m_midNodes[i].nodes);
			if (m_midNodes[i].w != NULL)
				delete[](m_midNodes[i].w);
		}
		delete[] m_midNodes;
		m_midNodes = NULL;
		m_N_midNodes = 0;
	}
	if (m_patternNodes != NULL) {
		for (int i = 0; i < 140; i++) {
			if (m_patternNodes[i].nodes != NULL)
				delete[](m_patternNodes[i].nodes);
			if (m_patternNodes[i].w != NULL)
				delete[](m_patternNodes[i].w);
			if (m_patternNodes[i].hnodes != NULL)
				delete[](m_patternNodes[i].hnodes);
		}
		delete[] m_patternNodes;
		m_patternNodes = NULL;
		m_N_patternNodes = 0;
	}
	if (m_wrapperNodes != NULL) {
		for (int i = 0; i < 49; i++) {
			if (m_wrapperNodes[i].nodes != NULL)
				delete[](m_wrapperNodes[i].nodes);
			if (m_wrapperNodes[i].w != NULL)
				delete[](m_wrapperNodes[i].w);
			if (m_wrapperNodes[i].hnodes != NULL)
				delete[](m_wrapperNodes[i].hnodes);
		}
		delete[] m_wrapperNodes;
		m_wrapperNodes = NULL;
		m_N_wrapperNodes = 0;
	}

	m_hexMaster = NULL;
}
void NNetTreeTest::setMidNodeWeights(int i, float ws[]){
	int N = m_midNodes[i].N;
        for(int j=0; j<N; j++){
		m_midNodes[i].w[j] = ws[j];
	}
}
float NNetTreeTest::NNetFunction(float nin) {
	/*1/(1+exp(-n))*/
	return Math::StepFunc(nin);
}
float NNetTreeTest::evalWeightedNs(int num, float ws[], float ns[])
{
	float sum = 0.f;
	for (int i = 0; i < num; i++) {
		sum += ws[i] * ns[i];
	}
	return sum;
}


void NNetTreeTest::AttachTreeToHexImg()
{
	AttachTreeToLevel2Hex(m_topHexi);
}
void NNetTreeTest::AttachTreeToLevel2Hex(int tophexi) {
	HexBase* hexLayer = m_hexMaster->getLayer(2);
	s_hex topHex = hexLayer->getHex(tophexi);


	int mid7indexes[7];
	mid7indexes[6] = topHex.centerLink;
	for (int i = 0; i < 6; i++) {
		mid7indexes[i] = topHex.downLinks[i];
	}
	s_nWrapperNode* pWrappers[7];
	for (int i = 0; i < 7; i++) {
		int wrapIndexStrt = i * 7;
		for (int wrpi = 0; wrpi < 7; wrpi++) {
			int curi = wrapIndexStrt + wrpi;
			pWrappers[wrpi] = &(m_wrapperNodes[curi]);
		}
		fillWrappers(mid7indexes[i], pWrappers);
	}
}
void NNetTreeTest::fillWrappers(int firstHokHexi, s_nWrapperNode* pWrappers[])
{
	HexBase* lowestHokHexLayer = m_hexMaster->getLayer(1);
	s_hex curHokHex = lowestHokHexLayer->getHex(firstHokHexi);
	for (int i = 0; i < 6; i++) {
		int webi = curHokHex.downLinks[i];
		pWrappers[i]->shex = getHexPtrFromIndex(webi);
	}
	int webcenteri = curHokHex.centerLink;
	pWrappers[6]->shex = getHexPtrFromIndex(webcenteri);
}

s_hex* NNetTreeTest::getHexPtrFromIndex(int origWebi) {
	HexBase* hexes = m_hexMaster->getLayer(0);
	s_hex* hexArray = hexes->getHexes();
	s_hex* specificHex = &(hexArray[origWebi]);
	return specificHex;
}

void NNetTreeTest::EvalWeights(s_nNode& node) {
	float culmSum = 0.f;
	for (int i = 0; i < node.N; i++) {
		float curW = node.w[i];
		s_nNode* curLowerNode = node.nodes[i];
		float cur_nin = curLowerNode->f;
		culmSum += curW * cur_nin;
	}
	node.f = NNetFunction(culmSum);
}
void NNetTreeTest::evalLevel0() {
	for (int i = 0; i < m_N_wrapperNodes; i++) {
		s_hex* curhex = m_wrapperNodes[i].shex;
		float rgb[3];
		for (int j = 0; j < 3; j++)
			rgb[j] = curhex->rgb[j];
		float baseColEval = m_colWheel->evalRGB(rgb);
		m_wrapperNodes[i].f = baseColEval;
	}
}
void NNetTreeTest::evalhigherLevels() {
	for (int i = 0; i < m_N_patternNodes; i++) {
		EvalWeights(m_patternNodes[i]);
		/*debug*/
		float tempf = m_patternNodes[i].f;
		int ttt=0;
		/*     */
	}
	for (int i = 0; i < m_N_midNodes; i++) {
		EvalWeights(m_midNodes[i]);
	}
}