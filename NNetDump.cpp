#include "NNetDump.h"
NNetDump::NNetDump() : 
	m_parse(NULL), m_do_dump(false), m_do_per_step_dump(false), 
	m_selq(0), m_do_final_dump(false), m_sel_node_index(0), m_node_index(0), 
	m_dump_len(0)
{
	for (int i = 0; i < NNETDUMPMAX; i++) {
		n_datLine::clear(m_dump[i]);
	}
}
NNetDump::~NNetDump() {
	;
}
unsigned char NNetDump::init(
	bool do_dump,
	bool do_per_step_dump,
	int  q,
	bool do_final_dump,
	int sel_node_index
) {
	m_do_dump = do_dump;
	m_do_per_step_dump = do_per_step_dump;
	m_selq = q;
	m_do_final_dump = do_final_dump;
	m_sel_node_index = sel_node_index;
	m_parse = new ParseTxt;
	std::string inF = "in.csv";
	std::string outF = "dDump/trainDump.csv";
	m_parse->init(inF, outF);
	m_dump_len = 0;
	return ECODE_OK;
}
void NNetDump::release() {
	m_dump_len = 0; 
	if (m_parse != NULL) {
		m_parse->release();
		delete m_parse;
	}
	m_parse = NULL;
}

void NNetDump::writeDumpLineQ(int nX, long step_cnt, float E, int q, float Es_q, float DeltaEs_q[], float steps[], int step_red[], float w[], float x[], float y) {
	if (!m_do_dump || !m_do_per_step_dump || q != m_selq || m_node_index != m_sel_node_index)
		return;
	if (m_dump_len >= NNETDUMPMAX)
		return;
	int dump_i = 0;
	m_dump[m_dump_len].v[dump_i] = (float)step_cnt;
	dump_i++;
	m_dump[m_dump_len].v[dump_i] = E;
	dump_i++;
	m_dump[m_dump_len].v[dump_i] = (float)q;
	dump_i++;
	m_dump[m_dump_len].v[dump_i] = Es_q;
	dump_i++;
	for (int i = 0; i < nX; i++) {
		m_dump[m_dump_len].v[dump_i] = DeltaEs_q[i];
		dump_i++;
	}
	for (int i = 0; i < nX; i++) {
		m_dump[m_dump_len].v[dump_i] = steps[i];
		dump_i++;
	}
	for (int i = 0; i < nX; i++) {
		m_dump[m_dump_len].v[dump_i] = (float)step_red[i];
		dump_i++;
	}
	for (int i = 0; i < nX; i++) {
		m_dump[m_dump_len].v[dump_i] = w[i];
		dump_i++;
	}
	for (int i = 0; i < nX; i++) {
		m_dump[m_dump_len].v[dump_i] = x[i];
		dump_i++;
	}
	m_dump[m_dump_len].v[dump_i] = y;
	dump_i++;
	m_dump[m_dump_len].n = dump_i;
	m_dump_len++;
}
void NNetDump::writeDumpFinalLine(int nX, int node_i, bool converged, long step_cnt, float E, float w[], long step_rev[], long step_red[]) {
	if (m_dump_len >= NNETDUMPMAX)
		return;
	int dump_i = 0;
	m_dump[m_dump_len].v[dump_i] = (float)node_i;
	dump_i++;
	m_dump[m_dump_len].v[dump_i] = (float)converged;
	dump_i++;
	m_dump[m_dump_len].v[dump_i] = (float)step_cnt;
	dump_i++;
	m_dump[m_dump_len].v[dump_i] = E;
	dump_i++;
	for (int i = 0; i < nX; i++) {
		m_dump[m_dump_len].v[dump_i] = w[i];
		dump_i++;
	}
	for (int i = 0; i < nX; i++) {
		m_dump[m_dump_len].v[dump_i] = (float)step_rev[i];
		dump_i++;
	}
	for (int i = 0; i < nX; i++) {
		m_dump[m_dump_len].v[dump_i] = (float)step_red[i];
		dump_i++;
	}
	m_dump[m_dump_len].n = dump_i;
	m_dump_len++;
}
void NNetDump::writeDump(int marker_i) {
	m_parse->writeCSVwithSpacer(marker_i, m_dump, m_dump_len);
	m_dump_len = 0;
}