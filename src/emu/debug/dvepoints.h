// license:BSD-3-Clause
// copyright-holders:Andrew Gardner, Vas Crabb
/*********************************************************************

    dvepoints.h

    Exceptionpoint debugger view.

***************************************************************************/
#ifndef MAME_EMU_DEBUG_DVEPOINTS_H
#define MAME_EMU_DEBUG_DVEPOINTS_H

#pragma once

#include "debugvw.h"

#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// debug view for exceptionpoints
class debug_view_exceptionpoints : public debug_view
{
	friend class debug_view_manager;

	// construction/destruction
	debug_view_exceptionpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_exceptionpoints();

protected:
	// view overrides
	virtual void view_update() override;
	virtual void view_click(const int button, const debug_view_xy& pos) override;

private:
	// internal helpers
	void enumerate_sources();
	void pad_ostream_to_length(std::ostream& str, int len);
	void gather_exceptionpoints();

	// internal state
	bool (*m_sortType)(const debug_exceptionpoint *, const debug_exceptionpoint *);
	std::vector<const debug_exceptionpoint *> m_buffer;
};

#endif // MAME_EMU_DEBUG_DVEPOINTS_H
