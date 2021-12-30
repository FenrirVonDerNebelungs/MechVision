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


	unsigned char convertLines();
	inline bool screenLineCoordToPlaneCoord(const s_2pt& screenXY, s_2pt& planeXY) { return m_cameraTrans->drivePlaneCoordFast(screenXY, planeXY); }



	bool LineLocToPlateLoc(float scaleFac, const s_2pt& lineXY, s_2pt& plateXY);


	bool PlateLocToHexCoord(const s_hexPlate& p, const s_2pt& plateXY, s_2pt_i& ij);
	bool fillHex(const s_2pt& plateXY, s_hexPlate& p);

	bool fillPlateHexSpotty(int plate_i, float scaleFac);

	/*utility*/
	void copyLinePts(const s_linePoint& p1, s_linePoint& p2);
};

#endif

void copyLinePts(const s_linePoint& p1, s_linePoint& p2) {
	p2.o = p1.o;
	p2.lunai = p1.lunai;
	p2.hexi = p1.hexi;
	p2.linei = p1.linei;
	p2.v = p1.v;
	p2.perp = p1.perp;
}
unsigned char DrivePlane::convertLines() {
	m_n_planelines = 0;
	for (int i = 0; i < m_n_lines; i++) {
		s_line* linep = m_lines[i];
		s_line& planeLine = m_planelines[m_n_planelines];
		bool lineAdded = false;
		int j_planeLine = 0;
		for (int j = 0; j < linep->n; j++) {
			if (screenLineCoordToPlaneCoord(linep->pts[j].loc, planeLine.pts[j_planeLine].loc)) {
				lineAdded = true;
				copyLinePts(linep->pts[j], planeLine[j_planeLine]);
				j_planeLine++;
				if (j_planeLine >= m_maxLinePts)
					break;
			}
			else if (lineAdded) /*if line exits allowed range it is not allowed to enter again*/
				break;
		}
		if (lineAdded) {
			m_n_planelines++;
		}
	}
}

bool DrivePlane::LineLocToPlateLoc(float scaleFac, const s_2pt& lineXY, s_2pt& plateXY) {
	plateXY.x1 = lineXY.x1 - m_screenClosestY_Unit_d;
	if (plateXY.x1 < 0.f)
		return false;
	plateXY.x0 = scaleFac * lineXY.x0;
	plateXY.x1 *=scaleFac;
	return true;
}
bool DrivePlane::PlateLocToHexCoord(const s_hexPlate& p, const s_2pt& plateXY, s_2pt_i& ij) {
	/*plateXY.x1 will be >= 0*/
	ij.x1 = roundf(plateXY.x1);
	if (ij.x1 >= p.m_height)
		return false;
	ij.x1 = p.m_height - 1 - ij.x1;
	ij.x0 = roundf(plateXY.x0) + p.m_width / 2;
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
			if (LineLocToPlateLoc(scaleFac, pt.loc, plateXY)) {
				fillHex(plateXY, dp.p);
			}
		}
	}
	return true;
}
