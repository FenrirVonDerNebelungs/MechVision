#pragma once
#ifndef PATLUNALAYER_H
#define PATLUNALAYER_H

#ifndef PATTERNLUNA_H
#include "PatternLuna.h"
#endif
#ifndef COLLAYER_H
#include "ColLayer.h"
#endif

#define PATLUNALAYER_NUMCOL 1
/*******************************************
*********************************************
* NEED TO SETUP WEB for s_fNode Plates!!!!!!!!!!!!!!!
*/

class PatLunaLayer : public Base {
public:
	PatLunaLayer();
	~PatLunaLayer();

	unsigned char init(HexBase* hexbase);
	void release();

	unsigned char Update();

	inline s_PlateLayer* getPlateLayers() { return m_plateLayers; }
	inline s_PlateLayer& getPlateLayer(int i) { return m_plateLayers[i]; }
	inline s_hexPlate& getPlate(int i_plate, int i_layer) { return m_plateLayers[i_plate].p[i_layer]; }
	inline s_hexPlate* getColPlate(int i_plate) { return m_colLay->getPlate(i_plate); }
protected:
	/*not owned*/
	HexBase* m_hexBase;

	/* owned */
	ColLayer* m_colLay;
	PatternLuna* m_patLuna;
	s_PlateLayer m_plateLayers[PATLUNALAYER_NUMCOL];

	void initHexPlate(s_hexPlate& o, s_hexPlate& n, s_fNode* patNode);
	void releaseHexPlate(s_hexPlate& p);
};
#endif