#pragma once
#ifndef STAMPEYE_H
#define STAMPEYE_H

#ifndef HEXEYE_H
#include "HexEye.h"
#endif
#ifndef PATTERNLUNA_H
#include "PatternLuna.h"
#endif

//#define STAMPEYE_DODEBUGIMG

#define STAMPEYEMAXANGRAD 3.1f
#define STAMPEYENUM 1000 /*6 * numAngDiv at least added 2 extra (72 +2)*/
#define STAMPEYEMAXNUM 20
#define STAMPEYENUMHOLEPATS 5/*number of pats for each curve/arc-corner with varying middles (holes cut out of middle)*/

const float stampeye_radincmul = 2.f;/*multiplicative factor that increases the circle radius each cycle*/
const float stampeye_openingAngDivisor = 2.f; /*divids PI to get opening angle*/

struct s_eyeStamp {
	/*multiple eyes are alowed to allow for stamp to shift around and still be considered same stamp*/
	s_hexEye* eyes[STAMPEYEMAXNUM];/*hex eyes that have been stamped, generally the eyes are NOT OWNED by the s_eyeStamp*/
	s_hexEye* mask;/*mask is zero where values are considered not to matter to the nnet */
	int n;/*number of eyes in stamp*/
	float o;/*target value for NNet*/

	int raw_eye_i[STAMPEYEMAXNUM];
	float ang[STAMPEYEMAXNUM];
	float center_ang[STAMPEYEMAXNUM];
	float opening_ang[STAMPEYEMAXNUM];
	float radius[STAMPEYEMAXNUM];
#ifdef STAMPEYE_DODEBUGIMG
	long img_dim;
	Img* imgs[STAMPEYEMAXNUM];
#endif
};

/*this function generates dummy patterns that the NNet nodes at level 2 (0,1,2) will be trained on */
/*after generation the lowest level (index 2) of the stamp contains the square blured image of the curve
  or other pattern
  the next level up (level 1) is currently  not set to anything but it is connected to the lower level
  the top level (level 0) is also connected but not set 
  */
/*
* The luna stamps are similar to the other stamps but are processed to include the results of evaluating the lowest
* level with luna patterns.
* luna stamps contain 2 levels and have a lowest radius of 2 * the base hex radius used by the other stamps at lowest level
* level 0 of the luna stamps doesn't contain anything but connects to the lower level
* level 1 of the luna stamps contain 7 hexes which correspond geometrically to level 1 of the other stamps
* below level 1 the s_fhex nodes (which are owned by these stamps) contain the results of evaluating the image
* on the lowest level of the other stamp with a luna pattern centered in each of the 7 level 1 hexes
* the number of nodes hanging from level 1 correspond to the number of luna patterns (usually 8)
* results of the pattern evaluation are stored in the o of the hanging nodes
*/
class StampEye : public Base {
public:
	StampEye();
	~StampEye();

	unsigned char init(
		PatternLuna* patLuna,
		HexBase* hexBase = NULL,
		int lowestStampLev = 2,
		float numAngDiv = 12.f,//36.f,//12.f,
		float numCircleRadii = 5.f,
		int smudgeNum = 1,/*number of smudges, along the r direction */
		int smudgeAngNum = 1,/*number of angle variants inside of each of the 12 main angles counted towards a single nnet trained node*/
		int finalOpeningAngs = 3, /*number of opening angles between opening ang and final backwards ang counting backwards ang*/
		float maxRadForFinalOpeningAngs_mul = 1.1f,
		float maskdim = 6.,/*mask dim as a multiple of the smallest r dim*/
		float r = 3.f,
		float thickness_in_2Runits = 2.f,
		float falloff_scale_factor_2Runits = 1.f,
		float gaussSigma_in_thicknessUnits = 1.f,
		float stampAng_falloff = PI / 3.f
	);
	void release();

	unsigned char spawn();
	unsigned char initNNets(HexEye* net);/*takes the hexEye and initializes it so that it goes with the luna eye stamp*/
	void          releaseNNets(HexEye* net);/*releases what is done in the above*/
	bool          setupForStampi(int i);/*sets up the stamps with the o's so that they are ready to be run with the i'th configuration selectedd*/

	inline HexEye* getLunaEyeGen() { return m_lunaEyeGen; }
	inline s_eyeStamp* getEyeStamps() { return m_stamps; }
	s_eyeStamp& getEyeStamp(int i) { return m_stamps[i]; }
	s_eyeStamp& getLunaEyeStamp(int i) { return m_lunaStamps[i]; }
	inline s_eyeStamp* getLunaEyeStamps() { return m_lunaStamps; }
	inline int numStamps() { return m_num_stamps; }
	inline int numEyeStamps() { return m_eyes_stamped; }/*returns raw index counting total number of eyes stamped in the hexEyeGen obj*/
	inline s_hexEye& getEyeRawIndex(int i) { return m_eyeGen->getEye(i); }
	inline s_hexEye* getLunaEye(int i_stamp, int i_sub_stamp) { return getLunaEyeStamp(i_stamp).eyes[i_sub_stamp]; }
	inline int getLowestStampLev() { return m_lowestStampLev; }

