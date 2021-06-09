#include "stdafx.h"
#include "DrawHexImg.h"

DrawHexImg::DrawHexImg() :m_hex(NULL), m_nHex(0), m_lowerHex(NULL), m_nLowerHex(0), m_hexMask(NULL), m_hexMaskPlus(NULL), m_lowerHexMask(NULL), m_hexedImg(NULL)
{
	;
}
DrawHexImg::~DrawHexImg()
{
	;
}
unsigned char DrawHexImg::Init(
	long width,
	long height,
	s_hex* hex,
	int nHex,
	Img* hexMask,
	Img* hexMaskPlus
) 
{
	m_hex = hex;
	m_nHex = nHex;
	m_hexMask = hexMask;
	m_hexMaskPlus = hexMaskPlus;
	m_hexedImg = new Img;
	if (m_hexedImg == NULL)
		return ECODE_MEMERR_FAIL;
	unsigned char err_code = m_hexedImg->init(width, height);
	return err_code;
}
unsigned char DrawHexImg::Init(HexBase* hBase)
{
	Img* img = hBase->getImg();
	return Init(img->getWidth(), img->getHeight(), hBase->getHexes(), hBase->getNHex(), hBase->getHexMask(), hBase->getHexMaskPlus());
}
unsigned char DrawHexImg::Init(HexBase* hBase, HexBase* lowerHBase) {
	Img* img = hBase->getImg();
	m_lowerHexMask = lowerHBase->getHexMask();
	m_lowerHex = lowerHBase->getHexes();
	m_nLowerHex = lowerHBase->getNHex();
	return Init(img->getWidth(), img->getHeight(), hBase->getHexes(), hBase->getNHex(), hBase->getHexMask(), hBase->getHexMaskPlus());
}
void DrawHexImg::Release()
{
	m_hex = NULL;
	m_nHex = 0;
	m_lowerHex = NULL;
	m_nLowerHex = 0; 
	m_hexMask = NULL;
	m_lowerHexMask = NULL;
	if (m_hexedImg != NULL) {
		m_hexedImg->release();
		delete m_hexedImg;
		m_hexedImg = NULL;
	}
}
unsigned char DrawHexImg::Run()
{
	return renderHexImg();//genHexImgDebug();
}
unsigned char DrawHexImg::renderHexImg()
{
	for (int i = 0; i < m_nHex; i++) {
		s_hex curHex = m_hex[i];
		s_rgba hexCol = imgMath::convToRGBA(curHex.rgb[0], curHex.rgb[1], curHex.rgb[2]);
		m_hexedImg->PrintMaskedImg(curHex.i, curHex.j, *m_hexMaskPlus, hexCol);
	}
	return ECODE_OK;
}
unsigned char DrawHexImg::genHexImgDebug()
{
	unsigned char err_code = ECODE_OK;
	/*gen image of hex mask*/
	s_rgba hexCol = { 0xff, 0xde, 0xee, 0xff };
	char rc[3] = { (char)0x00, (char)0xcc, (char)0xff };

	/*draw hexes under selected hex*/
	s_hex chosenHex = m_hex[100];
	if (chosenHex.centerLink >= 0) {
		s_hex centerHex = m_lowerHex[chosenHex.centerLink];
		m_hexedImg->PrintMaskedImg(centerHex.i, centerHex.j, *m_lowerHexMask, hexCol);
		for (int i = 0; i < 6; i++) {
			s_hex lowHex = m_lowerHex[chosenHex.downLinks[i]];
			int mi = i % 3;
			hexCol.r = rc[mi];
			m_hexedImg->PrintMaskedImg(lowHex.i, lowHex.j, *m_lowerHexMask, hexCol);
		}
	}
	/*                             */
	return ECODE_OK;

	for (int i = 0; i < m_nHex; i++) {
		s_hex curHex = m_hex[i];
		int mi = i % 3;
		hexCol.r = rc[mi];
		/*draw hexes and lines from web*/
		if (curHex.i > 0)
			m_hexedImg->PrintMaskedImg(curHex.i, curHex.j, *m_hexMask, hexCol);

		s_rgba lineCol = { 0x55, 0xcc, 0x55, 0xff };
		if (curHex.web[2] >= 0) {
			s_hex linkedHex = m_hex[curHex.web[2]];
			s_2pt_i pt0 = { curHex.i, curHex.j };
			s_2pt_i pt1 = { linkedHex.i, linkedHex.j };
			m_hexedImg->DrawLine(pt0, pt1, lineCol);
		}
		/********************************/
	}
	return ECODE_OK;
}