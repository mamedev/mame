// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Vas Crabb
/***************************************************************************

    ui/selmenu.cpp

    MAME system/software selection menu.

***************************************************************************/

#include "emu.h"
#include "ui/selmenu.h"

#include "ui/datmenu.h"
#include "ui/info.h"
#include "ui/inifile.h"

// these hold static bitmap images
#include "ui/defimg.ipp"
#include "ui/starimg.ipp"
#include "ui/toolbar.ipp"

#include "cheat.h"
#include "mame.h"
#include "mameopts.h"

#include "drivenum.h"
#include "emuopts.h"
#include "rendfont.h"
#include "rendutil.h"
#include "romload.h"
#include "softlist.h"
#include "softlist_dev.h"
#include "uiinput.h"
#include "luaengine.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <utility>


namespace ui {

namespace {

enum
{
	FIRST_VIEW = 0,
	SNAPSHOT_VIEW = FIRST_VIEW,
	CABINETS_VIEW,
	CPANELS_VIEW,
	PCBS_VIEW,
	FLYERS_VIEW,
	TITLES_VIEW,
	ENDS_VIEW,
	ARTPREV_VIEW,
	BOSSES_VIEW,
	LOGOS_VIEW,
	VERSUS_VIEW,
	GAMEOVER_VIEW,
	HOWTO_VIEW,
	SCORES_VIEW,
	SELECT_VIEW,
	MARQUEES_VIEW,
	COVERS_VIEW,
	LAST_VIEW = COVERS_VIEW
};

std::pair<char const *, char const *> const arts_info[] =
{
	{ __("Snapshots"),       OPTION_SNAPSHOT_DIRECTORY },
	{ __("Cabinets"),        OPTION_CABINETS_PATH },
	{ __("Control Panels"),  OPTION_CPANELS_PATH },
	{ __("PCBs"),            OPTION_PCBS_PATH },
	{ __("Flyers"),          OPTION_FLYERS_PATH },
	{ __("Titles"),          OPTION_TITLES_PATH },
	{ __("Ends"),            OPTION_ENDS_PATH },
	{ __("Artwork Preview"), OPTION_ARTPREV_PATH },
	{ __("Bosses"),          OPTION_BOSSES_PATH },
	{ __("Logos"),           OPTION_LOGOS_PATH },
	{ __("Versus"),          OPTION_VERSUS_PATH },
	{ __("Game Over"),       OPTION_GAMEOVER_PATH },
	{ __("HowTo"),           OPTION_HOWTO_PATH },
	{ __("Scores"),          OPTION_SCORES_PATH },
	{ __("Select"),          OPTION_SELECT_PATH },
	{ __("Marquees"),        OPTION_MARQUEES_PATH },
	{ __("Covers"),          OPTION_COVER_PATH },
};

char const *const hover_msg[] = {
	__("Add or remove favorites"),
	__("Export displayed list to file"),
	__("Show DATs view"),
};

} // anonymous namespace

constexpr std::size_t menu_select_launch::MAX_VISIBLE_SEARCH; // stupid non-inline semantics


class menu_select_launch::software_parts : public menu
{
public:
	software_parts(mame_ui_manager &mui, render_container &container, s_parts &&parts, ui_software_info const &ui_info);
	virtual ~software_parts() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	ui_software_info const &m_uiinfo;
	s_parts const          m_parts;
};

class menu_select_launch::bios_selection : public menu
{
public:
	bios_selection(mame_ui_manager &mui, render_container &container, s_bios &&biosname, game_driver const &driver, bool inlist);
	bios_selection(mame_ui_manager &mui, render_container &container, s_bios &&biosname, ui_software_info const &swinfo, bool inlist);
	virtual ~bios_selection() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	bios_selection(mame_ui_manager &mui, render_container &container, s_bios &&biosname, void const *driver, bool software, bool inlist);

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	void const  *m_driver;
	bool        m_software, m_inlist;
	s_bios      m_bios;
};

std::string menu_select_launch::reselect_last::s_driver;
std::string menu_select_launch::reselect_last::s_software;
std::string menu_select_launch::reselect_last::s_swlist;
bool menu_select_launch::reselect_last::s_reselect = false;

std::mutex menu_select_launch::s_cache_guard;
menu_select_launch::cache_ptr_map menu_select_launch::s_caches;

// instantiate possible variants of these so derived classes don't get link errors
template bool menu_select_launch::select_bios(game_driver const &, bool);
template bool menu_select_launch::select_bios(ui_software_info const &, bool);
template float menu_select_launch::draw_left_panel<machine_filter>(machine_filter::type current, std::map<machine_filter::type, machine_filter::ptr> const &filters, float x1, float y1, float x2, float y2);
template float menu_select_launch::draw_left_panel<software_filter>(software_filter::type current, std::map<software_filter::type, software_filter::ptr> const &filters, float x1, float y1, float x2, float y2);


menu_select_launch::system_flags::system_flags(machine_static_info const &info)
	: m_machine_flags(info.machine_flags())
	, m_unemulated_features(info.unemulated_features())
	, m_imperfect_features(info.imperfect_features())
	, m_has_keyboard(info.has_keyboard())
	, m_has_analog(info.has_analog())
	, m_status_color(info.status_color())
{
}


void menu_select_launch::reselect_last::reset()
{
	s_driver.clear();
	s_software.clear();
	s_swlist.clear();
	reselect(false);
}

void menu_select_launch::reselect_last::set_driver(std::string const &name)
{
	s_driver = name;
	s_software.clear();
	s_swlist.clear();
}

void menu_select_launch::reselect_last::set_software(game_driver const &driver, ui_software_info const &swinfo)
{
	s_driver = driver.name;
	if (swinfo.startempty)
	{
		// FIXME: magic strings are bad...
		s_software = "[Start empty]";
		s_swlist.clear();
	}
	else
	{
		s_software = swinfo.shortname;
		s_swlist = swinfo.listname;
	}
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_launch::software_parts::software_parts(mame_ui_manager &mui, render_container &container, s_parts &&parts, ui_software_info const &ui_info)
	: menu(mui, container)
	, m_uiinfo(ui_info)
	, m_parts(std::move(parts))
{
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_launch::software_parts::~software_parts()
{
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_select_launch::software_parts::populate(float &customtop, float &custombottom)
{
	std::vector<s_parts::const_iterator> parts;
	parts.reserve(m_parts.size());
	for (s_parts::const_iterator it = m_parts.begin(); m_parts.end() != it; ++it)
		parts.push_back(it);
	std::sort(parts.begin(), parts.end(), [] (auto const &left, auto const &right) { return 0 > core_stricmp(left->first.c_str(), right->first.c_str()); });
	for (auto const &elem : parts)
		item_append(elem->first, elem->second, 0, (void *)&*elem);

	item_append(menu_item_type::SEPARATOR);
	customtop = ui().get_line_height() + (3.0f * ui().box_tb_border());
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_select_launch::software_parts::handle()
{
	// process the menu
	const event *menu_event = process(0);
	if (menu_event && (menu_event->iptkey) == IPT_UI_SELECT && menu_event->itemref)
	{
		for (auto const &elem : m_parts)
		{
			if ((void*)&elem == menu_event->itemref)
			{
				launch_system(ui(), *m_uiinfo.driver, &m_uiinfo, &elem.first, nullptr);
				break;
			}
		}
	}
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_select_launch::software_parts::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	char const *const text[] = { _("Software part selection:") };
	draw_text_box(
			std::begin(text), std::end(text),
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			ui::text_layout::CENTER, ui::text_layout::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_launch::bios_selection::bios_selection(mame_ui_manager &mui, render_container &container, s_bios &&biosname, game_driver const &driver, bool inlist)
	: bios_selection(mui, container, std::move(biosname), reinterpret_cast<void const *>(&driver), false, inlist)
{
}

menu_select_launch::bios_selection::bios_selection(mame_ui_manager &mui, render_container &container, s_bios &&biosname, ui_software_info const &swinfo, bool inlist)
	: bios_selection(mui, container, std::move(biosname), reinterpret_cast<void const *>(&swinfo), true, inlist)
{
}

menu_select_launch::bios_selection::bios_selection(mame_ui_manager &mui, render_container &container, s_bios &&biosname, void const *driver, bool software, bool inlist)
	: menu(mui, container)
	, m_driver(driver)
	, m_software(software)
	, m_inlist(inlist)
	, m_bios(std::move(biosname))
{
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_launch::bios_selection::~bios_selection()
{
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_select_launch::bios_selection::populate(float &customtop, float &custombottom)
{
	for (auto & elem : m_bios)
		item_append(elem.first, "", 0, (void *)&elem.first);

	item_append(menu_item_type::SEPARATOR);
	customtop = ui().get_line_height() + (3.0f * ui().box_tb_border());
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_select_launch::bios_selection::handle()
{
	// process the menu
	const event *menu_event = process(0);
	if (menu_event && menu_event->iptkey == IPT_UI_SELECT && menu_event->itemref)
	{
		for (auto & elem : m_bios)
		{
			if ((void*)&elem.first == menu_event->itemref)
			{
				if (!m_software)
				{
					const game_driver *s_driver = (const game_driver *)m_driver;
					if (m_inlist)
					{
						ui_software_info empty(*s_driver);
						launch_system(ui(), *s_driver, &empty, nullptr, &elem.second);
					}
					else
					{
						reselect_last::reselect(true);
						launch_system(ui(), *s_driver, nullptr, nullptr, &elem.second);
					}
				}
				else
				{
					ui_software_info *ui_swinfo = (ui_software_info *)m_driver;
					machine().options().set_value(OPTION_BIOS, elem.second, OPTION_PRIORITY_CMDLINE); // oh dear, relying on this persisting through the part selection menu
					driver_enumerator drivlist(machine().options(), *ui_swinfo->driver);
					drivlist.next();
					software_list_device *swlist = software_list_device::find_by_name(*drivlist.config(), ui_swinfo->listname.c_str());
					const software_info *swinfo = swlist->find(ui_swinfo->shortname.c_str());
					if (!select_part(ui(), container(), *swinfo, *ui_swinfo))
					{
						reselect_last::reselect(true);
						launch_system(ui(), drivlist.driver(), ui_swinfo, nullptr, &elem.second);
					}
				}
			}
		}
	}
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_select_launch::bios_selection::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	char const *const text[] = { _("BIOS selection:") };
	draw_text_box(
			std::begin(text), std::end(text),
			origx1, origx2, origy1 - top, origy1 - ui().box_tb_border(),
			ui::text_layout::CENTER, ui::text_layout::TRUNCATE, false,
			ui().colors().text_color(), UI_GREEN_COLOR, 1.0f);
}


menu_select_launch::cache::cache(running_machine &machine)
	: m_snapx_bitmap(std::make_unique<bitmap_argb32>(0, 0))
	, m_snapx_texture(nullptr, machine.render())
	, m_snapx_driver(nullptr)
	, m_snapx_software(nullptr)
	, m_no_avail_bitmap(256, 256)
	, m_star_bitmap(32, 32)
	, m_star_texture(nullptr, machine.render())
	, m_toolbar_bitmap()
	, m_sw_toolbar_bitmap()
	, m_toolbar_texture()
	, m_sw_toolbar_texture()
{
	render_manager &render(machine.render());

	// create a texture for snapshot
	m_snapx_texture.reset(render.texture_alloc(render_texture::hq_scale));

	std::memcpy(&m_no_avail_bitmap.pix32(0), no_avail_bmp, 256 * 256 * sizeof(uint32_t));

	std::memcpy(&m_star_bitmap.pix32(0), favorite_star_bmp, 32 * 32 * sizeof(uint32_t));
	m_star_texture.reset(render.texture_alloc());
	m_star_texture->set_bitmap(m_star_bitmap, m_star_bitmap.cliprect(), TEXFORMAT_ARGB32);

	m_toolbar_bitmap.reserve(UI_TOOLBAR_BUTTONS);
	m_sw_toolbar_bitmap.reserve(UI_TOOLBAR_BUTTONS);
	m_toolbar_texture.reserve(UI_TOOLBAR_BUTTONS);
	m_sw_toolbar_texture.reserve(UI_TOOLBAR_BUTTONS);

	for (std::size_t i = 0; i < UI_TOOLBAR_BUTTONS; ++i)
	{
		m_toolbar_bitmap.emplace_back(32, 32);
		m_sw_toolbar_bitmap.emplace_back(32, 32);
		m_toolbar_texture.emplace_back(render.texture_alloc(), render);
		m_sw_toolbar_texture.emplace_back(render.texture_alloc(), render);

		std::memcpy(&m_toolbar_bitmap.back().pix32(0), toolbar_bitmap_bmp[i], 32 * 32 * sizeof(uint32_t));
		if (m_toolbar_bitmap.back().valid())
			m_toolbar_texture.back()->set_bitmap(m_toolbar_bitmap.back(), m_toolbar_bitmap.back().cliprect(), TEXFORMAT_ARGB32);
		else
			m_toolbar_bitmap.back().reset();

		if ((i == 0U) || (i == 2U))
		{
			std::memcpy(&m_sw_toolbar_bitmap.back().pix32(0), toolbar_bitmap_bmp[i], 32 * 32 * sizeof(uint32_t));
			if (m_sw_toolbar_bitmap.back().valid())
				m_sw_toolbar_texture.back()->set_bitmap(m_sw_toolbar_bitmap.back(), m_sw_toolbar_bitmap.back().cliprect(), TEXFORMAT_ARGB32);
			else
				m_sw_toolbar_bitmap.back().reset();
		}
		else
		{
			m_sw_toolbar_bitmap.back().reset();
		}
	}
}


menu_select_launch::cache::~cache()
{
}


menu_select_launch::~menu_select_launch()
{
}


menu_select_launch::menu_select_launch(mame_ui_manager &mui, render_container &container, bool is_swlist)
	: menu(mui, container)
	, m_prev_selected(nullptr)
	, m_total_lines(0)
	, m_topline_datsview(0)
	, m_filter_highlight(0)
	, m_ui_error(false)
	, m_info_driver(nullptr)
	, m_info_software(nullptr)
	, m_info_view(-1)
	, m_items_list()
	, m_info_buffer()
	, m_cache()
	, m_is_swlist(is_swlist)
	, m_focus(focused_menu::MAIN)
	, m_pressed(false)
	, m_repeat(0)
	, m_right_visible_lines(0)
	, m_has_icons(false)
	, m_switch_image(false)
	, m_default_image(true)
	, m_image_view(FIRST_VIEW)
	, m_flags(256)
{
	// set up persistent cache for machine run
	{
		std::lock_guard<std::mutex> guard(s_cache_guard);
		auto const found(s_caches.find(&machine()));
		if (found != s_caches.end())
		{
			assert(found->second);
			m_cache = found->second;
		}
		else
		{
			m_cache = std::make_shared<cache>(machine());
			s_caches.emplace(&machine(), m_cache);
			add_cleanup_callback(&menu_select_launch::exit);
		}
	}
}


void menu_select_launch::next_image_view()
{
	if (LAST_VIEW > m_image_view)
	{
		++m_image_view;
		set_switch_image();
		m_default_image = false;
	}
}


void menu_select_launch::previous_image_view()
{
	if (FIRST_VIEW < m_image_view)
	{
		--m_image_view;
		set_switch_image();
		m_default_image = false;
	}
}


bool menu_select_launch::dismiss_error()
{
	bool const result = m_ui_error;
	if (result)
	{
		m_ui_error = false;
		m_error_text.clear();
		machine().ui_input().reset();
	}
	return result;
}

void menu_select_launch::set_error(reset_options ropt, std::string &&message)
{
	reset(ropt);
	m_ui_error = true;
	m_error_text = std::move(message);
}


//-------------------------------------------------
//  get overall emulation status for a system
//-------------------------------------------------

menu_select_launch::system_flags const &menu_select_launch::get_system_flags(game_driver const &driver)
{
	// try the cache
	flags_cache::const_iterator const found(m_flags.find(&driver));
	if (m_flags.end() != found)
		return found->second;

	// aggregate flags
	emu_options clean_options;
	machine_config const mconfig(driver, clean_options);
	return m_flags.emplace(&driver, machine_static_info(ui().options(), mconfig)).first->second;
}


//-------------------------------------------------
//  actually start an emulation session
//-------------------------------------------------

void menu_select_launch::launch_system(mame_ui_manager &mui, game_driver const &driver, ui_software_info const *swinfo, std::string const *part, int const *bios)
{
	emu_options &moptions(mui.machine().options());
	moptions.set_system_name(driver.name);

	if (swinfo)
	{
		if (!swinfo->startempty)
		{
			if (part)
				moptions.set_value(swinfo->instance, util::string_format("%s:%s:%s", swinfo->listname, swinfo->shortname, *part), OPTION_PRIORITY_CMDLINE);
			else
				moptions.set_value(OPTION_SOFTWARENAME, util::string_format("%s:%s", swinfo->listname, swinfo->shortname), OPTION_PRIORITY_CMDLINE);

			moptions.set_value(OPTION_SNAPNAME, util::string_format("%s%s%s", swinfo->listname, PATH_SEPARATOR, swinfo->shortname), OPTION_PRIORITY_CMDLINE);
		}
		reselect_last::set_software(driver, *swinfo);
	}
	else
	{
		reselect_last::set_driver(driver);
	}

	if (bios)
		moptions.set_value(OPTION_BIOS, *bios, OPTION_PRIORITY_CMDLINE);

	mame_machine_manager::instance()->schedule_new_driver(driver);
	mui.machine().schedule_hard_reset();
	stack_reset(mui.machine());
}


//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_select_launch::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	std::string tempbuf[5];

	// determine the text for the header
	make_topbox_text(tempbuf[0], tempbuf[1], tempbuf[2]);
	float const y1 = origy1 - 3.0f * ui().box_tb_border() - ui().get_line_height();
	draw_text_box(
			tempbuf, tempbuf + 3,
			origx1, origx2, origy1 - top, y1,
			ui::text_layout::CENTER, ui::text_layout::NEVER, true,
			ui().colors().text_color(), ui().colors().background_color(), 1.0f);

	// draw toolbar
	draw_toolbar(origx1, y1, origx2, origy1 - ui().box_tb_border());

	// determine the text to render below
	ui_software_info const *swinfo;
	game_driver const *driver;
	get_selection(swinfo, driver);

	bool isstar = false;
	rgb_t color = ui().colors().background_color();
	if (swinfo && ((swinfo->startempty != 1) || !driver))
	{
		isstar = mame_machine_manager::instance()->favorite().is_favorite_system_software(*swinfo);

		// first line is long name or system
		tempbuf[0] = make_software_description(*swinfo);

		// next line is year, publisher
		tempbuf[1] = string_format(_("%1$s, %2$-.100s"), swinfo->year, swinfo->publisher);

		// next line is parent/clone
		if (!swinfo->parentname.empty())
			tempbuf[2] = string_format(_("Software is clone of: %1$-.100s"), !swinfo->parentlongname.empty() ? swinfo->parentlongname : swinfo->parentname);
		else
			tempbuf[2] = _("Software is parent");

		// next line is supported status
		if (swinfo->supported == SOFTWARE_SUPPORTED_NO)
		{
			tempbuf[3] = _("Supported: No");
			color = UI_RED_COLOR;
		}
		else if (swinfo->supported == SOFTWARE_SUPPORTED_PARTIAL)
		{
			tempbuf[3] = _("Supported: Partial");
			color = UI_YELLOW_COLOR;
		}
		else
		{
			tempbuf[3] = _("Supported: Yes");
			color = UI_GREEN_COLOR;
		}

		// last line is romset name
		tempbuf[4] = string_format(_("romset: %1$-.100s"), swinfo->shortname);
	}
	else if (driver)
	{
		isstar = mame_machine_manager::instance()->favorite().is_favorite_system(*driver);

		// first line is game description/game name
		tempbuf[0] = make_driver_description(*driver);

		// next line is year, manufacturer
		tempbuf[1] = string_format(_("%1$s, %2$-.100s"), driver->year, driver->manufacturer);

		// next line is clone/parent status
		int cloneof = driver_list::non_bios_clone(*driver);

		if (cloneof != -1)
			tempbuf[2] = string_format(_("Driver is clone of: %1$-.100s"), driver_list::driver(cloneof).type.fullname());
		else
			tempbuf[2] = _("Driver is parent");

		// next line is overall driver status
		system_flags const &flags(get_system_flags(*driver));
		if (flags.machine_flags() & machine_flags::NOT_WORKING)
			tempbuf[3] = _("Overall: NOT WORKING");
		else if ((flags.unemulated_features() | flags.imperfect_features()) & device_t::feature::PROTECTION)
			tempbuf[3] = _("Overall: Unemulated Protection");
		else
			tempbuf[3] = _("Overall: Working");

		// next line is graphics, sound status
		if (flags.unemulated_features() & device_t::feature::GRAPHICS)
			tempbuf[4] = _("Graphics: Unimplemented, ");
		else if ((flags.unemulated_features() | flags.imperfect_features()) & (device_t::feature::GRAPHICS | device_t::feature::PALETTE))
			tempbuf[4] = _("Graphics: Imperfect, ");
		else
			tempbuf[4] = _("Graphics: OK, ");

		if (driver->flags & machine_flags::NO_SOUND_HW)
			tempbuf[4].append(_("Sound: None"));
		else if (flags.unemulated_features() & device_t::feature::SOUND)
			tempbuf[4].append(_("Sound: Unimplemented"));
		else if (flags.imperfect_features() & device_t::feature::SOUND)
			tempbuf[4].append(_("Sound: Imperfect"));
		else
			tempbuf[4].append(_("Sound: OK"));

		color = flags.status_color();
	}
	else
	{
		std::string copyright(emulator_info::get_copyright());
		size_t found = copyright.find("\n");

		tempbuf[0].clear();
		tempbuf[1] = string_format(_("%1$s %2$s"), emulator_info::get_appname(), build_version);
		tempbuf[2] = copyright.substr(0, found);
		tempbuf[3] = copyright.substr(found + 1);
		tempbuf[4].clear();
	}

	// draw the footer
	draw_text_box(
			std::begin(tempbuf), std::end(tempbuf),
			origx1, origx2, origy2 + ui().box_tb_border(), origy2 + bottom,
			ui::text_layout::CENTER, ui::text_layout::NEVER, true,
			ui().colors().text_color(), color, 1.0f);

	// is favorite? draw the star
	if (isstar)
		draw_star(origx1 + ui().box_lr_border(), origy2 + (2.0f * ui().box_tb_border()));
}


void menu_select_launch::rotate_focus(int dir)
{
	switch (get_focus())
	{
	case focused_menu::MAIN:
		if (selected_index() <= m_available_items)
		{
			m_prev_selected = get_selection_ref();
			if ((0 < dir) || (ui_globals::panels_status == HIDE_BOTH))
				set_selected_index(m_available_items + 1);
			else if (ui_globals::panels_status == HIDE_RIGHT_PANEL)
				set_focus(focused_menu::LEFT);
			else
				set_focus(focused_menu::RIGHTBOTTOM);
		}
		else
		{
			if ((0 > dir) || (ui_globals::panels_status == HIDE_BOTH))
				select_prev();
			else if (ui_globals::panels_status == HIDE_LEFT_PANEL)
				set_focus(focused_menu::RIGHTTOP);
			else
				set_focus(focused_menu::LEFT);
		}
		break;

	case focused_menu::LEFT:
		if (0 > dir)
		{
			set_focus(focused_menu::MAIN);
			set_selected_index(m_available_items + 1);
		}
		else if (ui_globals::panels_status != HIDE_RIGHT_PANEL)
		{
			set_focus(focused_menu::RIGHTTOP);
		}
		else
		{
			set_focus(focused_menu::MAIN);
			select_prev();
		}
		break;

	case focused_menu::RIGHTTOP:
		if (0 < dir)
		{
			set_focus(focused_menu::RIGHTBOTTOM);
		}
		else if (ui_globals::panels_status != HIDE_LEFT_PANEL)
		{
			set_focus(focused_menu::LEFT);
		}
		else
		{
			set_focus(focused_menu::MAIN);
			set_selected_index(m_available_items + 1);
		}
		break;

	case focused_menu::RIGHTBOTTOM:
		if (0 > dir)
		{
			set_focus(focused_menu::RIGHTTOP);
		}
		else
		{
			set_focus(focused_menu::MAIN);
			select_prev();
		}
		break;
	}
}


void menu_select_launch::inkey_dats()
{
	ui_software_info const *software;
	game_driver const *driver;
	get_selection(software, driver);
	if (software)
	{
		if (software->startempty && mame_machine_manager::instance()->lua()->call_plugin_check<const char *>("data_list", software->driver->name, true))
			menu::stack_push<menu_dats_view>(ui(), container(), software->driver);
		else if (mame_machine_manager::instance()->lua()->call_plugin_check<const char *>("data_list", std::string(software->shortname).append(1, ',').append(software->listname).c_str()) || !software->usage.empty())
			menu::stack_push<menu_dats_view>(ui(), container(), software);
	}
	else if (driver)
	{
		if (mame_machine_manager::instance()->lua()->call_plugin_check<const char *>("data_list", driver->name, true))
			menu::stack_push<menu_dats_view>(ui(), container(), driver);
	}
}


//-------------------------------------------------
//  draw common arrows
//-------------------------------------------------

void menu_select_launch::draw_common_arrow(float origx1, float origy1, float origx2, float origy2, int current, int dmin, int dmax, float title_size)
{
	auto line_height = ui().get_line_height();
	auto lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	auto gutter_width = lr_arrow_width * 1.3f;

	// set left-right arrows dimension
	float const ar_x0 = 0.5f * (origx2 + origx1) + 0.5f * title_size + gutter_width - lr_arrow_width;
	float const ar_y0 = origy1 + 0.1f * line_height;
	float const ar_x1 = 0.5f * (origx2 + origx1) + 0.5f * title_size + gutter_width;
	float const ar_y1 = origy1 + 0.9f * line_height;

	float const al_x0 = 0.5f * (origx2 + origx1) - 0.5f * title_size - gutter_width;
	float const al_y0 = origy1 + 0.1f * line_height;
	float const al_x1 = 0.5f * (origx2 + origx1) - 0.5f * title_size - gutter_width + lr_arrow_width;
	float const al_y1 = origy1 + 0.9f * line_height;

	rgb_t fgcolor_right, fgcolor_left;
	fgcolor_right = fgcolor_left = ui().colors().text_color();

	// set hover
	if (mouse_in_rect(ar_x0, ar_y0, ar_x1, ar_y1) && current != dmax)
	{
		ui().draw_textured_box(container(), ar_x0 + 0.01f, ar_y0, ar_x1 - 0.01f, ar_y1, ui().colors().mouseover_bg_color(), rgb_t(43, 43, 43),
				hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		set_hover(HOVER_UI_RIGHT);
		fgcolor_right = ui().colors().mouseover_color();
	}
	else if (mouse_in_rect(al_x0, al_y0, al_x1, al_y1) && current != dmin)
	{
		ui().draw_textured_box(container(), al_x0 + 0.01f, al_y0, al_x1 - 0.01f, al_y1, ui().colors().mouseover_bg_color(), rgb_t(43, 43, 43),
				hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		set_hover(HOVER_UI_LEFT);
		fgcolor_left = ui().colors().mouseover_color();
	}

	// apply arrow
	if (dmax == dmin)
		return;
	else if (current == dmin)
		draw_arrow(ar_x0, ar_y0, ar_x1, ar_y1, fgcolor_right, ROT90);
	else if (current == dmax)
		draw_arrow(al_x0, al_y0, al_x1, al_y1, fgcolor_left, ROT90 ^ ORIENTATION_FLIP_X);
	else
	{
		draw_arrow(ar_x0, ar_y0, ar_x1, ar_y1, fgcolor_right, ROT90);
		draw_arrow(al_x0, al_y0, al_x1, al_y1, fgcolor_left, ROT90 ^ ORIENTATION_FLIP_X);
	}
}


//-------------------------------------------------
//  draw info arrow
//-------------------------------------------------

void menu_select_launch::draw_info_arrow(int ub, float origx1, float origx2, float oy1, float line_height, float text_size, float ud_arrow_width)
{
	rgb_t fgcolor = ui().colors().text_color();
	uint32_t orientation = (!ub) ? ROT0 : ROT0 ^ ORIENTATION_FLIP_Y;

	if (mouse_in_rect(origx1, oy1, origx2, oy1 + (line_height * text_size)))
	{
		ui().draw_textured_box(container(), origx1 + 0.01f, oy1, origx2 - 0.01f, oy1 + (line_height * text_size), ui().colors().mouseover_bg_color(),
				rgb_t(43, 43, 43), hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		set_hover((!ub) ? HOVER_DAT_UP : HOVER_DAT_DOWN);
		fgcolor = ui().colors().mouseover_color();
	}

	draw_arrow(0.5f * (origx1 + origx2) - 0.5f * (ud_arrow_width * text_size), oy1 + 0.25f * (line_height * text_size),
			0.5f * (origx1 + origx2) + 0.5f * (ud_arrow_width * text_size), oy1 + 0.75f * (line_height * text_size), fgcolor, orientation);
}

bool menu_select_launch::draw_error_text()
{
	if (m_ui_error)
		ui().draw_text_box(container(), m_error_text.c_str(), ui::text_layout::CENTER, 0.5f, 0.5f, UI_RED_COLOR);

	return m_ui_error;
}


template <typename Filter>
float menu_select_launch::draw_left_panel(
		typename Filter::type current,
		std::map<typename Filter::type, typename Filter::ptr> const &filters,
		float x1, float y1, float x2, float y2)
{
	if ((ui_globals::panels_status != SHOW_PANELS) && (ui_globals::panels_status != HIDE_RIGHT_PANEL))
		return draw_collapsed_left_panel(x1, y1, x2, y2);

	// calculate line height
	float const line_height(ui().get_line_height());
	float const text_size(ui().options().infos_size());
	float const sc(y2 - y1 - (2.0f * ui().box_tb_border()));
	float line_height_max(line_height * text_size);
	if ((Filter::COUNT * line_height_max) > sc)
	{
		float const lm(sc / Filter::COUNT);
		line_height_max = line_height * (lm / line_height);
	}

	// calculate horizontal offset for unadorned names
	std::string tmp("_# ");
	convert_command_glyph(tmp);
	float const text_sign = ui().get_string_width(tmp.c_str(), text_size);

	// get the maximum width of a filter name
	float left_width(0.0f);
	for (typename Filter::type x = Filter::FIRST; Filter::COUNT > x; ++x)
		left_width = std::max(ui().get_string_width(Filter::display_name(x), text_size) + text_sign, left_width);

	// outline the box and inset by the border width
	float const origy1(y1);
	float const origy2(y2);
	x2 = x1 + left_width + 2.0f * ui().box_lr_border();;
	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());
	x1 += ui().box_lr_border();
	x2 -= ui().box_lr_border();
	y1 += ui().box_tb_border();
	y2 -= ui().box_tb_border();

	// now draw the rows
	auto const active_filter(filters.find(current));
	for (typename Filter::type filter = Filter::FIRST; Filter::COUNT > filter; ++filter)
	{
		std::string str;
		if (filters.end() != active_filter)
		{
			str = active_filter->second->adorned_display_name(filter);
		}
		else
		{
			if (current == filter)
			{
				str = std::string("_> ");
				convert_command_glyph(str);
			}
			str.append(Filter::display_name(filter));
		}

		// handle mouse hover in passing
		rgb_t bgcolor = ui().colors().text_bg_color();
		rgb_t fgcolor = ui().colors().text_color();
		if (mouse_in_rect(x1, y1, x2, y1 + line_height_max))
		{
			bgcolor = ui().colors().mouseover_bg_color();
			fgcolor = ui().colors().mouseover_color();
			set_hover(HOVER_FILTER_FIRST + filter);
			highlight(x1, y1, x2, y1 + line_height_max, bgcolor);
		}

		// draw primary highlight if keyboard focus is here
		if ((m_filter_highlight == filter) && (get_focus() == focused_menu::LEFT))
		{
			fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
			ui().draw_textured_box(
					container(),
					x1, y1, x2, y1 + line_height_max,
					bgcolor, rgb_t(255, 43, 43, 43),
					hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}

		// finally draw the text itself and move to the next line
		float const x1t = x1 + ((str == Filter::display_name(filter)) ? text_sign : 0.0f);
		ui().draw_text_full(
				container(), str.c_str(),
				x1t, y1, x2 - x1,
				ui::text_layout::LEFT, ui::text_layout::NEVER,
				mame_ui_manager::NORMAL, fgcolor, bgcolor,
				nullptr, nullptr, text_size);
		y1 += line_height_max;
	}

	x1 = x2 + ui().box_lr_border();
	x2 = x1 + 2.0f * ui().box_lr_border();
	y1 = origy1;
	y2 = origy2;
	float const space = x2 - x1;
	float const lr_arrow_width = 0.4f * space * machine().render().ui_aspect();

	// set left-right arrows dimension
	float const ar_x0 = 0.5f * (x2 + x1) - 0.5f * lr_arrow_width;
	float const ar_y0 = 0.5f * (y2 + y1) + 0.1f * space;
	float const ar_x1 = ar_x0 + lr_arrow_width;
	float const ar_y1 = 0.5f * (y2 + y1) + 0.9f * space;

	ui().draw_outlined_box(container(), x1, y1, x2, y2, rgb_t(0xef, 0x12, 0x47, 0x7b));

	rgb_t fgcolor = ui().colors().text_color();
	if (mouse_in_rect(x1, y1, x2, y2))
	{
		fgcolor = ui().colors().mouseover_color();
		set_hover(HOVER_LPANEL_ARROW);
	}

	draw_arrow(ar_x0, ar_y0, ar_x1, ar_y1, fgcolor, ROT90 ^ ORIENTATION_FLIP_X);
	return x2 + ui().box_lr_border();
}


//-------------------------------------------------
//  icon helpers
//-------------------------------------------------

void menu_select_launch::check_for_icons(char const *listname)
{
	// only ever set the flag, never clear it
	if (m_has_icons)
		return;

	// iterate over configured icon paths
	path_iterator paths(ui().options().icons_directory());
	std::string current;
	while (paths.next(current))
	{
		// if we're doing a software list, append it to the configured path
		if (listname)
		{
			if (!current.empty() && !util::is_directory_separator(current.back()))
				current.append(PATH_SEPARATOR);
			current.append(listname);
		}
		osd_printf_verbose("Checking for icons in directory %s\n", current);

		// open and walk the directory
		osd::directory::ptr const dir(osd::directory::open(current));
		if (dir)
		{
			// this could be improved in many ways - it's just a rough go/no-go
			osd::directory::entry const *entry;
			while ((entry = dir->read()) != nullptr)
			{
				current = entry->name;
				std::string::size_type const found(current.rfind(".ico"));
				if ((std::string::npos != found) && ((current.length() - 4) == found))
				{
					osd_printf_verbose("Entry %s is a candidate icon file\n", entry->name);
					m_has_icons = true;
					return;
				}
				else if (("icons" == current) || (current.find("icons.") == 0U))
				{
					osd_printf_verbose("Entry %s is a candidate icon collection\n", entry->name);
					m_has_icons = true;
					return;
				}
			}
		}
	}

	// nothing promising
	osd_printf_verbose(
			"No candidate icons found for %s%s\n",
			listname ? "software list " : "",
			listname ? listname : "machines");
}

std::string menu_select_launch::make_icon_paths(char const *listname) const
{
	// iterate over configured icon paths
	path_iterator paths(ui().options().icons_directory());
	std::string current, result;
	while (paths.next(current))
	{
		// if we're doing a software list, append it to the configured path
		if (listname)
		{
			if (!current.empty() && !util::is_directory_separator(current.back()))
				current.append(PATH_SEPARATOR);
			current.append(listname);
		}

		// append the configured path
		if (!result.empty())
			result.append(1, ';'); // FIXME: should be a macro
		result.append(current);

		// append with "icons" appended so it'll search icons.zip or icons.7z in the directory
		if (!current.empty())
		{
			result.append(1, ';'); // FIXME: should be a macro
			result.append(current);
			if (!util::is_directory_separator(result.back()))
				result.append(PATH_SEPARATOR);
		}
		result.append("icons");
	}

	// log the result for debugging
	osd_printf_verbose(
			"Icon path for %s%s set to %s\n",
			listname ? "software list " : "",
			listname ? listname : "machines",
			result.c_str());
	return result;
}

bool menu_select_launch::scale_icon(bitmap_argb32 &&src, texture_and_bitmap &dst) const
{
	assert(dst.texture);
	if (src.valid())
	{
		// calculate available space for the icon in pixels
		float const height(ui().get_line_height());
		float const width(height * container().manager().ui_aspect());
		render_target const &target(machine().render().ui_target());
		uint32_t const dst_height(target.height());
		uint32_t const dst_width(target.width());
		bool const rotated((target.orientation() & ORIENTATION_SWAP_XY) != 0);
		int const max_height(int((rotated ? dst_width : dst_height) * height));
		int const max_width(int((rotated ? dst_height : dst_width) * width));

		// reduce the source bitmap if it's too big
		bitmap_argb32 tmp;
		float const ratio((std::min)({ float(max_height) / src.height(), float(max_width) / src.width(), 1.0F }));
		if (1.0F > ratio)
		{
			float const pix_height(src.height() * ratio);
			float const pix_width(src.width() * ratio);
			tmp.allocate(int32_t(pix_width), int32_t(pix_height));
			render_resample_argb_bitmap_hq(tmp, src, render_color{ 1.0F, 1.0F, 1.0F, 1.0F }, true);
		}
		else
		{
			tmp = std::move(src);
		}

		// copy into the destination
		dst.bitmap.allocate(max_width, max_height);
		for (int y = 0; tmp.height() > y; ++y)
			for (int x = 0; tmp.width() > x; ++x)
				dst.bitmap.pix32(y, x) = tmp.pix32(y, x);
		dst.texture->set_bitmap(dst.bitmap, dst.bitmap.cliprect(), TEXFORMAT_ARGB32);
		return true;
	}
	else
	{
		// couldn't load icon
		dst.bitmap.reset();
		return false;
	}
}


template <typename T> bool menu_select_launch::select_bios(T const &driver, bool inlist)
{
	s_bios biosname;
	if (ui().options().skip_bios_menu() || !has_multiple_bios(driver, biosname))
		return false;

	menu::stack_push<bios_selection>(ui(), container(), std::move(biosname), driver, inlist);
	return true;
}

bool menu_select_launch::select_part(software_info const &info, ui_software_info const &ui_info)
{
	return select_part(ui(), container(), info, ui_info);
}

bool menu_select_launch::select_part(mame_ui_manager &mui, render_container &container, software_info const &info, ui_software_info const &ui_info)
{
	if (mui.options().skip_parts_menu() || !info.has_multiple_parts(ui_info.interface.c_str()))
		return false;

	s_parts parts;
	for (software_part const &part : info.parts())
	{
		if (part.matches_interface(ui_info.interface.c_str()))
		{
			std::string menu_part_name(part.name());
			if (part.feature("part_id"))
				menu_part_name.assign("(").append(part.feature("part_id")).append(")");
			parts.emplace(part.name(), std::move(menu_part_name));
		}
	}
	menu::stack_push<software_parts>(mui, container, std::move(parts), ui_info);
	return true;
}


//-------------------------------------------------
//  draw toolbar
//-------------------------------------------------

void menu_select_launch::draw_toolbar(float x1, float y1, float x2, float y2)
{
	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

	// take off the borders
	x1 += ui().box_lr_border();
	x2 -= ui().box_lr_border();
	y1 += ui().box_tb_border();
	y2 -= ui().box_tb_border();

	texture_ptr_vector const &t_texture(m_is_swlist ? m_cache->sw_toolbar_texture() : m_cache->toolbar_texture());
	bitmap_vector const &t_bitmap(m_is_swlist ? m_cache->sw_toolbar_bitmap() : m_cache->toolbar_bitmap());

	auto const num_valid(std::count_if(std::begin(t_bitmap), std::end(t_bitmap), [](bitmap_argb32 const &e) { return e.valid(); }));

	float const space_x = (y2 - y1) * container().manager().ui_aspect(&container());
	float const total = (float(num_valid) * space_x) + (float(num_valid - 1) * 0.001f);
	x1 += (x2 - x1) * 0.5f - total * 0.5f;
	x2 = x1 + space_x;

	for (int z = 0; z < UI_TOOLBAR_BUTTONS; ++z)
	{
		if (t_bitmap[z].valid())
		{
			rgb_t color(0xEFEFEFEF);
			if (mouse_in_rect(x1, y1, x2, y2))
			{
				set_hover(HOVER_B_FAV + z);
				color = rgb_t::white();
				float ypos = y2 + ui().get_line_height() + 2.0f * ui().box_tb_border();
				ui().draw_text_box(container(), _(hover_msg[z]), ui::text_layout::CENTER, 0.5f, ypos, ui().colors().background_color());
			}

			container().add_quad(x1, y1, x2, y2, color, t_texture[z].get(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			x1 += space_x + ((z < UI_TOOLBAR_BUTTONS - 1) ? 0.001f : 0.0f);
			x2 = x1 + space_x;
		}
	}
}


//-------------------------------------------------
//  draw favorites star
//-------------------------------------------------

void menu_select_launch::draw_star(float x0, float y0)
{
	float y1 = y0 + ui().get_line_height();
	float x1 = x0 + ui().get_line_height() * container().manager().ui_aspect();
	container().add_quad(x0, y0, x1, y1, rgb_t::white(), m_cache->star_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_PACKABLE);
}


void menu_select_launch::set_pressed()
{
	(m_repeat == 0) ? m_repeat = osd_ticks() + osd_ticks_per_second() / 2 : m_repeat = osd_ticks() + osd_ticks_per_second() / 4;
	m_pressed = true;
}


//-------------------------------------------------
//  draw icons
//-------------------------------------------------

void menu_select_launch::draw_icon(int linenum, void *selectedref, float x0, float y0)
{
	render_texture *const icon(get_icon_texture(linenum, selectedref));
	if (icon)
	{
		float const ud_arrow_width = ui().get_line_height() * container().manager().ui_aspect(&container());
		float const x1 = x0 + ud_arrow_width;
		float const y1 = y0 + ui().get_line_height();
		container().add_quad(x0, y0, x1, y1, rgb_t::white(), icon, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	};
}


//-------------------------------------------------
//  get title and search path for right panel
//-------------------------------------------------

void menu_select_launch::get_title_search(std::string &snaptext, std::string &searchstr)
{
	// get arts title text
	snaptext.assign(_(arts_info[m_image_view].first));

	// get search path
	std::string addpath;
	if (m_image_view == SNAPSHOT_VIEW)
	{
		emu_options moptions;
		searchstr = machine().options().value(arts_info[m_image_view].second);
		addpath = moptions.value(arts_info[m_image_view].second);
	}
	else
	{
		ui_options moptions;
		searchstr = ui().options().value(arts_info[m_image_view].second);
		addpath = moptions.value(arts_info[m_image_view].second);
	}

	std::string tmp(searchstr);
	path_iterator path(tmp.c_str());
	path_iterator path_iter(addpath.c_str());
	std::string c_path, curpath;

	// iterate over path and add path for zipped formats
	while (path.next(curpath))
	{
		path_iter.reset();
		while (path_iter.next(c_path))
			searchstr.append(";").append(curpath).append(PATH_SEPARATOR).append(c_path);
	}
}


//-------------------------------------------------
//  handle keys for main menu
//-------------------------------------------------

void menu_select_launch::handle_keys(uint32_t flags, int &iptkey)
{
	bool const ignorepause = stack_has_special_main_menu();

	// bail if no items
	if (item_count() == 0)
		return;

	// if we hit select, return true or pop the stack, depending on the item
	if (exclusive_input_pressed(iptkey, IPT_UI_SELECT, 0))
	{
		if (m_ui_error)
		{
			// dismiss error
		}
		else if (m_focus == focused_menu::LEFT)
		{
			m_prev_selected = nullptr;
			filter_selected();
		}
		if (is_last_selected() && (m_focus == focused_menu::MAIN))
		{
			iptkey = IPT_UI_CANCEL;
			stack_pop();
		}
		return;
	}

	if (exclusive_input_pressed(iptkey, IPT_UI_CANCEL, 0))
	{
		if (m_ui_error)
		{
			// dismiss error
		}
		else if (menu_has_search_active())
		{
			// escape pressed with non-empty search text clears it
			m_search.clear();
			reset(reset_options::SELECT_FIRST);
		}
		else
		{
			// otherwise pop the stack
			stack_pop();
		}
		return;
	}

	// validate the current selection
	validate_selection(1);

	// swallow left/right keys if they are not appropriate
	bool const ignoreleft = ((selected_item().flags & FLAG_LEFT_ARROW) == 0);
	bool const ignoreright = ((selected_item().flags & FLAG_RIGHT_ARROW) == 0);
	bool const leftclose = (ui_globals::panels_status == HIDE_BOTH || ui_globals::panels_status == HIDE_LEFT_PANEL);
	bool const rightclose = (ui_globals::panels_status == HIDE_BOTH || ui_globals::panels_status == HIDE_RIGHT_PANEL);

	// accept left/right keys as-is with repeat
	if (!ignoreleft && exclusive_input_pressed(iptkey, IPT_UI_LEFT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
	{
		// Swap the right panel
		if (m_focus == focused_menu::RIGHTTOP)
			ui_globals::rpanel = RP_IMAGES;
		return;
	}

	if (!ignoreright && exclusive_input_pressed(iptkey, IPT_UI_RIGHT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
	{
		// Swap the right panel
		if (m_focus == focused_menu::RIGHTTOP)
			ui_globals::rpanel = RP_INFOS;
		return;
	}

	// up backs up by one item
	if (exclusive_input_pressed(iptkey, IPT_UI_UP, 6))
	{
		if (!leftclose && m_focus == focused_menu::LEFT)
		{
			return;
		}
		else if (!rightclose && m_focus == focused_menu::RIGHTBOTTOM)
		{
			m_topline_datsview--;
			return;
		}
		else if (selected_index() == m_available_items + 1 || is_first_selected() || m_ui_error)
		{
			return;
		}

		set_selected_index(selected_index() - 1);

		if (selected_index() == top_line && top_line != 0)
			top_line--;
	}

	// down advances by one item
	if (exclusive_input_pressed(iptkey, IPT_UI_DOWN, 6))
	{
		if (!leftclose && m_focus == focused_menu::LEFT)
		{
			return;
		}
		else if (!rightclose && m_focus == focused_menu::RIGHTBOTTOM)
		{
			m_topline_datsview++;
			return;
		}
		else if (is_last_selected() || selected_index() == m_available_items - 1 || m_ui_error)
		{
			return;
		}

		set_selected_index(selected_index() + 1);
		if (selected_index() == top_line + m_visible_items + (top_line != 0))
			top_line++;
	}

	// page up backs up by m_visible_items
	if (exclusive_input_pressed(iptkey, IPT_UI_PAGE_UP, 6))
	{
		// Infos
		if (!rightclose && m_focus == focused_menu::RIGHTBOTTOM)
		{
			m_topline_datsview -= m_right_visible_lines - 1;
			return;
		}

		if (selected_index() < m_available_items && !m_ui_error)
		{
			set_selected_index(std::max(selected_index() - m_visible_items, 0));

			top_line -= m_visible_items - (top_line + m_visible_lines == m_available_items);
		}
	}

	// page down advances by m_visible_items
	if (exclusive_input_pressed(iptkey, IPT_UI_PAGE_DOWN, 6))
	{
		// Infos
		if (!rightclose && m_focus == focused_menu::RIGHTBOTTOM)
		{
			m_topline_datsview += m_right_visible_lines - 1;
			return;
		}

		if (selected_index() < m_available_items && !m_ui_error)
		{
			set_selected_index(std::min(selected_index() + m_visible_lines - 2 + (selected_index() == 0), m_available_items - 1));

			top_line += m_visible_lines - 2;
		}
	}

	// home goes to the start
	if (exclusive_input_pressed(iptkey, IPT_UI_HOME, 0))
	{
		if (!leftclose && m_focus == focused_menu::LEFT)
		{
			return;
		}
		else if (!rightclose && m_focus == focused_menu::RIGHTBOTTOM)
		{
			m_topline_datsview = 0;
			return;
		}

		if (selected_index() < m_available_items && !m_ui_error)
			select_first_item();
	}

	// end goes to the last
	if (exclusive_input_pressed(iptkey, IPT_UI_END, 0))
	{
		if (!leftclose && m_focus == focused_menu::LEFT)
		{
			return;
		}
		else if (!rightclose && m_focus == focused_menu::RIGHTBOTTOM)
		{
			m_topline_datsview = m_total_lines;
			return;
		}

		if (selected_index() < m_available_items && !m_ui_error)
			set_selected_index(top_line = m_available_items - 1);
	}

	// focus next rotates throw targets forward
	if (exclusive_input_pressed(iptkey, IPT_UI_FOCUS_NEXT, 12))
	{
		if (!m_ui_error)
			rotate_focus(1);
	}

	// focus next rotates throw targets forward
	if (exclusive_input_pressed(iptkey, IPT_UI_FOCUS_PREV, 12))
	{
		if (!m_ui_error)
			rotate_focus(-1);
	}

	// pause enables/disables pause
	if (!m_ui_error && !ignorepause && exclusive_input_pressed(iptkey, IPT_UI_PAUSE, 0))
	{
		if (machine().paused())
			machine().resume();
		else
			machine().pause();
	}

	// handle a toggle cheats request
	if (!m_ui_error && machine().ui_input().pressed_repeat(IPT_UI_TOGGLE_CHEAT, 0))
		mame_machine_manager::instance()->cheat().set_enable(!mame_machine_manager::instance()->cheat().enabled());

	// see if any other UI keys are pressed
	if (iptkey == IPT_INVALID)
	{
		for (int code = IPT_UI_FIRST + 1; code < IPT_UI_LAST; code++)
		{
			if (m_ui_error)
				continue;

			switch (code)
			{
			case IPT_UI_FOCUS_NEXT:
			case IPT_UI_FOCUS_PREV:
				continue;
			case IPT_UI_LEFT:
				if (ignoreleft)
					continue;
				break;
			case IPT_UI_RIGHT:
				if (ignoreright)
					continue;
				break;
			case IPT_UI_PAUSE:
				if (ignorepause)
					continue;
				break;
			}

			if (exclusive_input_pressed(iptkey, code, 0))
				break;
		}
	}
}


//-------------------------------------------------
//  handle input events for main menu
//-------------------------------------------------

void menu_select_launch::handle_events(uint32_t flags, event &ev)
{
	if (m_pressed)
	{
		bool const pressed = mouse_pressed();
		int32_t target_x, target_y;
		bool button;
		render_target *const mouse_target = machine().ui_input().find_mouse(&target_x, &target_y, &button);
		if (mouse_target && button && (hover() == HOVER_ARROW_DOWN || hover() == HOVER_ARROW_UP))
		{
			if (pressed)
				machine().ui_input().push_mouse_down_event(mouse_target, target_x, target_y);
		}
		else
		{
			reset_pressed();
		}
	}

	// loop while we have interesting events
	bool stop(false), search_changed(false);
	ui_event local_menu_event;
	while (!stop && machine().ui_input().pop_event(&local_menu_event))
	{
		switch (local_menu_event.event_type)
		{
		// if we are hovering over a valid item, select it with a single click
		case ui_event::MOUSE_DOWN:
			if (m_ui_error)
			{
				ev.iptkey = IPT_OTHER;
				stop = true;
			}
			else
			{
				if (hover() >= 0 && hover() < item_count())
				{
					if (hover() >= m_available_items - 1 && selected_index() < m_available_items)
						m_prev_selected = get_selection_ref();
					set_selected_index(hover());
					m_focus = focused_menu::MAIN;
				}
				else if (hover() == HOVER_ARROW_UP)
				{
					set_selected_index(std::max(selected_index() - m_visible_items, 0));
					top_line -= m_visible_items - (top_line + m_visible_lines == m_available_items);
					set_pressed();
				}
				else if (hover() == HOVER_ARROW_DOWN)
				{
					set_selected_index(std::min(selected_index() + m_visible_lines - 2 + (selected_index() == 0), m_available_items - 1));
					top_line += m_visible_lines - 2;
					set_pressed();
				}
				else if (hover() == HOVER_UI_RIGHT)
					ev.iptkey = IPT_UI_RIGHT;
				else if (hover() == HOVER_UI_LEFT)
					ev.iptkey = IPT_UI_LEFT;
				else if (hover() == HOVER_DAT_DOWN)
					m_topline_datsview += m_right_visible_lines - 1;
				else if (hover() == HOVER_DAT_UP)
					m_topline_datsview -= m_right_visible_lines - 1;
				else if (hover() == HOVER_LPANEL_ARROW)
				{
					if (get_focus() == focused_menu::LEFT)
					{
						set_focus(focused_menu::MAIN);
						select_prev();
					}

					if (ui_globals::panels_status == HIDE_LEFT_PANEL)
						ui_globals::panels_status = SHOW_PANELS;
					else if (ui_globals::panels_status == HIDE_BOTH)
						ui_globals::panels_status = HIDE_RIGHT_PANEL;
					else if (ui_globals::panels_status == SHOW_PANELS)
						ui_globals::panels_status = HIDE_LEFT_PANEL;
					else if (ui_globals::panels_status == HIDE_RIGHT_PANEL)
						ui_globals::panels_status = HIDE_BOTH;
				}
				else if (hover() == HOVER_RPANEL_ARROW)
				{
					if ((get_focus() == focused_menu::RIGHTTOP) || (get_focus() == focused_menu::RIGHTBOTTOM))
					{
						set_focus(focused_menu::MAIN);
						select_prev();
					}

					if (ui_globals::panels_status == HIDE_RIGHT_PANEL)
						ui_globals::panels_status = SHOW_PANELS;
					else if (ui_globals::panels_status == HIDE_BOTH)
						ui_globals::panels_status = HIDE_LEFT_PANEL;
					else if (ui_globals::panels_status == SHOW_PANELS)
						ui_globals::panels_status = HIDE_RIGHT_PANEL;
					else if (ui_globals::panels_status == HIDE_LEFT_PANEL)
						ui_globals::panels_status = HIDE_BOTH;
				}
				else if (hover() == HOVER_B_FAV)
				{
					ev.iptkey = IPT_UI_FAVORITES;
					stop = true;
				}
				else if (hover() == HOVER_B_EXPORT)
				{
					inkey_export();
					stop = true;
				}
				else if (hover() == HOVER_B_DATS)
				{
					inkey_dats();
					stop = true;
				}
				else if (hover() >= HOVER_RP_FIRST && hover() <= HOVER_RP_LAST)
				{
					ui_globals::rpanel = (HOVER_RP_FIRST - hover()) * (-1);
					stop = true;
				}
				else if (hover() >= HOVER_FILTER_FIRST && hover() <= HOVER_FILTER_LAST)
				{
					m_prev_selected = nullptr;
					m_filter_highlight = hover() - HOVER_FILTER_FIRST;
					filter_selected();
					stop = true;
				}
			}
			break;

		// if we are hovering over a valid item, fake a UI_SELECT with a double-click
		case ui_event::MOUSE_DOUBLE_CLICK:
			if (hover() >= 0 && hover() < item_count())
			{
				set_selected_index(hover());
				ev.iptkey = IPT_UI_SELECT;
			}

			if (is_last_selected())
			{
				ev.iptkey = IPT_UI_CANCEL;
				stack_pop();
			}
			stop = true;
			break;

		// caught scroll event
		case ui_event::MOUSE_WHEEL:
			if (hover() >= 0 && hover() < item_count() - skip_main_items - 1)
			{
				if (local_menu_event.zdelta > 0)
				{
					if (selected_index() >= m_available_items || is_first_selected() || m_ui_error)
						break;
					set_selected_index(selected_index() - local_menu_event.num_lines);
					if (selected_index() < top_line + (top_line != 0))
						top_line -= local_menu_event.num_lines;
				}
				else
				{
					if (selected_index() >= m_available_items - 1 || m_ui_error)
						break;
					set_selected_index(std::min(selected_index() + local_menu_event.num_lines, m_available_items - 1));
					if (selected_index() >= top_line + m_visible_items + (top_line != 0))
						top_line += local_menu_event.num_lines;
				}
			}
			else if (hover() == HOVER_INFO_TEXT)
			{
				if (local_menu_event.zdelta > 0)
					m_topline_datsview -= local_menu_event.num_lines;
				else
					m_topline_datsview += local_menu_event.num_lines;
			}
			break;

		// translate CHAR events into specials
		case ui_event::IME_CHAR:
			if (exclusive_input_pressed(ev.iptkey, IPT_UI_FOCUS_NEXT, 0) || exclusive_input_pressed(ev.iptkey, IPT_UI_FOCUS_PREV, 0))
			{
				stop = true;
			}
			else if (m_ui_error)
			{
				ev.iptkey = IPT_SPECIAL;
				stop = true;
			}
			else if (accept_search())
			{
				if (input_character(m_search, local_menu_event.ch, uchar_is_printable))
					search_changed = true;
			}
			break;

		case ui_event::MOUSE_RDOWN:
			if (hover() >= 0 && hover() < item_count() - skip_main_items - 1)
			{
				set_selected_index(hover());
				m_prev_selected = get_selection_ref();
				m_focus = focused_menu::MAIN;
				ev.iptkey = IPT_CUSTOM;
				ev.mouse.x0 = local_menu_event.mouse_x;
				ev.mouse.y0 = local_menu_event.mouse_y;
				stop = true;
			}
			break;

		// ignore everything else
		default:
			break;
		}

		// need to update search before processing certain kinds of events, but others don't matter
		if (search_changed)
		{
			switch (machine().ui_input().peek_event_type())
			{
			case ui_event::MOUSE_DOWN:
			case ui_event::MOUSE_RDOWN:
			case ui_event::MOUSE_DOUBLE_CLICK:
			case ui_event::MOUSE_WHEEL:
				stop = true;
				break;
			case ui_event::NONE:
			case ui_event::MOUSE_MOVE:
			case ui_event::MOUSE_LEAVE:
			case ui_event::MOUSE_UP:
			case ui_event::MOUSE_RUP:
			case ui_event::IME_CHAR:
				break;
			}
		}
	}
	if (search_changed)
		reset(reset_options::SELECT_FIRST);
}


//-------------------------------------------------
//  draw main menu
//-------------------------------------------------

void menu_select_launch::draw(uint32_t flags)
{
	bool noinput = (flags & PROCESS_NOINPUT);
	float line_height = ui().get_line_height();
	float const ud_arrow_width = line_height * machine().render().ui_aspect();
	float const gutter_width = 0.52f * ud_arrow_width;
	float const icon_offset = m_has_icons ? (1.5f * ud_arrow_width) : 0.0f;
	float right_panel_size = (ui_globals::panels_status == HIDE_BOTH || ui_globals::panels_status == HIDE_RIGHT_PANEL) ? 2.0f * ui().box_lr_border() : 0.3f;
	float visible_width = 1.0f - 4.0f * ui().box_lr_border();
	float primary_left = (1.0f - visible_width) * 0.5f;
	float primary_width = visible_width;

	draw_background();

	clear_hover();
	m_available_items = (m_is_swlist) ? item_count() - 2 : item_count() - 2 - skip_main_items;
	float extra_height = (m_is_swlist) ? 2.0f * line_height : (2.0f + skip_main_items) * line_height;
	float visible_extra_menu_height = get_customtop() + get_custombottom() + extra_height;

	// locate mouse
	if (noinput)
		ignore_mouse();
	else
		map_mouse();

	// account for extra space at the top and bottom
	float visible_main_menu_height = 1.0f - 2.0f * ui().box_tb_border() - visible_extra_menu_height;
	m_visible_lines = int(std::trunc(visible_main_menu_height / line_height));
	visible_main_menu_height = float(m_visible_lines) * line_height;

	if (!m_is_swlist)
		ui_globals::visible_main_lines = m_visible_lines;
	else
		ui_globals::visible_sw_lines = m_visible_lines;

	// compute top/left of inner menu area by centering
	float visible_left = primary_left;
	float visible_top = (1.0f - (visible_main_menu_height + visible_extra_menu_height)) * 0.5f;

	// if the menu is at the bottom of the extra, adjust
	visible_top += get_customtop();

	// compute left box size
	float x1 = visible_left - ui().box_lr_border();
	float y1 = visible_top - ui().box_tb_border();
	float x2 = x1 + 2.0f * ui().box_lr_border();
	float y2 = visible_top + visible_main_menu_height + ui().box_tb_border() + extra_height;

	// add left box
	visible_left = draw_left_panel(x1, y1, x2, y2);
	visible_width -= right_panel_size + visible_left - 2.0f * ui().box_lr_border();

	// compute and add main box
	x1 = visible_left - ui().box_lr_border();
	x2 = visible_left + visible_width + ui().box_lr_border();
	float line = visible_top + (float(m_visible_lines) * line_height);
	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	if (m_available_items < m_visible_lines)
		m_visible_lines = m_available_items;
	if (top_line < 0 || is_first_selected())
		top_line = 0;
	if (selected_index() < m_available_items && top_line + m_visible_lines >= m_available_items)
		top_line = m_available_items - m_visible_lines;

	// determine effective positions taking into account the hilighting arrows
	float effective_width = visible_width - 2.0f * gutter_width;
	float effective_left = visible_left + gutter_width;

	if ((m_focus == focused_menu::MAIN) && (selected_index() < m_available_items))
		m_prev_selected = nullptr;

	int const n_loop = (std::min)(m_visible_lines, m_available_items);
	for (int linenum = 0; linenum < n_loop; linenum++)
	{
		float line_y = visible_top + (float)linenum * line_height;
		int itemnum = top_line + linenum;
		const menu_item &pitem = item(itemnum);
		const char *itemtext = pitem.text.c_str();
		rgb_t fgcolor = ui().colors().text_color();
		rgb_t bgcolor = ui().colors().text_bg_color();
		rgb_t fgcolor3 = ui().colors().clone_color();
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line_y;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line_y + line_height;

		// set the hover if this is our item
		if (mouse_in_rect(line_x0, line_y0, line_x1, line_y1) && is_selectable(pitem))
			set_hover(itemnum);

		if (is_selected(itemnum) && m_focus == focused_menu::MAIN)
		{
			// if we're selected, draw with a different background
			fgcolor = rgb_t(0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff);
			fgcolor3 = rgb_t(0xcc, 0xcc, 0x00);
			ui().draw_textured_box(
					container(),
					line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1,
					bgcolor, rgb_t(43, 43, 43),
					hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}
		else if (itemnum == hover())
		{
			// else if the mouse is over this item, draw with a different background
			fgcolor = fgcolor3 = ui().options().mouseover_color();
			bgcolor = ui().colors().mouseover_bg_color();
			highlight(line_x0, line_y0, line_x1, line_y1, bgcolor);
		}
		else if (pitem.ref == m_prev_selected)
		{
			fgcolor = fgcolor3 = ui().options().mouseover_color();
			bgcolor = ui().colors().mouseover_bg_color();
			ui().draw_textured_box(container(), line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1, bgcolor, rgb_t(43, 43, 43),
					hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}

		if (linenum == 0 && top_line != 0)
		{
			// if we're on the top line, display the up arrow
			draw_arrow(0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
				0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height, fgcolor, ROT0);

			if (hover() == itemnum)
				set_hover(HOVER_ARROW_UP);
		}
		else if (linenum == m_visible_lines - 1 && itemnum != m_available_items - 1)
		{
			// if we're on the bottom line, display the down arrow
			draw_arrow(0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
				0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height, fgcolor, ROT0 ^ ORIENTATION_FLIP_Y);

			if (hover() == itemnum)
				set_hover(HOVER_ARROW_DOWN);
		}
		else if (pitem.type == menu_item_type::SEPARATOR)
		{
			// if we're just a divider, draw a line
			container().add_line(visible_left, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height,
					UI_LINE_WIDTH, ui().colors().text_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
		else if (pitem.subtext.empty())
		{
			// draw the item centered
			int const item_invert = pitem.flags & FLAG_INVERT;
			if (m_has_icons)
				draw_icon(linenum, item(itemnum).ref, effective_left, line_y);
			ui().draw_text_full(
					container(),
					itemtext,
					effective_left + icon_offset, line_y, effective_width - icon_offset,
					ui::text_layout::LEFT, ui::text_layout::TRUNCATE,
					mame_ui_manager::NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor,
					nullptr, nullptr);
		}
		else
		{
			int const item_invert = pitem.flags & FLAG_INVERT;
			const char *subitem_text = pitem.subtext.c_str();
			float item_width, subitem_width;

			// compute right space for subitem
			ui().draw_text_full(
					container(),
					subitem_text,
					effective_left + icon_offset, line_y, ui().get_string_width(pitem.subtext.c_str()),
					ui::text_layout::RIGHT, ui::text_layout::NEVER,
					mame_ui_manager::NONE, item_invert ? fgcolor3 : fgcolor, bgcolor,
					&subitem_width, nullptr);
			subitem_width += gutter_width;

			// draw the item left-justified
			if (m_has_icons)
				draw_icon(linenum, item(itemnum).ref, effective_left, line_y);
			ui().draw_text_full(
					container(),
					itemtext,
					effective_left + icon_offset, line_y, effective_width - icon_offset - subitem_width,
					ui::text_layout::LEFT, ui::text_layout::TRUNCATE,
					mame_ui_manager::NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor,
					&item_width, nullptr);

			// draw the subitem right-justified
			ui().draw_text_full(
					container(),
					subitem_text,
					effective_left + icon_offset + item_width, line_y, effective_width - icon_offset - item_width,
					ui::text_layout::RIGHT, ui::text_layout::NEVER,
					mame_ui_manager::NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor,
					nullptr, nullptr);
		}
	}

	for (size_t count = m_available_items; count < item_count(); count++)
	{
		const menu_item &pitem = item(count);
		const char *itemtext = pitem.text.c_str();
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line + line_height;
		rgb_t fgcolor = ui().colors().text_color();
		rgb_t bgcolor = ui().colors().text_bg_color();

		if (mouse_in_rect(line_x0, line_y0, line_x1, line_y1) && is_selectable(pitem))
			set_hover(count);

		// if we're selected, draw with a different background
		if (is_selected(count) && m_focus == focused_menu::MAIN)
		{
			fgcolor = rgb_t(0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff);
			ui().draw_textured_box(container(), line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1, bgcolor, rgb_t(43, 43, 43),
					hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}
		// else if the mouse is over this item, draw with a different background
		else if (count == hover())
		{
			fgcolor = ui().options().mouseover_color();
			bgcolor = ui().colors().mouseover_bg_color();
			highlight(line_x0, line_y0, line_x1, line_y1, bgcolor);
		}

		if (pitem.type == menu_item_type::SEPARATOR)
		{
			container().add_line(visible_left, line + 0.5f * line_height, visible_left + visible_width, line + 0.5f * line_height,
					UI_LINE_WIDTH, ui().colors().text_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
		else
		{
			ui().draw_text_full(container(), itemtext, effective_left, line, effective_width, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
					mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr);
		}
		line += line_height;
	}

	x1 = x2;
	x2 += right_panel_size;

	draw_right_panel(x1, y1, x2, y2);

	x1 = primary_left - ui().box_lr_border();
	x2 = primary_left + primary_width + ui().box_lr_border();

	// if there is something special to add, do it by calling the virtual method
	custom_render(get_selection_ref(), get_customtop(), get_custombottom(), x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	m_visible_items = m_visible_lines - (top_line != 0) - (top_line + m_visible_lines != m_available_items);

	// noinput
	if (noinput)
	{
		int alpha = (1.0f - machine().options().pause_brightness()) * 255.0f;
		if (alpha > 255)
			alpha = 255;
		if (alpha >= 0)
			container().add_rect(0.0f, 0.0f, 1.0f, 1.0f, rgb_t(alpha, 0x00, 0x00, 0x00), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}
}


//-------------------------------------------------
//  draw right panel
//-------------------------------------------------

void menu_select_launch::draw_right_panel(float origx1, float origy1, float origx2, float origy2)
{
	bool const hide((ui_globals::panels_status == HIDE_RIGHT_PANEL) || (ui_globals::panels_status == HIDE_BOTH));
	float const x2(hide ? origx2 : (origx1 + 2.0f * ui().box_lr_border()));
	float const space(x2 - origx1);
	float const lr_arrow_width(0.4f * space * machine().render().ui_aspect());

	// set left-right arrows dimension
	float const ar_x0(0.5f * (x2 + origx1) - 0.5f * lr_arrow_width);
	float const ar_y0(0.5f * (origy2 + origy1) + 0.1f * space);
	float const ar_x1(ar_x0 + lr_arrow_width);
	float const ar_y1(0.5f * (origy2 + origy1) + 0.9f * space);

	ui().draw_outlined_box(container(), origx1, origy1, origx2, origy2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

	rgb_t fgcolor(ui().colors().text_color());
	if (mouse_in_rect(origx1, origy1, x2, origy2))
	{
		fgcolor = ui().options().mouseover_color();
		set_hover(HOVER_RPANEL_ARROW);
	}

	if (hide)
	{
		draw_arrow(ar_x0, ar_y0, ar_x1, ar_y1, fgcolor, ROT90 ^ ORIENTATION_FLIP_X);
		return;
	}

	draw_arrow(ar_x0, ar_y0, ar_x1, ar_y1, fgcolor, ROT90);
	origy1 = draw_right_box_title(x2, origy1, origx2, origy2);

	if (ui_globals::rpanel == RP_IMAGES)
		arts_render(x2, origy1, origx2, origy2);
	else
		infos_render(x2, origy1, origx2, origy2);
}


//-------------------------------------------------
//  draw right box title
//-------------------------------------------------

float menu_select_launch::draw_right_box_title(float x1, float y1, float x2, float y2)
{
	auto line_height = ui().get_line_height();
	float const midl = (x2 - x1) * 0.5f;

	// add outlined box for options
	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	// add separator line
	container().add_line(x1 + midl, y1, x1 + midl, y1 + line_height, UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	std::string buffer[RP_LAST + 1];
	buffer[RP_IMAGES] = _("Images");
	buffer[RP_INFOS] = _("Infos");

	// check size
	float text_size = 1.0f;
	for (auto & elem : buffer)
	{
		auto textlen = ui().get_string_width(elem.c_str()) + 0.01f;
		float tmp_size = (textlen > midl) ? (midl / textlen) : 1.0f;
		text_size = std::min(text_size, tmp_size);
	}

	for (int cells = RP_FIRST; cells <= RP_LAST; ++cells)
	{
		rgb_t bgcolor = ui().colors().text_bg_color();
		rgb_t fgcolor = ui().colors().text_color();

		if (mouse_in_rect(x1, y1, x1 + midl, y1 + line_height))
		{
			if (ui_globals::rpanel != cells)
			{
				bgcolor = ui().colors().mouseover_bg_color();
				fgcolor = ui().options().mouseover_color();
				set_hover(HOVER_RP_FIRST + cells);
			}
		}

		if (ui_globals::rpanel != cells)
		{
			container().add_line(x1, y1 + line_height, x1 + midl, y1 + line_height, UI_LINE_WIDTH,
					ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			if (fgcolor != ui().colors().mouseover_color())
				fgcolor = ui().colors().clone_color();
		}

		if (m_focus == focused_menu::RIGHTTOP && ui_globals::rpanel == cells)
		{
			fgcolor = rgb_t(0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff);
			ui().draw_textured_box(container(), x1 + UI_LINE_WIDTH, y1 + UI_LINE_WIDTH, x1 + midl - UI_LINE_WIDTH, y1 + line_height,
					bgcolor, rgb_t(43, 43, 43), hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}
		else if (bgcolor == ui().colors().mouseover_bg_color())
		{
			container().add_rect(x1 + UI_LINE_WIDTH, y1 + UI_LINE_WIDTH, x1 + midl - UI_LINE_WIDTH, y1 + line_height,
					bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}

		ui().draw_text_full(container(), buffer[cells].c_str(), x1 + UI_LINE_WIDTH, y1, midl - UI_LINE_WIDTH,
				ui::text_layout::CENTER, ui::text_layout::NEVER, mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr, text_size);
		x1 += midl;
	}

	return (y1 + line_height + UI_LINE_WIDTH);
}


//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_select_launch::arts_render(float origx1, float origy1, float origx2, float origy2)
{
	ui_software_info const *software;
	game_driver const *driver;
	get_selection(software, driver);

	if (software && (!software->startempty || !driver))
	{
		m_cache->set_snapx_driver(nullptr);

		if (m_default_image)
			m_image_view = (software->startempty == 0) ? SNAPSHOT_VIEW : CABINETS_VIEW;

		// arts title and searchpath
		std::string const searchstr = arts_render_common(origx1, origy1, origx2, origy2);

		// loads the image if necessary
		if (!m_cache->snapx_software_is(software) || !snapx_valid() || m_switch_image)
		{
			emu_file snapfile(searchstr.c_str(), OPEN_FLAG_READ);
			bitmap_argb32 tmp_bitmap;

			if (software->startempty == 1)
			{
				// Load driver snapshot
				std::string fullname = std::string(software->driver->name) + ".png";
				render_load_png(tmp_bitmap, snapfile, nullptr, fullname.c_str());

				if (!tmp_bitmap.valid())
				{
					fullname.assign(software->driver->name).append(".jpg");
					render_load_jpeg(tmp_bitmap, snapfile, nullptr, fullname.c_str());
				}
			}
			else
			{
				// First attempt from name list
				std::string pathname = software->listname;
				std::string fullname = software->shortname + ".png";
				render_load_png(tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());

				if (!tmp_bitmap.valid())
				{
					fullname.assign(software->shortname).append(".jpg");
					render_load_jpeg(tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());
				}

				if (!tmp_bitmap.valid())
				{
					// Second attempt from driver name + part name
					pathname.assign(software->driver->name).append(software->part);
					fullname.assign(software->shortname).append(".png");
					render_load_png(tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());

					if (!tmp_bitmap.valid())
					{
						fullname.assign(software->shortname).append(".jpg");
						render_load_jpeg(tmp_bitmap, snapfile, pathname.c_str(), fullname.c_str());
					}
				}
			}

			m_cache->set_snapx_software(software);
			m_switch_image = false;
			arts_render_images(std::move(tmp_bitmap), origx1, origy1, origx2, origy2);
		}

		// if the image is available, loaded and valid, display it
		draw_snapx(origx1, origy1, origx2, origy2);
	}
	else if (driver)
	{
		m_cache->set_snapx_software(nullptr);

		if (m_default_image)
			m_image_view = ((driver->flags & machine_flags::MASK_TYPE) != machine_flags::TYPE_ARCADE) ? CABINETS_VIEW : SNAPSHOT_VIEW;

		std::string const searchstr = arts_render_common(origx1, origy1, origx2, origy2);

		// loads the image if necessary
		if (!m_cache->snapx_driver_is(driver) || !snapx_valid() || m_switch_image)
		{
			emu_file snapfile(searchstr.c_str(), OPEN_FLAG_READ);
			snapfile.set_restrict_to_mediapath(true);
			bitmap_argb32 tmp_bitmap;

			// try to load snapshot first from saved "0000.png" file
			std::string fullname(driver->name);
			render_load_png(tmp_bitmap, snapfile, fullname.c_str(), "0000.png");

			if (!tmp_bitmap.valid())
				render_load_jpeg(tmp_bitmap, snapfile, fullname.c_str(), "0000.jpg");

			// if fail, attemp to load from standard file
			if (!tmp_bitmap.valid())
			{
				fullname.assign(driver->name).append(".png");
				render_load_png(tmp_bitmap, snapfile, nullptr, fullname.c_str());

				if (!tmp_bitmap.valid())
				{
					fullname.assign(driver->name).append(".jpg");
					render_load_jpeg(tmp_bitmap, snapfile, nullptr, fullname.c_str());
				}
			}

			// if fail again, attemp to load from parent file
			if (!tmp_bitmap.valid())
			{
				// set clone status
				bool cloneof = strcmp(driver->parent, "0");
				if (cloneof)
				{
					int cx = driver_list::find(driver->parent);
					if ((cx >= 0) && (driver_list::driver(cx).flags & machine_flags::IS_BIOS_ROOT))
						cloneof = false;
				}

				if (cloneof)
				{
					fullname.assign(driver->parent).append(".png");
					render_load_png(tmp_bitmap, snapfile, nullptr, fullname.c_str());

					if (!tmp_bitmap.valid())
					{
						fullname.assign(driver->parent).append(".jpg");
						render_load_jpeg(tmp_bitmap, snapfile, nullptr, fullname.c_str());
					}
				}
			}

			m_cache->set_snapx_driver(driver);
			m_switch_image = false;
			arts_render_images(std::move(tmp_bitmap), origx1, origy1, origx2, origy2);
		}

		// if the image is available, loaded and valid, display it
		draw_snapx(origx1, origy1, origx2, origy2);
	}
}


//-------------------------------------------------
//  common function for images render
//-------------------------------------------------

std::string menu_select_launch::arts_render_common(float origx1, float origy1, float origx2, float origy2)
{
	float const line_height = ui().get_line_height();
	float const gutter_width = 0.4f * line_height * machine().render().ui_aspect() * 1.3f;

	std::string snaptext, searchstr;
	get_title_search(snaptext, searchstr);

	// apply title to right panel
	float title_size = 0.0f;
	for (int x = FIRST_VIEW; x < LAST_VIEW; x++)
	{
		float text_length;
		ui().draw_text_full(container(),
				_(arts_info[x].first), origx1, origy1, origx2 - origx1,
				ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(),
				&text_length, nullptr);
		title_size = (std::max)(text_length + 0.01f, title_size);
	}

	rgb_t const fgcolor = (m_focus == focused_menu::RIGHTBOTTOM) ? rgb_t(0xff, 0xff, 0x00) : ui().colors().text_color();
	rgb_t const bgcolor = (m_focus == focused_menu::RIGHTBOTTOM) ? rgb_t(0xff, 0xff, 0xff) : ui().colors().text_bg_color();
	float const middle = origx2 - origx1;

	// check size
	float const sc = title_size + 2.0f * gutter_width;
	float const tmp_size = (sc > middle) ? ((middle - 2.0f * gutter_width) / sc) : 1.0f;
	title_size *= tmp_size;

	if (bgcolor != ui().colors().text_bg_color())
	{
		ui().draw_textured_box(
				container(),
				origx1 + ((middle - title_size) * 0.5f), origy1 + ui().box_tb_border(),
				origx1 + ((middle + title_size) * 0.5f), origy1 + ui().box_tb_border() + line_height,
				bgcolor, rgb_t(43, 43, 43),
				hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
	}

	ui().draw_text_full(container(),
			snaptext.c_str(), origx1, origy1 + ui().box_tb_border(), origx2 - origx1,
			ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, fgcolor, bgcolor,
			nullptr, nullptr, tmp_size);

	draw_common_arrow(origx1, origy1 + ui().box_tb_border(), origx2, origy2, m_image_view, FIRST_VIEW, LAST_VIEW, title_size);

	return searchstr;
}


//-------------------------------------------------
//  perform rendering of image
//-------------------------------------------------

void menu_select_launch::arts_render_images(bitmap_argb32 &&tmp_bitmap, float origx1, float origy1, float origx2, float origy2)
{
	bool no_available = false;
	float line_height = ui().get_line_height();

	// if it fails, use the default image
	if (!tmp_bitmap.valid())
	{
		tmp_bitmap.allocate(256, 256);
		const bitmap_argb32 &src(m_cache->no_avail_bitmap());
		for (int x = 0; x < 256; x++)
		{
			for (int y = 0; y < 256; y++)
				tmp_bitmap.pix32(y, x) = src.pix32(y, x);
		}
		no_available = true;
	}

	bitmap_argb32 &snapx_bitmap(m_cache->snapx_bitmap());
	if (tmp_bitmap.valid())
	{
		float panel_width = origx2 - origx1 - 0.02f;
		float panel_height = origy2 - origy1 - 0.02f - (3.0f * ui().box_tb_border()) - (2.0f * line_height);
		int screen_width = machine().render().ui_target().width();
		int screen_height = machine().render().ui_target().height();

		if (machine().render().ui_target().orientation() & ORIENTATION_SWAP_XY)
			std::swap(screen_height, screen_width);

		int panel_width_pixel = panel_width * screen_width;
		int panel_height_pixel = panel_height * screen_height;

		// Calculate resize ratios for resizing
		auto ratioW = (float)panel_width_pixel / tmp_bitmap.width();
		auto ratioH = (float)panel_height_pixel / tmp_bitmap.height();
		auto ratioI = (float)tmp_bitmap.height() / tmp_bitmap.width();
		auto dest_xPixel = tmp_bitmap.width();
		auto dest_yPixel = tmp_bitmap.height();

		// force 4:3 ratio min
		if (ui().options().forced_4x3_snapshot() && ratioI < 0.75f && m_image_view == SNAPSHOT_VIEW)
		{
			// smaller ratio will ensure that the image fits in the view
			dest_yPixel = tmp_bitmap.width() * 0.75f;
			ratioH = (float)panel_height_pixel / dest_yPixel;
			float ratio = std::min(ratioW, ratioH);
			dest_xPixel = tmp_bitmap.width() * ratio;
			dest_yPixel *= ratio;
		}
		// resize the bitmap if necessary
		else if (ratioW < 1 || ratioH < 1 || (ui().options().enlarge_snaps() && !no_available))
		{
			// smaller ratio will ensure that the image fits in the view
			float ratio = std::min(ratioW, ratioH);
			dest_xPixel = tmp_bitmap.width() * ratio;
			dest_yPixel = tmp_bitmap.height() * ratio;
		}

		bitmap_argb32 dest_bitmap;

		// resample if necessary
		if (dest_xPixel != tmp_bitmap.width() || dest_yPixel != tmp_bitmap.height())
		{
			dest_bitmap.allocate(dest_xPixel, dest_yPixel);
			render_color color = { 1.0f, 1.0f, 1.0f, 1.0f };
			render_resample_argb_bitmap_hq(dest_bitmap, tmp_bitmap, color, true);
		}
		else
			dest_bitmap = std::move(tmp_bitmap);

		snapx_bitmap.allocate(panel_width_pixel, panel_height_pixel);
		int x1 = (0.5f * panel_width_pixel) - (0.5f * dest_xPixel);
		int y1 = (0.5f * panel_height_pixel) - (0.5f * dest_yPixel);

		for (int x = 0; x < dest_xPixel; x++)
			for (int y = 0; y < dest_yPixel; y++)
				snapx_bitmap.pix32(y + y1, x + x1) = dest_bitmap.pix32(y, x);

		// apply bitmap
		m_cache->snapx_texture()->set_bitmap(snapx_bitmap, snapx_bitmap.cliprect(), TEXFORMAT_ARGB32);
	}
	else
	{
		snapx_bitmap.reset();
	}
}


//-------------------------------------------------
//  draw snapshot
//-------------------------------------------------

void menu_select_launch::draw_snapx(float origx1, float origy1, float origx2, float origy2)
{
	// if the image is available, loaded and valid, display it
	if (snapx_valid())
	{
		float const line_height = ui().get_line_height();
		float const x1 = origx1 + 0.01f;
		float const x2 = origx2 - 0.01f;
		float const y1 = origy1 + (2.0f * ui().box_tb_border()) + line_height;
		float const y2 = origy2 - ui().box_tb_border() - line_height;

		// apply texture
		container().add_quad(x1, y1, x2, y2, rgb_t::white(), m_cache->snapx_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}
}


//-------------------------------------------------
//  get bios count
//-------------------------------------------------

bool menu_select_launch::has_multiple_bios(ui_software_info const &swinfo, s_bios &biosname)
{
	return has_multiple_bios(*swinfo.driver, biosname);
}

bool menu_select_launch::has_multiple_bios(game_driver const &driver, s_bios &biosname)
{
	if (!driver.rom)
		return false;

	char const *default_name(nullptr);
	for (tiny_rom_entry const *rom = driver.rom; !ROMENTRY_ISEND(rom); ++rom)
	{
		if (ROMENTRY_ISDEFAULT_BIOS(rom))
			default_name = rom->name;
	}

	for (romload::system_bios const &bios : romload::entries(driver.rom).get_system_bioses())
	{
		std::string name(bios.get_description());
		uint32_t const bios_flags(bios.get_value());

		if (default_name && !std::strcmp(bios.get_name(), default_name))
		{
			name.append(_(" (default)"));
			biosname.emplace(biosname.begin(), std::move(name), bios_flags - 1);
		}
		else
		{
			biosname.emplace_back(std::move(name), bios_flags - 1);
		}
	}
	return biosname.size() > 1U;
}


void menu_select_launch::exit(running_machine &machine)
{
	std::lock_guard<std::mutex> guard(s_cache_guard);
	s_caches.erase(&machine);
}


//-------------------------------------------------
//  draw collapsed left panel
//-------------------------------------------------

float menu_select_launch::draw_collapsed_left_panel(float x1, float y1, float x2, float y2)
{
	float const space = x2 - x1;
	float const lr_arrow_width = 0.4f * space * machine().render().ui_aspect();

	// set left-right arrows dimension
	float const ar_x0 = 0.5f * (x2 + x1) - (0.5f * lr_arrow_width);
	float const ar_y0 = 0.5f * (y2 + y1) + (0.1f * space);
	float const ar_x1 = ar_x0 + lr_arrow_width;
	float const ar_y1 = 0.5f * (y2 + y1) + (0.9f * space);

	ui().draw_outlined_box(container(), x1, y1, x2, y2, rgb_t(0xef, 0x12, 0x47, 0x7b)); // FIXME: magic numbers in colour?

	rgb_t fgcolor = ui().colors().text_color();
	if (mouse_in_rect(x1, y1, x2, y2))
	{
		fgcolor = ui().options().mouseover_color();
		set_hover(HOVER_LPANEL_ARROW);
	}

	draw_arrow(ar_x0, ar_y0, ar_x1, ar_y1, fgcolor, ROT90);

	return x2 + ui().box_lr_border();
}


//-------------------------------------------------
//  draw infos
//-------------------------------------------------

void menu_select_launch::infos_render(float origx1, float origy1, float origx2, float origy2)
{
	float const line_height = ui().get_line_height();
	float text_size = ui().options().infos_size();
	std::vector<int> xstart;
	std::vector<int> xend;
	const char *first = "";
	ui_software_info const *software;
	game_driver const *driver;
	int total;
	get_selection(software, driver);

	if (software && (!software->startempty || !driver))
	{
		m_info_driver = nullptr;
		first = __("Usage");

		if ((m_info_software != software) || (m_info_view != ui_globals::cur_sw_dats_view))
		{
			m_info_buffer.clear();
			if (software == m_info_software)
			{
				m_info_view = ui_globals::cur_sw_dats_view;
			}
			else
			{
				m_info_view = 0;
				m_info_software = software;
				ui_globals::cur_sw_dats_view = 0;

				m_items_list.clear();
				mame_machine_manager::instance()->lua()->call_plugin("data_list", std::string(software->shortname).append(1, ',').append(software->listname).c_str(), m_items_list);
				ui_globals::cur_sw_dats_total = m_items_list.size() + 1;
			}

			if (m_info_view == 0)
			{
				m_info_buffer = software->usage;
			}
			else
			{
				m_info_buffer.clear();
				mame_machine_manager::instance()->lua()->call_plugin("data", m_info_view - 1, m_info_buffer);
			}
		}
		total = ui_globals::cur_sw_dats_total;
	}
	else if (driver)
	{
		m_info_software = nullptr;
		first = __("General Info");

		if (driver != m_info_driver || ui_globals::curdats_view != m_info_view)
		{
			m_info_buffer.clear();
			if (driver == m_info_driver)
			{
				m_info_view = ui_globals::curdats_view;
			}
			else
			{
				m_info_driver = driver;
				m_info_view = 0;
				ui_globals::curdats_view = 0;

				m_items_list.clear();
				mame_machine_manager::instance()->lua()->call_plugin("data_list", driver->name, m_items_list);
				ui_globals::curdats_total = m_items_list.size() + 1;
			}

			if (m_info_view == 0)
				general_info(driver, m_info_buffer);
			else
			{
				m_info_buffer = "";
				mame_machine_manager::instance()->lua()->call_plugin("data", m_info_view - 1, m_info_buffer);
			}
		}
		total = ui_globals::curdats_total;
	}
	else
	{
		return;
	}

	origy1 += ui().box_tb_border();
	float gutter_width = 0.4f * line_height * machine().render().ui_aspect() * 1.3f;
	float ud_arrow_width = line_height * machine().render().ui_aspect();
	float oy1 = origy1 + line_height;

	char const *const snaptext(m_info_view ? m_items_list[m_info_view - 1].c_str() : _(first));

	// get width of widest title
	float title_size(0.0f);
	for (std::size_t x = 0; total > x; ++x)
	{
		char const *const name(x ? m_items_list[x - 1].c_str() : _(first));
		float txt_length(0.0f);
		ui().draw_text_full(
				container(), name,
				origx1, origy1, origx2 - origx1,
				ui::text_layout::CENTER, ui::text_layout::NEVER,
				mame_ui_manager::NONE, ui().colors().text_color(), ui().colors().text_bg_color(),
				&txt_length, nullptr);
		txt_length += 0.01f;
		title_size = (std::max)(txt_length, title_size);
	}

	rgb_t fgcolor = ui().colors().text_color();
	rgb_t bgcolor = ui().colors().text_bg_color();
	if (get_focus() == focused_menu::RIGHTBOTTOM)
	{
		fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
		bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
	}

	float middle = origx2 - origx1;

	// check size
	float sc = title_size + 2.0f * gutter_width;
	float tmp_size = (sc > middle) ? ((middle - 2.0f * gutter_width) / sc) : 1.0f;
	title_size *= tmp_size;

	if (bgcolor != ui().colors().text_bg_color())
	{
		ui().draw_textured_box(container(), origx1 + ((middle - title_size) * 0.5f), origy1, origx1 + ((middle + title_size) * 0.5f),
				origy1 + line_height, bgcolor, rgb_t(255, 43, 43, 43), hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
	}

	ui().draw_text_full(container(), snaptext, origx1, origy1, origx2 - origx1, ui::text_layout::CENTER,
			ui::text_layout::NEVER, mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr, tmp_size);

	char justify = 'l'; // left justify
	if ((m_info_buffer.length() >= 3) && (m_info_buffer[0] == '#'))
	{
		if (m_info_buffer[1] == 'j')
			justify = m_info_buffer[2];
	}

	draw_common_arrow(origx1, origy1, origx2, origy2, m_info_view, 0, total - 1, title_size);
	if (justify == 'f')
	{
		m_total_lines = ui().wrap_text(
				container(), m_info_buffer.c_str(),
				0.0f, 0.0f, 1.0f - (2.0f * gutter_width),
				xstart, xend,
				text_size);
	}
	else
	{
		m_total_lines = ui().wrap_text(
				container(), m_info_buffer.c_str(),
				origx1, origy1, origx2 - origx1 - (2.0f * gutter_width),
				xstart, xend,
				text_size);
	}

	int r_visible_lines = floor((origy2 - oy1) / (line_height * text_size));
	if (m_total_lines < r_visible_lines)
		r_visible_lines = m_total_lines;
	if (m_topline_datsview < 0)
		m_topline_datsview = 0;
	if (m_topline_datsview + r_visible_lines >= m_total_lines)
		m_topline_datsview = m_total_lines - r_visible_lines;

	if (mouse_in_rect(origx1 + gutter_width, oy1, origx2 - gutter_width, origy2))
		set_hover(HOVER_INFO_TEXT);

	sc = origx2 - origx1 - (2.0f * gutter_width);
	for (int r = 0; r < r_visible_lines; ++r)
	{
		int itemline = r + m_topline_datsview;
		std::string const tempbuf(m_info_buffer.substr(xstart[itemline], xend[itemline] - xstart[itemline]));
		if (tempbuf[0] == '#')
			continue;

		if (r == 0 && m_topline_datsview != 0) // up arrow
		{
			draw_info_arrow(0, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
		}
		else if (r == r_visible_lines - 1 && itemline != m_total_lines - 1) // bottom arrow
		{
			draw_info_arrow(1, origx1, origx2, oy1, line_height, text_size, ud_arrow_width);
		}
		else if (justify == '2') // two-column layout
		{
			// split at first tab
			std::string::size_type const splitpos(tempbuf.find('\t'));
			std::string const leftcol(tempbuf.substr(0, (std::string::npos == splitpos) ? 0U : splitpos));
			std::string const rightcol(tempbuf.substr((std::string::npos == splitpos) ? 0U : (splitpos + 1U)));

			// measure space needed, condense if necessary
			float const leftlen(ui().get_string_width(leftcol.c_str(), text_size));
			float const rightlen(ui().get_string_width(rightcol.c_str(), text_size));
			float const textlen(leftlen + rightlen);
			float const tmp_size3((textlen > sc) ? (text_size * (sc / textlen)) : text_size);

			// draw in two parts
			ui().draw_text_full(
					container(), leftcol.c_str(),
					origx1 + gutter_width, oy1, sc,
					ui::text_layout::LEFT, ui::text_layout::TRUNCATE,
					mame_ui_manager::NORMAL, ui().colors().text_color(), ui().colors().text_bg_color(),
					nullptr, nullptr,
					tmp_size3);
			ui().draw_text_full(
					container(), rightcol.c_str(),
					origx1 + gutter_width, oy1, sc,
					ui::text_layout::RIGHT, ui::text_layout::TRUNCATE,
					mame_ui_manager::NORMAL, ui().colors().text_color(), ui().colors().text_bg_color(),
					nullptr, nullptr,
					tmp_size3);
		}
		else if (justify == 'f' || justify == 'p') // full or partial justify
		{
			// check size
			float const textlen = ui().get_string_width(tempbuf.c_str(), text_size);
			float tmp_size3 = (textlen > sc) ? text_size * (sc / textlen) : text_size;
			ui().draw_text_full(
					container(), tempbuf.c_str(),
					origx1 + gutter_width, oy1, origx2 - origx1,
					ui::text_layout::LEFT, ui::text_layout::TRUNCATE,
					mame_ui_manager::NORMAL, ui().colors().text_color(), ui().colors().text_bg_color(),
					nullptr, nullptr,
					tmp_size3);
		}
		else
		{
			ui().draw_text_full(
					container(), tempbuf.c_str(),
					origx1 + gutter_width, oy1, origx2 - origx1,
					ui::text_layout::LEFT, ui::text_layout::TRUNCATE,
					mame_ui_manager::NORMAL, ui().colors().text_color(), ui().colors().text_bg_color(),
					nullptr, nullptr,
					text_size);
		}

		oy1 += (line_height * text_size);
	}
	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	m_right_visible_lines = r_visible_lines - (m_topline_datsview != 0) - (m_topline_datsview + r_visible_lines != m_total_lines);
}

} // namespace ui
