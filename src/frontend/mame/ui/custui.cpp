// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/custui.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"

#include "ui/custui.h"

#include "ui/ui.h"
#include "ui/selector.h"
#include "ui/utils.h"

#include "drivenum.h"
#include "emuopts.h"
#include "osdepend.h"
#include "uiinput.h"

#include <algorithm>
#include <utility>


namespace ui {
const char *const menu_custom_ui::HIDE_STATUS[] = {
	__("Show All"),
	__("Hide Filters"),
	__("Hide Info/Image"),
	__("Hide Both") };

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_custom_ui::menu_custom_ui(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
	// load languages
	file_enumerator path(mui.machine().options().language_path());
	auto lang = mui.machine().options().language();
	const osd::directory::entry *dirent;
	std::size_t cnt = 0;
	while ((dirent = path.next()) != nullptr)
	{
		if (dirent->type == osd::directory::entry::entry_type::DIR && strcmp(dirent->name, ".") != 0 && strcmp(dirent->name, "..") != 0)
		{
			auto name = std::string(dirent->name);
			auto i = strreplace(name, "_", " (");
			if (i > 0) name = name.append(")");
			m_lang.push_back(name);
			if (strcmp(name.c_str(), lang) == 0)
				m_currlang = cnt;
			++cnt;
		}
	}
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_custom_ui::~menu_custom_ui()
{
	std::string error_string;
	ui().options().set_value(OPTION_HIDE_PANELS, ui_globals::panels_status, OPTION_PRIORITY_CMDLINE, error_string);
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

void menu_custom_ui::handle()
{
	bool changed = false;

	// process the menu
	const event *menu_event = process(0);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		switch ((FPTR)menu_event->itemref)
		{
			case FONT_MENU:
				if (menu_event->iptkey == IPT_UI_SELECT)
					menu::stack_push<menu_font_ui>(ui(), container());
				break;
			case COLORS_MENU:
				if (menu_event->iptkey == IPT_UI_SELECT)
					menu::stack_push<menu_colors_ui>(ui(), container());
				break;
			case HIDE_MENU:
			{
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					(menu_event->iptkey == IPT_UI_RIGHT) ? ui_globals::panels_status++ : ui_globals::panels_status--;
				}
				else if (menu_event->iptkey == IPT_UI_SELECT)
				{
					std::vector<std::string> s_sel(ARRAY_LENGTH(HIDE_STATUS));
					std::transform(std::begin(HIDE_STATUS), std::end(HIDE_STATUS), s_sel.begin(), [](auto &s) { return _(s); });
					menu::stack_push<menu_selector>(ui(), container(), std::move(s_sel), ui_globals::panels_status);
				}
				break;
			}
			case LANGUAGE_MENU:
			{
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					changed = true;
					(menu_event->iptkey == IPT_UI_RIGHT) ? m_currlang++ : m_currlang--;
				}
				else if (menu_event->iptkey == IPT_UI_SELECT)
				{
					menu::stack_push<menu_selector>(ui(), container(), m_lang, m_currlang);
				}
				break;
			}
		}
	}

