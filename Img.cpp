#include "Img.h"

Img::Img() :m_notOwned(false), m_img(NULL), m_colorMode(0), m_width(0), m_height(0), m_pixSize(0)
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
unsigned char Img::initNoOwn(unsigned char img[], long width, long height, long colorMode)
{
	if (width <= 0 || height <= 0 || colorMode <= 0)
		return ECODE_FAIL;
	m_pixSize = width * height;
	m_width = width;
	m_height = height;
	m_colorMode = colorMode;
	long total_size = m_pixSize * m_colorMode;
	m_notOwned = true;
	m_img = img;
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
void Img::release()
{
	m_pixSize = 0;
	m_height = 0;
	m_width = 0;
	m_colorMode = 0;
	if (m_img != NULL && !m_notOwned) {
		delete[] m_img;
	}
	m_img = NULL;
}
s_rgba Img::GetRGBA(long x_i, long y_i)
{
	long index = y_i * m_width + x_i;
	return GetRGBA(index);
}
s_rgb Img::GetRGB(long x_i, long y_i)
{
	long index = y_i * m_width + x_i;
	return GetRGB(index);
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
s_rgb Img::GetRGB(long index) {
	long i = index * m_colorMode;
	s_rgb rgb;
	rgb.r = m_img[i];
	rgb.g = m_img[i + 1];
	rgb.b = m_img[i + 2];
	return rgb;
}
void Img::SetRGBA(long x_i, long y_i, const s_rgba& rgba) {
	long index = y_i * m_width + x_i;
	SetRGBA(index, rgba);
}
void Img::SetRGB(long x_i, long y_i, const s_rgb& rgb) {
	long index = y_i * m_width + x_i;
	SetRGB(index, rgb);
}
void Img::SetRGBA(long index, const s_rgba& rgba) {
	index *= m_colorMode;
	m_img[index] = rgba.r;
	m_img[index + 1] = rgba.g;
	m_img[index + 2] = rgba.b;
	m_img[index + 3] = rgba.a;
}
void Img::SetRGB(long index, const s_rgb& rgb) {
	index *= m_colorMode;
	m_img[index] = rgb.r;
	m_img[index + 1] = rgb.g;
	m_img[index + 2] = rgb.b;
}
void Img::AddRGBA(long index, const s_rgba& rgba) {
	float r_n, g_n, b_n, a_n;
	float r_o, g_o, b_o, a_o;
	r_n = (float)rgba.r;
	g_n = (float)rgba.g;
	b_n = (float)rgba.b;
	a_n = (float)rgba.a;
	float a_frac = a_n / (255.f);
	r_o = (float)m_img[index];
	g_o = (float)m_img[index + 1];
	b_o = (float)m_img[index + 2];
	a_o = (float)m_img[index + 3];
	float a_old_frac = a_o /(255.f);
	float a_sum = a_frac + a_old_frac;

	r_n = floorf((r_o * a_old_frac + r_n * a_frac)/a_sum);
	g_n = floorf((g_o * a_old_frac + g_n * a_frac) / a_sum);
	b_n = floorf((b_o * a_old_frac + b_n * a_frac) / a_sum);
	a_n = a_sum;
	if (r_n > 255.f)
		r_n = 255.f;
	if (g_n > 255.f)
		g_n = 255.f;
	if (b_n > 255.f)
		b_n = 255.f;
	if (a_n > 255.f)
		a_n = 255.f;
	m_img[index] = (unsigned char)r_n;
	m_img[index + 1] = (unsigned char)g_n;
	m_img[index + 2] = (unsigned char)b_n;
	m_img[index + 3] = (unsigned char)a_n;
}
void Img::AddRGB(long index, const s_rgb& rgb, float alpha) {
	float r_n, g_n, b_n;
	float r_o, g_o, b_o;
	r_n = (float)rgb.r;
	g_n = (float)rgb.g;
	b_n = (float)rgb.b;
	float a_frac = alpha;
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
void Img::SetCol(long index, uint32_t col)
{
	if (m_colorMode == 4)
		SetColRGBA(index, col);
	else
		SetColRGB(index, col);
}
void Img::SetColRGBA(long index, uint32_t col)
{
	s_rgba rgba = UintToRGBA(col);
	SetRGBA(index, rgba);
}
void Img::SetColRGB(long index, uint32_t col)
{
	s_rgb rgb = UintToRGB(col);
	SetRGB(index, rgb);
}
void Img::SetColRGB(long x_i, long y_i, uint32_t col) {
	s_rgb rgb = UintToRGB(col);
	SetRGB(x_i, y_i, rgb);
}
void Img::AddCol(long index, uint32_t col)
{
	if (m_colorMode == 4) {
		s_rgba rgba = UintToRGBA(col);
		AddRGBA(index, rgba);
	}
	else {
		s_rgb rgb = UintToRGB(col);
		AddRGB(index, rgb);
	}
}
uint32_t Img::GetCol(long index) {
	uint32_t retv = 0x00;
	if (m_colorMode == 4) {
		s_rgba rgba = GetRGBA(index);
		retv=rgbaToUint(rgba);
	}
	else {
		s_rgb rgb = GetRGB(index);
		retv = rgbToUint(rgb);
	}
	return retv;
}
uint32_t Img::GetColRGBA(long index) {
	s_rgba rgba = GetRGBA(index);
	return rgbaToUint(rgba);
}
uint32_t Img::GetColRGB(long index) {
	s_rgb rgb = GetRGB(index);
	return rgbToUint(rgb);
}
uint32_t Img::GetColRGB(long x_i, long y_i) {
	s_rgb rgb = GetRGB(x_i, y_i);
	return rgbToUint(rgb);
}
unsigned char Img::PrintMaskedImg(long x_i, long y_j, const Img& pImg, const s_rgb& rgb)
{
	/*not fast just for debug*/
	long swidth = pImg.getWidth();
	long sheight = pImg.getHeight();
	long hwidth = swidth / 2;
	long hheight = sheight / 2;
	long itl = x_i - hwidth;
	long jtl = y_j - hheight;
	s_rgb _rgb = rgb;
	for (long j = 0; j < sheight; j++) {
		for (long i = 0; i < swidth; i++) {
			long img_index = j * swidth + i;
			unsigned char maskchar = pImg.getChar(img_index);
			if (maskchar > 0x10) {
				//_rgba.a = maskchar;
				long main_i = itl + i;
				long main_j = jtl + j;
				if (main_i < 0 || main_j < 0)
					continue;
				if (main_i >= m_width || main_j >= m_height)
					continue;
				long index = main_j * m_width + main_i;
				SetRGB(index, _rgb);
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
unsigned char Img::DrawLine(s_2pt_i& pt0, s_2pt_i& pt1, s_rgb& rgba)
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
			SetRGB(index, rgba);
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
uint32_t Img::rgbaToUint(s_rgba& rgba)
{
	uint32_t _r = (uint32_t)rgba.r;
	uint32_t _g = (uint32_t)rgba.g;
	uint32_t _b = (uint32_t)rgba.b;
	uint32_t _a = (uint32_t)rgba.a;
	_r = _r << 24;
	_g = _g << 16;
	_b = _b << 8;
	_a = _a | _b | _g | _r;
	return _a;
}
uint32_t Img::rgbToUint(s_rgb& rgb)
{
	uint32_t _r = (uint32_t)rgb.r;
	uint32_t _g = (uint32_t)rgb.g;
	uint32_t _b = (uint32_t)rgb.b;
	_r = _r << 16;
	_g = _g << 8;
	_b =  _b | _g | _r;
	return _b;
}
s_rgba Img::UintToRGBA(uint32_t col) {
	s_rgba rgba;
	uint32_t r = col & 0xFF000000;
	uint32_t g = col & 0x00FF0000;
	uint32_t b = col & 0x0000FF00;
	uint32_t a = col & 0x000000FF;
	rgba.r = (unsigned char)(r >> 24);
	rgba.g = (unsigned char)(g >> 16);
	rgba.b = (unsigned char)(b >> 8);
	rgba.a = (unsigned char)a;
	return rgba;
}
s_rgb Img::UintToRGB(uint32_t col) {
	s_rgb rgb;
	uint32_t r = col & 0x00FF0000;
	uint32_t g = col & 0x0000FF00;
	uint32_t b = col & 0x000000FF;
	rgb.r = (unsigned char)(r >> 16);
	rgb.g = (unsigned char)(g >> 8);
	rgb.b = (unsigned char)b;
	return rgb;
}
