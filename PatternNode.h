#pragma once
#ifndef PATTERNNODE_H
#define PATTERNNODE_H

#ifndef MATH_H
#include "Math.h"
#endif

#define MAXPLATESPERLAYER 100
#define PATTERNNODE_MAXTPTS 10

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

	long thislink;
	float o; /*result of this patterns evaluation*/
};
class s_fNode : public s_bNode {/*foot wrapper node*/
public:
	s_fNode();
	~s_fNode();
	void init(); /*should only be called if shex should be owned*/
	void initWebPtrs(int N_web);/*initalize the web ptrs*/
	void initNodePtrs(int N_nodes);/*initialize N node pointers and the corresponding weights*/
	void zero();
	void addHexData(s_hex& orig_hex) { PatStruct::copyHex(orig_hex, *shex); }/*creates a copy which is stored by this object*/
	void release();/*should only be called if s_hex is owned */
	void releaseWebPtrs();/*assumes the nodes of the web are not owned just the pointers*/
	void releaseNodePtrs();/*assumes the nodes themselves are not owned just the pointers*/

	s_hex* shex;
	float x;
	float y;

	int       Nweb;/*number of nodes in the web*/
	s_bNode** web;/*nodes around this node*/
	s_bNode** nodes;/*lower nodes that nodes that are pointed to are assumed not to be owned however the array is owned*/
	float* w;/*weights of each lower node the array is owned*/
	int N;/*number of lower nodes*/
	float b;/*offset used by nnets*/
};

class s_tNode : public s_bNode {/*trace node*/
public:
	long* trail;/*trail in indexes of previous locations*/
	int len;/*length of trail*/
	int t_i;/*current location in trail, the trail loops around on its end*/

	int frameN;/*age of the trail in frames*/
};
class s_pNode : public s_bNode {
public:
	s_pNode();
	~s_pNode();

	s_bNode** nodes;/*lower nodes typically 6 surrounding 7th is center*/
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
struct s_tPlate {
	s_tNode* m_tnodes;
	long     m_n;
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


	void genLayerUp(const s_hexPlate& plate0, s_hexPlate& plate1);
	void releaseLayerUp(s_hexPlate& plate);
	long rotateCK2(const s_hexPlate& plate0, long i, int web_strt);
	long rotateCCK2(const s_hexPlate& plate0, long i, int web_strt);
	void webLinkInLine(const s_hexPlate& plate0, s_hexPlate& plate1);
	bool findLeftNOneUp(const s_hexPlate& plate, long& i0, long& i1);
	bool findWeaveStart(const s_hexPlate& plate0, const s_hexPlate& plate1, int weave_dir, long& i0, long& i1);
	void weaveAdjRows(const s_hexPlate& plate0, s_hexPlate& plate1, int weave_dir, long& i0, long& i1);
	bool weaveRows(const s_hexPlate& plate0, s_hexPlate& plate1);

	void genPlateWSameWeb(const s_hexPlate& plate0, s_hexPlate& plate1);
	void releasePlateWSameWeb(s_hexPlate& plate);
}

#endif
