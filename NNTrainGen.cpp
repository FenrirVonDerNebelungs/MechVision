#include "stdafx.h"
#include "NNTrainGen.h"

NNTrainGen::NNTrainGen() : m_baseImg(NULL), m_trainImgs(NULL), m_ntrainImgs(0), 
m_targetWidth(0.f), m_targetLen(0.f), m_dpix(0.f), m_nShifts(0), m_nAngVals(0),
m_blockSize(0)
{
	utilStruct::zeroRGBA(m_col);
	utilStruct::zeroRGBA(m_bakcol);
	utilStruct::zero2pt(m_center);

}
NNTrainGen::~NNTrainGen() {
	;
}
unsigned char NNTrainGen::init(
	Img* baseImg,
	float center_x,
	float center_y,
	float targetWidth,
	float targetLen,
	float dpix,
	int nShifts,
	int nAngVals) 
{
	m_col.r = 0xff;
	m_col.g = 0xff;
	m_col.b = 0xff;
	m_col.a = 0xff;

	m_bakcol.r = 0x00;
	m_bakcol.g = 0x00;
	m_bakcol.b = 0x00;
	m_bakcol.a = 0xff;

	m_center.x0 = center_x;
	m_center.x1 = center_y;
	m_targetWidth = targetWidth;
	m_targetLen = targetLen;
	m_dpix = dpix;
	m_nShifts = nShifts;
	m_nAngVals = nAngVals;

	/*m_ntrainImgs = (2 * m_nShifts + 1) * m_nAngVals * m_blockSize;
	m_trainImgs = new s_trainImg[m_ntrainImgs];*/

	return ECODE_OK;
}
void NNTrainGen::release() {

	if (m_trainImgs != NULL) {
		for (int i = 0; i < m_ntrainImgs; i++) {
			if (m_trainImgs[i].img != NULL) {
				m_trainImgs[i].img->release();
				delete m_trainImgs[i].img;
			}
		}
		delete[]m_trainImgs;
		m_trainImgs = NULL;
	}
	m_ntrainImgs = 0;
}
unsigned char NNTrainGen::run() {
	return draw();
}
unsigned char NNTrainGen::draw() {

	for (int i = 0; i < m_ntrainImgs; i++) {
		m_trainImgs[i].rCurve = 0.f;
		m_trainImgs[i].img = new Img;
		m_trainImgs[i].img->init(m_baseImg->getWidth(), m_baseImg->getHeight());
		m_trainImgs[i].img->clearToChar(0x00);
		m_trainImgs[i].x_center = (long)roundf(m_center.x0);
		m_trainImgs[i].y_center = (long)roundf(m_center.x1);
	}
	float dAng = 2.f*PI / ((float)m_nAngVals);
	int totalShifts = 2 * m_nShifts + 1;
	for (int i_ang = 0; i_ang < m_nAngVals; i_ang++) {
		float ang = ((float)i_ang) * dAng;
		s_2pt UangSlope = { sinf(ang), cosf(ang) };
		s_2pt UangPerp = vecMath::perpUR(UangSlope);
		int curArray_i = 0;
		for (int i_offset = 1; i_offset < (m_nShifts+1); i_offset++) {
			curArray_i = i_ang * totalShifts  + (i_offset - 1);
			curArray_i *= m_blockSize;
			float offsetMag = ((float)i_offset) * -m_dpix;
			s_2pt offset = vecMath::mul(offsetMag, UangPerp);
			drawPos(offset, ang, m_targetWidth, m_targetLen, curArray_i);
			m_trainImgs[curArray_i].ang = ang;
			m_trainImgs[curArray_i].offset = offsetMag;
		}
		for (int i_offset = 0; i_offset < (m_nShifts+1); i_offset++) {
			curArray_i = i_ang * totalShifts + m_nShifts + i_offset;
			curArray_i *= m_blockSize;
			float offsetMag = ((float)i_offset) * m_dpix;
			s_2pt offset = vecMath::mul(offsetMag, UangPerp);
			drawPos(offset, ang, m_targetWidth, m_targetLen, curArray_i);
			m_trainImgs[curArray_i].ang = ang;
			m_trainImgs[curArray_i].offset = offsetMag;
		}
	}
	return ECODE_OK;
}
unsigned char NNTrainGen::draw2block() {
	m_ntrainImgs = 2;
	m_trainImgs = new s_trainImg[m_ntrainImgs];
	for (int i = 0; i < m_ntrainImgs; i++) {
		m_trainImgs[i].rCurve = 0.f;
		m_trainImgs[i].img = new Img;
		m_trainImgs[i].img->init(m_baseImg->getWidth(), m_baseImg->getHeight());
		m_trainImgs[i].img->clearToChar(0x00);
		m_trainImgs[i].x_center = (long)roundf(m_center.x0);
		m_trainImgs[i].y_center = (long)roundf(m_center.x1);
		m_trainImgs[i].offset = 0.f;
		m_trainImgs[i].ang = 0.f;
	}
	/*find bounds*/
	s_2pt boxBL = { -m_targetWidth, -m_targetWidth };
	s_2pt boxUR = { m_targetWidth, m_targetWidth };
	s_2pt_i BL = imgMath::convToVint(boxBL);
	s_2pt_i UR = imgMath::convToVint(boxUR);
	forceInImg(BL);
	forceInImg(UR);

	/*scan over entire box*/
	for (int i_y = BL.x1; i_y <= UR.x1; i_y++) {
		for (int i_x = BL.x0; i_x <= UR.x0; i_x++) {
				m_trainImgs[0].img->SetRGBA(i_x, i_y, m_col);
				m_trainImgs[1].img->SetRGBA(i_x, i_y, m_bakcol);
		}
	}
	return ECODE_OK;
}
unsigned char NNTrainGen::findBoxBoundsFromArray(s_2pt bxs[], int n, s_2pt& boxBL, s_2pt& boxUR) {
	boxBL = bxs[0];
	boxUR = bxs[0];
	for (int i = 0; i < n; i++) {
		if (bxs[i].x0 > boxUR.x0)
			boxUR.x0 = bxs[i].x0;
		if (bxs[i].x1 > boxUR.x1)
			boxUR.x1 = bxs[i].x1;
		if (bxs[i].x0 < boxBL.x0)
			boxBL.x0 = bxs[i].x0;
		if (bxs[i].x1 < boxBL.x1)
			boxBL.x1 = bxs[i].x1;
	}
	return ECODE_OK;
}

