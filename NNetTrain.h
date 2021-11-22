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

protected:
	/*not owned*/
	s_hexEye m_net;   
	/* owned   */
	StampEye* m_stampEye; /*Master function array of eye stamps that will act as the 'q' data set*/
	s_hexEye*  m_dataStamps[NNETTRAINMAXDATAN];
	float      m_y[NNETTRAINMAXDATAN];
	int        m_QN;/*total number of data stamps in the training set*/
	float*     m_steps;
	float*     m_DeltaWs;
	float*     m_E; /*current errors*/
	int        m_Ws_N;
	float      m_E_closeEnough;/*value of E at which the scan is considered to have converged*/
	/*         */
	bool resetWeightsForLowNode(); /*do step for one of the nodes in the luna selection*/

	unsigned char trainNet();

	bool          updateWs(float* prev_DeltaWs, s_hexPlate& netBot);
	bool          findNextSteps(float* prev_DeltaWs /*compairs to m_DeltaWs and resets m_steps*/);
	unsigned char findDeltaWs(s_hexPlate& netBot);

	unsigned char connNetToQthStamp(int q);/*q is the index of the currently selected stamp*/

	float evalEForQth_j(float y, s_fNode& n);

	float evalForQth_jk(float y, s_fNode& n, int k);/*eval the inside of the sum over targets 'q's', y is the target value*/
	/*w's hang off of s_fNode's, the input values should come through connected lower nodes*/
	float sumWs(s_fNode& n);
	float getX(s_fNode& n, int k);/*get the k'th node input value 'x_k' for the node n*/


	float NNetFunc(float x);
	float NNetDFunc(float x);/*gives the derivative of the NNetFunc*/
};

#endif
unsigned char NNetTrain::trainNet() {
	s_hexPlate& netBot = m_net.lev[1];
	//int totalWs = netBot.m_nHex * netBot.m_fhex[0].N all cells should have the same number of inputs from the base
	//float* Ws = new float[totalWs];

	float* prev_DeltaWs = new float[m_Ws_N];
	bool converged = false;
	do {
		for (int i = 0; i < m_Ws_N; i++)
			prev_DeltaWs[i] = m_DeltaWs[i];
		
		findDeltaWs(netBot);

	} while (!converged);

}
float NNetTrain::evalEForQth_j(float y, s_fNode& n) {
	float n_out = sumWs(n);
	float diff = n_out - y;
	return diff * diff;
}
bool NNetTrain::findNextSteps(float* prev_DeltaWs) {
	bool converged = true;
	for (int i = 0; i < m_Ws_N; i++) {
		if (prev_DeltaWs[i] != 0.f) {
			float signind = prev_DeltaWs[i] * m_DeltaWs[i];
			if (signind < 0.f) {
				m_steps[i] = 0.f;/*since this is going back and forth assume it has converged*/
			}
			else {
				converged = false;
				
			}
		}
	}
}
unsigned char NNetTrain::findDeltaWs(s_hexPlate& netBot) {
	/**

	*/
	for (int i = 0; i < m_Ws_N; i++) {
		m_DeltaWs[i] = 0.f;
		m_E[i] = 0.f;
	}
	
	for (int q = 0; q < m_QN; q++) {
		connNetToQthStamp(q);
		float y = m_y[q];/*each cell cluster (an therefor cell output node) in the stamp is going to have the same target, 
						 although in a general sense a NNet could have different target for each
						 -ignoring the 'j' on the y*/
		for (int j_indx = 0; j_indx < netBot.m_nHex; j_indx++) {
			for (int k_indx = 0; k_indx < netBot.m_fhex[j_indx].N; k_indx++) {
				int W_jk_indx = j_indx * netBot.m_fhex[j_indx].N + k_indx;
				m_DeltaWs[W_jk_indx] += m_steps[W_jk_indx] * evalForQth_jk(y, netBot.m_fhex[j_indx], k_indx);
				m_E[W_jk_indx] += 0.5*evalEForQth_j(y, netBot.m_fhex[j_indx]);
			}
		}
	}
	return ECODE_OK;
}

unsigned char NNetTrain::connNetToQthStamp(int q) {
	s_hexPlate& netBot = m_net.lev[1];
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