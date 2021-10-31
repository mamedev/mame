// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/swlist.cpp

    Internal MAME user interface for software list.

 *********************************************************************/

#include "emu.h"

#include "ui/ui.h"
#include "ui/swlist.h"
#include "ui/utils.h"

#include "corestr.h"
#include "softlist_dev.h"

#include <locale>


namespace ui {

/***************************************************************************
    CONSTANTS
***************************************************************************/

// time (in seconds) to display errors
#define ERROR_MESSAGE_TIME      5

// item reference for "Switch Item Ordering"
#define ITEMREF_SWITCH_ITEM_ORDERING    ((void *)1)


/***************************************************************************
    SOFTWARE PARTS
***************************************************************************/

//-------------------------------------------------
//  is_valid_softlist_part_char - returns whether
//  this character is a valid char for a softlist
//  part
//-------------------------------------------------

static bool is_valid_softlist_part_char(char32_t ch)
{
	return (ch == (char)ch) && isalnum(ch);
}


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_software_parts::menu_software_parts(mame_ui_manager &mui, render_container &container, const software_info *info, const char *interface, const software_part **part, bool other_opt, result &result)
	: menu(mui, container),
		m_result(result)
{
	m_info = info;
	m_interface = interface;
	m_selected_part = part;
	m_other_opt = other_opt;
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

void menu_software_parts::populate(float &customtop, float &custombottom)
{
	m_entries.clear();

	if (m_other_opt)
	{
		software_part_menu_entry &entry1(*m_entries.emplace(m_entries.end()));
		entry1.type = result::EMPTY;
		entry1.part = nullptr;
		item_append(_("[empty slot]"), 0, &entry1);

		software_part_menu_entry &entry2(*m_entries.emplace(m_entries.end()));
		entry2.type = result::FMGR;
		entry2.part = nullptr;
		item_append(_("[file manager]"), 0, &entry2);


		software_part_menu_entry &entry3(*m_entries.emplace(m_entries.end()));
		entry3.type = result::SWLIST;
		entry3.part = nullptr;
		item_append(_("[software list]"), 0, &entry3);
	}

	for (const software_part &swpart : m_info->parts())
	{
		if (swpart.matches_interface(m_interface))
		{
			software_part_menu_entry &entry(*m_entries.emplace(m_entries.end()));
			// check if the available parts have specific part_id to be displayed (e.g. "Map Disc", "Bonus Disc", etc.)
			// if not, we simply display "part_name"; if yes we display "part_name (part_id)"
			std::string menu_part_name(swpart.name());
			if (swpart.feature("part_id") != nullptr)
				menu_part_name.append(" (").append(swpart.feature("part_id")).append(")");
			entry.type = result::ENTRY;
			entry.part = &swpart;
			item_append(m_info->shortname(), menu_part_name, 0, &entry);
		}
	}

	item_append(menu_item_type::SEPARATOR);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_software_parts::handle(event const *ev)
{
	// process the menu
	if (ev && (ev->iptkey == IPT_UI_SELECT) && ev->itemref)
	{
		software_part_menu_entry *entry = (software_part_menu_entry *)ev->itemref;
		m_result = entry->type;
		*m_selected_part = entry->part;
		stack_pop();
	}
}


/***************************************************************************
    SOFTWARE LIST
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_software_list::menu_software_list(mame_ui_manager &mui, render_container &container, software_list_device *swlist, const char *interface, std::string &result)
	: menu(mui, container), m_result(result)
{
	m_swlist = swlist;
	m_interface = interface;
	m_ordered_by_shortname = false;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_software_list::~menu_software_list()
{
}


//-------------------------------------------------
//  append_software_entry - populate a specific list
//-------------------------------------------------

void menu_software_list::append_software_entry(const software_info &swinfo)
{
	entry_info entry;
	bool entry_updated = false;

	// check if at least one of the parts has the correct interface and add a menu entry only in this case
	for (const software_part &swpart : swinfo.parts())
	{
		if (swpart.matches_interface(m_interface) && m_swlist->is_compatible(swpart) == SOFTWARE_IS_COMPATIBLE)
		{
			entry_updated = true;
			entry.short_name.assign(swinfo.shortname());
			entry.long_name.assign(swinfo.longname());
			break;
		}
	}

	// skip this if no new entry has been allocated (e.g. if the software has no matching interface for this image device)
	if (entry_updated)
		m_entrylist.emplace_back(std::move(entry));
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_software_list::populate(float &customtop, float &custombottom)
{
	// build up the list of entries for the menu
	if (m_entrylist.empty())
		for (const software_info &swinfo : m_swlist->get_info())
			append_software_entry(swinfo);

	if (m_ordered_by_shortname)
	{
		// short names are restricted to lowercase ASCII anyway, a dumb compare works
		m_entrylist.sort([] (entry_info const &e1, entry_info const &e2) { return e1.short_name < e2.short_name; });
	}
	else
	{
		std::collate<wchar_t> const &coll = std::use_facet<std::collate<wchar_t>>(std::locale());
		m_entrylist.sort(
				[&coll] (entry_info const &e1, entry_info const &e2) -> bool
				{
					std::wstring const xstr = wstring_from_utf8(e1.long_name);
					std::wstring const ystr = wstring_from_utf8(e2.long_name);
					auto const cmp = coll.compare(xstr.data(), xstr.data() + xstr.size(), ystr.data(), ystr.data() + ystr.size());
					if (cmp)
						return cmp < 0;
					else
						return e1.short_name < e2.short_name;
				});
	}

	// add an entry to change ordering
	item_append(_("Switch Item Ordering"), 0, ITEMREF_SWITCH_ITEM_ORDERING);

	// append all of the menu entries
	for (auto &entry : m_entrylist)
		item_append(entry.long_name, entry.short_name, 0, &entry);

	item_append(menu_item_type::SEPARATOR);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_software_list::handle(event const *ev)
{
	// process the menu
	if (ev)
	{
		if (ev->iptkey == IPT_UI_SELECT)
		{
			if (ev->itemref == ITEMREF_SWITCH_ITEM_ORDERING)
			{
				m_ordered_by_shortname = !m_ordered_by_shortname;

				// reset the char buffer if we change ordering criterion
				m_search.clear();

				// reload the menu with the new order
				reset(reset_options::REMEMBER_REF);
				machine().popmessage(
						m_ordered_by_shortname
							? _("Switched Order: entries now ordered by shortname")
							: _("Switched Order: entries now ordered by description"));
			}
			else if (ev->itemref)
			{
				// handle selections
				entry_info *info = (entry_info *)ev->itemref;
				m_result = info->short_name;
				stack_pop();
			}
		}
		else if (ev->iptkey == IPT_SPECIAL)
		{
			if (input_character(m_search, ev->unichar, m_ordered_by_shortname ? is_valid_softlist_part_char : [] (char32_t ch) { return true; }))
			{
				// display the popup
				ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_search);

				// identify the selected entry
				entry_info const *const cur_selected = (uintptr_t(ev->itemref) != 1)
						? reinterpret_cast<entry_info const *>(get_selection_ref())
						: nullptr;

				// if it's a perfect match for the current selection, don't move it
				if (!cur_selected || core_strnicmp((m_ordered_by_shortname ? cur_selected->short_name : cur_selected->long_name).c_str(), m_search.c_str(), m_search.size()))
				{
					std::string::size_type bestmatch(0);
					entry_info const *selected_entry(cur_selected);
					for (auto &entry : m_entrylist)
					{
						// TODO: more efficient "common prefix" code
						auto const &compare_name = m_ordered_by_shortname ? entry.short_name : entry.long_name;
						std::string::size_type match(0);
						for (std::string::size_type i = 1; m_search.size() >= i; ++i)
						{
							if (!core_strnicmp(compare_name.c_str(), m_search.c_str(), i))
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
		else if (ev->iptkey == IPT_UI_CANCEL)
		{
			// reset the char buffer also in this case
			if (!m_search.empty())
			{
				m_search.clear();
				ui().popup_time(ERROR_MESSAGE_TIME, "%s", m_search);
			}
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

menu_software::menu_software(mame_ui_manager &mui, render_container &container, const char *interface, software_list_device **result)
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

void menu_software::populate(float &customtop, float &custombottom)
{
	bool have_compatible = false;

	// Add original software lists for this system
	software_list_device_enumerator iter(machine().config().root_device());
	for (software_list_device &swlistdev : iter)
		if (swlistdev.is_original())
			if (!swlistdev.get_info().empty() && m_interface != nullptr)
			{
				bool found = false;
				for (const software_info &swinfo : swlistdev.get_info())
					for (const software_part &swpart : swinfo.parts())
						if (swpart.matches_interface(m_interface))
						{
							found = true;
							break;
						}
				if (found)
					item_append(swlistdev.description(), 0, (void *)&swlistdev);
			}

	// add compatible software lists for this system
	for (software_list_device &swlistdev : iter)
		if (swlistdev.is_compatible())
			if (!swlistdev.get_info().empty() && m_interface != nullptr)
			{
				bool found = false;
				for (const software_info &swinfo : swlistdev.get_info())
					for (const software_part &swpart : swinfo.parts())
						if (swpart.matches_interface(m_interface))
						{
							found = true;
							break;
						}
				if (found)
				{
					if (!have_compatible)
						item_append(_("[compatible lists]"), FLAG_DISABLE, nullptr);
					item_append(swlistdev.description(), 0, (void *)&swlistdev);
				}
				have_compatible = true;
			}

	item_append(menu_item_type::SEPARATOR);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_software::handle(event const *ev)
{
	// process the menu
	if (ev && (ev->iptkey == IPT_UI_SELECT))
	{
		//menu::stack_push<menu_software_list>(ui(), container(), (software_list_config *)ev->itemref, image);
		*m_result = reinterpret_cast<software_list_device *>(ev->itemref);
		stack_pop();
	}
}

} // namespace ui
