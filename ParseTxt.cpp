#include "ParseTxt.h"

namespace n_datLine {
	void clear(s_datLine& dl) {
		for (int i = 0; i < PARSETXT_MAXAR; i++)
			dl.v[i] = 0.f;
		dl.n = 0;
	}	
}

ParseTxt::ParseTxt() { ; }
ParseTxt::~ParseTxt() { ; }

unsigned char ParseTxt::init(const string& inFile, const string& outFile) {
	m_inFile = inFile;
	m_outFile = outFile;
	m_header.clear();
	return ECODE_OK;
}

void ParseTxt::release() {
	;
}

int ParseTxt::readCSV(s_datLine dat[], int maxSize) {
	ifstream ffile(m_inFile);
	string line;
	int lcnt = 0;
	int arlen = 0;
	int lines_read = 0;
	while (getline(ffile,line) && lcnt<maxSize && arlen>0) {
		arlen = readFloatLine(line, dat[lcnt].v);
		dat[lcnt].n = arlen;
		lines_read++;
	};
	ffile.close();
	return lines_read;
}
unsigned char ParseTxt::writeCSVHeader(std::string& headerStr) {
	m_header=headerStr;
	return ECODE_OK;
}
unsigned char ParseTxt::writeCSVwithSpacer(int marker_i, const s_datLine dat[], int dat_size) {
	ofstream ffile;
	ffile.open(m_outFile, std::ios_base::out | std::ios_base::app);
	ffile << " ---------------------------------- \n";
	ffile << marker_i << "\n----\n";
	if (m_header.size() > 1)
		ffile << m_header;
	for (int i = 0; i < dat_size; i++) {
		string line = dumpFloatLine(dat[i].v, dat[i].n);
		ffile << line << '\n';
	}
	ffile << " ___________________________________ \n";
	ffile.close();
	return ECODE_OK;
}
unsigned char ParseTxt::writeCSV(const s_datLine dat[], int dat_size) {
	ofstream ffile;
	ffile.open(m_outFile);
	for (int i = 0; i < dat_size; i++) {
		string line = dumpFloatLine(dat[i].v, dat[i].n);
		ffile << line << '\n';
	}
	ffile.close();
	return ECODE_OK;
}

string ParseTxt::dumpFloatLine(const float* ar, int len) {
	string outStr = "";
	for (int i = 0; i < len; i++) {
		string el(to_string(ar[i]));
		outStr += el;
		outStr += ",";
	}
	return outStr;
}

int ParseTxt::readFloatLine(const string& str, float* ar) {
	string commaStr = ",";
	size_t foundLoc = str.find(commaStr);
	size_t startLoc = 0;
	int arLen = 0;
	while (foundLoc != string::npos) {
		size_t len = foundLoc - startLoc;
		if (len <= 0)
			break;
		startLoc = foundLoc + 1;
		string numStr = str.substr(startLoc, len);
		ar[arLen] = stof(numStr);
		arLen++;
		if (arLen >= PARSETXT_MAXAR)
			return -1;
		foundLoc = str.find(commaStr, startLoc);
	};
	return arLen;
}