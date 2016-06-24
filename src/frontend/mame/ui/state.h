// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

	ui/state.h

	Menus for saving and loading state

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_STATE_H
#define MAME_FRONTEND_UI_STATE_H

#include "ui/menu.h"

namespace ui {

// ======================> menu_load_save_state_base

class menu_load_save_state_base : public menu
{
public:
	virtual ~menu_load_save_state_base() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

protected:
	menu_load_save_state_base(mame_ui_manager &mui, render_container *container, const char *header, bool disable_not_found_items);
	virtual void process_file(const std::string &file_name) = 0;

private:
	const int SLOT_COUNT = 10;
	static int s_last_slot_selected;

	const char *m_header;
	bool m_disable_not_found_items;
	bool m_was_paused;
	UINT32 m_enabled_mask;

	void slot_selected(int slot);
	static void *itemref_from_slot_number(int slot);
};

// ======================> menu_load_state

class menu_load_state : public menu_load_save_state_base
{
public:
	menu_load_state(mame_ui_manager &mui, render_container *container);

protected:
	virtual void process_file(const std::string &file_name) override;
};

// ======================> menu_save_state

class menu_save_state : public menu_load_save_state_base
{
public:
	menu_save_state(mame_ui_manager &mui, render_container *container);

protected:
	virtual void process_file(const std::string &file_name) override;
};

}

#endif // MAME_FRONTEND_UI_STATE_H
