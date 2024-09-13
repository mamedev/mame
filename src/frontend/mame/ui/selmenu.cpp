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

#include "audit.h"
#include "cheat.h"
#include "infoxml.h"
#include "mame.h"
#include "mameopts.h"

#include "corestr.h"
#include "drivenum.h"
#include "emuopts.h"
#include "fileio.h"
#include "rendfont.h"
#include "rendutil.h"
#include "romload.h"
#include "softlist.h"
#include "softlist_dev.h"
#include "uiinput.h"
#include "luaengine.h"

#include "util/nanosvg.h"
#include "util/path.h"

#include <algorithm>
#include <cmath>
#include <cstring>


// these hold static bitmap images
#include "ui/defimg.ipp"
#include "ui/toolbar.ipp"


namespace ui {

namespace {

std::pair<char const *, char const *> RIGHT_PANEL_NAMES[RP_LAST + 1] = {
		{ "image", N_("Images") },
		{ "info",  N_("Info") } };

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

std::tuple<char const *, char const *, char const *> const ARTS_INFO[] =
{
	{ "snap",       N_p("selmenu-artwork", "Snapshots"),       OPTION_SNAPSHOT_DIRECTORY },
	{ "cabinet",    N_p("selmenu-artwork", "Cabinet"),         OPTION_CABINETS_PATH },
	{ "cpanel",     N_p("selmenu-artwork", "Control Panel"),   OPTION_CPANELS_PATH },
	{ "pcb",        N_p("selmenu-artwork", "PCB"),             OPTION_PCBS_PATH },
	{ "flyer",      N_p("selmenu-artwork", "Flyer"),           OPTION_FLYERS_PATH },
	{ "title",      N_p("selmenu-artwork", "Title Screen"),    OPTION_TITLES_PATH },
	{ "ending",     N_p("selmenu-artwork", "Ending"),          OPTION_ENDS_PATH },
	{ "artpreview", N_p("selmenu-artwork", "Artwork Preview"), OPTION_ARTPREV_PATH },
	{ "boss",       N_p("selmenu-artwork", "Bosses"),          OPTION_BOSSES_PATH },
	{ "logo",       N_p("selmenu-artwork", "Logo"),            OPTION_LOGOS_PATH },
	{ "versus",     N_p("selmenu-artwork", "Versus"),          OPTION_VERSUS_PATH },
	{ "gameover",   N_p("selmenu-artwork", "Game Over"),       OPTION_GAMEOVER_PATH },
	{ "howto",      N_p("selmenu-artwork", "HowTo"),           OPTION_HOWTO_PATH },
	{ "scores",     N_p("selmenu-artwork", "Scores"),          OPTION_SCORES_PATH },
	{ "select",     N_p("selmenu-artwork", "Select"),          OPTION_SELECT_PATH },
	{ "marquee",    N_p("selmenu-artwork", "Marquee"),         OPTION_MARQUEES_PATH },
	{ "cover",      N_p("selmenu-artwork", "Covers"),          OPTION_COVER_PATH },
};

char const *const hover_msg[] = {
	N_("Add or remove favorite"),
	N_("Export displayed list to file"),
	N_("Audit media"),
	N_("Show DATs view"),
};

std::tuple<unsigned, int, bool> SYS_TOOLBAR_BITMAPS[] = {
	{ TOOLBAR_BITMAP_FAVORITE, IPT_UI_FAVORITES, true  },
	{ TOOLBAR_BITMAP_SAVE,     IPT_UI_EXPORT,    false },
	{ TOOLBAR_BITMAP_AUDIT,    IPT_UI_AUDIT,     false },
	{ TOOLBAR_BITMAP_INFO,     IPT_UI_DATS,      true  }
};

std::tuple<unsigned, int, bool> SW_TOOLBAR_BITMAPS[] = {
	{ TOOLBAR_BITMAP_FAVORITE, IPT_UI_FAVORITES, true  },
	{ TOOLBAR_BITMAP_INFO,     IPT_UI_DATS,      true  }
};


void load_image(bitmap_argb32 &bitmap, emu_file &file, std::string const &base)
{
	if (!file.open(base + ".png"))
	{
		render_load_png(bitmap, file);
		file.close();
	}

	if (!bitmap.valid() && !file.open(base + ".jpg"))
	{
		render_load_jpeg(bitmap, file);
		file.close();
	}

	if (!bitmap.valid() && !file.open(base + ".bmp"))
	{
		render_load_msdib(bitmap, file);
		file.close();
	}
}


void load_driver_image(bitmap_argb32 &bitmap, emu_file &file, game_driver const &driver)
{
	// try to load snapshot first from saved "0000.png" file
	std::string fullname = driver.name;
	load_image(bitmap, file, util::path_concat(fullname, "0000"));

	// if fail, attempt to load from standard file
	if (!bitmap.valid())
		load_image(bitmap, file, fullname);

	// if fail again, attempt to load from parent file
	if (!bitmap.valid())
	{
		// ignore BIOS sets
		bool isclone = strcmp(driver.parent, "0") != 0;
		if (isclone)
		{
			int const cx = driver_list::find(driver.parent);
			if ((0 <= cx) && (driver_list::driver(cx).flags & machine_flags::IS_BIOS_ROOT))
				isclone = false;
		}

		if (isclone)
		{
			fullname = driver.parent;
			load_image(bitmap, file, util::path_concat(fullname, "0000"));

			if (!bitmap.valid())
				load_image(bitmap, file, fullname);
		}
	}
}

} // anonymous namespace


class menu_select_launch::software_parts : public menu
{
public:
	software_parts(mame_ui_manager &mui, render_container &container, s_parts &&parts, ui_software_info const &ui_info);
	virtual ~software_parts() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	ui_software_info const &m_uiinfo;
	s_parts const          m_parts;
};

class menu_select_launch::bios_selection : public menu
{
public:
	bios_selection(mame_ui_manager &mui, render_container &container, s_bios &&biosname, game_driver const &driver, bool inlist);
	bios_selection(mame_ui_manager &mui, render_container &container, s_bios &&biosname, ui_software_info const &swinfo, bool inlist);
	virtual ~bios_selection() override;

private:
	bios_selection(mame_ui_manager &mui, render_container &container, s_bios &&biosname, void const *driver, bool software, bool inlist);

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	void const  *m_driver;
	bool        m_software, m_inlist;
	s_bios      m_bios;
};

std::string menu_select_launch::reselect_last::s_driver;
std::string menu_select_launch::reselect_last::s_software;
std::string menu_select_launch::reselect_last::s_swlist;
bool menu_select_launch::reselect_last::s_reselect = false;

// instantiate possible variants of these so derived classes don't get link errors
template bool menu_select_launch::select_bios(game_driver const &, bool);
template bool menu_select_launch::select_bios(ui_software_info const &, bool);
template void menu_select_launch::draw_left_panel<machine_filter>(u32 flags, machine_filter::type current, std::map<machine_filter::type, machine_filter::ptr> const &filters);
template void menu_select_launch::draw_left_panel<software_filter>(u32 flags, software_filter::type current, std::map<software_filter::type, software_filter::ptr> const &filters);


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
	set_heading(_("Select Software Package Part"));
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

void menu_select_launch::software_parts::populate()
{
	std::vector<s_parts::const_iterator> parts;
	parts.reserve(m_parts.size());
	for (s_parts::const_iterator it = m_parts.begin(); m_parts.end() != it; ++it)
		parts.push_back(it);
	std::sort(parts.begin(), parts.end(), [] (auto const &left, auto const &right) { return 0 > core_stricmp(left->first, right->first); });
	for (auto const &elem : parts)
		item_append(elem->first, elem->second, 0, (void *)&*elem);

	item_append(menu_item_type::SEPARATOR);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_select_launch::software_parts::handle(event const *ev)
{
	if (ev && (ev->iptkey == IPT_UI_SELECT) && ev->itemref)
	{
		for (auto const &elem : m_parts)
		{
			if ((void*)&elem == ev->itemref)
			{
				launch_system(ui(), *m_uiinfo.driver, &m_uiinfo, &elem.first, nullptr);
				break;
			}
		}
	}

	return false;
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
	set_heading(_("Select System BIOS"));
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

void menu_select_launch::bios_selection::populate()
{
	for (auto &elem : m_bios)
		item_append(elem.first, 0, (void *)&elem.first);

	item_append(menu_item_type::SEPARATOR);
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_select_launch::bios_selection::handle(event const *ev)
{
	if (ev && (ev->iptkey == IPT_UI_SELECT) && ev->itemref)
	{
		for (auto & elem : m_bios)
		{
			if ((void*)&elem.first == ev->itemref)
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
					software_list_device *swlist = software_list_device::find_by_name(*drivlist.config(), ui_swinfo->listname);
					const software_info *swinfo = swlist->find(ui_swinfo->shortname);
					if (!select_part(ui(), container(), *swinfo, *ui_swinfo))
					{
						reselect_last::reselect(true);
						launch_system(ui(), drivlist.driver(), ui_swinfo, nullptr, &elem.second);
					}
				}
			}
		}
	}

	return false;
}


menu_select_launch::cache::cache(running_machine &machine)
	: m_snapx_bitmap(std::make_unique<bitmap_argb32>(0, 0))
	, m_snapx_texture(nullptr, machine.render())
	, m_snapx_driver(nullptr)
	, m_snapx_software(nullptr)
	, m_no_avail_bitmap(256, 256)
	, m_toolbar_bitmaps()
	, m_toolbar_textures()
{
	render_manager &render(machine.render());

	// create a texture for snapshot
	m_snapx_texture.reset(render.texture_alloc(render_texture::hq_scale));

	std::memcpy(&m_no_avail_bitmap.pix(0), no_avail_bmp, 256 * 256 * sizeof(u32));

	m_toolbar_bitmaps.resize(UI_TOOLBAR_BUTTONS * 2);
	m_toolbar_textures.reserve(UI_TOOLBAR_BUTTONS * 2);
}


menu_select_launch::cache::~cache()
{
}


void menu_select_launch::cache::cache_toolbar(running_machine &machine, float width, float height)
{
	// not bothering to transform for non-square pixels greatly simplifies this
	render_manager &render(machine.render());
	render_target const &target(render.ui_target());
	s32 const pix_size(std::ceil(std::max(width * target.width(), height * target.height())));
	if (m_toolbar_textures.empty() || (m_toolbar_bitmaps[0].width() != pix_size) || (m_toolbar_bitmaps[0].height() != pix_size))
	{
		m_toolbar_textures.clear();
		util::nsvg_rasterizer_ptr rasterizer(nsvgCreateRasterizer());
		std::string xml;
		for (unsigned i = 0; UI_TOOLBAR_BUTTONS > i; ++i)
		{
			// parse SVG and calculate scale
			xml = toolbar_icons_svg[i];
			util::nsvg_image_ptr svg(nsvgParse(xml.data(), "px", 72));
			float const xscale(float(pix_size) / svg->width);
			float const yscale(float(pix_size) / svg->height);
			float const drawscale((std::max)(xscale, yscale));

			// rasterise the SVG and clear it out of memory
			bitmap_argb32 &bitmap(m_toolbar_bitmaps[2 * i]);
			bitmap.resize(pix_size, pix_size);
			nsvgRasterize(
					rasterizer.get(),
					svg.get(),
					0, 0, drawscale,
					reinterpret_cast<unsigned char *>(&bitmap.pix(0)),
					pix_size, pix_size,
					bitmap.rowbytes());
			svg.reset();

			// correct colour format
			bitmap_argb32 &disabled_bitmap(m_toolbar_bitmaps[(2 * i) + 1]);
			disabled_bitmap.resize(pix_size, pix_size);
			for (s32 y = 0; bitmap.height() > y; ++y)
			{
				u32 *cdst(&bitmap.pix(y));
				u32 *mdst(&disabled_bitmap.pix(y));
				for (s32 x = 0; bitmap.width() > x; ++x, ++cdst, ++mdst)
				{
					u8 const *const src(reinterpret_cast<u8 const *>(cdst));
					rgb_t const c(src[3], src[0], src[1], src[2]);
					u8 const l(std::clamp(std::lround((0.2126 * src[0]) + (0.7152 * src[1]) + (0.0722 * src[2])), 0L, 255L));
					rgb_t const m(src[3], l, l, l);
					*cdst = c;
					*mdst = m;
				}
			}

			// make textures
			render_texture &texture(*m_toolbar_textures.emplace_back(render.texture_alloc(), render));
			render_texture &disabled_texture(*m_toolbar_textures.emplace_back(render.texture_alloc(), render));
			texture.set_bitmap(bitmap, bitmap.cliprect(), TEXFORMAT_ARGB32);
			disabled_texture.set_bitmap(disabled_bitmap, disabled_bitmap.cliprect(), TEXFORMAT_ARGB32);
		}
	}
}


menu_select_launch::~menu_select_launch()
{
}


menu_select_launch::menu_select_launch(mame_ui_manager &mui, render_container &container, bool is_swlist)
	: menu(mui, container)
	, m_skip_main_items(0)
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
	, m_info_layout()
	, m_icon_width(0)
	, m_icon_height(0)
	, m_divider_width(0.0F)
	, m_divider_arrow_width(0.0F)
	, m_divider_arrow_height(0.0F)
	, m_info_line_height(0.0F)
	, m_cache(mui.get_session_data<menu_select_launch, cache_wrapper>(machine()))
	, m_is_swlist(is_swlist)
	, m_focus(focused_menu::MAIN)
	, m_pointer_action(pointer_action::NONE)
	, m_scroll_repeat(std::chrono::steady_clock::time_point::min())
	, m_base_pointer(0.0F, 0.0F)
	, m_last_pointer(0.0F, 0.0F)
	, m_clicked_line(0)
	, m_wheel_target(focused_menu::MAIN)
	, m_wheel_movement(0)
	, m_primary_vbounds(0.0F, -1.0F)
	, m_primary_items_top(-1.0F)
	, m_primary_items_hbounds(0.0F, -1.0F)
	, m_primary_lines(0)
	, m_left_panel_width(-1.0F)
	, m_left_items_hbounds(0.0F, -1.0F)
	, m_left_items_top(1.0F)
	, m_left_item_count(0)
	, m_left_visible_lines(0)
	, m_left_visible_top(0)
	, m_right_panel_width(-1.0F)
	, m_right_tabs_bottom(-1.0F)
	, m_right_heading_top(-1.0F)
	, m_right_content_vbounds(0.0F, -1.0F)
	, m_right_content_hbounds(0.0F, -1.0F)
	, m_right_visible_lines(0)
	, m_toolbar_button_vbounds(0.0F, -1.0F)
	, m_toolbar_button_width(-1.0)
	, m_toolbar_button_spacing(-1.0)
	, m_toolbar_backtrack_left(-1.0)
	, m_toolbar_main_left(-1.0)
	, m_panels_status(SHOW_PANELS)
	, m_right_panel(RP_FIRST)
	, m_has_icons(false)
	, m_switch_image(false)
	, m_image_view(FIRST_VIEW)
	, m_flags(256)
{
	set_needs_prev_menu_item(false);
	set_process_flags(PROCESS_LR_REPEAT);
}


std::pair<bool, bool> menu_select_launch::next_right_panel_view()
{
	if (right_panel() == RP_IMAGES)
		return next_image_view();
	else if (right_panel() == RP_INFOS)
		return next_info_view();
	else
		return std::make_pair(false, false);
}


std::pair<bool, bool> menu_select_launch::previous_right_panel_view()
{
	if (right_panel() == RP_IMAGES)
		return previous_image_view();
	else if (right_panel() == RP_INFOS)
		return previous_info_view();
	else
		return std::make_pair(false, false);
}


std::pair<bool, bool> menu_select_launch::next_image_view()
{
	if (LAST_VIEW > m_image_view)
	{
		++m_image_view;
		set_switch_image();
		return std::make_pair(true, (LAST_VIEW > m_image_view));
	}
	else
	{
		return std::make_pair(false, false);
	}
}


std::pair<bool, bool> menu_select_launch::previous_image_view()
{
	if (FIRST_VIEW < m_image_view)
	{
		--m_image_view;
		set_switch_image();
		return std::make_pair(true, (FIRST_VIEW < m_image_view));
	}
	else
	{
		return std::make_pair(false, false);
	}
}


std::pair<bool, bool> menu_select_launch::next_info_view()
{
	ui_software_info const *software;
	ui_system_info const *system;
	get_selection(software, system);
	if (software && !software->startempty)
	{
		if ((ui_globals::cur_sw_dats_total - 1) > ui_globals::cur_sw_dats_view)
		{
			++ui_globals::cur_sw_dats_view;
			m_topline_datsview = 0;
			return std::make_pair(true, (ui_globals::cur_sw_dats_total - 1) > ui_globals::cur_sw_dats_view);
		}
	}
	else if (system || (software && software->driver))
	{
		if ((ui_globals::curdats_total - 1) > ui_globals::curdats_view)
		{
			++ui_globals::curdats_view;
			m_topline_datsview = 0;
			return std::make_pair(true, (ui_globals::curdats_total - 1) > ui_globals::curdats_view);
		}
	}
	return std::make_pair(false, false);
}


std::pair<bool, bool> menu_select_launch::previous_info_view()
{
	ui_software_info const *software;
	ui_system_info const *system;
	get_selection(software, system);
	if (software && !software->startempty)
	{
		if (0 < ui_globals::cur_sw_dats_view)
		{
			--ui_globals::cur_sw_dats_view;
			m_topline_datsview = 0;
			return std::make_pair(true, 0 < ui_globals::cur_sw_dats_view);
		}
	}
	else if (system || (software && software->driver))
	{
		if (0 < ui_globals::curdats_view)
		{
			--ui_globals::curdats_view;
			m_topline_datsview = 0;
			return std::make_pair(true, 0 < ui_globals::curdats_view);
		}
	}
	return std::make_pair(false, false);
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

			moptions.set_value(OPTION_SNAPNAME, util::path_concat(swinfo->listname, swinfo->shortname), OPTION_PRIORITY_CMDLINE);
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
	stack_reset(mui);
}


//-------------------------------------------------
//  recompute metrics
//-------------------------------------------------

void menu_select_launch::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	// calculate icon size in pixels
	render_target const &target(machine().render().ui_target());
	bool const rotated((target.orientation() & ORIENTATION_SWAP_XY) != 0);
	m_icon_width = int((rotated ? height : width) * line_height() * aspect);
	m_icon_height = int((rotated ? width : height) * line_height());

	// force info text to be laid out again
	m_info_layout = std::nullopt;

	// calculate size of dividers between panes
	m_divider_width = 0.8F * line_height() * x_aspect();
	m_divider_arrow_width = 0.32F * line_height() * x_aspect();
	m_divider_arrow_height = 0.64F * line_height();

	// calculate info text size
	m_info_line_height = ui().get_line_height(ui().options().infos_size());

	// invalidate panel metrics
	m_primary_vbounds = std::make_pair(0.0F, -1.0F);
	m_primary_items_hbounds = std::make_pair(0.0F, -1.0F);
	m_left_panel_width = -1.0F;
	m_left_items_hbounds = std::make_pair(0.0F, -1.0F);
	m_right_panel_width = -1.0F;
	m_right_heading_top = -1.0F;
	m_right_content_vbounds = std::make_pair(0.0F, -1.0F);
	m_right_content_hbounds = std::make_pair(0.0F, -1.0F);
	m_toolbar_button_vbounds = std::make_pair(0.0F, -1.0F);

	// abandon pointer input
	m_pointer_action = pointer_action::NONE;

	// force right panel images to be redrawn
	m_cache.set_snapx_driver(nullptr);
	m_cache.set_snapx_software(nullptr);
}


//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_select_launch::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	std::string tempbuf[4];

