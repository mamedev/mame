/*********************************************************************

    miscmenu.c

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "osdnet.h"
#include "emuopts.h"
#include "ui/ui.h"
#include "rendutil.h"
#include "cheat.h"
#include "uiinput.h"
#include "ui/filemngr.h"
#include "ui/miscmenu.h"
#include "audit.h"
#include "crsshair.h"
#include <ctype.h>
#include "imagedev/cassette.h"
#include "imagedev/bitbngr.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_PHYSICAL_DIPS       10
#define MAX_INPUT_PORTS         32
#define MAX_BITS_PER_PORT       32

/* DIP switch rendering parameters */
#define DIP_SWITCH_HEIGHT       0.05f
#define DIP_SWITCH_SPACING      0.01
#define SINGLE_TOGGLE_SWITCH_FIELD_WIDTH 0.025f
#define SINGLE_TOGGLE_SWITCH_WIDTH 0.020f
/* make the switch 80% of the width space and 1/2 of the switch height */
#define PERCENTAGE_OF_HALF_FIELD_USED 0.80f
#define SINGLE_TOGGLE_SWITCH_HEIGHT ((DIP_SWITCH_HEIGHT / 2) * PERCENTAGE_OF_HALF_FIELD_USED)

/*-------------------------------------------------
    ui_slider_ui_handler - pushes the slider
    menu on the stack and hands off to the
    standard menu handler
-------------------------------------------------*/

UINT32 ui_menu_sliders::ui_handler(running_machine &machine, render_container *container, UINT32 state)
{
	UINT32 result;

	/* if this is the first call, push the sliders menu */
	if (state)
		ui_menu::stack_push(auto_alloc_clear(machine, ui_menu_sliders(machine, container, true)));

	/* handle standard menus */
	result = ui_menu::ui_handler(machine, container, state);

	/* if we are cancelled, pop the sliders menu */
	if (result == UI_HANDLER_CANCEL)
		ui_menu::stack_pop(machine);

	ui_menu_sliders *uim = dynamic_cast<ui_menu_sliders *>(menu_stack);
	return uim && uim->menuless_mode ? 0 : UI_HANDLER_CANCEL;
}


/***************************************************************************
    MENU HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ui_menu_keyboard_mode - menu that
-------------------------------------------------*/

ui_menu_keyboard_mode::ui_menu_keyboard_mode(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_keyboard_mode::populate()
{
	bool natural = machine().ui().use_natural_keyboard();
	item_append("Keyboard Mode:", natural ? "Natural" : "Emulated", natural ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, NULL);
}

ui_menu_keyboard_mode::~ui_menu_keyboard_mode()
{
}

void ui_menu_keyboard_mode::handle()
{
	bool natural = machine().ui().use_natural_keyboard();

	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL)
	{
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
		{
			machine().ui().set_use_natural_keyboard(natural ^ true);
			reset(UI_MENU_RESET_REMEMBER_REF);
		}
	}
}


/*-------------------------------------------------
    ui_slot_get_current_option - returns
-------------------------------------------------*/
device_slot_option *ui_menu_slot_devices::slot_get_current_option(device_slot_interface *slot)
{
	const char *current;
	if (slot->fixed())
	{
		current = slot->default_option();
	}
	else
	{
		astring temp;
		current = machine().options().main_value(temp,slot->device().tag()+1);
	}

	return slot->option(current);
}

/*-------------------------------------------------
    ui_slot_get_current_index - returns
-------------------------------------------------*/
int ui_menu_slot_devices::slot_get_current_index(device_slot_interface *slot)
{
	const device_slot_option *current = slot_get_current_option(slot);

	if (current != NULL)
	{
		int val = 0;
		for (const device_slot_option *option = slot->first_option(); option != NULL; option = option->next())
		{
			if (option == current)
				return val;

			if (option->selectable())
				val++;
		}
	}

	return -1;
}

/*-------------------------------------------------
    ui_slot_get_length - returns
-------------------------------------------------*/
int ui_menu_slot_devices::slot_get_length(device_slot_interface *slot)
{
	const device_slot_option *current = slot_get_current_option(slot);

	int val = 0;
	for (const device_slot_option *option = slot->first_option(); option != NULL; option = option->next())
		if (option == current || option->selectable())
			val++;

	return val;
}

/*-------------------------------------------------
    ui_slot_get_next - returns
-------------------------------------------------*/
const char *ui_menu_slot_devices::slot_get_next(device_slot_interface *slot)
{
	int idx = slot_get_current_index(slot);
	if (idx < 0)
		idx = 0;
	else
		idx++;

	if (idx >= slot_get_length(slot))
		return "";

	return slot_get_option(slot, idx);
}

/*-------------------------------------------------
    ui_slot_get_prev - returns
-------------------------------------------------*/
const char *ui_menu_slot_devices::slot_get_prev(device_slot_interface *slot)
{
	int idx = slot_get_current_index(slot);
	if (idx < 0)
		idx = slot_get_length(slot) - 1;
	else
		idx--;

	if (idx < 0)
		return "";

	return slot_get_option(slot, idx);
}

/*-------------------------------------------------
    ui_slot_get_option - returns
-------------------------------------------------*/
const char *ui_menu_slot_devices::slot_get_option(device_slot_interface *slot, int index)
{
	const device_slot_option *current = slot_get_current_option(slot);

	if (index >= 0 )
	{
		int val = 0;
		for (const device_slot_option *option = slot->first_option(); option != NULL; option = option->next())
		{
			if (val == index)
				return option->name();

			if (option == current || option->selectable())
				val++;
		}
	}

	return "";
}


/*-------------------------------------------------
    ui_set_use_natural_keyboard - specifies
    whether the natural keyboard is active
-------------------------------------------------*/

void ui_menu_slot_devices::set_slot_device(device_slot_interface *slot, const char *val)
{
	astring error;
	machine().options().set_value(slot->device().tag()+1, val, OPTION_PRIORITY_CMDLINE, error);
	assert(!error);
}

/*-------------------------------------------------
    menu_slot_devices_populate - populates the main
    slot device menu
-------------------------------------------------*/

ui_menu_slot_devices::ui_menu_slot_devices(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_slot_devices::populate()
{
	/* cycle through all devices for this system */
	slot_interface_iterator iter(machine().root_device());
	for (device_slot_interface *slot = iter.first(); slot != NULL; slot = iter.next())
	{
		/* record the menu item */
		const device_slot_option *option = slot_get_current_option(slot);

		item_append(slot->device().tag()+1, option == NULL ? "------" : option->name(), (slot->fixed() || slot_get_length(slot) == 0) ? 0 : (MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW), (void *)slot);
	}
	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	item_append("Reset",  NULL, 0, NULL);
}

ui_menu_slot_devices::~ui_menu_slot_devices()
{
}

/*-------------------------------------------------
    ui_menu_slot_devices - menu that
-------------------------------------------------*/

void ui_menu_slot_devices::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT) {
			device_slot_interface *slot = (device_slot_interface *)menu_event->itemref;
			const char *val = (menu_event->iptkey == IPT_UI_LEFT) ? slot_get_prev(slot) : slot_get_next(slot);
			set_slot_device(slot,val);
			reset(UI_MENU_RESET_REMEMBER_REF);
		}
	} else if (menu_event != NULL && menu_event->iptkey == IPT_UI_SELECT) {
		machine().schedule_hard_reset();
	}
}

/*-------------------------------------------------
    ui_menu_bios_selection - populates the main
    bios selection menu
-------------------------------------------------*/

ui_menu_bios_selection::ui_menu_bios_selection(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_bios_selection::populate()
{
	/* cycle through all devices for this system */
	device_iterator deviter(machine().root_device());
	for (device_t *device = deviter.first(); device != NULL; device = deviter.next())
	{
		if (device->rom_region()) {
			const char *val = "default";
			for (const rom_entry *rom = device->rom_region(); !ROMENTRY_ISEND(rom); rom++)
			{
				if (ROMENTRY_ISSYSTEM_BIOS(rom) && ROM_GETBIOSFLAGS(rom)==device->system_bios())
				{
					val = ROM_GETHASHDATA(rom);
				}
			}
			item_append(strcmp(device->tag(),":")==0 ? "driver" : device->tag()+1, val, MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)device);
		}
	}

	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	item_append("Reset",  NULL, 0, NULL);
}

