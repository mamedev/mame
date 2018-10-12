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
	void watchdog_enable(bool enable = true);
	int32_t get_vblank_counter() const { return m_counter; }

	// read/write handlers
	DECLARE_WRITE8_MEMBER( reset_w );
	DECLARE_READ8_MEMBER( reset_r );
	DECLARE_WRITE16_MEMBER( reset16_w );
	DECLARE_READ16_MEMBER( reset16_r );
	DECLARE_WRITE32_MEMBER( reset32_w );
	DECLARE_READ32_MEMBER( reset32_r );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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
	int32_t                 m_counter;      // counter for VBLANK tracking
	emu_timer *             m_timer;        // timer for triggering reset
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(WATCHDOG_TIMER, watchdog_timer_device)


#endif // MAME_MACHINE_WATCHDOG_H