	// determine the text for the header
	make_topbox_text(tempbuf[0], tempbuf[1], tempbuf[2]);
	float const y1 = origy1 - 3.0f * tb_border() - line_height();
	draw_text_box(
			tempbuf, tempbuf + 3,
			origx1, origx2, origy1 - top, y1,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, true,
			ui().colors().text_color(), ui().colors().background_color());

	// draw toolbar
	draw_toolbar(flags, origx1, y1, origx2, origy1 - tb_border());

	// determine the text to render below
	ui_software_info const *swinfo;
	ui_system_info const *system;
	get_selection(swinfo, system);

	bool isstar = false;
	rgb_t color = ui().colors().background_color();
	if (swinfo && !swinfo->startempty)
	{
		isstar = mame_machine_manager::instance()->favorite().is_favorite_system_software(*swinfo);

		// first line is long name or system
		tempbuf[0] = make_software_description(*swinfo, system);

		// next line is year, publisher
		tempbuf[1] = string_format(_("%1$s, %2$s"), swinfo->year, swinfo->publisher);

		// next line is parent/clone
		if (!swinfo->parentname.empty())
			tempbuf[2] = string_format(_("Software is clone of: %1$s"), !swinfo->parentlongname.empty() ? swinfo->parentlongname : swinfo->parentname);
		else
			tempbuf[2] = _("Software is parent");

		// next line is supported status
		if (swinfo->supported == software_support::UNSUPPORTED)
		{
			tempbuf[3] = _("Supported: No");
			color = UI_RED_COLOR;
		}
		else if (swinfo->supported == software_support::PARTIALLY_SUPPORTED)
		{
			tempbuf[3] = _("Supported: Partial");
			color = UI_YELLOW_COLOR;
		}
		else
		{
			tempbuf[3] = _("Supported: Yes");
			color = UI_GREEN_COLOR;
		}
	}
	else if (system || (swinfo && swinfo->driver))
	{
		game_driver const &driver(system ? *system->driver : *swinfo->driver);
		isstar = mame_machine_manager::instance()->favorite().is_favorite_system(driver);

		// first line is year, manufacturer
		tempbuf[0] = string_format(_("%1$s, %2$s"), driver.year, driver.manufacturer);

		// next line is clone/parent status
		int cloneof = driver_list::non_bios_clone(driver);

		if (0 > cloneof)
			tempbuf[1] = _("System is parent");
		else if (system)
			tempbuf[1] = string_format(_("System is clone of: %1$s"), system->parent);
		else
			tempbuf[1] = string_format(_("System is clone of: %1$s"), driver_list::driver(cloneof).type.fullname());

		// next line is overall driver status
		system_flags const &flags(get_system_flags(driver));
		if (flags.machine_flags() & machine_flags::NOT_WORKING)
			tempbuf[2] = _("Status: NOT WORKING");
		else if ((flags.unemulated_features() | flags.imperfect_features()) & device_t::feature::PROTECTION)
			tempbuf[2] = _("Status: Unemulated Protection");
		else
			tempbuf[2] = _("Status: Working");

		// next line is graphics, sound status
		if (flags.unemulated_features() & device_t::feature::GRAPHICS)
			tempbuf[3] = _("Graphics: Unimplemented, ");
		else if ((flags.unemulated_features() | flags.imperfect_features()) & (device_t::feature::GRAPHICS | device_t::feature::PALETTE))
			tempbuf[3] = _("Graphics: Imperfect, ");
		else
			tempbuf[3] = _("Graphics: OK, ");

		if (driver.flags & machine_flags::NO_SOUND_HW)
			tempbuf[3].append(_("Sound: None"));
		else if (flags.unemulated_features() & device_t::feature::SOUND)
			tempbuf[3].append(_("Sound: Unimplemented"));
		else if (flags.imperfect_features() & device_t::feature::SOUND)
			tempbuf[3].append(_("Sound: Imperfect"));
		else
			tempbuf[3].append(_("Sound: OK"));

		color = flags.status_color();
	}
	else
	{
		std::string_view copyright(emulator_info::get_copyright());
		unsigned line(0);

		// first line is version string
		tempbuf[line++] = string_format("%s %s", emulator_info::get_appname(), build_version);

		// output message
		while (line < std::size(tempbuf))
		{
			auto const found = copyright.find('\n');
			if (std::string::npos != found)
			{
				tempbuf[line++] = copyright.substr(0, found);
				copyright.remove_prefix(found + 1);
			}
			else
			{
				tempbuf[line++] = copyright;
				copyright = std::string_view();
			}
		}
	}

	// draw the footer
	draw_text_box(
			std::begin(tempbuf), std::end(tempbuf),
			origx1, origx2, origy2 + tb_border(), origy2 + bottom,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, true,
			ui().colors().text_color(), color);

	// is favorite? draw the star
	if (isstar)
		draw_star(origx1 + lr_border(), origy2 + (2.0f * tb_border()));
}


void menu_select_launch::menu_activated()
{
	m_panels_status = ui().options().hide_panels();
	m_wheel_target = focused_menu::MAIN;
	m_wheel_movement = 0;
}


void menu_select_launch::menu_deactivated()
{
	ui().options().set_value(OPTION_HIDE_PANELS, m_panels_status, OPTION_PRIORITY_CMDLINE);
}


