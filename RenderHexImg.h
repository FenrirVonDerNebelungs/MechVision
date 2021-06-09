#pragma once
#ifndef RENDERHEXIMG_H
#define RENDERHEXIMG_H
#ifndef HEXBASE_H
#include "HexBase.h"
#endif
class RenderHexImg {
public:
	RenderHexImg();
	~RenderHexImg();
	unsigned char Init(HexBase* hexObj);
	void Release();
	unsigned char Run();
protected:
	/*not owned*/
	s_hex* m_hex;
	int    m_nHex;
	Img* m_hexMask;
	/*owned*/
	Img* m_imgRender;

	unsigned char Render();
};
#endif