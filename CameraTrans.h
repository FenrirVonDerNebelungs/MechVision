#pragma once
#ifndef CAMERATRANS_H
#define CAMERATRANS_H
#ifndef BASE_H
#include "Base.h"
#endif

/*designed to convert screen coordinates to locations on a drive plane that is
  assumed to be parallel to the horizontal of the camera at a fixed distance
  d below the camera*/
class CameraTrans : public Base {
public:

protected:
	float m_y_pixH;/*max y coordinate in pix accross screen*/
	float m_x_pixW;/*max x coordinate in pix accross screen*/
	float m_ang_openingH;/*camera vertical opening max angle in radians for entire screen*/
	float m_ang_openingW;/* "     horizontal "                                        "  */

	float m_y_horizion;/* y coord of screen where horizion should be*/

	/*set in init*/
	float m_ang_mid;/*middle angle for camera where horizon crosses*/
	float m_screen_yToAng;/*scale factor multiplies y dist below horizon to convert it to an angle in radians*/
	float m_screen_xToAng;/*scale factor that covers x dist in screen to radians*/

	bool screenToDriveplane_noOffset(const s_2pt& screenXY, s_2pt& XY); /*takes screen pix coord in xy and converts
															   to coordinates on the drive plane but with the
															   plane y zero staring at the closest point 
															   the camera can see 
															   */
};

#endif

bool CameraTrans::screenToDriveplane_noOffset(const s_2pt& screenXY, s_2pt& XY) {
	/*y starts at top so angle from top of screen*/
	float yScdown = m_y_horizion - screenXY.x1;
	if (yScdown <= 0.f)
		return false;
	float yScAng = m_screen_yToAng * yScdown;

	/*ignore flat vs curved screen*/

}