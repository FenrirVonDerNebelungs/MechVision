#pragma once
#ifndef DRAWHEXIMG_H
#define DRAWHEXIMG_H
#ifndef HEXBASE_H
#include "HexBase.h"
#endif
#ifndef STAMPEYE_H
#include "StampEye.h"
#endif
#ifndef DRIVEPLANE_H
#include "DrivePlane.h"
#endif

class DrawHexImg : public Base{
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
	unsigned char Init(s_hexPlate* plate, Img* hexMask);
	unsigned char Init(HexBase* hbase, StampEye* stampEyep);
	void Release();
	unsigned char Run();
	unsigned char renderHexImg();
	unsigned char renderLineImg(LineFinder* lineFinder);
	unsigned char renderEyeImg(s_hexEye& eye);
	unsigned char drawDrivePlates(s_DrivePlate plates[], int numPlates);/*should be initialized with the s_hexPlate, img initialializer*/

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
	StampEye* m_stampEye;
	s_hexPlate* m_curStampPlate;
	int m_cur_stampEye_i;
	int m_cur_stamp_i;
	int m_cur_sub_stamp_i;
	/*owned*/
	Img* m_hexedImg;
	s_rgb m_defOCol;


	unsigned char genHexImgDebug();
	unsigned char renderHexOuput(float offset_x=0.f, float offset_y=0.f);/*renders monoscale in col of hex o variable*/
	unsigned char renderAdditiveHexOuput();
	unsigned char renderIncStamp();/*renders a stamp then inc so that next stamp is rendered next call, loops over raw indexted eye stamps*/

	unsigned char genSingLunaLineImg(LineFinder* lineFinder);
	unsigned char genLineImg(LineFinder* lineFinder);
	s_rgb genLineCol(int lunai);
	unsigned char genEyeImgDebug(s_hexEye& eye);
	unsigned char drawWebHexPlate(s_hexPlate& plate, s_2pt& offset, int web_i);
	unsigned char drawHexPlate(s_hexPlate& plate, Img* hexMask, s_2pt& offset);
	unsigned char drawStampLunaHexPlate(s_hexPlate& plate, Img* hexMask, s_2pt& offset);
	unsigned char drawLowerNodes(s_fNode* hiNode, Img* hexMask, s_2pt& offset, s_rgb& col);
	void colRotate(s_rgb& col, unsigned char addc, unsigned char basecol=0x00);

	bool setStampEye(int i);
	s_hexPlate& setStampEyeAsRaw(int i);
	s_hexPlate& setStampEyeAsLuna(int i_stamp, int i_sub_stamp);
};


#endif