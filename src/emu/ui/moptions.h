// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/moptions.h

    UI main options manager.

***************************************************************************/

#pragma once

#ifndef __UI_OPTS_H__
#define __UI_OPTS_H__

#include "options.h"

// core directory options
#define OPTION_HISTORY_PATH           "historypath"
#define OPTION_EXTRAINI_PATH          "extrainipath"
#define OPTION_CABINETS_PATH          "cabinets_directory"
#define OPTION_CPANELS_PATH           "cpanels_directory"
#define OPTION_PCBS_PATH              "pcbs_directory"
#define OPTION_FLYERS_PATH            "flyers_directory"
#define OPTION_TITLES_PATH            "titles_directory"
#define OPTION_ENDS_PATH              "ends_directory"
#define OPTION_MARQUEES_PATH          "marquees_directory"
#define OPTION_ARTPREV_PATH           "artwork_preview_directory"
#define OPTION_BOSSES_PATH            "bosses_directory"
#define OPTION_LOGOS_PATH             "logos_directory"
#define OPTION_SCORES_PATH            "scores_directory"
#define OPTION_VERSUS_PATH            "versus_directory"
#define OPTION_GAMEOVER_PATH          "gameover_directory"
#define OPTION_HOWTO_PATH             "howto_directory"
#define OPTION_SELECT_PATH            "select_directory"
#define OPTION_ICONS_PATH             "icons_directory"
#define OPTION_COVER_PATH             "covers_directory"
#define OPTION_UI_PATH                "ui_path"

// core misc options
#define OPTION_DATS_ENABLED           "dats_enabled"
#define OPTION_REMEMBER_LAST          "remember_last"
#define OPTION_ENLARGE_SNAPS          "enlarge_snaps"
#define OPTION_FORCED4X3              "forced4x3"
#define OPTION_USE_BACKGROUND         "use_background"
#define OPTION_SKIP_BIOS_MENU         "skip_biosmenu"
#define OPTION_SKIP_PARTS_MENU        "skip_partsmenu"
#define OPTION_LAST_USED_FILTER       "last_used_filter"
#define OPTION_LAST_USED_MACHINE      "last_used_machine"
#define OPTION_INFO_AUTO_AUDIT        "info_audit_enabled"

// core UI options
#define OPTION_INFOS_SIZE             "infos_text_size"
#define OPTION_FONT_ROWS              "font_rows"
#define OPTION_HIDE_PANELS            "hide_main_panel"

#define OPTION_UI_BORDER_COLOR        "ui_border_color"
#define OPTION_UI_BACKGROUND_COLOR    "ui_bg_color"
#define OPTION_UI_GFXVIEWER_BG_COLOR  "ui_gfxviewer_color"
#define OPTION_UI_UNAVAILABLE_COLOR   "ui_unavail_color"
#define OPTION_UI_TEXT_COLOR          "ui_text_color"
#define OPTION_UI_TEXT_BG_COLOR       "ui_text_bg_color"
#define OPTION_UI_SUBITEM_COLOR       "ui_subitem_color"
#define OPTION_UI_CLONE_COLOR         "ui_clone_color"
#define OPTION_UI_SELECTED_COLOR      "ui_selected_color"
#define OPTION_UI_SELECTED_BG_COLOR   "ui_selected_bg_color"
#define OPTION_UI_MOUSEOVER_COLOR     "ui_mouseover_color"
#define OPTION_UI_MOUSEOVER_BG_COLOR  "ui_mouseover_bg_color"
#define OPTION_UI_MOUSEDOWN_COLOR     "ui_mousedown_color"
#define OPTION_UI_MOUSEDOWN_BG_COLOR  "ui_mousedown_bg_color"
#define OPTION_UI_DIPSW_COLOR         "ui_dipsw_color"
#define OPTION_UI_SLIDER_COLOR        "ui_slider_color"

class ui_options : public core_options
{
public:
	// construction/destruction
	ui_options();

