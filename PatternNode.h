#pragma once
#ifndef PATTERNNODE_H
#define PATTERNNODE_H
#ifndef HEXBASE_H
#include "HexBase.h"
#endif
class s_bNode {/*base node*/
public:
	s_bNode();
	~s_bNode();
	float f; /*result of this patterns evaluation*/
};
class s_fNode : public s_bNode{/*foot wrapper node*/
public:
	s_fNode();
	~s_fNode();

	s_hex* shex;
	float x;
	float y;
};

class s_pNode : public s_bNode{
public:
	s_pNode();
	~s_pNode();
	s_bNode** nodes;/*lower nodes*/
	float* w;/*weights of each lower node*/
	int N;/*number of lower nodes*/
};
class s_edgeNode : public s_pNode{
public:
	s_edgeNode();
	~s_edgeNode();
	float uPerp_x; /*coordinates of unit vector pointed away from the line/arc edge*/
	float uPerp_y;
};
#endif
