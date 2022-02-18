#include "ComServer.h"
namespace n_ComSteer {
	void zero(s_ComSteer& s) {
		s.steerActive = false;
		s.dist = 0.f;
		s.ang = 0.f;
	}
}
ComServer::ComServer() :m_img(NULL){
	for (int i = 0; i < DRIVEPLANE_NUMLUNALINE; i++)
		n_DrivePlate::zeroPlate(m_plates[i]);
	n_ComSteer::zero(m_steer);
}
ComServer::~ComServer() {
	;
}
bool ComServer::init(const unsigned char msg[], int msg_len) {
	/*check if init has already been done*/
	if (m_img != NULL)
		return false;
	/*if message len is too small this can't be the correct init message*/
	if (msg_len < 14)
		return false;
	/*check if header matches code for init height width*/
	bool isHeader = isHeaderWithCode(msg, 0x00, com_code_intro);
	if (!isHeader)
		return false;
	unsigned char w_msg[2] = { msg[4], msg[5] };
	unsigned char h_msg[2] = { msg[6], msg[7] };
	short _width = convertCharArrayToShort(w_msg);
	short _height = convertCharArrayToShort(h_msg);
	unsigned char n_msg[4] = { msg[8], msg[9], msg[10], msg[11] };
	int sent_num_hexes = convertCharArrayToInt32(n_msg);
	m_img = new Img;/*dummy image is just used to set dimensions for the first pass generation of the hexes*/
	m_img->init((long)_width, (long)_height, 3L);
	m_hexAr = new Hex;
	m_hexAr->Init(m_img);/*the structures of the node plates and the basic hex plate should be the same, with the same number of nodes/hexes*/
	int num_hexes = m_hexAr->getNHex();
	if (sent_num_hexes != num_hexes) {
		release();
		return false;
	}
	unsigned char num_msg[2] = { msg[12], msg[13] };
	short snum_msg = convertCharArrayToShort(num_msg);
	m_num_msgs = (int)snum_msg;

	initDrivePlates();
	n_ComSteer::zero(m_steer);
	m_dataFull = false;
	return false;
}
void ComServer::release() {
	releaseDrivePlates();
	if (m_hexAr != NULL) {
		m_hexAr->Release();
		delete m_hexAr;
		m_hexAr = NULL;
	}
	if (m_img != NULL) {
		m_img->release();
		delete m_img;
		m_img = NULL;
	}
}

unsigned char ComServer::initDrivePlates() {
	for (int i = 0; i < DRIVEPLANE_NUMLUNALINE; i++) {
		m_hexAr->genStructuredPlate(m_plates[i].p);
	}
	return ECODE_OK;
}
void ComServer::releaseDrivePlates() {
	for (int i = 0; i < DRIVEPLANE_NUMLUNALINE; i++) {
		m_hexAr->releaseStructuredPlate(m_plates[i].p);
	}
}
bool ComServer::transNext(const unsigned char msg[], int msg_len) {
	if (m_msg_cnt >= m_num_msgs) {
		if (!m_dataFull) {
			m_dataFull = true;
		}
		else {
			reset();
		}
		return false;
	}
	if (m_msg_cnt < DRIVEPLANE_NUMLUNALINE) {
		if (!recvPlate(msg, msg_len)) {
			reset();
			return false;
		}
	}
	else {
		if (!recvSteering(msg, msg_len)) {
			reset();
			return false;
		}
	}
	return true;
}

void ComServer::reset() {
	m_img->clearToChar(0x00);
	s_hex* hexes = m_hexAr->getHexes();
	for (int i = 0; i < m_hexAr->getNHex(); i++) {
		hexes[i].rgb[0] = 0x00;
	}
	m_steer.steerActive = false;
	m_msg_cnt = 0;
	false;
}
bool ComServer::recvSteering(const unsigned char msg[], int msg_len) {
	if (msg_len < 13)
		return false;
	if (!isHeaderWithCode(msg, 0x00, com_dat_code_steer))
		return false;
	bool steerActive = msg[4] > 0x00;
	unsigned char ar[4];
	for (int i = 0; i < 4; i++)
		ar[i] = msg[5 + i];
	float dist = convert2CharDec2CharToFloat(ar);
	for (int i = 0; i < 4; i++)
		ar[i] = msg[9 + i];
	float ang = convert2CharDec2CharToFloat(ar);
	m_steer.steerActive = steerActive;
	m_steer.dist = dist;
	m_steer.ang = ang;
	m_msg_cnt++;
	return true;
}
bool ComServer::recvPlate(const unsigned char msg[], int msg_len) {
	bool isPlateHeader = isHeaderWithCode(msg, 0x00, com_draw_code_driveP);
	if (!isPlateHeader)
		return false;
	int Plate_i = (int)msg[4];
	if (Plate_i < 0 || Plate_i >= DRIVEPLANE_NUMLUNALINE)
		return false;
	long msg_start = 5;
	s_hex* baseHexes = m_hexAr->getHexes();
	/*red char in m_hexAR will be used to record highest value of line*/
	for (long i = 0; i < m_hexAr->getNHex(); i++) {
		long msg_i = msg_start + i;
		if (msg_i >= msg_len)
			break;
		float msg_val = convertCharToFloat(msg[msg_i]);
		if (baseHexes[i].rgb[0] <= msg_val) {
			long x_i = baseHexes[i].i;
			long y_j = baseHexes[i].j;
			s_rgb hexCol = convertPlateCharToRGB(Plate_i, msg[msg_i]);
			m_img->PrintMaskedImg(x_i, y_j, *(m_hexAr->getHexMaskPlus()), hexCol);
			baseHexes[i].rgb[0] = msg_val;
		}
	}
	m_msg_cnt++;
	return true;
}

s_rgb ComServer::convertPlateCharToRGB(int plate_i, const unsigned char ch) {
	s_rgb lunCol = genLunaCol(plate_i);
	return lunCol;/*not going to scale yet*/
}
s_rgb ComServer::genLunaCol(int lunai) {
	s_rgb rgb = { 0xff, 0xff, 0xff };
	float mul1 = 0x33;
	float mul2 = 0x44;
	if (lunai < 3) {
		float r = mul1 * (float)lunai;
		float g = mul2 * (float)lunai;
		rgb = imgMath::convToRGB(0xff,r, g);
	}
	else {
		float li = (float)lunai - 3;
		float g = mul2 * li;
		float b = mul1 * li;
		rgb = imgMath::convToRGB(g, 0xff, b);
	}
	return rgb;
}