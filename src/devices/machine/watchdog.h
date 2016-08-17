// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#pragma once

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_WATCHDOG_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, WATCHDOG_TIMER, 0)
#define MCFG_WATCHDOG_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)
#define MCFG_WATCHDOG_VBLANK_INIT(_screen, _count) \
	watchdog_timer_device::static_set_vblank_count(*device, _screen, _count);
#define MCFG_WATCHDOG_TIME_INIT(_time) \
	watchdog_timer_device::static_set_time(*device, _time);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> watchdog_timer_device

class watchdog_timer_device : public device_t
{
public:
	// construction/destruction
	watchdog_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// inline configuration helpers
	static void static_set_vblank_count(device_t &device, const char *screen_tag, INT32 count);
	static void static_set_time(device_t &device, attotime time);

	// watchdog control
	void watchdog_reset();
	void watchdog_enable(bool enable = true);
	INT32 get_vblank_counter() const { return m_counter; }

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
	INT32                   m_vblank_count; // number of VBLANKs until resetting the machine
	attotime                m_time;         // length of time until resetting the machine
	const char *            m_screen_tag;   // the tag of the screen this timer tracks

	// internal state
	bool                    m_enabled;      // is the watchdog enabled?
	INT32                   m_counter;      // counter for VBLANK tracking
	emu_timer *             m_timer;        // timer for triggering reset
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const device_type WATCHDOG_TIMER;


#endif
