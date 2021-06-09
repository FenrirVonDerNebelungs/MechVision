#pragma once
#ifndef PATTERNDRAW_H
#define PATTERNDRAW_H

#ifndef LANELINEFINDER_H
#include "LaneLineFinder.h"
#endif

class PatternDraw {
public:
	PatternDraw();
	~PatternDraw();

	unsigned char init(PatternScan* patScan, LineFinder* linefinder=NULL, LaneLineFinder* lanelinefind=NULL);
	void release();

	unsigned char run();


	inline Img* getImg() { return m_imgOut; }
protected:
	/*not owned*/
	HexStack* m_hexStack;
	PatternScan* m_patScan;
	LineFinder* m_lineFinder;
	LaneLineFinder* m_laneLineFinder;
	/*         */
	Img* m_imgOut;

	void drawFeetfs();
	void drawEdgeStrength();
	void drawMaxEdgeStrengthAbove(float minf=0.f);

	void convFloatToGrayScale(float f, s_rgba& grayCol);

	/*draw lines*/
	void drawLines();
	void drawLaneLines();
	void drawLine(s_line& line);
	/*          */
};
#endif


