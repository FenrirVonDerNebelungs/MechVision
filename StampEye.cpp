#include "StampEye.h"
#include "PatternL1.h"
StampEye::StampEye() : m_numAngDiv(0.f), m_smudgeNum(0), m_patternLuna(NULL), m_eyeGen(NULL), m_lunaEyeGen(NULL), m_eyes_stamped(0), m_circle_radius(0.f) {
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
	float numAngDiv,
	int smudgeNum,
	float r,
	HexBase* hexBase
) {
	m_patternLuna = patLuna;
	m_numAngDiv = numAngDiv;
	m_smudgeNum = smudgeNum;
	m_eyeGen = new HexEye;
	m_lunaEyeGen = new HexEye;

	float targr = (hexBase == NULL) ? r : hexBase->getRhex();
	if (RetOk(m_eyeGen->init(targr, 2))) {
		int numSmudgeArray = smudgeNum * smudgeNum;
		int totalNumEyes = numSmudgeArray * STAMPEYENUM;
		for (int i = 0; i < totalNumEyes; i++) {
			m_eyeGen->spawn();
		}
	}
	else return ECODE_FAIL;
	if (RetOk(m_lunaEyeGen->init(2 * targr, 1, PATTERNLUNA_NUM))) {
		for (int i = 0; i < m_eyeGen->getNEyes(); i++) {
			m_lunaEyeGen->spawn();
			/*this is an unusual case where the lowest of nodes of the hexEyes will be owned by the eye*/
			s_hexEye lastEye = m_lunaEyeGen->getLastEye();
			for (int h_i = 0; h_i < lastEye.lev[1].m_nHex; h_i++) {
				for (int l_i = 0; l_i < lastEye.lev[1].m_fhex[h_i].N; l_i++) {
					lastEye.lev[1].m_fhex[h_i].nodes[l_i] = (s_bNode*)new s_fNode;
					lastEye.lev[1].m_fhex[h_i].nodes[l_i]->zero();
				}
			}
		}
	}
	else return ECODE_FAIL;



	return ECODE_OK;
}
void StampEye::release() {
	if (m_lunaEyeGen != NULL) {
		/*this is an case where the node pointers of the lowest level of the eyes is actually owned by the hexEyes so this must be released before they are deleted*/
		for (int i = 0; i < m_lunaEyeGen->getNEyes(); i++) {
			s_hexEye* Eye = m_lunaEyeGen->getEyePtr(i);
			if (Eye != 0) {
				for (int h_i = 0; h_i < Eye->lev[1].m_nHex; h_i++) {
					for (int l_i = 0; l_i < Eye->lev[1].m_fhex[h_i].N; l_i++) {
						if (Eye->lev[1].m_fhex[h_i].nodes[l_i] != NULL)
							delete Eye->lev[1].m_fhex[h_i].nodes[l_i];
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
	return calcLunaStampEye();
}

void StampEye::clearEyeStamps() {
	for (int i = 0; i < STAMPEYENUM; i++) {
		for (int j = 0; j < STAMPEYEMAXNUM; j++) {
			m_lunaStamps[i].eyes[j] = NULL;
			m_stamps[i].eyes[j] = NULL;
		}
		m_lunaStamps[i].n = 0;
		m_stamps[i].n = 0;
	}
}
unsigned char StampEye::calcLunaStampEye(const s_hexEye& seye, s_hexEye& slunaeye) {
	for (int s_i = 0; s_i < STAMPEYENUM; s_i++) {
		for (int e_i = 0; e_i < m_stamps[s_i].n; e_i++) {
			s_hexEye* curLunaStampEye = m_lunaStamps[s_i].eyes[e_i];
			s_hexEye* curStampEye = m_stamps[s_i].eyes[e_i];
			/*loop over the lowest level of the lunaeyestamps and the 2nd lowest of eyestamps to fill luna value for each hex in the luna eye stamp*/
			for (int l_i = 0; l_i < curLunaStampEye->lev[1].m_nHex; l_i++) {
				/*the nodes hanging from lev 1of the stamp eye are already generated and owned by the stamp eye
				  the number of these nodes corresponds to the number of luna's */
				for (int lun_i = 0; lun_i < curLunaStampEye->lev[1].m_fhex[l_i].N; lun_i++) {
					s_fNode& lunaPatNode = *(m_patternLuna->getPatNode(lun_i));
					s_fNode& curStampNode = *(s_fNode*)(curLunaStampEye->lev[1].m_fhex[l_i].nodes[lun_i]);
					PatternLunaThreaded::evalAtPlateNode(lunaPatNode, curStampNode);/*this fills the o of the curStampNode so that it returns the luna value from evaluating in the center of the hex*/
				}
			}
		}
	}
	return ECODE_OK;
}
unsigned char StampEye::stampFullNewMoons() {
	stampMoonEye(m_eyeGen->getEye(m_eyes_stamped), 0.f);
	m_stamps[m_eyes_stamped].eyes[0] = m_eyeGen->getEyePtr(m_eyes_stamped);
	m_stamps[m_eyes_stamped].n = 1;
	m_eyes_stamped++;
	stampMoonEye(m_eyeGen->getEye(m_eyes_stamped), 1.f);
	m_stamps[m_eyes_stamped].eyes[0] = m_eyeGen->getEyePtr(m_eyes_stamped);
	m_stamps[m_eyes_stamped].n = 1;
	m_eyes_stamped++;

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
	float r_scale = m_eyeGen->getRSHex(0, m_eyeGen->getMaxLevi(0));
	r_scale *= 2.f;
	float R = 2.f * r_scale / sqrtf(3);
	float smudgeD = r_scale / ((float)m_smudgeNum);
	s_2pt corner_start = { -r_scale / 2.f, -r_scale / 2.f };
	s_2pt* hexU = m_eyeGen->getUHex();
	s_2pt center = { 0.f, 0.f };
	int cur_eye = m_eyes_stamped;
	int stamp_inc_i_start = m_eyes_stamped;
	for (int j = 0; j < m_smudgeNum; j++) {
		for (int i = 0; i < m_smudgeNum; i++) {
			s_2pt corner_center = { corner_start.x0 + ((float)i) * smudgeD, corner_start.x1 + ((float)j) * smudgeD };
			if (hexMath::inHex(R, r_scale, hexU, center, corner_center)) {
				cur_eye = stampRoundedCornersAtCenter(corner_center, cur_eye, m_stamps);
				incStampEyeNs(stamp_inc_i_start);
			}
		}
	}
	m_eyes_stamped = cur_eye;
	return ECODE_OK;
}
int StampEye::stampRoundedCornersAtCenter(const s_2pt& corner_center, int eye_cnt, s_eyeStamp stamp[]) {
	float angD = 2 * PI/m_numAngDiv;
	int NAng = (int)floorf(m_numAngDiv);
	float curAng=0.f;

	float r_scale = m_eyeGen->getRSHex(0, m_eyeGen->getMaxLevi(0));
	r_scale *=2.f;
	float opening_ang = PI / 2.f;

	float circle_scale = 2.f * r_scale;

	for (int i_ang = 0; i_ang < NAng; i_ang++) {
		setBasisFromAng(curAng);
		for (float i_opening = 2.f; i_opening <= 4.f; i_opening += 1.f) {
			opening_ang = PI / i_opening;
			setRoundedCorner(corner_center, r_scale, opening_ang);
			stampEyeRoundedCorner(m_eyeGen->getEye(eye_cnt));
			stamp[eye_cnt].eyes[stamp[eye_cnt].n] = m_eyeGen->getEyePtr(eye_cnt);
			eye_cnt++;
		}
		for (int i_circlescale = 1; i_circlescale <= 3; i_circlescale++) {
			setRoundedCorner(corner_center, circle_scale, PI / 4.f);
			stampEyeRoundedCorner(m_eyeGen->getEye(eye_cnt));
			stamp[eye_cnt].eyes[stamp[eye_cnt].n] = m_eyeGen->getEyePtr(eye_cnt);
			eye_cnt++;

			circle_scale *= 2.f;
		}
		curAng += angD;
	}
	return eye_cnt;
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
	float av = 0;
	for (int j = 0; j < WH; j++) {
		for (int i = 0; i < WH; i++) {
			pt.x0 = startPt.x0 + (float)i;
			pt.x1 = startPt.x1 + (float)j;
			if (hexMath::inHex(R, RS, hexU, center, pt)) {
				if (isInRoundedCorner(pt))
					av += 1.f;
			}
		}
	}
	float tot = WH * WH;
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
	s_2pt offset = vecMath::mul(radius, m_UcenterIn);
	m_circle_center = vecMath::add(center, offset);
	float halfAng = ang_rad / 2.f;
	s_2pt l1 = { cosf(halfAng), sinf(halfAng) };
	s_2pt l2 = { l1.x0, -l1.x1 };
	m_Uline_perp1 = vecMath::perpUL(l1);
	m_Uline_perp2 = vecMath::perpUR(l2);
	/* CircCent + (-r*Uperp1) + Lunknw*l1U  = Ounknw * (-1, 0) + CircCent */
	/* -r*Uperp1 + Lunknw * l1U = Ounknw * (-1, 0) */
	/* -r * (Uly, -Ulx) + Lunknw * (U1x, U1y) = Ounknw * (-1, 0) */
	/* -r*Uly + Lunknw*Ulx = -Ounknw */
	/* r*Uly - Lunknw*Ulx = Ounknw */
	/*  r*Ulx + Lunknw*Uly = 0 */
	/*  Lunknw = -r*Ulx/Uly */
	float Ltanvec = -radius * l1.x0 / l1.x1;
	float OutToPt = radius * l1.x1 - Ltanvec * l1.x0;
	m_line_intersect.x1 = 0.f;
	m_line_intersect.x0 = center.x0 - OutToPt;
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
	bool isInLines = isUnderLine(pt, m_Uline_perp1) & isUnderLine(pt, m_Uline_perp2);
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

bool StampEye::incStampEyeNs(int start_i) {
	for (int i = start_i; i < STAMPEYENUM; i++) {
		if (m_stamps[i].n + 1 < STAMPEYEMAXNUM)
			m_stamps[i].n += 1;
		else
			return false;
	}
	return true;
}