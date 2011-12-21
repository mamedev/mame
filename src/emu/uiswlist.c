/*********************************************************************

    uiswlist.c

    Internal MAME user interface for software list.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

 *********************************************************************/

#include "emu.h"
#include "ui.h"
#include "uimenu.h"
#include "uiswlist.h"
#include "softlist.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* time (in seconds) to display errors */
#define ERROR_MESSAGE_TIME		5


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

ui_menu_software_parts::ui_menu_software_parts(running_machine &machine, render_container *container, ui_menu_software_entry_info *_entry) : ui_menu(machine, container)
{
	entry = _entry;
}

void ui_menu_software_parts::populate()
{
	if(entry->list_name) {
		software_list *list = software_list_open(machine().options(), entry->list_name, false, NULL);

		if (list)
		{
			software_info *info = software_list_find(list, entry->short_name, NULL);

			if (info)
			{
				for (software_part *swpart = software_find_part(info, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
				{
					if (strcmp(entry->interface, swpart->interface_) == 0)
					{
						software_part_info *entry = (software_part_info *) m_pool_alloc(sizeof(*entry));
						// check if the available parts have specific part_id to be displayed (e.g. "Map Disc", "Bonus Disc", etc.)
						// if not, we simply display "part_name"; if yes we display "part_name (part_id)"
						astring menu_part_name(swpart->name);
						if (software_part_get_feature(swpart, "part_id") != NULL)
						{
							menu_part_name.cat(" (");
							menu_part_name.cat(software_part_get_feature(swpart, "part_id"));
							menu_part_name.cat(")");
						}
						entry->part_name = pool_strdup(swpart->name);	// part_name is later used to build up the filename to load, so we use swpart->name!
						entry->interface = pool_strdup(swpart->interface_);
						item_append(info->shortname, menu_part_name.cstr(), 0, entry);
					}
				}
			}

			software_list_close(list);
		}
	}
}

ui_menu_software_parts::~ui_menu_software_parts()
{
}

void ui_menu_software_parts::handle()
{
	/* process the menu */
	const ui_menu_event *event = process(0);

	if (event != NULL && event->iptkey == IPT_UI_SELECT && event->itemref != NULL)
	{
		software_part_info *pentry = (software_part_info *) event->itemref;

		// build the name for the part to be loaded
		astring temp_name(entry->short_name);
		temp_name.cat(":");
		temp_name.cat(pentry->part_name);
		//printf("%s\n", temp_name.cstr());

		entry->image->load(temp_name.cstr());
	}
}

int ui_menu_software_list::compare_entries(const ui_menu_software_entry_info *e1, const ui_menu_software_entry_info *e2, bool shortname)
{
	int result;
	const char *e1_basename;
	const char *e2_basename;

	if (shortname)
	{
		e1_basename = (e1->short_name != NULL) ? e1->short_name : "";
		e2_basename = (e2->short_name != NULL) ? e2->short_name : "";
	}
	else
	{
		e1_basename = (e1->long_name != NULL) ? e1->long_name : "";
		e2_basename = (e2->long_name != NULL) ? e2->long_name : "";
	}

	result = mame_stricmp(e1_basename, e2_basename);
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

/* populate a specific list */

ui_menu_software_entry_info *ui_menu_software_list::append_software_entry(software_info *swinfo, char *list_name, device_image_interface* image)
{
	ui_menu_software_entry_info *entry = NULL;
	ui_menu_software_entry_info **entryptr;
	const char *interface = image->image_interface();
	bool entry_updated = FALSE;

	// check if at least one of the parts has the correct interface and add a menu entry only in this case
	for (software_part *swpart = software_find_part(swinfo, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
	{
		if (strcmp(interface, swpart->interface_) == 0)
		{
			entry_updated = TRUE;
			// allocate a new entry
			entry = (ui_menu_software_entry_info *) m_pool_alloc(sizeof(*entry));
			memset(entry, 0, sizeof(*entry));

			entry->short_name = pool_strdup(swinfo->shortname);
			entry->long_name = pool_strdup(swinfo->longname);
			entry->list_name = list_name;
			entry->image = image;
			entry->interface = pool_strdup(swpart->interface_);
			break;
		}
	}

	// skip this if no new entry has been allocated (e.g. if the software has no matching interface for this image device)
	if (entry_updated)
	{
		// find the end of the list
		entryptr = &entrylist;
		while ((*entryptr != NULL) && (compare_entries(entry, *entryptr, ordered_by_shortname) >= 0))
			entryptr = &(*entryptr)->next;

		// insert the entry
		entry->next = *entryptr;
		*entryptr = entry;
	}

	return entry;
}

ui_menu_software_list::ui_menu_software_list(running_machine &machine, render_container *container, char *_list_name, device_image_interface *_image) : ui_menu(machine, container)
{
	list_name = _list_name;
	image = _image;
	entrylist = NULL;
	ordered_by_shortname = true;
}

ui_menu_software_list::~ui_menu_software_list()
{
}

void ui_menu_software_list::populate()
{
	software_list *list = software_list_open(machine().options(), list_name, false, NULL);

	// build up the list of entries for the menu
	if (list)
	{
		for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
			append_software_entry(swinfo, list_name, image);

		software_list_close(list);
	}

	// add an entry to change ordering
	item_append("Switch Item Ordering", NULL, 0, (void *)1);

	// append all of the menu entries
	for (ui_menu_software_entry_info *entry = entrylist; entry != NULL; entry = entry->next)
		item_append(entry->short_name, entry->long_name, 0, entry);
}

bool ui_menu_software_list::swinfo_has_multiple_parts(software_info *swinfo, const char *interface)
{
	int count = 0;

	for (software_part *swpart = software_find_part(swinfo, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
	{
		if (strcmp(interface, swpart->interface_) == 0)
			count++;
	}
	return (count > 1) ? true : false;
}

void ui_menu_software_list::handle()
{
	const ui_menu_software_entry_info *entry;
	const ui_menu_software_entry_info *selected_entry = NULL;
	int bestmatch = 0;

	/* process the menu */
	const ui_menu_event *event = process(0);

	if (event != NULL && event->itemref != NULL)
	{
		if ((FPTR)event->itemref == 1 && event->iptkey == IPT_UI_SELECT)
		{
			ordered_by_shortname = !ordered_by_shortname;
			entrylist = NULL;
			// reset the char buffer if we change ordering criterion
			memset(filename_buffer, '\0', ARRAY_LENGTH(filename_buffer));

			// reload the menu with the new order
			reset(UI_MENU_RESET_REMEMBER_REF);
			popmessage("Switched Order: entries now ordered by %s", ordered_by_shortname ? "shortname" : "description");
		}
		/* handle selections */
		else if (event->iptkey == IPT_UI_SELECT)
		{
			ui_menu_software_entry_info *entry = (ui_menu_software_entry_info *) event->itemref;
			software_list *tmp_list = software_list_open(machine().options(), list_name, false, NULL);
			software_info *tmp_info = software_list_find(tmp_list, entry->short_name, NULL);

			// if the selected software has multiple parts that can be loaded, open the submenu
			if (swinfo_has_multiple_parts(tmp_info, image->image_interface()))
			{
				ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_software_parts(machine(), container, entry)));
			}
			else
			{
				// otherwise, load the file
				image->load(entry->short_name);
			}
			software_list_close(tmp_list);

			// reset the char buffer when pressing IPT_UI_SELECT
			if (filename_buffer[0] != '\0')
				memset(filename_buffer, '\0', ARRAY_LENGTH(filename_buffer));
		}
		else if (event->iptkey == IPT_SPECIAL)
		{
			int buflen = strlen(filename_buffer);
			bool update_selected = false;

			/* if it's a backspace and we can handle it, do so */
			if ((event->unichar == 8 || event->unichar == 0x7f) && buflen > 0)
			{
				*(char *)utf8_previous_char(&filename_buffer[buflen]) = 0;
				update_selected = true;

				if (ARRAY_LENGTH(filename_buffer) > 0)
					ui_popup_time(ERROR_MESSAGE_TIME, "%s", filename_buffer);
			}
			/* if it's any other key and we're not maxed out, update */
			else if (event->unichar >= ' ' && event->unichar < 0x7f)
			{
				buflen += utf8_from_uchar(&filename_buffer[buflen], ARRAY_LENGTH(filename_buffer) - buflen, event->unichar);
				filename_buffer[buflen] = 0;
				update_selected = true;

				if (ARRAY_LENGTH(filename_buffer) > 0)
					ui_popup_time(ERROR_MESSAGE_TIME, "%s", filename_buffer);
			}

			if (update_selected)
			{
				const ui_menu_software_entry_info *cur_selected;

				// if the current selection is a software entry, start search from here
				if ((FPTR)event->itemref != 1)
					cur_selected= (const ui_menu_software_entry_info *)get_selection();
				// else (if we are on the 'Switch Order' entry) start from the beginning
				else
					cur_selected= entrylist;

				// check for entries which matches our filename_buffer:
				// from current entry to the end
				for (entry = cur_selected; entry != NULL; entry = entry->next)
				{
					const char *compare_name = ordered_by_shortname ? entry->short_name : entry->long_name;

					if (compare_name != NULL && filename_buffer != NULL)
					{
						int match = 0;
						for (int i = 0; i < ARRAY_LENGTH(filename_buffer); i++)
						{
							if (mame_strnicmp(compare_name, filename_buffer, i) == 0)
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
				for (entry = entrylist; entry != cur_selected; entry = entry->next)
				{
					const char *compare_name = ordered_by_shortname ? entry->short_name : entry->long_name;

					if (compare_name != NULL && filename_buffer != NULL)
					{
						int match = 0;
						for (int i = 0; i < ARRAY_LENGTH(filename_buffer); i++)
						{
							if (mame_strnicmp(compare_name, filename_buffer, i) == 0)
								match = i;
						}

						if (match > bestmatch)
						{
							bestmatch = match;
							selected_entry = entry;
						}
					}
				}

				if (selected_entry != NULL && selected_entry != cur_selected)
					set_selection((void *) selected_entry);
			}
		}
		else if (event->iptkey == IPT_UI_CANCEL)
		{
			// reset the char buffer also in this case
			if (filename_buffer[0] != '\0')
				memset(filename_buffer, '\0', ARRAY_LENGTH(filename_buffer));
		}
	}
}

/* list of available software lists - i.e. cartridges, floppies */
ui_menu_software::ui_menu_software(running_machine &machine, render_container *container, device_image_interface* _image) : ui_menu(machine, container)
{
	image = _image;
}

void ui_menu_software::populate()
{
	bool haveCompatible = false;
	const char *interface = image->image_interface();

	// Add original software lists for this system
	for (const device_t *dev = machine().config().devicelist().first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
	{
		software_list_config *swlist = (software_list_config *)downcast<const legacy_device_base *>(dev)->inline_config();

		for (int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++)
		{
			if (swlist->list_name[i] && (swlist->list_type == SOFTWARE_LIST_ORIGINAL_SYSTEM))
			{
				software_list *list = software_list_open(machine().options(), swlist->list_name[i], false, NULL);

				if (list)
				{
					bool found = false;
					for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
					{
						software_part *part = software_find_part(swinfo, NULL, NULL);
						if (strcmp(interface,part->interface_)==0) {
							found = true;
						}
					}
					if (found) {
						item_append(list->description, NULL, 0, swlist->list_name[i]);
					}

					software_list_close(list);
				}
			}
		}
	}

	// Add compatible software lists for this system
	for (const device_t *dev = machine().config().devicelist().first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
	{
		software_list_config *swlist = (software_list_config *)downcast<const legacy_device_base *>(dev)->inline_config();

		for (int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++)
		{
			if (swlist->list_name[i] && (swlist->list_type == SOFTWARE_LIST_COMPATIBLE_SYSTEM))
			{
				software_list *list = software_list_open(machine().options(), swlist->list_name[i], false, NULL);

				if (list)
				{
					bool found = false;
					for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
					{
						software_part *part = software_find_part(swinfo, NULL, NULL);
						if (strcmp(interface,part->interface_)==0) {
							found = true;
						}
					}
					if (found) {
						if (!haveCompatible) {
							item_append("[compatible lists]", NULL, MENU_FLAG_DISABLE, NULL);
						}
						item_append(list->description, NULL, 0, swlist->list_name[i]);
					}

					haveCompatible = true;
					software_list_close(list);
				}
			}
		}
	}

}

ui_menu_software::~ui_menu_software()
{
}

void ui_menu_software::handle()
{
	/* process the menu */
	const ui_menu_event *event = process(0);

	if (event != NULL && event->iptkey == IPT_UI_SELECT)
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_software_list(machine(), container, (char *)event->itemref, image)));
}
