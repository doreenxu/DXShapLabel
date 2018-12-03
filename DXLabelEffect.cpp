#include "DXLabelEffect.h"
#include "DXLabelParser.h"
#include "2d/CCFontFreeType.h"
#include <ft2build.h>
#include "freetype2/fttypes.h"
#include "freetype2/ftimage.h"

namespace cocos2d
{
#if CC_STUDIO_ENABLED_VIEW
	const char *WeCLabel::sVertexShaderSource =
		"attribute vec4 a_position;\n"
		"attribute vec2 a_texCoord;\n"
		"attribute vec4 a_color;\n"
		"#ifdef GL_ES\n"
		"varying lowp vec4 v_fragmentColor;\n"
		"varying mediump vec2 v_texCoord;\n"
		"\n#else\n"
		"varying vec4 v_fragmentColor;\n"
		"varying vec2 v_texCoord;\n"
		"\n#endif\n"
		"void main()\n"
		"{\n"
		"   gl_Position = CC_PMatrix * a_position;\n"
		"   v_fragmentColor = a_color;\n"
		"   v_texCoord = a_texCoord;\n"
		"}\n";

	const char *WeCLabel::sFragmentShaderSource =
		"#ifdef GL_ES\n"
		"precision lowp float;\n"
		"#endif\n"
		"varying vec4 v_fragmentColor;"
		"varying vec2 v_texCoord;"
		"void main()\n"
		"{\n"
		"   gl_FragColor = vec4(v_fragmentColor.rgb, v_fragmentColor.a * texture2D(CC_Texture0, v_texCoord).a);\n"
		"}";

	const char *WeCLabel::sShaderName = "ShaderWeCLabel";
#endif
	
	namespace ui
    {   
		void DXLabelBitmapGenerator::getBitmap(Glyph* _glyph)
		{
			//如果贴图缓存已经存在
			// 公司pc修改此函数没有提交2018-12-2晚
		}

		bool DXLabelBitmapGenerator::getNormalBitmap(Glyph* _glyph)
		{
			FT_Int32 flags = FT_LOAD_RENDER;//¾ö¶¨¼ÓÔØºó×ÖÌåÍ¼ÏñµÄÊôÐÔ
											 //´ÓfaceÖÐ×°ÔØ¶ÔÓ¦µÄ×ÖÐÎÍ¼Ïñ
			int dpi = 72;
			int fontSizePoints = (int)(64.f * _glyph->fontSize * CC_CONTENT_SCALE_FACTOR());
			FT_Error error = FT_Set_Char_Size(*m_ftface, 0, fontSizePoints, dpi, dpi);
			if (error) return false;

			error = FT_Load_Glyph(*m_ftface,
				_glyph->glyphIndex, // the glyph_index in the font file
				flags
			);
			if (error) return false;

			// FT_GlyphSlot is a pointer
			FT_GlyphSlot slot = (*m_ftface)->glyph;
			error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
			if (error) return false;

			FT_Bitmap ftBitmap = slot->bitmap;
			glyph->buffer = ftBitmap.buffer;
			glyph->width = ftBitmap.width;
			glyph->height = ftBitmap.rows;
			glyph->bearing_x = slot->bitmap_left;
			glyph->bearing_y = slot->bitmap_top;

			int twidth = pow(2, ceil(std::log(glyph->width) / std::log(2)));
			int theight = pow(2, ceil(std::log(glyph->height) / std::log(2)));

			float x = 0.0;
			float y = 0.0;
			float s0 = 0.0;
			float t0 = 0.0;
			float s1 = (float)glyph->width / twidth;
			float t1 = (float)glyph->height / theight;
			float x0 = x + xo + glyph->bearing_x;
			float y0 = floor(y + yo + glyph->bearing_y);
			float x1 = x0 + glyph->width;
			float y1 = floor(y0 - glyph->height);

			return true;
		}



