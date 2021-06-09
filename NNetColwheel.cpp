#include "stdafx.h"
#include "NNetColwheel.h"
NNetTestColwheel::NNetTestColwheel() : m_pixMax(0.f), m_Dhue(0.f), m_DI(0.f), m_hueFadeV(0.f), m_I_target(0.f), m_stepSteepness(0.f), m_DhueRes(0.f){
	m_hue_target.x0 = 0.f;
	m_hue_target.x1 = 0.f;
	m_Ur.x0 = 0.f;
	m_Ur.x1 = 0.f;
	m_Ug.x0 = 0.f;
	m_Ug.x1 = 0.f;
	m_Ub.x0 = 0.f;
	m_Ub.x1 = 0.f;
}
NNetTestColwheel::~NNetTestColwheel() {
	;
}
unsigned char NNetTestColwheel::Init(
	float pixMax,
	float Dhue,
	float DI,
	float hueFadeV,
	float I_target,
	float hue_target_x,
	float hue_target_y,
	float stepSteepness
) {
	m_pixMax = pixMax;
	m_Dhue = Dhue;
	m_DI = DI;
	m_hueFadeV = hueFadeV;
	m_I_target = I_target;
	m_hue_target.x0 = hue_target_x;
	m_hue_target.x1 = hue_target_y;
	m_stepSteepness = stepSteepness;

	setColwheelUnitVectors();/*sets Ur, Ug, Ub*/
	m_DhueRes = 2.f - m_Dhue;
	return ECODE_OK;
}
void NNetTestColwheel::Release() {
	;
}
float NNetTestColwheel::evalRGB(float rgb[]) {
	float lowHi = findHueIDist(rgb);
	lowHi *= m_stepSteepness;
	return Math::StepFunc(lowHi);
}
float NNetTestColwheel::findHueIDist(float rgb[]) {
	s_2pt hueV = findColwheelHue(rgb);
	float hDist = findHueDistance(hueV);
	float IDist = findIDistance(rgb);
	return hDist * IDist;
}
float NNetTestColwheel::findHueDistance(s_2pt& hueV) {
	if (m_DhueRes <= 0.f)
		return 1.f;
	float hueLen = vecMath::len(hueV);
	if (hueLen == 0.f)
		return 0.5f;
	/*find fade multiplier*/
	float fadeMult = 1.f;
	if (hueLen < m_hueFadeV) {
		fadeMult = hueLen / m_hueFadeV;
	}
	/*find rot dist in color wheel*/
	s_2pt hueU = { hueV.x0 / hueLen, hueV.x1 / hueLen };
	float hueDot = vecMath::dot(hueU, m_hue_target);/*this should range from 1 to -1*/
	float hueDiff = 1.f - hueDot;

	if (hueDiff <= m_Dhue) {
		float sepval = hueDiff / m_Dhue;
		sepval = 1.f - sepval;
		return sepval * fadeMult;
	}
	else if (hueDiff > m_Dhue) {
		float resid = 2.f - hueDiff;
		float sepval = resid / m_DhueRes;
		sepval = 1.f - sepval;
		sepval = -sepval;
		return sepval * fadeMult;
	}
	return 0.f;/*this point should never be reached*/
}
float NNetTestColwheel::findIDistance(float rgb[]) {
	float r = rgb[0] / m_pixMax;
	float g = rgb[1] / m_pixMax;
	float b = rgb[2] / m_pixMax;
	float totI = (r + g + b) / 3.f;
	float Idist = fabsf(totI - m_I_target);
	float norDist = Idist / m_DI;
	norDist -= 1.f;
	norDist = -norDist;
	return norDist;
}
s_2pt NNetTestColwheel::findColwheelHue(float rgb[]) {
	float r = rgb[0] / m_pixMax;
	float g = rgb[1] / m_pixMax;
	float b = rgb[2] / m_pixMax;
	s_2pt Rv = { r * m_Ur.x0, r * m_Ur.x1 };
	s_2pt Gv = { g * m_Ug.x0, g * m_Ug.x1 };
	s_2pt Bv = { b * m_Ub.x0, b * m_Ub.x1 };
	s_2pt hueV = vecMath::add(Rv, Gv, Bv);
	return hueV;
}
void NNetTestColwheel::setColwheelUnitVectors() {
	/*asume Ur goes out along the x0 axis*/
	m_Ur.x0 = 1.f;
	m_Ur.x1 = 0.f;
	/*the other 2 vectors are 120 degrees or 2PI/3 rad away from this one*/
	m_Ug.x0 = -cosf(PI / 3.f);
	m_Ug.x1 = sinf(PI / 3.f);
	m_Ub.x0 = m_Ug.x0;
	m_Ub.x1 = -m_Ug.x1;
}
