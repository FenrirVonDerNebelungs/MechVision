#pragma once
#ifndef PATTERNL1_H
#define PATTERNL1_H

#ifndef STAMPEYE_H
#include "StampEye.h"
#endif

#define PATTERNL1L0WNUM 7

class PatternL1 : public Base {
public:

	unsigned char init(s_PlateLayer& lunaPlates, HexEye NNetEyes[], int NNets);
	void release();
protected:
	/*not owned*/
	HexEye*   m_NNetEyes;/*trained NNets that are set to search for each of the STAMPEYENUM stamps*/
	int       m_numNNets;/*should be STAMPEYENUM*/
	/*owned*/
	s_PlateLayer m_L0Plates;/*reduced to one hex up, with the luna node in the center picked for the luna value*/
	s_PlateLayer m_L1Plates;/*results of the trained nnets run, will be STAMPEYENUM plates actually used*/
	/*     */
	
	unsigned char scan();
	unsigned char updateL0();
	unsigned char fullyRoot(s_hexEye& e0, long i);
	unsigned char evalAtRoot(long i_base);/*assumes that the NNet eyes have been fully rooted*/
	float evalNet(s_hexEye& net);

	inline float NNetFunc(float sum) { return Math::StepFuncSym(sum); }
};
#endif


