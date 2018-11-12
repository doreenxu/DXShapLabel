		class DXFontMaterialGenerator
		{
		public:
			typedef struct {
				unsigned char* buffer;
				unsigned int width;
				unsigned int height;
				float bearing_x;
				float bearing_y;
			} Glyph;

			void AddCharactor(unsigned short charCode)
			{
				
			}

			void GetQuadByGlyphIndex()
			{
				
			}


			void AddGlyph(int glyphIndex, bool outline = false, int fontSize = 20)
			{
				
			}
			Bitmap GetBitmap(int glyphIndex, bool outline = false)
			{
				Glyph glyphInfo;
				FT_Int32 flags = FT_LOAD_DEFAULT;
				FT_Load_Glyph(*m_ftface, glyphIndex, // the glyph_index in the font file
					flags
				);

				FT_GlyphSlot slot = (*m_ftface)->glyph;
				FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

				FT_Bitmap ftBitmap = slot->bitmap;

				glyphInfo.buffer = ftBitmap.buffer;
				glyphInfo.width = ftBitmap.width;
				glyphInfo.height = ftBitmap.rows;
				glyphInfo.bearing_x = slot->bitmap_left;
				glyphInfo.bearing_y = slot->bitmap_top;

				Rect outRect;
				auto& metrics = slot->metrics;
				outRect.origin.x = metrics.horiBearingX >> 6;
				outRect.origin.y = -(metrics.horiBearingY >> 6);
				outRect.size.width = (metrics.width >> 6);
				outRect.size.height = (metrics.height >> 6);
				xadvance = (static_cast<int>(slot->metrics.horiAdvance >> 6));
				ascender = static_cast<int>((*m_ftface)->size->metrics.ascender >> 6);


				// TODO outline
				return glyphInfo;
			}
			void InsertAtals(int glyphIndex, bool outline = false, int fontSize = 20)
			{
				//TODO 需要先设置一下FT的属性，FontSize, Stroker(outline)
				
			}
			private:
				FT_Face* m_ftface;

				std::map<std::string, FontAtlas> atlasMap; // {fontKey, atlas}
				std::vector<Glyph> glyphinfoList; // 

				int WeCCharFontManager::sAtlasWidth = 1024;
				int WeCCharFontManager::sAtlasHeight = 1024;
				int WeCCharFontManager::sAtlasLineOffset = 2;
				float WeCCharFontManager::sFontScale = 1.0f;
				bool WeCCharFontManager::sHasInit = false;
				cocos2d::Mat4 WeCCharFontManager::sProjectionMat = cocos2d::Mat4::IDENTITY;
				GLProgramState* WeCCharFontManager::sWecLabelProgram = nullptr;
				GLProgramState* WeCCharFontManager::sWecLabelImgProgram = nullptr;
				GLProgramState* WeCCharFontManager::sWecLabelImgETCProgram = nullptr;
				GLProgramState* WeCCharFontManager::sWecLabelImgPackAlphaHorizontalProgram = nullptr;
				GLProgramState* WeCCharFontManager::sWecLabelImgPackAlphaVerticalProgram = nullptr;

		};