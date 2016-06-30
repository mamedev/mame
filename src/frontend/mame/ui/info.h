// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/info.h

    System and image info screens

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_INFO_H
#define MAME_FRONTEND_UI_INFO_H

#include "ui/menu.h"

namespace ui {
class menu_game_info : public menu
{
public:
	menu_game_info(mame_ui_manager &mui, render_container *container);
	virtual ~menu_game_info() override;
	virtual void populate() override;
	virtual void handle() override;
};


class menu_image_info : public menu
{
public:
	menu_image_info(mame_ui_manager &mui, render_container *container);
	virtual ~menu_image_info() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	void image_info(device_image_interface *image);
};

} // namespace ui

#endif // MAME_FRONTEND_UI_INFO_H