ui_menu_bios_selection::~ui_menu_bios_selection()
{
}

/*-------------------------------------------------
    ui_menu_bios_selection - menu that
-------------------------------------------------*/

void ui_menu_bios_selection::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT) {
			device_t *dev = (device_t *)menu_event->itemref;
			int cnt = 0;
			for (const rom_entry *rom = dev->rom_region(); !ROMENTRY_ISEND(rom); rom++)
			{
				if (ROMENTRY_ISSYSTEM_BIOS(rom)) cnt ++;
			}
			int val = dev->system_bios() + ((menu_event->iptkey == IPT_UI_LEFT) ? -1 : +1);
			if (val<1) val=cnt;
			if (val>cnt) val=1;
			dev->set_system_bios(val);
			if (strcmp(dev->tag(),":")==0) {
				astring error;
				machine().options().set_value("bios", val-1, OPTION_PRIORITY_CMDLINE, error);
				assert(!error);
			} else {
				astring error;
				astring value;
				astring temp;
				value.printf("%s,bios=%d",machine().options().main_value(temp,dev->owner()->tag()+1),val-1);
				machine().options().set_value(dev->owner()->tag()+1, value.cstr(), OPTION_PRIORITY_CMDLINE, error);
				assert(!error);
			}
			reset(UI_MENU_RESET_REMEMBER_REF);
		}
	} else if (menu_event != NULL && menu_event->iptkey == IPT_UI_SELECT) {
		machine().schedule_hard_reset();
	}
}

ui_menu_network_devices::ui_menu_network_devices(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

ui_menu_network_devices::~ui_menu_network_devices()
{
}

/*-------------------------------------------------
    menu_network_devices_populate - populates the main
    network device menu
-------------------------------------------------*/

void ui_menu_network_devices::populate()
{
	/* cycle through all devices for this system */
	network_interface_iterator iter(machine().root_device());
	for (device_network_interface *network = iter.first(); network != NULL; network = iter.next())
	{
		int curr = network->get_interface();
		const char *title = NULL;
		const netdev_entry_t *entry = netdev_first();
		while(entry) {
			if(entry->id==curr) {
				title = entry->description;
				break;
			}
			entry = entry->m_next;
		}

		item_append(network->device().tag(),  (title) ? title : "------", MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)network);
	}
}

/*-------------------------------------------------
    ui_menu_network_devices - menu that
-------------------------------------------------*/

void ui_menu_network_devices::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT) {
			device_network_interface *network = (device_network_interface *)menu_event->itemref;
			int curr = network->get_interface();
			if (menu_event->iptkey == IPT_UI_LEFT) curr--; else curr++;
			if (curr==-2) curr = netdev_count() - 1;
			network->set_interface(curr);
			reset(UI_MENU_RESET_REMEMBER_REF);
		}
	}
}

/*-------------------------------------------------
    menu_input_groups_populate - populate the
    input groups menu
-------------------------------------------------*/

ui_menu_input_groups::ui_menu_input_groups(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_input_groups::populate()
{
	int player;

	/* build up the menu */
	item_append("User Interface", NULL, 0, (void *)(IPG_UI + 1));
	for (player = 0; player < MAX_PLAYERS; player++)
	{
		char buffer[40];
		sprintf(buffer, "Player %d Controls", player + 1);
		item_append(buffer, NULL, 0, (void *)(FPTR)(IPG_PLAYER1 + player + 1));
	}
	item_append("Other Controls", NULL, 0, (void *)(FPTR)(IPG_OTHER + 1));
}

ui_menu_input_groups::~ui_menu_input_groups()
{
}

/*-------------------------------------------------
    menu_input_groups - handle the input groups
    menu
-------------------------------------------------*/

void ui_menu_input_groups::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);
	if (menu_event != NULL && menu_event->iptkey == IPT_UI_SELECT)
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_input_general(machine(), container, int((long long)(menu_event->itemref)-1))));
}



/*-------------------------------------------------
    menu_input_general - handle the general
    input menu
-------------------------------------------------*/

ui_menu_input_general::ui_menu_input_general(running_machine &machine, render_container *container, int _group) : ui_menu_input(machine, container)
{
	group = _group;
}

void ui_menu_input_general::populate()
{
	input_item_data *itemlist = NULL;
	int suborder[SEQ_TYPE_TOTAL];
	astring tempstring;
	int sortorder = 1;

	/* create a mini lookup table for sort order based on sequence type */
	suborder[SEQ_TYPE_STANDARD] = 0;
	suborder[SEQ_TYPE_DECREMENT] = 1;
	suborder[SEQ_TYPE_INCREMENT] = 2;

	/* iterate over the input ports and add menu items */
	for (input_type_entry *entry = machine().ioport().first_type(); entry != NULL; entry = entry->next())

		/* add if we match the group and we have a valid name */
		if (entry->group() == group && entry->name() != NULL && entry->name()[0] != 0)
		{
			input_seq_type seqtype;

			/* loop over all sequence types */
			sortorder++;
			for (seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; seqtype++)
			{
				/* build an entry for the standard sequence */
				input_item_data *item = (input_item_data *)m_pool_alloc(sizeof(*item));
				memset(item, 0, sizeof(*item));
				item->ref = entry;
				if(pollingitem && pollingref == entry && pollingseq == seqtype)
					pollingitem = item;
				item->seqtype = seqtype;
				item->seq = machine().ioport().type_seq(entry->type(), entry->player(), seqtype);
				item->defseq = &entry->defseq(seqtype);
				item->sortorder = sortorder * 4 + suborder[seqtype];
				item->type = ioport_manager::type_is_analog(entry->type()) ? (INPUT_TYPE_ANALOG + seqtype) : INPUT_TYPE_DIGITAL;
				item->name = entry->name();
				item->next = itemlist;
				itemlist = item;

				/* stop after one, unless we're analog */
				if (item->type == INPUT_TYPE_DIGITAL)
					break;
			}
		}

	/* sort and populate the menu in a standard fashion */
	populate_and_sort(itemlist);
}

ui_menu_input_general::~ui_menu_input_general()
{
}

/*-------------------------------------------------
    menu_input_specific - handle the game-specific
    input menu
-------------------------------------------------*/

ui_menu_input_specific::ui_menu_input_specific(running_machine &machine, render_container *container) : ui_menu_input(machine, container)
{
}

void ui_menu_input_specific::populate()
{
	input_item_data *itemlist = NULL;
	ioport_field *field;
	ioport_port *port;
	int suborder[SEQ_TYPE_TOTAL];
	astring tempstring;

	/* create a mini lookup table for sort order based on sequence type */
	suborder[SEQ_TYPE_STANDARD] = 0;
	suborder[SEQ_TYPE_DECREMENT] = 1;
	suborder[SEQ_TYPE_INCREMENT] = 2;

	/* iterate over the input ports and add menu items */
	for (port = machine().ioport().first_port(); port != NULL; port = port->next())
		for (field = port->first_field(); field != NULL; field = field->next())
		{
			const char *name = field->name();

			/* add if we match the group and we have a valid name */
			if (name != NULL && field->enabled() &&
				((field->type() == IPT_OTHER && field->name() != NULL) || machine().ioport().type_group(field->type(), field->player()) != IPG_INVALID))
			{
				input_seq_type seqtype;
				UINT16 sortorder;

				/* determine the sorting order */
				if (field->type() >= IPT_START1 && field->type() < IPT_ANALOG_LAST)
					sortorder = (field->type() << 2) | (field->player() << 12);
				else
					sortorder = field->type() | 0xf000;

				/* loop over all sequence types */
				for (seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; seqtype++)
				{
					/* build an entry for the standard sequence */
					input_item_data *item = (input_item_data *)m_pool_alloc(sizeof(*item));
					memset(item, 0, sizeof(*item));
					item->ref = field;
					item->seqtype = seqtype;
					if(pollingitem && pollingref == field && pollingseq == seqtype)
						pollingitem = item;
					item->seq = field->seq(seqtype);
					item->defseq = &field->defseq(seqtype);
					item->sortorder = sortorder + suborder[seqtype];
					item->type = field->is_analog() ? (INPUT_TYPE_ANALOG + seqtype) : INPUT_TYPE_DIGITAL;
					item->name = name;
					item->next = itemlist;
					itemlist = item;

					/* stop after one, unless we're analog */
					if (item->type == INPUT_TYPE_DIGITAL)
						break;
				}
			}
		}

	/* sort and populate the menu in a standard fashion */
	populate_and_sort(itemlist);
}

