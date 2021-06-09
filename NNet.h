#pragma once
#ifndef NNET_H
#define NNET_H

#ifndef NNETTREE_H
#include "NNetTree.h"
#endif 
#ifndef NNETTRAINIMGS_H
#include "NNetTrainImgs.h"
#endif

class NNetTestTrain {
public:
	NNetTestTrain();
	~NNetTestTrain();
	unsigned char Init(
		HexStack* hexMaster,
		NNetTreeTest* pNet,
		int maxNumLoops = 1000,
		float step = 0.02
	);
	void Release();

	unsigned char Run();/*assumes m_pNet has already been run*/

	inline float* getWeights() { return m_ws; }
protected:
	/*not owned*/
	HexStack* m_hexMaster;
	NNetTreeTest* m_pNet;

	NNetTestTrainImgs* m_imgHandler;/*owned*/
	int m_numSet;/*number of images in training set*/
	Img* m_imgSet[MAXNUMTRAINIMGS];
	float m_targetOutput[MAXNUMTRAINIMGS];/*value indicating whether or not the image was a "good" image, 1 good, 0.5 neutral, 0 bad*/

	int    m_maxNumLoops;

	int    m_numLoops;

	float  m_step;

	float* m_ws;
	bool* m_converged;

	void   runTraining();/*return pointer to array of weights*/

	void   adjustWeights(float step, float ws[], bool converged[]);
	void   stepAdjustWeights(float step, float ns_array[], float ws[], bool converged[]);
	void   compute_ns(Img* inImg, float ns[]);
	void   RunLowerNet(float ns_array[]);/*attack image to hexStack and run over net to get result filling netResult*/

};

class NNetTest {
public:
	NNetTest();
	~NNetTest();
	unsigned char Init(HexStack* hexMaster);
	void Release();
	unsigned char Run();/*train net on test images, then go back, run it again to see results*/
protected:
	/*owned*/
	NNetTreeTest* m_pNet;
	NNetTestTrain* m_NNetTestTrain;
	/*not owned*/
	HexStack* m_hexMaster;

	float m_result;

	unsigned char RunTrainedNet();
	unsigned char TrainNet();
};


#endif
