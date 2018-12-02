#include "DXShaper.h"
#include "DXLabel.h"
//#include "freetypelib.h"
#include "DXLabelParser.h"

namespace cocos2d
{
	namespace ui
	{
		enum I18nType
		{
			I18N_ZH_CN = 0,
			I18N_ZH_TW,
			I18N_EN_US,
			I18N_JA_JP,
			I18N_KO_KR,
			I18N_TH_TH,
			I18N_VI_VN,
			I18N_ID_ID,
			I18N_COUNT
		};

		DXShaper::DXShaper()
		{
		}
		DXShaper::DXShaper(FT_Face* face)
		{
			m_ftface = face;
			m_buffer = nullptr;
			m_font = nullptr;
		}

		DXShaper::~DXShaper()
		{
			hb_buffer_destroy(m_buffer);
			hb_font_destroy(m_font);
		}

		void DXShaper::init()
		{
			m_font = hb_ft_font_create(*m_ftface, NULL);
			m_buffer = hb_buffer_create();

			hb_buffer_allocation_successful(m_buffer);
		}

		// Ù–‘
		hb_script_t DXShaper::gethb_script(int curlan)
		{
			hb_script_t hb_script = HB_SCRIPT_COMMON;
			switch (curlan)
			{
			case I18N_ZH_CN:
			case I18N_ZH_TW:
				hb_script = HB_SCRIPT_HAN;
				break;
			case I18N_EN_US:
				hb_script = HB_SCRIPT_LATIN;
				break;
			case I18N_JA_JP:
				hb_script = HB_SCRIPT_KATAKANA;
				break;
			case I18N_KO_KR:
				hb_script = HB_SCRIPT_HANGUL;
				break;
			case I18N_TH_TH:
				hb_script = HB_SCRIPT_THAI;
				break;
			case I18N_VI_VN:
				hb_script = HB_SCRIPT_CHAM;
				break;
			case I18N_ID_ID:
				hb_script = HB_SCRIPT_DEVANAGARI;
				break;
			default:
				break;
			}
			return hb_script;
		}
		//  Ù–‘
		std::string DXShaper::gethb_lan_code(int curlan)
		{
			switch (curlan)
			{
			case I18N_ZH_CN:
			case I18N_ZH_TW:
				return "zh";
			case I18N_EN_US:
				return "en";
			case I18N_JA_JP:
				return "ja";
			case I18N_KO_KR:
				return "ko";
			case I18N_TH_TH:
				return "th";
			case I18N_VI_VN:
				return "cjm";
			case I18N_ID_ID:
				return "hi";
			}
			return "";
		}

		void DXShaper::doShap(std::vector<Glyph>& glyphList )
		{
			if (!m_font || !m_buffer) return;

			m_textInfo.data.clear();

			m_textInfo.direction = HB_DIRECTION_LTR;
			for (auto glyph : glyphList)
			{
				m_textInfo.data.push_back(glyph.charCode);
				m_textInfo.language = gethb_lan_code(glyph.language);
				m_textInfo.script = gethb_script(glyph.language);
			}

			hb_buffer_reset(m_buffer);

			hb_buffer_set_direction(m_buffer, m_textInfo.direction);
			hb_buffer_set_script(m_buffer, m_textInfo.script);
			hb_buffer_set_language(m_buffer, hb_language_from_string(m_textInfo.language.c_str(), m_textInfo.language.size()));
			size_t length = m_textInfo.data.size();

			hb_buffer_add_utf8(m_buffer, m_textInfo.c_data(), length, 0, length);

			// harfbuzz shaping
			hb_shape(m_font, m_buffer, NULL/*features.empty() ? NULL : &features[0]*/, 0/*features.size()*/);

			unsigned int glyphCount;
			hb_glyph_info_t *glyphInfo = hb_buffer_get_glyph_infos(m_buffer, &glyphCount);
			hb_glyph_position_t *glyphPos = hb_buffer_get_glyph_positions(m_buffer, &glyphCount);

			LabelComponent::TriangleInfo triangleInfo;
			float textWidth = 0;
			float textHeight = 0;

			if (glyphList.size() == glyphCount)
			{
				for (unsigned int i = 0; i < glyphCount; ++i) {
					Glyph* glyph = &(glyphList.at(i));
					glyph->glyphIndex = glyphInfo[i].codepoint;
					glyph->x_advance = (float)glyphPos[i].x_advance / 64;
					glyph->y_advance = (float)glyphPos[i].y_advance / 64;
					glyph->x_offset = (float)glyphPos[i].x_offset / 64;
					glyph->y_offset = (float)glyphPos[i].y_offset / 64;
				}
			}
			else
			{
				// some glyph must been connect togather;
			}
			
		}
	}
}