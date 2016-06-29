// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/dirmenu.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_DIRMENU_H
#define MAME_FRONTEND_UI_DIRMENU_H

#include "ui/menu.h"

#include <string>
#include <vector>

namespace ui {
//-------------------------------------------------
//  class directory menu
//-------------------------------------------------

class menu_directory : public menu
{
public:
	menu_directory(mame_ui_manager &mui, render_container *container);
	virtual ~menu_directory() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
};

//-------------------------------------------------
//  class directory specific menu
//-------------------------------------------------

class menu_display_actual : public menu
{
public:
	menu_display_actual(mame_ui_manager &mui, render_container *container, int selectedref);
	virtual ~menu_display_actual() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	std::string              m_tempbuf, m_searchpath;
	std::vector<std::string> m_folders;
	int                      m_ref;

	enum
	{
		ADD_CHANGE = 1,
		REMOVE,
	};
};

//-------------------------------------------------
//  class remove folder menu
//-------------------------------------------------

class menu_remove_folder : public menu
{
public:
	menu_remove_folder(mame_ui_manager &mui, render_container *container, int ref);
	virtual ~menu_remove_folder() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	std::string  m_searchpath;
	int          m_ref;
	std::vector<std::string> m_folders;
};

//-------------------------------------------------
//  class add / change folder menu
//-------------------------------------------------

class menu_add_change_folder : public menu
{
public:
	menu_add_change_folder(mame_ui_manager &mui, render_container *container, int ref);
	virtual ~menu_add_change_folder() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	virtual bool menu_has_search_active() override { return (m_search[0] != 0); }

private:
	int          m_ref;
	std::string  m_current_path;
	char         m_search[40];
	bool         m_change;
	std::vector<std::string> m_folders;
};

} // namesapce ui

#endif /* MAME_FRONTEND_UI_DIRMENU_H */
