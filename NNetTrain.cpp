#include "NNetTrain.h"
namespace NNet {
	unsigned char initNNetL1X(int N, s_NNetL1X& X) {
		X.m_x = new float[N];
		if (X.m_x == NULL)
			return ECODE_FAIL;
		for (int i = 0; i < N; i++)
			X.m_x[i] = 0.f;
		X.m_n = N;
		return ECODE_OK;
	}
	unsigned char initNNetL1Xs(long datasize, int nX, s_NNetL1X X[]) {
		X = new s_NNetL1X[datasize];
		if (X == NULL)
			return ECODE_FAIL;
		for (long i = 0; i < datasize; i++) {
			if (ECODE_OK!=(initNNetL1X(nX, X[i])))
				return ECODE_FAIL;
		}
		return ECODE_OK;
	}
	void releaseNNetL1X(s_NNetL1X& X) {
		if (X.m_x != NULL)
			delete[] X.m_x;
		X.m_x = NULL;
	}
	void releaseNNetL1Xs(long datasize, s_NNetL1X X[]) {
		if (X != NULL) {
			for (long i = 0; i < datasize; i++) {
				releaseNNetL1X(X[i]);
			}
			delete[] X;
		}
		X = NULL;
	}
	bool oNNetL0(s_hexEye& net, const s_hexEye& lunaEye) {/*results are storned in the net eye*/
	/*the two eyes must have the same structure*/
		int lowestLevel = net.n;
		lowestLevel -= 1;
		s_hexPlate& lowestNetLev = net.lev[lowestLevel];
		const s_hexPlate& lowestLunaLev = lunaEye.lev[lowestLevel];
		for (int i = 0; i < lowestNetLev.m_nHex; i++) {
			s_fNode& netNode = lowestNetLev.m_fhex[i];
			const s_fNode& lunaNode = lowestLunaLev.m_fhex[i];
			float sumW = 0.f;
			for (int j = 0; j < netNode.N; j++) {
				sumW += netNode.w[j] * lunaNode.nodes[j]->o;
				/*option netNode.nodes[j]->o = lunaNode.nodes[j]->o*/
			}
			netNode.o = sumW;
		}
		return true;
	}
}

NNetTrain0::NNetTrain0() :m_DeltaE_closeEnough(0.f), m_max_loop_cnt(0), m_w_init(0.f), m_stepSize(0.f),
m_X(NULL), m_nX(0), m_y(NULL), m_nData(0), m_w(NULL), m_nNodes(0),
m_DeltaEs(NULL), m_steps(NULL), m_E(0.f)
{
	;
}
NNetTrain0::~NNetTrain0() {
	;
}
unsigned char NNetTrain0::init(
	float stepSize,
	float DeltaE_closeEnough,
	long max_loop_cnt,
	float init_all_ws
) {
	m_stepSize = stepSize;
	m_DeltaE_closeEnough = DeltaE_closeEnough;
	m_max_loop_cnt = max_loop_cnt;
	m_w_init = init_all_ws;
	m_nNodes = 1;/*this variable is not actually used in NNetTrain0 */
}
void NNetTrain0::release() {
	releaseMem();
}
unsigned char NNetTrain0::setNet(long datasize, s_NNetL1X X[], float y[]) {
	unsigned char e_code = initMem(datasize, X[0].m_n);
	if (IsErrFail(e_code))
		return e_code;
	for (long i = 0; i < datasize; i++) {
		for (int x_i = 0; x_i < X[i].m_n; x_i++) {
			m_X[i].m_x[x_i] = X[i].m_x[x_i];
		}
		m_X[i].m_n = X[i].m_n;
		m_y[i] = y[i];
	}
	return ECODE_OK;
}
unsigned char NNetTrain0::initMem(long datasize, int nX) {
	if (datasize == m_nData && nX == m_nX)
		return ECODE_ABORT;/*nothing needs to be done the data is still the same size*/
	releaseMem();
	m_nData = datasize;
	m_nX = nX;
	NNet::initNNetL1Xs(m_nData, m_nX, m_X);
	m_y = new float[m_nData];
	m_w = new float[m_nX];
	
	m_DeltaEs = new float[m_nData];
	m_steps = new float[m_nX];
	if (m_X == NULL || m_y == NULL || m_w == NULL || m_DeltaEs == NULL || m_steps == NULL) {
		releaseMem();
		return ECODE_FAIL;
	}
	return ECODE_OK;
}
void NNetTrain0::releaseMem() {
	if(m_steps!=NULL)
		delete[] m_steps;
	m_steps = NULL;
	if(m_DeltaEs!=NULL)
		delete[] m_DeltaEs;
	m_DeltaEs = NULL;
	if (m_w != NULL)
		delete[] m_w;
	m_w = NULL;
	if (m_y != NULL)
		delete[] m_y;
	m_y = NULL;
	NNet::releaseNNetL1Xs(m_nData, m_X);
	m_nX = 0;
	m_nData = 0;
}
void NNetTrain0::reset() {
	for (int i = 0; i < m_nX; i++) {
		m_w[i] = m_w_init;
		m_steps[i] = -m_stepSize;
	}
}

