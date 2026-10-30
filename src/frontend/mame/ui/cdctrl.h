// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    ui/cdctrl.h

    Audio CD player control

***************************************************************************/

#ifndef MAME_FRONTEND_UI_CDCTRL_H
#define MAME_FRONTEND_UI_CDCTRL_H

#pragma once

#include "ui/devctrl.h"

#include "imagedev/cdromimg.h"

#include "notifier.h"


namespace ui {

class menu_cd_control : public menu_device_control<device_cd_player_interface, cd_player_interface_enumerator>
{
public:
	menu_cd_control(mame_ui_manager &mui, render_target &target, device_cd_player_interface *device);
	virtual ~menu_cd_control() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	util::notifier_subscription m_notifier;
	int m_status_item_index;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_CDCTRL_H
