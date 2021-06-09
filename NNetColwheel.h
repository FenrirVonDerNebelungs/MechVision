#pragma once
#ifndef NNETCOLWHEEL_H
#define NNETCOLWHEEL_H

#ifndef BASE_H
#include "Base.h"
#endif 

class NNetTestColwheel {
public:
	NNetTestColwheel();
	~NNetTestColwheel();
	unsigned char Init(
		float pixMax = 255.f,
		float Dhue = 3.f,
		float DI = 0.3f,
		float hueFadeV=0.3f,
		float I_target = 0.9f,
		float hue_target_x=1.f,
		float hue_target_y=0.f,
		float stepSteepness = 20.f
		);
	void Release();
	float evalRGB(float rgb[]);/*3 input col*/
protected:
	float m_pixMax;/*typically 255,  maximum value per r,g,b pixel*/
	float m_Dhue;  /*distance in cosine this needs to be greater than zero*/
	float m_DI;    /*intensity*/
	float m_hueFadeV;/*hue vector length at which the hue is considered to start to fade to a neutral return*/

	float m_I_target;
	s_2pt m_hue_target;/*rgb vector, in colorwheel*/

	float m_stepSteepness;/*mulitplier that makes the step steeper*/

	s_2pt m_Ur;
	s_2pt m_Ug;
	s_2pt m_Ub;
	float m_DhueRes;/*2 - m_Dhue*/

	float findHueIDist(float rgb[]);

	float findHueDistance(s_2pt& hueV);
	float findIDistance(float rgb[]);

	s_2pt findColwheelHue(float rgb[]);

	/*init/Util*/
	void setColwheelUnitVectors();
};
#endif