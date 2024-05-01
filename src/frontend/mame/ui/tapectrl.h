// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/tapectrl.h

    Tape control

***************************************************************************/

#ifndef MAME_FRONTEND_UI_TAPECTRL_H
#define MAME_FRONTEND_UI_TAPECTRL_H

#pragma once

#include "ui/devctrl.h"

#include "imagedev/cassette.h"

#include "notifier.h"


namespace ui {

class menu_tape_control : public menu_device_control<cassette_image_device>
{
public:
	menu_tape_control(mame_ui_manager &mui, render_container &container, cassette_image_device *device);
	virtual ~menu_tape_control() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	util::notifier_subscription m_notifier;
	int m_slider_item_index;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_TAPECTRL_H
