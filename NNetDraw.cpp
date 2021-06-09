#include "stdafx.h"
#include "NNetDraw.h"

NNetDraw::NNetDraw() : m_NNimg(NULL), m_hexStack(NULL), m_NNTree(NULL) 
{
	;
}
NNetDraw::~NNetDraw()
{
	;
}
unsigned char NNetDraw::Init(HexStack* phexStack, NNetTreeTest* pNNTree) {
	m_hexStack = phexStack;
	m_NNTree = pNNTree;

	m_NNimg = new Img;
	HexBase* hexb = m_hexStack->getLayer(0);
	Img* baseImg = hexb->getImg();
	m_NNimg->init(baseImg->getWidth(), baseImg->getHeight());
	m_NNimg->clearToChar(0x00);
	return ECODE_OK;
}
void NNetDraw::Release() {
	if (m_NNimg != NULL) {
		m_NNimg->release();
		delete m_NNimg;
		m_NNimg = NULL;
	}
	m_NNTree = NULL;
	m_hexStack = NULL;
}
unsigned char NNetDraw::Run()
{
	return DrawNetStrongPat();//DrawLowestfs();//
}
unsigned char NNetDraw::DrawWrapperNodeClusters() {
	s_rgba hexCol[7];
	for (int i = 0; i < 7; i++) {
		hexCol[i].a = 0xff;
		hexCol[i].b = 0x00;
		hexCol[i].g = 0x11;
		hexCol[i].r = 0x00;
	}
	hexCol[1].r = 0x50;
	hexCol[2].r = 0xaa;
	hexCol[3].r = 0xff;
	hexCol[4].b = 0x50;
	hexCol[5].b = 0xaa;
	hexCol[6].b = 0xff;
	/*
	for (int i = 0; i < 7; i++) {
		DrawWrapperNodeCluster(i, hexCol[i]);
	}
	*/
	DrawWrapperNodeCluster(0, hexCol[0]);
	DrawWrapperNodeCluster(2, hexCol[2]);
	DrawWrapperNodeCluster(4, hexCol[4]);
	//DrawWrapperNodeCluster(5, hexCol[5]);
	return ECODE_OK;
}
unsigned char NNetDraw::DrawPatterns() {
	s_rgba patBak{ 0x11, 0x33, 0x22, 0xff };
	s_rgba patCol{ 0xaa, 0xff, 0xaa, 0xff };
	DrawPattern(1, 18, patBak, patCol);
	DrawPattern(3, 0, patBak, patCol);
	DrawPattern(5, 0, patBak, patCol);
	patCol.r = 0xff;
	DrawPattern(6, 0, patBak, patCol);
	return ECODE_OK;
}
unsigned char NNetDraw::DrawLowestfs()
{
	s_nWrapperNode* wrapNodes = m_NNTree->getWrapperNodes();
	HexBase* lowestHexBase = m_hexStack->getLayer(0);
	Img* pMask = lowestHexBase->getHexMask();
	for (int wni = 0; wni < 49; wni++) {
		s_hex* curHex = wrapNodes[wni].shex;
		float f = wrapNodes[wni].f;
		float intens = f * 255.f;
		s_rgba curCol = imgMath::convToRGBA(intens, intens, intens);
		m_NNimg->PrintMaskedImg(curHex->i, curHex->j, *pMask, curCol);
	}
	return ECODE_OK;
}
unsigned char NNetDraw::DrawNetStrongPat()
{
	s_rgba patBak{ 0x11, 0x33, 0x22, 0xff };
	s_rgba patCol{ 0x77, 0xff, 0x77, 0xff };
	for (int i = 0; i < 7; i++) {
		int strongestPati = FindStrongestPat(i);
		s_rgba colType1 = { 0xff, 0x00, 0x00, 0xff };
		s_rgba colType2 = { 0x00, 0xff, 0x00, 0xff };
		s_rgba colType3 = { 0x00, 0x00, 0xff, 0xff };
		if (strongestPati >= 1 && strongestPati <= 6)
			patCol = colType1;
		else if (strongestPati >= 6 && strongestPati <= 13)
			patCol = colType2;
		else if (strongestPati >= 14 && strongestPati <= 19)
			patCol = colType3;
		if(strongestPati>=0)
			DrawPattern(i, strongestPati, patBak, patCol);
	}
	return ECODE_OK;
}
void NNetDraw::DrawWrapperNodeCluster(int clusterN, s_rgba& hexCol) {
	/*draw wrapper nodes*/
	HexBase* lowestHexBase = m_hexStack->getLayer(0);
	Img* pMask = lowestHexBase->getHexMask();
	s_nWrapperNode* pWrapperNodes = m_NNTree->getWrapperNodes();

	int starti = clusterN * 7;

	for (int i = 0; i < 7; i++) {
		int curi = i + starti;
		s_hex* phex = pWrapperNodes[curi].shex;
		s_rgba curCol = hexCol;
		curCol.g += i * 0x11;
		m_NNimg->PrintMaskedImg(phex->i, phex->j, *pMask, curCol);
	}
}
void NNetDraw::DrawPattern(int paintedClusterN, int patternN, s_rgba& patBack, s_rgba& patCol) {
	HexBase* lowestHexBase = m_hexStack->getLayer(0);
	Img* pMask = lowestHexBase->getHexMask();
	s_nNode* patNodes = m_NNTree->getPatternNodes();
	/*find the index of the pat node*/
	int starti = paintedClusterN * NUMPATNODES;
	int cur_patNode = starti + patternN;
	/*                              */

	for (int i = 0; i < 7; i++) 
	{
		float curWeight = patNodes[cur_patNode].w[i];
		s_rgba curCol = (curWeight > 0.01f) ? patCol : patBack;
		s_hex* curHex = getHexFromPatternNode(patNodes[cur_patNode], i);
		
		m_NNimg->PrintMaskedImg(curHex->i, curHex->j, *pMask, curCol);
	}
}
int NNetDraw::FindStrongestPat(int clusterN) {
	int starti = clusterN * NUMPATNODES;
	float min = 0.6f;
	int hii = -1;
	s_nNode* patNodes = m_NNTree->getPatternNodes();
	for (int i = 0; i < NUMPATNODES; i++) {
		int curi = starti + i;
		/*debug*/
		float debugf = patNodes[curi].f;
		/*******/
		if (patNodes[curi].f > min) {
			min = patNodes[curi].f;
			hii = i;
		}
	}
	return hii;
}
s_hex* NNetDraw::getHexFromPatternNode(s_nNode& pPatNode, int wrapperi)
{
	s_nWrapperNode* curWrapper = (s_nWrapperNode*)pPatNode.nodes[wrapperi];
	return curWrapper->shex;
}