ui_menu_input_specific::~ui_menu_input_specific()
{
}

/*-------------------------------------------------
    menu_input - display a menu for inputs
-------------------------------------------------*/
ui_menu_input::ui_menu_input(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
	pollingitem = 0;
	pollingref = 0;
	pollingseq = SEQ_TYPE_STANDARD;
}

ui_menu_input::~ui_menu_input()
{
}

/*-------------------------------------------------
    toggle_none_default - toggle between "NONE"
    and the default item
-------------------------------------------------*/

void ui_menu_input::toggle_none_default(input_seq &selected_seq, input_seq &original_seq, const input_seq &selected_defseq)
{
	/* if we used to be "none", toggle to the default value */
	if (original_seq.length() == 0)
		selected_seq = selected_defseq;

	/* otherwise, toggle to "none" */
	else
		selected_seq.reset();
}

void ui_menu_input::handle()
{
	input_item_data *seqchangeditem = NULL;
	const ui_menu_event *menu_event;
	int invalidate = false;

	/* process the menu */
	menu_event = process((pollingitem != NULL) ? UI_MENU_PROCESS_NOKEYS : 0);

	/* if we are polling, handle as a special case */
	if (pollingitem != NULL)
	{
		input_item_data *item = pollingitem;
		input_seq newseq;

		/* if UI_CANCEL is pressed, abort */
		if (ui_input_pressed(machine(), IPT_UI_CANCEL))
		{
			pollingitem = NULL;
			record_next = false;
			toggle_none_default(item->seq, starting_seq, *item->defseq);
			seqchangeditem = item;
		}

		/* poll again; if finished, update the sequence */
		if (machine().input().seq_poll())
		{
			pollingitem = NULL;
			record_next = true;
			item->seq = machine().input().seq_poll_final();
			seqchangeditem = item;
		}
	}

	/* otherwise, handle the events */
	else if (menu_event != NULL && menu_event->itemref != NULL)
	{
		input_item_data *item = (input_item_data *)menu_event->itemref;
		switch (menu_event->iptkey)
		{
			/* an item was selected: begin polling */
			case IPT_UI_SELECT:
				pollingitem = item;
				last_sortorder = item->sortorder;
				starting_seq = item->seq;
				machine().input().seq_poll_start((item->type == INPUT_TYPE_ANALOG) ? ITEM_CLASS_ABSOLUTE : ITEM_CLASS_SWITCH, record_next ? &item->seq : NULL);
				invalidate = true;
				break;

			/* if the clear key was pressed, reset the selected item */
			case IPT_UI_CLEAR:
				toggle_none_default(item->seq, item->seq, *item->defseq);
				record_next = false;
				seqchangeditem = item;
				break;
		}

		/* if the selection changed, reset the "record next" flag */
		if (item->sortorder != last_sortorder)
			record_next = false;
		last_sortorder = item->sortorder;
	}

	/* if the sequence changed, update it */
	if (seqchangeditem != NULL)
	{
		update_input(seqchangeditem);

		/* invalidate the menu to force an update */
		invalidate = true;
	}

	/* if the menu is invalidated, clear it now */
	if (invalidate)
	{
		pollingref = NULL;
		if (pollingitem != NULL)
		{
			pollingref = pollingitem->ref;
			pollingseq = pollingitem->seqtype;
		}
		reset(UI_MENU_RESET_REMEMBER_POSITION);
	}
}

void ui_menu_input_general::update_input(struct input_item_data *seqchangeditem)
{
	const input_type_entry *entry = (const input_type_entry *)seqchangeditem->ref;
	machine().ioport().set_type_seq(entry->type(), entry->player(), seqchangeditem->seqtype, seqchangeditem->seq);
}

void ui_menu_input_specific::update_input(struct input_item_data *seqchangeditem)
{
	ioport_field::user_settings settings;

	((ioport_field *)seqchangeditem->ref)->get_user_settings(settings);
	settings.seq[seqchangeditem->seqtype] = seqchangeditem->seq;
	((ioport_field *)seqchangeditem->ref)->set_user_settings(settings);
}


/*-------------------------------------------------
    menu_input_compare_items - compare two
    items for quicksort
-------------------------------------------------*/

int ui_menu_input::compare_items(const void *i1, const void *i2)
{
	const input_item_data * const *data1 = (const input_item_data * const *)i1;
	const input_item_data * const *data2 = (const input_item_data * const *)i2;
	if ((*data1)->sortorder < (*data2)->sortorder)
		return -1;
	if ((*data1)->sortorder > (*data2)->sortorder)
		return 1;
	return 0;
}


/*-------------------------------------------------
    menu_input_populate_and_sort - take a list
    of input_item_data objects and build up the
    menu from them
-------------------------------------------------*/

void ui_menu_input::populate_and_sort(input_item_data *itemlist)
{
	const char *nameformat[INPUT_TYPE_TOTAL] = { 0 };
	input_item_data **itemarray, *item;
	int numitems = 0, curitem;
	astring subtext;
	astring text;

	/* create a mini lookup table for name format based on type */
	nameformat[INPUT_TYPE_DIGITAL] = "%s";
	nameformat[INPUT_TYPE_ANALOG] = "%s Analog";
	nameformat[INPUT_TYPE_ANALOG_INC] = "%s Analog Inc";
	nameformat[INPUT_TYPE_ANALOG_DEC] = "%s Analog Dec";

	/* first count the number of items */
	for (item = itemlist; item != NULL; item = item->next)
		numitems++;

	/* now allocate an array of items and fill it up */
	itemarray = (input_item_data **)m_pool_alloc(sizeof(*itemarray) * numitems);
	for (item = itemlist, curitem = 0; item != NULL; item = item->next)
		itemarray[curitem++] = item;

	/* sort it */
	qsort(itemarray, numitems, sizeof(*itemarray), compare_items);

	/* build the menu */
	for (curitem = 0; curitem < numitems; curitem++)
	{
		UINT32 flags = 0;

		/* generate the name of the item itself, based off the base name and the type */
		item = itemarray[curitem];
		assert(nameformat[item->type] != NULL);
		text.printf(nameformat[item->type], item->name);

		/* if we're polling this item, use some spaces with left/right arrows */
		if (pollingref == item->ref)
		{
			subtext.cpy("   ");
			flags |= MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
		}

		/* otherwise, generate the sequence name and invert it if different from the default */
		else
		{
			machine().input().seq_name(subtext, item->seq);
			flags |= (item->seq != *item->defseq) ? MENU_FLAG_INVERT : 0;
		}

		/* add the item */
		item_append(text, subtext, flags, item);
	}
}


/*-------------------------------------------------
    menu_settings_dip_switches - handle the DIP
    switches menu
-------------------------------------------------*/

ui_menu_settings_dip_switches::ui_menu_settings_dip_switches(running_machine &machine, render_container *container) : ui_menu_settings(machine, container, IPT_DIPSWITCH)
{
}

ui_menu_settings_dip_switches::~ui_menu_settings_dip_switches()
{
}

/*-------------------------------------------------
    menu_settings_driver_config - handle the
    driver config menu
-------------------------------------------------*/

