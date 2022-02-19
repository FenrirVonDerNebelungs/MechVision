#include "Com.h"
Com::Com() :m_dataFull(false), m_hexAr(NULL), m_num_msgs(0), m_msg_cnt(0) {
	;
}
Com::~Com() {
	;
}
bool Com::init() { return true; }
void Com::reset(){ ; }
void Com::convertShortToCharArray(const short s, unsigned char ar[]) {
	short hi_ = s & 0xFF00;
	short hi = hi_ >> 8;
	short lw = s & 0x00FF;
	ar[0] = (unsigned char)hi;
	ar[1] = (unsigned char)lw;
}
short Com::convertCharArrayToShort(const unsigned char ar[]) {
	short hi = (short)ar[0];
	short hi_m = hi << 8;
	short lw = (short)ar[1];
	return hi_m | lw;
}
void Com::convertInt32ToCharArray(const int s, unsigned char ar[]) {
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
int Com::convertCharArrayToInt32(const unsigned char ar[]) {
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
unsigned char Com::convertFloatRange1ToChar(float f) {
	float ff = f * 255.f;
	unsigned char cf = 0xFF;
	if (ff >= 255.f)
		return cf;
	if (ff <= 0.f)
		return 0x00;
	cf = (unsigned char)ff;
	return cf;
}
unsigned char Com::convertFloatToChar(float f) {
	float fr = roundf(f);
	if (fr > 255.f)
		fr = 255.f;
	if (fr < 0)
		fr = 0.f;
	return (unsigned char)fr;
}

bool Com::convertFloatTo2CharDec2Char(float f, unsigned char ar[]) {
	short hi, lo;
	if (!convertFloatTo2shorts(f, hi, lo))
		return false;
	unsigned char hiar[2];
	convertShortToCharArray(hi, hiar);
	ar[0] = hiar[0];
	ar[1] = hiar[1];
	unsigned char loar[2];
	convertShortToCharArray(lo, loar);
	ar[2] = loar[0];
	ar[3] = loar[1];
	return true;
}
float Com::convert2CharDec2CharToFloat(unsigned char ar[]) {
	unsigned char ar2char[2];
	ar2char[0] = ar[0];
	ar2char[1] = ar[1];
	short hi = convertCharArrayToShort(ar2char);
	ar2char[0] = ar[2];
	ar2char[0] = ar[3];
	short lo = convertCharArrayToShort(ar2char);
	float fwhole = getFloatWholeFromHi(hi);
	float fdec = getDecFromLow(lo);
	return fwhole + fdec;
}

bool Com::convertFloatTo2shorts(float f, short& hi, short& lo) {
	bool ispos = (f >= 0.f);
	float af = fabsf(f);
	float fwhole, fdec;
	fdec = std::modf(af, &fwhole);
	short iwhole = (short)fwhole;
	if (!ispos) {
		iwhole = 0x8000 | iwhole;
	}
	fdec *= 1000.f;
	short idec = (short)floorf(fdec);
	hi = iwhole;
	lo = idec;
	return true;
}
float Com::getFloatWholeFromHi(short hi) {
	short hbit = 0x8000 & hi;
	bool isPos = (hbit < 0x01);
	short iwhole = 0x7FFF & hi;
	if (!isPos)
		iwhole = -iwhole;
	return (float)iwhole;
}
float Com::getDecFromLow(short low) {
	float fdecmul = (float)low;
	float fdec = low / 1000.f;
	return fdec;
}

int Com::sendHeaderWithCode(unsigned char head[], unsigned char code0, unsigned char code1) {
	for (int i = 0; i < 2; i++)
		head[i] = com_draw_header[i];
	head[2] = code0;
	head[3] = code1;
	return 4;
}
bool Com::isHeaderWithCode(const unsigned char head[], unsigned char code0, unsigned char code1) {
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