
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
#include "external/HarfBuzz/hbshaper.h"
#include "DXLabelParser.h"

// CUSTOM_INCLUDE_END

#ifndef _CUSTOM_DXLabelBBCodeParser
#define _CUSTOM_DXLabelBBCodeParser

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
    	}
		/*
		*/
		class DXLabelBBCodeParser : public DXLabelParseOper
		{
		public:

		protected:
			virtual void initRule(const std::string& text)
			{
			}
			virtual bool TryParse(const std::string& text, 
			int begin_offset, int& end_offset)
			{
				initRule(text);

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
						end_offset = foundBegin + item.m_beginSymble;
						return true;
					}
					foundEnd = text.find(item.m_endSymble);
					if (foundEnd != std::string::npos)
					{
						popEffect(item.m_style);
						end_offset = foundBegin + item.m_beginSymble;
						return true;
					}
				

					// 修改思路：不要去找结尾符号，这样不易嵌套，
					// foundEnd = text.find(m_endSymble, foundBegin+1);
					// if (foundEnd != std::string::npos)
					// {
					// 	end_offset = foundEnd + m_endSymbleSize;
					// 	std::string content = subText.substr(m_beginSymbleSize + 1, foundEnd - foundBegin - m_beginSymbleSize);

					// 	*refComp = CreateComponent();
					// 	if (*refComp != nullptr)
					// 	{
					// 		(*refComp)->setContent(content.c_str());
					// 	}
					// 	return true;
					// }
				}
				return false;
			}

			virtual EffectStyle GetCurrentStyle()
			{

			}

			void pushEffect()
			{
				m_curStyle = m_curStyle | 
			}
			void popEffect()
			{

			}


		protected:
			


			std::vector<BBCodeItem> m_bbcodeItemVec;
			EffectStyle m_curStyle;
		};

		// ����BBCodeParser�����ƥ��ɹ�������һ��BoldComponent��Ʒ
		class BBCodeBoldParser : public DXLabelBBCodeParser
		{
		public:
			virtual void initRule() 
			{
				m_beginSymble = { '[', 'b', ']' };
				m_endSymble = { '[', '/', 'b', ']' };
				m_beginSymbleSize = 3;
				m_endSymbleSize = 4;
			}
		protected:
			// TODO： effect可以加入参数
			virtual LabelEffect* CreateEffect()
			{
				return new BoldEffect();
			}
		};

		// 这种类可以用宏定义生成
		class UnderlineParser : public DXLabelBBCodeParser
		{
		public:
			virtual void initRule() 
			{
				m_beginSymble = { '[', 'u', ']' };
				m_endSymble = { '[', '/', 'u', ']' };
				m_beginSymbleSize = 3;
				m_endSymbleSize = 4;
			}
		protected:
			// TODO： effect可以加入参数
			virtual LabelEffect* CreateEffect()
			{
				return new UnderlineEffect();
			}
		};

		class ColorParser : public DXLabelBBCodeParser
		{
		public:
			virtual void initRule() 
			{
				m_beginSymble = { '[', 'i','m','g' };
				m_endSymble = { '[', '-', ']' };
				m_beginSymbleSize = 4;
				m_endSymbleSize = 3;
			}

			virtual void parseParam()
			{
				imageName
			}
		protected:
			// TODO： effect可以加入参数
			virtual LabelEffect* CreateEffect()
			{
				return new UnderlineEffect();
			}
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
