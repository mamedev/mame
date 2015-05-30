/***************************************************************************

	mewui/custui.h

	Internal MEWUI user interface.

***************************************************************************/

#pragma once

#ifndef __MEWUI_CUSTUI_H__
#define __MEWUI_CUSTUI_H__

#ifdef MEWUI_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include "strconv.h"
#endif

struct s_color_table
{
	rgb_t	   color;
	const char  *option;
};

enum
{
	MUI_BACKGROUND_COLOR = 1,
	MUI_BORDER_COLOR,
	MUI_CLONE_COLOR,
	MUI_DIPSW_COLOR,
	MUI_GFXVIEWER_BG_COLOR,
	MUI_MOUSEDOWN_BG_COLOR,
	MUI_MOUSEDOWN_COLOR,
	MUI_MOUSEOVER_BG_COLOR,
	MUI_MOUSEOVER_COLOR,
	MUI_SELECTED_BG_COLOR,
	MUI_SELECTED_COLOR,
	MUI_SLIDER_COLOR,
	MUI_SUBITEM_COLOR,
	MUI_TEXT_BG_COLOR,
	MUI_TEXT_COLOR,
	MUI_UNAVAILABLE_COLOR,
	MUI_RESTORE
};

/***************************************************************************
	CUSTOM UI CLASS
***************************************************************************/
class ui_menu_custom_ui : public ui_menu
{
public:
	ui_menu_custom_ui(running_machine &machine, render_container *container);
	virtual ~ui_menu_custom_ui();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	enum
	{
		M_UI_FONT = 1,
		M_UI_COLORS
	};
};

#ifdef MEWUI_WINDOWS
// Fonts struct
struct c_uifonts
{
	std::vector<std::string> ui;
	UINT16 actual;
};
#endif

/***************************************************************************
	FONT UI CLASS
***************************************************************************/
class ui_menu_font_ui : public ui_menu
{
public:
	ui_menu_font_ui(running_machine &machine, render_container *container);
	virtual ~ui_menu_font_ui();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	enum
	{
		INFOS_SIZE = 1,
		FONT_SIZE,
		MUI_FNT,
		MUI_BOLD,
		MUI_ITALIC
	};

#ifdef MEWUI_WINDOWS
	c_uifonts m_class;
	void list();
	bool m_bold, m_italic;
#endif

	float info_min_size, info_max_size, info_size;
	int font_min_size, font_max_size, font_size;
};

/***************************************************************************
	COLORS UI CLASS
***************************************************************************/
class ui_menu_colors_ui : public ui_menu
{
public:
	ui_menu_colors_ui(running_machine &machine, render_container *container);
	virtual ~ui_menu_colors_ui();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:

	s_color_table color_table[MUI_RESTORE];
	void restore_colors();
};

/***************************************************************************
	RGB UI CLASS
***************************************************************************/
class ui_menu_rgb_ui : public ui_menu
{
public:
	ui_menu_rgb_ui(running_machine &machine, render_container *container, rgb_t *_color, std::string _title);
	virtual ~ui_menu_rgb_ui();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:

	rgb_t		   *color;
	char			m_search[4];
	bool			key_active;
	int			 lock_ref;
	std::string 	title;

	enum
	{
		RGB_ALPHA = 1,
		RGB_RED,
		RGB_GREEN,
		RGB_BLUE
	};

	void inkey_special(const ui_menu_event *menu_event);
};

#endif /* __MEWUI_CUSTUI_H__ */
