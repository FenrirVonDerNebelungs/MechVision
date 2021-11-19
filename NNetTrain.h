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
	/*         */
	bool resetWeightsForLowNode(); /*do step for one of the nodes in the luna selection*/
	float deltaForW_jk(float step, float y, s_fNode& n, int k);

	unsigned char findWs();
	unsigned char connNetToQthStamp(int q);/*q is the index of the currently selected stamp*/

	float evalForQth_jk(float y, s_fNode& n, int k);/*eval the inside of the sum over targets 'q's', y is the target value*/
	/*w's hang off of s_fNode's, the input values should come through connected lower nodes*/
	float sumWs(s_fNode& n);
	float getX(s_fNode& n, int k);/*get the k'th node input value 'x_k' for the node n*/


	float NNetFunc(float x);
	float NNetDFunc(float x);/*gives the derivative of the NNetFunc*/
};

#endif
unsigned char NNetTrain::findWs() {
	
	for (int q = 0; q < m_QN; q++) {
		connNetToQthStamp(q);
		float y = m_y[q];
	}
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