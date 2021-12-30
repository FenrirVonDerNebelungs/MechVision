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
	s_line** lines; /*not owned, lines in*/
	int      n_lines;
	s_hexPlate p;
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



	bool screenLocToPlateLoc(float scaleFac, const s_2pt& screenXY, s_2pt& plateXY);
	bool PlateLocToHexCoord(const s_hexPlate& p, const s_2pt& plateXY, s_2pt_i& ij);
	bool fillHex(const s_2pt& plateXY, s_hexPlate& p);

	bool fillPlateHexSpotty(int plate_i, float scaleFac);

	//unsigned char transLineToPlate(float scaleFac, s_hexPlate& plate, const s_line* pline);
    //bool linePtToHex(float scaleFac, const s_linePoint& linePt, s_hexPlate& plate);  
};

#endif

bool DrivePlane::PlateLocToHexCoord(const s_hexPlate& p, const s_2pt& plateXY, s_2pt_i& ij) {
	/*plateXY.x1 will be >= 0*/
	ij.x1 = roundf(plateXY.x1);
	if (ij.x1 >= p.m_height)
		return false;
	ij.x1 = p.m_height - 1 - ij.x1;
	ij.x0 = roundf(plateXY.x0) + p.m_width/2;
	if (ij.x0 < 0 || ij.x0 >= p.m_width)
		return false;
	return true;
}
bool DrivePlane::fillHex(const s_2pt& plateXY, s_hexPlate& p) {
	s_2pt_i ij;
	if (!PlateLocToHexCoord(p, plateXY, ij))
		return false;

}

bool DrivePlane::fillPlateHexSpotty(int plate_i, float scaleFac) {
	s_DrivePlate& dp = plates[plate_i];
	for (int i = 0; i < dp.n_lines; i++) {
		s_line* line = dp.lines[i];
		for (int pt_i = 0; pt_i < line->n; pt_i++) {
			s_linePoint& pt = line->pts[pt_i];
			s_2pt plateXY;
			if (screenLocToPlateLoc(scaleFac, pt.loc, plateXY)) {
				fillHex(plateXY, dp.p);
			}
		}
	}
	return true;
}

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
