
#include <map>
#include <vector>
#include "DXBreakParser.h"
#include "base/ccUTF8.h"

namespace cocos2d
{
	namespace ui
	{
		bool BreakStretagy::insertChar(char* charCode) {
			charList.push_back(*charCode);
			// 空格分割
			if (StringUtils::isUnicodeSpace(*charCode))
			{
				m_breakState = true;
			}
			return m_breakState;
		}
		
		DXBreakParser::DXBreakParser() {
			m_defaultBkStretagy = new BreakStretagy();
		}
		DXBreakParser::~DXBreakParser() {
			m_defaultBkStretagy = nullptr;
		}
		void DXBreakParser::insertChar(char* charCode, int curLan)
		{
			bool currentCanBreak = true;
			if (stretagyMap.size() > 0 && stretagyMap.find(curLan) != stretagyMap.end())
			{
				std::vector<BreakStretagy*>& curLanStretageList = stretagyMap[curLan];
				for (auto stretagy : curLanStretageList)
				{
					currentCanBreak = currentCanBreak & stretagy->insertChar(charCode);
				}
			}
			else
			{
				currentCanBreak = m_defaultBkStretagy->insertChar(charCode);
			}

			// 如果当前字符是短词点
			if (currentCanBreak)
			{
				m_lastBreakPtr = charList.length();
			}

		}
		// 当需要断词时，获取最近一个断词点
		int DXBreakParser::revertToLastBreakPtr()
		{
			return m_lastBreakPtr;
		}

	}
}