unsigned char NNetTrain0::trainNet() {
	bool converged = false;
	long loop_cnt = 0;
	reset();
	do {
		findDeltaEs();
		converged = updateWs();
	} while (!converged && loop_cnt < m_max_loop_cnt);
	return converged ? ECODE_OK : ECODE_ABORT;
}
unsigned char NNetTrain0::findDeltaEs() {
	m_E = 0;
	for (int i = 0; i < m_nX; i++) {
		m_DeltaEs[i] = 0.f;
	}
	for (int q = 0; q < m_nData; q++) {
		float y = m_y[q];/* */
		for (int k_indx = 0; k_indx < m_nX; k_indx++) {
			int W_jk_indx = k_indx;/*since just training one node j=0 */
			for (long q_indx = 0; q_indx < m_nData; q_indx++) {
				m_DeltaEs[W_jk_indx] += m_steps[W_jk_indx] * evalForQth_jk(m_y[q_indx], m_X[q_indx], q_indx, k_indx);
				m_E += 0.5f * evalEForQth_j(y, m_X[q_indx]);
			}
		}
	}
	return ECODE_OK;
}
bool NNetTrain0::updateWs() {
	bool converged = true;
	for (int k_indx = 0; k_indx < m_nX; k_indx++) {
		int W_jk_indx = k_indx;/*since there is only one node j=0*/
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
				m_w[W_jk_indx] += m_DeltaEs[W_jk_indx];
			}
		}
	}
	return converged;
}
float NNetTrain0::evalForQth_jk(float y, s_NNetL1X& X, long q, int k) {
	float sumW = sumWs(X.m_x);
	float n_j = NNet::NNetFunc(sumW);
	float e_j = n_j - y;
	float df = NNet::NNetDFunc(sumW);
	return e_j * df * X.m_x[k];
}
float NNetTrain0::evalEForQth_j(float y, s_NNetL1X& X) {
	float n_out = sumWs(X.m_x);
	float diff = n_out - y;
	return diff * diff;
}
float NNetTrain0::sumWs(float X[]) {
	float sum = 0.f;
	for (int i = 0; i < m_nX; i++) {
		sum += m_w[i] * X[i];
	}
	return sum;
}


