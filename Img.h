#pragma once
#ifndef IMG_H
#define IMG_H
#ifndef FILEIO_TGA_HEADER
#include "FileIO.h"
#endif
#include <math.h>
#ifndef MATH_H
#include "Math.h"
#endif


class Img {
public:
	Img();
	~Img();

	virtual unsigned char init(const Img& other);
	virtual unsigned char init(const unsigned char img[], long width, long height, long colorMode = 4);
	virtual unsigned char init(long width, long height, long colorMode = 4);
	virtual unsigned char init(CTargaImage* inFImg);
	void   clearToChar(unsigned char cval);
	void release();

	s_rgba GetRGBA(long x_i, long y_i);
	s_rgba GetRGBA(long index);
	UINT32 GetCol(long index);
	void   SetRGBA(long x_i, long y_i, const s_rgba& rgba);
	void   SetRGBA(long index, const s_rgba& rgba);
	void   AddRGBA(long index, const s_rgba& rgba);
	void   SetCol(long index, UINT32 col);
	void   AddCol(long index, UINT32 col);

	unsigned char PrintMaskedImg(long x_i, long y_j, const Img& pImg, const s_rgba& rgba);
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

	bool inImg(long x_i, long y_i);

protected:
	unsigned char* m_img;
	long m_colorMode;
	long m_width;
	long m_height;
	long m_pixSize;

	UINT32 rgbaToUint(s_rgba& rgba);
	s_rgba UintToRGBA(UINT32 col);

};
#endif