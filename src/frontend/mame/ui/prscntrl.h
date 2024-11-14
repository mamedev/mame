// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/prscntrl.h

    MAME's clunky built-in preset selection in a fixed list

***************************************************************************/

#ifndef MAME_FRONTEND_UI_PRESETCNTRL_H
#define MAME_FRONTEND_UI_PRESETCNTRL_H

#pragma once

#include "ui/filesel.h"
#include "ui/menu.h"
#include "ui/swlist.h"


namespace ui {

// ======================> menu_control_device_preset

class menu_control_device_preset : public menu
{
public:
	menu_control_device_preset(mame_ui_manager &mui, render_container &container, device_image_interface &image);
	virtual ~menu_control_device_preset() override;

protected:
	// methods
	virtual bool handle(event const *ev) override;

private:
	// instance variables
	device_image_interface &        m_image;

	// methods
	virtual void populate() override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_PRESETCNTRL_H
