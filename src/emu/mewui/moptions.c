// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/options.c

    MEWUI main options manager.

***************************************************************************/

#include "emu.h"
#include "mewui/moptions.h"


//**************************************************************************
//  MEWUI EXTRA OPTIONS
//**************************************************************************

const options_entry mewui_options::s_option_entries[] =
{
	// seach path options
	{ NULL,                                 NULL,  OPTION_HEADER,  "MEWUI SEARCH PATH OPTIONS" },
	{ OPTION_HISTORY_PATH,             "history",  OPTION_STRING,  "path to history files" },
	{ OPTION_EXTRAINI_PATH,            "folders",  OPTION_STRING,  "path to extra ini files" },
	{ OPTION_CABINETS_PATH,   "cabinets;cabdevs",  OPTION_STRING,  "path to cabinets / devices image" },
	{ OPTION_CPANELS_PATH,              "cpanel",  OPTION_STRING,  "path to control panel image" },
	{ OPTION_PCBS_PATH,                    "pcb",  OPTION_STRING,  "path to pcbs image" },
	{ OPTION_FLYERS_PATH,               "flyers",  OPTION_STRING,  "path to flyers image" },
	{ OPTION_TITLES_PATH,               "titles",  OPTION_STRING,  "path to titles image" },
	{ OPTION_MARQUEES_PATH,           "marquees",  OPTION_STRING,  "path to marquees image" },
	{ OPTION_ARTPREV_PATH,     "artwork preview",  OPTION_STRING,  "path to artwork preview image" },
	{ OPTION_BOSSES_PATH,               "bosses",  OPTION_STRING,  "path to bosses image" },
	{ OPTION_LOGOS_PATH,                  "logo",  OPTION_STRING,  "path to logos image" },
	{ OPTION_SCORES_PATH,               "scores",  OPTION_STRING,  "path to scores image" },
	{ OPTION_VERSUS_PATH,               "versus",  OPTION_STRING,  "path to versus image" },
	{ OPTION_GAMEOVER_PATH,           "gameover",  OPTION_STRING,  "path to gameover image" },
	{ OPTION_HOWTO_PATH,                 "howto",  OPTION_STRING,  "path to howto image" },
	{ OPTION_SELECT_PATH,               "select",  OPTION_STRING,  "path to select image" },
	{ OPTION_ICONS_PATH,                 "icons",  OPTION_STRING,  "path to ICOns image" },
	{ OPTION_MEWUI_PATH,                 "mewui",  OPTION_STRING,  "path to MEWUI files" },

	// misc options
	{ NULL,                                 NULL,   OPTION_HEADER, "MEWUI MISC OPTIONS" },
	{ OPTION_DATS_ENABLED,                   "1",  OPTION_BOOLEAN, "enable DATs support" },
	{ OPTION_REMEMBER_LAST,                  "1",  OPTION_BOOLEAN, "reselect in main menu last played game" },
	{ OPTION_ENLARGE_SNAPS,                  "1",  OPTION_BOOLEAN, "enlarge arts (snapshot, title, etc...) in right panel (keeping aspect ratio)" },
	{ OPTION_FORCED4X3,                      "1",  OPTION_BOOLEAN, "force the appearance of the snapshot in the list software to 4:3" },
	{ OPTION_USE_BACKGROUND,                 "0",  OPTION_BOOLEAN, "enable background image in main view" },
	{ OPTION_SKIP_BIOS_MENU,                 "0",  OPTION_BOOLEAN, "skip bios submenu, start with configured or default" },
	{ OPTION_SKIP_PARTS_MENU,                "0",  OPTION_BOOLEAN, "skip parts submenu, start with first part" },
	{ OPTION_START_FILTER,                   "0",  OPTION_INTEGER, "startup filter (0 = ALL, 1 = ARCADES, 2 = SYSTEMS)" },

	// UI options
	{ NULL,                                 NULL,   OPTION_HEADER, "MEWUI UI OPTIONS" },
	{ OPTION_INFOS_SIZE "(0.05-1.00)",    "0.75",    OPTION_FLOAT, "UI right panel infos text size (0.05 - 1.00)" },
	{ OPTION_FONT_ROWS "(25-40)",           "30",  OPTION_INTEGER, "UI font text size (25 - 40)" },
	{ OPTION_UI_BORDER_COLOR,         "ffffffff",   OPTION_STRING, "UI border color (ARGB)" },
	{ OPTION_UI_BACKGROUND_COLOR,     "ef101030",   OPTION_STRING, "UI background color (ARGB)" },
	{ OPTION_UI_CLONE_COLOR,          "ff808080",   OPTION_STRING, "UI clone color (ARGB)" },
	{ OPTION_UI_DIPSW_COLOR,          "ffffff00",   OPTION_STRING, "UI dipswitch color (ARGB)" },
	{ OPTION_UI_GFXVIEWER_BG_COLOR,   "ef101030",   OPTION_STRING, "UI gfx viewer color (ARGB)" },
	{ OPTION_UI_MOUSEDOWN_BG_COLOR,   "b0606000",   OPTION_STRING, "UI mouse down bg color (ARGB)" },
	{ OPTION_UI_MOUSEDOWN_COLOR,      "ffffff80",   OPTION_STRING, "UI mouse down color (ARGB)" },
	{ OPTION_UI_MOUSEOVER_BG_COLOR,   "70404000",   OPTION_STRING, "UI mouse over bg color (ARGB)" },
	{ OPTION_UI_MOUSEOVER_COLOR,      "ffffff80",   OPTION_STRING, "UI mouse over color (ARGB)" },
	{ OPTION_UI_SELECTED_BG_COLOR,    "ef808000",   OPTION_STRING, "UI selected bg color (ARGB)" },
	{ OPTION_UI_SELECTED_COLOR,       "ffffff00",   OPTION_STRING, "UI selected color (ARGB)" },
	{ OPTION_UI_SLIDER_COLOR,         "ffffffff",   OPTION_STRING, "UI slider color (ARGB)" },
	{ OPTION_UI_SUBITEM_COLOR,        "ffffffff",   OPTION_STRING, "UI subitem color (ARGB)" },
	{ OPTION_UI_TEXT_BG_COLOR,        "ef000000",   OPTION_STRING, "UI text bg color (ARGB)" },
	{ OPTION_UI_TEXT_COLOR,           "ffffffff",   OPTION_STRING, "UI text color (ARGB)" },
	{ OPTION_UI_UNAVAILABLE_COLOR,    "ff404040",   OPTION_STRING, "UI unavailable color (ARGB)" },
	{ NULL }
};



//**************************************************************************
//  EMU OPTIONS
//**************************************************************************

//-------------------------------------------------
//  mewui_options - constructor
//-------------------------------------------------

mewui_options::mewui_options()
: core_options()
{
	add_entries(mewui_options::s_option_entries);
}
