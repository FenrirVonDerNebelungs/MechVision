//#include "framework.h"
#include "DrawHexImg.h"

DrawHexImg::DrawHexImg() :m_hex(NULL), m_nodes(NULL), m_nHex(0), m_lowerHex(NULL), m_nLowerHex(0), m_hexMask(NULL), m_hexMaskPlus(NULL), m_lowerHexMask(NULL), 
m_stampEye(NULL), m_curStampPlate(NULL), m_curLunaStampPlate(NULL), m_cur_stampEye_i(0), m_cur_stamp_i(0), m_cur_sub_stamp_i(0),
m_hexedImg(NULL)
{
	m_defOCol.r= 0x00;
	m_defOCol.g = 0x00;
	m_defOCol.b = 0x00;
	for (int i = 0; i < PATTERNLUNA_NUM; i++) {
		m_lunaMiniMask[i] = NULL;
	}
}
DrawHexImg::~DrawHexImg()
{
	;
}
unsigned char DrawHexImg::Init(
	long width,
	long height,
	long bpp,
	s_hex* hex,
	int nHex,
	Img* hexMask,
	Img* hexMaskPlus
) 
{
	m_hex = hex;
	m_nHex = nHex;
	m_hexMask = hexMask;
	m_hexMaskPlus = hexMaskPlus;
	m_hexedImg = new Img;
	if (m_hexedImg == NULL)
		return ECODE_MEMERR_FAIL;
	unsigned char err_code = m_hexedImg->init(width, height, bpp);
	return err_code;
}
unsigned char DrawHexImg::Init(HexBase* hBase)
{
	Img* img = hBase->getImg();
	return Init(img->getWidth(), img->getHeight(), img->getColorMode(), hBase->getHexes(), hBase->getNHex(), hBase->getHexMask(), hBase->getHexMaskPlus());
}
unsigned char DrawHexImg::Init(HexBase* hBase, HexBase* lowerHBase) {
	Img* img = hBase->getImg();
	m_lowerHexMask = lowerHBase->getHexMask();
	m_lowerHex = lowerHBase->getHexes();
	m_nLowerHex = lowerHBase->getNHex();
	return Init(img->getWidth(), img->getHeight(), img->getColorMode(), hBase->getHexes(), hBase->getNHex(), hBase->getHexMask(), hBase->getHexMaskPlus());
}
unsigned char DrawHexImg::Init(HexBase* hbase, s_hexPlate* plate) {
	unsigned char err_code = Init(hbase);
	m_nodes = plate->m_fhex;
	m_nHex = plate->m_nHex;
	m_defOCol.r = 0xFF;
	m_defOCol.g = 0xFF;
	m_defOCol.b = 0xFF;
	return ECODE_OK;
}
unsigned char DrawHexImg::Init(s_hexPlate* plate, Img* hexMask) {
	Init(plate->m_width, plate->m_height, 3L, NULL, plate->m_nHex, hexMask, hexMask);
	m_nodes = plate->m_fhex;
	m_defOCol.r = 0xFF;
	m_defOCol.g = 0xFF;
	m_defOCol.b = 0xFF;
	return ECODE_OK;
}
unsigned char DrawHexImg::Init(HexBase* hbase, StampEye* stampEyep) {
	m_stampEye = stampEyep;
	if (Err(Init(hbase)))
		return ECODE_FAIL;
	if (!setStampEye(0))
		return ECODE_ABORT;
	m_hexedImg->clearToChar(0x00);
	initLunaMiniMasks(hbase);
	m_defOCol.r = 0xFF;
	m_defOCol.g = 0xFF;
	m_defOCol.b = 0xFF;
	return ECODE_OK;
}
void DrawHexImg::Release()
{
	m_hex = NULL;
	m_nHex = 0;
	m_lowerHex = NULL;
	m_nLowerHex = 0; 
	m_hexMask = NULL;
	m_lowerHexMask = NULL;
	if (m_hexedImg != NULL) {
		m_hexedImg->release();
		delete m_hexedImg;
		m_hexedImg = NULL;
	}
}
unsigned char DrawHexImg::Run()
{
	return renderIncStamp();//renderHexOuput();//renderHexImg();//genHexImgDebug();
}
unsigned char DrawHexImg::renderHexImg()
{
	for (int i = 0; i < m_nHex; i++) {
		s_hex curHex = m_hex[i];
		s_rgb hexCol = imgMath::convToRGB(curHex.rgb[0], curHex.rgb[1], curHex.rgb[2]);
		m_hexedImg->PrintMaskedImg(curHex.i, curHex.j, *m_hexMaskPlus, hexCol);
	}
	return ECODE_OK;
}
unsigned char DrawHexImg::renderLineImg(LineFinder* lineFinder) {
	return genSingLunaLineImg(lineFinder);//genLineImg(lineFinder);//genSingLunaLineImg(lineFinder);
}
unsigned char DrawHexImg::renderEyeImg(s_hexEye& eye) {
	return genEyeImgDebug(eye);
}
unsigned char DrawHexImg::genHexImgDebug()
{
	unsigned char err_code = ECODE_OK;
	/*gen image of hex mask*/
	s_rgb hexCol = { 0xff, 0xde, 0xee };
	char rc[3] = { (char)0x00, (char)0xcc, (char)0xff };

	/*draw hexes under selected hex*/
	s_hex chosenHex = m_hex[100];
	if (chosenHex.centerLink >= 0) {
		s_hex centerHex = m_lowerHex[chosenHex.centerLink];
		m_hexedImg->PrintMaskedImg(centerHex.i, centerHex.j, *m_lowerHexMask, hexCol);
		for (int i = 0; i < 6; i++) {
			s_hex lowHex = m_lowerHex[chosenHex.downLinks[i]];
			int mi = i % 3;
			hexCol.r = rc[mi];
			m_hexedImg->PrintMaskedImg(lowHex.i, lowHex.j, *m_lowerHexMask, hexCol);
		}
	}
	/*                             */
	return ECODE_OK;
}
unsigned char DrawHexImg::renderHexOuput(float offset_x, float offset_y) {
	for (int i = 0; i < m_nHex; i++) {
		float r = m_nodes[i].o * (float)m_defOCol.r;
		float g = m_nodes[i].o * (float)m_defOCol.g;
		float b = m_nodes[i].o * (float)m_defOCol.b;
		s_rgb hexCol = imgMath::convToRGB(r, g, b);
		/*debug*/
		if (m_nodes[i].o < 0.0)
			hexCol = imgMath::convToRGB(0, 0, 0);
		long img_i = (long)roundf(m_nodes[i].x+offset_x);
		long img_j = (long)roundf(m_nodes[i].y+offset_y);
		m_hexedImg->PrintMaskedImg(img_i, img_j, *m_hexMaskPlus, hexCol);
	}
	return ECODE_OK;
}
unsigned char DrawHexImg::renderAdditiveHexOuput() {
	float thresh = 0.3f;
	for (int i = 0; i < m_nHex; i++) {
		if (m_nodes[i].o >= thresh) {
			float r = m_nodes[i].o * (float)m_defOCol.r;
			float g = m_nodes[i].o * (float)m_defOCol.g;
			float b = m_nodes[i].o * (float)m_defOCol.b;
			s_rgb hexCol = imgMath::convToRGB(r, g, b);
			/*debug*/
			if (m_nodes[i].o < 0.0)
				hexCol = imgMath::convToRGB(0, 0, 0);
			long img_i = (long)roundf(m_nodes[i].x);
			long img_j = (long)roundf(m_nodes[i].y);
			m_hexedImg->PrintMaskedImg(img_i, img_j, *m_hexMaskPlus, hexCol);
		}
	}
	return ECODE_OK;
}
unsigned char DrawHexImg::renderIncStamp() {
	m_hexedImg->clearToChar(0x11);
	s_2pt offset = { 200.f, 200.f };
	/*stamp eye raw*/
	if (Err(renderHexOuput(offset.x0 + 200.f, offset.x1)))
		return ECODE_FAIL;
	if (Err(renderHexOuput(offset.x0, offset.x1)))
		return ECODE_FAIL;
	/*stamp eye luna*/
	if(Err(drawStampLunaHexPlate(*m_curLunaStampPlate, m_hexMaskPlus, offset)))
		return ECODE_FAIL;
	if (!setStampEye(m_cur_stampEye_i + 1))
		return ECODE_ABORT;
	std::cout << "setup to render stamp:" << m_cur_stampEye_i << "\n";
	return ECODE_OK;
}
unsigned char DrawHexImg::drawDrivePlates(s_DrivePlate plates[], int numPlates) {
	m_defOCol = genLineCol(0);
	m_nodes = plates[0].p.m_fhex;
	m_nHex = plates[0].p.m_nHex;
	renderHexOuput();
	for (int i = 1; i < numPlates; i++) {
		m_defOCol = genLineCol(i);
		m_nodes = plates[i].p.m_fhex;
		m_nHex = plates[i].p.m_nHex;
		renderAdditiveHexOuput();
	}
	return ECODE_OK;
}
unsigned char DrawHexImg::genSingLunaLineImg(LineFinder* lineFinder) {
	m_hexedImg->clearToChar(0x00);
	int nlines = lineFinder->getNSingLunaLines();
	if (nlines <= 0)
		return ECODE_ABORT;
	s_line* lines = lineFinder->getSingLunaLines();
	for (int i_line = 0; i_line < nlines; i_line++) {
		for (int i = 0; i < lines[i_line].n; i++) {
			s_linePoint& pt = lines[i_line].pts[i];
			s_rgb hexCol = genLineCol(pt.lunai);
			int hex_i = pt.hexi;
			m_hexedImg->PrintMaskedImg(m_hex[hex_i].i, m_hex[hex_i].j, *m_hexMaskPlus, hexCol);
		}
	}
	return ECODE_OK;
}
unsigned char DrawHexImg::genLineImg(LineFinder* lineFinder) {
	m_hexedImg->clearToChar(0x00);
	int nlines = lineFinder->getNLines();
	if (nlines <= 0)
		return ECODE_ABORT;
	s_line** lines = lineFinder->getLines();
	for (int i_line = 0; i_line < nlines; i_line++) {
		for (int i = 0; i < lines[i_line]->n; i++) {
			s_linePoint& pt = lines[i_line]->pts[i];
			s_rgb hexCol = genLineCol(pt.lunai);
			int hex_i = pt.hexi;
			m_hexedImg->PrintMaskedImg(m_hex[hex_i].i, m_hex[hex_i].j, *m_hexMaskPlus, hexCol);
		}
	}
	return ECODE_OK;
}
s_rgb DrawHexImg::genLineCol(int lunai) {
	s_rgb rgb = { 0x22, 0x22, 0x22 };
	float mul1 = 0x33;
	float mul2 = 0x44;
	if (lunai < 3) {
		float r = mul1 * (float)lunai;
		float g = mul2 * (float)lunai;
		rgb = imgMath::convToRGB(r, g, 0xff);
	}
	else if(lunai<6) {
		float li = (float)lunai - 3;
		float g = mul2 * li;
		float b = mul1 * li;
		rgb = imgMath::convToRGB(0xff, g, b);
	}
	else if (lunai == 6) {
		rgb.r = 0xdd;
		rgb.g = 0xdd;
		rgb.b = 0xdd;
	}
	return rgb;
}
s_rgb DrawHexImg::genLunaCol(int lunai, float o) {
	s_rgb rgb = { 0xff, 0x66, 0x00 };
	float o_norm = (o - 0.5f) * 2.f;
	o_norm = (o >= 1.f) ? 1.f: o_norm;
	if (o_norm < 0.f)
		o_norm = 0.f;
	float o_norm_rev = 1.f - o_norm;
	if (o_norm_rev > 1.f)
		o_norm_rev = 1.f;
	if (o_norm_rev < 0.f)
		o_norm_rev = 0.f;
	float maxv = (float)0xff;
	float red = maxv * o_norm;
	float blue = maxv * o_norm_rev;
	rgb.b = (unsigned char)roundf(red);
	rgb.r = (unsigned char)roundf(blue);
	rgb.g = (unsigned char)roundf(100.f*o_norm);
	return rgb;
}
unsigned char DrawHexImg::genEyeImgDebug(s_hexEye& eye) {
	s_hexPlate& botPlate = eye.lev[eye.n - 1];
	s_hexPlate& topPlate = eye.lev[0];
	s_hexPlate& midPlate = eye.lev[1];
	s_2pt offset = { 200, 200 };
	//drawHexPlate(botPlate, m_hexMask, offset);
	s_rgb col = { 0x00, 0x22, 0x00 };
	s_fNode* topNode = &(topPlate.m_fhex[0]);
	drawLowerNodes(topNode, m_hexMask, offset, col);
	col.r = 0x22;
	col.g = 0x00;
	col.b = 0xaa;
	s_fNode* midNode = &(midPlate.m_fhex[6]);
	drawLowerNodes(midNode, m_hexMask, offset, col);
	//drawWebHexPlate(botPlate, offset, 5);
	return ECODE_OK;
}
unsigned char DrawHexImg::drawHexPlate(s_hexPlate& plate, Img* hexMask, s_2pt& offset) {
	s_rgb col = { 0x00, 0x00, 0x22 };
	//float width = (float)m_hexedImg->getWidth();
	unsigned char addc = 0x22;

	for (long i = 0; i < plate.m_nHex; i++) {
		long Ii = (long)floorf(plate.m_fhex[i].x + offset.x0);
		long Ij = (long)floorf(plate.m_fhex[i].y + offset.x1);
		colRotate(col, addc);
		m_hexedImg->PrintMaskedImg(Ii, Ij, *hexMask, col);
	}
	return ECODE_OK;
}
unsigned char DrawHexImg::drawStampLunaHexPlate(s_hexPlate& plate, Img* hexMask, s_2pt& offset) {
	for (long i = 0; i < plate.m_nHex; i++) {
		float cutoff_o = 0.501f;
		int lunais[PATTERNLUNA_NUM];
		float lunaos[PATTERNLUNA_NUM];
		int lunaN = 0;
		for (int luna_i = 0; luna_i < PATTERNLUNA_NUM; luna_i++) {
			lunais[luna_i] = -1;
			lunaos[luna_i] = 0.f;
		}
		for (int luna_i = 0; luna_i < PATTERNLUNA_NUM; luna_i++){//PATTERNLUNA_NUM; luna_i++) {
			float luna_o = plate.m_fhex[i].nodes[luna_i]->o;
			if (luna_o > cutoff_o) {
				int end_i = arrMath::getFirstBelow(luna_o, lunaos, lunaN);
				arrMath::put(luna_i, end_i, lunais, PATTERNLUNA_NUM);
				arrMath::put(luna_o, end_i, lunaos, PATTERNLUNA_NUM);
				lunaN++;
			}
		}
		long Ii = (long)floorf(plate.m_fhex[i].x + offset.x0);
		long Ij = (long)floorf(plate.m_fhex[i].y + offset.x1);
		/*draw the results from the weakest to the strongest*/
		for (int cnt = 0; cnt < lunaN; cnt++) {
			s_rgb col = genLunaCol(lunais[cnt], lunaos[cnt]);
			if (lunais[cnt] < 6)
				m_hexedImg->PrintMaskedImg(Ii, Ij, *(m_lunaMiniMask[lunais[cnt]]), col);
			//else
				//m_hexedImg->PrintMaskedImg(Ii, Ij, *m_hexMask, col);
		}
	}
	return ECODE_OK;
}
unsigned char DrawHexImg::drawWebHexPlate(s_hexPlate& plate, s_2pt& offset, int web_i) {
	s_rgb col = { 0xdd, 0xdd, 0xdd };
	unsigned char addc = 0x22;
	for (long i = 0; i < plate.m_nHex; i++) {
		s_2pt_i hcent;
		hcent.x0 = (long)floorf(plate.m_fhex[i].x + offset.x0);
		hcent.x1 = (long)floorf(plate.m_fhex[i].y + offset.x1);
		s_2pt_i nextCent;
		s_fNode* nextNode = (s_fNode*)plate.m_fhex[i].web[web_i];
		if (nextNode != NULL) {
			nextCent.x0 = (long)floorf(nextNode->x + offset.x0);
			nextCent.x1 = (long)floorf(nextNode->y + offset.x1);
			colRotate(col, addc, 0xdd);
			m_hexedImg->DrawLine(hcent, nextCent, col);
		}
	}
	return ECODE_OK;
}

