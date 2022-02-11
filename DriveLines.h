#pragma once
#ifndef DRIVELINES_H
#define DRIVELINES_H
#ifndef DRIVEPLANE_H
#include "DrivePlane.h"
#endif

class DriveLines : public Base {
public:
	DriveLines();
	~DriveLines();
	unsigned char init(
		DrivePlane* driveP,
		float targetLineOffset = 3.f,
		int drivePlate_i = 0,
		bool useLineR = true,
		bool useLineL = true,
		float minSegCos = 0.80,
		float minSegStraightLen_cm = 0.01f,
		int minLinePts = 9,
		int NsegPts = 9,
		float maxDev_cm = 1.f,
		float boxCorner_Y_cm = 10.f,
		float boxCorner_X_cm = 1.f,
		float robotDriveCenter_cm = 7.f
	);
	void release();
	bool steerTargetDistAng(float& targ, float& ang);/*gives the distance,targ, between the steering target center and the target drive line
													 and the angle between the robot forward and the drive line seg, primary direction
													 positive values for targ when the steering center is to the right of the drive line target 
													 positive vales for ang when rotated to the right(clockwise, left-handed) of the target line direction*/
protected:
	DrivePlane* m_drivePlane;

	int findDriveLine(s_2pt& perp_seg, int& start_i);

	/*these functions return the first point that is in the box*/
	int findLineR(const s_line& curline, s_2pt& perpUR, int& start_i);
	int findLineL(const s_line& curline, s_2pt& perpUR, int& start_i);

	int goodSeg(const s_line& curline, int start_i, s_2pt& perpUR);

	int m_drivePlate_i;/*index of drive plate that will be used to find lines, useful if lines are more than one color*/

	bool m_useLineR;/*use the right line as the guide line*/
	bool m_useLineL;/*use the left line as the guide line*/
	/*currently not configured to use both*/

	float m_minSegCos;/*limits how much of an angle the segment may make with respect to the direction of the robot*/
	float m_minSegStraightLen;/*min distance from the start to the end points of the line segment*/
	int m_minLinePts;
	int m_NsegPts;/*number of points in a seg, used for curvature*/
	float m_maxDev;/*maximum deviation from end-to-end line used to limit curvature or jitter*/
	s_2pt m_boxCorner; /*anything below and to the side of this counts as in the zone
					    coordinates are given in camera centered camera_d normalized */

	s_2pt m_robotDriveCenter;/*the steering point for the robot in camera centered camera_d normalized coord*/
	float m_targetLineOffset;/*offset of drive line from lane line seg*/
};

#endif
