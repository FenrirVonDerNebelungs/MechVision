#pragma once
#ifndef NNETDUMP_H
#define NNETDUMP_H

#ifndef PARSETXT_H
#include "ParseTxt.h"
#endif

#define NNETDUMPMAX 800000
class NNetDump : public Base {
public:
	NNetDump();
	~NNetDump();
	unsigned char init(
		bool do_dump = true,
		bool do_per_step_dump = false,
		int  q = 0,
		bool do_final_dump = true,
		int sel_node_index=0
	);
	void release();

	inline void resetDump() { m_dump_len = 0; }
	void writeDumpLineQ(int nX, long step_cnt, float E, int q, float Es_q, float DeltaEs_q[], float steps[], int step_red[], float w[], float x[], float y);
	inline void setDumpNodeIndx(int i) { m_node_index = i; }
	void writeDumpFinalLine(int nX, int node_i, bool converged, long step_cnt, float E, float w[], long step_rev[], long step_red[]);
	void writeDump(int marker_i);
protected:
	/*owned*/
	ParseTxt* m_parse;

	bool m_do_dump;
	bool m_do_per_step_dump;
	int m_selq;
	bool m_do_final_dump;
	int m_sel_node_index;

	int m_node_index;

	s_datLine m_dump[NNETDUMPMAX];
	long m_dump_len;
};
#endif
