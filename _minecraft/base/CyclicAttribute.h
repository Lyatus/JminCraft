#pragma once

#include <stdio.h>
#include <math.h>

class CyclicAttribute
{
private:
	float _a, _b, _last;
public:
	CyclicAttribute(float a, float b) : _a(a), _b(b), _last(0) {}
	void reset(float last) {
		_last = last;
	}
	float value(float t) {
		float s(t - _last);
		return (s > 0) ? (_a*s + _b*s*cos(t)) : 0;
	}
};