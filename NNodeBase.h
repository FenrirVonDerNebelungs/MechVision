#pragma once
#ifndef NNODEBASE_H
#define NNODEBASE_H
#ifndef HEXSTACK_H
#include "HexStack.h"
#endif

#define NUMMIDNODES 2 /*this number will change*/
#define NUMPATNODES 20
#define DEFAULTW -1.f
#define MAXNUMTRAINIMGS 100


class s_nNode {
public:
	s_nNode();
	~s_nNode();
	s_nNode** nodes;/*lower nodes*/
	float* w;/*weights of each node*/
	int    N;/*number of w's & lower nodes*/
	float  f;/*result of the network*/
	s_nNode** hnodes;/*higher node*/
	int    hN;/*number of higher nodes*/

};
class s_nWrapperNode : public s_nNode {
public:
	s_nWrapperNode();
	~s_nWrapperNode();
	s_hex* shex;
};

#endif





