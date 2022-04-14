// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Vas Crabb
/*********************************************************************

    ui/analogipt.cpp

    Analog inputs menu.

*********************************************************************/

#include "emu.h"
#include "ui/analogipt.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>


namespace ui {

inline menu_analog::item_data::item_data(ioport_field &f, int t) noexcept
	: field(f)
	, type(t)
	, defvalue(
			(ANALOG_ITEM_KEYSPEED == t) ? f.delta() :
			(ANALOG_ITEM_CENTERSPEED == t) ? f.centerdelta() :
			(ANALOG_ITEM_REVERSE == t) ? f.analog_reverse() :
			(ANALOG_ITEM_SENSITIVITY == t) ? f.sensitivity() :
			-1)
	, min((ANALOG_ITEM_SENSITIVITY == t) ? 1 : 0)
	, max((ANALOG_ITEM_REVERSE == t) ? 1 : std::max(defvalue * 4, 255))
	, cur(-1)
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
	, m_prompt()
	, m_visible_fields(0U)
	, m_top_field(0)
	, m_hide_menu(false)
{
	set_process_flags(PROCESS_LR_REPEAT);
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

	// make a box or two
	rgb_t const fgcolor(ui().colors().text_color());
	float const lineheight(ui().get_line_height());
	float const border(ui().box_tb_border());
	float const boxleft((1.0f - namewidth - extrawidth) / 2.0f);
	float const boxright(boxleft + namewidth + extrawidth);
	float boxtop;
	float boxbottom;
	float firstliney;
	int visible_fields;
	if (m_hide_menu)
	{
		if (m_prompt.empty())
			m_prompt = util::string_format(_("Press %s to show menu"), ui().get_general_input_setting(IPT_UI_ON_SCREEN_DISPLAY));
		draw_text_box(
				&m_prompt, &m_prompt + 1,
				boxleft, boxright, y - top, y - top + lineheight + (border * 2.0f),
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
				fgcolor, ui().colors().background_color(), 1.0f);
		boxtop = y - top + lineheight + (border * 3.0f);
		firstliney = y - top + lineheight + (border * 4.0f);
		visible_fields = std::min<int>(m_field_data.size(), int((y2 + bottom - border - firstliney) / lineheight));
		boxbottom = firstliney + (lineheight * visible_fields) + border;
	}
	else
	{
		boxtop = y2 + border;
		boxbottom = y2 + bottom;
		firstliney = y2 + (border * 2.0f);
		visible_fields = m_visible_fields;
	}
	ui().draw_outlined_box(container(), boxleft, boxtop, boxright, boxbottom, ui().colors().background_color());

	// force the field being configured to be visible
	ioport_field *const selfield(selectedref ? &reinterpret_cast<item_data *>(selectedref)->field.get() : nullptr);
	if (!m_hide_menu && selfield)
	{
		auto const found(
				std::find_if(
					m_field_data.begin(),
					m_field_data.end(),
					[selfield] (field_data const &d) { return &d.field.get() == selfield; }));
		if (m_field_data.end() != found)
		{
			auto const i(std::distance(m_field_data.begin(), found));
			if (m_top_field > i)
				m_top_field = i;
			if ((m_top_field + visible_fields) <= i)
				m_top_field = i - m_visible_fields + 1;
		}
	}
	if (0 > m_top_field)
		m_top_field = 0;
	if ((m_top_field + visible_fields) > m_field_data.size())
		m_top_field = m_field_data.size() - visible_fields;

	// show live fields
	namewidth += lineheight * aspect;
	float const nameleft(boxleft + (ui().box_lr_border() * aspect));
	float const indleft(nameleft + namewidth);
	float const indright(indleft + 0.4f);
	for (unsigned line = 0; visible_fields > line; ++line)
	{
		// draw arrows if scrolling is possible and menu is hidden
		float const liney(firstliney + (lineheight * line));
		if (m_hide_menu)
		{
			bool const uparrow(!line && m_top_field);
			bool const downarrow(((visible_fields - 1) == line) && ((m_field_data.size() - 1) > (line + m_top_field)));
			if (uparrow || downarrow)
			{
				float const arrowwidth = lineheight * aspect;
				draw_arrow(
						0.5f * (nameleft + indright - arrowwidth), liney + (0.25f * lineheight),
						0.5f * (nameleft + indright + arrowwidth), liney + (0.75f * lineheight),
						fgcolor, line ? (ROT0 ^ ORIENTATION_FLIP_Y) : ROT0);
				continue;
			}
		}

		// draw the field name, using the selected colour if it's being configured
		field_data &data(m_field_data[line + m_top_field]);
		bool const selected(&data.field.get() == selfield);
		rgb_t const fieldcolor(selected ? ui().colors().selected_color() : fgcolor);
		ui().draw_text_full(
				container(),
				data.field.get().name(),
				nameleft, liney, namewidth,
				text_layout::text_justify::LEFT, text_layout::word_wrapping::NEVER,
				mame_ui_manager::NORMAL, fieldcolor, ui().colors().text_bg_color());

		ioport_value cur(0U);
		data.field.get().live().analog->read(cur);
		cur = (cur >> data.shift) - data.field.get().minval();
		float fill(float(cur) / data.range);
		if (data.field.get().analog_reverse())
			fill = 1.0f - fill;

		float const indtop(liney + (lineheight * 0.2f));
		float const indbottom(liney + (lineheight * 0.8f));
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
	}
}


void menu_analog::menu_activated()
{
	// scripts could have changed something in the mean time
	reset(reset_options::REMEMBER_POSITION);
}


void menu_analog::handle(event const *ev)
{
	// handle events
	if (ev)
	{
		if (IPT_UI_ON_SCREEN_DISPLAY == ev->iptkey)
		{
			m_hide_menu = !m_hide_menu;
			set_process_flags(PROCESS_LR_REPEAT | (m_hide_menu ? (PROCESS_CUSTOM_NAV | PROCESS_CUSTOM_ONLY) : 0));
		}
		else if (m_hide_menu)
		{
			switch (ev->iptkey)
			{
			case IPT_UI_UP:
				--m_top_field;
				break;
			case IPT_UI_DOWN:
				++m_top_field;
				break;
			case IPT_UI_HOME:
				m_top_field = 0;
				break;
			case IPT_UI_END:
				m_top_field = m_field_data.size();
				break;
			}
		}
		else if (ev->itemref)
		{
			item_data &data(*reinterpret_cast<item_data *>(ev->itemref));
			int newval(data.cur);

			switch (ev->iptkey)
			{
			// if selected, reset to default value
			case IPT_UI_SELECT:
			case IPT_UI_CLEAR:
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

			// move to first item for previous device
			case IPT_UI_PREV_GROUP:
				{
					auto current = std::distance(m_item_data.data(), &data);
					device_t const *dev(&data.field.get().device());
					bool found_break = false;
					while (0 < current)
					{
						if (!found_break)
						{
							device_t const *prev(&m_item_data[--current].field.get().device());
							if (prev != dev)
							{
								dev = prev;
								found_break = true;
							}
						}
						else if (&m_item_data[current - 1].field.get().device() != dev)
						{
							set_selection(&m_item_data[current]);
							set_top_line(selected_index() - 1);
							break;
						}
						else
						{
							--current;
						}
						if (found_break && !current)
						{
							set_selection(&m_item_data[current]);
							set_top_line(selected_index() - 1);
							break;
						}
					}
				}
				break;

			// move to first item for next device
			case IPT_UI_NEXT_GROUP:
				{
					auto current = std::distance(m_item_data.data(), &data);
					device_t const *const dev(&data.field.get().device());
					while (m_item_data.size() > ++current)
					{
						if (&m_item_data[current].field.get().device() != dev)
						{
							set_selection(&m_item_data[current]);
							set_top_line(selected_index() - 1);
							break;
						}
					}
				}
				break;
			}

			// clamp to range
			newval = std::clamp(newval, data.min, data.max);

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
				data.cur = newval;

				// update the menu item
				ev->item->set_subtext(item_text(data.type, newval));
				ev->item->set_flags((data.cur <= data.min) ? FLAG_RIGHT_ARROW : (data.cur >= data.max) ? FLAG_LEFT_ARROW : FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW);
			}
		}
	}
}


