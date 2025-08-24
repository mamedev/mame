// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    ui/midiinout.cpp

    Midi channel selection

*********************************************************************/

#include "emu.h"
#include "ui/midiinout.h"

#include "ui/ui.h"

#include "osdepend.h"

namespace ui {

menu_midi_inout::menu_midi_inout(mame_ui_manager &mui, render_container &container, bool is_input, std::string *channel)
	: menu(mui, container)
	, m_channel(channel)
	, m_is_input(is_input)
{
	set_heading(m_is_input ? _("MIDI input channel") : _("MIDI output channel"));
}

menu_midi_inout::~menu_midi_inout()
{
}

bool menu_midi_inout::handle(event const *ev)
{
	if(!ev)
		return false;

	if(ev->iptkey == IPT_UI_SELECT) {
		*m_channel = m_port_names[uintptr_t(ev->itemref)];
		stack_pop();
		return true;
	}

	return false;
}


//-------------------------------------------------
//  menu_midi_inout_populate - populate the midi_inout
//  menu
//-------------------------------------------------

void menu_midi_inout::populate()
{
	auto ports = machine().osd().list_midi_ports();
	for(auto &p : ports)
		if((m_is_input && p.input) || (!m_is_input && p.output)) {
			item_append(p.name, "", 0, (void *)(m_port_names.size()));
			m_port_names.push_back(p.name);
		}
}


//-------------------------------------------------
//  recompute_metrics - recompute metrics
//-------------------------------------------------

void menu_midi_inout::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);
}


//-------------------------------------------------
//  menu_midi_inout_custom_render - perform our special
//  rendering
//-------------------------------------------------

void menu_midi_inout::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
}


//-------------------------------------------------
//  menu_activated - handle menu gaining focus
//-------------------------------------------------

void menu_midi_inout::menu_activated()
{
	// scripts or the other form of the menu could have changed something in the mean time
	reset(reset_options::REMEMBER_POSITION);
}


//-------------------------------------------------
//  menu_deactivated - handle menu losing focus
//-------------------------------------------------

void menu_midi_inout::menu_deactivated()
{
}

} // namespace ui

