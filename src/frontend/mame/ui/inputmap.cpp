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
	set_heading(_("Input Assignments (general)"));
}

menu_input_groups::~menu_input_groups()
{
}

void menu_input_groups::populate()
{
	// build up the menu
	item_append(_("User Interface"), 0, (void *)uintptr_t(IPG_UI + 1));
	for (int player = 0; player < MAX_PLAYERS; player++)
	{
		auto s = string_format(_("Player %1$d Controls"), player + 1);
		item_append(s, 0, (void *)uintptr_t(IPG_PLAYER1 + player + 1));
	}
	item_append(_("Other Controls"), 0, (void *)uintptr_t(IPG_OTHER + 1));
	item_append(menu_item_type::SEPARATOR);
}

bool menu_input_groups::handle(event const *ev)
{
	if (ev && (ev->iptkey == IPT_UI_SELECT))
	{
		menu::stack_push<menu_input_general>(
				ui(),
				container(),
				int(uintptr_t(ev->itemref) - 1),
				util::string_format(_("Input Assignments (%1$s)"), ev->item->text()));
	}

	return false;
}


/*-------------------------------------------------
    menu_input_general - handle the general
    input menu
-------------------------------------------------*/

menu_input_general::menu_input_general(mame_ui_manager &mui, render_container &container, int _group, std::string &&heading)
	: menu_input(mui, container)
	, group(_group)
{
	set_heading(std::move(heading));
}

menu_input_general::~menu_input_general()
{
}

void menu_input_general::menu_activated()
{
	// scripts can change settings out from under us
	reset(reset_options::REMEMBER_POSITION);
}

