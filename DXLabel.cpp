#include "DXLabel.h"
//----------- DX ---------
#include "DXLabelParser.h"
#include "DXLabelEscapeParser.h"
#include "DXLabelBBCodeParser.h"
#include "DXLabelEffect.h"
#include "DXShaper.h"
#include "DXBreakParser.h"

#include "base/ccUTF8.h"
#include "base/CCDirector.h"
#include "platform/CCPlatformMacros.h"

namespace cocos2d
{
	namespace ui
	{
		DXLabel::DXLabel()
		{
#if CC_LABEL_DEBUG_DRAW
            mDebugDrawNode = DrawNode::create();
            addChild(mDebugDrawNode);
#endif
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
			m_breakParser = new DXBreakParser();
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
			for(auto curGlyph : m_charList)
			{
				m_bitmapGen->getBitmap(curGlyph);

				int twidth = pow(2, ceil(log(curGlyph->width)/log(2)));
			    int theight = pow(2, ceil(log(curGlyph->height)/log(2)));

					float s0 = 0.0;
			        float t0 = 0.0;
			        float s1 = (float) curGlyph->width / twidth;
			        float t1 = (float) curGlyph->height / theight;
			        float xa = (float) curGlyph.x_advance;
			        float ya = (float) curGlyph.y_advance;
			        float xo = (float) curGlyph.x_offset;
			        float yo = (float) curGlyph.y_offset;
					float x0 = pen_x + xo + curGlyph->bearing_x;
			        float y0 = floor(pen_y + yo + curGlyph->bearing_y);
			        float x1 = x0 + curGlyph->width;
			        float y1 = floor(y0 - curGlyph->height);

			        gl::Vertex* vertices = new gl::Vertex[4];
			        vertices[0] = gl::Vertex(x0,y0, s0,t0);
			        vertices[1] = gl::Vertex(x0,y1, s0,t1);
			        vertices[2] = gl::Vertex(x1,y1, s1,t1);
			        vertices[3] = gl::Vertex(x1,y0, s1,t0);

			        unsigned short* indices = new unsigned short[6];
			        indices[0] = 0; indices[1] = 1;
			        indices[2] = 2; indices[3] = 0;
			        indices[4] = 2; indices[5] = 3;
					// 计算uv
					auto &mat = ch->getNodeToParentTransform();
                    auto q = ch->getRotationQuat();
                    Mat4 rot;
                    Mat4::createRotation(q, &rot);
                    Mat4 curMat = rot * mat;
                    auto quad = ch->getQuad();
                    curMat.transformPoint(&(quad.bl.vertices));
                    curMat.transformPoint(&(quad.br.vertices));
                    curMat.transformPoint(&(quad.tl.vertices));
                    curMat.transformPoint(&(quad.tr.vertices));
                    found->second.emplace_back(quad);
			}

		}

		DXBreakParser* DXLabel::getBreakParserByLan()
		{
			//判断当前是什么语种，
			std::string lan = "";
			return m_breakParserMap[lan];
		}

		// 排版，是一种有规则的装箱算法
		std::vector<float> lineOffset;
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
			// step1. 排列每一行
			while (cur_offset < iMax)
			{
				pen_y = curLine * lineHeight;

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
				Glyph curGlyph = m_charList[cur_offset];
				m_breakParser->insertChar(curGlyph.charCode);

				if(curLineWidth + curGlyph.x_advance > curLineMaxWidth)
				{
					// 如果当前语种有断词规则，则回退到上一个断词点
					if(m_breakParser!=nullptr)
					{
						auto lastBreakPtr = m_breakParser->revertToLastBreakPtr();
						if(lastBreakPtr != 0)//如果当前行放不下，当前是不可分割状态，且分割点是行首，说明无论如何都放不下，就强制换行
						{
							cur_offset = lastBreakPtr;
						}
					}
					// 没有断词规则的话就开始新的一行
					curLine ++;
				}
				else
				{
					pen_x += curGlyph.x_advance;
					cur_offset++;
				}

				curGlyph.pen_x_offset = pen_x;
				curGlyph.pen_y_offset = pen_y;
			}

