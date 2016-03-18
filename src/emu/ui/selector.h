// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/selector.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef __UI_SELECTOR_H__
#define __UI_SELECTOR_H__

enum
{
	SELECTOR_INIFILE = 1,
	SELECTOR_CATEGORY,
	SELECTOR_GAME,
	SELECTOR_SOFTWARE
};

//-------------------------------------------------
//  class selector menu
//-------------------------------------------------

class ui_menu_selector : public ui_menu
{
public:
	ui_menu_selector(running_machine &machine, render_container *container, std::vector<std::string> const &_sel, UINT16 &_actual, int _category = 0, int _hover = 0);
	ui_menu_selector(running_machine &machine, render_container *container, std::vector<std::string> &&_sel, UINT16 &_actual, int _category = 0, int _hover = 0);
	virtual ~ui_menu_selector();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	virtual bool menu_has_search_active() override { return (m_search[0] != 0); }

private:
	enum { VISIBLE_GAMES_IN_SEARCH = 200 };
	char                       m_search[40];
	UINT16                     &m_selector;
	int                        m_category, m_hover;
	bool                       m_first_pass;
	std::vector<std::string>   m_str_items;
	std::string                *m_searchlist[VISIBLE_GAMES_IN_SEARCH + 1];

	void find_matches(const char *str);
};

#endif /* __UI_SELECTOR_H__ */