void menu_analog::populate(float &customtop, float &custombottom)
{
	// loop over input ports
	if (m_item_data.empty())
		find_fields();

	device_t *prev_owner(nullptr);
	ioport_field *field(nullptr);
	ioport_field::user_settings settings;

	// add the items
	std::string text;
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
				if (prev_owner->owner())
					item_append(string_format(_("%1$s [root%2$s]"), prev_owner->type().fullname(), prev_owner->tag()), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
				else
					item_append(string_format(_("[root%1$s]"), prev_owner->tag()), FLAG_UI_HEADING | FLAG_DISABLE, nullptr);
			}
		}

		// determine the properties of this item
		switch (data.type)
		{
		default:
		case ANALOG_ITEM_KEYSPEED:
			text = string_format(_("%1$s Increment/Decrement Speed"), field->name());
			data.cur = settings.delta;
			break;

		case ANALOG_ITEM_CENTERSPEED:
			text = string_format(_("%1$s Auto-centering Speed"), field->name());
			data.cur = settings.centerdelta;
			break;

		case ANALOG_ITEM_REVERSE:
			text = string_format(_("%1$s Reverse"), field->name());
			data.cur = settings.reverse;
			break;

		case ANALOG_ITEM_SENSITIVITY:
			text = string_format(_("%1$s Sensitivity"), field->name());
			data.cur = settings.sensitivity;
			break;
		}

		// append a menu item
		item_append(
				std::move(text),
				item_text(data.type, data.cur),
				(data.cur <= data.min) ? FLAG_RIGHT_ARROW : (data.cur >= data.max) ? FLAG_LEFT_ARROW : FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW,
				&data);
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

	// restrict live display to 40% height plus borders
	if ((ui().get_line_height() * m_field_data.size()) > 0.4f)
		m_visible_fields = unsigned(0.4f / ui().get_line_height());
	else
		m_visible_fields = m_field_data.size();
}


std::string menu_analog::item_text(int type, int value)
{
	switch (type)
	{
	default:
	case ANALOG_ITEM_KEYSPEED:
		return string_format("%d", value);

	case ANALOG_ITEM_CENTERSPEED:
		return string_format("%d", value);

	case ANALOG_ITEM_REVERSE:
		return value ? _("On") : _("Off");

	case ANALOG_ITEM_SENSITIVITY:
		return string_format("%d", value);
	}
}

} // namespace ui
