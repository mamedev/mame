// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/filesel.cpp

    MESS's clunky built-in file manager

    TODO
        - Support image creation arguments
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

// itemrefs for key menu items
#define ITEMREF_NEW_IMAGE_NAME  ((void *) 0x0001)
#define ITEMREF_CREATE          ((void *) 0x0002)
#define ITEMREF_FORMAT          ((void *) 0x0003)
#define ITEMREF_NO              ((void *) 0x0004)
#define ITEMREF_YES             ((void *) 0x0005)


/***************************************************************************
    MENU HELPERS
***************************************************************************/

//-------------------------------------------------
//  input_character - inputs a typed character
//  into a buffer
//-------------------------------------------------

template <std::size_t N, typename F>
static void input_character(char (&buffer)[N], unicode_char unichar, F &&filter)
{
	auto buflen = std::strlen(buffer);

	if ((unichar == 8) || (unichar == 0x7f))
	{
		if (0 < buflen)
			*const_cast<char *>(utf8_previous_char(&buffer[buflen])) = 0;
	}
	else if ((unichar >= ' ') && (!filter || filter(unichar)))
	{
		auto const chlen = utf8_from_uchar(&buffer[buflen], N - buflen - 1, unichar);
		if (0 <= chlen)
		{
			buflen += chlen;
			buffer[buflen] = 0;
		}
	}
}


/***************************************************************************
    CONFIRM SAVE AS MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_confirm_save_as::menu_confirm_save_as(mame_ui_manager &mui, render_container *container, bool *yes)
	: menu(mui, container)
{
	m_yes = yes;
	*m_yes = false;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_confirm_save_as::~menu_confirm_save_as()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_confirm_save_as::populate()
{
	item_append(_("File Already Exists - Override?"), nullptr, FLAG_DISABLE, nullptr);
	item_append(MENU_SEPARATOR_ITEM, nullptr, FLAG_DISABLE, nullptr);
	item_append(_("No"), nullptr, 0, ITEMREF_NO);
	item_append(_("Yes"), nullptr, 0, ITEMREF_YES);
}

//-------------------------------------------------
//  handle - confirm save as menu
//-------------------------------------------------

void menu_confirm_save_as::handle()
{
	// process the menu
	const event *event = process(0);

	// process the event
	if ((event != nullptr) && (event->iptkey == IPT_UI_SELECT))
	{
		if (event->itemref == ITEMREF_YES)
			*m_yes = true;

		// no matter what, pop out
		menu::stack_pop(machine());
	}
}



/***************************************************************************
    FILE CREATE MENU
***************************************************************************/

//-------------------------------------------------
//  is_valid_filename_char - tests to see if a
//  character is valid in a filename
//-------------------------------------------------

