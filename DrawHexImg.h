#pragma once
#ifndef DRAWHEXIMG_H
#define DRAWHEXIMG_H
#ifndef HEXBASE_H
#include "HexBase.h"
#endif
#ifndef LINEFINDER_H
#include "LineFinder.h"
#endif

class DrawHexImg {
public:
	DrawHexImg();
	~DrawHexImg();
	unsigned char Init(
		long width,
		long height,
		long bpp,
		s_hex* hex,
		int    nHex,
		Img*   hexMask,
		Img*   hexMaskPlus
	);
	unsigned char Init(HexBase* hbase);
	unsigned char Init(HexBase* hBase, HexBase* lowerHBase);
	unsigned char Init(HexBase* hbase, s_hexPlate* plate);
	void Release();
	unsigned char Run();
	unsigned char renderHexImg();
	unsigned char renderLineImg(LineFinder* lineFinder);

	inline void setHexes(s_hex* hex) { m_hex = hex; }
	inline void setNodes(s_fNode* nodes) { m_nodes = nodes; }

	inline Img* getHexedImg() { return m_hexedImg; }
protected:
	/*not owned*/
	s_hex* m_hex;
	s_fNode* m_nodes;
	int    m_nHex;
	s_hex* m_lowerHex;
	int    m_nLowerHex;
	Img* m_hexMask;
	Img* m_hexMaskPlus;
	Img* m_lowerHexMask;
	/*owned*/
	Img* m_hexedImg;
	s_rgb m_defOCol;


	unsigned char genHexImgDebug();
	unsigned char renderHexOuput();/*renders monoscale in col of hex o variable*/
	unsigned char genLineImg(LineFinder* lineFinder);
	s_rgb genLineCol(int lunai);
};


#endif