			//step2.整体排列
			// Process line by verAlign.
            float yOffset = 0;
            float tmpTextHeight = (lineIndex + 1) * lineHeightOffset - mSpacingY * mScale;
            switch (mVerAlign)
            {
            case cocos2d::ui::WeCLabel::VerAlign::Top:
                yOffset = contentSize.height;
                break;
            case cocos2d::ui::WeCLabel::VerAlign::Center:
                yOffset = (tmpTextHeight * scale + contentSize.height) * 0.5f;
                break;
            case cocos2d::ui::WeCLabel::VerAlign::Bottom:
                yOffset = tmpTextHeight * scale;
                break;
            default:
                break;
            }

            // Process the line by the HorAlign.
            for (unsigned int i = 0; i < lineOffset.size(); ++i)
            {
                switch (mHorAlign)
                {
                case cocos2d::ui::WeCLabel::HorAlign::Left:
                    lineOffset[i] = 0;
                    break;
                case cocos2d::ui::WeCLabel::HorAlign::Center:
                    lineOffset[i] = (contentSize.width - lineOffset[i] * scale) * 0.5f;
                    break;
                case cocos2d::ui::WeCLabel::HorAlign::Right:
                    lineOffset[i] = contentSize.width - lineOffset[i] * scale;
                    break;
                default:
                    break;
                }
            }

            //step3.构建quads
            
		}


		DXLabelBBCodeParser bbCodeParser;
		DXLabelEscapeParser escapeParser;
		bool DXLabel::_parse(const std::u16string &text)
		{
			std::string strUtf8;
			StringUtils::UTF16ToUTF8(text, strUtf8);

			int cur_offset = 0;
			int end_offset;
			int iLength = strUtf8.size();

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
				bool rs = escapeParser.TryParse(strUtf8, cur_offset, end_offset, &action);
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
			auto mvMat = parentTransform;
            float scale = WeCCharFontManager::getInstance()->sFontScale;

            auto mask = this->getCameraMask();
            if (mask & static_cast<unsigned short>(CameraFlag::DEFAULT))
            {
                // Ceil the transform matrix, to avoid sample problems.
                mvMat.m[12] = std::ceil(mvMat.m[12] * scale);
                mvMat.m[13] = std::ceil(mvMat.m[13] * scale);
                mvMat.m[14] = std::ceil(mvMat.m[14] * scale);
            }
            else
            {
                mvMat.m[0] = mvMat.m[0] / scale;
                mvMat.m[5] = mvMat.m[5] / scale;
                mvMat.m[10] = mvMat.m[10] / scale;
            }

            int i = 0;
            for (auto &iter : mQuads)
            {
                mQuadCmd[i].init(this->getGlobalZOrder(), iter.first,
                    this->getGLProgramState(), mBlend, iter.second.data(),
                    iter.second.size(), mvMat, parentFlags);

                mQuadCmd[i].setSkipBatching(false);

                renderer->addCommand(&mQuadCmd[i]);
                ++i;
            }

            i = 0;
            for (auto &iter : mImgQuads)
            {
                // Fix the bug of ETC.
                auto alphaFound = mQuadAlphas.find(iter.first);

                auto program = alphaFound == mQuadAlphas.end() ?
                    GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_WEC_LABEL_IMG) :
                    GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_WEC_LABEL_IMG_ETC);

                mImgQuadCmd[i].init(this->getGlobalZOrder(), iter.first, program, mBlend, iter.second.data(),
                    iter.second.size(), mvMat, parentFlags);

                mImgQuadCmd[i].setSkipBatching(false);
                // mImgQuadCmd[i].setTextureID2(alphaFound == mQuadAlphas.end() ? 0 : alphaFound->second);

                renderer->addCommand(&mImgQuadCmd[i]);
                ++i;
            }
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
