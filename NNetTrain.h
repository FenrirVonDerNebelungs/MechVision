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
	unsigned char init(
		StampEye* stampEyep,
		s_hexEye* net,
		float stepSize=0.01,
		float DeltaE_closeEnough=0.001,
		long  max_loop_cnt = 10000
	);

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
	unsigned char findDeltaEs(s_hexPlate& netBot);

	unsigned char connNetToQthStamp(s_hexPlate& netBot, int q);/*q is the index of the currently selected stamp*/

	float evalEForQth_j(float y, s_fNode& n);

	float evalForQth_jk(float y, s_fNode& n, int k);/*eval the inside of the sum over targets 'q's', y is the target value*/
	/*w's hang off of s_fNode's, the input values should come through connected lower nodes*/
	float sumWs(s_fNode& n);
	float getX(s_fNode& n, int k);/*get the k'th node input value 'x_k' for the node n*/


	inline float NNetFunc(float x) { return Math::StepFuncSym(x); }
	inline float NNetDFunc(float x) { return Math::DStepFuncSym(x); }/*gives the derivative of the NNetFunc*/
};

#endif
unsigned char NNetTrain::trainNet() {
	s_hexPlate& netBot = m_net->lev[1];
	//int totalWs = netBot.m_nHex * netBot.m_fhex[0].N all cells should have the same number of inputs from the base
	//float* Ws = new float[totalWs];

	bool converged = false;
	long loop_cnt=0;
	do {		
		findDeltaEs(netBot);
		converged = updateWs(netBot);
	} while (!converged && loop_cnt < m_max_loop_cnt);
	return converged ? ECODE_OK : ECODE_ABORT;
}
float NNetTrain::evalEForQth_j(float y, s_fNode& n) {
	float n_out = sumWs(n);
	float diff = n_out - y;
	return diff * diff;
}
bool NNetTrain::updateWs(s_hexPlate& netBot) {
	bool converged = true;
	for (int j_indx = 0; j_indx < netBot.m_nHex; j_indx++) {
		for (int k_indx = 0; k_indx < netBot.m_fhex[j_indx].N; k_indx++) {
			int W_jk_indx = j_indx * netBot.m_fhex[j_indx].N + k_indx;
			if (fabs(m_DeltaEs[W_jk_indx]) < m_DeltaE_closeEnough) {
				/*consider this converged*/
				m_steps[W_jk_indx] = 0.f;
			}
			else {
				converged = false;/*at least one value is not converged*/
				/*not yet converged*/
				if (m_DeltaEs[W_jk_indx] > 0.f) {
					/*going in wrong direction error is getting bigger*/
					m_steps[W_jk_indx] = -m_steps[W_jk_indx];/*reverse the direction of the steps*/
				}
				else {
					/*going in the correct direction, so update the w*/
					netBot.m_fhex[j_indx].w[k_indx] += m_DeltaEs[W_jk_indx];
				}
			}
		}
	}
	return converged;
}

unsigned char NNetTrain::findDeltaEs(s_hexPlate& netBot) {
	/**

	*/
	for (int i = 0; i < m_Ws_N; i++) {
		m_DeltaEs[i] = 0.f;
		m_E[i] = 0.f;
	}
	
	for (int q = 0; q < m_QN; q++) {
		connNetToQthStamp(netBot, q);
		float y = m_y[q];/*each cell cluster (an therefor cell output node) in the stamp is going to have the same target, 
						 although in a general sense a NNet could have different target for each
						 a.k.a. ignoring the 'j' on the y*/
		for (int j_indx = 0; j_indx < netBot.m_nHex; j_indx++) {
			for (int k_indx = 0; k_indx < netBot.m_fhex[j_indx].N; k_indx++) {
				int W_jk_indx = j_indx * netBot.m_fhex[j_indx].N + k_indx;
				m_DeltaEs[W_jk_indx] += m_steps[W_jk_indx] * evalForQth_jk(y, netBot.m_fhex[j_indx], k_indx);
				m_E[W_jk_indx] += 0.5*evalEForQth_j(y, netBot.m_fhex[j_indx]);
			}
		}
	}
	return ECODE_OK;
}

unsigned char NNetTrain::connNetToQthStamp(s_hexPlate& netBot, int q) {
	s_hexPlate& stampBot = m_dataStamps[q]->lev[1];
	for (int i = 0; i < netBot.m_nHex; i++) {
		s_fNode& net_nd = netBot.m_fhex[i];
		s_fNode& stamp_nd = stampBot.m_fhex[i];
		for (int j = 0; j < net_nd.N; j++) {
			net_nd.nodes[j] = stamp_nd.nodes[j];
		}
	}
	return ECODE_OK;
}
float NNetTrain::evalForQth_jk(float y, s_fNode& n, int k) {
	float sumW = sumWs(n);
	float n_j = NNetFunc(sumW);
	float e_j = n_j - y;
	float df = NNetDFunc(sumW);
	return e_j * df * getX(n, k);
}
float NNetTrain::sumWs(s_fNode& n) {
	float sum = 0.f;
	for (int i = 0; i < n.N; i++) {
		sum += n.w[i] * n.nodes[i]->o;
	}
	return sum;
}
float NNetTrain::getX(s_fNode& n, int k) {
	return n.nodes[k]->o;
}