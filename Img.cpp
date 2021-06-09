#include "stdafx.h"
#include "img.h"

Img::Img() :m_img(NULL), m_colorMode(0), m_width(0), m_height(0), m_pixSize(0)
{
	;
}
Img::~Img()
{
	;
}
unsigned char Img::init(const Img& other) {
	return init(other.m_img, other.m_width, other.m_height, other.m_colorMode);
}
unsigned char Img::init(const unsigned char img[], long width, long height, long colorMode)
{
	unsigned char err_code = init(width, height, colorMode);
	if (err_code != ECODE_OK)
		return err_code;
	long total_size = m_pixSize * m_colorMode;
	for (long i = 0; i < total_size; i++) {
		m_img[i] = img[i];
	}
	return ECODE_OK;
}
unsigned char Img::init(long width, long height, long colorMode)
{
	if (width <= 0 || height <= 0 || colorMode <= 0)
		return ECODE_FAIL;
	m_pixSize = width * height;
	m_width = width;
	m_height = height;
	m_colorMode = colorMode;
	long total_size = m_pixSize * m_colorMode;
	m_img = new unsigned char[total_size];
	if (m_img == NULL)
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char Img::init(CTargaImage* inFImg)
{
	if (inFImg == NULL)
		return ECODE_ABORT;
	if (!inFImg->IsStateOpen())
		return ECODE_ABORT;
	return init(inFImg->GetImage(), inFImg->GetWidth(), inFImg->GetHeight(), (long)inFImg->GetColorMode());
}
void Img::release()
{
	m_pixSize = 0;
	m_height = 0;
	m_width = 0;
	m_colorMode = 0;
	if (m_img != NULL) {
		delete[] m_img;
	}
	m_img = NULL;
}
s_rgba Img::GetRGBA(long x_i, long y_i)
{
	long index = y_i * m_width + x_i;
	return GetRGBA(index);
}
s_rgba Img::GetRGBA(long index) {
	long i = index * m_colorMode;
	s_rgba rgba;
	rgba.r = m_img[i];
	rgba.g = m_img[i + 1];
	rgba.b = m_img[i + 2];
	rgba.a = m_img[i + 3];
	return rgba;
}
void Img::SetRGBA(long x_i, long y_i, const s_rgba& rgba) {
	long index = y_i * m_width + x_i;
	SetRGBA(index, rgba);
}
void Img::SetRGBA(long index, const s_rgba& rgba) {
	index *= m_colorMode;
	m_img[index] = rgba.r;
	m_img[index + 1] = rgba.g;
	m_img[index + 2] = rgba.b;
	m_img[index + 3] = rgba.a;
}
void Img::AddRGBA(long index, const s_rgba& rgba) {
	float r_n, g_n, b_n, a_n;
	float r_o, g_o, b_o;
	r_n = (float)rgba.r;
	g_n = (float)rgba.g;
	b_n = (float)rgba.b;
	a_n = (float)rgba.a;
	float a_frac = a_n / (255.f);
	float a_old_frac = 1.f - a_frac;
	r_o = (float)m_img[index];
	g_o = (float)m_img[index + 1];
	b_o = (float)m_img[index + 2];
	r_n = floorf(r_o * a_old_frac + r_n * a_frac);
	g_n = floorf(g_o * a_old_frac + g_n * a_frac);
	b_n = floorf(b_o * a_old_frac + b_n * a_frac);
	if (r_n > 255.f)
		r_n = 255.f;
	if (g_n > 255.f)
		g_n = 255.f;
	if (b_n > 255.f)
		b_n = 255.f;
	m_img[index] = (unsigned char)r_n;
	m_img[index + 1] = (unsigned char)g_n;
	m_img[index + 2] = (unsigned char)b_n;
}
void Img::SetCol(long index, UINT32 col)
{
	s_rgba rgba = UintToRGBA(col);
	SetRGBA(index, rgba);
}
void Img::AddCol(long index, UINT32 col)
{
	s_rgba rgba = UintToRGBA(col);
	AddRGBA(index, rgba);
}
UINT32 Img::GetCol(long index) {
	s_rgba rgba = GetRGBA(index);
	return rgbaToUint(rgba);
}
unsigned char Img::PrintMaskedImg(long x_i, long y_j, const Img& pImg, const s_rgba& rgba)
{
	/*not fast just for debug*/
	long swidth = pImg.getWidth();
	long sheight = pImg.getHeight();
	long hwidth = swidth / 2;
	long hheight = sheight / 2;
	long itl = x_i - hwidth;
	long jtl = y_j - hheight;
	s_rgba _rgba = rgba;
	for (long j = 0; j < sheight; j++) {
		for (long i = 0; i < swidth; i++) {
			long img_index = j * swidth + i;
			unsigned char maskchar = pImg.getChar(img_index);
			if (maskchar > 0x00) {
				_rgba.a = maskchar;
				long main_i = itl + i;
				long main_j = jtl + j;
				if (main_i < 0 || main_j < 0)
					continue;
				if (main_i >= m_width || main_j >= m_height)
					continue;
				long index = main_j * m_width + main_i;
				SetRGBA(index, _rgba);
			}
		}
	}
	return ECODE_OK;
}
unsigned char Img::PrintSquare(long x_i, long y_j, int size, s_rgba& rgba)
{
	long itl = x_i - size;
	long jtl = y_j - size;
	for (long j = 0; j < size; j++) {
		for (long i = 0; i < size; i++) {
			long box_index = j * size + i;
			long main_i = itl + i;
			long main_j = itl + j;
			if (main_i < 0 || main_j < 0)
				continue;
			if (main_i >= m_width || main_j >= m_height)
				continue;
			long index = main_j * m_width + main_i;
			SetRGBA(index, rgba);
		}
	}
	return ECODE_OK;
}
unsigned char Img::DrawLine(s_2pt_i& pt0, s_2pt_i& pt1, s_rgba& rgba)
{
	s_2pt X0 = { (float)pt0.x0, (float)pt0.x1 };
	s_2pt X1 = { (float)pt1.x0, (float)pt1.x1 };
	s_2pt v01 = vecMath::v12(X0, X1);
	float vmag = vecMath::len(v01);
	if (vmag <= 0.f)
		return ECODE_OK;
	s_2pt u01 = { v01.x0 / vmag, v01.x1 / vmag };
	float vlen = 0.f;
	float vinc = 0.5;
	do {
		s_2pt vloc = { u01.x0*vlen, u01.x1*vlen };
		vloc.x0 += X0.x0;
		vloc.x1 += X0.x1;
		long x_i = (long)roundf(vloc.x0);
		long y_i = (long)roundf(vloc.x1);
		if (inImg(x_i, y_i)) {
			long index = y_i * m_width + x_i;
			SetRGBA(index, rgba);
		}
		vlen += vinc;
	} while (vlen <= vmag);
	return ECODE_OK;
}
unsigned char Img::setChar(long x, long y, unsigned char pt)
{
	long index = y * m_width + x;
	return setChar(index, pt);
}
void Img::clearToChar(unsigned char cval) {
	long total_size = m_colorMode * m_pixSize;
	for (long i = 0; i < total_size; i++) {
		m_img[i] = cval;
	}
}
bool Img::inImg(long x_i, long y_i)
{
	if (x_i < 0 || y_i < 0)
		return false;
	if (x_i >= m_width || y_i >= m_height)
		return false;
	return true;
}
UINT32 Img::rgbaToUint(s_rgba& rgba)
{
	UINT32 _r = (UINT32)rgba.r;
	UINT32 _g = (UINT32)rgba.g;
	UINT32 _b = (UINT32)rgba.b;
	UINT32 _a = (UINT32)rgba.a;
	_r = _r << 24;
	_g = _g << 16;
	_b = _b << 8;
	_a = _a | _b | _g | _r;
	return _a;
}
s_rgba Img::UintToRGBA(UINT32 col) {
	s_rgba rgba;
	UINT32 r = col & 0xFF000000;
	UINT32 g = col & 0x00FF0000;
	UINT32 b = col & 0x0000FF00;
	UINT32 a = col & 0x000000FF;
	rgba.r = (unsigned char)(r >> 24);
	rgba.g = (unsigned char)(g >> 16);
	rgba.b = (unsigned char)(b >> 8);
	rgba.a = (unsigned char)a;
	return rgba;
}