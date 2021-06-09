#pragma once
#ifndef LANELINEFINDER_H
#define LANELINEFINDER_H

#ifndef LINEFINDER_H
#include "LineFinder.h"
#endif


class LaneLineFinder {
public:
	LaneLineFinder();
	~LaneLineFinder();

	unsigned char init(
		LineFinder* lineFinder,
		Img*  img,
		float ZoneTopFrac = 0.2f,
		int minNumLaneLinePts = 6,
		float fracForMostlyOut = 0.7f,
		int   maxPtsToSearchForZipStart = 10,
		float maxZipStartLineWidth = 40.f,
		float maxZipLineWidth = 60.f,
		float minNumZipMatched = 6.f,
		float fracForZipMatch = 0.8f
	);
	void release();

	unsigned char spawn();

	inline int getNLines() { return m_nZippedLines; }
	inline s_line& getLine(int i) { return m_zippedLines[i]; }
protected:
	/*not owned*/
	LineFinder* m_rawLineFinder;
	s_line* m_rawlines;
	int m_nraw;
	/*         */
	float m_maxImgDim;
	float m_ZoneTopFrac;/*frac of total image heigth for zone extending from bottom of image where lines must enter to be considered lane canidates*/
	int m_minNumLaneLinePts;
	float m_fracForMostlyOut;/*fraction of points that indicate that the edge is going from black to white for the line to be considered 'out'*/


	s_2pt m_laneCandZoneTopMid;

	int   m_linesFoundIs[LINEFINDERMAXLINES];
	bool  m_lineMostlyOut[LINEFINDERMAXLINES];
	bool  m_ForwardLine[LINEFINDERMAXLINES];
	int   m_numLinesFound;

	s_line m_zippedLines[LINEFINDERMAXLINES];/*outward facing (black to white from center) lines that are in the bottom section of the screen and have a zip match*/
	int   m_nZippedLines;

	int   m_maxPtsToSearchForZipStart;
	float m_maxZipStartLineWidth;
	float m_maxZipLineWidth;
	float m_minNumZipMatched;/*min number of total matched points during zip to consider it a good zip*/
	float m_fracForZipMatch;/*fraction of points that have to be oriented white to black leaving 'matching' line for line to be considered a zip match to 'out' line*/

	void reset();

	unsigned char findZoneLines();
	bool zoneLine(s_line& rawLine, bool& mostlyOut, bool& forwardDir);
	bool isWhiteOut(s_linePoint& pt);

	unsigned char zipLines();
	int isZipped(s_line& lineSide, bool lineForward, int& i_zip, int& i_side);/*returns line index that goes to rawlines*/
	bool getZipStartPoints(s_line& lineZip, s_line& lineSide, int& i_zip, int& i_side, bool zipForward, bool lineForward);
	bool closestPoints(s_line& lineZip, s_line& lineSide, bool zipForward, bool lineForward, int& i_zip, int& i_side);
	bool doZip(int i_zip, int i_side, s_line& lineZip, s_line& lineSide, bool zipForward, bool lineForward);
	unsigned char addZippedLine(s_line& rawLine, bool isForward, int& i_side);
};

#endif










