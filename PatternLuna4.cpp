#include "stdafx.h"
#include "PatternLuna4.h"


PatternLuna4::PatternLuna4() : m_hokPat(NULL)
{
	for (int i = 0; i < PATTERNLUNA4_NUM; i++)
		m_patNodes[i] = NULL;
}
PatternLuna4::~PatternLuna4() { ; }

unsigned char PatternLuna4::init()
{
	m_hokPat = new s_pNode;
	m_hokPat->nodes = new s_bNode * [PATTERNLUNA4_NUM];
	m_hokPat->w = new float[PATTERNLUNA4_NUM];
	for (int i = 0; i < PATTERNLUNA4_NUM; i++) {
		m_hokPat->nodes[i] = NULL;

		m_patNodes[i] = new s_pNode;
		m_patNodes[i]->nodes = new s_bNode * [PATTERNLUNA4_FOOTS];
		m_patNodes[i]->w = new float[PATTERNLUNA4_FOOTS];
		for (int j = 0; j < PATTERNLUNA4_FOOTS; j++)
			m_patNodes[i]->nodes[j] = NULL;
	}
	return GenNodeStructure();
}
void PatternLuna4::release() {
	for (int i = 0; i < PATTERNLUNA4_NUM; i++) {
		if (m_patNodes[i] != NULL) {
			if (m_patNodes[i]->nodes != NULL)
				delete m_patNodes[i]->nodes;
			if (m_patNodes[i]->w != NULL)
				delete m_patNodes[i]->w;
			delete m_patNodes[i];
			m_patNodes[i] = NULL;
		}
	}
	if (m_hokPat != NULL) {
		if (m_hokPat->nodes != NULL)
			delete m_hokPat->nodes;
		if (m_hokPat->w != NULL)
			delete m_hokPat->w;
		delete m_hokPat;
		m_hokPat = NULL;
	}
}

unsigned char PatternLuna4::eval(s_fNode* footIN[], s_hex* lowHexIN) {
	float highestf = -1.f;
	for (int i = 0; i < 6; i++) {
		evalSubLunaPatForFoot(footIN, lowHexIN, m_patNodes[i]);
		float curf = m_patNodes[i]->f;
		if (curf > highestf)
			highestf = curf;
	}
	for (int i = 6; i < PATTERNLUNA4_NUM; i++) {
		evalSubLunaPatForFoot(footIN, lowHexIN, m_patNodes[i]);
		float curf = m_patNodes[i]->f;
	}
	m_hokPat->f = highestf;
	return ECODE_OK;
}

unsigned char PatternLuna4::GenNodeStructure() {
	for (int i_rot = 0; i_rot < PATTERNLUNA4_NUM; i_rot++) {
		m_hokPat->nodes[i_rot] = (s_bNode*)m_patNodes[i_rot];
		m_hokPat->w[i_rot] = PATTERNLUNA4_WSCALE / ((float)PATTERNLUNA4_NUM);
	}
	for (int i_rot = 0; i_rot < 6; i_rot++) {
		genNodePattern(i_rot, m_patNodes[i_rot]);
		(m_hokPat->N)++;
	}
	/*create full white/black patterns*/
	float patWeight = PATTERNLUNA4_WSCALE / ((float)PATTERNLUNA4_FOOTS);
	for (int i_ft = 0; i_ft < PATTERNLUNA4_FOOTS; i_ft++) {
		m_patNodes[6]->w[i_ft] = patWeight;
		m_patNodes[7]->w[i_ft] = -patWeight;
	}
	m_patNodes[6]->N = PATTERNLUNA4_FOOTS;
	m_patNodes[7]->N = PATTERNLUNA4_FOOTS;
	(m_hokPat->N) += 2;

	return ECODE_OK;
}
void PatternLuna4::genNodePattern(int nodeRot, s_pNode* patOUT) {
	int arcLen = 3;
	float posFracWeight = 1.f / (3.5f);
	float negFracWeight = -3.5f / 2.f;//-1.f / arcLen;
	for (int i = 0; i < 7; i++)
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

	for (int i = 0; i < 7; i++)
		patOUT->w[i_line] = PATTERNLUNA4_WSCALE * (patOUT->w[i_line]);
	patOUT->N = 7;
}
unsigned char PatternLuna4::evalSubLunaPatForFoot(s_fNode* footIN[], s_hex* lowHexIN, s_pNode* lunaPatternOUT) {
	float sumN = 0.f;

	for (int i = 0; i < 6; i++) {
		int sidelinki = lowHexIN->web[i];
		if (sidelinki >= 0)
			sumN += footIN[sidelinki]->f * lunaPatternOUT->w[i];
	}
	int centerLinki = lowHexIN->thisLink;
	sumN += footIN[centerLinki]->f * lunaPatternOUT->w[6];
	lunaPatternOUT->f = sumN;//Math::StepFunc(sumN);
	return ECODE_OK;
}