ui_menu_settings_driver_config::ui_menu_settings_driver_config(running_machine &machine, render_container *container) : ui_menu_settings(machine, container, IPT_CONFIG)
{
}

ui_menu_settings_driver_config::~ui_menu_settings_driver_config()
{
}

/*-------------------------------------------------
    menu_settings_common - handle one of the
    switches menus
-------------------------------------------------*/

void ui_menu_settings::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);

	/* handle events */
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		ioport_field *field = (ioport_field *)menu_event->itemref;
		ioport_field::user_settings settings;
		int changed = false;

		switch (menu_event->iptkey)
		{
			/* if selected, reset to default value */
			case IPT_UI_SELECT:
				field->get_user_settings(settings);
				settings.value = field->defvalue();
				field->set_user_settings(settings);
				changed = true;
				break;

			/* left goes to previous setting */
			case IPT_UI_LEFT:
				field->select_previous_setting();
				changed = true;
				break;

			/* right goes to next setting */
			case IPT_UI_RIGHT:
				field->select_next_setting();
				changed = true;
				break;
		}

		/* if anything changed, rebuild the menu, trying to stay on the same field */
		if (changed)
			reset(UI_MENU_RESET_REMEMBER_REF);
	}
}


/*-------------------------------------------------
    menu_settings_populate - populate one of the
    switches menus
-------------------------------------------------*/

ui_menu_settings::ui_menu_settings(running_machine &machine, render_container *container, UINT32 _type) : ui_menu(machine, container)
{
	type = _type;
}

void ui_menu_settings::populate()
{
	ioport_field *field;
	ioport_port *port;
	dip_descriptor **diplist_tailptr;

	/* reset the dip switch tracking */
	dipcount = 0;
	diplist = NULL;
	diplist_tailptr = &diplist;

	/* loop over input ports and set up the current values */
	for (port = machine().ioport().first_port(); port != NULL; port = port->next())
		for (field = port->first_field(); field != NULL; field = field->next())
			if (field->type() == type && field->enabled())
			{
				UINT32 flags = 0;

				/* set the left/right flags appropriately */
				if (field->has_previous_setting())
					flags |= MENU_FLAG_LEFT_ARROW;
				if (field->has_next_setting())
					flags |= MENU_FLAG_RIGHT_ARROW;

				/* add the menu item */
				item_append(field->name(), field->setting_name(), flags, (void *)field);

				/* for DIP switches, build up the model */
				if (type == IPT_DIPSWITCH && field->first_diplocation() != NULL)
				{
					const ioport_diplocation *diploc;
					ioport_field::user_settings settings;
					UINT32 accummask = field->mask();

					/* get current settings */
					field->get_user_settings(settings);

					/* iterate over each bit in the field */
					for (diploc = field->first_diplocation(); diploc != NULL; diploc = diploc->next())
					{
						UINT32 mask = accummask & ~(accummask - 1);
						dip_descriptor *dip;

						/* find the matching switch name */
						for (dip = diplist; dip != NULL; dip = dip->next)
							if (strcmp(dip->name, diploc->name()) == 0)
								break;

						/* allocate new if none */
						if (dip == NULL)
						{
							dip = (dip_descriptor *)m_pool_alloc(sizeof(*dip));
							dip->next = NULL;
							dip->name = diploc->name();
							dip->mask = dip->state = 0;
							*diplist_tailptr = dip;
							diplist_tailptr = &dip->next;
							if (core_stricmp(dip->name, "FAKE") != 0)
								dipcount++;
						}

						/* apply the bits */
						dip->mask |= 1 << (diploc->number() - 1);
						if (((settings.value & mask) != 0 && !diploc->inverted()) || ((settings.value & mask) == 0 && diploc->inverted()))
							dip->state |= 1 << (diploc->number() - 1);

						/* clear the relevant bit in the accumulated mask */
						accummask &= ~mask;
					}
				}
			}
	if (type == IPT_DIPSWITCH)
		custombottom = dipcount * (DIP_SWITCH_HEIGHT + DIP_SWITCH_SPACING) + DIP_SWITCH_SPACING;
}

ui_menu_settings::~ui_menu_settings()
{
}

/*-------------------------------------------------
    menu_settings_custom_render - perform our special
    rendering
-------------------------------------------------*/

void ui_menu_settings_dip_switches::custom_render(void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
	ioport_field *field = (ioport_field *)selectedref;
	dip_descriptor *dip;

	if (field==NULL || field->first_diplocation() == NULL)
		return;

	/* add borders */
	y1 = y2 + UI_BOX_TB_BORDER;
	y2 = y1 + bottom;

	/* draw extra menu area */
	machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);
	y1 += (float)DIP_SWITCH_SPACING;

	/* iterate over DIP switches */
	for (dip = diplist; dip != NULL; dip = dip->next)
	{
		if (core_stricmp(dip->name, "FAKE") != 0)
		{
			const ioport_diplocation *diploc;
			UINT32 selectedmask = 0;

			/* determine the mask of selected bits */
			if (field != NULL)
				for (diploc = field->first_diplocation(); diploc != NULL; diploc = diploc->next())
					if (strcmp(dip->name, diploc->name()) == 0)
						selectedmask |= 1 << (diploc->number() - 1);

			/* draw one switch */
			custom_render_one(x1, y1, x2, y1 + DIP_SWITCH_HEIGHT, dip, selectedmask);
			y1 += (float)(DIP_SWITCH_SPACING + DIP_SWITCH_HEIGHT);
		}
	}
}


/*-------------------------------------------------
    menu_settings_custom_render_one - draw a single
    DIP switch
-------------------------------------------------*/

void ui_menu_settings_dip_switches::custom_render_one(float x1, float y1, float x2, float y2, const dip_descriptor *dip, UINT32 selectedmask)
{
	float switch_field_width = SINGLE_TOGGLE_SWITCH_FIELD_WIDTH * container->manager().ui_aspect();
	float switch_width = SINGLE_TOGGLE_SWITCH_WIDTH * container->manager().ui_aspect();
	int numtoggles, toggle;
	float switch_toggle_gap;
	float y1_off, y1_on;

	/* determine the number of toggles in the DIP */
	numtoggles = 32 - count_leading_zeros(dip->mask);

	/* center based on the number of switches */
	x1 += (x2 - x1 - numtoggles * switch_field_width) / 2;

	/* draw the dip switch name */
	machine().ui().draw_text_full(  container,
						dip->name,
						0,
						y1 + (DIP_SWITCH_HEIGHT - UI_TARGET_FONT_HEIGHT) / 2,
						x1 - machine().ui().get_string_width(" "),
						JUSTIFY_RIGHT,
						WRAP_NEVER,
						DRAW_NORMAL,
						UI_TEXT_COLOR,
						PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA),
						NULL ,
						NULL);

	/* compute top and bottom for on and off positions */
	switch_toggle_gap = ((DIP_SWITCH_HEIGHT/2) - SINGLE_TOGGLE_SWITCH_HEIGHT)/2;
	y1_off = y1 + UI_LINE_WIDTH + switch_toggle_gap;
	y1_on = y1 + DIP_SWITCH_HEIGHT/2 + switch_toggle_gap;

	/* iterate over toggles */
	for (toggle = 0; toggle < numtoggles; toggle++)
	{
		float innerx1;

		/* first outline the switch */
		machine().ui().draw_outlined_box(container, x1, y1, x1 + switch_field_width, y2, UI_BACKGROUND_COLOR);

		/* compute x1/x2 for the inner filled in switch */
		innerx1 = x1 + (switch_field_width - switch_width) / 2;

		/* see if the switch is actually used */
		if (dip->mask & (1 << toggle))
		{
			float innery1 = (dip->state & (1 << toggle)) ? y1_on : y1_off;
			container->add_rect(innerx1, innery1, innerx1 + switch_width, innery1 + SINGLE_TOGGLE_SWITCH_HEIGHT,
								(selectedmask & (1 << toggle)) ? UI_DIPSW_COLOR : UI_TEXT_COLOR,
								PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
		else
		{
			container->add_rect(innerx1, y1_off, innerx1 + switch_width, y1_on + SINGLE_TOGGLE_SWITCH_HEIGHT,
								UI_UNAVAILABLE_COLOR,
								PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}

		/* advance to the next switch */
		x1 += switch_field_width;
	}
}


/*-------------------------------------------------
    menu_analog - handle the analog settings menu
-------------------------------------------------*/

void ui_menu_analog::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);

	/* handle events */
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		analog_item_data *data = (analog_item_data *)menu_event->itemref;
		int newval = data->cur;

		switch (menu_event->iptkey)
		{
			/* if selected, reset to default value */
			case IPT_UI_SELECT:
				newval = data->defvalue;
				break;

			/* left decrements */
			case IPT_UI_LEFT:
				newval -= machine().input().code_pressed(KEYCODE_LSHIFT) ? 10 : 1;
				break;

			/* right increments */
			case IPT_UI_RIGHT:
				newval += machine().input().code_pressed(KEYCODE_LSHIFT) ? 10 : 1;
				break;
		}

		/* clamp to range */
		if (newval < data->min)
			newval = data->min;
		if (newval > data->max)
			newval = data->max;

		/* if things changed, update */
		if (newval != data->cur)
		{
			ioport_field::user_settings settings;

			/* get the settings and set the new value */
			data->field->get_user_settings(settings);
			switch (data->type)
			{
				case ANALOG_ITEM_KEYSPEED:      settings.delta = newval;        break;
				case ANALOG_ITEM_CENTERSPEED:   settings.centerdelta = newval;  break;
				case ANALOG_ITEM_REVERSE:       settings.reverse = newval;      break;
				case ANALOG_ITEM_SENSITIVITY:   settings.sensitivity = newval;  break;
			}
			data->field->set_user_settings(settings);

			/* rebuild the menu */
			reset(UI_MENU_RESET_REMEMBER_POSITION);
		}
	}
}


