#include "CameraTrans.h"
CameraTrans::CameraTrans():m_yPinHole_screenLowPt(0.f), m_camera_d(0.f),
m_y_pixH(0.f), m_x_pixW(0.f),
m_ang_openingH(0.f), m_ang_openingW(0.f),
m_camera_ang(0.f),
m_screen_y_horizion(0.f), m_screen_x_center(0.f),
m_cot_90_camAng(0.f), m_cos_camAng(0.f), m_sin_camAng(0.f),
m_camera_y(0.f),
m_f_pix(0.f)
{
	;
}
CameraTrans::~CameraTrans() {
	;
}
unsigned char CameraTrans::init(
	s_hexPlate& plate,
	float yPinHole_screenLowPt,
	float camera_d,
	float camera_y,
	float camera_ang,
	float screen_y_horizion_offset,
	float screen_x_center_offset
)
{
	if (camera_d <= 0.f)
		return ECODE_ABORT;

	m_yPinHole_screenLowPt = yPinHole_screenLowPt;
	m_camera_d = camera_d;
	m_y_pixH = (float)plate.m_height;
	m_x_pixW = (float)plate.m_width;
	float screen_half = m_y_pixH / 2.f;
	m_screen_y_horizion = screen_half + screen_y_horizion_offset;
	screen_half = m_x_pixW / 2.f;
	m_screen_x_center = screen_half + screen_x_center_offset;
	m_camera_y = camera_y;
	m_camera_ang = camera_ang;
	float _camAng = PI / 2.f - m_camera_ang;
	float t_camAng = tanf(_camAng);
	m_cot_90_camAng = (t_camAng>=0.00001f) ? 1.f /t_camAng : 0.f;
	m_sin_camAng = sinf(m_camera_ang);
	m_cos_camAng = cosf(m_camera_ang);

	screen_half = m_y_pixH - m_screen_y_horizion;
	/* pix_y/f = d/y
	* f = y*pix_y/d
	*/
	m_f_pix = yPinHole_screenLowPt * screen_half / m_camera_d;
	return ECODE_OK;
}
unsigned char CameraTrans::setFocalFromOpeningAngle(float openingAngleH) {
	if (m_camera_d <= 0.f)
		return ECODE_ABORT;
	if (openingAngleH <= 0.f)
		return ECODE_ABORT;
	if (m_y_pixH <= 0.f)
		return ECODE_ABORT;
	/* tan ang = d/y = pix_y/f */
	float ang = openingAngleH / 2.f;
	float PixPerAng = m_y_pixH / openingAngleH;
	float screen_half = m_y_pixH / 2.f;
	float angPerPix = openingAngleH / m_y_pixH;
	float angOffset = (screen_half - m_screen_y_horizion)*angPerPix;
	ang += angOffset;
	if (ang <= 0.f)
		return ECODE_ABORT;
	m_f_pix = (m_y_pixH - m_screen_y_horizion) / tanf(ang);
	s_2pt lowestPix = { 0.f, m_y_pixH };
	s_2pt closestYPt;
	if (!drivePlaneCoordCameraCent(lowestPix, closestYPt))
		return ECODE_FAIL;
	m_yPinHole_screenLowPt = closestYPt.x1;
	return ECODE_OK;
}
void CameraTrans::release() {
	m_yPinHole_screenLowPt = 0.f;
	m_camera_d = 0.f;
}

