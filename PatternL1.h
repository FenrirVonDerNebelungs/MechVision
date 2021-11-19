#pragma once
#ifndef PATTERNL1_H
#define PaTTERNL1_H

#ifndef STAMPEYE_H
#include "StampEye.h"
#endif

class PatternL1 : public Base {
public:


protected:
	/*not owned*/
	HexEye*   m_NNetEyes;/*trained NNets that are set to search for each of the STAMPEYENUM stamps*/
	int       m_numNNets;/*should be STAMPEYENUM*/
	/*owned*/
	s_PlateLayer m_L0Plates;/*reduced to one hex up, with the luna node in the center picked for the luna value*/
	s_PlateLayer m_L1Plates;/*results of the trained nnets run, will be STAMPEYENUM plates actually used*/
	/*     */
	
	unsigned char scan();
	unsigned char fullyRoot(s_hexEye& e0, long i);
	unsigned char evalAtRoot(long i_base);/*assumes that the NNet eyes have been fully rooted*/
	float evalNet(s_hexEye& net);
};
#endif
unsigned char PatternL1::evalAtRoot(long i_base) {
	for (int i = 0; i < m_numNNets; i++) {
		s_hexEye& net = m_NNetEyes->getEye(i);
		m_L1Plates.p[i].m_fhex[i_base].o = evalNet(net);
	}
	return ECODE_OK;
}
float PatternL1::evalNet(s_hexEye& net) {
	/*net should have 2 levels level 0 top and level 1*/
	s_hexPlate& l2p = net.lev[1];
	for (int i = 0; i < l2p.m_nHex; i++) {
		float sum = 0.f;
		for (int j = 0; j < l2p.m_fhex[i].N; j++) {

			sum += l2p.m_fhex[i].w[j] * l2p.m_fhex[i].nodes[j].o;
		}
	}
}
unsigned char PatternL1::scan() {
	s_hexPlate& p0 = m_L0Plates.p[0];
	s_hexEye& e0 = m_NNetEyes->getEye(0);
	for (long i = 0; i < p0.m_nHex; i++) {
		if (RetOk(m_NNetEyes->rootOn(e0, p0, i))) {
			fullyRoot(e0, i);
		}
	}
}
unsigned char PatternL1::fullyRoot(s_hexEye& e0, long i) {
	/*extend eye root from p0 to the other plates*/
	int highestLev = e0.n - 1;
	s_hexPlate& plt0 = e0.lev[highestLev];
	for (int p_i = 1; p_i < m_L0Plates.n; p_i++) {
		s_hexPlate& basePlate = m_L0Plates.p[p_i];
		for (int nd_i = 0; nd_i < plt0.m_nHex; nd_i++) {
			long plateIndex = plt0.m_fhex[nd_i].nodes[0]->thislink;
			plt0.m_fhex[nd_i].nodes[p_i] = (s_bNode*)&(basePlate.m_fhex[plateIndex]);
		}
	}
	/*now that the first NNet eye is filled with links to all of the plates, copy its links to the other NNet eyes*/
	for (int net_i = 1; net_i < m_numNNets; net_i++) {
		s_hexEye& erep = m_NNetEyes->getEye(net_i);
		m_NNetEyes->rootOnDup(e0, erep);
	}
	return ECODE_OK;
}