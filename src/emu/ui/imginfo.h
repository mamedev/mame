/***************************************************************************

    ui/imginfo.h

    Image info screen

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_IMGINFO_H__
#define __UI_IMGINFO_H__

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

#endif // __UI_IMGINFO_H__
