#ifndef _CUSTOM_DXLabelEscapeParser
#define _CUSTOM_DXLabelEscapeParser

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

#define WEC_LABEL_VBO_SIZE 65536

// CUSTOM_INCLUDE_END


namespace cocos2d
{

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
		struct LabelBreakAction : LabelAction
		{
            
		};
		/**
		* ��һ��������ת����Ľ���
		*/
		class DXLabelEscapeParser : public DXLabelParseOper
		{
		public:
			virtual bool TryParse(const std::string& text, 
            int begin_offset, 
            int& end_offset, 
			LabelAction** refAction)
			{
				return false;
			}
		protected:
		private:
		};
						
	}

}  // namespace cocos2d
#endif  // _CUSTOM_WIDGET_DXLabelParser
