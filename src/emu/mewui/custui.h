// license:BSD-3-Clause
// copyright-holders:Dankan1890
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
	rgb_t       color;
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
		FONT_MENU = 1,
		COLORS_MENU
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
	bool      m_bold, m_italic;

	void      list();
#endif

	float m_info_min, m_info_max, m_info_size;
	int m_font_min, m_font_max, m_font_size;
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

	s_color_table m_color_table[MUI_RESTORE];
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

	rgb_t           *m_color;
	char            m_search[4];
	bool            m_key_active;
	int             m_lock_ref;
	std::string     m_title;

	enum
	{
		RGB_ALPHA = 1,
		RGB_RED,
		RGB_GREEN,
		RGB_BLUE,
		PALETTE_CHOOSE
	};

	void inkey_special(const ui_menu_event *menu_event);
};

/***************************************************************************
    PALETTE UI CLASS
***************************************************************************/

struct palcolor
{
	const char *name;
	const char *argb;
};

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

#endif /* __MEWUI_CUSTUI_H__ */
