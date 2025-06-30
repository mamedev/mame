// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Roland MB63H114 Multiple Address Counter

***************************************************************************/

#ifndef MAME_ROLAND_MB63H114_H
#define MAME_ROLAND_MB63H114_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb63h114_device

class mb63h114_device : public device_t
{
public:
	// device type constructor
	mb63h114_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// write16 callback.
	// - offset: Output pins DCBA. Pins CBA hold the current counter.
	// - data: The current counter's 13-bit value.
	auto counter_cb() { return m_counter_func.bind(); }

	// CPU write handler
	void xst_w(u8 data);

	// Clock inputs for the 8 counters.
	void xck_w(u8 data);

protected:
	// device-specific overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static inline constexpr int COUNTERS = 8;
	static inline constexpr int MAX_COUNT = 0x1fff;

	TIMER_CALLBACK_MEMBER(timer_tick);

	devcb_write16 m_counter_func;
	emu_timer *m_timer;

	u8 m_xst;  // Last XST input.
	u8 m_xck;  // Last XCK input.
	u8 m_active_counter;  // Outputs C, B, A.
	u8 m_d;  // Output D.
	std::array<u16, COUNTERS> m_counters;
};


// device type declaration
DECLARE_DEVICE_TYPE(MB63H114, mb63h114_device)

#endif // MAME_ROLAND_MB63H114_H
