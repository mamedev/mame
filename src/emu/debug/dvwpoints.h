// license:BSD-3-Clause
// copyright-holders:Andrew Gardner, Vas Crabb
/*********************************************************************

    dvwpoints.h

    Watchpoint debugger view.

***************************************************************************/
#ifndef MAME_EMU_DEBUG_DVWPOINTS_H
#define MAME_EMU_DEBUG_DVWPOINTS_H

#pragma once

#include "debugvw.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// debug view for watchpoints
class debug_view_watchpoints : public debug_view
{
	friend class debug_view_manager;

	// construction/destruction
	debug_view_watchpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_watchpoints();

protected:
	// view overrides
	virtual void view_update() override;
	virtual void view_click(const int button, const debug_view_xy& pos) override;

private:
	// internal helpers
	void enumerate_sources();
	void pad_ostream_to_length(std::ostream& str, int len);
	void gather_watchpoints();


	// internal state
	bool (*m_sortType)(const debug_watchpoint *, const debug_watchpoint *);
	std::vector<debug_watchpoint *> m_buffer;
};

#endif // MAME_EMU_DEBUG_DVWPOINTS_H
