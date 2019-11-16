// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/inputmap.cpp

    Internal menus for input mappings.

*********************************************************************/

#include "emu.h"
#include "ui/inputmap.h"

#include "uiinput.h"
#include "ui/ui.h"

#include <algorithm>


namespace ui {

/*-------------------------------------------------
    menu_input_groups_populate - populate the
    input groups menu
-------------------------------------------------*/

menu_input_groups::menu_input_groups(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

void menu_input_groups::populate(float &customtop, float &custombottom)
{
	int player;

	/* build up the menu */
	item_append(_("User Interface"), "", 0, (void *)(IPG_UI + 1));
	for (player = 0; player < MAX_PLAYERS; player++)
	{
		auto s = string_format("Player %d Controls", player + 1);
		item_append(s, "", 0, (void *)(uintptr_t)(IPG_PLAYER1 + player + 1));
	}
	item_append(_("Other Controls"), "", 0, (void *)(uintptr_t)(IPG_OTHER + 1));
}

menu_input_groups::~menu_input_groups()
{
}

/*-------------------------------------------------
    menu_input_groups - handle the input groups
    menu
-------------------------------------------------*/

void menu_input_groups::handle()
{
	/* process the menu */
	const event *menu_event = process(0);
	if (menu_event != nullptr && menu_event->iptkey == IPT_UI_SELECT)
		menu::stack_push<menu_input_general>(ui(), container(), int((long long)(menu_event->itemref)-1));
}



/*-------------------------------------------------
    menu_input_general - handle the general
    input menu
-------------------------------------------------*/

menu_input_general::menu_input_general(mame_ui_manager &mui, render_container &container, int _group)
	: menu_input(mui, container)
	, group(_group)
{
}

void menu_input_general::populate(float &customtop, float &custombottom)
{
	if (data.empty())
	{
		assert(!pollingitem);

		// iterate over the input ports and add menu items
		for (input_type_entry &entry : machine().ioport().types())
		{
			// add if we match the group and we have a valid name
			if ((entry.group() == group) && entry.name() && entry.name()[0])
			{
				// loop over all sequence types
				for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
				{
					// build an entry for the standard sequence
					input_item_data &item(*data.emplace(data.end()));
					item.ref = &entry;
					item.seqtype = seqtype;
					item.seq = machine().ioport().type_seq(entry.type(), entry.player(), seqtype);
					item.defseq = &entry.defseq(seqtype);
					item.group = entry.group();
					item.type = ioport_manager::type_is_analog(entry.type()) ? (INPUT_TYPE_ANALOG + seqtype) : INPUT_TYPE_DIGITAL;
					item.is_optional = false;
					item.name = entry.name();
					item.owner_name = nullptr;

					// stop after one, unless we're analog
					if (item.type == INPUT_TYPE_DIGITAL)
						break;
				}
			}
		}
	}
	else
	{
		for (input_item_data &item : data)
		{
			const input_type_entry &entry(*reinterpret_cast<const input_type_entry *>(item.ref));
			item.seq = machine().ioport().type_seq(entry.type(), entry.player(), item.seqtype);
		}
	}

	// populate the menu in a standard fashion
	populate_sorted();
}

menu_input_general::~menu_input_general()
{
}

/*-------------------------------------------------
    menu_input_specific - handle the game-specific
    input menu
-------------------------------------------------*/

menu_input_specific::menu_input_specific(mame_ui_manager &mui, render_container &container) : menu_input(mui, container)
{
}

void menu_input_specific::populate(float &customtop, float &custombottom)
{
	if (data.empty())
	{
		assert(!pollingitem);

		// iterate over the input ports and add menu items
		for (auto &port : machine().ioport().ports())
		{
			for (ioport_field &field : port.second->fields())
			{
				const ioport_type_class type_class = field.type_class();

				// add if it's enabled and it's a system-specific class
				if (field.enabled() && (type_class == INPUT_CLASS_CONTROLLER || type_class == INPUT_CLASS_MISC || type_class == INPUT_CLASS_KEYBOARD))
				{
					// loop over all sequence types
					for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
					{
						// build an entry for the standard sequence
						input_item_data &item(*data.emplace(data.end()));
						item.ref = &field;
						item.seqtype = seqtype;
						item.seq = field.seq(seqtype);
						item.defseq = &field.defseq(seqtype);
						item.group = machine().ioport().type_group(field.type(), field.player());
						item.type = field.is_analog() ? (INPUT_TYPE_ANALOG + seqtype) : INPUT_TYPE_DIGITAL;
						item.is_optional = field.optional();
						item.name = field.name();
						item.owner_name = field.device().tag();

						// stop after one, unless we're analog
						if (item.type == INPUT_TYPE_DIGITAL)
							break;
					}
				}
			}
		}

		// sort it
		std::sort(
				data.begin(),
				data.end(),
				[] (const input_item_data &i1, const input_item_data &i2)
				{
					int cmp = strcmp(i1.owner_name, i2.owner_name);
					if (cmp < 0)
						return true;
					if (cmp > 0)
						return false;
					if (i1.group < i2.group)
						return true;
					if (i1.group > i2.group)
						return false;
					const ioport_field &field1 = *reinterpret_cast<const ioport_field *>(i1.ref);
					const ioport_field &field2 = *reinterpret_cast<const ioport_field *>(i2.ref);
					if (field1.type() < field2.type())
						return true;
					if (field1.type() > field2.type())
						return false;
					std::vector<char32_t> codes1 = field1.keyboard_codes(0);
					std::vector<char32_t> codes2 = field2.keyboard_codes(0);
					if (!codes1.empty() && (codes2.empty() || codes1[0] < codes2[0]))
						return true;
					if (!codes2.empty() && (codes1.empty() || codes1[0] > codes2[0]))
						return false;
					cmp = strcmp(i1.name, i2.name);
					if (cmp < 0)
						return true;
					if (cmp > 0)
						return false;
					return i1.type < i2.type;
				});
	}
	else
	{
		for (input_item_data &item : data)
		{
			const ioport_field &field(*reinterpret_cast<const ioport_field *>(item.ref));
			item.seq = field.seq(item.seqtype);
		}
	}

	// populate the menu in a standard fashion
	populate_sorted();
}

menu_input_specific::~menu_input_specific()
{
}

/*-------------------------------------------------
    menu_input - display a menu for inputs
-------------------------------------------------*/
menu_input::menu_input(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
	, data()
	, pollingitem(nullptr)
	, lastitem(nullptr)
	, record_next(false)
{
}

menu_input::~menu_input()
{
}

/*-------------------------------------------------
    toggle_none_default - toggle between "NONE"
    and the default item
-------------------------------------------------*/

void menu_input::toggle_none_default(input_seq &selected_seq, input_seq &original_seq, const input_seq &selected_defseq)
{
	/* if we used to be "none", toggle to the default value */
	if (original_seq.length() == 0)
		selected_seq = selected_defseq;

	/* otherwise, toggle to "none" */
	else
		selected_seq.reset();
}

void menu_input::handle()
{
	input_item_data *seqchangeditem = nullptr;
	const event *menu_event;
	int invalidate = false;

	/* process the menu */
	menu_event = process((pollingitem != nullptr) ? PROCESS_NOKEYS : 0);

	/* if we are polling, handle as a special case */
	if (pollingitem != nullptr)
	{
		input_item_data *item = pollingitem;

		/* if UI_CANCEL is pressed, abort */
		if (machine().ui_input().pressed(IPT_UI_CANCEL))
		{
			pollingitem = nullptr;
			record_next = false;
			toggle_none_default(item->seq, starting_seq, *item->defseq);
			seqchangeditem = item;
		}

		/* poll again; if finished, update the sequence */
		if (machine().input().seq_poll())
		{
			pollingitem = nullptr;
			record_next = true;
			item->seq = machine().input().seq_poll_final();
			seqchangeditem = item;
		}
	}

	/* otherwise, handle the events */
	else if (menu_event != nullptr && menu_event->itemref != nullptr)
	{
		input_item_data *item = (input_item_data *)menu_event->itemref;
		switch (menu_event->iptkey)
		{
			/* an item was selected: begin polling */
			case IPT_UI_SELECT:
				pollingitem = item;
				lastitem = item;
				starting_seq = item->seq;
				machine().input().seq_poll_start((item->type == INPUT_TYPE_ANALOG) ? ITEM_CLASS_ABSOLUTE : ITEM_CLASS_SWITCH, record_next ? &item->seq : nullptr);
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
		if (item != lastitem)
			record_next = false;
		lastitem = item;
	}

	/* if the sequence changed, update it */
	if (seqchangeditem != nullptr)
	{
		update_input(seqchangeditem);

		/* invalidate the menu to force an update */
		invalidate = true;
	}

	/* if the menu is invalidated, clear it now */
	if (invalidate)
	{
		reset(reset_options::REMEMBER_POSITION);
	}
}

void menu_input_general::update_input(struct input_item_data *seqchangeditem)
{
	const input_type_entry *entry = (const input_type_entry *)seqchangeditem->ref;
	machine().ioport().set_type_seq(entry->type(), entry->player(), seqchangeditem->seqtype, seqchangeditem->seq);
}

void menu_input_specific::update_input(struct input_item_data *seqchangeditem)
{
	ioport_field::user_settings settings;

	((ioport_field *)seqchangeditem->ref)->get_user_settings(settings);
	settings.seq[seqchangeditem->seqtype] = seqchangeditem->seq;
	((ioport_field *)seqchangeditem->ref)->set_user_settings(settings);
}


//-------------------------------------------------
//  populate_sorted - take a sorted list of
//  input_item_data objects and build up the
//  menu from them
//-------------------------------------------------

void menu_input::populate_sorted()
{
	const char *nameformat[INPUT_TYPE_TOTAL] = { nullptr };
	std::string subtext;
	std::string prev_owner;
	bool first_entry = true;

	// create a mini lookup table for name format based on type
	nameformat[INPUT_TYPE_DIGITAL] = "%s";
	nameformat[INPUT_TYPE_ANALOG] = "%s Analog";
	nameformat[INPUT_TYPE_ANALOG_INC] = "%s Analog Inc";
	nameformat[INPUT_TYPE_ANALOG_DEC] = "%s Analog Dec";

	// build the menu
	for (input_item_data &item : data)
	{
		uint32_t flags = 0;

		// generate the name of the item itself, based off the base name and the type
		assert(nameformat[item.type] != nullptr);

		if (item.owner_name && strcmp(item.owner_name, prev_owner.c_str()) != 0)
		{
			if (first_entry)
				first_entry = false;
			else
				item_append(menu_item_type::SEPARATOR);
			item_append(string_format("[root%s]", item.owner_name), "", 0, nullptr);
			prev_owner.assign(item.owner_name);
		}

		std::string text = string_format(nameformat[item.type], item.name);
		if (item.is_optional)
			text = "(" + text + ")";

		/* if we're polling this item, use some spaces with left/right arrows */
		if (&item == pollingitem)
		{
			subtext = "   ";
			flags |= FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW;
		}

		/* otherwise, generate the sequence name and invert it if different from the default */
		else
		{
			subtext = machine().input().seq_name(item.seq);
			flags |= (item.seq != *item.defseq) ? FLAG_INVERT : 0;
		}

		/* add the item */
		item_append(std::move(text), std::move(subtext), flags, &item);
	}
}

} // namespace ui