		bool DXLabelBitmapGenerator::setRotateBitmap(Glyph* _glyph)
		{
			float angle = (25.0 / 360) * 3.14159 * 2;      /* use 25 degrees     */

			int dpi = 72;
			int fontSizePoints = (int)(64.f * _glyph->fontSize * CC_CONTENT_SCALE_FACTOR());
			FT_Error error = FT_Set_Char_Size(*m_ftface, 0, fontSizePoints, dpi, dpi);
			if (error) return false;

			/* set up matrix */
			FT_Vector pen;
			FT_Matrix matrix;
			if (lastEffectStyle == _glyph->effect)
			{
				matrix = lastMatrix;
			}
			else
			{
				matrix.xx = (FT_Fixed)(cos(angle) * 0x10000L);
				matrix.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
				matrix.yx = (FT_Fixed)(sin(angle) * 0x10000L);
				matrix.yy = (FT_Fixed)(cos(angle) * 0x10000L);

				/* the pen position in 26.6 cartesian space coordinates; */
				/* start at (300,200) relative to the upper left corner  */
				pen.x = 300 * 64;
				pen.y = (target_height - 200) * 64;
			}

			/* set transformation */
			FT_Set_Transform(face, &matrix, &pen);
		}
		//---------- ��Ʒ ----------
		void DXLabelBitmapGenerator::getOutLineBitmap(LabelComponent* comp)
		{
			do {
				auto library = cocos2d::WeCFontFreeType::getFTLibrary();
				if (library == nullptr)
					break;

				FT_Stroker stroker;
				FT_Stroker_New(library, &stroker);
				FT_Stroker_Set(stroker,
					(int)(m_width * 64),
					FT_STROKER_LINECAP_ROUND,
					FT_STROKER_LINEJOIN_ROUND,
					0);

				if (stroker == nullptr)
					break;

				FT_Face face;
				FT_Encoding encoding;
				cocos2d::WeCFontFreeType::getFontFace(comp->getFontName(), face, encoding);

				FT_Int32 flags = FT_LOAD_DEFAULT;//决定加载后字体图像的属性
				const std::vector<DXShaper::Glyph>& cref_glyphinfoVec = comp->getGlyphInfoVec();
				for (auto glyphinfo : cref_glyphinfoVec)
				{
					if (FT_Load_Glyph(face, glyphinfo.glyphIndex, flags))
					{

					}
				}

				if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
					break;

				FT_Glyph glyph;

				if (FT_Get_Glyph(face->glyph, &glyph))
				{
					if (glyph)
						FT_Done_Glyph(glyph);
					glyph = nullptr;
					break;
				}

				FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);

				// Clean up afterwards.
				FT_Stroker_Done(stroker);
				if (glyph->format != FT_GLYPH_FORMAT_OUTLINE)
				{
					if (glyph)
						FT_Done_Glyph(glyph);
					glyph = nullptr;
					break;
				}

				FT_BBox bbox;

				FT_Outline *outline = &reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
				FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_GRIDFIT, &bbox);

				int width = (bbox.xMax - bbox.xMin) >> 6;
				int rows = (bbox.yMax - bbox.yMin) >> 6;

				int sizeNeed = width * rows * sizeof(unsigned char);
				auto cache = WeCCharFontManager::getInstance()->tryGetStrokerReusedCache(sizeNeed);

				if (cache == nullptr)
				{
					if (glyph)
						FT_Done_Glyph(glyph);
					glyph = nullptr;
					break;
				}

				FT_Bitmap bmp;
				bmp.buffer = cache;
				memset(bmp.buffer, 0, width * rows);
				bmp.width = (int)width;
				bmp.rows = (int)rows;
				bmp.pitch = (int)width;
				bmp.pixel_mode = FT_PIXEL_MODE_GRAY;
				bmp.num_grays = 256;

				FT_Raster_Params params;
				memset(&params, 0, sizeof(params));
				params.source = outline;
				params.target = &bmp;
				params.flags = FT_RASTER_FLAG_AA;
				FT_Outline_Translate(outline, -bbox.xMin, -bbox.yMin);
				FT_Outline_Render(library, outline, &params);

				auto& metrics = face->glyph->metrics;
				outRect.origin.x = metrics.horiBearingX >> 6;
				outRect.origin.y = -(metrics.horiBearingY >> 6);
				outRect.size.width = (metrics.width >> 6);
				outRect.size.height = (metrics.height >> 6);
				xadvance = (static_cast<int>(face->glyph->metrics.horiAdvance >> 6));
				ascender = static_cast<int>(face->size->metrics.ascender >> 6);

				outWidth = width;
				outHeight = rows;
				re = cache;
			} while (0);
		}
            // 效果对象会对component的triangle属性进行修改
			void UnderlineEffect::execute(LabelComponent* comp)
            {
                //原实现是在每个字符加入“_“下划线，这样实现，下划线之间会有间隙
                //考虑加入一条（textwidth，1）的线，
            }
         
			void BoldEffect::execute(){}

            void OutlineEffect::execute(LabelComponent* comp)
            {
				
            }

			//https://www.freetype.org/freetype2/docs/tutorial/example2.cpp
            // Set up the raster parameters and render the outline.

		void ColorEffect::execute(LabelComponent* comp)
		{
			Color4B tmpFontClr = iter.ignoreColor ? Color4B(iter.color.r, iter.color.g, iter.color.b,
				static_cast<GLubyte>((static_cast<float>(iter.color.a) / 255.0f) * this->getDisplayedOpacity())) :
				Color4B(mFontColor.r, mFontColor.g, mFontColor.b,
					static_cast<GLubyte>((static_cast<float>(mFontColor.a) / 255.0f) * this->getDisplayedOpacity()));
		}
    }
}