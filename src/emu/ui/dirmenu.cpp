// license:BSD-3-Clause
// copyright-holders:Dankan1890
/*********************************************************************

    ui/dirmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/dirmenu.h"
#include "ui/datfile.h"
#include "ui/utils.h"
#include "ui/optsmenu.h"

struct folders_entry
{
	const char *name;
	const char *option;
};

static const folders_entry s_folders_entry[] =
{
	{ "ROMs",                OPTION_MEDIAPATH },
	{ "UI",                  OPTION_UI_PATH },
	{ "Samples",             OPTION_SAMPLEPATH },
	{ "DATs",                OPTION_HISTORY_PATH },
	{ "INIs",                OPTION_INIPATH },
	{ "Extra INIs",          OPTION_EXTRAINI_PATH },
	{ "Icons",               OPTION_ICONS_PATH },
	{ "Cheats",              OPTION_CHEATPATH },
	{ "Snapshots",           OPTION_SNAPSHOT_DIRECTORY },
	{ "Cabinets",            OPTION_CABINETS_PATH },
	{ "Flyers",              OPTION_FLYERS_PATH },
	{ "Titles",              OPTION_TITLES_PATH },
	{ "Ends",                OPTION_ENDS_PATH },
	{ "PCBs",                OPTION_PCBS_PATH },
	{ "Marquees",            OPTION_MARQUEES_PATH },
	{ "Controls Panels",     OPTION_CPANELS_PATH },
	{ "Crosshairs",          OPTION_CROSSHAIRPATH },
	{ "Artworks",            OPTION_ARTPATH },
	{ "Bosses",              OPTION_BOSSES_PATH },
	{ "Artworks Preview",    OPTION_ARTPREV_PATH },
	{ "Select",              OPTION_SELECT_PATH },
	{ "GameOver",            OPTION_GAMEOVER_PATH },
	{ "HowTo",               OPTION_HOWTO_PATH },
	{ "Logos",               OPTION_LOGOS_PATH },
	{ "Scores",              OPTION_SCORES_PATH },
	{ "Versus",              OPTION_VERSUS_PATH },
	{ nullptr }
};

/**************************************************
    MENU ADD FOLDER
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_add_change_folder::ui_menu_add_change_folder(running_machine &machine, render_container *container, int ref, bool _change) : ui_menu(machine, container)
{
	m_ref = ref - 1;
	m_change = _change;
	m_search[0] = '\0';

	// configure the starting path
	char *dst = nullptr;
	osd_get_full_path(&dst, ".");
	m_current_path = dst;
	osd_free(dst);
}

ui_menu_add_change_folder::~ui_menu_add_change_folder()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_add_change_folder::handle()
{
	// process the menu
	const ui_menu_event *m_event = process(0);

	if (m_event != nullptr && m_event->itemref != nullptr)
	{
		if (m_event->iptkey == IPT_UI_SELECT)
		{
			int index = (FPTR)m_event->itemref - 1;
			const ui_menu_item &pitem = item[index];

			// go up to the parent path
			if (!strcmp(pitem.text, ".."))
			{
				size_t first_sep = m_current_path.find_first_of(PATH_SEPARATOR[0]);
				size_t last_sep = m_current_path.find_last_of(PATH_SEPARATOR[0]);
				if (first_sep != last_sep)
					m_current_path.erase(++last_sep);
			}
			else
			{
				// if isn't a drive, appends the directory
				if (strcmp(pitem.subtext, "[DRIVE]") != 0)
				{
					if (m_current_path[m_current_path.length() - 1] == PATH_SEPARATOR[0])
						m_current_path.append(pitem.text);
					else
						m_current_path.append(PATH_SEPARATOR).append(pitem.text);
				}
				else
					m_current_path = pitem.text;
			}

			// reset the char buffer also in this case
			if (m_search[0] != 0)
				m_search[0] = '\0';
			reset(UI_MENU_RESET_SELECT_FIRST);
		}
		else if (m_event->iptkey == IPT_SPECIAL)
		{
			int buflen = strlen(m_search);
			bool update_selected = FALSE;

			// if it's a backspace and we can handle it, do so
			if ((m_event->unichar == 8 || m_event->unichar == 0x7f) && buflen > 0)
			{
				*(char *)utf8_previous_char(&m_search[buflen]) = 0;
				update_selected = TRUE;
			}
			// if it's any other key and we're not maxed out, update
			else if (m_event->unichar >= ' ' && m_event->unichar < 0x7f)
			{
				buflen += utf8_from_uchar(&m_search[buflen], ARRAY_LENGTH(m_search) - buflen, m_event->unichar);
				m_search[buflen] = 0;
				update_selected = TRUE;
			}
			// Tab key, save current path
			else if (m_event->unichar == 0x09)
			{
				std::string error_string;
				if (m_change)
				{
					if (machine().ui().options().exists(s_folders_entry[m_ref].option))
					{
						machine().ui().options().set_value(s_folders_entry[m_ref].option, m_current_path.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
					}
					else {
						if (strcmp(machine().options().value(s_folders_entry[m_ref].option), m_current_path.c_str()) != 0)
						{
							machine().options().set_value(s_folders_entry[m_ref].option, m_current_path.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
							machine().options().mark_changed(s_folders_entry[m_ref].option);
						}
					}
					machine().datfile().reset_run();
				}
				else
				{
					std::string tmppath;
					if (machine().ui().options().exists(s_folders_entry[m_ref].option)) {
						tmppath.assign(machine().ui().options().value(s_folders_entry[m_ref].option)).append(";").append(m_current_path.c_str());
					}
					else {
						tmppath.assign(machine().options().value(s_folders_entry[m_ref].option)).append(";").append(m_current_path.c_str());
					}
					
					if (machine().ui().options().exists(s_folders_entry[m_ref].option))
					{
						machine().ui().options().set_value(s_folders_entry[m_ref].option, tmppath.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
					}
					else {
						if (strcmp(machine().options().value(s_folders_entry[m_ref].option), tmppath.c_str()) != 0)
						{
							machine().options().set_value(s_folders_entry[m_ref].option, tmppath.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
							machine().options().mark_changed(s_folders_entry[m_ref].option);
						}
					}
				}

				ui_menu::menu_stack->parent->reset(UI_MENU_RESET_SELECT_FIRST);
				ui_menu::stack_pop(machine());
			}

			// check for entries which matches our search buffer
			if (update_selected)
			{
				const int cur_selected = selected;
				int entry, bestmatch = 0;

				// from current item to the end
				for (entry = cur_selected; entry < item.size(); entry++)
					if (item[entry].ref != nullptr && m_search[0] != 0)
					{
						int match = 0;
						for (int i = 0; i < ARRAY_LENGTH(m_search); i++)
						{
							if (core_strnicmp(item[entry].text, m_search, i) == 0)
								match = i;
						}

						if (match > bestmatch)
						{
							bestmatch = match;
							selected = entry;
						}
					}

				// and from the first item to current one
				for (entry = 0; entry < cur_selected; entry++)
				{
					if (item[entry].ref != nullptr && m_search[0] != 0)
					{
						int match = 0;
						for (int i = 0; i < ARRAY_LENGTH(m_search); i++)
						{
							if (core_strnicmp(item[entry].text, m_search, i) == 0)
								match = i;
						}

						if (match > bestmatch)
						{
							bestmatch = match;
							selected = entry;
						}
					}
				}
				top_line = selected - (visible_lines / 2);
			}
		}
		else if (m_event->iptkey == IPT_UI_CANCEL)
		{
			// reset the char buffer also in this case
			if (m_search[0] != 0)
				m_search[0] = '\0';
		}
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_add_change_folder::populate()
{
	// open a path
	const char *volume_name;
	file_enumerator path(m_current_path.c_str());
	const osd_directory_entry *dirent;
	int folders_count = 0;

	// add the drives
	for (int i = 0; (volume_name = osd_get_volume_name(i)) != nullptr; i++)
		item_append(volume_name, "[DRIVE]", 0, (void *)(FPTR)++folders_count);

	// add the directories
	while ((dirent = path.next()) != nullptr)
	{
		if (dirent->type == ENTTYPE_DIR && strcmp(dirent->name, ".") != 0)
			item_append(dirent->name, "[DIR]", 0, (void *)(FPTR)++folders_count);
	}

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	// configure the custom rendering
	customtop = 2.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	custombottom = 1.0f * machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_add_change_folder::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;
	ui_manager &mui = machine().ui();
	std::string tempbuf[2];
	tempbuf[0] = (m_change) ? _("Change)") : _("Add");
	tempbuf[0].append(" ").append(s_folders_entry[m_ref].name).append(_(" Folder - Search: ")).append(m_search).append("_");
	tempbuf[1] = m_current_path;

	// get the size of the text
	for (auto & elem: tempbuf)
	{
		mui.draw_text_full(container, elem.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
		                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
		width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
		maxwidth = MAX(width, maxwidth);
	}

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
	for (auto & elem : tempbuf)
	{
		mui.draw_text_full(container, elem.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
			DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
		y1 = y1 + mui.get_line_height();
	}

	// bottom text
	tempbuf[0] = _("Press TAB to set");

	mui.draw_text_full(container, tempbuf[0].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
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
	mui.draw_text_full(container, tempbuf[0].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

}

/**************************************************
    MENU DIRECTORY
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_directory::ui_menu_directory(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

ui_menu_directory::~ui_menu_directory()
{
	save_ui_options(machine());
	ui_globals::reset = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_directory::handle()
{
	// process the menu
	const ui_menu_event *m_event = process(0);

	if (m_event != nullptr && m_event->itemref != nullptr && m_event->iptkey == IPT_UI_SELECT)
	{
		int ref = (FPTR)m_event->itemref;
		bool change = (ref == HISTORY_FOLDERS || ref == EXTRAINI_FOLDERS || ref == UI_FOLDERS);
		ui_menu::stack_push(global_alloc_clear<ui_menu_display_actual>(machine(), container, ref, change));
	}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_directory::populate()
{
	item_append("Roms", nullptr, 0, (void *)(FPTR)ROM_FOLDERS);
	item_append("UI", nullptr, 0, (void *)(FPTR)UI_FOLDERS);
	item_append("Samples", nullptr, 0, (void *)(FPTR)SAMPLE_FOLDERS);
	item_append("INIs", nullptr, 0, (void *)(FPTR)INI_FOLDERS);
	item_append("Artwork", nullptr, 0, (void *)(FPTR)ARTWORK_FOLDERS);
	item_append("DATs (History, Mameinfo, etc...)", nullptr, 0, (void *)(FPTR)HISTORY_FOLDERS);
	item_append("Extra INI (Category, etc...)", nullptr, 0, (void *)(FPTR)EXTRAINI_FOLDERS);
	item_append("Icons", nullptr, 0, (void *)(FPTR)ICON_FOLDERS);
	item_append("Cheats", nullptr, 0, (void *)(FPTR)CHEAT_FOLDERS);
	item_append("Snapshots", nullptr, 0, (void *)(FPTR)SNAPSHOT_FOLDERS);
	item_append("Cabinets", nullptr, 0, (void *)(FPTR)CABINET_FOLDERS);
	item_append("Flyers", nullptr, 0, (void *)(FPTR)FLYER_FOLDERS);
	item_append("Titles", nullptr, 0, (void *)(FPTR)TITLE_FOLDERS);
	item_append("Ends", nullptr, 0, (void *)(FPTR)ENDS_FOLDERS);
	item_append("PCBs", nullptr, 0, (void *)(FPTR)PCB_FOLDERS);
	item_append("Marquees", nullptr, 0, (void *)(FPTR)MARQUEES_FOLDERS);
	item_append("Control Panels", nullptr, 0, (void *)(FPTR)CPANEL_FOLDERS);
	item_append("Bosses", nullptr, 0, (void *)(FPTR)BOSSES_FOLDERS);
	item_append("Versus", nullptr, 0, (void *)(FPTR)VERSUS_FOLDERS);
	item_append("Game Over", nullptr, 0, (void *)(FPTR)GAMEOVER_FOLDERS);
	item_append("How To", nullptr, 0, (void *)(FPTR)HOWTO_FOLDERS);
	item_append("Select", nullptr, 0, (void *)(FPTR)SELECT_FOLDERS);
	item_append("Artwork Preview", nullptr, 0, (void *)(FPTR)ARTPREV_FOLDERS);
	item_append("Scores", nullptr, 0, (void *)(FPTR)SCORES_FOLDERS);
	item_append("Logos", nullptr, 0, (void *)(FPTR)LOGO_FOLDERS);
	item_append("Crosshairs", nullptr, 0, (void *)(FPTR)CROSSHAIR_FOLDERS);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_directory::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();

	// get the size of the text
	mui.draw_text_full(container, _("Folders Setup"), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

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
	mui.draw_text_full(container, _("Folders Setup"), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}

/**************************************************
    MENU DISPLAY PATH
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_display_actual::ui_menu_display_actual(running_machine &machine, render_container *container, int ref, bool _change) : ui_menu(machine, container)
{
	m_ref = ref;
	m_change = _change;
}

ui_menu_display_actual::~ui_menu_display_actual()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_display_actual::handle()
{
	// process the menu
	const ui_menu_event *m_event = process(0);
	if (m_event != nullptr && m_event->itemref != nullptr && m_event->iptkey == IPT_UI_SELECT)
		switch ((FPTR)m_event->itemref)
		{
			case REMOVE_FOLDER:
				ui_menu::stack_push(global_alloc_clear<ui_menu_remove_folder>(machine(), container, m_ref));
				break;

			case ADD_FOLDER:
			case CHANGE_FOLDER:
				ui_menu::stack_push(global_alloc_clear<ui_menu_add_change_folder>(machine(), container, m_ref, m_change));
				break;
		}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_display_actual::populate()
{
	m_tempbuf.assign(_("Current ")).append(s_folders_entry[m_ref - 1].name).append(_(" Folders"));
	if (machine().ui().options().exists(s_folders_entry[m_ref - 1].option)) {
		m_searchpath.assign(machine().ui().options().value(s_folders_entry[m_ref - 1].option));
	}
	else {
		m_searchpath.assign(machine().options().value(s_folders_entry[m_ref - 1].option));
	}
	path_iterator path(m_searchpath.c_str());
	std::string curpath;
	m_folders.clear();
	while (path.next(curpath, nullptr))
		m_folders.push_back(curpath);

	if (m_change)
		item_append(_("Change Folder"), nullptr, 0, (void *)CHANGE_FOLDER);
	else
		item_append(_("Add Folder"), nullptr, 0, (void *)ADD_FOLDER);

	if (m_folders.size() > 1)
		item_append(_("Remove Folder"), nullptr, 0, (void *)REMOVE_FOLDER);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);
	customtop = (m_folders.size() + 1) * machine().ui().get_line_height() + 6.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_display_actual::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth = origx2 - origx1;
	ui_manager &mui = machine().ui();
	float lineh = mui.get_line_height();

	for (auto & elem : m_folders)
	{
		mui.draw_text_full(container, elem.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
		width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
		maxwidth = MAX(maxwidth, width);
	}

	// get the size of the text
	mui.draw_text_full(container, m_tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	maxwidth = MAX(width, maxwidth);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = y1 + lineh + 2.0f * UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	mui.draw_text_full(container, m_tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE, 
		DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = y2 + 2.0f * UI_BOX_TB_BORDER;
	y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	for (auto & elem : m_folders)
	{
		mui.draw_text_full(container, elem.c_str(), x1, y1, x2 - x1, JUSTIFY_LEFT, WRAP_TRUNCATE,
			DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
		y1 += lineh;
	}

}

/**************************************************
    MENU REMOVE FOLDER
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_remove_folder::ui_menu_remove_folder(running_machine &machine, render_container *container, int ref) : ui_menu(machine, container)
{
	m_ref = ref - 1;
	if (machine.ui().options().exists(s_folders_entry[m_ref].option)) {
		m_searchpath.assign(machine.ui().options().value(s_folders_entry[m_ref].option));
	}
	else {
		m_searchpath.assign(machine.options().value(s_folders_entry[m_ref].option));
	}

}

ui_menu_remove_folder::~ui_menu_remove_folder()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_remove_folder::handle()
{
	// process the menu
	const ui_menu_event *m_event = process(0);
	if (m_event != nullptr && m_event->itemref != nullptr && m_event->iptkey == IPT_UI_SELECT)
	{
		int index = (FPTR)m_event->itemref - 1;
		std::string tmppath;
		for (size_t i = 0; i < item.size() - 2; i++)
			if (i != index)
				tmppath.append(item[i].text).append(";");

		tmppath.substr(0, tmppath.size() - 1);
		std::string error_string;
		if (machine().ui().options().exists(s_folders_entry[m_ref].option))
		{
			machine().ui().options().set_value(s_folders_entry[m_ref].option, tmppath.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
		}
		else {
			if (strcmp(machine().options().value(s_folders_entry[m_ref].option),tmppath.c_str())!=0)
			{
				machine().options().set_value(s_folders_entry[m_ref].option, tmppath.c_str(), OPTION_PRIORITY_CMDLINE, error_string);				
				machine().options().mark_changed(s_folders_entry[m_ref].option);
			}
		}

		ui_menu::menu_stack->parent->reset(UI_MENU_RESET_REMEMBER_REF);
		ui_menu::stack_pop(machine());
	}
}

//-------------------------------------------------
//  populate menu
//-------------------------------------------------

void ui_menu_remove_folder::populate()
{
	path_iterator path(m_searchpath.c_str());
	std::string curpath;
	int folders_count = 0;

	while (path.next(curpath, nullptr))
		item_append(curpath.c_str(), nullptr, 0, (void *)(FPTR)++folders_count);

	item_append(MENU_SEPARATOR_ITEM, nullptr, 0, nullptr);

	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_remove_folder::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	ui_manager &mui = machine().ui();
	std::string tempbuf = std::string(_("Remove ")).append(s_folders_entry[m_ref].name).append(_(" Folder"));

	// get the size of the text
	mui.draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, nullptr);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

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
	mui.draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER, DRAW_NORMAL, 
		UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);
}
