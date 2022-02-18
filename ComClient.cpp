#include "ComClient.h"
ComClient::ComClient() :m_DPlane(NULL), m_plates(NULL), m_DLines(NULL) { ; }
ComClient::~ComClient() {
	;
}
bool ComClient::init(unsigned char msg[],
	Hex* hexAr,
	DrivePlane* DPlane,
	DriveLines* DLines,
	bool send_drive_txt,
	bool send_line_img)
{
	m_hexAr = hexAr;
	m_DPlane = DPlane;
	m_plates = DPlane->getPlates();
	m_DLines = DLines;
	sendHeaderWithCode(msg, 0x00, com_code_intro);

	short width = (short)hexAr->getWHex();
	short height = (short)hexAr->getHHex();
	unsigned char ar[2];
	convertShortToCharArray(width, ar);
	for (int i = 0; i < 2; i++)
		msg[i + 4] = ar[i];
	convertShortToCharArray(height, ar);
	for (int i = 0; i < 2; i++)
		msg[i + 6] = ar[i];
	int num_hexes = hexAr->getNHex();
	unsigned char ar4[4];
	convertInt32ToCharArray(num_hexes, ar4);
	for (int i = 0; i < 4; i++)
		msg[i + 8] = ar4[i];
	/*find number of messages to send*/
	m_num_msgs = DRIVEPLANE_NUMLUNALINE;/*after intro send are the drive plates*/
	m_num_msgs++;/*also send the estimated distance from line*/
	short msg_num = (short)m_num_msgs;
	unsigned char msgLenar[2];
	convertShortToCharArray(msg_num, msgLenar);
	for (int i = 0; i < 2; i++)
		msg[i + 12] = msgLenar[i];
	m_msg_cnt = 0;
	return true;
}
void ComClient::release() {
	;
}

int ComClient::TransNext(unsigned char msg[]) {
	if (m_msg_cnt >= m_num_msgs) {
		reset();
		return -1;
	}
	int msg_len = 0;
	if (m_msg_cnt < DRIVEPLANE_NUMLUNALINE)
		sendPlate(m_plates, m_msg_cnt, msg);
	else
		msg_len = sendSteering(msg);
	return msg_len;
}
void ComClient::reset() {
	m_msg_cnt = 0;
}
int ComClient::sendSteering(unsigned char msg[]) {
	float dist, ang;
	unsigned char steerActive = 0x00;
	if (m_DLines->steerTargetDistAng(dist, ang))
		steerActive = 0x01;
	sendHeaderWithCode(msg, 0x00, com_dat_code_steer);
	/*first char is 1 if steering values returned and zero otherwise*/
	msg[4] = steerActive;
	unsigned char wholear[4];
	convertFloatTo2CharDec2Char(dist, wholear);
	unsigned char decar[4];
	convertFloatTo2CharDec2Char(ang, wholear);
	for (int i = 0; i < 4; i++) {
		msg[5 + i] = wholear[i];
		msg[9 + i] = decar[i];
	}
	m_msg_cnt++;
	return 13;
}
int ComClient::sendPlate(s_DrivePlate* plates, int Plate_i, unsigned char msg[])
{
	/*send header identifying as plate*/
	sendHeaderWithCode(msg, 0x00, com_draw_code_driveP);
	/*after header send the i of the plate using one char*/
	msg[4] = (unsigned char)Plate_i;
	/*plate data will now start at 5*/
	int msg_start = 5;
	s_hexPlate& hexP = plates[Plate_i].p;
	if (hexP.m_nHex != m_hexAr->getNHex())
		return -1;
	for (int i = 0; i < hexP.m_nHex; i++) {
		unsigned char sendc = convertFloatRange1ToChar(hexP.m_fhex->o);
		msg[msg_start + i] = sendc;
	}
	m_msg_cnt++;
	return msg_start+hexP.m_nHex;
}