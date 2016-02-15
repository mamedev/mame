// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/cheatopt.h

    Internal menu for the cheat interface.

***************************************************************************/

#pragma once

#ifndef __UI_CHEATOPT_H__
#define __UI_CHEATOPT_H__

// itemrefs for key menu items
#define ITEMREF_CHEATS_RESET_ALL            ((void *) 0x0001)
#define ITEMREF_CHEATS_RELOAD_ALL           ((void *) 0x0002)
#define ITEMREF_CHEATS_AUTOFIRE_SETTINGS    ((void *) 0x0003)
#define ITEMREF_CHEATS_FIRST_ITEM           ((void *) 0x0004)

class ui_menu_cheat : public ui_menu {
public:
	ui_menu_cheat(running_machine &machine, render_container *container);
	virtual ~ui_menu_cheat();
	virtual void populate() override;
	virtual void handle() override;
};


// itemrefs for key menu items
#define ITEMREF_AUTOFIRE_STATUS       ((void *) 0x0001)
#define ITEMREF_AUTOFIRE_DELAY        ((void *) 0x0002)
#define ITEMREF_AUTOFIRE_FIRST_BUTTON ((void *) 0x0003)

class ui_menu_autofire : public ui_menu {
public:
	ui_menu_autofire(running_machine &machine, render_container *container);
	virtual ~ui_menu_autofire();
	virtual void populate() override;
	virtual void handle() override;

private:
	float refresh;
	bool last_toggle;
};


#endif  /* __UI_CHEATOPT_H__ */
