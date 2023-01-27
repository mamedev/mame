// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/moptions.cpp

    UI main options manager.

***************************************************************************/

#include "emu.h"
#include "options.h"
#include "ui/moptions.h"


//**************************************************************************
//  UI EXTRA OPTIONS
//**************************************************************************

const options_entry ui_options::s_option_entries[] =
{
	// search path options
	{ nullptr,                              nullptr,                       option_type::HEADER,     "UI SEARCH PATH OPTIONS" },
	{ OPTION_HISTORY_PATH,                  "history;dats;.",              option_type::MULTIPATH,  "path to system/software info files" },
	{ OPTION_CATEGORYINI_PATH,              "folders",                     option_type::MULTIPATH,  "path to category ini files" },
	{ OPTION_CABINETS_PATH,                 "cabinets;cabdevs",            option_type::MULTIPATH,  "path to cabinets / devices image" },
	{ OPTION_CPANELS_PATH,                  "cpanel",                      option_type::MULTIPATH,  "path to control panel image" },
	{ OPTION_PCBS_PATH,                     "pcb",                         option_type::MULTIPATH,  "path to pcbs image" },
	{ OPTION_FLYERS_PATH,                   "flyers",                      option_type::MULTIPATH,  "path to flyers image" },
	{ OPTION_TITLES_PATH,                   "titles",                      option_type::MULTIPATH,  "path to titles image" },
	{ OPTION_ENDS_PATH,                     "ends",                        option_type::MULTIPATH,  "path to ends image" },
	{ OPTION_MARQUEES_PATH,                 "marquees",                    option_type::MULTIPATH,  "path to marquees image" },
	{ OPTION_ARTPREV_PATH,                  "artwork preview;artpreview",  option_type::MULTIPATH,  "path to artwork preview image" },
	{ OPTION_BOSSES_PATH,                   "bosses",                      option_type::MULTIPATH,  "path to bosses image" },
	{ OPTION_LOGOS_PATH,                    "logo",                        option_type::MULTIPATH,  "path to logos image" },
	{ OPTION_SCORES_PATH,                   "scores",                      option_type::MULTIPATH,  "path to scores image" },
	{ OPTION_VERSUS_PATH,                   "versus",                      option_type::MULTIPATH,  "path to versus image" },
	{ OPTION_GAMEOVER_PATH,                 "gameover",                    option_type::MULTIPATH,  "path to gameover image" },
	{ OPTION_HOWTO_PATH,                    "howto",                       option_type::MULTIPATH,  "path to howto image" },
	{ OPTION_SELECT_PATH,                   "select",                      option_type::MULTIPATH,  "path to select image" },
	{ OPTION_ICONS_PATH,                    "icons",                       option_type::MULTIPATH,  "path to ICOns image" },
	{ OPTION_COVER_PATH,                    "covers",                      option_type::MULTIPATH,  "path to software cover image" },
	{ OPTION_UI_PATH,                       "ui",                          option_type::MULTIPATH,  "path to UI files" },

	// misc options
	{ nullptr,                              nullptr,    option_type::HEADER,      "UI MISC OPTIONS" },
	{ OPTION_SYSTEM_NAMES,                  "",         option_type::MULTIPATH,   "translated system names file" },
	{ OPTION_SKIP_WARNINGS,                 "0",        option_type::BOOLEAN,     "display fewer repeated warnings about imperfect emulation" },
	{ OPTION_UNTHROTTLE_MUTE ";utm",        "0",        option_type::BOOLEAN,     "mute audio when running unthrottled" },

	// UI options
	{ nullptr,                              nullptr,        option_type::HEADER,      "UI OPTIONS" },
	{ OPTION_INFOS_SIZE "(0.20-1.00)",      "0.75",         option_type::FLOAT,       "UI right panel infos text size (0.20 - 1.00)" },
	{ OPTION_FONT_ROWS "(25-40)",           "30",           option_type::INTEGER,     "UI font lines per screen (25 - 40)" },
	{ OPTION_UI_BORDER_COLOR,               "ffffffff",     option_type::STRING,      "UI border color (ARGB)" },
	{ OPTION_UI_BACKGROUND_COLOR,           "ef101030",     option_type::STRING,      "UI background color (ARGB)" },
	{ OPTION_UI_CLONE_COLOR,                "ff808080",     option_type::STRING,      "UI clone color (ARGB)" },
	{ OPTION_UI_DIPSW_COLOR,                "ffffff00",     option_type::STRING,      "UI dipswitch color (ARGB)" },
	{ OPTION_UI_GFXVIEWER_BG_COLOR,         "ef101030",     option_type::STRING,      "UI gfx viewer color (ARGB)" },
	{ OPTION_UI_MOUSEDOWN_BG_COLOR,         "b0606000",     option_type::STRING,      "UI mouse down bg color (ARGB)" },
	{ OPTION_UI_MOUSEDOWN_COLOR,            "ffffff80",     option_type::STRING,      "UI mouse down color (ARGB)" },
	{ OPTION_UI_MOUSEOVER_BG_COLOR,         "70404000",     option_type::STRING,      "UI mouse over bg color (ARGB)" },
	{ OPTION_UI_MOUSEOVER_COLOR,            "ffffff80",     option_type::STRING,      "UI mouse over color (ARGB)" },
	{ OPTION_UI_SELECTED_BG_COLOR,          "ef808000",     option_type::STRING,      "UI selected bg color (ARGB)" },
	{ OPTION_UI_SELECTED_COLOR,             "ffffff00",     option_type::STRING,      "UI selected color (ARGB)" },
	{ OPTION_UI_SLIDER_COLOR,               "ffffffff",     option_type::STRING,      "UI slider color (ARGB)" },
	{ OPTION_UI_SUBITEM_COLOR,              "ffffffff",     option_type::STRING,      "UI subitem color (ARGB)" },
	{ OPTION_UI_TEXT_BG_COLOR,              "ef000000",     option_type::STRING,      "UI text bg color (ARGB)" },
	{ OPTION_UI_TEXT_COLOR,                 "ffffffff",     option_type::STRING,      "UI text color (ARGB)" },
	{ OPTION_UI_UNAVAILABLE_COLOR,          "ff404040",     option_type::STRING,      "UI unavailable color (ARGB)" },

	// system/software selection menu options
	{ nullptr,                              nullptr,    option_type::HEADER,      "SYSTEM/SOFTWARE SELECTION MENU OPTIONS" },
	{ OPTION_HIDE_PANELS "(0-3)",           "0",        option_type::INTEGER,     "UI hide left/right panel in main view (0 = Show all, 1 = hide left, 2 = hide right, 3 = hide both" },
	{ OPTION_USE_BACKGROUND,                "1",        option_type::BOOLEAN,     "enable background image in main view" },
	{ OPTION_SKIP_BIOS_MENU,                "0",        option_type::BOOLEAN,     "skip bios submenu, start with configured or default" },
	{ OPTION_SKIP_PARTS_MENU,               "0",        option_type::BOOLEAN,     "skip parts submenu, start with first part" },
	{ OPTION_REMEMBER_LAST,                 "1",        option_type::BOOLEAN,     "initially select last used system in main menu" },
	{ OPTION_LAST_USED_MACHINE,             "",         option_type::STRING,      "last selected system" },
	{ OPTION_LAST_USED_FILTER,              "",         option_type::STRING,      "last used system filter" },
	{ OPTION_SYSTEM_RIGHT_PANEL,            "image",    option_type::STRING,      "selected system right panel tab" },
	{ OPTION_SOFTWARE_RIGHT_PANEL,          "image",    option_type::STRING,      "selected software right panel tab" },
	{ OPTION_SYSTEM_RIGHT_IMAGE,            "snap",     option_type::STRING,      "selected system right panel image" },
	{ OPTION_SOFTWARE_RIGHT_IMAGE,          "snap",     option_type::STRING,      "selected software right panel image" },
	{ OPTION_ENLARGE_SNAPS,                 "1",        option_type::BOOLEAN,     "enlarge images in right panel (keeping aspect ratio)" },
	{ OPTION_FORCED4X3,                     "1",        option_type::BOOLEAN,     "force 4:3 aspect ratio for snapshots in the software menu" },
	{ OPTION_INFO_AUTO_AUDIT,               "0",        option_type::BOOLEAN,     "automatically audit media for the general info panel" },
	{ OPTION_HIDE_ROMLESS,                  "1",        option_type::BOOLEAN,     "hide systems that don't require ROMs in the available system filter" },

	// sentinel
	{ nullptr }
};

//-------------------------------------------------
//  ui_options - constructor
//-------------------------------------------------

ui_options::ui_options() : core_options()
{
	add_entries(ui_options::s_option_entries);
}

//-------------------------------------------------
//  rgb_value - decode an RGB option
//-------------------------------------------------

rgb_t ui_options::rgb_value(const char *option) const
{
	// find the entry
	core_options::entry::shared_const_ptr entry = get_entry(option);

	// look up the value, and sanity check the result
	const char *value = entry->value();
	int len = strlen(value);
	if (len != 8)
		value = entry->default_value().c_str();

	// convert to an rgb_t
	return rgb_t((uint32_t)strtoul(value, nullptr, 16));
}
