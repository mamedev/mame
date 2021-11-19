// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/devopt.h

    Internal menu for the device configuration.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_DEVOPT_H
#define MAME_FRONTEND_UI_DEVOPT_H

#pragma once

#include "ui/textbox.h"


namespace ui {

class menu_device_config : public menu_textbox
{
public:
	menu_device_config(mame_ui_manager &mui, render_container &container, device_slot_interface *slot, device_slot_interface::slot_option const *option);
	virtual ~menu_device_config() override;

protected:
	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	device_slot_interface::slot_option const *const m_option;
	bool const m_mounted;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_DEVOPT_H