	if (changed)
		reset(reset_options::REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_custom_ui::populate()
{
	UINT32 arrow_flags;
	item_append(_("Fonts"), "", 0, (void *)(FPTR)FONT_MENU);
	item_append(_("Colors"), "", 0, (void *)(FPTR)COLORS_MENU);

	if (!m_lang.empty())
	{
		arrow_flags = get_arrow_flags<std::uint16_t>(0, m_lang.size() - 1, m_currlang);
		item_append(_("Language"), m_lang[m_currlang].c_str(), arrow_flags, (void *)(FPTR)LANGUAGE_MENU);
	}

	arrow_flags = get_arrow_flags<UINT16>(0, HIDE_BOTH, ui_globals::panels_status);
	item_append(_("Show side panels"), _(HIDE_STATUS[ui_globals::panels_status]), arrow_flags, (void *)(FPTR)HIDE_MENU);

	item_append(menu_item_type::SEPARATOR);
	customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_custom_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;

	ui().draw_text_full(container(), _("Custom UI Settings"), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
									mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = std::max(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container(), _("Custom UI Settings"), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
									mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_font_ui::menu_font_ui(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
	ui_options &moptions = mui.options();
	std::string name(mui.machine().options().ui_font());
	list();

#ifdef UI_WINDOWS
	m_bold = (strreplace(name, "[B]", "") + strreplace(name, "[b]", "") > 0);
	m_italic = (strreplace(name, "[I]", "") + strreplace(name, "[i]", "") > 0);
#endif
	m_actual = 0;

	for (std::size_t index = 0; index < m_fonts.size(); index++)
	{
		if (m_fonts[index].first == name)
		{
			m_actual = index;
			break;
		}
	}

	m_info_size = moptions.infos_size();
	m_font_size = moptions.font_rows();

	for (ui_options::entry &f_entry : moptions)
	{
		const char *entry_name = f_entry.name();
		if (entry_name && strlen(entry_name) && !strcmp(OPTION_INFOS_SIZE, f_entry.name()))
		{
			m_info_max = atof(f_entry.maximum());
			m_info_min = atof(f_entry.minimum());
		}
		else if (entry_name && strlen(entry_name) && !strcmp(OPTION_FONT_ROWS, f_entry.name()))
		{
			m_font_max = atof(f_entry.maximum());
			m_font_min = atof(f_entry.minimum());
		}
	}

}

//-------------------------------------------------
//  create fonts list
//-------------------------------------------------

void menu_font_ui::list()
{
	machine().osd().get_font_families(machine().options().font_path(), m_fonts);

	// add default string to the top of array
	m_fonts.emplace(m_fonts.begin(), std::string("default"), std::string(_("default")));
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_font_ui::~menu_font_ui()
{
	std::string error_string;
	ui_options &moptions = ui().options();

	std::string name(m_fonts[m_actual].first);
#ifdef UI_WINDOWS
	if (name != "default")
	{
		if (m_italic)
			name.insert(0, "[I]");
		if (m_bold)
			name.insert(0, "[B]");
	}
#endif
	machine().options().set_value(OPTION_UI_FONT, name.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
	machine().options().mark_changed(OPTION_UI_FONT);

	moptions.set_value(OPTION_INFOS_SIZE, m_info_size, OPTION_PRIORITY_CMDLINE, error_string);
	moptions.set_value(OPTION_FONT_ROWS, m_font_size, OPTION_PRIORITY_CMDLINE, error_string);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_font_ui::handle()
{
	bool changed = false;

	// process the menu
	const event *menu_event = process(PROCESS_LR_REPEAT);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
		switch ((FPTR)menu_event->itemref)
		{
			case INFOS_SIZE:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? m_info_size += 0.05f : m_info_size -= 0.05f;
					changed = true;
				}
				break;

			case FONT_SIZE:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? m_font_size++ : m_font_size--;
					changed = true;
				}
				break;


			case MUI_FNT:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					(menu_event->iptkey == IPT_UI_RIGHT) ? m_actual++ : m_actual--;
					changed = true;
				}
				else if (menu_event->iptkey == IPT_UI_SELECT)
				{
					std::vector<std::string> display_names;
					display_names.reserve(m_fonts.size());
					for (auto const &font : m_fonts) display_names.emplace_back(font.second);
					menu::stack_push<menu_selector>(ui(), container(), std::move(display_names), m_actual);
					changed = true;
				}
				break;

#ifdef UI_WINDOWS
			case MUI_BOLD:
			case MUI_ITALIC:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT || menu_event->iptkey == IPT_UI_SELECT)
				{
					((FPTR)menu_event->itemref == MUI_BOLD) ? m_bold = !m_bold : m_italic = !m_italic;
					changed = true;
				}
				break;
#endif
		}

	if (changed)
		reset(reset_options::REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_font_ui::populate()
{
	// set filter arrow
	UINT32 arrow_flags;

	// add fonts option
	arrow_flags = get_arrow_flags<std::uint16_t>(0, m_fonts.size() - 1, m_actual);
	item_append(_("UI Font"), m_fonts[m_actual].second, arrow_flags, (void *)(FPTR)MUI_FNT);

#ifdef UI_WINDOWS
	if (m_fonts[m_actual].first != "default")
	{
		item_append(_("Bold"), m_bold ? "On" : "Off", m_bold ? FLAG_RIGHT_ARROW : FLAG_LEFT_ARROW, (void *)(FPTR)MUI_BOLD);
		item_append(_("Italic"), m_italic ? "On" : "Off", m_italic ? FLAG_RIGHT_ARROW : FLAG_LEFT_ARROW, (void *)(FPTR)MUI_ITALIC);
	}
#endif

	arrow_flags = get_arrow_flags(m_font_min, m_font_max, m_font_size);
	item_append(_("Lines"), string_format("%2d", m_font_size), arrow_flags, (void *)(FPTR)FONT_SIZE);

	item_append(menu_item_type::SEPARATOR);

	// add item
	arrow_flags = get_arrow_flags(m_info_min, m_info_max, m_info_size);
	item_append(_("Infos text size"), string_format("%3.2f", m_info_size), arrow_flags, (void *)(FPTR)INFOS_SIZE);

	item_append(menu_item_type::SEPARATOR);

	custombottom = customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_font_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;

	// top text
	std::string topbuf(_("UI Fonts Settings"));

	ui().draw_text_full(container(), topbuf.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
									mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	float maxwidth = std::max(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container(), topbuf.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
									mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	if ((FPTR)selectedref == INFOS_SIZE)
	{
		topbuf = _("Sample text - Lorem ipsum dolor sit amet, consectetur adipiscing elit.");

		ui().draw_text_full(container(), topbuf.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::LEFT, ui::text_layout::NEVER,
										mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr, m_info_size);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = std::max(origx2 - origx1, width);

		// compute our bounds
		x1 = 0.5f - 0.5f * maxwidth;
		x2 = x1 + maxwidth;
		y1 = origy2 + UI_BOX_TB_BORDER;
		y2 = origy2 + bottom;

		// draw a box
		ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

		// take off the borders
		x1 += UI_BOX_LR_BORDER;
		x2 -= UI_BOX_LR_BORDER;
		y1 += UI_BOX_TB_BORDER;

		// draw the text within it
		ui().draw_text_full(container(), topbuf.c_str(), x1, y1, x2 - x1, ui::text_layout::LEFT, ui::text_layout::NEVER,
										mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr, m_info_size);
	}
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------
#define SET_COLOR_UI(var, opt) var[M##opt].color = opt; var[M##opt].option = OPTION_##opt

menu_colors_ui::menu_colors_ui(mame_ui_manager &mui, render_container &container) : menu(mui, container)
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

menu_colors_ui::~menu_colors_ui()
{
	std::string error_string, dec_color;
	for (int index = 1; index < MUI_RESTORE; index++)
	{
		dec_color = string_format("%x", (UINT32)m_color_table[index].color);
		ui().options().set_value(m_color_table[index].option, dec_color.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
	}
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_colors_ui::handle()
{
	bool changed = false;

	// process the menu
	const event *menu_event = process(0);

	if (menu_event != nullptr && menu_event->itemref != nullptr && menu_event->iptkey == IPT_UI_SELECT)
	{
		if ((FPTR)menu_event->itemref != MUI_RESTORE)
			menu::stack_push<menu_rgb_ui>(ui(), container(), &m_color_table[(FPTR)menu_event->itemref].color, item[selected].text);
		else
		{
			changed = true;
			restore_colors();
		}
	}

	if (changed)
		reset(reset_options::REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_colors_ui::populate()
{
	item_append(_("Normal text"), "", 0, (void *)(FPTR)MUI_TEXT_COLOR);
	item_append(_("Selected color"), "", 0, (void *)(FPTR)MUI_SELECTED_COLOR);
	item_append(_("Normal text background"), "", 0, (void *)(FPTR)MUI_TEXT_BG_COLOR);
	item_append(_("Selected background color"), "", 0, (void *)(FPTR)MUI_SELECTED_BG_COLOR);
	item_append(_("Subitem color"), "", 0, (void *)(FPTR)MUI_SUBITEM_COLOR);
	item_append(_("Clone"), "", 0, (void *)(FPTR)MUI_CLONE_COLOR);
	item_append(_("Border"), "", 0, (void *)(FPTR)MUI_BORDER_COLOR);
	item_append(_("Background"), "", 0, (void *)(FPTR)MUI_BACKGROUND_COLOR);
	item_append(_("Dipswitch"), "", 0, (void *)(FPTR)MUI_DIPSW_COLOR);
	item_append(_("Unavailable color"), "", 0, (void *)(FPTR)MUI_UNAVAILABLE_COLOR);
	item_append(_("Slider color"), "", 0, (void *)(FPTR)MUI_SLIDER_COLOR);
	item_append(_("Gfx viewer background"), "", 0, (void *)(FPTR)MUI_GFXVIEWER_BG_COLOR);
	item_append(_("Mouse over color"), "", 0, (void *)(FPTR)MUI_MOUSEOVER_COLOR);
	item_append(_("Mouse over background color"), "", 0, (void *)(FPTR)MUI_MOUSEOVER_BG_COLOR);
	item_append(_("Mouse down color"), "", 0, (void *)(FPTR)MUI_MOUSEDOWN_COLOR);
	item_append(_("Mouse down background color"), "", 0, (void *)(FPTR)MUI_MOUSEDOWN_BG_COLOR);

	item_append(menu_item_type::SEPARATOR);
	item_append(_("Restore originals colors"), "", 0, (void *)(FPTR)MUI_RESTORE);

	custombottom = customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_colors_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;
	float line_height = ui().get_line_height();

	// top text
	std::string topbuf(_("UI Colors Settings"));

	ui().draw_text_full(container(), topbuf.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = std::max(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container(), topbuf.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// bottom text
	// get the text for 'UI Select'
	std::string ui_select_text = machine().input().seq_name(machine().ioport().type_seq(IPT_UI_SELECT, 0, SEQ_TYPE_STANDARD));
	topbuf = string_format(_("Double click or press %1$s to change the color value"), ui_select_text);

	ui().draw_text_full(container(), topbuf.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = std::max(maxwidth, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_RED_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container(), topbuf.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// compute maxwidth
	topbuf = _("Menu Preview");

	ui().draw_text_full(container(), topbuf.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	maxwidth = width + 2.0f * UI_BOX_LR_BORDER;

	std::string sampletxt[5];

	sampletxt[0] = _("Normal");
	sampletxt[1] = _("Subitem");
	sampletxt[2] = _("Selected");
	sampletxt[3] = _("Mouse Over");
	sampletxt[4] = _("Clone");

	for (auto & elem: sampletxt)
	{
		ui().draw_text_full(container(), elem.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::NEVER,
										mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = std::max(maxwidth, width);
	}

	// compute our bounds for header
	x1 = origx2 + 2.0f * UI_BOX_LR_BORDER;
	x2 = x1 + maxwidth;
	y1 = origy1;
	y2 = y1 + bottom - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container(), topbuf.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// compute our bounds for menu preview
	x1 -= UI_BOX_LR_BORDER;
	x2 += UI_BOX_LR_BORDER;
	y1 = y2 + 2.0f * UI_BOX_TB_BORDER;
	y2 = y1 + 5.0f * line_height + 2.0f * UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, m_color_table[MUI_BACKGROUND_COLOR].color);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw normal text
	ui().draw_text_full(container(), sampletxt[0].c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NORMAL, m_color_table[MUI_TEXT_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw subitem text
	ui().draw_text_full(container(), sampletxt[1].c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NORMAL, m_color_table[MUI_SUBITEM_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw selected text
	highlight(x1, y1, x2, y1 + line_height, m_color_table[MUI_SELECTED_BG_COLOR].color);
	ui().draw_text_full(container(), sampletxt[2].c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NORMAL, m_color_table[MUI_SELECTED_COLOR].color, m_color_table[MUI_SELECTED_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw mouse over text
	highlight(x1, y1, x2, y1 + line_height, m_color_table[MUI_MOUSEOVER_BG_COLOR].color);
	ui().draw_text_full(container(), sampletxt[3].c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NORMAL, m_color_table[MUI_MOUSEOVER_COLOR].color, m_color_table[MUI_MOUSEOVER_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw clone text
	ui().draw_text_full(container(), sampletxt[4].c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NORMAL, m_color_table[MUI_CLONE_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, nullptr, nullptr);

}

//-------------------------------------------------
//  restore original colors
//-------------------------------------------------

void menu_colors_ui::restore_colors()
{
	ui_options options;
	for (int index = 1; index < MUI_RESTORE; index++)
		m_color_table[index].color = rgb_t((UINT32)strtoul(options.value(m_color_table[index].option), nullptr, 16));
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_rgb_ui::menu_rgb_ui(mame_ui_manager &mui, render_container &container, rgb_t *_color, std::string _title) : menu(mui, container)
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

menu_rgb_ui::~menu_rgb_ui()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_rgb_ui::handle()
{
	bool changed = false;

	// process the menu
	const event *menu_event;

	if (!m_key_active)
		menu_event = process(PROCESS_LR_REPEAT);
	else
		menu_event = process(PROCESS_ONLYCHAR);

	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		switch ((FPTR)menu_event->itemref)
		{
			case RGB_ALPHA:
				if (menu_event->iptkey == IPT_UI_LEFT && m_color->a() > 1)
				{
					m_color->set_a(m_color->a() - 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_RIGHT && m_color->a() < 255)
				{
					m_color->set_a(m_color->a() + 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_SELECT || menu_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(menu_event);
					changed = true;
				}

				break;

			case RGB_RED:
				if (menu_event->iptkey == IPT_UI_LEFT && m_color->r() > 1)
				{
					m_color->set_r(m_color->r() - 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_RIGHT && m_color->r() < 255)
				{
					m_color->set_r(m_color->r() + 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_SELECT || menu_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(menu_event);
					changed = true;
				}

				break;

			case RGB_GREEN:
				if (menu_event->iptkey == IPT_UI_LEFT && m_color->g() > 1)
				{
					m_color->set_g(m_color->g() - 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_RIGHT && m_color->g() < 255)
				{
					m_color->set_g(m_color->g() + 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_SELECT || menu_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(menu_event);
					changed = true;
				}

				break;

			case RGB_BLUE:
				if (menu_event->iptkey == IPT_UI_LEFT && m_color->b() > 1)
				{
					m_color->set_b(m_color->b() - 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_RIGHT && m_color->b() < 255)
				{
					m_color->set_b(m_color->b() + 1);
					changed = true;
				}

				else if (menu_event->iptkey == IPT_UI_SELECT || menu_event->iptkey == IPT_SPECIAL)
				{
					inkey_special(menu_event);
					changed = true;
				}

				break;

			case PALETTE_CHOOSE:
				if (menu_event->iptkey == IPT_UI_SELECT)
					menu::stack_push<menu_palette_sel>(ui(), container(), *m_color);
				break;
		}
	}

	if (changed)
		reset(reset_options::REMEMBER_REF);
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_rgb_ui::populate()
{
	// set filter arrow
	UINT32 arrow_flags = FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW;
	std::string s_text = std::string(m_search).append("_");

	if (m_lock_ref != RGB_ALPHA)
	{
		arrow_flags = get_arrow_flags<UINT8>(0, 255, m_color->a());
		item_append(_("Alpha"), string_format("%3u", m_color->a()), arrow_flags, (void *)(FPTR)RGB_ALPHA);
	}
	else
		item_append(_("Alpha"), s_text, 0, (void *)(FPTR)RGB_ALPHA);

	if (m_lock_ref != RGB_RED)
	{
		arrow_flags = get_arrow_flags<UINT8>(0, 255, m_color->r());
		item_append(_("Red"), string_format("%3u", m_color->r()), arrow_flags, (void *)(FPTR)RGB_RED);
	}
	else
		item_append(_("Red"), s_text, 0, (void *)(FPTR)RGB_RED);

	if (m_lock_ref != RGB_GREEN)
	{
		arrow_flags = get_arrow_flags<UINT8>(0, 255, m_color->g());
		item_append(_("Green"), string_format("%3u", m_color->g()), arrow_flags, (void *)(FPTR)RGB_GREEN);
	}
	else
		item_append(_("Green"), s_text, 0, (void *)(FPTR)RGB_GREEN);

	if (m_lock_ref != RGB_BLUE)
	{
		arrow_flags = get_arrow_flags<UINT8>(0, 255, m_color->b());
		item_append(_("Blue"), string_format("%3u", m_color->b()), arrow_flags, (void *)(FPTR)RGB_BLUE);
	}
	else
		item_append(_("Blue"), s_text, 0, (void *)(FPTR)RGB_BLUE);

	item_append(menu_item_type::SEPARATOR);
	item_append(_("Choose from palette"), "", 0, (void *)(FPTR)PALETTE_CHOOSE);
	item_append(menu_item_type::SEPARATOR);

	custombottom = customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_rgb_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;

	// top text
	std::string topbuf = std::string(m_title).append(_(" - ARGB Settings"));
	ui().draw_text_full(container(), topbuf.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = std::max(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	ui().draw_text_full(container(), topbuf.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	std::string sampletxt(_("Color preview ="));
	maxwidth = origx2 - origx1;
	ui().draw_text_full(container(), sampletxt.c_str(), 0.0f, 0.0f, 1.0f, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &width, nullptr);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = std::max(maxwidth, width);

	// compute our bounds
	x1 -= UI_BOX_LR_BORDER;
	x2 = x1 + width;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_RED_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the normal text
	ui().draw_text_full(container(), sampletxt.c_str(), x1, y1, x2 - x1, ui::text_layout::CENTER, ui::text_layout::NEVER,
									mame_ui_manager::NORMAL, rgb_t::white, rgb_t::black, nullptr, nullptr);

	float t_x2 = x1 - UI_BOX_LR_BORDER + maxwidth;
	x1 = x2 + 2.0f * UI_BOX_LR_BORDER;
	x2 = t_x2;
	y1 -= UI_BOX_TB_BORDER;

	ui().draw_outlined_box(container(), x1, y1, x2, y2, *m_color);

}

//-------------------------------------------------
//  handle special key event
//-------------------------------------------------

void menu_rgb_ui::inkey_special(const event *menu_event)
{
	if (menu_event->iptkey == IPT_UI_SELECT)
	{
		m_key_active = !m_key_active;
		m_lock_ref = (FPTR)menu_event->itemref;

		if (!m_key_active)
		{
			int val = atoi(m_search);
			val = m_color->clamp(val);

			switch ((FPTR)menu_event->itemref)
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

	auto const buflen = std::strlen(m_search);
	if ((menu_event->unichar == 8) || (menu_event->unichar == 0x7f))
	{
		// if it's a backspace and we can handle it, do so
		if (0 < buflen)
			*const_cast<char *>(utf8_previous_char(&m_search[buflen])) = 0;
	}
	else if (buflen >= 3)
	{
		return;
	}
	else if ((menu_event->unichar >= '0' && menu_event->unichar <= '9'))
	{
		// if it's any other key and we're not maxed out, update
		menu_event->append_char(m_search, buflen);
	}
}

std::pair<const char *, const char *> const menu_palette_sel::s_palette[] = {
	{ __("White"),  "FFFFFFFF" },
	{ __("Silver"), "FFC0C0C0" },
	{ __("Gray"),   "FF808080" },
	{ __("Black"),  "FF000000" },
	{ __("Red"),    "FFFF0000" },
	{ __("Orange"), "FFFFA500" },
	{ __("Yellow"), "FFFFFF00" },
	{ __("Green"),  "FF00FF00" },
	{ __("Blue"),   "FF0000FF" },
	{ __("Violet"), "FF8F00FF" }
};

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_palette_sel::menu_palette_sel(mame_ui_manager &mui, render_container &container, rgb_t &_color)
	: menu(mui, container), m_original(_color)
{
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_palette_sel::~menu_palette_sel()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_palette_sel::handle()
{
	// process the menu
	const event *menu_event = process(0);
	if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		if (menu_event->iptkey == IPT_UI_SELECT)
		{
			m_original = rgb_t((UINT32)strtoul(item[selected].subtext.c_str(), nullptr, 16));
			reset_parent(reset_options::SELECT_FIRST);
			stack_pop();
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_palette_sel::populate()
{
	for (unsigned x = 0; x < ARRAY_LENGTH(s_palette); ++x)
		item_append(_(s_palette[x].first), s_palette[x].second, 0, (void *)(FPTR)(x + 1));

	item_append(menu_item_type::SEPARATOR);
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_palette_sel::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
}

//-------------------------------------------------
//  draw - draw palette menu
//-------------------------------------------------

void menu_palette_sel::draw(UINT32 flags)
{
	auto line_height = ui().get_line_height();
	auto lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	auto ud_arrow_width = line_height * machine().render().ui_aspect();
	auto gutter_width = lr_arrow_width * 1.3f;
	int itemnum, linenum;

	if (&machine().system() == &GAME_NAME(___empty))
		draw_background();

	// compute the width and height of the full menu
	auto visible_width = 0.0f;
	auto visible_main_menu_height = 0.0f;
	for (auto & pitem : item)
	{
		// compute width of left hand side
		auto total_width = gutter_width + ui().get_string_width(pitem.text.c_str()) + gutter_width;

		// add in width of right hand side
		if (!pitem.subtext.empty())
			total_width += 2.0f * gutter_width + ui().get_string_width(pitem.subtext.c_str());

		// track the maximum
		if (total_width > visible_width)
			visible_width = total_width;

		// track the height as well
		visible_main_menu_height += line_height;
	}

	// account for extra space at the top and bottom
	auto visible_extra_menu_height = customtop + custombottom;

	// add a little bit of slop for rounding
	visible_width += 0.01f;
	visible_main_menu_height += 0.01f;

	// if we are too wide or too tall, clamp it down
	if (visible_width + 2.0f * UI_BOX_LR_BORDER > 1.0f)
		visible_width = 1.0f - 2.0f * UI_BOX_LR_BORDER;

	// if the menu and extra menu won't fit, take away part of the regular menu, it will scroll
	if (visible_main_menu_height + visible_extra_menu_height + 2.0f * UI_BOX_TB_BORDER > 1.0f)
		visible_main_menu_height = 1.0f - 2.0f * UI_BOX_TB_BORDER - visible_extra_menu_height;

	int visible_lines = floor(visible_main_menu_height / line_height);
	visible_main_menu_height = (float)visible_lines * line_height;

	// compute top/left of inner menu area by centering
	float visible_left = (1.0f - visible_width) * 0.5f;
	float visible_top = (1.0f - (visible_main_menu_height + visible_extra_menu_height)) * 0.5f;

	// if the menu is at the bottom of the extra, adjust
	visible_top += customtop;

	// first add us a box
	float x1 = visible_left - UI_BOX_LR_BORDER;
	float y1 = visible_top - UI_BOX_TB_BORDER;
	float x2 = visible_left + visible_width + UI_BOX_LR_BORDER;
	float y2 = visible_top + visible_main_menu_height + UI_BOX_TB_BORDER;
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// determine the first visible line based on the current selection
	int top_line = selected - visible_lines / 2;
	if (top_line < 0)
		top_line = 0;
	if (top_line + visible_lines >= item.size())
		top_line = item.size() - visible_lines;

	// determine effective positions taking into account the hilighting arrows
	float effective_width = visible_width - 2.0f * gutter_width;
	float effective_left = visible_left + gutter_width;

	// locate mouse
	mouse_hit = false;
	mouse_button = false;
	mouse_target = machine().ui_input().find_mouse(&mouse_target_x, &mouse_target_y, &mouse_button);
	if (mouse_target != nullptr)
		if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, container(), mouse_x, mouse_y))
			mouse_hit = true;

	// loop over visible lines
	hover = item.size() + 1;
	float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
	float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;

	for (linenum = 0; linenum < visible_lines; linenum++)
	{
		float line_y = visible_top + (float)linenum * line_height;
		itemnum = top_line + linenum;
		const menu_item &pitem = item[itemnum];
		const char *itemtext = pitem.text.c_str();
		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		float line_y0 = line_y;
		float line_y1 = line_y + line_height;

		// set the hover if this is our item
		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && is_selectable(pitem))
			hover = itemnum;

		// if we're selected, draw with a different background
		if (itemnum == selected)
		{
			fgcolor = UI_SELECTED_COLOR;
			bgcolor = UI_SELECTED_BG_COLOR;
		}

		// else if the mouse is over this item, draw with a different background
		else if (itemnum == hover)
		{
			fgcolor = UI_MOUSEOVER_COLOR;
			bgcolor = UI_MOUSEOVER_BG_COLOR;
		}

		// if we have some background hilighting to do, add a quad behind everything else
		if (bgcolor != UI_TEXT_BG_COLOR)
			highlight(line_x0, line_y0, line_x1, line_y1, bgcolor);

		// if we're on the top line, display the up arrow
		if (linenum == 0 && top_line != 0)
		{
			draw_arrow(
				0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
				line_y + 0.25f * line_height,
				0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
				line_y + 0.75f * line_height,
				fgcolor,
				ROT0);
			if (hover == itemnum)
				hover = HOVER_ARROW_UP;
		}

		// if we're on the bottom line, display the down arrow
		else if (linenum == visible_lines - 1 && itemnum != item.size() - 1)
		{
			draw_arrow(
				0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
				line_y + 0.25f * line_height,
				0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
				line_y + 0.75f * line_height,
				fgcolor,
				ROT0 ^ ORIENTATION_FLIP_Y);
			if (hover == itemnum)
				hover = HOVER_ARROW_DOWN;
		}

		// if we're just a divider, draw a line
		else if (pitem.type == menu_item_type::SEPARATOR)
			container().add_line(visible_left, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		// if we don't have a subitem, just draw the string centered
		else if (pitem.subtext.empty())
			ui().draw_text_full(container(), itemtext, effective_left, line_y, effective_width,
				ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr);

		// otherwise, draw the item on the left and the subitem text on the right
		else
		{
			const char *subitem_text = pitem.subtext.c_str();
			rgb_t color = rgb_t((UINT32)strtoul(subitem_text, nullptr, 16));

			// draw the left-side text
			ui().draw_text_full(container(), itemtext, effective_left, line_y, effective_width,
				ui::text_layout::LEFT, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr);

			// give 2 spaces worth of padding
			float subitem_width = ui().get_string_width("FF00FF00");

			ui().draw_outlined_box(container(), effective_left + effective_width - subitem_width, line_y0,
				effective_left + effective_width, line_y1, color);
		}
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render(get_selection_ref(), customtop, custombottom, x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	m_visible_items = visible_lines - (top_line != 0) - (top_line + visible_lines != item.size());
}

} // namespace ui