void menu_select_launch::rotate_focus(int dir)
{
	switch (get_focus())
	{
	case focused_menu::MAIN:
		if (selected_index() >= m_available_items)
		{
			if ((m_panels_status == HIDE_BOTH) || ((0 > dir) && m_available_items))
				select_prev();
			else if (0 > dir)
				set_focus((m_panels_status == HIDE_RIGHT_PANEL) ? focused_menu::LEFT : focused_menu::RIGHTBOTTOM);
			else
				set_focus((m_panels_status == HIDE_LEFT_PANEL) ? focused_menu::RIGHTTOP : focused_menu::LEFT);
		}
		else if (m_skip_main_items || (m_panels_status != HIDE_BOTH))
		{
			m_prev_selected = get_selection_ref();
			if (0 < dir)
			{
				if (m_skip_main_items)
					set_selected_index(m_available_items + 1);
				else if (m_panels_status == HIDE_LEFT_PANEL)
					set_focus(focused_menu::RIGHTTOP);
				else
					set_focus(focused_menu::LEFT);
			}
			else if (m_panels_status == HIDE_RIGHT_PANEL)
			{
				set_focus(focused_menu::LEFT);
			}
			else if (m_panels_status != HIDE_BOTH)
			{
				set_focus(focused_menu::RIGHTBOTTOM);
			}
			else
			{
				set_selected_index(m_available_items + 1);
			}
		}
		break;

	case focused_menu::LEFT:
		if (0 > dir)
		{
			set_focus(focused_menu::MAIN);
			if (m_skip_main_items)
				set_selected_index(m_available_items + 1);
			else
				select_prev();
		}
		else if (m_panels_status != HIDE_RIGHT_PANEL)
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
		else if (m_panels_status != HIDE_LEFT_PANEL)
		{
			set_focus(focused_menu::LEFT);
		}
		else
		{
			set_focus(focused_menu::MAIN);
			if (m_skip_main_items)
				set_selected_index(m_available_items + 1);
			else
				select_prev();
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
	ui_system_info const *system;
	get_selection(software, system);
	if (software && !software->startempty)
		menu::stack_push<menu_dats_view>(ui(), container(), *software);
	else if (system)
		menu::stack_push<menu_dats_view>(ui(), container(), system);
}


//-------------------------------------------------
//  draw info arrow
//-------------------------------------------------

void menu_select_launch::draw_info_arrow(u32 flags, int line)
{
	float const linetop(m_right_content_vbounds.first + (float(line) * m_info_line_height));
	float const linebottom(m_right_content_vbounds.first + (float(line + 1) * m_info_line_height));
	bool const hovered(pointer_in_rect(m_right_content_hbounds.first, linetop, m_right_content_hbounds.second, linebottom));
	bool const clicked((pointer_action::RIGHT_TRACK_LINE == m_pointer_action) && (line == m_clicked_line));

	rgb_t bgcolor = ui().colors().text_bg_color();
	rgb_t fgcolor = ui().colors().text_color();
	if (clicked && hovered)
	{
		// draw selected highlight for tracked item
		bgcolor = ui().colors().selected_bg_color();
		fgcolor = ui().colors().selected_color();
		highlight(m_right_content_hbounds.first, linetop, m_right_content_hbounds.second, linebottom, bgcolor);
	}
	else if (clicked || (!m_ui_error && !(flags & PROCESS_NOINPUT) && hovered && pointer_idle()))
	{
		// draw hover highlight when hovered over or dragged off
		bgcolor = ui().colors().mouseover_bg_color();
		fgcolor = ui().colors().mouseover_color();
		highlight(m_right_content_hbounds.first, linetop, m_right_content_hbounds.second, linebottom, bgcolor);
	}

	draw_arrow(
			0.5F * (m_right_content_hbounds.first + m_right_content_hbounds.second - (x_aspect() * m_info_line_height)),
			linetop + (0.25F * m_info_line_height),
			0.5F * (m_right_content_hbounds.first + m_right_content_hbounds.second + (x_aspect() * m_info_line_height)),
			linetop + (0.75F * m_info_line_height),
			fgcolor,
			line ? (ROT0 ^ ORIENTATION_FLIP_Y) : ROT0);
}


//-------------------------------------------------
//  draw vertical divider
//-------------------------------------------------

void menu_select_launch::draw_divider(u32 flags, float x1, bool right)
{
	// work out colours
	rgb_t fgcolor = ui().colors().text_color();
	rgb_t bgcolor(0xef, 0x12, 0x47, 0x7b); // FIXME: magic numbers in colour?
	bool const hovered(pointer_in_rect(x1, m_primary_vbounds.first, x1 + m_divider_width, m_primary_vbounds.second));
	bool const clicked((pointer_action::DIVIDER_TRACK == m_pointer_action) && ((right ? 1 : 0) == m_clicked_line));
	if (hovered && clicked)
	{
		fgcolor = ui().colors().selected_color();
		bgcolor = ui().colors().selected_bg_color();
	}
	else if (clicked || (!m_ui_error && !(flags & PROCESS_NOINPUT) && hovered))
	{
		fgcolor = ui().colors().mouseover_color();
	}

	// draw the divider pane
	ui().draw_outlined_box(container(), x1, m_primary_vbounds.first, x1 + m_divider_width, m_primary_vbounds.second, bgcolor);

	// draw the arrow
	uint32_t orientation(ROT90);
	if (right ? !show_right_panel() : show_left_panel())
		orientation ^= ORIENTATION_FLIP_X;
	float const ar_x0 = x1 + (0.5F * (m_divider_width - m_divider_arrow_width));
	float const ar_y0 = 0.5F * (m_primary_vbounds.first + m_primary_vbounds.second - m_divider_arrow_height);
	draw_arrow(ar_x0, ar_y0, ar_x0 + m_divider_arrow_width, ar_y0 + m_divider_arrow_height, fgcolor, orientation);
}



//-------------------------------------------------
//  draw left panel (filter list)
//-------------------------------------------------

template <typename Filter>
void menu_select_launch::draw_left_panel(u32 flags, typename Filter::type current, std::map<typename Filter::type, typename Filter::ptr> const &filters)
{
	if (!show_left_panel())
	{
		// left panel hidden, but no need to recompute metrics if target isn't resized
		m_left_panel_width = 0.0F;

		draw_divider(flags, lr_border(), false);
		return;
	}

	// get the width of the selection indicator glyphs
	float const checkmark_width = ui().get_string_width(convert_command_glyph("_# "), m_info_line_height);

	if (m_left_items_hbounds.first >= m_left_items_hbounds.second)
	{
		// calculate number of lines that will fit - centre vertically if we need scroll arrows
		float const height(m_primary_vbounds.second - m_primary_vbounds.first);
		int const lines((height - (tb_border() * 2.0F)) / m_info_line_height);
		if (Filter::COUNT <= lines)
			m_left_items_top = m_primary_vbounds.first + tb_border();
		else
			m_left_items_top = m_primary_vbounds.first + ((height - (float(lines) * m_info_line_height)) * 0.5F);
		float const pixelheight(target_size().second);
		m_left_items_top = std::round(m_left_items_top * pixelheight) / pixelheight;
		m_left_item_count = Filter::COUNT;
		m_left_visible_lines = std::min<int>(Filter::COUNT, lines);

		// get the maximum filter name width, restricted to a quarter of the target width
		float line_width(0.0F);
		for (typename Filter::type x = Filter::FIRST; Filter::COUNT > x; ++x)
			line_width = std::max(ui().get_string_width(Filter::display_name(x), m_info_line_height) + checkmark_width, line_width);
		line_width = std::min(line_width + (lr_border() * 2.0F), 0.25F);
		m_left_items_hbounds = std::make_pair(2.0F * lr_border(), (2.0F * lr_border()) + line_width);

		// make sure the scroll position makes sense
		m_left_visible_top = (std::min)(m_left_visible_top, m_left_item_count - m_left_visible_lines);
	}
	m_left_panel_width = (m_left_items_hbounds.second - m_left_items_hbounds.first) + (lr_border() * 2.0F);

	// ensure the highlighted item is visible
	if ((m_filter_highlight - Filter::FIRST) < (m_left_visible_top + 1))
		m_left_visible_top = (Filter::FIRST == m_filter_highlight) ? 0 : (m_filter_highlight - 1);
	else if ((m_filter_highlight - Filter::FIRST) > (m_left_visible_top + m_left_visible_lines - 2))
		m_left_visible_top = (std::min)(m_filter_highlight - Filter::FIRST + 2 - m_left_visible_lines, m_left_item_count - m_left_visible_lines);

	// outline the box and inset by the border width
	ui().draw_outlined_box(
			container(),
			lr_border(), m_primary_vbounds.first, lr_border() + m_left_panel_width, m_primary_vbounds.second,
			ui().colors().background_color());

	// now draw the rows
	typename Filter::type filter(Filter::FIRST);
	for (int i = 0; i < m_left_visible_top; ++i)
		++filter;
	auto const active_filter(filters.find(current));
	std::string str;
	bool const atbottom((m_left_visible_top + m_left_visible_lines) == m_left_item_count);
	for (int line = 0; line < m_left_visible_lines; ++line, ++filter)
	{
		float const line_top(m_left_items_top + (float(line) * m_info_line_height));
		bool const uparrow(!line && m_left_visible_top);
		bool const downarrow(!atbottom  && ((m_left_visible_lines - 1) == line));

		// work out the colours for this item
		rgb_t bgcolor = ui().colors().text_bg_color();
		rgb_t fgcolor = ui().colors().text_color();
		bool const hovered(pointer_in_rect(m_left_items_hbounds.first, line_top, m_left_items_hbounds.second, line_top + m_info_line_height));
		bool const pointerline((pointer_action::LEFT_TRACK_LINE == m_pointer_action) && (line == m_clicked_line));
		if (pointerline && hovered)
		{
			// draw selected highlight for tracked item
			bgcolor = ui().colors().selected_bg_color();
			fgcolor = ui().colors().selected_color();
			highlight(m_left_items_hbounds.first, line_top, m_left_items_hbounds.second, line_top + m_info_line_height, bgcolor);
		}
		else if ((m_filter_highlight == filter) && (get_focus() == focused_menu::LEFT))
		{
			// draw primary highlight if keyboard focus is here
			fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
			ui().draw_textured_box(
					container(),
					m_left_items_hbounds.first, line_top, m_left_items_hbounds.second, line_top + m_info_line_height,
					bgcolor, rgb_t(255, 43, 43, 43),
					hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}
		else if (pointerline || (!m_ui_error && !(flags & PROCESS_NOINPUT) && hovered && pointer_idle()))
		{
			// draw hover highlight when hovered over or dragged off
			bgcolor = ui().colors().mouseover_bg_color();
			fgcolor = ui().colors().mouseover_color();
			highlight(m_left_items_hbounds.first, line_top, m_left_items_hbounds.second, line_top + m_info_line_height, bgcolor);
		}

		// finally draw the text itself
		if (uparrow || downarrow)
		{
			draw_arrow(
					0.5F * (m_left_items_hbounds.first + m_left_items_hbounds.second + (x_aspect() * m_info_line_height)),
					line_top + (0.25F * m_info_line_height),
					0.5F * (m_left_items_hbounds.first + m_left_items_hbounds.second - (x_aspect() * m_info_line_height)),
					line_top + (0.75F * m_info_line_height),
					fgcolor,
					downarrow ? (ROT0 ^ ORIENTATION_FLIP_Y) : ROT0);
		}
		else
		{
			if (filters.end() != active_filter)
				str = active_filter->second->adorned_display_name(filter);
			else if (current == filter)
				(str = convert_command_glyph("_> ")).append(Filter::display_name(filter));
			else
				str = Filter::display_name(filter);
			float const x1t = m_left_items_hbounds.first + lr_border() + ((str == Filter::display_name(filter)) ? checkmark_width : 0.0F);
			ui().draw_text_full(
					container(), str,
					x1t, line_top, m_left_items_hbounds.second - x1t - lr_border() + (1.0F / float(target_size().second)),
					text_layout::text_justify::LEFT, text_layout::word_wrapping::TRUNCATE,
					mame_ui_manager::NORMAL, fgcolor, bgcolor,
					nullptr, nullptr,
					m_info_line_height);
		}
	}

	// draw the divider
	draw_divider(flags, lr_border() + m_left_panel_width, false);
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
			util::path_append(current, listname);
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
			util::path_append(current, listname);

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
		// reduce the source bitmap if it's too big
		bitmap_argb32 tmp;
		float const ratio((std::min)({ float(m_icon_height) / src.height(), float(m_icon_width) / src.width(), 1.0F }));
		if (1.0F > ratio)
		{
			float const pix_height(std::ceil(src.height() * ratio));
			float const pix_width(std::ceil(src.width() * ratio));
			tmp.allocate(s32(pix_width), s32(pix_height));
			render_resample_argb_bitmap_hq(tmp, src, render_color{ 1.0F, 1.0F, 1.0F, 1.0F }, true);
		}
		else
		{
			tmp = std::move(src);
		}

		// copy into the destination
		dst.bitmap.allocate(m_icon_width, m_icon_height);
		for (int y = 0; tmp.height() > y; ++y)
			for (int x = 0; tmp.width() > x; ++x)
				dst.bitmap.pix(y, x) = tmp.pix(y, x);
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

void menu_select_launch::draw_toolbar(u32 flags, float x1, float y1, float x2, float y2)
{
	// work out which buttons we're going to draw
	bool const have_parent(m_is_swlist || !stack_has_special_main_menu());
	auto const *const toolbar_bitmaps(m_is_swlist ? SW_TOOLBAR_BITMAPS : SYS_TOOLBAR_BITMAPS);
	unsigned const toolbar_count(m_is_swlist ? std::size(SW_TOOLBAR_BITMAPS) : std::size(SYS_TOOLBAR_BITMAPS));

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, rgb_t(0xef, 0x12, 0x47, 0x7b));

	// cache metrics and bitmaps if necessary
	if (m_toolbar_button_vbounds.first >= m_toolbar_button_vbounds.second)
	{
		m_toolbar_button_vbounds.first = y1 + tb_border();
		m_toolbar_button_vbounds.second = y2 - tb_border();
		float const button_height(m_toolbar_button_vbounds.second - m_toolbar_button_vbounds.first);
		m_toolbar_button_width = button_height * float(x_aspect());
		m_toolbar_button_spacing = m_toolbar_button_width * 1.5F;
		float const total_width((float(toolbar_count) + (float(toolbar_count - 1) * 0.5F)) * m_toolbar_button_width);
		m_toolbar_backtrack_left = x2 - lr_border() - m_toolbar_button_width;
		m_toolbar_main_left = (std::min)(m_toolbar_backtrack_left - (float(toolbar_count) * m_toolbar_button_spacing), (x1 + x2 - total_width) * 0.5F);
		m_cache.cache_toolbar(machine(), m_toolbar_button_width, button_height);
	}

	// tooltip needs to be above for pen/touch to avoid being obscured
	float tooltip_pos;
	switch (pointer_type())
	{
	case ui_event::pointer::PEN:
	case ui_event::pointer::TOUCH:
		tooltip_pos = m_toolbar_button_vbounds.first - line_height() - (2.0F * tb_border());
		break;
	default:
		tooltip_pos = m_toolbar_button_vbounds.second + line_height() + tb_border();
		break;
	}


	{
		// add backtrack button
		bool const hovered(pointer_in_rect(m_toolbar_backtrack_left, m_toolbar_button_vbounds.first, m_toolbar_backtrack_left + m_toolbar_button_width, m_toolbar_button_vbounds.second));
		bool const tracked((pointer_action::TOOLBAR_TRACK == m_pointer_action) && (0 > m_clicked_line));
		rgb_t const color((hovered && tracked) ? rgb_t::white() : rgb_t(0xffcccccc));
		if (tracked || (hovered && !(flags & PROCESS_NOINPUT) && pointer_idle()))
		{
			ui().draw_text_box(
					container(),
					have_parent ? _("Return to Previous Menu") : _("Exit"),
					text_layout::text_justify::RIGHT, 1.0F - lr_border(), tooltip_pos,
					ui().colors().background_color());
		}
		container().add_quad(
				m_toolbar_backtrack_left, m_toolbar_button_vbounds.first, m_toolbar_backtrack_left + m_toolbar_button_width, m_toolbar_button_vbounds.second,
				color,
				m_cache.toolbar_textures()[2 * (have_parent ? TOOLBAR_BITMAP_PREVMENU : TOOLBAR_BITMAP_EXIT)].get(),
				PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	// now add the other buttons
	for (int z = 0; toolbar_count > z; ++z)
	{
		auto const [bitmap, action, need_selection] = toolbar_bitmaps[z];
		float const button_left(m_toolbar_main_left + (float(z) * m_toolbar_button_spacing));
		float const button_right(button_left + m_toolbar_button_width);
		bool const enabled(!need_selection || get_selection_ptr());
		bool const hovered(pointer_in_rect(button_left, m_toolbar_button_vbounds.first, button_right, m_toolbar_button_vbounds.second));
		bool const tracked((pointer_action::TOOLBAR_TRACK == m_pointer_action) && (z == m_clicked_line));
		rgb_t color((hovered && tracked && enabled) ? rgb_t::white() : rgb_t(0xffcccccc));
		if (tracked || (hovered && !(flags & PROCESS_NOINPUT) && pointer_idle()))
		{
			ui().draw_text_box(
					container(),
					_(hover_msg[bitmap]),
					text_layout::text_justify::CENTER, (button_left + button_right) * 0.5F, tooltip_pos,
					ui().colors().background_color());
		}
		container().add_quad(
				button_left, m_toolbar_button_vbounds.first, button_right, m_toolbar_button_vbounds.second,
				color,
				m_cache.toolbar_textures()[(2 * bitmap) + (enabled ? 0 : 1)].get(),
				PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}
}


//-------------------------------------------------
//  draw favorites star
//-------------------------------------------------

void menu_select_launch::draw_star(float x0, float y0)
{
	if (TOOLBAR_BITMAP_FAVORITE < m_cache.toolbar_textures().size())
	{
		float const y1 = y0 + line_height();
		float const x1 = x0 + line_height() * container().manager().ui_aspect(&container());
		container().add_quad(
				x0, y0, x1, y1,
				rgb_t::white(),
				m_cache.toolbar_textures()[TOOLBAR_BITMAP_FAVORITE].get(),
				PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_PACKABLE);
	}
}


//-------------------------------------------------
//  draw icons
//-------------------------------------------------

void menu_select_launch::draw_icon(int linenum, void *selectedref, float x0, float y0)
{
	render_texture *const icon(get_icon_texture(linenum, selectedref));
	if (icon)
	{
		float const x1 = x0 + ud_arrow_width();
		float const y1 = y0 + line_height();
		container().add_quad(x0, y0, x1, y1, rgb_t::white(), icon, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	};
}


//-------------------------------------------------
//  get title and search path for right panel
//-------------------------------------------------

std::string menu_select_launch::get_arts_searchpath()
{
	std::string searchstr;

	// get search path
	std::string addpath;
	if (m_image_view == SNAPSHOT_VIEW)
	{
		emu_options moptions;
		searchstr = machine().options().value(std::get<2>(ARTS_INFO[m_image_view]));
		addpath = moptions.value(std::get<2>(ARTS_INFO[m_image_view]));
	}
	else
	{
		ui_options moptions;
		searchstr = ui().options().value(std::get<2>(ARTS_INFO[m_image_view]));
		addpath = moptions.value(std::get<2>(ARTS_INFO[m_image_view]));
	}

	std::string tmp(searchstr);
	path_iterator path(tmp);
	path_iterator path_iter(addpath);
	std::string c_path, curpath;

	// iterate over path and add path for zipped formats
	while (path.next(curpath))
	{
		path_iter.reset();
		while (path_iter.next(c_path))
			searchstr.append(";").append(curpath).append(PATH_SEPARATOR).append(c_path);
	}

	return searchstr;
}


//-------------------------------------------------
//  handle UI input events for main menu
//-------------------------------------------------

bool menu_select_launch::handle_events(u32 flags, event &ev)
{
	// loop while we have interesting events
	bool stop(false), need_update(false), search_changed(false);
	ui_event local_menu_event;
	while (!stop && machine().ui_input().pop_event(&local_menu_event))
	{
		switch (local_menu_event.event_type)
		{
		// deal with pointer-like input (mouse, pen, touch, etc.)
		case ui_event::type::POINTER_UPDATE:
			{
				auto const [key, redraw] = handle_pointer_update(flags, local_menu_event);
				need_update = need_update || redraw;
				if (IPT_INVALID != key)
				{
					ev.iptkey = key;
					stop = true;
				}
			}
			break;

		// pointer left the normal way, possibly releasing buttons
		case ui_event::type::POINTER_LEAVE:
			{
				auto const [key, redraw] = handle_pointer_leave(flags, local_menu_event);
				need_update = need_update || redraw;
				if (IPT_INVALID != key)
				{
					ev.iptkey = key;
					stop = true;
				}
			}
			break;

		// pointer left in some abnormal way - cancel any associated actions
		case ui_event::type::POINTER_ABORT:
			{
				auto const [key, redraw] = handle_pointer_abort(flags, local_menu_event);
				need_update = need_update || redraw;
				if (IPT_INVALID != key)
				{
					ev.iptkey = key;
					stop = true;
				}
			}
			break;

		// caught scroll event
		case ui_event::type::MOUSE_WHEEL:
			if ((&machine().render().ui_target() == local_menu_event.target) && pointer_idle() && !m_ui_error)
			{
				// check whether it's over something scrollable
				float x, y;
				bool const hit(local_menu_event.target->map_point_container(local_menu_event.mouse_x, local_menu_event.mouse_y, container(), x, y));
				if (!hit)
				{
					m_wheel_movement = 0;
					break;
				}
				focused_menu hover;
				if ((x >= m_primary_items_hbounds.first) && (x < m_primary_items_hbounds.second) && (y >= m_primary_items_top) && (y < (m_primary_items_top + (float(m_primary_lines) * line_height()))))
				{
					hover = focused_menu::MAIN;
				}
				else if (show_left_panel() && (x >= m_left_items_hbounds.first) && (x < m_left_items_hbounds.second) && (y >= m_left_items_top) && (y < (m_left_items_top + (float(m_left_visible_lines) * m_info_line_height))))
				{
					hover = focused_menu::LEFT;
				}
				else if (show_right_panel() && (x >= m_right_content_hbounds.first) && (x < m_right_content_hbounds.second) && (y >= m_right_content_vbounds.first) && (y < m_right_content_vbounds.second))
				{
					hover = focused_menu::RIGHTBOTTOM;
				}
				else
				{
					m_wheel_movement = 0;
					break;
				}

				// clear out leftovers if it isn't the last thing to be scrolled
				if (hover != m_wheel_target)
					m_wheel_movement = 0;
				m_wheel_target = hover;

				// the value is scaled to 120 units per "click"
				m_wheel_movement += local_menu_event.zdelta * local_menu_event.num_lines;
				int const lines((m_wheel_movement + ((0 < local_menu_event.zdelta) ? 36 : -36)) / 120);
				if (!lines)
					break;
				m_wheel_movement -= lines * 120;

				switch (hover)
				{
				case focused_menu::MAIN:
					if (lines > 0)
					{
						if ((selected_index() >= m_available_items) || is_first_selected())
							break;
						stop = true;
						ev.iptkey = IPT_CUSTOM; // stop processing events so info can be rebuilt
						set_selected_index(selected_index() - lines);
						if (selected_index() < top_line + (top_line != 0))
							top_line -= lines;
					}
					else
					{
						if (selected_index() >= (m_available_items - 1))
							break;
						stop = true;
						ev.iptkey = IPT_CUSTOM; // stop processing events so info can be rebuilt
						set_selected_index(std::min(selected_index() - lines, m_available_items - 1));
						if (selected_index() >= top_line + m_visible_items + (top_line != 0))
							top_line -= lines;
					}
					break;
				case focused_menu::LEFT:
					{
						m_left_visible_top = std::clamp(m_left_visible_top - lines, 0, m_left_item_count - m_left_visible_lines);
						int const first(left_at_top() ? 0 : (m_left_visible_top + 1));
						int const last(m_left_visible_top + m_left_visible_lines - (left_at_bottom() ? 1 : 2));
						m_filter_highlight = std::clamp(m_filter_highlight, first, last);
						m_filter_highlight = std::clamp(m_filter_highlight - lines, 0, m_left_item_count - 1);
					}
					break;
				case focused_menu::RIGHTBOTTOM:
					if (RP_INFOS == m_right_panel)
						m_topline_datsview -= lines;
					break;
				case focused_menu::RIGHTTOP:
					break; // never gets here
				}
			}
			break;

		// text input goes to the search field unless there's an error message displayed
		case ui_event::type::IME_CHAR:
			if (!pointer_idle())
				break;

			if (exclusive_input_pressed(ev.iptkey, IPT_UI_FOCUS_NEXT, 0) || exclusive_input_pressed(ev.iptkey, IPT_UI_FOCUS_PREV, 0))
			{
				stop = true;
			}
			else if (m_ui_error)
			{
				ev.iptkey = IPT_CUSTOM;
				stop = true;
			}
			else if (accept_search())
			{
				if (input_character(m_search, local_menu_event.ch, uchar_is_printable))
					search_changed = true;
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
			case ui_event::type::MOUSE_WHEEL:
			case ui_event::type::POINTER_UPDATE:
			case ui_event::type::POINTER_LEAVE:
			case ui_event::type::POINTER_ABORT:
				stop = true;
				break;
			case ui_event::type::NONE:
			case ui_event::type::WINDOW_FOCUS:
			case ui_event::type::WINDOW_DEFOCUS:
			case ui_event::type::IME_CHAR:
				break;
			}
		}
	}

	//  deal with repeating main scroll arrows
	if ((pointer_action::MAIN_TRACK_LINE == m_pointer_action) && (is_main_up_arrow(m_clicked_line) || (is_main_down_arrow(m_clicked_line))))
	{
		if (check_scroll_repeat(m_primary_items_top, m_primary_items_hbounds, line_height()))
		{
			if (!m_clicked_line)
			{
				// scroll up
				--top_line;
				if (main_at_top())
					m_pointer_action = pointer_action::NONE;
			}
			else
			{
				// scroll down
				++top_line;
				if (main_at_bottom())
					m_pointer_action = pointer_action::NONE;
			}
			if (main_force_visible_selection())
			{
				if (IPT_INVALID == ev.iptkey)
					ev.iptkey = IPT_CUSTOM; // stop processing events so the info pane can be rebuilt
			}
			need_update = true;
		}
	}

	//  deal with repeating info view arrows
	if (pointer_action::RIGHT_TRACK_ARROW == m_pointer_action)
	{
		float const left(m_clicked_line ? (m_right_content_hbounds.second - lr_border() - lr_arrow_width()) : (m_right_content_hbounds.first + lr_border()));
		float const right(m_clicked_line ? (m_right_content_hbounds.second - lr_border()) : (m_right_content_hbounds.first + lr_border() + lr_arrow_width()));
		if (pointer_in_rect(left, right_arrows_top(), right, right_arrows_bottom()))
		{
			if (std::chrono::steady_clock::now() >= m_scroll_repeat)
			{
				m_scroll_repeat += std::chrono::milliseconds(200);
				if (!(m_clicked_line ? next_right_panel_view() : previous_right_panel_view()).second)
					m_pointer_action = pointer_action::NONE;
				need_update = true;
			}
		}
	}

	//  deal with repeating filter scroll arrows
	if ((pointer_action::LEFT_TRACK_LINE == m_pointer_action) && (is_left_up_arrow(m_clicked_line) || (is_left_down_arrow(m_clicked_line))))
	{
		if (check_scroll_repeat(m_left_items_top, m_left_items_hbounds, m_info_line_height))
		{
			if (!m_clicked_line)
			{
				// scroll up
				--m_left_visible_top;
				m_filter_highlight = std::min(m_left_visible_top + m_left_visible_lines - 2, m_filter_highlight);
				if (left_at_top())
					m_pointer_action = pointer_action::NONE;
			}
			else
			{
				// scroll down
				++m_left_visible_top;
				m_filter_highlight = std::max(m_left_visible_top + 1, m_filter_highlight);
				if (left_at_bottom())
					m_pointer_action = pointer_action::NONE;
			}
			need_update = true;
		}
	}

	//  deal with repeating filter scroll arrows
	if (pointer_action::RIGHT_TRACK_LINE == m_pointer_action)
	{
		if (check_scroll_repeat(m_right_content_vbounds.first, m_right_content_hbounds, m_info_line_height))
		{
			if (!m_clicked_line)
			{
				// scroll up
				--m_topline_datsview;
				if (info_at_top())
					m_pointer_action = pointer_action::NONE;
			}
			else
			{
				// scroll down
				++m_topline_datsview;
				if (info_at_bottom())
					m_pointer_action = pointer_action::NONE;
			}
			need_update = true;
		}
	}

	if (search_changed)
		reset(reset_options::SELECT_FIRST);

	return need_update;
}


//-------------------------------------------------
//  handle keys for main menu
//-------------------------------------------------

bool menu_select_launch::handle_keys(u32 flags, int &iptkey)
{
	// bail if no items
	if (item_count() == 0)
		return false;

	// if we hit select, return true or pop the stack, depending on the item
	if (exclusive_input_pressed(iptkey, IPT_UI_SELECT, 0))
	{
		if (m_ui_error)
		{
			// dismiss error
		}
		else if (m_focus == focused_menu::LEFT)
		{
			filter_selected(m_filter_highlight);
		}
		return false;
	}

	if (exclusive_input_pressed(iptkey, IPT_UI_BACK, 0))
	{
		if (m_ui_error)
		{
			// dismiss error
			return false;
		}
		else if (!is_special_main_menu() && m_search.empty())
		{
			// pop the stack if this isn't the root session menu
			stack_pop();
			return false;
		}
	}

	if (exclusive_input_pressed(iptkey, IPT_UI_CANCEL, 0))
	{
		if (m_ui_error)
		{
			// dismiss error
		}
		else if (!m_search.empty())
		{
			// escape pressed with non-empty search text clears it
			m_search.clear();
			reset(reset_options::REMEMBER_REF);
		}
		else if (is_special_main_menu())
		{
			// this is the root session menu, exit
			stack_pop();
			machine().schedule_exit();
		}
		return false;
	}

	// validate the current selection
	validate_selection(1);
	bool updated(false);

	// accept left/right keys as-is with repeat
	if (exclusive_input_pressed(iptkey, IPT_UI_LEFT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
	{
		if (m_ui_error)
		{
			// dismiss error
			return false;
		}
		else if (m_focus == focused_menu::RIGHTTOP)
		{
			// Swap the right panel and swallow it
			iptkey = IPT_INVALID;
			if (right_panel() != RP_IMAGES)
			{
				m_right_panel = RP_IMAGES;
				updated = true;
			}
		}
		else if (show_right_panel())
		{
			// Swap the right panel page and swallow it
			if (right_panel() == RP_IMAGES)
			{
				iptkey = IPT_INVALID;
				if (previous_image_view().first)
					updated = true;
			}
			else if (right_panel() == RP_INFOS)
			{
				iptkey = IPT_INVALID;
				if (previous_info_view().first)
					updated = true;
			}
		}
	}

	// swallow left/right keys if they are not appropriate
	if (exclusive_input_pressed(iptkey, IPT_UI_RIGHT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
	{
		if (m_ui_error)
		{
			// dismiss error
			return false;
		}
		else if (m_focus == focused_menu::RIGHTTOP)
		{
			// Swap the right panel and swallow it
			iptkey = IPT_INVALID;
			if (right_panel() != RP_INFOS)
			{
				m_right_panel = RP_INFOS;
				updated = true;
			}
		}
		else if (show_right_panel())
		{
			// Swap the right panel page and swallow it
			if (right_panel() == RP_IMAGES)
			{
				iptkey = IPT_INVALID;
				if (next_image_view().first)
					updated = true;
			}
			else if (right_panel() == RP_INFOS)
			{
				iptkey = IPT_INVALID;
				if (next_info_view().first)
					updated = true;
			}
		}
	}

	// up backs up by one item
	if (exclusive_input_pressed(iptkey, IPT_UI_UP, 6))
	{
		if (m_focus == focused_menu::LEFT)
		{
			// swallow it
			iptkey = IPT_INVALID;
			if (m_filter_highlight)
			{
				--m_filter_highlight;
				updated = true;
			}
		}
		else if ((m_focus == focused_menu::RIGHTTOP) || (m_focus == focused_menu::RIGHTBOTTOM))
		{
			// swallow it
			iptkey = IPT_INVALID;
			if (m_topline_datsview)
			{
				m_topline_datsview--;
				updated = true;
			}
		}
		else if (selected_index() == m_available_items + 1 || is_first_selected() || m_ui_error)
		{
			return updated;
		}
		else
		{
			set_selected_index(selected_index() - 1);
			if (selected_index() == top_line && top_line != 0)
				top_line--;
		}
	}

	// down advances by one item
	if (exclusive_input_pressed(iptkey, IPT_UI_DOWN, 6))
	{
		if (m_focus == focused_menu::LEFT)
		{
			// swallow it
			iptkey = IPT_INVALID;
			if ((m_left_item_count - 1) > m_filter_highlight)
			{
				++m_filter_highlight;
				updated = true;
			}
		}
		else if ((m_focus == focused_menu::RIGHTTOP) || (m_focus == focused_menu::RIGHTBOTTOM))
		{
			// swallow it
			iptkey = IPT_INVALID;
			updated = true;
			m_topline_datsview++;
		}
		else if (is_last_selected() || selected_index() == m_available_items - 1 || m_ui_error)
		{
			return updated;
		}
		else
		{
			set_selected_index(selected_index() + 1);
			if (selected_index() == top_line + m_visible_items + (top_line != 0))
				top_line++;
		}
	}

	// page up backs up by m_visible_items
	if (exclusive_input_pressed(iptkey, IPT_UI_PAGE_UP, 6))
	{
		if (m_focus == focused_menu::LEFT)
		{
			// Filters - swallow it
			iptkey = IPT_INVALID;
			if (!left_at_top())
			{
				updated = true;
				m_left_visible_top -= std::min(std::max(m_left_visible_lines - 3, 1), m_left_visible_top);
				m_filter_highlight = std::min(m_left_visible_top + m_left_visible_lines - 2, m_filter_highlight);
			}
		}
		else if ((m_focus == focused_menu::RIGHTTOP) || (m_focus == focused_menu::RIGHTBOTTOM))
		{
			// Infos - swallow it
			iptkey = IPT_INVALID;
			updated = true;
			m_topline_datsview -= m_right_visible_lines - 3;
		}
		else if (selected_index() < m_available_items && !m_ui_error)
		{
			set_selected_index(std::max(selected_index() - m_visible_items, 0));

			top_line -= m_visible_items - (top_line + m_visible_lines == m_available_items);
		}
	}

	// page down advances by m_visible_items
	if (exclusive_input_pressed(iptkey, IPT_UI_PAGE_DOWN, 6))
	{
		if (m_focus == focused_menu::LEFT)
		{
			// Filters - swallow it
			iptkey = IPT_INVALID;
			if (!left_at_bottom())
			{
				updated = true;
				m_left_visible_top += std::min(std::max(m_left_visible_lines - 3, 1), m_left_item_count - m_left_visible_lines - m_left_visible_top);
				m_filter_highlight = std::max(m_left_visible_top + 1, m_filter_highlight);
			}
		}
		else if ((m_focus == focused_menu::RIGHTTOP) || (m_focus == focused_menu::RIGHTBOTTOM))
		{
			// Infos - swallow it
			iptkey = IPT_INVALID;
			updated = true;
			m_topline_datsview += m_right_visible_lines - 3;
		}
		else if (selected_index() < m_available_items && !m_ui_error)
		{
			set_selected_index(std::min(selected_index() + m_visible_lines - 2 + (selected_index() == 0), m_available_items - 1));

			top_line += m_visible_lines - 2;
		}
	}

	// home goes to the start
	if (exclusive_input_pressed(iptkey, IPT_UI_HOME, 0))
	{
		if (m_focus == focused_menu::LEFT)
		{
			// Filters - swallow it
			iptkey = IPT_INVALID;
			if (m_filter_highlight)
			{
				updated = true;
				m_left_visible_top = 0;
				m_filter_highlight = 0;
			}
		}
		else if ((m_focus == focused_menu::RIGHTTOP) || (m_focus == focused_menu::RIGHTBOTTOM))
		{
			// Infos - swallow it
			iptkey = IPT_INVALID;
			if (m_topline_datsview)
				updated = true;
			m_topline_datsview = 0;
		}
		else if (selected_index() < m_available_items && !m_ui_error)
		{
			select_first_item();
		}
	}

	// end goes to the last
	if (exclusive_input_pressed(iptkey, IPT_UI_END, 0))
	{
		if (m_focus == focused_menu::LEFT)
		{
			// Filters - swallow it
			iptkey = IPT_INVALID;
			if ((m_left_item_count - 1) != m_filter_highlight)
			{
				updated = true;
				if (!left_at_bottom())
					m_left_visible_top = m_left_item_count - m_left_visible_lines;
				m_filter_highlight = m_left_item_count - 1;
			}
		}
		else if ((m_focus == focused_menu::RIGHTTOP) || (m_focus == focused_menu::RIGHTBOTTOM))
		{
			// Infos - swallow it
			iptkey = IPT_INVALID;
			updated = true;
			m_topline_datsview = m_total_lines;
		}
		else if (selected_index() < m_available_items && !m_ui_error)
		{
			set_selected_index(top_line = m_available_items - 1);
		}
	}

	// focus next rotates throw targets forward
	if (exclusive_input_pressed(iptkey, IPT_UI_FOCUS_NEXT, 12))
	{
		if (!m_ui_error)
		{
			rotate_focus(1);
			updated = true;
		}
	}

	// focus next rotates throw targets forward
	if (exclusive_input_pressed(iptkey, IPT_UI_FOCUS_PREV, 12))
	{
		if (!m_ui_error)
		{
			rotate_focus(-1);
			updated = true;
		}
	}

	// handle a toggle cheats request
	if (!m_ui_error && machine().ui_input().pressed_repeat(IPT_UI_TOGGLE_CHEAT, 0))
		mame_machine_manager::instance()->cheat().set_enable(!mame_machine_manager::instance()->cheat().enabled(), true);

	// handle pasting text into the search
	if (exclusive_input_pressed(iptkey, IPT_UI_PASTE, 0))
	{
		if (!m_ui_error && accept_search())
		{
			if (paste_text(m_search, uchar_is_printable))
				reset(reset_options::SELECT_FIRST);
		}
	}

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
			case IPT_UI_PAUSE:
				continue;
			}

			if (exclusive_input_pressed(iptkey, code, 0))
				break;
		}
	}
	return updated;
}


//-------------------------------------------------
//  handle pointer input for main menu
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::custom_pointer_updated(bool changed, ui_event const &uievt)
{
	if (ui_event::type::POINTER_ABORT == uievt.event_type)
		return std::make_tuple(IPT_INVALID, false, false);

	// if nothing's happening, check for clicks
	if (pointer_idle())
	{
		if ((uievt.pointer_pressed & 0x01) && !(uievt.pointer_buttons & ~u32(0x01)))
			return handle_primary_down(changed, uievt);
		else if ((uievt.pointer_pressed & 0x02) && !(uievt.pointer_buttons & ~u32(0x02)))
			return handle_right_down(changed, uievt);
		else if ((uievt.pointer_pressed & 0x04) && !(uievt.pointer_buttons & ~u32(0x04)))
			return handle_middle_down(changed, uievt);
		else
			return std::make_tuple(IPT_INVALID, false, false);
	}

	// handle in-progress actions
	switch (m_pointer_action)
	{
	case pointer_action::NONE:
		break;
	case pointer_action::MAIN_TRACK_LINE:
		return update_main_track_line(changed, uievt);
	case pointer_action::MAIN_TRACK_RBUTTON:
		return update_main_track_rbutton(changed, uievt);
	case pointer_action::MAIN_DRAG:
		return update_main_drag(changed, uievt);
	case pointer_action::LEFT_TRACK_LINE:
		return update_left_track_line(changed, uievt);
	case pointer_action::LEFT_DRAG:
		return update_left_drag(changed, uievt);
	case pointer_action::RIGHT_TRACK_TAB:
		return update_right_track_tab(changed, uievt);
	case pointer_action::RIGHT_TRACK_ARROW:
		return update_right_track_arrow(changed, uievt);
	case pointer_action::RIGHT_TRACK_LINE:
		return update_right_track_line(changed, uievt);
	case pointer_action::RIGHT_SWITCH:
		return update_right_switch(changed, uievt);
	case pointer_action::RIGHT_DRAG:
		return update_right_drag(changed, uievt);
	case pointer_action::TOOLBAR_TRACK:
		return update_toolbar_track(changed, uievt);
	case pointer_action::DIVIDER_TRACK:
		return update_divider_track(changed, uievt);
	}
	return std::make_tuple(IPT_INVALID, false, false);
}


//-------------------------------------------------
//  handle primary click
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::handle_primary_down(bool changed, ui_event const &uievt)
{
	if (m_ui_error)
	{
		m_ui_error = false;
		m_error_text.clear();
		return std::make_tuple(IPT_INVALID, true, true);
	}

	auto const [x, y] = pointer_location();

	// check main item list
	if ((x >= m_primary_items_hbounds.first) && (x < m_primary_items_hbounds.second) && (y > m_primary_items_top))
	{
		int const line((y - m_primary_items_top) / line_height());
		if (line < (m_primary_lines + m_skip_main_items))
		{
			int key(IPT_INVALID);
			m_pointer_action = pointer_action::MAIN_TRACK_LINE;
			m_base_pointer = m_last_pointer = std::make_pair(x, y);
			m_clicked_line = line;
			if (is_main_up_arrow(line))
			{
				// top line is a scroll arrow
				--top_line;
				if (main_force_visible_selection())
					key = IPT_CUSTOM; // stop processing events so the info pane can be rebuilt
				if (!main_at_top())
					m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
				else
					m_pointer_action = pointer_action::NONE;
			}
			else if (is_main_down_arrow(line))
			{
				// bottom line is a scroll arrow
				++top_line;
				if (main_force_visible_selection())
					key = IPT_CUSTOM; // stop processing events so the info pane can be rebuilt
				if (!main_at_bottom())
					m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
				else
					m_pointer_action = pointer_action::NONE;
			}
			return std::make_tuple(key, true, true);
		}
	}

	// check filter list
	if (show_left_panel() && (x >= m_left_items_hbounds.first) && (x < m_left_items_hbounds.second) && (y >= m_left_items_top))
	{
		int const line((y - m_left_items_top) / m_info_line_height);
		if (line < m_left_visible_lines)
		{
			m_pointer_action = pointer_action::LEFT_TRACK_LINE;
			m_base_pointer = m_last_pointer = std::make_pair(x, y);
			m_clicked_line = line;
			if (is_left_up_arrow(line))
			{
				// top line is a scroll arrow
				--m_left_visible_top;
				m_filter_highlight = std::min(m_left_visible_top + m_left_visible_lines - 2, m_filter_highlight);
				if (!left_at_top())
					m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
				else
					m_pointer_action = pointer_action::NONE;
			}
			else if (is_left_down_arrow(line))
			{
				// bottom line is a scroll arrow
				++m_left_visible_top;
				m_filter_highlight = std::max(m_left_visible_top + 1, m_filter_highlight);
				if (!left_at_bottom())
					m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
				else
					m_pointer_action = pointer_action::NONE;
			}
			else
			{
				// ignore multi-click actions - someone hammered on the scroll arrow and hit the end
				if (1 < uievt.pointer_clicks)
					m_pointer_action = pointer_action::NONE;
			}
			return std::make_tuple(IPT_INVALID, true, true);
		}
	}

	// check right panel content
	if (show_right_panel())
	{
		// check right tabs
		if ((x >= right_panel_left()) && (x < right_panel_right()) && (y >= m_primary_vbounds.first) && (y < m_right_tabs_bottom))
		{
			int const tab((x - right_panel_left()) / right_tab_width());
			if (tab != m_right_panel)
			{
				m_pointer_action = pointer_action::RIGHT_TRACK_TAB;
				m_clicked_line = tab;
			}
			return std::make_tuple(IPT_INVALID, true, pointer_action::NONE != m_pointer_action);
		}

		if ((x >= m_right_content_hbounds.first) && (x < m_right_content_hbounds.second))
		{
			// check right panel heading arrows
			if ((ui_event::pointer::TOUCH != uievt.pointer_type) && (y >= right_arrows_top()) && (y < right_arrows_bottom()))
			{
				if ((x >= (m_right_content_hbounds.first + lr_border())) && (x < (m_right_content_hbounds.first + lr_border() + lr_arrow_width())))
				{
					auto const [updated, notend] = previous_right_panel_view();
					if (notend)
					{
						m_pointer_action = pointer_action::RIGHT_TRACK_ARROW;
						m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(600);
						m_base_pointer = m_last_pointer = std::make_pair(x, y);
						m_clicked_line = 0;
					}
					return std::make_tuple(IPT_INVALID, true, updated);
				}
				else if ((x >= (m_right_content_hbounds.second - lr_border() - lr_arrow_width())) && (x < (m_right_content_hbounds.second - lr_border())))
				{
					auto const [updated, notend] = next_right_panel_view();
					if (notend)
					{
						m_pointer_action = pointer_action::RIGHT_TRACK_ARROW;
						m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(600);
						m_base_pointer = m_last_pointer = std::make_pair(x, y);
						m_clicked_line = 1;
					}
					return std::make_tuple(IPT_INVALID, true, updated);
				}
			}

			// check right panel heading touch swipe
			if ((ui_event::pointer::TOUCH == uievt.pointer_type) && (y >= m_right_heading_top) && (y < (m_right_heading_top + line_height())))
			{
				m_pointer_action = pointer_action::RIGHT_SWITCH;
				m_base_pointer = m_last_pointer = std::make_pair(x, y);
				m_clicked_line = 0;
				if (right_panel() == RP_IMAGES)
				{
					m_clicked_line = m_image_view;
				}
				else if (right_panel() == RP_INFOS)
				{
					ui_software_info const *software;
					ui_system_info const *system;
					get_selection(software, system);
					if (software && !software->startempty)
						m_clicked_line = ui_globals::cur_sw_dats_view;
					else if (system || (software && software->driver))
						m_clicked_line = ui_globals::curdats_view;
				}
				return std::make_tuple(IPT_INVALID, true, true);
			}

			// check info scroll
			if ((RP_INFOS == m_right_panel) && (y >= m_right_content_vbounds.first) && (y < (m_right_content_vbounds.first + (float(m_right_visible_lines) * m_info_line_height))))
			{
				int const line((y - m_right_content_vbounds.first) / m_info_line_height);
				if (line < m_right_visible_lines)
				{
					bool redraw(false);
					m_base_pointer = m_last_pointer = std::make_pair(x, y);
					m_clicked_line = line;
					if (!line && !info_at_top())
					{
						redraw = true;
						--m_topline_datsview;
						if (!info_at_top())
						{
							m_pointer_action = pointer_action::RIGHT_TRACK_LINE;
							m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
						}
					}
					else if ((line == (m_right_visible_lines - 1)) && !info_at_bottom())
					{
						redraw = true;
						++m_topline_datsview;
						if (!info_at_bottom())
						{
							m_pointer_action = pointer_action::RIGHT_TRACK_LINE;
							m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
						}
					}
					else if (ui_event::pointer::TOUCH == uievt.pointer_type)
					{
						m_pointer_action = pointer_action::RIGHT_DRAG;
						m_clicked_line = m_topline_datsview;
					}
					return std::make_tuple(IPT_INVALID, true, redraw);
				}
			}
		}
	}

	// check toolbar
	if ((y >= m_toolbar_button_vbounds.first) && (y < m_toolbar_button_vbounds.second))
	{
		if ((x >= m_toolbar_backtrack_left) && (x < (m_toolbar_backtrack_left + m_toolbar_button_width)))
		{
			m_pointer_action = pointer_action::TOOLBAR_TRACK;
			m_clicked_line = -1;
			return std::make_tuple(IPT_INVALID, true, true);
		}
		else
		{
			unsigned const toolbar_count(m_is_swlist ? std::size(SW_TOOLBAR_BITMAPS) : std::size(SYS_TOOLBAR_BITMAPS));
			float const button(std::floor((x - m_toolbar_main_left) / m_toolbar_button_spacing));
			int const n(button);
			if ((n >= 0) && (n < toolbar_count) && (x < (m_toolbar_main_left + (button * m_toolbar_button_spacing) + m_toolbar_button_width)))
			{
				m_pointer_action = pointer_action::TOOLBAR_TRACK;
				m_clicked_line = n;
				return std::make_tuple(IPT_INVALID, true, true);
			}
		}
	}

	// check dividers
	if ((y >= m_primary_vbounds.first) && (y < m_primary_vbounds.second))
	{
		if ((x >= left_divider_left()) && (x < left_divider_right()))
		{
			m_pointer_action = pointer_action::DIVIDER_TRACK;
			m_clicked_line = 0;
			return std::make_tuple(IPT_INVALID, true, true);
		}
		else if ((x >= right_divider_left()) && (x < right_divider_right()))
		{
			m_pointer_action = pointer_action::DIVIDER_TRACK;
			m_clicked_line = 1;
			return std::make_tuple(IPT_INVALID, true, true);
		}
	}

	return std::make_tuple(IPT_INVALID, false, false);
}


//-------------------------------------------------
//  handle right click
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::handle_right_down(bool changed, ui_event const &uievt)
{
	if (m_ui_error)
		return std::make_tuple(IPT_INVALID, false, false);

	// check main item list
	auto const [x, y] = pointer_location();
	if ((x >= m_primary_items_hbounds.first) && (x < m_primary_items_hbounds.second) && (y > m_primary_items_top))
	{
		int const line((y - m_primary_items_top) / line_height());
		if ((line < m_primary_lines) && !is_main_up_arrow(line) && (!is_main_down_arrow(line)))
		{
			m_pointer_action = pointer_action::MAIN_TRACK_RBUTTON;
			m_base_pointer = m_last_pointer = std::make_pair(x, y);
			m_clicked_line = line;
			return std::make_tuple(IPT_INVALID, true, true);
		}
	}

	return std::make_tuple(IPT_INVALID, false, false);
}


//-------------------------------------------------
//  handle middle click
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::handle_middle_down(bool changed, ui_event const &uievt)
{
	if (m_ui_error)
		return std::make_tuple(IPT_INVALID, false, false);

	auto const [x, y] = pointer_location();
	if ((y >= m_primary_vbounds.first) && (y < m_primary_vbounds.second))
	{
		if ((x >= left_divider_right()) && (x < right_divider_left()))
		{
			// main list
			if (m_skip_main_items && (y >= (m_primary_items_top + ((float(m_primary_lines) + ((item(m_available_items).type() == menu_item_type::SEPARATOR) ? 0.5F : 0.0F)) * line_height()))))
			{
				if (selected_index() < m_available_items)
				{
					m_prev_selected = get_selection_ref();
					set_selected_index(m_available_items + 1);
				}
			}
			else
			{
				if ((get_focus() != focused_menu::MAIN) || (selected_index() > m_available_items))
					select_prev();
			}
			set_focus(focused_menu::MAIN);
		}
		else if ((x >= left_panel_left()) && (x < left_panel_right()))
		{
			// left panel
			assert(show_left_panel());
			if ((get_focus() == focused_menu::MAIN) && (selected_index() < m_available_items))
				m_prev_selected = get_selection_ref();
			set_focus(focused_menu::LEFT);
			return std::make_tuple(IPT_INVALID, true, true);
		}
		else if ((x >= right_panel_left()) && (x < right_panel_right()))
		{
			// right panel
			assert(show_right_panel());
			if ((get_focus() == focused_menu::MAIN) && (selected_index() < m_available_items))
				m_prev_selected = get_selection_ref();
			set_focus((y < m_right_tabs_bottom) ? focused_menu::RIGHTTOP : focused_menu::RIGHTBOTTOM);
			return std::make_tuple(IPT_INVALID, true, true);
		}
	}

	return std::make_tuple(IPT_INVALID, false, false);
}


//-------------------------------------------------
//  track click on main item
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_main_track_line(bool changed, ui_event const &uievt)
{
	auto const [x, y] = pointer_location();
	float const itemtop(m_primary_items_top + (float(m_clicked_line) * line_height()));
	float const itembottom(m_primary_items_top + (float(m_clicked_line + 1) * line_height()));
	int key(IPT_INVALID);
	bool redraw(false);

	if (is_main_up_arrow(m_clicked_line) || is_main_down_arrow(m_clicked_line))
	{
		// top or bottom line is a scroll arrow
		bool const reentered(reentered_rect(m_last_pointer.first, m_last_pointer.second, x, y, m_primary_items_hbounds.first, itemtop, m_primary_items_hbounds.second, itembottom));
		if (reentered)
		{
			auto const now(std::chrono::steady_clock::now());
			if (now >= m_scroll_repeat)
			{
				m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
				if (!m_clicked_line)
				{
					--top_line;
					if (main_at_top())
						m_pointer_action = pointer_action::NONE;
				}
				else
				{
					++top_line;
					if (main_at_bottom())
						m_pointer_action = pointer_action::NONE;
				}
				if (main_force_visible_selection())
					key = IPT_CUSTOM; // stop processing events so the info pane can be rebuilt
				redraw = true;
			}
		}
	}
	else
	{
		// check for conversion to touch scroll
		if (ui_event::pointer::TOUCH == uievt.pointer_type)
		{
			auto const [h, v] = check_drag_conversion(x, y, m_base_pointer.first, m_base_pointer.second, line_height());
			if (h || (v && (m_clicked_line >= m_primary_lines)))
			{
				m_pointer_action = pointer_action::NONE;
				return std::make_tuple(IPT_INVALID, false, true);
			}
			else if (v)
			{
				m_pointer_action = pointer_action::MAIN_DRAG;
				m_last_pointer = m_base_pointer;
				m_clicked_line = top_line;
				return update_main_drag(changed, uievt);
			}
		}

		// check to see if they released over the item
		if (uievt.pointer_released & 0x01)
		{
			m_pointer_action = pointer_action::NONE;
			if ((0 < uievt.pointer_clicks) || ((x >= m_primary_items_hbounds.first) && (x < m_primary_items_hbounds.second) && (y >= itemtop) && (y < itembottom)))
			{
				if (m_clicked_line < m_visible_lines)
				{
					// systems or software items are always selectable
					if (2 == uievt.pointer_clicks)
						key = IPT_UI_SELECT;
					else if (selected_index() != (m_clicked_line + top_line))
						key = IPT_CUSTOM; // stop processing events so the info pane can be rebuilt
					set_selected_index(m_clicked_line + top_line);
					set_focus(focused_menu::MAIN);
				}
				else if ((m_clicked_line >= m_primary_lines) && (m_clicked_line < (m_primary_lines + m_skip_main_items)))
				{
					// need to ensure this is a selectable item
					int const itemnum(m_available_items + m_clicked_line - m_primary_lines);
					if (is_selectable(item(itemnum)))
					{
						if (selected_index() < m_available_items)
							m_prev_selected = get_selection_ref();
						set_selected_index(itemnum);
						set_focus(focused_menu::MAIN);
						if (2 == uievt.pointer_clicks)
							key = IPT_UI_SELECT;
					}
				}
			}
			return std::make_tuple(key, false, true);
		}
	}

	// stop tracking if the primary button is released or another button is pressed
	if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
		m_pointer_action = pointer_action::NONE;
	if (pointer_action::NONE != m_pointer_action)
		m_last_pointer = std::make_pair(x, y);
	return std::make_tuple(key, pointer_action::NONE != m_pointer_action, redraw);
}


//-------------------------------------------------
//  track right click
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_main_track_rbutton(bool changed, ui_event const &uievt)
{
	// see if it was released inside the line
	if (uievt.pointer_released & 0x02)
	{
		m_pointer_action = pointer_action::NONE;
		auto const [x, y] = pointer_location();
		float const linetop(m_primary_items_top + (float(m_clicked_line) * line_height()));
		float const linebottom(m_primary_items_top + (float(m_clicked_line + 1) * line_height()));
		if ((x >= m_primary_items_hbounds.first) && (x < m_primary_items_hbounds.second) && (y >= linetop) && (y < linebottom))
		{
			show_config_menu(m_clicked_line + top_line);
			return std::make_tuple(IPT_CUSTOM, false, false); // return IPT_CUSTOM to stop processing events
		}
	}

	// stop tracking if another button is pressed
	if (uievt.pointer_pressed & ~u32(0x02))
		m_pointer_action = pointer_action::NONE;
	return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, pointer_action::DIVIDER_TRACK != m_pointer_action);
}


//-------------------------------------------------
//  track main touch drag scroll
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_main_drag(bool changed, ui_event const &uievt)
{
	auto const newtop(drag_scroll(
			pointer_location().second, m_base_pointer.second, m_last_pointer.second, -line_height(),
			m_clicked_line, 0, m_available_items - m_primary_lines));
	bool const moved(newtop != top_line);
	int key(IPT_INVALID);
	if (moved)
	{
		// scroll and move the selection if necessary to keep it in the visible range
		top_line = newtop;
		if (main_force_visible_selection())
			key = IPT_CUSTOM; // stop processing events so the info pane can be rebuilt
	}

	// stop tracking if the primary button is released or another button is pressed
	if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
		m_pointer_action = pointer_action::NONE;
	return std::make_tuple(key, pointer_action::NONE != m_pointer_action, moved);
}


//-------------------------------------------------
//  track click on left panel item
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_left_track_line(bool changed, ui_event const &uievt)
{
	auto const [x, y] = pointer_location();
	float const itemtop(m_left_items_top + (float(m_clicked_line) * m_info_line_height));
	float const itembottom(m_left_items_top + (float(m_clicked_line + 1) * m_info_line_height));
	bool redraw(false);

	if (is_left_up_arrow(m_clicked_line) || is_left_down_arrow(m_clicked_line))
	{
		// top or bottom line is a scroll arrow
		bool const reentered(reentered_rect(m_last_pointer.first, m_last_pointer.second, x, y, m_left_items_hbounds.first, itemtop, m_left_items_hbounds.second, itembottom));
		if (reentered)
		{
			auto const now(std::chrono::steady_clock::now());
			if (now >= m_scroll_repeat)
			{
				m_scroll_repeat = now + std::chrono::milliseconds(100);
				if (!m_clicked_line)
				{
					--m_left_visible_top;
					m_filter_highlight = std::min(m_left_visible_top + m_left_visible_lines - 2, m_filter_highlight);
					if (left_at_top())
						m_pointer_action = pointer_action::NONE;
				}
				else
				{
					++m_left_visible_top;
					m_filter_highlight = std::max(m_left_visible_top + 1, m_filter_highlight);
					if (left_at_bottom())
						m_pointer_action = pointer_action::NONE;
				}
				redraw = true;
			}
		}
	}
	else
	{
		// check for conversion to touch scroll
		if (ui_event::pointer::TOUCH == uievt.pointer_type)
		{
			auto const [h, v] = check_drag_conversion(x, y, m_base_pointer.first, m_base_pointer.second, m_info_line_height);
			if (h)
			{
				m_pointer_action = pointer_action::NONE;
				return std::make_tuple(IPT_INVALID, false, true);
			}
			else if (v)
			{
				m_pointer_action = pointer_action::LEFT_DRAG;
				m_last_pointer = m_base_pointer;
				m_clicked_line = m_left_visible_top;
				return update_left_drag(changed, uievt);
			}
		}

		// this is a filter - check to see if they released over the item
		if ((uievt.pointer_released & 0x01) && (x >= m_left_items_hbounds.first) && (x < m_left_items_hbounds.second) && (y >= itemtop) && (y < itembottom))
		{
			m_pointer_action = pointer_action::NONE;
			filter_selected(m_left_visible_top + m_clicked_line);
			return std::make_tuple(IPT_CUSTOM, false, true); // return IPT_CUSTOM to stop processing events
		}
	}

	// stop tracking if the primary button is released or another button is pressed
	if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
		m_pointer_action = pointer_action::NONE;
	if (pointer_action::NONE != m_pointer_action)
		m_last_pointer = std::make_pair(x, y);
	return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, redraw);
}


//-------------------------------------------------
//  track left panel touch drag scroll
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_left_drag(bool changed, ui_event const &uievt)
{
	auto const newtop(drag_scroll(
			pointer_location().second, m_base_pointer.second, m_last_pointer.second, -m_info_line_height,
			m_clicked_line, 0, m_left_item_count - m_left_visible_lines));
	bool const moved(newtop != m_left_visible_top);
	if (moved)
	{
		// scroll and move the selection if necessary to keep it in the visible range
		m_left_visible_top = newtop;
		int const first(left_at_top() ? 0 : (newtop + 1));
		int const last(newtop + m_left_visible_lines - (left_at_bottom() ? 1 : 2));
		m_filter_highlight = std::clamp(m_filter_highlight, first, last);
	}

	// stop tracking if the primary button is released or another button is pressed
	if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
		m_pointer_action = pointer_action::NONE;
	return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, moved);
}


//-------------------------------------------------
//  track click on right panel tab
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_right_track_tab(bool changed, ui_event const &uievt)
{
	// see if it was released inside the divider
	if (uievt.pointer_released & 0x01)
	{
		m_pointer_action = pointer_action::NONE;
		auto const [x, y] = pointer_location();
		float const left(right_panel_left() + (float(m_clicked_line) * right_tab_width()));
		float const right(right_panel_left() + (float(m_clicked_line + 1) * right_tab_width()));
		if ((x >= left) && (x < right) && (y >= m_primary_vbounds.first) && (y < m_right_tabs_bottom))
		{
			m_right_panel = m_clicked_line;
			return std::make_tuple(IPT_CUSTOM, false, true); // return IPT_CUSTOM to stop processing events
		}
	}

	// stop tracking if another button is pressed
	if (uievt.pointer_pressed & ~u32(0x01))
		m_pointer_action = pointer_action::NONE;
	return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, pointer_action::DIVIDER_TRACK != m_pointer_action);
}


//-------------------------------------------------
//  track right panel heading left/right arrows
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_right_track_arrow(bool changed, ui_event const &uievt)
{
	auto const [x, y] = pointer_location();
	float const left(m_clicked_line ? (m_right_content_hbounds.second - lr_border() - lr_arrow_width()) : (m_right_content_hbounds.first + lr_border()));
	float const right(m_clicked_line ? (m_right_content_hbounds.second - lr_border()) : (m_right_content_hbounds.first + lr_border() + lr_arrow_width()));

	// check for reentry
	bool redraw(false);
	bool const reentered(reentered_rect(m_last_pointer.first, m_last_pointer.second, x, y, left, right_arrows_top(), right, right_arrows_bottom()));
	if (reentered)
	{
		auto const now(std::chrono::steady_clock::now());
		if (now >= m_scroll_repeat)
		{
			m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(200);
			bool notend;
			std::tie(redraw, notend) = m_clicked_line ? next_right_panel_view() : previous_right_panel_view();
			if (!notend)
				m_pointer_action = pointer_action::NONE;
		}
	}

	// stop tracking if the primary button is released or another button is pressed
	if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
		m_pointer_action = pointer_action::NONE;
	if (pointer_action::NONE != m_pointer_action)
		m_last_pointer = std::make_pair(x, y);
	return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, redraw);
}


//-------------------------------------------------
//  track right scroll arrows
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_right_track_line(bool changed, ui_event const &uievt)
{
	auto const [x, y] = pointer_location();
	float const itemtop(m_right_content_vbounds.first + (float(m_clicked_line) * m_info_line_height));
	float const itembottom(m_right_content_vbounds.first + (float(m_clicked_line + 1) * m_info_line_height));
	bool redraw(false);

	bool const reentered(reentered_rect(m_last_pointer.first, m_last_pointer.second, x, y, m_right_content_hbounds.first, itemtop, m_right_content_hbounds.second, itembottom));
	if (reentered)
	{
		auto const now(std::chrono::steady_clock::now());
		if (now >= m_scroll_repeat)
		{
			m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
			if (!m_clicked_line)
			{
				--m_topline_datsview;
				if (info_at_top())
					m_pointer_action = pointer_action::NONE;
			}
			else
			{
				++m_topline_datsview;
				if (info_at_bottom())
					m_pointer_action = pointer_action::NONE;
			}
			redraw = true;
		}
	}

	// stop tracking if the primary button is released or another button is pressed
	if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
		m_pointer_action = pointer_action::NONE;
	if (pointer_action::NONE != m_pointer_action)
		m_last_pointer = std::make_pair(x, y);
	return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, redraw);
}


//-------------------------------------------------
//  track panel heading touch drag switch
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_right_switch(bool changed, ui_event const &uievt)
{
	// get new page
	ui_software_info const *software;
	ui_system_info const *system;
	get_selection(software, system);
	int const min((RP_IMAGES == m_right_panel) ? FIRST_VIEW : 0);
	int const max(
			(RP_IMAGES == m_right_panel) ? LAST_VIEW :
			(software && !software->startempty) ? (ui_globals::cur_sw_dats_total - 1) :
			(system || (software && software->driver)) ? (ui_globals::curdats_total - 1) :
			0);
	auto const newpage(drag_scroll(
			pointer_location().first, m_base_pointer.first, m_last_pointer.first, 0.125F * (m_right_content_hbounds.first - m_right_content_hbounds.second),
			m_clicked_line, min, max));

	// switch page
	u8 dummy(newpage);
	u8 &current(
			(RP_IMAGES == m_right_panel) ? m_image_view :
			(software && !software->startempty) ? ui_globals::cur_sw_dats_view :
			(system || (software && software->driver)) ? ui_globals::curdats_view :
			dummy);
	bool const redraw(newpage != current);
	if (redraw)
	{
		current = newpage;
		if (RP_IMAGES == m_right_panel)
			set_switch_image();
		else
			m_topline_datsview = 0;
	}

	// stop tracking if the primary button is released or another button is pressed
	if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
		m_pointer_action = pointer_action::NONE;
	return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, redraw);
}


//-------------------------------------------------
//  track right panel touch drag scroll
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_right_drag(bool changed, ui_event const &uievt)
{
	auto const newtop(drag_scroll(
			pointer_location().second, m_base_pointer.second, m_last_pointer.second, -m_info_line_height,
			m_clicked_line, 0, m_total_lines - m_right_visible_lines));
	bool const moved(newtop != m_topline_datsview);
	m_topline_datsview = newtop;

	// stop tracking if the primary button is released or another button is pressed
	if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
		m_pointer_action = pointer_action::NONE;
	return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, moved);
}