bool CameraTrans::drivePlaneCoord(const s_2pt& screenCoord, s_2pt& planeCoord) {
	if (!screenToDriveplane(screenCoord, planeCoord))
		return false;
	planeCoord.x1 += m_camera_y;
	return true;
}
bool CameraTrans::DriveplaneToScreenCameraCent(const s_2pt& XY, s_2pt& screenXY) {
	s_2pt XYNorm = { XY.x0 / m_camera_d, XY.x1 / m_camera_d };
	return Driveplane_Unit_d_To_screen(XYNorm, screenXY);
}
bool CameraTrans::screenToDriveplane_Unit_d(const s_2pt& screenXY, s_2pt& XY) {
	/*
	* theta_cd = 90-theta_ca
	* 
	*{1+(y_pix/f_pix)*cot(theta_cd)} / {cot(theta_cd) - (y_pix/f_pix)} = y (in units of d a.k.a y/d)
	*/
	float yScdown =  m_screen_y_horizion - screenXY.x1;/*take into account that y starts from top, so sign must be reversed*/
	float y_f_pix = yScdown / m_f_pix;
	float cot_y_f_pix = m_cot_90_camAng - y_f_pix;
	if (cot_y_f_pix <= 0.f)
		return false;
	float sum_1_y_f_pixCot = 1 + y_f_pix * m_cot_90_camAng;
	XY.x1 = sum_1_y_f_pixCot / cot_y_f_pix;

	/*
	* y *{ [x_pix/f_pix]/[ cos(theta_ca) + (y_pix/f_pix)*sin(theta_ca)] = x
	*/
	float xScAccross = screenXY.x0 - m_screen_x_center;
	float x_f_pix = xScAccross / m_f_pix;
	float cos_plus_sin = m_cos_camAng + y_f_pix * m_sin_camAng;
	if (cos_plus_sin <= 0.f)
		return false;
	XY.x0 = XY.x1 * x_f_pix / cos_plus_sin;
	return true;
}
bool CameraTrans::screenToDriveplane_Unit_d_Horiz(const s_2pt& screenXY, s_2pt& XY) {
	/*y starts at top so angle from top of screen*/
	float yScdown = screenXY.x1 - m_screen_y_horizion;
	if (yScdown <= 0.f)
		return false;
	/*assume the camer is oriented so that the plane it is projecting on is at right angles
	  to the drive plane
	  */
	  /* camera_y/f = d/y
	  *  y = d*f/camera_y
	  */
	XY.x1 = m_f_pix / yScdown;/*y when * m_camera_d */
	/*
	*camera_x/f =x/y
	* x = camera_x/f * y
	*/
	float xScAccross = screenXY.x0 - m_screen_x_center;
	/* Xsc/f * y -> substitung for y: Xsc/Ysc * d */
	XY.x0 = xScAccross / yScdown; /*x when * m_camera_d */
	return true;
}
bool CameraTrans::screenToDriveplane(const s_2pt& screenXY, s_2pt& XY) {
	if (!screenToDriveplane_Unit_d(screenXY, XY))
		return false;
	convFastCoordToCoord(XY);
	return true;
}
bool CameraTrans::Driveplane_Unit_d_To_screenFNorm(const s_2pt& XY, s_2pt& screenXY) {
	/*XY is divided by camera_d already*/

	/* y_pix/f_pix = {1+(y/d) * cot(theta_cd)}{y/d - cot(theta_cd)}*/
	float denom = XY.x1 - m_cot_90_camAng;
	if (denom <= 0.f)
		return false;
	screenXY.x1 = (1 + XY.x1 * m_cot_90_camAng) / denom;

	/*x_pix/f_pix = (x/y){cos(theta_ca) + (y_pix/f_pix)*sin(theta_ca)}*/
	if (XY.x1 <= 0.f)
		return false;
	screenXY.x0 = XY.x0 / XY.x1;
	float numer = m_cos_camAng + screenXY.x1 * m_sin_camAng;
	screenXY.x0 *= numer;
	return true;
}
bool CameraTrans::Driveplane_Unit_d_To_screen(const s_2pt& XY, s_2pt& screenXY) {
	if (!Driveplane_Unit_d_To_screenFNorm(XY, screenXY))
		return false;
	screenXY.x0 *= m_f_pix;
	screenXY.x1 *= m_f_pix;
	/*now take into account that screen y starts at top and not middle, and that y =  -y*/
	screenXY.x1 = m_screen_y_horizion - screenXY.x1;
	/*also set the x pix to the correct value*/
	screenXY.x0 += m_screen_x_center;

	if (screenXY.x0<0.f || screenXY.x0 > m_x_pixW || screenXY.x1<0.f || screenXY.x1 > m_y_pixH)
		return false;
	return true;
}