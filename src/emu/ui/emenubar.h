/***************************************************************************

    emenubar.h

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_EMENUBAR_H__
#define __UI_EMENUBAR_H__

#include "ui/menubar.h"

class ui_emu_menubar : public ui_menubar
{
public:
	ui_emu_menubar(running_machine &machine, render_container *container);

protected:
	virtual void build_menus();

private:
	char			m_dummy[256];
};


#endif // __UI_EMENUBAR_H__
