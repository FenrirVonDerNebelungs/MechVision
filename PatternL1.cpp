#include "PatternL1.h"
unsigned char PatternL1::init(s_PlateLayer& lunaPlates,s_hexEye NNetEyes[], int NNets) {
	m_NNetEyes = NNetEyes;
	m_numNNets = NNets;
	if (m_numNNets > PATTERNL1MAXNUMNETS)
		return ECODE_FAIL;
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
	long Nnodes_L1_l1 = m_NNetEyes[0].lev[0].m_fhex[0].N * m_L1Plates.p[0].m_nHex; 
	for (int ey_i = 0; ey_i < m_numNNets; ey_i++) {
		PatStruct::genPlateWSameWeb(m_L0Plates.p[0], m_L1Plates.p[ey_i]);
		PatStruct::genLowerNodePtrsForPlate(m_L1Plates.p[ey_i], m_NNetEyes[0].lev[0].m_fhex[0].N);
		/*each node in the ey plates draws from all luna plates, 
		geometrically all plates have the same structure */
		m_L1MemNodes[ey_i].nd = new s_fNode[Nnodes_L1_l1];
		m_L1MemNodes[ey_i].n = Nnodes_L1_l1;
		genL1midNodes(m_L1MemNodes[ey_i]);
	}

	return ECODE_OK;
}
void PatternL1::genL1midNodes(s_patL1Nodes& patL1Nds) {
	for (long i = 0; i < patL1Nds.n; i++) {
		patL1Nds.nd[i].zero();
		patL1Nds.nd[i].initNodePtrs(m_NNetEyes[0].lev[1].m_fhex[0].N);
	}
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
			n_PatternL1::updateL0Node(curNode);
		}
	}
	return ECODE_OK;
}
unsigned char PatternL1::transNNetToPlates(int net_index) {

	/*L1 & L2 should have the same hexes with the same indexes as L0 but above L0*/
	/* add the lowest level of the NNet to Pattern L1, connecting its nodes to the different L0 plates */
	s_hexEye& NNet = m_NNetEyes[net_index];
	s_hexPlate& L1Plate = m_L1Plates.p[net_index];
	/*connect the network of nodes hanging from the L1 plate to the L0 plates so that the weights and
	  network are duplicates for each high L1 node of the NNet */

	if (L1Plate.m_nHex != m_L0Plates.p[0].m_nHex)
		return ECODE_FAIL;
	if (NNet.lev[1].m_nHex != m_NNetEyes[0].lev[0].m_fhex[0].N)
		return ECODE_FAIL;

	for (long i_hex = 0; i_hex < L1Plate.m_nHex; i_hex++) {
		if (NNet.lev[1].m_nHex != L1Plate.m_fhex[i_hex].N)
			return ECODE_FAIL;
		if (NNet.lev[0].m_fhex[0].N != NNet.lev[1].m_nHex)
			return ECODE_FAIL;
		for (int i_1nd = 0; i_1nd < NNet.lev[1].m_nHex; i_1nd++) {
			/*connnect the L1 plate to the memory chain of nodes*/
			long memNd_i = m_NNetEyes[0].lev[0].m_fhex[0].N * i_hex + i_1nd;
			L1Plate.m_fhex[i_hex].nodes[i_1nd] = (s_bNode*)&(m_L1MemNodes[net_index].nd[memNd_i]);
			L1Plate.m_fhex[i_hex].w[i_1nd] = NNet.lev[0].m_fhex[0].w[i_1nd];
			/*now connect each 1 level down node to the final level down which connects to the plates*/
			s_fNode* hangingNode = (s_fNode*)L1Plate.m_fhex[i_hex].nodes[i_1nd];
			if (NNet.lev[1].m_fhex[i_1nd].N != m_L0Plates.n)
				return ECODE_FAIL;
			for (int i_luna = 0; i_luna < NNet.lev[1].m_fhex[1].N; i_luna++) {
				s_fNode* centerPlateHex = (s_fNode*)&(m_L0Plates.p[i_luna].m_fhex[i_hex]);
				if (NNet.lev[1].m_nHex != 7)
					return ECODE_FAIL;
				if (i_1nd == 0) {
					/*this is the center*/
					hangingNode->nodes[i_luna] = (s_bNode*)centerPlateHex;
					hangingNode->w[i_luna] = NNet.lev[1].m_fhex[i_1nd].w[i_luna];
				}
				else {
					/*these need to be connected to the web around the center node on the plate*/
					int web_i = i_1nd - 1;
					s_bNode* adjNodePlate_ptr = centerPlateHex->web[web_i];
					hangingNode->nodes[i_luna] = adjNodePlate_ptr;
					if (adjNodePlate_ptr != NULL) {
						hangingNode->w[i_luna] = NNet.lev[1].m_fhex[i_1nd].w[i_luna];
					}
				}
			}
		}
	}
	return ECODE_OK;
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
