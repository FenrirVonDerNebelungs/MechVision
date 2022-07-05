#pragma once
#ifndef PARSETXT_H
#define PARSETXT_H
#ifndef BASE_H
#include "Base.h"
#endif

#define PARSETXT_MAXAR 80
using namespace std;

struct s_datLine {
	float v[PARSETXT_MAXAR];
	int n;
};
namespace n_datLine {
	void clear(s_datLine& dl);
}

class ParseTxt : public Base {
public:
	ParseTxt();
	~ParseTxt();

	unsigned char init(
		const string& inFile, 
		const string& outFile);
	void release() { ; }

	unsigned char readCSV(s_datLine dat[], int maxSize=5000);
	unsigned char writeCSV(const s_datLine dat[], int dat_size);
protected:
	string m_inFile;
	string m_outFile;

	string dumpFloatLine(const float* ar, int len);

	int readFloatLine(const string& str, float* ar);
};
#endif