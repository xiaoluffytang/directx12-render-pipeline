#pragma once
#include "../../../Common/d3dApp.h"
#include "ftgl/Point.h"
#include "ftgl/Vectoriser.h"
#include "poly2tri/poly2tri.h"

using namespace std;
using namespace p2t;
#include <ft2build.h>
#include FT_FREETYPE_H

struct Vector3df
{
	float x, y, z;
};
struct Tri
{
	Vector3df a, b, c;
};
class Font3D
{
public:
	Font3D(const char* path);
	~Font3D();
	std::vector<Tri> getVectoriser(string str);
private :
	FT_Library lib;
	FT_Face face;
	unsigned short bezierSteps = 0;
	float extrude = 5;
	std::vector<Tri> tris;
	float AddCharacter(long charcode, float offset);
	std::vector<p2t::Point*> triangulateContour(Vectoriser *vectoriser, int c, float offset);
};

