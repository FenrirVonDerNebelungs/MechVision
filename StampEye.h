#pragma once
#ifndef STAMPEYE_H
#define STAMPEYE_H

#ifndef HEXEYE_H
#include "HexEye.h"
#endif

#define STAMPEYEMINANGRAD 0.001f
#define STAMPEYENUM 74 /*6 * numAngDiv at least added 2 extra (72 +2)*/
#define STAMPEYEMAXNUM 100
#define STAMPEYL0WNUM 7

struct s_eyeStamp {
	/*multiple eyes are alowed to allow for stamp to shift around and still be considered same stamp*/
	s_hexEye* eyes[STAMPEYEMAXNUM];/*hex eyes that have been stamped, generally the eyes are NOT OWNED by the s_eyeStamp*/
	int n;/*number of eyes in stamp*/
	float o;/*target value for NNet*/
};

/*this function generates dummy patterns that the NNet nodes at level 2 (0,1,2) will be trained on */
/*after generation the lowest level (index 2) of the stamp contains the square blured image of the curve
  or other pattern
  the next level up (level 1) is currently  not set to anything but it is connected to the lower level
  the top level (level 0) is also connected but not set 
  */
class StampEye : public Base {
public:
	StampEye();
	~StampEye();

	unsigned char init(
		float numAngDiv = 12.f,
		int smudgeNum = 10,
		float r=3.f,
		HexBase* hexBase = NULL
	);
	void release();

	unsigned char spawn();

	inline s_eyeStamp* getEyeStamps() { return m_stamps; }
	inline int numEyeStamps() { return m_eyes_stamped; }
protected:
	float  m_numAngDiv;
	int    m_smudgeNum;/*number of division over which the corner is smudged*/
	/*owned*/
	HexEye* m_eyeGen;
	s_eyeStamp m_stamps[STAMPEYENUM];
	int   m_eyes_stamped;

	s_2pt m_circle_center;
	float m_circle_radius;
	s_2pt m_line_intersect;
	s_2pt m_Uline_perp1;/*points perp to line 1 into region beetween lines*/
	s_2pt m_Uline_perp2;
	s_2pt m_UcenterIn;/*points inward from center of where lines would intersect, should be set to 1,0*/

	s_2pt m_UBasis0;
	s_2pt m_UBasis1;
	s_2pt m_UrevBasis0;/*vectors that determine the rotation of the curent pattern*/
	s_2pt m_UrevBasis1;

	void clearEyeStamps();

	unsigned char stampFullNewMoons();
	unsigned char stampMoonEye(s_hexEye& seye, float o);
	unsigned char calcL1();

	unsigned char stampRoundedCorners();
	int stampRoundedCornersAtCenter(const s_2pt& corner_center, int eye_cnt, s_eyeStamp stamp[]);
	unsigned char stampEyeRoundedCorner(s_hexEye& seye);
	float AveOverHexRoundedCorner(const s_hexPlate& eyeplate, const s_2pt& center);

	unsigned char setBasisFromAng(float ang);
	unsigned char setRoundedCorner(const s_2pt& center/*center of rounded middle, in middle of edge*/, float radius, float ang_rad);

	bool isInCircle(const s_2pt& pt) { return vecMath::inCircle(m_circle_radius, m_circle_center, pt); }
	bool isUnderLine(const s_2pt& pt, const s_2pt& Uline_perp);
	bool isInsideCurveHalf(const s_2pt& pt);/*is inside vs the center of the circle*/

	bool isInRoundedCornerNoRot(const s_2pt& pt);
	bool isInRoundedCorner(const s_2pt& pt);/*rotation right handed from x0 axis by angle rotAng in rad*/

	bool incStampEyeNs(int start_i);
};

#endif
