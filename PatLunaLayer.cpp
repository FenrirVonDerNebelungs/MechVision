#include "PatLunaLayer.h"
PatLunaLayer::PatLunaLayer() : m_hexBase(NULL), m_colLay(NULL), m_patLuna(NULL) {
	for (int i = 0; i < PATLUNALAYER_NUMCOL; i++) {
		PatStruct::zeroPlateLayer(m_plateLayers[i]);
	}
}
PatLunaLayer::~PatLunaLayer() {
	;
}
unsigned char PatLunaLayer::init(HexBase* hexbase) {
	m_hexBase = hexbase;
	m_colLay = new ColLayer;
	m_colLay->init(hexbase);
	for(int i=0; i<PATLUNALAYER_NUMCOL; i++)
		m_colLay->addColPlate();

	m_patLuna = new PatternLuna;
	m_patLuna->init();

	for (int i = 0; i < PATLUNALAYER_NUMCOL; i++) {
		s_hexPlate colPlate = *(m_colLay->getPlate(i));
		m_plateLayers[i].n = 0;
		for (int j = 0; j < PATTERNLUNA_NUM; j++) {/*PATTERNLUNA_NUM must be less than MAXPLATESPERLAYER*/
			initHexPlate(colPlate, m_plateLayers[i].p[j], m_patLuna->getPatNode(j));
			m_plateLayers[i].n++;
		}
	}
	return ECODE_OK;
}
void PatLunaLayer::release() {
	for (int i = 0; i < PATLUNALAYER_NUMCOL; i++) {
		for (int j = 0; j < PATTERNLUNA_NUM; j++) {
			releaseHexPlate(m_plateLayers[i].p[j]);
		}
		m_plateLayers[i].n = 0;
	}
	if (m_patLuna != NULL) {
		m_patLuna->release();
		delete m_patLuna;
	}
	m_patLuna = NULL;
	if (m_colLay != NULL) {
		m_colLay->release();
		delete m_colLay;
	}
	m_colLay = NULL;
	m_hexBase = NULL;
}
unsigned char PatLunaLayer::Update() {
	/*assumes update has already been called for the hexbase layer*/
	m_colLay->Update();
	for (int i = 0; i < PATLUNALAYER_NUMCOL; i++) {
		m_patLuna->run(m_colLay->getPlate(i), m_plateLayers[i]);
	}
	return ECODE_OK;
}
void PatLunaLayer::initHexPlate(s_hexPlate& o, s_hexPlate& n, s_fNode* patNode) {
	PatStruct::genPlateWSameWeb(o, n);
	for (int i = 0; i < o.m_nHex; i++) {
		n.m_fhex[i].shex = o.m_fhex[i].shex;
		n.m_fhex[i].initNodePtrs(7);
		n.m_fhex[i].nodes[6] = &o.m_fhex[i];
		n.m_fhex[i].w[6] = patNode->w[6];
		for (int j = 0; j < 6; j++) {
			n.m_fhex[i].nodes[j] = o.m_fhex[i].web[j];
			n.m_fhex[i].w[j] = patNode->w[j];
		}
		n.m_fhex[i].N = 7;
	}
	/*
	n.m_height = o.m_height;
	n.m_width = o.m_width;
	n.m_Rhex = 2.f * o.m_Rhex;
	n.m_RShex = 2.f * o.m_RShex;
	n.m_Shex = 2.f * o.m_Shex;
	for (int i = 0; i < 6; i++)
		n.m_hexU[i] = o.m_hexU[i];
	n.m_fhex = new s_fNode[o.m_nHex];
	for (int i = 0; i < o.m_nHex; i++) {
		n.m_fhex[i].o = 0.f;
		n.m_fhex[i].thislink = i;
		n.m_fhex[i].shex = o.m_fhex[i].shex;
		n.m_fhex[i].x = o.m_fhex[i].x;
		n.m_fhex[i].y = o.m_fhex[i].y;
		n.m_fhex[i].initNodePtrs(7);
		n.m_fhex[i].nodes[6] = &o.m_fhex[i];
		n.m_fhex[i].w[6] = patNode->w[6];
		for (int j = 0; j < 6; j++) {
			n.m_fhex[i].nodes[j] = o.m_fhex[i].nodes[j];
			n.m_fhex[i].w[j] = patNode->w[j];
		}
		n.m_fhex[i].N = 7;
	}
	n.m_nHex = o.m_nHex;
	*/
}
void PatLunaLayer::releaseHexPlate(s_hexPlate& p) {
	if (p.m_fhex != NULL) {
		for (int i = 0; i < p.m_nHex; i++) {
			p.m_fhex[i].releaseNodePtrs();
		}
		delete[] p.m_fhex;
	}
	p.m_fhex = NULL;
	PatStruct::zeroHexPlate(p);
}