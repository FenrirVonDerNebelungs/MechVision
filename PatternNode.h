#pragma once
#ifndef PATTERNNODE_H
#define PATTERNNODE_H

#ifndef MATH_H
#include "Math.h"
#endif

#define MAXPLATESPERLAYER 100


struct s_hex {
	bool colSet;
	long i;
	long j;
	float x;
	float y;

	int thisLink;
	int web[6];
	int downLinks[6];
	int centerLink;

	float rgb[3];
};
namespace PatStruct {
	void zeroHex(s_hex& hex);
	void copyHex(s_hex& orig, s_hex& hex);
}
class s_bNode {/*base node*/
public:
	s_bNode();
	~s_bNode();
	virtual void zero() { o = 0.f; thislink = -1; }

	int thislink;
	float o; /*result of this patterns evaluation*/
};
class s_fNode : public s_bNode {/*foot wrapper node*/
public:
	s_fNode();
	~s_fNode();
	void init(); /*should only be called if shex should be owned*/
	void initNodePtrs(int N_nodes);/*initialize N node pointers and the corresponding weights*/
	void zero();
	void addHexData(s_hex& orig_hex) { PatStruct::copyHex(orig_hex, *shex); }/*creates a copy which is stored by this object*/
	void release();/*should only be called if s_hex is owned */
	void releaseNodePtrs();/*assumes the nodes themselves are not owned just the pointers*/

	s_hex* shex;
	float x;
	float y;

	s_bNode** nodes;/*lower nodes that nodes that are pointed to are assumed not to be owned however the array is owned*/
	float* w;/*weights of each lower node the array is owned*/
	int N;/*number of lower nodes*/
};

class s_pNode : public s_bNode {
public:
	s_pNode();
	~s_pNode();

	s_bNode** nodes;/*lower nodes*/
	float* w;/*weights of each lower node*/
	int N;/*number of lower nodes*/
};

struct s_hexPlate {
	s_fNode* m_fhex;
	long    m_nHex;

	/*for img output*/
	float m_Rhex;
	float m_RShex;
	float m_Shex;
	s_2pt m_hexU[6];
};
struct s_PlateLayer {
	s_hexPlate p[MAXPLATESPERLAYER];
	int n;
};

class s_edgeNode : public s_pNode{
public:
	s_edgeNode();
	~s_edgeNode();
	float uPerp_x; /*coordinates of unit vector pointed away from the line/arc edge*/
	float uPerp_y;
};

namespace PatStruct{
	void zeroHexPlate(s_hexPlate& plate);
	void zeroPlateLayer(s_PlateLayer& player);

	void hexPlateConnectWeb(s_hexPlate& plate);/*assumes the hexes in the plates' s_fNodes have already been filled
											    connects the splayed s_fnode that attached directly to the plate
												so that the nodes in this case point to the surroundign 6 web of 
												s_fnodes instead of pointing downward */
	void hexPlateReleaseWeb(s_hexPlate& plate);
}

#endif
