
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
#include "2d/WeCChartFontManager.h"

#include "external/HarfBuzz/hbshaper.h"
#include "DXLabelParser.h"
#include "i18n/I18nMgr.h"

#define WEC_LABEL_VBO_SIZE 65536

// CUSTOM_INCLUDE_END

#ifndef _CUSTOM_WIDGET_DXLabel
#define _CUSTOM_WIDGET_DXLabel

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
		class CC_DLL DXLabel : public Widget
		{
			DECLARE_CLASS_GUI_INFO

			/*  Custom code block. You should write
				your own code in this block only. Do not
				change the Tag "CUSTOM_CODE_BEGIN" &
				"CUSTOM_CODE_END", The generator use these
				tags to determine the block which need to
				copy at next auto-generation.
				*/

				// CUSTOM_CODE_BEGIN
		public:
			friend class CC_DLL cocos2d::WeCCharFontManager;

			enum class HorAlign
			{
				Left = 0,
				Center = 1,
				Right = 2,
			};

			enum class VerAlign
			{
				Top = 0,
				Center = 1,
				Bottom = 2,
			};

			enum class Overflow
			{
				Shrink = 0,
				Clamp = 1,
				ResizeFreely = 2,
				ResizeHeight = 3,
			};

			enum class Effect
			{
				None = 0,
				Shadow = 1,
				Outline = 2,
			};

		public:
			virtual bool init() override;
			Size         getLabelSize();

			virtual void setContentSize(const Size& contentSize) override;

			virtual void onEnter() override;
			virtual void onExit() override;

			int          getLineCount() const;
			void         forceProcess();

#ifdef CC_STUDIO_ENABLED_VIEW
			virtual int  getHorAlign() const;
			virtual int  getVerAlign() const;
			virtual int  getOverflow() const;
			virtual int  getEffect() const;
#else
			virtual HorAlign getHorAlign() const;
			virtual VerAlign getVerAlign() const;
			virtual Overflow getOverflow() const;
			virtual Effect getEffect() const;
#endif

#ifdef CC_STUDIO_ENABLED_VIEW
			virtual void setHorAlign(int val);
			virtual void setVerAlign(int val);
			virtual void setOverflow(int val);
			virtual void setEffect(int val);
#else
			virtual void setHorAlign(HorAlign val);
			virtual void setVerAlign(VerAlign val);
			virtual void setOverflow(Overflow val);
			virtual void setEffect(Effect val);
#endif
			/*
			首先就纯粹解析文本，再去收集字符；

			MeshGen：顶点信息生成器
			MaterialGen：材质生成器
			Renderer：输入atlas，quads；
			*/
			virtual void visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags) override
			{
				Parse(m_oriText);
				Shap();
			}
			virtual void draw(Renderer *renderer, const Mat4& transform, uint32_t flags) override;

			void AddParser(LabelParseOper* _parser);
			/*
			首先是不考虑塑形的解析
			*/
			void Parse(std::string text)
			{
				LabelComponent* ptrComp = nullptr;
				bool findParser = false;
				int parserIndex = 0;
				int begin_offset = 0;
				int end_offset = 0;
				while (!findParser)
				{
					auto parser = m_parserList[parserIndex];
					findParser = parser->TryParse(text, 0, end_offset, &ptrComp);
					if (findParser)
					{
						m_componentList.push_back(ptrComp);
					}
					++parserIndex;
				}
			}

			void CreateMaterial()
			{

			}

			/* shap的时候，

			*/
			void Shap()
			{

				for (auto word : m_componentList)
				{

				}
			}

			/*
			统计需要显示的字符
			*/
			void Collect()
			{

			}

			private:
				std::string m_oriText; // 原始文本
				std::vector<LabelParseOper*> m_parserList; // 文本解析器
				std::vector<LabelComponent*> m_componentList;// 文本元素
				HBShaper* m_shaper;

				std::string m_fontName;
				std::string m_fontKey;// 每个字体+字号对应一个唯一key
		};


		//////////////////////////////////////////////////////////////////////////
		

		//////////////////////////////////////////////////////////////////////////
		class DXFontManager
		{
			//TODO
			//操作Freetype
			void AddCharactor();
		};

		// 排版
		/*

		*/
		class TypoOper
		{

		public: 
			

		private:
			WeCChartFontDefinitionPtr m_ptrCurFontDefinition;

		};
		
	}

}  // namespace cocos2d
#endif  // _CUSTOM_WIDGET_
