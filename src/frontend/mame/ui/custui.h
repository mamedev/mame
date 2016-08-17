// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/custui.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_CUSTUI_H
#define MAME_FRONTEND_UI_CUSTUI_H

#include "ui/menu.h"

namespace ui {
//-------------------------------------------------
//  Custom UI menu
//-------------------------------------------------

class menu_custom_ui : public menu
{
public:
	menu_custom_ui(mame_ui_manager &mui, render_container &container);
	virtual ~menu_custom_ui() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	enum
	{
		LANGUAGE_MENU = 1,
		FONT_MENU,
		COLORS_MENU,
		HIDE_MENU
	};

	virtual void populate() override;
	virtual void handle() override;

	static const char *const    HIDE_STATUS[];

	std::vector<std::string>    m_lang;
	std::uint16_t               m_currlang;
};

//-------------------------------------------------
//  Font UI menu
//-------------------------------------------------

class menu_font_ui : public menu
{
public:
	menu_font_ui(mame_ui_manager &mui, render_container &container);
	virtual ~menu_font_ui() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	enum
	{
		INFOS_SIZE = 1,
		FONT_SIZE,
		MUI_FNT,
		MUI_BOLD,
		MUI_ITALIC
	};

	virtual void populate() override;
	virtual void handle() override;

	void list();

	std::uint16_t                                       m_actual;
	std::vector<std::pair<std::string, std::string> >   m_fonts;
#ifdef UI_WINDOWS
	bool                                                m_bold, m_italic;
#endif

	float m_info_min, m_info_max, m_info_size;
	int m_font_min, m_font_max, m_font_size;
};

//-------------------------------------------------
//  Colors UI menu
//-------------------------------------------------

class menu_colors_ui : public menu
{
public:
	menu_colors_ui(mame_ui_manager &mui, render_container &container);
	virtual ~menu_colors_ui() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
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

	struct s_color_table
	{
		rgb_t       color;
		const char  *option;
	};

	virtual void populate() override;
	virtual void handle() override;

	s_color_table m_color_table[MUI_RESTORE];
	void restore_colors();
};

//-------------------------------------------------
//  ARGB UI menu
//-------------------------------------------------

class menu_rgb_ui : public menu
{
public:
	menu_rgb_ui(mame_ui_manager &mui, render_container &container, rgb_t *_color, std::string _title);
	virtual ~menu_rgb_ui() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	enum
	{
		RGB_ALPHA = 1,
		RGB_RED,
		RGB_GREEN,
		RGB_BLUE,
		PALETTE_CHOOSE
	};

	virtual void populate() override;
	virtual void handle() override;

	void inkey_special(const event *menu_event);

	rgb_t           *m_color;
	char            m_search[4];
	bool            m_key_active;
	int             m_lock_ref;
	std::string     m_title;
};

//-------------------------------------------------
//  Palette UI menu
//-------------------------------------------------

class menu_palette_sel : public menu
{
public:
	menu_palette_sel(mame_ui_manager &mui, render_container &container, rgb_t &_color);
	virtual ~menu_palette_sel() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	// draw palette menu
	virtual void draw(UINT32 flags) override;

	virtual void populate() override;
	virtual void handle() override;

	static std::pair<const char *, const char *> const s_palette[];
	rgb_t &m_original;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_CUSTUI_H
