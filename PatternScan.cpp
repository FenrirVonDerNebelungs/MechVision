#include "stdafx.h"
#include "PatternScan.h"

PatternScan::PatternScan() : m_hexStack(NULL),
m_footNodes(NULL), m_nfootNodes(0), m_nedgefs(0), m_edgefs(NULL),
m_colWheel(NULL), m_patternLuna(NULL)
{
	;
}
PatternScan::~PatternScan()
{
	;
}

unsigned char PatternScan::init(HexStack* stack) {
	m_hexStack = stack;

	HexBase* hexLayer = m_hexStack->getLayer(0);
	int numHexes = hexLayer->getNHex();

	m_footNodes = new s_fNode * [numHexes];
	for (int i = 0; i < numHexes; i++)
		m_footNodes[i] = new s_fNode;
	m_nfootNodes = numHexes;

	m_nedgefs = 0;
	m_edgefs = new s_luna4fs[m_nfootNodes];
	for (int i = 0; i < m_nfootNodes; i++) {
		m_edgefs[i].edgef = -1.f;
		m_edgefs[i].lineNodei = -1;
	}

	m_colWheel = new NNetTestColwheel;
	m_colWheel->Init();
	m_patternLuna = new PatternLuna4;
	m_patternLuna->init();

	attachFootNodes();

	return ECODE_OK;
}
void PatternScan::release() {
	if (m_patternLuna != NULL) {
		m_patternLuna->release();
		delete m_patternLuna;
	}
	m_patternLuna = NULL;
	if (m_colWheel != NULL) {
		m_colWheel->Release();
		delete m_colWheel;
	}
	m_colWheel = NULL;
	if (m_edgefs != NULL)
		delete m_edgefs;
	m_edgefs = NULL;
	m_nedgefs = 0;
	if (m_footNodes != NULL) {
		for (int i = 0; i < m_nfootNodes; i++) {
			if (m_footNodes[i] != NULL)
				delete m_footNodes[i];
		}
		delete m_footNodes;
		m_footNodes = NULL;
	}
	m_footNodes = 0;
	m_hexStack = NULL;
}

unsigned char PatternScan::eval() {
	evalFootNodes();

	HexBase* hexLayer = m_hexStack->getLayer(0);
	s_hex* hexes = hexLayer->getHexes();
	int numHexs = hexLayer->getNHex();

	/*convul sweep accross the entire low hex array*/
	for (int i_layer0 = 0; i_layer0 < numHexs; i_layer0++) {
		if (hexes[i_layer0].thisLink < 0) {
			putdummyIntoFs();
			continue;
		}
		m_patternLuna->eval(m_footNodes, &(hexes[i_layer0]));
		s_pNode* curNode = (s_pNode*)m_patternLuna->getPat();
		putLunaNodeIntoFs(curNode);
	}
	return ECODE_OK;
}

unsigned char PatternScan::attachFootNodes() {
	HexBase* hexLayer = m_hexStack->getLayer(0);
	s_hex* hexes = hexLayer->getHexes();
	/*number of hexes is the same as number of foot nodes*/
	int numHexes = hexLayer->getNHex();
	for (int i = 0; i < numHexes; i++) {
		m_footNodes[i]->shex = &(hexes[i]);
		m_footNodes[i]->f = 0.f;
	}
	return ECODE_OK;
}
unsigned char PatternScan::evalFootNodes() {
	for (int i = 0; i < m_nfootNodes; i++) {
		m_footNodes[i]->f = m_colWheel->evalRGB(m_footNodes[i]->shex->rgb);
	}
	return ECODE_OK;
}

unsigned char PatternScan::putLunaNodeIntoFs(s_pNode* node) {
	m_edgefs[m_nedgefs].edgef = node->f;
	for (int i = 0; i < PATTERNLUNA4_NUM; i++) {
		m_edgefs[m_nedgefs].f[i] = node->nodes[i]->f;
	}
	m_nedgefs++;
	return ECODE_OK;
}
unsigned char PatternScan::putdummyIntoFs() {
	m_edgefs[m_nedgefs].edgef = -1.f;
	for (int i = 0; i < PATTERNLUNA4_NUM; i++) {
		m_edgefs[m_nedgefs].f[i] = -1.f;
	}
	m_nedgefs++;
	return ECODE_OK;
}