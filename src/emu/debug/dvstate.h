// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvstate.h

    State debugger view.

***************************************************************************/

#ifndef MAME_EMU_DEBUG_DVSTATE_H
#define MAME_EMU_DEBUG_DVSTATE_H

#pragma once

#include "debugvw.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// data sources for state views
class debug_view_state_source : public debug_view_source
{
	friend class debug_view_state;

	// construction/destruction
	debug_view_state_source(const char *name, device_t &device);
private:
	// internal state
	device_state_interface *m_stateintf;        // state interface
	device_execute_interface *m_execintf;       // execution interface
};


// debug view for state
class debug_view_state : public debug_view
{
	friend resource_pool_object<debug_view_state>::~resource_pool_object();
	friend class debug_view_manager;

	// construction/destruction
	debug_view_state(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_state();

protected:
	// view overrides
	virtual void view_update() override;
	virtual void view_notify(debug_view_notification type) override;

private:
	class state_item
	{
	public:
		state_item(int index, const char *name, UINT8 valuechars);
		state_item(const state_item &) = default;
		state_item(state_item &&) = default;
		state_item &operator=(const state_item &) = default;
		state_item &operator=(state_item &&) = default;

		UINT64 value() const { return m_currval; }
		bool changed() const { return m_lastval != m_currval; }
		int index() const { return m_index; }
		UINT8 value_length() const { return m_vallen; }

		void update(UINT64 newval, bool save);

	private:
		UINT64      m_lastval;          // last value
		UINT64      m_currval;          // current value
		int         m_index;            // index
		UINT8       m_vallen;           // number of value chars

	public:
		std::string m_symbol;           // symbol
	};

	// internal helpers
	void enumerate_sources();
	void reset();
	void recompute();

	// internal state
	int                     m_divider;              // dividing column
	UINT64                  m_last_update;          // execution counter at last update
	std::vector<state_item> m_state_list;           // state data

	// constants
	static const int REG_DIVIDER    = -10;
	static const int REG_CYCLES     = -11;
	static const int REG_BEAMX      = -12;
	static const int REG_BEAMY      = -13;
	static const int REG_FRAME      = -14;
};


#endif // MAME_EMU_DEBUG_DVSTATE_H
