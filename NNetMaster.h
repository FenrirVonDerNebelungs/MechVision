#pragma once
#ifndef NNETMASTER_H
#define NNETMASTER_H

#ifndef PATTERNL1_H
#include "PatternL1.h"
#endif
#ifndef NNETTRAIN_H
#include "NNetTrain.h"
#endif

/*
this class is currently independent of the other 'master' class RoachNet_Client
class mostly just for testing of nnets. Once nets work will derive means to write
results and then read in without regenerating on the same computer that actually
runs the nets for image processing
*/
class NNetMaster : public Base{
public:
	NNetMaster();
	~NNetMaster();

	unsigned char init();
	void release();
	unsigned char trainNets();
	
	inline s_hexEye* getNets() { return m_NNetsL1->getEyes(); }
	inline int getNumNets() { return m_numNets; }
protected:
	/* owned */
	PatternLuna* m_patLuna;
	StampEye* m_stampEyeL1;
	HexEye* m_NNetsL1;
	NNetTrain* m_trainL1;

	int m_numNets;
};

#endif