//-------------------------------------------------
//  track click on toolbar button
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_toolbar_track(bool changed, ui_event const &uievt)
{
	// see if it was released inside the button
	if (uievt.pointer_released & 0x01)
	{
		m_pointer_action = pointer_action::NONE;
		auto const [x, y] = pointer_location();
		float const left((0 > m_clicked_line) ? m_toolbar_backtrack_left : (m_toolbar_main_left + (float(m_clicked_line) * m_toolbar_button_spacing)));
		if ((x >= left) && (x < (left + m_toolbar_button_width)) && (y >= m_toolbar_button_vbounds.first) && (y < m_toolbar_button_vbounds.second))
		{
			if (0 > m_clicked_line)
			{
				// backtrack button
				stack_pop();
				if (is_special_main_menu())
					machine().schedule_exit();
				return std::make_tuple(IPT_UI_BACK, false, true);
			}
			else
			{
				// main buttons
				auto const *const toolbar_bitmaps(m_is_swlist ? SW_TOOLBAR_BITMAPS : SYS_TOOLBAR_BITMAPS);
				auto const [bitmap, action, need_selection] = toolbar_bitmaps[m_clicked_line];
				switch (action)
				{
				case IPT_UI_EXPORT:
					inkey_export();
					break;
				case IPT_UI_DATS:
					inkey_dats();
					break;
				default:
					return std::make_tuple(action, false, true);
				}
				return std::make_tuple(IPT_CUSTOM, false, true); // return IPT_CUSTOM to stop processing events
			}
		}
	}

	// stop tracking if another button is pressed
	if (uievt.pointer_pressed & ~u32(0x01))
		m_pointer_action = pointer_action::NONE;
	return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, pointer_action::TOOLBAR_TRACK != m_pointer_action);
}


