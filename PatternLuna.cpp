#include "PatternLuna.h"

PatternLuna::PatternLuna() : m_lev1(NULL) {
	for (int i = 0; i < PATTERNLUNA_NUM; i++)
		m_lev0[i] = NULL;
}
PatternLuna::~PatternLuna() {
	;
}
unsigned char PatternLuna::init() {
	m_lev1 = new s_fNode;
	m_lev1->initNodePtrs(PATTERNLUNA_NUM);
	for (int i = 0; i < PATTERNLUNA_NUM; i++) {
		m_lev0[i] = new s_fNode;
		m_lev0[i]->initNodePtrs(PATTERNLUNA_FOOTS);
	}
	return GenNodeStructure();
}
void PatternLuna::release() {
	for (int i = 0; i < PATTERNLUNA_NUM; i++) {
		if (m_lev0[i] != NULL) {
			m_lev0[i]->releaseNodePtrs();
			delete m_lev0[i];
		}
		m_lev0[i] = NULL;
	}
	m_lev1->releaseNodePtrs();
	delete m_lev1;
}

unsigned char PatternLuna::run(s_hexPlate* inp, s_PlateLayer& outLay) {
	for (int i = 0; i < PATTERNLUNA_NUM; i++) {
		PatternLunaThreaded::runPlate(*(m_lev0[i]), *inp, outLay.p[i]);
	}
	return ECODE_OK;
}

unsigned char PatternLuna::GenNodeStructure() {
	for (int i_rot = 0; i_rot < PATTERNLUNA_NUM; i_rot++) {
		m_lev1->nodes[i_rot] = (s_bNode*)m_lev0[i_rot];
		m_lev1->w[i_rot] = PATTERNLUNA_WSCALE / ((float)PATTERNLUNA_NUM);
	}
	for (int i_rot = 0; i_rot < 6; i_rot++) {
		genNodePattern(i_rot, m_lev0[i_rot]);
		m_lev1->N++;
	}
	/*create full white/black patterns*/
	float patWeight = PATTERNLUNA_WSCALE / ((float)PATTERNLUNA_FOOTS);
	for (int i_ft = 0; i_ft < PATTERNLUNA_FOOTS; i_ft++) {
		m_lev0[6]->w[i_ft] = patWeight;
		m_lev0[7]->w[i_ft] = -patWeight;
	}
	m_lev0[6]->N = PATTERNLUNA_FOOTS;
	m_lev0[7]->N = PATTERNLUNA_FOOTS;
	(m_lev1->N) += 2;

	return ECODE_OK;
}
void PatternLuna::genNodePattern(int nodeRot, s_fNode* patOUT) {
	int arcLen = 3;
	float posFracWeight = 1.f / (3.5f);
	float negFracWeight = -3.5f / 2.f;//-1.f / arcLen;
	for (int i = 0; i < PATTERNLUNA_FOOTS; i++)
		patOUT->w[i] = negFracWeight;
	for (int i = 0; i <= arcLen; i++) {
		int i_Nd = Math::loop(nodeRot + i, 6);
		patOUT->w[i_Nd] = posFracWeight;
	}
	int i_line;
	i_line = 6;
	float lineWeight = 0.5f / 3.5f;
	patOUT->w[i_line] = lineWeight;
	i_line = Math::loop(nodeRot, 6);
	patOUT->w[i_line] = lineWeight;
	i_line = Math::loop(nodeRot + 3, 6);
	patOUT->w[i_line] = lineWeight;

	for (int i = 0; i < PATTERNLUNA_FOOTS; i++)
		patOUT->w[i] = PATTERNLUNA_WSCALE * (patOUT->w[i]);
	patOUT->N = PATTERNLUNA_FOOTS;
}

namespace PatternLunaThreaded {
	void runPlate(s_fNode& lev0, s_hexPlate& baseplate, s_hexPlate& plate) {
		for (long i = 0; i < baseplate.m_nHex; i++) {
			evalAtPlateNode(lev0, baseplate.m_fhex[i]);
			plate.m_fhex[i].o = lev0.o;
		}
	}
	void evalAtPlateNode(s_fNode& lev0, s_fNode& plateNode){
		float sumN = 0.f;
		for (int i = 0; i < 6; i++) {
			s_bNode* sidelinkPtr = plateNode.web[i];
			if (sidelinkPtr != NULL)
				sumN += sidelinkPtr->o * lev0.w[i];
		}
		sumN += plateNode.o * lev0.w[6];
		lev0.o = Math::StepFunc(sumN-0.5f);
	}
	void evalLowerNode(s_fNode& lev0, s_fNode& colNode) {
		float sumN = 0.f;
		for (int i = 0; i < 7; i++) {
			s_bNode* lowLinkPtr = colNode.nodes[i];
			if (lowLinkPtr != NULL)
				sumN += lowLinkPtr->o * lev0.w[i];
		}
		lev0.o = Math::StepFunc(sumN);// -0.5f);
	}
}