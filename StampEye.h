#pragma once
#ifndef STAMPEYE_H
#define STAMPEYE_H

#ifndef HEXEYE_H
#include "HexEye.h"
#endif

#define STAMPEYEMINANGRAD 0.001f
#define STAMPEYENUM 10
#define STAMPEYEMAXNUM 100

struct s_eyeStamp {
	/*multiple eyes are alowed to allow for stamp to shift around and still be considered same stamp*/
	s_hexEye* eyes[STAMPEYEMAXNUM];/*hex eyes that have been stamped*/
	int n;/*number of eyes in stamp*/
};

/*this function generates dummy patterns that the NNet nodes at level 2 (0,1,2) will be trained on */
class StampEye : public Base {
public:
	StampEye();
	~StampEye();

	unsigned char spawn();

protected:
	float  m_numAngDiv;
	int    m_smudgeNum;/*number of division over which the corner is smudged*/
	/*owned*/
	HexEye* m_eyeGen;
	s_eyeStamp m_stamps[STAMPEYENUM];

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

	bool incStampEyeNs();
};

#endif
