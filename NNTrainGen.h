#pragma once
#ifndef NNTRAINGEN_H
#define NNTRAINGEN_H
#ifndef IMG_H
#include "Img.h"
#endif

struct s_trainImg {
	Img* img;
	long x_center;
	long y_center;
	float offset;/*magnitude of the offset*/
	float ang;
	float rCurve;
};

class NNTrainGen {
public:
	NNTrainGen();
	~NNTrainGen();
	unsigned char init(
		Img* baseImg=NULL, 
		float center_x=48.f, 
		float center_y=33.f,
		float targetWidth=20.f, 
		float targetLen=30.f, 
		float dpix=0.f, 
		int nShifts=1, 
		int nAngVals=1
	);
	void release();

	virtual unsigned char run();
	inline Img* getTrainImg(int i) { return m_trainImgs[i].img; }
protected:
	Img* m_baseImg;/*not owned*/
	s_rgba m_col;
	s_rgba m_bakcol;

	s_trainImg* m_trainImgs;
	int m_ntrainImgs;

	s_2pt m_center;
	float m_targetWidth;
	float m_targetLen;
	float m_dpix;/*4.f*/
	int m_nShifts;/*4*/
	int m_nAngVals;/*24*/

	int m_blockSize;/*for each scan over angles and shifts how many images are in this scan*/

	unsigned char draw();
	unsigned char draw2block();
	virtual unsigned char drawPos(s_2pt& offset, float ang, float width, float len, int curArray_i) { return ECODE_FAIL; }

	unsigned char findBoxBoundsFromArray(s_2pt bxs[], int n, s_2pt& boxBL, s_2pt& boxUR);
	void forceInImg(s_2pt_i& pt);
};
class NNTrainGenLine : public NNTrainGen {
public:
	NNTrainGenLine();
	~NNTrainGenLine();
	unsigned char init(
		Img* baseImg = NULL,
		float center_x = 48.f,
		float center_y = 33.f,
		float targetWidth = 20.f,
		float targetLen = 30.f,
		float dpix = 4.f,
		int nShifts = 4,
		int nAngVals = 24
	);
	unsigned char run();
protected:
	unsigned char drawPos(s_2pt& offset, float ang, float width, float len, int curArray_i);

	unsigned char drawStraightLine(s_2pt& offset, float ang, float width, float len, s_trainImg& simg);
	unsigned char findBoxBounds(s_2pt& box1, s_2pt& box2, s_2pt& boxBL, s_2pt& boxUR);
	bool isInLine(s_2pt& center, s_2pt& slope, float width, long i_x, long i_y);
};
class NNTrainGenArc : public NNTrainGen {
public:
	NNTrainGenArc();
	~NNTrainGenArc();
	unsigned char init(
		Img* baseImg = NULL,
		float center_x = 48.f,
		float center_y = 33.f,
		float targetWidth = 20.f,
		float targetLen = 30.f,
		float dpix = 4.f,
		int nShifts = 4,
		int nAngVals = 24,
		float minRCurve = 5.f,
		float minArc = 0.261799f,
		float drCurve = 5.f,
		float dArc = 0.261799f,
		int nCurveVals = 4,
		int nArcVales = 3
	);
	unsigned char run();
protected:
	float m_minRCurve;
	float m_minArc;
	float m_drCurve;
	float m_dArc;
	int m_nCurveVals;
	int m_nArcVals;

	unsigned char drawPos(s_2pt& offset, float ang, float width, float len, int curArray_i);

	unsigned char drawLineArc(s_2pt& offset, float ang, float width, float len, float rCurve, float cornerAng, s_trainImg& simg);
	unsigned char findBoxBounds(s_2pt& box1, s_2pt& box2, s_2pt& box3, s_2pt& box4, s_2pt& box5, s_2pt& boxBL, s_2pt& boxUR);
	bool isInArc(s_2pt& center, float innerR, float outerR, long i_x, long i_y);
	bool isInTrap(s_2pt& PerpIn, s_2pt& pt_r, s_2pt& Perp_r, s_2pt& pt_l, s_2pt& Perp_l, long i_x, long i_y);
};
#endif





