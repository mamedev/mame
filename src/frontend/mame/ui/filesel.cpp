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
#include "util/path.h"

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
	, m_file_extensions(image.file_extensions() ? image.file_extensions() : "")
	, m_clicked_directory(std::string::npos, std::string::npos)
{
	set_process_flags(PROCESS_IGNOREPAUSE | PROCESS_LR_ALWAYS);
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

	set_custom_space(2.0F * line_height() + 6.0F * tb_border(), 0.0F);
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_file_selector::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// lay out breadcrumb text
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

	// compute shared width: max of breadcrumb text, shortcut text, and menu
	float const path_text_w = m_path_layout->actual_width();
	float const menu_w = origx2 - origx1;

	std::string combined;
	m_shortcut_offsets.clear();
	for (size_t i = 0; i < m_shortcuts.size(); i++)
	{
		if (i > 0)
			combined += "   ";
		size_t const start = combined.size();
		combined += m_shortcuts[i].label;
		m_shortcut_offsets.push_back({start, combined.size()});
	}
	m_shortcut_layout.emplace(create_layout());
	m_shortcut_layout->add_text(combined);
	float const sc_text_w = m_shortcuts.empty() ? 0.0F : m_shortcut_layout->actual_width();

	// all panels share the widest width
	float const panel_w = std::max({path_text_w + 2.0F * lr_border(),
	                                 sc_text_w + 2.0F * lr_border(),
	                                 menu_w});
	float const panel_x1 = 0.5F - 0.5F * panel_w;
	float const panel_x2 = panel_x1 + panel_w;

	// compute breadcrumb box
	float const path_y1 = origy1 - top;
	float const path_y2 = path_y1 + line_height() + 2.0F * tb_border();
	m_path_position.first = panel_x1 + lr_border();
	m_path_position.second = path_y1 + tb_border();

	ui().draw_outlined_box(container(), panel_x1, path_y1, panel_x2, path_y2, ui().colors().background_color());

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

	// draw the breadcrumb text
	m_path_layout->emit(container(), m_path_position.first, m_path_position.second);

	// draw shortcut bar directly below breadcrumb box
	if (!m_shortcuts.empty())
	{
		if (m_clicked_shortcut >= 0)
		{
			auto const &[off_start, off_end] = m_shortcut_offsets[m_clicked_shortcut];
			rgb_t const fg = ui().colors().selected_color();
			rgb_t const bg = ui().colors().selected_bg_color();
			m_shortcut_layout->restyle(off_start, off_end - off_start, &fg, &bg);
		}
		else if (m_shortcut_mode || m_shortcut_armed)
		{
			auto const &[off_start, off_end] = m_shortcut_offsets[m_shortcut_focus];
			rgb_t const fg = ui().colors().selected_color();
			rgb_t const bg = ui().colors().selected_bg_color();
			m_shortcut_layout->restyle(off_start, off_end - off_start, &fg, &bg);
		}

		float const sc_y1 = path_y2 + tb_border();
		float const sc_y2 = sc_y1 + line_height() + 2.0F * tb_border();
		ui().draw_outlined_box(container(), panel_x1, sc_y1, panel_x2, sc_y2, ui().colors().background_color());
		m_shortcut_position.first = panel_x1 + lr_border();
		m_shortcut_position.second = sc_y1 + tb_border();
		m_shortcut_layout->emit(container(), m_shortcut_position.first, m_shortcut_position.second);
	}
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

	// track pointer after clicking a shortcut
	if (m_clicked_shortcut >= 0)
	{
		if (ui_event::type::POINTER_ABORT == uievt.event_type)
		{
			m_clicked_shortcut = -1;
			return std::make_tuple(IPT_INVALID, false, true);
		}
		else if (uievt.pointer_released & 0x01)
		{
			auto const [x, y] = pointer_location();
			if (get_shortcut_at(x, y) == m_clicked_shortcut)
				return std::make_tuple(IPT_CUSTOM, false, true);
			m_clicked_shortcut = -1;
			return std::make_tuple(IPT_INVALID, false, true);
		}
		else if (uievt.pointer_buttons & ~u32(1))
		{
			m_clicked_shortcut = -1;
			return std::make_tuple(IPT_INVALID, false, true);
		}
		else
		{
			return std::make_tuple(IPT_INVALID, true, false);
		}
	}

	// check for clicks if we have up-to-date content on-screen
	if (pointer_idle() && (uievt.pointer_buttons & 0x01) && !(uievt.pointer_buttons & ~u32(0x01)))
	{
		auto const [x, y] = pointer_location();
		// check breadcrumb click
		if (m_path_layout)
		{
			auto const [target_dir_start, target_dir_end] = get_directory_range(x, y);
			if (target_dir_end > target_dir_start)
			{
				m_clicked_directory = std::make_pair(target_dir_start, target_dir_end);
				return std::make_tuple(IPT_INVALID, true, true);
			}
		}
		// check shortcut bar click
		if (!m_shortcuts.empty())
		{
			int const sc = get_shortcut_at(x, y);
			if (sc >= 0)
			{
				m_clicked_shortcut = sc;
				return std::make_tuple(IPT_INVALID, true, true);
			}
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
	m_clicked_shortcut = -1;
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
		if (m_filter_extensions && !m_file_extensions.empty() && strcmp(dirent->name, "..") && !is_volume_root(m_current_directory))
		{
			std::string const dir_path = util::zippath_combine(m_current_directory, dirent->name);
			if (!directory_has_matching_files(dir_path, 1))
				return nullptr;
		}
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

		case SELECTOR_ENTRY_TYPE_HOME:
			text = _("[home]");
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
	case SELECTOR_ENTRY_TYPE_HOME:
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
		m_handler(result::FILE, m_current_directory, m_current_file);
		break;
	}
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
		exts = (comma == std::string_view::npos) ? std::string_view() : exts.substr(comma + 1);
	}

	return false;
}