/*-------------------------------------------------
    menu_analog_populate - populate the analog
    settings menu
-------------------------------------------------*/

ui_menu_analog::ui_menu_analog(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_analog::populate()
{
	ioport_field *field;
	ioport_port *port;
	astring subtext;
	astring text;

	/* loop over input ports and add the items */
	for (port = machine().ioport().first_port(); port != NULL; port = port->next())
		for (field = port->first_field(); field != NULL; field = field->next())
			if (field->is_analog() && field->enabled())
			{
				ioport_field::user_settings settings;
				int use_autocenter = false;
				int type;

				/* based on the type, determine if we enable autocenter */
				switch (field->type())
				{
					case IPT_POSITIONAL:
					case IPT_POSITIONAL_V:
						if (field->analog_wraps())
							break;

					case IPT_AD_STICK_X:
					case IPT_AD_STICK_Y:
					case IPT_AD_STICK_Z:
					case IPT_PADDLE:
					case IPT_PADDLE_V:
					case IPT_PEDAL:
					case IPT_PEDAL2:
					case IPT_PEDAL3:
						use_autocenter = true;
						break;

					default:
						break;
				}

				/* get the user settings */
				field->get_user_settings(settings);

				/* iterate over types */
				for (type = 0; type < ANALOG_ITEM_COUNT; type++)
					if (type != ANALOG_ITEM_CENTERSPEED || use_autocenter)
					{
						analog_item_data *data;
						UINT32 flags = 0;

						/* allocate a data item for tracking what this menu item refers to */
						data = (analog_item_data *)m_pool_alloc(sizeof(*data));
						data->field = field;
						data->type = type;

						/* determine the properties of this item */
						switch (type)
						{
							default:
							case ANALOG_ITEM_KEYSPEED:
								text.printf("%s Digital Speed", field->name());
								subtext.printf("%d", settings.delta);
								data->min = 0;
								data->max = 255;
								data->cur = settings.delta;
								data->defvalue = field->delta();
								break;

							case ANALOG_ITEM_CENTERSPEED:
								text.printf("%s Autocenter Speed", field->name());
								subtext.printf("%d", settings.centerdelta);
								data->min = 0;
								data->max = 255;
								data->cur = settings.centerdelta;
								data->defvalue = field->centerdelta();
								break;

							case ANALOG_ITEM_REVERSE:
								text.printf("%s Reverse", field->name());
								subtext.cpy(settings.reverse ? "On" : "Off");
								data->min = 0;
								data->max = 1;
								data->cur = settings.reverse;
								data->defvalue = field->analog_reverse();
								break;

							case ANALOG_ITEM_SENSITIVITY:
								text.printf("%s Sensitivity", field->name());
								subtext.printf("%d", settings.sensitivity);
								data->min = 1;
								data->max = 255;
								data->cur = settings.sensitivity;
								data->defvalue = field->sensitivity();
								break;
						}

						/* put on arrows */
						if (data->cur > data->min)
							flags |= MENU_FLAG_LEFT_ARROW;
						if (data->cur < data->max)
							flags |= MENU_FLAG_RIGHT_ARROW;

						/* append a menu item */
						item_append(text, subtext, flags, data);
					}
			}
}

ui_menu_analog::~ui_menu_analog()
{
}

/*-------------------------------------------------
    menu_bookkeeping - handle the bookkeeping
    information menu
-------------------------------------------------*/

void ui_menu_bookkeeping::handle()
{
	attotime curtime;

	/* if the time has rolled over another second, regenerate */
	curtime = machine().time();
	if (prevtime.seconds != curtime.seconds)
	{
		reset(UI_MENU_RESET_SELECT_FIRST);
		prevtime = curtime;
		populate();
	}

	/* process the menu */
	process(0);
}


/*-------------------------------------------------
    menu_bookkeeping - handle the bookkeeping
    information menu
-------------------------------------------------*/
ui_menu_bookkeeping::ui_menu_bookkeeping(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

ui_menu_bookkeeping::~ui_menu_bookkeeping()
{
}

void ui_menu_bookkeeping::populate()
{
	int tickets = get_dispensed_tickets(machine());
	astring tempstring;
	int ctrnum;

	/* show total time first */
	if (prevtime.seconds >= 60 * 60)
		tempstring.catprintf("Uptime: %d:%02d:%02d\n\n", prevtime.seconds / (60*60), (prevtime.seconds / 60) % 60, prevtime.seconds % 60);
	else
		tempstring.catprintf("Uptime: %d:%02d\n\n", (prevtime.seconds / 60) % 60, prevtime.seconds % 60);

	/* show tickets at the top */
	if (tickets > 0)
		tempstring.catprintf("Tickets dispensed: %d\n\n", tickets);

	/* loop over coin counters */
	for (ctrnum = 0; ctrnum < COIN_COUNTERS; ctrnum++)
	{
		int count = coin_counter_get_count(machine(), ctrnum);

		/* display the coin counter number */
		tempstring.catprintf("Coin %c: ", ctrnum + 'A');

		/* display how many coins */
		if (count == 0)
			tempstring.cat("NA");
		else
			tempstring.catprintf("%d", count);

		/* display whether or not we are locked out */
		if (coin_lockout_get_state(machine(), ctrnum))
			tempstring.cat(" (locked)");
		tempstring.cat("\n");
	}

	/* append the single item */
	item_append(tempstring, NULL, MENU_FLAG_MULTILINE, NULL);
}


/*-------------------------------------------------
    menu_game_info - handle the game information
    menu
-------------------------------------------------*/

ui_menu_game_info::ui_menu_game_info(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_game_info::populate()
{
	astring tempstring;
	item_append(machine().ui().game_info_astring(tempstring), NULL, MENU_FLAG_MULTILINE, NULL);
}

void ui_menu_game_info::handle()
{
	/* process the menu */
	process(0);
}

ui_menu_game_info::~ui_menu_game_info()
{
}

/*-------------------------------------------------
    menu_cheat - handle the cheat menu
-------------------------------------------------*/

void ui_menu_cheat::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);

	/* handle events */
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		bool changed = false;

		/* clear cheat comment on any movement or keypress */
		popmessage(NULL);

		/* handle reset all + reset all cheats for reload all option */
		if ((FPTR)menu_event->itemref < 3 && menu_event->iptkey == IPT_UI_SELECT)
		{
			for (cheat_entry *curcheat = machine().cheat().first(); curcheat != NULL; curcheat = curcheat->next())
				if (curcheat->select_default_state())
					changed = true;
		}


		/* handle individual cheats */
		else if ((FPTR)menu_event->itemref > 2)
		{
			cheat_entry *curcheat = reinterpret_cast<cheat_entry *>(menu_event->itemref);
			const char *string;
			switch (menu_event->iptkey)
			{
				/* if selected, activate a oneshot */
				case IPT_UI_SELECT:
					changed = curcheat->activate();
					break;

				/* if cleared, reset to default value */
				case IPT_UI_CLEAR:
					changed = curcheat->select_default_state();
					break;

				/* left decrements */
				case IPT_UI_LEFT:
					changed = curcheat->select_previous_state();
					break;

				/* right increments */
				case IPT_UI_RIGHT:
					changed = curcheat->select_next_state();
					break;

				/* bring up display comment if one exists */
				case IPT_UI_DISPLAY_COMMENT:
				case IPT_UI_UP:
				case IPT_UI_DOWN:
					string = curcheat->comment();
					if (string != NULL && string[0] != 0)
						popmessage("Cheat Comment:\n%s", string);
					break;
			}
		}

		/* handle reload all  */
		if ((FPTR)menu_event->itemref == 2 && menu_event->iptkey == IPT_UI_SELECT)
		{
			/* re-init cheat engine and thus reload cheats/cheats have already been turned off by here */
			machine().cheat().reload();

			/* display the reloaded cheats */
			reset(UI_MENU_RESET_REMEMBER_REF);
			popmessage("All cheats reloaded");
		}

		/* if things changed, update */
		if (changed)
			reset(UI_MENU_RESET_REMEMBER_REF);
	}
}


