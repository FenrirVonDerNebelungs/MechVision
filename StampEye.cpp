#include "StampEye.h"
#include "PatternL1.h"
StampEye::StampEye() : m_numAngDiv(0.f), m_numCircleRadii(0.f), m_minCircleRadius(0.f), m_smudgeNum(0), m_smudgeAngNum(0), m_finalOpeningAngs(0), 
m_maskdim(0.), m_maxRadForFinalOpeningAng(0.f), 
m_minThickness(0.f),m_gaussSigma(0.f),
m_patternLuna(NULL), m_eyeGen(NULL), m_lunaEyeGen(NULL), 
m_num_stamps(0), m_eyes_stamped(0), m_lowestStampLev(0), m_circle_radius(0.f),
m_thickness(0.f),m_cosFalloff(false),m_linearFalloff(false),m_gaussFalloff(false),m_sharpFalloff(false)
{
	clearEyeStamps();

	m_UBasis0.x0 = 0.f;
	m_UBasis0.x1 = 0.f;
	m_UBasis1.x0 = 0.f;
	m_UBasis1.x1 = 0.f;
	m_UrevBasis0.x0 = 0.f;
	m_UrevBasis0.x1 = 0.f;
	m_UrevBasis1.x0 = 0.f;
	m_UrevBasis1.x1 = 0.f;


	m_circle_center.x0 = 0.f;
	m_circle_center.x1 = 0.f;
	m_line_intersect.x0 = 0.f;
	m_line_intersect.x1 = 0.f;
	m_circle_half_pt.x0 = 0.f;
	m_circle_half_pt.x1 = 0.f;
	m_Uline_perp1.x0 = 0.f;
	m_Uline_perp1.x1 = 0.f;
	m_Uline_perp2.x0 = 0.f;
	m_Uline_perp2.x1 = 0.f;
	m_UcenterIn.x0 = 0.f;
	m_UcenterIn.x1 = 0.f;
}
StampEye::~StampEye() {
	;
}
unsigned char StampEye::init(
	PatternLuna* patLuna,
	HexBase* hexBase,
	int lowestStampLev,
	float numAngDiv,
	float numCircleRadii,
	int smudgeNum,
	int smudgeAngNum,
	int finalOpeningAngs,
	float maxRadForFinalOpeningAngs_mul,
	float maskdim,
	float r,
	float thickness_in_2Runits,
	float gaussSigma_in_thicknessUnits
) {
	m_lowestStampLev = lowestStampLev;
	m_patternLuna = patLuna;
	m_num_stamps = 0;
	m_numAngDiv = numAngDiv;
	m_numCircleRadii = numCircleRadii;
	m_smudgeNum = smudgeNum;
	m_smudgeAngNum = smudgeAngNum;
	m_finalOpeningAngs = finalOpeningAngs;
	m_maskdim = maskdim;
	m_eyeGen = new HexEye;
	m_lunaEyeGen = new HexEye;

	float targr = (hexBase == NULL) ? r : 2.f*hexBase->getRhex();/*for hexEye radius overlap even at lowest level so 2 needed*/
	m_minCircleRadius = targr;/* because the hexes in the eye even at the lowest level overlap*/

	m_maxRadForFinalOpeningAng = maxRadForFinalOpeningAngs_mul*targr;

	m_minThickness = thickness_in_2Runits * targr;
	m_gaussSigma = gaussSigma_in_thicknessUnits * m_minThickness;
	m_thickness = m_minThickness;
	m_cosFalloff = false;
	m_linearFalloff = false;
	m_gaussFalloff = false;
	m_sharpFalloff = false;

	int numSmudgeArray = smudgeNum * smudgeAngNum *STAMPEYENUMHOLEPATS;
	int totalNumEyes = numSmudgeArray * STAMPEYENUM;
	if (totalNumEyes > HEXEYE_MAXEYES)
		return ECODE_FAIL;
	if (RetOk(m_eyeGen->init(targr, m_lowestStampLev+1))) {
		for (int i = 0; i < totalNumEyes; i++) {
			m_eyeGen->spawn();
		}
	}
	else return ECODE_FAIL;
	if (RetOk(m_lunaEyeGen->init(2.f*targr, m_lowestStampLev, PATTERNLUNA_NUM))) {/*patternluna_num means this generates 8 node pointers at the lowest level*/
		for (int i = 0; i < m_eyeGen->getNEyes(); i++) {
			m_lunaEyeGen->spawn();
			/*this is an unusual case where the lowest of nodes of the hexEyes will be owned by the eye*/
			s_hexEye& lastEye = m_lunaEyeGen->getLastEye();
			int maxLunLev = m_lowestStampLev - 1;
			for (int h_i = 0; h_i < lastEye.lev[maxLunLev].m_nHex; h_i++) {
				for (int l_i = 0; l_i < PATTERNLUNA_NUM; l_i++) {
					lastEye.lev[maxLunLev].m_fhex[h_i].nodes[l_i] = (s_bNode*)new s_fNode;
					lastEye.lev[maxLunLev].m_fhex[h_i].nodes[l_i]->zero();
					(lastEye.lev[maxLunLev].m_fhex[h_i].N)++;
				}
			}
		}
	}
	else return ECODE_FAIL;
#ifdef STAMPEYE_DODEBUGIMG
	/*calculate size of img*/
	/*1+x^1 + x^2 + x^3 +... = (1-x^(n-1))/(1-x)*/
	/*for level = N max radius = (1-2^N)/(1-2) = 2^N-1*/
	float multFac = Math::power(2.f, m_lowestStampLev + 1);
	float maxRad = targr * multFac; /*this is really targr/2 but add a factor of 2 padding*/
	for (int i = 0; i < STAMPEYENUM; i++) {
		m_stamps[i].img_dim = 2*(long)(ceilf(maxRad));
		for (int j = 0; j < STAMPEYEMAXNUM; j++)
			m_stamps[i].imgs[j] = new Img;
	}
#endif
	return ECODE_OK;
}
void StampEye::release() {
	int maxLunLev = m_lowestStampLev - 1;
#ifdef STAMPEYE_DODEBUGIMG
	for (int i = 0; i < STAMPEYENUM; i++) {
		for (int j = 0; j < STAMPEYEMAXNUM; j++) {
			if (m_stamps[i].imgs[j] != NULL) {
				m_stamps[i].imgs[j]->release();
				delete m_stamps[i].imgs[j];
				m_stamps[i].imgs[j] = NULL;
			}
		}
		m_stamps[i].img_dim = 0;
	}
#endif
 	if (m_lunaEyeGen != NULL) {
		/*this is an case where the node pointers of the lowest level of the eyes is actually owned by the hexEyes so this must be released before they are deleted*/
		for (int i = 0; i < m_lunaEyeGen->getNEyes(); i++) {
			s_hexEye* Eye = m_lunaEyeGen->getEyePtr(i);
			if (Eye != 0) {
				for (int h_i = 0; h_i < Eye->lev[maxLunLev].m_nHex; h_i++) {
					for (int l_i = 0; l_i < Eye->lev[maxLunLev].m_fhex[h_i].N; l_i++) {
						if (Eye->lev[maxLunLev].m_fhex[h_i].nodes[l_i] != NULL)
							delete Eye->lev[maxLunLev].m_fhex[h_i].nodes[l_i];
					}
				}
			}
		}

		m_lunaEyeGen->release();
		delete m_lunaEyeGen;
	}
	m_lunaEyeGen = NULL;
	if (m_eyeGen != NULL) {
		m_eyeGen->release();
		delete m_eyeGen;
	}
	m_eyeGen = NULL;
	m_smudgeNum = 0;
	m_numAngDiv = 0.f;
	m_eyes_stamped = 0;

	clearEyeStamps();
}
unsigned char StampEye::spawn() {
	stampFullNewMoons();
	stampRoundedCorners();
	calcLunaStampEyes();

	return ECODE_OK;
}
unsigned char StampEye::initNNets(HexEye* net) {
	/*must have initiated the luna stamps first*/
	if (m_num_stamps < 1)
		return ECODE_ABORT;
	float R = m_lunaStamps[0].eyes[0]->lev[m_lowestStampLev - 1].m_Rhex;
	if (RetOk(net->init(R, m_lowestStampLev, PATTERNLUNA_NUM))) {
		for(int i=0; i<m_num_stamps; i++)
			net->spawn();
	}else
		return ECODE_FAIL;
	return ECODE_OK;
}
void StampEye::releaseNNets(HexEye* net) {
	if (net != NULL) {
		net->release();
	}
}
bool StampEye::setupForStampi(int i) {
	if (i < 0 || i >= m_num_stamps)
		return false;
	for (int j = 0; j < m_num_stamps; j++) {
		m_lunaStamps[j].o = 0.f;
	}
	m_lunaStamps[i].o = 1.f;
	return true;
}