static int is_valid_filename_char(unicode_char unichar)
{
	// this should really be in the OSD layer, and it shouldn't be 7-bit bullshit
	static const char valid_filename_char[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 00-0f
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 10-1f
		1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0,     //  !"#$%&'()*+,-./
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,     // 0123456789:;<=>?
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // @ABCDEFGHIJKLMNO
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,     // PQRSTUVWXYZ[\]^_
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     // `abcdefghijklmno
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0,     // pqrstuvwxyz{|}~
	};
	return (unichar < ARRAY_LENGTH(valid_filename_char)) && valid_filename_char[unichar];
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_file_create::menu_file_create(mame_ui_manager &mui, render_container *container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool *ok)
	: menu(mui, container)
	, m_current_directory(current_directory)
	, m_current_file(current_file)
	, m_current_format(nullptr)
{
	m_image = image;
	m_ok = ok;
	*m_ok = true;
	auto const sep = current_file.rfind(PATH_SEPARATOR);
	std::strncpy(m_filename_buffer, current_file.c_str() + ((std::string::npos == sep) ? 0 : (sep + 1)), sizeof(m_filename_buffer));
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_file_create::~menu_file_create()
{
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_file_create::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	extra_text_render(top, bottom, origx1, origy1, origx2, origy2,
		m_current_directory.c_str(),
		nullptr);
}


//-------------------------------------------------
//  populate - populates the file creator menu
//-------------------------------------------------

void menu_file_create::populate()
{
	std::string buffer;
	const image_device_format *format;
	const char *new_image_name;

	// append the "New Image Name" item
	if (get_selection() == ITEMREF_NEW_IMAGE_NAME)
	{
		buffer.append(m_filename_buffer).append("_");
		new_image_name = buffer.c_str();
	}
	else
	{
		new_image_name = m_filename_buffer;
	}
	item_append(_("New Image Name:"), new_image_name, 0, ITEMREF_NEW_IMAGE_NAME);

	// do we support multiple formats?
	if (ENABLE_FORMATS) format = m_image->formatlist().first();
	if (ENABLE_FORMATS && (format != nullptr))
	{
		item_append(_("Image Format:"), m_current_format->description(), 0, ITEMREF_FORMAT);
		m_current_format = format;
	}

	// finish up the menu
	item_append(menu_item_type::SEPARATOR);
	item_append(_("Create"), nullptr, 0, ITEMREF_CREATE);

	customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}


//-------------------------------------------------
//  handle - file creator menu
//-------------------------------------------------

void menu_file_create::handle()
{
	// process the menu
	const event *event = process(0);

	// process the event
	if (event)
	{
		// handle selections
		switch (event->iptkey)
		{
		case IPT_UI_SELECT:
			if ((event->itemref == ITEMREF_CREATE) || (event->itemref == ITEMREF_NEW_IMAGE_NAME))
			{
				std::string tmp_file(m_filename_buffer);
				if (tmp_file.find(".") != -1 && tmp_file.find(".") < tmp_file.length() - 1)
				{
					m_current_file = m_filename_buffer;
					menu::stack_pop(machine());
				}
				else
					ui().popup_time(1, "%s", _("Please enter a file extension too"));
			}
			break;

		case IPT_SPECIAL:
			if (get_selection() == ITEMREF_NEW_IMAGE_NAME)
			{
				input_character(m_filename_buffer,event->unichar, is_valid_filename_char);
				reset(reset_options::REMEMBER_POSITION);
			}
			break;
		case IPT_UI_CANCEL:
			*m_ok = false;
			break;
		}
	}
}

/***************************************************************************
    FILE SELECTOR MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_file_selector::menu_file_selector(mame_ui_manager &mui, render_container *container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool has_empty, bool has_softlist, bool has_create, int *result)
	: menu(mui, container)
	, m_current_directory(current_directory)
	, m_current_file(current_file)
	, m_entrylist(nullptr)
{
	m_image = image;
	m_has_empty = has_empty;
	m_has_softlist = has_softlist;
	m_has_create = has_create;
	m_result = result;
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
	extra_text_render(top, bottom,
		origx1, origy1, origx2, origy2,
		m_current_directory.c_str(),
		nullptr);
}



//-------------------------------------------------
//  compare_file_selector_entries - sorting proc
//  for file selector entries
//-------------------------------------------------

int menu_file_selector::compare_entries(const file_selector_entry *e1, const file_selector_entry *e2)
{
	int result;
	const char *e1_basename = (e1->basename != nullptr) ? e1->basename : "";
	const char *e2_basename = (e2->basename != nullptr) ? e2->basename : "";

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

menu_file_selector::file_selector_entry *menu_file_selector::append_entry(
	file_selector_entry_type entry_type, const char *entry_basename, const char *entry_fullpath)
{
	file_selector_entry *entry;
	file_selector_entry **entryptr;

	// allocate a new entry
	entry = (file_selector_entry *) m_pool_alloc(sizeof(*entry));
	memset(entry, 0, sizeof(*entry));
	entry->type = entry_type;
	entry->basename = (entry_basename != nullptr) ? pool_strdup(entry_basename) : entry_basename;
	entry->fullpath = (entry_fullpath != nullptr) ? pool_strdup(entry_fullpath) : entry_fullpath;

	// find the end of the list
	entryptr = &m_entrylist;
	while ((*entryptr != nullptr) && (compare_entries(entry, *entryptr) >= 0))
		entryptr = &(*entryptr)->next;

	// insert the entry
	entry->next = *entryptr;
	*entryptr = entry;
	return entry;
}


//-------------------------------------------------
//  append_entry_menu_item - appends
//  a menu item for a file selector entry
//-------------------------------------------------

menu_file_selector::file_selector_entry *menu_file_selector::append_dirent_entry(const osd_directory_entry *dirent)
{
	std::string buffer;
	file_selector_entry_type entry_type;
	file_selector_entry *entry;

	switch(dirent->type)
	{
		case ENTTYPE_FILE:
			entry_type = SELECTOR_ENTRY_TYPE_FILE;
			break;

		case ENTTYPE_DIR:
			entry_type = SELECTOR_ENTRY_TYPE_DIRECTORY;
			break;

		default:
			// exceptional case; do not add a menu item
			return nullptr;
	}

	// determine the full path
	util::zippath_combine(buffer, m_current_directory.c_str(), dirent->name);

	// create the file selector entry
	entry = append_entry(
		entry_type,
		dirent->name,
		buffer.c_str());

	return entry;
}


//-------------------------------------------------
//  append_entry_menu_item - appends
//  a menu item for a file selector entry
//-------------------------------------------------

void menu_file_selector::append_entry_menu_item(const file_selector_entry *entry)
{
	const char *text = nullptr;
	const char *subtext = nullptr;

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
	item_append(text, subtext, 0, (void *) entry);
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_file_selector::populate()
{
	util::zippath_directory *directory = nullptr;
	osd_file::error err;
	const osd_directory_entry *dirent;
	const file_selector_entry *entry;
	const file_selector_entry *selected_entry = nullptr;
	int i;
	const char *volume_name;
	const char *path = m_current_directory.c_str();

	// open the directory
	err = util::zippath_opendir(path, &directory);

	// clear out the menu entries
	m_entrylist = nullptr;

	if (m_has_empty)
	{
		// add the "[empty slot]" entry
		append_entry(SELECTOR_ENTRY_TYPE_EMPTY, nullptr, nullptr);
	}

	if (m_has_create)
	{
		// add the "[create]" entry
		append_entry(SELECTOR_ENTRY_TYPE_CREATE, nullptr, nullptr);
	}

	if (m_has_softlist)
	{
		// add the "[software list]" entry
		entry = append_entry(SELECTOR_ENTRY_TYPE_SOFTWARE_LIST, nullptr, nullptr);
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
	for (entry = m_entrylist; entry != nullptr; entry = entry->next)
		append_entry_menu_item(entry);

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
	const file_selector_entry *entry;
	const file_selector_entry *selected_entry = nullptr;
	int bestmatch = 0;

	// process the menu
	const event *event = process(0);
	if (event != nullptr && event->itemref != nullptr)
	{
		// handle selections
		if (event->iptkey == IPT_UI_SELECT)
		{
			entry = (const file_selector_entry *) event->itemref;
			switch (entry->type)
			{
			case SELECTOR_ENTRY_TYPE_EMPTY:
				// empty slot - unload
				*m_result = R_EMPTY;
				menu::stack_pop(machine());
				break;

			case SELECTOR_ENTRY_TYPE_CREATE:
				// create
				*m_result = R_CREATE;
				menu::stack_pop(machine());
				break;

			case SELECTOR_ENTRY_TYPE_SOFTWARE_LIST:
				*m_result = R_SOFTLIST;
				menu::stack_pop(machine());
				break;

			case SELECTOR_ENTRY_TYPE_DRIVE:
			case SELECTOR_ENTRY_TYPE_DIRECTORY:
				// drive/directory - first check the path
				err = util::zippath_opendir(entry->fullpath, nullptr);
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
				*m_result = R_FILE;
				menu::stack_pop(machine());
				break;
			}

			// reset the char buffer when pressing IPT_UI_SELECT
			if (m_filename_buffer[0] != '\0')
				memset(m_filename_buffer, '\0', ARRAY_LENGTH(m_filename_buffer));
		}
		else if (event->iptkey == IPT_SPECIAL)
		{
			auto const buflen = std::strlen(m_filename_buffer);
			bool update_selected = FALSE;

			if ((event->unichar == 8) || (event->unichar == 0x7f))
			{
				// if it's a backspace and we can handle it, do so
				if (0 < buflen)
				{
					*const_cast<char *>(utf8_previous_char(&m_filename_buffer[buflen])) = 0;
					update_selected = TRUE;

					ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename_buffer);
				}
			}
			else if (event->is_char_printable())
			{
				// if it's any other key and we're not maxed out, update
				if (event->append_char(m_filename_buffer, buflen))
				{
					update_selected = TRUE;

					ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename_buffer);
				}
			}

			if (update_selected)
			{
				const file_selector_entry *cur_selected = (const file_selector_entry *)get_selection();

				// check for entries which matches our m_filename_buffer:
				// from current entry to the end
				for (entry = cur_selected; entry != nullptr; entry = entry->next)
				{
					if (entry->basename != nullptr && m_filename_buffer[0] != '\0')
					{
						int match = 0;
						for (int i = 0; i < ARRAY_LENGTH(m_filename_buffer); i++)
						{
							if (core_strnicmp(entry->basename, m_filename_buffer, i) == 0)
								match = i;
						}

						if (match > bestmatch)
						{
							bestmatch = match;
							selected_entry = entry;
						}
					}
				}
				// and from the first entry to current one
				for (entry = m_entrylist; entry != cur_selected; entry = entry->next)
				{
					if (entry->basename != nullptr && m_filename_buffer[0] != '\0')
					{
						int match = 0;
						for (int i = 0; i < ARRAY_LENGTH(m_filename_buffer); i++)
						{
							if (core_strnicmp(entry->basename, m_filename_buffer, i) == 0)
								match = i;
						}

						if (match > bestmatch)
						{
							bestmatch = match;
							selected_entry = entry;
						}
					}
				}

				if (selected_entry != nullptr && selected_entry != cur_selected)
				{
					set_selection((void *)selected_entry);
					top_line = selected - (visible_lines / 2);
				}
			}
		}
		else if (event->iptkey == IPT_UI_CANCEL)
		{
			// reset the char buffer also in this case
			if (m_filename_buffer[0] != '\0')
				memset(m_filename_buffer, '\0', ARRAY_LENGTH(m_filename_buffer));
		}
	}
}



/***************************************************************************
    SELECT FORMAT MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_format::menu_select_format(mame_ui_manager &mui, render_container *container, floppy_image_format_t **formats, int ext_match, int total_usable, int *result)
	: menu(mui, container)
{
	m_formats = formats;
	m_ext_match = ext_match;
	m_total_usable = total_usable;
	m_result = result;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_format::~menu_select_format()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_select_format::populate()
{
	item_append(_("Select image format"), nullptr, FLAG_DISABLE, nullptr);
	for (int i = 0; i < m_total_usable; i++)
	{
		const floppy_image_format_t *fmt = m_formats[i];

		if (i && i == m_ext_match)
			item_append(menu_item_type::SEPARATOR);
		item_append(fmt->description(), fmt->name(), 0, (void *)(FPTR)i);
	}
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_select_format::handle()
{
	// process the menu
	const event *event = process(0);
	if (event != nullptr && event->iptkey == IPT_UI_SELECT)
	{
		*m_result = int(FPTR(event->itemref));
		menu::stack_pop(machine());
	}
}


/***************************************************************************
    SELECT RW
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_rw::menu_select_rw(mame_ui_manager &mui, render_container *container,
										bool can_in_place, int *result)
	: menu(mui, container)
{
	m_can_in_place = can_in_place;
	m_result = result;
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
	item_append(_("Select access mode"), nullptr, FLAG_DISABLE, nullptr);
	item_append(_("Read-only"), nullptr, 0, (void *)READONLY);
	if (m_can_in_place)
		item_append(_("Read-write"), nullptr, 0, (void *)READWRITE);
	item_append(_("Read this image, write to another image"), nullptr, 0, (void *)WRITE_OTHER);
	item_append(_("Read this image, write to diff"), nullptr, 0, (void *)WRITE_DIFF);
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
		*m_result = int(FPTR(event->itemref));
		menu::stack_pop(machine());
	}
}

} // namespace ui
