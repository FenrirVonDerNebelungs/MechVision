#pragma once
#ifndef PATTERNL1_H
#define PATTERNL1_H

#ifndef STAMPEYE_H
#include "StampEye.h"
#endif

#define PATTERNL1L0WNUM 7

class PatternL1 : public Base {
public:

	unsigned char init(s_PlateLayer& lunaPlates, HexEye NNetEyes[], int NNets);/*****!!need to get lunaWeights from stamp*/
	void release();
protected:
	/*not owned*/
	HexEye*   m_NNetEyes;/*trained NNets that are set to search for each of the STAMPEYENUM stamps*/
	int       m_numNNets;/*should be STAMPEYENUM*/
	/*owned*/
	s_PlateLayer m_L0Plates;/* each plate in layer corresponds to one of the lunas, reduced to one hex up, with the luna node in the center picked for the luna value*/
	s_PlateLayer m_L1Plates;/*results of first layer of the eyes run, setup with the proper connections before the run*/
	s_PlateLayer m_L2Plates;/*results of the trained nnets run, will be STAMPEYENUM plates actually used*/
	/*     */
	
	unsigned char scan();
	unsigned char updateL0();
	unsigned char transEyeToPlates(s_hexEye& e0, long i);

	unsigned char fullyRoot(s_hexEye& e0, long i);
	unsigned char evalAtRoot(long i_base);/*assumes that the NNet eyes have been fully rooted*/
	float evalNet(s_hexEye& net);

	inline float NNetFunc(float sum) { return Math::StepFuncSym(sum); }
};

namespace n_PatternL1 {
	float L0ToL1(s_fNode& curNode) {
		/*ave over all the input luna  values*/
		float ave_o = 0.f;
		for (long w_i = 0; w_i < PATTERNL1L0WNUM; w_i++)
			ave_o += curNode.w[w_i] * curNode.nodes[w_i]->o;
		curNode.o = ave_o;
		return ave_o;
	}
};
#endif


