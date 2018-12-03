#ifndef _CUSTOM_DXBreakParser_H_
#define _CUSTOM_DXBreakParser_H_

#include <map>
#include <vector>

namespace cocos2d
{
	namespace ui
	{
		class BreakStretagy
		{
		public:
			bool insertChar(char* charCode);
		private:
			bool canBreak() { return m_breakState; }
		private:
			bool m_breakState;
			std::string charList;
		};
		// 根据不同的语言上下文判断当前是否可以断词
		class DXBreakParser
		{
		public:
			DXBreakParser();
			~DXBreakParser();

			// typo的过程中塞入char
			void insertChar(char* charCode, int curLan);

		protected:
			int m_lastBreakPtr;
			//{lan, stretagelist} 不同语言有不同断词策略
			BreakStretagy* m_defaultBkStretagy;
			std::map<int, std::vector<BreakStretagy*>> stretagyMap;
			std::string charList;
		};
	}
}
#endif //_CUSTOM_DXBreakParser_H_