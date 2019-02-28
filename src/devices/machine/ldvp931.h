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
	void data_w(uint8_t data) { synchronize(TID_DEFERRED_DATA, data); }
	void reset_w(uint8_t data);
	uint8_t data_r();
	uint8_t ready_r() { return m_fromcontroller_pending ? CLEAR_LINE : ASSERT_LINE; }
	uint8_t data_available_r() { return m_tocontroller_pending ? ASSERT_LINE : CLEAR_LINE; }

	// configuration
	void set_data_ready_callback(data_ready_delegate callback) { m_data_ready = callback; }

protected:
	// timer IDs
	enum
	{
		TID_IRQ_OFF = TID_FIRST_PLAYER_TIMER,
		TID_DATA_STROBE_OFF,
		TID_ERP_OFF,
		TID_HALF_TRACK,
		TID_VBI_DATA_FETCH,
		TID_DEFERRED_DATA
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual int32_t player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

private:
	// internal read/write handlers
	DECLARE_WRITE8_MEMBER( i8049_output0_w );
	DECLARE_WRITE8_MEMBER( i8049_output1_w );
	DECLARE_WRITE8_MEMBER( i8049_lcd_w );
	DECLARE_READ8_MEMBER( i8049_unknown_r );
	DECLARE_READ8_MEMBER( i8049_keypad_r );
	DECLARE_READ8_MEMBER( i8049_datic_r );
	DECLARE_READ8_MEMBER( i8049_from_controller_r );
	DECLARE_WRITE8_MEMBER( i8049_to_controller_w );
	DECLARE_READ8_MEMBER( i8049_port1_r );
	DECLARE_WRITE8_MEMBER( i8049_port1_w );
	DECLARE_READ8_MEMBER( i8049_port2_r );
	DECLARE_WRITE8_MEMBER( i8049_port2_w );
	DECLARE_READ_LINE_MEMBER( i8049_t0_r );
	DECLARE_READ_LINE_MEMBER( i8049_t1_r );

	void vp931_portmap(address_map &map);

	// internal state
	required_device<i8049_device> m_i8049_cpu;      // CPU index of the 8049
	emu_timer *         m_tracktimer;               // timer device
	data_ready_delegate m_data_ready;               // data ready callback

	// I/O port states
	uint8_t               m_i8049_out0;               // output 0 state
	uint8_t               m_i8049_out1;               // output 1 state
	uint8_t               m_i8049_port1;              // port 1 state

	// DATIC circuit implementation
	uint8_t               m_daticval;                 // latched DATIC value
	uint8_t               m_daticerp;                 // /ERP value from DATIC
	uint8_t               m_datastrobe;               // DATA STROBE line from DATIC

	// communication status
	//uint8_t               m_reset_state;              // state of the reset input
	uint8_t               m_fromcontroller;           // command byte from the controller
	bool                m_fromcontroller_pending;   // true if data is pending
	uint8_t               m_tocontroller;             // command byte to the controller
	bool                m_tocontroller_pending;     // true if data is pending

	// tracking
	int8_t                m_trackdir;                 // direction of tracking
	uint8_t               m_trackstate;               // state of tracking

	// debugging
	uint8_t               m_cmdbuf[3];                // 3 bytes worth of commands
	uint8_t               m_cmdcount;                 // number of command bytes seen
	int16_t               m_advanced;                 // number of frames advanced
};

#endif // MAME_MACHINE_LDVP931_H
