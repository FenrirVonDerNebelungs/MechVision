#pragma once
#ifndef PATTERNLUNA4_H
#define PATTERNLUNA4_H
#ifndef NNODEBASE_H
#include "NNodeBase.h"
#endif
#ifndef PATTERNNODE_H
#include "PatternNode.h"
#endif


#define PATTERNLUNA4_NUM 8
#define PATTERNLUNA4_FOOTS 7
#define PATTERNLUNA4_WSCALE 1.f

class PatternLuna4 {
public:
	PatternLuna4();
	~PatternLuna4();

	unsigned char init();
	void release();

	unsigned char eval(s_fNode* footIN[], s_hex* lowHexIN); /*assumes that foot node f's have already been filled*/
	inline s_bNode* getPat() { return (s_bNode*)m_hokPat; }
protected:
	/*these are generated and setup during initization*/
	s_pNode* m_hokPat;
	s_pNode* m_patNodes[PATTERNLUNA4_NUM];
	/*********                                   ******/
	unsigned char GenNodeStructure();
	void genNodePattern(int nodeRot, s_pNode* patOUT);

	unsigned char evalSubLunaPatForFoot(s_fNode* footIN[], s_hex* hokHexIN, s_pNode* lunaPatternOUT);
};
#endif