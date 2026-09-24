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
#include "path.h"

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

std::size_t palette_count()
{
	return ui_colors::predefined_palettes().size();
}

char const *palette_name(std::size_t index)
{
	return ui_colors::predefined_palettes()[index].name;
}

char const *palette_value(std::size_t index)
{
	return ui_colors::predefined_palettes()[index].id;
}

std::size_t palette_index(char const *palette)
{
	auto const &palettes(ui_colors::predefined_palettes());
	for (std::size_t index = 0; palettes.size() > index; index++)
		if (!strcmp(palette, palettes[index].id))
			return index;
	return 0; // Classic
}

std::vector<std::pair<char const *, rgb_t>> palette_tokens(ui_colors const &colors)
{
	std::vector<std::pair<char const *, rgb_t>> result;
	result.reserve(colors.palette_colors().size());
	for (ui_colors::palette_color const &color : colors.palette_colors())
		result.emplace_back(color.name, color.value);
	return result;
}

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

menu_custom_ui::menu_custom_ui(mame_ui_manager &mui, render_target &target, std::function<void ()> &&handler)
	: menu(mui, target)
	, m_handler(std::move(handler))
	, m_currlang(0)
	, m_currsysnames(0)
	, m_currpanels(ui().options().hide_panels())
{
	set_process_flags(PROCESS_LR_REPEAT);
	set_heading(_("Customize UI"));

	find_languages();
	find_sysnames();
}

//-------------------------------------------------
//  menu dismissed
//-------------------------------------------------

