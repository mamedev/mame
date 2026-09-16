// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_MACHINE_WATCHDOG_H
#define MAME_MACHINE_WATCHDOG_H

#pragma once

#include <screen.h>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> watchdog_timer_device

class watchdog_timer_device : public device_t
{
public:
	// construction/destruction
	watchdog_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// inline configuration helpers
	template <typename T> void set_vblank_count(T &&screen_tag, int32_t count) { m_screen.set_tag(std::forward<T>(screen_tag)); m_vblank_count = count; }
	void set_time(attotime time) { m_time = time; }

	// watchdog control
	void watchdog_reset();
	void watchdog_enable(int state = 1);
	int32_t get_vblank_counter() const { return m_counter; }

	// watchdog reset read/write handlers (strobe on R/W pin)
	void reset_w(u8 data = 0);
	u8 reset_r(address_space &space);
	void reset16_w(u16 data = 0);
	u16 reset16_r(address_space &space);
	void reset32_w(u32 data = 0);
	u32 reset32_r(address_space &space);

	// watchdog reset writeline strobe
	void reset_line_w(int state);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(watchdog_expired);

private:
	// internal helpers
	void watchdog_fired();
	void watchdog_vblank(screen_device &screen, bool vblank_state);

	// configuration data
	int32_t                 m_vblank_count; // number of VBLANKs until resetting the machine
	attotime                m_time;         // length of time until resetting the machine
	optional_device<screen_device> m_screen; // the tag of the screen this timer tracks

	// internal state
	bool                    m_enabled;      // is the watchdog enabled?
	int                     m_reset_line;   // watchdog reset writeline state
	int32_t                 m_counter;      // counter for VBLANK tracking
	emu_timer *             m_timer;        // timer for triggering reset
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(WATCHDOG_TIMER, watchdog_timer_device)


#endif // MAME_MACHINE_WATCHDOG_H
