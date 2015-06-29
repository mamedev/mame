/***************************************************************************

    mewui/palsel.h

    Internal MEWUI user interface.

***************************************************************************/

#pragma once

#ifndef __MEWUI_PALSEL_H__
#define __MEWUI_PALSEL_H__

struct palcolor
{
	const char *m_name;
	const char *m_argb;
};

//-------------------------------------------------
//  class sound options menu
//-------------------------------------------------
class ui_menu_palette_sel : public ui_menu
{
public:
	ui_menu_palette_sel(running_machine &machine, render_container *container, rgb_t &_color);
	virtual ~ui_menu_palette_sel();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	rgb_t &m_original;
};

#endif /* __MEWUI_PALSEL_H__ */
