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
#include "ui/utils.h"

#include "imagedev/floppy.h"

#include "corestr.h"
#include "zippath.h"

#include <cstring>
#include <locale>


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

menu_file_selector::menu_file_selector(mame_ui_manager &mui, render_container &container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool has_empty, bool has_softlist, bool has_create, menu_file_selector::result &result)
	: menu(mui, container)
	, m_image(image)
	, m_current_directory(current_directory)
	, m_current_file(current_file)
	, m_has_empty(has_empty)
	, m_has_softlist(has_softlist)
	, m_has_create(has_create)
	, m_result(result)
{
	(void)m_image;
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
	auto layout = ui().create_layout(container());
	layout.add_text(m_current_directory);

	// position this extra text
	float x1, y1, x2, y2;
	extra_text_position(origx1, origx2, origy1, top, layout, -1, x1, y1, x2, y2);

	// draw a box
	ui().draw_outlined_box(container(), x1, y1, x2, y2, ui().colors().background_color());

	// take off the borders
	x1 += ui().box_lr_border() * machine().render().ui_aspect(&container());
	y1 += ui().box_tb_border();

	size_t hit_start = 0, hit_span = 0;
	if (is_mouse_hit()
		&& layout.hit_test(get_mouse_x() - x1, get_mouse_y() - y1, hit_start, hit_span)
		&& m_current_directory.substr(hit_start, hit_span) != PATH_SEPARATOR)
	{
		// we're hovering over a directory!  highlight it
		auto target_dir_start = m_current_directory.rfind(PATH_SEPARATOR, hit_start) + 1;
		auto target_dir_end = m_current_directory.find(PATH_SEPARATOR, hit_start + hit_span);
		m_hover_directory = m_current_directory.substr(0, target_dir_end + strlen(PATH_SEPARATOR));

		// highlight the text in question
		rgb_t fgcolor = ui().colors().mouseover_color();
		rgb_t bgcolor = ui().colors().mouseover_bg_color();
		layout.restyle(target_dir_start, target_dir_end - target_dir_start, &fgcolor, &bgcolor);
	}
	else
	{
		// we are not hovering over anything
		m_hover_directory.clear();
	}

	// draw the text within it
	layout.emit(container(), x1, y1);
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
		file_selector_entry_type entry_type,
		const std::string &entry_basename,
		const std::string &entry_fullpath)
{
	return append_entry(entry_type, std::string(entry_basename), std::string(entry_fullpath));
}


//-------------------------------------------------
//  append_entry - appends a new
//  file selector entry to an entry list
//-------------------------------------------------

menu_file_selector::file_selector_entry &menu_file_selector::append_entry(
		file_selector_entry_type entry_type,
		std::string &&entry_basename,
		std::string &&entry_fullpath)
{
	// allocate a new entry
	file_selector_entry entry;
	entry.type = entry_type;
	entry.basename = std::move(entry_basename);
	entry.fullpath = std::move(entry_fullpath);

	// find the end of the list
	return m_entrylist.emplace_back(std::move(entry));
}


//-------------------------------------------------
//  append_dirent_entry - appends
//  a menu item for a file selector entry
//-------------------------------------------------

