// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/info.h

    System and image info screens

***************************************************************************/

#pragma once

#ifndef __UI_INFO_H__
#define __UI_INFO_H__

class ui_menu_game_info : public ui_menu {
public:
	ui_menu_game_info(running_machine &machine, render_container *container);
	virtual ~ui_menu_game_info();
	virtual void populate() override;
	virtual void handle() override;
};


class ui_menu_image_info : public ui_menu
{
public:
	ui_menu_image_info(running_machine &machine, render_container *container);
	virtual ~ui_menu_image_info();
	virtual void populate() override;
	virtual void handle() override;

private:
	void image_info(device_image_interface *image);
};

#endif // __UI_INFO_H__
