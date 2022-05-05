#include "StampEye.h"
#include "PatternL1.h"
StampEye::StampEye() : m_numAngDiv(0.f), m_numCircleRadii(0.f), m_minCircleRadius(0.f), m_smudgeNum(0), m_smudgeAngNum(0), m_maskdim(0.), m_patternLuna(NULL), m_eyeGen(NULL), m_lunaEyeGen(NULL), m_num_stamps(0), m_eyes_stamped(0), m_lowestStampLev(0), m_circle_radius(0.f) {
	clearEyeStamps();
	m_circle_center.x0 = 0.f;
	m_circle_center.x1 = 0.f;
	m_line_intersect.x0 = 0.f;
	m_line_intersect.x1 = 0.f;
	m_Uline_perp1.x0 = 0.f;
	m_Uline_perp1.x1 = 0.f;
	m_Uline_perp2.x0 = 0.f;
	m_Uline_perp2.x1 = 0.f;
	m_UcenterIn.x0 = 0.f;
	m_UcenterIn.x1 = 0.f;
	m_UBasis0.x0 = 0.f;
	m_UBasis0.x1 = 0.f;
	m_UBasis1.x0 = 0.f;
	m_UBasis1.x1 = 0.f;
	m_UrevBasis0.x0 = 0.f;
	m_UrevBasis0.x1 = 0.f;
	m_UrevBasis1.x0 = 0.f;
	m_UrevBasis1.x1 = 0.f;
}
StampEye::~StampEye() {
	;
}
unsigned char StampEye::init(
	PatternLuna* patLuna,
	int lowestStampLev,
	float numAngDiv,
	float numCircleRadii,
	int smudgeNum,
	int smudgeAngNum,
	float maskdim,
	float r,
	HexBase* hexBase
) {
	m_lowestStampLev = lowestStampLev;
	m_patternLuna = patLuna;
	m_num_stamps = 0;
	m_numAngDiv = numAngDiv;
	m_numCircleRadii = numCircleRadii;
	m_smudgeNum = smudgeNum;
	m_smudgeAngNum = smudgeAngNum;
	m_maskdim = maskdim;
	m_eyeGen = new HexEye;
	m_lunaEyeGen = new HexEye;

	float targr = (hexBase == NULL) ? r : hexBase->getRhex();
	m_minCircleRadius = 2.f * targr;/*2 because the hexes in the eye even at the lowest level overlap*/

	int numSmudgeArray = smudgeNum * smudgeAngNum;
	int totalNumEyes = numSmudgeArray * STAMPEYENUM;
	if (totalNumEyes > HEXEYE_MAXEYES)
		return ECODE_FAIL;
	if (RetOk(m_eyeGen->init(targr, m_lowestStampLev+1))) {
		for (int i = 0; i < totalNumEyes; i++) {
			m_eyeGen->spawn();
		}
	}
	else return ECODE_FAIL;
	if (RetOk(m_lunaEyeGen->init(targr, m_lowestStampLev, PATTERNLUNA_NUM))) {/*patternluna_num means this generates 8 node pointers at the lowest level*/
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
	return ECODE_OK;
}
void StampEye::release() {
	int maxLunLev = m_lowestStampLev - 1;
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
void StampEye::setupForStampi(int i) {
	for (int j = 0; j < STAMPEYENUM; j++) {
		m_lunaStamps[j].o = 0.f;
	}
	m_lunaStamps[i].o = 1.f;
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
	}
	s.mask = NULL;
	s.n = 0;
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
					s_fNode& curLunaStampNode = *(s_fNode*)(curLunaStampEye->lev[maxLunLev].m_fhex[l_i].nodes[lun_i]);
					PatternLunaThreaded::evalLowerNode(lunaPatNode, curStampNode);/*this fills the o of the curStampNode so that it returns the luna value from evaluating in the center of the hex*/
					curLunaStampNode.o = lunaPatNode.o;
				}
			}
		}
	}
	return ECODE_OK;
}
unsigned char StampEye::stampFullNewMoons() {
	stampMoonEye(m_eyeGen->getEye(m_eyes_stamped), 0.f);
	m_stamps[m_num_stamps].eyes[0] = m_eyeGen->getEyePtr(m_eyes_stamped);
	m_stamps[m_num_stamps].n = 1;
	m_eyes_stamped++;
	m_num_stamps++;
	stampMoonEye(m_eyeGen->getEye(m_eyes_stamped), 1.f);
	m_stamps[m_num_stamps].eyes[0] = m_eyeGen->getEyePtr(m_eyes_stamped);
	m_stamps[m_num_stamps].n = 1;
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
	float cur_circleRadius = m_minCircleRadius;
	int stamp_cnt = m_num_stamps;
	for (int i_ang = 0; i_ang < n_ang; i_ang++) {
		for (int i_rad = 0; i_rad < n_circleRadii; i_rad++) {
			stampRoundedCornersAtCenterAndAng(center, cur_ang, cur_circleRadius, PI / 4.f, stamp_cnt);/*increaments stamp count appropriately*/
			m_num_stamps++;
			if (!stampEyeIncOk(stamp_cnt))
				break;
			cur_circleRadius *= stampeye_radincmul;
		}
		if (!stampEyeIncOk(stamp_cnt))
			break;
		cur_ang += DAng;
	}
	return ECODE_OK;
}
unsigned char StampEye::stampRoundedCornersAtCenterAndAng(const s_2pt& center, float ang, float circle_scale, float opening_ang, int& stamp_cnt) {
	float r_scale = m_eyeGen->getRSHex(0, m_eyeGen->getMaxLevi(0));
	r_scale *= 2.f;
	float R = 2.f * r_scale / sqrtf(3);
	s_2pt* hexU = m_eyeGen->getUHex();
	s_2pt smudge_center = { 0.f, 0.f };

	float angD = 2 * PI / m_numAngDiv;/*width of master angle*/
	float smudgeAngD = (m_smudgeAngNum>=3) ? angD / (float)(m_smudgeAngNum-1) : 0.f;
	float smudgeAngStart = (m_smudgeAngNum >= 3) ? smudgeAngD * ((float)m_smudgeAngNum - 1.f) / 2.f : 0.f;
	float curSmudgeAng = -smudgeAngStart;

	m_stamps[stamp_cnt].n = 0;
	for (int j = 0; j < m_smudgeNum; j++) {
		/*not yet adding the smudge from the translational smudge*/
		/*may or  may not have the translation set up properly with stampEyeRoundedCorner*/
		if (hexMath::inHex(R, r_scale, hexU, smudge_center, center)) {/*not really needed since center is identical to corner_center*/
			for (int i_ang = 0; i_ang < m_smudgeAngNum; i_ang++) {
				setBasisFromAng(curSmudgeAng);

				setRoundedCorner(smudge_center, circle_scale, opening_ang);
				stampEyeRoundedCorner(m_eyeGen->getEye(m_eyes_stamped));
				m_stamps[stamp_cnt].eyes[m_stamps[stamp_cnt].n] = m_eyeGen->getEyePtr(m_eyes_stamped);
				(m_stamps[stamp_cnt].n)++;
				m_eyes_stamped++;

				curSmudgeAng += smudgeAngD;
			}
		}
	}
	stamp_cnt++;
	return ECODE_OK;
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
float StampEye::AveOverHexRoundedCorner(const s_hexPlate& eyeplate, const s_2pt& center) {
	float R = eyeplate.m_Rhex;
	float RS = eyeplate.m_RShex;
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
				if (isInRoundedCorner(pt))
					av += 1.f;
			}
		}
	}
	return av / tot;
}
unsigned char StampEye::setBasisFromAng(float ang) {
	vecMath::setBasis(ang, m_UBasis0, m_UBasis1);
	vecMath::revBasis(m_UBasis0, m_UBasis1, m_UrevBasis0, m_UrevBasis1);
	return ECODE_OK;
}
unsigned char StampEye::setRoundedCorner(const s_2pt& center, float radius, float ang_rad) {
	if (ang_rad < STAMPEYEMINANGRAD)
		return ECODE_ABORT;
	/*assumes that the m_UcenterIn points along the x direction*/
	m_circle_radius = radius;
	m_UcenterIn.x0 = -1.f;
	m_UcenterIn.x1 = 0.f;
	s_2pt offset = vecMath::mul(radius, m_UcenterIn);
	m_circle_center = vecMath::add(center, offset);
	float halfAng = ang_rad / 2.f;
	s_2pt l1 = { cosf(halfAng), sinf(halfAng) };
	s_2pt l2 = { l1.x0, -l1.x1 };
	m_Uline_perp1.x0 = -l1.x0; // vecMath::perpUL(l1);
	m_Uline_perp1.x1 = -l1.x1;
	m_Uline_perp2.x0 = -l2.x0; //vecMath::perpUR(l2);
	m_Uline_perp2.x1 = -l2.x1;
	/* CircCent + (-r*Uperp1) + Lunknw*l1U  = Ounknw * (-1, 0) + CircCent */
	/* -r*Uperp1 + Lunknw * l1U = Ounknw * (-1, 0) */
	/* -r * (Uly, -Ulx) + Lunknw * (U1x, U1y) = Ounknw * (-1, 0) */
	/* -r*Uly + Lunknw*Ulx = -Ounknw */
	/* r*Uly - Lunknw*Ulx = Ounknw */
	/*  r*Ulx + Lunknw*Uly = 0 */
	/*  Lunknw = -r*Ulx/Uly */
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
	s_2pt VtoPt = vecMath::v12(m_circle_center, pt);
	return vecMath::dot(VtoPt, m_UcenterIn) >= 0;
}
bool StampEye::isInRoundedCornerNoRot(const s_2pt& pt) {
	bool isInLines = isUnderLine(pt, m_Uline_perp1) && isUnderLine(pt, m_Uline_perp2);
	if (!isInLines)
		return false;
	if (isInsideCurveHalf(pt))
		return true;
	return isInCircle(pt);
}
bool StampEye::isInRoundedCorner(const s_2pt& pt) {
	s_2pt convPt = vecMath::convBasis(m_UrevBasis0, m_UrevBasis1, pt);
	return isInRoundedCornerNoRot(convPt);
}

bool StampEye::stampEyeIncOk(int stamp_cnt) {
	return stamp_cnt < STAMPEYEMAXNUM;
}