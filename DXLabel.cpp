#include "DXLabel.h"
//----------- DX ---------
#include "DXLabelParser.h"
#include "DXLabelEscapeParser.h"
#include "DXLabelBBCodeParser.h"
#include "DXLabelEffect.h"
#include "DXShaper.h"

#include "base/ccUTF8.h"
#include "base/CCDirector.h"
#include "platform/CCPlatformMacros.h"

namespace cocos2d
{
	namespace ui
	{
		DXLabel::DXLabel()
		{
			_director = Director::getInstance();

			_registeParser("", nullptr);
		}

		DXLabel::~DXLabel()
		{
		}

		bool DXLabel::init()
		{
			m_shaper = new DXShaper();
			m_bitmapGen = new DXLabelBitmapGenerater();
		}

		void DXLabel::setContent(const std::string &text)
		{
			if (0 != m_text.compare(text))
			{
				m_text = text;
			}
		}

		// ���ݵ�ǰchar��ȡ��Ӧ�Ľ�����
		void DXLabel::_getParserForChar(char ch, std::vector<DXLabelParseOper*>& matchParserList)
		{
			std::string strCh;
			for (auto const &ent : m_parserMap)
			{
				std::string strSymbol = ent.first;
				if (0 == strSymbol.compare(strCh))
				{
					matchParserList.emplace_back(ent.second);
				}
			}
		}

		void DXLabel::_registeParser(std::string symbol, DXLabelParseOper* parser)
		{
			if (parser != nullptr)
			{
				m_parserMap[symbol] = parser;
			}
			// test
			m_parserMap["["] = new DXLabelBBCodeParser();// BBCode
			m_parserMap["\\"] = new DXLabelEscapeParser();// 转义

		}

		// 塑形之后得到glyphindex
		void DXLabel::_shap(const std::u16string &text)
		{
			m_shaper->doShap(m_charList);
		}

		// 栅格化，根据特效获取Bitmap，且完善glyph的属性
		void DXLabel::_rasterize()
		{
			for(auto glyph : m_charList)
			{
				m_bitmapGen->getBitmap(glyph);
			}
		}

		// 排版，是一种有规则的装箱算法
		void DXLabel::_typo(const std::u16string &text)
		{
			int cur_offset = 0;
			int iMax = text.length();

			int curLine = 0;
			Size maxSize;
			Size curLineSize;
			bool canBreak = true;
			int lastBreakPtr = 0;
			float pen_x = 0;
			float pen_y = 0;
			// {linenumber, comp}
			while (cur_offset < iMax)
			{
				// 有没有action？
				if(actionMap.find(cur_offset)!=actionMap.end())
				{
					if(actionMap[cur_offset] == ActionStyle.BreakLine)
					{
						curLine++;
						continue;
					}
					if(actionMap[cur_offset] == ActionStyle.WeakBreakLine)
					{
						lastBreakPtr = cur_offset;
					}
				}
				// 不可分割状态判断
				// 英语环境下空格可分，
				// 其他大多语言环境下，就是当前即可分；

				// 
				Glyph curGlyph = m_charList[cur_offset];
				if(curLineWidth + curGlyph.x_advance > curLineMaxWidth)
				{
						auto lastBreakPtr = revertToLastBreakPtr();
						if(lastBreakPtr != 0)//如果当前行放不下，当前是不可分割状态，且分割点是行首，说明无论如何都放不下，就强制换行
						{
							cur_offset = lastBreakPtr;
						}
						curLine ++;
				}
				else
				{
					// 计算uv
				}
			}
		}


		DXLabelBBCodeParser bbCodeParser;
		bool DXLabel::_parse(const std::u16string &text)
		{
			std::string strUtf8;
			StringUtils::UTF16ToUTF8(text, strUtf8);

			int cur_offset = 0;
			int end_offset;
			int iLength = strUtf8.size();
			LabelComponent* ptrComp = nullptr;
			LabelAction* ptrAction = nullptr;

			while (cur_offset != iLength)
			{
				// 如果有解析器可以解析
				//_getParserForChar(strUtf8[cur_offset], matchParserList);

				// BBCode解析后只是对Effect的栈操作
				bool rs = bbCodeParser.TryParse(strUtf8, cur_offset, end_offset);
				if (rs == true)
				{
					cur_offset = end_offset;
					continue;
				}
				// Escape解析会有一些操作,插入img和换行都可以视为操作吧，单词分割符
				LabelAction action;
				bool rs = DXLabelEscapeParser::TryParse(strUtf8, cur_offset, end_offset, &action);
				if (rs == true)
				{
					actionMap[cursorIndex] = action;//当前位置插入一个动作，后面排版使用
					cur_offset = end_offset;
					continue;
				}

				// 如果都没有解析道，说明是个普通字符
				Glyph glyph;
				glyph.effect = DXLabelBBCodeParser::getCurEffectStyle();
				glyph.charCode = text[cur_offset];
				m_charList.emplace_back(glyph);


			}

			return true;
		}

		void DXLabel::draw(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
		{
			
		}
		void DXLabel::visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags)
		{

			if (!StringUtils::UTF8ToUTF16(m_text, m_reuseU16Text))
				return;

			_parse(m_reuseU16Text);

			uint32_t flags = processParentFlags(parentTransform, parentFlags);
			_director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);
			this->draw(renderer, _modelViewTransform, flags);

		}


	}
}
