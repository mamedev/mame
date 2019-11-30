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
    menu_input_groups - handle the input groups
    menu
-------------------------------------------------*/

menu_input_groups::menu_input_groups(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

menu_input_groups::~menu_input_groups()
{
}

void menu_input_groups::populate(float &customtop, float &custombottom)
{
	// build up the menu
	item_append(_("User Interface"), "", 0, (void *)uintptr_t(IPG_UI + 1));
	for (int player = 0; player < MAX_PLAYERS; player++)
	{
		auto s = string_format("Player %d Controls", player + 1);
		item_append(s, "", 0, (void *)uintptr_t(IPG_PLAYER1 + player + 1));
	}
	item_append(_("Other Controls"), "", 0, (void *)uintptr_t(IPG_OTHER + 1));
}

void menu_input_groups::handle()
{
	// process the menu
	const event *const menu_event = process(0);
	if (menu_event && menu_event->iptkey == IPT_UI_SELECT)
		menu::stack_push<menu_input_general>(ui(), container(), int(uintptr_t(menu_event->itemref) - 1));
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

menu_input_general::~menu_input_general()
{
}

void menu_input_general::populate(float &customtop, float &custombottom)
{
	if (data.empty())
	{
		assert(!pollingitem);

		// iterate over the input ports and add menu items
		for (const input_type_entry &entry : machine().ioport().types())
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
	populate_sorted(customtop, custombottom);
}

void menu_input_general::update_input(input_item_data &seqchangeditem)
{
	const input_type_entry &entry = *reinterpret_cast<const input_type_entry *>(seqchangeditem.ref);
	machine().ioport().set_type_seq(entry.type(), entry.player(), seqchangeditem.seqtype, seqchangeditem.seq);
}


/*-------------------------------------------------
    menu_input_specific - handle the game-specific
    input menu
-------------------------------------------------*/

menu_input_specific::menu_input_specific(mame_ui_manager &mui, render_container &container) : menu_input(mui, container)
{
}

menu_input_specific::~menu_input_specific()
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
	populate_sorted(customtop, custombottom);
}

void menu_input_specific::update_input(input_item_data &seqchangeditem)
{
	ioport_field::user_settings settings;

	// yeah, the const_cast is naughty, but we know we stored a non-const reference in it
	reinterpret_cast<const ioport_field *>(seqchangeditem.ref)->get_user_settings(settings);
	settings.seq[seqchangeditem.seqtype] = seqchangeditem.seq;
	reinterpret_cast<ioport_field *>(const_cast<void *>(seqchangeditem.ref))->set_user_settings(settings);
}


/*-------------------------------------------------
    menu_input - display a menu for inputs
-------------------------------------------------*/
menu_input::menu_input(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
	, data()
	, pollingitem(nullptr)
	, seq_poll(machine().input())
	, errormsg()
	, erroritem(nullptr)
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
	if (original_seq.empty()) // if we used to be "none", toggle to the default value
		selected_seq = selected_defseq;
	else // otherwise, toggle to "none"
		selected_seq.reset();
}

void menu_input::custom_render(void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
	if (pollingitem)
	{
		const std::string seqname = machine().input().seq_name(seq_poll.sequence());
		char const *const text[] = { seqname.c_str() };
		draw_text_box(
				std::begin(text), std::end(text),
				x1, x2, y2 + ui().box_tb_border(), y2 + bottom,
				ui::text_layout::CENTER, ui::text_layout::NEVER, false,
				ui().colors().text_color(), ui().colors().background_color(), 1.0f);
	}
	else
	{
		if (erroritem && (selectedref != erroritem))
		{
			errormsg.clear();
			erroritem = nullptr;
		}

		if (erroritem)
		{
			char const *const text[] = { errormsg.c_str() };
			draw_text_box(
					std::begin(text), std::end(text),
					x1, x2, y2 + ui().box_tb_border(), y2 + bottom,
					ui::text_layout::CENTER, ui::text_layout::NEVER, false,
					ui().colors().text_color(), UI_RED_COLOR, 1.0f);
		}
		else if (selectedref)
		{
			const input_item_data &item = *reinterpret_cast<input_item_data *>(selectedref);
			if ((INPUT_TYPE_ANALOG != item.type) && machine().input().seq_pressed(item.seq))
			{
				char const *const text[] = { _("Pressed") };
				draw_text_box(
						std::begin(text), std::end(text),
						x1, x2, y2 + ui().box_tb_border(), y2 + bottom,
						ui::text_layout::CENTER, ui::text_layout::NEVER, false,
						ui().colors().text_color(), ui().colors().background_color(), 1.0f);
			}
		}
	}
}

void menu_input::handle()
{
	input_item_data *seqchangeditem = nullptr;
	bool invalidate = false;

	// process the menu
	const event *const menu_event = process((pollingitem != nullptr) ? PROCESS_NOKEYS : 0);
	if (pollingitem)
	{
		// if we are polling, handle as a special case
		input_item_data *const item = pollingitem;

		if (machine().ui_input().pressed(IPT_UI_CANCEL))
		{
			// if UI_CANCEL is pressed, abort
			pollingitem = nullptr;
			if (!seq_poll.modified())
			{
				// cancelled immediately - toggle between default and none
				record_next = false;
				toggle_none_default(item->seq, starting_seq, *item->defseq);
				seqchangeditem = item;
			}
			else
			{
				// entered something before cancelling - abandon change
				invalidate = true;
			}
		}
		else if (seq_poll.poll()) // poll again; if finished, update the sequence
		{
			pollingitem = nullptr;
			if (seq_poll.valid())
			{
				record_next = true;
				item->seq = seq_poll.sequence();
				seqchangeditem = item;
			}
			else
			{
				// entered invalid sequence - abandon change
				invalidate = true;
				errormsg = _("Invalid sequence entered");
				erroritem = item;
			}
		}
	}
	else if (menu_event && menu_event->itemref)
	{
		// otherwise, handle the events
		input_item_data &item = *reinterpret_cast<input_item_data *>(menu_event->itemref);
		switch (menu_event->iptkey)
		{
		case IPT_UI_SELECT: // an item was selected: begin polling
			errormsg.clear();
			erroritem = nullptr;
			pollingitem = &item;
			lastitem = &item;
			starting_seq = item.seq;
			if (record_next)
				seq_poll.start((item.type == INPUT_TYPE_ANALOG) ? ITEM_CLASS_ABSOLUTE : ITEM_CLASS_SWITCH, item.seq);
			else
				seq_poll.start((item.type == INPUT_TYPE_ANALOG) ? ITEM_CLASS_ABSOLUTE : ITEM_CLASS_SWITCH);
			invalidate = true;
			break;

		case IPT_UI_CLEAR: // if the clear key was pressed, reset the selected item
			errormsg.clear();
			erroritem = nullptr;
			toggle_none_default(item.seq, item.seq, *item.defseq);
			record_next = false;
			seqchangeditem = &item;
			break;
		}

		// if the selection changed, reset the "record next" flag
		if (&item != lastitem)
		{
			if (erroritem)
			{
				errormsg.clear();
				erroritem = nullptr;
			}
			record_next = false;
			lastitem = &item;
		}
	}

	// if the sequence changed, update it
	if (seqchangeditem)
	{
		update_input(*seqchangeditem);

		// invalidate the menu to force an update
		invalidate = true;
	}

	// if the menu is invalidated, clear it now
	if (invalidate)
		reset(reset_options::REMEMBER_POSITION);
}


//-------------------------------------------------
//  populate_sorted - take a sorted list of
//  input_item_data objects and build up the
//  menu from them
//-------------------------------------------------

void menu_input::populate_sorted(float &customtop, float &custombottom)
{
	const char *nameformat[INPUT_TYPE_TOTAL] = { nullptr };

	// create a mini lookup table for name format based on type
	nameformat[INPUT_TYPE_DIGITAL] = "%s";
	nameformat[INPUT_TYPE_ANALOG] = "%s Analog";
	nameformat[INPUT_TYPE_ANALOG_INC] = "%s Analog Inc";
	nameformat[INPUT_TYPE_ANALOG_DEC] = "%s Analog Dec";

	// build the menu
	std::string text, subtext;
	std::string prev_owner;
	bool first_entry = true;
	for (input_item_data &item : data)
	{
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

		text = string_format(nameformat[item.type], item.name);
		if (item.is_optional)
			text = "(" + text + ")";

		uint32_t flags = 0;
		if (&item == pollingitem)
		{
			// if we're polling this item, use some spaces with left/right arrows
			subtext = "   ";
			flags |= FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW;
		}
		else
		{
			// otherwise, generate the sequence name and invert it if different from the default
			subtext = machine().input().seq_name(item.seq);
			flags |= (item.seq != *item.defseq) ? FLAG_INVERT : 0;
		}

		// add the item
		item_append(std::move(text), std::move(subtext), flags, &item);
	}

	// leave space for showing the input sequence below the menu
	custombottom = ui().get_line_height() + 3.0f * ui().box_tb_border();
}

} // namespace ui
