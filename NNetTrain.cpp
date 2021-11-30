#include "NNetTrain.h"
unsigned char NNetTrain::init(StampEye* stampEyep,/*contain patterns in the o's of its lowest level [2]*/
	s_hexEye* net,/*net descends to one level above the base of the stamp eye*/
	float stepSize,
	float DeltaE_closeEnough,
	long  max_loop_cnt) 
{	
	m_stampEye = stampEyep;
	m_net = net;
	m_DeltaE_closeEnough = DeltaE_closeEnough;
	m_max_loop_cnt = max_loop_cnt;
	s_eyeStamp* eyeStampsp = m_stampEye->getEyeStamps();
	m_QN = 0;
	for (int i = 0; i < m_stampEye->numEyeStamps(); i++) {
		for (int j = 0; j < eyeStampsp[i].n; i++) {
			m_y[m_QN] = eyeStampsp[i].o;
			m_dataStamps[m_QN] = eyeStampsp[i].eyes[j]; /*filling the pointers in another array to make this easier to read later*/
			m_QN++;
		}
	}
	/*all eyes used in training should have the same structure, and all of the 7 pack on the bottom should descend to 7 sub nodes each*/
	int m_Ws_N = net->lev[1].m_nHex * net->lev[1].m_fhex[0].N;
	if (m_Ws_N != m_dataStamps[0]->lev[2].m_nHex)
		return ECODE_ABORT;
	m_steps = new float[m_Ws_N];
	m_DeltaEs = new float[m_Ws_N];
	m_E = new float[m_Ws_N];
	for (int i = 0; i < m_Ws_N; i++) {
		m_steps[i] = stepSize;
		m_DeltaEs[i] = 0.f;
		m_E[i] = 0.f;
	}
	return ECODE_OK;
}