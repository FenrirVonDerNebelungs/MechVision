#pragma once
#ifndef DRIVEPLANE_H
#define DRIVEPLANE_H
#ifndef CAMERATRANS_H
#include "CameraTrans.h"
#endif
#ifndef LINEFINDER_H
#include "LineFinder.h"
#endif
struct s_DrivePlate {
	/*not owned input*/
	s_line** screen_lines; /*not owned, lines in*/
	int      screen_n_lines;/*number of lines on screen*/

	/*output*/
	int      maxLinePts;/*max allowed points for a plane line in memory*/
	s_line   lines[LINEFINDERMAXLINES];/*the lines coverted to almost robot coord on plane but 
									    camera centerd coordinates with camera_d as the unit distance */
	int      n_lines;/*number of converted lines on the plate*/
	s_hexPlate p;/*hex plate of the drive plane with the converted lines on it*/
};
/* Drive plane is designed to take the lines from lineFinder and project them onto
 a plate (or set of plates) that represent the flat area the 'drive plane' in front
 of the robot 
 */
class DrivePlane : public Base {
public:

	unsigned char update();
protected:
	float m_screenClosestY;/*distance from camera center in y to point closest viewable to the camera on the plane*/
	float m_screenClosestY_Unit_d;/*same as above but with the distance of the camera above the plane set to the unit distance for y
								    subtract 1 from this for a buffer*/
	float m_plateXcenter;/*center of hex plate that image is being projected onto*/
	float m_screenHYDim;/*determines zoom of image, how far in outer robot coord the top of the proj region is from the bottom, in this case used to scale all plates*/
	/*plates are assumed to start at closest visible distance */
	/*mixed, owned/not owned*/
	s_DrivePlate plates[MAXPLATESPERLAYER];
	/*owned    */
	CameraTrans* m_cameraTrans;
	float m_plateDimToPix;    /*scale factor that scale the dimension of the
							real distance on the drive plane to the pix dim of
							the plate representing
							the plane seen by the camera */


	unsigned char convertLines(s_DrivePlate& dp);
	inline bool screenLineCoordToPlaneCoord(const s_2pt& screenXY, s_2pt& planeXY) { return m_cameraTrans->drivePlaneCoordFast(screenXY, planeXY); }


	bool fillPlateHexSpotty(int plate_i, float scaleFac);/*assume the lines have already been converted to drive coord*/
	bool LineLocToPlateLoc(float scaleFac, const s_2pt& lineXY, s_2pt& plateXY);
	bool fillHex(const s_2pt& plateXY, float line_o, s_hexPlate& p);



	/*utility*/
	void copyLinePts(const s_linePoint& p1, s_linePoint& p2);/*copies everything except loc*/
};

#endif



bool DrivePlane::LineLocToPlateLoc(float scaleFac, const s_2pt& lineXY, s_2pt& plateXY) {
	plateXY.x1 = lineXY.x1 - m_screenClosestY_Unit_d;
	if (plateXY.x1 < 0.f)
		return false;
	plateXY.x0 = scaleFac * lineXY.x0;
	plateXY.x1 *=scaleFac;
	return true;
}
bool DrivePlane::fillHex(const s_2pt& plateXY, float line_o, s_hexPlate& p) {
	/*find which hex*/
	long hex_i = PatStruct::squarePlate_xyToHexi(p, plateXY);
	if (hex_i < 0)
		return false;
	p.m_fhex[hex_i].o = line_o;
	return true;
}
bool DrivePlane::fillPlateHexSpotty(int plate_i, float scaleFac) {
	s_DrivePlate& dp = plates[plate_i];
	for (int i = 0; i < dp.n_lines; i++) {
		s_line& line = dp.lines[i];
		for (int pt_i = 0; pt_i < line.n; pt_i++) {
			s_linePoint& pt = line.pts[pt_i];
			s_2pt plateXY;
			if (LineLocToPlateLoc(scaleFac, pt.loc, plateXY)) {
				fillHex(plateXY, dp.o, dp.p);
			}
		}
	}
	return true;
}
