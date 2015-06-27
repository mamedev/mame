/***************************************************************************

    mewui/custmenu.h

    Internal MEWUI user interface.


***************************************************************************/

#pragma once

#ifndef __MEWUI_CUSTMENU_H__
#define __MEWUI_CUSTMENU_H__

#include "mewui/utils.h"

//-------------------------------------------------
//  custom filter menu class
//-------------------------------------------------
class ui_menu_custom_filter : public ui_menu
{
public:
	ui_menu_custom_filter(running_machine &machine, render_container *container, bool _single_menu = false);
	virtual ~ui_menu_custom_filter();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	enum
	{
		MAIN_FILTER = 1,
		ADD_FILTER,
		REMOVE_FILTER,
		MNFCT_FILTER,
		YEAR_FILTER = MNFCT_FILTER + MAX_CUST_FILTER + 1,
		OTHER_FILTER = YEAR_FILTER + MAX_CUST_FILTER + 1
	};

	bool single_menu, added;
};

#endif  /* __MEWUI_CUSTMENU_H__ */
