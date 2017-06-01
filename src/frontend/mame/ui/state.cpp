// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

	ui/state.cpp

	Menus for saving and loading state

***************************************************************************/

#include "emu.h"
#include "machine.h"
#include "emuopts.h"
#include "ui/state.h"


/***************************************************************************
	ANONYMOUS NAMESPACE
***************************************************************************/

namespace {

//-------------------------------------------------
//  is_valid_state_char - is the specified character
//	a valid state filename character?
//-------------------------------------------------

bool is_valid_state_char(char32_t ch)
{
	return uchar_is_printable(ch) && osd_is_valid_filename_char(ch);
}


//-------------------------------------------------
//  get_entry_char
//-------------------------------------------------

char32_t get_entry_char(const osd::directory::entry &entry)
{
	char32_t result = 0;

	// first, is this a file that ends with *.sta?
	if (entry.type == osd::directory::entry::entry_type::FILE
		&& core_filename_ends_with(entry.name, ".sta"))
	{
		std::string basename = core_filename_extract_base(entry.name);


		char32_t ch;
		if (uchar_from_utf8(&ch, basename.c_str(), basename.length() == basename.length()) && is_valid_state_char(ch))			
			result = ch;
	}
	return result;
}


};

namespace ui {

/***************************************************************************
	FILE ENTRY
***************************************************************************/

char32_t menu_load_save_state_base::s_last_file_selected;

//-------------------------------------------------
//  file_entry ctor
//-------------------------------------------------

menu_load_save_state_base::file_entry::file_entry(char32_t entry_char, const std::chrono::system_clock::time_point &last_modified)
	: m_entry_char(entry_char)
	, m_last_modified(last_modified)
{
}


/***************************************************************************
	BASE CLASS FOR LOAD AND SAVE
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_load_save_state_base::menu_load_save_state_base(mame_ui_manager &mui, render_container &container, const char *header, const char *footer, bool must_exist)
	: menu(mui, container)
	, m_header(header)
	, m_footer(footer)
	, m_must_exist(must_exist)
	, m_was_paused(false)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_load_save_state_base::~menu_load_save_state_base()
{
	// resume if appropriate (is the destructor really the right place
	// to do this sort of activity?)
	if (!m_was_paused)
		machine().resume();
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_load_save_state_base::populate(float &customtop, float &custombottom)
{
	// open the state directory
	std::string statedir = state_directory();
	osd::directory::ptr dir = osd::directory::open(statedir);

	// create a separate vector, so we can add sorted entries to the menu
	std::vector<const file_entry *> m_entries_vec;

	// populate all file entries
	m_file_entries.clear();
	if (dir)
	{
		const osd::directory::entry *entry;
		while ((entry = dir->read()) != nullptr)
		{
			char32_t entry_char = get_entry_char(*entry);
			if (entry_char)
			{
				if (core_filename_ends_with(entry->name, ".sta"))
				{
					file_entry fileent(entry_char, entry->last_modified);
					auto iter = m_file_entries.emplace(std::make_pair(entry_char, std::move(fileent))).first;
					m_entries_vec.push_back(&iter->second);
				}
			}
		}
	}

	// sort the vector; put recently modified state files at the top
	std::sort(
		m_entries_vec.begin(),
		m_entries_vec.end(),
		[this](const file_entry *a, const file_entry *b)
		{
			return a->last_modified() > b->last_modified();
		});

	// add the entries
	for (const file_entry *entry : m_entries_vec)
	{
		// get the time as a local time string
		char time_string[128];
		auto last_modified_time_t = std::chrono::system_clock::to_time_t(entry->last_modified());
		std::strftime(time_string, sizeof(time_string), "%c", std::localtime(&last_modified_time_t));

		// format the text
		std::string text = util::string_format("%s: %s",
			utf8_from_uchar(entry->entry_char()),
			time_string);

		// append the menu item
		void *itemref = itemref_from_file_entry(*entry);
		item_append(std::move(text), std::string(), 0, itemref);

		// is this item selected?
		if (entry->entry_char() == s_last_file_selected)
			set_selection(itemref);
	}

	// set up custom render proc
	customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	custombottom = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;

	// pause if appropriate
	m_was_paused = machine().paused();
	if (!m_was_paused)
		machine().pause();
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_load_save_state_base::handle()
{
	// process the menu
	const event *event = process(0);

	// process the event
	if ((event != nullptr) && (event->iptkey == IPT_UI_SELECT))
	{
		// user selected one of the entries
		const file_entry &entry = file_entry_from_itemref(event->itemref);
		slot_selected(entry.entry_char());
	}
	else if ((event != nullptr) && (event->iptkey == IPT_SPECIAL)
		&& is_valid_state_char(event->unichar)
		&& (!m_must_exist || is_present(event->unichar)))
	{
		// user pressed a shortcut key
		slot_selected(event->unichar);
	}
}


//-------------------------------------------------
//  slot_selected
//-------------------------------------------------

void menu_load_save_state_base::slot_selected(char32_t entry_char)
{
	// handle it
	process_file(utf8_from_uchar(entry_char));

	// record the last slot touched
	s_last_file_selected = entry_char;

	// no matter what, pop out
	menu::stack_pop(machine());
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_load_save_state_base::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	extra_text_render(top, bottom, origx1, origy1, origx2, origy2,
		m_header,
		m_footer);
}


//-------------------------------------------------
//  itemref_from_file_entry
//-------------------------------------------------

void *menu_load_save_state_base::itemref_from_file_entry(const menu_load_save_state_base::file_entry &entry)
{
	return (void *)&entry;
}


//-------------------------------------------------
//  file_entry_from_itemref
//-------------------------------------------------

const menu_load_save_state_base::file_entry &menu_load_save_state_base::file_entry_from_itemref(void *itemref)
{
	return *((const file_entry *)itemref);
}


//-------------------------------------------------
//  is_present
//-------------------------------------------------

std::string menu_load_save_state_base::state_directory() const
{
	return util::string_format("%s%s%s",
		machine().options().state_directory(),
		PATH_SEPARATOR,
		machine().system().name);
}


//-------------------------------------------------
//  is_present
//-------------------------------------------------

bool menu_load_save_state_base::is_present(char32_t entry_char) const
{
	return m_file_entries.find(entry_char) != m_file_entries.end();
}


/***************************************************************************
	LOAD STATE
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_load_state::menu_load_state(mame_ui_manager &mui, render_container &container)
	: menu_load_save_state_base(mui, container, _("Load State"), _("Select position to load from"), true)
{
}


//-------------------------------------------------
//  process_file
//-------------------------------------------------

void menu_load_state::process_file(std::string &&file_name)
{
	machine().schedule_load(std::move(file_name));
}


/***************************************************************************
	SAVE STATE
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_save_state::menu_save_state(mame_ui_manager &mui, render_container &container)
	: menu_load_save_state_base(mui, container, _("Save State"), _("Select position to save to"), false)
{
}


//-------------------------------------------------
//  process_file
//-------------------------------------------------

void menu_save_state::process_file(std::string &&file_name)
{
	machine().schedule_save(std::move(file_name));
}


} // namespace ui
