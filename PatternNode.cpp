#include "PatternNode.h"

s_bNode::s_bNode() :o(0.f), thislink(-1) { ; }
s_bNode::~s_bNode() { ; }

s_fNode::s_fNode() : shex(NULL), x(0.f), y(0.f), Nweb(0), web(NULL), nodes(NULL), w(NULL), N(0), b(0.f) { ; }
s_fNode::~s_fNode() { ; }
void s_fNode::zero()
{ 
	shex = NULL;
	x = 0.f; 
	y = 0.f; 
	Nweb = 0;
	web = NULL;
	nodes = NULL;
	w = NULL;
	N = 0;
	b = 0.f;
	s_bNode::zero(); 
}

void s_fNode::init() {
	shex = new s_hex;
	PatStruct::zeroHex(*shex);
}
void s_fNode::initWebPtrs(int N_web) {
	web = new s_bNode * [N_web];
	for (int i = 0; i < N_web; i++) {
		web[i] = NULL;
	}
	N_web = 0;
}
void s_fNode::initNodePtrs(int N_nodes) {
	nodes = new s_bNode * [N_nodes];
	w = new float[N_nodes];
	N = 0;
	for (int i = 0; i < N_nodes; i++) {
		nodes[i] = NULL;
		w[i] = -1.f;
	}
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
	if (web != NULL)
		delete[] web;
	web = NULL;
}
void s_fNode::releaseWebPtrs() {
	if (web != NULL)
		delete[] web;
	web = NULL;
	Nweb = 0;
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
		plate.m_RowStart = NULL;
		plate.m_RowStart_is = NULL;
		plate.m_Row_N = 0;
		plate.m_Col_d = 0.f;
		plate.m_Row_d = 0.f;
	}
	void zeroPlateLayer(s_PlateLayer& player) {
		for (int i = 0; i < MAXPLATESPERLAYER; i++)
			zeroHexPlate(player.p[i]);
		player.n = 0;
	}
	long squarePlate_xyToHexi(const s_hexPlate& p, const s_2pt& xy) {
		if (xy.x0 < 0.f || xy.x1 < 0.f)
			return -1;
		if (xy.x1 >= p.m_height || xy.x0 >= p.m_width)
			return -1;
		//float colH = 0.66666f * p.m_Rhex;
		//float rowW = 2.f * p.m_RShex;
		float cnt_y = (xy.x1 - p.m_RowStart[0].x1)/p.m_Row_d;
		if (cnt_y < 0.f)
			cnt_y = 0.f;
		long row_y_i = (long)roundf(cnt_y);
		if (row_y_i >= p.m_Row_N)
			return -1;

		/*find how far the first x_offset is in using the known row_i*/
		float offset_x = p.m_RowStart[row_y_i].x0;
		float cnt_x = (xy.x0 - offset_x) / p.m_Col_d;
		if (cnt_x < 0.f)
			cnt_x = 0.f;
		long col_x_i = (long)roundf(cnt_x);
		if (col_x_i >= p.m_RowStart_is[row_y_i].x1)
			return -1;
		return p.m_RowStart_is[row_y_i].x0 + col_x_i;
	}
	unsigned char initHexPlateRowColStart(s_hexPlate& p) {
		if (p.m_fhex == NULL)
			return 0x01;
		p.m_Row_N = 0;
		p.m_Col_d = 2.f / 3.f * p.m_Rhex;
		p.m_Row_d = 2.f * p.m_RShex;
		for (long i = 0; i < p.m_nHex; i++) {
			if (p.m_fhex[i].web[3] < 0) {
				p.m_Row_N++;
			}
		}
		p.m_RowStart = new s_2pt[p.m_Row_N];
		p.m_RowStart_is = new s_2pt_i[p.m_Row_N];
		long rowCnt = 0;
		for (long i = 0; i < p.m_nHex; i++) {
			if (p.m_fhex[i].web[3] < 0) {
				p.m_RowStart[rowCnt].x0 = p.m_fhex[i].x;
				p.m_RowStart[rowCnt].x1 = p.m_fhex[i].y;
				p.m_RowStart_is[rowCnt].x0 = i;
				p.m_RowStart_is[rowCnt].x1 = 0;
				while (p.m_fhex[i + p.m_RowStart_is[rowCnt].x1].web[0] >= 0) {
					p.m_RowStart_is[rowCnt].x1 += 1;
				};
			}
		}
		return 0x00;
	}
	void releaseHexPlateRowColStart(s_hexPlate& p) {
		if (p.m_RowStart_is != NULL)
			delete[] p.m_RowStart_is;
		if (p.m_RowStart != NULL)
			delete[] p.m_RowStart;
		p.m_Row_N = 0;
	}
	void hexPlateConnectWeb(s_hexPlate& plate) {
		for (long i = 0; i < plate.m_nHex; i++) {
			plate.m_fhex[i].nodes = new s_bNode * [6];
			plate.m_fhex[i].web = new s_bNode * [6];
			plate.m_fhex[i].N = 6;
			plate.m_fhex[i].Nweb = 6;
			for (int j = 0; j < 6; j++) {
				int curwebhex_i = plate.m_fhex[i].shex->web[j];
				if (curwebhex_i >= 0) {
					plate.m_fhex[i].nodes[j] = (s_bNode*)&(plate.m_fhex[curwebhex_i]);
					plate.m_fhex[i].web[j] = (s_bNode*)&(plate.m_fhex[curwebhex_i]);
				}
				else {
					plate.m_fhex[i].nodes[j] = NULL;
					plate.m_fhex[i].web[j] = NULL;
				}
			}
		}
	}
	void hexPlateReleaseWeb(s_hexPlate& plate) {
		for (long i = 0; i < plate.m_nHex; i++) {
			if (plate.m_fhex[i].nodes != NULL)
				delete[] plate.m_fhex[i].nodes;
			plate.m_fhex[i].nodes = NULL;
			if (plate.m_fhex[i].web != NULL)
				delete[] plate.m_fhex[i].web;
			plate.m_fhex[i].web = NULL;
		}
	}

	void genLayerUp(const s_hexPlate& plate0, s_hexPlate& plate1) {
		plate1.m_Rhex = 2.f * plate0.m_Rhex;
		plate1.m_RShex = 2.f * plate0.m_RShex;
		plate1.m_Shex = 2.f * plate0.m_Shex;
		for (int i = 0; i < 6; i++)
			plate1.m_hexU[i] = plate0.m_hexU[i];
		plate1.m_nHex = 0;
		long numNewNodes = plate0.m_nHex / 4 + 1;
		plate1.m_fhex = new s_fNode[numNewNodes];
		for (long i = 0; i < numNewNodes; i++) {
			plate1.m_fhex[i].initNodePtrs(7);
			plate1.m_fhex[i].initWebPtrs(6);
		}
		int web_dir = 0;
		long i;
		do{
			/*check if this hex has all 6 connections*/
			bool conn = true;
			for (int web_i = 0; web_i < 6; web_i++) {
				if (plate0.m_fhex[i].web[web_i] == NULL) {
					conn = false;
					break;
				}
			}
			if (conn) {
				/*following what is done for the eye the center is at zero then the surroundings are 1->7*/
				plate1.m_fhex[plate1.m_nHex].nodes[0] = (s_bNode*)&(plate0.m_fhex[i]);
				for (int web_i = 0; web_i < 6; web_i++) {
					plate1.m_fhex[plate1.m_nHex].nodes[1 + web_i] = plate0.m_fhex[i].web[web_i];
				}
				plate1.m_fhex[plate1.m_nHex].thislink = plate1.m_nHex;
				plate1.m_fhex[plate1.m_nHex].x = plate0.m_fhex[i].x;
				plate1.m_fhex[plate1.m_nHex].y = plate0.m_fhex[i].y;
				plate1.m_nHex++;
				/*try to go 2 over in the current web direction*/
				s_fNode* oneOver = (s_fNode*)plate0.m_fhex[i].web[web_dir];
				/*know one over not null since already checked*/
				s_fNode* twoOver = (s_fNode*)oneOver->web[web_dir];
				if (twoOver != NULL) {
					i = twoOver->thislink;
				}
				else {
					long skip_line_i = -1;
					if (web_dir == 0)
						skip_line_i = rotateCCK2(plate0, i, web_dir);
					else
						skip_line_i = rotateCK2(plate0, i, web_dir);
					if (skip_line_i >= 0) {
						int new_web_dir = (web_dir == 0) ? 3 : 0;
						web_dir = new_web_dir;
						i = skip_line_i;
					}
					else
						break;
				}
			}
			else
				i++;
		} while (i < plate0.m_nHex);
		webLinkInLine(plate0, plate1);
		weaveRows(plate0, plate1);
	}
	void releaseLayerUp(s_hexPlate& plate) {
		;
	}
	void webLinkInLine(const s_hexPlate& plate0, s_hexPlate& plate1) {
		int web_dir = 0;
		int web_dir_op = 3;
		for (long i=0; i < (plate1.m_nHex-1); i++) {
			long lowMid_i = plate1.m_fhex[i].nodes[web_dir]->thislink;
			long backLowMid_i = plate1.m_fhex[i+1].nodes[web_dir_op]->thislink;
			if (lowMid_i == backLowMid_i) {
				plate1.m_fhex[i].web[web_dir] = (s_bNode*)&(plate1.m_fhex[i + 1]);
				plate1.m_fhex[i + 1].web[web_dir_op] = (s_bNode*)&(plate1.m_fhex[i]);
			}
			else {
				int hold_web_dir = web_dir;
				web_dir = web_dir_op;
				web_dir_op = hold_web_dir;
			}
		}
	}
	bool weaveRows(const s_hexPlate& plate0, s_hexPlate& plate1) {
		long i0 = 0;
		long i1 = 1;
		do {
			if (!findLeftNOneUp(plate1, i0, i1))
				return false;
			int weave_dir = 1;
			long wi0 = i0;
			long wi1 = i1;
			findWeaveStart(plate0, plate1, weave_dir, wi0, wi1);
			int hiw1 = wi1;
			weaveAdjRows(plate0, plate1, weave_dir, wi0, wi1);
			weave_dir = 2;
			wi0 = i0;
			wi1 = i1;
			findWeaveStart(plate0, plate1, weave_dir, wi0, wi1);
			if (wi1 > hiw1)
				hiw1 = wi1;
			weaveAdjRows(plate0, plate1, weave_dir, wi0, wi1);
			i0 = i1;
			i1 = hiw1 + 1;
		} while (i1 < plate1.m_nHex);
		return true;
	}
	bool findLeftNOneUp(const s_hexPlate& plate, long& i0, long& i1) {
		long i0_cur = i0;
		bool foundendleft = false;
		for (long ii0 = i0; ii0 >= 0; ii0--) {
			if (plate.m_fhex->web[3] == NULL) {
				foundendleft = true;
				i0_cur = ii0;
				break;
			}
		}
		if (!foundendleft)
			return false;
		i0 = i0_cur;
		long next_i0 = i0_cur + 1;
		long i1_start = (i1 >= next_i0) ? i1 : next_i0;
		foundendleft = false;
		for (long i = i1_start; i < plate.m_nHex; i++) {
			if (plate.m_fhex->web[3] == NULL) {
				foundendleft = true;
				i1 = i;
				break;
			}
		}
		return foundendleft;
	}
	bool findWeaveStart(const s_hexPlate& plate0, const s_hexPlate& plate1, int weave_dir, long& i0, long& i1) {
		bool foundstart = false;
		/*do weave slanted from bottom to right*/
		int weave_i = weave_dir;
		int weave_op = Math::loop(weave_i + 3, 6);
		/*find first upper index that can match this weave*/
		s_fNode* overlapNd = (s_fNode*)plate1.m_fhex[i0].nodes[weave_i];
		for (long i = i1; i < 2; i++) {
			/*this should end way before m_nHex*/
			s_fNode* overlapNd1 = (s_fNode*)plate1.m_fhex[i1].nodes[weave_op];
			if (overlapNd->thislink == overlapNd1->thislink) {
				i1 = i;
				foundstart = true;
				break;
			}
		}
		overlapNd = (s_fNode*)plate1.m_fhex[i1].nodes[weave_op];
		for (long i = i0; i < 2; i++) {
			/*this should end way before m_nHex*/
			s_fNode* overlapNd0 = (s_fNode*)plate1.m_fhex[i0].nodes[weave_i];
			if (overlapNd->thislink == overlapNd0->thislink) {
				i0 = i;
				foundstart = true;
				break;
			}
		}
		return foundstart;
	}
	void weaveAdjRows(const s_hexPlate& plate0, s_hexPlate& plate1, int weave_dir, long& i0, long& i1) {
		int weave_op = Math::loop(weave_dir + 3, 6);
		long i_top = i1;
		long i_bot = i0;
		s_fNode* nextTopNd = NULL;
		s_fNode* nextBotNd = NULL;
		do {
			plate1.m_fhex[i_bot].web[weave_dir] = (s_bNode*)&(plate1.m_fhex[i_top]);
			plate1.m_fhex[i_top].web[weave_op] = (s_bNode*)&(plate1.m_fhex[i_bot]);
			nextTopNd = (s_fNode*)plate1.m_fhex[i_top].web[0];
			nextBotNd = (s_fNode*)plate1.m_fhex[i_bot].web[0];
			i1 = i_top;
			i0 = i_bot;
			i_top--;
			i_bot++;
		} while (nextBotNd != NULL && nextTopNd!=NULL);

	}
	long rotateCCK2(const s_hexPlate& plate0, long i, int web_strt) {
		long nd_i = -1;
		for (int i = 0; i < 3; i++) {
			int web_i = Math::loop(web_strt + i, 6);
			s_fNode* nextNd = (s_fNode*)plate0.m_fhex[i].web[web_i];
			if (nextNd != NULL) {
				s_fNode* nextNextNd = (s_fNode*)nextNd->web[web_i];
				if (nextNextNd != NULL) {
					nd_i = nextNextNd->thislink;
					break;
				}
			}
		}
		return nd_i;
	}
	long rotateCK2(const s_hexPlate& plate0, long i, int web_strt) {
		long nd_i = -1;
		for (int i = 0; i < 3; i++) {
			int web_i = Math::loop(web_strt - i, 6);
			s_fNode* nextNd = (s_fNode*)plate0.m_fhex[i].web[web_i];
			if (nextNd != NULL) {
				s_fNode* nextNextNd = (s_fNode*)nextNd->web[web_i];
				if (nextNextNd != NULL) {
					nd_i = nextNextNd->thislink;
					break;
				}
			}
		}
		return nd_i;
	}

	void genPlateWSameWeb(const s_hexPlate& plate0, s_hexPlate& plate1) {
		plate1.m_nHex = plate0.m_nHex;
		plate1.m_Rhex = plate0.m_Rhex;
		plate1.m_RShex = plate0.m_RShex;
		plate1.m_Shex = plate0.m_Shex;
		for (int i = 0; i < 6; i++)
			plate1.m_hexU[i] = plate0.m_hexU[i];
		plate1.m_fhex = new s_fNode[plate1.m_nHex];
		for (long i = 0; i < plate1.m_nHex; i++) {
			plate1.m_fhex[i].thislink = i;
			plate1.m_fhex[i].o = plate0.m_fhex[i].o;
			plate1.m_fhex[i].shex = NULL;
			plate1.m_fhex[i].x = plate0.m_fhex[i].x;
			plate1.m_fhex[i].y = plate0.m_fhex[i].y;

			plate1.m_fhex[i].nodes = NULL;
			plate1.m_fhex[i].w = NULL;
			plate1.m_fhex[i].N = 0;
			plate1.m_fhex[i].b = 0.f;

			plate1.m_fhex[i].Nweb = plate0.m_fhex[i].Nweb;
			plate1.m_fhex[i].web = new s_bNode * [plate1.m_fhex[i].Nweb];
			for (int j = 0; j < plate0.m_fhex[i].Nweb; j++) {
				/*the web must be the equivalent nodes in the new plate*/
				long webmaster_i = plate0.m_fhex[i].web[j]->thislink;
				plate1.m_fhex[i].web[j] = (s_bNode*)&(plate1.m_fhex[webmaster_i]);
			}
		}
	}
	void genLowerNodePtrsForPlate(s_hexPlate& plate, int N) {
		for (long i = 0; i < plate.m_nHex; i++) {
			plate.m_fhex[i].nodes = new s_bNode * [N];
			plate.m_fhex[i].w = new float[N];
			plate.m_fhex[i].N = N;
			for (int j = 0; j < N; j++) {
				plate.m_fhex[i].nodes[j] = NULL;
				plate.m_fhex[i].w[j] = 0.f;
			}
		}
	}
	void releasePlateWSameWeb(s_hexPlate& plate) {
		if (plate.m_fhex != NULL) {
			for (long i = 0; i < plate.m_nHex; i++) {
				if (plate.m_fhex[i].web != NULL) {
					delete[] plate.m_fhex[i].web;
				}
			}
			delete[] plate.m_fhex;
		}
		plate.m_fhex = NULL;
	}
	void releaseLowerNodePtrsForPlate(s_hexPlate& plate) {
		for (long i = 0; i < plate.m_nHex; i++) 
		{
			if (plate.m_fhex[i].nodes != NULL)
				delete[] plate.m_fhex[i].nodes;
			plate.m_fhex[i].nodes = NULL;
			if (plate.m_fhex[i].w != NULL)
				delete[] plate.m_fhex[i].w;
			plate.m_fhex[i].w = NULL;
			plate.m_fhex[i].N = 0;
		}
	}
}
