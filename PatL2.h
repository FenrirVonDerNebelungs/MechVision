#pragma once
#ifndef PATL2_H
#define PATL2_H

#ifndef PATLUNALAYER_H
#include "PatLunaLayer.h"
#endif

#define PATL2_NUML2PAT 8 //49
#define PATL2_NUMW 7

struct s_PatLunaL2 {
	int luna_is[PATL2_NUMW];
	float w[PATL2_NUMW];
	float Norm;
};
class PatL2 : public Base {
public:
protected:
	s_PatLunaL2 m_PatL2[PATL2_NUML2PAT];

	s_PlateLayer* m_lunaLay;

	s_PlateLayer m_patLay[PATLUNALAYER_NUMCOL];

	unsigned char genPatLunaL2s();/*generate 49 s_patLunaL2 patterns from all permutaitons of the 7 luna's in 7 hex matrix*/
	unsigned char convulL2(s_PlateLayer& inPlateLay, s_PlateLayer& L2PlateLay);
	unsigned char convulL2ForPat(long hex_i, s_PlateLayer& inPlateLay, int Pat_i, s_hexPlate& L2Plate);
};

#endif
unsigned char PatL2::genPatLunaL2s() {
	int patCnt = 0;
	for (int sweep_i = 0; sweep_i < 7; sweep_i++) {
		for (int pat_i = 0; pat_i < 7; pat_i++) {
			
		}
	}
}

unsigned char PatL2::convulL2(s_PlateLayer& inPlateLay, s_PlateLayer& L2PlateLay) {
	if (L2PlateLay.n <= 0)
		return ECODE_ABORT;
	long nHex = L2PlateLay.p[0].m_nHex;
	/*all plates should have the same number of hexes*/
	for (long i = 0; i < nHex; i++) {
		for (int Pat_i = 0; Pat_i < PATL2_NUML2PAT; Pat_i++) {
			convulL2ForPat(i, inPlateLay, Pat_i, L2PlateLay.p[Pat_i]);
		}
	}
	return ECODE_OK;
}
unsigned char PatL2::convulL2ForPat(long hex_i, s_PlateLayer& inPlateLay, int Pat_i, s_hexPlate& L2Plate) {
	float cell_sum = 0.f;
	for (int cell_i = 0; cell_i < 6; cell_i++) {
		int luna_i = m_PatL2[Pat_i].luna_is[cell_i];
		s_hexPlate& lunaPlate = inPlateLay.p[luna_i];
		s_fNode* over1 = (s_fNode*)lunaPlate.m_fhex[hex_i].nodes[cell_i];
		if (over1 != NULL) {
			s_bNode* over2 = over1->nodes[cell_i];
			if (over2 != NULL) {
				float luna_w = m_PatL2[Pat_i].w[cell_i];
				cell_sum += over2->o * luna_w;
			}
		}
	}
	int center_lun_i = m_PatL2[Pat_i].luna_is[6];
	float center_lun_w = m_PatL2[Pat_i].w[6];
	s_hexPlate& center_lunPlate = inPlateLay.p[center_lun_i];
	cell_sum += center_lunPlate.m_fhex[hex_i].o * center_lun_w;
	
	cell_sum /= m_PatL2[Pat_i].Norm;
	L2Plate.m_fhex[hex_i].o = cell_sum;
	return ECODE_OK;
}
/*
unsigned char PatL2::convulL2ForPat(long hex_i, s_PlateLayer& inPlateLay, int Pat_i, s_hexPlate& L2Plate) {
	float cell_sum = 0.f;
	int luna_w_i = 0;
	s_hexPlate& fullLunaPlate = inPlateLay.p[6];
	s_fNode& fullLunaCentNd = fullLunaPlate.m_fhex[hex_i];
	for (int cell_i = 0; cell_i < 6; cell_i++) {
		luna_w_i = 7 * cell_i;
		find index of cell in layer
		all plates should have the same structure so use the full luna plate
		s_fNode* over1 = (s_fNode*)fullLunaCentNd.nodes[cell_i];
		if (over1 != NULL) {
			s_bNode* over2 = over1->nodes[cell_i];
			if (over2 != NULL) {
				long over_hex_i = over2->thislink;
				for (int luna_i = 0; luna_i < 7; luna_i++) {
					float lun_w = m_PatL2[Pat_i].w[luna_w_i];
					cell_sum += inPlateLay.p[luna_i].m_fhex[over_hex_i].o * lun_w;
					luna_w_i++; luna_w_i = 7*cell_i + luna_i 
				}

			}
		}

	}
	luna_w_i = 42;  7*6 
	for (int luna_i = 0; luna_i < 7; luna_i++) {
		float lun_w = m_PatL2[Pat_i].w[luna_w_i];
		cell_sum += inPlateLay.p[luna_i].m_fhex[hex_i].o * lun_w;
		luna_w_i++;
	}

	cell_sum /= m_PatL2[Pat_i].Norm;
	L2Plate.m_fhex[hex_i].o = cell_sum;
	return ECODE_OK;
}
*/