#pragma once
#ifndef CAMERATRANS_H
#define CAMERATRANS_H
#ifndef BASE_H
#include "Base.h"
#endif
#ifndef PATTERNNODE_H
#include "PatternNode.h"
#endif
/*designed to convert screen coordinates to locations on a drive plane that is
  assumed to be parallel to the horizontal of the camera at a fixed distance
  d below the camera

  make the aproximation of a pinhole camera with a screen that is aproximately verticle 
  */
class CameraTrans : public Base {
public:
	CameraTrans();
	~CameraTrans();

	unsigned char init(
		s_hexPlate& plate,
		float yPinHole_screenLowPt = 10.f, /*dimmensions in cm distance from camera end of closest plane point*/
		float camera_d = 7.f, /*distance camera is above drive plane in cm*/
		float camera_y = 10.f,/*distance end of camera is from center of robot in forward 'y' direction */
		float screen_y_horizion_offset = 0.f,/*offset for horizontal of screen horizion in pix*/
		float screen_x_center_offset = 0.f /*offset from center of screen in pix of camera center*/
	);
	/*h opening angle raspberry pi 0.85172067497, 48.8 degrees for 480 H, 640 62.2degrees*/
	unsigned char setFocalFromOpeningAngle(float openingAngleH);/*use the vertical opening angle (in rad) of the camera to reset focal lenght in pix*/
	void release();

	bool drivePlaneCoord(const s_2pt& screenCoord, s_2pt& planeCoord);
	inline bool drivePlaneCoordCameraCent(const s_2pt& screenCoord, s_2pt& planeCoord) { return screenToDriveplane(screenCoord, planeCoord); }
	inline bool drivePlaneCoordFast(const s_2pt& screenCoord, s_2pt& planeCoord) { return screenToDriveplane_Unit_d(screenCoord, planeCoord); }
	inline void convFastCoordToCoord(s_2pt& XY) { XY.x0 *= m_camera_d; XY.x1 *= m_camera_d; }
	inline float getScreenLowPt() { return m_yPinHole_screenLowPt; }
	inline float getCamera_d() { return m_camera_d; }
	inline float getfPix() { return m_f_pix; }/*focal length in pixels*/
	inline float getScreen_y_horizion() { return m_screen_y_horizion; }
protected:
	float m_yPinHole_screenLowPt;/*dist in y from pin hole camera of closest point viewable by camera in drive plane*/
	float m_camera_d;/*height of camera off the drive plane*/
	float m_y_pixH;/*max y coordinate in pix accross screen*/
	float m_x_pixW;/*max x coordinate in pix accross screen*/
	float m_ang_openingH;/*camera vertical opening max angle in radians for entire screen*/
	float m_ang_openingW;/* "     horizontal "                                        "  */
	
	float m_screen_y_horizion;/* y coord in pix of screen where horizion should be*/
	float m_screen_x_center;/*x  coord in pix of center of screen */

	float m_camera_y;/*distance of pin hole camera center in y from what is considered the 
					   center of the robot
					   */
	/*set in init*/
	float m_f_pix;/* focal length in pix dim*/

	bool screenToDriveplane(const s_2pt& screenXY, s_2pt& XY); /*takes screen pix coord in xy and converts
															   to coordinates on the drive plane but with the
															   plane y zero starting at the center in y of the 
															   'pinhole' camera
															   */
	bool screenToDriveplane_Unit_d(const s_2pt& screenXY, s_2pt& XY);/*same as above but all values must be multiplied
																	  by d to get the correct distances, 
																	  camera_d is effectively set to the unit distance by this
																	  function */
};

#endif

