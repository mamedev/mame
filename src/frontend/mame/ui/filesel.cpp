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

#include "emuopts.h"
#include "fileio.h"
#include "imagedev/floppy.h"
#include "uiinput.h"

#include "util/corestr.h"
#include "util/path.h"
#include "util/zippath.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"

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
		render_target &target,
		device_image_interface &image,
		std::string_view directory,
		std::string_view file,
		bool has_empty,
		bool has_softlist,
		bool has_create,
		handler_function &&handler)
	: menu(mui, target)
	, m_handler(std::move(handler))
	, m_current_directory(directory)
	, m_current_file(file)
	, m_has_empty(has_empty)
	, m_has_softlist(has_softlist)
	, m_has_create(has_create)
	, m_is_midi((image.device().type() == MIDIIN) || (image.device().type() == MIDIOUT))
	, m_clicked_directory(std::string::npos, std::string::npos)
	, m_file_extensions(image.file_extensions() ? image.file_extensions() : "")
{
	set_process_flags(PROCESS_IGNOREPAUSE);
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_file_selector::~menu_file_selector()
{
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
		if (m_filter_extensions && !is_archive(dirent->name) && !extension_matches(dirent->name))
			return nullptr;
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

		case SELECTOR_ENTRY_TYPE_MIDI:
			text = _("[MIDI port]");
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
		m_handler(result::EMPTY, m_current_directory, m_current_file);
		break;

	case SELECTOR_ENTRY_TYPE_MIDI:
		m_handler(result::MIDI, m_current_directory, m_current_file);
		break;

	case SELECTOR_ENTRY_TYPE_CREATE:
		m_handler(result::CREATE, m_current_directory, m_current_file);
		break;

	case SELECTOR_ENTRY_TYPE_SOFTWARE_LIST:
		m_handler(result::SOFTLIST, m_current_directory, m_current_file);
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
	m_filename.clear();
	m_path_layout.reset();
	m_clicked_directory = std::make_pair(std::string::npos, std::string::npos);
	reset(reset_options::SELECT_FIRST);
		break;

	case SELECTOR_ENTRY_TYPE_FILE:
		// file
		m_current_file.assign(entry.fullpath);
		m_handler(result::FILE, m_current_directory, m_current_file);
		break;
	}
}


//-------------------------------------------------
//  update_search
//-------------------------------------------------

void menu_file_selector::update_search()
{
	ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename);
	reset(reset_options::REMEMBER_POSITION);
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
//  extension_matches - check if filename has one
//  of the configured extensions
//-------------------------------------------------

bool menu_file_selector::extension_matches(std::string_view filename) const
{
	if (m_file_extensions.empty())
		return true;

	auto const dot = filename.rfind('.');
	if (dot == std::string_view::npos)
		return false;

	std::string_view const ext = filename.substr(dot + 1);

	std::string_view exts(m_file_extensions);
	while (!exts.empty())
	{
		auto const comma = exts.find(',');
		std::string_view const cur = exts.substr(0, comma);
		if (cur.length() == ext.length())
		{
			bool match = true;
			for (size_t i = 0; i < ext.length(); ++i)
			{
				if (std::tolower(static_cast<unsigned char>(cur[i])) != std::tolower(static_cast<unsigned char>(ext[i])))
				{
					match = false;
					break;
				}
			}
			if (match)
				return true;
		}
		if (comma == std::string_view::npos)
			break;
		exts.remove_prefix(comma + 1);
	}

	return false;
}

//-------------------------------------------------
//  is_archive - check if filename is a known
//  archive type (zip, imz, 7z)
//-------------------------------------------------

