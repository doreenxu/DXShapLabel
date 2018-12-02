#include "DXFontMaterialGenerator.h"

#define MAX_FONT_KEY_LEN 128
namespace cocos2d
{
	namespace ui
	{
		std::string DXFontMaterialGenerator::generateKey(const std::string &fontName, int fontSize, float outline, bool ignoreScale)
		{
			char key[MAX_FONT_KEY_LEN];
			_snprintf(key, MAX_FONT_KEY_LEN, "%s_%d_%f_%d", fontName.c_str(), fontSize, outline, ignoreScale == true ? 1 : 0);
			std::string str(key);
			return str;
		}

		void DXFontMaterialGenerator::createAtlas(const std::string &fontName, int fontSize, float outline, bool ignoreScale)
		{

		}

		void DXFontMaterialGenerator::AddCharactor(unsigned short charCode)
		{

		}

		void DXFontMaterialGenerator::GetQuadByGlyphIndex()
		{

		}


		void DXFontMaterialGenerator::AddGlyph(int glyphIndex, bool outline, int fontSize)
		{

		}
		void DXFontMaterialGenerator::InsertAtals(int glyphIndex, bool outline, int fontSize)
		{
			//TODO 需要先设置一下FT的属性，FontSize, Stroker(outline)

		}

	};
}
