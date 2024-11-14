// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Vas Crabb
/*********************************************************************

    ui/analogipt.cpp

    Analog inputs menu.

*********************************************************************/

#include "emu.h"
#include "ui/analogipt.h"

#include "ui/textbox.h"

#include "uiinput.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>


namespace ui {

namespace {

char const HELP_TEXT[] = N_p("menu-analoginput",
		"Show/hide settings  \t\t%1$s\n"
		"Decrease value  \t\t%2$s\n"
		"Increase value  \t\t%3$s\n"
		"Restore default value  \t\t%4$s\n"
		"Previous device  \t\t%5$s\n"
		"Next device  \t\t%6$s\n"
		"Return to previous menu  \t\t%7$s");

} // anonymous namespace


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
	, range(0.0F)
	, neutral(0.0F)
	, origin(0.0F)
	, shift(0U)
	, show_neutral((f.defvalue() != f.minval()) && (f.defvalue() != f.maxval()))
{
	for (ioport_value m = f.mask(); m && !BIT(m, 0); m >>= 1, ++shift) { }
	ioport_value const m(f.mask() >> shift);
	range = (f.maxval() - f.minval()) & m;
	ioport_value const n((f.analog_reverse() ? (f.maxval() - f.defvalue()) : (f.defvalue() - f.minval())) & m);
	neutral = float(n) / range;
	if (!f.analog_wraps() || (f.defvalue() == f.minval()) || (f.defvalue() == f.maxval()))
		origin = neutral;
}


menu_analog::menu_analog(mame_ui_manager &mui, render_container &container)
	: menu(mui, container)
	, m_item_data()
	, m_field_data()
	, m_prompt()
	, m_bottom_fields(0U)
	, m_visible_fields(0)
	, m_top_field(0)
	, m_hide_menu(false)
	, m_box_left(1.0F)
	, m_box_top(1.0F)
	, m_box_right(0.0F)
	, m_box_bottom(0.0F)
	, m_pointer_action(pointer_action::NONE)
	, m_scroll_repeat(std::chrono::steady_clock::time_point::min())
	, m_base_pointer(0.0F, 0.0F)
	, m_last_pointer(0.0F, 0.0F)
	, m_scroll_base(0)
	, m_arrow_clicked_first(false)
{
	set_process_flags(PROCESS_LR_REPEAT);
	set_heading(_("menu-analoginput", "Analog Input Adjustments"));
}


menu_analog::~menu_analog()
{
}


void menu_analog::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	m_box_left = m_box_top = 1.0F;
	m_box_right = m_box_bottom = 0.0F;

	// space for live display
	set_custom_space(0.0F, (line_height() * m_bottom_fields) + (tb_border() * 3.0F));
}


