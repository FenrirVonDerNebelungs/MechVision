#pragma once
#ifndef COM_H
#define COM_H
#ifndef DRIVELINES_H
#include "DriveLines.h"
#endif
#ifndef HEX_H
#include "Hex.h"
#endif
const unsigned char com_draw_header[] = { 0xFF, 0xA1, 0x00, 0x00 };
const unsigned char com_code_intro = 0x01;
const unsigned char com_code_eom = 0x03;
const unsigned char com_draw_code_driveP = 0x05;
const unsigned char com_dat_code_steer = 0x06;
/*class sets up strings that are sent and recived by the bluetooth */
class Com : public Base {
public:
	Com();
	~Com();

	virtual void release()
	{
		;
	}
protected:
    /*owned by Server not owned by client*/
	Hex* m_hexAr;/*reference hex array that is structured the same on the server and the client side*/

	/*owned*/
	int m_num_msgs;/*total number of messages that should be sent excluding the intro*/
	int m_msg_cnt;/*counts the number of messages that have been transmitted so far*/

	void convertShortToCharArray(const short s, unsigned char ar[]);/*ar is expected to be 2 chars long*/
	short convertCharArrayToShort(const unsigned char ar[]);/*ar is 2 chars long at least*/
	void convertInt32ToCharArray(const int s, unsigned char ar[]);/*ar is expected to be 4 chars long*/
	int convertCharArrayToInt32(const unsigned char ar[]);/*ar is 4 chars long at least*/
	unsigned char convertFloatRange1ToChar(float f);/*converts a float that is expected to go from 0 to 1 to a char*/
	unsigned char convertFloatToChar(float f);
	inline float convertCharToFloat(unsigned char c) { return (float)c; }

	bool convertFloatTo2CharDec2Char(float f, unsigned char ar[]);/*converts a float into a 4 char array by sending the non decimal
																  into the 1st 2 chars, and the decimal * 1000 into the
																  last 2 chars */
	float convert2CharDec2CharToFloat(unsigned char ar[]);/*undoes the above conversion*/
	/*helpers to convert char to float*/
	bool convertFloatTo2shorts(float f, short& hi, short& lo);/*here the sign is just - if the highest bit is 1, however this is not the typicall
										unsigned number*/

	/*convert back float to char*/
	float getFloatWholeFromHi(short hi);/*takes hi short part and test for positive*/
	float getDecFromLow(short low);

	int sendHeaderWithCode(unsigned char head[],
		unsigned char code0 = 0x00,
		unsigned char code1 = 0x00);
	bool isHeaderWithCode(const unsigned char head[],
		unsigned char code0 = 0x00,
		unsigned char code1 = 0x00);
};
#endif
