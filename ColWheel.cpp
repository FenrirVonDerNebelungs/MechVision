#include "ColWheel.h"
ColWheel::ColWheel() : m_pixMax(0.f), m_Dhue(0.f), m_DI(0.f), m_DSat(0.f), m_hueFadeV(0.f), m_I_target(0.f), m_Sat_target(0.f), m_stepSteepness(0.f), m_DhueRes(0.f), m_hueLen(0.f) {
	m_hue_target.x0 = 0.f;
	m_hue_target.x1 = 0.f;
	m_Ur.x0 = 0.f;
	m_Ur.x1 = 0.f;
	m_Ug.x0 = 0.f;
	m_Ug.x1 = 0.f;
	m_Ub.x0 = 0.f;
	m_Ub.x1 = 0.f;
	m_hueRGBVec.x0 = 0.f;
	m_hueRGBVec.x1 = 0.f;
}
ColWheel::~ColWheel() {
	;
}
unsigned char ColWheel::Init(
	float pixMax,
	float Dhue,
	float DI,
	float DSat,
	float hueFadeV,
	float I_target,
	float hue_target_x,
	float hue_target_y,
	float Sat_target,
	float stepSteepness
) {
	m_pixMax = pixMax;
	m_Dhue = Dhue;
	m_DI = DI;
	m_DSat = DSat;
	m_hueFadeV = hueFadeV;
	m_I_target = I_target;
	m_hue_target.x0 = hue_target_x;
	m_hue_target.x1 = hue_target_y;
	m_Sat_target = Sat_target;
	m_stepSteepness = stepSteepness;

	setColwheelUnitVectors();/*sets Ur, Ug, Ub*/
	m_DhueRes = 2.f - m_Dhue;
	return ECODE_OK;
}
unsigned char ColWheel::Update(
	float Dhue,
	float DI,
	float DSat,
	float hueFadeV,
	float I_target,
	float hue_target_x,
	float hue_target_y,
	float Sat_target,
	float stepSteepness
) {
	m_Dhue = Dhue;
	m_DI = DI;
	m_DSat = DSat;
	m_hueFadeV = hueFadeV;
	m_I_target = I_target;
	m_hue_target.x0 = hue_target_x;
	m_hue_target.x1 = hue_target_y;
	m_Sat_target = Sat_target;
	m_stepSteepness = stepSteepness;

	m_DhueRes = 2.f - m_Dhue;
	return ECODE_OK;
}
void ColWheel::Release() {
	;
}
float ColWheel::evalRGB(float rgb[]) {
	float lowHi = findHueIDist(rgb);
	lowHi *= m_stepSteepness;
	return Math::StepFunc(lowHi);
}

float ColWheel::findHueIDist(float rgb[]) {
	s_2pt hueV = findColwheelHue(rgb);
	float hDist = findHueDistance(hueV);
	float IDist = findIDistance(rgb);
	float SatDist = findSatDistance(rgb);
	return (hDist * IDist * SatDist)-0.5f;
}
float ColWheel::findHueDistance(s_2pt& hueV) {
	if (m_DhueRes <= 0.f)
		return 1.f;
	if (m_hueLen == 0.f)
		return 0.f;
	/*find fade multiplier*/
	float fadeMult = 1.f;
	if (m_hueLen < m_hueFadeV) {
		fadeMult = m_hueLen / m_hueFadeV;
	}
	/*find rot dist in color wheel*/
	s_2pt hueU = { m_hueRGBVec.x0 / m_hueLen, m_hueRGBVec.x1 / m_hueLen };
	float hueDot = vecMath::dot(hueU, m_hue_target);/*this should range from 1 to -1*/
	float hueDiff = 1.f - hueDot;

	float hueRet=0.f;
	if (hueDiff <= m_Dhue) {
		float sepval = hueDiff / m_Dhue;
		sepval = 1.f - sepval;
		hueRet = sepval * fadeMult;
	}
	else if (hueDiff > m_Dhue) {
		float resid = 2.f - hueDiff;
		float sepval = resid / m_DhueRes;
		sepval = 1.f - sepval;
		sepval = -sepval;
		hueRet = sepval * fadeMult;
	}
	return (hueRet >= 0.f) ? hueRet : 0.f;
}
float ColWheel::findIDistance(float rgb[]) {
	float r = rgb[0] / m_pixMax;
	float g = rgb[1] / m_pixMax;
	float b = rgb[2] / m_pixMax;
	float totI = (r + g + b) / 3.f;
	float Idist = fabsf(totI - m_I_target);
	float norDist = Idist / m_DI;
	norDist -= 1.f;
	norDist = -norDist;
	return (norDist >= 0.f) ? norDist : 0.f;
}
float ColWheel::findSatDistance(float rgb[]) {
	if (m_DSat > 1.f)
		return 1.f;
	float maxVal = 0.f;
	for (int i = 0; i < 3; i++) {
		if (rgb[i] > maxVal)
			maxVal = rgb[i];
	}
	maxVal /= m_pixMax;
	float satVal= (maxVal > 0.f) ? m_hueLen / maxVal : 0.f;
	float valDiff = fabsf(satVal - m_Sat_target);
	float NormValDiff = (m_DSat > 0.f) ? valDiff / m_DSat : valDiff;
	float satDist= (1.f - NormValDiff);
	return (satDist >= 0.f) ? satDist : 0.f;
}
s_2pt ColWheel::findColwheelHue(float rgb[]) {
	float r = rgb[0] / m_pixMax;
	float g = rgb[1] / m_pixMax;
	float b = rgb[2] / m_pixMax;
	s_2pt Rv = { r * m_Ur.x0, r * m_Ur.x1 };
	s_2pt Gv = { g * m_Ug.x0, g * m_Ug.x1 };
	s_2pt Bv = { b * m_Ub.x0, b * m_Ub.x1 };
	s_2pt hueV = vecMath::add(Rv, Gv, Bv);
	m_hueLen = vecMath::len(hueV);
	m_hueRGBVec = hueV;
	return hueV;
}
void ColWheel::setColwheelUnitVectors() {
	/*asume Ur goes out along the x0 axis*/
	m_Ur.x0 = 1.f;
	m_Ur.x1 = 0.f;
	/*the other 2 vectors are 120 degrees or 2PI/3 rad away from this one*/
	m_Ug.x0 = -cosf(PI / 3.f);
	m_Ug.x1 = sinf(PI / 3.f);
	m_Ub.x0 = m_Ug.x0;
	m_Ub.x1 = -m_Ug.x1;
}
