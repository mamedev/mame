// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/swlist.cpp

    Internal MAME user interface for software list.

 *********************************************************************/

#include "emu.h"

#include "ui/ui.h"
#include "ui/swlist.h"

#include "softlist.h"


namespace ui {
/***************************************************************************
    CONSTANTS
***************************************************************************/

/* time (in seconds) to display errors */
#define ERROR_MESSAGE_TIME      5


/***************************************************************************
    SOFTWARE PARTS
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_software_parts::menu_software_parts(mame_ui_manager &mui, render_container *container, const software_info *info, const char *interface, const software_part **part, bool other_opt, int *result)
	: menu(mui, container)
{
	m_info = info;
	m_interface = interface;
	m_selected_part = part;
	m_other_opt = other_opt;
	m_result = result;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_software_parts::~menu_software_parts()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_software_parts::populate()
{
	if (m_other_opt)
	{
		software_part_menu_entry *entry1 = (software_part_menu_entry *) m_pool_alloc(sizeof(*entry1));
		entry1->type = T_EMPTY;
		entry1->part = nullptr;
		item_append(_("[empty slot]"), "", 0, entry1);

		software_part_menu_entry *entry2 = (software_part_menu_entry *) m_pool_alloc(sizeof(*entry2));
		entry2->type = T_FMGR;
		entry2->part = nullptr;
		item_append(_("[file manager]"), "", 0, entry2);


		software_part_menu_entry *entry3 = (software_part_menu_entry *) m_pool_alloc(sizeof(*entry3));
		entry3->type = T_SWLIST;
		entry3->part = nullptr;
		item_append(_("[software list]"), "", 0, entry3);
	}

	for (const software_part &swpart : m_info->parts())
	{
		if (swpart.matches_interface(m_interface))
		{
			software_part_menu_entry *entry = (software_part_menu_entry *) m_pool_alloc(sizeof(*entry));
			// check if the available parts have specific part_id to be displayed (e.g. "Map Disc", "Bonus Disc", etc.)
			// if not, we simply display "part_name"; if yes we display "part_name (part_id)"
			std::string menu_part_name(swpart.name());
			if (swpart.feature("part_id") != nullptr)
				menu_part_name.append(" (").append(swpart.feature("part_id")).append(")");
			entry->type = T_ENTRY;
			entry->part = &swpart;
			item_append(m_info->shortname(), menu_part_name, 0, entry);
		}
	}
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_software_parts::handle()
{
	// process the menu
	const event *event = process(0);

	if (event != nullptr && event->iptkey == IPT_UI_SELECT && event->itemref != nullptr)
	{
		software_part_menu_entry *entry = (software_part_menu_entry *) event->itemref;
		*m_result = entry->type;
		*m_selected_part = entry->part;
		menu::stack_pop(machine());
	}
}


/***************************************************************************
    SOFTWARE LIST
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_software_list::menu_software_list(mame_ui_manager &mui, render_container *container, software_list_device *swlist, const char *interface, std::string &result)
	: menu(mui, container), m_result(result)
{
	m_swlist = swlist;
	m_interface = interface;
	m_entrylist = nullptr;
	m_ordered_by_shortname = true;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_software_list::~menu_software_list()
{
}


//-------------------------------------------------
//  compare_entries
//-------------------------------------------------

int menu_software_list::compare_entries(const entry_info *e1, const entry_info *e2, bool shortname)
{
	int result;
	const char *e1_basename;
	const char *e2_basename;

	if (shortname)
	{
		e1_basename = (e1->short_name != nullptr) ? e1->short_name : "";
		e2_basename = (e2->short_name != nullptr) ? e2->short_name : "";
	}
	else
	{
		e1_basename = (e1->long_name != nullptr) ? e1->long_name : "";
		e2_basename = (e2->long_name != nullptr) ? e2->long_name : "";
	}

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

	return result;
}


//-------------------------------------------------
//  append_software_entry - populate a specific list
//-------------------------------------------------

menu_software_list::entry_info *menu_software_list::append_software_entry(const software_info &swinfo)
{
	entry_info *entry = nullptr;
	entry_info **entryptr;
	bool entry_updated = FALSE;

	// check if at least one of the parts has the correct interface and add a menu entry only in this case
	for (const software_part &swpart : swinfo.parts())
	{
		if (swpart.matches_interface(m_interface) && swpart.is_compatible(*m_swlist) == SOFTWARE_IS_COMPATIBLE)
		{
			entry_updated = TRUE;
			// allocate a new entry
			entry = (entry_info *) m_pool_alloc(sizeof(*entry));
			memset(entry, 0, sizeof(*entry));

			entry->short_name = pool_strdup(swinfo.shortname());
			entry->long_name = pool_strdup(swinfo.longname());
			break;
		}
	}

	// skip this if no new entry has been allocated (e.g. if the software has no matching interface for this image device)
	if (entry_updated)
	{
		// find the end of the list
		entryptr = &m_entrylist;
		while ((*entryptr != nullptr) && (compare_entries(entry, *entryptr, m_ordered_by_shortname) >= 0))
			entryptr = &(*entryptr)->next;

		// insert the entry
		entry->next = *entryptr;
		*entryptr = entry;
	}

	return entry;
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_software_list::populate()
{
	// build up the list of entries for the menu
	for (const software_info &swinfo : m_swlist->get_info())
		append_software_entry(swinfo);

	// add an entry to change ordering
	item_append(_("Switch Item Ordering"), "", 0, (void *)1);

	// append all of the menu entries
	for (entry_info *entry = m_entrylist; entry != nullptr; entry = entry->next)
		item_append(entry->short_name, entry->long_name, 0, entry);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_software_list::handle()
{
	const entry_info *entry;
	const entry_info *selected_entry = nullptr;
	int bestmatch = 0;

	// process the menu
	const event *event = process(0);

	if (event && event->itemref)
	{
		if ((FPTR)event->itemref == 1 && event->iptkey == IPT_UI_SELECT)
		{
			m_ordered_by_shortname = !m_ordered_by_shortname;
			m_entrylist = nullptr;

			// reset the char buffer if we change ordering criterion
			memset(m_filename_buffer, '\0', ARRAY_LENGTH(m_filename_buffer));

			// reload the menu with the new order
			reset(reset_options::REMEMBER_REF);
			machine().popmessage(_("Switched Order: entries now ordered by %s"), m_ordered_by_shortname ? _("shortname") : _("description"));
		}
		// handle selections
		else if (event->iptkey == IPT_UI_SELECT)
		{
			entry_info *info = (entry_info *) event->itemref;
			m_result = info->short_name;
			menu::stack_pop(machine());
		}
		else if (event->iptkey == IPT_SPECIAL)
		{
			auto const buflen = std::strlen(m_filename_buffer);
			bool update_selected = false;

			if ((event->unichar == 8) || (event->unichar == 0x7f))
			{
				// if it's a backspace and we can handle it, do so
				if (0 < buflen)
				{
					*const_cast<char *>(utf8_previous_char(&m_filename_buffer[buflen])) = 0;
					update_selected = true;

					ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename_buffer);
				}
			}
			else if (event->is_char_printable())
			{
				// if it's any other key and we're not maxed out, update
				if (event->append_char(m_filename_buffer, buflen))
				{
					update_selected = true;

					ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_filename_buffer);
				}
			}

			if (update_selected)
			{
				const entry_info *cur_selected;

				// if the current selection is a software entry, start search from here
				if ((FPTR)event->itemref != 1)
					cur_selected= (const entry_info *)get_selection();
				// else (if we are on the 'Switch Order' entry) start from the beginning
				else
					cur_selected = m_entrylist;

				// check for entries which matches our filename_buffer:
				// from current entry to the end
				for (entry = cur_selected; entry != nullptr; entry = entry->next)
				{
					const char *compare_name = m_ordered_by_shortname ? entry->short_name : entry->long_name;

					if (compare_name != nullptr && m_filename_buffer != nullptr)
					{
						int match = 0;
						for (int i = 0; i < ARRAY_LENGTH(m_filename_buffer); i++)
						{
							if (core_strnicmp(compare_name, m_filename_buffer, i) == 0)
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
					const char *compare_name = m_ordered_by_shortname ? entry->short_name : entry->long_name;

					if (compare_name != nullptr && m_filename_buffer != nullptr)
					{
						int match = 0;
						for (int i = 0; i < ARRAY_LENGTH(m_filename_buffer); i++)
						{
							if (core_strnicmp(compare_name, m_filename_buffer, i) == 0)
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
			m_result = m_filename_buffer;
			menu::stack_pop(machine());
		}
	}
}


/***************************************************************************
    SOFTWARE MENU - list of available software lists - i.e. cartridges,
    floppies
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_software::menu_software(mame_ui_manager &mui, render_container *container, const char *interface, software_list_device **result)
	: menu(mui, container)
{
	m_interface = interface;
	m_result = result;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_software::~menu_software()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_software::populate()
{
	bool have_compatible = false;

	// Add original software lists for this system
	software_list_device_iterator iter(machine().config().root_device());
	for (software_list_device &swlistdev : iter)
		if (swlistdev.list_type() == SOFTWARE_LIST_ORIGINAL_SYSTEM)
			if (!swlistdev.get_info().empty() && m_interface != nullptr)
			{
				bool found = false;
				for (const software_info &swinfo : swlistdev.get_info())
					if (swinfo.first_part()->matches_interface(m_interface))
						found = true;
				if (found)
					item_append(swlistdev.description(), "", 0, (void *)&swlistdev);
			}

	// add compatible software lists for this system
	for (software_list_device &swlistdev : iter)
		if (swlistdev.list_type() == SOFTWARE_LIST_COMPATIBLE_SYSTEM)
			if (!swlistdev.get_info().empty() && m_interface != nullptr)
			{
				bool found = false;
				for (const software_info &swinfo : swlistdev.get_info())
					if (swinfo.first_part()->matches_interface(m_interface))
						found = true;
				if (found)
				{
					if (!have_compatible)
						item_append(_("[compatible lists]"), "", FLAG_DISABLE, nullptr);
					item_append(swlistdev.description(), "", 0, (void *)&swlistdev);
				}
				have_compatible = true;
			}
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_software::handle()
{
	// process the menu
	const event *event = process(0);

	if (event != nullptr && event->iptkey == IPT_UI_SELECT) {
		//menu::stack_push<menu_software_list>(ui(), container, (software_list_config *)event->itemref, image);
		*m_result = (software_list_device *)event->itemref;
		menu::stack_pop(machine());
	}
}

} // namespace ui
