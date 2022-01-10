#include "CameraTrans.h"
CameraTrans::CameraTrans():m_yPinHole_screenLowPt(0.f), m_camera_d(0.f),
m_y_pixH(0.f), m_x_pixW(0.f),
m_ang_openingH(0.f), m_ang_openingW(0.f),
m_screen_y_horizion(0.f), m_screen_x_center(0.f),
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
	float screen_y_horizion_offset,
	float screen_x_center_offset
)
{
	if (m_camera_d <= 0.f)
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
	/* tan ang = d/y = pix_y/f */
	float ang = openingAngleH / 2.f;
	float angPerPix = m_y_pixH / openingAngleH;
	float screen_half = m_y_pixH / 2.f;
	float angOffset = angPerPix + (screen_half - m_screen_y_horizion);
	ang += angOffset;
	if (ang <= 0.f)
		return ECODE_ABORT;
	m_yPinHole_screenLowPt = m_camera_d / tanf(ang);
	m_f_pix = (m_y_pixH - m_screen_y_horizion) / tanf(ang);
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
bool CameraTrans::screenToDriveplane_Unit_d(const s_2pt& screenXY, s_2pt& XY) {
	/*y starts at top so angle from top of screen*/
	float yScdown = m_screen_y_horizion - screenXY.x1;
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