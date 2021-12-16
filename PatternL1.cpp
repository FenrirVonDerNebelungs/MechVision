#include "PatternL1.h"
PatternL1::PatternL1() :m_NNetEyes(NULL), m_numNNets(0) {
	PatStruct::zeroPlateLayer(m_L0Plates);
	PatStruct::zeroPlateLayer(m_L1Plates);
	for (int i = 0; i < PATTERNL1MAXNUMNETS; i++) {
		m_L1MemNodes[i].nd = NULL;
		m_L1MemNodes[i].n = 0;
	}
}
PatternL1::~PatternL1() {
	;
}
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
		m_L1Plates.n++;
	}

	transNNets();
	return ECODE_OK;
}

void PatternL1::release() {
	for (int ey_i = 0; ey_i < m_numNNets; ey_i++) {
		releaseL1midNodes(m_L1MemNodes[ey_i]);
		PatStruct::releaseLowerNodePtrsForPlate(m_L1Plates.p[ey_i]);
		PatStruct::releasePlateWSameWeb(m_L1Plates.p[ey_i]);
	}
	m_numNNets = 0;
	for (int i = 0; i < m_L0Plates.n; i++) {
		PatStruct::releaseLayerUp(m_L0Plates.p[i]);
	}
	m_L0Plates.n = 0;
}
void PatternL1::genL1midNodes(s_patL1Nodes& patL1Nds) {
	for (long i = 0; i < patL1Nds.n; i++) {
		patL1Nds.nd[i].zero();
		patL1Nds.nd[i].initNodePtrs(m_NNetEyes[0].lev[1].m_fhex[0].N);
	}
}
void PatternL1::releaseL1midNodes(s_patL1Nodes& patL1Nds) {
	for (long i = 0; i < patL1Nds.n; i++)
		patL1Nds.nd[i].releaseNodePtrs();
}
unsigned char PatternL1::transNNets() {
	for (int i = 0; i < m_numNNets; i++)
		transNNetToPlates(i);
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



/*note on next steps
* strongest feature
* features present from strongest feature
* multi hex location of features present
*/
unsigned char PatternL1::scan() {
	updateL0();
	for (int plate_i = 0; plate_i < m_L1Plates.n; plate_i++) {
		s_hexPlate& l1Plate = m_L1Plates.p[plate_i];
		for (long hex_i=0; hex_i < l1Plate.m_nHex; hex_i++) {
			s_fNode& tophex = l1Plate.m_fhex[hex_i];
			/*scan over l1 of net*/
			float l1_sum = 0.f;
			for (int l1_i = 0; l1_i < tophex.N; l1_i++) {
				s_fNode* l1node = (s_fNode*)tophex.nodes[l1_i];
				/*scan over bottom l2 of net*/
				float l2_sum = 0.f;
				if (l1node != NULL) {
					for (int l2_i = 0; l2_i < l1node->N; l2_i++) {
						s_fNode* l2node = (s_fNode*)l1node->nodes[l2_i];
						if (l2node != NULL)
							l2_sum += l1node->w[l2_i] * l2node->o;
					}
				}
				l1_sum += tophex.w[l1_i] * l2_sum;
			}
			tophex.o = l1_sum;
		}
	}
	return ECODE_OK;
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
