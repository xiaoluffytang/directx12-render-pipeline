#include "Font3D.h"
#include <assert.h>
#include <fstream>

Font3D::Font3D(const char* path)
{
	if (FT_Init_FreeType(&lib))
	{
		FT_Done_FreeType(lib);
		lib = NULL;
		return;
	}
	if (FT_New_Face(lib, path, 0, &face))
	{
		FT_Done_FreeType(lib);
		lib = NULL;
		return;
	}
	FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	FT_Set_Pixel_Sizes(face, 24, 24);
}


Font3D::~Font3D()
{
}

std::vector<Tri> Font3D::getVectoriser(string str)
{
	float offset = 0;
	tris.clear();
	const char* p = str.c_str();
	int charcode = 0;
	while (*p)
	{
		charcode = (*p & 0x80) ? *(WORD*)p : *(char*)p;

		offset = AddCharacter(charcode, offset);

		p += (*p & 0x80) ? 2 : 1;
	}
	return tris;
}

float Font3D::AddCharacter(long charcode, float offset) {
	FT_UInt prevCharIndex = 0, curCharIndex = 0;
	FT_Pos  prev_rsb_delta = 0;

	char*  chr = (char*)&(charcode);
	wchar_t wchar;
	MultiByteToWideChar(CP_ACP, 0, chr, strlen(chr) + 1, &wchar, 1);
	if (FT_Load_Char(face, wchar, FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT))
		return 0;

	FT_Glyph glyph;
	if (FT_Get_Glyph(face->glyph, &glyph))
		printf("FT_Get_Glyph failed\n");

	if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
		printf("Invalid Glyph Format\n");
		glyph->format = FT_GLYPH_FORMAT_OUTLINE;
	}

	short nCountour = 0;
	nCountour = face->glyph->outline.n_contours;

	int startIndex = 0, endIndex = 0;
	FT_Outline* o = &face->glyph->outline;

	if (FT_HAS_KERNING(face) && prevCharIndex) {
		FT_Vector  kerning;
		FT_Get_Kerning(face, prevCharIndex, curCharIndex, FT_KERNING_DEFAULT, &kerning);
		offset += kerning.x >> 6;
	}

	if (prev_rsb_delta - face->glyph->lsb_delta >= 32)
		offset -= 1.0f;
	else if (prev_rsb_delta - face->glyph->lsb_delta < -32)
		offset += 1.0f;

	prev_rsb_delta = face->glyph->rsb_delta;

	Vectoriser *vectoriser = new Vectoriser(face->glyph, bezierSteps);
	for (size_t c = 0; c < vectoriser->ContourCount(); ++c) {
		const Contour* contour = vectoriser->GetContour(c);

		for (size_t p = 0; p < contour->PointCount() - 1; ++p) {
			const double* d1 = contour->GetPoint(p);
			const double* d2 = contour->GetPoint(p + 1);
			Tri t1;
			t1.a.x = (d1[0] / 64.0f) + offset;
			t1.a.y = d1[1] / 64.0f;
			t1.a.z = 0.0f;
			t1.b.x = (d2[0] / 64.0f) + offset;
			t1.b.y = d2[1] / 64.0f;
			t1.b.z = 0.0f;
			t1.c.x = (d1[0] / 64.0f) + offset;
			t1.c.y = d1[1] / 64.0f;
			t1.c.z = extrude;
			tris.push_back(t1);

			Tri t2;
			t2.a.x = (d1[0] / 64.0f) + offset;
			t2.a.y = d1[1] / 64.0f;
			t2.a.z = extrude;
			t2.c.x = (d2[0] / 64.0f) + offset;
			t2.c.y = d2[1] / 64.0f;
			t2.c.z = extrude;
			t2.b.x = (d2[0] / 64.0f) + offset;
			t2.b.y = d2[1] / 64.0f;
			t2.b.z = 0.0f;

			tris.push_back(t2);
		}

		if (contour->GetDirection()) {
			std::vector<p2t::Point*> polyline = triangulateContour(vectoriser, c, offset);
			CDT* cdt = new CDT(polyline);

			for (size_t cm = 0; cm < vectoriser->ContourCount(); ++cm) {
				const Contour* sm = vectoriser->GetContour(cm);
				if (c != cm && !sm->GetDirection() && sm->IsInside(contour)) {
					std::vector<p2t::Point*> pl = triangulateContour(vectoriser, cm, offset);
					cdt->AddHole(pl);
				}
			}

			cdt->Triangulate();
			std::vector<p2t::Triangle*> ts = cdt->GetTriangles();
			for (int i = 0; i < ts.size(); i++) {
				Triangle* ot = ts[i];

				Tri t1;
				t1.a.x = ot->GetPoint(0)->x;
				t1.a.y = ot->GetPoint(0)->y;
				t1.a.z = 0.0f;
				t1.b.x = ot->GetPoint(1)->x;
				t1.b.y = ot->GetPoint(1)->y;
				t1.b.z = 0.0f;
				t1.c.x = ot->GetPoint(2)->x;
				t1.c.y = ot->GetPoint(2)->y;
				t1.c.z = 0.0f;
				tris.push_back(t1);

				Tri t2;
				t2.a.x = ot->GetPoint(1)->x;
				t2.a.y = ot->GetPoint(1)->y;
				t2.a.z = extrude;
				t2.b.x = ot->GetPoint(0)->x;
				t2.b.y = ot->GetPoint(0)->y;
				t2.b.z = extrude;
				t2.c.x = ot->GetPoint(2)->x;
				t2.c.y = ot->GetPoint(2)->y;
				t2.c.z = extrude;
				tris.push_back(t2);
			}
			delete cdt;
		}
	}

	delete vectoriser;
	vectoriser = NULL;

	prevCharIndex = curCharIndex;
	float chSize = face->glyph->advance.x >> 6;
	return offset + chSize;
}

std::vector<p2t::Point*> Font3D::triangulateContour(Vectoriser *vectoriser, int c, float offset) {
	std::vector<p2t::Point*> polyline;
	const Contour* contour = vectoriser->GetContour(c);
	for (size_t p = 0; p < contour->PointCount(); ++p) {
		const double* d = contour->GetPoint(p);
		polyline.push_back(new p2t::Point((d[0] / 64.0f) + offset, d[1] / 64.0f));
	}
	return polyline;
}
