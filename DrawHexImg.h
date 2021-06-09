#pragma once
#ifndef DRAWHEXIMG_H
#define DRAWHEXIMG_H
#ifndef HEXBASE_H
#include "HexBase.h"
#endif

class DrawHexImg {
public:
	DrawHexImg();
	~DrawHexImg();
	unsigned char Init(
		long width,
		long height,
		s_hex* hex,
		int    nHex,
		Img*   hexMask,
		Img*   hexMaskPlus
	);
	unsigned char Init(HexBase* hbase);
	unsigned char Init(HexBase* hBase, HexBase* lowerHBase);
	void Release();
	unsigned char Run();

	inline Img* getHexedImg() { return m_hexedImg; }
protected:
	/*not owned*/
	s_hex* m_hex;
	int    m_nHex;
	s_hex* m_lowerHex;
	int    m_nLowerHex;
	Img* m_hexMask;
	Img* m_hexMaskPlus;
	Img* m_lowerHexMask;
	/*owned*/
	Img* m_hexedImg;

	unsigned char renderHexImg();
	unsigned char genHexImgDebug();
};


#endif