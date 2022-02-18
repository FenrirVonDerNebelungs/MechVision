#pragma once
#ifndef HEXBASE_H
#define HEXBASE_H
#ifndef IMG_H
#include "Img.h"
#endif

#define HEX_MAXDOWNLINK 36
#define HEX_THRESHOLDFRAC 0.4f
#define HEX_CHARMAXVAL 255.f


class HexBase : public Base {
public:
	HexBase();
	~HexBase();
	virtual unsigned char Init(
		Img* img,
		float Rhex = 5.f,
		float sigmaVsR = 0.75f,
		float IMaskRVsR = 1.5f
	);
	virtual unsigned char Update(Img* img);/*image must have same dimensions as original*/

	virtual unsigned char Run();
	virtual void Release();
	inline Img* getImg() { return m_img; }
	inline long getImgWidth() { return m_img->getWidth(); }
	inline long getImgHeight() { return m_img->getHeight(); }

	inline Img* getHexMask() { return m_hexMask; }
	inline Img* getHexMaskPlus() { return m_hexMaskPlus; }

	inline int getNHex() const{ return m_nHex; }
	inline s_hex getHex(int i) { return m_hex[i]; }
	inline s_hex* getHexes() { return m_hex; }

	inline float getRhex() { return m_Rhex; }
	inline float getShex() { return m_Shex; }
	inline float getRShex() { return m_RShex; }
	inline s_2pt* getHexUs() { return m_hexU; }

	unsigned char genStructuredPlate(s_hexPlate& plate);/*generates a plate from the hexes & webs currently in the system does not fill the rgb*/
	void releaseStructuredPlate(s_hexPlate& plate);
protected:
	/*not owned*/
	Img* m_img;
	/*owned*/
	s_hex* m_hex;
	int    m_nHex;

	/*for img output*/
	float m_Rhex;
	float m_RShex;
	float m_Shex;
	s_2pt m_hexU[6];
	s_2pt_i m_hexMaskBL_offset;
	float m_totpixHex;

	Img* m_hexMask;
	Img* m_hexMaskPlus;
	void genHexU_0();
	void genHexU_1();

	virtual void genHexU() { genHexU_0(); }
	unsigned char genHexMask();
	bool inHex(const s_hex& h, const s_2pt& pt, float padding=0.f);

	virtual unsigned char setRGBFor(int s_hex_i) { return ECODE_ABORT; }
	/*              */
};
#endif
