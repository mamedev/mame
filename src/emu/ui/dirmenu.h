// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/dirmenu.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef __UI_DIRMENU_H__
#define __UI_DIRMENU_H__

//-------------------------------------------------
//  class directory menu
//-------------------------------------------------

class ui_menu_directory : public ui_menu
{
public:
	ui_menu_directory(running_machine &machine, render_container *container);
	virtual ~ui_menu_directory();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
};

//-------------------------------------------------
//  class directory specific menu
//-------------------------------------------------

class ui_menu_display_actual : public ui_menu
{
public:
	ui_menu_display_actual(running_machine &machine, render_container *container, int selectedref);
	virtual ~ui_menu_display_actual();
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

class ui_menu_remove_folder : public ui_menu
{
public:
	ui_menu_remove_folder(running_machine &machine, render_container *container, int ref);
	virtual ~ui_menu_remove_folder();
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

class ui_menu_add_change_folder : public ui_menu
{
public:
	ui_menu_add_change_folder(running_machine &machine, render_container *container, int ref);
	virtual ~ui_menu_add_change_folder();
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

#endif /* __UI_DIRMENU_H__ */
