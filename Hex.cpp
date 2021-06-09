#include "stdafx.h"
#include "Hex.h"

Hex::Hex():m_Convol(NULL), m_wHex(0), m_hHex(0)
{
	m_BR.x0=0.f;
	m_BR.x1 = 0.f;
}
Hex::~Hex()
{
	;
}
unsigned char Hex::Init(Img* img, float Rhex, float sigmaVsR, float IMaskRvsR)
{
	HexBase::Init(img, Rhex, sigmaVsR, IMaskRvsR);
	m_Convol = new ConvolHex;

	m_wHex = 0;
	m_hHex = 0;

	genMesh();

	m_Convol->Init(img, m_hex, Rhex, sigmaVsR, IMaskRvsR);

	return ECODE_OK;
}
unsigned char Hex::Update(Img* img) {
	unsigned char err=HexBase::Update(img);
	if (RetOk(err)) {
		err = Run();
	}
	return err;
}
unsigned char Hex::Run()
{
	for (int i = 0; i < m_nHex; i++) {
		m_Convol->convulToHex(i);
	}
	return ECODE_OK;
}

void Hex::Release() {
	if (m_hex != NULL)
		delete[] m_hex;
	if (m_Convol != NULL) {
		m_Convol->Release();
		delete m_Convol;
	}
	m_Convol = NULL;
	HexBase::Release();
}
unsigned char Hex::genMesh()
{
	genMeshLoc();
	genMeshWeb();
	return ECODE_OK;
}
unsigned char Hex::genMeshLoc()
{
	if (m_hex != NULL)
		return ECODE_FAIL;
	float width = (float)m_img->getWidth();
	float height = (float)m_img->getHeight();
	float segLen_y = m_Rhex + (m_Shex / 2.f);
	float segLen_x = m_RShex * 2.f;
	float nW = floorf(width / segLen_x);
	float nH = floorf(height / segLen_y);
	nH--;
	if (nW < 1.f || nH < 1.f)
		return ECODE_ABORT;
	float w_margin = width - nW * segLen_x;
	w_margin /= 2.f;
	float h_margin = height - nH * segLen_y;
	h_margin /= 2.f;

	m_BR.x0 = w_margin + m_RShex;
	m_BR.x1 = h_margin + m_Rhex;
	return genMeshLocFromBR(nW, nH);
}
unsigned char Hex::genMeshWeb()
{
	if (m_wHex < 2)
		return ECODE_FAIL;
	/*rotate counterclockwise starting from full right pos x*/
	int index = 0;
	int rHigh_index = 0;
	for (int j = 1; j < m_hHex; j += 2) {
		for (int i = 0; i < (m_wHex - 1); i++) {
			index = j * m_wHex + i;
			if (m_hex[index].i >= 0) {
				m_hex[index].web[0] = (i < (m_wHex - 2)) ? index + 1 : -1;
				if (j >= 1) {
					m_hex[index].web[1] = index - m_wHex + 1;
					m_hex[index].web[2] = index - m_wHex;
				}
				m_hex[index].web[3] = (i > 0) ? index - 1 : -1;
				if (j <= (m_hHex - 2)) {
					m_hex[index].web[4] = index + m_wHex;
					m_hex[index].web[5] = m_hex[index].web[4] + 1;
				}
			}
		}
	}
	for (int j = 0; j < m_hHex; j += 2) {
		index = j * m_wHex;
		m_hex[index].web[0] = index + 1;
		if (j >= 1)
			m_hex[index].web[1] = index - m_wHex;
		if (j <= (m_hHex - 2))
			m_hex[index].web[5] = index + m_wHex;
		for (int i = 1; i < (m_wHex - 1); i++) {
			index = j * m_wHex + i;
			m_hex[index].web[0] = index + 1;
			if (j >= 1) {
				m_hex[index].web[1] = index - m_wHex;
				m_hex[index].web[2] = m_hex[index].web[1] - 1;
			}
			m_hex[index].web[3] = index - 1;
			if (j <= (m_hHex - 2)) {
				m_hex[index].web[5] = index + m_wHex;
				m_hex[index].web[4] = m_hex[index].web[5] - 1;
			}
		}
		index = j * m_wHex + (m_wHex - 1);
		if (j >= 1)
			m_hex[index].web[2] = index - m_wHex - 1;
		m_hex[index].web[3] = index - 1;
		if (j <= (m_hHex - 2))
			m_hex[index].web[4] = index + m_wHex - 1;
	}
	return ECODE_OK;
}
unsigned char Hex::genMeshLocFromBR(float nW, float nH)
{
	float segLen_y = m_Rhex + (m_Shex / 2.f);
	float segLen_x = m_RShex * 2.f;
	float w_margin = m_BR.x0;
	float h_margin = m_BR.x1;
	int n_w = (int)nW;
	int n_h = (int)ceilf(nH / 2.f);
	long hex_size = (long)ceilf(2.f * nW * nH);
	m_hex = new s_hex[hex_size];
	for (int i = 0; i < hex_size; i++) {
		m_hex[i].x = -1.f;
		m_hex[i].y = -1.f;
		m_hex[i].i = -1;
		m_hex[i].j = -1;
		m_hex[i].thisLink = -1;
		for (int ii = 0; ii < 6; ii++) {
			m_hex[i].web[ii] = -1;
			m_hex[i].downLinks[ii] = -1;
		}
		m_hex[i].centerLink = -1;
	}
	float x = w_margin;
	float y;
	for (int j = 0; j < n_h; j++) {
		y = h_margin + 2.f * segLen_y * (float)j;
		x = w_margin;
		for (int i = 0; i < n_w; i++) {
			int index = 2 * j * n_w + i;
			m_hex[index].x = x;
			m_hex[index].y = y;
			m_hex[index].i = (long)roundf(x);
			m_hex[index].j = (long)roundf(y);
			m_hex[index].thisLink = index;
			x += segLen_x;
		}
	}
	m_nHex = n_w * n_h;
	m_wHex = n_w;
	m_hHex = n_h;
	n_h = (int)floorf(nH / 2.f);
	int n_w_full = n_w;
	n_w -= 1;
	for (int j = 0; j < n_h; j++) {
		y = h_margin + segLen_y + 2.f * segLen_y * (float)j;
		x = w_margin + m_RShex;
		for (int i = 0; i < n_w; i++) {
			int index = (2 * j + 1) * n_w_full + i;
			m_hex[index].x = x;
			m_hex[index].y = y;
			m_hex[index].i = (long)roundf(x);
			m_hex[index].j = (long)roundf(y);
			m_hex[index].thisLink = index;
			x += segLen_x;
		}
	}
	m_nHex += n_w_full * n_h;
	m_hHex += n_h;
	return ECODE_OK;
}
