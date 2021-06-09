#include "stdafx.h"
#include "HokHex.h"
HokHex::HokHex():m_lowHex(NULL), m_nlowHex(0)
{
	;
}
HokHex::~HokHex() {
	;
}
unsigned char HokHex::Init(
	HexBase* pLowHex
) {
	float Rhex = 2.f*pLowHex->getRhex();
	HexBase::Init(pLowHex->getImg(), Rhex, 0.f, 0.f);
	m_lowHex = pLowHex->getHexes();
	m_nlowHex = pLowHex->getNHex();
	m_hex = new s_hex[m_nlowHex];/* this will generate more than necessary*/
	return genMesh();
}
void HokHex::Release() {
	if (m_hex != NULL) {
		delete[]m_hex;
		m_hex = NULL;
	}
	m_lowHex = NULL;
	m_nlowHex = 0;
	HexBase::Release();
}
unsigned char HokHex::Update(Img* img) {
	unsigned char err = HexBase::Update(img);
	if (RetOk(err)) {
		err = Run();
	}
	return err;
}
unsigned char HokHex::Run()
{
	unsigned char err = ECODE_OK;
	for (int i = 0; i < m_nHex; i++) {
		/*find ave of colors in the under hexes*/
		err = aveDownLinkedCols(i);
	}
	return err;
}
unsigned char HokHex::genMesh()/*returnes ABORT if image too small*/
{
	unsigned char err = ECODE_FAIL;
	err = genHexes();
	if (RetOk(err)) {
		err = genWebHiLo();
	}
	return err;
}
unsigned char HokHex::genHexes()
{
	int first_hex_i = -1;
	for (int i_hex = 0; i_hex <= m_nlowHex; i_hex++) {
		bool allConn = true;
		for (int i_web = 0; i_web < 6; i_web++) {
			int web_link = m_lowHex[i_hex].web[i_web];
			if (web_link < 0)
				allConn = false;
		}
		if (allConn) {
			first_hex_i = i_hex;
			break;
		}
	}
	if (first_hex_i < 0) {/*the image is too small for this layer, no hex with a complete set of lower hexes can be found*/
		return ECODE_ABORT;
	}
	while (first_hex_i > 0) {
		genXweb(first_hex_i);
		/*try to find start of line beneath*/
		int new_first_hex_i = -1;
		int next_hex_i = m_lowHex[first_hex_i].web[4];
		if (next_hex_i >= 0) {
			new_first_hex_i = m_lowHex[next_hex_i].web[4];
		}
		if (new_first_hex_i < 0) {
			next_hex_i = m_lowHex[first_hex_i].web[5];
			if (next_hex_i >= 0) {
				new_first_hex_i = m_lowHex[next_hex_i].web[5];
			}
		}
		first_hex_i = new_first_hex_i;
		if (first_hex_i < 0)
			break;
		/*check if all conected around new first hex canidate*/
		bool allConn = true;
		for (int i_web = 0; i_web < 6; i_web++) {
			if (m_lowHex[first_hex_i].web[i_web] < 0)
				allConn = false;
		}
		if (!allConn) {
			/*if not all connected move one double link left*/
			int new_inter_link = m_lowHex[first_hex_i].web[0];
			first_hex_i = -1;
			if (new_inter_link > 0)
				first_hex_i = m_lowHex[new_inter_link].web[0];
		}
	}
	return 0x00;
}
unsigned char HokHex::genWebHiLo()
{

	int top_hex = 0;// findFirstHex(-1);
	int cur_hex = findFirstHex(top_hex + 1);
	if (cur_hex <= 0)
		return ECODE_ABORT;
	unsigned char err_code = ECODE_ABORT;
	do {
		err_code = zipForwardDown(top_hex, cur_hex);
		int new_top_hex = findFirstHex(top_hex);
		if (new_top_hex <= 0)
			break;
		int new_bot_hex = findFirstHex(new_top_hex + 1);
		if (new_bot_hex <= 0)
			break;
		top_hex = new_top_hex;
		cur_hex = new_bot_hex;
	} while (RetOk(err_code));
	top_hex = 0;
	cur_hex = findFirstHex(top_hex + 1);
	err_code = ECODE_ABORT;
	do {
		err_code = zipBackDown(top_hex, cur_hex);
		int new_top_hex = findFirstHex(top_hex);
		if (new_top_hex <= 0)
			break;
		int new_bot_hex = findFirstHex(new_top_hex + 1);
		if (new_bot_hex <= 0)
			break;
		top_hex = new_top_hex;
		cur_hex = new_bot_hex;
	} while (RetOk(err_code));

	return ECODE_OK;
}

