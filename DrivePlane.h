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
	float m_screenClosestY_Unit_d;/*same as above but with the distance of the camera above the plane set to the unit distance for y
								    subtract 1 from this for a buffer*/
	float m_plateXcenter;/*center of hex plate that image is being projected onto*/
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
	int          m_maxLinePts;/*max allowed points for a plane line in memory*/
	s_line       m_planelines[LINEFINDERMAXLINES];/*the lines coverted by into camera centerd coordinates with camera_d as the unit distance */
	int          m_n_planelines;


	unsigned char convertLines();
	inline bool screenLineCoordToPlaneCoord(const s_2pt& screenXY, s_2pt& planeXY) { return m_cameraTrans->drivePlaneCoordFast(screenXY, planeXY); }

	unsigned char transLineToPlate(float scaleFac, s_hexPlate& plate, const s_line* pline);
	bool linePtToHexCoord(float scaleFac, const s_linePoint& linePt, const s_hexPlate& plate, s_2pt_i& ij);
	bool LineLocToPlateLoc(float scaleFac, const s_2pt& lineXY, s_2pt& plateXY);

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
bool DrivePlane::linePtToHexCoord(float scaleFac, const s_linePoint& linePt, const s_hexPlate& plate, s_2pt_i& ij) {
	s_2pt plateXY;
	if (!LineLocToPlateLoc(scaleFac, linePt.loc, plateXY))
		return false;
	plateXY.x1 = plate.m_height - 1 - plateXY.x1;
	if (plateXY.x1 < 0.f)
		return false;
	plateXY.x0 += m_plateXcenter;/*check bounds after rounded*/
	/*the i, j of a hex on a plate are essentially the same as its location on the plate*/
	long plateX = (long)roundf(plateXY.x0);
	if (plateX < 0 || plateX >= plate.m_width)
		return false;
	long plateY = (long)roundf(plateXY.x1);
	ij.x0 = plateX;
	ij.x1 = plateY;
	return true;
}
unsigned char DrivePlane::transLineToPlate(float scaleFac, s_hexPlate& plate, const s_line* pline)
{

}