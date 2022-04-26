#pragma once
#ifndef PATTERNLUNA_H
#define PATTERNLUNA_H
#ifndef BASE_H
#include "Base.h"
#endif
#ifndef PATTERNNODE_H
#include "PatternNode.h"
#endif

#define PATTERNLUNA_NUM 8 //Number of luna patterns 6 round and 2 full
#define PATTERNLUNA_FOOTS 7
#define PATTERNLUNA_WSCALE 60.f //1.f

class PatternLuna : public Base {
public:
	PatternLuna();
	~PatternLuna();

	unsigned char init();
	void release();

	unsigned char run(s_hexPlate* inp, s_PlateLayer& outLay);
	inline s_fNode* getPatNode(int i) { return m_lev0[i]; }
protected:
	s_fNode* m_lev0[PATTERNLUNA_NUM];/*lower nodes for unattached pattern*/
	s_fNode* m_lev1;/*top node for unattached pattern*/

	/*initialization*/
	unsigned char GenNodeStructure();
	void genNodePattern(int nodeRot, s_fNode* patOUT);
	/****************/

};

namespace PatternLunaThreaded {
	void runPlate(s_fNode& lev0, s_hexPlate& baseplate, s_hexPlate& plate);
	void evalAtPlateNode(s_fNode& lev0, s_fNode& plateNode);
	void evalLowerNode(s_fNode& lev0, s_fNode& colNode);
}
#endif