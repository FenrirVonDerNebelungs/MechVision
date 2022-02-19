#include "DriveLines.h"
DriveLines::DriveLines() :m_drivePlate_i(0), m_useLineR(false), m_useLineL(false), m_minSegCos(0.f), m_minSegStraightLen(0.f),
m_minLinePts(0), m_NsegPts(0), m_maxDev(0.f), m_targetLineOffset(0.f)
{
	m_boxCorner.x0 = 0.f;
	m_boxCorner.x1 = 0.f;
	m_robotDriveCenter.x0 = 0.f;
	m_robotDriveCenter.x1 = 0.f;
}
DriveLines::~DriveLines() {
	;
}
unsigned char DriveLines::init(
	DrivePlane* driveP,
	float targetLineOffset,
	int drivePlate_i,
	bool useLineR,
	bool useLineL,
	float minSegCos,
	float minSegStraightLen_cm,
	int minLinePts,
	int NsegPts,
	float maxDev_cm,
	float boxCorner_Y_cm,
	float boxCorner_X_cm,
	float robotDriveCenter_cm 
) {
	m_drivePlane = driveP;
	float camera_d = driveP->getCamera_d();
	if (camera_d <= 0.f)
		return ECODE_ABORT;
	m_targetLineOffset = targetLineOffset / camera_d;
	m_useLineR = useLineR;
	m_useLineL = useLineL;
	m_minSegCos = minSegCos;
	m_minSegStraightLen = minSegStraightLen_cm / camera_d;
	m_minLinePts = minLinePts;
	m_NsegPts = NsegPts;
	m_maxDev = maxDev_cm / camera_d;
	m_boxCorner.x0 = boxCorner_X_cm / camera_d;
	m_boxCorner.x1 = boxCorner_Y_cm / camera_d;
	m_robotDriveCenter.x1 = robotDriveCenter_cm / camera_d;
	m_robotDriveCenter.x0 = 0.f;
	return ECODE_OK;
}
void DriveLines::release() {
	m_drivePlane = NULL;
}
bool DriveLines::steerTargetDistAng(float& targ, float& ang) {
	s_DrivePlate drivePlate = m_drivePlane->getPlate(m_drivePlate_i);

	s_2pt perp_seg;
	int start_i = -1;
	int line_i = findDriveLine(perp_seg, start_i);
	if (line_i < 0)
		return false;
	/*find the distance  between the steer point and the line*/
	s_2pt vecToSteer = vecMath::v12(drivePlate.lines[line_i].pts[start_i].loc, m_robotDriveCenter);
	float proj_vecToSteer = vecMath::dot(vecToSteer, perp_seg);/*perp seg is the right rot so for the right this points in
																 and for the left it points out*/
	if (m_useLineR)
		targ = m_targetLineOffset - proj_vecToSteer;
	else if (m_useLineL)
		targ = -(m_targetLineOffset + proj_vecToSteer);
	/*y component of perp is sin of angle between the robot dir and the line seg*/
	ang = asinf(perp_seg.x1);
	return true;
}
int DriveLines::findDriveLine(s_2pt& perp_seg, int& start_i) {
	s_DrivePlate drivePlate = m_drivePlane->getPlate(m_drivePlate_i);
	int line_i = -1;
	for (int i = 0; i < drivePlate.n_lines; i++) {
		if (m_useLineR) {
			line_i = findLineR(drivePlate.lines[i], perp_seg, start_i);
			if (line_i >= 0)
				break;
		}
		else if (m_useLineL) {
			line_i = findLineL(drivePlate.lines[i], perp_seg, start_i);
			if (line_i >= 0)
				break;
		}
	}
	return line_i;
}
int DriveLines::findLineR(const s_line& curline, s_2pt& perpUR, int& start_i) {
	if (curline.n < m_minLinePts)
		return -1;/*if too few points then don't need*/
	/*find out it line is in zone*/
	int line_i = 0;
	bool found = false;
	do {
		s_linePoint& curpt = curline.pts[line_i];
		if (curpt.loc.x0 > m_boxCorner.x0 && curpt.loc.x1 < m_boxCorner.x1) {
			found = true;
			break;
		}
		line_i++;
	} while (line_i < curline.n);
	if (!found)
		return -1;
	start_i = line_i;
	return goodSeg(curline, start_i, perpUR);
}
int DriveLines::findLineL(const s_line& curline, s_2pt& perpUR, int& start_i) {
	if (curline.n < m_minLinePts)
		return -1;/*if too few points then don't need*/
	/*find out it line is in zone*/
	int line_i = 0;
	bool found = false;
	do {
		s_linePoint& curpt = curline.pts[line_i];
		if (curpt.loc.x0 < -m_boxCorner.x0 && curpt.loc.x1 < m_boxCorner.x1) {
			found = true;
			break;
		}
		line_i++;
	} while (line_i < curline.n);
	if (!found)
		return -1;
	start_i = line_i;
	return goodSeg(curline, start_i, perpUR);
}
int DriveLines::goodSeg(const s_line& curline, int start_i, s_2pt& perpUR) {
	int line_i = start_i;

	/*find if there are enough remaining points*/
	int total_pts = curline.n - line_i;
	if (total_pts < m_minLinePts)
		return -1;
	/*found will be true currently
	 * check for curvature/jitter deviation
	 */
	 /*find the line that extends from the start of the segment to the end of the segment*/
	int end_i = line_i + m_minLinePts - 1;
	s_2pt v_seg = vecMath::v12(curline.pts[start_i].loc, curline.pts[end_i].loc);
	/*if segment is too short this is not a good line*/
	float len_seg = vecMath::len(v_seg);
	if (len_seg < m_minSegStraightLen)
		return -1;
	/*if segment deviates too much from the forward direction it is not a good line*/
	v_seg.x0 /= len_seg;
	v_seg.x1 /= len_seg;
	if (v_seg.x1 < m_minSegCos)
		return -1;
	/*find the unit perp to this line*/
	s_2pt perp_seg = vecMath::perpUR(v_seg);
	/*now find how far the points deviate from this line*/
	line_i = start_i + 1;
	do {
		s_2pt dev_seg = vecMath::v12(curline.pts[start_i].loc, curline.pts[line_i].loc);
		float dev = vecMath::dot(dev_seg, perp_seg);
		if (dev > m_maxDev)
			return -1;
		line_i++;
	} while (line_i <= end_i);
	perpUR = perp_seg;
	return start_i;
}
