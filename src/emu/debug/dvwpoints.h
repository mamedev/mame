// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvwpoints.h

    Watchpoint debugger view.

***************************************************************************/

#ifndef __DVWPOINTS_H__
#define __DVWPOINTS_H__

#include "debugvw.h"
#include "debugcpu.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// debug view for watchpoints
class debug_view_watchpoints : public debug_view
{
	friend resource_pool_object<debug_view_watchpoints>::~resource_pool_object();
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
	int (*m_sortType)(void const *, void const *);
	std::vector<device_debug::watchpoint *> m_buffer;
};


#endif
