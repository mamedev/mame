// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/selector.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_SELECTOR_H
#define MAME_FRONTEND_UI_SELECTOR_H

#include "ui/menu.h"

namespace ui {

//-------------------------------------------------
//  class selector menu
//-------------------------------------------------

class menu_selector : public menu
{
public:
	enum
	{
		INIFILE = 1,
		CATEGORY,
		GAME,
		SOFTWARE
	};

	menu_selector(mame_ui_manager &mui, render_container *container, std::vector<std::string> const &_sel, UINT16 &_actual, int _category = 0, int _hover = 0);
	menu_selector(mame_ui_manager &mui, render_container *container, std::vector<std::string> &&_sel, UINT16 &_actual, int _category = 0, int _hover = 0);
	virtual ~menu_selector() override;
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

} // namespace ui

#endif /* MAME_FRONTEND_UI_SELECTOR_H */
