// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldvp931.h

    Philips 22VP931 laserdisc emulation.

*************************************************************************/

#ifndef MAME_MACHINE_LDVP931_H
#define MAME_MACHINE_LDVP931_H

#pragma once

#include "laserdsc.h"
#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(PHILIPS_22VP931, philips_22vp931_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> philips_22vp931_device

// base _22vp931 class
class philips_22vp931_device : public laserdisc_device
{
public:
	// types
	typedef delegate<void (philips_22vp931_device &, int)> data_ready_delegate;

	// construction/destruction
	philips_22vp931_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// input and output
	void data_w(uint8_t data) { machine().scheduler().synchronize(timer_expired_delegate(FUNC(philips_22vp931_device::process_deferred_data), this), data); }
	void reset_w(uint8_t data);
	uint8_t data_r();
	uint8_t ready_r() { return m_fromcontroller_pending ? CLEAR_LINE : ASSERT_LINE; }
	uint8_t data_available_r() { return m_tocontroller_pending ? ASSERT_LINE : CLEAR_LINE; }

	// configuration
	void set_data_ready_callback(data_ready_delegate callback) { m_data_ready = callback; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual int32_t player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

	TIMER_CALLBACK_MEMBER(process_vbi_data);
	TIMER_CALLBACK_MEMBER(process_deferred_data);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_CALLBACK_MEMBER(data_strobe_off);
	TIMER_CALLBACK_MEMBER(erp_off);
	TIMER_CALLBACK_MEMBER(half_track_tick);

private:
	// internal read/write handlers
	void i8049_output0_w(uint8_t data);
	void i8049_output1_w(uint8_t data);
	void i8049_lcd_w(uint8_t data);
	uint8_t i8049_unknown_r();
	uint8_t i8049_keypad_r();
	uint8_t i8049_datic_r();
	uint8_t i8049_from_controller_r();
	void i8049_to_controller_w(uint8_t data);
	uint8_t i8049_port1_r();
	void i8049_port1_w(uint8_t data);
	uint8_t i8049_port2_r();
	void i8049_port2_w(uint8_t data);
	int i8049_t0_r();
	int i8049_t1_r();

	void vp931_portmap(address_map &map) ATTR_COLD;

	// internal state
	required_device<i8049_device> m_i8049_cpu;        // CPU index of the 8049
	emu_timer *         m_initial_vbi_timer;
	emu_timer *         m_process_vbi_timer;
	emu_timer *         m_irq_off_timer;
	emu_timer *         m_strobe_off_timer;
	emu_timer *         m_erp_off_timer;
	emu_timer *         m_track_timer;                // half-track timer device
	data_ready_delegate m_data_ready;                 // data ready callback

	// I/O port states
	uint8_t               m_i8049_out0;               // output 0 state
	uint8_t               m_i8049_out1;               // output 1 state
	uint8_t               m_i8049_port1;              // port 1 state

	// DATIC circuit implementation
	uint8_t               m_daticval;                 // latched DATIC value
	uint8_t               m_daticerp;                 // /ERP value from DATIC
	uint8_t               m_datastrobe;               // DATA STROBE line from DATIC

	// communication status
	//uint8_t               m_reset_state;            // state of the reset input
	uint8_t               m_fromcontroller;           // command byte from the controller
	bool                  m_fromcontroller_pending;   // true if data is pending
	uint8_t               m_tocontroller;             // command byte to the controller
	bool                  m_tocontroller_pending;     // true if data is pending

	// tracking
	int8_t                m_trackdir;                 // direction of tracking
	uint8_t               m_trackstate;               // state of tracking

	// debugging
	uint8_t               m_cmdbuf[3];                // 3 bytes worth of commands
	uint8_t               m_cmdcount;                 // number of command bytes seen
	int16_t               m_advanced;                 // number of frames advanced
};

#endif // MAME_MACHINE_LDVP931_H
