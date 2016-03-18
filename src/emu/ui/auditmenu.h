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

class ui_menu_audit : public ui_menu
{
public:
	ui_menu_audit(running_machine &machine, render_container *container, std::vector<const game_driver *> &availablesorted, std::vector<const game_driver *> &unavailablesorted, int audit_mode);
	virtual ~ui_menu_audit();
	virtual void populate() override;
	virtual void handle() override;

private:
	std::vector<const game_driver *> &m_availablesorted;
	std::vector<const game_driver *> &m_unavailablesorted;

	int m_audit_mode;
	void save_available_machines();
	bool m_first;
};

#endif /* __UI_AUDIT_H__ */
