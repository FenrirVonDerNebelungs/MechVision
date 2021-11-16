#include "ColLayer.h"

ColLayer::ColLayer() : m_hex(NULL), m_hexes(NULL), m_nHexes(0), m_pixMax(0.f){
	for (int i = 0; i < MAXPLATESPERLAYER; i++) {
		m_colWheels[i] = NULL;
	}
	PatStruct::zeroPlateLayer(m_plates);
}
ColLayer::~ColLayer() {
	;
}
unsigned char ColLayer::init(HexBase* hex) {
	m_hex = hex;
	m_hexes = hex->getHexes();
	m_nHexes = hex->getNHex();
	m_pixMax = 255.f;
	m_plates.n = 0;
	return ECODE_OK;
}
unsigned char ColLayer::addColPlate(
	float Dhue,
	float DI,
	float DSat,
	float hueFadeV,
	float I_target,
	float hue_target_x,
	float hue_target_y,
	float Sat_target,
	float stepSteepness
) {
	if (m_plates.n >= MAXPLATESPERLAYER)
		return ECODE_ABORT;
	m_colWheels[m_plates.n] = new ColWheel;
	m_colWheels[m_plates.n]->Init(m_pixMax, Dhue, DI, DSat, hueFadeV, I_target, hue_target_x, hue_target_y, Sat_target, stepSteepness);
	m_hex->genStructuredPlate(m_plates.p[m_plates.n]);
	m_plates.n++;
	return ECODE_OK;
}
void ColLayer::release() {
	for (int i = 0; i < MAXPLATESPERLAYER; i++) {
		m_hex->releaseStructuredPlate(m_plates.p[i]);
	}
	for (int i = 0; i < MAXPLATESPERLAYER; i++) {
		if (m_colWheels[i] != NULL) {
			m_colWheels[i]->Release();
			delete m_colWheels[i];
		}
		m_colWheels[i] = NULL;
	}
	m_nHexes = 0;
	m_hexes = NULL;
	m_hex = NULL;
	m_plates.n = 0;
}
unsigned char ColLayer::Update() {
	for (int ipl = 0; ipl < m_plates.n; ipl++) {
		for (int i = 0; i < m_plates.p[ipl].m_nHex; i++) {
			float o = m_colWheels[ipl]->evalRGB(m_hexes[i].rgb);
			m_plates.p[ipl].m_fhex[i].o = o;
		}
	}
	return ECODE_OK;
}