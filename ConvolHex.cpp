#include "stdafx.h"
#include "ConvolHex.h"
ConvolHex::ConvolHex():m_img(NULL), m_hex(NULL), m_IMask(NULL)
{
	;
}
ConvolHex::~ConvolHex()
{
	;
}
unsigned char ConvolHex::Init(Img* img, s_hex hex[], float Rhex, float sigmaVsR, float IMaskRVsR)
{
	m_img = img;
	m_hex = hex;
	m_Rhex = Rhex;
	m_sigmaVsR = sigmaVsR;
	m_IMaskRVsR = IMaskRVsR;

	genIMask();
	return ECODE_OK;
}
void ConvolHex::Release()
{
	if (m_IMask != NULL) {
		m_IMask->release();
		delete m_IMask;
	}
	m_IMask = NULL;
}
unsigned char ConvolHex::convulToHex(int col_i)
{
	return convulMaskToHex(col_i);
}
unsigned char ConvolHex::genIMask()
{
	/*assume m_Rhex and m_sigmaVsR m_IMaskRVsR have already been set */
	m_sigma = m_sigmaVsR * m_Rhex;
	if (m_sigma < 1.f)
		return ECODE_ABORT;
	m_gaussNorm = 1.f / (m_sigma * sqrt(2.f *PI));
	m_gaussExpConst = 2 * m_sigma*m_sigma;
	m_IMaskR = m_IMaskRVsR * m_Rhex;
	if (m_IMaskR < 1.f)
		return ECODE_ABORT;
	int sizeM = (int)ceilf(2 * m_IMaskR) + 1;
	m_IMask = new Img;
	if (m_IMask == NULL)
		return ECODE_MEMERR_FAIL;
	m_IMask->init((long)sizeM, (long)sizeM, 1);
	m_IMask->clearToChar(0x00);

	m_IMaskCenter = { roundf(m_IMaskR), roundf(m_IMaskR) };
	s_2pt centerPt = { 0.f, 0.f };
	float gaussMax = calcGaussian(centerPt);
	for (int j = 0; j < sizeM; j++) {
		for (int i = 0; i < sizeM; i++) {
			s_2pt pt = { (float)i, float(j) };
			int index = j * m_IMask->getWidth() + i;
			s_2pt ptRel = vecMath::v12(m_IMaskCenter, pt);
			float gaussVal = calcGaussian(ptRel);
			gaussVal = 255.f * gaussVal / gaussMax;
			gaussVal = roundf(gaussVal);
			if (gaussVal > 255.f)
				gaussVal = 255.f;
			if (gaussVal < 0.f)
				gaussVal = 0.f;
			unsigned char maskVal = (unsigned char)gaussVal;
			m_IMask->setChar(index, maskVal);
		}
	}
	m_IMaskBL_offset = { (long)m_IMaskCenter.x0, (long)m_IMaskCenter.x1 };
	return ECODE_OK;
}

float ConvolHex::calcGaussian(s_2pt& pt)
{
	float dist2 = pt.x0*pt.x0 + pt.x1*pt.x1;
	float expval = dist2 / m_gaussExpConst;
	float val = expf(-expval);
	return val * m_gaussNorm;
}
unsigned char ConvolHex::convulMaskToHex(int col_i)
{
	/*not the fastest convul*/
	s_hex hx = m_hex[col_i];
	if (hx.i < 0 || hx.j < 0)
		return ECODE_ABORT;
	long i_start = hx.i - m_IMaskBL_offset.x0;
	long j_start = hx.j - m_IMaskBL_offset.x1;
	long i_big = i_start;
	long j_big = j_start;
	float r_ = 0.f;
	float g_ = 0.f;
	float b_ = 0.f;
	float cnt = 0.f;
	for (long j = 0; j < m_IMask->getHeight(); j++) {
		j_big = (j_start + j);
		i_big = i_start;
		for (long i = 0; i < m_IMask->getWidth(); i++) {
			if (m_img->inImg(i_big, j_big)) {
				s_rgba brgba = m_img->GetRGBA(i_big, j_big);
				long i_small = j * m_IMask->getWidth() + i;
				unsigned char maskc = m_IMask->getChar(i_small);
				float maskVal = ((float)maskc) / 255.f;
				float R = (float)brgba.r;
				float G = (float)brgba.g;
				float B = (float)brgba.b;
				R *= maskVal;
				G *= maskVal;
				B *= maskVal;
				r_ += R;
				g_ += G;
				b_ += B;
				cnt += maskVal;
			}
			i_big++;
		}
	}
	if (cnt >= 0.00001f) {
		r_ /= cnt;
		g_ /= cnt;
		b_ /= cnt;
	}
	if (r_ >= 255.f)
		r_ = 255.f;
	if (g_ >= 255.f)
		g_ = 255.f;
	if (b_ >= 255.f)
		b_ = 255.f;
	m_hex[col_i].rgb[0] = r_;
	m_hex[col_i].rgb[1] = g_;
	m_hex[col_i].rgb[2] = b_;
	m_hex[col_i].colSet = true;

	return ECODE_OK;
}
bool ConvolHex::isIMaskInside(long hi, long hj)
{

	if (hi < 0 || hj < 0)
		return false;
	long hhi = hi + m_IMask->getWidth();
	long hhj = hj + m_IMask->getHeight();
	if (hhi >= m_img->getWidth() || hhj >= m_img->getHeight())
		return false;
	return true;
}
s_rgba ConvolHex::convToRGBA(float r, float g, float b) {
	if (r > 255.f)
		r = 255.f;
	if (r < 0.f)
		r = 0.f;
	if (g > 255.f)
		g = 255.f;
	if (g < 0.f)
		g = 0.f;
	if (b > 255.f)
		b = 255.f;
	if (b < 0.f)
		b = 0.f;
	s_rgba rgba = { (unsigned char)r, (unsigned char)g, (unsigned char)b, 0xFF };
	return rgba;
}