EyeNetTrain::EyeNetTrain():m_net(NULL), m_stampEye(NULL), m_lowestLevel(0), m_stepSize(0.f), m_QN(0), m_steps(NULL), m_DeltaEs(NULL), m_E(NULL), m_Ws_N(0), m_DeltaE_closeEnough(0.f), m_max_loop_cnt(0)
{
	;
}
EyeNetTrain::~EyeNetTrain() {
	;
}
unsigned char EyeNetTrain::init(
	StampEye* stampEyep,/*contain patterns in the o's of its lowest level [2]*/
	float stepSize,
	float DeltaE_closeEnough,
	long  max_loop_cnt)
{
	m_stampEye = stampEyep;
	m_net = NULL;
	s_eyeStamp* eyeStampsp = m_stampEye->getLunaEyeStamps();
	if (eyeStampsp->n < 1)
		return ECODE_ABORT;
	if (eyeStampsp[0].eyes[0]->n < 1)
		return ECODE_ABORT;
	m_lowestLevel = eyeStampsp[0].eyes[0]->n - 1;
	m_NNetTrain0 = new NNetTrain0;
	if (Err(m_NNetTrain0->init(stepSize, DeltaE_closeEnough, max_loop_cnt)))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char EyeNetTrain::setDataForNode(int node_i) {
	if (m_net == NULL)
		return ECODE_FAIL;
	if (m_lowestLevel != m_net->n - 1)
		return ECODE_FAIL;
	s_eyeStamp* lunaDatStamps = m_stampEye->getLunaEyeStamps();
	long dataSize = (long)m_stampEye->numEyeStamps();
	long numStamps = (long)m_stampEye->numStamps();
	if (lunaDatStamps[0].eyes[0]->n != m_net->n)
		return ECODE_FAIL;
	s_hexPlate& lowestLunaPlate0 = lunaDatStamps[0].eyes[0]->lev[m_lowestLevel];
	int numNodesLowest = lowestLunaPlate0.m_nHex;
	if (numNodesLowest < 1 || numNodesLowest<=node_i)
		return ECODE_FAIL;
	if (numNodesLowest != m_net->lev[m_lowestLevel].m_nHex)
		return ECODE_FAIL;
	s_fNode& stampNode0 = lowestLunaPlate0.m_fhex[node_i];
	int nX = stampNode0.N;
	if (nX < 1)
		return ECODE_FAIL;
	if (nX != m_net->lev[m_lowestLevel].m_fhex[node_i].N)
		return ECODE_FAIL;
	s_NNetL1X* Xvec = NULL;
	NNet::initNNetL1Xs(dataSize, nX, Xvec);
	float* y = new float[dataSize];
	long i_dat = 0;
	for (long i_stamp = 0; i_stamp < numStamps; i_stamp++) {
		for (long i_sub = 0; i_sub < lunaDatStamps[i_stamp].n; i_sub++) {
			if (i_dat >= dataSize)
				return ECODE_ABORT;
			y[i_dat] = lunaDatStamps[i_stamp].o;
			s_hexPlate& lowestLunaPlate = lunaDatStamps[i_stamp].eyes[i_sub]->lev[m_lowestLevel];
			s_fNode& dataNode = lowestLunaPlate.m_fhex[node_i];
			for (int i_X = 0; i_X < nX; i_X++) {
				Xvec[i_dat].m_x[i_X] = dataNode.nodes[i_X]->o;
			}
			i_dat++;
		}
	}
	unsigned char ErrC=m_NNetTrain0->setNet(dataSize, Xvec, y);
	if (y != NULL) {
		delete[] y;
	}
	NNet::releaseNNetL1Xs(dataSize, Xvec);
	return ErrC;
}

unsigned char EyeNetTrain::getResultsIntoNode(int node_i) {
	s_hexPlate& lowestNetLevel = m_net->lev[m_lowestLevel];
	s_fNode& netNode = lowestNetLevel.m_fhex[node_i];
	float* Ws = m_NNetTrain0->getWs();
	int nX = m_NNetTrain0->getnX();
	if (nX != netNode.N)
		return ECODE_FAIL;
	for (int i = 0; i < nX; i++) {
		netNode.w[i] = Ws[i];
	}
	return ECODE_OK;
}
unsigned char EyeNetTrain::setDataForTopNode() {
	long dataSize = (long)m_stampEye->numEyeStamps();
	s_hexPlate& lowestNetLevel = m_net->lev[m_lowestLevel];
	int numNodes = lowestNetLevel.m_nHex;
	s_NNetL1X* Xvec = NULL;
	NNet::initNNetL1Xs(dataSize, numNodes, Xvec);
	float* y = new float[dataSize];

	long numStamps = (long)m_stampEye->numStamps();
	s_eyeStamp* lunaDatStamps = m_stampEye->getLunaEyeStamps();
	long i_dat = 0;
	for (long i_stamp = 0; i_stamp < numStamps; i_stamp++) {
		for (long i_sub = 0; i_sub < lunaDatStamps[i_stamp].n; i_sub++) {
			if (i_dat >= dataSize)
				return ECODE_ABORT;
			y[i_dat] = lunaDatStamps[i_stamp].o;
			NNet::oNNetL0(*m_net, *lunaDatStamps[i_stamp].eyes[i_sub]);
			for (int i_node = 0; i_node < numNodes; i_node++)
				Xvec[i_dat].m_x[i_node] = lowestNetLevel.m_fhex[i_node].o;
		}
	}
	if (Err(m_NNetTrain0->setNet(dataSize, Xvec, y)))
		return ECODE_FAIL;
	if (y != NULL)
		delete[] y;
	y = NULL;
	NNet::releaseNNetL1Xs(dataSize, Xvec);
	return ECODE_OK;
}
unsigned char EyeNetTrain::getResultsIntoTopNode() {
	float* Ws = m_NNetTrain0->getWs();
	int nX = m_NNetTrain0->getnX();
	if (m_net->n != 2)
		return ECODE_FAIL;
	s_hexPlate& topLevel = m_net->lev[0];
	if (topLevel.m_nHex < 1)
		return ECODE_FAIL;
	if (topLevel.m_fhex[0].N != nX)
		return ECODE_FAIL;
	for (int i = 0; i < nX; i++) {
		topLevel.m_fhex[0].w[i] = Ws[i];
	}
	return ECODE_OK;
}

unsigned char EyeNetTrain::runL0(s_hexEye* net) {
	if (net == NULL)
		return ECODE_FAIL;
	m_net = net;
	int numNodes = m_net->lev[m_lowestLevel].m_nHex;
	for (int i = 0; i < numNodes; i++) {
		if (Err(setDataForNode(i)))
			return ECODE_ABORT;
		if(Err(m_NNetTrain0->run()))
			return ECODE_FAIL;
		if (Err(getResultsIntoNode(i)))
			return ECODE_FAIL;
	}
	if (Err(setDataForTopNode()))
		return ECODE_FAIL; 
	if (Err(m_NNetTrain0->run()))
		return ECODE_FAIL;
	if (Err(getResultsIntoTopNode()))
		return ECODE_FAIL;
	return ECODE_OK;
}

