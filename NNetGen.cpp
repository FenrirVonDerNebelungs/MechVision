#include "stdafx.h"
#include "NNetGen.h"
NNetTestGen::NNetTestGen() :m_hexMaster(NULL), 
m_pmidNodes(NULL), m_ppatternNodes(NULL), m_pwrapperNodes(NULL){
	;
}
NNetTestGen::~NNetTestGen() {
	;
}
void NNetTestGen::fillUnattachedTree(s_nNode midNodes[], s_nNode patternNodes[], s_nWrapperNode wrapperNodes[])
{
	/*get an array of the pointer of the nodes instead of array of the nodes*/
	m_pmidNodes = new s_nNode * [NUMMIDNODES];
	m_ppatternNodes = new s_nNode * [140];
	m_pwrapperNodes = new s_nNode * [49];

	for(int i=0; i<NUMMIDNODES; i++)
		m_pmidNodes[i] = &(midNodes[i]);
	for (int i = 0; i < 140; i++)
		m_ppatternNodes[i] = &(patternNodes[i]);
	for (int i = 0; i < 49; i++) 
		m_pwrapperNodes[i] = (s_nNode*)&(wrapperNodes[i]);

	/*seperate the wrappper nodes & pattern nodes into sets that will correspond to each of the 7 level 1 hexes
	  then fill pattern nodes and link the 7 empty level 0 wrapper nodes to their coresponding 19 pattern nodes*/
	for (int hexCnt = 0; hexCnt < 7; hexCnt++) {
		s_nNode* lowNodes[NUMPATNODES];
		s_nNode* wrapNodes[7];
		int strt7 = 7 * hexCnt;
		int strtPat = NUMPATNODES * hexCnt;
		for (int i = 0; i < 7; i++) {
			wrapNodes[i] = (m_pwrapperNodes[strt7 + i]);
		}
		for (int i = 0; i < NUMPATNODES; i++) {
			lowNodes[i] = (m_ppatternNodes[strtPat + i]);
		}
		genLowNodeSet(lowNodes);
		connLowNodesToEmptyHexWrappers(lowNodes, wrapNodes);
	}
	connMidNodes(m_pmidNodes, m_ppatternNodes);
}
void NNetTestGen::releaseUnattachedTree() {
	if (m_pmidNodes != NULL)
		delete m_pmidNodes;
	m_pmidNodes = NULL;
	if (m_ppatternNodes != NULL)
		delete m_ppatternNodes;
	m_ppatternNodes = NULL;
	if (m_pwrapperNodes != NULL)
		delete m_pwrapperNodes;
	m_pwrapperNodes = NULL;
}
void NNetTestGen::genLowNodeSet(s_nNode* pNodes[]) {
	s_nNode* pN2[6];
	s_nNode* pN3[6];
	s_nNode* pN4[6];
	s_nNode* pNFull;
	s_nNode* pNEmpt;
	pNFull = pNodes[0];
	for (int i = 0; i < 6; i++) {
		pN4[i] = pNodes[i + 1];
		pN3[i] = pNodes[i + 7];
		pN2[i] = pNodes[i + 13];
	}
	pNEmpt = pNodes[19];
	genLowestNodePatterns(pN2, pN3, pN4, pNFull, pNEmpt);
}
void NNetTestGen::genLowestNodePatterns(s_nNode* pN2[], s_nNode* pN3[], s_nNode* pN4[], s_nNode* pNFull, s_nNode* pNEmpt) {
	if (pNFull->w == NULL) {
		pNFull->w = new float[7];
	}
	pNFull->N = 7;
	float fullw = 1.f / 7.f;
	for (int i = 0; i < 7; i++) {
		pNFull->w[i] = fullw;
		pNEmpt->w[i] = -fullw;
	}
	genLowestNodePattern(pN4, 3);
	genLowestNodePattern(pN3, 2);
	genLowestNodePattern(pN2, 1);

}
void NNetTestGen::genLowestNodePattern(s_nNode* pN[], int arcLen) {
	for (int curNd = 0; curNd < 6; curNd++) {
		pN[curNd]->N = 7;
		float posFracWeight = 1.f / (arcLen + 2.f);//1.f / (7.f - arcLen);
		float negFracWeight = -(1.f/(5.f-arcLen));//-1.f / arcLen;
		for (int i = 0; i < 7; i++)
			pN[curNd]->w[i] = negFracWeight;
		for (int i = 0; i <= arcLen; i++) {
			int i_Nd = wrapI(curNd+i, 6);
			pN[curNd]->w[i_Nd] = posFracWeight;
		}
		pN[curNd]->w[6] = posFracWeight;
	}
}
void NNetTestGen::connLowNodesToEmptyHexWrappers(s_nNode* pNodes[], s_nNode* pWrappers[]) {
	for (int i = 0; i < NUMPATNODES; i++) {
		connLowNodeToEmptyHexWrappers(i, pNodes[i], pWrappers);
	}
}
void NNetTestGen::connLowNodeToEmptyHexWrappers(int pNodei, s_nNode* pNode, s_nNode* pWrappers[]) {
	for (int i = 0; i < 7; i++) {
		pNode->nodes[i] = pWrappers[i];
		pWrappers[i]->hnodes[pNodei] = pNode;
	}
}
void NNetTestGen::connMidNodes(s_nNode* pMidNodes[], s_nNode* pPattNodes[]) {
	for (int i = 0; i < NUMMIDNODES; i++) {
		connMidNodeToLowNode(i, pMidNodes[i], pPattNodes);
	}
}
void NNetTestGen::connMidNodeToLowNode(int midNodei, s_nNode* pMidNode, s_nNode* pPattNodes[]) {
	for (int i = 0; i < 140; i++) {
		pMidNode->nodes[i] = pPattNodes[i];
		pPattNodes[i]->hnodes[midNodei] = pMidNode;
	}
	for(int i=0; i<NUMPATNODES; i++)
		pMidNode->w[i] = DEFAULTW;
}
int NNetTestGen::wrapI(int i, int n)
{
	int newi = i;
	if (i >= n)
		newi -= n;
	if (i < 0)
		newi += n;
	return newi;
}