unsigned char DrawHexImg::drawLowerNodes(s_fNode* hiNode, Img* hexMask, s_2pt& offset, s_rgb& col) {
	unsigned char addc = 0x22;
	for (int i = 0; i < 7; i++) {
		s_fNode* nd = (s_fNode*)hiNode->nodes[i];
		if (nd != NULL) {
			s_2pt_i hcent;
			hcent.x0 = (long)floorf(nd->x + offset.x0);
			hcent.x1 = (long)floorf(nd->y + offset.x1);
			colRotate(col, addc);
			m_hexedImg->PrintMaskedImg(hcent.x0, hcent.x1, *hexMask, col);
		}
	}
	return ECODE_OK;
}
void DrawHexImg::colRotate(s_rgb& col, unsigned char addc, unsigned char basecol) {
	if (col.b < 0xff) {
		col.b += addc;
		if (col.b > 0xff)
			col.b = 0xff;
	}
	else if (col.g < 0xff) {
		col.g += addc;
		if (col.g > 0xff)
			col.g = 0xff;
	}
	else if (col.r < 0xff) {
		col.r += addc;
		if (col.r > 0xff)
			col.r = 0xff;
	}
	else {
		col.r = basecol;
		col.g = basecol;
		col.b = addc;
	}
}

bool DrawHexImg::setStampEye(int i) {
	int numStamps = m_stampEye->numEyeStamps();
	if (i >= numStamps)
		return false;

	m_cur_stampEye_i = i;
	int stamp_i, sub_stamp_i;
	m_cur_stamp_i = -1;
	m_cur_sub_stamp_i = -1;
	if (m_stampEye->getStampIs_fromRawIndex(i, stamp_i, sub_stamp_i)) {
		m_cur_stamp_i = stamp_i;
		m_cur_sub_stamp_i = sub_stamp_i;
		s_eyeStamp* stmps = m_stampEye->getEyeStamps();
		std::cout << "rad: " << stmps[stamp_i].radius[sub_stamp_i] << "  ang: " << stmps[stamp_i].ang[sub_stamp_i]
			<< "  center_ang: " << stmps[stamp_i].center_ang[sub_stamp_i] << "  opening_ang: " << stmps[stamp_i].opening_ang[sub_stamp_i] 
			<< "\n";
	}
	else
		return false;
	/*stamp eye raw*/
	s_hexPlate& lowPlate = setStampEyeAsRaw(i);
	m_nHex = lowPlate.m_nHex;
	m_nodes = lowPlate.m_fhex;
	/*stamp eye luna*/
	s_hexPlate& lowLunaPlate = setStampEyeAsLuna(m_cur_stamp_i, m_cur_sub_stamp_i);
	return true;
}
s_hexPlate& DrawHexImg::setStampEyeAsRaw(int i) {
	s_hexEye& hEye = m_stampEye->getEyeRawIndex(i);
	int eye_n = hEye.n;
	m_curStampPlate = &(hEye.lev[eye_n - 1]);
	return *m_curStampPlate;
}
s_hexPlate& DrawHexImg::setStampEyeAsLuna(int i_stamp, int i_sub_stamp) {
	s_hexEye* lunaEye = m_stampEye->getLunaEye(i_stamp, i_sub_stamp);
	int eye_n = lunaEye->n;
	m_curLunaStampPlate = &(lunaEye->lev[eye_n - 1]);
	return *m_curLunaStampPlate;
}
unsigned char DrawHexImg::initLunaMiniMasks(HexBase* hbase) {
	Img* oldMask = hbase->getHexMask();
	for (int i = 0; i < PATTERNLUNA_NUM; i++) {
		m_lunaMiniMask[i] = new Img;
		m_lunaMiniMask[i]->init(oldMask->getWidth(), oldMask->getHeight(), oldMask->getColorMode());
		m_lunaMiniMask[i]->clearToChar(0x00);
	}
	s_2pt* hexUs = hbase->getHexUs();
	s_2pt maskCenter = { ((float)oldMask->getWidth())/2.f, ((float)oldMask->getHeight())/2.f };
	for (int i = 0; i < 6; i++) {
		s_2pt perpAtU = getPerpAtU(hexUs, i);
		for (long i_mask = 0; i_mask < m_lunaMiniMask[i]->getWidth(); i_mask++) {
			for (long j_mask = 0; j_mask < m_lunaMiniMask[i]->getHeight(); j_mask++) {
				uint32_t mask_col = oldMask->GetColRGB(i_mask, j_mask);
				uint32_t luna_mask = getInHalf(maskCenter, i_mask, j_mask, perpAtU);
				uint32_t new_col = mask_col & luna_mask;
				m_lunaMiniMask[i]->SetColRGB(i_mask, j_mask, new_col);
			}
		}
	}
	return ECODE_OK;
}
void DrawHexImg::releaseLunaMiniMasks() {
	for (int i = 0; i < PATTERNLUNA_NUM; i++) {
		if (m_lunaMiniMask[i] != NULL) {
			m_lunaMiniMask[i]->release();
			delete m_lunaMiniMask[i];
			m_lunaMiniMask[i] = NULL;
		}
	}
}
s_2pt DrawHexImg::getPerpAtU(s_2pt* hexUs, int i_u) {
	return vecMath::perpUR(hexUs[i_u]);
}
uint32_t DrawHexImg::getInHalf(const s_2pt& center, long mask_x, long mask_y, const s_2pt& perp) {
	/*been drawing the image with the y up in the internal coord, which is actually down on screen*/
	s_2pt xy = { (float)mask_x, (float)mask_y };
	s_2pt vTop = vecMath::v12(center, xy);
	float dotp = vecMath::dot(perp, vTop);
	/*if on same side as the perp vector points mask is white*/
	return (dotp >= 0.f) ? 0xffffff : 0x000000;
}