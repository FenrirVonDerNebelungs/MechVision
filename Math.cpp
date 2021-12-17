#include "Math.h"
#include "Img.h"

namespace Math {
	float StepFunc(float x) {
		/*1/(1+exp(-x))*/
		float expofn = expf(-x);
		//if (expofn <= 0.f)/*shouldn't happen*/
		//	return 0.f;
		float denom = 1.f + expofn;
		return 1.f / denom;
	}
	int loop(int i, int n) {
		int finali = i;
		while (finali >= n)
			finali -= n;
		while (finali < 0)
			finali += n;
		return finali;
	}
	float power(float x, int y) {
		float res = 1.f;
		for (int i = 0; i < y; i++)
			res *= x;
		return res;
	}
}
namespace vecMath {
	s_2pt add(const s_2pt& v1, const s_2pt& v2) {
		s_2pt v = { v1.x0 + v2.x0, v1.x1 + v2.x1 };
		return v;
	}
	s_2pt add(const s_2pt& v1, const s_2pt& v2, const s_2pt& v3) {
		s_2pt v = add(v1, v2);
		v.x0 += v3.x0;
		v.x1 += v3.x1;
		return v;
	}
	s_2pt v12(const s_2pt& v1, const s_2pt& v2) {
		s_2pt v = { v2.x0 - v1.x0, v2.x1 - v1.x1 };
		return v;
	}

	Tup3 v12(const Tup3& v1, const Tup3& v2) {
		Tup3 v = { v2.x - v1.x, v2.y - v1.y, v2.z - v1.z };
		return v;
	}

	float len(const s_2pt& v) {
		return sqrtf(v.x0*v.x0 + v.x1*v.x1);
	}
	float len(const Tup3& v) {
		return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
	}
	float dist(const s_2pt& v1, const s_2pt& v2) {
		s_2pt v = v12(v1, v2);
		return len(v);
	}
	float dist(const Tup3& v1, const Tup3& v2) {
		Tup3 v = v12(v1, v2);
		return len(v);
	}
	float dot(const s_2pt& v1, const s_2pt& v2) {
		return (v1.x0*v2.x0 + v1.x1*v2.x1);
	}
	s_2pt mul(float a, const s_2pt& vec) {
		s_2pt v1 = { a * vec.x0, a * vec.x1 };
		return v1;
	}
	s_2pt perpUR(s_2pt& vec) {
		s_2pt vperp = { -vec.x1, vec.x0 };
		return vperp;
	}
	s_2pt perpUL(s_2pt& vec) {
		s_2pt vperp = { vec.x1, -vec.x0 };
		return vperp;
	}
	void  setBasis(float angRad, s_2pt& basisU0, s_2pt& basisU1) {
		basisU0.x0 = cosf(angRad);
		basisU0.x1 = sinf(angRad);
		basisU1.x0 = -sinf(angRad);
		basisU1.x1 = cosf(angRad);
	}
	void revBasis(const s_2pt& basisU0, const s_2pt& basisU1, s_2pt& revBasisU0, s_2pt& revBasisU1) {
		revBasisU0.x0 = basisU1.x1;
		revBasisU0.x1 = -basisU0.x1;

		revBasisU1.x0 = -basisU1.x0;
		revBasisU1.x1 = basisU0.x0;
	}
	s_2pt convBasis(const s_2pt& basisU0, const s_2pt& basisU1, const s_2pt& vecInBasis) {
		s_2pt v0 = mul(vecInBasis.x0, basisU0);
		s_2pt v1 = mul(vecInBasis.x1, basisU1);
		return add(v0, v1);
	}
	bool inCircle(float r, const s_2pt& center, const s_2pt& pt) {
		float d = dist(center, pt);
		return d <= r;
	}
}

