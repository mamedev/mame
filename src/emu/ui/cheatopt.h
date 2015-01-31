/***************************************************************************

    ui/cheatopt.h

    Internal menu for the cheat interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_CHEATOPT_H__
#define __UI_CHEATOPT_H__

class ui_menu_cheat : public ui_menu {
public:
	ui_menu_cheat(running_machine &machine, render_container *container);
	virtual ~ui_menu_cheat();
	virtual void populate();
	virtual void handle();
};

#endif  /* __UI_CHEATOPT_H__ */