void StampEye::clearEyeStamps() {
	for (int i = 0; i < STAMPEYENUM; i++) {
		zeroStampS(m_lunaStamps[i]);
		zeroStampS(m_stamps[i]);
	}
	m_num_stamps = 0;
	m_eyes_stamped = 0;
}
void StampEye::zeroStampS(s_eyeStamp& s) {
	for (int j = 0; j < STAMPEYEMAXNUM; j++) {
		s.eyes[j] = NULL;
		s.raw_eye_i[j] = 0;
		s.ang[j] = 0.f;
		s.center_ang[j] = 0.f;
		s.opening_ang[j] = 0.f;
		s.radius[j] = 0.f;
#ifdef STAMPEYE_DODEBUGIMG
		s.imgs[j] = NULL;
#endif
	}
	s.mask = NULL;
	s.n = 0;
}

int StampEye::getStampI_fromRawIndex(int i) {
	int stamp_i, sub_stamp_i;
	if (!getStampIs_fromRawIndex(i, stamp_i, sub_stamp_i))
		return -1;
	return sub_stamp_i;
}
int StampEye::getStampSubI_fromRawIndex(int i) {
	int stamp_i, sub_stamp_i;
	if (!getStampIs_fromRawIndex(i, stamp_i, sub_stamp_i))
		return -1;
	return sub_stamp_i;
}
bool StampEye::getStampIs_fromRawIndex(int i, int& stamp_i_ret, int& sub_stamp_i_ret) {
	for (int stamp_i = 0; stamp_i < m_num_stamps; stamp_i++) {
		for (int sub_stamp_i=0; sub_stamp_i < m_stamps[stamp_i].n; sub_stamp_i++)
			if (m_stamps[stamp_i].raw_eye_i[sub_stamp_i] == i) {
				stamp_i_ret = stamp_i;
				sub_stamp_i_ret = sub_stamp_i;
				return true;
			}
	}
	return false;
}

