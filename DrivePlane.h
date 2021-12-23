#pragma once
#ifndef DRIVEPLANE_H
#define DRIVEPLANE_H
#ifndef CAMERATRANS_H
#include "CameraTrans.h"
#endif
#ifndef LINEFINDER_H
#include "LineFinder.h"
#endif

/* Drive plane is designed to take the lines from lineFinder and project them onto
 a plate (or set of plates) that represent the flat area the 'drive plane' in front
 of the robot 
 */
class DrivePlane : public Base {
public:

	unsigned char update();
protected:
	float m_screenClosestY;/*distance from camera center in y to point closest viewable to the camera on the plane*/
	/*not owned*/
	s_line** m_lines;
	int      m_n_lines;
	/*owned    */
	CameraTrans* m_cameraTrans;
	s_PlateLayer m_plateLayer;/*plates are assumed to start at closest visible distance */
	float m_plateDimToPix[MAXPLATESPERLAYER]; /*scale factors that scale the dimension of the 
											   real distance on the drive plane to the pix dim of
											   the plate representing 
											   the plane seen by the camera */

	unsigned char transLineToPlate(float scaleFac, s_hexPlate& plate, const s_line* pline);
	bool linePtToHex(float scaleFac, const s_linePoint& linePt, s_hexPlate& plate);
	bool screenLocToPlateLoc(float scaleFac, const s_2pt& screenXY, s_2pt& plateXY);
};

#endif

bool DrivePlane::screenLocToPlateLoc(float scaleFac, const s_2pt& screenXY, s_2pt& plateXY) {
	s_2pt driveXY;
	bool lowEnough = m_cameraTrans->drivePlaneCoordCameraCent(screenXY, driveXY);
	if (!lowEnough)
		return false;
	driveXY.x1 -= m_screenClosestY;
	if (driveXY.x1 < 0.f)
		return false;
	plateXY.x0 = scaleFac * driveXY.x0;
	plateXY.x1 = scaleFac * driveXY.x1;
	return true;
}
unsigned char DrivePlane::transLineToPlate(float scaleFac, s_hexPlate& plate, const s_line* pline)
{

}