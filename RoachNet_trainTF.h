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
#define ROACHNET_TRAINTF_NUMTRAILVALS 3

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

	unsigned char gen();/*dumps result as (stamp index), (lowest plate hex, lowest hanging nodes --each per luna-- for X's), (3 values ang, center_ang, radius) */
	unsigned char getNet(HexEye* netEyes);/*netEyes should be an object that exists but has not been initialized 
										    sets up the net eyes so that each of the seven base nodes has connections to all the  hanging nodes, each of the seven
										    lowest plate nodes is one of the nodes in the hidden layer
											the top node is the final output node which has weighted connections to the hidden layer nodes
											the geometry does not necessarly match exactly, as in the training network could have trained the hidden layer nodes
											to match just about anything
										  assumes the data is in the format (stamp index), (weights from lower hanging nodes to 1st hidden layer), (weights from hidden layer to final node) */
protected:
	/* owned */
	Img* m_imgLow;
	Hex* m_hexLow;
	PatternLuna* m_patLuna;
	StampEye* m_stampEye;
	ParseTxt* m_parse;
	s_datLine m_datLines[STAMPEYENUM];
	int       m_numDatLines;
	/*       */
	int m_frame_width;
	int m_frame_height;

	unsigned char genDatLines();

	unsigned char initHexEyes(HexEye* netEyes);
	unsigned char getDatLines();
};
#endif