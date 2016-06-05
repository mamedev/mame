// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/miscmenu.h

    Internal MAME menus for the user interface.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_SLIDERS_H
#define MAME_FRONTEND_UI_SLIDERS_H

#include "ui/menu.h"

namespace ui {

class menu_sliders : public menu
{
public:
	menu_sliders(mame_ui_manager &mui, render_container *container, bool menuless_mode = false);
	virtual ~menu_sliders() override;
	virtual void populate() override;
	virtual void handle() override;

	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	static UINT32 ui_handler(render_container *container, mame_ui_manager &mui, bool state);

private:
	enum {
		INPUT_GROUPS,
		INPUT_SPECIFIC,
	};
	bool m_menuless_mode;
	bool m_hidden;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_SLIDERS_H */
