#ifndef _CUSTOM_DXLabelEffect
#define _CUSTOM_DXLabelEffect

#include "DXLabelParser.h"
#include "2d/CCFontFreeType.h"
#include "hb-ft.h"

namespace cocos2d
{
    namespace ui
    {       
		struct Glyph;
		enum class EffectStyle : int
		{
			None = 0,
			EffectStyle_Bold = 1 << 0,
			EffectStyle_Underline = 1 << 1,
			EffectStyle_Italic = 1 << 2,
			EffectStyle_Outline = 1 << 3,
			EffectStyle_Color = 1 << 4,

			EffectStyle_All = (EffectStyle_Bold | EffectStyle_Underline 
												| EffectStyle_Italic 
												| EffectStyle_Outline 
												| EffectStyle_Color),
		};

		// 根据特效生成bitmap是有关联性的，必须要一起考虑
		class DXLabelBitmapGenerator
		{
		public:
			bool getBitmap(Glyph* _glyph);
			
			bool getNormalBitmap(Glyph* _glyph);
			bool setRotateBitmap(Glyph* _glyph);
			bool setOutLineBitmap(Glyph* _glyph);
		};

		struct ColorEffect : public DXLabelBitmapGenerator{
		public:
			void execute(LabelComponent* comp);
		private:

			// cache
			FT_Matrix lastMatrix;              /* transformation matrix */
			FT_Vector lastPen;
			EffectStyle lastEffectStyle; // 
			cocos2d::Color4B m_fontColor;
        };
    }
}
#endif//_CUSTOM_DXLabelEffect