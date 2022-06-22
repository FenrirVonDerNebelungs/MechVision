#pragma once
#ifndef MATH_H
#define MATH_H
#ifndef TUPLES_H
#include "Tuples.h"
#endif

class Img;

struct s_rgba {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};
struct s_rgb {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct s_2pt_i {
	long x0;
	long x1;
};
struct s_2pt {
	float x0;
	float x1;
};

namespace utilStruct {
	inline void zeroRGBA(s_rgba& rgba) { rgba.r = 0x00; rgba.g = 0x00; rgba.b = 0x00; rgba.a = 0x00; }
	inline void zeroRGB(s_rgb& rgb) { rgb.r = 0x00; rgb.g = 0x00; rgb.b = 0x00; }
	inline void zero2pt_i(s_2pt_i& spt) { spt.x0 = 0; spt.x1 = 0; }
	inline void zero2pt(s_2pt& pt) { pt.x0 = 0.f; pt.x1 = 0.f; }
}
namespace Math {
	float StepFunc(float x);/* 1/(1+exp(-x))  takes an x value and returns a step function where - x goes to 0, 0 goes to 1/2 and +x goes to 1*/
	inline float StepFuncSym(float x) { return tanhf(x); }/* hyper bolic tangent function (exp(x) - exp(-x))/(exp(x) + exp(-x)) 0 at 0, -1 at neg inf reached close to 2, symetric pos 1 at pos inf*/
	inline float DStepFuncSym(float x) {
		float t = tanhf(x); return 1.f - t * t;
	}/* derivitave of tanhf with respect to x is 1-tanhf^2(x)*/
	int loop(int i, int n);
	float power(float x, int y);
	float Gaussian(float pt, float norm_const, float center = 0.f);
	float GaussianFast(float pt, float sqrt2_ExpConst, float Norm, float center);
}
namespace arrMath {
	/*puts value into array at i and moves array forward after the value*/
	void put(float v, int i, float ar[], int n);
	void put(int v, int i, int ar[], int n);
	int getFirstBelow(float v, float ar[], int n, int i_start=0);
}
namespace vecMath {
	s_2pt add(const s_2pt& v1, const s_2pt& v2);
	s_2pt add(const s_2pt& v1, const s_2pt& v2, const s_2pt& v3);
	s_2pt v12(const s_2pt& v1, const s_2pt& v2);
	Tup3 v12(const Tup3& v1, const Tup3& v2);
	float len(const s_2pt& v);
	float len(const Tup3& v);
	float dist(const s_2pt& v1, const s_2pt& v2);
	float dist(const Tup3& v1, const Tup3& v2);
	float dot(const s_2pt& v1, const s_2pt& v2);
	s_2pt mul(float a, const s_2pt& vec);
	s_2pt perpUR(s_2pt& vec);
	s_2pt perpUL(s_2pt& vec);
	void  setBasis(float angRad, s_2pt& basisU0, s_2pt& basisU1);
	void  revBasis(const s_2pt& basisU0, const s_2pt& basisU1, s_2pt& revBasisU0, s_2pt& revBasisU1);
	s_2pt convBasis(const s_2pt& basisU0, const s_2pt& basisU1, const s_2pt& vecInBasis);
	bool inCircle(float r, const s_2pt& center, const s_2pt& pt);
	float distCircleArc(float r, const s_2pt& center, const s_2pt& pt);/*returns negative if point is inside circle*/
	float distLineGivenPerp(const s_2pt& line_pt, const s_2pt& line_perp, const s_2pt& pt);/* returns pos distance from line in direction of line_perp, line_perp must be of length 1*/
}
namespace imgMath {
	s_rgba convToRGBA(float r, float g, float b);
	s_rgb  convToRGB(float r, float g, float b);
	s_2pt_i convToVint(const s_2pt& vec);
	s_2pt convToVfloat(const s_2pt_i& vi);
	void drawPoint(long i, long j, const s_rgba& col, Img* canvas);
	void drawLine(long i_start, long j_start, long i_end, long j_end, const s_rgba& col, float thickness, Img* canvas);
	void drawV(const s_2pt& v, long i, long j, const s_rgba& col, float thickness, Img* canvas);
}
namespace hexMath {
	bool inHex(float hexR, float hexRS, const s_2pt hexU[], const s_2pt& center, const s_2pt& pt, float padding = 0.f);
}

#endif