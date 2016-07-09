// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/filesel.cpp

    MAME's clunky built-in file manager

    TODO
        - Restrict empty slot if image required

***************************************************************************/

#include "emu.h"

#include "ui/filesel.h"
#include "ui/ui.h"

#include "imagedev/floppy.h"

#include "zippath.h"

#include <cstring>


namespace ui {
/***************************************************************************
    CONSTANTS
***************************************************************************/

// conditional compilation to enable chosing of image formats - this is not
// yet fully implemented
#define ENABLE_FORMATS          0

// time (in seconds) to display errors
#define ERROR_MESSAGE_TIME      5


/***************************************************************************
    FILE SELECTOR MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_file_selector::menu_file_selector(mame_ui_manager &mui, render_container *container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool has_empty, bool has_softlist, bool has_create, menu_file_selector::result &result)
	: menu(mui, container)
	, m_current_directory(current_directory)
	, m_current_file(current_file)
	, m_result(result)
{
	m_image = image;
	m_has_empty = has_empty;
	m_has_softlist = has_softlist;
	m_has_create = has_create;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_file_selector::~menu_file_selector()
{
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_file_selector::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// lay out extra text
	auto layout = ui().create_layout(container);
	layout.add_text(m_current_directory.c_str());

	// position this extra text
	float x1, y1, x2, y2;
	extra_text_position(origx1, origx2, origy1, top, layout, -1, x1, y1, x2, y2);

	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	size_t hit_start = 0, hit_span = 0;
	if (mouse_hit
		&& layout.hit_test(mouse_x - x1, mouse_y - y1, hit_start, hit_span)
		&& m_current_directory.substr(hit_start, hit_span) != PATH_SEPARATOR)
	{
		// we're hovering over a directory!  highlight it
		auto target_dir_start = m_current_directory.rfind(PATH_SEPARATOR, hit_start) + 1;
		auto target_dir_end = m_current_directory.find(PATH_SEPARATOR, hit_start + hit_span);
		m_hover_directory = m_current_directory.substr(0, target_dir_end + strlen(PATH_SEPARATOR));

		// highlight the text in question
		rgb_t fgcolor = UI_MOUSEOVER_COLOR;
		rgb_t bgcolor = UI_MOUSEOVER_BG_COLOR;
		layout.restyle(target_dir_start, target_dir_end - target_dir_start, &fgcolor, &bgcolor);
	}
	else
	{
		// we are not hovering over anything
		m_hover_directory.clear();
	}

	// draw the text within it
	layout.emit(container, x1, y1);
}


//-------------------------------------------------
//  custom_mouse_down - perform our special mouse down
//-------------------------------------------------

bool menu_file_selector::custom_mouse_down()
{
	if (m_hover_directory.length() > 0)
	{
		m_current_directory = m_hover_directory;
		reset(reset_options::SELECT_FIRST);
		return true;
	}

	return false;
}


//-------------------------------------------------
//  compare_file_selector_entries - sorting proc
//  for file selector entries
//-------------------------------------------------

int menu_file_selector::compare_entries(const file_selector_entry *e1, const file_selector_entry *e2)
{
	int result;
	const char *e1_basename = e1->basename.c_str();
	const char *e2_basename = e2->basename.c_str();

	if (e1->type < e2->type)
	{
		result = -1;
	}
	else if (e1->type > e2->type)
	{
		result = 1;
	}
	else
	{
		result = core_stricmp(e1_basename, e2_basename);
		if (result == 0)
		{
			result = strcmp(e1_basename, e2_basename);
			if (result == 0)
			{
				if (e1 < e2)
					result = -1;
				else if (e1 > e2)
					result = 1;
			}
		}
	}

	return result;
}


//-------------------------------------------------
//  append_entry - appends a new
//  file selector entry to an entry list
//-------------------------------------------------

menu_file_selector::file_selector_entry &menu_file_selector::append_entry(
	file_selector_entry_type entry_type, const std::string &entry_basename, const std::string &entry_fullpath)
{
	return append_entry(entry_type, std::string(entry_basename), std::string(entry_fullpath));
}


//-------------------------------------------------
//  append_entry - appends a new
//  file selector entry to an entry list
//-------------------------------------------------

menu_file_selector::file_selector_entry &menu_file_selector::append_entry(
	file_selector_entry_type entry_type, std::string &&entry_basename, std::string &&entry_fullpath)
{
	// allocate a new entry
	file_selector_entry entry;
	entry.type = entry_type;
	entry.basename = std::move(entry_basename);
	entry.fullpath = std::move(entry_fullpath);

	// find the end of the list
	m_entrylist.emplace_back(std::move(entry));
	return m_entrylist[m_entrylist.size() - 1];
}


//-------------------------------------------------
//  append_entry_menu_item - appends
//  a menu item for a file selector entry
//-------------------------------------------------

menu_file_selector::file_selector_entry *menu_file_selector::append_dirent_entry(const osd::directory::entry *dirent)
{
	std::string buffer;
	file_selector_entry_type entry_type;
	file_selector_entry *entry;

	switch(dirent->type)
	{
		case osd::directory::entry::entry_type::FILE:
			entry_type = SELECTOR_ENTRY_TYPE_FILE;
			break;

		case osd::directory::entry::entry_type::DIR:
			entry_type = SELECTOR_ENTRY_TYPE_DIRECTORY;
			break;

		default:
			// exceptional case; do not add a menu item
			return nullptr;
	}

	// determine the full path
	buffer = util::zippath_combine(m_current_directory.c_str(), dirent->name);

	// create the file selector entry
	entry = &append_entry(
		entry_type,
		dirent->name,
		std::move(buffer));

	return entry;
}


//-------------------------------------------------
//  append_entry_menu_item - appends
//  a menu item for a file selector entry
//-------------------------------------------------

void menu_file_selector::append_entry_menu_item(const file_selector_entry *entry)
{
	std::string text;
	std::string subtext;

	switch(entry->type)
	{
		case SELECTOR_ENTRY_TYPE_EMPTY:
			text = _("[empty slot]");
			break;

		case SELECTOR_ENTRY_TYPE_CREATE:
			text = _("[create]");
			break;

		case SELECTOR_ENTRY_TYPE_SOFTWARE_LIST:
			text = _("[software list]");
			break;

		case SELECTOR_ENTRY_TYPE_DRIVE:
			text = entry->basename;
			subtext = "[DRIVE]";
			break;

		case SELECTOR_ENTRY_TYPE_DIRECTORY:
			text = entry->basename;
			subtext = "[DIR]";
			break;

		case SELECTOR_ENTRY_TYPE_FILE:
			text = entry->basename;
			subtext = "[FILE]";
			break;
	}
	item_append(std::move(text), std::move(subtext), 0, (void *) entry);
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_file_selector::populate()
{
	util::zippath_directory *directory = nullptr;
	osd_file::error err;
	const osd::directory::entry *dirent;
	const file_selector_entry *entry;
	const file_selector_entry *selected_entry = nullptr;
	int i;
	const char *volume_name;
	const char *path = m_current_directory.c_str();

	// open the directory
	err = util::zippath_opendir(path, &directory);

	// clear out the menu entries
	m_entrylist.clear();

	if (m_has_empty)
	{
		// add the "[empty slot]" entry
		append_entry(SELECTOR_ENTRY_TYPE_EMPTY, "", "");
	}

	if (m_has_create && !util::zippath_is_zip(directory))
	{
		// add the "[create]" entry
		append_entry(SELECTOR_ENTRY_TYPE_CREATE, "", "");
	}

	if (m_has_softlist)
	{
		// add the "[software list]" entry
		entry = &append_entry(SELECTOR_ENTRY_TYPE_SOFTWARE_LIST, "", "");
		selected_entry = entry;
	}

	// add the drives
	i = 0;
	while((volume_name = osd_get_volume_name(i))!=nullptr)
	{
		append_entry(SELECTOR_ENTRY_TYPE_DRIVE,
			volume_name, volume_name);
		i++;
	}

	// build the menu for each item
	if (err == osd_file::error::NONE)
	{
		while((dirent = util::zippath_readdir(directory)) != nullptr)
		{
			// append a dirent entry
			entry = append_dirent_entry(dirent);

			if (entry != nullptr)
			{
				// set the selected item to be the first non-parent directory or file
				if ((selected_entry == nullptr) && strcmp(dirent->name, ".."))
					selected_entry = entry;

				// do we have to select this file?
				if (!core_stricmp(m_current_file.c_str(), dirent->name))
					selected_entry = entry;
			}
		}
	}

	// append all of the menu entries
	for (auto &entry : m_entrylist)
		append_entry_menu_item(&entry);

	// set the selection (if we have one)
	if (selected_entry != nullptr)
		set_selection((void *) selected_entry);

	// set up custom render proc
	customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;

	if (directory != nullptr)
		util::zippath_closedir(directory);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_file_selector::handle()
{
	osd_file::error err;
	const file_selector_entry *selected_entry = nullptr;
	int bestmatch = 0;

	// process the menu
	const event *event = process(0);
	if (event != nullptr && event->itemref != nullptr)
	{
		// handle selections
		if (event->iptkey == IPT_UI_SELECT)
		{
			auto entry = (const file_selector_entry *) event->itemref;
			switch (entry->type)
			{
			case SELECTOR_ENTRY_TYPE_EMPTY:
				// empty slot - unload
				m_result = result::EMPTY;
				menu::stack_pop(machine());
				break;

			case SELECTOR_ENTRY_TYPE_CREATE:
				// create
				m_result = result::CREATE;
				menu::stack_pop(machine());
				break;

			case SELECTOR_ENTRY_TYPE_SOFTWARE_LIST:
				m_result = result::SOFTLIST;
				menu::stack_pop(machine());
				break;

			case SELECTOR_ENTRY_TYPE_DRIVE:
			case SELECTOR_ENTRY_TYPE_DIRECTORY:
				// drive/directory - first check the path
				err = util::zippath_opendir(entry->fullpath.c_str(), nullptr);
				if (err != osd_file::error::NONE)
				{
					// this path is problematic; present the user with an error and bail
					ui().popup_time(1, "Error accessing %s", entry->fullpath);
					break;
				}
				m_current_directory.assign(entry->fullpath);
				reset(reset_options::SELECT_FIRST);
				break;

			case SELECTOR_ENTRY_TYPE_FILE:
				// file
				m_current_file.assign(entry->fullpath);
				m_result = result::FILE;
				menu::stack_pop(machine());
				break;
			}

			// reset the char buffer when pressing IPT_UI_SELECT
			m_filename.clear();
		}
		else if (event->iptkey == IPT_SPECIAL)
		{
			bool update_selected = false;

			if ((event->unichar == 8) || (event->unichar == 0x7f))
			{
				// if it's a backspace and we can handle it, do so
				auto const buflen = m_filename.size();
				if (0 < buflen)
				{
					auto buffer_oldend = m_filename.c_str() + buflen;
					auto buffer_newend = utf8_previous_char(buffer_oldend);
					m_filename.resize(buffer_newend - m_filename.c_str());
					update_selected = true;

					ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename.c_str());
				}
			}
			else if (event->is_char_printable())
			{
				// if it's any other key and we're not maxed out, update
				m_filename += utf8_from_uchar(event->unichar);
				update_selected = true;

				ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename.c_str());
			}

			if (update_selected)
			{
				file_selector_entry const *const cur_selected(reinterpret_cast<file_selector_entry const *>(get_selection_ref()));

				// check for entries which matches our m_filename_buffer:
				for (auto &entry : m_entrylist)
				{
					if (cur_selected != &entry)
					{
						int match = 0;
						for (int i = 0; i < m_filename.size(); i++)
						{
							if (core_strnicmp(entry.basename.c_str(), m_filename.c_str(), i) == 0)
								match = i;
						}

						if (match > bestmatch)
						{
							bestmatch = match;
							selected_entry = &entry;
						}
					}
				}

				if (selected_entry != nullptr && selected_entry != cur_selected)
				{
					set_selection((void *)selected_entry);
					centre_selection();
				}
			}
		}
		else if (event->iptkey == IPT_UI_CANCEL)
		{
			// reset the char buffer also in this case
			m_filename.clear();
		}
	}
}



/***************************************************************************
    SELECT RW
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_rw::menu_select_rw(mame_ui_manager &mui, render_container *container,
										bool can_in_place, result &result)
	: menu(mui, container),
	  m_can_in_place(can_in_place),
	  m_result(result)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_rw::~menu_select_rw()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_select_rw::populate()
{
	item_append(_("Select access mode"), "", FLAG_DISABLE, nullptr);
	item_append(_("Read-only"), "", 0, itemref_from_result(result::READONLY));
	if (m_can_in_place)
		item_append(_("Read-write"), "", 0, itemref_from_result(result::READWRITE));
	item_append(_("Read this image, write to another image"), "", 0, itemref_from_result(result::WRITE_OTHER));
	item_append(_("Read this image, write to diff"), "", 0, itemref_from_result(result::WRITE_DIFF));
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_select_rw::handle()
{
	// process the menu
	const event *event = process(0);
	if (event != nullptr && event->iptkey == IPT_UI_SELECT)
	{
		m_result = result_from_itemref(event->itemref);
		menu::stack_pop(machine());
	}
}


//-------------------------------------------------
//  itemref_from_result
//-------------------------------------------------

void *menu_select_rw::itemref_from_result(menu_select_rw::result result)
{
	return (void *)(FPTR)(unsigned int)result;
}


//-------------------------------------------------
//  result_from_itemref
//-------------------------------------------------

menu_select_rw::result menu_select_rw::result_from_itemref(void *itemref)
{
	return (menu_select_rw::result) (unsigned int) (FPTR)itemref;
}


} // namespace ui
