/*********************************************************************

    uiswlist.c

    Internal MAME user interface for software list.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

 *********************************************************************/

#include "emu.h"
#include "ui.h"
#include "uimenu.h"
#include "softlist.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* time (in seconds) to display errors */
#define ERROR_MESSAGE_TIME		5


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* state of a software entry */
typedef struct _software_entry_state software_entry_state;
struct _software_entry_state
{
	software_entry_state *next;

	const char *short_name;
	const char *long_name;
	const char *interface;
	char *list_name;
	device_image_interface* image;
};

/* state of the software menu */
typedef struct _software_menu_state software_menu_state;
struct _software_menu_state
{
	char *list_name;	/* currently selected list */
	device_image_interface* image;
	software_entry_state *entrylist;
	char filename_buffer[1024];
	int ordered_by_shortname;
	int reorder;
};

/* state of a software part */
typedef struct _software_part_state software_part_state;
struct _software_part_state
{
	const char *part_name;
	const char *interface;
};


static void ui_mess_menu_populate_software_parts(running_machine &machine, ui_menu *menu, const char *swlist, const char *swinfo, const char *interface)
{
	software_list *list = software_list_open(machine.options(), swlist, FALSE, NULL);

	if (list)
	{
		software_info *info = software_list_find(list, swinfo, NULL);

		if (info)
		{
			for (software_part *swpart = software_find_part(info, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
			{
				if (strcmp(interface, swpart->interface_) == 0)
				{
					software_part_state *entry = (software_part_state *) ui_menu_pool_alloc(menu, sizeof(*entry));
					// check if the available parts have specific part_id to be displayed (e.g. "Map Disc", "Bonus Disc", etc.)
					// if not, we simply display "part_name"; if yes we display "part_name (part_id)"
					astring menu_part_name(swpart->name);
					if (software_part_get_feature(swpart, "part_id") != NULL)
					{
						menu_part_name.cat(" (");
						menu_part_name.cat(software_part_get_feature(swpart, "part_id"));
						menu_part_name.cat(")");
					}
					entry->part_name = ui_menu_pool_strdup(menu, swpart->name);	// part_name is later used to build up the filename to load, so we use swpart->name!
					entry->interface = ui_menu_pool_strdup(menu, swpart->interface_);
					ui_menu_item_append(menu, info->shortname, menu_part_name.cstr(), 0, entry);
				}
			}
		}

		software_list_close(list);
	}
}

void ui_mess_menu_software_parts(running_machine &machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;
	software_entry_state *sw_state = (software_entry_state *)state;
	const char *swlist = sw_state->list_name;
	const char *swinfo = sw_state->short_name;
	const char *interface = sw_state->interface;

	// generate list of available parts
	if (!ui_menu_populated(menu))
	{
		if (sw_state->list_name)
		{
			ui_mess_menu_populate_software_parts(machine, menu, swlist, swinfo, interface);
		}
	}

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);

	if (event != NULL && event->iptkey == IPT_UI_SELECT && event->itemref != NULL)
	{
		software_part_state *entry = (software_part_state *) event->itemref;

		// build the name for the part to be loaded
		astring temp_name(sw_state->short_name);
		temp_name.cat(":");
		temp_name.cat(entry->part_name);
		//printf("%s\n", temp_name.cstr());

		sw_state->image->load(temp_name.cstr());
	}
}

static int compare_software_entries(const software_entry_state *e1, const software_entry_state *e2, int shortname)
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

static software_entry_state *append_software_entry(ui_menu *menu, software_menu_state *menustate,
												   software_info *swinfo, char *list_name, device_image_interface* image)
{
	software_entry_state *entry = NULL;
	software_entry_state **entryptr;
	const char *interface = image->image_interface();

	// check if at least one of the parts has the correct interface and add a menu entry only in this case
	for (software_part *swpart = software_find_part(swinfo, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
	{
		if (strcmp(interface, swpart->interface_) == 0)
		{
			// allocate a new entry
			entry = (software_entry_state *) ui_menu_pool_alloc(menu, sizeof(*entry));
			memset(entry, 0, sizeof(*entry));

			entry->short_name = ui_menu_pool_strdup(menu, swinfo->shortname);
			entry->long_name = ui_menu_pool_strdup(menu, swinfo->longname);
			entry->list_name = list_name;
			entry->image = image;
			entry->interface = ui_menu_pool_strdup(menu, swpart->interface_);
			break;
		}
	}

	// find the end of the list
	entryptr = &menustate->entrylist;
	while ((*entryptr != NULL) && (compare_software_entries(entry, *entryptr, menustate->ordered_by_shortname) >= 0))
		entryptr = &(*entryptr)->next;

	// insert the entry
	entry->next = *entryptr;
	*entryptr = entry;

	return entry;
}


static void ui_mess_menu_populate_software_entries(running_machine &machine, ui_menu *menu, software_menu_state *menustate)
{
	software_list *list = software_list_open(machine.options(), menustate->list_name, FALSE, NULL);

	// build up the list of entries for the menu
	if (list)
	{
		for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
			append_software_entry(menu, menustate, swinfo, menustate->list_name, menustate->image);

		software_list_close(list);
	}

	// add an entry to change ordering
	ui_menu_item_append(menu, "Switch Item Ordering", NULL, 0, (void *)1);

	// append all of the menu entries
	for (software_entry_state *entry = menustate->entrylist; entry != NULL; entry = entry->next)
		ui_menu_item_append(menu, entry->short_name, entry->long_name, 0, entry);
}

bool swinfo_has_multiple_parts(software_info *swinfo, const char *interface)
{
	int count = 0;

	for (software_part *swpart = software_find_part(swinfo, NULL, NULL); swpart != NULL; swpart = software_part_next(swpart))
	{
		if (strcmp(interface, swpart->interface_) == 0)
			count++;
	}
	return (count > 1) ? TRUE : FALSE;
}

void ui_mess_menu_software_list(running_machine &machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;
	software_menu_state *sw_state = (software_menu_state *)state;
	const software_entry_state *entry;
	const software_entry_state *selected_entry = NULL;
	int bestmatch = 0;

	if (!ui_menu_populated(menu) || sw_state->reorder)
	{
		sw_state->reorder = 0;

		if (sw_state->list_name)
		{
			ui_mess_menu_populate_software_entries(machine, menu, sw_state);
		}
	}

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);

	if (event != NULL && event->itemref != NULL)
	{
		if ((FPTR)event->itemref == 1 && event->iptkey == IPT_UI_SELECT)
		{
			sw_state->ordered_by_shortname ^= 1;
			sw_state->reorder = 1;
			sw_state->entrylist = NULL;
			// reset the char buffer if we change ordering criterion
			memset(sw_state->filename_buffer, '\0', ARRAY_LENGTH(sw_state->filename_buffer));

			// reload the menu with the new order
			ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_REF);
			popmessage("Switched Order: entries now ordered by %s", sw_state->ordered_by_shortname ? "shortname" : "description");
		}
		/* handle selections */
		else if (event->iptkey == IPT_UI_SELECT)
		{
			device_image_interface *image = sw_state->image;
			software_entry_state *entry = (software_entry_state *) event->itemref;
			software_list *tmp_list = software_list_open(machine.options(), sw_state->list_name, FALSE, NULL);
			software_info *tmp_info = software_list_find(tmp_list, entry->short_name, NULL);

			// if the selected software has multiple parts that can be loaded, open the submenu
			if (swinfo_has_multiple_parts(tmp_info, image->image_interface()))
			{
				ui_menu *child_menu = ui_menu_alloc(machine, &machine.render().ui_container(), ui_mess_menu_software_parts, entry);
				software_entry_state *child_menustate = (software_entry_state *)ui_menu_alloc_state(child_menu, sizeof(*child_menustate), NULL);
				child_menustate->short_name = entry->short_name;
				child_menustate->interface = image->image_interface();
				child_menustate->list_name = sw_state->list_name;
				child_menustate->image = image;
				ui_menu_stack_push(child_menu);
			}
			else
			{
				// otherwise, load the file
				image->load(entry->short_name);
			}
			software_list_close(tmp_list);

			// reset the char buffer when pressing IPT_UI_SELECT
			if (sw_state->filename_buffer[0] != '\0')
				memset(sw_state->filename_buffer, '\0', ARRAY_LENGTH(sw_state->filename_buffer));
		}
		else if (event->iptkey == IPT_SPECIAL)
		{
			int buflen = strlen(sw_state->filename_buffer);
			bool update_selected = FALSE;

			/* if it's a backspace and we can handle it, do so */
			if ((event->unichar == 8 || event->unichar == 0x7f) && buflen > 0)
			{
				*(char *)utf8_previous_char(&sw_state->filename_buffer[buflen]) = 0;
				update_selected = TRUE;

				if (ARRAY_LENGTH(sw_state->filename_buffer) > 0)
					ui_popup_time(ERROR_MESSAGE_TIME, "%s", sw_state->filename_buffer);
			}
			/* if it's any other key and we're not maxed out, update */
			else if (event->unichar >= ' ' && event->unichar < 0x7f)
			{
				buflen += utf8_from_uchar(&sw_state->filename_buffer[buflen], ARRAY_LENGTH(sw_state->filename_buffer) - buflen, event->unichar);
				sw_state->filename_buffer[buflen] = 0;
				update_selected = TRUE;

				if (ARRAY_LENGTH(sw_state->filename_buffer) > 0)
					ui_popup_time(ERROR_MESSAGE_TIME, "%s", sw_state->filename_buffer);
			}

			if (update_selected)
			{
				const software_entry_state *cur_selected;

				// if the current selection is a software entry, start search from here
				if ((FPTR)event->itemref != 1)
					cur_selected= (const software_entry_state *)ui_menu_get_selection(menu);
				// else (if we are on the 'Switch Order' entry) start from the beginning
				else
					cur_selected= sw_state->entrylist;

				// check for entries which matches our filename_buffer:
				// from current entry to the end
				for (entry = cur_selected; entry != NULL; entry = entry->next)
				{
					const char *compare_name = sw_state->ordered_by_shortname ? entry->short_name : entry->long_name;

					if (compare_name != NULL && sw_state->filename_buffer != NULL)
					{
						int match = 0;
						for (int i = 0; i < ARRAY_LENGTH(sw_state->filename_buffer); i++)
						{
							if (mame_strnicmp(compare_name, sw_state->filename_buffer, i) == 0)
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
				for (entry = sw_state->entrylist; entry != cur_selected; entry = entry->next)
				{
					const char *compare_name = sw_state->ordered_by_shortname ? entry->short_name : entry->long_name;

					if (compare_name != NULL && sw_state->filename_buffer != NULL)
					{
						int match = 0;
						for (int i = 0; i < ARRAY_LENGTH(sw_state->filename_buffer); i++)
						{
							if (mame_strnicmp(compare_name, sw_state->filename_buffer, i) == 0)
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
					ui_menu_set_selection(menu, (void *) selected_entry);
			}
		}
		else if (event->iptkey == IPT_UI_CANCEL)
		{
			// reset the char buffer also in this case
			if (sw_state->filename_buffer[0] != '\0')
				memset(sw_state->filename_buffer, '\0', ARRAY_LENGTH(sw_state->filename_buffer));
		}
	}
}

/* list of available software lists - i.e. cartridges, floppies */
static void ui_mess_menu_populate_software_list(running_machine &machine, ui_menu *menu, device_image_interface* image)
{
	bool haveCompatible = FALSE;
	const char *interface = image->image_interface();

	// Add original software lists for this system
	for (const device_t *dev = machine.config().devicelist().first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
	{
		software_list_config *swlist = (software_list_config *)downcast<const legacy_device_base *>(dev)->inline_config();

		for (int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++)
		{
			if (swlist->list_name[i] && (swlist->list_type == SOFTWARE_LIST_ORIGINAL_SYSTEM))
			{
				software_list *list = software_list_open(machine.options(), swlist->list_name[i], FALSE, NULL);

				if (list)
				{
					bool found = FALSE;
					for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
					{
						software_part *part = software_find_part(swinfo, NULL, NULL);
						if (strcmp(interface,part->interface_)==0) {
							found = TRUE;
						}
					}
					if (found) {
						ui_menu_item_append(menu, list->description, NULL, 0, swlist->list_name[i]);
					}

					software_list_close(list);
				}
			}
		}
	}

	// Add compatible software lists for this system
	for (const device_t *dev = machine.config().devicelist().first(SOFTWARE_LIST); dev != NULL; dev = dev->typenext())
	{
		software_list_config *swlist = (software_list_config *)downcast<const legacy_device_base *>(dev)->inline_config();

		for (int i = 0; i < DEVINFO_STR_SWLIST_MAX - DEVINFO_STR_SWLIST_0; i++)
		{
			if (swlist->list_name[i] && (swlist->list_type == SOFTWARE_LIST_COMPATIBLE_SYSTEM))
			{
				software_list *list = software_list_open(machine.options(), swlist->list_name[i], FALSE, NULL);

				if (list)
				{
					bool found = FALSE;
					for (software_info *swinfo = software_list_find(list, "*", NULL); swinfo != NULL; swinfo = software_list_find(list, "*", swinfo))
					{
						software_part *part = software_find_part(swinfo, NULL, NULL);
						if (strcmp(interface,part->interface_)==0) {
							found = TRUE;
						}
					}
					if (found) {
						if (!haveCompatible) {
							ui_menu_item_append(menu, "[compatible lists]", NULL, MENU_FLAG_DISABLE, NULL);
						}
						ui_menu_item_append(menu, list->description, NULL, 0, swlist->list_name[i]);
					}

					haveCompatible = TRUE;
					software_list_close(list);
				}
			}
		}
	}

}

void ui_image_menu_software(running_machine &machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;
	device_image_interface* image = (device_image_interface*)parameter;
	if (!ui_menu_populated(menu))
		ui_mess_menu_populate_software_list(machine, menu, image);

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);

	if (event != NULL && event->iptkey == IPT_UI_SELECT)
	{
		ui_menu *child_menu = ui_menu_alloc(machine, &machine.render().ui_container(), ui_mess_menu_software_list, NULL);
		software_menu_state *child_menustate = (software_menu_state *)ui_menu_alloc_state(child_menu, sizeof(*child_menustate), NULL);
		child_menustate->list_name = (char *)event->itemref;
		child_menustate->image = image;
		child_menustate->entrylist = NULL;
		child_menustate->ordered_by_shortname = 1;
		ui_menu_stack_push(child_menu);
	}
}
