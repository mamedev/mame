// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Vas Crabb
/*********************************************************************

    ui/analogipt.cpp

    Analog inputs menu.

*********************************************************************/

#include "emu.h"
#include "ui/analogipt.h"

#include <algorithm>
#include <string>
#include <utility>


namespace ui {

inline menu_analog::item_data::item_data(ioport_field &f, int t) noexcept
	: field(f)
	, type(t)
	, min((ANALOG_ITEM_SENSITIVITY == t) ? 1 : 0)
	, max((ANALOG_ITEM_REVERSE == t) ? 1 : 255)
	, cur(-1)
	, defvalue(
			(ANALOG_ITEM_KEYSPEED == t) ? f.delta() :
			(ANALOG_ITEM_CENTERSPEED == t) ? f.centerdelta() :
			(ANALOG_ITEM_REVERSE == t) ? f.analog_reverse() :
			(ANALOG_ITEM_SENSITIVITY == t) ? f.sensitivity() :
			-1)
{
}


inline menu_analog::field_data::field_data(ioport_field &f) noexcept
	: field(f)
	, range(f.maxval() - f.minval())
	, neutral(float(f.analog_reverse() ? (f.maxval() - f.defvalue()) : (f.defvalue() - f.minval())) / range)
	, origin((f.analog_wraps() && (f.defvalue() != f.minval()) && (f.defvalue() != f.maxval())) ? 0.0f : neutral)
	, shift(0U)
	, show_neutral((f.defvalue() != f.minval()) && (f.defvalue() != f.maxval()))
{
	for (ioport_value m = f.mask(); m && !BIT(m, 0); m >>= 1, ++shift) { }
}


menu_analog::menu_analog(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
	, m_item_data()
	, m_field_data()
	, m_visible_fields(0U)
{
}


menu_analog::~menu_analog()
{
}


void menu_analog::custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2)
{
	// work out how much space to use for field names
	float const aspect(machine().render().ui_aspect(&container()));
	float const extrawidth(0.4f + (((ui().box_lr_border() * 2.0f) + ui().get_line_height()) * aspect));
	float const nameavail(1.0f - (ui().box_lr_border() * 2.0f * aspect) - extrawidth);
	float namewidth(0.0f);
	for (field_data &data : m_field_data)
		namewidth = (std::min)((std::max)(ui().get_string_width(data.field.get().name()), namewidth), nameavail);

	// make a box
	float const boxleft((1.0f - namewidth - extrawidth) / 2.0f);
	float const boxright(boxleft + namewidth + extrawidth);
	ui().draw_outlined_box(container(), boxleft, y2 + ui().box_tb_border(), boxright, y2 + bottom, ui().colors().background_color());

	// show live fields
	namewidth += ui().get_line_height() * aspect;
	unsigned line(0U);
	float const nameleft(boxleft + (ui().box_lr_border() * aspect));
	float const indleft(nameleft + namewidth);
	float const indright(indleft + 0.4f);
	ioport_field *const selfield(selectedref ? &reinterpret_cast<item_data *>(selectedref)->field.get() : nullptr);
	for (field_data &data : m_field_data)
	{
		bool const selected(&data.field.get() == selfield);
		rgb_t const fgcolor(selected ? ui().colors().selected_color() : ui().colors().text_color());
		float const liney(y2 + (ui().box_tb_border() * 2.0f) + (ui().get_line_height() * line));
		ui().draw_text_full(
				container(),
				data.field.get().name(),
				nameleft, liney, namewidth,
				text_layout::text_justify::LEFT, text_layout::word_wrapping::NEVER,
				mame_ui_manager::NORMAL, fgcolor, ui().colors().text_bg_color());

		ioport_value cur(0U);
		data.field.get().live().analog->read(cur);
		cur = (cur >> data.shift) - data.field.get().minval();
		float fill(float(cur) / data.range);
		if (data.field.get().analog_reverse())
			fill = 1.0f - fill;

		float const indtop(liney + (ui().get_line_height() * 0.2f));
		float const indbottom(liney + (ui().get_line_height() * 0.8f));
		if (data.origin > fill)
			container().add_rect(indleft + (fill * 0.4f), indtop, indleft + (data.origin * 0.4f), indbottom, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		else
			container().add_rect(indleft + (data.origin * 0.4f), indtop, indleft + (fill * 0.4f), indbottom, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(indleft, indtop, indright, indtop, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(indright, indtop, indright, indbottom, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(indright, indbottom, indleft, indbottom, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(indleft, indbottom, indleft, indtop, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		if (data.show_neutral)
			container().add_line(indleft + (data.neutral * 0.4f), indtop, indleft + (data.neutral * 0.4f), indbottom, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		// TODO: ensure field being configured is visible
		if (++line >= m_visible_fields)
			break;
	}
}


void menu_analog::handle()
{
	// process the menu
	event const *const menu_event(process(PROCESS_LR_REPEAT));

	// handle events
	if (menu_event && menu_event->itemref)
	{
		item_data &data(*reinterpret_cast<item_data *>(menu_event->itemref));
		int newval(data.cur);

		switch (menu_event->iptkey)
		{
		// if selected, reset to default value
		case IPT_UI_SELECT:
			newval = data.defvalue;
			break;

		// left decrements
		case IPT_UI_LEFT:
			newval -= machine().input().code_pressed(KEYCODE_LSHIFT) ? 10 : 1;
			break;

		// right increments
		case IPT_UI_RIGHT:
			newval += machine().input().code_pressed(KEYCODE_LSHIFT) ? 10 : 1;
			break;
		}

		// clamp to range
		if (newval < data.min)
			newval = data.min;
		if (newval > data.max)
			newval = data.max;

		// if things changed, update
		if (newval != data.cur)
		{
			ioport_field::user_settings settings;

			// get the settings and set the new value
			data.field.get().get_user_settings(settings);
			switch (data.type)
			{
				case ANALOG_ITEM_KEYSPEED:      settings.delta = newval;        break;
				case ANALOG_ITEM_CENTERSPEED:   settings.centerdelta = newval;  break;
				case ANALOG_ITEM_REVERSE:       settings.reverse = newval;      break;
				case ANALOG_ITEM_SENSITIVITY:   settings.sensitivity = newval;  break;
			}
			data.field.get().set_user_settings(settings);

			// rebuild the menu
			reset(reset_options::REMEMBER_POSITION);
		}
	}
}


void menu_analog::populate(float &customtop, float &custombottom)
{
	// loop over input ports
	if (m_item_data.empty())
		find_fields();

	device_t *prev_owner(nullptr);
	bool first_entry(true);
	ioport_field *field(nullptr);
	ioport_field::user_settings settings;

	// add the items
	std::string text;
	std::string subtext;
	for (item_data &data : m_item_data)
	{
		// get the user settings
		if (&data.field.get() != field)
		{
			field = &data.field.get();
			field->get_user_settings(settings);

			if (&field->device() != prev_owner)
			{
				prev_owner = &field->device();
				if (first_entry)
					first_entry = false;
				else
					item_append(menu_item_type::SEPARATOR);
				item_append(string_format("[root%s]", prev_owner->tag()), 0, nullptr);
			}
		}

		// determine the properties of this item
		switch (data.type)
		{
		default:
		case ANALOG_ITEM_KEYSPEED:
			text = string_format("%s Digital Speed", field->name());
			subtext = string_format("%d", settings.delta);
			data.cur = settings.delta;
			break;

		case ANALOG_ITEM_CENTERSPEED:
			text = string_format("%s Autocenter Speed", field->name());
			subtext = string_format("%d", settings.centerdelta);
			data.cur = settings.centerdelta;
			break;

		case ANALOG_ITEM_REVERSE:
			text = string_format("%s Reverse", field->name());
			subtext.assign(settings.reverse ? "On" : "Off");
			data.cur = settings.reverse;
			break;

		case ANALOG_ITEM_SENSITIVITY:
			text = string_format("%s Sensitivity", field->name());
			subtext = string_format("%d", settings.sensitivity);
			data.cur = settings.sensitivity;
			break;
		}

		// put on arrows
		uint32_t flags(0U);
		if (data.cur > data.min)
			flags |= FLAG_LEFT_ARROW;
		if (data.cur < data.max)
			flags |= FLAG_RIGHT_ARROW;

		// append a menu item
		item_append(std::move(text), std::move(subtext), flags, &data);
	}

	item_append(menu_item_type::SEPARATOR);

	// space for live display
	custombottom = (ui().get_line_height() * m_visible_fields) + (ui().box_tb_border() * 3.0f);
}


void menu_analog::find_fields()
{
	assert(m_field_data.empty());

	// collect analog fields
	for (auto &port : machine().ioport().ports())
	{
		for (ioport_field &field : port.second->fields())
		{
			if (field.is_analog() && field.enabled())
			{
				// based on the type, determine if we enable autocenter
				bool use_autocenter = false;
				switch (field.type())
				{
				case IPT_POSITIONAL:
				case IPT_POSITIONAL_V:
					use_autocenter = !field.analog_wraps();
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

				// iterate over types
				for (int type = 0; type < ANALOG_ITEM_COUNT; type++)
				{
					if ((ANALOG_ITEM_CENTERSPEED != type) || use_autocenter)
						m_item_data.emplace_back(field, type);
				}

				m_field_data.emplace_back(field);
			}
		}
	}

	// restrict live display to 60% height plus borders
	if ((ui().get_line_height() * m_field_data.size()) > 0.6f)
		m_visible_fields = unsigned(0.6f / ui().get_line_height());
	else
		m_visible_fields = m_field_data.size();
}

} // namespace ui