void menu_custom_ui::menu_dismissed()
{
	ui().options().set_value(OPTION_HIDE_PANELS, m_currpanels, OPTION_PRIORITY_CMDLINE);

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

bool menu_custom_ui::handle(event const *ev)
{
	if (!ev || !ev->itemref)
		return false;

	switch ((uintptr_t)ev->itemref)
	{
	case FONT_MENU:
		if (ev->iptkey == IPT_UI_SELECT)
			menu::stack_push<menu_font_ui>(ui(), target(), nullptr);
		break;
	case COLORS_MENU:
		if (ev->iptkey == IPT_UI_SELECT)
			menu::stack_push<menu_colors_ui>(ui(), target());
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
			return true;
		}
		else if (ev->iptkey == IPT_UI_SELECT)
		{
			// copying list of language names - expensive
			menu::stack_push<menu_selector>(
					ui(), target(), _("UI Language"), std::vector<std::string>(m_languages), m_currlang,
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
			return true;
		}
		else if (ev->iptkey == IPT_UI_SELECT)
		{
			// copying list of file names - expensive
			menu::stack_push<menu_selector>(
					ui(), target(), _("System Names"), std::vector<std::string>(m_sysnames), m_currsysnames,
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
				--m_currpanels;
			else if (ev->iptkey == IPT_UI_RIGHT)
				++m_currpanels;
			else
				m_currpanels = 0;
			ev->item->set_subtext(_(HIDE_STATUS[m_currpanels]));
			ev->item->set_flags(get_arrow_flags<uint16_t>(0, HIDE_BOTH, m_currpanels));
			return true;
		}
		else if (ev->iptkey == IPT_UI_SELECT)
		{
			std::vector<std::string> s_sel(std::size(HIDE_STATUS));
			std::transform(std::begin(HIDE_STATUS), std::end(HIDE_STATUS), s_sel.begin(), [](auto &s) { return _(s); });
			menu::stack_push<menu_selector>(
					ui(), target(), _("Show Side Panels"), std::move(s_sel), m_currpanels,
					[this, item = ev->item] (int selection)
					{
						m_currpanels = selection;
						item->set_subtext(_(HIDE_STATUS[selection]));
						item->set_flags(get_arrow_flags<uint16_t>(0, HIDE_BOTH, selection));
					});
		}
		break;
	}

	return false;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_custom_ui::populate()
{
	uint32_t arrow_flags;
	item_append(_("Fonts"), 0, (void *)(uintptr_t)FONT_MENU);
	item_append(_("Colors"), 0, (void *)(uintptr_t)COLORS_MENU);

	arrow_flags = get_arrow_flags<std::size_t>(0, m_languages.size() - 1, m_currlang);
	item_append(_("Language"), m_languages[m_currlang], arrow_flags, (void *)(uintptr_t)LANGUAGE_MENU);

	arrow_flags = get_arrow_flags<std::size_t>(0, m_sysnames.size() - 1, m_currsysnames);
	item_append(_("System Names"), m_sysnames[m_currsysnames], arrow_flags, (void *)(uintptr_t)SYSNAMES_MENU);

	arrow_flags = get_arrow_flags<uint16_t>(0, HIDE_BOTH, m_currpanels);
	item_append(_("Show Side Panels"), _(HIDE_STATUS[m_currpanels]), arrow_flags, (void *)(uintptr_t)HIDE_MENU);

	item_append(menu_item_type::SEPARATOR);
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
			[] (std::string const &x, std::string const &y) { return 0 > core_stricmp(x, y); });

	char const *const lang = machine().options().language();
	if (*lang)
	{
		auto const found = std::lower_bound(
				std::next(m_languages.begin()),
				m_languages.end(),
				lang,
				[] (std::string const &x, char const *y) { return 0 > core_stricmp(x, y); });
		if ((m_languages.end() != found) && !core_stricmp(*found, lang))
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
			[] (std::string const &x, std::string const &y) { return 0 > core_stricmp(x, y); });

	char const *const names = ui().options().system_names();
	if (*names)
	{
		auto const found = std::lower_bound(
				std::next(m_sysnames.begin()),
				m_sysnames.end(),
				names,
				[] (std::string const &x, char const *y) { return 0 > core_stricmp(x, y); });
		m_currsysnames = std::distance(m_sysnames.begin(), found);
		if ((m_sysnames.end() == found) || core_stricmp(*found, names))
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

menu_font_ui::menu_font_ui(mame_ui_manager &mui, render_target &target, std::function<void (bool)> &&handler)
	: menu(mui, target)
	, m_handler(std::move(handler))
	, m_fonts()
	, m_font_min(parse_number<int>(mui.options().get_entry(OPTION_FONT_ROWS)->minimum()))
	, m_font_max(parse_number<int>(mui.options().get_entry(OPTION_FONT_ROWS)->maximum()))
	, m_font_size(mui.options().font_rows())
	, m_info_min(parse_number<float>(mui.options().get_entry(OPTION_INFOS_SIZE)->minimum()))
	, m_info_max(parse_number<float>(mui.options().get_entry(OPTION_INFOS_SIZE)->maximum()))
	, m_info_size(mui.options().infos_size())
	, m_face_changed(false)
	, m_changed(false)
	, m_actual(0U)
{
	set_process_flags(PROCESS_LR_REPEAT);
	set_heading(_("UI Fonts"));

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

		if (m_face_changed)
		{
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
		}
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

bool menu_font_ui::handle(event const *ev)
{
	if (!ev || !ev->itemref)
		return false;

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
			return true;
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
			return true;
		}
		break;

	case MUI_FNT:
		if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT) || (ev->iptkey == IPT_UI_CLEAR))
		{
			m_face_changed = true;
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
					ui(), target(), _("UI Font"), std::move(display_names), m_actual,
					[this] (int selection)
					{
						m_face_changed = true;
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
			m_face_changed = true;
			m_changed = true;
			bool &val = ((uintptr_t)ev->itemref == MUI_BOLD) ? m_bold : m_italic;
			if (ev->iptkey == IPT_UI_CLEAR)
				val = false;
			else
				val = !val;
			ev->item->set_subtext(val ? _("On") : _("Off"));
			ev->item->set_color_state(val ? menu_item_color_state::ON : menu_item_color_state::OFF);
			ev->item->set_flags(val ? FLAG_LEFT_ARROW : FLAG_RIGHT_ARROW);
			return true;
		}
		break;
#endif
	}

	return false;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_font_ui::populate()
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
}

//-------------------------------------------------
//  recompute metrics
//-------------------------------------------------

void menu_font_ui::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	set_custom_space(0.0f, line_height() + 3.0f * tb_border());
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_font_ui::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	if (uintptr_t(selectedref) == INFOS_SIZE)
	{
		char const *const bottomtext[] = { _("Sample text - Lorem ipsum dolor sit amet, consectetur adipiscing elit.") };
		draw_text_box(
				std::begin(bottomtext), std::end(bottomtext),
				origx1, origx2, origy2 + tb_border(), origy2 + bottom,
				text_layout::text_justify::LEFT, text_layout::word_wrapping::NEVER, false,
				ui().colors().colored_text_color(), ui().colors().accent_color(), ui().get_line_height(target(), m_info_size));
	}
}

//-------------------------------------------------
//  ctor
//-------------------------------------------------
#define SET_COLOR_UI(var, opt) var[M##opt].color = mui.options().rgb_value(OPTION_##opt); var[M##opt].option = OPTION_##opt

menu_colors_ui::menu_colors_ui(mame_ui_manager &mui, render_target &target)
	: menu(mui, target)
	, m_color_table{}
	, m_palette(palette_index(mui.options().palette()))
{
	set_process_flags(PROCESS_LR_REPEAT);
	set_heading(_("UI Colors"));

	// the editor draft always holds the persisted custom slots
	SET_COLOR_UI(m_color_table, UI_TEXT_COLOR);
	SET_COLOR_UI(m_color_table, UI_TEXT_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_SELECTED_COLOR);
	SET_COLOR_UI(m_color_table, UI_SELECTED_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_SUBITEM_COLOR);
	SET_COLOR_UI(m_color_table, UI_CLONE_COLOR);
	SET_COLOR_UI(m_color_table, UI_BORDER_COLOR);
	SET_COLOR_UI(m_color_table, UI_BACKGROUND_COLOR);
	SET_COLOR_UI(m_color_table, UI_DIPSW_COLOR);
	SET_COLOR_UI(m_color_table, UI_UNAVAILABLE_COLOR);
	SET_COLOR_UI(m_color_table, UI_SLIDER_COLOR);
	SET_COLOR_UI(m_color_table, UI_GFXVIEWER_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_MOUSEOVER_COLOR);
	SET_COLOR_UI(m_color_table, UI_MOUSEOVER_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_MOUSEDOWN_COLOR);
	SET_COLOR_UI(m_color_table, UI_MOUSEDOWN_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_CONFIG_DEEMPHASIZED_COLOR);
	SET_COLOR_UI(m_color_table, UI_ACCENT_COLOR);
	SET_COLOR_UI(m_color_table, UI_STATUS_GOOD_COLOR);
	SET_COLOR_UI(m_color_table, UI_STATUS_WARNING_COLOR);
	SET_COLOR_UI(m_color_table, UI_STATUS_ERROR_COLOR);
	SET_COLOR_UI(m_color_table, UI_FOCUS_COLOR);
	SET_COLOR_UI(m_color_table, UI_FOCUS_BG_COLOR);
	SET_COLOR_UI(m_color_table, UI_FOCUS_OUTLINE_COLOR);
	SET_COLOR_UI(m_color_table, UI_FOCUS_GRADIENT_TOP);
	SET_COLOR_UI(m_color_table, UI_FOCUS_GRADIENT_BOTTOM);
	SET_COLOR_UI(m_color_table, UI_OVERLAY_COLOR);
}

//-------------------------------------------------
//  menu dismissed
//-------------------------------------------------

void menu_colors_ui::menu_dismissed()
{
	write_color_options();
	ui().options().set_value(OPTION_UI_PALETTE, palette_value(m_palette), OPTION_PRIORITY_CMDLINE);

	// refresh our cached colors
	ui().colors().refresh(ui().options());
}

//-------------------------------------------------
//  select a palette, copy its roles into the editor
//  and keep those roles independently editable
//-------------------------------------------------

void menu_colors_ui::set_palette(std::size_t index)
{
	m_palette = index;
	ui_colors selected;
	selected.load_palette(palette_value(index));
	copy_effective(selected, m_color_table);
	write_color_options();
	ui().options().set_value(OPTION_UI_PALETTE, palette_value(index), OPTION_PRIORITY_CMDLINE);
	ui().colors().refresh(ui().options());
}

//-------------------------------------------------
//  write the editor values into the color options
//-------------------------------------------------

void menu_colors_ui::write_color_options()
{
	std::string dec_color;
	for (int index = MUI_TEXT_COLOR; index < MUI_RESET; index++)
	{
		dec_color = string_format("%x", (uint32_t)m_color_table[index].color);
		ui().options().set_value(m_color_table[index].option, dec_color, OPTION_PRIORITY_CMDLINE);
	}
}

//-------------------------------------------------
//  palette colors customized
//-------------------------------------------------

bool menu_colors_ui::palette_colors_customized() const
{
	ui_colors selected;
	selected.load_palette(palette_value(m_palette));
	s_color_table palette[MUI_RESET]{};
	copy_effective(selected, palette);

	for (int index = MUI_TEXT_COLOR; index < MUI_RESET; index++)
		if (m_color_table[index].color != palette[index].color)
			return true;

	return false;
}

//-------------------------------------------------
//  copy an effective palette into a color table
//-------------------------------------------------

void menu_colors_ui::copy_effective(ui_colors const &colors, s_color_table *table) const
{
	table[MUI_TEXT_COLOR].color = colors.text_color();
	table[MUI_TEXT_BG_COLOR].color = colors.text_bg_color();
	table[MUI_SELECTED_COLOR].color = colors.selected_color();
	table[MUI_SELECTED_BG_COLOR].color = colors.selected_bg_color();
	table[MUI_SUBITEM_COLOR].color = colors.subitem_color();
	table[MUI_CLONE_COLOR].color = colors.clone_color();
	table[MUI_BORDER_COLOR].color = colors.border_color();
	table[MUI_BACKGROUND_COLOR].color = colors.background_color();
	table[MUI_DIPSW_COLOR].color = colors.dipsw_color();
	table[MUI_UNAVAILABLE_COLOR].color = colors.unavailable_color();
	table[MUI_SLIDER_COLOR].color = colors.slider_color();
	table[MUI_GFXVIEWER_BG_COLOR].color = colors.gfxviewer_bg_color();
	table[MUI_MOUSEOVER_COLOR].color = colors.mouseover_color();
	table[MUI_MOUSEOVER_BG_COLOR].color = colors.mouseover_bg_color();
	table[MUI_MOUSEDOWN_COLOR].color = colors.mousedown_color();
	table[MUI_MOUSEDOWN_BG_COLOR].color = colors.mousedown_bg_color();
	table[MUI_CONFIG_DEEMPHASIZED_COLOR].color = colors.config_deemphasized_color();
	table[MUI_ACCENT_COLOR].color = colors.accent_color();
	table[MUI_STATUS_GOOD_COLOR].color = colors.status_good_color();
	table[MUI_STATUS_WARNING_COLOR].color = colors.status_warning_color();
	table[MUI_STATUS_ERROR_COLOR].color = colors.status_error_color();
	table[MUI_FOCUS_COLOR].color = colors.focus_color();
	table[MUI_FOCUS_BG_COLOR].color = colors.focus_bg_color();
	table[MUI_FOCUS_OUTLINE_COLOR].color = colors.focus_outline_color();
	table[MUI_FOCUS_GRADIENT_TOP].color = colors.focus_gradient_top();
	table[MUI_FOCUS_GRADIENT_BOTTOM].color = colors.focus_gradient_bottom();
	table[MUI_OVERLAY_COLOR].color = colors.overlay_color();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_colors_ui::handle(event const *ev)
{
	if (!ev || !ev->itemref)
		return false;

	uintptr_t const ref((uintptr_t)ev->itemref);

	if (MUI_PALETTE == ref)
	{
		if ((ev->iptkey == IPT_UI_LEFT) || (ev->iptkey == IPT_UI_RIGHT))
		{
			if ((IPT_UI_LEFT == ev->iptkey) && (0U < m_palette))
				set_palette(m_palette - 1);
			else if ((IPT_UI_RIGHT == ev->iptkey) && (m_palette + 1U < palette_count()))
				set_palette(m_palette + 1);
			ev->item->set_subtext(_(palette_name(m_palette)));
			ev->item->set_flags(get_arrow_flags<std::size_t>(0U, palette_count() - 1U, m_palette));
			reset(reset_options::REMEMBER_REF);
			return true;
		}
		else if (ev->iptkey == IPT_UI_SELECT)
		{
			std::vector<std::string> palettelist;
			palettelist.reserve(palette_count());
			for (std::size_t index = 0; palette_count() > index; index++)
				palettelist.emplace_back(_(palette_name(index)));
			menu::stack_push<menu_selector>(
					ui(), target(), _("Palette"), std::move(palettelist), int(m_palette),
					[this, item = ev->item] (int selection)
					{
						set_palette(selection);
						item->set_subtext(_(palette_name(selection)));
						item->set_flags(get_arrow_flags<std::size_t>(0U, palette_count() - 1U, selection));
						reset(reset_options::REMEMBER_REF);
					});
		}
	}
	else if (ev->iptkey == IPT_UI_SELECT)
	{
		if (MUI_RESET == ref)
		{
			// Restore the selected palette's role values without changing the palette.
			ui_colors selected;
			selected.load_palette(palette_value(m_palette));
			copy_effective(selected, m_color_table);
			write_color_options();
			ui().colors().refresh(ui().options());
			reset(reset_options::REMEMBER_REF);
			return true;
		}

		std::string title(selected_item().text());
		menu::stack_push<menu_rgb_ui>(ui(), target(), &m_color_table[ref].color, std::move(title));
	}

	return false;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_colors_ui::populate()
{

	uint32_t const arrow_flags(get_arrow_flags<std::size_t>(0U, palette_count() - 1U, m_palette));
	item_append(_("Palette"), std::string(_(palette_name(m_palette))), arrow_flags, (void *)(uintptr_t)MUI_PALETTE);
	item_append(menu_item_type::SEPARATOR);
	item_append(_("Main"), FLAG_DISABLE | FLAG_UI_HEADING, nullptr);
	item_append(_("color-option", "Surface Background"),         0, (void *)(uintptr_t)MUI_BACKGROUND_COLOR);
	item_append(_("color-option", "Graphics Viewer Background"), 0, (void *)(uintptr_t)MUI_GFXVIEWER_BG_COLOR);
	item_append(_("color-option", "Overlay"),                    0, (void *)(uintptr_t)MUI_OVERLAY_COLOR);
	item_append(_("color-option", "Border"),                     0, (void *)(uintptr_t)MUI_BORDER_COLOR);
	item_append(_("color-option", "Accent"),                     0, (void *)(uintptr_t)MUI_ACCENT_COLOR);
	item_append(_("color-option", "Normal Text Background"),     0, (void *)(uintptr_t)MUI_TEXT_BG_COLOR);
	item_append(_("color-option", "Normal Text Foreground"),     0, (void *)(uintptr_t)MUI_TEXT_COLOR);
	item_append(_("color-option", "Selected Text Background"),   0, (void *)(uintptr_t)MUI_SELECTED_BG_COLOR);
	item_append(_("color-option", "Selected Text Foreground"),   0, (void *)(uintptr_t)MUI_SELECTED_COLOR);
	item_append(_("color-option", "Clone Text (Selection)"),     0, (void *)(uintptr_t)MUI_CLONE_COLOR);
	item_append(_("color-option", "Mouse Over Background"),      0, (void *)(uintptr_t)MUI_MOUSEOVER_BG_COLOR);
	item_append(_("color-option", "Mouse Over Foreground"),      0, (void *)(uintptr_t)MUI_MOUSEOVER_COLOR);
	item_append(_("color-option", "Mouse Down Background"),      0, (void *)(uintptr_t)MUI_MOUSEDOWN_BG_COLOR);
	item_append(_("color-option", "Mouse Down Foreground"),      0, (void *)(uintptr_t)MUI_MOUSEDOWN_COLOR);
	item_append(_("color-option", "Focus Background"),           0, (void *)(uintptr_t)MUI_FOCUS_BG_COLOR);
	item_append(_("color-option", "Focus Foreground"),           0, (void *)(uintptr_t)MUI_FOCUS_COLOR);
	item_append(_("color-option", "Focus Outline"),              0, (void *)(uintptr_t)MUI_FOCUS_OUTLINE_COLOR);
	item_append(_("color-option", "Focus Gradient Top"),         0, (void *)(uintptr_t)MUI_FOCUS_GRADIENT_TOP);
	item_append(_("color-option", "Focus Gradient Bottom"),      0, (void *)(uintptr_t)MUI_FOCUS_GRADIENT_BOTTOM);
	item_append(_("color-option", "Status Good"),                0, (void *)(uintptr_t)MUI_STATUS_GOOD_COLOR);
	item_append(_("color-option", "Status Warning"),             0, (void *)(uintptr_t)MUI_STATUS_WARNING_COLOR);
	item_append(_("color-option", "Status Bad"),                 0, (void *)(uintptr_t)MUI_STATUS_ERROR_COLOR);

	item_append(_("Configuration"), FLAG_DISABLE | FLAG_UI_HEADING, nullptr);
	item_append(_("color-option", "Subitem Foreground"),         0, (void *)(uintptr_t)MUI_SUBITEM_COLOR);
	item_append(_("color-option", "Default/Inherited"),          0, (void *)(uintptr_t)MUI_CONFIG_DEEMPHASIZED_COLOR);
	item_append(_("color-option", "Unavailable"),                0, (void *)(uintptr_t)MUI_UNAVAILABLE_COLOR);
	item_append(_("color-option", "Slider"),                     0, (void *)(uintptr_t)MUI_SLIDER_COLOR);
	item_append(_("color-option", "DIP Switch"),                 0, (void *)(uintptr_t)MUI_DIPSW_COLOR);

	item_append(menu_item_type::SEPARATOR);

	item_append(_("Restore Palette Colors"), palette_colors_customized() ? 0U : FLAG_DISABLE, (void *)(uintptr_t)MUI_RESET);
}

//-------------------------------------------------
//  recompute metrics
//-------------------------------------------------

void menu_colors_ui::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	set_custom_space(0.0f, line_height() + 3.0f * tb_border());
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_colors_ui::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// get the text for 'UI Select'
	std::string const bottomtext[] = { util::string_format(_("Double-click or press %1$s to change color"), ui().get_general_input_setting(IPT_UI_SELECT)) };
	draw_text_box(
			std::begin(bottomtext), std::end(bottomtext),
			origx1, origx2, origy2 + tb_border(), origy2 + bottom,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
			ui().colors().text_color(), ui().colors().background_color());

	// Resolve editable colors from the editor draft so in-progress edits are
	// previewed live.
	s_color_table preview[MUI_COLOR_COUNT];
	std::copy(std::begin(m_color_table), std::end(m_color_table), std::begin(preview));

	// compute maxwidth
	char const *const topbuf = _("Preview");

	enum sample_id
	{
		SAMPLE_SUBITEM,
		SAMPLE_MOUSE_OVER,
		SAMPLE_MOUSE_DOWN,
		SAMPLE_CLONE,
		SAMPLE_DEFAULT,
		SAMPLE_UNAVAILABLE,
		SAMPLE_STATUS_GOOD,
		SAMPLE_STATUS_WARNING,
		SAMPLE_STATUS_BAD,
		SAMPLE_SLIDER,
		SAMPLE_DIP_SWITCH,
		SAMPLE_MAIN_MENU,
		SAMPLE_SELECTED_FOCUSED,
		SAMPLE_SELECTED_UNFOCUSED,
		SAMPLE_ON,
		SAMPLE_OFF,
		SAMPLE_AUTO,
		SAMPLE_COUNT
	};
	std::array<std::string, SAMPLE_COUNT> const sampletxt = {
			_("color-sample", "Subitem"),
			_("color-sample", "Mouse Over"),
			_("color-sample", "Mouse Down"),
			_("color-sample", "Clone"),
			_("color-sample", "Default"),
			_("color-sample", "Unavailable"),
			_("color-sample", "Good"),
			_("color-sample", "Warning"),
			_("color-sample", "Bad"),
			_("color-sample", "Slider"),
			_("color-sample", "DIP Switch"),
			_("color-sample", "Main"),
			_("color-sample", "Selected (focused)"),
			_("color-sample", "Selected (unfocused)"),
			_("color-sample", "On"),
			_("color-sample", "Off"),
			_("color-sample", "Auto"),
			};

	float width = get_string_width(topbuf);
	float labelwidth(0.0f);
	float maxwidth = width + 2.0f * lr_border();
	for (auto const &elem : sampletxt)
	{
		width = get_string_width(elem);
		labelwidth = std::max(labelwidth, width);
		maxwidth = std::max(maxwidth, width + 2.0f * lr_border());
	}
	maxwidth = std::max(maxwidth, 2.0f * labelwidth + 2.0f * lr_border());
	maxwidth = std::max(maxwidth, get_string_width(topbuf) + 2.0f * lr_border());
	maxwidth = std::max(maxwidth, get_string_width(_("Configuration")) + 2.0f * lr_border());

	// The preview is drawn to the right of the menu. Keep its outer box
	// inside the normalized target even when localized labels grow wider.
	float const preview_x1(origx2 + 2.0f * lr_border());
	float const preview_max_width(1.0f - preview_x1 - lr_border());
	if (preview_max_width <= 0.0f)
		return;
	maxwidth = std::min(maxwidth, preview_max_width);

	// Draw the active palette to the left of the menu. The two-column layout
	// keeps all sixteen swatches within the available vertical space.
	auto const &palette(ui().colors().palette_colors());
	float palette_label_width(0.0f);
	for (ui_colors::palette_color const &color : palette)
		palette_label_width = std::max(palette_label_width, get_string_width(_(color.name)));
	float const palette_gap(lr_border());
	float const swatch_width(1.25f * line_height() * x_aspect());
	float const palette_cell_width(swatch_width + palette_gap + palette_label_width + 2.0f * lr_border());
	float palette_width(2.0f * palette_cell_width + 2.0f * lr_border());
	float const palette_x2(origx1 - 2.0f * lr_border());
	float const palette_max_width(palette_x2 - lr_border());
	if (palette_max_width > 0.0f)
	{
		palette_width = std::min(palette_width, palette_max_width);
		float const palette_x1(palette_x2 - palette_width);
		float const palette_y1(origy1);
		float const palette_y2(palette_y1 + 9.0f * line_height() + 5.0f * tb_border());
		ui().draw_outlined_box(
				container(), palette_x1, palette_y1, palette_x2, palette_y1 + tb_border() + line_height(),
				preview[MUI_BORDER_COLOR].color, preview[MUI_GFXVIEWER_BG_COLOR].color);
		ui().draw_outlined_box(
				container(), palette_x1, palette_y1 + tb_border() + line_height() + 2.0f * tb_border(), palette_x2, palette_y2,
				preview[MUI_BORDER_COLOR].color, preview[MUI_GFXVIEWER_BG_COLOR].color);

		float const inner_x1(palette_x1 + lr_border());
		float const inner_x2(palette_x2 - lr_border());
		float const inner_width((inner_x2 - inner_x1) * 0.5f);
		float const actual_swatch_width(std::min(swatch_width, inner_width * 0.35f));
		draw_text_normal(
				_("Palette"), inner_x1, palette_y1 + (0.5f * tb_border()), inner_x2 - inner_x1,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				preview[MUI_TEXT_COLOR].color);


		std::array<std::size_t, 16> palette_order{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		if (m_palette == palette_index("classic"))
			palette_order = { 0, 2, 1, 3, 7, 4, 12, 5, 8, 9, 10, 11, 6, 13, 14, 15 };

		for (std::size_t display_index = 0; palette.size() > display_index; ++display_index)
		{
			std::size_t const index(palette_order[display_index]);
			std::size_t const column(display_index / 8);
			std::size_t const row(display_index % 8);
			float const cell_x1(inner_x1 + float(column) * inner_width);
			float const cell_x2(cell_x1 + inner_width);
			float const cell_y(palette_y1 + tb_border() + line_height() + 3.0f * tb_border() + float(row) * line_height());
			float const swatch_x2(cell_x1 + actual_swatch_width);
			container().add_rect(
					cell_x1, cell_y, swatch_x2, cell_y + line_height(),
					palette[index].value, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			ui().draw_text_full(
					target(), _(palette[index].name), swatch_x2 + palette_gap, cell_y,
					std::max(0.0f, cell_x2 - swatch_x2 - palette_gap),
					text_layout::text_justify::LEFT, text_layout::word_wrapping::TRUNCATE,
					mame_ui_manager::NORMAL, preview[MUI_TEXT_COLOR].color, preview[MUI_GFXVIEWER_BG_COLOR].color,
					nullptr, nullptr, line_height());
		}
	}

	// compute our bounds for header
	float x1 = origx2 + 2.0f * lr_border();
	float x2 = x1 + maxwidth;
	float y1 = origy1;
	float y2 = y1 + bottom - tb_border();

	// draw a box
	ui().draw_outlined_box(
			container(), x1, y1, x2, y2,
			preview[MUI_BORDER_COLOR].color, preview[MUI_BACKGROUND_COLOR].color);

	// take off the borders
	x1 += lr_border();
	x2 -= lr_border();
	y1 += tb_border();
	y2 -= tb_border();

	// draw the text within it
	draw_text_normal(
			topbuf,
			x1, y1, x2 - x1,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
			ui().colors().colored_text_color());

	// compute our bounds for the grouped samples
	x1 -= lr_border();
	x2 += lr_border();
	y1 = y2 + 2.0f * tb_border();

	float const ix1(x1 + lr_border());
	float const ix2(x2 - lr_border());
	float const mid((ix1 + ix2) * 0.5f);
	float const third((ix2 - ix1) / 3.0f);
	float const gap(lr_border() * 0.5f);
	constexpr float main_preview_lines(6.0f);
	constexpr float selection_tooltip_preview_lines(6.6f);
	float const main_row_spacing(tb_border());
	float const preview_panel_y1(y1);
	float const main_section_y1(preview_panel_y1 + tb_border());
	float const main_section_y2(main_section_y1 + main_preview_lines * line_height() + 3.0f * main_row_spacing);
	float const configuration_section_y1(main_section_y2 + 4.0f * tb_border());
	float const configuration_section_y2(configuration_section_y1 + tb_border() + selection_tooltip_preview_lines * line_height() + 2.0f * main_row_spacing);
	float const preview_panel_y2(configuration_section_y2 + 2.0f * tb_border());
	ui().draw_outlined_box(
			container(), x1, preview_panel_y1, x2, preview_panel_y2,
			ui().colors().background_color());
	float const section_x1(x1 + lr_border());
	float const section_x2(x2 - lr_border());
	container().add_rect(section_x1, main_section_y1, section_x2, main_section_y2, preview[MUI_BACKGROUND_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container().add_rect(section_x1, configuration_section_y1, section_x2, configuration_section_y2, preview[MUI_BACKGROUND_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	auto const sample = [this] (std::string const &text, float cx1, float cx2, float cy, rgb_t fgcolor, rgb_t bgcolor, bool highlighted)
	{
		if (highlighted)
			highlight(cx1, cy, cx2, cy + line_height(), bgcolor);
		ui().draw_text_full(
				target(),
				text,
				cx1, cy, cx2 - cx1,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				mame_ui_manager::NORMAL, fgcolor, bgcolor,
				nullptr, nullptr,
				line_height());
	};

	auto const clone_sample = [this, &preview] (std::string const &text, float cx1, float cx2, float cy, rgb_t fgcolor)
	{
		ui().draw_text_full(
				target(), text, cx1, cy, cx2 - cx1,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				mame_ui_manager::NORMAL, fgcolor, preview[MUI_BACKGROUND_COLOR].color,
				nullptr, nullptr, line_height());
	};

	auto const status_sample = [this] (std::string const &text, float cx1, float cx2, float cy, rgb_t bgcolor)
	{
		container().add_rect(cx1, cy, cx2, cy + line_height(), bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		ui().draw_text_full(
				target(), text, cx1, cy, cx2 - cx1,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				mame_ui_manager::NORMAL, ui().colors().colored_text_color(), rgb_t::transparent(),
				nullptr, nullptr, line_height());
	};

	auto const slider_sample = [this, &preview] (std::string const &text, float cx1, float cx2, float cy)
	{
		float const available_width(std::max(0.0f, cx2 - cx1));
		float const control_x2(std::max(cx1, cx2 - lr_border()));
		float const label_width(std::min(available_width * 0.3f, std::max(0.0f, control_x2 - cx1 - lr_border())));
		float const control_x1(std::min(cx1 + label_width + lr_border(), control_x2));
		float const bar_top(cy + 0.2f * line_height());
		float const bar_bottom(cy + 0.8f * line_height());
		float const current_x(control_x1 + 0.7f * (control_x2 - control_x1));
		float const default_x(control_x1 + 0.5f * (control_x2 - control_x1));
		ui().draw_text_full(
				target(), text, cx1, cy, label_width,
				text_layout::text_justify::LEFT, text_layout::word_wrapping::TRUNCATE,
				mame_ui_manager::NORMAL, preview[MUI_TEXT_COLOR].color, rgb_t::transparent(),
				nullptr, nullptr, line_height());
		container().add_rect(control_x1, bar_top, current_x, bar_bottom, preview[MUI_SLIDER_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(control_x1, bar_top, control_x2, bar_top, UI_LINE_WIDTH, preview[MUI_BORDER_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(control_x1, bar_bottom, control_x2, bar_bottom, UI_LINE_WIDTH, preview[MUI_BORDER_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(default_x, cy, default_x, bar_top, UI_LINE_WIDTH, preview[MUI_BORDER_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(default_x, bar_bottom, default_x, cy + line_height(), UI_LINE_WIDTH, preview[MUI_BORDER_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	};

	auto const dipsw_sample = [this, &preview] (std::string const &text, float cx1, float cx2, float cy)
	{
		constexpr int switch_count(4);
		constexpr float dip_switch_height = 1.6f;
		constexpr float single_toggle_field_width = dip_switch_height / 2.0f;
		constexpr float single_toggle_width = single_toggle_field_width * 0.8f;
		constexpr float single_toggle_height = (dip_switch_height / 2.0f) * 0.8f;
		float const line_height_value(line_height());
		float const single_width(line_height_value * single_toggle_field_width * x_aspect());
		float const nub_width(line_height_value * single_toggle_width * x_aspect());
		float const nub_height(line_height_value * single_toggle_height);
		float const label_width((cx2 - cx1) * 0.3f);
		float const control_x1(cx1 + label_width + lr_border());
		float const control_x2(cx2 - lr_border());
		float const switch_width(switch_count * single_width);
		float const switch_x1(control_x1 + 0.5f * (control_x2 - control_x1 - switch_width));
		float const switch_x2(switch_x1 + switch_width);
		float const y1(cy);
		float const y2(cy + dip_switch_height * line_height_value);
		float const ygap(line_height_value * ((dip_switch_height * 0.5f) - single_toggle_height) * 0.5f);
		float const xgap((single_width + (UI_LINE_WIDTH * 0.5f) - nub_width) * 0.5f);

		ui().draw_text_full(
				target(), text, cx1, cy + line_height_value * (dip_switch_height - 1.0f) * 0.5f, label_width,
				text_layout::text_justify::LEFT, text_layout::word_wrapping::NEVER,
				mame_ui_manager::NORMAL, preview[MUI_TEXT_COLOR].color, rgb_t::transparent(),
				nullptr, nullptr, line_height_value);
		ui().draw_outlined_box(container(), switch_x1, y1, switch_x2, y2, preview[MUI_BORDER_COLOR].color, preview[MUI_BACKGROUND_COLOR].color);
		for (int toggle = 1; switch_count > toggle; ++toggle)
			container().add_line(
					switch_x1 + toggle * single_width, y1, switch_x1 + toggle * single_width, y2,
					UI_LINE_WIDTH, preview[MUI_TEXT_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		for (int toggle = 0; switch_count > toggle; ++toggle)
		{
			float const nub_x(switch_x1 + toggle * single_width + xgap);
			float const nub_y((toggle & 1) ? y2 - UI_LINE_WIDTH - ygap - nub_height : y1 + UI_LINE_WIDTH + ygap);
			container().add_rect(
					nub_x, nub_y, nub_x + nub_width, nub_y + nub_height,
					(toggle & 1) ? preview[MUI_DIPSW_COLOR].color : preview[MUI_TEXT_COLOR].color,
					PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
	};

	auto const category_sample = [this, &preview] (std::string const &text, float cx1, float cx2, float cy)
	{
		float const heading_width(get_string_width(text));
		float const line_y(cy + 0.5f * line_height());
		float const line_width((cx2 - cx1 - heading_width) * 0.5f);
		container().add_line(cx1, line_y, cx1 + line_width - lr_border(), line_y, UI_LINE_WIDTH, preview[MUI_BORDER_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(cx2 - line_width + lr_border(), line_y, cx2, line_y, UI_LINE_WIDTH, preview[MUI_BORDER_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		ui().draw_text_full(
				target(), text, cx1, cy, cx2 - cx1,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				mame_ui_manager::NORMAL, preview[MUI_TEXT_COLOR].color, preview[MUI_BACKGROUND_COLOR].color,
				nullptr, nullptr, line_height());
	};

	auto const focus_sample = [this, &preview] (std::string const &text, float cx1, float cx2, float cy, rgb_t fgcolor)
	{
		constexpr int gradient_steps(128); // widgets_manager::generate_hilight_main resolution
		int const gradient_denominator(gradient_steps - 1);
		rgb_t const topcolor(preview[MUI_FOCUS_GRADIENT_TOP].color);
		rgb_t const bottomcolor(preview[MUI_FOCUS_GRADIENT_BOTTOM].color);
		float const step_height(line_height() / gradient_steps);
		for (int step = 0; gradient_steps > step; ++step)
		{
			u8 const alpha(rgb_t::clamp(int(topcolor.a()) + (int(bottomcolor.a()) - int(topcolor.a())) * step / gradient_denominator));
			u8 const red(rgb_t::clamp(int(topcolor.r()) + (int(bottomcolor.r()) - int(topcolor.r())) * step / gradient_denominator));
			u8 const green(rgb_t::clamp(int(topcolor.g()) + (int(bottomcolor.g()) - int(topcolor.g())) * step / gradient_denominator));
			u8 const blue(rgb_t::clamp(int(topcolor.b()) + (int(bottomcolor.b()) - int(topcolor.b())) * step / gradient_denominator));
			container().add_rect(cx1, cy + step * step_height, cx2, cy + (step + 1) * step_height,
					rgb_t(alpha, red, green, blue), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
		container().add_line(cx1, cy, cx2, cy, UI_LINE_WIDTH, preview[MUI_FOCUS_OUTLINE_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(cx1, cy + line_height(), cx2, cy + line_height(), UI_LINE_WIDTH, preview[MUI_FOCUS_OUTLINE_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(cx1, cy, cx1, cy + line_height(), UI_LINE_WIDTH, preview[MUI_FOCUS_OUTLINE_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(cx2, cy, cx2, cy + line_height(), UI_LINE_WIDTH, preview[MUI_FOCUS_OUTLINE_COLOR].color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		ui().draw_text_full(
				target(), text, cx1, cy, cx2 - cx1,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				mame_ui_manager::NORMAL, fgcolor, rgb_t::transparent(),
				nullptr, nullptr, line_height());
	};

	float cy(y1 + tb_border());
	auto const textured_sample = [this] (std::string const &text, float cx1, float cx2, float cy, rgb_t fgcolor, rgb_t bgcolor, rgb_t bordercolor)
	{
		ui().draw_textured_box(
				container(), cx1, cy, cx2, cy + line_height(), bgcolor, bordercolor,
				hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		ui().draw_text_full(
				target(), text, cx1, cy, cx2 - cx1,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
				mame_ui_manager::NORMAL, fgcolor, bgcolor,
				nullptr, nullptr, line_height());
	};

	category_sample(sampletxt[SAMPLE_MAIN_MENU], ix1, ix2, cy);
	cy += line_height();
	clone_sample(sampletxt[SAMPLE_CLONE], ix1, ix2, cy, preview[MUI_CLONE_COLOR].color);
	cy += line_height();

	focus_sample(sampletxt[SAMPLE_SELECTED_FOCUSED], ix1, ix2, cy, preview[MUI_FOCUS_COLOR].color);
	cy += line_height() + main_row_spacing;
	textured_sample(sampletxt[SAMPLE_SELECTED_UNFOCUSED], ix1, ix2, cy, preview[MUI_MOUSEOVER_COLOR].color, preview[MUI_MOUSEOVER_BG_COLOR].color, preview[MUI_FOCUS_OUTLINE_COLOR].color);
	cy += line_height() + main_row_spacing;

	sample(sampletxt[SAMPLE_MOUSE_OVER], ix1, mid - gap, cy, preview[MUI_MOUSEOVER_COLOR].color, preview[MUI_MOUSEOVER_BG_COLOR].color, true);
	sample(sampletxt[SAMPLE_MOUSE_DOWN], mid + gap, ix2, cy, preview[MUI_MOUSEDOWN_COLOR].color, preview[MUI_MOUSEDOWN_BG_COLOR].color, true);
	cy += line_height() + main_row_spacing;

	status_sample(sampletxt[SAMPLE_STATUS_GOOD], ix1, ix1 + third, cy, preview[MUI_STATUS_GOOD_COLOR].color);
	status_sample(sampletxt[SAMPLE_STATUS_WARNING], ix1 + third, ix2 - third, cy, preview[MUI_STATUS_WARNING_COLOR].color);
	status_sample(sampletxt[SAMPLE_STATUS_BAD], ix2 - third, ix2, cy, preview[MUI_STATUS_ERROR_COLOR].color);
	cy += line_height();
	cy = configuration_section_y1 + tb_border();
	category_sample(_("Configuration"), ix1, ix2, cy);
	cy += line_height();

	sample(sampletxt[SAMPLE_SUBITEM], ix1, ix2, cy, preview[MUI_SUBITEM_COLOR].color, preview[MUI_TEXT_BG_COLOR].color, false);
	cy += line_height();
	sample(sampletxt[SAMPLE_DEFAULT], ix1, mid - gap, cy, preview[MUI_CONFIG_DEEMPHASIZED_COLOR].color, preview[MUI_TEXT_BG_COLOR].color, false);
	sample(sampletxt[SAMPLE_UNAVAILABLE], mid + gap, ix2, cy, preview[MUI_UNAVAILABLE_COLOR].color, preview[MUI_TEXT_BG_COLOR].color, false);
	cy += line_height();

	sample(sampletxt[SAMPLE_ON], ix1, ix1 + third, cy, preview[MUI_STATUS_GOOD_COLOR].color, preview[MUI_TEXT_BG_COLOR].color, false);
	sample(sampletxt[SAMPLE_OFF], ix1 + third, ix2 - third, cy, preview[MUI_STATUS_ERROR_COLOR].color, preview[MUI_TEXT_BG_COLOR].color, false);
	sample(sampletxt[SAMPLE_AUTO], ix2 - third, ix2, cy, preview[MUI_STATUS_WARNING_COLOR].color, preview[MUI_TEXT_BG_COLOR].color, false);
	cy += line_height() + main_row_spacing;

	slider_sample(sampletxt[SAMPLE_SLIDER], ix1, ix2, cy);
	cy += line_height() + main_row_spacing;
	dipsw_sample(sampletxt[SAMPLE_DIP_SWITCH], ix1, ix2, cy);

}

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_rgb_ui::menu_rgb_ui(mame_ui_manager &mui, render_target &target, rgb_t *color, std::string &&title)
	: menu(mui, target)
	, m_color(color)
	, m_search()
	, m_key_active(false)
	, m_lock_ref(0)
{
	set_process_flags(PROCESS_LR_REPEAT);
	set_heading(std::move(title));
}

//-------------------------------------------------
//  menu dismissed
//-------------------------------------------------

void menu_rgb_ui::menu_dismissed()
{
	reset_parent(reset_options::REMEMBER_REF);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_rgb_ui::handle(event const *ev)
{
	if (!ev || !ev->itemref)
		return false;

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
				return true;
			}
		}
		break;

	case IPT_UI_SELECT:
		if (uintptr_t(ev->itemref) == PALETTE_CHOOSE)
		{
			menu::stack_push<menu_palette_sel>(ui(), target(), *m_color, palette_tokens(ui().colors()));
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
			return inkey_special(ev);
		}
		break;
	}

	return false;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_rgb_ui::populate()
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
}

//-------------------------------------------------
//  recompute metrics
//-------------------------------------------------

void menu_rgb_ui::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	set_custom_space(0.0f, line_height() + 3.0f * tb_border());
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_rgb_ui::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float maxwidth = origx2 - origx1;

	std::string sampletxt(_("Color preview:"));
	float width = get_string_width(sampletxt);
	width += 2 * lr_border();
	maxwidth = std::max(origx2 - origx1, width);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy2 + tb_border();
	float y2 = origy2 + bottom;

	// draw a box - force black to ensure the text is legible
	ui().draw_outlined_box(container(), x1, y1, x2, y2, rgb_t::black());

	// take off the borders
	x1 += lr_border();
	y1 += tb_border();

	// draw the text label - force white to ensure it's legible
	ui().draw_text_full(
			target(),
			sampletxt,
			x1, y1, width - lr_border(),
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
			mame_ui_manager::NORMAL, rgb_t::white(), rgb_t::black(),
			nullptr, nullptr,
			line_height());

	x1 += width + (lr_border() * 2.0f);
	x2 -= lr_border();
	y2 -= tb_border();

	// add white under half the sample swatch to make alpha effects visible
	container().add_rect((x1 + x2) * 0.5f, y1, x2, y2, rgb_t::white(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	container().add_rect(x1, y1, x2, y2, *m_color, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}

//-------------------------------------------------
//  handle special key event
//-------------------------------------------------

bool menu_rgb_ui::inkey_special(const event *menu_event)
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
		return true;
	}
	else if (m_key_active && input_character(m_search, 3, menu_event->unichar, uchar_is_digit))
	{
		menu_event->item->set_subtext(m_search + "_");
		return true;
	}
	else
	{
		return false;
	}
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_palette_sel::menu_palette_sel(mame_ui_manager &mui, render_target &target, rgb_t &_color, std::vector<palette_entry> &&palette)
	: menu(mui, target)
	, m_original(_color)
	, m_palette(std::move(palette))
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_palette_sel::handle(event const *ev)
{
	if (ev && ev->itemref)
	{
		if (ev->iptkey == IPT_UI_SELECT)
		{
			std::size_t const index(uintptr_t(ev->itemref) - 1U);
			if (m_palette.size() > index)
				m_original = m_palette[index].second;
			reset_parent(reset_options::REMEMBER_REF);
			stack_pop();
		}
	}

	return false;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_palette_sel::populate()
{
	for (std::size_t x = 0; m_palette.size() > x; ++x)
		item_append(_(m_palette[x].first), string_format("%08x", uint32_t(m_palette[x].second)), FLAG_COLOR_BOX, (void *)(uintptr_t)(x + 1));

	item_append(menu_item_type::SEPARATOR);
}

} // namespace ui
