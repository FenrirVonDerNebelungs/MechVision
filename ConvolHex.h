#pragma once
#ifndef CONVOLHEX_H
#define CONVOLHEX_H
#ifndef HEXBASE_H
#include "HexBase.h"
#endif
class ConvolHex {
public:
	ConvolHex();
	~ConvolHex();
	unsigned char Init(Img* img, s_hex hex[], float Rhex, float sigmaVsR, float IMaskRVsR);
	void Release();
	unsigned char convulToHex(int col_i);
protected:
	/*not owned*/
	Img*   m_img;
	s_hex* m_hex;
	/*owned*/
	/*integration to fill hex with col, convulution*/
	Img* m_IMask;

	float m_Rhex;

	float m_sigmaVsR;
	float m_IMaskRVsR;

	float m_sigma;
	float  m_gaussNorm;/*1/sigma*sqrt(2pi)*/
	float  m_gaussExpConst;/*2*sigma^2*/
	float  m_IMaskR;
	s_2pt  m_IMaskCenter;
	s_2pt_i m_IMaskBL_offset;

	/* helpers to init */
	unsigned char genIMask();
	/**helpers to generating Integral mask**/
	float calcGaussian(s_2pt& pt);
	/**helpers to convolsion **/

	unsigned char convulMaskToHex(int col_i);/*uses the gausian with all three cols to set hex to single col*//*only one of the two convul functions will generally be used*/
	//unsigned char convulThreshtoHex(int col_i);/*  */
	/****                     ****/
	bool isIMaskInside(long hi, long hj);/*hi and hj represent the coords of the BL (or UL depending on y axis orientation)*/
	s_rgba convToRGBA(float r, float g, float b);
	/**                                   **/

	/*                                */
};

#endif