#pragma once
#ifndef NNETTRAIN_H
#define NNETTRAIN_H

#ifndef STAMPEYE_H
#include "StampEye.h"
#endif

#define NNETTRAINMAXDATAN 10000

/*class will currently train a simple two stage neural net into a 2 level hexeye,
  The lowest level will be on independent data and therefore will be trained seperately from the 2nd level */
class NNetTrain : public Base {
public:
	NNetTrain();
	~NNetTrain();

	unsigned char init(
		StampEye* stampEyep,
		s_hexEye* net,
		float stepSize=0.01,
		float DeltaE_closeEnough=0.001,
		long  max_loop_cnt = 10000
	);
	void release();
	void run() { trainNet(); }
protected:
	/*not owned*/
	s_hexEye* m_net;   
	StampEye* m_stampEye; /*Master function array of eye stamps that will act as the 'q' data set*/
	s_hexEye* m_dataStamps[NNETTRAINMAXDATAN];/*broken out from stampEye */

	/* owned   */
	float      m_y[NNETTRAINMAXDATAN];
	int        m_QN;/*total number of data stamps in the training set*/
	float*     m_steps;
	float*     m_DeltaEs;
	float*     m_E; /*current errors*/
	int        m_Ws_N;
	float      m_DeltaE_closeEnough;/*value of dE/dw that is low enough that the scan is considered to have converged*/
	long       m_max_loop_cnt;
	/*         */

	unsigned char trainNet();

	bool          updateWs(s_hexPlate& netBot);
	unsigned char findDeltaEs(s_hexPlate& netBot, int level_num);

	unsigned char connNetToQthStamp(s_hexPlate& netBot, int q, int level_num);/*q is the index of the currently selected stamp*/

	float evalEForQth_j(float y, s_fNode& n);

	float evalForQth_jk(float y, s_fNode& n, int k);/*eval the inside of the sum over targets 'q's', y is the target value*/
	/*w's hang off of s_fNode's, the input values should come through connected lower nodes*/
	float sumWs(s_fNode& n);
	float getX(s_fNode& n, int k);/*get the k'th node input value 'x_k' for the node n*/


	inline float NNetFunc(float x) { return Math::StepFuncSym(x); }
	inline float NNetDFunc(float x) { return Math::DStepFuncSym(x); }/*gives the derivative of the NNetFunc*/
};

#endif








