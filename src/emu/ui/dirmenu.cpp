// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
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

static int ADDING = 1;
static int CHANGE = 2;

struct folders_entry
{
	const char *name;
	const char *option;
	const int   action;
};

static const folders_entry s_folders[] =
{
	{ __("ROMs"),                OPTION_MEDIAPATH,          ADDING },
	{ __("UI"),                  OPTION_UI_PATH,            CHANGE },
	{ __("Language"),            OPTION_LANGUAGEPATH,       CHANGE },
	{ __("Samples"),             OPTION_SAMPLEPATH,         ADDING },
	{ __("DATs"),                OPTION_HISTORY_PATH,       ADDING },
	{ __("INIs"),                OPTION_INIPATH,            ADDING },
	{ __("Extra INIs"),          OPTION_EXTRAINI_PATH,      CHANGE },
	{ __("Icons"),               OPTION_ICONS_PATH,         ADDING },
	{ __("Cheats"),              OPTION_CHEATPATH,          ADDING },
	{ __("Snapshots"),           OPTION_SNAPSHOT_DIRECTORY, ADDING },
	{ __("Cabinets"),            OPTION_CABINETS_PATH,      ADDING },
	{ __("Flyers"),              OPTION_FLYERS_PATH,        ADDING },
	{ __("Titles"),              OPTION_TITLES_PATH,        ADDING },
	{ __("Ends"),                OPTION_ENDS_PATH,          ADDING },
	{ __("PCBs"),                OPTION_PCBS_PATH,          ADDING },
	{ __("Marquees"),            OPTION_MARQUEES_PATH,      ADDING },
	{ __("Controls Panels"),     OPTION_CPANELS_PATH,       ADDING },
	{ __("Crosshairs"),          OPTION_CROSSHAIRPATH,      ADDING },
	{ __("Artworks"),            OPTION_ARTPATH,            ADDING },
	{ __("Bosses"),              OPTION_BOSSES_PATH,        ADDING },
	{ __("Artworks Preview"),    OPTION_ARTPREV_PATH,       ADDING },
	{ __("Select"),              OPTION_SELECT_PATH,        ADDING },
	{ __("GameOver"),            OPTION_GAMEOVER_PATH,      ADDING },
	{ __("HowTo"),               OPTION_HOWTO_PATH,         ADDING },
	{ __("Logos"),               OPTION_LOGOS_PATH,         ADDING },
	{ __("Scores"),              OPTION_SCORES_PATH,        ADDING },
	{ __("Versus"),              OPTION_VERSUS_PATH,        ADDING },
	{ __("Covers"),              OPTION_COVER_PATH,         ADDING }
};


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
	machine().datfile().reset_run();
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_directory::handle()
{
	// process the menu
	const ui_menu_event *m_event = process(0);

	if (m_event != nullptr && m_event->itemref != nullptr && m_event->iptkey == IPT_UI_SELECT)
		ui_menu::stack_push(global_alloc_clear<ui_menu_display_actual>(machine(), container, selected));
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_directory::populate()
{
	for (auto & elem : s_folders)
		item_append(_(elem.name), nullptr, 0, (void *)(FPTR)elem.action);

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

ui_menu_display_actual::ui_menu_display_actual(running_machine &machine, render_container *container, int ref)
	: ui_menu(machine, container), m_ref(ref)
{
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
			case REMOVE:
				ui_menu::stack_push(global_alloc_clear<ui_menu_remove_folder>(machine(), container, m_ref));
				break;

			case ADD_CHANGE:
				ui_menu::stack_push(global_alloc_clear<ui_menu_add_change_folder>(machine(), container, m_ref));
				break;
		}
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_display_actual::populate()
{
	m_tempbuf = string_format(_("Current %1$s Folders"), _(s_folders[m_ref].name));
	if (machine().ui().options().exists(s_folders[m_ref].option))
		m_searchpath.assign(machine().ui().options().value(s_folders[m_ref].option));
	else
		m_searchpath.assign(machine().options().value(s_folders[m_ref].option));

	path_iterator path(m_searchpath.c_str());
	std::string curpath;
	m_folders.clear();
	while (path.next(curpath, nullptr))
		m_folders.push_back(curpath);

	item_append((s_folders[m_ref].action == CHANGE) ? _("Change Folder") : _("Add Folder"), nullptr, 0, (void *)ADD_CHANGE);

	if (m_folders.size() > 1)
		item_append(_("Remove Folder"), nullptr, 0, (void *)REMOVE);

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
MENU ADD FOLDER
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_add_change_folder::ui_menu_add_change_folder(running_machine &machine, render_container *container, int ref) : ui_menu(machine, container)
{
	m_ref = ref;
	m_change = (s_folders[ref].action == CHANGE);
	m_search[0] = '\0';

	// configure the starting path
	osd_get_full_path(m_current_path, ".");

	std::string searchpath;
	if (machine.ui().options().exists(s_folders[m_ref].option))
		searchpath = machine.ui().options().value(s_folders[m_ref].option);
	else
		searchpath = machine.options().value(s_folders[m_ref].option);

	path_iterator path(searchpath.c_str());
	std::string curpath;
	while (path.next(curpath, nullptr))
		m_folders.push_back(curpath);
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
					if (machine().ui().options().exists(s_folders[m_ref].option))
						machine().ui().options().set_value(s_folders[m_ref].option, m_current_path.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
					else if (strcmp(machine().options().value(s_folders[m_ref].option), m_current_path.c_str()) != 0)
					{
						machine().options().set_value(s_folders[m_ref].option, m_current_path.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
						machine().options().mark_changed(s_folders[m_ref].option);
					}
					machine().datfile().reset_run();
				}
				else
				{
					m_folders.push_back(m_current_path);
					std::string tmppath;
					for (int x = 0; x < m_folders.size(); ++x)
					{
						tmppath.append(m_folders[x]);
						if (x != m_folders.size() - 1)
							tmppath.append(";");
					}

					if (machine().ui().options().exists(s_folders[m_ref].option))
						machine().ui().options().set_value(s_folders[m_ref].option, tmppath.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
					else if (strcmp(machine().options().value(s_folders[m_ref].option), tmppath.c_str()) != 0)
					{
						machine().options().set_value(s_folders[m_ref].option, tmppath.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
						machine().options().mark_changed(s_folders[m_ref].option);
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
	const char *volume_name = nullptr;
	file_enumerator path(m_current_path.c_str());
	const osd_directory_entry *dirent;
	int folders_count = 0;

	// add the drives
	for (int i = 0; (volume_name = osd_get_volume_name(i)) != nullptr; ++i)
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
	tempbuf[0] = string_format(
			(m_change)
				? _("Change %1$s Folder - Search: %2$s_")
				: _("Add %1$s Folder - Search: %2$s_"),
			_(s_folders[m_ref].name),
			m_search);
	tempbuf[1] = m_current_path;

	// get the size of the text
	for (auto & elem : tempbuf)
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
    MENU REMOVE FOLDER
**************************************************/
//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_remove_folder::ui_menu_remove_folder(running_machine &machine, render_container *container, int ref) : ui_menu(machine, container)
{
	m_ref = ref;
	if (machine.ui().options().exists(s_folders[m_ref].option))
		m_searchpath.assign(machine.ui().options().value(s_folders[m_ref].option));
	else
		m_searchpath.assign(machine.options().value(s_folders[m_ref].option));

	path_iterator path(m_searchpath.c_str());
	std::string curpath;
	while (path.next(curpath, nullptr))
		m_folders.push_back(curpath);
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
		std::string tmppath, error_string;
		m_folders.erase(m_folders.begin() + selected);
		for (int x = 0; x < m_folders.size(); ++x)
		{
			tmppath.append(m_folders[x]);
			if (x < m_folders.size() - 1)
				tmppath.append(";");
		}

		if (machine().ui().options().exists(s_folders[m_ref].option))
			machine().ui().options().set_value(s_folders[m_ref].option, tmppath.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
		else if (strcmp(machine().options().value(s_folders[m_ref].option),tmppath.c_str())!=0)
		{
			machine().options().set_value(s_folders[m_ref].option, tmppath.c_str(), OPTION_PRIORITY_CMDLINE, error_string);
			machine().options().mark_changed(s_folders[m_ref].option);
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
	int folders_count = 0;
	for (auto & elem : m_folders)
		item_append(elem.c_str(), nullptr, 0, (void *)(FPTR)++folders_count);

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
	std::string tempbuf = string_format(_("Remove %1$s Folder"), _(s_folders[m_ref].name));

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
