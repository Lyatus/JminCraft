#pragma once

#include <stdio.h>

enum NYCubeType
{
	CUBE_HERBE,
	CUBE_EAU,
	CUBE_TERRE,
	CUBE_AIR
};

class NYCube
{
public:
	bool _Draw;
	NYCubeType _Type;
	static const int CUBE_SIZE = 10;

	NYCube() {
		_Draw = true;
		_Type = CUBE_AIR;
	}

	bool isSolid(void) {
		return (_Type != CUBE_AIR);
	}

	void saveToFile(FILE * fs) {
		fputc(_Draw ? 1 : 0, fs);
		fputc(_Type, fs);
	}

	void loadFromFile(FILE * fe) {
		_Draw = fgetc(fe) ? true : false;
		_Type = (NYCubeType)fgetc(fe);
	}
	float* getColor() {
		static float defColor[4] = { 0, 0, 0, 0 };
		static float herbeColor[4] = { 1.0f / 255.0f, 112.0f / 255.0f, 12.0f / 255.0f, 1 };
		static float eauColor[4] = { 0.0f / 255.0f, 48.0f / 255.0f, 255.0f / 255.0f, 1 };
		static float terreColor[4] = { 101.0f / 255.0f, 74.0f / 255.0f, 0.0f / 255.0f, 1 };
		switch (_Type) {
			case CUBE_HERBE: return herbeColor;
			case CUBE_EAU: return eauColor;
			case CUBE_TERRE: return terreColor;
			default: return defColor;
		}
	}
};