	// Search path options
	const char *history_path() const { return value(OPTION_HISTORY_PATH); }
	const char *extraini_path() const { return value(OPTION_EXTRAINI_PATH); }
	const char *cabinets_directory() const { return value(OPTION_CABINETS_PATH); }
	const char *cpanels_directory() const { return value(OPTION_CPANELS_PATH); }
	const char *pcbs_directory() const { return value(OPTION_PCBS_PATH); }
	const char *flyers_directory() const { return value(OPTION_FLYERS_PATH); }
	const char *titles_directory() const { return value(OPTION_TITLES_PATH); }
	const char *ends_directory() const { return value(OPTION_ENDS_PATH); }
	const char *marquees_directory() const { return value(OPTION_MARQUEES_PATH); }
	const char *artprev_directory() const { return value(OPTION_ARTPREV_PATH); }
	const char *bosses_directory() const { return value(OPTION_BOSSES_PATH); }
	const char *logos_directory() const { return value(OPTION_LOGOS_PATH); }
	const char *scores_directory() const { return value(OPTION_SCORES_PATH); }
	const char *versus_directory() const { return value(OPTION_VERSUS_PATH); }
	const char *gameover_directory() const { return value(OPTION_GAMEOVER_PATH); }
	const char *howto_directory() const { return value(OPTION_HOWTO_PATH); }
	const char *select_directory() const { return value(OPTION_SELECT_PATH); }
	const char *icons_directory() const { return value(OPTION_ICONS_PATH); }
	const char *covers_directory() const { return value(OPTION_COVER_PATH); }
	const char *ui_path() const { return value(OPTION_UI_PATH); }

	// Misc options
	bool enabled_dats() const { return bool_value(OPTION_DATS_ENABLED); }
	bool remember_last() const { return bool_value(OPTION_REMEMBER_LAST); }
	bool enlarge_snaps() const { return bool_value(OPTION_ENLARGE_SNAPS); }
	bool forced_4x3_snapshot() const { return bool_value(OPTION_FORCED4X3); }
	bool use_background_image() const { return bool_value(OPTION_USE_BACKGROUND); }
	bool skip_bios_menu() const { return bool_value(OPTION_SKIP_BIOS_MENU); }
	bool skip_parts_menu() const { return bool_value(OPTION_SKIP_PARTS_MENU); }
	const char *last_used_machine() const { return value(OPTION_LAST_USED_MACHINE); }
	const char *last_used_filter() const { return value(OPTION_LAST_USED_FILTER); }
	bool info_audit() const { return bool_value(OPTION_INFO_AUTO_AUDIT); }

	// UI options
	float infos_size() const { return float_value(OPTION_INFOS_SIZE); }
	int font_rows() const { return int_value(OPTION_FONT_ROWS); }
	int hide_panels() const { return int_value(OPTION_HIDE_PANELS); }

	const char *ui_border_color() const { return value(OPTION_UI_BORDER_COLOR); }
	const char *ui_bg_color() const { return value(OPTION_UI_BACKGROUND_COLOR); }
	const char *ui_gfx_bg_color() const { return value(OPTION_UI_GFXVIEWER_BG_COLOR); }
	const char *ui_unavail_color() const { return value(OPTION_UI_UNAVAILABLE_COLOR); }
	const char *ui_text_color() const { return value(OPTION_UI_TEXT_COLOR); }
	const char *ui_text_bg_color() const { return value(OPTION_UI_TEXT_BG_COLOR); }
	const char *ui_subitem_color() const { return value(OPTION_UI_SUBITEM_COLOR); }
	const char *ui_clone_color() const { return value(OPTION_UI_CLONE_COLOR); }
	const char *ui_selected_color() const { return value(OPTION_UI_SELECTED_COLOR); }
	const char *ui_selected_bg_color() const { return value(OPTION_UI_SELECTED_BG_COLOR); }
	const char *ui_mouseover_color() const { return value(OPTION_UI_MOUSEOVER_COLOR); }
	const char *ui_mouseover_bg_color() const { return value(OPTION_UI_MOUSEOVER_BG_COLOR); }
	const char *ui_mousedown_color() const { return value(OPTION_UI_MOUSEDOWN_COLOR); }
	const char *ui_mousedown_bg_color() const { return value(OPTION_UI_MOUSEDOWN_BG_COLOR); }
	const char *ui_dipsw_color() const { return value(OPTION_UI_DIPSW_COLOR); }
	const char *ui_slider_color() const { return value(OPTION_UI_SLIDER_COLOR); }
private:
	static const options_entry s_option_entries[];
};

#endif /* __UI_OPTS_H__ */
