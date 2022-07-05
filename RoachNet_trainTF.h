#pragma once
#ifndef ROACHNET_TRAINTF_H
#define ROACHNET_TRAINTF_H

#ifndef STAMPEYE_H
#include "StampEye.h"
#endif

#ifndef PARSETXT_H
#include "ParseTxt.h"
#endif

#define ROACHNET_TRAINTF_INFNAME "TFtrained.csv"
#define ROACHNET_TRAINTF_OUTFNAME "TFstamps.csv"
#define ROACHNET_TRAINTF_NUMTRAILVALS 4

class RoachNet_trainTF : public Base
{
public:
	RoachNet_trainTF();
	~RoachNet_trainTF();

	unsigned char init(
		int frame_width=640,
		int frame_height=480
	);
	void release();

	unsigned char gen();
protected:
	/* owned */
	Img* m_imgLow;
	Hex* m_hexLow;
	PatternLuna* m_patLuna;
	StampEye* m_stampEye;
	ParseTxt* m_parse;
	/*       */
	int m_frame_width;
	int m_frame_height;
};
#endif