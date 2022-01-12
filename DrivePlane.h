#pragma once
#ifndef DRIVEPLANE_H
#define DRIVEPLANE_H
#ifndef CAMERATRANS_H
#include "CameraTrans.h"
#endif
#ifndef LINEFINDER_H
#include "LineFinder.h"
#endif

#define DRIVEPLANE_NUMLUNALINE 6
#define DRIVEPLANE_MAXPTS_X_HEIGHT_FAC 1
#define DRIVEPLANE_MOREMAXDIM 9999.f
#define DRIVEPLANE_MAXLOOPCATCH 100
#define DRIVEPLANE_NOLINEHEXVALUE 0.f
struct s_DrivePlate {
	/*not owned input*/
	s_line** screen_lines; /*line pointers are owned but not the lines*/
	int      screen_n_lines;/*number of lines on screen*/

	/*output*/
	int      maxLinePts;/*max allowed points for a plane line in memory*/
	s_line   lines[LINEFINDERMAXLINES];/*the lines coverted to almost robot coord on plane but 
									    camera centerd coordinates with camera_d as the unit distance */
	int      n_lines;/*number of converted lines on the plate*/
	s_hexPlate p;/*hex plate of the drive plane with the converted lines on it*/
};
namespace n_DrivePlate {
	void zeroPlate(s_DrivePlate& p);
}
/* Drive plane is designed to take the lines from lineFinder and project them onto
 a plate (or set of plates) that represent the flat area the 'drive plane' in front
 of the robot 
 */
class DrivePlane : public Base {
public:
	DrivePlane();
	~DrivePlane();

	unsigned char init(
		LineFinder* lineF,
		/*variables put into the cameraTrans*/
		float yPinHole_screenLowPt = -1.f, /*dimmensions in cm distance from camera end of closest plane point
										    if -1 compute from camera opening angle */
		float camera_openingAngle = 0.85172067497f,/*if not zero or less use this to compute camera dim, otherwise use scree low point*/
		float camera_d = 3.f, /*distance camera is above drive plane in cm*/
		float camera_y = 10.f,/*distance end of camera is from center of robot in forward 'y' direction */
		float screen_y_horizion_offset = 0.f,/*offset for horizontal of screen horizion in pix*/
		float screen_x_center_offset = 0.f, /*offset from center of screen in pix of camera center*/
		/*                                  */
		float screenHYDim = 20.f /*determines zoom
								   if less than 1 then reset it to 5*the camera dist */
	);
	unsigned char setPlateForwardSpan(float plateSpanH);/*sets how far in robot coordinates the plate 
											   projects from its closest visible dist forward of the robot.
											   or how forward tall the plate is in robot coordinates*/
	void release();

	unsigned char update();

	inline s_hexPlate* getHexPlate(int i) { return &(m_plates[i].p); }
protected:
	float m_screenClosestY;/*distance from camera center in y to point closest viewable to the camera on the plane*/
	float m_screenClosestY_Unit_d;/*same as above but with the distance of the camera above the plane set to the unit distance for y
								    subtract 1 from this for a buffer*/
	float m_screenHYDim;/*determines zoom of image, how far in outer robot coord the top of the proj region is from the bottom, in this case used to scale all plates*/
	/*plates are assumed to start at closest visible distance */
	/*mixed, owned/not owned*/
	s_DrivePlate m_plates[DRIVEPLANE_NUMLUNALINE];/*plates will correspond to each single luna collection of lines*/
	/*not owned*/
	LineFinder* m_lineFinder;
	/*owned    */
	CameraTrans* m_cameraTrans;
	float m_plateDimToPix;    /*scale factor that scale the dimension of the
							real distance (normalized so that camera_d =1)
							on the drive plane to the pix dim of
							the plate representing
							the plane seen by the camera */
	float m_XcenterPix;/*center of x in pixel coordinates*/
	unsigned char initDrivePlateHexPlate(const s_hexPlate& dim_plate, s_hexPlate& p);/*plate dim will be the pixel dim of the plate*/
	void releaseDrivePlateHexPlate(s_hexPlate& p);
	void reset();
	void resetDriveHexPlate(s_hexPlate& p);

	bool loadLinesByLuna();/*split up lines by luna and load them into the drive plates*/
	unsigned char convertLines(s_DrivePlate& dp);
	inline bool screenLineCoordToPlaneCoord(const s_2pt& screenXY, s_2pt& planeXY) { return m_cameraTrans->drivePlaneCoordFast(screenXY, planeXY); }


	bool fillPlateHexSpotty(int plate_i, float scaleFac);/*assume the lines have already been converted to drive coord*/
	bool LineLocToPlateLoc(float scaleFac, const s_2pt& lineXY, s_2pt& plateXY);
	bool fillHex(const s_2pt& plateXY, float line_o, s_hexPlate& p);

	bool fillPlateHex(int plate_i, float scaleFac);/*fills plate with converted lines extrapolating between line points for continous lines*/
	bool fillLineBetweenPts(float scaleFac, const s_linePoint& pt1, const s_linePoint& pt2, s_hexPlate& p);
	float findUsForLinePts(float R, const s_2pt& pt1, const s_2pt& pt2, s_2pt& Uline, s_2pt& Uline_perp);
	int findWebArcStartForLine(s_hexPlate& p, const s_2pt& Uline);/*Uline is unit vector in direction of line*/
	long findNextHexToFill(s_hexPlate& p, long hexi, int hexweb_start, const s_2pt& lineStart, const s_2pt& line_perp);/*rotates around the current hex and picks the next hex the line is considered to enter*/
	float distHexRough(const s_2pt& lineStart, const s_2pt& line_perp, const s_2pt& hexXY);
	inline bool inHexRough(const s_hexPlate& p, float d) { return d < p.m_Rhex; }

	/*utility*/
	void copyLinePts(const s_linePoint& p1, s_linePoint& p2);/*copies everything except loc*/
};

#endif



