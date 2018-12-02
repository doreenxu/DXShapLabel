#ifndef _DXFontMaterialGenerator_H_
#define _DXFontMaterialGenerator_H_

#include "hb-ft.h"
#include <string>
#include <map>
#include <vector>
#include "2d/CCFontAtlas.h"

namespace cocos2d
{
	namespace ui
	{

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

			void AddCharactor(unsigned short charCode);
			void GetQuadByGlyphIndex();
			void AddGlyph(int glyphIndex, bool outline = false, int fontSize = 20);
			void InsertAtals(int glyphIndex, bool outline = false, int fontSize = 20);
			std::string generateKey(const std::string &fontName, int fontSize, float outline, bool ignoreScale);
			static void createAtlas(const std::string &fontName, int fontSize, float outline, bool ignoreScale);
		private:
			FT_Face * m_ftface;

			std::map<std::string, FontAtlas> atlasMap; // {fontKey, atlas}
			std::vector<Glyph> glyphinfoList; // 

// 			int WeCCharFontManager::sAtlasWidth = 1024;
// 			int WeCCharFontManager::sAtlasHeight = 1024;
// 			int WeCCharFontManager::sAtlasLineOffset = 2;
// 			float WeCCharFontManager::sFontScale = 1.0f;
// 			bool WeCCharFontManager::sHasInit = false;
// 			cocos2d::Mat4 WeCCharFontManager::sProjectionMat = cocos2d::Mat4::IDENTITY;
// 			GLProgramState* WeCCharFontManager::sWecLabelProgram = nullptr;
// 			GLProgramState* WeCCharFontManager::sWecLabelImgProgram = nullptr;
// 			GLProgramState* WeCCharFontManager::sWecLabelImgETCProgram = nullptr;
// 			GLProgramState* WeCCharFontManager::sWecLabelImgPackAlphaHorizontalProgram = nullptr;
// 			GLProgramState* WeCCharFontManager::sWecLabelImgPackAlphaVerticalProgram = nullptr;

		};
	}
}
#endif