	int getStampI_fromRawIndex(int i);
	int getStampSubI_fromRawIndex(int i);
	bool getStampIs_fromRawIndex(int i, int& stamp_i_ret, int& sub_stamp_i_ret);
protected:
	float  m_numAngDiv;
	float  m_numCircleRadii;
	float  m_minCircleRadius;
	int    m_smudgeNum;/*number of division over which the corner is smudged*/
	int    m_smudgeAngNum;/*should be odd number of smudge off angle stamps that still count towards the same 12 division*/
	int    m_finalOpeningAngs;/*final decreases opening angs by these number of divisions from last opening ang to full backwards*/
	float  m_maskdim;/*max dim away from mask before values no longer matter for NNet*/
	float  m_maxRadForFinalOpeningAng;/*for circles larger than this the final set of opening angs narrowing to almost parallel is not computed*/
	float  m_minThickness;
	float  m_falloffScale;
	float m_gaussSigma;
	float m_stampAng_falloff;/*falloff for cos from stamp target =1 to stamp train target =0*/
	/*not owned*/
	PatternLuna* m_patternLuna;
	/*owned*/
	HexEye* m_eyeGen;
	HexEye* m_lunaEyeGen;
	s_eyeStamp m_stamps[STAMPEYENUM];/*lowest level of 3 total levels has square averaged o hexes corresponding to pattern*/
	s_eyeStamp m_lunaStamps[STAMPEYENUM]; /*these are the stamp eyes but with 2 levels, lowest nodes (beneath luna 2) are for the luna o values*/
	int   m_num_stamps;/*total number of stamps, or stamp packets, of eyes corresponding to the total number of nnet nodes that are being trained*/
	int   m_eyes_stamped;/*total number of eyes stamped combined over ALL stamps including the smudge stamps in the number*/
	int   m_lowestStampLev;

	s_2pt m_UBasis0;
	s_2pt m_UBasis1;
	s_2pt m_UrevBasis0;/*vectors that determine the rotation of the curent pattern*/
	s_2pt m_UrevBasis1;

	/*working scratch*/
	s_2pt m_circle_center;
	float m_circle_radius;
	s_2pt m_line_intersect;
	s_2pt m_circle_half_pt;/*point in the x direction that is the x coord of the line intercepts*/
	s_2pt m_Uline_perp1;/*points perp to line 1 into region beetween lines*/
	s_2pt m_Uline_perp2;/* "                  2         "                 */
	s_2pt m_UcenterIn;/*points inward from center of where lines would intersect, should be set to 1,0*/
	float m_thickness;
	bool  m_cosFalloff;
	bool  m_linearFalloff;
	bool  m_gaussFalloff;
	bool  m_sharpFalloff;


	void clearEyeStamps();
	/*utility*/
	void zeroStampS(s_eyeStamp& s);

	unsigned char stampFullNewMoons();
	unsigned char stampMoonEye(s_hexEye& seye, float o);
	unsigned char calcLunaStampEyes();/*find the */

	unsigned char stampRoundedCorners();
	bool stampRoundedCornersAtCenterAndAng(const s_2pt& center, float ang, float circle_scale, float opening_ang, int& stamp_cnt);
	bool stampRoundedCornerAtCenterAndAng(const s_2pt& center, float ang, float circle_scale, float opening_ang, int& stamp_cnt);
	bool stampHoleSetRoundedCornersAtCenterAndAng(const s_2pt& center, float ang, float circle_scale, float opening_ang, int& stamp_cnt);
	bool stampFinalCornerOpeningAngs(const s_2pt& center, float ang, float circle_scale, float opening_ang_start, int& stamp_cnt);

	unsigned char stampEyeRoundedCorner(s_hexEye& seye);
#ifdef STAMPEYE_DODEBUGIMG
	unsigned char RenderCornerImage(Img* img, s_hexEye& seye);
#endif
	float AveOverHexRoundedCorner(const s_hexPlate& eyeplate, const s_2pt& center);
#ifdef STAMPEYE_DODEBUGIMG
	unsigned char RenderPerHexCornerImage(Img* img, const s_hexPlate& eyeplate, const s_2pt& center);
	bool stampCoordToImgCoord(Img* img, const s_2pt& pt, s_2pt_i& img_pt);
#endif

	unsigned char setBasisFromAng(float ang);
	unsigned char setRoundedCorner(const s_2pt& center/*center of rounded middle, in middle of edge*/, float radius, float ang_rad);

	bool isInCircle(const s_2pt& pt) { return vecMath::inCircle(m_circle_radius, m_circle_center, pt); }
	bool isUnderLine(const s_2pt& pt, const s_2pt& Uline_perp);
	bool isInsideCurveHalf(const s_2pt& pt);/*is inside vs the center of the circle*/

	float distFromRoundedCorner(const s_2pt& pt);
	/*helpers to distFromRoundedCorner*/
	inline float distFromLine(const s_2pt& pt, const s_2pt& Uline_perp) { return vecMath::distLineGivenPerp(m_line_intersect, Uline_perp, pt); }
	float distFromClosestLine(const s_2pt& pt);
	inline float distFromCircle(const s_2pt& pt) { return -vecMath::distCircleArc(m_circle_radius, m_circle_center, pt); }

	bool isInRoundedCornerNoRot(const s_2pt& pt);
	float RoundedCornerIntensityNoRot(const s_2pt& pt);
	bool isInRoundedCorner(const s_2pt& pt, float& intensity);/*rotation right handed from x0 axis by angle rotAng in rad*/

	bool stampEyeIncOk(int stamp_cnt);

	bool setupForFullMoonStamp(int i);
	bool setupForLunaStamp(int i);
};

#endif