/*-------------------------------------------------
    menu_cheat_populate - populate the cheat menu
-------------------------------------------------*/

ui_menu_cheat::ui_menu_cheat(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_cheat::populate()
{
	/* iterate over cheats */
	astring text;
	astring subtext;
	for (cheat_entry *curcheat = machine().cheat().first(); curcheat != NULL; curcheat = curcheat->next())
	{
		UINT32 flags;
		curcheat->menu_text(text, subtext, flags);
		item_append(text, subtext, flags, curcheat);
	}

	/* add a separator */
	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	/* add a reset all option */
	item_append("Reset All", NULL, 0, (void *)1);

	/* add a reload all cheats option */
	item_append("Reload All", NULL, 0, (void *)2);
}

ui_menu_cheat::~ui_menu_cheat()
{
}

/*-------------------------------------------------
    menu_sliders - handle the sliders menu
-------------------------------------------------*/

void ui_menu_sliders::handle()
{
	const ui_menu_event *menu_event;

	/* process the menu */
	menu_event = process(UI_MENU_PROCESS_LR_REPEAT | (hidden ? UI_MENU_PROCESS_CUSTOM_ONLY : 0));
	if (menu_event != NULL)
	{
		/* handle keys if there is a valid item selected */
		if (menu_event->itemref != NULL)
		{
			const slider_state *slider = (const slider_state *)menu_event->itemref;
			INT32 curvalue = (*slider->update)(machine(), slider->arg, NULL, SLIDER_NOCHANGE);
			INT32 increment = 0;

			switch (menu_event->iptkey)
			{
				/* toggle visibility */
				case IPT_UI_ON_SCREEN_DISPLAY:
					if (menuless_mode)
						ui_menu::stack_pop(machine());
					else
						hidden = !hidden;
					break;

				/* decrease value */
				case IPT_UI_LEFT:
					if (machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT))
						increment = -1;
					else if (machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT))
						increment = (slider->incval > 10) ? -(slider->incval / 10) : -1;
					else if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL))
						increment = -slider->incval * 10;
					else
						increment = -slider->incval;
					break;

				/* increase value */
				case IPT_UI_RIGHT:
					if (machine().input().code_pressed(KEYCODE_LALT) || machine().input().code_pressed(KEYCODE_RALT))
						increment = 1;
					else if (machine().input().code_pressed(KEYCODE_LSHIFT) || machine().input().code_pressed(KEYCODE_RSHIFT))
						increment = (slider->incval > 10) ? (slider->incval / 10) : 1;
					else if (machine().input().code_pressed(KEYCODE_LCONTROL) || machine().input().code_pressed(KEYCODE_RCONTROL))
						increment = slider->incval * 10;
					else
						increment = slider->incval;
					break;

				/* restore default */
				case IPT_UI_SELECT:
					increment = slider->defval - curvalue;
					break;
			}

			/* handle any changes */
			if (increment != 0)
			{
				INT32 newvalue = curvalue + increment;

				/* clamp within bounds */
				if (newvalue < slider->minval)
					newvalue = slider->minval;
				if (newvalue > slider->maxval)
					newvalue = slider->maxval;

				/* update the slider and recompute the menu */
				(*slider->update)(machine(), slider->arg, NULL, newvalue);
				reset(UI_MENU_RESET_REMEMBER_REF);
			}
		}

		/* if we are selecting an invalid item and we are hidden, skip to the next one */
		else if (hidden)
		{
			/* if we got here via up or page up, select the previous item */
			if (menu_event->iptkey == IPT_UI_UP || menu_event->iptkey == IPT_UI_PAGE_UP)
			{
				selected = (selected + numitems - 1) % numitems;
				validate_selection(-1);
			}

			/* otherwise select the next item */
			else if (menu_event->iptkey == IPT_UI_DOWN || menu_event->iptkey == IPT_UI_PAGE_DOWN)
			{
				selected = (selected + 1) % numitems;
				validate_selection(1);
			}
		}
	}
}


/*-------------------------------------------------
    menu_sliders_populate - populate the sliders
    menu
-------------------------------------------------*/

ui_menu_sliders::ui_menu_sliders(running_machine &machine, render_container *container, bool _menuless_mode) : ui_menu(machine, container)
{
	menuless_mode = hidden = _menuless_mode;
}

void ui_menu_sliders::populate()
{
	const slider_state *curslider;
	astring tempstring;

	/* add all sliders */
	for (curslider = machine().ui().get_slider_list(); curslider != NULL; curslider = curslider->next)
	{
		INT32 curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);
		UINT32 flags = 0;
		if (curval > curslider->minval)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (curval < curslider->maxval)
			flags |= MENU_FLAG_RIGHT_ARROW;
		item_append(curslider->description, tempstring, flags, (void *)curslider);

		if (menuless_mode)
			break;
	}

	/* add all sliders */
	for (curslider = (slider_state*)machine().osd().get_slider_list(); curslider != NULL; curslider = curslider->next)
	{
		INT32 curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);
		UINT32 flags = 0;
		if (curval > curslider->minval)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (curval < curslider->maxval)
			flags |= MENU_FLAG_RIGHT_ARROW;
		item_append(curslider->description, tempstring, flags, (void *)curslider);
	}

	custombottom = 2.0f * machine().ui().get_line_height() + 2.0f * UI_BOX_TB_BORDER;
}

ui_menu_sliders::~ui_menu_sliders()
{
}

/*-------------------------------------------------
    menu_sliders_custom_render - perform our special
    rendering
-------------------------------------------------*/

