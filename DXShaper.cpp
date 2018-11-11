#include "DXShaper.h"
#include "freetypelib.h"

namespace cocos2d
{
	namespace ui
	{
		DXShaper::DXShaper()
		{
		}


		DXShaper::~DXShaper()
		{
		}

		void DXShaper::doShap(LabelComponent* lbComp)
		{
			if (!m_font || !m_buffer) return;

			HBText textInfo;

			hb_buffer_reset(m_buffer);

			hb_buffer_set_direction(m_buffer, m_textInfo.direction);
			hb_buffer_set_script(m_buffer, m_textInfo.script);
			hb_buffer_set_language(m_buffer, hb_language_from_string(m_textInfo.language.c_str(), m_textInfo.language.size()));
			size_t length = m_textInfo.data.size();

			hb_buffer_add_utf8(m_buffer, m_textInfo.c_data(), length, 0, length);

			// harfbuzz shaping
			hb_shape(m_font, m_buffer, NULL/*features.empty() ? NULL : &features[0]*/, 0/*features.size()*/);

			unsigned int glyphCount;
			hb_glyph_info_t *glyphInfo = hb_buffer_get_glyph_infos(m_buffer, &glyphCount);
			hb_glyph_position_t *glyphPos = hb_buffer_get_glyph_positions(m_buffer, &glyphCount);

			LabelComponent::TriangleInfo triangleInfo;
			float textWidth = 0;
			float textHeight = 0;
			
			for (int i = 0; i < glyphCount; ++i) {
				Glyph* glyph = m_ftLib->rasterize(m_ftface, glyphInfo[i].codepoint);

				int twidth = pow(2, ceil(log(glyph->width) / log(2)));
				int theight = pow(2, ceil(log(glyph->height) / log(2)));

				float x = 0.0;
				float y = 0.0;
				float s0 = 0.0;
				float t0 = 0.0;
				float s1 = (float)glyph->width / twidth;
				float t1 = (float)glyph->height / theight;
				float xa = (float)glyphPos[i].x_advance / 64;
				float ya = (float)glyphPos[i].y_advance / 64;
				float xo = (float)glyphPos[i].x_offset / 64;
				float yo = (float)glyphPos[i].y_offset / 64;
				float x0 = x + xo + glyph->bearing_x;
				float y0 = floor(y + yo + glyph->bearing_y);
				float x1 = x0 + glyph->width;
				float y1 = floor(y0 - glyph->height);

				gl::Vertex* vertices = new gl::Vertex[4];
				vertices[0] = gl::Vertex(x0, y0, s0, t0);
				vertices[1] = gl::Vertex(x0, y1, s0, t1);
				vertices[2] = gl::Vertex(x1, y1, s1, t1);
				vertices[3] = gl::Vertex(x1, y0, s1, t0);

				unsigned short* indices = new unsigned short[6];
				indices[0] = 0; indices[1] = 1;
				indices[2] = 2; indices[3] = 0;
				indices[4] = 2; indices[5] = 3;

				// gl::Mesh* m = new gl::Mesh;

				// m->indices = indices;
				// m->textureData = tdata;

				// // don't do this!! use atlas texture instead
				// m->textureId = gl::getTextureId(twidth, theight);

				// m->vertices = vertices;
				// m->nbIndices = 6;
				// m->nbVertices = 4;

				// gl::uploadTextureData(m->textureId, twidth, theight, tdata);

				// meshes.push_back(m);
				triangleInfo.triangles.emplace_back(vertices);
				triangleInfo.indices.emplace_back(indices);

				lbComp.addGlyphQuad(i, triangleInfo);

				x += xa;
				y += ya;

				lib->freeGlyph(glyph);
			}

			//return meshes;

		}
	}