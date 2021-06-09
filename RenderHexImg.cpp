#include "stdafx.h"
#include "RenderHexImg.h"
RenderHexImg::RenderHexImg() :m_hex(NULL), m_nHex(0), m_hexMask(NULL), m_imgRender(NULL)
{
	;
}
RenderHexImg::~RenderHexImg()
{
	;
}
unsigned char RenderHexImg::Init(HexBase* hexObj) {
	m_hex = hexObj->getHexes();
	m_nHex = hexObj->getNHex();
	m_hexMask = hexObj->getHexMask();
	m_imgRender = new Img;
	Img* mainImg = hexObj->getImg();
	unsigned char err_code = m_imgRender->init(mainImg->getWidth(), mainImg->getHeight());
	return err_code;
}
void RenderHexImg::Release()
{
	if (m_imgRender != NULL) {
		m_imgRender->release();
		delete m_imgRender;
	}
	m_imgRender = NULL;
	m_hex = NULL;
	m_nHex = 0;
	m_hexMask = NULL;
}
unsigned char RenderHexImg::Run()
{
	return Render();
}
unsigned char RenderHexImg::Render()
{
	s_rgba hexCol = { 0xee, 0xee, 0xff, 0xff };
	for (int i = 0; i < m_nHex; i++) {
		s_hex curhex = m_hex[i];
		hexCol.r = (unsigned char)roundf(curhex.rgb[0]);
		hexCol.g = (unsigned char)roundf(curhex.rgb[1]);
		hexCol.b = (unsigned char)roundf(curhex.rgb[2]);
		m_imgRender->PrintMaskedImg(curhex.i, curhex.j, *m_hexMask, hexCol);
	}
	return ECODE_OK;
}