void menu_analog::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	// work out how much space to use for field names
	float const extrawidth(0.4F + (((ui().box_lr_border() * 2.0F) + ui().get_line_height()) * x_aspect()));
	float const nameavail(1.0F - (lr_border() * 2.0F) - extrawidth);
	float namewidth(0.0F);
	for (field_data &data : m_field_data)
		namewidth = (std::min)((std::max)(get_string_width(data.field.get().name()), namewidth), nameavail);

	// make a box or two
	rgb_t const fgcolor(ui().colors().text_color());
	m_box_left = (1.0F - namewidth - extrawidth) * 0.5F;
	m_box_right = m_box_left + namewidth + extrawidth;
	float firstliney;
	if (m_hide_menu)
	{
		if (m_prompt.empty())
			m_prompt = util::string_format(_("menu-analoginput", "Press %s to show settings"), ui().get_general_input_setting(IPT_UI_ON_SCREEN_DISPLAY));
		draw_text_box(
				&m_prompt, &m_prompt + 1,
				m_box_left, m_box_right, origy1 - top, origy1 - top + line_height() + (tb_border() * 2.0F),
				text_layout::text_justify::CENTER, text_layout::word_wrapping::TRUNCATE, false,
				fgcolor, ui().colors().background_color());
		m_box_top = origy1 - top + line_height() + (tb_border() * 3.0F);
		firstliney = origy1 - top + line_height() + (tb_border() * 4.0F);
		m_visible_fields = std::min<int>(m_field_data.size(), int((origy2 + bottom - tb_border() - firstliney) / line_height()));
		m_box_bottom = firstliney + (line_height() * m_visible_fields) + tb_border();
	}
	else
	{
		m_box_top = origy2 + tb_border();
		m_box_bottom = origy2 + bottom;
		firstliney = origy2 + (tb_border() * 2.0F);
		m_visible_fields = m_bottom_fields;
	}
	ui().draw_outlined_box(container(), m_box_left, m_box_top, m_box_right, m_box_bottom, ui().colors().background_color());

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
			if ((m_top_field + m_visible_fields) <= i)
				m_top_field = i - m_bottom_fields + 1;
		}
	}
	if (0 > m_top_field)
		m_top_field = 0;
	if ((m_top_field + m_visible_fields) > m_field_data.size())
		m_top_field = m_field_data.size() - m_visible_fields;

	// show live fields
	namewidth += line_height() * x_aspect();
	float const nameleft(m_box_left + lr_border());
	float const indleft(nameleft + namewidth);
	float const indright(indleft + 0.4F);
	for (unsigned line = 0; m_visible_fields > line; ++line)
	{
		// draw arrows if scrolling is possible and menu is hidden
		float const liney(firstliney + (line_height() * float(line)));
		if (m_hide_menu)
		{
			bool const uparrow(!line && m_top_field);
			bool const downarrow(((m_visible_fields - 1) == line) && ((m_field_data.size() - 1) > (line + m_top_field)));
			if (uparrow || downarrow)
			{
				bool const active((uparrow && (pointer_action::SCROLL_UP == m_pointer_action)) || (downarrow && (pointer_action::SCROLL_DOWN == m_pointer_action)));
				bool const hovered((active || pointer_idle()) && pointer_in_rect(nameleft, liney, indright, liney + line_height()));
				float const arrowwidth(line_height() * x_aspect());
				rgb_t const arrowcolor(!(active || hovered) ? fgcolor : (active && hovered) ? ui().colors().selected_color() : ui().colors().mouseover_color());
				if (active || hovered)
				{
					highlight(
							nameleft, liney, indright, liney + line_height(),
							(active && hovered) ? ui().colors().selected_bg_color() : ui().colors().mouseover_bg_color());
				}
				draw_arrow(
						0.5F * (nameleft + indright - arrowwidth), liney + (0.25F * line_height()),
						0.5F * (nameleft + indright + arrowwidth), liney + (0.75F * line_height()),
						arrowcolor, line ? (ROT0 ^ ORIENTATION_FLIP_Y) : ROT0);
				continue;
			}
		}

		// draw the field name, using the selected colour if it's being configured
		field_data &data(m_field_data[line + m_top_field]);
		bool const selected(&data.field.get() == selfield);
		rgb_t const fieldcolor(selected ? ui().colors().selected_color() : fgcolor);
		draw_text_normal(
				data.field.get().name(),
				nameleft, liney, namewidth,
				text_layout::text_justify::LEFT, text_layout::word_wrapping::NEVER,
				fieldcolor);

		ioport_value cur(0U);
		data.field.get().live().analog->read(cur);
		cur = ((cur >> data.shift) - data.field.get().minval()) & (data.field.get().mask() >> data.shift);
		float fill(float(cur) / data.range);
		if (data.field.get().analog_reverse())
			fill = 1.0F - fill;

		float const indtop(liney + (line_height() * 0.2F));
		float const indbottom(liney + (line_height() * 0.8F));
		if (data.origin > fill)
			container().add_rect(indleft + (fill * 0.4F), indtop, indleft + (data.origin * 0.4F), indbottom, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		else
			container().add_rect(indleft + (data.origin * 0.4F), indtop, indleft + (fill * 0.4F), indbottom, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(indleft, indtop, indright, indtop, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(indright, indtop, indright, indbottom, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(indright, indbottom, indleft, indbottom, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		container().add_line(indleft, indbottom, indleft, indtop, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		if (data.show_neutral)
			container().add_line(indleft + (data.neutral * 0.4F), indtop, indleft + (data.neutral * 0.4F), indbottom, UI_LINE_WIDTH, fgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}
}


std::tuple<int, bool, bool> menu_analog::custom_pointer_updated(bool changed, ui_event const &uievt)
{
	// no pointer input if we don't have up-to-date content on-screen
	if ((m_box_left > m_box_right) || (ui_event::type::POINTER_ABORT == uievt.event_type))
	{
		m_pointer_action = pointer_action::NONE;
		return std::make_tuple(IPT_INVALID, false, false);
	}

	// if nothing's happening, check for clicks
	if (pointer_idle())
	{
		if ((uievt.pointer_pressed & 0x01) && !(uievt.pointer_buttons & ~u32(0x01)))
		{
			if (1 == uievt.pointer_clicks)
				m_arrow_clicked_first = false;

			float const firstliney(m_box_top + tb_border());
			float const fieldleft(m_box_left + lr_border());
			float const fieldright(m_box_right - lr_border());
			auto const [x, y] = pointer_location();
			bool const inwidth((x >= fieldleft) && (x < fieldright));
			if (m_hide_menu && m_top_field && inwidth && (y >= firstliney) && (y < (firstliney + line_height())))
			{
				// scroll up arrow
				if (1 == uievt.pointer_clicks)
					m_arrow_clicked_first = true;

				--m_top_field;
				m_pointer_action = pointer_action::SCROLL_UP;
				m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
				m_last_pointer = std::make_pair(x, y);
				return std::make_tuple(IPT_INVALID, true, true);
			}
			else if (m_hide_menu && ((m_top_field + m_visible_fields) < m_field_data.size()) && inwidth && (y >= (firstliney + (float(m_visible_fields - 1) * line_height()))) && (y < (firstliney + (float(m_visible_fields) * line_height()))))
			{
				// scroll down arrow
				if (1 == uievt.pointer_clicks)
					m_arrow_clicked_first = true;

				++m_top_field;
				m_pointer_action = pointer_action::SCROLL_DOWN;
				m_scroll_repeat = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
				m_last_pointer = std::make_pair(x, y);
				return std::make_tuple(IPT_INVALID, true, true);
			}
			else if ((x >= m_box_left) && (x < m_box_right) && (y >= m_box_top) && (y < m_box_bottom))
			{
				if (!m_arrow_clicked_first && (2 == uievt.pointer_clicks))
				{
					// toggle menu display
					// FIXME: this should really use the start point of the multi-click action
					m_pointer_action = pointer_action::CHECK_TOGGLE_MENU;
					m_base_pointer = std::make_pair(x, y);
					return std::make_tuple(IPT_INVALID, true, false);
				}
				else if (ui_event::pointer::TOUCH == uievt.pointer_type)
				{
					m_pointer_action = pointer_action::SCROLL_DRAG;
					m_base_pointer = std::make_pair(x, y);
					m_last_pointer = m_base_pointer;
					m_scroll_base = m_top_field;
					return std::make_tuple(IPT_INVALID, true, false);
				}
			}
		}
		return std::make_tuple(IPT_INVALID, false, false);
	}

	// handle in-progress actions
	switch (m_pointer_action)
	{
	case pointer_action::NONE:
		break;

	case pointer_action::SCROLL_UP:
	case pointer_action::SCROLL_DOWN:
		{
			// check for re-entry
			bool redraw(false);
			float const linetop(m_box_top + tb_border() + ((pointer_action::SCROLL_DOWN == m_pointer_action) ? (float(m_visible_fields - 1) * line_height()) : 0.0F));
			float const linebottom(linetop + line_height());
			auto const [x, y] = pointer_location();
			bool const reentered(reentered_rect(m_last_pointer.first, m_last_pointer.second, x, y, m_box_left + lr_border(), linetop, m_box_right - lr_border(), linebottom));
			if (reentered)
			{
				auto const now(std::chrono::steady_clock::now());
				if (scroll_if_expired(now))
				{
					redraw = true;
					m_scroll_repeat = now + std::chrono::milliseconds(100);
				}
			}
			m_last_pointer = std::make_pair(x, y);
			if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
				m_pointer_action = pointer_action::NONE;
			return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, redraw);
		}
		break;

	case pointer_action::SCROLL_DRAG:
		{
			bool const scrolled(update_scroll_drag(uievt));
			return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, scrolled);
		}

	case pointer_action::CHECK_TOGGLE_MENU:
		if ((ui_event::pointer::TOUCH == uievt.pointer_type) && (0 > uievt.pointer_clicks))
		{
			// converted to hold/drag - treat as scroll if it's touch
			m_pointer_action = pointer_action::SCROLL_DRAG;
			m_last_pointer = m_base_pointer;
			m_scroll_base = m_top_field;
			bool const scrolled(update_scroll_drag(uievt));
			return std::make_tuple(IPT_INVALID, pointer_action::NONE != m_pointer_action, scrolled);
		}
		else if (uievt.pointer_released & 0x01)
		{
			// primary button released - simulate the on-screen display key if it wasn't converted to a hold/drag
			return std::make_tuple((2 == uievt.pointer_clicks) ? IPT_UI_ON_SCREEN_DISPLAY : IPT_INVALID, false, false);
		}
		else if ((2 != uievt.pointer_clicks) || (uievt.pointer_buttons & ~u32(0x01)))
		{
			// treat converting to a hold/drag or pressing another button as cancelling the action
			return std::make_tuple(IPT_INVALID, false, false);
		}
		return std::make_tuple(IPT_INVALID, true, false);
	}
	return std::make_tuple(IPT_INVALID, false, false);
}


void menu_analog::menu_activated()
{
	// scripts could have changed something in the mean time
	m_item_data.clear();
	m_field_data.clear();
	reset(reset_options::REMEMBER_POSITION);

	m_box_left = m_box_top = 1.0F;
	m_box_right = m_box_bottom = 0.0F;
	m_pointer_action = pointer_action::NONE;
	m_arrow_clicked_first = false;
}


bool menu_analog::handle(event const *ev)
{
	// deal with repeating scroll arrows
	bool scrolled(false);
	if ((pointer_action::SCROLL_UP == m_pointer_action) || (pointer_action::SCROLL_DOWN == m_pointer_action))
	{
		float const linetop(m_box_top + tb_border() + ((pointer_action::SCROLL_DOWN == m_pointer_action) ? (float(m_visible_fields - 1) * line_height()) : 0.0F));
		float const linebottom(linetop + line_height());
		if (pointer_in_rect(m_box_left + lr_border(), linetop, m_box_right - lr_border(), linebottom))
		{
			while (scroll_if_expired(std::chrono::steady_clock::now()))
			{
				scrolled = true;
				m_scroll_repeat += std::chrono::milliseconds(100);
			}
		}
	}

	if (!ev)
	{
		return scrolled;
	}
	else if (IPT_UI_ON_SCREEN_DISPLAY == ev->iptkey)
	{
		m_hide_menu = !m_hide_menu;

		m_box_left = m_box_top = 1.0F;
		m_box_right = m_box_bottom = 0.0F;
		m_pointer_action = pointer_action::NONE;
		m_arrow_clicked_first = false;

		set_process_flags(PROCESS_LR_REPEAT | (m_hide_menu ? (PROCESS_CUSTOM_NAV | PROCESS_CUSTOM_ONLY) : 0));
		return true;
	}
	else if (IPT_UI_HELP == ev->iptkey)
	{
		stack_push<menu_fixed_textbox>(
				ui(),
				container(),
				_("menu-analoginput", "Analog Input Adjustments Help"),
				util::string_format(
					_("menu-analoginput", HELP_TEXT),
					ui().get_general_input_setting(IPT_UI_ON_SCREEN_DISPLAY),
					ui().get_general_input_setting(IPT_UI_LEFT),
					ui().get_general_input_setting(IPT_UI_RIGHT),
					ui().get_general_input_setting(IPT_UI_CLEAR),
					ui().get_general_input_setting(IPT_UI_PREV_GROUP),
					ui().get_general_input_setting(IPT_UI_NEXT_GROUP),
					ui().get_general_input_setting(IPT_UI_BACK)));
	}
	else if (m_hide_menu)
	{
		switch (ev->iptkey)
		{
		case IPT_UI_UP:
			if (m_top_field)
			{
				--m_top_field;
				return true;
			}
			break;
		case IPT_UI_DOWN:
			if ((m_top_field + m_visible_fields) < m_field_data.size())
			{
				++m_top_field;
				return true;
			}
			break;
		case IPT_UI_PAGE_UP:
			if (m_visible_fields)
			{
				m_top_field -= std::min<int>(m_visible_fields - 3, m_top_field);
				return true;
			}
			break;
		case IPT_UI_PAGE_DOWN:
			if ((m_top_field + m_visible_fields) < m_field_data.size())
			{
				m_top_field = std::min<int>(m_top_field + m_visible_fields - 3, m_field_data.size() - m_visible_fields);
				return true;
			}
			break;
		case IPT_UI_HOME:
			if (m_top_field)
			{
				m_top_field = 0;
				return true;
			}
			break;
		case IPT_UI_END:
			if ((m_top_field + m_visible_fields) < m_field_data.size())
			{
				m_top_field = m_field_data.size() - m_visible_fields;
				return true;
			}
			break;
		}
	}
	else if (ev->itemref)
	{
		item_data &data(*reinterpret_cast<item_data *>(ev->itemref));
		int newval(data.cur);

		switch (ev->iptkey)
		{
		// flip toggles when selected
		case IPT_UI_SELECT:
			if (ANALOG_ITEM_REVERSE == data.type)
				newval = newval ? 0 : 1;
			break;

		// if cleared, reset to default value
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
						return true;
					}
					else
					{
						--current;
					}
					if (found_break && !current)
					{
						set_selection(&m_item_data[current]);
						set_top_line(selected_index() - 1);
						return true;
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
						return true;
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
			return true;
		}
	}

	return scrolled;
}


void menu_analog::populate()
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
			text = string_format(_("menu-analoginput", "%1$s Increment/Decrement Speed"), field->name());
			data.cur = settings.delta;
			break;

		case ANALOG_ITEM_CENTERSPEED:
			text = string_format(_("menu-analoginput", "%1$s Auto-centering Speed"), field->name());
			data.cur = settings.centerdelta;
			break;

		case ANALOG_ITEM_REVERSE:
			text = string_format(_("menu-analoginput", "%1$s Reverse"), field->name());
			data.cur = settings.reverse;
			break;

		case ANALOG_ITEM_SENSITIVITY:
			text = string_format(_("menu-analoginput", "%1$s Sensitivity"), field->name());
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

	// display a message if there are toggle inputs enabled
	if (!prev_owner)
		item_append(_("menu-analoginput", "[no analog inputs are enabled]"), FLAG_DISABLE, nullptr);

	item_append(menu_item_type::SEPARATOR);

	// space for live display
	set_custom_space(0.0F, (line_height() * m_bottom_fields) + (tb_border() * 3.0F));
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
	if ((line_height() * m_field_data.size()) > 0.4F)
		m_bottom_fields = unsigned(0.4F / line_height());
	else
		m_bottom_fields = m_field_data.size();
}


bool menu_analog::scroll_if_expired(std::chrono::steady_clock::time_point now)
{
	if (now < m_scroll_repeat)
		return false;

	if (pointer_action::SCROLL_DOWN == m_pointer_action)
	{
		if ((m_top_field + m_visible_fields) < m_field_data.size())
			++m_top_field;
		if ((m_top_field + m_visible_fields) == m_field_data.size())
			m_pointer_action = pointer_action::NONE;
	}
	else
	{
		if (0 < m_top_field)
			--m_top_field;
		if (!m_top_field)
			m_pointer_action = pointer_action::NONE;
	}
	return true;
}


bool menu_analog::update_scroll_drag(ui_event const &uievt)
{
	// set thresholds depending on the direction for hysteresis
	float const y(pointer_location().second);
	float const base(m_base_pointer.second + (line_height() * ((y > m_last_pointer.second) ? -0.3F : 0.3F)));
	auto const target(int((base - y) / line_height()));
	m_last_pointer.second = base + (float(target) * line_height());

	// scroll if it moved
	int newtop(std::clamp<int>(m_scroll_base + target, 0, m_field_data.size() - m_visible_fields));
	if (!m_hide_menu && (newtop != m_top_field))
	{
		// if the menu is visible, keep the highlighted field on-screen
		void *const selectedref(get_selection_ref());
		ioport_field *const selfield(selectedref ? &reinterpret_cast<item_data *>(selectedref)->field.get() : nullptr);
		if (selfield)
		{
			auto const found(
					std::find_if(
						m_field_data.begin(),
						m_field_data.end(),
						[selfield] (field_data const &d) { return &d.field.get() == selfield; }));
			if (m_field_data.end() != found)
			{
				auto const i(std::distance(m_field_data.begin(), found));
				newtop = std::clamp<int>(newtop, i + 1 - m_visible_fields, i);
			}
		}
	}
	bool const scrolled(newtop != m_top_field);
	m_top_field = newtop;

	// catch the end of the gesture
	if ((uievt.pointer_released & 0x01) || (uievt.pointer_pressed & ~u32(0x01)))
		m_pointer_action = pointer_action::NONE;
	return scrolled;
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
