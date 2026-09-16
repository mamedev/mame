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

menu_midi_inout::menu_midi_inout(
		mame_ui_manager &mui,
		render_target &target,
		bool is_input,
		handler_function &&handler)
	: menu(mui, target)
	, m_handler(std::move(handler))
	, m_is_input(is_input)
{
	set_heading(m_is_input ? _("menu-midiport", "MIDI input port") : _("menu-midiport", "MIDI output port"));
}

menu_midi_inout::~menu_midi_inout()
{
}

bool menu_midi_inout::handle(event const *ev)
{
	if (!ev)
		return false;

	if (ev->iptkey == IPT_UI_SELECT)
		m_handler(m_port_names[uintptr_t(ev->itemref)]);

	return false;
}


//-------------------------------------------------
//  menu_midi_inout_populate - populate the midi_inout
//  menu
//-------------------------------------------------

void menu_midi_inout::populate()
{
	m_port_names.clear();
	auto const ports = machine().osd().list_midi_ports();
	m_port_names.reserve(ports.size());
	for (auto &p : ports)
	{
		if ((m_is_input && p.input) || (!m_is_input && p.output))
		{
			item_append(p.name, 0, (void *)(m_port_names.size()));
			m_port_names.emplace_back(p.name);
		}
	}

	if (m_port_names.empty())
	{
		item_append(
				m_is_input ? _("menu-midiport", "[no MIDI input ports available]") : _("menu-midiport", "[no MIDI output ports available]"),
				FLAG_DISABLE,
				nullptr);
	}

	item_append(menu_item_type::SEPARATOR);
}

} // namespace ui
