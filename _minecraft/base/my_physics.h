#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "engine/utils/types_3d.h"

float equation_plan(NYVert3Df n, NYVert3Df p) {
	return -n.scalProd(p);
}
bool intersection_plan(NYVert3Df n, float d, NYVert3Df a, NYVert3Df b, NYVert3Df& point) {
	float divider(n.X*(b.X - a.X) + n.Y*(b.Y - a.Y) + n.Z*(b.Z - a.Z));
	if (divider == 0) return false;
	float t((n.X*a.X + n.Y*a.Y + n.Z*a.Z + d) / divider);
	if (t < 0 || t>1) return false;
	point = a + (b - a)*t;
}
bool dans_carre(NYVert3Df a, NYVert3Df b, NYVert3Df c, NYVert3Df d, NYVert3Df p) {

}

#endif