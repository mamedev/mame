/***************************************************************************

    ui/bbcontrl.h

    MESS's "bit banger" control

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_BBCONTRL_H__
#define __UI_BBCONTRL_H__

#include "imagedev/bitbngr.h"
#include "ui/devctrl.h"

class ui_menu_mess_bitbanger_control : public ui_menu_device_control<bitbanger_device> {
public:
	ui_menu_mess_bitbanger_control(running_machine &machine, render_container *container, bitbanger_device *bitbanger);
	virtual ~ui_menu_mess_bitbanger_control();
	virtual void populate();
	virtual void handle();
};

#endif // __UI_BBCONTRL_H__
