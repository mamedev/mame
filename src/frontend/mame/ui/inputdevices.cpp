// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/inputdevices.cpp

    Input devices menu.

***************************************************************************/

#include "emu.h"
#include "inputdevices.h"

#include "inputdev.h"

// FIXME: allow OSD module headers to be included in a less ugly way
#include "../osd/modules/lib/osdlib.h"


namespace ui {

namespace {

class menu_input_device : public menu
{
public:
	menu_input_device(mame_ui_manager &mui, render_container &container, input_device &device)
		: menu(mui, container)
		, m_device(device)
		, m_have_analog(false)
	{
		set_heading(
				util::string_format(_("menu-inputdev", "%1$s (%2$s %3$d)"),
					device.name(),
					machine().input().device_class(device.devclass()).name(),
					device.devindex() + 1));
	}

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override
	{
		menu::recompute_metrics(width, height, aspect);

		set_custom_space(0.0F, (line_height() * (m_have_analog ? 2.0F : 1.0F)) + (tb_border() * 3.0F));
	}

	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override
	{
		if (selectedref)
		{
			// get the complete token for the highlighted input
			input_device_item &input = *reinterpret_cast<input_device_item *>(selectedref);
			input_code code = input.code();
			if (!machine().input().device_class(m_device.devclass()).multi())
				code.set_device_index(0);
			std::string const token = machine().input().code_to_token(code);

			// measure the name of the token string
			float const tokenwidth = (std::min)(get_string_width(token) + (gutter_width() * 2.0F), 1.0F);
			float const boxwidth = (std::max)(tokenwidth, origx2 - origx1);
			rgb_t const fgcolor(ui().colors().text_color());

			// draw the outer box
			ui().draw_outlined_box(
					container(),
					(1.0F - boxwidth) * 0.5F, origy2 + tb_border(),
					(1.0F + boxwidth) * 0.5F, origy2 + bottom,
					ui().colors().background_color());

			// show the token
			draw_text_normal(
					token,
					(1.0F - boxwidth) * 0.5F, origy2 + (tb_border() * 2.0F), boxwidth,
					text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE,
					fgcolor);

			// first show the token
			switch (input.itemclass())
			{
			case ITEM_CLASS_ABSOLUTE:
			case ITEM_CLASS_RELATIVE:
				{
					// draw the indicator
					float const indleft = origx1 + gutter_width();
					float const indright = origx2 - gutter_width();
					float const indtop = origy2 + (tb_border() * 2.0F) + (line_height() * 1.2F);
					float const indbottom = origy2 + (tb_border() * 2.0F) + (line_height() * 1.8F);
					float const indcentre = (origx1 + origx2) * 0.5F;
					s32 const value = (input.itemclass() == ITEM_CLASS_ABSOLUTE) ? input.read_as_absolute(ITEM_MODIFIER_NONE) : input.read_as_relative(ITEM_MODIFIER_NONE);
					if (0 < value)
					{
						float const fillright = indcentre + (float(value) / float(osd::input_device::ABSOLUTE_MAX) * (indright - indcentre));
						container().add_rect(indcentre, indtop, (std::min)(fillright, indright), indbottom, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					}
					else if (0 > value)
					{
						float const fillleft = indcentre - (float(value) / float(osd::input_device::ABSOLUTE_MIN) * (indcentre - indleft));
						container().add_rect((std::max)(fillleft, indleft), indtop, indcentre, indbottom, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					}
					container().add_line(indleft, indtop, indright, indtop, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					container().add_line(indright, indtop, indright, indbottom, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					container().add_line(indright, indbottom, indleft, indbottom, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					container().add_line(indleft, indbottom, indleft, indtop, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					container().add_line(indcentre, indtop, indcentre, indbottom, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
				}
				break;
			default:
				break;
			}
		}
	}

private:
	virtual void populate() override
	{
		item_append(_("menu-inputdev", "Copy Device ID"), 0U, nullptr);
		item_append(menu_item_type::SEPARATOR);

		for (input_item_id itemid = ITEM_ID_FIRST_VALID; m_device.maxitem() >= itemid; ++itemid)
		{
			input_device_item *const input = m_device.item(itemid);
			if (input)
			{
				switch (input->itemclass())
				{
				case ITEM_CLASS_ABSOLUTE:
				case ITEM_CLASS_RELATIVE:
					m_have_analog = true;
					break;
				default:
					break;
				}
				item_append(input->name(), format_value(*input), 0U, input);
			}
		}

		item_append(menu_item_type::SEPARATOR);

		set_custom_space(0.0F, (line_height() * (m_have_analog ? 2.0F : 1.0F)) + (tb_border() * 3.0F));
	}

	virtual bool handle(event const *ev) override
	{
		// FIXME: hacky, depending on first item being "copy ID", but need a better model for item reference values
		if (ev && ev->item && (IPT_UI_SELECT == ev->iptkey) && (&item(0) == ev->item))
		{
			if (!osd_set_clipboard_text(m_device.id()))
				machine().popmessage(_("menu-inputdev", "Copied device ID to clipboard"));
			else
				machine().popmessage(_("menu-inputdev", "Error copying device ID to clipboard"));
		}

		bool updated = false;
		for (int i = 0; item_count() > i; ++i)
		{
			void *const ref(item(i).ref());
			if (ref)
			{
				input_device_item &input = *reinterpret_cast<input_device_item *>(ref);
				std::string value(format_value(input));
				if (item(i).subtext() != value)
				{
					item(i).set_subtext(std::move(value));
					updated = true;
				}
			}
		}

		return updated;
	}

	static std::string format_value(input_device_item &input)
	{
		switch (input.itemclass())
		{
		default:
		case ITEM_CLASS_SWITCH:
			return util::string_format("%d", input.read_as_switch(ITEM_MODIFIER_NONE));
		case ITEM_CLASS_ABSOLUTE:
			return util::string_format("%d", input.read_as_absolute(ITEM_MODIFIER_NONE));
		case ITEM_CLASS_RELATIVE:
			return util::string_format("%d", input.read_as_relative(ITEM_MODIFIER_NONE));
		}
	}

	input_device &m_device;
	bool m_have_analog;
};

} // anonymous namespace



menu_input_devices::menu_input_devices(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
{
	set_heading(_("menu-inputdev", "Input Devices"));
}


menu_input_devices::~menu_input_devices()
{
}


void menu_input_devices::populate()
{
	// iterate input device classes and devices within each class
	bool found = false;
	for (input_device_class classno = DEVICE_CLASS_FIRST_VALID; DEVICE_CLASS_LAST_VALID >= classno; ++classno)
	{
		input_class &devclass = machine().input().device_class(classno);
		if (devclass.enabled())
		{
			bool first = true;
			for (int devnum = 0; devclass.maxindex() >= devnum; ++devnum)
			{
				input_device *const device = devclass.device(devnum);
				if (device)
				{
					// add a device class heading
					found = true;
					if (first)
					{
						first = false;
						item_append(devclass.name(), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
					}

					// add the item for the device itself
					item_append(util::string_format("%d", device->devindex() + 1), device->name(), 0U, device);
				}
			}
		}
	}

	// highly unlikely - at least one keyboard or mouse will be enabled in almost all cases
	if (!found)
		item_append(_("menu-inputdev", "[no input devices are enabled]"), FLAG_DISABLE, nullptr);

	item_append(menu_item_type::SEPARATOR);
}


bool menu_input_devices::handle(event const *ev)
{
	if (!ev || !ev->itemref)
		return false;

	input_device &dev = *reinterpret_cast<input_device *>(ev->itemref);
	switch (ev->iptkey)
	{
	case IPT_UI_SELECT:
		stack_push<menu_input_device>(ui(), container(), dev);
		break;

	case IPT_UI_PREV_GROUP:
		{
			auto group = dev.devclass();
			bool found_break = false;
			int target = 0;
			for (auto i = selected_index(); 0 < i--; )
			{
				input_device *const candidate = reinterpret_cast<input_device *>(item(i).ref());
				if (candidate)
				{
					if (candidate->devclass() == group)
					{
						target = i;
					}
					else if (!found_break)
					{
						group = candidate->devclass();
						found_break = true;
						target = i;
					}
					else
					{
						set_selected_index(target);
						return true;
					}
				}
				if (!i && found_break)
				{
					set_selected_index(target);
					return true;
				}
			}
		}
		break;

	case IPT_UI_NEXT_GROUP:
		{
			auto const group = dev.devclass();
			for (auto i = selected_index(); item_count() > ++i; )
			{
				input_device *const candidate = reinterpret_cast<input_device *>(item(i).ref());
				if (candidate && (candidate->devclass() != group))
				{
					set_selected_index(i);
					return true;
				}
			}
		}
		break;
	}

	return false;
}

} // namespace ui
