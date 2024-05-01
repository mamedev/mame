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

#include "uiinput.h"

#include "util/corestr.h"
#include "util/zippath.h"

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

menu_file_selector::menu_file_selector(
		mame_ui_manager &mui,
		render_container &container,
		device_image_interface *image,
		std::string_view directory,
		std::string_view file,
		bool has_empty,
		bool has_softlist,
		bool has_create,
		handler_function &&handler)
	: menu(mui, container)
	, m_handler(std::move(handler))
	, m_image(image)
	, m_current_directory(directory)
	, m_current_file(file)
	, m_result(result::INVALID)
	, m_has_empty(has_empty)
	, m_has_softlist(has_softlist)
	, m_has_create(has_create)
	, m_clicked_directory(std::string::npos, std::string::npos)
{
	(void)m_image;
	set_process_flags(PROCESS_IGNOREPAUSE);
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_file_selector::~menu_file_selector()
{
	if (m_handler)
		m_handler(m_result, std::move(m_current_directory), std::move(m_current_file));
}


//-------------------------------------------------
//  recompute_metrics - recompute metrics
//-------------------------------------------------

void menu_file_selector::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	m_path_layout.reset();
	m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);

	set_custom_space(line_height() + 3.0F * tb_border(), 0.0F);
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_file_selector::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// lay out extra text
	if (!m_path_layout)
	{
		m_path_layout.emplace(create_layout());
		m_path_layout->add_text(m_current_directory);
	}
	else
	{
		rgb_t const fgcolor = ui().colors().text_color();
		rgb_t const bgcolor = rgb_t::transparent();
		m_path_layout->restyle(0, m_current_directory.length(), &fgcolor, &bgcolor);
	}

	// position this extra text
	float x2, y2;
	extra_text_position(origx1, origx2, origy1, top, *m_path_layout, -1, m_path_position.first, m_path_position.second, x2, y2);

	// draw a box
	ui().draw_outlined_box(container(), m_path_position.first, m_path_position.second, x2, y2, ui().colors().background_color());

	// take off the borders
	m_path_position.first += lr_border();
	m_path_position.second += tb_border();

	if (m_clicked_directory.second > m_clicked_directory.first)
	{
		// see if it's still over the clicked path component
		auto const [x, y] = pointer_location();
		size_t start = 0, span = 0;
		if (m_path_layout->hit_test(x - m_path_position.first, y - m_path_position.second, start, span))
		{
			if ((start >= m_clicked_directory.first) && ((start + span) <= m_clicked_directory.second))
			{
				rgb_t const fgcolor = ui().colors().selected_color();
				rgb_t const bgcolor = ui().colors().selected_bg_color();
				m_path_layout->restyle(m_clicked_directory.first, m_clicked_directory.second - m_clicked_directory.first, &fgcolor, &bgcolor);
			}
		}
	}
	else if (pointer_idle())
	{
		// see if it's hovering over a path component
		auto const [x, y] = pointer_location();
		auto const [target_dir_start, target_dir_end] = get_directory_range(x, y);
		if (target_dir_end > target_dir_start)
		{
			rgb_t const fgcolor = ui().colors().mouseover_color();
			rgb_t const bgcolor = ui().colors().mouseover_bg_color();
			m_path_layout->restyle(target_dir_start, target_dir_end - target_dir_start, &fgcolor, &bgcolor);
		}
	}

	// draw the text within it
	m_path_layout->emit(container(), m_path_position.first, m_path_position.second);
}


//-------------------------------------------------
//  custom_pointer_updated - perform our special
//  pointer handling
//-------------------------------------------------

std::tuple<int, bool, bool> menu_file_selector::custom_pointer_updated(bool changed, ui_event const &uievt)
{
	// track pointer after clicking a path component
	if (m_clicked_directory.second > m_clicked_directory.first)
	{
		if (ui_event::type::POINTER_ABORT == uievt.event_type)
		{
			// abort always cancels
			m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
			return std::make_tuple(IPT_INVALID, false, true);
		}
		else if (uievt.pointer_released & 0x01)
		{
			// releasing the primary button - check for dragging out
			auto const [x, y] = pointer_location();
			size_t start = 0, span = 0;
			if (m_path_layout->hit_test(x - m_path_position.first, y - m_path_position.second, start, span))
			{
				// abuse IPT_CUSTOM to change to the clicked directory
				if ((start >= m_clicked_directory.first) && ((start + span) <= m_clicked_directory.second))
					return std::make_tuple(IPT_CUSTOM, false, true);
			}
			m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
			return std::make_tuple(IPT_INVALID, false, true);
		}
		else if (uievt.pointer_buttons & ~u32(1))
		{
			// pressing more buttons cancels
			m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
			return std::make_tuple(IPT_INVALID, false, true);
		}
		else
		{
			// keep tracking the pointer
			return std::make_tuple(IPT_INVALID, true, false);
		}
	}

	// check for clicks if we have up-to-date content on-screen
	if (m_path_layout && pointer_idle() && (uievt.pointer_buttons & 0x01) && !(uievt.pointer_buttons & ~u32(0x01)))
	{
		auto const [x, y] = pointer_location();
		auto const [target_dir_start, target_dir_end] = get_directory_range(x, y);
		if (target_dir_end > target_dir_start)
		{
			m_clicked_directory = std::make_pair(target_dir_start, target_dir_end);
			return std::make_tuple(IPT_INVALID, true, true);
		}
	}

	return std::make_tuple(IPT_INVALID, false, false);
}