//-------------------------------------------------
//  is_volume_root - check if path is a drive/volume
//-------------------------------------------------

bool menu_file_selector::is_volume_root(std::string_view path) const
{
	for (std::string const &vol : osd_get_volume_names())
		if (path == vol)
			return true;
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
//  search_rank - rank filename against query
//  returns 0 = no match, 1 = prefix, 2 = subsequence
//-------------------------------------------------

int menu_file_selector::search_rank(std::string_view name, std::string_view query)
{
	if (query.empty())
		return 1;

	auto const last_sep = name.find_last_of(PATH_SEPARATOR);
	std::string_view const base = (last_sep != std::string_view::npos)
		? name.substr(last_sep + 1)
		: name;

	// prefix match (case-insensitive) -> top group
	if (base.size() >= query.size() && !core_strnicmp(base.data(), query.data(), query.size()))
		return 1;

	// subsequence match (ordered, case-insensitive) -> bottom group
	size_t qi = 0;
	for (size_t ni = 0; ni < base.size() && qi < query.size(); ++ni)
	{
		if (tolower(base[ni]) == tolower(query[qi]))
			++qi;
	}
	return (qi == query.size()) ? 2 : 0;
}

//-------------------------------------------------
//  directory_has_matching_files - check if a
//  directory contains any files matching the
//  extension filter, recursing into subdirectories
//-------------------------------------------------

bool menu_file_selector::directory_has_matching_files(std::string_view path, int depth) const
{
	if (depth > DIR_CHECK_MAX_DEPTH)
		return true; // tree deeper than MAX_DEPTH - might contain matching files

	auto dir = osd::directory::open(std::string(path));
	if (!dir)
		return false;

	while (osd::directory::entry const *const dirent = dir->read())
	{
		if (!strcmp(dirent->name, ".") || !strcmp(dirent->name, ".."))
			continue;

		if (dirent->type == osd::directory::entry::entry_type::FILE)
		{
			if (extension_matches(dirent->name))
				return true;
		}
		else if (dirent->type == osd::directory::entry::entry_type::DIR)
		{
			std::string const subpath = util::zippath_combine(std::string(path), dirent->name);
			if (directory_has_matching_files(subpath, depth + 1))
				return true;
		}
	}

	return false;
}

//-------------------------------------------------
//  update_search
//-------------------------------------------------

void menu_file_selector::update_search()
{
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
//  get_shortcut_at - hit-test shortcut bar
//  returns shortcut index or -1
//-------------------------------------------------

int menu_file_selector::get_shortcut_at(float x, float y)
{
	if (m_shortcuts.empty() || !m_shortcut_layout)
		return -1;

	size_t start = 0, span = 0;
	if (m_shortcut_layout->hit_test(x - m_shortcut_position.first, y - m_shortcut_position.second, start, span))
	{
		for (size_t i = 0; i < m_shortcut_offsets.size(); i++)
		{
			if (start >= m_shortcut_offsets[i].first && (start + span) <= m_shortcut_offsets[i].second)
				return int(i);
		}
	}
	return -1;
}


//-------------------------------------------------
//  auto_descend - skip through chains of single-
//  subdirectory folders with no relevant content
//-------------------------------------------------

bool menu_file_selector::auto_descend()
{
	bool const navigating_up = !m_prev_directory.empty() &&
		m_current_directory.length() < m_prev_directory.length();
	if (navigating_up)
		return false;

	if (is_volume_root(m_current_directory))
		return false;

	bool descended = false;
	for (;;)
	{
		auto dir = osd::directory::open(m_current_directory);
		if (!dir)
			break;

		int subdir_count = 0;
		std::string subdir_name;
		bool has_content = false;

		while (osd::directory::entry const *const dirent = dir->read())
		{
			if (!strcmp(dirent->name, ".") || !strcmp(dirent->name, ".."))
				continue;

			if (dirent->type == osd::directory::entry::entry_type::FILE)
			{
				if (!m_filter_extensions || extension_matches(dirent->name))
				{
					has_content = true;
					break;
				}
			}
			else if (dirent->type == osd::directory::entry::entry_type::DIR)
			{
				// with extension filter on, only count subdirs that have matching content
				if (m_filter_extensions && !m_file_extensions.empty())
				{
					std::string const subpath = util::zippath_combine(m_current_directory, dirent->name);
					if (!directory_has_matching_files(subpath, 1))
						continue;
				}
				subdir_count++;
				if (subdir_count == 1)
					subdir_name = dirent->name;
				else
					break;
			}
		}
		dir.reset();

		if (!has_content && subdir_count == 1)
		{
			m_current_directory = util::zippath_combine(m_current_directory, subdir_name);
			m_path_layout.reset();
			descended = true;
		}
		else
			break;
	}

	return descended;
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_file_selector::populate()
{
	const file_selector_entry *selected_entry = nullptr;

	// clear out the menu entries
	m_entrylist.clear();
	auto_descend();


	// open the directory
	util::zippath_directory::ptr directory;
	std::error_condition const err = util::zippath_directory::open(m_current_directory, directory);

	// collect special entries into shortcut bar instead of the file list
	m_shortcuts.clear();
	if (m_has_empty)
		m_shortcuts.push_back({ SELECTOR_ENTRY_TYPE_EMPTY, _("[empty slot]"), "" });
	if (m_is_midi)
		m_shortcuts.push_back({ SELECTOR_ENTRY_TYPE_MIDI, _("[MIDI port]"), "" });
	if (m_has_create && directory && !directory->is_archive())
		m_shortcuts.push_back({ SELECTOR_ENTRY_TYPE_CREATE, _("[create]"), "" });
	if (m_has_softlist)
		m_shortcuts.push_back({ SELECTOR_ENTRY_TYPE_SOFTWARE_LIST, _("[software list]"), "" });
	for (std::string const &volume_name : osd_get_volume_names())
		m_shortcuts.push_back({ SELECTOR_ENTRY_TYPE_DRIVE, volume_name, volume_name });

	char const *home = osd_getenv("HOME");
	if (!home || !*home)
		home = osd_getenv("USERPROFILE");
	if (home && *home)
		m_shortcuts.push_back({ SELECTOR_ENTRY_TYPE_HOME, std::string(_("[home]")), home });

	if (m_shortcut_focus >= int(m_shortcuts.size()))
		m_shortcut_focus = std::max(0, int(m_shortcuts.size()) - 1);

	std::string heading_text = m_filename.empty()
		? std::string(_("File Manager"))
		: util::string_format(_("Search: %1$s_"), m_filename);
	float panel_text_w = get_string_width(m_current_directory);
	std::string combined;
	for (size_t i = 0; i < m_shortcuts.size(); i++)
	{
		if (i > 0)
			combined += "   ";
		combined += m_shortcuts[i].label;
	}
	panel_text_w = std::max(panel_text_w, get_string_width(combined));
	float heading_w = get_string_width(heading_text);
	if (heading_w < panel_text_w)
	{
		float space_w = get_string_width(" ");
		if (space_w > 0.0F)
		{
			int padding = int((panel_text_w - heading_w) / space_w / 2) + 1;
			std::string pad(padding, ' ');
			heading_text = pad + heading_text + pad;
		}
	}
	set_heading(std::move(heading_text));
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

	bool const searching = !m_filename.empty();
	if (m_entrylist.size() > first)
	{
		std::locale const lcl;
		std::collate<wchar_t> const &coll = std::use_facet<std::collate<wchar_t> >(lcl);
		auto collator = [&coll] (file_selector_entry const &x, file_selector_entry const &y)
		{
			std::wstring const xstr = wstring_from_utf8(x.basename);
			std::wstring const ystr = wstring_from_utf8(y.basename);
			return coll.compare(xstr.data(), xstr.data() + xstr.size(), ystr.data(), ystr.data() + ystr.size()) < 0;
		};
		std::sort(
				m_entrylist.begin() + first,
				m_entrylist.end(),
				[&] (file_selector_entry const &x, file_selector_entry const &y)
				{
					if (searching)
					{
						int const rx = search_rank(x.basename, m_filename);
						int const ry = search_rank(y.basename, m_filename);
						if (rx != ry)
							return rx < ry;
					}
					bool const x_is_dir = (x.type == SELECTOR_ENTRY_TYPE_DIRECTORY || x.type == SELECTOR_ENTRY_TYPE_DRIVE || x.type == SELECTOR_ENTRY_TYPE_HOME);
					bool const y_is_dir = (y.type == SELECTOR_ENTRY_TYPE_DIRECTORY || y.type == SELECTOR_ENTRY_TYPE_DRIVE || y.type == SELECTOR_ENTRY_TYPE_HOME);
					if (x_is_dir != y_is_dir)
						return x_is_dir;
					return collator(x, y);
				});
	}

	// append all of the menu entries, applying search filter if active
	for (file_selector_entry const &entry : m_entrylist)
	{
		bool const is_special = (entry.type != SELECTOR_ENTRY_TYPE_FILE &&
		                         entry.type != SELECTOR_ENTRY_TYPE_DIRECTORY);
		if (searching && !is_special)
		{
			if (search_rank(entry.basename, m_filename) == 0)
				continue;
		}
		append_entry_menu_item(&entry);
	}
	item_append(menu_item_type::SEPARATOR);

	// set the selection (if we have one)
	if (selected_entry)
		set_selection((void *)selected_entry);

	m_prev_directory = m_current_directory;
}


//-------------------------------------------------
bool menu_file_selector::handle_keys(uint32_t flags, int &iptkey)
{
	if (m_shortcut_mode)
	{
		if ((iptkey == IPT_INVALID) && exclusive_input_pressed(iptkey, IPT_UI_UP, 6))
		{
			m_shortcut_mode = false;
			m_shortcut_armed = false;
			select_last_item();
			return true;
		}
		if ((iptkey == IPT_INVALID) && exclusive_input_pressed(iptkey, IPT_UI_DOWN, 6))
		{
			m_shortcut_mode = false;
			m_shortcut_armed = false;
			select_first_item();
			return true;
		}
	}
	else if (!m_shortcuts.empty() && (iptkey == IPT_INVALID))
	{
		if (is_first_selected() && exclusive_input_pressed(iptkey, IPT_UI_UP, 6))
		{
			m_shortcut_mode = true;
			m_shortcut_armed = true;
			return true;
		}
		if (is_last_selected() && exclusive_input_pressed(iptkey, IPT_UI_DOWN, 6))
		{
			m_shortcut_mode = true;
			m_shortcut_armed = true;
			return true;
		}
	}

	if (m_shortcut_armed && (iptkey == IPT_INVALID) && exclusive_input_pressed(iptkey, IPT_UI_SELECT, 0))
		return true;

	return menu::handle_keys(flags, iptkey);
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
		// if search filter is active, clear it and repopulate
		if (!m_filename.empty())
		{
			m_filename.clear();
			update_search();
			return true;
		}
	}
	else if (ev->iptkey == IPT_UI_LEFT && !m_shortcuts.empty())
	{
		m_shortcut_armed = true;
		m_shortcut_focus = (m_shortcut_focus - 1 + int(m_shortcuts.size())) % int(m_shortcuts.size());
		return true;
	}
	else if (ev->iptkey == IPT_UI_RIGHT && !m_shortcuts.empty())
	{
		m_shortcut_armed = true;
		m_shortcut_focus = (m_shortcut_focus + 1) % int(m_shortcuts.size());
		return true;
	}
	else if (ev->iptkey == IPT_UI_CLEAR)
	{
		if (!m_filename.empty())
		{
			m_filename.clear();
			reset(reset_options::REMEMBER_POSITION);
			return true;
		}
		else if (!m_file_extensions.empty())
		{
			m_filter_extensions = !m_filter_extensions;
			if (m_filter_extensions)
			{
				ui().popup_time(3, "%s", util::string_format(
					"%s\n%s\n%s",
					util::string_format(_("Filter: %s"), _("on")),
					util::string_format(_("Extensions: %s"), m_file_extensions),
					util::string_format(_("Scan depth: %d"), DIR_CHECK_MAX_DEPTH)));
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

		if (m_clicked_shortcut >= 0)
		{
			auto const &sc = m_shortcuts[m_clicked_shortcut];
			file_selector_entry fake;
			fake.type = sc.type;
			fake.basename = sc.label;
			fake.fullpath = sc.fullpath;
			m_clicked_shortcut = -1;
			m_filename.clear();
			m_shortcut_armed = false;
			m_shortcut_mode = false;
			select_item(fake);
			return true;
		}
	}
	else if (ev->iptkey == IPT_UI_SELECT)
	{
		if (m_shortcut_armed && !m_shortcuts.empty())
		{
			// activate the focused shortcut
			auto const &sc = m_shortcuts[m_shortcut_focus];
			file_selector_entry fake;
			fake.type = sc.type;
			fake.basename = sc.label;
			fake.fullpath = sc.fullpath;
			m_filename.clear();
			m_shortcut_armed = false;
			m_shortcut_mode = false;
			select_item(fake);
			return true;
		}
		else if (ev->itemref)
		{
			// reset search when selecting an item
			m_filename.clear();
			m_shortcut_armed = false;
			select_item(*reinterpret_cast<file_selector_entry const *>(ev->itemref));
			return true;
		}
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
