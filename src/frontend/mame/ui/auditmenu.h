// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/auditmenu.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_AUDITMENU_H
#define MAME_FRONTEND_UI_AUDITMENU_H

#include "ui/menu.h"

namespace ui {
//-------------------------------------------------
//  class audit menu
//-------------------------------------------------
using vptr_game = std::vector<const game_driver *>;

class menu_audit : public menu
{
public:
	menu_audit(mame_ui_manager &mui, render_container *container, vptr_game &availablesorted, vptr_game &unavailablesorted, int audit_mode);
	virtual ~menu_audit() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	vptr_game &m_availablesorted;
	vptr_game &m_unavailablesorted;

	int m_audit_mode;
	void save_available_machines();
	bool m_first;
};

bool sorted_game_list(const game_driver *x, const game_driver *y);

} // namespace ui

#endif /* MAME_FRONTEND_UI_AUDITMENU_H */
