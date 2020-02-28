// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Vas Crabb
/*********************************************************************

    ui/confswitch.cpp

    Configuration/DIP switches menu.

*********************************************************************/

#include "emu.h"
#include "ui/confswitch.h"

#include <algorithm>
#include <cstring>


namespace ui {

namespace {

/***************************************************************************
    CONSTANTS
***************************************************************************/

// DIP switch rendering parameters in terms of line height
constexpr float DIP_SWITCH_HEIGHT = 1.6f;
constexpr float DIP_SWITCH_SPACING = DIP_SWITCH_HEIGHT / 5.0f;
constexpr float SINGLE_TOGGLE_SWITCH_FIELD_WIDTH = DIP_SWITCH_HEIGHT / 2.0f;
constexpr float SINGLE_TOGGLE_SWITCH_WIDTH = SINGLE_TOGGLE_SWITCH_FIELD_WIDTH * 0.8f;
// make the switch nub 80% of the width space and 1/2 of the switch height
constexpr float SINGLE_TOGGLE_SWITCH_HEIGHT = (DIP_SWITCH_HEIGHT / 2.0f) * 0.8f;

} // anonymous namespace



/*-------------------------------------------------
    menu_confswitch
-------------------------------------------------*/

menu_confswitch::field_descriptor::field_descriptor(ioport_field &f) noexcept
	: field(f)
{
}


menu_confswitch::switch_group_descriptor::switch_group_descriptor(ioport_field const &f, ioport_diplocation const &loc) noexcept
	: name(loc.name())
	, owner(f.device())
	, mask(0U)
	, state(0U)
{
}


inline bool menu_confswitch::switch_group_descriptor::matches(ioport_field const &f, ioport_diplocation const &loc) const noexcept
{
	return (&owner.get() == &f.device()) && !strcmp(loc.name(), name);
}


inline unsigned menu_confswitch::switch_group_descriptor::switch_count() const noexcept
{
	return (sizeof(mask) * 8) - count_leading_zeros(mask);
}


menu_confswitch::menu_confswitch(mame_ui_manager &mui, render_container &container, uint32_t type)
	: menu(mui, container)
	, m_fields()
	, m_switch_groups()
	, m_active_switch_groups(0U)
	, m_type(type)
{
}


menu_confswitch::~menu_confswitch()
{
}


void menu_confswitch::populate(float &customtop, float &custombottom)
{
	// locate relevant fields if necessary
	if (m_fields.empty())
		find_fields();

	// reset switch group masks
	m_active_switch_groups = 0U;
	for (switch_group_descriptor &group : m_switch_groups)
		group.mask = group.state = 0U;

	// loop over input ports and set up the current values
	device_t *prev_owner(nullptr);
	bool first_entry(true);
	for (field_descriptor &desc : m_fields)
	{
		ioport_field &field(desc.field);
		if (field.enabled())
		{
			if (!field.settings().empty())
			{
				// add a device heading if necessary
				if (&field.device() != prev_owner)
				{
					prev_owner = &field.device();
					if (first_entry)
						first_entry = false;
					else
						item_append(menu_item_type::SEPARATOR);
					item_append(string_format("[root%s]", prev_owner->tag()), "", 0, nullptr);
				}

				// set the left/right flags appropriately
				uint32_t flags(0U);
				if (field.has_previous_setting())
					flags |= FLAG_LEFT_ARROW;
				if (field.has_next_setting())
					flags |= FLAG_RIGHT_ARROW;

				// add the menu item
				item_append(field.name(), field.setting_name(), flags, &field);
			}

			// track switch groups
			if (!field.diplocations().empty())
			{
				// get current settings
				ioport_field::user_settings settings;
				field.get_user_settings(settings);

				// iterate over each bit in the field
				ioport_value accummask(field.mask());
				for (ioport_diplocation const &loc : field.diplocations())
				{
					// find the matching switch group
					switch_group_descriptor &group(
							*std::find_if(
								m_switch_groups.begin(),
								m_switch_groups.end(), [&field, &loc] (switch_group_descriptor const &sw) { return sw.matches(field, loc); }));

					// count if this is the first switch in the group
					if (!group.mask)
						++m_active_switch_groups;

					// apply the bits
					group.mask |= uint32_t(1) << (loc.number() - 1);
					ioport_value const mask(accummask & ~(accummask - 1));
					if (((settings.value & mask) && !loc.inverted()) || (!(settings.value & mask) && loc.inverted()))
						group.state |= uint32_t(1) << (loc.number() - 1);

					// clear the relevant bit in the accumulated mask
					accummask &= ~mask;
				}
			}
		}
	}

	item_append(menu_item_type::SEPARATOR);
	item_append(_("Reset"), "", 0, (void *)1);
}


void menu_confswitch::handle()
{
	// process the menu
	event const *const menu_event(process(0));

	// handle events
	if (menu_event && menu_event->itemref)
	{
		if (uintptr_t(menu_event->itemref) == 1U)
		{
			// reset
			if (menu_event->iptkey == IPT_UI_SELECT)
				machine().schedule_hard_reset();
		}
		else
		{
			// actual settings
			ioport_field &field(*reinterpret_cast<ioport_field *>(menu_event->itemref));
			bool changed(false);

			switch (menu_event->iptkey)
			{
			// if selected, reset to default value
			case IPT_UI_SELECT:
				{
					ioport_field::user_settings settings;
					field.get_user_settings(settings);
					settings.value = field.defvalue();
					field.set_user_settings(settings);
				}
				changed = true;
				break;

			// left goes to previous setting
			case IPT_UI_LEFT:
				field.select_previous_setting();
				changed = true;
				break;

			// right goes to next setting
			case IPT_UI_RIGHT:
				field.select_next_setting();
				changed = true;
				break;
			}

			// if anything changed, rebuild the menu, trying to stay on the same field
			if (changed)
				reset(reset_options::REMEMBER_REF);
		}
	}
}


void menu_confswitch::find_fields()
{
	assert(m_fields.empty());
	assert(m_switch_groups.empty());

	// find relevant input ports
	for (auto &port : machine().ioport().ports())
	{
		for (ioport_field &field : port.second->fields())
		{
			if (field.type() == m_type)
			{
				m_fields.emplace_back(field);

				// iterate over locations
				for (ioport_diplocation const &loc : field.diplocations())
				{
					auto const found(
							std::find_if(
								m_switch_groups.begin(),
								m_switch_groups.end(), [&field, &loc] (switch_group_descriptor const &sw) { return sw.matches(field, loc); }));
					if (m_switch_groups.end() == found)
						m_switch_groups.emplace_back(field, loc);
				}
			}
		}
	}
}



/*-------------------------------------------------
    menu_settings_dip_switches
-------------------------------------------------*/

menu_settings_dip_switches::menu_settings_dip_switches(mame_ui_manager &mui, render_container &container)
	: menu_confswitch(mui, container, IPT_DIPSWITCH)
	, m_visible_switch_groups(0U)
{
}


menu_settings_dip_switches::~menu_settings_dip_switches()
{
}


void menu_settings_dip_switches::custom_render(void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
	// catch if no diploc has to be drawn
	if (!m_visible_switch_groups)
		return;

	// calculate optimal width
	float const aspect(machine().render().ui_aspect(&container()));
	float const lineheight(ui().get_line_height());
	float const singlewidth(lineheight * SINGLE_TOGGLE_SWITCH_FIELD_WIDTH * aspect);
	float const maxwidth(1.0f - (ui().box_lr_border() * 2.0f * aspect));
	float width(0.0f);
	unsigned maxswitches(0U);
	for (switch_group_descriptor const &group : switch_groups())
	{
		if (group.mask)
		{
			maxswitches = (std::max)(group.switch_count(), maxswitches);
			float const namewidth(ui().get_string_width(group.name));
			float const switchwidth(singlewidth * maxswitches);
			width = (std::min)((std::max)(namewidth + switchwidth + (lineheight * aspect), width), maxwidth);
		}
	}

	// draw extra menu area
	float const boxwidth((std::max)(width + (ui().box_lr_border() * 2.0f * aspect), x2 - x1));
	float const boxleft((1.0f - boxwidth) / 2.0f);
	ui().draw_outlined_box(container(), boxleft, y2 + ui().box_tb_border(), boxleft + boxwidth, y2 + bottom, ui().colors().background_color());

	// calculate centred layout
	float const nameleft((1.0f - width) / 2.0f);
	float const switchleft(nameleft + width - (singlewidth * maxswitches));
	float const namewidth(width - (singlewidth * maxswitches) - (lineheight * aspect));

	// iterate over switch groups
	ioport_field *const field((uintptr_t(selectedref) != 1U) ? reinterpret_cast<ioport_field *>(selectedref) : nullptr);
	float const nubheight(lineheight * SINGLE_TOGGLE_SWITCH_HEIGHT);
	float const nubwidth(lineheight * SINGLE_TOGGLE_SWITCH_WIDTH * aspect);
	float const ygap(lineheight * ((DIP_SWITCH_HEIGHT / 2) - SINGLE_TOGGLE_SWITCH_HEIGHT) / 2);
	float const xgap((singlewidth + (UI_LINE_WIDTH / 2) - nubwidth) / 2);
	unsigned line(0U);
	for (switch_group_descriptor const &group : switch_groups())
	{
		if (group.mask)
		{
			// determine the mask of selected bits
			uint32_t selectedmask(0U);
			if (field)
			{
				for (ioport_diplocation const &loc : field->diplocations())
					if (group.matches(*field, loc))
						selectedmask |= uint32_t(1) << (loc.number() - 1);
			}

			// draw the name
			float const liney(y2 + (ui().box_tb_border() * 2.0f) + (lineheight * (DIP_SWITCH_HEIGHT + DIP_SWITCH_SPACING) * line));
			ui().draw_text_full(
					container(),
					group.name,
					nameleft, liney + (lineheight * (DIP_SWITCH_HEIGHT - 1.0f) / 2.0f), namewidth,
					ui::text_layout::RIGHT, ui::text_layout::NEVER,
					mame_ui_manager::NORMAL, ui().colors().text_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			// draw the group outline
			float const switchbottom(liney + (DIP_SWITCH_HEIGHT * lineheight));
			unsigned const cnt(group.switch_count());
			ui().draw_outlined_box(
					container(),
					switchleft, liney, switchleft + (singlewidth * cnt), switchbottom,
					ui().colors().background_color());
			for (unsigned i = 1; cnt > i; ++i)
			{
				container().add_line(
						switchleft + (singlewidth * i), liney, switchleft + (singlewidth * i), switchbottom,
						UI_LINE_WIDTH, ui().colors().text_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			}

			// compute top and bottom for on and off positions
			float const yoff(liney + UI_LINE_WIDTH + ygap);
			float const yon(switchbottom - UI_LINE_WIDTH - ygap - nubheight);

			// draw the switch nubs
			for (unsigned toggle = 0; cnt > toggle; ++toggle)
			{
				float const nubleft(switchleft + (singlewidth * toggle) + xgap);
				if (BIT(group.mask, toggle))
				{
					float const nubtop(BIT(group.state, toggle) ? yon : yoff);
					container().add_rect(
							nubleft, nubtop, nubleft + nubwidth, nubtop + nubheight,
							BIT(selectedmask, toggle) ? ui().colors().dipsw_color() : ui().colors().text_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
				}
				else
				{
					container().add_rect(
							nubleft, yoff, nubleft + nubwidth, yon + nubheight,
							ui().colors().unavailable_color(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
				}
			}

			// limit the number of visible switch groups
			if (++line >= m_visible_switch_groups)
				break;
		}
	}
}


void menu_settings_dip_switches::populate(float &customtop, float &custombottom)
{
	// let the base class add items
	menu_confswitch::populate(customtop, custombottom);

	// use up to about 70% of height for DIP switch display
	if (active_switch_groups())
	{
		float const lineheight(ui().get_line_height());
		float const groupheight(DIP_SWITCH_HEIGHT * lineheight);
		float const groupspacing(DIP_SWITCH_SPACING * lineheight);
		if ((active_switch_groups() * (groupheight + groupspacing)) > 0.7f)
			m_visible_switch_groups = unsigned(0.7f / (groupheight + groupspacing));
		else
			m_visible_switch_groups = active_switch_groups();
		custombottom = (m_visible_switch_groups * groupheight) + ((m_visible_switch_groups - 1) * groupspacing) + (ui().box_tb_border() * 3.0f);
	}
	else
	{
		m_visible_switch_groups = 0U;
		custombottom = 0.0f;
	}
}



/*-------------------------------------------------
    menu_settings_machine_config
-------------------------------------------------*/

menu_settings_machine_config::menu_settings_machine_config(mame_ui_manager &mui, render_container &container) : menu_confswitch(mui, container, IPT_CONFIG)
{
}

menu_settings_machine_config::~menu_settings_machine_config()
{
}


} // namespace ui
