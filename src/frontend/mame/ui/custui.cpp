// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/custui.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/custui.h"

#include "ui/selector.h"
#include "ui/ui.h"
#include "ui/utils.h"

#include "drivenum.h"
#include "emuopts.h"
#include "fileio.h"
#include "uiinput.h"

#include "corestr.h"
#include "osdepend.h"

#include <algorithm>
#include <iterator>
#include <locale>
#include <sstream>
#include <utility>


namespace ui {

namespace {

enum
{
	LANGUAGE_MENU = 1,
	SYSNAMES_MENU,
	FONT_MENU,
	COLORS_MENU,
	HIDE_MENU,

	INFOS_SIZE = 1,
	FONT_SIZE,
	MUI_FNT,
	MUI_BOLD,
	MUI_ITALIC
};

const char *const HIDE_STATUS[] = {
		N_("Show All"),
		N_("Hide Filters"),
		N_("Hide Info/Image"),
		N_("Hide Both") };

template <typename T, typename U>
T parse_number(U &&s)
{
	T result(T(0));
	std::istringstream ss(std::forward<U>(s));
	ss.imbue(std::locale::classic());
	ss >> result;
	return result;
}

} // anonymous namespace


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_custom_ui::menu_custom_ui(mame_ui_manager &mui, render_container &container, std::function<void ()> &&handler)
	: menu(mui, container)
	, m_handler(std::move(handler))
	, m_currlang(0)
	, m_currsysnames(0)
{
	set_process_flags(PROCESS_LR_REPEAT);
	find_languages();
	find_sysnames();
}

//-------------------------------------------------
//  menu dismissed
//-------------------------------------------------

void menu_custom_ui::menu_dismissed()
{
	ui().options().set_value(OPTION_HIDE_PANELS, ui_globals::panels_status, OPTION_PRIORITY_CMDLINE);

	machine().options().set_value(OPTION_LANGUAGE, m_currlang ? m_languages[m_currlang] : "", OPTION_PRIORITY_CMDLINE);
	load_translation(machine().options());

	ui().options().set_value(OPTION_SYSTEM_NAMES, m_currsysnames ? m_sysnames[m_currsysnames] : "", OPTION_PRIORITY_CMDLINE);

	ui_globals::reset = true;

	if (m_handler)
		m_handler();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_custom_ui::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref)
	{
		switch ((uintptr_t)ev->itemref)
		{
		case FONT_MENU:
			if (ev->iptkey == IPT_UI_SELECT)
				menu::stack_push<menu_font_ui>(ui(), container(), nullptr);
			break;
		case COLORS_MENU:
			if (ev->iptkey == IPT_UI_SELECT)
				menu::stack_push<menu_colors_ui>(ui(), container());
			break;
		case LANGUAGE_MENU:
			if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT) || (ev->iptkey == IPT_UI_CLEAR))
			{
				if (ev->iptkey == IPT_UI_LEFT)
					--m_currlang;
				else if (ev->iptkey == IPT_UI_RIGHT)
					++m_currlang;
				else
					m_currlang = 0;
				ev->item->set_subtext(m_languages[m_currlang]);
				ev->item->set_flags(get_arrow_flags<std::size_t>(0, m_languages.size() - 1, m_currlang));
			}
			else if (ev->iptkey == IPT_UI_SELECT)
			{
				// copying list of language names - expensive
				menu::stack_push<menu_selector>(
						ui(), container(), std::vector<std::string>(m_languages), m_currlang,
						[this, item = ev->item] (int selection)
						{
							m_currlang = selection;
							item->set_subtext(m_languages[selection]);
							item->set_flags(get_arrow_flags<std::size_t>(0, m_languages.size() - 1, selection));
						});
			}
			break;
		case SYSNAMES_MENU:
			if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT) || (ev->iptkey == IPT_UI_CLEAR))
			{
				if (ev->iptkey == IPT_UI_LEFT)
					--m_currsysnames;
				else if (ev->iptkey == IPT_UI_RIGHT)
					++m_currsysnames;
				else
					m_currsysnames = 0;
				ev->item->set_subtext(m_sysnames[m_currsysnames]);
				ev->item->set_flags(get_arrow_flags<std::size_t>(0, m_sysnames.size() - 1, m_currsysnames));
			}
			else if (ev->iptkey == IPT_UI_SELECT)
			{
				// copying list of file names - expensive
				menu::stack_push<menu_selector>(
						ui(), container(), std::vector<std::string>(m_sysnames), m_currsysnames,
						[this, item = ev->item] (int selection)
						{
							m_currsysnames = selection;
							item->set_subtext(m_sysnames[selection]);
							item->set_flags(get_arrow_flags<std::size_t>(0, m_sysnames.size() - 1, selection));
						});
			}
			break;
		case HIDE_MENU:
			if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT) || (ev->iptkey == IPT_UI_CLEAR))
			{
				if (ev->iptkey == IPT_UI_LEFT)
					--ui_globals::panels_status;
				else if (ev->iptkey == IPT_UI_RIGHT)
					++ui_globals::panels_status;
				else
					ui_globals::panels_status = 0;
				ev->item->set_subtext(_(HIDE_STATUS[ui_globals::panels_status]));
				ev->item->set_flags(get_arrow_flags<uint16_t>(0, HIDE_BOTH, ui_globals::panels_status));
			}
			else if (ev->iptkey == IPT_UI_SELECT)
			{
				std::vector<std::string> s_sel(std::size(HIDE_STATUS));
				std::transform(std::begin(HIDE_STATUS), std::end(HIDE_STATUS), s_sel.begin(), [](auto &s) { return _(s); });
				menu::stack_push<menu_selector>(
						ui(), container(), std::move(s_sel), ui_globals::panels_status,
						[item = ev->item] (int selection)
						{
							ui_globals::panels_status = selection;
							item->set_subtext(_(HIDE_STATUS[selection]));
							item->set_flags(get_arrow_flags<uint16_t>(0, HIDE_BOTH, selection));
						});
			}
			break;
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_custom_ui::populate(float &customtop, float &custombottom)
{
	uint32_t arrow_flags;
	item_append(_("Fonts"), 0, (void *)(uintptr_t)FONT_MENU);
	item_append(_("Colors"), 0, (void *)(uintptr_t)COLORS_MENU);

	arrow_flags = get_arrow_flags<std::size_t>(0, m_languages.size() - 1, m_currlang);
	item_append(_("Language"), m_languages[m_currlang], arrow_flags, (void *)(uintptr_t)LANGUAGE_MENU);

	arrow_flags = get_arrow_flags<std::size_t>(0, m_sysnames.size() - 1, m_currsysnames);
	item_append(_("System Names"), m_sysnames[m_currsysnames], arrow_flags, (void *)(uintptr_t)SYSNAMES_MENU);

	arrow_flags = get_arrow_flags<uint16_t>(0, HIDE_BOTH, ui_globals::panels_status);
	item_append(_("Show side panels"), _(HIDE_STATUS[ui_globals::panels_status]), arrow_flags, (void *)(uintptr_t)HIDE_MENU);

	item_append(menu_item_type::SEPARATOR);
	customtop = ui().get_line_height() + 3.0f * ui().box_tb_border();
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_custom_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	char const *const text[] = { _("UI Customization Settings") };
	draw_text_box(
			std::begin(text), std::end(text),
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
}

//-------------------------------------------------
//  find UI translation files
//-------------------------------------------------

void menu_custom_ui::find_languages()
{
	m_languages.emplace_back(_("[built-in]"));

	file_enumerator path(machine().options().language_path());
	osd::directory::entry const *dirent;
	std::string name;
	while ((dirent = path.next()))
	{
		if (dirent->type == osd::directory::entry::entry_type::DIR && strcmp(dirent->name, ".") != 0 && strcmp(dirent->name, "..") != 0)
		{
			name = dirent->name;
			auto i = strreplace(name, "_", " (");
			if (i > 0)
				name.append(")");
			m_languages.emplace_back(std::move(name));
		}
	}
	std::sort(
			std::next(m_languages.begin()),
			m_languages.end(),
			[] (std::string const &x, std::string const &y) { return 0 > core_stricmp(x.c_str(), y.c_str()); });

	char const *const lang = machine().options().language();
	if (*lang)
	{
		auto const found = std::lower_bound(
				std::next(m_languages.begin()),
				m_languages.end(),
				lang,
				[] (std::string const &x, char const *y) { return 0 > core_stricmp(x.c_str(), y); });
		if ((m_languages.end() != found) && !core_stricmp(found->c_str(), lang))
			m_currlang = std::distance(m_languages.begin(), found);
	}
	else
	{
		m_currlang = 0;
	}
}

//-------------------------------------------------
//  find translated system names
//-------------------------------------------------

void menu_custom_ui::find_sysnames()
{
	m_sysnames.emplace_back(_("[built-in]"));

	path_iterator search(ui().options().history_path());
	std::string path;
	while (search.next(path))
	{
		file_enumerator dir(path);
		osd::directory::entry const *dirent;
		while ((dirent = dir.next()))
		{
			if (dirent->type == osd::directory::entry::entry_type::FILE && core_filename_ends_with(dirent->name, ".lst"))
				m_sysnames.emplace_back(dirent->name);
		}
	}
	std::sort(
			m_sysnames.begin(),
			m_sysnames.end(),
			[] (std::string const &x, std::string const &y) { return 0 > core_stricmp(x.c_str(), y.c_str()); });

	char const *const names = ui().options().system_names();
	if (*names)
	{
		auto const found = std::lower_bound(
				std::next(m_sysnames.begin()),
				m_sysnames.end(),
				names,
				[] (std::string const &x, char const *y) { return 0 > core_stricmp(x.c_str(), y); });
		m_currsysnames = std::distance(m_sysnames.begin(), found);
		if ((m_sysnames.end() == found) || core_stricmp(found->c_str(), names))
			m_sysnames.emplace(found, names);
	}
	else
	{
		m_currsysnames = 0;
	}
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_font_ui::menu_font_ui(mame_ui_manager &mui, render_container &container, std::function<void (bool)> &&handler)
	: menu(mui, container)
	, m_handler(std::move(handler))
	, m_fonts()
	, m_font_min(parse_number<int>(mui.options().get_entry(OPTION_FONT_ROWS)->minimum()))
	, m_font_max(parse_number<int>(mui.options().get_entry(OPTION_FONT_ROWS)->maximum()))
	, m_font_size(mui.options().font_rows())
	, m_info_min(parse_number<float>(mui.options().get_entry(OPTION_INFOS_SIZE)->minimum()))
	, m_info_max(parse_number<float>(mui.options().get_entry(OPTION_INFOS_SIZE)->maximum()))
	, m_info_size(mui.options().infos_size())
	, m_changed(false)
	, m_actual(0U)
{
	set_process_flags(PROCESS_LR_REPEAT);

	std::string name(mui.machine().options().ui_font());
	list();

#ifdef UI_WINDOWS
	m_bold = (strreplace(name, "[B]", "") + strreplace(name, "[b]", "") > 0);
	m_italic = (strreplace(name, "[I]", "") + strreplace(name, "[i]", "") > 0);
#endif

	for (std::size_t index = 0; index < m_fonts.size(); index++)
	{
		if (m_fonts[index].first == name)
		{
			m_actual = index;
			break;
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
//  menu dismissed
//-------------------------------------------------

void menu_font_ui::menu_dismissed()
{
	if (m_changed)
	{
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
		machine().options().set_value(OPTION_UI_FONT, name, OPTION_PRIORITY_CMDLINE);
		moptions.set_value(OPTION_INFOS_SIZE, m_info_size, OPTION_PRIORITY_CMDLINE);
		moptions.set_value(OPTION_FONT_ROWS, m_font_size, OPTION_PRIORITY_CMDLINE);

		// OPTION_FONT_ROWS was changed; update the font info
		ui().update_target_font_height();
	}

	if (m_handler)
		m_handler(m_changed);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_font_ui::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref)
	{
		switch ((uintptr_t)ev->itemref)
		{
			case FONT_SIZE:
				if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT) || (ev->iptkey == IPT_UI_CLEAR))
				{
					m_changed = true;
					if (ev->iptkey == IPT_UI_LEFT)
						--m_font_size;
					else if (ev->iptkey == IPT_UI_RIGHT)
						++m_font_size;
					else
						m_font_size = parse_number<int>(ui().options().get_entry(OPTION_FONT_ROWS)->default_value().c_str());
					ev->item->set_subtext(string_format("%d", m_font_size));
					ev->item->set_flags(get_arrow_flags(m_font_min, m_font_max, m_font_size));
				}
				break;

			case INFOS_SIZE:
				if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT) || (ev->iptkey == IPT_UI_CLEAR))
				{
					m_changed = true;
					if (ev->iptkey == IPT_UI_LEFT)
						m_info_size -= 0.05f;
					else if (ev->iptkey == IPT_UI_RIGHT)
						m_info_size += 0.05f;
					else
						m_info_size = parse_number<float>(ui().options().get_entry(OPTION_INFOS_SIZE)->default_value().c_str());
					ev->item->set_subtext(string_format("%.2f", m_info_size));
					ev->item->set_flags(get_arrow_flags(m_info_min, m_info_max, m_info_size));
				}
				break;

			case MUI_FNT:
				if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT) || (ev->iptkey == IPT_UI_CLEAR))
				{
					m_changed = true;
					if (ev->iptkey == IPT_UI_LEFT)
						--m_actual;
					else if (ev->iptkey == IPT_UI_RIGHT)
						++m_actual;
					else
						m_actual = 0;
					reset(reset_options::REMEMBER_REF);
				}
				else if (ev->iptkey == IPT_UI_SELECT)
				{
					std::vector<std::string> display_names;
					display_names.reserve(m_fonts.size());
					for (auto const &font : m_fonts)
						display_names.emplace_back(font.second);
					menu::stack_push<menu_selector>(
							ui(), container(), std::move(display_names), m_actual,
							[this] (int selection)
							{
								m_changed = true;
								m_actual = selection;
								reset(reset_options::REMEMBER_REF);
							});
				}
				break;

#ifdef UI_WINDOWS
			case MUI_BOLD:
			case MUI_ITALIC:
				if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT) || (ev->iptkey == IPT_UI_SELECT) || (ev->iptkey == IPT_UI_CLEAR))
				{
					m_changed = true;
					bool &val = ((uintptr_t)ev->itemref == MUI_BOLD) ? m_bold : m_italic;
					if (ev->iptkey == IPT_UI_CLEAR)
						val = false;
					else
						val = !val;
					ev->item->set_subtext(val ? _("On") : _("Off"));
					ev->item->set_flags(val ? FLAG_LEFT_ARROW : FLAG_RIGHT_ARROW);
				}
				break;
