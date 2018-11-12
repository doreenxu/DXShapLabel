#include "DXLabelParser.h"


namespace cocos2d
{
    namespace ui
    {       
		class UnderlineEffect : public LabelEffect
		{
            // 效果对象会对component的triangle属性进行修改
			void execute(LabelComponent* comp)
            {
                //原实现是在每个字符加入“_“下划线，这样实现，下划线之间会有间隙
                //考虑加入一条（textwidth，1）的线，
            }
            
		};
		class BoldEffect : public LabelEffect
		{
			void execute();
		};
		class OutlineEffect : public LabelEffect
		{
            void execute(LabelComponent* comp)
            {

            }

            //https://www.freetype.org/freetype2/docs/tutorial/example2.cpp
            // Set up the raster parameters and render the outline.
            void
            RenderSpans(FT_Library &library,
                        FT_Outline * const outline,
                        Spans *spans) 
            {
            FT_Raster_Params params;
            memset(&params, 0, sizeof(params));
            params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
            params.gray_spans = RasterCallback;
            params.user = spans;

            FT_Outline_Render(library, outline, &params);
            }
		};
		class ShadowEffect : public LabelEffect
		{

		};
        class ColorEffect : public LabelEffect{
            public:
            void execute(LabelComponent* comp)
            {
                 Color4B tmpFontClr = iter.ignoreColor ? Color4B(iter.color.r, iter.color.g, iter.color.b,
                    static_cast<GLubyte>((static_cast<float>(iter.color.a) / 255.0f) * this->getDisplayedOpacity())) :
                    Color4B(mFontColor.r, mFontColor.g, mFontColor.b,
                    static_cast<GLubyte>((static_cast<float>(mFontColor.a) / 255.0f) * this->getDisplayedOpacity()));
            }
			cocos2d::Color4B m_fontColor;
        };
    }
}