//-------------------------------------------------
//  menu_activated - menu has gained focus
//-------------------------------------------------

void menu_file_selector::menu_activated()
{
	m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
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
		m_current_directory = entry.fullpath;
		m_path_layout.reset();
		m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
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
//  update_search
//-------------------------------------------------

void menu_file_selector::update_search()
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


//-------------------------------------------------
//  get_directory_range
//-------------------------------------------------

std::pair<size_t, size_t> menu_file_selector::get_directory_range(float x, float y)
{
	size_t start = 0, span = 0;
	if (m_path_layout->hit_test(x - m_path_position.first, y - m_path_position.second, start, span))
	{
		if (std::string_view(m_current_directory).substr(start, span) != PATH_SEPARATOR)
		{
			auto target_start = m_current_directory.rfind(PATH_SEPARATOR, start);
			if (std::string::npos == target_start)
				target_start = 0;
			else
				target_start += 1;

			auto target_end = m_current_directory.find(PATH_SEPARATOR, start + span);
			if (std::string::npos == target_end)
				target_end = m_current_directory.length();

			return std::make_pair(target_start, target_end);
		}
	}

	return std::make_pair(std::string::npos, std::string::npos);
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_file_selector::populate()
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
				if (!core_stricmp(m_current_file, dirent->name))
					selected_entry = entry;
			}
		}
	}
	directory.reset();

	if (m_entrylist.size() > first)
	{
		// sort the menu entries
		std::locale const lcl;
		std::collate<wchar_t> const &coll = std::use_facet<std::collate<wchar_t> >(lcl);
		std::sort(
				m_entrylist.begin() + first,
				m_entrylist.end(),
				[&coll] (file_selector_entry const &x, file_selector_entry const &y)
				{
					std::wstring const xstr = wstring_from_utf8(x.basename);
					std::wstring const ystr = wstring_from_utf8(y.basename);
					return coll.compare(xstr.data(), xstr.data() + xstr.size(), ystr.data(), ystr.data() + ystr.size()) < 0;
				});
	}

	// append all of the menu entries
	for (file_selector_entry const &entry : m_entrylist)
		append_entry_menu_item(&entry);

	// set the selection (if we have one)
	if (selected_entry)
		set_selection((void *)selected_entry);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_file_selector::handle(event const *ev)
{
	if (!ev)
		return false;

	if (ev->iptkey == IPT_SPECIAL)
	{
		// if it's any other key and we're not maxed out, update
		if (input_character(m_filename, ev->unichar, uchar_is_printable))
		{
			update_search();
			return true;
		}
	}
	else if (ev->iptkey == IPT_UI_PASTE)
	{
		if (paste_text(m_filename, uchar_is_printable))
		{
			update_search();
			return true;
		}
	}
	else if (ev->iptkey == IPT_UI_CANCEL)
	{
		// reset the char buffer also in this case
		if (!m_filename.empty())
		{
			m_filename.clear();
			ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename);
			return true;
		}
	}
	else if (ev->iptkey == IPT_CUSTOM)
	{
		// clicked a path component
		if (m_clicked_directory.second > m_clicked_directory.first)
		{
			m_current_directory.resize(m_clicked_directory.second + strlen(PATH_SEPARATOR));
			m_path_layout.reset();
			m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
			reset(reset_options::SELECT_FIRST);
			return true;
		}
	}
	else if (ev->itemref && (ev->iptkey == IPT_UI_SELECT))
	{
		// handle selections
		select_item(*reinterpret_cast<file_selector_entry const *>(ev->itemref));

		// reset the char buffer when pressing IPT_UI_SELECT
		m_filename.clear();
		return true;
	}

	return false;
}



/***************************************************************************
    SELECT RW
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_rw::menu_select_rw(
		mame_ui_manager &mui,
		render_container &container,
		bool can_in_place,
		handler_function &&handler)
	: menu(mui, container)
	, m_handler(std::move(handler))
	, m_can_in_place(can_in_place)
	, m_result(result::INVALID)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_rw::~menu_select_rw()
{
	if (m_handler)
		m_handler(m_result);
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_select_rw::populate()
{
	set_heading(_("Select access mode"));

	item_append(_("Read-only"), 0, itemref_from_result(result::READONLY));
	if (m_can_in_place)
		item_append(_("Read-write"), 0, itemref_from_result(result::READWRITE));
	item_append(_("Read this image, write to another image"), 0, itemref_from_result(result::WRITE_OTHER));
	item_append(_("Read this image, write to diff"), 0, itemref_from_result(result::WRITE_DIFF));
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_select_rw::handle(event const *ev)
{
	if (ev && ev->iptkey == IPT_UI_SELECT)
	{
		m_result = result_from_itemref(ev->itemref);
		stack_pop();
	}

	return false;
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
