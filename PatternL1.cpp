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
		PatStruct::getLayerUp(lunaPlates.p[i], m_L0Plates.p[i]);
		for (long i_h = 0; i_h < lunaPlates.p[i].m_nHex; i_h++) {
			for (int i_w = 0; i_w < PATTERNL1L0WNUM; i_w++) {
				lunaPlates.p[i].m_fhex[i_h].w[i_w] = lunaWeights[i_w]; /*setting the weighted average since this a reduced layer, it has 7 smaller hexes in it*/
			}
		}
		m_L0Plates.n++;
	}
	PatStruct::zeroPlateLayer(m_L1Plates);
	for (int ey_i = 0; ey_i < m_numNNets; ey_i++) {
		PatStruct::copyPlateWSameWeb(m_L0Plates.p[0], m_L1Plates.p[ey_i]);
		/*each node in the ey plates draws from all luna plates, 
		geometrically all plates have the same structure */
	}

	return ECODE_OK;
}