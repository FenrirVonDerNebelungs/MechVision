#include "Com.h"
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