#pragma once
#ifndef PATTERNL1_H
#define PATTERNL1_H

#ifndef STAMPEYE_H
#include "StampEye.h"
#endif

#define PATTERNL1L0WNUM 7
/*once initiallized the entire web, with the results of the 1st level nnet is constructed
* for each frame this web is triggered
* this leads to larger memory size but nnet does not need to be rerooted and scanned each frame
*/
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
	unsigned char transNNetToPlates();

	unsigned char fullyRoot(s_hexEye& e0, long i);
	unsigned char evalAtRoot(long i_base);/*assumes that the NNet eyes have been fully rooted*/
	float evalNet(s_hexEye& net);

	inline float NNetFunc(float sum) { return Math::StepFuncSym(sum); }
};

namespace n_PatternL1 {
	inline float updateL0Node(s_fNode& curNode) {
		/*since the stamp is trained where only the luna value in the center matterns
		  this will be the only luna value used 
		  center node should be at 0th index in lower node web???check this
		  */
		return curNode.nodes[0]->o;		
	}
};
#endif


