/***************************************************************************

    menubar.h

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_MENUBAR_H__
#define __UI_MENUBAR_H__

#include "render.h"
#include "ui/stackable.h"


//**************************************************************************
//  MENU BAR
//**************************************************************************

class ui_menubar : public ui_stackable
{
public:
	ui_menubar(running_machine &machine, render_container *container);

	virtual void reset();
	virtual void do_handle();
};


#endif /* __UI_MENUBAR_H__ */