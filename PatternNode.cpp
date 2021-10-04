#include "PatternNode.h"

s_bNode::s_bNode() :o(0.f), thislink(-1) { ; }
s_bNode::~s_bNode() { ; }

s_fNode::s_fNode() : shex(NULL), x(0.f), y(0.f), nodes(NULL), w(NULL), N(0) { ; }
s_fNode::~s_fNode() { ; }
void s_fNode::zero()
{ 
	shex = NULL;
	x = 0.f; 
	y = 0.f; 
	nodes = NULL;
	w = NULL;
	N = 0;
	s_bNode::zero(); 
}

void s_fNode::init() {
	shex = new s_hex;
	PatStruct::zeroHex(*shex);
}
void s_fNode::initNodePtrs(int N_nodes) {
	nodes = new s_bNode * [N_nodes];
	w = new float[N_nodes];
	N = 0;
	for (int i = 0; i < N_nodes; i++)
		nodes[i] = NULL;
}
void s_fNode::release() {
	if (shex != NULL)
		delete shex;
	shex = NULL;
	if (nodes != NULL)
		delete[] nodes;
	nodes = NULL;
	if (w != NULL)
		delete[] w;
	w = NULL;
}
void s_fNode::releaseNodePtrs() {
	if (w != NULL)
		delete[] w;
	if (nodes != NULL)
		delete[] nodes;
	N = 0;
}
s_pNode::s_pNode() : nodes(NULL), w(NULL), N(0) { ; }
s_pNode::~s_pNode() { ; }

s_edgeNode::s_edgeNode() : uPerp_x(0.f), uPerp_y(0.f) { ; }
s_edgeNode::~s_edgeNode() { ; }

namespace PatStruct {
	void zeroHex(s_hex& hex) {
		hex.colSet = false;
		hex.i = -1;
		hex.j = -1;
		hex.x = 0.f;
		hex.y = 0.f;
		hex.thisLink = -1;
		for (int i = 0; i < 6; i++) {
			hex.web[i] = -1;
			hex.downLinks[i] = -1;
		}
		hex.centerLink = -1;
		hex.rgb[0] = 0.f;
		hex.rgb[1] = 0.f;
		hex.rgb[2] = 0.f;
	}
	void copyHex(s_hex& orig, s_hex& hex) {
		hex.colSet = orig.colSet;
		hex.i = orig.i;
		hex.j = orig.j;
		hex.x = orig.x;
		hex.y = orig.y;
		hex.thisLink = orig.thisLink;
		for (int i = 0; i < 6; i++) {
			hex.web[i] = orig.web[i];
			hex.downLinks[i] = orig.downLinks[i];
			hex.centerLink = orig.centerLink;
		}
		for (int i = 0; i < 3; i++) {
			hex.rgb[i] = orig.rgb[i];
		}
	}
	void zeroHexPlate(s_hexPlate& plate) {
		plate.m_fhex = NULL;
		plate.m_nHex = 0;
		plate.m_Rhex = 0.f;
		plate.m_RShex = 0.f;
		plate.m_Shex = 0.f;
		for (int i = 0; i < 6; i++)
			utilStruct::zero2pt(plate.m_hexU[i]);
	}
	void zeroPlateLayer(s_PlateLayer& player) {
		for (int i = 0; i < MAXPLATESPERLAYER; i++)
			zeroHexPlate(player.p[i]);
		player.n = 0;
	}
	void hexPlateConnectWeb(s_hexPlate& plate) {
		for (long i = 0; i < plate.m_nHex; i++) {
			plate.m_fhex[i].nodes = new s_bNode * [6];
			plate.m_fhex[i].N = 6;
			for (int j = 0; j < 6; j++) {
				int curwebhex_i = plate.m_fhex[i].shex->web[j];
				if (curwebhex_i >= 0)
					plate.m_fhex[i].nodes[j] = (s_bNode*)&(plate.m_fhex[curwebhex_i]);
				else
					plate.m_fhex[i].nodes[j] = NULL;
			}
		}
	}
	void hexPlateReleaseWeb(s_hexPlate& plate) {
		for (long i = 0; i < plate.m_nHex; i++) {
			if (plate.m_fhex[i].nodes != NULL)
				delete[] plate.m_fhex[i].nodes;
			plate.m_fhex[i].nodes = NULL;
		}
	}
}