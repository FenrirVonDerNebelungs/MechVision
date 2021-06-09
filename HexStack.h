#pragma once
#ifndef HEXSTACK_H
#define HEXSTACK_H
#ifndef HOKHEX_H
#include "HokHex.h"
#endif

#define HEXSTACK_STACKN 10

class HexStack : public Base{
public:
	HexStack();
	~HexStack();

	unsigned char Init(
		Img* img,
		float Rhex = 3.f,
		float sigmaVsR = 0.75f,
		float IMaskRVsR = 1.5f
	);
	void Release();

	inline HexBase* getLayer(int layerLevel) { return (layerLevel<m_Nlayers) ? m_layers[layerLevel] : NULL; }
	unsigned char Update(Img* img);
protected:
	HexBase* m_layers[HEXSTACK_STACKN];
	int      m_Nlayers;
};

#endif

