#ifndef _DXShaper_H_
#define _DXShaper_H_

#include <string>
#include "hb-ft.h"

#include "DXLabel.h"

namespace cocos2d
{
	namespace ui
	{
		struct LabelComponent;
		class DXShaper
		{
		public:

			typedef struct {
				std::string data;
				std::string language;
				hb_script_t script;
				hb_direction_t direction;
				const char* c_data() { return data.c_str(); };
			} HBText;

		public:
			DXShaper();
			DXShaper(FT_Face* face);
			virtual ~DXShaper();

			void init();
			hb_script_t gethb_script(int curlan);
			std::string gethb_lan_code(int curlan);

			virtual void doShap(std::vector<Glyph>& glyphList);
		private:
			hb_buffer_t * m_buffer;
			hb_font_t* m_font;
			HBText m_textInfo;

			// Freetypelib和face都需要缓存，需要时获取
			FT_Face* m_ftface;
		};

	};
};
#endif//_DXShaper_H_