//-------------------------------------------------
//  track click on divider
//-------------------------------------------------

std::tuple<int, bool, bool> menu_select_launch::update_divider_track(bool changed, ui_event const &uievt)
{
	// see if it was released inside the divider
	if (uievt.pointer_released & 0x01)
	{
		m_pointer_action = pointer_action::NONE;
		auto const [x, y] = pointer_location();
		float const left(m_clicked_line ? right_divider_left() : left_divider_left());
		float const right(m_clicked_line ? right_divider_right() : left_divider_right());
		if ((x >= left) && (x < right) && (y >= m_primary_vbounds.first) && (y < m_primary_vbounds.second))
		{
			if (m_clicked_line)
			{
				if ((get_focus() == focused_menu::RIGHTTOP) || (get_focus() == focused_menu::RIGHTBOTTOM))
				{
					set_focus(focused_menu::MAIN);
					select_prev();
				}
				m_panels_status ^= HIDE_RIGHT_PANEL;
			}
			else
			{
				if (get_focus() == focused_menu::LEFT)
				{
					set_focus(focused_menu::MAIN);
					select_prev();
				}
				m_panels_status ^= HIDE_LEFT_PANEL;
			}
			return std::make_tuple(IPT_CUSTOM, false, true); // return IPT_CUSTOM to stop processing events
		}
	}

	// stop tracking if another button is pressed
	if (uievt.pointer_pressed & ~u32(0x01))
		m_pointer_action = pointer_action::NONE;
	return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, pointer_action::DIVIDER_TRACK != m_pointer_action);
}


