// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/selsoft.h

    MEWUI softwares menu.

***************************************************************************/
#pragma once

#ifndef __MEWUI_SELSOFT_H__
#define __MEWUI_SELSOFT_H__

#include "mewui/utils.h"
#include "mewui/custmenu.h"

// Menu Class
class ui_menu_select_software : public ui_menu
{
public:
	ui_menu_select_software(running_machine &machine, render_container *container, const game_driver *driver);
	virtual ~ui_menu_select_software();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	virtual bool menu_has_search_active() { return (m_search[0] != 0); }

private:
	enum { VISIBLE_GAMES_IN_SEARCH = 200 };
	char                m_search[40];
	const game_driver   *m_driver;
	bool                m_has_empty_start;
	s_filter            m_filter;

	ui_software_info                  *m_searchlist[VISIBLE_GAMES_IN_SEARCH + 1];
	std::vector<ui_software_info *>   m_displaylist;
	std::vector<ui_software_info>     m_swinfo;
	std::vector<ui_software_info *>   m_tmp;
	std::vector<ui_software_info *>   m_sortedlist;

	void build_software_list();
	void build_list(std::vector<ui_software_info *> &vec, const char *filter_text = NULL, int filter = -1);
	void build_custom();
	void find_matches(const char *str, int count);
	void load_sw_custom_filters();

	// handlers
	void inkey_select(const ui_menu_event *menu_event);
	void inkey_special(const ui_menu_event *menu_event);
};

class ui_mewui_software_parts : public ui_menu
{
public:
	ui_mewui_software_parts(running_machine &machine, render_container *container, std::vector<std::string> partname, std::vector<std::string> partdesc, ui_software_info *ui_info);
	virtual ~ui_mewui_software_parts();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:

	ui_software_info          *m_uiinfo;
	std::vector<std::string>  m_nameparts, m_descpart;
};

class ui_mewui_bios_selection : public ui_menu
{
public:
	ui_mewui_bios_selection(running_machine &machine, render_container *container, std::vector<std::string> biosname, void *driver, bool software, bool inlist);
	virtual ~ui_mewui_bios_selection();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:

	void                      *m_driver;
	bool m_software,          m_inlist;
	std::vector<std::string>  m_bios;
};

struct reselect_last
{
	static std::string driver, software, swlist;
};

// Getter
int get_bios_count(const game_driver *driver, std::vector<std::string> &biosname);


#endif /* __MEWUI_SELSOFT_H__ */
