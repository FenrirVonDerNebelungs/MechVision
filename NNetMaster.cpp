#include "NNetMaster.h"
NNetMaster::NNetMaster() :m_patLuna(NULL), m_stampEyeL1(NULL), m_NNetsL1(NULL), m_trainL1(NULL),m_numNets(0)
{
	;
}
NNetMaster::~NNetMaster() {
	;
}
unsigned char NNetMaster::init() {
	m_patLuna = new PatternLuna;
	m_stampEyeL1 = new StampEye;
	m_NNetsL1 = new HexEye;
	m_trainL1 = new NNetTrain;

	m_patLuna->init();
	m_stampEyeL1->init(m_patLuna);
	m_stampEyeL1->initNNets(m_NNetsL1);/*this initializes the hexEye*/
	m_numNets = m_NNetsL1->getNEyes();
	/*check that this matches the number of stamps*/
	int numStamps = m_stampEyeL1->numStamps();
	if (numStamps != m_numNets)
		return ECODE_FAIL;
	if (m_numNets < 1)
		return ECODE_ABORT;
	s_hexEye initNet = m_NNetsL1->getEye(0);
	if (Err(m_trainL1->init(m_stampEyeL1, &initNet)))
		return ECODE_FAIL;

	return ECODE_OK;
}
void NNetMaster::release() {
	if (m_trainL1 != NULL) {
		m_trainL1->release();
		delete m_trainL1;
	}
	m_trainL1 = NULL;
	m_numNets = 0;
	if (m_stampEyeL1 != NULL) {
		m_stampEyeL1->releaseNNets(m_NNetsL1);
	}
	if (m_NNetsL1 != NULL)
		delete m_NNetsL1;
	m_NNetsL1 = NULL;
	if (m_stampEyeL1 != NULL) {
		m_stampEyeL1->release();
		delete m_stampEyeL1;
	}
	m_stampEyeL1 = NULL;
	if (m_patLuna != NULL) {
		m_patLuna->release();
		delete m_patLuna;
	}
	m_patLuna = NULL;
}

/*results are written into the HexEyes of NNetsL1*/
unsigned char NNetMaster::trainNets() {
	for (int i = 0; i < m_numNets; i++) {
		/*setups the stamp so that it is programed to train the correct eye*/
		m_stampEyeL1->setupForStampi(i);
		if (!m_trainL1->run(m_NNetsL1->getEyePtr(i)))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}