// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/inputtoggle.cpp

    Toggle inputs menu.

***************************************************************************/

#include "emu.h"
#include "ui/inputtoggle.h"

#include <iterator>


namespace ui {

menu_input_toggles::menu_input_toggles(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
{
	set_heading(_("menu-inputtoggle", "Toggle Inputs"));
}


menu_input_toggles::~menu_input_toggles()
{
}


void menu_input_toggles::menu_activated()
{
	// enabled inputs and state of inputs can change while menu is inactive
	reset(reset_options::REMEMBER_REF);
}


void menu_input_toggles::populate(float &customtop, float &custombottom)
{
	// find toggle fields
	if (m_fields.empty())
	{
		for (auto &port : machine().ioport().ports())
		{
			for (ioport_field &field : port.second->fields())
			{
				switch (field.type_class())
				{
				case INPUT_CLASS_CONTROLLER:
				case INPUT_CLASS_MISC:
				case INPUT_CLASS_KEYBOARD:
					if (field.live().toggle)
						m_fields.emplace_back(field);
					break;
				default:
					break;
				}
			}
		}
	}

	// create corresponding items for enabled fields
	device_t *prev_owner = nullptr;
	for (auto &field : m_fields)
	{
		if (field.get().enabled())
		{
			// add a device heading if necessary
			if (&field.get().device() != prev_owner)
			{
				prev_owner = &field.get().device();
				if (prev_owner->owner())
					item_append(string_format(_("%1$s [root%2$s]"), prev_owner->type().fullname(), prev_owner->tag()), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
				else
					item_append(string_format(_("[root%1$s]"), prev_owner->tag()), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
			}

			// choose the display name for the value
			char const *setting;
			u32 flags = 0U;
			if (!field.get().settings().empty())
			{
				setting = field.get().setting_name();
				if (field.get().has_previous_setting())
					flags |= FLAG_LEFT_ARROW;
				if (field.get().has_next_setting())
					flags |= FLAG_RIGHT_ARROW;
			}
			else if (field.get().defvalue() == field.get().live().value)
			{
				setting = _("Off");
				flags = FLAG_RIGHT_ARROW;
			}
			else
			{
				setting = _("On");
				flags = FLAG_LEFT_ARROW;
			}

			// actually create the item
			item_append(field.get().name(), setting, flags, &field);
		}
	}

	// display a message if there are toggle inputs enabled
	if (!prev_owner)
		item_append(_("menu-inputtoggle", "[no toggle inputs are enabled]"), FLAG_DISABLE, nullptr);

	item_append(menu_item_type::SEPARATOR);
}


void menu_input_toggles::handle(event const *ev)
{
	if (ev && ev->itemref)
	{
		auto const ref = reinterpret_cast<std::reference_wrapper<ioport_field> *>(ev->itemref);
		ioport_field &field = ref->get();
		bool invalidate = false;
		switch (ev->iptkey)
		{
		case IPT_UI_SELECT: // toggle regular items, set multi-value items to default
			if (field.settings().empty())
			{
				field.live().value ^= field.mask();
				invalidate = true;
				break;
			}
			[[fallthrough]];

		case IPT_UI_CLEAR: // set to default
			if (field.defvalue() != field.live().value)
			{
				field.live().value = field.defvalue();
				invalidate = true;
			}
			break;

		case IPT_UI_LEFT: // toggle or select previous setting
			if (field.settings().empty())
				field.live().value ^= field.mask();
			else
				field.select_previous_setting();
			invalidate = true;
			break;

		case IPT_UI_RIGHT: // toggle or select next setting
			if (field.settings().empty())
				field.live().value ^= field.mask();
			else
				field.select_next_setting();
			invalidate = true;
			break;

		case IPT_UI_PREV_GROUP: // previous device if any
			{
				auto current = std::distance(m_fields.data(), ref);
				device_t const *dev = &field.device();
				bool found_break = false;
				void *candidate = nullptr;
				while (0 < current)
				{
					if (!found_break)
					{
						if (m_fields[--current].get().enabled())
						{
							device_t const *prev = &m_fields[current].get().device();
							if (prev != dev)
							{
								dev = prev;
								found_break = true;
								candidate = &m_fields[current];
							}
						}
					}
					else if (&m_fields[--current].get().device() != dev)
					{
						set_selection(candidate);
						set_top_line(selected_index() - 1);
						break;
					}
					else if (m_fields[current].get().enabled())
					{
						candidate = &m_fields[current];
					}
					if (found_break && !current)
					{
						set_selection(candidate);
						set_top_line(selected_index() - 1);
						break;
					}
				}
			}
			break;

		case IPT_UI_NEXT_GROUP: // next device if any
			{
				auto current = std::distance(m_fields.data(), ref);
				device_t const *const dev = &field.device();
				while (m_fields.size() > ++current)
				{
					if (m_fields[current].get().enabled() && (&m_fields[current].get().device() != dev))
					{
						set_selection(&m_fields[current]);
						set_top_line(selected_index() - 1);
						break;
					}
				}
			}
			break;
		}

		// changing value can enable or disable other fields
		if (invalidate)
			reset(reset_options::REMEMBER_REF);
	}
}

} // namespace ui
