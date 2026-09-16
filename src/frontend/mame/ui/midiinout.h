// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/midiinout.h

    Midi channel selection

***************************************************************************/

#ifndef MAME_FRONTEND_UI_MIDIINOUT_H
#define MAME_FRONTEND_UI_MIDIINOUT_H

#pragma once

#include "ui/menu.h"

#include <functional>
#include <string>
#include <vector>


namespace ui {

class menu_midi_inout : public menu
{
public:
	using handler_function = std::function<void (std::string const &)>;

	menu_midi_inout(
			mame_ui_manager &mui,
			render_target &target,
			bool is_input,
			handler_function &&handler);
	virtual ~menu_midi_inout() override;

private:
	handler_function const      m_handler;
	std::vector<std::string>    m_port_names;
	bool const                  m_is_input;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_MIDIINOUT_H
