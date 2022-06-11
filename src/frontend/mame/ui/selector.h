// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/selector.h

    Internal UI user interface.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_SELECTOR_H
#define MAME_FRONTEND_UI_SELECTOR_H

#pragma once

#include "ui/menu.h"

#include <functional>
#include <string>
#include <vector>


namespace ui {

//-------------------------------------------------
//  class selector menu
//-------------------------------------------------

class menu_selector : public menu
{
public:
	menu_selector(
			mame_ui_manager &mui,
			render_container &container,
			std::string &&title,
			std::vector<std::string> &&sel,
			int initial,
			std::function<void (int)> &&handler);
	virtual ~menu_selector() override;

protected:
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual bool custom_ui_cancel() override { return !m_search.empty(); }

private:
	enum { VISIBLE_SEARCH_ITEMS = 200 };

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle(event const *ev) override;

	void find_matches(const char *str);

	std::string const              m_title;
	std::string                    m_search;
	std::vector<std::string>       m_str_items;
	std::function<void (int)>      m_handler;
	std::vector<std::u32string>    m_ucs_items;
	int                            m_initial;
	std::string                    *m_searchlist[VISIBLE_SEARCH_ITEMS + 1];
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SELECTOR_H
