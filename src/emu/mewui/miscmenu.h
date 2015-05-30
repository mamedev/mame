/***************************************************************************

	mewui/miscmenu.h

	Internal MEWUI user interface.

***************************************************************************/
#pragma once

#ifndef __MEWUI_MISCMENU_H__
#define __MEWUI_MISCMENU_H__

//-------------------------------------------------
//  class miscellaneous options menu
//-------------------------------------------------
class ui_menu_misc_options : public ui_menu
{
public:
	ui_menu_misc_options(running_machine &machine, render_container *container);
	virtual ~ui_menu_misc_options();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	enum
	{
		REMEMBER_LAST_GAME = 1,
		ENLARGE_ARTS,
		DATS_ENABLED,
		CHEAT_ENABLED,
		MOUSE_ENABLED,
		CONFIRM_QUIT_ENABLED,
		SKIP_GAMEINFO_ENABLED,
		FORCED_4X3,
		USE_BGRND,
		LAST_MOPTION
	};

	bool m_options[LAST_MOPTION];
};

#endif /* __MEWUI_MISCMENU_H__ */
