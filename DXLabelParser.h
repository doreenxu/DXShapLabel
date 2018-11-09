
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
#include "hbshaper.h"
#include "WeCChartFontManager.h"

#define WEC_LABEL_VBO_SIZE 65536

// CUSTOM_INCLUDE_END

#ifndef _CUSTOM_DXLabelParseOper
#define _CUSTOM_DXLabelParseOper

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

		//---------- ��Ʒ ----------
		struct LabelEffect
		{

		};

		struct LabelAction 
		{

		};

		struct LabelComponent 
		{
		public: 
			static LabelComponent* create()
			{
				auto comp = new LabelComponent();
				if (comp && comp->init())
				{
					return comp;
				}
			}

			bool init() { return true; }
			const std::string& getCContent() { return m_content; }
			void setContent(const char* _content) { m_content = _content; }


			void addEffect(LabelEffect* _eff) 
			{
				if (!_eff) 
					return;

				m_effList.push_back(_eff); 
			}

			// ���߶�
			size_t getTextMaxHeight() { return 0; }
			// ����
			size_t getTextMaxWidth() { return 0; }


		private:
			std::string m_content;

			unsigned int m_fontSize;
			// color
			cocos2d::Color4B m_fontColor;

			// shap
			unsigned int glyph_count;
			hb_glyph_info_t *glyph_info;
			hb_glyph_position_t *glyph_pos;

			// 
			std::map<unsigned short, unsigned int> m_glyphIndexMap;
			// style, outline, shadow
			std::vector<LabelEffect*> m_effList;
			// render
			std::vector<V3F_C4B_T2F_Quad> m_quads;

		};


		// ���н����Č���
		/*ϣ�������ɿɲ���ʽ�Ľ����������磬������text����'['��ѯ�����е�
		������˭֪�������ɶ��BBCodeParser˵����֪����Ȼ���������text
		�ù�ȥ���Ž���һ�£���tryParser��ȷ��������Ҫ�ĸ�ʽ������ɹ������ؽ�����
		�Ĳ��һ��LabelWord��label�е���С����飩�����򷵻ؽ���ʧ��,
		*/

		class DXLabelParseOper
		{
		public:
			virtual bool TryParse(const std::string& text, 
			int begin_offset, 
			int& end_offset, 
			LabelComponent** refComp,
			LabelAction** refAction)
			{
				return false;
			}

			// 
			//virtual bool TryParse(const std::string& text, int begin_offset, int& end_offset, LabelComponent& refComp) { return false; }

		};


				
	}

}  // namespace cocos2d
#endif  // _CUSTOM_WIDGET_
