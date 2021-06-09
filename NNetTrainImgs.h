#pragma once
#ifndef NNETTRAINIMGS_H
#define NNETTRAINIMGS_H

#ifndef IMG_H
#include "Img.h"
#endif
#ifndef NNODEBASE_H
#include "NNodeBase.h"
#endif

class NNetTestTrainImgs {
public:
	NNetTestTrainImgs();
	~NNetTestTrainImgs();
	unsigned char Init();
	void Release();

	int transferImgs(Img* imgs[]);//Transfer ownership of images to new img* array "imgs" return len of array
	void transferTargetOutputs(float targetOutput[]);
protected:
	Img* m_imgSet[MAXNUMTRAINIMGS];
	float m_targetOutput[MAXNUMTRAINIMGS];
	int m_numSet;

	void LoadImgs();
	void LoadImg(const char* filename = NULL);
};

#endif
