#include "NNetTrain.h"
NNetTrain::NNetTrain():m_net(NULL), m_stampEye(NULL), m_QN(0), m_steps(NULL), m_DeltaEs(NULL), m_E(NULL), m_Ws_N(0), m_DeltaE_closeEnough(0.f), m_max_loop_cnt(0)
{
	for (int i = 0; i < NNETTRAINMAXDATAN; i++) {
		m_dataStamps[i] = NULL;
		m_y[i] = 0.f;
	}
}
NNetTrain::~NNetTrain() {
	;
}
unsigned char NNetTrain::init(StampEye* stampEyep,/*contain patterns in the o's of its lowest level [2]*/
	s_hexEye* net,/*net has essentially same structure as the stampe eye's s_hexEye's*/
	float stepSize,
	float DeltaE_closeEnough,
	long  max_loop_cnt) 
{	
	m_stampEye = stampEyep;
	m_net = net;
	m_DeltaE_closeEnough = DeltaE_closeEnough;
	m_max_loop_cnt = max_loop_cnt;
	s_eyeStamp* eyeStampsp = m_stampEye->getLunaEyeStamps();
	m_QN = 0;
	for (int i = 0; i < m_stampEye->numEyeStamps(); i++) {
		for (int j = 0; j < eyeStampsp[i].n; i++) {
			m_y[m_QN] = eyeStampsp[i].o;
			m_dataStamps[m_QN] = eyeStampsp[i].eyes[j]; /*filling the pointers in another array to make this easier to read later*/
			m_QN++;
		}
	}
	/*all eyes used in training should have the same structure, and all of the 7 pack on the bottom should descend to the number of luna pattern sub nodes each*/
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
void NNetTrain::release() {
	if (m_E != NULL) {
		delete[] m_E;
	}
	m_E = NULL;
	if (m_DeltaEs != NULL) {
		delete[] m_DeltaEs;
	}
	m_DeltaEs = NULL;
	if (m_steps != NULL) {
		delete[] m_steps;
	}
	m_steps = NULL;
}
unsigned char NNetTrain::trainNet() {
	int level_num = m_net->n - 1;/*should be 1*/
	s_hexPlate& netBot = m_net->lev[level_num];
	//int totalWs = netBot.m_nHex * netBot.m_fhex[0].N all cells should have the same number of inputs from the base
	//float* Ws = new float[totalWs];

	bool converged = false;
	long loop_cnt = 0;
	do {
		findDeltaEs(netBot, level_num);
		converged = updateWs(netBot);
	} while (!converged && loop_cnt < m_max_loop_cnt);
	return converged ? ECODE_OK : ECODE_ABORT;
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
unsigned char NNetTrain::findDeltaEs(s_hexPlate& netBot, int level_num) {
	/**

	*/
	for (int i = 0; i < m_Ws_N; i++) {
		m_DeltaEs[i] = 0.f;
		m_E[i] = 0.f;
	}

	for (int q = 0; q < m_QN; q++) {
		connNetToQthStamp(netBot, q, level_num);
		float y = m_y[q];/*each cell cluster (an therefor cell output node) in the stamp is going to have the same target,
						 although in a general sense a NNet could have different target for each
						 a.k.a. ignoring the 'j' on the y*/
		for (int j_indx = 0; j_indx < netBot.m_nHex; j_indx++) {
			for (int k_indx = 0; k_indx < netBot.m_fhex[j_indx].N; k_indx++) {
				int W_jk_indx = j_indx * netBot.m_fhex[j_indx].N + k_indx;
				m_DeltaEs[W_jk_indx] += m_steps[W_jk_indx] * evalForQth_jk(y, netBot.m_fhex[j_indx], k_indx);
				m_E[W_jk_indx] += 0.5f * evalEForQth_j(y, netBot.m_fhex[j_indx]);
			}
		}
	}
	return ECODE_OK;
}
unsigned char NNetTrain::connNetToQthStamp(s_hexPlate& netBot, int q, int level_num) {
	s_hexPlate& stampBot = m_dataStamps[q]->lev[level_num];
	for (int i = 0; i < netBot.m_nHex; i++) {
		s_fNode& net_nd = netBot.m_fhex[i];
		s_fNode& stamp_nd = stampBot.m_fhex[i];
		for (int j = 0; j < net_nd.N; j++) {
			net_nd.nodes[j] = stamp_nd.nodes[j];
		}
	}
	return ECODE_OK;
}
float NNetTrain::evalEForQth_j(float y, s_fNode& n) {
	float n_out = sumWs(n);
	float diff = n_out - y;
	return diff * diff;
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