namespace imgMath {
	s_rgba convToRGBA(float r, float g, float b) {
		if (r > 255.f)
			r = 255.f;
		if (r < 0.f)
			r = 0.f;
		if (g > 255.f)
			g = 255.f;
		if (g < 0.f)
			g = 0.f;
		if (b > 255.f)
			b = 255.f;
		if (b < 0.f)
			b = 0.f;
		s_rgba rgba = { (unsigned char)r, (unsigned char)g, (unsigned char)b, 0xFF };
		return rgba;
	}
	s_rgb convToRGB(float r, float g, float b) {
		if (r > 255.f)
			r = 255.f;
		if (r < 0.f)
			r = 0.f;
		if (g > 255.f)
			g = 255.f;
		if (g < 0.f)
			g = 0.f;
		if (b > 255.f)
			b = 255.f;
		if (b < 0.f)
			b = 0.f;
		s_rgb rgb = { (unsigned char)r, (unsigned char)g, (unsigned char)b };
		return rgb;
	}
	s_2pt_i convToVint(const s_2pt& vec) {
		s_2pt_i vi;
		vi.x0 = (long)floorf(vec.x0);
		vi.x1 = (long)floorf(vec.x1);
		return vi;
	}
	s_2pt convToVfloat(const s_2pt_i& vi) {
		s_2pt vec;
		vec.x0 = (float)vi.x0;
		vec.x1 = (float)vi.x1;
		return vec;
	}
	s_2pt perpUR(s_2pt& vec) {
		s_2pt perp = { -vec.x1, vec.x0 };
		return perp;
	}
	void drawPoint(long i, long j, const s_rgba& col, Img* canvas) {
		if (i < 0 || j < 0)
			return;
		if (i >= canvas->getWidth() || j >= canvas->getHeight())
			return;
		canvas->SetRGBA(i, j, col);
	}
	void drawLine(long i_start, long j_start, long i_end, long j_end, const s_rgba& col, float thickness, Img* canvas) {
		s_2pt v = { (float)(i_end - i_start), (float)(j_end - j_start) };
		drawV(v, i_start, j_start, col, thickness, canvas);
	}
	void drawV(const s_2pt& v, long i, long j, const s_rgba& col, float thickness, Img* canvas)
	{
		long width = canvas->getWidth();
		long height = canvas->getHeight();
		float x = v.x0;
		float y = v.x1;
		if (fabs(x) <= 1 && fabs(y) <= 1) {
			return;
		}

		long x_max = (long)roundf(x);
		long x_absmax = abs(x_max);
		long y_max = (long)roundf(y);
		long y_absmax = abs(y_max);
		if (fabs(x) >= fabs(y)) {
			long sign_val = (x >= 0.f) ? 1 : -1;
			float slope = y / x;
			long x_i = 0;
			while (x_i <= x_absmax) {
				long x_i_ = x_i * sign_val;
				float y_cur = slope * ((float)x_i_);
				long y_j = (long)roundf(y_cur);
				drawPoint(x_i_ + i, y_j + j, col, canvas);
				x_i++;
			}
		}
		else {
			long sign_val = (y >= 0.f) ? 1 : -1;
			float slope = x / y;
			long y_j = 0;
			while (y_j <= y_absmax) {
				long y_j_ = sign_val * y_j;
				float x_cur = slope * ((float)y_j_);
				long x_i = (long)roundf(x_cur);
				drawPoint(x_i + i, y_j_ + j, col, canvas);
				y_j++;
			}
		}
	}
}
namespace hexMath {
	bool inHex(float hexR, float hexRS, const s_2pt hexU[], const s_2pt& center, const s_2pt& pt, float padding) {
		float xdiff = pt.x0 - center.x0;
		float ydiff = pt.x1 - center.x1;
		float diff = sqrtf(xdiff * xdiff + ydiff * ydiff);
		if (diff > hexR)
			return false;
		s_2pt vpt = { xdiff, ydiff };
		float max_proj = 0.f;
		for (int i = 0; i < 6; i++) {
			float proj = vecMath::dot(vpt, hexU[i]);
			if (proj > max_proj)
				max_proj = proj;
		}
		bool inside = false;
		if (max_proj <= (hexRS + padding))
			inside = true;
		return inside;
	}
}
