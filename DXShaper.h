#pragma once

#include <string>
#include "hb-ft.h"

class LabelComponent;
namespace cocos2d
{
	namespace ui
	{

		class DXShaper
		{
			typedef struct {
				std::string data;
				std::string language;
				hb_script_t script;
				hb_direction_t direction;
				const char* c_data() { return data.c_str(); };
			} HBText;

		public:
			DXShaper();
			virtual ~DXShaper();
			DXShaper(FT_Face* face)
			{
				m_ftface = face;
			}
			virtual ~DXShaper()
			{
				hb_buffer_destroy(m_buffer);
				hb_font_destroy(m_font);
			}

			void init()
			{
				m_font = hb_ft_font_create(*m_ftface, NULL);
				m_buffer = hb_buffer_create();

				hb_buffer_allocation_successful(m_buffer);
			}
			// 	 Ù–‘
			// 		hb_script_t gethb_script(int curlan)
			// 		{
			// 			hb_script_t hb_script = HB_SCRIPT_COMMON;
			// 			switch (curlan)
			// 			{
			// 			case I18N_ZH_CN:
			// 			case I18N_ZH_TW:
			// 				hb_script = HB_SCRIPT_HAN;
			// 				break;
			// 			case I18N_EN_US:
			// 				hb_script = HB_SCRIPT_LATIN;
			// 				break;
			// 			case I18N_JA_JP:
			// 				hb_script = HB_SCRIPT_KATAKANA;
			// 				break;
			// 			case I18N_KO_KR:
			// 				hb_script = HB_SCRIPT_HANGUL;
			// 				break;
			// 			case I18N_TH_TH:
			// 				hb_script = HB_SCRIPT_THAI;
			// 				break;
			// 			case I18N_VI_VN:
			// 				hb_script = HB_SCRIPT_CHAM;
			// 				break;
			// 			case I18N_ID_ID:
			// 				hb_script = HB_SCRIPT_DEVANAGARI;
			// 				break;
			// 			default:
			// 			break;
			// 			}
			// 			return hb_script;
			// 		}
			// 		//  Ù–‘
			// 		std::string gethb_lan_code(int curlan)
			// 		{
			// 			switch (curlan)
			// 			{
			// 			case I18N_ZH_CN:
			// 			case I18N_ZH_TW:
			// 				return "zh";
			// 			case I18N_EN_US:
			// 				return "en";
			// 			case I18N_JA_JP:
			// 				return "ja";
			// 			case I18N_KO_KR:
			// 				return "ko";
			// 			case I18N_TH_TH:
			// 				return "th";
			// 			case I18N_VI_VN:
			// 				return "cjm";
			// 			case I18N_ID_ID:
			// 				return "hi";
			// 			}
			// 			return "";
			// 		}

			virtual void doShap(LabelComponent* lbComp);
		private:
			hb_buffer_t * m_buffer;
			hb_font_t* m_font;
			HBText m_textInfo;

			FT_Face* m_ftface;
			FreeTypeLib* m_ftLib;
		};

	}
}

class DXFontMaterialGen
{

};

