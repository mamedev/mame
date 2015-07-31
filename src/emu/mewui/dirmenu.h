// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/dirmenu.h

    Internal MEWUI user interface.

***************************************************************************/

#pragma once

#ifndef __MEWUI_DIRMENU_H__
#define __MEWUI_DIRMENU_H__

// GLOBAL
enum
{
	ROM_FOLDERS = 1,
	MEWUI_FOLDERS,
	SAMPLE_FOLDERS,
	HISTORY_FOLDERS,
	INI_FOLDERS,
	EXTRAINI_FOLDERS,
	ICON_FOLDERS,
	CHEAT_FOLDERS,
	SNAPSHOT_FOLDERS,
	CABINET_FOLDERS,
	FLYER_FOLDERS,
	TITLE_FOLDERS,
	PCB_FOLDERS,
	MARQUEES_FOLDERS,
	CPANEL_FOLDERS,
	CROSSHAIR_FOLDERS,
	ARTWORK_FOLDERS,
	BOSSES_FOLDERS,
	ARTPREV_FOLDERS,
	SELECT_FOLDERS,
	GAMEOVER_FOLDERS,
	HOWTO_FOLDERS,
	LOGO_FOLDERS,
	SCORES_FOLDERS,
	VERSUS_FOLDERS
};

struct folders_entry
{
	const char *name;
	const char *option;
};

//-------------------------------------------------
//  class directory menu
//-------------------------------------------------

class ui_menu_directory : public ui_menu
{
public:
	ui_menu_directory(running_machine &machine, render_container *container);
	virtual ~ui_menu_directory();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);
};

//-------------------------------------------------
//  class directory specific menu
//-------------------------------------------------

class ui_menu_display_actual : public ui_menu
{
public:
	ui_menu_display_actual(running_machine &machine, render_container *container, int selectedref);
	virtual ~ui_menu_display_actual();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	std::string              tempbuf, searchpath;
	std::vector<std::string> folders;
	int                      ref_path;

	enum
	{
		ADD_FOLDER = 1,
		REMOVE_FOLDER,
		CHANGE_FOLDER
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
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	std::string  searchpath;
	int          path_ref;
};

//-------------------------------------------------
//  class add / change folder menu
//-------------------------------------------------

class ui_menu_add_change_folder : public ui_menu
{
public:
	ui_menu_add_change_folder(running_machine &machine, render_container *container, int ref, bool change);
	virtual ~ui_menu_add_change_folder();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	virtual bool menu_has_search_active() { return (m_search[0] != 0); }

private:
	int          path_ref;
	std::string  current_path;
	char         m_search[40];
	bool         change;
};

#endif /* __MEWUI_DIRMENU_H__ */
