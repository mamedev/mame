// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    ui/inputdevices.cpp

    Input devices menu.

***************************************************************************/

#include "emu.h"
#include "inputdevices.h"

#include "inputdev.h"


namespace ui {

namespace {

class menu_input_device : public menu
{
public:
	menu_input_device(mame_ui_manager &mui, render_container &container, input_device &device)
		: menu(mui, container)
		, m_device(device)
	{
		set_heading(
				util::string_format(_("menu-inputdev", "%1$s (%2$s %3$d)"),
					device.name(),
					machine().input().device_class(device.devclass()).name(),
					device.devindex() + 1));
	}

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override
	{
		if (selectedref)
		{
			input_device_item &input = *reinterpret_cast<input_device_item *>(selectedref);
			switch (input.itemclass())
			{
			case ITEM_CLASS_ABSOLUTE:
			case ITEM_CLASS_RELATIVE:
				{
					// draw the outer box
					ui().draw_outlined_box(container(), x, y2 + ui().box_tb_border(), x2, y2 + bottom, ui().colors().background_color());

					// draw the indicator
					rgb_t const fgcolor(ui().colors().text_color());
					float const border = ui().box_lr_border() * machine().render().ui_aspect(&container());
					float const lineheight = ui().get_line_height();
					float const indleft = x + border;
					float const indright = x2 - border;
					float const indtop = y2 + (ui().box_tb_border() * 2.0F) + (lineheight * 0.2F);
					float const indbottom = y2 + (ui().box_tb_border() * 2.0F) + (lineheight * 0.8F);
					float const indcentre = (x + x2) * 0.5F;
					s32 const value = (input.itemclass() == ITEM_CLASS_ABSOLUTE) ? input.read_as_absolute(ITEM_MODIFIER_NONE) : input.read_as_relative(ITEM_MODIFIER_NONE);
					if (0 < value)
					{
						float const fillright = indcentre + (float(value) / float(osd::INPUT_ABSOLUTE_MAX) * (indright - indcentre));
						container().add_rect(indcentre, indtop, (std::min)(fillright, indright), indbottom, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					}
					else if (0 > value)
					{
						float const fillleft = indcentre - (float(value) / float(osd::INPUT_ABSOLUTE_MIN) * (indcentre - indleft));
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
	virtual void populate(float &customtop, float &custombottom) override
	{
		bool haveanalog = false;
		for (input_item_id itemid = ITEM_ID_FIRST_VALID; m_device.maxitem() >= itemid; ++itemid)
		{
			input_device_item *const input = m_device.item(itemid);
			if (input)
			{
				switch (input->itemclass())
				{
				case ITEM_CLASS_ABSOLUTE:
				case ITEM_CLASS_RELATIVE:
					haveanalog = true;
					break;
				default:
					break;
				}
				item_append(input->name(), format_value(*input), 0U, input);
			}
		}

		item_append(menu_item_type::SEPARATOR);

		if (haveanalog)
			custombottom = ui().get_line_height() + (ui().box_tb_border() * 3.0F);
	}

	virtual void handle(event const *ev) override
	{
		for (int i = 0; item_count() > i; ++i)
		{
			void *const ref(item(i).ref());
			if (ref)
			{
				input_device_item &input = *reinterpret_cast<input_device_item *>(ref);
				item(i).set_subtext(format_value(input));
			}
		}
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


void menu_input_devices::populate(float &customtop, float &custombottom)
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


void menu_input_devices::handle(event const *ev)
{
	if (ev && ev->itemref)
	{
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
							break;
						}
					}
					if (!i && found_break)
					{
						set_selected_index(target);
						break;
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
						break;
					}
				}
			}
			break;
		}
	}
}

} // namespace ui
