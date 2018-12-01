#ifndef _CUSTOM_DXLabel
#define _CUSTOM_DXLabel

#include <map>
#include <vector>

namespace cocos2d
{
	namespace ui
	{

		class BreakStretagy
		{
		public:
			bool canBreak(){ return m_breakState; }

		private:
			bool m_breakState;
		}
		// 根据不同的语言上下文判断当前是否可以断词
		class DXBreakParser
		{
		public:
			DXBreakParser();
			~DXBreakParser();

			// typo的过程中塞入char
			void insertChar(char* charCode)
			{
				m_breakState = true;
				for(auto stretagy : stetagyList)
				{
					if(!stretagy.canBreak())
					{
						m_breakState = false;
					}
				}
			}
			// 当需要断词时，获取最近一个断词点
			int revertToLastBreakPtr()
			{

			}


		protected:
			int m_lastBreakPtr;
			std::vector<BreakStretagy> stetagyList;
		};
	}
}