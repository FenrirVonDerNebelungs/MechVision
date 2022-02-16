#pragma once
#ifndef COMDRAW_H
#define COMDRAW_H
#ifndef DRIVEPLANE_H
#include "DrivePlane.h"
#endif
#ifndef HEX_H
#include "Hex.h"
#endif
const unsigned char com_draw_header[] = { 0xFF, 0xA1, 0x00, 0x00 };
const unsigned char com_draw_code_wh = 0x01;
const unsigned char com_draw_code_driveP = 0x02;
/*class sets up strings that are sent and recived by the bluetooth */
class ComDraw : public Base {
public:
	ComDraw();
	~ComDraw();
	bool init_client(unsigned char msg[], 
		const Hex* hexScreen, 
		bool send_drive_txt=true,
		bool send_line_img=true);                 /*msg is what the client will sends
												    it should be at least 12 chars long and will be filled with the
												    header (4 chars), 2 chars for the width and 2 chars for the height,
												    4 chars for the number of hexes*/

	bool init_server(const unsigned char msg[], int msg_len);/*msg is what the server recives from the initialized client
												 it should be at least 12 chars long
												 the server uses the msg to determine the width/height of the
												 display and generate the appropriate mesh of hexes and
												 check that the expected number of hexes matches from server to
												 client*/
	void release_server();

	unsigned char sendPlate_client(s_DrivePlate* plates, int Plate_i, unsigned char msg[]); /*returns message length*/
	bool recvPlate_server(const unsigned char msg[], int msg_len);
	bool renderPlate_server();
protected:
	bool m_send_drive_txt;
	bool m_send_line_img;

	/*used by  the server*/
	Img* m_img;
	Hex* m_hexAr;
	s_DrivePlate m_plates[DRIVEPLANE_NUMLUNALINE];/*full or partially filled plates filled from client side data*/

	unsigned char initDrivePlates_server();
	void releaseDrivePlates_server();

	void convertShortToCharArray(const short s, unsigned char ar[]);/*ar is expected to be 2 chars long*/
	short convertCharArrayToShort(const unsigned char ar[]);/*ar is 2 chars long at least*/
	void convertInt32ToCharArray(const int s, unsigned char ar[]);/*ar is expected to be 4 chars long*/
	int convertCharArrayToInt32(const unsigned char ar[]);/*ar is 4 chars long at least*/
	unsigned char convertFloatRange1ToChar(float f);/*converts a float that is expected to go from 0 to 1 to a char*/
	unsigned char convertFloatToChar(float f);
	inline float convertCharToFloat(unsigned char c) { return (float)c; }

	int sendHeaderWithCode(unsigned char head[],
		unsigned char code0 = 0x00,
		unsigned char code1 = 0x00);
	bool isHeaderWithCode(const unsigned char head[],
		unsigned char code0 = 0x00,
		unsigned char code1 = 0x00);

