#pragma once
#ifndef NNETTRAIN_H
#define NNETTRAIN_H

#ifndef STAMPEYE_H
#include "StampEye.h"
#endif

#ifndef PARSETXT_H
#include "ParseTxt.h"
#endif

#define NNETTRAINMAXDUMP 800000
#define NNETTRAIN_DEBUG

struct s_NNetL1X {
	float* m_x; /*has length the number of input x's */
	int    m_n; /* length of the x vector of inputs */
};
namespace NNet {
	inline float NNetFunc(float x) { return Math::StepFuncSym(x); }
	inline float NNetDFunc(float x) { return Math::DStepFuncSym(x); }/*gives the derivative of the NNetFunc*/

	unsigned char initNNetL1X(int N, s_NNetL1X& X);
	unsigned char initNNetL1Xs(long datasize, int nX, s_NNetL1X X[]);
	void releaseNNetL1X(s_NNetL1X& X);
	void releaseNNetL1Xs(long datasize, s_NNetL1X X[]);

	bool oNNetL0(s_hexEye& net, const s_hexEye& lunaEye);
}

/*
  trains simplest kind of NNet with only one plane that seperates all variables in hyperspace with a simple cut
  this NNet has only one node
 */
class NNetTrain0 : public Base {
public:
	NNetTrain0();
	~NNetTrain0();
	
	unsigned char init(
		float stepSize = 10.,/* eta */
		float DeltaE_closeEnough = 0.01,
		long max_loop_cnt = 1000,
		float init_all_ws=0.f,
		int   max_red=100
	);
	void release();
	unsigned char setNet(long dataSize, s_NNetL1X X[], float y[]);
	
	inline unsigned char run() { return trainNet(); }

	inline float* getWs() {return m_w;}
	inline int    getnX() { return m_nX; }
	inline float* getDeltaEs() { return m_DeltaEs; }
	inline float getE() { return m_E; }
#ifdef NNETTRAIN_DEBUG
	inline void setDump(bool do_dump) { m_do_dump = do_dump; }
	inline s_datLine* getDump() { return m_dump; }
	inline void resetDump() { m_dump_len = 0; }
	inline long getDumpLen() { return m_dump_len; }
	void writeDumpFinalLine(int node_i);
#endif
protected:
	/*owned*/
	float      m_DeltaE_closeEnough_unscaled;
	float      m_DeltaE_closeEnough;/*value of dE/dw that is low enough that the scan is considered to have converged*/
	long       m_max_loop_cnt;
	float      m_w_init;
	float      m_stepSize;
	int        m_max_red;/*maximum number of times the step can reduce for a given w*/

	long       m_step_cnt;
	bool       m_converged;

	s_NNetL1X* m_X;/* this array has length of data q max*/
	int        m_nX;/* m_x.m_n length of input vector of x's */
	float*     m_y;/* has length of data q max*/
	long       m_nData;/*number of samples of data or q max*/
	float*     m_w;/*has length number of inputs x's m_x[].m_n */
	int        m_nNodes;/*number of nodes  m_max*/

	float* m_DeltaEs;/*change in error \delta_ji; j=0 since only one node is being trained at L0 therefore this has the length m_nX*/
	float*  m_steps;/*has same length as w's, since some steps are pos and some are neg*/
	float  m_E; /*current error */
	long* m_step_rev;/*number of times the step has reversed, has same length as steps, w's */
	long* m_step_red;/*number of times the step has been reduced*/

	unsigned char initMem(long dataSize, int nX);
	void          releaseMem();

	void          reset();

	unsigned char trainNet();
	/*helpers to trainNet*/
	unsigned char findDeltaEs();
	bool          updateWs();
	/*** helpers to findDeltaEs*/
	float evalForQth_jk(float y, s_NNetL1X& X, long q, int k);/*eval the inside of the sum over targets 'q's', y is the target value*/
	float evalEForQth_j(float y, s_NNetL1X& X);/* find the sum squared error for 'all', in this case 1, node(s) for a given q*/
	/******* helpers for evalForQth_jk*/
	float sumWs(float X[]);
#ifdef NNETTRAIN_DEBUG
	bool      m_do_dump;
	int       m_selq;
	s_datLine m_dump[NNETTRAINMAXDUMP];
	long      m_dump_len;
	void writeDumpLineQ(int q, float Es_q, float DeltaEs_q[]);
	void DumpNet(int q, float DeltaEs_q[]);
	void DumpNetVerbose(int q);
#endif
};


class EyeNetTrain : public Base {
public:
	EyeNetTrain();
	~EyeNetTrain();

	unsigned char init(
		StampEye* stampEyep,/*stampEye object containing the input data which is the stamps evaluated on the lunas for each of the test patterns*/
		float stepSize=1.0,
		float DeltaE_closeEnough=0.00001,
		long  max_loop_cnt = 10000
	);
	void release();
	inline unsigned char run(s_hexEye* net) { return runL0(net); }/*net has the same structure as the stampe eye's s_hexEye's for the luna output*/
#ifdef NNETTRAIN_DEBUG
	int m_dumpSubNode;/*-1 and dumps high node*/
	ParseTxt* m_parse;
	void writeDump();
#endif
protected:
	int        m_lowestLevel;
	/*owned*/
	NNetTrain0* m_NNetTrain0;

	/*not owned*/
	s_hexEye* m_net;   
	StampEye* m_stampEye; /*Master function array of eye stamps that will act as the 'q' data set*/

	unsigned char runL0(s_hexEye* net);/* runs but trains each node independently so only one node in the net, not a true NNet just one planar cuts per node*/

	unsigned char setDataForNode(int node_i);
	unsigned char getResultsIntoNode(int node_i);

	unsigned char setDataForTopNode();
	unsigned char getResultsIntoTopNode();
};

#endif








