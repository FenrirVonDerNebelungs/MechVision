#pragma once
#ifndef IMG_H
#define IMG_H
#ifndef BASE_H
#include "Base.h"
#endif
#ifndef PATTERNNODE_H
#include "PatternNode.h"
#endif


class Img {
public:
	Img();
	~Img();

	virtual unsigned char init(const Img& other);
	virtual unsigned char init(const unsigned char img[], long width, long height, long colorMode = 4);
	unsigned char initNoOwn(unsigned char img[], long width, long height, long colorMode = 4);
	virtual unsigned char init(long width, long height, long colorMode = 4);
	void   clearToChar(unsigned char cval);
	void release();

	s_rgba GetRGBA(long x_i, long y_i);
	s_rgb  GetRGB(long x_i, long y_i);
	s_rgba GetRGBA(long index);
	s_rgb  GetRGB(long index);
	uint32_t GetCol(long index);
	uint32_t GetColRGBA(long index);
	uint32_t GetColRGB(long index);
	void   SetRGBA(long x_i, long y_i, const s_rgba& rgba);
	void   SetRGB(long x_i, long y_i, const s_rgb& rgb);
	void   SetRGBA(long index, const s_rgba& rgba);
	void   SetRGB(long index, const s_rgb& rgb);
	void   AddRGBA(long index, const s_rgba& rgba);
	void   AddRGB(long index, const s_rgb& rgb, float alpha=0.5f);
	void   SetCol(long index, uint32_t col);
	void   SetColRGBA(long index, uint32_t col);
	void   SetColRGB(long index, uint32_t col);
	void   AddCol(long index, uint32_t col);

	unsigned char PrintMaskedImg(long x_i, long y_j, const Img& pImg, const s_rgb& rgb);
	unsigned char PrintSquare(long x_i, long y_j, int size, s_rgba& rgba);
	unsigned char DrawLine(s_2pt_i& pt0, s_2pt_i& pt1, s_rgba& rgba);

	inline long getWidth() const { return m_width; }
	inline long getHeight() const { return m_height; }
	inline unsigned char* getImg() { return m_img; }
	inline long getIndex(long i, long j) { long index = j * m_width + i; return (index < m_pixSize) ? index : -1; }
	unsigned char setChar(long x, long y, unsigned char pt);
	inline unsigned char setChar(long index, unsigned char pt) { m_img[index] = pt; return ECODE_OK; }
	inline unsigned char getChar(long index) const { return m_img[index]; }
	inline long getColorMode() const { return m_colorMode; }
	inline long getMaxIndex() const { return m_colorMode * m_pixSize; }

	bool inImg(long x_i, long y_i);

protected:
	bool m_notOwned;
	unsigned char* m_img;
	long m_colorMode;
	long m_width;
	long m_height;
	long m_pixSize;

	uint32_t rgbaToUint(s_rgba& rgba);
	uint32_t rgbToUint(s_rgb& rgb);
	s_rgba UintToRGBA(uint32_t col);
	s_rgb  UintToRGB(uint32_t col);

};
#endif