unsigned char StampEye::calcLunaStampEyes() {
	int maxLunLev = m_lowestStampLev - 1;
	int lunEyeRawCnt = 0;
	for (int s_i = 0; s_i < m_num_stamps; s_i++) {
		for (int e_i = 0; e_i < m_stamps[s_i].n; e_i++) {
			s_hexEye* curStampEye = m_stamps[s_i].eyes[e_i];
			s_hexEye* curLunaStampEye = m_lunaEyeGen->getEyePtr(lunEyeRawCnt);/*lunaEyeGen acts as a resevor for the eye stamps, it is a hexEye type object*/
			m_lunaStamps[s_i].eyes[e_i]=curLunaStampEye;/*transfer the eye pointer to the object that is actually used to reference it*/
			/*loop over the lowest level of the lunaeyestamps and the 2nd lowest of eyestamps to fill luna value for each hex in the luna eye stamp*/
			/*this curLunaStampEye->lev[m_lowestStampLev].m_nHex == curStampEye->lev[m_lowestStampLev].m_nHex should be true*/
			for (int l_i = 0; l_i < curLunaStampEye->lev[maxLunLev].m_nHex; l_i++) {
				/*the nodes hanging from lev 1of the stamp eye are already generated and owned by the stamp eye
				  the number of these nodes corresponds to the number of luna's */
				s_fNode& curStampNode = (curStampEye->lev[maxLunLev].m_fhex[l_i]);
				for (int lun_i = 0; lun_i < curLunaStampEye->lev[maxLunLev].m_fhex[l_i].N; lun_i++) {
					s_fNode& lunaPatNode = *(m_patternLuna->getPatNode(lun_i));
					s_fNode* curLunaStampNode = (s_fNode*)(curLunaStampEye->lev[maxLunLev].m_fhex[l_i].nodes[lun_i]);
					curLunaStampEye->lev[maxLunLev].m_fhex[l_i].o = 0.f;
					PatternLunaThreaded::evalLowerNode(lunaPatNode, curStampNode);/*this fills the o of the curStampNode so that it returns the luna value from evaluating in the center of the hex*/
					curLunaStampNode->o = lunaPatNode.o;
				}
			}
			lunEyeRawCnt++;
		}
	}
	return ECODE_OK;
}
unsigned char StampEye::stampFullNewMoons() {
	stampMoonEye(m_eyeGen->getEye(m_eyes_stamped), 0.f);
	m_stamps[m_num_stamps].eyes[0] = m_eyeGen->getEyePtr(m_eyes_stamped);
	m_stamps[m_num_stamps].n = 1;
	m_stamps[m_num_stamps].raw_eye_i[0] = m_eyes_stamped;
#ifdef STAMPEYE_DODEBUGIMG
	m_stamps[m_num_stamps].imgs[0]->init(m_stamps[m_num_stamps].img_dim, m_stamps[m_num_stamps].img_dim, 3L);
#endif
	m_eyes_stamped++;
	m_num_stamps++;
	stampMoonEye(m_eyeGen->getEye(m_eyes_stamped), 1.f);
	m_stamps[m_num_stamps].eyes[0] = m_eyeGen->getEyePtr(m_eyes_stamped);
	m_stamps[m_num_stamps].n = 1;
	m_stamps[m_num_stamps].raw_eye_i[0] = m_eyes_stamped;
#ifdef STAMPEYE_DODEBUGIMG
	m_stamps[m_num_stamps].imgs[0]->init(m_stamps[m_num_stamps].img_dim, m_stamps[m_num_stamps].img_dim, 3L);
#endif
	m_eyes_stamped++;
	m_num_stamps++;

	return ECODE_OK;
}
unsigned char StampEye::stampMoonEye(s_hexEye& seye, float o) {
	int lowestN = seye.n - 1;
	s_hexPlate& eyeplate = seye.lev[lowestN];
	for (int i = 0; i < eyeplate.m_nHex; i++) {
		eyeplate.m_fhex[i].o = o;
	}
	return ECODE_OK;
}
unsigned char StampEye::stampRoundedCorners() {
	s_2pt center = { 0.f, 0.f };/*this is not scanned over*/
	/*run over each stamp which contains one smudged set*/
	/*sets depend on angle and scale of circle creating individual stamps*/
	float DAng = 2.f * PI / m_numAngDiv;
	int n_ang = (int)floorf(m_numAngDiv);
	float cur_ang = 0.f;

	int n_circleRadii = (int)floorf(m_numCircleRadii);
	int stamp_cnt = m_num_stamps;
	for (int i_ang = 0; i_ang < n_ang; i_ang++) {
		float cur_circleRadius = m_minCircleRadius;
		for (int i_rad = 0; i_rad < n_circleRadii; i_rad++) {
			/*increaments stamp count appropriately*/
			if (!stampRoundedCornersAtCenterAndAng(center, cur_ang, cur_circleRadius, PI / stampeye_openingAngDivisor, stamp_cnt))
				break;
			if (!stampFinalCornerOpeningAngs(center, cur_ang, cur_circleRadius, PI / stampeye_openingAngDivisor, stamp_cnt))
				break;
			cur_circleRadius *= stampeye_radincmul;
		}
		if (!stampEyeIncOk(stamp_cnt))
			break;
		cur_ang += DAng;
	}
	return ECODE_OK;
}
bool StampEye::stampRoundedCornersAtCenterAndAng(const s_2pt& center, float ang, float circle_scale, float opening_ang, int& stamp_cnt) {
	float r_scale = m_eyeGen->getRSHex(0, m_eyeGen->getMaxLevi(0));
	r_scale *= 2.f;
	float R = 2.f * r_scale / sqrtf(3);
	s_2pt* hexU = m_eyeGen->getUHex();
	s_2pt smudge_center = { 0.f, 0.f };
	m_stamps[stamp_cnt].n = 0;
	stampHoleSetRoundedCornersAtCenterAndAng(smudge_center, ang, circle_scale, opening_ang, stamp_cnt);
	stamp_cnt++;
	m_num_stamps++;
	if (!stampEyeIncOk(m_num_stamps))
		return false;
	return true;
}
bool StampEye::stampHoleSetRoundedCornersAtCenterAndAng(const s_2pt& center, float ang, float circle_scale, float opening_ang, int& stamp_cnt) {
	m_cosFalloff = false;
	m_linearFalloff = false;
	m_gaussFalloff = false;
	m_sharpFalloff = false;
	//if (hexMath::inHex(R, r_scale, hexU, smudge_center, center)) 
	stampRoundedCornerAtCenterAndAng(center, ang, circle_scale, opening_ang, stamp_cnt);
	m_linearFalloff = true;
	stampRoundedCornerAtCenterAndAng(center, ang, circle_scale, opening_ang, stamp_cnt);
	m_linearFalloff = false;
	m_cosFalloff = true;
	stampRoundedCornerAtCenterAndAng(center, ang, circle_scale, opening_ang, stamp_cnt);
	m_cosFalloff = false;
	m_gaussFalloff = true;
	stampRoundedCornerAtCenterAndAng(center, ang, circle_scale, opening_ang, stamp_cnt);
	m_gaussFalloff = false;
	m_sharpFalloff = true;
	stampRoundedCornerAtCenterAndAng(center, ang, circle_scale, opening_ang, stamp_cnt);
	return true;
}
bool StampEye::stampRoundedCornerAtCenterAndAng(const s_2pt& center, float ang, float circle_scale, float opening_ang, int& stamp_cnt) {
#ifdef STAMPEYE_DODEBUGIMG
	m_stamps[stamp_cnt].imgs[m_stamps[stamp_cnt].n]->init(m_stamps[stamp_cnt].img_dim, m_stamps[stamp_cnt].img_dim, 3L);
	m_stamps[stamp_cnt].imgs[m_stamps[stamp_cnt].n]->clearToChar(0x00);
#endif
	setBasisFromAng(ang);
	setRoundedCorner(center, circle_scale, opening_ang);
	stampEyeRoundedCorner(m_eyeGen->getEye(m_eyes_stamped));
#ifdef STAMPEYE_DODEBUGIMG
	RenderCornerImage(m_stamps[stamp_cnt].imgs[m_stamps[stamp_cnt].n], m_eyeGen->getEye(m_eyes_stamped));
#endif
	m_stamps[stamp_cnt].eyes[m_stamps[stamp_cnt].n] = m_eyeGen->getEyePtr(m_eyes_stamped);
	m_stamps[stamp_cnt].raw_eye_i[m_stamps[stamp_cnt].n] = m_eyes_stamped;
	m_stamps[stamp_cnt].ang[m_stamps[stamp_cnt].n] = ang;
	m_stamps[stamp_cnt].center_ang[m_stamps[stamp_cnt].n] = ang;
	m_stamps[stamp_cnt].opening_ang[m_stamps[stamp_cnt].n] = opening_ang;
	m_stamps[stamp_cnt].radius[m_stamps[stamp_cnt].n] = circle_scale;

	(m_stamps[stamp_cnt].n)++;
	m_eyes_stamped++;
	return true;
}
bool StampEye::stampFinalCornerOpeningAngs(const s_2pt& center, float ang, float circle_scale, float opening_ang_start, int& stamp_cnt) {
	if (circle_scale > m_maxRadForFinalOpeningAng)
		return true;
	float r_scale = m_eyeGen->getRSHex(0, m_eyeGen->getMaxLevi(0));
	r_scale *= 2.f;
	float R = 2.f * r_scale / sqrtf(3);
	s_2pt* hexU = m_eyeGen->getUHex();
	s_2pt smudge_center = { 0.f, 0.f };
	float opening_ang_end = STAMPEYEMAXANGRAD;

	float dAng = (opening_ang_end-opening_ang_start) / ((float)m_finalOpeningAngs);
	m_stamps[stamp_cnt].n = 0;
	for (int i = m_finalOpeningAngs-1; i >=0; i--) {
		float opening_ang = opening_ang_end - ((float)i) * dAng;
		//if (hexMath::inHex(R, r_scale, hexU, smudge_center, center)) smudge center is at 0,0 same as center
		stampHoleSetRoundedCornersAtCenterAndAng(smudge_center, ang, circle_scale, opening_ang, stamp_cnt);
	}	
	stamp_cnt++;
	m_num_stamps++;
	if (!stampEyeIncOk(m_num_stamps))
		return false;
	return true;
}
unsigned char StampEye::stampEyeRoundedCorner(s_hexEye& seye) {
	int lowestN = seye.n - 1;
	s_hexPlate & eyeplate = seye.lev[lowestN];
	s_2pt xy;
	for (int i = 0; i < eyeplate.m_nHex; i++) {
		xy.x0 = eyeplate.m_fhex[i].x;
		xy.x1 = eyeplate.m_fhex[i].y;
		float val = AveOverHexRoundedCorner(eyeplate, xy);
		eyeplate.m_fhex[i].o = val;
	}
	return ECODE_OK;
}

