#include <cmath>
#include <GL/GLU.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "BSP.h"
#pragma once

namespace Utils {
	inline static double randomF(double low, double up) {
		return low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (up - low)));
	}

	inline static BSP_vertex vec3toBSPVertex(const glm::vec3 & v3) {
		BSP_vertex v;
		v.x = v3.x;
		v.y = v3.y;
		v.z = v3.z;

		return v;
	}

	inline static glm::vec3 BSPVertexToVec3(const BSP_vertex & v) {
		glm::vec3 v3;
		v3.x = v.x;
		v3.y = v.y;
		v3.z = v.z;

		return v3;
	}

	inline static glm::vec3 bezier3(const glm::vec3 & p0, const glm::vec3 & p1, const glm::vec3 & p2, float a) {
		float b = 1.0 - a;
		return 
			p0 * (b * b) +
			p1 * (2 * b * a) +
			p2 * (a * a);
	}

	inline static BSP_vertex bezier3(const BSP_vertex & p0, const BSP_vertex & p1, const BSP_vertex & p2, float a) {
		float b = 1.0 - a;
		glm::vec3 _p0, _p1, _p2;
		_p0.x = p0.x; _p0.y = p0.y; _p0.z = p0.z;
		_p1.x = p1.x; _p1.y = p1.y; _p1.z = p1.z;
		_p2.x = p2.x; _p2.y = p2.y; _p2.z = p2.z;

		auto finalVec3 = bezier3(_p0, _p1, _p2, a);
		BSP_vertex finalBSP;
		finalBSP.x = finalVec3.x;
		finalBSP.y = finalVec3.y;
		finalBSP.z = finalVec3.z;

		return finalBSP;
	}

	constexpr unsigned int bytes2megabytes(unsigned int bytes) {
		return bytes / 1024 / 1024;
	}

	typedef struct {
		double r;       // percent
		double g;       // percent
		double b;       // percent
	} rgb;

	typedef struct {
		double h;       // angle in degrees
		double s;       // percent
		double v;       // percent
	} hsv;

	inline static hsv rgb2hsv(rgb in)
	{
		hsv         out;
		double      min, max, delta;

		min = in.r < in.g ? in.r : in.g;
		min = min  < in.b ? min : in.b;

		max = in.r > in.g ? in.r : in.g;
		max = max  > in.b ? max : in.b;

		out.v = max;                                // v
		delta = max - min;
		if (delta < 0.00001)
		{
			out.s = 0;
			out.h = 0; // undefined, maybe nan?
			return out;
		}
		if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
			out.s = (delta / max);                  // s
		}
		else {
			// if max is 0, then r = g = b = 0              
			// s = 0, v is undefined
			out.s = 0.0;
			out.h = NAN;                            // its now undefined
			return out;
		}
		if (in.r >= max)                           // > is bogus, just keeps compilor happy
			out.h = (in.g - in.b) / delta;        // between yellow & magenta
		else
			if (in.g >= max)
				out.h = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
			else
				out.h = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan

		out.h *= 60.0;                              // degrees

		if (out.h < 0.0)
			out.h += 360.0;

		return out;
	}


	inline static rgb hsv2rgb(hsv in)
	{
		double      hh, p, q, t, ff;
		long        i;
		rgb         out;

		if (in.s <= 0.0) {       // < is bogus, just shuts up warnings
			out.r = in.v;
			out.g = in.v;
			out.b = in.v;
			return out;
		}
		hh = in.h;
		if (hh >= 360.0) hh = 0.0;
		hh /= 60.0;
		i = (long)hh;
		ff = hh - i;
		p = in.v * (1.0 - in.s);
		q = in.v * (1.0 - (in.s * ff));
		t = in.v * (1.0 - (in.s * (1.0 - ff)));

		switch (i) {
		case 0:
			out.r = in.v;
			out.g = t;
			out.b = p;
			break;
		case 1:
			out.r = q;
			out.g = in.v;
			out.b = p;
			break;
		case 2:
			out.r = p;
			out.g = in.v;
			out.b = t;
			break;

		case 3:
			out.r = p;
			out.g = q;
			out.b = in.v;
			break;
		case 4:
			out.r = t;
			out.g = p;
			out.b = in.v;
			break;
		case 5:
		default:
			out.r = in.v;
			out.g = p;
			out.b = q;
			break;
		}
		return out;
	}

}