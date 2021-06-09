#pragma once
#ifndef HOKHEX_H
#define HOKHEX_H

#ifndef HEX_H
#include "Hex.h"
#endif

class HokHex : public HexBase
{
public:
	HokHex();
	~HokHex();
	unsigned char Init(
		HexBase* pLowHex
	);
	void Release();
	unsigned char Update(Img* img);
	unsigned char Run();

private:
	s_hex* m_lowHex;/*not owned*/
	int    m_nlowHex;

	unsigned char genMesh();

	unsigned char genHexes();
	unsigned char genWebHiLo();

	/*helper to genMesh*/
	unsigned char genXweb(int first_hex_i);
	
	/*helpers to genWebHiLo*/
	int findFirstHex(int linestart_i);

	unsigned char zipForwardDown(int& top_hex, int& cur_hex);
	unsigned char zipBackDown(int& top_hex, int& cur_hex);
	/**helpers to zip...Down...**/
	bool isForwardDownLinked(int& top_hex, int& cur_hex);
	bool isBackDownLinked(int& top_hex, int& cur_hex);

	unsigned char aveDownLinkedCols(int web_i, float center_frac=1.f);/*takes the hex at web_i 
																			   and averages all seven down linked hexes
																			   uses center fraction to determine 
																			   how much relative weight should be 
																			   given to the center hex*/
};
#endif
