#pragma once
#ifndef HEX_H
#define HEX_H
#ifndef CONVOLHEX_H
#include "ConvolHex.h"
#endif

class Hex : public HexBase
{
public:
	Hex();
	~Hex();
	unsigned char Init(
		Img* img,
		float Rhex = 3.f,//7.f,//3.f,//5.f, //3.f,  7 for debug of eye
		float sigmaVsR = 0.75,//1.f, //0.75f,
		float IMaskRVsR = 1.5f
	);
	unsigned char Update(Img* img);
	unsigned char Run();
#ifndef MECVISPI_WIN
	unsigned char RunThreaded();
#endif
	void Release();
	inline int getWHex() { return m_wHex; }
	inline int getHHex() { return m_hHex; }
protected:
	ConvolHex* m_Convol;
	s_2pt      m_BR;/*lower right center start*/

	int m_wHex;
	int m_hHex;

	unsigned char genMesh();
	/***************************************************************/
	/* helpers to gen Mesh                                         */
	unsigned char genMeshLoc();
	unsigned char genMeshWeb();
	/** helpers to helpers **/
	unsigned char genMeshLocFromBR(float nW, float nH);
	/****                  ***/
	/***************************************************************/
};

#endif