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

public:
	// construction/destruction
	debug_view_state_source(std::string &&name, device_t &device);

private:
	// internal state
	device_state_interface *m_stateintf;        // state interface
	device_execute_interface *m_execintf;       // execution interface
};


// debug view for state
class debug_view_state : public debug_view
{
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
		state_item(int index, const char *name, u8 valuechars);
		state_item(const state_item &) = default;
		state_item(state_item &&) = default;
		state_item &operator=(const state_item &) = default;
		state_item &operator=(state_item &&) = default;

		u64 value() const { return m_currval; }
		bool changed() const { return m_lastval != m_currval; }
		int index() const { return m_index; }
		u8 value_length() const { return m_vallen; }

		void update(u64 newval, bool save);

	private:
		u64         m_lastval;          // last value
		u64         m_currval;          // current value
		int         m_index;            // index
		u8          m_vallen;           // number of value chars

	public:
		std::string m_symbol;           // symbol
	};

	// internal helpers
	void enumerate_sources();
	void reset();
	void recompute();

	// internal state
	int                     m_divider;              // dividing column
	u64                     m_last_update;          // execution counter at last update
	std::vector<state_item> m_state_list;           // state data

	// constants
	static constexpr int REG_DIVIDER    = -10;
	static constexpr int REG_CYCLES     = -11;
	static constexpr int REG_BEAMX      = -12;
	static constexpr int REG_BEAMX_S0   = -12;
	static constexpr int REG_BEAMX_S1   = -13;
	static constexpr int REG_BEAMX_S2   = -14;
	static constexpr int REG_BEAMX_S3   = -15;
	static constexpr int REG_BEAMX_S4   = -16;
	static constexpr int REG_BEAMX_S5   = -17;
	static constexpr int REG_BEAMX_S6   = -18;
	static constexpr int REG_BEAMX_S7   = -19;
	static constexpr int REG_BEAMY      = -20;
	static constexpr int REG_BEAMY_S0   = -20;
	static constexpr int REG_BEAMY_S1   = -21;
	static constexpr int REG_BEAMY_S2   = -22;
	static constexpr int REG_BEAMY_S3   = -23;
	static constexpr int REG_BEAMY_S4   = -24;
	static constexpr int REG_BEAMY_S5   = -25;
	static constexpr int REG_BEAMY_S6   = -26;
	static constexpr int REG_BEAMY_S7   = -27;
	static constexpr int REG_FRAME      = -28;
	static constexpr int REG_FRAME_S0   = -28;
	static constexpr int REG_FRAME_S1   = -29;
	static constexpr int REG_FRAME_S2   = -30;
	static constexpr int REG_FRAME_S3   = -31;
	static constexpr int REG_FRAME_S4   = -32;
	static constexpr int REG_FRAME_S5   = -33;
	static constexpr int REG_FRAME_S6   = -34;
	static constexpr int REG_FRAME_S7   = -35;
};


#endif // MAME_EMU_DEBUG_DVSTATE_H