#ifdef STAMPEYE_DODEBUGIMG
unsigned char StampEye::RenderCornerImage(Img* img, s_hexEye& seye) {
	int lowestN = seye.n - 1;
	s_hexPlate& eyeplate = seye.lev[lowestN];
	s_2pt xy;
	for (int i = 0; i < eyeplate.m_nHex; i++) {
		xy.x0 = eyeplate.m_fhex[i].x;
		xy.x1 = eyeplate.m_fhex[i].y;
		RenderPerHexCornerImage(img, eyeplate, xy);
	}
	return ECODE_OK;
}
#endif
float StampEye::AveOverHexRoundedCorner(const s_hexPlate& eyeplate, const s_2pt& center) {
	float R = eyeplate.m_Rhex;
	float RS = eyeplate.m_RShex;
	/*the eye has overlaps but the base hex level does not, so reduce radius*/
	R /= 2.f;
	RS /= 2.f;
	s_2pt* hexU = m_eyeGen->getUHex();
	const s_2pt startPt = { center.x0 - R, center.x1 - R };
	/*inc will be 1*/
	long WH = (long)ceilf(2.f * R);
	s_2pt pt = { 0.f, 0.f };
	float av = 0.f;
	float tot = 0.f;
	for (int j = 0; j < WH; j++) {
		for (int i = 0; i < WH; i++) {
			pt.x0 = startPt.x0 + (float)i;
			pt.x1 = startPt.x1 + (float)j;
			if (hexMath::inHex(R, RS, hexU, center, pt)) {
				tot += 1.f;
				float intensity = 1.f;
				if (isInRoundedCorner(pt, intensity))
					av += intensity;
			}
		}
	}
	return av / tot;
}
#ifdef STAMPEYE_DODEBUGIMG
unsigned char StampEye::RenderPerHexCornerImage(Img* img, const s_hexPlate& eyeplate, const s_2pt& center) {
	s_rgb img_set_col = { 0xFF, 0xFF, 0xFF };
	float R = eyeplate.m_Rhex;
	float RS = eyeplate.m_RShex;
	/*the eye has overlaps but the base hex level does not, so reduce radius*/
	R /= 2.f;
	RS /= 2.f;
	s_2pt* hexU = m_eyeGen->getUHex();
	const s_2pt startPt = { center.x0 - R, center.x1 - R };
	/*inc will be 1*/
	long WH = (long)ceilf(2.f * R);
	s_2pt pt = { 0.f, 0.f };
	for (int j = 0; j < WH; j++) {
		for (int i = 0; i < WH; i++) {
			pt.x0 = startPt.x0 + (float)i;
			pt.x1 = startPt.x1 + (float)j;
			if (hexMath::inHex(R, RS, hexU, center, pt)) {
				float intensity=0;
				if (isInRoundedCorner(pt,intensity)) {
					s_2pt_i img_pt = { 0, 0 };
					s_rgb curCol = imgMath::mulIntensity(img_set_col, intensity);
					if (stampCoordToImgCoord(img, pt, img_pt)) {
						img->SetRGB(img_pt.x0, img_pt.x1, curCol);
					}
				}
			}
		}
	}
	return ECODE_OK;
}
bool StampEye::stampCoordToImgCoord(Img* img, const s_2pt& pt, s_2pt_i& img_pt) {
	float img_mid_calc = (float)img->getWidth();
	img_mid_calc /= 2.f;
	long img_mid = (long)floorf(img_mid_calc);
	img_pt.x0 = (long)floorf(pt.x0);
	img_pt.x1 = (long)floorf(pt.x1);
	img_pt.x0 += img_mid;
	img_pt.x1 += img_mid;
	if (img_pt.x0 < 0 || img_pt.x1 < 0)
		return false;
	if (img_pt.x1 >= img->getHeight() || img_pt.x0 >= img->getWidth())
		return false;
	return true;
}
#endif
unsigned char StampEye::setBasisFromAng(float ang) {
	vecMath::setBasis(ang, m_UBasis0, m_UBasis1);
	vecMath::revBasis(m_UBasis0, m_UBasis1, m_UrevBasis0, m_UrevBasis1);
	return ECODE_OK;
}
unsigned char StampEye::setRoundedCorner(const s_2pt& center, float radius, float ang_rad) {
	if (ang_rad > STAMPEYEMAXANGRAD)
		return ECODE_ABORT;
	/*assumes that the m_UcenterIn points along the x direction*/
	m_circle_radius = radius;
	m_UcenterIn.x0 = -1.f;
	m_UcenterIn.x1 = 0.f;
	s_2pt offset = vecMath::mul(radius, m_UcenterIn);
	m_circle_center = vecMath::add(center, offset);
	float halfAng = ang_rad / 2.f;
	s_2pt l1 = { cosf(halfAng), sinf(halfAng) };
	if (l1.x0 <= 0.f)
		return ECODE_FAIL;
	s_2pt l2 = { l1.x0, -l1.x1 };
	s_2pt circle_half_pt = { radius * l1.x0, 0.f };/*l1.x0=l2.x0 this is the point on the center line between the enclosing line intercepts*/
	m_circle_half_pt = vecMath::add(circle_half_pt, offset);/*correct since coord system is centered at circle edge*/
	m_Uline_perp1.x0 = -l1.x0;/*these are the lines perp to the enclosing lines*/
	m_Uline_perp1.x1 = -l1.x1;
	m_Uline_perp2.x0 = -l2.x0; 
	m_Uline_perp2.x1 = -l2.x1;
	float Ltanvec = radius * l1.x1 / l1.x0;
	float OutToPt = radius * l1.x0 + Ltanvec * l1.x1;
	m_line_intersect.x1 = center.x1;
	m_line_intersect.x0 = center.x0 + OutToPt-radius;
	return ECODE_OK;
}
bool StampEye::isUnderLine(const s_2pt& pt, const s_2pt& Uline_perp) {
	s_2pt VtoPt = vecMath::v12(m_line_intersect, pt);
	return vecMath::dot(VtoPt, Uline_perp) >= 0;
}
bool StampEye::isInsideCurveHalf(const s_2pt& pt) {
	s_2pt VtoPt = vecMath::v12(m_circle_half_pt, pt);
	return vecMath::dot(VtoPt, m_UcenterIn) >= 0;
}
float StampEye::distFromRoundedCorner(const s_2pt& pt) {
	float dist = 0.f;
	if (isInsideCurveHalf(pt)) {
		dist = distFromClosestLine(pt);
	}
	else
		dist = distFromCircle(pt);
	return dist;
}
float StampEye::distFromClosestLine(const s_2pt& pt) {
	float dist1 = distFromLine(pt, m_Uline_perp1);
	float dist2 = distFromLine(pt, m_Uline_perp2);
	return (dist1 >= dist2) ? dist2 : dist1;
}
bool StampEye::isInRoundedCornerNoRot(const s_2pt& pt) {
	bool isInLines = isUnderLine(pt, m_Uline_perp1) && isUnderLine(pt, m_Uline_perp2);
	if (!isInLines)
		return false;
	if (isInsideCurveHalf(pt))
		return true;
	return isInCircle(pt);
}
float StampEye::RoundedCornerIntensityNoRot(const s_2pt& pt) {
	float dist = distFromRoundedCorner(pt);
	if (dist < m_thickness)
		return 1.f;
	float in_lin=1.f;
	float in_cos=1.f;
	float in_gau=1.f;
	float in_line = 1.f;
	if (m_linearFalloff) {
		in_lin = 1.f - (dist-m_thickness) / m_circle_radius;
		if (in_lin < 0.f)
			in_lin = 0.f;
		if (in_lin > 1.f)
			in_lin = 1.f;
	}
	if (m_cosFalloff) {
		float cmet = (PI / 2.f) * (dist / m_circle_radius);
		in_cos = cosf(cmet);
		if (in_cos < 0.f)
			in_cos = 0.f;
	}
	if (m_gaussFalloff) {
		float gmet = dist / m_circle_radius;
		in_gau = Math::GaussianOneMax(gmet, m_gaussSigma/m_circle_radius);
	}
	if (m_sharpFalloff) {
		if (dist > m_thickness)
			in_line = 0.f;
	}
	return in_lin * in_cos * in_gau*in_line;
}
bool StampEye::isInRoundedCorner(const s_2pt& pt, float& intensity) {
	s_2pt convPt = vecMath::convBasis(m_UrevBasis0, m_UrevBasis1, pt);
	intensity = RoundedCornerIntensityNoRot(pt);
	return isInRoundedCornerNoRot(convPt);
}

bool StampEye::stampEyeIncOk(int stamp_cnt) {
	return stamp_cnt < STAMPEYENUM;
}