	void clearRender_server();
	s_rgb convertPlateCharToRGB(int plate_i, const unsigned char ch);
};
#endif
unsigned char ComDraw::initDrivePlates_server() {
	for (int i = 0; i < DRIVEPLANE_NUMLUNALINE; i++) {
		m_hexAr->genStructuredPlate(m_plates[i].p);
	}
	return ECODE_OK;
}
void ComDraw::releaseDrivePlates_server() {
	for (int i = 0; i < DRIVEPLANE_NUMLUNALINE; i++) {
		m_hexAr->releaseStructuredPlate(m_plates[i].p);
	}
}
unsigned char ComDraw::convertFloatRange1ToChar(float f) {
	float ff = f * 255.f;
	unsigned char cf = 0xFF;
	if (ff >= 255.f)
		return cf;
	if (ff <= 0.f)
		return 0x00;
	cf = (unsigned char)ff;
	return cf;
}
unsigned char ComDraw::convertFloatToChar(float f) {
	float fr = roundf(f);
	if (fr > 255.f)
		fr = 255.f;
	if (fr < 0)
		fr = 0.f;
	return (unsigned char)fr;
}
int ComDraw::sendHeaderWithCode(unsigned char head[], unsigned char code0, unsigned char code1) {
	for (int i = 0; i < 2; i++)
		head[i] = com_draw_header[i];
	head[2] = code0;
	head[3] = code1;
	return 4;
}
bool ComDraw::isHeaderWithCode(const unsigned char head[], unsigned char code0, unsigned char code1) {
	bool isHeader = true;
	for (int i = 0; i < 2; i++) {
		if (head[i] != com_draw_header[i])
			isHeader = false;
	}
	if (head[2] != code0)
		isHeader = false;
	if (head[3] != code1)
		isHeader = false;
	return isHeader;
}
unsigned char ComDraw::sendPlate_client(s_DrivePlate* plates, int Plate_i, unsigned char msg[])
{
	/*send header identifying as plate*/
	sendHeaderWithCode(msg, 0x00, com_draw_code_driveP);
	/*after header send the i of the plate using one char*/
	msg[4] = (unsigned char)Plate_i;
	/*plate data will now start at 5*/
	int msg_start = 5;
	s_hexPlate& hexP = plates[Plate_i].p;
	if (hexP.m_nHex != m_hexAr->getNHex())
		return ECODE_ABORT;
	for (int i = 0; i < hexP.m_nHex; i++) {
		unsigned char sendc = convertFloatRange1ToChar(hexP.m_fhex->o);
		msg[msg_start + i] = sendc;
	}
	return ECODE_OK;
}
void ComDraw::clearRender_server() {
	m_img->clearToChar(0x00);
	s_hex* hexes = m_hexAr->getHexes();
	for (int i = 0; i < m_hexAr->getNHex(); i++) {
		hexes[i].rgb[0] = 0x00;
	}
}
bool ComDraw::recvPlate_server(const unsigned char msg[], int msg_len) {
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
	return true;
}
bool ComDraw::init_server(const unsigned char msg[], int msg_len) {
	/*check if init has already been done*/
	if (m_img != NULL)
		return false;
	/*if message len is too small this can't be the correct init message*/
	if (msg_len < 12)
		return false;
	/*check if header matches code for init height width*/
	bool isHeader = isHeaderWithCode(msg, 0x00, com_draw_code_wh);
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
		release_server();
		return false;
	}
	initDrivePlates_server();
	return true;
}
bool ComDraw::init_client(unsigned char msg[], 
	const Hex* hexScreen,
	bool send_drive_txt,
	bool send_line_img) 
{
	m_send_drive_txt = send_drive_txt;
	m_send_line_img = send_line_img;

	sendHeaderWithCode(msg, 0x00, com_draw_code_wh);

	short width = (short)hexScreen->getWHex();
	short height = (short)hexScreen->getHHex();
	unsigned char ar[2];
	convertShortToCharArray(width, ar);
	for (int i = 0; i < 2; i++)
		msg[i + 4] = ar[i];
	convertShortToCharArray(height, ar);
	for (int i = 0; i < 2; i++)
		msg[i + 6] = ar[i];
	int num_hexes = hexScreen->getNHex();
	unsigned char ar4[4];
	convertInt32ToCharArray(num_hexes, ar4);
	for (int i = 0; i < 4; i++)
		msg[i + 8] = ar4[i];
	return true;
}
void ComDraw::convertInt32ToCharArray(const int s, unsigned char ar[]) {
	int hi_ = s & 0xFF000000;
	int hi = hi_ >> 24;
	ar[0] = (unsigned char)hi;
	hi_ = s & 0x00FF0000;
	hi = hi_ >> 16;
	ar[1] = (unsigned char)hi;
	hi_ = s & 0x0000FF00;
	hi = hi_ >> 8;
	ar[2] = (unsigned char)hi;
	hi_ = s & 0x000000FF;
	ar[3] = (unsigned char)hi_;
}
int ComDraw::convertCharArrayToInt32(const unsigned char ar[]) {
	int hi = (int)ar[0];
	int hi_ = hi << 24;
	int mer = hi_;
	hi = (int)ar[1];
	hi_ = hi << 16;
	mer = mer | hi_;
	hi = (int)ar[2];
	hi_ = hi << 8;
	mer = mer | hi_;
	hi = (int)ar[3];
	mer = mer | hi;
	return mer;
}
short ComDraw::convertCharArrayToShort(const unsigned char ar[]) {
	short hi = (short)ar[0];
	short hi_m = hi << 8;
	short lw = (short)ar[1];
	return hi_m | lw;
}
void ComDraw::convertShortToCharArray(const short s, unsigned char ar[]) {
	short hi_ = s & 0xFF00;
	short hi = hi_ >> 8;
	short lw = s & 0x00FF;
	ar[0] = (unsigned char)hi;
	ar[1] = (unsigned char)lw;
}