menu_file_selector::file_selector_entry *menu_file_selector::append_dirent_entry(const osd::directory::entry *dirent)
{
	file_selector_entry_type entry_type;
	switch (dirent->type)
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
	std::string buffer = util::zippath_combine(m_current_directory, dirent->name);

	// create the file selector entry
	return &append_entry(
			entry_type,
			dirent->name,
			std::move(buffer));
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
//  select_item
//-------------------------------------------------

void menu_file_selector::select_item(const file_selector_entry &entry)
{
	switch (entry.type)
	{
	case SELECTOR_ENTRY_TYPE_EMPTY:
		// empty slot - unload
		m_result = result::EMPTY;
		stack_pop();
		break;

	case SELECTOR_ENTRY_TYPE_CREATE:
		// create
		m_result = result::CREATE;
		stack_pop();
		break;

	case SELECTOR_ENTRY_TYPE_SOFTWARE_LIST:
		m_result = result::SOFTLIST;
		stack_pop();
		break;

	case SELECTOR_ENTRY_TYPE_DRIVE:
	case SELECTOR_ENTRY_TYPE_DIRECTORY:
		{
			// drive/directory - first check the path
			util::zippath_directory::ptr dir;
			std::error_condition const err = util::zippath_directory::open(entry.fullpath, dir);
			if (err)
			{
				// this path is problematic; present the user with an error and bail
				ui().popup_time(1, _("Error accessing %s"), entry.fullpath);
				break;
			}
		}
		m_current_directory.assign(entry.fullpath);
		reset(reset_options::SELECT_FIRST);
		break;

	case SELECTOR_ENTRY_TYPE_FILE:
		// file
		m_current_file.assign(entry.fullpath);
		m_result = result::FILE;
		stack_pop();
		break;
	}
}


//-------------------------------------------------
//  type_search_char
//-------------------------------------------------

void menu_file_selector::type_search_char(char32_t ch)
{
	std::string const current(m_filename);
	if (input_character(m_filename, ch, uchar_is_printable))
	{
		ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename);

		file_selector_entry const *const cur_selected(reinterpret_cast<file_selector_entry const *>(get_selection_ref()));

		// if it's a perfect match for the current selection, don't move it
		if (!cur_selected || core_strnicmp(cur_selected->basename.c_str(), m_filename.c_str(), m_filename.size()))
		{
			std::string::size_type bestmatch(0);
			file_selector_entry const *selected_entry(cur_selected);
			for (auto &entry : m_entrylist)
			{
				// TODO: more efficient "common prefix" code
				std::string::size_type match(0);
				for (std::string::size_type i = 1; m_filename.size() >= i; ++i)
				{
					if (!core_strnicmp(entry.basename.c_str(), m_filename.c_str(), i))
						match = i;
					else
						break;
				}

				if (match > bestmatch)
				{
					bestmatch = match;
					selected_entry = &entry;
				}
			}

			if (selected_entry && (selected_entry != cur_selected))
			{
				set_selection((void *)selected_entry);
				centre_selection();
			}
		}
	}
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_file_selector::populate(float &customtop, float &custombottom)
{
	const file_selector_entry *selected_entry = nullptr;

	// clear out the menu entries
	m_entrylist.clear();

	// open the directory
	util::zippath_directory::ptr directory;
	std::error_condition const err = util::zippath_directory::open(m_current_directory, directory);

	// add the "[empty slot]" entry if available
	if (m_has_empty)
		append_entry(SELECTOR_ENTRY_TYPE_EMPTY, "", "");

	// add the "[create]" entry
	if (m_has_create && directory && !directory->is_archive())
		append_entry(SELECTOR_ENTRY_TYPE_CREATE, "", "");

	// add and select the "[software list]" entry if available
	if (m_has_softlist)
		selected_entry = &append_entry(SELECTOR_ENTRY_TYPE_SOFTWARE_LIST, "", "");

	// add the drives
	for (std::string const &volume_name : osd_get_volume_names())
		append_entry(SELECTOR_ENTRY_TYPE_DRIVE, volume_name, volume_name);

	// mark first filename entry
	std::size_t const first = m_entrylist.size() + 1;

	// build the menu for each item
	if (err)
	{
		osd_printf_verbose(
				"menu_file_selector::populate: error opening directory '%s' (%s:%d %s)\n",
				m_current_directory, err.category().name(), err.value(), err.message());
	}
	else
	{
		for (osd::directory::entry const *dirent = directory->readdir(); dirent; dirent = directory->readdir())
		{
			// append a dirent entry
			file_selector_entry const *entry = append_dirent_entry(dirent);
			if (entry)
			{
				// set the selected item to be the first non-parent directory or file
				if (!selected_entry && strcmp(dirent->name, ".."))
					selected_entry = entry;

				// do we have to select this file?
				if (!core_stricmp(m_current_file.c_str(), dirent->name))
					selected_entry = entry;
			}
		}
	}
	directory.reset();

	if (m_entrylist.size() > first)
	{
		// sort the menu entries
		const std::collate<wchar_t> &coll = std::use_facet<std::collate<wchar_t>>(std::locale());
		std::sort(
				m_entrylist.begin() + first,
				m_entrylist.end(),
				[&coll] (file_selector_entry const &x, file_selector_entry const &y)
				{
					std::wstring const xstr = wstring_from_utf8(x.basename);
					std::wstring const ystr = wstring_from_utf8(y.basename);
					return coll.compare(xstr.data(), xstr.data()+xstr.size(), ystr.data(), ystr.data()+ystr.size()) < 0;
				});
	}

	// append all of the menu entries
	for (file_selector_entry const &entry : m_entrylist)
		append_entry_menu_item(&entry);

	// set the selection (if we have one)
	if (selected_entry)
		set_selection((void *)selected_entry);

	// set up custom render proc
	customtop = ui().get_line_height() + 3.0f * ui().box_tb_border();
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_file_selector::handle(event const *ev)
{
	// process the menu
	if (ev)
	{
		if (ev->iptkey == IPT_SPECIAL)
		{
			// if it's any other key and we're not maxed out, update
			type_search_char(ev->unichar);
		}
		else if (ev->iptkey == IPT_UI_CANCEL)
		{
			// reset the char buffer also in this case
			if (!m_filename.empty())
			{
				m_filename.clear();
				ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename);
			}
		}
		else if (ev->itemref && (ev->iptkey == IPT_UI_SELECT))
		{
			// handle selections
			select_item(*reinterpret_cast<file_selector_entry const *>(ev->itemref));

			// reset the char buffer when pressing IPT_UI_SELECT
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

menu_select_rw::menu_select_rw(mame_ui_manager &mui, render_container &container,
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

void menu_select_rw::populate(float &customtop, float &custombottom)
{
	item_append(_("Select access mode"), FLAG_DISABLE, nullptr);
	item_append(_("Read-only"), 0, itemref_from_result(result::READONLY));
	if (m_can_in_place)
		item_append(_("Read-write"), 0, itemref_from_result(result::READWRITE));
	item_append(_("Read this image, write to another image"), 0, itemref_from_result(result::WRITE_OTHER));
	item_append(_("Read this image, write to diff"), 0, itemref_from_result(result::WRITE_DIFF));
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_select_rw::handle(event const *ev)
{
	// process the menu
	if (ev && ev->iptkey == IPT_UI_SELECT)
	{
		m_result = result_from_itemref(ev->itemref);
		stack_pop();
	}
}


//-------------------------------------------------
//  itemref_from_result
//-------------------------------------------------

void *menu_select_rw::itemref_from_result(menu_select_rw::result result)
{
	return (void *)(uintptr_t)(unsigned int)result;
}


//-------------------------------------------------
//  result_from_itemref
//-------------------------------------------------

menu_select_rw::result menu_select_rw::result_from_itemref(void *itemref)
{
	return (menu_select_rw::result) (unsigned int) (uintptr_t)itemref;
}


} // namespace ui
