#pragma once
#ifndef COLWHEEL_H
#define COLWHEEL_H

#ifndef BASE_H
#include "Base.h"
#endif 

class ColWheel {
public:
	ColWheel();
	~ColWheel();
	unsigned char Init(
		float pixMax = 255.f,
		float Dhue = 3.f,
		float DI = 0.2f,//0.4f,//0.3f
		float DSat = 0.3f,
		float hueFadeV = 0.3f,
		float I_target = 1.0f,//0.9f,
		float hue_target_x = 1.f,
		float hue_target_y = 0.f,
		float Sat_target = 0.f,
		float stepSteepness = 20.f
	);
	unsigned char Update(
		float Dhue = 3.f,
		float DI = 0.4f,//0.3f
		float DSat = 0.3f,
		float hueFadeV = 0.3f,
		float I_target = 0.9f,//0.9f,
		float hue_target_x = 1.f,
		float hue_target_y = 0.f,
		float Sat_target = 0.f,
		float stepSteepness = 20.f
	);
	void Release();
	float evalRGB(float rgb[]);/*3 input col*/

	inline void SetTargI(float I_target) { if(I_target>=0.f && I_target<=1.f) m_I_target = I_target; }
	inline void SetTargSat(float Sat_target) { if(Sat_target>=0.f && Sat_target<=1.f) m_Sat_target = Sat_target; }
protected:
	float m_pixMax;/*typically 255,  maximum value per r,g,b pixel*/
	float m_Dhue;  /*distance in cosine this needs to be greater than zero*/
	float m_DI;    /*intensity*/
	float m_DSat;  /*saturation*/
	float m_hueFadeV;/*hue vector length at which the hue is considered to start to fade to a neutral return*/

	float m_I_target;
	s_2pt m_hue_target;/*rgb vector, in colorwheel*/
	float m_Sat_target;

	float m_stepSteepness;/*mulitplier that makes the step steeper*/

	s_2pt m_Ur;
	s_2pt m_Ug;
	s_2pt m_Ub;
	float m_DhueRes;/*2 - m_Dhue*/

	s_2pt m_hueRGBVec;/*current hue expressed in the vectors*/
	float m_hueLen;


	float findHueIDist(float rgb[]);

	float findHueDistance(s_2pt& hueV);
	float findIDistance(float rgb[]);
	float findSatDistance(float rgb[]);

	s_2pt findColwheelHue(float rgb[]);

	/*init/Util*/
	void setColwheelUnitVectors();
};
#endif