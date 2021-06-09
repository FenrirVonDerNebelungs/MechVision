#include "stdafx.h"
#include "HexStack.h"
HexStack::HexStack() {
	m_Nlayers = 0;
	for (int i = 0; i < HEXSTACK_STACKN; i++)
		m_layers[i] = NULL;
}
HexStack::~HexStack() {
	;
}
unsigned char HexStack::Update(Img* img)
{
	for (int i = 0; i < m_Nlayers; i++) {
		m_layers[i]->Update(img);
	}
	return ECODE_OK;
}
unsigned char HexStack::Init(Img* img, float Rhex, float sigmaVsR, float IMaskRvsR) {
	Hex* footHex = new Hex;
	if (Err(footHex->Init(img, Rhex, sigmaVsR, IMaskRvsR))) {
		delete footHex;
		return ECODE_FAIL;
	}
	m_layers[0] = (HexBase*)footHex;
	unsigned char err = ECODE_FAIL;
	int stackLev = 1;
	do {
		int prevStackLev = stackLev - 1;
		HokHex* nextHexLayer = new HokHex;
		m_layers[stackLev] = (HexBase*)nextHexLayer;
		err = nextHexLayer->Init(m_layers[prevStackLev]);
		if (Err(err)) {
			nextHexLayer->Release();
			delete nextHexLayer;
			m_layers[stackLev] = NULL;
			continue;
		}
		stackLev++;
	} while (RetOk(err) && (stackLev < HEXSTACK_STACKN));
	m_Nlayers = stackLev;
	if (IsErrFail(err))
		return ECODE_FAIL;
	return ECODE_OK;
}
void HexStack::Release() {
	for (int i = 0; i < m_Nlayers; i++) {
		if (m_layers[i] != NULL) {
			m_layers[i]->Release();
			delete m_layers[i];
			m_layers[i] = NULL;
		}
	}
	m_Nlayers = 0;
}