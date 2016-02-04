// license:BSD-3-Clause
// copyright-holders:Dankan1890
/***************************************************************************

    mewui/datmenu.h

    Internal MEWUI user interface.


***************************************************************************/

#pragma once

#ifndef __MEWUI_DATMENU_H__
#define __MEWUI_DATMENU_H__

struct ui_software_info;

//-------------------------------------------------
//  class dats menu
//-------------------------------------------------

class ui_menu_dats : public ui_menu
{
public:
	ui_menu_dats(running_machine &machine, render_container *container, int _flags, const game_driver *driver = nullptr);
	virtual ~ui_menu_dats();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	const game_driver  *m_driver;
	int                m_flags;

	bool get_data(const game_driver *driver, int flags);
};

//-------------------------------------------------
//  class command data menu
//-------------------------------------------------

class ui_menu_command : public ui_menu
{
public:
	ui_menu_command(running_machine &machine, render_container *container, const game_driver *driver = nullptr);
	virtual ~ui_menu_command();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	const game_driver *m_driver;
};

//-------------------------------------------------
//  class command content data menu
//-------------------------------------------------

class ui_menu_command_content : public ui_menu
{
public:
	ui_menu_command_content(running_machine &machine, render_container *container, std::string title, const game_driver *driver = nullptr);
	virtual ~ui_menu_command_content();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	const game_driver   *m_driver;
	std::string          m_title;
};

//-------------------------------------------------
//  class software history menu
//-------------------------------------------------

class ui_menu_history_sw : public ui_menu
{
public:
	ui_menu_history_sw(running_machine &machine, render_container *container, ui_software_info *swinfo, const game_driver *driver = nullptr);
	ui_menu_history_sw(running_machine &machine, render_container *container, const game_driver *driver = nullptr);
	virtual ~ui_menu_history_sw();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	std::string m_list, m_short, m_long, m_parent;
	const game_driver *m_driver;
};

#endif  /* __MEWUI_DATMENU_H__ */
