#include "RoachNet_trainTF.h"

unsigned char RoachNet_trainTF::init(int frame_width, int frame_height) {
	m_frame_width = frame_width;
	m_frame_height = frame_height;
	m_imgLow = new Img;
	m_imgLow->init(m_frame_width, m_frame_height, 3L);
	m_hexLow = new Hex;
	m_hexLow->Init(m_imgLow);
	m_patLuna = new PatternLuna;
	m_patLuna->init();
	m_stampEye = new StampEye;
	m_stampEye->init(m_patLuna, m_hexLow);
	m_parse = new ParseTxt;
	string infName(ROACHNET_TRAINTF_INFNAME);
	string outfName(ROACHNET_TRAINTF_OUTFNAME);
	m_parse->init(infName, outfName);
	return ECODE_OK;
}
void RoachNet_trainTF::release() {
	if (m_parse != NULL) {
		m_parse->release();
		delete m_parse;
	}
	m_parse = NULL;
	if (m_stampEye != NULL) {
		m_stampEye->release();
		delete m_stampEye;
	}
	m_stampEye = NULL;
	if (m_patLuna != NULL) {
		m_patLuna->release();
		delete m_patLuna;
	}
	m_patLuna = NULL;
	if (m_hexLow != NULL) {
		m_hexLow->Release();
		delete m_hexLow;
	}
	m_hexLow = NULL;
	if (m_imgLow != NULL) {
		m_imgLow->release();
		delete m_imgLow;
	}
	m_imgLow = NULL;
	m_frame_width = 0;
	m_frame_height = 0;
}
unsigned char RoachNet_trainTF::gen() {
	m_stampEye->spawn();
	int numStamps = m_stampEye->numStamps();
	HexEye* netEye = new HexEye;
	if (Err(m_stampEye->initNNets(netEye))) {
		if (netEye != NULL) {
			netEye->release();
			delete netEye;
		}
		return ECODE_FAIL;
	}
	s_hexEye& s_netEye = netEye->getEye(0);
	int lowest_net_lev = s_netEye.n - 1;
	s_hexPlate& lowest_net_plate = s_netEye.lev[lowest_net_lev];
	int num_lowest_hex = lowest_net_plate.m_nHex;
	/*assume all hanging nodes have the same number which should be equal to the number of luna nodes of interest*/
	int num_hanging_per_hex = lowest_net_plate.m_fhex[0].N;
	/*calculate the total number that the nodes contribute to the array*/
	int num_hanging = num_hanging_per_hex * num_lowest_hex;
	int dump_ar_len = 1 + num_hanging + ROACHNET_TRAINTF_NUMTRAILVALS;
	float* dump_ar = new float[dump_ar_len];
	for (int i = 0; i < numStamps; i++) {
		int dump_ar_index = 0;
		dump_ar[dump_ar_index] = (float)i;
		dump_ar_index++;
		s_eyeStamp eyeStamp = m_stampEye->getLunaEyeStamp(i);
		for (int i_sub = 0; i_sub < eyeStamp.n; i_sub++) {
			s_hexEye* lunaEye = eyeStamp.eyes[i_sub];
			s_hexPlate& lunaPlate = lunaEye->lev[lowest_net_lev];
			for (int i_hex = 0; i_hex < lunaPlate.m_nHex; i_hex++) {
				s_fNode& hex_node = lunaPlate.m_fhex[i_hex];
				for (int i_hanging = 0; i_hanging < hex_node.N; i_hanging++) {
					//int dump_ar_index = 1 + i_hex * num_hanging_per_hex + i_hanging;
					dump_ar[dump_ar_index] = hex_node.nodes[i_hanging]->o;
					dump_ar_index++;
				}
			}
			
			/*add the trailing values*/
			
		}
	}
	if (dump_ar != NULL)
		delete dump_ar;
	m_stampEye->releaseNNets(netEye);
	if (netEye != NULL)
		delete netEye;
}