//-------------------------------------------------
//  move selection to visible range
//-------------------------------------------------

bool menu_select_launch::main_force_visible_selection()
{
	int const first(main_at_top() ? 0 : (top_line + 1));
	int const last(top_line + m_primary_lines - (main_at_bottom() ? 1 : 2));
	if (selected_index() < m_available_items)
	{
		int const restricted(std::clamp(selected_index(), first, last));
		if (selected_index() != restricted)
		{
			set_selected_index(restricted);
			return true;
		}
	}
	else if (m_prev_selected)
	{
		int selection(0);
		while ((m_available_items > selection) && (item(selection).ref() != m_prev_selected))
			++selection;
		auto const ref(item(std::clamp(selection, first, last)).ref());
		if (ref != m_prev_selected)
		{
			m_prev_selected = ref;
			return true;
		}
	}
	return false;
}


//-------------------------------------------------
//  draw main menu
//-------------------------------------------------

void menu_select_launch::draw(u32 flags)
{

	// recompute height of primary menu area if necessary
	if (m_primary_vbounds.first >= m_primary_vbounds.second)
	{
		float const pixelheight(target_size().second);
		float const lines(std::floor((1.0F - (4.0F * tb_border()) - get_customtop() - get_custombottom()) / line_height()));
		float const itemsheight(line_height() * lines);
		float const space(1.0F - itemsheight - get_customtop() - get_custombottom());
		m_primary_items_top = std::round((get_customtop() + (0.5F * space)) * pixelheight) / pixelheight;
		m_primary_vbounds = std::make_pair(m_primary_items_top - tb_border(), m_primary_items_top + itemsheight + tb_border());
		m_primary_lines = int(lines) - m_skip_main_items;
	}

	// ensure the selection is visible
	m_available_items = item_count() - m_skip_main_items;
	m_visible_lines = (std::min)(m_primary_lines, m_available_items);
	int selection;
	if (selected_index() < m_available_items)
	{
		selection = selected_index();
	}
	else
	{
		selection = 0;
		while ((m_available_items > selection) && (item(selection).ref() != m_prev_selected))
			++selection;
	}
	if (top_line < 0 || !selection)
	{
		top_line = 0;
	}
	else if (selection < m_available_items)
	{
		if ((selection >= (top_line + m_visible_lines)) || (selection <= top_line))
			top_line = (std::max)(selection - (m_visible_lines / 2), 0);
		if ((top_line + m_visible_lines) >= m_available_items)
			top_line = m_available_items - m_visible_lines;
		else if (selection >= (top_line + m_visible_lines - 2))
			top_line = selection - m_visible_lines + ((selection == (m_available_items - 1)) ? 1: 2);
	}
	if ((m_focus == focused_menu::MAIN) && (selected_index() < m_available_items))
		m_prev_selected = nullptr;

	// draw background, left and right panels, and outline of main box
	draw_background();
	draw_left_panel(flags);
	draw_right_panel(flags);
	ui().draw_outlined_box(
			container(),
			left_divider_right(), m_primary_vbounds.first, right_divider_left(), m_primary_vbounds.second,
			ui().colors().background_color());

	// calculate horizontal geometry of main item list
	m_primary_items_hbounds = std::make_pair(left_divider_right() + lr_border(), right_divider_left() - lr_border());
	float const item_text_left(m_primary_items_hbounds.first + gutter_width());
	float const item_text_width(m_primary_items_hbounds.second - m_primary_items_hbounds.first - (2.0F * gutter_width()));
	float const icon_offset(m_has_icons ? (1.5F * ud_arrow_width()) : 0.0F);

	// draw main scrolling items
	for (int linenum = 0; linenum < m_visible_lines; linenum++)
	{
		int const itemnum(top_line + linenum);
		menu_item const &pitem(item(itemnum));
		std::string_view const itemtext(pitem.text());
		float const linetop(m_primary_items_top + (float(linenum) * line_height()));
		float const linebottom(linetop + line_height());

		// work out colours
		rgb_t fgcolor = ui().colors().text_color();
		rgb_t bgcolor = ui().colors().text_bg_color();
		rgb_t fgcolor3 = ui().colors().clone_color();
		bool const hovered(is_selectable(pitem) && pointer_in_rect(m_primary_items_hbounds.first, linetop, m_primary_items_hbounds.second, linebottom));
		bool const pointerline((pointer_action::MAIN_TRACK_LINE == m_pointer_action) && (linenum == m_clicked_line));
		bool const rclickline((pointer_action::MAIN_TRACK_RBUTTON == m_pointer_action) && (linenum == m_clicked_line));
		if (!rclickline && is_selected(itemnum) && (get_focus() == focused_menu::MAIN))
		{
			// if we're selected, draw with a different background
			fgcolor = rgb_t(0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff);
			fgcolor3 = rgb_t(0xcc, 0xcc, 0x00);
			ui().draw_textured_box(
					container(),
					m_primary_items_hbounds.first, linetop, m_primary_items_hbounds.second, linebottom,
					bgcolor, rgb_t(43, 43, 43),
					hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}
		else if ((pointerline || rclickline) && hovered)
		{
			// draw selected highlight for tracked item
			fgcolor = fgcolor3 = ui().colors().selected_color();
			bgcolor = ui().colors().selected_bg_color();
			highlight(m_primary_items_hbounds.first, linetop, m_primary_items_hbounds.second, linebottom, bgcolor);
		}
		else if (pointerline || rclickline || (!m_ui_error && !(flags & PROCESS_NOINPUT) && hovered && pointer_idle()))
		{
			// draw hover highlight when hovered over or dragged off
			fgcolor = fgcolor3 = ui().colors().mouseover_color();
			bgcolor = ui().colors().mouseover_bg_color();
			highlight(m_primary_items_hbounds.first, linetop, m_primary_items_hbounds.second, linebottom, bgcolor);
		}
		else if (pitem.ref() == m_prev_selected)
		{
			fgcolor = fgcolor3 = ui().colors().mouseover_color();
			bgcolor = ui().colors().mouseover_bg_color();
			ui().draw_textured_box(
					container(),
					m_primary_items_hbounds.first, linetop, m_primary_items_hbounds.second, linebottom,
					bgcolor, rgb_t(43, 43, 43),
					hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}

		if ((!linenum && top_line) || ((linenum == (m_visible_lines - 1)) && (itemnum != m_available_items - 1)))
		{
			// if we're on the top or bottom line, display the up or down arrow
			draw_arrow(
					0.5F * (m_primary_items_hbounds.first + m_primary_items_hbounds.second - ud_arrow_width()), linetop + (0.25F * line_height()),
					0.5F * (m_primary_items_hbounds.first + m_primary_items_hbounds.second + ud_arrow_width()), linetop + (0.75F * line_height()),
					fgcolor,
					linenum ? (ROT0 ^ ORIENTATION_FLIP_Y) : ROT0);
		}
		else if (pitem.type() == menu_item_type::SEPARATOR)
		{
			// if we're just a divider, draw a line
			container().add_line(
					m_primary_items_hbounds.first, linetop + (0.5F * line_height()),
					m_primary_items_hbounds.second, linetop + (0.5F * line_height()),
					UI_LINE_WIDTH, ui().colors().text_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
		else
		{
			bool const item_invert(pitem.flags() & FLAG_INVERT);
			if (m_has_icons)
				draw_icon(linenum, pitem.ref(), item_text_left, linetop);
			if (pitem.subtext().empty())
			{
				// draw the item left-aligned
				ui().draw_text_full(
						container(),
						itemtext,
						item_text_left + icon_offset, linetop, item_text_width - icon_offset,
						text_layout::text_justify::LEFT, text_layout::word_wrapping::TRUNCATE,
						mame_ui_manager::NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor,
						nullptr, nullptr,
						line_height());
			}
			else
			{
				// compute right space for subitem
				std::string_view const subitem_text(pitem.subtext());
				float const subitem_width(get_string_width(subitem_text) + gutter_width());

				// draw the item left-aligned
				float item_width;
				ui().draw_text_full(
						container(),
						itemtext,
						item_text_left + icon_offset, linetop, item_text_width - icon_offset - subitem_width,
						text_layout::text_justify::LEFT, text_layout::word_wrapping::TRUNCATE,
						mame_ui_manager::NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor,
						&item_width, nullptr,
						line_height());

				// draw the subitem right-aligned
				ui().draw_text_full(
						container(),
						subitem_text,
						item_text_left + icon_offset + item_width, linetop, item_text_width - icon_offset - item_width,
						text_layout::text_justify::RIGHT, text_layout::word_wrapping::NEVER,
						mame_ui_manager::NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor,
						nullptr, nullptr,
						line_height());
			}
		}
	}

	// draw extra fixed items
	for (size_t linenum = 0; linenum < m_skip_main_items; linenum++)
	{
		int const itemnum(m_available_items + linenum);
		menu_item const &pitem(item(itemnum));
		std::string_view const itemtext(pitem.text());
		float const linetop(m_primary_items_top + (float(m_primary_lines + linenum) * line_height()));
		float const linebottom(linetop + line_height());

		// work out colours
		rgb_t fgcolor = ui().colors().text_color();
		rgb_t bgcolor = ui().colors().text_bg_color();
		if (is_selected(itemnum) && (get_focus() == focused_menu::MAIN))
		{
			// if we're selected, draw with a different background
			fgcolor = rgb_t(0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff);
			ui().draw_textured_box(
					container(),
					m_primary_items_hbounds.first, linetop, m_primary_items_hbounds.second, linebottom,
					bgcolor, rgb_t(43, 43, 43),
					hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}
		else if (is_selectable(pitem))
		{
			bool const hovered(pointer_in_rect(m_primary_items_hbounds.first, linetop, m_primary_items_hbounds.second, linebottom));
			bool const pointerline((pointer_action::MAIN_TRACK_LINE == m_pointer_action) && ((m_primary_lines + linenum) == m_clicked_line));
			if (pointerline && hovered)
			{
				// draw selected highlight for tracked item
				fgcolor = ui().colors().selected_color();
				bgcolor = ui().colors().selected_bg_color();
				highlight(m_primary_items_hbounds.first, linetop, m_primary_items_hbounds.second, linebottom, bgcolor);
			}
			else if (pointerline || (!m_ui_error && !(flags & PROCESS_NOINPUT) && hovered && pointer_idle()))
			{
				// draw hover highlight when hovered over or dragged off
				fgcolor = ui().colors().mouseover_color();
				bgcolor = ui().colors().mouseover_bg_color();
				highlight(m_primary_items_hbounds.first, linetop, m_primary_items_hbounds.second, linebottom, bgcolor);
			}
		}

		if (pitem.type() == menu_item_type::SEPARATOR)
		{
			// if we're just a divider, draw a line
			container().add_line(
					m_primary_items_hbounds.first, linetop + (0.5F * line_height()),
					m_primary_items_hbounds.second, linetop + (0.5F * line_height()),
					UI_LINE_WIDTH, ui().colors().text_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
		else
		{
			// draw the item centred
			ui().draw_text_full(
					container(),
					itemtext,
					item_text_left, linetop, item_text_width,
					text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
					mame_ui_manager::NORMAL, fgcolor, bgcolor,
					nullptr, nullptr,
					line_height());
		}
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render(
			flags,
			get_selection_ref(),
			get_customtop(), get_custombottom(),
			lr_border(), m_primary_vbounds.first, 1.0F - lr_border(), m_primary_vbounds.second);

	// show error text if necessary
	if (m_ui_error)
	{
		container().add_rect(0.0F, 0.0F, 1.0F, 1.0F, rgb_t(114, 0, 0, 0), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		ui().draw_text_box(container(), m_error_text, text_layout::text_justify::CENTER, 0.5f, 0.5f, UI_RED_COLOR);
	}

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	m_visible_items = m_visible_lines - (top_line != 0) - (top_line + m_visible_lines != m_available_items);
}


//-------------------------------------------------
//  draw right panel
//-------------------------------------------------

void menu_select_launch::draw_right_panel(u32 flags)
{
	if (show_right_panel())
	{
		m_right_panel_width = 0.3F - m_divider_width;

		ui().draw_outlined_box(
				container(), right_panel_left(), m_primary_vbounds.first, right_panel_right(), m_primary_vbounds.second,
				ui().colors().background_color());
		draw_right_box_tabs(flags);

		if (0.0F >= m_right_heading_top)
		{
			float const pixelheight(target_size().second);
			m_right_heading_top = std::round((m_right_tabs_bottom + UI_LINE_WIDTH + tb_border()) * pixelheight) / pixelheight;
			m_right_content_vbounds = std::make_pair(
					std::round((m_right_heading_top + line_height() + tb_border()) * pixelheight) / pixelheight,
					m_primary_vbounds.second - tb_border());
			m_right_content_hbounds = std::make_pair(right_panel_left() + lr_border(), right_panel_right() - lr_border());
		}

		if (m_right_panel == RP_IMAGES)
			arts_render(flags);
		else
			infos_render(flags);
	}
	else
	{
		m_right_panel_width = 0.0F;
	}

	draw_divider(flags, 1.0F - lr_border() - m_right_panel_width - m_divider_width, true);
}


//-------------------------------------------------
//  draw right box tabs
//-------------------------------------------------

void menu_select_launch::draw_right_box_tabs(u32 flags)
{
	m_right_tabs_bottom = m_primary_vbounds.first + line_height();

	float const x1(1.0F - lr_border() - m_right_panel_width);
	float const x2(1.0F - lr_border());
	float const tabwidth = right_tab_width();

	std::string const buffer[RP_LAST + 1] = { _(RIGHT_PANEL_NAMES[0].second), _(RIGHT_PANEL_NAMES[1].second) };

	// check size
	float text_size = 1.0f;
	for (auto & elem : buffer)
	{
		auto textlen = get_string_width(elem) + 0.01f;
		float tmp_size = (textlen > tabwidth) ? (tabwidth / textlen) : 1.0f;
		text_size = std::min(text_size, tmp_size);
	}

	for (int cells = RP_FIRST; cells <= RP_LAST; ++cells)
	{
		float const tableft(x1 + (float(cells - RP_FIRST) * tabwidth));

		rgb_t fgcolor = ui().colors().text_color();
		rgb_t bgcolor = ui().colors().text_bg_color();
		if ((focused_menu::RIGHTTOP == m_focus) && (cells == m_right_panel))
		{
			// draw primary highlight if keyboard focus is here
			fgcolor = rgb_t(0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff);
			ui().draw_textured_box(
					container(),
					tableft + UI_LINE_WIDTH, m_primary_vbounds.first + UI_LINE_WIDTH, tableft + tabwidth - UI_LINE_WIDTH, m_right_tabs_bottom,
					bgcolor, rgb_t(43, 43, 43), hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
		}
		else if (cells != m_right_panel)
		{
			bool const hovered(pointer_in_rect(tableft, m_primary_vbounds.first, tableft + tabwidth, m_right_tabs_bottom));
			bool const pointertab((pointer_action::RIGHT_TRACK_TAB == m_pointer_action) && (cells == m_clicked_line));
			if (pointertab && hovered)
			{
				// draw selected highlight for tracked item
				fgcolor = ui().colors().selected_color();
				bgcolor = ui().colors().selected_bg_color();
				highlight(tableft + UI_LINE_WIDTH, m_primary_vbounds.first + UI_LINE_WIDTH, tableft + tabwidth - UI_LINE_WIDTH, m_right_tabs_bottom, bgcolor);
			}
			else if (pointertab || (!m_ui_error && !(flags & PROCESS_NOINPUT) && hovered && pointer_idle()))
			{
				// draw hover highlight when hovered over or dragged off
				fgcolor = ui().colors().mouseover_color();
				bgcolor = ui().colors().mouseover_bg_color();
				highlight(tableft + UI_LINE_WIDTH, m_primary_vbounds.first + UI_LINE_WIDTH, tableft + tabwidth - UI_LINE_WIDTH, m_right_tabs_bottom, bgcolor);
			}
			else
			{
				// dim unselected tab title
				fgcolor = ui().colors().clone_color();
			}
		}

		ui().draw_text_full(
				container(),
				buffer[cells],
				tableft + UI_LINE_WIDTH, m_primary_vbounds.first, tabwidth - UI_LINE_WIDTH,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER,
				mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr,
				line_height() * text_size);

		// add lines when appropriate
		if (RP_FIRST < cells)
		{
			container().add_line(
					tableft, m_primary_vbounds.first, tableft, m_right_tabs_bottom,
					UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			if (m_right_panel == cells)
			{
				container().add_line(
						x1, m_primary_vbounds.first + line_height(), tableft, m_right_tabs_bottom,
						UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			}
		}
	}
	if (RP_LAST != m_right_panel)
	{
		container().add_line(
				x1 + (float(m_right_panel + 1) * tabwidth), m_right_tabs_bottom, x2, m_right_tabs_bottom,
				UI_LINE_WIDTH, ui().colors().border_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}
}


//-------------------------------------------------
//  draw right box heading
//-------------------------------------------------

void menu_select_launch::draw_right_box_heading(u32 flags, bool larrow, bool rarrow, std::string_view text)
{
	float const text_left(m_right_content_hbounds.first + (2.0F * lr_border()) + lr_arrow_width());
	float const text_width(m_right_content_hbounds.second - m_right_content_hbounds.first - (4.0F * lr_border()) - (2.0F * lr_arrow_width()));

	rgb_t fgcolor(ui().colors().text_color());
	rgb_t bgcolor(ui().colors().text_bg_color());
	if (pointer_action::RIGHT_SWITCH == m_pointer_action)
	{
		// touch swipe to switch
		fgcolor = ui().colors().selected_color();
		bgcolor = ui().colors().selected_bg_color();
		highlight(m_right_content_hbounds.first, m_right_heading_top, m_right_content_hbounds.second, m_right_heading_top + line_height(), bgcolor);
	}
	else if (focused_menu::RIGHTBOTTOM == m_focus)
	{
		// keyboard focus
		fgcolor = rgb_t(0xff, 0xff, 0x00);
		bgcolor = rgb_t(0xff, 0xff, 0xff);
		ui().draw_textured_box(
				container(),
				m_right_content_hbounds.first, m_right_heading_top, m_right_content_hbounds.second, m_right_heading_top + line_height(),
				bgcolor, rgb_t(43, 43, 43),
				hilight_main_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(1));
	}

	ui().draw_text_full(container(),
			text, text_left, m_right_heading_top, text_width,
			text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, mame_ui_manager::NORMAL, fgcolor, bgcolor,
			nullptr, nullptr,
			line_height());

	if (larrow)
	{
		// not using the selected colour here because the background isn't changed
		float const left(m_right_content_hbounds.first + lr_border());
		float const right(m_right_content_hbounds.first + lr_border() + lr_arrow_width());
		bool const hovered(pointer_in_rect(left, right_arrows_top(), right, right_arrows_bottom()));
		bool const tracked((pointer_action::RIGHT_TRACK_ARROW == m_pointer_action) && !m_clicked_line);
		rgb_t fg(fgcolor);
		if ((focused_menu::RIGHTBOTTOM != m_focus) && (tracked || (!m_ui_error && !(flags & PROCESS_NOINPUT) && pointer_idle() && hovered)))
			fg = ui().colors().mouseover_color();
		draw_arrow(left, right_arrows_top(), right, right_arrows_bottom(), fg, ROT90 ^ ORIENTATION_FLIP_X);
	}

	if (rarrow)
	{
		// not using the selected colour here because the background isn't changed
		float const left(m_right_content_hbounds.second - lr_border() - lr_arrow_width());
		float const right(m_right_content_hbounds.second - lr_border());
		bool const hovered(pointer_in_rect(left, right_arrows_top(), right, right_arrows_bottom()));
		bool const tracked((pointer_action::RIGHT_TRACK_ARROW == m_pointer_action) && m_clicked_line);
		rgb_t fg(fgcolor);
		if ((focused_menu::RIGHTBOTTOM != m_focus) && (tracked || (!m_ui_error && !(flags & PROCESS_NOINPUT) && pointer_idle() && hovered)))
			fg = ui().colors().mouseover_color();
		draw_arrow(left, right_arrows_top(), right, right_arrows_bottom(), fg, ROT90);
	}
}


//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void menu_select_launch::arts_render(u32 flags)
{
	// draw the heading
	draw_right_box_heading(flags, FIRST_VIEW < m_image_view, LAST_VIEW > m_image_view, _("selmenu-artwork", std::get<1>(ARTS_INFO[m_image_view])));

	ui_software_info const *software;
	ui_system_info const *system;
	get_selection(software, system);

	if (software && (!software->startempty || !system))
	{
		m_cache.set_snapx_driver(nullptr);

		// loads the image if necessary
		if (!m_cache.snapx_software_is(software) || !snapx_valid() || m_switch_image)
		{
			emu_file snapfile(get_arts_searchpath(), OPEN_FLAG_READ);
			bitmap_argb32 tmp_bitmap;

			if (software->startempty == 1)
			{
				// Load driver snapshot
				load_driver_image(tmp_bitmap, snapfile, *software->driver);
			}
			else
			{
				// First attempt from name list
				load_image(tmp_bitmap, snapfile, util::path_concat(software->listname, software->shortname));

				// Second attempt from driver name + part name
				if (!tmp_bitmap.valid())
					load_image(tmp_bitmap, snapfile, util::path_concat(software->driver->name + software->part, software->shortname));
			}

			m_cache.set_snapx_software(software);
			m_switch_image = false;
			arts_render_images(std::move(tmp_bitmap));
		}

		// if the image is available, loaded and valid, display it
		draw_snapx();
	}
	else if (system)
	{
		m_cache.set_snapx_software(nullptr);

		// loads the image if necessary
		if (!m_cache.snapx_driver_is(system->driver) || !snapx_valid() || m_switch_image)
		{
			emu_file snapfile(get_arts_searchpath(), OPEN_FLAG_READ);
			bitmap_argb32 tmp_bitmap;
			load_driver_image(tmp_bitmap, snapfile, *system->driver);

			m_cache.set_snapx_driver(system->driver);
			m_switch_image = false;
			arts_render_images(std::move(tmp_bitmap));
		}

		// if the image is available, loaded and valid, display it
		draw_snapx();
	}
}


//-------------------------------------------------
//  perform rendering of image
//-------------------------------------------------

void menu_select_launch::arts_render_images(bitmap_argb32 &&tmp_bitmap)
{
	// if it fails, use the default image
	bool no_available(!tmp_bitmap.valid());
	if (no_available)
	{
		tmp_bitmap.allocate(256, 256);
		const bitmap_argb32 &src(m_cache.no_avail_bitmap());
		for (int x = 0; x < 256; x++)
		{
			for (int y = 0; y < 256; y++)
				tmp_bitmap.pix(y, x) = src.pix(y, x);
		}
	}

	bitmap_argb32 &snapx_bitmap(m_cache.snapx_bitmap());
	if (!tmp_bitmap.valid())
	{
		snapx_bitmap.reset();
		return;
	}

	float const panel_width(m_right_content_hbounds.second - m_right_content_hbounds.first);
	float const panel_height(m_right_content_vbounds.second - m_right_content_vbounds.first);

	auto [screen_width, screen_height] = target_size();
	if (machine().render().ui_target().orientation() & ORIENTATION_SWAP_XY)
	{
		using std::swap;
		swap(screen_height, screen_width);
	}

	int const panel_width_pixel(panel_width * screen_width);
	int const panel_height_pixel(panel_height * screen_height);

	// Calculate resize ratios for resizing
	auto const ratioW(float(panel_width_pixel) / float(tmp_bitmap.width()));
	auto const ratioH(float(panel_height_pixel) / float(tmp_bitmap.height()));
	auto const ratioI(float(tmp_bitmap.height()) / float(tmp_bitmap.width()));

	auto dest_xPixel(tmp_bitmap.width());
	auto dest_yPixel(tmp_bitmap.height());
	if (ui().options().forced_4x3_snapshot() && (ratioI < 0.75F) && (m_image_view == SNAPSHOT_VIEW))
	{
		// force 4:3 ratio min
		dest_yPixel = tmp_bitmap.width() * 0.75F;
		float const ratio = std::min(ratioW, float(panel_height_pixel) / float(dest_yPixel));
		dest_xPixel = tmp_bitmap.width() * ratio;
		dest_yPixel *= ratio;
	}
	else if ((ratioW < 1.0F) || (ratioH < 1.0F) || (ui().options().enlarge_snaps() && !no_available))
	{
		// resize the bitmap if necessary
		float const ratio(std::min(ratioW, ratioH));
		dest_xPixel = tmp_bitmap.width() * ratio;
		dest_yPixel = tmp_bitmap.height() * ratio;
	}


	// resample if necessary
	bitmap_argb32 dest_bitmap;
	if ((dest_xPixel != tmp_bitmap.width()) || (dest_yPixel != tmp_bitmap.height()))
	{
		dest_bitmap.allocate(dest_xPixel, dest_yPixel);
		render_resample_argb_bitmap_hq(dest_bitmap, tmp_bitmap, render_color{ 1.0F, 1.0F, 1.0F, 1.0F }, true);
	}
	else
	{
		dest_bitmap = std::move(tmp_bitmap);
	}

	snapx_bitmap.allocate(panel_width_pixel, panel_height_pixel);
	int x1(0.5F * (float(panel_width_pixel) - float(dest_xPixel)));
	int y1(0.5F * (float(panel_height_pixel) - float(dest_yPixel)));

	for (int x = 0; x < dest_xPixel; x++)
		for (int y = 0; y < dest_yPixel; y++)
			snapx_bitmap.pix(y + y1, x + x1) = dest_bitmap.pix(y, x);

	// apply bitmap
	m_cache.snapx_texture()->set_bitmap(snapx_bitmap, snapx_bitmap.cliprect(), TEXFORMAT_ARGB32);
}


//-------------------------------------------------
//  draw snapshot
//-------------------------------------------------

void menu_select_launch::draw_snapx()
{
	// if the image is available, loaded and valid, display it
	if (snapx_valid())
	{
		container().add_quad(
				m_right_content_hbounds.first, m_right_content_vbounds.first, m_right_content_hbounds.second, m_right_content_vbounds.second,
				rgb_t::white(), m_cache.snapx_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}
}


char const *menu_select_launch::right_panel_config_string() const
{
	assert(std::size(RIGHT_PANEL_NAMES) > m_right_panel);
	return RIGHT_PANEL_NAMES[m_right_panel].first;
}

char const *menu_select_launch::right_image_config_string() const
{
	assert(std::size(ARTS_INFO) > m_image_view);
	return std::get<0>(ARTS_INFO[m_image_view]);
}

void menu_select_launch::set_right_panel(u8 index)
{
	assert(std::size(RIGHT_PANEL_NAMES) > index);
	m_right_panel = index;
}

void menu_select_launch::set_right_image(u8 index)
{
	assert(std::size(ARTS_INFO) > index);
	if (index != m_image_view)
	{
		m_image_view = index;
		set_switch_image();
	}
}

void menu_select_launch::set_right_panel(std::string_view value)
{
	auto const found = std::find_if(
			std::begin(RIGHT_PANEL_NAMES),
			std::end(RIGHT_PANEL_NAMES),
			[&value] (auto const &that) { return value == that.first; });
	if (std::end(RIGHT_PANEL_NAMES) != found)
		m_right_panel = found - std::begin(RIGHT_PANEL_NAMES);
}

void menu_select_launch::set_right_image(std::string_view value)
{
	auto const found = std::find_if(
			std::begin(ARTS_INFO),
			std::end(ARTS_INFO),
			[&value] (auto const &that) { return value == std::get<0>(that); });
	if (std::end(ARTS_INFO) != found)
		m_image_view = found - std::begin(ARTS_INFO);
}


std::string menu_select_launch::make_system_audit_fail_text(media_auditor const &auditor, media_auditor::summary summary)
{
	std::ostringstream str;
	if (!auditor.records().empty())
	{
		str << "System media audit failed:\n";
		auditor.summarize(nullptr, &str);
		osd_printf_info(str.str());
		str.str("");
	}
	str << _("Required ROM/disk images for the selected system are missing or incorrect. Please acquire the correct files or select a different system.\n\n");
	make_audit_fail_text(str, auditor, summary);
	return str.str();
}


std::string menu_select_launch::make_software_audit_fail_text(media_auditor const &auditor, media_auditor::summary summary)
{
	std::ostringstream str;
	if (!auditor.records().empty())
	{
		str << "System media audit failed:\n";
		auditor.summarize(nullptr, &str);
		osd_printf_info(str.str());
		str.str("");
	}
	str << _("Required ROM/disk images for the selected software are missing or incorrect. Please acquire the correct files or select a different software item.\n\n");
	make_audit_fail_text(str, auditor, summary);
	return str.str();
}


void menu_select_launch::make_audit_fail_text(std::ostream &str, media_auditor const &auditor, media_auditor::summary summary)
{
	if ((media_auditor::NOTFOUND != summary) && !auditor.records().empty())
	{
		char const *message = nullptr;
		for (media_auditor::audit_record const &record : auditor.records())
		{
			switch (record.substatus())
			{
			case media_auditor::audit_substatus::FOUND_BAD_CHECKSUM:
				message = _("incorrect checksum");
				break;
			case media_auditor::audit_substatus::FOUND_WRONG_LENGTH:
				message = _("incorrect length");
				break;
			case media_auditor::audit_substatus::NOT_FOUND:
				message = _("not found");
				break;
			case media_auditor::audit_substatus::GOOD:
			case media_auditor::audit_substatus::GOOD_NEEDS_REDUMP:
			case media_auditor::audit_substatus::FOUND_NODUMP:
			case media_auditor::audit_substatus::NOT_FOUND_NODUMP:
			case media_auditor::audit_substatus::NOT_FOUND_OPTIONAL:
			case media_auditor::audit_substatus::UNVERIFIED:
				continue;
			}
			if (record.shared_device())
				util::stream_format(str, _("%1$s (%2$s) - %3$s\n"), record.name(), record.shared_device()->shortname(), message);
			else
				util::stream_format(str, _("%1$s - %2$s\n"), record.name(), message);
		}
		str << '\n';
	}
	str << _("Press any key to continue.");
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
		u32 const bios_flags(bios.get_value());

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


//-------------------------------------------------
//  draw infos
//-------------------------------------------------

void menu_select_launch::infos_render(u32 flags)
{
	std::string_view first;
	ui_software_info const *software;
	ui_system_info const *system;
	int total;
	get_selection(software, system);

	if (software && !software->startempty)
	{
		m_info_driver = nullptr;
		first = _("Software List Info");

		if ((m_info_software != software) || (m_info_view != ui_globals::cur_sw_dats_view))
		{
			m_info_buffer.clear();
			m_info_layout = std::nullopt;
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
				m_info_buffer = software->infotext;
			}
			else
			{
				m_info_buffer.clear();
				mame_machine_manager::instance()->lua()->call_plugin("data", m_info_view - 1, m_info_buffer);
			}
		}
		total = ui_globals::cur_sw_dats_total;
	}
	else if (system || (software && software->driver))
	{
		game_driver const &driver(system ? *system->driver : *software->driver);
		m_info_software = nullptr;
		first = _("General Info");

		if (&driver != m_info_driver || ui_globals::curdats_view != m_info_view)
		{
			m_info_buffer.clear();
			m_info_layout = std::nullopt;
			if (&driver == m_info_driver)
			{
				m_info_view = ui_globals::curdats_view;
			}
			else
			{
				m_info_driver = &driver;
				m_info_view = 0;
				ui_globals::curdats_view = 0;

				m_items_list.clear();
				mame_machine_manager::instance()->lua()->call_plugin("data_list", driver.name, m_items_list);
				ui_globals::curdats_total = m_items_list.size() + 1;
			}

			if (m_info_view == 0)
			{
				general_info(system, driver, m_info_buffer);
			}
			else
			{
				m_info_buffer.clear();
				mame_machine_manager::instance()->lua()->call_plugin("data", m_info_view - 1, m_info_buffer);
			}
		}
		total = ui_globals::curdats_total;
	}
	else
	{
		return;
	}

	// draw the heading
	std::string_view const snaptext(m_info_view ? std::string_view(m_items_list[m_info_view - 1]) : first);
	draw_right_box_heading(flags, 0 < m_info_view, (total - 1) > m_info_view, snaptext);

	float const sc(m_right_panel_width - (2.0F * gutter_width()));
	if (!m_info_layout || (m_info_layout->width() != sc))
	{
		m_info_layout.emplace(
				*ui().get_font(),
				x_aspect() * m_info_line_height, m_info_line_height,
				sc,
				text_layout::text_justify::LEFT, text_layout::word_wrapping::WORD);
		menu_dats_view::add_info_text(*m_info_layout, m_info_buffer, ui().colors().text_color());
		m_total_lines = m_info_layout->lines();
	}

	m_right_visible_lines = floor((m_right_content_vbounds.second - m_right_content_vbounds.first) / m_info_line_height);
	if (m_total_lines < m_right_visible_lines)
		m_right_visible_lines = m_total_lines;
	if (m_topline_datsview < 0)
		m_topline_datsview = 0;
	if ((m_topline_datsview + m_right_visible_lines) >= m_total_lines)
		m_topline_datsview = m_total_lines - m_right_visible_lines;

	// get the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	bool const up_arrow(!info_at_top());
	bool const down_arrow(!info_at_bottom());
	int const r_visible_lines(m_right_visible_lines - (up_arrow ? 1 : 0) - (down_arrow ? 1 : 0));

	if (up_arrow)
		draw_info_arrow(flags, 0);
	if (down_arrow)
		draw_info_arrow(flags, m_right_visible_lines - 1);

	m_info_layout->emit(
			container(),
			m_topline_datsview ? (m_topline_datsview + 1) : 0, r_visible_lines,
			right_panel_left() + gutter_width(), m_right_content_vbounds.first + (m_topline_datsview ? m_info_line_height : 0.0f));
}


//-------------------------------------------------
//  generate general info
//-------------------------------------------------

void menu_select_launch::general_info(ui_system_info const *system, game_driver const &driver, std::string &buffer)
{
	system_flags const &flags(get_system_flags(driver));
	std::ostringstream str;

	str << "#j2\n";

	if (system)
		str << system->description;
	else
		str << driver.type.fullname();
	str << "\t\n\n";

	util::stream_format(str, _("Short Name\t%1$s\n"), driver.name);
	util::stream_format(str, _("Year\t%1$s\n"), driver.year);
	util::stream_format(str, _("Manufacturer\t%1$s\n"), driver.manufacturer);

	int cloneof = driver_list::non_bios_clone(driver);
	if (0 <= cloneof)
	{
		util::stream_format(
				str,
				_("System is Clone of\t%1$s\n"),
				system ? std::string_view(system->parent) : std::string_view(driver_list::driver(cloneof).type.fullname()));
	}
	else
	{
		str << _("System is Parent\t\n");
	}

	if (flags.has_analog())
		str << _("Analog Controls\tYes\n");
	if (flags.has_keyboard())
		str << _("Keyboard Inputs\tYes\n");

	if (flags.machine_flags() & machine_flags::NOT_WORKING)
		str << _("Overall\tNOT WORKING\n");
	else if ((flags.unemulated_features() | flags.imperfect_features()) & device_t::feature::PROTECTION)
		str << _("Overall\tUnemulated Protection\n");
	else
		str << _("Overall\tWorking\n");

	if (flags.unemulated_features() & device_t::feature::GRAPHICS)
		str << _("Graphics\tUnimplemented\n");
	else if (flags.unemulated_features() & device_t::feature::PALETTE)
		str << _("Graphics\tWrong Colors\n");
	else if (flags.imperfect_features() & device_t::feature::PALETTE)
		str << _("Graphics\tImperfect Colors\n");
	else if (flags.imperfect_features() & device_t::feature::GRAPHICS)
		str << _("Graphics\tImperfect\n");
	else
		str << _("Graphics\tOK\n");

	if (flags.machine_flags() & machine_flags::NO_SOUND_HW)
		str << _("Sound\tNone\n");
	else if (flags.unemulated_features() & device_t::feature::SOUND)
		str << _("Sound\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::SOUND)
		str << _("Sound\tImperfect\n");
	else
		str << _("Sound\tOK\n");

	if (flags.unemulated_features() & device_t::feature::CAPTURE)
		str << _("Capture\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::CAPTURE)
		str << _("Capture\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::CAMERA)
		str << _("Camera\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::CAMERA)
		str << _("Camera\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::MICROPHONE)
		str << _("Microphone\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::MICROPHONE)
		str << _("Microphone\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::CONTROLS)
		str << _("Controls\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::CONTROLS)
		str << _("Controls\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::KEYBOARD)
		str << _("Keyboard\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::KEYBOARD)
		str << _("Keyboard\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::MOUSE)
		str << _("Mouse\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::MOUSE)
		str << _("Mouse\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::MEDIA)
		str << _("Media\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::MEDIA)
		str << _("Media\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::DISK)
		str << _("Disk\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::DISK)
		str << _("Disk\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::PRINTER)
		str << _("Printer\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::PRINTER)
		str << _("Printer\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::TAPE)
		str << _("Mag. Tape\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::TAPE)
		str << _("Mag. Tape\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::PUNCH)
		str << _("Punch Tape\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::PUNCH)
		str << _("Punch Tape\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::DRUM)
		str << _("Mag. Drum\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::DRUM)
		str << _("Mag. Drum\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::ROM)
		str << _("(EP)ROM\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::ROM)
		str << _("(EP)ROM\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::COMMS)
		str << _("Communications\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::COMMS)
		str << _("Communications\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::LAN)
		str << _("LAN\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::LAN)
		str << _("LAN\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::WAN)
		str << _("WAN\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::WAN)
		str << _("WAN\tImperfect\n");

	if (flags.unemulated_features() & device_t::feature::TIMING)
		str << _("Timing\tUnimplemented\n");
	else if (flags.imperfect_features() & device_t::feature::TIMING)
		str << _("Timing\tImperfect\n");

	str << ((flags.machine_flags() & machine_flags::MECHANICAL)        ? _("Mechanical System\tYes\n")          : _("Mechanical System\tNo\n"));
	str << ((flags.machine_flags() & machine_flags::REQUIRES_ARTWORK)  ? _("Requires Artwork\tYes\n")           : _("Requires Artwork\tNo\n"));
	if (flags.machine_flags() & machine_flags::NO_COCKTAIL)
		str << _("Support Cocktail\tNo\n");
	str << ((flags.machine_flags() & machine_flags::IS_BIOS_ROOT)      ? _("System is BIOS\tYes\n")             : _("System is BIOS\tNo\n"));
	str << ((flags.machine_flags() & machine_flags::SUPPORTS_SAVE)     ? _("Support Save\tYes\n")               : _("Support Save\tNo\n"));
	str << ((flags.machine_flags() & ORIENTATION_SWAP_XY)              ? _("Screen Orientation\tVertical\n")    : _("Screen Orientation\tHorizontal\n"));
	bool found = false;
	for (romload::region const &region : romload::entries(driver.rom).get_regions())
	{
		if (region.is_diskdata())
		{
			found = true;
			break;
		}
	}
	str << (found ? _("Requires CHD\tYes\n") : _("Requires CHD\tNo\n"));

	// audit the game first to see if we're going to work
	if (ui().options().info_audit())
	{
		driver_enumerator enumerator(machine().options(), driver);
		enumerator.next();
		media_auditor auditor(enumerator);
		media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);
		media_auditor::summary summary_samples = auditor.audit_samples();

		// if everything looks good, schedule the new driver
		if (audit_passed(summary))
			str << _("Media Audit Result\tOK\n");
		else
			str << _("Media Audit Result\tBAD\n");

		if (summary_samples == media_auditor::NONE_NEEDED)
			str << _("Samples Audit Result\tNone Needed\n");
		else if (audit_passed(summary_samples))
			str << _("Samples Audit Result\tOK\n");
		else
			str << _("Samples Audit Result\tBAD\n");
	}
	else
	{
		str << _("Media Audit\tDisabled\nSamples Audit\tDisabled\n");
	}

	util::stream_format(str, _("Source File\t%1$s\n"), info_xml_creator::format_sourcefile(driver.type.source()));

	buffer = std::move(str).str();
}

} // namespace ui
