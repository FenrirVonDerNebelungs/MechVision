#pragma once
#ifndef NNETDRAW_H
#define NNETDRAW_H

#ifndef NNETTREE_H
#include "NNetTree.h"
#endif

class NNetDraw {
public:
	NNetDraw();
	~NNetDraw();

	unsigned char Init(HexStack* phexStack, NNetTreeTest* pNNTree);
	void Release();

	unsigned char Run();

	inline Img* getNNImg() { return m_NNimg; }
private:
	Img* m_NNimg;

	/*not owned*/
	HexStack* m_hexStack;
	NNetTreeTest* m_NNTree;
	/*         */

	unsigned char DrawWrapperNodeClusters();
	unsigned char DrawPatterns();
	unsigned char DrawLowestfs();
	unsigned char DrawNetStrongPat();

	void DrawWrapperNodeCluster(int clusterN, s_rgba& hexCol);
	void DrawPattern(int paintedClusterN, int patternN, s_rgba& patBack, s_rgba& patCol);
	int FindStrongestPat(int clusterN);

	s_hex* getHexFromPatternNode(s_nNode& pPatNode, int wrapperi);
};

#endif
