/***************************************************************************

	mewui/options.c

	MEWUI options management.

***************************************************************************/

#include "emu.h"
#include "mewui/options.h"


//**************************************************************************
//  MEWUI EXTRA OPTIONS
//**************************************************************************

const options_entry mewui_options::s_option_entries[] =
{
	// seach path options
	{ NULL,								         NULL,	OPTION_HEADER,	"MEWUI SEARCH PATH OPTIONS" },
	{ OPTION_HISTORYPATH,				    "history",  OPTION_STRING,	"path to history files" },
	{ OPTION_EXTRAINIPATH,				    "folders",  OPTION_STRING,	"path to extra ini files" },
	{ OPTION_CABINETS_DIRECTORY,   "cabinets;cabdevs",  OPTION_STRING,	"directory to load cabinets / devices image" },
	{ OPTION_CPANELS_DIRECTORY,			     "cpanel",	OPTION_STRING,	"directory to load control panel image" },
	{ OPTION_PCBS_DIRECTORY,				    "pcb",	OPTION_STRING,	"directory to load pcbs image" },
	{ OPTION_FLYERS_DIRECTORY,			     "flyers",	OPTION_STRING,	"directory to load flyers image" },
	{ OPTION_TITLES_DIRECTORY,			     "titles",	OPTION_STRING,	"directory to load titles image" },
	{ OPTION_MARQUEES_DIRECTORY,		   "marquees",  OPTION_STRING,	"directory to load marquees image" },
	{ OPTION_ARTPREV_DIRECTORY,		"artwork preview",  OPTION_STRING,	"directory to load artwork preview image" },
	{ OPTION_BOSSES_DIRECTORY,			     "bosses",	OPTION_STRING,	"directory to load game's bosses image" },
	{ OPTION_LOGOS_DIRECTORY,			       "logo",	OPTION_STRING,	"directory to load logos image" },
	{ OPTION_SCORES_DIRECTORY,			     "scores",	OPTION_STRING,	"directory to load scores image" },
	{ OPTION_VERSUS_DIRECTORY,			     "versus",	OPTION_STRING,	"directory to load versus image" },
	{ OPTION_GAMEOVER_DIRECTORY,		   "gameover",  OPTION_STRING,	"directory to load gameover image" },
	{ OPTION_HOWTO_DIRECTORY,			      "howto",	OPTION_STRING,	"directory to load howto image" },
	{ OPTION_SELECT_DIRECTORY,			     "select",	OPTION_STRING,	"directory to load select image" },
	{ OPTION_ICONS_DIRECTORY,			      "icons",	OPTION_STRING,	"directory to load ICOns image" },


	// misc options
	{ NULL,								 		 NULL,	 OPTION_HEADER,	"MEWUI MISC OPTIONS" },
	{ OPTION_DATS_ENABLED,				  		  "1",	OPTION_BOOLEAN,	"enable DATs support" },
	{ OPTION_REMEMBER_LAST,				  		  "1",	OPTION_BOOLEAN,	"reselect in main menu last played game" },
	{ OPTION_ENLARGE_SNAPS,				  		  "1",	OPTION_BOOLEAN,	"enlarge arts (snapshot, title, etc...) in right panel (keeping aspect ratio)" },
	{ OPTION_FORCED4X3,					  		  "1",	OPTION_BOOLEAN,	"force the appearance of the snapshot in the list software to 4:3" },
	{ OPTION_GROUPED,					  		  "1",	OPTION_BOOLEAN,	"set display mode in selection game menu" },
	{ OPTION_AUDIT_MODE,				  		  "0",	OPTION_BOOLEAN,	"set audit mode at mewui start" },
	{ OPTION_USE_BACKGROUND,			  		  "0",	OPTION_BOOLEAN,	"use background images in main view" },

	// UI options
	{ NULL,								      	 NULL,	 OPTION_HEADER,	"MEWUI UI OPTIONS" },
	{ OPTION_INFOS_SIZE "(0.05-1.00)",	       "0.75",	  OPTION_FLOAT,	"right panel infos text size (0.05 - 1.00)" },
	{ OPTION_FONT_ROWS "(25-40)",		      	 "30",	OPTION_INTEGER, "UI font text size (25 - 40)" },
	{ OPTION_UI_BORDER_COLOR,			   "ffffffff",   OPTION_STRING, "UI border color (ARGB)" },
	{ OPTION_UI_BACKGROUND_COLOR,		   "ef101030",   OPTION_STRING, "UI background color (ARGB)" },
	{ OPTION_UI_CLONE_COLOR,			   "ff808080",   OPTION_STRING, "UI clone color (ARGB)" },
	{ OPTION_UI_DIPSW_COLOR,			   "ffffff00",   OPTION_STRING, "UI dipswitch color (ARGB)" },
	{ OPTION_UI_GFXVIEWER_BG_COLOR,		   "ef101030",   OPTION_STRING, "UI gfx viewer color (ARGB)" },
	{ OPTION_UI_MOUSEDOWN_BG_COLOR,		   "b0606000",   OPTION_STRING, "UI mouse down bg color (ARGB)" },
	{ OPTION_UI_MOUSEDOWN_COLOR,		   "ffffff80",   OPTION_STRING, "UI mouse down color (ARGB)" },
	{ OPTION_UI_MOUSEOVER_BG_COLOR,		   "70404000",   OPTION_STRING, "UI mouse over bg color (ARGB)" },
	{ OPTION_UI_MOUSEOVER_COLOR,		   "ffffff80",   OPTION_STRING, "UI mouse over color (ARGB)" },
	{ OPTION_UI_SELECTED_BG_COLOR,	       "ef808000",   OPTION_STRING, "UI selected bg color (ARGB)" },
	{ OPTION_UI_SELECTED_COLOR,			   "ffffff00",   OPTION_STRING, "UI selected color (ARGB)" },
	{ OPTION_UI_SLIDER_COLOR,			   "ffffffff",   OPTION_STRING, "UI slider color (ARGB)" },
	{ OPTION_UI_SUBITEM_COLOR,	           "ffffffff",   OPTION_STRING, "UI subitem color (ARGB)" },
	{ OPTION_UI_TEXT_BG_COLOR,			   "ef000000",   OPTION_STRING, "UI text bg color (ARGB)" },
	{ OPTION_UI_TEXT_COLOR,				   "ffffffff",   OPTION_STRING, "UI text color (ARGB)" },
	{ OPTION_UI_UNAVAILABLE_COLOR,		   "ff404040",   OPTION_STRING, "UI unavailable color (ARGB)" },
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
