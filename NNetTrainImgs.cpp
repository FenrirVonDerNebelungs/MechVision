#include "stdafx.h"
#include "NNetTrainImgs.h"

NNetTestTrainImgs::NNetTestTrainImgs() :m_numSet(0) {
	for (int i = 0; i < MAXNUMTRAINIMGS; i++) {
		m_imgSet[i] = NULL;
		m_targetOutput[i] = 0.f;
	}
}
NNetTestTrainImgs::~NNetTestTrainImgs() {
	;
}
unsigned char NNetTestTrainImgs::Init() {
	m_numSet = 0;
	for (int i = 0; i < MAXNUMTRAINIMGS; i++) {
		m_imgSet[i] = NULL;
		m_targetOutput[i] = 0.f;
	}
	LoadImgs();
	return ECODE_OK;
}
void NNetTestTrainImgs::Release() {
	for (int i = 0; i < m_numSet; i++) {
		if (m_imgSet[i] != NULL) {
			m_imgSet[i]->release();
			delete m_imgSet[i];
			m_imgSet[i] = NULL;
		}
		m_targetOutput[i] = 0.f;
	}
	m_numSet = 0;
}

int NNetTestTrainImgs::transferImgs(Img* imgs[]) {
	for (int i = 0; i < m_numSet; i++) {
		imgs[i] = m_imgSet[i];
		m_imgSet[i] = NULL;
	}
	int numRet = m_numSet;
	m_numSet = 0;
	return numRet;
}
void NNetTestTrainImgs::transferTargetOutputs(float targetOutput[]) {
	for (int i = 0; i < m_numSet; i++) {
		targetOutput[i] = m_targetOutput[i];
	}
}

void NNetTestTrainImgs::LoadImgs() {
	LoadImg("IOFiles/train/img0");
	LoadImg("IOFiles/train/img1");
	LoadImg("IOFiles/train/img2");
	LoadImg("IOFiles/train/img3");
	LoadImg("IOFiles/train/img4");
	LoadImg("IOFiles/train/img5");
	LoadImg("IOFiles/train/img6");
	LoadImg("IOFiles/train/img7");
	LoadImg("IOFiles/train/img8");
	m_targetOutput[0] = 1.f;
}
void NNetTestTrainImgs::LoadImg(const char* filename) {
	if (m_numSet >= MAXNUMTRAINIMGS)
		return;
	CTargaImage targImg;
	targImg.Init();
	targImg.Open(filename);
	m_imgSet[m_numSet] = new Img;
	(m_imgSet[m_numSet])->init(&targImg);
	targImg.Close();
	targImg.Release();
	m_numSet++;
}