void ui_menu_sliders::custom_render(void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
	const slider_state *curslider = (const slider_state *)selectedref;
	if (curslider != NULL)
	{
		float bar_left, bar_area_top, bar_width, bar_area_height, bar_top, bar_bottom, default_x, current_x;
		float line_height = machine().ui().get_line_height();
		float percentage, default_percentage;
		astring tempstring;
		float text_height;
		INT32 curval;

		/* determine the current value and text */
		curval = (*curslider->update)(machine(), curslider->arg, &tempstring, SLIDER_NOCHANGE);

		/* compute the current and default percentages */
		percentage = (float)(curval - curslider->minval) / (float)(curslider->maxval - curslider->minval);
		default_percentage = (float)(curslider->defval - curslider->minval) / (float)(curslider->maxval - curslider->minval);

		/* assemble the text */
		tempstring.ins(0, " ").ins(0, curslider->description);

		/* move us to the bottom of the screen, and expand to full width */
		y2 = 1.0f - UI_BOX_TB_BORDER;
		y1 = y2 - bottom;
		x1 = UI_BOX_LR_BORDER;
		x2 = 1.0f - UI_BOX_LR_BORDER;

		/* draw extra menu area */
		machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);
		y1 += UI_BOX_TB_BORDER;

		/* determine the text height */
		machine().ui().draw_text_full(container, tempstring, 0, 0, x2 - x1 - 2.0f * UI_BOX_LR_BORDER,
					JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, NULL, &text_height);

		/* draw the thermometer */
		bar_left = x1 + UI_BOX_LR_BORDER;
		bar_area_top = y1;
		bar_width = x2 - x1 - 2.0f * UI_BOX_LR_BORDER;
		bar_area_height = line_height;

		/* compute positions */
		bar_top = bar_area_top + 0.125f * bar_area_height;
		bar_bottom = bar_area_top + 0.875f * bar_area_height;
		default_x = bar_left + bar_width * default_percentage;
		current_x = bar_left + bar_width * percentage;

		/* fill in the percentage */
		container->add_rect(bar_left, bar_top, current_x, bar_bottom, UI_SLIDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw the top and bottom lines */
		container->add_line(bar_left, bar_top, bar_left + bar_width, bar_top, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container->add_line(bar_left, bar_bottom, bar_left + bar_width, bar_bottom, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw default marker */
		container->add_line(default_x, bar_area_top, default_x, bar_top, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container->add_line(default_x, bar_bottom, default_x, bar_area_top + bar_area_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw the actual text */
		machine().ui().draw_text_full(container, tempstring, x1 + UI_BOX_LR_BORDER, y1 + line_height, x2 - x1 - 2.0f * UI_BOX_LR_BORDER,
					JUSTIFY_CENTER, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, &text_height);
	}
}


/*-------------------------------------------------
    menu_video_targets - handle the video targets
    menu
-------------------------------------------------*/

void ui_menu_video_targets::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);
	if (menu_event != NULL && menu_event->iptkey == IPT_UI_SELECT)
		ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_video_options(machine(), container, static_cast<render_target *>(menu_event->itemref))));
}


/*-------------------------------------------------
    menu_video_targets_populate - populate the
    video targets menu
-------------------------------------------------*/

ui_menu_video_targets::ui_menu_video_targets(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_video_targets::populate()
{
	int targetnum;

	/* find the targets */
	for (targetnum = 0; ; targetnum++)
	{
		render_target *target = machine().render().target_by_index(targetnum);
		char buffer[40];

		/* stop when we run out */
		if (target == NULL)
			break;

		/* add a menu item */
		sprintf(buffer, "Screen #%d", targetnum);
		item_append(buffer, NULL, 0, target);
	}
}

ui_menu_video_targets::~ui_menu_video_targets()
{
}

/*-------------------------------------------------
    menu_video_options - handle the video options
    menu
-------------------------------------------------*/

void ui_menu_video_options::handle()
{
	bool changed = false;

	/* process the menu */
	const ui_menu_event *menu_event = process(0);
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		switch ((FPTR)menu_event->itemref)
		{
			/* rotate adds rotation depending on the direction */
			case VIDEO_ITEM_ROTATE:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					int delta = (menu_event->iptkey == IPT_UI_LEFT) ? ROT270 : ROT90;
					target->set_orientation(orientation_add(delta, target->orientation()));
					if (target->is_ui_target())
					{
						render_container::user_settings settings;
						container->get_user_settings(settings);
						settings.m_orientation = orientation_add(delta ^ ROT180, settings.m_orientation);
						container->set_user_settings(settings);
					}
					changed = true;
				}
				break;

			/* layer config bitmasks handle left/right keys the same (toggle) */
			case VIDEO_ITEM_BACKDROPS:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_backdrops_enabled(!target->backdrops_enabled());
					changed = true;
				}
				break;

			case VIDEO_ITEM_OVERLAYS:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_overlays_enabled(!target->overlays_enabled());
					changed = true;
				}
				break;

			case VIDEO_ITEM_BEZELS:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_bezels_enabled(!target->bezels_enabled());
					changed = true;
				}
				break;

			case VIDEO_ITEM_CPANELS:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_cpanels_enabled(!target->cpanels_enabled());
					changed = true;
				}
				break;

			case VIDEO_ITEM_MARQUEES:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_marquees_enabled(!target->marquees_enabled());
					changed = true;
				}
				break;

			case VIDEO_ITEM_ZOOM:
				if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
				{
					target->set_zoom_to_screen(!target->zoom_to_screen());
					changed = true;
				}
				break;

			/* anything else is a view item */
			default:
				if (menu_event->iptkey == IPT_UI_SELECT && (int)(FPTR)menu_event->itemref >= VIDEO_ITEM_VIEW)
				{
					target->set_view((FPTR)menu_event->itemref - VIDEO_ITEM_VIEW);
					changed = true;
				}
				break;
		}
	}

	/* if something changed, rebuild the menu */
	if (changed)
		reset(UI_MENU_RESET_REMEMBER_REF);
}


/*-------------------------------------------------
    menu_video_options_populate - populate the
    video options menu
-------------------------------------------------*/

ui_menu_video_options::ui_menu_video_options(running_machine &machine, render_container *container, render_target *_target) : ui_menu(machine, container)
{
	target = _target;
}

void ui_menu_video_options::populate()
{
	const char *subtext = "";
	astring tempstring;
	int viewnum;
	int enabled;

	/* add items for each view */
	for (viewnum = 0; ; viewnum++)
	{
		const char *name = target->view_name(viewnum);
		if (name == NULL)
			break;

		/* create a string for the item, replacing underscores with spaces */
		tempstring.cpy(name).replace(0, "_", " ");
		item_append(tempstring, NULL, 0, (void *)(FPTR)(VIDEO_ITEM_VIEW + viewnum));
	}

	/* add a separator */
	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	/* add a rotate item */
	switch (target->orientation())
	{
		case ROT0:      subtext = "None";                   break;
		case ROT90:     subtext = "CW 90" UTF8_DEGREES;     break;
		case ROT180:    subtext = "180" UTF8_DEGREES;       break;
		case ROT270:    subtext = "CCW 90" UTF8_DEGREES;    break;
	}
	item_append("Rotate", subtext, MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_ROTATE);

	/* backdrop item */
	enabled = target->backdrops_enabled();
	item_append("Backdrops", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_BACKDROPS);

	/* overlay item */
	enabled = target->overlays_enabled();
	item_append("Overlays", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_OVERLAYS);

	/* bezel item */
	enabled = target->bezels_enabled();
	item_append("Bezels", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_BEZELS);

	/* cpanel item */
	enabled = target->cpanels_enabled();
	item_append("CPanels", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_CPANELS);

	/* marquee item */
	enabled = target->marquees_enabled();
	item_append("Marquees", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_MARQUEES);

	/* cropping */
	enabled = target->zoom_to_screen();
	item_append("View", enabled ? "Cropped" : "Full", enabled ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)VIDEO_ITEM_ZOOM);
}

ui_menu_video_options::~ui_menu_video_options()
{
}

/*-------------------------------------------------
    menu_crosshair - handle the crosshair settings
    menu
-------------------------------------------------*/

