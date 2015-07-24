/*********************************************************************

    mewui/dirmenu.c

    Internal MEWUI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "mewui/dirmenu.h"
#include "mewui/inifile.h"
#include "mewui/datfile.h"
#include "mewui/utils.h"

static const folders_entry s_folders_entry[] =
{
	{ "ROMs",                OPTION_MEDIAPATH },
	{ "MEWUI",               OPTION_MEWUI_PATH },
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
	{ NULL }
};

/**************************************************
    MENU ADD FOLDER
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_add_change_folder::ui_menu_add_change_folder(running_machine &machine, render_container *container, int ref_path, bool _change) : ui_menu(machine, container)
{
	path_ref = ref_path - 1;
	change = _change;

	// configure the starting's path
	char *dst = NULL;
	osd_get_full_path(&dst, ".");
	current_path.assign(dst);
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
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		if (menu_event->iptkey == IPT_UI_SELECT)
		{
			int index = (FPTR)menu_event->itemref - 1;
			const ui_menu_item &pitem = item[index];

			// go up to the parent path
			if (!strcmp(pitem.text, ".."))
			{
				int first_sep = current_path.find_first_of(PATH_SEPARATOR[0]);

				int last_sep = current_path.find_last_of(PATH_SEPARATOR[0]);

				if (first_sep == last_sep)
					last_sep++;

				current_path.substr(0, last_sep);
			}
			else
			{
				// if isn't a drive, appends the directory
				if (strcmp(pitem.subtext, "[DRIVE]"))
				{
					if (current_path[current_path.length() - 1] == PATH_SEPARATOR[0])
						current_path.append(pitem.text);
					else
						current_path.append(PATH_SEPARATOR).append(pitem.text);
				}
				else
					current_path.assign(pitem.text);
			}

			// reset the char buffer also in this case
			if (m_search[0] != 0)
				m_search[0] = '\0';

			reset(UI_MENU_RESET_SELECT_FIRST);
		}
		else if (menu_event->iptkey == IPT_SPECIAL)
		{
			int buflen = strlen(m_search);
			bool update_selected = FALSE;

			// if it's a backspace and we can handle it, do so
			if ((menu_event->unichar == 8 || menu_event->unichar == 0x7f) && buflen > 0)
			{
				*(char *)utf8_previous_char(&m_search[buflen]) = 0;
				update_selected = TRUE;
			}
			// if it's any other key and we're not maxed out, update
			else if (menu_event->unichar >= ' ' && menu_event->unichar < 0x7f)
			{
				buflen += utf8_from_uchar(&m_search[buflen], ARRAY_LENGTH(m_search) - buflen, menu_event->unichar);
				m_search[buflen] = 0;
				update_selected = TRUE;
			}
			// Tab key, save current path
			else if (menu_event->unichar == 0x09)
			{
				std::string error_string;
				if (change)
				{
					machine().options().set_value(s_folders_entry[path_ref].option, current_path.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
				}
				else
				{
					std::string tmppath = std::string(machine().options().value(s_folders_entry[path_ref].option)).append(";").append(current_path.c_str());
					machine().options().set_value(s_folders_entry[path_ref].option, tmppath.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
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
					if (item[entry].ref != NULL && m_search != NULL)
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
					if (item[entry].ref != NULL && m_search != NULL)
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
			}
		}
		else if (menu_event->iptkey == IPT_UI_CANCEL)
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
	file_enumerator path(current_path.c_str());
	const osd_directory_entry *dirent;
	int folders_count = 0;

	// add the drives
	for (int i = 0; (volume_name = osd_get_volume_name(i)) != NULL; i++)
		item_append(volume_name, "[DRIVE]", 0, (void *)(FPTR)++folders_count);

	// add the directories
	while ((dirent = path.next()) != NULL)
	{
		if (dirent->type == ENTTYPE_DIR && strcmp(dirent->name, "."))
			item_append(dirent->name, "[DIR]", 0, (void *)(FPTR)++folders_count);
	}

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

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
	std::string tempbuf[2];
	const char *s_change = (change) ? "Change" : "Add";
	tempbuf[0].assign(s_change).append(" ").append(s_folders_entry[path_ref].name).append(" Folder - Search: ").append(m_search).append("_");
	tempbuf[1].assign(current_path.c_str());

	// get the size of the text
	for (int i = 0; i < 2; i++)
	{
		machine().ui().draw_text_full(container, tempbuf[i].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
		                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
		width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
		maxwidth = MAX(width, maxwidth);
	}

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	for (int i = 0; i < 2; i++)
	{
		machine().ui().draw_text_full(container, tempbuf[i].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
		                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
		y1 = y1 + machine().ui().get_line_height();
	}

	// bottom text
	tempbuf[0].assign("Press TAB to set");

	machine().ui().draw_text_full(container, tempbuf[0].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(maxwidth, width);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + bottom;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_RED_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	machine().ui().draw_text_full(container, tempbuf[0].c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

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
	save_game_options(machine());
	mewui_globals::force_reset_main = true;
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_directory::handle()
{
	// process the menu
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL && menu_event->iptkey == IPT_UI_SELECT)
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_display_actual(machine(), container, int((long long)(menu_event->itemref)))));
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_directory::populate()
{
	item_append("Roms", NULL, 0, (void *)ROM_FOLDERS);
	item_append("MEWUI", NULL, 0, (void *)MEWUI_FOLDERS);
	item_append("Samples", NULL, 0, (void *)SAMPLE_FOLDERS);
	item_append("INIs", NULL, 0, (void *)INI_FOLDERS);
	item_append("Artwork", NULL, 0, (void *)ARTWORK_FOLDERS);
	item_append("DATs (History, Mameinfo, etc...)", NULL, 0, (void *)HISTORY_FOLDERS);
	item_append("Extra INI (Category, etc...)", NULL, 0, (void *)EXTRAINI_FOLDERS);
	item_append("Icons", NULL, 0, (void *)ICON_FOLDERS);
	item_append("Cheats", NULL, 0, (void *)CHEAT_FOLDERS);
	item_append("Snapshots", NULL, 0, (void *)SNAPSHOT_FOLDERS);
	item_append("Cabinets", NULL, 0, (void *)CABINET_FOLDERS);
	item_append("Flyers", NULL, 0, (void *)FLYER_FOLDERS);
	item_append("Titles", NULL, 0, (void *)TITLE_FOLDERS);
	item_append("PCBs", NULL, 0, (void *)PCB_FOLDERS);
	item_append("Marquees", NULL, 0, (void *)MARQUEES_FOLDERS);
	item_append("Control Panels", NULL, 0, (void *)CPANEL_FOLDERS);
	item_append("Bosses", NULL, 0, (void *)BOSSES_FOLDERS);
	item_append("Versus", NULL, 0, (void *)VERSUS_FOLDERS);
	item_append("Game Over", NULL, 0, (void *)GAMEOVER_FOLDERS);
	item_append("How To", NULL, 0, (void *)HOWTO_FOLDERS);
	item_append("Select", NULL, 0, (void *)SELECT_FOLDERS);
	item_append("Artwork Preview", NULL, 0, (void *)ARTPREV_FOLDERS);
	item_append("Scores", NULL, 0, (void *)SCORES_FOLDERS);
	item_append("Logos", NULL, 0, (void *)LOGO_FOLDERS);
	item_append("Crosshairs", NULL, 0, (void *)CROSSHAIR_FOLDERS);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_directory::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;

	// get the size of the text
	machine().ui().draw_text_full(container, "Folder Setup", 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	machine().ui().draw_text_full(container, "Folder Setup", x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}

/**************************************************
    MENU DISPLAY PATH
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_display_actual::ui_menu_display_actual(running_machine &machine, render_container *container, int selectedref) : ui_menu(machine, container)
{
	ref_path = selectedref;
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
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL && menu_event->iptkey == IPT_UI_SELECT)
		switch ((FPTR)menu_event->itemref)
		{
			case REMOVE_FOLDER:
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_remove_folder(machine(), container, ref_path)));
				break;

			case ADD_FOLDER:
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_add_change_folder(machine(), container, ref_path, false)));
				break;

			case CHANGE_FOLDER:
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_add_change_folder(machine(), container, ref_path, true)));
				break;
		}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_display_actual::populate()
{
	tempbuf.assign("Current ").append(s_folders_entry[ref_path - 1].name).append(" Folders");
	searchpath.assign(machine().options().value(s_folders_entry[ref_path - 1].option));

	path_iterator path(searchpath.c_str());
	std::string curpath;
	folders.clear();

	while (path.next(curpath, NULL))
		folders.push_back(curpath);

	if (ref_path == HISTORY_FOLDERS || ref_path == EXTRAINI_FOLDERS || ref_path == MEWUI_FOLDERS)
		item_append("Change Folder", NULL, 0, (void *)CHANGE_FOLDER);
	else
		item_append("Add Folder", NULL, 0, (void *)ADD_FOLDER);

	if (folders.size() > 1)
		item_append("Remove Folder", NULL, 0, (void *)REMOVE_FOLDER);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	customtop = (folders.size() + 1) * machine().ui().get_line_height() + 6.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_display_actual::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width, maxwidth;
	maxwidth = origx2 - origx1;

	for (size_t line = 0; line < folders.size(); line++)
	{
		machine().ui().draw_text_full(container, folders[line].c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_LEFT, WRAP_TRUNCATE,
		                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
		width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
		maxwidth = MAX(maxwidth, width);
	}

	// get the size of the text
	machine().ui().draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	maxwidth = MAX(width, maxwidth);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = y1 + (machine().ui().get_line_height() + 2.0f * UI_BOX_TB_BORDER);

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	machine().ui().draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = y2 + 2.0f * UI_BOX_TB_BORDER;
	y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	for (size_t line = 0; line < folders.size(); line++)
	{
		machine().ui().draw_text_full(container, folders[line].c_str(), x1, y1, x2 - x1, JUSTIFY_LEFT, WRAP_TRUNCATE,
		                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
		y1 += machine().ui().get_line_height();
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
	path_ref = ref - 1;
	searchpath.assign(machine.options().value(s_folders_entry[path_ref].option));
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
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL && menu_event->iptkey == IPT_UI_SELECT)
	{
		int index = (FPTR)menu_event->itemref - 1;
		std::string tmppath;

		for (size_t i = 0; i < item.size() - 2; i++)
			if (i != index)
				tmppath.append(item[i].text).append(";");

		tmppath.substr(0, tmppath.length() - 1);

		std::string error_string;
		machine().options().set_value(s_folders_entry[path_ref].option, tmppath.c_str(), OPTION_PRIORITY_CMDLINE, error_string);

		ui_menu::menu_stack->parent->reset(UI_MENU_RESET_REMEMBER_REF);
		ui_menu::stack_pop(machine());
	}
}

//-------------------------------------------------
//  populate menu
//-------------------------------------------------

void ui_menu_remove_folder::populate()
{
	path_iterator path(searchpath.c_str());
	std::string curpath;
	int folders_count = 0;

	while (path.next(curpath, NULL))
		item_append(curpath.c_str(), NULL, 0, (void *)(FPTR)++folders_count);

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}

//-------------------------------------------------
//  perform our special rendering
//-------------------------------------------------

void ui_menu_remove_folder::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	float width;
	std::string tempbuf = std::string("Remove ").append(s_folders_entry[path_ref].name).append(" Folder");

	// get the size of the text
	machine().ui().draw_text_full(container, tempbuf.c_str(), 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += (2.0f * UI_BOX_LR_BORDER) + 0.01f;
	float maxwidth = MAX(width, origx2 - origx1);

	// compute our bounds
	float x1 = 0.5f - 0.5f * maxwidth;
	float x2 = x1 + maxwidth;
	float y1 = origy1 - top;
	float y2 = origy1 - UI_BOX_TB_BORDER;

	// draw a box
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_GREEN_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	machine().ui().draw_text_full(container, tempbuf.c_str(), x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_NEVER,
	                              DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

}
