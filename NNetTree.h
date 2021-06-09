#pragma once
#ifndef NNETTREE_H
#define NNETTREE_H

#ifndef NNETGEN_H
#include "NNetGen.h"
#endif
#ifndef NNETCOLWHEEL_H
#include "NNetColwheel.h"
#endif

class NNetTreeTest {
public:
	NNetTreeTest();
	~NNetTreeTest();

	/*construct the nodes up 3 levels to level 2 but without attaching to pattern
      these nodes hang from the level 3 node*/
	unsigned char Init(
		HexStack* hexMaster,
		int topHexi=0
		);
	void Release();

	void run();

	inline s_nWrapperNode* getWrapperNodes() { return m_wrapperNodes; }

	void setMidNodeWeights(int i, float ws[]);
	inline float getMidNodef(int i) { return m_midNodes[i].f; }
	inline s_nNode& getMidNode(int i) { return m_midNodes[i]; }
	inline s_nNode* getPatternNodes() { return m_patternNodes; }
	inline int getNpatternNodes() { return m_N_patternNodes; }
	float NNetFunction(float nin);/*step function that converts input to value between 0 and 1*/ /*1/(1+exp(-n))*/
	float evalWeightedNs(int num, float ws[], float ns[]);/*multiply w*n +... for all terms */
protected:
	HexStack* m_hexMaster;/*not owned*/

	int m_topHexi;/*level2 hex 0,1,2 levels of the image which is being evaluated*/
	/*owned*/
	NNetTestGen* m_nnetTestGen;/*helps Init*/
	NNetTestColwheel* m_colWheel;
	/*      */

	/*owned*/
	s_nWrapperNode* m_wrapperNodes;
	s_nNode* m_patternNodes;/*"lowest nodes"*/
	s_nNode* m_midNodes;
	int m_N_wrapperNodes;
	int m_N_patternNodes;
	int m_N_midNodes;
	/*    */

	void AttachTreeToHexImg();
	void AttachTreeToLevel2Hex(int tophexi);
	void fillWrappers(int firstHokHexi, s_nWrapperNode* pWrappers[]);

	s_hex* getHexPtrFromIndex(int origWebi);
	/****/

	void EvalWeights(s_nNode& node);
	void evalLevel0();
	void evalhigherLevels();
};

#endif