void menu_input_general::populate()
{
	if (data.empty())
	{
		assert(!pollingitem);

		// iterate over the input ports and add menu items
		for (const input_type_entry &entry : machine().ioport().types())
		{
			// add if we match the group and we have a valid name
			if (entry.group() == group)
			{
				std::string name = entry.name();
				if (!name.empty())
				{
					// loop over all sequence types
					for (input_seq_type seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; ++seqtype)
					{
						// build an entry for the standard sequence
						input_item_data &item(data.emplace_back());
						item.ref = &entry;
						item.seqtype = seqtype;
						item.seq = machine().ioport().type_seq(entry.type(), entry.player(), seqtype);
						item.defseq = &entry.defseq(seqtype);
						item.group = entry.group();
						item.type = ioport_manager::type_is_analog(entry.type()) ? (INPUT_TYPE_ANALOG + seqtype) : INPUT_TYPE_DIGITAL;
						item.is_optional = false;
						item.name = name;
						item.owner = nullptr;

						// stop after one, unless we're analog
						if (item.type == INPUT_TYPE_DIGITAL)
							break;
					}
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
	item_append(menu_item_type::SEPARATOR);
}

void menu_input_general::update_input(input_item_data &seqchangeditem)
{
	const input_type_entry &entry = *reinterpret_cast<const input_type_entry *>(seqchangeditem.ref);
	machine().ioport().set_type_seq(entry.type(), entry.player(), seqchangeditem.seqtype, seqchangeditem.seq);
	seqchangeditem.seq = machine().ioport().type_seq(entry.type(), entry.player(), seqchangeditem.seqtype);
}


/*-------------------------------------------------
    menu_input_specific - handle the game-specific
    input menu
-------------------------------------------------*/

menu_input_specific::menu_input_specific(mame_ui_manager &mui, render_container &container) : menu_input(mui, container)
{
	set_heading(_("Input Assignments (this system)"));
}

menu_input_specific::~menu_input_specific()
{
}

void menu_input_specific::menu_activated()
{
	// scripts can change settings out from under us
	assert(!pollingitem);
	data.clear();
	reset(reset_options::REMEMBER_POSITION);
}

void menu_input_specific::populate()
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
						input_item_data &item(data.emplace_back());
						item.ref = &field;
						item.seqtype = seqtype;
						item.seq = field.seq(seqtype);
						item.defseq = &field.defseq(seqtype);
						item.group = machine().ioport().type_group(field.type(), field.player());
						item.type = field.is_analog() ? (INPUT_TYPE_ANALOG + seqtype) : INPUT_TYPE_DIGITAL;
						item.is_optional = field.optional();
						item.name = field.name();
						item.owner = &field.device();

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
					int cmp = strcmp(i1.owner->tag(), i2.owner->tag());
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
					cmp = i1.name.compare(i2.name);
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
	if (!data.empty())
		populate_sorted();
	else
		item_append(_("[no assignable inputs are enabled]"), FLAG_DISABLE, nullptr);

	item_append(menu_item_type::SEPARATOR);
}

void menu_input_specific::update_input(input_item_data &seqchangeditem)
{
	ioport_field::user_settings settings;

	// yeah, the const_cast is naughty, but we know we stored a non-const reference in it
	ioport_field const &field(*reinterpret_cast<ioport_field const *>(seqchangeditem.ref));
	field.get_user_settings(settings);
	settings.seq[seqchangeditem.seqtype] = seqchangeditem.seq;
	if (seqchangeditem.seq.is_default())
		settings.cfg[seqchangeditem.seqtype].clear();
	else if (!seqchangeditem.seq.length())
		settings.cfg[seqchangeditem.seqtype] = "NONE";
	else
		settings.cfg[seqchangeditem.seqtype] = machine().input().seq_to_tokens(seqchangeditem.seq);
	const_cast<ioport_field &>(field).set_user_settings(settings);
	seqchangeditem.seq = field.seq(seqchangeditem.seqtype);
}


/*-------------------------------------------------
    menu_input - display a menu for inputs
-------------------------------------------------*/

menu_input::menu_input(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
	, data()
	, pollingitem(nullptr)
	, seq_poll()
	, errormsg()
	, erroritem(nullptr)
	, lastitem(nullptr)
	, record_next(false)
	, modified_ticks(0)
{
	set_process_flags(PROCESS_LR_ALWAYS);
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
		selected_seq.set_default();
	else // otherwise, toggle to "none"
		selected_seq.reset();
}


void menu_input::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	// leave space for showing the input sequence below the menu
	set_custom_space(0.0F, 2.0F * line_height() + 3.0F * tb_border());
}


void menu_input::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	if (pollingitem)
	{
		const std::string seqname = machine().input().seq_name(seq_poll->sequence());
		char const *const text[] = { seqname.c_str() };
		draw_text_box(
				std::begin(text), std::end(text),
				origx1, origx2, origy2 + tb_border(), origy2 + bottom,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, false,
				ui().colors().text_color(), ui().colors().background_color());
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
					origx1, origx2, origy2 + tb_border(), origy2 + bottom,
					text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, false,
					ui().colors().text_color(), UI_RED_COLOR);
		}
		else if (selectedref)
		{
			const input_item_data &item = *reinterpret_cast<input_item_data *>(selectedref);
			if ((INPUT_TYPE_ANALOG != item.type) && machine().input().seq_pressed(item.seq))
			{
				char const *const text[] = { _("Pressed") };
				draw_text_box(
						std::begin(text), std::end(text),
						origx1, origx2, origy2 + tb_border(), origy2 + bottom,
						text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, false,
						ui().colors().text_color(), ui().colors().background_color());
			}
			else
			{
				char const *const text[] = {
					record_next ? appendprompt.c_str() : assignprompt.c_str(),
					(!item.seq.empty() || item.defseq->empty()) ? clearprompt.c_str() : defaultprompt.c_str() };
				draw_text_box(
						std::begin(text), std::end(text),
						origx1, origx2, origy2 + tb_border(), origy2 + bottom,
						text_layout::text_justify::CENTER, text_layout::word_wrapping::NEVER, false,
						ui().colors().text_color(), ui().colors().background_color());
			}
		}
	}
}

