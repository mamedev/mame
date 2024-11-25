// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/custui.h

    Internal UI user interface.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_CUSTUI_H
#define MAME_FRONTEND_UI_CUSTUI_H

#pragma once

#include "ui/menu.h"

#include <functional>
#include <string>
#include <vector>


namespace ui {

//-------------------------------------------------
//  Custom UI menu
//-------------------------------------------------

class menu_custom_ui : public menu
{
public:
	menu_custom_ui(mame_ui_manager &mui, render_container &container, std::function<void ()> &&handler);

protected:
	virtual void menu_dismissed() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	void find_languages();
	void find_sysnames();

	std::function<void ()>      m_handler;
	std::vector<std::string>    m_languages;
	std::vector<std::string>    m_sysnames;
	std::size_t                 m_currlang;
	std::size_t                 m_currsysnames;
	u8                          m_currpanels;
};

//-------------------------------------------------
//  Font UI menu
//-------------------------------------------------

class menu_font_ui : public menu
{
public:
	menu_font_ui(mame_ui_manager &mui, render_container &container, std::function<void (bool)> &&handler);

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual void menu_dismissed() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	void list();

	std::function<void (bool)> m_handler;
	std::vector<std::pair<std::string, std::string> > m_fonts;
	int const m_font_min, m_font_max;
	int m_font_size;
	float const m_info_min, m_info_max;
	float m_info_size;
	bool m_face_changed;
	bool m_changed;

	std::uint16_t m_actual;
#ifdef UI_WINDOWS
	bool m_bold, m_italic;
#endif

};

//-------------------------------------------------
//  Colors UI menu
//-------------------------------------------------

class menu_colors_ui : public menu
{
public:
	menu_colors_ui(mame_ui_manager &mui, render_container &container);

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual void menu_dismissed() override;

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
	virtual bool handle(event const *ev) override;

	s_color_table m_color_table[MUI_RESTORE];
	void restore_colors();
};

//-------------------------------------------------
//  ARGB UI menu
//-------------------------------------------------

class menu_rgb_ui : public menu
{
public:
	menu_rgb_ui(mame_ui_manager &mui, render_container &container, rgb_t *color, std::string &&title);

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;

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
	virtual bool handle(event const *ev) override;

	bool inkey_special(const event *menu_event);

	rgb_t           *m_color;
	std::string     m_search;
	bool            m_key_active;
	int             m_lock_ref;
};

//-------------------------------------------------
//  Palette UI menu
//-------------------------------------------------

class menu_palette_sel : public menu
{
public:
	menu_palette_sel(mame_ui_manager &mui, render_container &container, rgb_t &_color);

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	static std::pair<const char *, const char *> const s_palette[];
	rgb_t &m_original;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_CUSTUI_H
