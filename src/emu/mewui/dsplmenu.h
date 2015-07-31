// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/dsplmenu.h

    Internal MEWUI user interface.

***************************************************************************/

#pragma once

#ifndef __MEWUI_DSPLMENU_H__
#define __MEWUI_DSPLMENU_H__

//-------------------------------------------------
//  class display options menu
//-------------------------------------------------
class ui_menu_display_options : public ui_menu
{
public:
	ui_menu_display_options(running_machine &machine, render_container *container);
	virtual ~ui_menu_display_options();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	enum
	{
		VIDEO_MODE = 1,
		HWSTRETCH_ENABLED,
		FILTER_ENABLED,
		PRESCALE_ENABLED,
		HLSL_ENABLED,
		MT_ENABLED,
		WINDOW_ENABLED,
		KAR_ENABLED,
		TBUFFER_ENABLED,
		MAXIM_ENABLED,
		SYNCREF_ENABLED,
		WAITSYNC_ENABLED,
		LAST_DISPLAY
	};

	UINT16            cur_video;
	static const char *video_modes[], *video_modes_label[];
	bool              m_options[LAST_DISPLAY];
};

#endif /* __MEWUI_DSPLMENU_H__ */
