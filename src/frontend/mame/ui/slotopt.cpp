// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

    ui/slotopt.cpp

    Internal menu for the slot options.

*********************************************************************/

#include "emu.h"

#include "ui/ui.h"
#include "ui/slotopt.h"
#include "ui/devopt.h"

#include "emuopts.h"
#include "mameopts.h"

#include <algorithm>


/***************************************************************************
    UTILITY
***************************************************************************/

namespace {

// constants
void *const ITEMREF_RESET = ((void *)1);
constexpr char const DIVIDER[] = "------";

} // anonymous namespace


/***************************************************************************
    SLOT MENU
***************************************************************************/

namespace ui {

//-------------------------------------------------
//  menu_slot_devices constructor
//-------------------------------------------------

menu_slot_devices::menu_slot_devices(mame_ui_manager &mui, render_container &container) : menu(mui, container)
{
}

//-------------------------------------------------
//  menu_slot_devices destructor
//-------------------------------------------------

menu_slot_devices::~menu_slot_devices()
{
}


//-------------------------------------------------
//  get_current_option - returns the current
//  slot option
//-------------------------------------------------

device_slot_interface::slot_option const *menu_slot_devices::get_current_option(device_slot_interface &slot) const
{
	std::string current;

	if (!slot.fixed())
	{
		char const *const slot_option_name = slot.slot_name();
		current = machine().options().slot_option(slot_option_name).value();
	}
	else
	{
		if (!slot.default_option())
			return nullptr;
		current.assign(slot.default_option());
	}

	return slot.option(current.c_str());
}


//-------------------------------------------------
//  set_slot_device
//-------------------------------------------------

void menu_slot_devices::set_slot_device(device_slot_interface &slot, std::string_view val)
{
	// we might change slot options; in the spirit of user friendliness, we should record all current
	// options
	record_current_options();

	// find the slot option
	slot_option &opt(machine().options().slot_option(slot.slot_name()));

	// specify it
	opt.specify(val);

	// erase this from our recorded options list - this is the slot we're trying to change!
	m_slot_options.erase(slot.slot_name());

	// refresh any options that we might have annotated earlier
	while (try_refresh_current_options())
		;

	// changing the options may result in options changing; we need to reset
	reset(reset_options::REMEMBER_POSITION);
}


//-------------------------------------------------
//  record_current_options
//-------------------------------------------------

void menu_slot_devices::record_current_options()
{
	for (device_slot_interface &slot : slot_interface_enumerator(m_config->root_device()))
	{
		// we're doing this out of a desire to honor user-selectable options; therefore it only
		// makes sense to record values for selectable options
		if (slot.has_selectable_options())
		{
			// get the slot option
			const slot_option &opt(machine().options().slot_option(slot.slot_name()));

			// and record the value in our local cache
			m_slot_options[slot.slot_name()] = opt.specified_value();
		}
	}
}


//-------------------------------------------------
//  try_refresh_current_options
//-------------------------------------------------

bool menu_slot_devices::try_refresh_current_options()
{
	// enumerate through all slot options that we've tracked
	for (const auto &opt : m_slot_options)
	{
		// do we have a value different than what we're tracking?
		slot_option *slotopt = machine().options().find_slot_option(opt.first);
		if (slotopt && slotopt->specified_value() != opt.second)
		{
			// specify this option (but catch errors)
			try
			{
				slotopt->specify(opt.second);

				// the option was successfully specified; it isn't safe to continue
				// checking slots as the act of specifying the slot may have drastically
				// changed the options list
				return true;
			}
			catch (options_exception &)
			{
				// this threw an exception - that is fine; we can just proceed
			}
		}
	}

	// we've went through all options without changing anything
	return false;
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_slot_devices::populate(float &customtop, float &custombottom)
{
	// we need to keep our own copy of the machine_config because we
	// can change this out from under the caller
	m_config = std::make_unique<machine_config>(machine().system(), machine().options());

	// cycle through all devices for this system
	for (device_slot_interface &slot : slot_interface_enumerator(m_config->root_device()))
	{
		// does this slot have any selectable options?
		bool has_selectable_options = slot.has_selectable_options();

		// name this option
		std::string opt_name(DIVIDER);
		device_slot_interface::slot_option const *option = get_current_option(slot);
		if (option)
		{
			opt_name = has_selectable_options
					? option->name()
					: string_format(_("%s [internal]"), option->name());
		}

		// choose item flags
		uint32_t const item_flags = has_selectable_options
				? FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW
				: FLAG_DISABLE;

		item_append(slot.slot_name(), opt_name, item_flags, (void *)&slot);
	}
	item_append(menu_item_type::SEPARATOR);
	item_append(_("Reset"), 0, ITEMREF_RESET);

	// leave space for the name of the current option at the bottom
	custombottom = ui().get_line_height() + 3.0f * ui().box_tb_border();
}


//-------------------------------------------------
//  custom_render - draw extra menu content
//-------------------------------------------------

void menu_slot_devices::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	if (selectedref && (ITEMREF_RESET != selectedref))
	{
		device_slot_interface *const slot(reinterpret_cast<device_slot_interface *>(selectedref));
		device_slot_interface::slot_option const *const option(get_current_option(*slot));
		char const *const text[] = { option ? option->devtype().fullname() : _("[empty slot]") };
		draw_text_box(
				std::begin(text), std::end(text),
				origx1, origx2, origy2 + ui().box_tb_border(), origy2 + bottom,
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
				ui().colors().text_color(), ui().colors().background_color(), 1.0f);
	}
}


//-------------------------------------------------
//  handle - process an input event
//-------------------------------------------------

void menu_slot_devices::handle()
{
	// process the menu
	event const *const menu_event(process(0));

	if (menu_event && menu_event->itemref != nullptr)
	{
		if (menu_event->itemref == ITEMREF_RESET)
		{
			if (menu_event->iptkey == IPT_UI_SELECT)
				machine().schedule_hard_reset();
		}
		else if (menu_event->iptkey == IPT_UI_LEFT || menu_event->iptkey == IPT_UI_RIGHT)
		{
			device_slot_interface *slot = (device_slot_interface *)menu_event->itemref;
			rotate_slot_device(*slot, menu_event->iptkey == IPT_UI_LEFT ? step_t::PREVIOUS : step_t::NEXT);
		}
		else if (menu_event->iptkey == IPT_UI_SELECT)
		{
			device_slot_interface *slot = (device_slot_interface *)menu_event->itemref;
			device_slot_interface::slot_option const *const option = get_current_option(*slot);
			if (option)
				menu::stack_push<menu_device_config>(ui(), container(), slot, option);
		}
	}
}


//-------------------------------------------------
//  rotate_slot_device - rotates the specified slot
//-------------------------------------------------

void menu_slot_devices::rotate_slot_device(device_slot_interface &slot, menu_slot_devices::step_t step)
{
	// first, we need to make sure our cache of options is up to date
	if (m_current_option_list_slot_tag != slot.device().tag())
	{
		device_slot_interface::slot_option const *current = get_current_option(slot);

		// build the option list, including the blank option
		m_current_option_list.clear();
		m_current_option_list.emplace_back("");
		for (const auto &ent : slot.option_list())
		{
			if (ent.second->selectable())
				m_current_option_list.emplace_back(ent.second->name());
		}

		// since the order is indeterminate, we need to sort the options
		std::sort(m_current_option_list.begin(), m_current_option_list.end());

		// find the current position
		char const *const target = current ? current->name() : "";
		m_current_option_list_iter = std::find_if(
				m_current_option_list.begin(),
				m_current_option_list.end(),
				[target] (const std::string &opt_value)
				{
					return opt_value == target;
				});

		// we expect the above search to succeed, because if an internal
		// option was selected, the menu item should be disabled
		assert(m_current_option_list_iter != m_current_option_list.end());

		// we've succeeded; don't do this again
		m_current_option_list_slot_tag.assign(slot.device().tag());
	}

	// At this point, the current option list and accompanying iterator should
	// be good; perform the rotation
	switch (step)
	{
	case step_t::NEXT:
		m_current_option_list_iter++;
		if (m_current_option_list_iter == m_current_option_list.end())
			m_current_option_list_iter = m_current_option_list.begin();
		break;

	case step_t::PREVIOUS:
		if (m_current_option_list_iter == m_current_option_list.begin())
			m_current_option_list_iter = m_current_option_list.end();
		m_current_option_list_iter--;
		break;

	default:
		throw false;
	}

	set_slot_device(slot, *m_current_option_list_iter);
}

} // namespace ui
