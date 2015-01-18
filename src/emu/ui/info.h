/***************************************************************************

    ui/info.h

    System and image info screens

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_INFO_H__
#define __UI_INFO_H__

class ui_menu_game_info : public ui_menu {
public:
	ui_menu_game_info(running_machine &machine, render_container *container);
	virtual ~ui_menu_game_info();
	virtual void populate();
	virtual void handle();
};


class ui_menu_image_info : public ui_menu
{
public:
	ui_menu_image_info(running_machine &machine, render_container *container);
	virtual ~ui_menu_image_info();
	virtual void populate();
	virtual void handle();

private:
	void image_info(device_image_interface *image);
};

#endif // __UI_INFO_H__
