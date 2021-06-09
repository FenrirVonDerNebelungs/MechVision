#include "stdafx.h"
#include "NNet.h"
NNetTest::NNetTest():m_pNet(NULL), m_NNetTestTrain(NULL), m_hexMaster(NULL), m_result(0.f)
{
	;
}
NNetTest::~NNetTest()
{
	;
}
unsigned char NNetTest::Init(HexStack* hexMaster) {
	m_hexMaster = hexMaster;
	m_pNet = new NNetTreeTest;
	m_pNet->Init(m_hexMaster);
	m_NNetTestTrain = new NNetTestTrain;
    m_NNetTestTrain->Init(m_hexMaster, m_pNet);
    return ECODE_OK;
}
void NNetTest::Release()
{
	m_hexMaster=NULL;
	if(m_NNetTestTrain!=NULL){
		m_NNetTestTrain->Release();
		delete m_NNetTestTrain;
	}
	m_NNetTestTrain=NULL;
	if(m_pNet!=NULL){
		m_pNet->Release();
		delete m_pNet;
	}
	m_pNet=NULL;
}
unsigned char NNetTest::Run(){
	TrainNet();
	RunTrainedNet();
	return ECODE_OK;
}
unsigned char NNetTest::RunTrainedNet(){
	float* ws = m_NNetTestTrain->getWeights();
        m_pNet->setMidNodeWeights(0, ws);
	m_pNet->run();
	m_result = m_pNet->getMidNodef(0);
	return ECODE_OK;
}
unsigned char NNetTest::TrainNet()
{
	m_pNet->run();
	m_NNetTestTrain->Run();
	return ECODE_OK;
}
NNetTestTrain::NNetTestTrain() : m_hexMaster(NULL), m_pNet(NULL), m_imgHandler(NULL),
m_numSet(0), m_maxNumLoops(0), m_numLoops(0), m_step(0.f), m_ws(NULL), m_converged(NULL)
{
	for (int i = 0; i < MAXNUMTRAINIMGS; i++) {
		m_imgSet[i] = NULL;
		m_targetOutput[i] = 0.f;
	}
}
NNetTestTrain::~NNetTestTrain()
{
	;
}
unsigned char NNetTestTrain::Init(
	HexStack* hexMaster,
	NNetTreeTest* pNet,
	int maxNumLoops,
	float step
) 
{
	m_hexMaster = hexMaster;
	m_pNet = pNet;
	m_maxNumLoops = maxNumLoops;
	m_numLoops = 0;
	m_step = step;
	m_imgHandler = new NNetTestTrainImgs;
	m_imgHandler->Init();
	m_numSet = m_imgHandler->transferImgs(m_imgSet);
	m_imgHandler->transferTargetOutputs(m_targetOutput);
	m_ws = NULL;
	m_converged = NULL;

	return ECODE_OK;
}
void NNetTestTrain::Release()
{
	if (m_converged != NULL)
		delete[] m_converged;
	m_converged = NULL;
	if (m_ws != NULL)
		delete[] m_ws;
	m_ws = NULL;

	for (int i = 0; i < m_numSet; i++) {
		if (m_imgSet[i] != NULL) {
			m_imgSet[i]->release();
			delete m_imgSet[i];
			m_imgSet[i] = NULL;
		}
	}
	if (m_imgHandler != NULL) {
		m_imgHandler->Release();
		delete m_imgHandler;
	}
	m_imgHandler = NULL;
}
unsigned char NNetTestTrain::Run() {
	runTraining();
	return ECODE_OK;
}
void NNetTestTrain::runTraining() {
	int numLowerNodes = m_pNet->getNpatternNodes();

	int ns_array_size = numLowerNodes * m_numSet;
	float* ns_array = new float[ns_array_size];
	RunLowerNet(ns_array);
	float* ws = new float[numLowerNodes];
	for (int i = 0; i < numLowerNodes; i++)
		ws[i] = 0.f;
	bool* converged = new bool[numLowerNodes];
	for (int i = 0; i < numLowerNodes; i++)
		converged[i] = false;
	adjustWeights(m_step, ws, converged);
	m_ws = ws;
	m_converged = converged;
}

void NNetTestTrain::adjustWeights(float step, float ws[], bool converged[]) {
	int numLowerNodes = m_pNet->getNpatternNodes();
	int len_ns_array = numLowerNodes * m_numSet;
	float* ns_array = new float[len_ns_array];

	for (int i = 0; i < numLowerNodes; i++) {
		ws[i] = 0.f;
		converged[i] = false;
	}

	do {
		m_numLoops++;
		stepAdjustWeights(step, ns_array, ws, converged);
		bool isConverged = true;
		for (int i = 0; i < numLowerNodes; i++) {
			if (!(converged[i])) {
				isConverged = false;
				break;
			}
		}
		if (isConverged)
			break;
	} while (m_maxNumLoops > m_numLoops);
}
void NNetTestTrain::stepAdjustWeights(float step, float ns_array[], float ws[], bool converged[]) {
	int numLowerNodes = m_pNet->getNpatternNodes();
	float* ns = new float[numLowerNodes];
	float* wn_sums = new float[m_numSet];
	for (int i = 0; i < m_numSet; i++) {
		for (int ni = 0; ni < numLowerNodes; ni++) {
			int ns_array_index = ni * m_numSet + i;
			ns[ni] = ns_array[ns_array_index];
		}
		wn_sums[i] = m_pNet->evalWeightedNs(numLowerNodes, ws, ns);
	}
	for (int wi = 0; wi < numLowerNodes; wi++) {
		int ns_array_index_start = wi * m_numSet;
		float S_err2 = 0.f;
		float S_err2_p = 0.f;
		float S_err2_n = 0.f;
		for (int i = 0; i < m_numSet; i++) {
			int ns_array_index = ns_array_index_start + i;
			float netout = Math::StepFunc(wn_sums[i]);
			float err2 = m_targetOutput[i] - netout;
			err2 = err2 * err2;
			S_err2 += err2;
			float perterb = step * ns_array[ns_array_index];
			float netout_p = Math::StepFunc(wn_sums[i] + perterb);
			float netout_n = Math::StepFunc(wn_sums[i] - perterb);
			float err2_p = m_targetOutput[i] - netout_p;
			err2_p = err2_p * err2_p;
			float err2_n = m_targetOutput[i] - netout_n;
			err2_n = err2_n * err2_n;
			S_err2_p += err2_p;
			S_err2_n += err2_n;
		}
		converged[wi] = false;
		if (S_err2_n < S_err2) {
			ws[wi] -= step;
		}
		else if (S_err2_p < S_err2) {
			ws[wi] += step;
		}
		else {
			converged[wi] = true;
		}
	}
}

void NNetTestTrain::compute_ns(Img* inImg, float ns[]) {
	m_hexMaster->Update(inImg);
	m_pNet->run();
	s_nNode topN = m_pNet->getMidNode(0);
	for (int i = 0; i < topN.N; i++) {
		s_nNode* lowerNode = topN.nodes[i];
		ns[i] = lowerNode->f;
	}
}
void NNetTestTrain::RunLowerNet(float ns_array[]) {
	int numLowerNodes = m_pNet->getNpatternNodes();
	float* ns = new float[numLowerNodes];
	for (int i = 0; i < m_numSet; i++) {
		compute_ns(m_imgSet[i], ns);
		for (int ni = 0; ni < numLowerNodes; ni++) {
			int ns_array_index = ni * m_numSet + i;
			ns_array[ns_array_index] = ns[ni];
		}
	}
}