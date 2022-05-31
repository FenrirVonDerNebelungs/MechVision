#pragma once
#ifndef NNETMASTER_H
#define NNETMASTER_H

#ifndef PATTERNL1_H
#include "PatternL1.h"
#endif

class NNetMaster {
public:
	NNetMaster();
	~NNetMaster();


protected:
	/* owned */
	HexEye* m_HexEyeL1;
	StampEye* m_stampEyeL1;
	PatternL1* m_patL1;


};

#endif