void NNTrainGen::forceInImg(s_2pt_i& pt) {
	if (pt.x0 < 0)
		pt.x0 = 0;
	if (pt.x1 < 0)
		pt.x1 = 0;
	if (pt.x0 >= m_baseImg->getWidth())
		pt.x0 = m_baseImg->getWidth() - 1;
	if (pt.x1 >= m_baseImg->getHeight())
		pt.x1 = m_baseImg->getHeight() - 1;
}
NNTrainGenLine::NNTrainGenLine() {
	;
}
NNTrainGenLine::~NNTrainGenLine() {
	;
}
unsigned char NNTrainGenLine::init(
	Img* baseImg,
	float center_x,
	float center_y,
	float targetWidth,
	float targetLen,
	float dpix,
	int nShifts,
	int nAngVals)
{
	NNTrainGen::init(baseImg, center_x, center_y, targetWidth, targetLen, dpix, nShifts, nAngVals);
	m_blockSize = 1;
	m_ntrainImgs = (2 * m_nShifts + 1) * m_nAngVals * m_blockSize;
	m_trainImgs = new s_trainImg[m_ntrainImgs];

	return ECODE_OK;
}
unsigned char NNTrainGenLine::run() {
	return draw();
}
unsigned char NNTrainGenLine::drawPos(s_2pt& offset, float ang, float width, float len, int curArray_i) {
	return drawStraightLine(offset, ang, width, len, m_trainImgs[curArray_i]);
}
unsigned char NNTrainGenLine::drawStraightLine(s_2pt& offset, float ang, float width, float len, s_trainImg& simg) {
	s_2pt center = vecMath::add(m_center, offset);
	s_2pt slope = { cosf(ang), sinf(ang) };
	s_2pt box1 = vecMath::mul(len, slope);
	s_2pt box2 = vecMath::mul(-len, slope);
	s_2pt boxBL;
	s_2pt boxUR;
	findBoxBounds(box1, box2, boxBL, boxUR);
	s_2pt_i BL = imgMath::convToVint(boxBL);
	s_2pt_i UR = imgMath::convToVint(boxUR);
	forceInImg(BL);
	forceInImg(UR);
	for (int i_y = BL.x1; i_y <= UR.x1; i_y++) {
		for (int i_x = BL.x0; i_x <= UR.x0; i_x++) {
			if (isInLine(center, slope, width, i_x, i_y))
				simg.img->SetRGBA(i_x, i_y, m_col);
			else
				simg.img->SetRGBA(i_x, i_y, m_bakcol);
		}
	}
	return ECODE_OK;
}
unsigned char NNTrainGenLine::findBoxBounds(
	s_2pt& box1, s_2pt& box2,
	s_2pt& boxBL, s_2pt& boxUR)
{
	s_2pt bxs[2];
	bxs[0] = box1;
	bxs[1] = box2;
	return findBoxBoundsFromArray(bxs, 2, boxBL, boxUR);
}
bool NNTrainGenLine::isInLine(s_2pt& center, s_2pt& slope, float width, long i_x, long i_y) {
	s_2pt_i vi = { i_x, i_y };
	s_2pt pt = imgMath::convToVfloat(vi);
	s_2pt perp = vecMath::perpUR(slope);
	float distFromLine = vecMath::dot(pt, perp);
	return (distFromLine <= width);
}
NNTrainGenArc::NNTrainGenArc() : m_minRCurve(0.f), m_minArc(0.f), m_drCurve(0.f), m_dArc(0.f),
m_nCurveVals(0), m_nArcVals(0)
{
	;
}
NNTrainGenArc::~NNTrainGenArc() {
	;
}
unsigned char NNTrainGenArc::init(
	Img* baseImg,
	float center_x,
	float center_y,
	float targetWidth,
	float targetLen,
	float dpix,
	int nShifts,
	int nAngVals,
	float minRCurve ,
	float minArc,
	float drCurve,
	float dArc,
	int nCurveVals,
	int nArcVales)
{
	NNTrainGen::init(baseImg, center_x, center_y, targetWidth, targetLen, dpix, nShifts, nAngVals);
	m_minRCurve = minRCurve;
	m_minArc = minArc;
	m_drCurve = drCurve;
	m_dArc = dArc;
	m_nCurveVals = nCurveVals;
	m_nArcVals = nArcVales;

	m_blockSize = nCurveVals * nArcVales;
	m_ntrainImgs = (2 * m_nShifts + 1) * m_nAngVals * m_blockSize;
	m_trainImgs = new s_trainImg[m_ntrainImgs];

	return ECODE_OK;
}
unsigned char NNTrainGenArc::run() {
	return draw();
}
unsigned char NNTrainGenArc::drawPos(s_2pt& offset, float ang, float width, float len, int curArray_i) {
	for (int i_arc = 0; i_arc < m_nArcVals; i_arc++) {
		float Arc = m_minArc + ((float)i_arc) * m_dArc;
		for (int i_curve = 0; i_curve < m_nCurveVals; i_curve++) {
			float rCurve = m_minRCurve + ((float)i_curve) * m_drCurve;
			int cur_i = curArray_i + i_arc * m_nCurveVals + i_curve;
			drawLineArc(offset, ang, width, len, rCurve, Arc, m_trainImgs[cur_i]);
		}
	}
	return ECODE_OK;
}
unsigned char NNTrainGenArc::drawLineArc(s_2pt& offset, float ang, float width, float len, float rCurve, float cornerAng, s_trainImg& simg) {
	if (width > rCurve)
		width = rCurve;
	s_2pt lineCenter = vecMath::add(m_center, offset);
	s_2pt rPerp = { cosf(ang), sinf(ang) };
	s_2pt rU = vecMath::perpUR(rPerp);
	s_2pt curveOppositeU = vecMath::mul(-1.f, rU);
	s_2pt curvCentOffset = vecMath::mul(rCurve, curveOppositeU);

	s_2pt rCenter = vecMath::add(lineCenter, curvCentOffset);

	/*find trapozoide bounds*/
	float RBAng = PI / 2.f - cornerAng;
	s_2pt tPtRBasis_r = { cosf(RBAng), sinf(RBAng) };
	tPtRBasis_r = vecMath::mul(rCurve, tPtRBasis_r);
	s_2pt tPtRBasis_l = { -tPtRBasis_r.x0, tPtRBasis_r.x1 };
	/*convert basis*/
	s_2pt tPt_r = vecMath::convBasis(rPerp, rU, tPtRBasis_r);
	s_2pt tPt_l = vecMath::convBasis(rPerp, rU, tPtRBasis_l);
	/*find the line perps facing in*/
	s_2pt tPerpRBasis_r = { -cosf(RBAng), -sinf(RBAng) };
	s_2pt tPerpRBasis_l = { cosf(RBAng), -sinf(RBAng) };
	s_2pt tPerp_r = vecMath::convBasis(rPerp, rU, tPerpRBasis_r);
	s_2pt tPerp_l = vecMath::convBasis(rPerp, rU, tPerpRBasis_l);
	/*tPerp pointing in is curveOpposite U*/

	/*find bounds*/
	s_2pt boxBL = { -m_targetWidth, -m_targetWidth };
	s_2pt boxUR = { m_targetWidth, m_targetWidth };
	s_2pt_i BL = imgMath::convToVint(boxBL);
	s_2pt_i UR = imgMath::convToVint(boxUR);
	forceInImg(BL);
	forceInImg(UR);

	/*scan over entire box*/
	float outerR = width + rCurve;
	float innerR = rCurve - width;
	if (innerR < 0.f)
		innerR = 0.f;
	for (int i_y = BL.x1; i_y <= UR.x1; i_y++) {
		for (int i_x = BL.x0; i_x <= UR.x0; i_x++) {
			if (isInArc(curvCentOffset, innerR, outerR, i_x, i_y))
				simg.img->SetRGBA(i_x, i_y, m_col);
			else if (isInTrap(curveOppositeU, tPt_r, tPerp_r, tPt_l, tPerp_l, i_x, i_y))
				simg.img->SetRGBA(i_x, i_y, m_col);
			else
				simg.img->SetRGBA(i_x, i_y, m_bakcol);
		}
	}
	return ECODE_OK;
}
unsigned char NNTrainGenArc::findBoxBounds(
	s_2pt& box1, s_2pt& box2, s_2pt& box3,
	s_2pt& box4, s_2pt& box5,
	s_2pt& boxBL, s_2pt& boxUR)
{
	s_2pt bxs[5];
	bxs[0] = box1;
	bxs[1] = box2;
	bxs[2] = box3;
	bxs[3] = box4;
	bxs[4] = box5;
	return findBoxBoundsFromArray(bxs, 5, boxBL, boxUR);
}
bool NNTrainGenArc::isInArc(s_2pt& center, float innerR, float outerR, long i_x, long i_y) {
	s_2pt_i vi = { i_x, i_y };
	s_2pt pt = imgMath::convToVfloat(vi);
	float distFromCenter = vecMath::dist(pt, center);
	return (distFromCenter <= outerR && distFromCenter >= innerR);
}
bool NNTrainGenArc::isInTrap(s_2pt& PerpIn, s_2pt& pt_r, s_2pt& Perp_r, s_2pt& pt_l, s_2pt& Perp_l, long i_x, long i_y) 
{
	s_2pt_i vi = { i_x, i_y };
	s_2pt pt = imgMath::convToVfloat(vi);
	s_2pt ptVs_r = vecMath::v12(pt_r, pt);
	s_2pt ptVs_l = vecMath::v12(pt_l, pt);
	float ptDot_r = vecMath::dot(ptVs_r, Perp_r);
	float ptDot_l = vecMath::dot(ptVs_l, Perp_l);
	float ptDot_middle = vecMath::dot(ptVs_r, PerpIn);
	return (ptDot_r >= 0.f && ptDot_l >= 0.f && ptDot_middle >= 0.f);
}