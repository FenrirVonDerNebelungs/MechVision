#include "stdafx.h"
#include "PatternDraw.h"

PatternDraw::PatternDraw() : m_hexStack(NULL), m_patScan(NULL), m_imgOut(NULL)
{
	;
}
PatternDraw::~PatternDraw() { ; }

unsigned char PatternDraw::init(PatternScan* patScan, LineFinder* linefinder, LaneLineFinder* lanelinefind) {
	m_patScan = patScan;
	m_lineFinder = linefinder;
	m_laneLineFinder = lanelinefind;
	m_hexStack = patScan->getHexStack();

	m_imgOut = new Img;
	HexBase* hexb = m_hexStack->getLayer(0);
	Img* baseImg = hexb->getImg();
	m_imgOut->init(baseImg->getWidth(), baseImg->getHeight());
	m_imgOut->clearToChar(0x00);
	return ECODE_OK;
}
void PatternDraw::release() {
	if (m_imgOut != NULL) {
		m_imgOut->release();
		delete m_imgOut;
		m_imgOut = NULL;
	}
	m_hexStack = NULL;
	m_patScan = NULL;
}

unsigned char PatternDraw::run() {
	//drawLines();
	drawLaneLines();
	//drawMaxEdgeStrengthAbove(0.4f);
	return ECODE_OK;
}

void PatternDraw::drawFeetfs() {
	int num = m_patScan->getNFootNodes();
	s_fNode** fNodes = m_patScan->getFootNodes();
	Img* pMask = m_hexStack->getLayer(0)->getHexMask();
	for (int i = 0; i < num; i++) {
		float curF = fNodes[i]->f;
		s_hex* curHex = fNodes[i]->shex;
		s_rgba grayCol;
		convFloatToGrayScale(curF, grayCol);
		m_imgOut->PrintMaskedImg(curHex->i, curHex->j, *pMask, grayCol);
	}
}
void PatternDraw::drawEdgeStrength() {
	int num = m_patScan->getNFootNodes();
	s_fNode** fNodes = m_patScan->getFootNodes();
	Img* pMask = m_hexStack->getLayer(0)->getHexMask();

	s_luna4fs* lunafs = m_patScan->getEdgefs();
	int selPatOrient = 5;

	for (int i = 0; i < num; i++) {
		float curF = lunafs[i].f[selPatOrient];
		s_hex* curHex = fNodes[i]->shex;
		s_rgba grayCol;
		convFloatToGrayScale(curF, grayCol);
		m_imgOut->PrintMaskedImg(curHex->i, curHex->j, *pMask, grayCol);
	}
}
void PatternDraw::drawMaxEdgeStrengthAbove(float minf) {
	int num = m_patScan->getNFootNodes();
	s_fNode** fNodes = m_patScan->getFootNodes();
	Img* pMask = m_hexStack->getLayer(0)->getHexMask();

	s_luna4fs* lunafs = m_patScan->getEdgefs();
	int selPatOrient = 5;

	for (int i = 0; i < num; i++) {
		float curF = lunafs[i].edgef;
		if (curF > minf) {
			s_hex* curHex = fNodes[i]->shex;
			s_rgba grayCol;
			convFloatToGrayScale(curF, grayCol);
			//grayCol.b = 0x33;
			m_imgOut->PrintMaskedImg(curHex->i, curHex->j, *pMask, grayCol);
		}
	}
}

void PatternDraw::convFloatToGrayScale(float f, s_rgba& grayCol) {
	grayCol.a = 0xff;
	if (f < 0.f)
		f = 0.f;
	float gcol = 255.f * f;
	if (gcol > 255.f)
		gcol = 255.f;
	int gcoli = (int)floorf(gcol);
	grayCol.r = (unsigned char)gcoli;
	grayCol.g = (unsigned char)gcoli;
	grayCol.b = (unsigned char)gcoli;
}
void PatternDraw::drawLines() {
	int n = m_lineFinder->getNLines();
	for (int i = 0; i < n; i++) {
		if (m_lineFinder->getLine(i).n >= 3)
			drawLine(m_lineFinder->getLine(i));
	}
}
void PatternDraw::drawLaneLines() {
	int n = m_laneLineFinder->getNLines();
	for (int i = 0; i < n; i++) {
		if (m_laneLineFinder->getLine(i).n >= 1)
			drawLine(m_laneLineFinder->getLine(i));
	}
}
void PatternDraw::drawLine(s_line& line) {
	float vecScale = 7.f;
	float vecThickness = 2.f;
	s_rgba hexMarkerCol = { 0xff, 0x44, 0xff, 0xff };
	s_rgba hexMarkerHi = { 0xff, 0xee, 0xff, 0xff };
	s_rgba vecCol = { 0x33, 0xff, 0xff, 0xff };
	s_rgba lineCol = { 0xaa, 0x44, 0xff, 0xff };
	HexBase* curLayer = m_hexStack->getLayer(m_lineFinder->getStackLevel());
	s_hex* hexes = curLayer->getHexes();
	Img* pMask = curLayer->getHexMask();
	
	for (int i = 0; i < line.n; i++) {
		int hexi = line.pts[i].hexi;
		m_imgOut->PrintMaskedImg(hexes[hexi].i, hexes[hexi].j, *pMask, hexMarkerCol);
		s_2pt scaledPerp = vecMath::mul(vecScale, line.pts[i].perp);
		imgMath::drawV(scaledPerp, hexes[hexi].i, hexes[hexi].j, vecCol, vecThickness, m_imgOut);
	}
	for (int i = 2; i < line.n; i++) {
		int hsi = line.pts[i - 1].hexi;
		int hei = line.pts[i].hexi;
		imgMath::drawLine(hexes[hsi].i, hexes[hsi].j, hexes[hei].i, hexes[hei].j, lineCol, vecThickness, m_imgOut);
	}
	int hsi = line.pts[0].hexi;
	int hei = line.pts[1].hexi;
	imgMath::drawLine(hexes[hsi].i, hexes[hsi].j, hexes[hei].i, hexes[hei].j, hexMarkerHi, vecThickness, m_imgOut);
}