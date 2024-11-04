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


namespace ui {

class menu_midi_inout : public menu
{
public:
	menu_midi_inout(mame_ui_manager &mui, render_container &container, bool is_input, std::string *channel);
	virtual ~menu_midi_inout() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual void menu_activated() override;
	virtual void menu_deactivated() override;

private:
	std::vector<std::string> m_port_names;
	std::string *m_channel;
	bool m_is_input;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_MIDIINOUT_H
