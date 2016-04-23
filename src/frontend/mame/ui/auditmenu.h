// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/***************************************************************************

    ui/auditmenu.h

    Internal UI user interface.

***************************************************************************/

#pragma once

#ifndef __UI_AUDIT_H__
#define __UI_AUDIT_H__

//-------------------------------------------------
//  class audit menu
//-------------------------------------------------
using vptr_game = std::vector<const game_driver *>;

class ui_menu_audit : public ui_menu
{
public:
	ui_menu_audit(running_machine &machine, render_container *container, vptr_game &availablesorted, vptr_game &unavailablesorted, int audit_mode);
	virtual ~ui_menu_audit();
	virtual void populate() override;
	virtual void handle() override;

private:
	vptr_game &m_availablesorted;
	vptr_game &m_unavailablesorted;

	int m_audit_mode;
	void save_available_machines();
	bool m_first;
};

inline int cs_stricmp(const char *s1, const char *s2);
bool sorted_game_list(const game_driver *x, const game_driver *y);

#endif /* __UI_AUDIT_H__ */
