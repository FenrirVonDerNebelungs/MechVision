//#include "framework.h"
#include "ConvolHex.h"
ConvolHex::ConvolHex():m_img(NULL), m_hex(NULL), m_IMask(NULL), m_IMaskF(NULL), m_Rhex(0.f), m_sigmaVsR(0.f), m_IMaskRVsR(0.f), m_sigma(0.f), m_gaussNorm(0.f), m_gaussExpConst(0.f), m_IMaskR(0.f)
{
	m_IMaskCenter.x0=0.f;
	m_IMaskCenter.x1 = 0.f;
	m_IMaskBL_offset.x0 = 0L;
	m_IMaskBL_offset.x1 = 0L;
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
	long maskTotalSize = (long)(sizeM * sizeM);
	m_IMaskF = new float[maskTotalSize];
	for (int i = 0; i < maskTotalSize; i++)
		m_IMaskF[i] = 0.f;

	m_IMaskCenter = { roundf(m_IMaskR), roundf(m_IMaskR) };
	s_2pt centerPt = { 0.f, 0.f };
	float gaussMax = calcGaussian(centerPt);
	for (int j = 0; j < sizeM; j++) {
		for (int i = 0; i < sizeM; i++) {
			s_2pt pt = { (float)i, float(j) };
			int index = j * m_IMask->getWidth() + i;
			s_2pt ptRel = vecMath::v12(m_IMaskCenter, pt);
			float gaussVal = calcGaussian(ptRel);
			m_IMaskF[index] = gaussVal/gaussMax;
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
				//s_rgba brgba = m_img->GetRGBA(i_big, j_big);
				s_rgb brgba = m_img->GetRGB(i_big, j_big);
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

namespace threadedConvol {
#ifndef MECVISPI_WIN
  inline bool convInImg(long width, long height, long x_i, long y_i) {
    if (x_i < 0 || y_i < 0)
      return false;
    if (x_i >= width || y_i >= height)
      return false;
    return true;
  }
  void* runConvThread(void* IOVarsVoid) {
    s_convKernVars* IOVars = (s_convKernVars*)IOVarsVoid;
    long indexShift = (long)IOVars->hex_index;
    /*divide number of hexes by number of threads*/
    long num_passes = IOVars->num_Hex / THREADEDCONVOL_NUMTHREADS;
    for (long i = 0; i < num_passes; i++) {
      long cur_index = i * THREADEDCONVOL_NUMTHREADS + indexShift;
      IOVars->hex_index = cur_index;
      convCellKernel(*IOVars);
    }
    pthread_exit(NULL);
	return NULL;
  }
  void convCellKernel(s_convKernVars IOVars) {
    long hex_i = IOVars.outHex[IOVars.hex_index].i;
    long hex_j = IOVars.outHex[IOVars.hex_index].j;
    if (hex_i < 0 || hex_j < 0)
      return;
    long i_start = hex_i - IOVars.MaskBL_offsetX;
    long j_start = hex_j - IOVars.MaskBL_offsetY;
    long i_big = i_start;
    long j_big = j_start;
    float r_ = 0.f;
    float g_ = 0.f;
    float b_ = 0.f;
    float cnt = 0.f;
    for (long j = 0; j < IOVars.Mask_height; j++) {
      j_big = (j_start + j);
      i_big = i_start;
      long img_index = j_big * IOVars.Img_width + i_big;
      img_index *= IOVars.Img_bpp;
      for (long i = 0; i < IOVars.Mask_width; i++) {
		  if (img_index<IOVars.Img_maxIndex) {
			  long i_small = j * IOVars.Mask_width + i;
			  float maskVal = IOVars.mask_pix[i_small];
	  
			  float R = (float)IOVars.img_pix[img_index];
			  float G = (float)IOVars.img_pix[img_index + 1];
			  float B = (float)IOVars.img_pix[img_index + 2];
	  
			  R *= maskVal;
			  G *= maskVal;
			  B *= maskVal;
			  r_ += R;
			  g_ += G;
			  b_ += B;
			  cnt += maskVal;
		  }
		  img_index+=IOVars.Img_bpp;
	  }
    }
    if (cnt >= 0.00001f) {
      r_ /= cnt;
      g_ /= cnt;
      b_ /= cnt;
    }
    
    IOVars.outHex[IOVars.hex_index].rgb[0] = r_;
    IOVars.outHex[IOVars.hex_index].rgb[1] = g_;
    IOVars.outHex[IOVars.hex_index].rgb[2] = b_;

    IOVars.outHex[IOVars.hex_index].colSet = true;
    
    return;
  }
#endif
}
