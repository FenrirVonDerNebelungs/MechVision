#include "PatternL1.h"
unsigned char PatternL1::init(s_PlateLayer& lunaPlates, HexEye NNetEyes[], int NNets) {
	m_NNetEyes = NNetEyes;
	m_numNNets = NNets;
	if (NNets < 1 || lunaPlates.n < 1)
		return ECODE_ABORT;
	m_L0Plates.n = 0;
	float lunaWeights[PATTERNL1L0WNUM];
	for (int i = 0; i < PATTERNL1L0WNUM; i++)
		lunaWeights[i] = 1.f/PATTERNL1L0WNUM;
	for (int i = 0; i < lunaPlates.n; i++) {
		PatStruct::genLayerUp(lunaPlates.p[i], m_L0Plates.p[i]);
		for (long i_h = 0; i_h < lunaPlates.p[i].m_nHex; i_h++) {
			for (int i_w = 0; i_w < PATTERNL1L0WNUM; i_w++) {
				lunaPlates.p[i].m_fhex[i_h].w[i_w] = lunaWeights[i_w]; /*setting the weighted average since this a reduced layer, it has 7 smaller hexes in it*/
			}
		}
		m_L0Plates.n++;
	}
	PatStruct::zeroPlateLayer(m_L1Plates);
	PatStruct::zeroPlateLayer(m_L2Plates);
	for (int ey_i = 0; ey_i < m_numNNets; ey_i++) {
		PatStruct::genPlateWSameWeb(m_L0Plates.p[0], m_L1Plates.p[ey_i]);
		PatStruct::genPlateWSameWeb(m_L0Plates.p[0], m_L1Plates.p[ey_i]);
		/*each node in the ey plates draws from all luna plates, 
		geometrically all plates have the same structure */
	}

	return ECODE_OK;
}
void PatternL1::release() {
	for (int ey_i = 0; ey_i < m_numNNets; ey_i++) {
		PatStruct::releasePlateWSameWeb(m_L1Plates.p[ey_i]);
	}
	m_numNNets = 0;
	for (int i = 0; i < m_L0Plates.n; i++) {
		PatStruct::releaseLayerUp(m_L0Plates.p[i]);
	}
	m_L0Plates.n = 0;
}



unsigned char PatternL1::scan() {
	updateL0();
	s_hexPlate& p0 = m_L0Plates.p[0];
	s_hexEye& e0 = m_NNetEyes->getEye(0);
	for (long i = 0; i < p0.m_nHex; i++) {
		if (RetOk(m_NNetEyes->rootOn(e0, p0, i))) {
			fullyRoot(e0, i);
		}
	}
}
unsigned char PatternL1::updateL0() {
	for (long i = 0; i < m_L0Plates.n; i++) {
		s_hexPlate& curPlate = m_L0Plates.p[i];
		for (long hex_i = 0; hex_i < m_L0Plates.p[i].m_nHex; hex_i++) {
			s_fNode& curNode = curPlate.m_fhex[hex_i];
			n_PatternL1::L0ToL1(curNode);
		}
	}
	return ECODE_OK;
}
unsigned char PatternL1::transEyeToPlates(s_hexEye& e0, long i) {
	int highestEyLev = e0.n - 1;
	s_hexPlate& plt0 = e0.lev[highestEyLev];
	for (int p_i = 1; p_i < m_L0Plates.n; p_i++) {
		s_hexPlate& baseL0Plate = m_L0Plates.p[p_i];
		for (int nd_i = 0; nd_i < plt0.m_nHex; nd_i++) {
			
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
	s_hexPlate& l1p = net.lev[0];
	float sum = 0.f;
	for (int i = 0; i < l2p.m_nHex; i++) {
		float sumHid = 0.f;
		for (int j = 0; j < l2p.m_fhex[i].N; j++) {
			/*this goes over the lunas from the different plates connected to different nodes, for the same location*/
			sumHid += l2p.m_fhex[i].w[j] * l2p.m_fhex[i].nodes[j]->o;
		}
		sum += NNetFunc(sumHid) * l1p.m_fhex[0].w[i]; /*the top level has only one node*/
	}
	return NNetFunc(sum);
}