unsigned char HokHex::genXweb(int first_hex_i)
{
	int center_i = first_hex_i;
	int next_center_i = -1;
	int startnHex = m_nHex;
	int cntHexes = 0;
	do {
		bool allConn = true;
		for (int i = 0; i < 6; i++) {
			if (m_lowHex[center_i].web[i] < 0)
				allConn = false;
		}
		if (!allConn)
			break;
		s_hex& newHex = m_hex[m_nHex];
		newHex.thisLink = m_nHex;
		m_nHex++;
		newHex.colSet = false;
		for (int i = 0; i < 6; i++) {
			newHex.web[i] = -1;
		}
		newHex.centerLink = center_i;
		newHex.i = m_lowHex[center_i].i;
		newHex.j = m_lowHex[center_i].j;
		newHex.x = m_lowHex[center_i].x;
		newHex.y = m_lowHex[center_i].y;
		/*put all 6 surrounding into master hex*/
		for (int i_down = 0; i_down < 6; i_down++) {
			newHex.downLinks[i_down] = m_lowHex[center_i].web[i_down];
		}
		int close_next_i = m_lowHex[center_i].web[0];
		next_center_i = m_lowHex[close_next_i].web[0];
		center_i = next_center_i;
		cntHexes++;
	} while (next_center_i >= 0);
	/*weave the web back and forth in X*/
	for (int i = 0; i < (cntHexes-1); i++) {
		int cur_i = i + startnHex;
		m_hex[cur_i].web[0] = cur_i + 1;
		m_hex[cur_i + 1].web[3] = cur_i;
	}
	return 0x00;
}
int HokHex::findFirstHex(int linestart_i)
{
	if (m_nHex < 2)
		return -1;
	int last_link_i = -1;
	int cur_hex = linestart_i;
	do {
		last_link_i = m_hex[cur_hex].web[3];
		cur_hex++;
		if (cur_hex >= m_nHex)
			return -1;
	} while (last_link_i >= 0);
	return cur_hex-1;
}

unsigned char HokHex::zipForwardDown(int& top_hex, int& cur_hex) {
	if (!isForwardDownLinked(top_hex, cur_hex))
		cur_hex++;
	while (isForwardDownLinked(top_hex, cur_hex)) {
		m_hex[top_hex].web[5] = cur_hex;
		m_hex[cur_hex].web[2] = top_hex;
		if (m_hex[top_hex].web[0] < 0 || m_hex[cur_hex].web[0] < 0)
			break;
		top_hex++;
		cur_hex++;
	}
	if (cur_hex >= m_nHex || top_hex>=m_nHex)
		return ECODE_ABORT;
	return ECODE_OK;
}
unsigned char HokHex::zipBackDown(int& top_hex, int& cur_hex)
{
	if (!isBackDownLinked(top_hex, cur_hex))
		top_hex++;
	while (isBackDownLinked(top_hex, cur_hex)) {
		m_hex[top_hex].web[4] = cur_hex;
		m_hex[cur_hex].web[1] = top_hex;
		if (m_hex[top_hex].web[0] < 0 || m_hex[cur_hex].web[0] < 0)
			break;
		top_hex++;
		cur_hex++;
	}
	if (cur_hex >= m_nHex || top_hex >= m_nHex)
		return ECODE_ABORT;
	return ECODE_OK;
}
bool HokHex::isForwardDownLinked(int& top_hex, int& cur_hex)
{
	if (cur_hex >= m_nHex)
		return false;
	int lowwebR_1 = m_lowHex[m_hex[top_hex].centerLink].web[5];
	int lowwebR_center = m_lowHex[lowwebR_1].web[5];
	if (lowwebR_center < 0)
		return false;
	return (lowwebR_center == m_hex[cur_hex].centerLink) ? true : false;
}
bool HokHex::isBackDownLinked(int& top_hex, int& cur_hex)
{
	if (cur_hex >= m_nHex)
		return false;
	int lowwebL_1 = m_lowHex[m_hex[top_hex].centerLink].web[4];
	int lowwebL_center = m_lowHex[lowwebL_1].web[4];
	if (lowwebL_center < 0)
		return false;
	return (lowwebL_center == m_hex[cur_hex].centerLink) ? true : false;
}
unsigned char HokHex::aveDownLinkedCols(int web_i, float center_frac)
{
	
	float rgbsum[3];
	for (int i = 0; i < 3; i++)
		rgbsum[i] = 0.f;
	for (int i = 0; i < 6; i++) {
		for(int j=0; j<3; j++)
			rgbsum[j] += m_lowHex[m_hex[web_i].downLinks[i]].rgb[j];
	}
	float totalWeight = 6.f + center_frac;
	for (int i = 0; i < 3; i++) {
		rgbsum[i] += center_frac * m_lowHex[m_hex[web_i].centerLink].rgb[i];
		m_hex[web_i].rgb[i] = rgbsum[i] / totalWeight;
	}
	return ECODE_OK;
}