bool menu_file_selector::is_archive(std::string_view filename)
{
	return core_filename_ends_with(filename, ".zip") ||
		core_filename_ends_with(filename, ".imz") ||
		core_filename_ends_with(filename, ".7z");
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_file_selector::populate()
{
	std::size_t selected_index = 0;
	bool have_selection = false;

	// clear out the menu entries
	m_entrylist.clear();

	// set heading - centered, padded symmetrically to match breadcrumb width
	std::string heading_text = m_filename.empty()
		? std::string(_("File Manager"))
		: util::string_format(_("Search: %1$s_"), m_filename);
	float const heading_w = get_string_width(heading_text);
	float const path_w = get_string_width(m_current_directory);
	if (heading_w < path_w)
	{
		float const space_w = get_string_width(" ");
		if (space_w > 0.0F)
		{
			int const padding = int((path_w - heading_w) / space_w / 2) + 1;
			std::string const pad(padding, ' ');
			heading_text = pad + heading_text + pad;
		}
	}
	set_heading(std::move(heading_text));

	// open the directory
	util::zippath_directory::ptr directory;
	std::error_condition const err = util::zippath_directory::open(m_current_directory, directory);

	// add the "[empty slot]" entry if available
	if (m_has_empty)
		append_entry(SELECTOR_ENTRY_TYPE_EMPTY, "", "");

	// add the "[midi port]" entry if available
	if (m_is_midi)
		append_entry(SELECTOR_ENTRY_TYPE_MIDI, "", "");

	// add the "[create]" entry
	if (m_has_create && directory && !directory->is_archive())
		append_entry(SELECTOR_ENTRY_TYPE_CREATE, "", "");

	// add the "[software list]" entry if available
	if (m_has_softlist)
		append_entry(SELECTOR_ENTRY_TYPE_SOFTWARE_LIST, "", "");

	// add the drives
	for (std::string const &volume_name : osd_get_volume_names())
		append_entry(SELECTOR_ENTRY_TYPE_DRIVE, volume_name, volume_name);

	// add the user's home directory
	char const *home = osd_getenv("HOME");
	if (!home || !*home)
		home = osd_getenv("USERPROFILE");
	if (home && *home)
		append_entry(SELECTOR_ENTRY_TYPE_DIRECTORY, home, home);

	// add software path entries from swpath option (semicolon-separated)
	// paths may be relative - resolve to full path so navigation works
	std::string const swpath(machine().options().sw_path());
	if (!swpath.empty())
	{
		path_iterator iter(swpath);
		std::string path;
		while (iter.next(path))
		{
			if (!path.empty())
			{
				std::string fullpath;
				if (!osd_get_full_path(fullpath, path))
					append_entry(SELECTOR_ENTRY_TYPE_DIRECTORY, fullpath, fullpath);
			}
		}
	}

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
			bool const is_parent = !strcmp(dirent->name, "..");
			if (!m_filename.empty() && !is_parent)
			{
				if (subsequence_rank(dirent->name, m_filename) == 0)
					continue;
			}
			append_dirent_entry(dirent);
		}
	}

	if (m_entrylist.size() > first)
	{
		// sort the menu entries - by search rank when filtering, else alphabetical
		std::locale const lcl;
		std::collate<wchar_t> const &coll = std::use_facet<std::collate<wchar_t> >(lcl);
		auto const alpha_less = [&coll] (file_selector_entry const &x, file_selector_entry const &y)
		{
			std::wstring const xstr = wstring_from_utf8(x.basename);
			std::wstring const ystr = wstring_from_utf8(y.basename);
			return coll.compare(xstr.data(), xstr.data() + xstr.size(), ystr.data(), ystr.data() + ystr.size()) < 0;
		};

		if (m_filename.empty())
		{
			std::sort(m_entrylist.begin() + first, m_entrylist.end(), alpha_less);
		}
		else
		{
			// rank 1 = prefix (best), 2 = subsequence; parent ".." always first
			std::stable_sort(
					m_entrylist.begin() + first,
					m_entrylist.end(),
					[this, &coll] (file_selector_entry const &x, file_selector_entry const &y)
					{
						bool const x_parent = (x.basename == "..");
						bool const y_parent = (y.basename == "..");
						if (x_parent != y_parent)
							return x_parent;
						if (x_parent)
							return false;
						int const xr = subsequence_rank(x.basename, m_filename);
						int const yr = subsequence_rank(y.basename, m_filename);
						if (xr != yr)
							return xr < yr;
						// same rank: alphabetical
						std::wstring const xs = wstring_from_utf8(x.basename);
						std::wstring const ys = wstring_from_utf8(y.basename);
						return coll.compare(xs.data(), xs.data() + xs.size(), ys.data(), ys.data() + ys.size()) < 0;
					});
		}
	}

	// resolve selection after sort - find first non-parent directory item
	// or the matching current file
	if (m_entrylist.size() >= first)
	{
		for (std::size_t i = first - 1; i < m_entrylist.size(); i++)
		{
			file_selector_entry const &e = m_entrylist[i];
			bool const is_parent = (e.basename == "..");

			if (!have_selection && !is_parent)
			{
				selected_index = i;
				have_selection = true;
			}

			if (!m_current_file.empty() && !core_stricmp(m_current_file.c_str(), e.basename.c_str()))
			{
				selected_index = i;
				have_selection = true;
				break;
			}
		}
	}

	// fallback: select software list entry if no directory item was selected
	if (!have_selection && m_has_softlist)
	{
		for (std::size_t i = 0; i < m_entrylist.size(); i++)
		{
			if (m_entrylist[i].type == SELECTOR_ENTRY_TYPE_SOFTWARE_LIST)
			{
				selected_index = i;
				have_selection = true;
				break;
			}
		}
	}

	// append all of the menu entries
	bool const have_directory_items = (m_entrylist.size() >= first);
	bool separator_inserted = false;
	for (std::size_t i = 0; i < m_entrylist.size(); i++)
	{
		if (have_directory_items && !separator_inserted && (i + 1 >= first))
		{
			item_append(menu_item_type::SEPARATOR);
			separator_inserted = true;
		}
		append_entry_menu_item(&m_entrylist[i]);
	}
	item_append(menu_item_type::SEPARATOR);

	// set the selection (if we have one) - resolve index to valid pointer
	if (have_selection && selected_index < m_entrylist.size())
		set_selection((void *)&m_entrylist[selected_index]);
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
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
	}
	else if (ev->iptkey == IPT_UI_CLEAR)
	{
		if (!m_file_extensions.empty())
		{
			m_filter_extensions = !m_filter_extensions;
			if (m_filter_extensions)
			{
				ui().popup_time(3, "%s", util::string_format(
					"%s\n%s",
					util::string_format(_("Filter: %s"), _("on")),
					util::string_format(_("Extensions: %s"), m_file_extensions)));
			}
			else
			{
				ui().popup_time(3, "%s", util::string_format(_("Filter: %s"), _("off")));
			}
			reset(reset_options::REMEMBER_POSITION);
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
		// reset search when selecting an item
		m_filename.clear();

		select_item(*reinterpret_cast<file_selector_entry const *>(ev->itemref));
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
		render_target &target,
		bool can_in_place,
		handler_function &&handler)
	: menu(mui, target)
	, m_handler(std::move(handler))
	, m_can_in_place(can_in_place)
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
	set_heading(_("Select access mode"));

	item_append(_("Read-only"), 0, itemref_from_result(result::READONLY));
	if (m_can_in_place)
		item_append(_("Read-write"), 0, itemref_from_result(result::READWRITE));
	item_append(_("Read this image, write to another image"), 0, itemref_from_result(result::WRITE_OTHER));
	//item_append(_("Read this image, write to diff"), 0, itemref_from_result(result::WRITE_DIFF));
	item_append(menu_item_type::SEPARATOR);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_select_rw::handle(event const *ev)
{
	if (ev && ev->iptkey == IPT_UI_SELECT)
		m_handler(result_from_itemref(ev->itemref));

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
