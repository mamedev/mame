// license:BSD-3-Clause
// copyright-holders:Dankan1890
/*********************************************************************

    ui/custui.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/selector.h"
#include "ui/custui.h"
#include "ui/utils.h"
#include <algorithm>

const char *ui_menu_custom_ui::hide_status[] = { "Show All", "Hide Filters", "Hide Info/Image", "Hide Both" };

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_custom_ui::ui_menu_custom_ui(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	// load languages
	file_enumerator path(machine.options().language_path());
	const char *lang = machine.options().language();
	const osd_directory_entry *dirent;
	int cnt = 0;
	for (int x = 0; (dirent = path.next()) != nullptr; ++x)
		if (dirent->type == ENTTYPE_DIR && strcmp(dirent->name, ".") != 0 && strcmp(dirent->name, "..") != 0)
		{
			auto name = std::string(dirent->name);
			int i = strreplace(name, "_", " (");
			if (i > 0) name = name.append(")");
			m_lang.push_back(name);
			if (strcmp(name.c_str(), lang) == 0)
				m_currlang = cnt;
			++cnt;
		}
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_custom_ui::~ui_menu_custom_ui()
{
	std::string error_string;
	machine().ui().options().set_value(OPTION_HIDE_PANELS, ui_globals::panels_status, OPTION_PRIORITY_CMDLINE, error_string);
	if (!m_lang.empty())
	{
		machine().options().set_value(OPTION_LANGUAGE, m_lang[m_currlang].c_str(), OPTION_PRIORITY_CMDLINE, error_string);
		machine().options().mark_changed(OPTION_LANGUAGE);
		load_translation(machine().options());
	}
	ui_globals::reset = true;
}

//-------------------------------------------------	
//  handle
//-------------------------------------------------

void ui_menu_custom_ui::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *m_event = process(0);

	if (m_event != nullptr && m_event->itemref != nullptr)
	{
		switch ((FPTR)m_event->itemref)
		{
			case FONT_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_font_ui>(machine(), container));
				break;
			case COLORS_MENU:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_colors_ui>(machine(), container));
				break;
			case HIDE_MENU:
			{
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					(m_event->iptkey == IPT_UI_RIGHT) ? ui_globals::panels_status++ : ui_globals::panels_status--;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
				{
					int total = ARRAY_LENGTH(hide_status);
					std::vector<std::string> s_sel(total);
					for (int index = 0; index < total; ++index)
						s_sel[index] = hide_status[index];

					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, s_sel, ui_globals::panels_status));
				}
			}
			case LANGUAGE_MENU:
			{
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					(m_event->iptkey == IPT_UI_RIGHT) ? m_currlang++ : m_currlang--;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
				{
					int total = m_lang.size();
					std::vector<std::string> s_sel(total);
					for (int index = 0; index < total; ++index)
						s_sel[index] = m_lang[index];

					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, s_sel, m_currlang));
				}
			}
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_custom_ui::populate()
{
	UINT32 arrow_flags;
	item_append("Fonts", nullptr, 0, (void *)(FPTR)FONT_MENU);
	item_append("Colors", nullptr, 0, (void *)(FPTR)COLORS_MENU);

	if (!m_lang.empty())
	{
		arrow_flags = get_arrow_flags(0, m_lang.size() - 1, m_currlang);
		item_append("Language", m_lang[m_currlang].c_str(), arrow_flags, (void *)(FPTR)LANGUAGE_MENU);
	}

	arrow_flags = get_arrow_flags(0, (int)HIDE_BOTH, ui_globals::panels_status);
	item_append("Show side panels", hide_status[ui_globals::panels_status], arrow_flags, (void *)(FPTR)HIDE_MENU);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_custom_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();

	mui.draw_text_full(container, "Custom UI Settings", 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, "Custom UI Settings", x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_font_ui::ui_menu_font_ui(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	ui_options &moptions = machine.ui().options();
#ifdef UI_WINDOWS

	std::string name(machine.ui().options().ui_font());
	list();

	m_bold = (strreplace(name, "[B]", "") + strreplace(name, "[b]", "") > 0);
	m_italic = (strreplace(name, "[I]", "") + strreplace(name, "[i]", "") > 0);
	m_actual = 0;

	for (size_t index = 0; index < m_fonts.size(); index++)
	{
		if (m_fonts[index] == name)
		{
			m_actual = index;
			break;
		}
	}
#endif

	m_info_size = moptions.infos_size();
	m_font_size = moptions.font_rows();

	for (ui_options::entry *f_entry = moptions.first(); f_entry != nullptr; f_entry = f_entry->next())
	{
		const char *name = f_entry->name();
		if (name && strlen(name) && !strcmp(OPTION_INFOS_SIZE, f_entry->name()))
		{
			m_info_max = atof(f_entry->maximum());
			m_info_min = atof(f_entry->minimum());
		}
		else if (name && strlen(name) && !strcmp(OPTION_FONT_ROWS, f_entry->name()))
		{
			m_font_max = atof(f_entry->maximum());
			m_font_min = atof(f_entry->minimum());
		}
	}

}

#ifdef UI_WINDOWS
//-------------------------------------------------
//  fonts enumerator CALLBACK
//-------------------------------------------------

int CALLBACK ui_menu_font_ui::EnumFontFamiliesExProc(const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam)
{
	std::vector<std::string> *lpc = (std::vector<std::string>*)lParam;
	std::string utf((char *)lpelfe->lfFaceName);
	if (utf[0] != '@')
		lpc->push_back(utf);

	return 1;
}

//-------------------------------------------------
//  create fonts list
//-------------------------------------------------

void ui_menu_font_ui::list()
{
	// create LOGFONT structure
	LOGFONT lf;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfFaceName[0] = '\0';

	HDC hDC = GetDC( nullptr );
	EnumFontFamiliesEx( hDC, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, (LPARAM)&m_fonts, 0 );
	ReleaseDC( nullptr, hDC );

	// sort
	std::stable_sort(m_fonts.begin(), m_fonts.end());

	// add default string to the top of array
	m_fonts.insert(m_fonts.begin(), std::string("default"));
}
#endif

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_font_ui::~ui_menu_font_ui()
{
	std::string error_string;
	ui_options &moptions = machine().ui().options();

#ifdef UI_WINDOWS
	std::string name(m_fonts[m_actual]);
	if (m_fonts[m_actual] != "default")
	{
		if (m_italic)
			name.insert(0, "[I]");
		if (m_bold)
			name.insert(0, "[B]");
	}
	moptions.set_value(OPTION_UI_FONT, name.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
#endif

	moptions.set_value(OPTION_INFOS_SIZE, m_info_size, OPTION_PRIORITY_CMDLINE, error_string);
	moptions.set_value(OPTION_FONT_ROWS, m_font_size, OPTION_PRIORITY_CMDLINE, error_string);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_font_ui::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *m_event = process(UI_MENU_PROCESS_LR_REPEAT);

	if (m_event != nullptr && m_event->itemref != nullptr)
		switch ((FPTR)m_event->itemref)
		{
			case INFOS_SIZE:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					(m_event->iptkey == IPT_UI_RIGHT) ? m_info_size += 0.05f : m_info_size -= 0.05f;
					changed = true;
				}
				break;

			case FONT_SIZE:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					(m_event->iptkey == IPT_UI_RIGHT) ? m_font_size++ : m_font_size--;
					changed = true;
				}
				break;

#ifdef UI_WINDOWS

			case MUI_FNT:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT)
				{
					(m_event->iptkey == IPT_UI_RIGHT) ? m_actual++ : m_actual--;
					changed = true;
				}
				else if (m_event->iptkey == IPT_UI_SELECT)
				{
					ui_menu::stack_push(global_alloc_clear<ui_menu_selector>(machine(), container, m_fonts, m_actual));
					changed = true;
				}
				break;

			case MUI_BOLD:
			case MUI_ITALIC:
				if (m_event->iptkey == IPT_UI_LEFT || m_event->iptkey == IPT_UI_RIGHT || m_event->iptkey == IPT_UI_SELECT)
				{
					((FPTR)m_event->itemref == MUI_BOLD) ? m_bold = !m_bold : m_italic = !m_italic;
					changed = true;
				}
				break;
#endif
		}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_font_ui::populate()
{
	// set filter arrow
	UINT32 arrow_flags;
	std::string tmptxt;

#ifdef UI_WINDOWS
	// add fonts option
	arrow_flags = get_arrow_flags(0, m_fonts.size() - 1, m_actual);
	std::string name(m_fonts[m_actual]);
	item_append("UI Font", name.c_str(), arrow_flags, (void *)(FPTR)MUI_FNT);

	if (name != "default")
	{
		item_append("Bold", m_bold ? "On" : "Off", m_bold ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)(FPTR)MUI_BOLD);
		item_append("Italic", m_italic ? "On" : "Off", m_italic ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)(FPTR)MUI_ITALIC);
	}
#endif

	arrow_flags = get_arrow_flags(m_font_min, m_font_max, m_font_size);
	strprintf(tmptxt, "%2d", m_font_size);
	item_append("Lines", tmptxt.c_str(), arrow_flags, (void *)(FPTR)FONT_SIZE);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	// add item
	tmptxt.clear();
	strprintf(tmptxt, "%3.2f", m_info_size);
	arrow_flags = get_arrow_flags(m_info_min, m_info_max, m_info_size);
	item_append("Infos text size", tmptxt.c_str(), arrow_flags, (void *)(FPTR)INFOS_SIZE);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	custombottom = customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_font_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();

	// top text
	std::string topbuf("UI Fonts Settings");

	mui.draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = MAX(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	if ((FPTR)selectedref == INFOS_SIZE)
	{
		topbuf = "Sample text - Lorem ipsum dolor sit amet, consectetur adipiscing elit.";

		mui.draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_LEFT, WRAP_NEVER,
		                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr, m_info_size);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(origx2 - origx1, width);

		// compute our bounds
		x1 = 0.5f - 0.5f * maxwidth;
		x2 = x1 + maxwidth;
		y1 = origy2 + UI_BOX_TB_BORDER;
		y2 = origy2 + bottom;

		// draw a box
		mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

		// take off the borders
		x1 += UI_BOX_LR_BORDER;
		x2 -= UI_BOX_LR_BORDER;
		y1 += UI_BOX_TB_BORDER;

		// draw the text within it
		mui.draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_LEFT, WRAP_NEVER,
		                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, m_info_size);
	}
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------
#define SET_COLOR_UI(var, opt) var[M##opt].color = opt; var[M##opt].option = OPTION_##opt

ui_menu_colors_ui::ui_menu_colors_ui(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	SET_COLOR_UI(m_color_table, UI_BACKGROUND_COLOR);
	SET_COLOR_UI(m_color_table, UI_BORDER_COLOR);
	SET_COLOR_UI(m_color_table, UI_CLONE_COLOR);
	SET_COLOR_UI(m_color_table, UI_DIPSW_COLOR);
	SET_COLOR_UI(m_color_table, UI_GFXVIEWER_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_MOUSEDOWN_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_MOUSEDOWN_COLOR);
	SET_COLOR_UI(m_color_table, UI_MOUSEOVER_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_MOUSEOVER_COLOR);
	SET_COLOR_UI(m_color_table, UI_SELECTED_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_SELECTED_COLOR);
	SET_COLOR_UI(m_color_table, UI_SLIDER_COLOR);
	SET_COLOR_UI(m_color_table, UI_SUBITEM_COLOR);
	SET_COLOR_UI(m_color_table, UI_TEXT_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_TEXT_COLOR);
	SET_COLOR_UI(m_color_table, UI_UNAVAILABLE_COLOR);
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_colors_ui::~ui_menu_colors_ui()
{
	std::string error_string, dec_color;
	for (int index = 1; index < MUI_RESTORE; index++)
	{
		strprintf(dec_color, "%x", (UINT32)m_color_table[index].color);
		machine().ui().options().set_value(m_color_table[index].option, dec_color.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
	}
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_colors_ui::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *m_event = process(0);

	if (m_event != nullptr && m_event->itemref != nullptr && m_event->iptkey == IPT_UI_SELECT)
	{
		if ((FPTR)m_event->itemref != MUI_RESTORE)
			ui_menu::stack_push(global_alloc_clear<ui_menu_rgb_ui>(machine(), container, &m_color_table[(FPTR)m_event->itemref].color, item[selected].text));
		else
		{
			changed = true;
			restore_colors();
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_colors_ui::populate()
{
	item_append("Normal text", nullptr, 0, (void *)(FPTR)MUI_TEXT_COLOR);
	item_append("Selected color", nullptr, 0, (void *)(FPTR)MUI_SELECTED_COLOR);
	item_append("Normal text background", nullptr, 0, (void *)(FPTR)MUI_TEXT_BG_COLOR);
	item_append("Selected background color", nullptr, 0, (void *)(FPTR)MUI_SELECTED_BG_COLOR);
	item_append("Subitem color", nullptr, 0, (void *)(FPTR)MUI_SUBITEM_COLOR);
	item_append("Clone", nullptr, 0, (void *)(FPTR)MUI_CLONE_COLOR);
	item_append("Border", nullptr, 0, (void *)(FPTR)MUI_BORDER_COLOR);
	item_append("Background", nullptr, 0, (void *)(FPTR)MUI_BACKGROUND_COLOR);
	item_append("Dipswitch", nullptr, 0, (void *)(FPTR)MUI_DIPSW_COLOR);
	item_append("Unavailable color", nullptr, 0, (void *)(FPTR)MUI_UNAVAILABLE_COLOR);
	item_append("Slider color", nullptr, 0, (void *)(FPTR)MUI_SLIDER_COLOR);
	item_append("Gfx viewer background", nullptr, 0, (void *)(FPTR)MUI_GFXVIEWER_BG_COLOR);
	item_append("Mouse over color", nullptr, 0, (void *)(FPTR)MUI_MOUSEOVER_COLOR);
	item_append("Mouse over background color", nullptr, 0, (void *)(FPTR)MUI_MOUSEOVER_BG_COLOR);
	item_append("Mouse down color", nullptr, 0, (void *)(FPTR)MUI_MOUSEDOWN_COLOR);
	item_append("Mouse down background color", nullptr, 0, (void *)(FPTR)MUI_MOUSEDOWN_BG_COLOR);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	item_append("Restore originals colors", nullptr, 0, (void *)(FPTR)MUI_RESTORE);

	custombottom = customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_colors_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;
	ui_manager &mui = machine().ui();
	float line_height = mui.get_line_height();

	// top text
	std::string topbuf("UI Colors Settings");

	mui.draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// bottom text
	// get the text for 'UI Select'
	std::string ui_select_text = machine().input().seq_name(machine().ioport().type_seq(IPT_UI_SELECT, 0, SEQ_TYPE_STANDARD));
	topbuf.assign("Double click or press ").append(ui_select_text.c_str()).append(" to change the color value");

	mui.draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_RED_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// compute maxwidth
	topbuf = "Menu Preview";

	mui.draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	maxwidth = width + 2.0f * UI_BOX_LR_BORDER;

	std::string sampletxt[5];

	sampletxt[0] = "Normal";
	sampletxt[1] = "Subitem";
	sampletxt[2] = "Selected";
	sampletxt[3] = "Mouse Over";
	sampletxt[4] = "Clone";

	for (auto & elem: sampletxt)
	{
		mui.draw_text_full(container, elem.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
		                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(maxwidth, width);
	}

	// compute our bounds for header
	x1 = origx2 + 2.0f * UI_BOX_LR_BORDER;
	x2 = x1 + maxwidth;
	y1 = origy1;
	y2 = y1 + bottom - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// compute our bounds for menu preview
	x1 -= UI_BOX_LR_BORDER;
	x2 += UI_BOX_LR_BORDER;
	y1 = y2 + 2.0f * UI_BOX_TB_BORDER;
	y2 = y1 + 5.0f * line_height + 2.0f * UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, m_color_table[MUI_BACKGROUND_COLOR].color);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw normal text
	mui.draw_text_full(container, sampletxt[0].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, m_color_table[MUI_TEXT_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw subitem text
	mui.draw_text_full(container, sampletxt[1].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, m_color_table[MUI_SUBITEM_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw selected text
	highlight(container, x1, y1, x2, y1 + line_height, m_color_table[MUI_SELECTED_BG_COLOR].color);
	mui.draw_text_full(container, sampletxt[2].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, m_color_table[MUI_SELECTED_COLOR].color, m_color_table[MUI_SELECTED_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw mouse over text
	highlight(container, x1, y1, x2, y1 + line_height, m_color_table[MUI_MOUSEOVER_BG_COLOR].color);
	mui.draw_text_full(container, sampletxt[3].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, m_color_table[MUI_MOUSEOVER_COLOR].color, m_color_table[MUI_MOUSEOVER_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw clone text
	mui.draw_text_full(container, sampletxt[4].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, m_color_table[MUI_CLONE_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, nullptr, nullptr);

}

//-------------------------------------------------
//  restore original colors
//-------------------------------------------------

void ui_menu_colors_ui::restore_colors()
{
	ui_options options;
	for (int index = 1; index < MUI_RESTORE; index++)
		m_color_table[index].color = rgb_t((UINT32)strtoul(options.value(m_color_table[index].option), nullptr, 16));
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_rgb_ui::ui_menu_rgb_ui(running_machine &machine, render_container *container, rgb_t *_color, std::string _title) : ui_menu(machine, container)
{
	m_color = _color;
	m_key_active = false;
	m_lock_ref = 0;
	m_title = _title;
	m_search[0] = '\0';
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_rgb_ui::~ui_menu_rgb_ui()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_rgb_ui::handle()
{
	bool changed = false;

	// process the menu
	const ui_menu_event *m_event;

	if (!m_key_active)
		m_event = process(UI_MENU_PROCESS_LR_REPEAT);
	else
		m_event = process(UI_MENU_PROCESS_ONLYCHAR);

	if (m_event != nullptr && m_event->itemref != nullptr)
	{
		switch ((FPTR)m_event->itemref)
		{
			case RGB_ALPHA:
				if (m_event->iptkey == IPT_UI_LEFT && m_color->a() > 1)
				{
					m_color->set_a(m_color->a() - 1);
					changed = true;
				}

				else if (m_event->iptkey == IPT_UI_RIGHT && m_color->a() < 255)
				{
					m_color->set_a(m_color->a() + 1);
					changed = true;
				}

				else if (m_event->iptkey == IPT_UI_SELECT || m_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(m_event);
					changed = true;
				}

				break;

			case RGB_RED:
				if (m_event->iptkey == IPT_UI_LEFT && m_color->r() > 1)
				{
					m_color->set_r(m_color->r() - 1);
					changed = true;
				}

				else if (m_event->iptkey == IPT_UI_RIGHT && m_color->r() < 255)
				{
					m_color->set_r(m_color->r() + 1);
					changed = true;
				}

				else if (m_event->iptkey == IPT_UI_SELECT || m_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(m_event);
					changed = true;
				}

				break;

			case RGB_GREEN:
				if (m_event->iptkey == IPT_UI_LEFT && m_color->g() > 1)
				{
					m_color->set_g(m_color->g() - 1);
					changed = true;
				}

				else if (m_event->iptkey == IPT_UI_RIGHT && m_color->g() < 255)
				{
					m_color->set_g(m_color->g() + 1);
					changed = true;
				}

				else if (m_event->iptkey == IPT_UI_SELECT || m_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(m_event);
					changed = true;
				}

				break;

			case RGB_BLUE:
				if (m_event->iptkey == IPT_UI_LEFT && m_color->b() > 1)
				{
					m_color->set_b(m_color->b() - 1);
					changed = true;
				}

				else if (m_event->iptkey == IPT_UI_RIGHT && m_color->b() < 255)
				{
					m_color->set_b(m_color->b() + 1);
					changed = true;
				}

				else if (m_event->iptkey == IPT_UI_SELECT || m_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(m_event);
					changed = true;
				}

				break;

			case PALETTE_CHOOSE:
				if (m_event->iptkey == IPT_UI_SELECT)
					ui_menu::stack_push(global_alloc_clear<ui_menu_palette_sel>(machine(), container, *m_color));
				break;
		}
	}

	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_rgb_ui::populate()
{
	// set filter arrow
	UINT32 arrow_flags = MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
	std::string text;
	std::string s_text = std::string(m_search).append("_");

	if (m_lock_ref != RGB_ALPHA)
	{
		arrow_flags = get_arrow_flags(0, 255, m_color->a());
		strprintf(text, "%3d", m_color->a());
		item_append("Alpha", text.c_str(), arrow_flags, (void *)(FPTR)RGB_ALPHA);
	}
	else
		item_append("Alpha", s_text.c_str(), 0, (void *)(FPTR)RGB_ALPHA);

	if (m_lock_ref != RGB_RED)
	{
		arrow_flags = get_arrow_flags(0, 255, m_color->r());
		strprintf(text, "%3d", m_color->r());
		item_append("Red", text.c_str(), arrow_flags, (void *)(FPTR)RGB_RED);
	}
	else
		item_append("Red", s_text.c_str(), 0, (void *)(FPTR)RGB_RED);

	if (m_lock_ref != RGB_GREEN)
	{
		arrow_flags = get_arrow_flags(0, 255, m_color->g());
		strprintf(text, "%3d", m_color->g());
		item_append("Green", text.c_str(), arrow_flags, (void *)(FPTR)RGB_GREEN);
	}
	else
		item_append("Green", s_text.c_str(), 0, (void *)(FPTR)RGB_GREEN);

	if (m_lock_ref != RGB_BLUE)
	{
		arrow_flags = get_arrow_flags(0, 255, m_color->b());
		strprintf(text, "%3d", m_color->b());
		item_append("Blue", text.c_str(), arrow_flags, (void *)(FPTR)RGB_BLUE);
	}
	else
		item_append("Blue", s_text.c_str(), 0, (void *)(FPTR)RGB_BLUE);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	item_append("Choose from palette", nullptr, 0, (void *)(FPTR)PALETTE_CHOOSE);
	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	custombottom = customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_rgb_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;
	ui_manager &mui = machine().ui();

	// top text
	std::string topbuf = std::string(m_title).append(" - ARGB Settings");
	mui.draw_text_full(container, topbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, topbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	std::string sampletxt("Color preview =");
	maxwidth = origx2 - origx1;
	mui.draw_text_full(container, sampletxt.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	x1 -= UI_BOX_LR_BORDER;
	x2 = x1 + width;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_RED_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the normal text
	mui.draw_text_full(container, sampletxt.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, ARGB_WHITE, ARGB_BLACK, nullptr, nullptr);

	float t_x2 = x1 - UI_BOX_LR_BORDER + maxwidth;
	x1 = x2 + 2.0f * UI_BOX_LR_BORDER;
	x2 = t_x2;
	y1 -= UI_BOX_TB_BORDER;

	mui.draw_outlined_box(container, x1, y1, x2, y2, *m_color);

}

//-------------------------------------------------
//  handle special key event
//-------------------------------------------------

void ui_menu_rgb_ui::inkey_special(const ui_menu_event *m_event)
{
	if (m_event->iptkey == IPT_UI_SELECT)
	{
		m_key_active = !m_key_active;
		m_lock_ref = (FPTR)m_event->itemref;

		if (!m_key_active)
		{
			int val = atoi(m_search);
			val = m_color->clamp(val);

			switch ((FPTR)m_event->itemref)
			{
				case RGB_ALPHA:
					m_color->set_a(val);
					break;

				case RGB_RED:
					m_color->set_r(val);
					break;

				case RGB_GREEN:
					m_color->set_g(val);
					break;

				case RGB_BLUE:
					m_color->set_b(val);
					break;
			}

			m_search[0] = 0;
			m_lock_ref = 0;
			return;
		}
	}

	if (!m_key_active)
	{
		m_search[0] = 0;
		return;
	}

	int buflen = strlen(m_search);

	// if it's a backspace and we can handle it, do so
	if (((m_event->unichar == 8 || m_event->unichar == 0x7f) && buflen > 0))
		*(char *)utf8_previous_char(&m_search[buflen]) = 0;
	else if (buflen >= 3)
		return;
	// if it's any other key and we're not maxed out, update
	else if ((m_event->unichar >= '0' && m_event->unichar <= '9'))
		buflen += utf8_from_uchar(&m_search[buflen], ARRAY_LENGTH(m_search) - buflen, m_event->unichar);

	m_search[buflen] = 0;
}

ui_menu_palette_sel::palcolor ui_menu_palette_sel::m_palette[] = {
	{ "White",  "FFFFFFFF" },
	{ "Silver", "FFC0C0C0" },
	{ "Gray",   "FF808080" },
	{ "Black",  "FF000000" },
	{ "Red",    "FFFF0000" },
	{ "Orange", "FFFFA500" },
	{ "Yellow", "FFFFFF00" },
	{ "Green",  "FF00FF00" },
	{ "Blue",   "FF0000FF" },
	{ "Violet", "FF8F00FF" }
};

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_palette_sel::ui_menu_palette_sel(running_machine &machine, render_container *container, rgb_t &_color) 
	: ui_menu(machine, container), m_original(_color)
{
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_palette_sel::~ui_menu_palette_sel()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_palette_sel::handle()
{
	// process the menu
	const ui_menu_event *m_event = process(MENU_FLAG_UI_PALETTE);
	if (m_event != nullptr && m_event->itemref != nullptr)
	{
		if (m_event->iptkey == IPT_UI_SELECT)
		{
			m_original = rgb_t((UINT32)strtoul(item[selected].subtext, nullptr, 16));
			ui_menu::menu_stack->parent->reset(UI_MENU_RESET_SELECT_FIRST);
			ui_menu::stack_pop(machine());
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_palette_sel::populate()
{
	for (int x = 0; x < ARRAY_LENGTH(m_palette); ++x)
		item_append(m_palette[x].name, m_palette[x].argb, MENU_FLAG_UI_PALETTE, (void *)(FPTR)(x + 1));

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_palette_sel::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
}
