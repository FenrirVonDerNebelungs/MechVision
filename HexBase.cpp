#include "stdafx.h"
#include "HexBase.h"
HexBase::HexBase():m_img(NULL), m_hex(NULL), m_nHex(0), m_Rhex(0.f), m_RShex(0.f), m_Shex(0.f), m_totpixHex(0.f), 
	m_hexMask(NULL), m_hexMaskPlus(NULL)
{
	for (int i = 0; i < 6; i++) {
		m_hexU[i].x0 = 0.f;
		m_hexU[i].x1 = 0.f;
	}
	m_hexMaskBL_offset.x0 = 0;
	m_hexMaskBL_offset.x1 = 0;
}
HexBase::~HexBase() {
	;
}
unsigned char HexBase::Init(
	Img* img,
	float Rhex,
	float sigmaVsR,
	float IMaskRVsR
)
{
	m_img = img;

	m_Rhex = Rhex;
	m_RShex = Rhex * sqrt(3.f) / 2.f;
	m_Shex = m_Rhex; //equallat tri

	m_hexMask = new Img;
	m_hexMaskPlus = new Img;

	genHexU();
	unsigned char err_code = genHexMask();
	if (Err(err_code)) {
		Release();
		return err_code;
	}
	return ECODE_OK;
}
void HexBase::Release()
{
	if (m_hexMask != NULL) {
		m_hexMask->release();
		delete m_hexMask;
	}
	m_hexMask = NULL;
}
unsigned char HexBase::Update(Img* img) {
	if (img->getWidth() != m_img->getWidth() || img->getHeight() != m_img->getHeight())
		return ECODE_ABORT;
	m_img = img;
	return ECODE_OK;
}
unsigned char HexBase::Run()
{
	return ECODE_ABORT;
}
void HexBase::genHexU_0()
{
	float longs = sqrtf(3.f) / 2.f;
	float shorts = 0.5f;
	/*start with to the right*/
	m_hexU[0].x0 = 1.f;
	m_hexU[0].x1 = 0.f;

	m_hexU[1].x0 = shorts;
	m_hexU[1].x1 = longs;

	m_hexU[2].x0 = -shorts;
	m_hexU[2].x1 = longs;

	m_hexU[3].x0 = -1.f;
	m_hexU[3].x1 = 0.f;

	m_hexU[4].x0 = -shorts;
	m_hexU[4].x1 = -longs;

	m_hexU[5].x0 = shorts;
	m_hexU[5].x1 = -longs;
}
void HexBase::genHexU_1()
{
	float longs = sqrtf(3.f) / 2.f;
	float shorts = 0.5f;
	float s2 = sqrt(2.f);
	/*start with to the right up*/
	m_hexU[0].x0 = longs;
	m_hexU[0].x1 = shorts;

	m_hexU[1].x0 = 0.f;
	m_hexU[1].x1 = 1.f;

	m_hexU[2].x0 = -longs;
	m_hexU[2].x1 = shorts;

	m_hexU[3].x0 = -longs;
	m_hexU[3].x1 = -shorts;

	m_hexU[4].x0 = 0.f;
	m_hexU[4].x1 = -1.f;

	m_hexU[5].x0 = longs;
	m_hexU[5].x1 = -shorts;
}
unsigned char HexBase::genHexMask()
{

	if (m_hexMask == NULL || m_hexMaskPlus==NULL)
		return ECODE_MEMERR_FAIL;
	long sizeM = (long)ceilf(2.f*m_Rhex) + 1;
	long BL_offset = (long)floorf(m_Rhex);
	if (BL_offset < 0) {
		delete m_hexMask;
		delete m_hexMaskPlus;
		m_hexMask = NULL;
		m_hexMaskPlus = NULL;
		return ECODE_FAIL;
	}
	unsigned char err_code = m_hexMask->init(sizeM, sizeM, 1L);
	err_code = err_code | m_hexMaskPlus->init(sizeM, sizeM, 1L);
	if (Err(err_code)) {
		delete m_hexMask;
		delete m_hexMaskPlus;
		m_hexMask = NULL;
		m_hexMaskPlus = NULL;
		return ECODE_FAIL;
	}
	m_hexMask->clearToChar(0x00);
	m_hexMaskPlus->clearToChar(0x00);
	m_hexMaskBL_offset.x0 = BL_offset;
	m_hexMaskBL_offset.x1 = BL_offset;

	s_hex mhex;
	mhex.i = BL_offset;
	mhex.j = BL_offset;
	mhex.x = (float)mhex.i;
	mhex.y = (float)mhex.j;
	m_totpixHex = 0.f;
	for (int j = 0; j < sizeM; j++) {
		for (int i = 0; i < sizeM; i++) {
			s_2pt pt = { (float)i, (float)j };
			bool inMask = inHex(mhex, pt);
			long index = j * sizeM + i;
			if (inMask) {
				m_hexMask->setChar(index, 0xFF);
				m_totpixHex += 1.f;
			}
			bool inMaskPlus = inHex(mhex, pt, 1.f);
			if (inMaskPlus)
				m_hexMaskPlus->setChar(index, 0xFF);
		}
	}
	return ECODE_OK;
}
bool HexBase::inHex(const s_hex& h, const s_2pt& pt, float padding)
{
	float xdiff = pt.x0 - h.x;
	float ydiff = pt.x1 - h.y;
	float diff = sqrtf(xdiff*xdiff + ydiff * ydiff);
	if (diff > m_Rhex)
		return false;
	s_2pt vpt = { xdiff, ydiff };
	float max_proj = 0.f;
	for (int i = 0; i < 6; i++) {
		float proj = vecMath::dot(vpt, m_hexU[i]);
		if (proj > max_proj)
			max_proj = proj;
	}
	bool inside = false;
	if (max_proj <= (m_RShex + padding))
		inside = true;
	return inside;
}
