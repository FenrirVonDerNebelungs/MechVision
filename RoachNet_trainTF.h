#pragma once
#ifndef ROACHNET_TRAINTF_H
#define ROACHNET_TRAINTF_H


#ifndef NNETTRAIN_H
#include "NNetTrain.h"
#endif

#ifndef PARSETXT_H
#include "ParseTxt.h"
#endif

#define ROACHNET_TRAINTF_DEBUG
#define ROACHNET_TRAINTF_DEBUG_NUM 3

#define ROACHNET_TRAINTF_INFNAME "dDump/TFtrained.csv"
#define ROACHNET_TRAINTF_OUTFNAME "dDump/TFstamps.csv"
#define ROACHNET_TRAINTF_NUMTRAILVALS 3
#define ROACHNET_TRAINTF_LOWESTLEVNUMHEX 7

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

	unsigned char gen();/*dumps result as (stamp index), (X's lowest hanging nodes --num = num lowest hexes x lunas-- ), 
						(pre train weights for highest node--num = num lowest hexes), (pre train weights for hanging nodes X's) (3 values ang, center_ang, radius) */
	unsigned char setTrainedNets(HexEye* netEyes);/*netEyes should be an object that exists but has not been initialized 
										    sets up the net eyes so that each of the seven base nodes has connections to all the  hanging nodes, each of the seven
										    lowest plate nodes is one of the nodes in the hidden layer
											the top node is the final output node which has weighted connections to the hidden layer nodes
											the geometry does not necessarly match exactly, as in the training network could have trained the hidden layer nodes
											to match just about anything
										  assumes the data is in the format (stamp index), (weights from hidden layer to final node), b final node, (weights from lower hanging nodes to 1st hidden layer), (b's for each of the hidden nodes)  */
protected:
	/* owned */
	Img* m_imgLow;
	Hex* m_hexLow;
	PatternLuna* m_patLuna;
	StampEye* m_stampEye;
	ParseTxt* m_parse;
	s_datLine m_datLines[STAMPEYENUM];
	int       m_numDatLines;
	HexEye* m_NNetsPreTrained;
	EyeNetTrain* m_preTrain;
	/*       */
	int m_frame_width;
	int m_frame_height;

	float m_lowestNetR;
	int m_lowestNetLevel;
	int m_numLowestHex;
	int m_numHangingPerHex;/*number hanging in original simple NNet not the total number of hanging*/
	int m_numNNetLowestXs;/*total number hanging*/
	int m_numNNetLineVals;/*number of input values expected on one line of the dat line for the nnet*/

	unsigned char genDatLines();
	unsigned char preTrain();

	unsigned char setNetDim();
	unsigned char initHexEyes(HexEye* netEyes);

	unsigned char getDatLines();
	unsigned char setTrainedNet(int i_net, HexEye* netEyes);
#ifdef NNETTRAIN_DUMP
	void writeDebugNNetTrainLines(int net_i);
#endif
};
#endif