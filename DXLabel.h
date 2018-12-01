#ifndef _CUSTOM_DXLabel
#define _CUSTOM_DXLabel

#include <map>
#include <vector>
#include "ui/UIWidget.h"
#include "DXLabelBBCodeParser.h"

namespace cocos2d
{
	namespace ui
	{
		class LabelComponent;
		class LabelAction;
		class DXLabelParseOper;
		class DXShaper;
		class DXLabelBitmapGenerater;
		class DXBreakParser;


		// 一个字形渲染时需要用到的属性，此Glyph生成与Parse阶段，经历Shape，Sample，Rasterize 阶段，
		struct Glyph{
				unsigned int charCode;
				unsigned int glyphIndex;
				float fontSize;
				float bearing_x;
				float bearing_y;
				float x_advance;
				float y_advance;
				float x_offset;
				float y_offset;

				int fontAtlasTexIndex;
				float fontAtlasU;
				float fontAtlasV;
				unsigned char* buffer;
				unsigned int width;
				unsigned int height;
				EffectStyle effect;//特效关系到bitmap采样方式，且需要与FT结合
				Color4B color;
				int language;//语言不完全和unicode符合，需要在便利解析时根据上下文情况
			} ;

		class DXLabel : public Widget
		{
		public:
			DXLabel();
			~DXLabel();

			bool init();
			void setContent(const std::string &text);
			void draw(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags);
			void visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags);

		protected:
			bool _parse(const std::u16string &text);
			void _registeParser(std::string symbol, DXLabelParseOper* parser);
			void _getParserForChar(char ch, std::vector<DXLabelParseOper*>& matchParserList);
			void _typo(const std::u16string &text);
			void _shap(const std::u16string &text);
			void _rasterize();

		private:
			std::string m_text;
			std::u16string m_reuseU16Text;
			// {offset, comp}
			std::map<int, LabelComponent* > m_compMap;
			std::vector<Glyph> m_charList;


			std::map<int, LabelAction* > actionMap;
			// ÓÃparser¹ØÐÄµÄchar×¢²áparser
			std::map<std::string, DXLabelParseOper*> m_parserMap;
			DXShaper* m_shaper;
			DXLabelBitmapGenerater* m_bitmapGen;
			st::map<std::string, DXBreakParser*> m_breakParserMap; //不同的语言，断词规则不同
			//EN：空格处断词;
			//Thai: 使用libThai的断词逻辑；
			//JP： 

			Director* _director;            //cached director pointer to improve rendering performance
			
			typedef std::vector<V3F_C4B_T2F_Quad>           QuadList;
            typedef std::unordered_map<GLuint, QuadList>    QuadListMap;
            QuadListMap m_quadMap;
		};
	}
}
#endif