#endif
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_font_ui::populate(float &customtop, float &custombottom)
{
	// set filter arrow
	uint32_t arrow_flags;

	arrow_flags = get_arrow_flags<std::uint16_t>(0, m_fonts.size() - 1, m_actual);
	item_append(_("UI Font"), m_fonts[m_actual].second, arrow_flags, (void *)(uintptr_t)MUI_FNT);

#ifdef UI_WINDOWS
	if (m_fonts[m_actual].first != "default")
	{
		item_append_on_off(_("Bold"), m_bold, 0, (void *)(uintptr_t)MUI_BOLD);
		item_append_on_off(_("Italic"), m_italic, 0, (void *)(uintptr_t)MUI_ITALIC);
	}
#endif

	arrow_flags = get_arrow_flags(m_font_min, m_font_max, m_font_size);
	item_append(_("Lines"), string_format("%d", m_font_size), arrow_flags, (void *)(uintptr_t)FONT_SIZE);

	item_append(menu_item_type::SEPARATOR);

	arrow_flags = get_arrow_flags(m_info_min, m_info_max, m_info_size);
	item_append(_("Infos text size"), string_format("%.2f", m_info_size), arrow_flags, (void *)(uintptr_t)INFOS_SIZE);

	item_append(menu_item_type::SEPARATOR);

	custombottom = customtop = ui().get_line_height() + 3.0f * ui().box_tb_border();
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_font_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// top text
	char const *const toptext[] = { _("UI Fonts Settings") };
	draw_text_box(
			std::begin(toptext), std::end(toptext),
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);

	if (uintptr_t(selectedref) == INFOS_SIZE)
	{
		char const *const bottomtext[] = { _("Sample text - Lorem ipsum dolor sit amet, consectetur adipiscing elit.") };
		draw_text_box(
				std::begin(bottomtext), std::end(bottomtext),
				origx1, origx2, origy2 + ui().box_tb_border(), origy2 + bottom,
				text_layout::text_justify::LEFT, text_layout::word_wrapping::NEVER, false,
				ui().colors().text_color(), UI_GREEN_COLOR, m_info_size);
	}
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------
#define SET_COLOR_UI(var, opt) var[M##opt].color = mui.options().rgb_value(OPTION_##opt); var[M##opt].option = OPTION_##opt

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
//  menu dismissed
//-------------------------------------------------

void menu_colors_ui::menu_dismissed()
{
	std::string dec_color;
	for (int index = 1; index < MUI_RESTORE; index++)
	{
		dec_color = string_format("%x", (uint32_t)m_color_table[index].color);
		ui().options().set_value(m_color_table[index].option, dec_color, OPTION_PRIORITY_CMDLINE);
	}

	// refresh our cached colors
	ui().colors().refresh(ui().options());
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_colors_ui::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref && ev->iptkey == IPT_UI_SELECT)
	{
		if ((uintptr_t)ev->itemref != MUI_RESTORE)
		{
			menu::stack_push<menu_rgb_ui>(ui(), container(), &m_color_table[(uintptr_t)ev->itemref].color, std::string(selected_item().text()));
		}
		else
		{
			restore_colors();
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_colors_ui::populate(float &customtop, float &custombottom)
{
	item_append(_("color-option", "Normal text"),                 0, (void *)(uintptr_t)MUI_TEXT_COLOR);
	item_append(_("color-option", "Selected color"),              0, (void *)(uintptr_t)MUI_SELECTED_COLOR);
	item_append(_("color-option", "Normal text background"),      0, (void *)(uintptr_t)MUI_TEXT_BG_COLOR);
	item_append(_("color-option", "Selected background color"),   0, (void *)(uintptr_t)MUI_SELECTED_BG_COLOR);
	item_append(_("color-option", "Subitem color"),               0, (void *)(uintptr_t)MUI_SUBITEM_COLOR);
	item_append(_("color-option", "Clone"),                       0, (void *)(uintptr_t)MUI_CLONE_COLOR);
	item_append(_("color-option", "Border"),                      0, (void *)(uintptr_t)MUI_BORDER_COLOR);
	item_append(_("color-option", "Background"),                  0, (void *)(uintptr_t)MUI_BACKGROUND_COLOR);
	item_append(_("color-option", "DIP switch"),                  0, (void *)(uintptr_t)MUI_DIPSW_COLOR);
	item_append(_("color-option", "Unavailable color"),           0, (void *)(uintptr_t)MUI_UNAVAILABLE_COLOR);
	item_append(_("color-option", "Slider color"),                0, (void *)(uintptr_t)MUI_SLIDER_COLOR);
	item_append(_("color-option", "Graphics viewer background"),  0, (void *)(uintptr_t)MUI_GFXVIEWER_BG_COLOR);
	item_append(_("color-option", "Mouse over color"),            0, (void *)(uintptr_t)MUI_MOUSEOVER_COLOR);
	item_append(_("color-option", "Mouse over background color"), 0, (void *)(uintptr_t)MUI_MOUSEOVER_BG_COLOR);
	item_append(_("color-option", "Mouse down color"),            0, (void *)(uintptr_t)MUI_MOUSEDOWN_COLOR);
	item_append(_("color-option", "Mouse down background color"), 0, (void *)(uintptr_t)MUI_MOUSEDOWN_BG_COLOR);

	item_append(menu_item_type::SEPARATOR);

	item_append(_("Restore default colors"), 0, (void *)(uintptr_t)MUI_RESTORE);

	custombottom = customtop = ui().get_line_height() + 3.0f * ui().box_tb_border();
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_colors_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// top text
	char const *const toptext[] = { _("UI Color Settings") };
	draw_text_box(
			std::begin(toptext), std::end(toptext),
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);

	// bottom text
	// get the text for 'UI Select'
	std::string const bottomtext[] = { util::string_format(_("Double-click or press %1$s to change color"), ui().get_general_input_setting(IPT_UI_SELECT)) };
	draw_text_box(
			std::begin(bottomtext), std::end(bottomtext),
			origx1, origx2, origy2 + ui().box_tb_border(), origy2 + bottom,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), ui().colors().background_color(), 1.0f);

	// compute maxwidth
	char const *const topbuf = _("Menu Preview");

	const float lr_border = ui().box_lr_border() * machine().render().ui_aspect(&container());
	float width;
	ui().draw_text_full(
			container(),
			topbuf,
			0.0f, 0.0f, 1.0f,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(), &width, nullptr);
	float maxwidth = width + 2.0f * lr_border;

	std::string sampletxt[5];

	sampletxt[0] = _("color-sample", "Normal");
	sampletxt[1] = _("color-sample", "Subitem");
	sampletxt[2] = _("color-sample", "Selected");
	sampletxt[3] = _("color-sample", "Mouse Over");
	sampletxt[4] = _("color-sample", "Clone");

	for (auto & elem: sampletxt)
	{
		ui().draw_text_full(
				container(),
				elem,
				0.0f, 0.0f, 1.0f,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
				mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(), &width, nullptr);
		width += 2 * lr_border;
		maxwidth = std::max(maxwidth, width);
	}

	// compute our bounds for header
	float x1 = origx2 + 2.0f * lr_border;
	float x2 = x1 + maxwidth;
	float y1 = origy1;
	float y2 = y1 + bottom - ui().box_tb_border();

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += lr_border;
	x2 -= lr_border;
	y1 += ui().box_tb_border();
	y2 -= ui().box_tb_border();

	// draw the text within it
	ui().draw_text_full(
			container(),
			topbuf,
			x1, y1, x2 - x1,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NORMAL, ui().colors().text_color(), ui().colors().text_bg_color(), nullptr, nullptr);

	// compute our bounds for menu preview
	float line_height = ui().get_line_height();
	x1 -= lr_border;
	x2 += lr_border;
	y1 = y2 + 2.0f * ui().box_tb_border();
	y2 = y1 + 5.0f * line_height + 2.0f * ui().box_tb_border();

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, m_color_table[MUI_BACKGROUND_COLOR].color);

	// take off the borders
	x1 += lr_border;
	x2 -= lr_border;
	y1 += ui().box_tb_border();

	// draw normal text
	ui().draw_text_full(
			container(),
			sampletxt[0],
			x1, y1, x2 - x1,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NORMAL, m_color_table[MUI_TEXT_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw subitem text
	ui().draw_text_full(
			container(),
			sampletxt[1],
			x1, y1, x2 - x1,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NORMAL, m_color_table[MUI_SUBITEM_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw selected text
	highlight(x1, y1, x2, y1 + line_height, m_color_table[MUI_SELECTED_BG_COLOR].color);
	ui().draw_text_full(
			container(),
			sampletxt[2],
			x1, y1, x2 - x1,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NORMAL, m_color_table[MUI_SELECTED_COLOR].color, m_color_table[MUI_SELECTED_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw mouse over text
	highlight(x1, y1, x2, y1 + line_height, m_color_table[MUI_MOUSEOVER_BG_COLOR].color);
	ui().draw_text_full(
			container(),
			sampletxt[3],
			x1, y1, x2 - x1,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NORMAL, m_color_table[MUI_MOUSEOVER_COLOR].color, m_color_table[MUI_MOUSEOVER_BG_COLOR].color, nullptr, nullptr);
	y1 += line_height;

	// draw clone text
	ui().draw_text_full(
			container(),
			sampletxt[4],
			x1, y1, x2 - x1,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NORMAL, m_color_table[MUI_CLONE_COLOR].color, m_color_table[MUI_TEXT_BG_COLOR].color, nullptr, nullptr);

}

//-------------------------------------------------
//  restore original colors
//-------------------------------------------------

void menu_colors_ui::restore_colors()
{
	ui_options options;
	for (int index = 1; index < MUI_RESTORE; index++)
		m_color_table[index].color = rgb_t((uint32_t)strtoul(options.value(m_color_table[index].option), nullptr, 16));
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_rgb_ui::menu_rgb_ui(mame_ui_manager &mui, render_container &container, rgb_t *color, std::string &&title)
	: menu(mui, container)
	, m_color(color)
	, m_search()
	, m_key_active(false)
	, m_lock_ref(0)
	, m_title(std::move(title))
{
	set_process_flags(PROCESS_LR_REPEAT);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_rgb_ui::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref)
	{
		switch (ev->iptkey)
		{
		case IPT_UI_LEFT:
		case IPT_UI_RIGHT:
			{
				bool changed = false;
				int updated = (IPT_UI_LEFT == ev->iptkey) ? -1 : 1;
				switch (uintptr_t(ev->itemref))
				{
				case RGB_ALPHA:
					updated += m_color->a();
					if ((0 <= updated) && (255 >= updated))
					{
						m_color->set_a(updated);
						changed = true;
					}
					break;
				case RGB_RED:
					updated += m_color->r();
					if ((0 <= updated) && (255 >= updated))
					{
						m_color->set_r(updated);
						changed = true;
					}
					break;
				case RGB_GREEN:
					updated += m_color->g();
					if ((0 <= updated) && (255 >= updated))
					{
						m_color->set_g(updated);
						changed = true;
					}
					break;
				case RGB_BLUE:
					updated += m_color->b();
					if ((0 <= updated) && (255 >= updated))
					{
						m_color->set_b(updated);
						changed = true;
					}
					break;
				}
				if (changed)
				{
					ev->item->set_subtext(string_format("%3u", updated));
					ev->item->set_flags(get_arrow_flags<uint8_t>(0, 255, updated));
				}
			}
			break;

		case IPT_UI_SELECT:
			if (uintptr_t(ev->itemref) == PALETTE_CHOOSE)
			{
				menu::stack_push<menu_palette_sel>(ui(), container(), *m_color);
				break;
			}
			[[fallthrough]];
		case IPT_SPECIAL:
			switch (uintptr_t(ev->itemref))
			{
			case RGB_ALPHA:
			case RGB_RED:
			case RGB_GREEN:
			case RGB_BLUE:
				inkey_special(ev);
				break;
			}
			break;
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_rgb_ui::populate(float &customtop, float &custombottom)
{
	// set filter arrow
	std::string s_text = std::string(m_search).append("_");
	item_append(_("ARGB Settings"), FLAG_DISABLE | FLAG_UI_HEADING, nullptr);

	if (m_lock_ref != RGB_ALPHA)
	{
		uint32_t arrow_flags = get_arrow_flags<uint8_t>(0, 255, m_color->a());
		item_append(_("color-channel", "Alpha"), string_format("%3u", m_color->a()), arrow_flags, (void *)(uintptr_t)RGB_ALPHA);
	}
	else
		item_append(_("color-channel", "Alpha"), s_text, 0, (void *)(uintptr_t)RGB_ALPHA);

	if (m_lock_ref != RGB_RED)
	{
		uint32_t arrow_flags = get_arrow_flags<uint8_t>(0, 255, m_color->r());
		item_append(_("color-channel", "Red"), string_format("%3u", m_color->r()), arrow_flags, (void *)(uintptr_t)RGB_RED);
	}
	else
		item_append(_("color-channel", "Red"), s_text, 0, (void *)(uintptr_t)RGB_RED);

	if (m_lock_ref != RGB_GREEN)
	{
		uint32_t arrow_flags = get_arrow_flags<uint8_t>(0, 255, m_color->g());
		item_append(_("color-channel", "Green"), string_format("%3u", m_color->g()), arrow_flags, (void *)(uintptr_t)RGB_GREEN);
	}
	else
		item_append(_("color-channel", "Green"), s_text, 0, (void *)(uintptr_t)RGB_GREEN);

	if (m_lock_ref != RGB_BLUE)
	{
		uint32_t arrow_flags = get_arrow_flags<uint8_t>(0, 255, m_color->b());
		item_append(_("color-channel", "Blue"), string_format("%3u", m_color->b()), arrow_flags, (void *)(uintptr_t)RGB_BLUE);
	}
	else
		item_append(_("color-channel", "Blue"), s_text, 0, (void *)(uintptr_t)RGB_BLUE);

	item_append(menu_item_type::SEPARATOR);
	item_append(_("Choose from palette"), 0, (void *)(uintptr_t)PALETTE_CHOOSE);
	item_append(menu_item_type::SEPARATOR);

	custombottom = customtop = ui().get_line_height() + 3.0f * ui().box_tb_border();
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_rgb_ui::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;

	// top text
	ui().draw_text_full(
			container(),
			m_title,
			0.0f, 0.0f, 1.0f,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(), &width);
	const float lr_border = ui().box_lr_border() * machine().render().ui_aspect(&container());
	width += 2 * lr_border;
	maxwidth = std::max(maxwidth, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - ui().box_tb_border();

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += lr_border;
	x2 -= lr_border;
	y1 += ui().box_tb_border();

	// draw the text within it
	ui().draw_text_full(
			container(),
			m_title,
			x1, y1, x2 - x1,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NORMAL, ui().colors().text_color(), ui().colors().text_bg_color());

	std::string sampletxt(_("Color preview:"));
	ui().draw_text_full(
			container(),
			sampletxt,
			0.0f, 0.0f, 1.0f,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(),
			&width);
	width += 2 * lr_border;
	maxwidth = std::max(origx2 - origx1, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + ui().box_tb_border();
	y2 = origy2 + bottom;

	// draw a box - force black to ensure the text is legible
	ui().draw_outlined_box(container(), x1, y1, x2, y2, rgb_t::black());

	// take off the borders
	x1 += lr_border;
	y1 += ui().box_tb_border();

	// draw the text label - force white to ensure it's legible
	ui().draw_text_full(
			container(),
			sampletxt,
			x1, y1, width - lr_border,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NORMAL, rgb_t::white(), rgb_t::black());

	x1 += width + (lr_border * 2.0f);
	x2 -= lr_border;
	y2 -= ui().box_tb_border();

	// add white under half the sample swatch to make alpha effects visible
	container().add_rect((x1 + x2) * 0.5f, y1, x2, y2, rgb_t::white(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container().add_rect(x1, y1, x2, y2, *m_color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}

//-------------------------------------------------
//  handle special key event
//-------------------------------------------------

void menu_rgb_ui::inkey_special(const event *menu_event)
{
	if (menu_event->iptkey == IPT_UI_SELECT)
	{
		m_key_active = !m_key_active;
		set_process_flags(m_key_active ? PROCESS_ONLYCHAR : PROCESS_LR_REPEAT);
		m_lock_ref = (uintptr_t)menu_event->itemref;

		if (!m_key_active)
		{
			int val = atoi(m_search.data());
			val = m_color->clamp(val);

			switch ((uintptr_t)menu_event->itemref)
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

			m_search.erase();
			m_lock_ref = 0;

			menu_event->item->set_subtext(string_format("%3u", val));
			menu_event->item->set_flags(get_arrow_flags<uint8_t>(0, 255, val));
		}
		else
		{
			menu_event->item->set_subtext("_");
			menu_event->item->set_flags(0);
		}
	}
	else if (m_key_active)
	{
		input_character(m_search, 3, menu_event->unichar, uchar_is_digit);
		menu_event->item->set_subtext(m_search + "_");
	}
}

std::pair<const char *, const char *> const menu_palette_sel::s_palette[] = {
	{ N_p("color-preset", "White"),  "FFFFFFFF" },
	{ N_p("color-preset", "Silver"), "FFC0C0C0" },
	{ N_p("color-preset", "Gray"),   "FF808080" },
	{ N_p("color-preset", "Black"),  "FF000000" },
	{ N_p("color-preset", "Red"),    "FFFF0000" },
	{ N_p("color-preset", "Orange"), "FFFFA500" },
	{ N_p("color-preset", "Yellow"), "FFFFFF00" },
	{ N_p("color-preset", "Green"),  "FF00FF00" },
	{ N_p("color-preset", "Blue"),   "FF0000FF" },
	{ N_p("color-preset", "Violet"), "FF8F00FF" }
};

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_palette_sel::menu_palette_sel(mame_ui_manager &mui, render_container &container, rgb_t &_color)
	: menu(mui, container), m_original(_color)
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_palette_sel::handle(event const *ev)
{
	// process the menu
	if (ev && ev->itemref)
	{
		if (ev->iptkey == IPT_UI_SELECT)
		{
			m_original = rgb_t(uint32_t(strtoul(selected_item().subtext().c_str(), nullptr, 16)));
			reset_parent(reset_options::REMEMBER_REF);
			stack_pop();
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_palette_sel::populate(float &customtop, float &custombottom)
{
	for (unsigned x = 0; x < std::size(s_palette); ++x)
		item_append(_("color-preset", s_palette[x].first), s_palette[x].second, FLAG_COLOR_BOX, (void *)(uintptr_t)(x + 1));

	item_append(menu_item_type::SEPARATOR);
}

} // namespace ui