bool menu_input::handle(event const *ev)
{
	input_item_data *seqchangeditem = nullptr;
	bool invalidate = false;
	bool redraw = false;

	// process the menu
	if (pollingitem)
	{
		// if we are polling, handle as a special case
		input_item_data *const item = pollingitem;

		// prevent race condition between ui_input().pressed() and poll()
		if (modified_ticks == 0 && seq_poll->modified())
			modified_ticks = osd_ticks();

		if (machine().ui_input().pressed(IPT_UI_CANCEL))
		{
			// if UI_CANCEL is pressed, abort and abandon changes
			pollingitem = nullptr;
			set_process_flags(PROCESS_LR_ALWAYS);
			invalidate = true;
			seq_poll.reset();
			machine().ui_input().reset();
		}
		else if (seq_poll->poll()) // poll again; if finished, update the sequence
		{
			pollingitem = nullptr;
			set_process_flags(PROCESS_LR_ALWAYS);
			if (seq_poll->valid())
			{
				record_next = true;
				item->seq = seq_poll->sequence();
				seqchangeditem = item;
			}
			else
			{
				// entered invalid sequence - abandon change
				invalidate = true;
				errormsg = _("Invalid combination entered");
				erroritem = item;
			}
			seq_poll.reset();
			machine().ui_input().reset();
		}
		else
		{
			// always redraw to ensure it updates as soon as possible in response to changes
			redraw = true;
		}
	}
	else if (ev && ev->itemref)
	{
		// otherwise, handle the events
		input_item_data &item = *reinterpret_cast<input_item_data *>(ev->itemref);
		input_item_data *newsel = &item;
		switch (ev->iptkey)
		{
		case IPT_UI_SELECT: // an item was selected: begin polling
			set_process_flags(PROCESS_NOKEYS);
			errormsg.clear();
			erroritem = nullptr;
			modified_ticks = 0;
			pollingitem = &item;
			lastitem = &item;
			starting_seq = item.seq;
			if (INPUT_TYPE_ANALOG == item.type)
				seq_poll.reset(new axis_sequence_poller(machine().input()));
			else
				seq_poll.reset(new switch_sequence_poller(machine().input()));
			if (record_next)
				seq_poll->start(item.seq);
			else
				seq_poll->start();
			invalidate = true;
			break;

		case IPT_UI_CLEAR: // if the clear key was pressed, reset the selected item
			errormsg.clear();
			erroritem = nullptr;
			toggle_none_default(item.seq, item.seq, *item.defseq);
			record_next = false;
			seqchangeditem = &item;
			break;

		case IPT_UI_PREV_GROUP:
			{
				auto current = std::distance(data.data(), &item);
				bool found_break = false;
				while (0 < current)
				{
					if (!found_break)
					{
						if (data[--current].owner != item.owner)
							found_break = true;
					}
					else if (data[current].owner != data[current - 1].owner)
					{
						newsel = &data[current];
						set_selection(newsel);
						set_top_line(selected_index() - 1);
						break;
					}
					else
					{
						--current;
					}
					if (found_break && !current)
					{
						newsel = &data[current];
						set_selection(newsel);
						set_top_line(selected_index() - 1);
						break;
					}
				}
			}
			break;

		case IPT_UI_NEXT_GROUP:
			{
				auto current = std::distance(data.data(), &item);
				while (data.size() > ++current)
				{
					if (data[current].owner != item.owner)
					{
						newsel = &data[current];
						set_selection(newsel);
						set_top_line(selected_index() - 1);
						break;
					}
				}
			}
			break;
		}

		// if the selection changed, reset the "record next" flag
		if (newsel != lastitem)
		{
			if (erroritem)
			{
				errormsg.clear();
				erroritem = nullptr;
			}
			record_next = false;
			lastitem = &item;
			redraw = true;
		}

		// flip between set and append
		// not very discoverable, but with the prompt it isn't completely opaque
		if ((IPT_UI_LEFT == ev->iptkey) || (IPT_UI_RIGHT == ev->iptkey))
		{
			if (erroritem)
			{
				errormsg.clear();
				erroritem = nullptr;
			}
			else if (record_next || !item.seq.empty())
			{
				record_next = !record_next;
			}
			redraw = true;
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

	return redraw && !invalidate;
}


//-------------------------------------------------
//  populate_sorted - take a sorted list of
//  input_item_data objects and build up the
//  menu from them
//-------------------------------------------------

void menu_input::populate_sorted()
{
	const char *nameformat[INPUT_TYPE_TOTAL] = { nullptr };

	// create a mini lookup table for name format based on type
	nameformat[INPUT_TYPE_DIGITAL] = "%1$s";
	nameformat[INPUT_TYPE_ANALOG] = _("input-name", "%1$s Analog");
	nameformat[INPUT_TYPE_ANALOG_INC] = _("input-name", "%1$s Analog Inc");
	nameformat[INPUT_TYPE_ANALOG_DEC] = _("input-name", "%1$s Analog Dec");

	// build the menu
	std::string text, subtext;
	const device_t *prev_owner = nullptr;
	for (input_item_data &item : data)
	{
		// generate the name of the item itself, based off the base name and the type
		assert(nameformat[item.type] != nullptr);

		if (item.owner && (item.owner != prev_owner))
		{
			if (item.owner->owner())
				item_append(string_format(_("%1$s [root%2$s]"), item.owner->type().fullname(), item.owner->tag()), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
			else
				item_append(string_format(_("[root%1$s]"), item.owner->tag()), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
			prev_owner = item.owner;
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

	// pre-format messages
	assignprompt = util::string_format(_("Press %1$s to set\n"), ui().get_general_input_setting(IPT_UI_SELECT));
	appendprompt = util::string_format(_("Press %1$s to append\n"), ui().get_general_input_setting(IPT_UI_SELECT));
	clearprompt = util::string_format(_("Press %1$s to clear\n"), ui().get_general_input_setting(IPT_UI_CLEAR));
	defaultprompt = util::string_format(_("Press %1$s to restore default\n"), ui().get_general_input_setting(IPT_UI_CLEAR));
}

} // namespace ui
