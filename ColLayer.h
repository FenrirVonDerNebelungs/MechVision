#pragma once
#ifndef COLLAYER_H
#define COLLAYER_H
#ifndef HEXBASE_H
#include "HexBase.h"
#endif
#ifndef COLWHEEL_H
#include "ColWheel.h"
#endif





class ColLayer : public Base {
public:
	ColLayer();
	~ColLayer();

	unsigned char init(HexBase* hex);
	unsigned char addColPlate(
		float Dhue = 3.f,
		float DI = 0.8f,//0.3f
		float DSat = 0.5f,//3f,//0.5f,
		float hueFadeV = 0.3f,
		float I_target = 1.0f,//0.9f,
		float hue_target_x = 1.f,
		float hue_target_y = 0.f,
		float Sat_target = 0.f,
		float stepSteepness = 7.f
	);
	void release();

	unsigned char Update();

	inline s_PlateLayer* getPlates() { return &m_plates; }
	inline s_hexPlate* getPlate(int i) { return &(m_plates.p[i]); };
	inline int getNPlates() { return m_plates.n; }

	inline s_hex* getBaseHexes() { return m_hexes; }
protected:
	/*not ownned*/
	HexBase* m_hex;
	s_hex* m_hexes;
	long   m_nHexes;
	/*          */
	/* owned    */
	float m_pixMax;
	ColWheel* m_colWheels[MAXPLATESPERLAYER];
	s_PlateLayer m_plates;
	/*          */
};

#endif
