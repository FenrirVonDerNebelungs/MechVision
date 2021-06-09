#pragma once
#ifndef NNETGEN_H
#define NNETGEN_H

#ifndef HEXSTACK_H
#include "HexStack.h"
#endif
#ifndef NNODEBASE_H
#include "NNodeBase.h"
#endif

class NNetTestGen {
public:
	NNetTestGen();
	~NNetTestGen();
	void fillUnattachedTree(s_nNode midNodes[], s_nNode patternNodes[], s_nWrapperNode wrapperNodes[]);
	void releaseUnattachedTree();
protected:
	HexStack* m_hexMaster;
	/*arrays owned but not pointers in them*/
	s_nNode** m_pmidNodes;
	s_nNode** m_ppatternNodes;
	s_nNode** m_pwrapperNodes;
	/*     */

	void genLowNodeSet(s_nNode* pNodes[]);
	void genLowestNodePatterns(s_nNode* pN2[], s_nNode* pN3[], s_nNode* pN4[], s_nNode* pNFull, s_nNode* pNEmpt);
	void genLowestNodePattern(s_nNode* pN[], int arcLen);

	void connLowNodesToEmptyHexWrappers(s_nNode* pNodes[], s_nNode* pWrappers[]);/*pNodes is pNFull followed by 3 sets of 6 nodes,
																							  pWrappers are the 7 wrapper nodes that sit directly
																							  on top of the lowest hex*/
	void connLowNodeToEmptyHexWrappers(int pNodei, s_nNode* pNode, s_nNode* pWrappers[]);

	void connMidNodes(s_nNode* pMidNodes[], s_nNode* pPattNodes[]);/*this is connecting the entire set of low nodes, all 7 sets of 19, to the mid nodes*/
	void connMidNodeToLowNode(int midNodei, s_nNode* pMidNode, s_nNode* pPattNodes[]);

	int wrapI(int i, int n);
};

#endif