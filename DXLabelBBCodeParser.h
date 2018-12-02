#ifndef _CUSTOM_DXLabelBBCodeParser
#define _CUSTOM_DXLabelBBCodeParser

#include "base/ccTypes.h"

/*  Custom include block. You should write
    your own include in this block only. Do not
    change the Tag "CUSTOM_INCLUDE_BEGIN" &
    "CUSTOM_INCLUDE_END", The generator use these
    tags to determine the block which need to
    copy at next auto-generation.
    */

    // CUSTOM_INCLUDE_BEGIN
#include <string>
#include <array>
#include <vector>
#include <unordered_map>

#include "2d/CCNode.h"
#include "2d/CCSprite.h"
#include "renderer/CCQuadCommand.h"
#include "ui/UIWidget.h"
//#include "external/HarfBuzz/hbshaper.h"
#include "DXLabelParser.h"
#include "DXLabelEffect.h"

// CUSTOM_INCLUDE_END



namespace cocos2d
{

    class CC_DLL WeCCharFontManager;
    //class CC_DLL LabelForSystemFont;

    /*  Custom namespace block. You should write
        your own include in this block only. Do not
        change the Tag "CUSTOM_NAMESPACE_START_BEGIN" &
        "CUSTOM_NAMESPACE_START_END", The generator use these
        tags to determine the block which need to
        copy at next auto-generation.
        */

        // force namespace to ui, because the auto generate binding config is only one namespace.
    namespace ui
    {
    	struct BBCodeItem
    	{
    		EffectStyle m_style;

    		std::string m_beginSymble;
			std::string m_endSymble;
			std::string m_param;//参数用空格分割，[img scale=1 path=2] 
		};

		struct ImgItem
		{
			std::string m_beginSymble;
			std::string m_endSymble;
			std::string m_imgPath;
		};
		/*
		*/
		class DXLabelBBCodeParser : public DXLabelParseOper
		{
		public:
			void init()
			{
				addBBCodeItem({ '[', 'b', ']' }, { '[', '/','b', ']' }, EffectStyle::EffectStyle_Bold);
				addBBCodeItem({ '[', 'u', ']' }, { '[', '/','u', ']' }, EffectStyle::EffectStyle_Underline);
				addBBCodeItem({ '[', 'i', ']' }, { '[', '/','i', ']' }, EffectStyle::EffectStyle_Italic);
				addImageItem({ '[','i','m','g','=' }, { '[', '-', ']' });
			}

			virtual bool TryParse(const std::string& text, int begin_offset, int& end_offset)
			{
				for(auto item : m_bbcodeItemVec)
				{
					if (item.m_beginSymble.empty() || item.m_endSymble.empty())
						continue;

					size_t foundBegin;
					size_t foundEnd;

					std::string subText = text.substr(begin_offset, text.length() - begin_offset);
					foundBegin = text.find(item.m_beginSymble);
					if (foundBegin != std::string::npos)
					{
						pushEffect(item.m_style);
						end_offset = foundBegin + item.m_beginSymble.length();
						return true;
					}
					foundEnd = text.find(item.m_endSymble);
					if (foundEnd != std::string::npos)
					{
						popEffect(item.m_style);
						end_offset = foundBegin + item.m_beginSymble.length();
						return true;
					}
				}

				for (auto item : m_imgItemVec)
				{

				}
				return false;
			}

			virtual EffectStyle GetCurrentStyle()
			{
				return m_curStyle;
			}

			void pushEffect(EffectStyle _style)
			{
				m_curStyle = (EffectStyle)((int)m_curStyle | (int)_style);
			}
			void popEffect(EffectStyle _style)
			{
				auto a = (int)_style ^ 1;
				m_curStyle = (EffectStyle)((int)m_curStyle & a);
			}

			void addBBCodeItem(std::string beginSymble, std::string endSymble, EffectStyle _style)
			{
				BBCodeItem item;
				item.m_beginSymble = beginSymble;
				item.m_endSymble = endSymble;
				item.m_style = _style;
				m_bbcodeItemVec.emplace_back(item);
			}

			void addImageItem(std::string beginSymble, std::string endSymble)
			{
				ImgItem item;
				item.m_beginSymble = beginSymble;
				item.m_endSymble = endSymble;
				m_imgItemVec.emplace_back(item);
			}


		protected:
			std::vector<BBCodeItem> m_bbcodeItemVec;
			std::vector<ImgItem> m_imgItemVec;
			EffectStyle m_curStyle;
		};
		

		class DXLabelBitmapGenerater
		{

		};

		/*
		// [b]
        static const std::u16string BOLD_TAG_BEGIN = { '[', 'b', ']' };
        // [/b]
        static const std::u16string BOLD_TAG_END = { '[', '/', 'b', ']' };
        // [i]
        static const std::u16string ITALIC_TAG_BEGIN = { '[', 'i', ']' };
        // [/i]
        static const std::u16string ITALIC_TAG_END = { '[', '/', 'i', ']' };
        // [u]
        static const std::u16string UNDERLINE_TAG_BEGIN = { '[', 'u', ']' };
        // [/u]
        static const std::u16string UNDERLINE_TAG_END = { '[', '/', 'u', ']' };
        // [s]
        static const std::u16string STRIKE_TAG_BEGIN = { '[', 's', ']' };
        // [/s]
        static const std::u16string STRIKE_TAG_END = { '[', '/', 's', ']' };
        // [c]
        static const std::u16string COLOR_TAG_BEGIN = { '[', 'c', ']' };
        // [/c]
        static const std::u16string COLOR_TAG_END = { '[', '/', 'c', ']' };
        // [sub]
        static const std::u16string SUB_TAG_BEGIN = { '[', 's', 'u', 'b', ']' };
        // [/sub]
        static const std::u16string SUB_TAG_END = { '[', '/', 's', 'u', 'b', ']' };
        // [sup]
        static const std::u16string SUP_TAG_BEGIN = { '[', 's', 'u', 'p', ']' };
        // [/sup]
        static const std::u16string SUP_TAG_END = { '[', '/', 's', 'u', 'p', ']' };
        //[-]
        static const std::u16string STRIP_TAG_END = { '[', '-', ']' };
        //[@]
        static const std::u16string RAW_TAG_BEGIN = { '[', '@', ']' };
        //[/@]
        static const std::u16string RAW_TAB_END = { '[', '/', '@', ']' };
		*/

						
	}

}  // namespace cocos2d
#endif  // _CUSTOM_WIDGET_
