#pragma once
#ifndef HEXEYE_H
#define HEXEYE_H
#ifndef HEXBASE_H
#include "HexBase.h"
#endif
#ifndef PATTERNNODE_H
#include "PatternNode.h"
#endif

#define HEXEYE_MAXLEVELS 10
#define HEXEYE_MAXEYES 100
#define HEXEYE_MAXNEBINDXS 6 

/*
struct s_hexLevel {
	float Rs;
	s_hex* hexes;
	int    n;
};*/
struct s_hexEye {
	s_hexPlate lev[HEXEYE_MAXLEVELS];
	//s_hexLevel lev[HEXEYE_MAXLEVELS];
	int n;/*starting from one*/
};

class HexEye : public Base{
public:
	HexEye();
	~HexEye();

	unsigned char init(HexBase* lowHexes);
	unsigned char init(float r, int NLevels);

	unsigned char spawn();

	void release();

	inline s_fNode* getNodes(int ieye, int ilevel) { return m_eye[ieye].lev[ilevel].m_fhex; }
	inline int getNHexes(int ieye, int ilevel) { return m_eye[ieye].lev[ilevel].m_nHex; }
	inline float getRSHex(int ieye, int ilevel) {return m_eye[ieye].lev[ilevel].m_RShex; }//Rs)*2.f/sqrtf(3.f); }
	inline int getMaxLevi(int ieye) { return m_eye[ieye].n - 1; }
	inline s_2pt* getUHex() { return m_hexU; }
	inline s_hexEye& getEye(int n) { return m_eye[n]; }
	inline s_hexEye* getEyePtr(int n) { return &(m_eye[n]); }
protected:
	float m_r;
	float m_rs;
	float m_R;
	float m_Rs;

	long m_imgWidth;
	long m_imgHeight;
	int  m_N_levels;/*curent number of levels being used by this spawn*/

	s_2pt m_hexU[6];/*unit vectors down center of sides*/

	s_hexEye m_eye[HEXEYE_MAXEYES];
	int m_N_eyes;

	void setHexUs();
	int setRfromImg();/*return number of levels*/
	unsigned char spawnEye();
	void releaseEyes();

	unsigned char initLevels();
	void          releaseLevels(s_hexEye& eye);
	unsigned char initWithNLevels(int NLevels);
	void initNode(s_fNode& h, int nd_i);
	unsigned char genPatternsForLevels(s_hexPlate* levels);
	int           collectNebIndexes(s_hexPlate& topLevel, int curTopIndex, s_2pt_i nebi[]);/*nebi has length of HEXEYE_MAXNEBINDXS */
	int           getNebLevIndex(s_2pt_i nebi[], int num_neb, int web_i);
	unsigned char genLowerPattern(s_fNode* lhxs, float Rs, s_2pt& loc, s_2pt_i nebi[], int num_neb, int& indx);
	unsigned char weaveRound(s_fNode& lhx);

	unsigned char rootOn(s_hexEye& eye, s_hexPlate& lowHexes, long center_i);/*assumes the eye does not go off screen*/
	int rotateCLK(s_fNode* pat_hex, int strt_i);
	int rotateCCLK(s_fNode* pat_hex, int strt_i);
	s_fNode* runLine(s_fNode* pat_hex, s_fNode* low_hex, int next_web_i);
	int turnCorner(s_fNode** pat_node, s_fNode** low_hex, int fwd_web_i, int rev_web_i);

	/*Util*/
	int foundInAr(s_2pt_i ar[], int n, int val);
};

#endif