void ui_menu_crosshair::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(UI_MENU_PROCESS_LR_REPEAT);

	/* handle events */
	if (menu_event != NULL && menu_event->itemref != NULL)
	{
		crosshair_user_settings settings;
		crosshair_item_data *data = (crosshair_item_data *)menu_event->itemref;
		bool changed = false;
		//int set_def = false;
		int newval = data->cur;

		/* retreive the user settings */
		crosshair_get_user_settings(machine(), data->player, &settings);

		switch (menu_event->iptkey)
		{
			/* if selected, reset to default value */
			case IPT_UI_SELECT:
				newval = data->defvalue;
				//set_def = true;
				break;

			/* left decrements */
			case IPT_UI_LEFT:
				newval -= machine().input().code_pressed(KEYCODE_LSHIFT) ? 10 : 1;
				break;

			/* right increments */
			case IPT_UI_RIGHT:
				newval += machine().input().code_pressed(KEYCODE_LSHIFT) ? 10 : 1;
				break;
		}

		/* clamp to range */
		if (newval < data->min)
			newval = data->min;
		if (newval > data->max)
			newval = data->max;

		/* if things changed, update */
		if (newval != data->cur)
		{
			switch (data->type)
			{
				/* visibility state */
				case CROSSHAIR_ITEM_VIS:
					settings.mode = newval;
					changed = true;
					break;

				/* auto time */
				case CROSSHAIR_ITEM_AUTO_TIME:
					settings.auto_time = newval;
					changed = true;
					break;
			}
		}

		/* crosshair graphic name */
		if (data->type == CROSSHAIR_ITEM_PIC)
		{
			switch (menu_event->iptkey)
			{
				case IPT_UI_SELECT:
					/* clear the name string to reset to default crosshair */
					settings.name[0] = 0;
					changed = true;
					break;

				case IPT_UI_LEFT:
					strcpy(settings.name, data->last_name);
					changed = true;
					break;

				case IPT_UI_RIGHT:
					strcpy(settings.name, data->next_name);
					changed = true;
					break;
			}
		}

		if (changed)
		{
			/* save the user settings */
			crosshair_set_user_settings(machine(), data->player, &settings);

			/* rebuild the menu */
			reset(UI_MENU_RESET_REMEMBER_POSITION);
		}
	}
}


/*-------------------------------------------------
    menu_crosshair_populate - populate the
    crosshair settings menu
-------------------------------------------------*/

ui_menu_crosshair::ui_menu_crosshair(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_crosshair::populate()
{
	crosshair_user_settings settings;
	crosshair_item_data *data;
	char temp_text[16];
	int player;
	UINT8 use_auto = false;
	UINT32 flags = 0;

	/* loop over player and add the manual items */
	for (player = 0; player < MAX_PLAYERS; player++)
	{
		/* get the user settings */
		crosshair_get_user_settings(machine(), player, &settings);

		/* add menu items for usable crosshairs */
		if (settings.used)
		{
			/* Make sure to keep these matched to the CROSSHAIR_VISIBILITY_xxx types */
			static const char *const vis_text[] = { "Off", "On", "Auto" };

			/* track if we need the auto time menu */
			if (settings.mode == CROSSHAIR_VISIBILITY_AUTO) use_auto = true;

			/* CROSSHAIR_ITEM_VIS - allocate a data item and fill it */
			data = (crosshair_item_data *)m_pool_alloc(sizeof(*data));
			data->type = CROSSHAIR_ITEM_VIS;
			data->player = player;
			data->min = CROSSHAIR_VISIBILITY_OFF;
			data->max = CROSSHAIR_VISIBILITY_AUTO;
			data->defvalue = CROSSHAIR_VISIBILITY_DEFAULT;
			data->cur = settings.mode;

			/* put on arrows */
			if (data->cur > data->min)
				flags |= MENU_FLAG_LEFT_ARROW;
			if (data->cur < data->max)
				flags |= MENU_FLAG_RIGHT_ARROW;

			/* add CROSSHAIR_ITEM_VIS menu */
			sprintf(temp_text, "P%d Visibility", player + 1);
			item_append(temp_text, vis_text[settings.mode], flags, data);

			/* CROSSHAIR_ITEM_PIC - allocate a data item and fill it */
			data = (crosshair_item_data *)m_pool_alloc(sizeof(*data));
			data->type = CROSSHAIR_ITEM_PIC;
			data->player = player;
			data->last_name[0] = 0;
			/* other data item not used by this menu */

			/* search for crosshair graphics */

			/* open a path to the crosshairs */
			file_enumerator path(machine().options().crosshair_path());
			const osd_directory_entry *dir;
			/* reset search flags */
			int using_default = false;
			int finished = false;
			int found = false;

			/* if we are using the default, then we just need to find the first in the list */
			if (*(settings.name) == 0)
				using_default = true;

			/* look for the current name, then remember the name before */
			/* and find the next name */
			while (((dir = path.next()) != NULL) && !finished)
			{
				int length = strlen(dir->name);

				/* look for files ending in .png with a name not larger then 9 chars*/
				if ((length > 4) && (length <= CROSSHAIR_PIC_NAME_LENGTH + 4) &&
					dir->name[length - 4] == '.' &&
					tolower((UINT8)dir->name[length - 3]) == 'p' &&
					tolower((UINT8)dir->name[length - 2]) == 'n' &&
					tolower((UINT8)dir->name[length - 1]) == 'g')

				{
					/* remove .png from length */
					length -= 4;

					if (found || using_default)
					{
						/* get the next name */
						strncpy(data->next_name, dir->name, length);
						data->next_name[length] = 0;
						finished = true;
					}
					else if (!strncmp(dir->name, settings.name, length))
					{
						/* we found the current name */
						/* so loop once more to find the next name */
						found = true;
					}
					else
						/* remember last name */
						/* we will do it here in case files get added to the directory */
					{
						strncpy(data->last_name, dir->name, length);
						data->last_name[length] = 0;
					}
				}
			}
			/* if name not found then next item is DEFAULT */
			if (!found && !using_default)
			{
				data->next_name[0] = 0;
				finished = true;
			}
			/* setup the selection flags */
			flags = 0;
			if (finished)
				flags |= MENU_FLAG_RIGHT_ARROW;
			if (found)
				flags |= MENU_FLAG_LEFT_ARROW;

			/* add CROSSHAIR_ITEM_PIC menu */
			sprintf(temp_text, "P%d Crosshair", player + 1);
			item_append(temp_text, using_default ? "DEFAULT" : settings.name, flags, data);
		}
	}
	if (use_auto)
	{
		/* any player can be used to get the autotime */
		crosshair_get_user_settings(machine(), 0, &settings);

		/* CROSSHAIR_ITEM_AUTO_TIME - allocate a data item and fill it */
		data = (crosshair_item_data *)m_pool_alloc(sizeof(*data));
		data->type = CROSSHAIR_ITEM_AUTO_TIME;
		data->min = CROSSHAIR_VISIBILITY_AUTOTIME_MIN;
		data->max = CROSSHAIR_VISIBILITY_AUTOTIME_MAX;
		data->defvalue = CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT;
		data->cur = settings.auto_time;

		/* put on arrows in visible menu */
		if (data->cur > data->min)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (data->cur < data->max)
			flags |= MENU_FLAG_RIGHT_ARROW;

		/* add CROSSHAIR_ITEM_AUTO_TIME menu */
		sprintf(temp_text, "%d", settings.auto_time);
		item_append("Visible Delay", temp_text, flags, data);
	}
//  else
//      /* leave a blank filler line when not in auto time so size does not rescale */
//      item_append("", "", NULL, NULL);
}

ui_menu_crosshair::~ui_menu_crosshair()
{
}

/*-------------------------------------------------
    menu_quit_game - handle the "menu" for
    quitting the game
-------------------------------------------------*/

ui_menu_quit_game::ui_menu_quit_game(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

ui_menu_quit_game::~ui_menu_quit_game()
{
}

void ui_menu_quit_game::populate()
{
}

void ui_menu_quit_game::handle()
{
	/* request a reset */
	machine().schedule_exit();

	/* reset the menu stack */
	ui_menu::stack_reset(machine());
}
