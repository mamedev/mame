// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldvp931.h

    Philips 22VP931 laserdisc emulation.

*************************************************************************/

#pragma once

#ifndef __LDVP931_H__
#define __LDVP931_H__

#include "laserdsc.h"
#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LASERDISC_22VP931_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PHILLIPS_22VP931, 0)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type PHILLIPS_22VP931;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> phillips_22vp931_device

// base _22vp931 class
class phillips_22vp931_device : public laserdisc_device
{
public:
	// types
	typedef delegate<void (phillips_22vp931_device &, int)> data_ready_delegate;

	// construction/destruction
	phillips_22vp931_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// input and output
	void data_w(UINT8 data) { synchronize(TID_DEFERRED_DATA, data); }
	void reset_w(UINT8 data);
	UINT8 data_r();
	UINT8 ready_r() { return m_fromcontroller_pending ? CLEAR_LINE : ASSERT_LINE; }
	UINT8 data_available_r() { return m_tocontroller_pending ? ASSERT_LINE : CLEAR_LINE; }

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
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

public:
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
	DECLARE_READ8_MEMBER( i8049_t0_r );
	DECLARE_READ8_MEMBER( i8049_t1_r );

protected:
	// internal state
	required_device<i8049_device> m_i8049_cpu;      // CPU index of the 8049
	emu_timer *         m_tracktimer;               // timer device
	data_ready_delegate m_data_ready;               // data ready callback

	// I/O port states
	UINT8               m_i8049_out0;               // output 0 state
	UINT8               m_i8049_out1;               // output 1 state
	UINT8               m_i8049_port1;              // port 1 state

	// DATIC circuit implementation
	UINT8               m_daticval;                 // latched DATIC value
	UINT8               m_daticerp;                 // /ERP value from DATIC
	UINT8               m_datastrobe;               // DATA STROBE line from DATIC

	// communication status
	UINT8               m_reset_state;              // state of the reset input
	UINT8               m_fromcontroller;           // command byte from the controller
	bool                m_fromcontroller_pending;   // true if data is pending
	UINT8               m_tocontroller;             // command byte to the controller
	bool                m_tocontroller_pending;     // true if data is pending

	// tracking
	INT8                m_trackdir;                 // direction of tracking
	UINT8               m_trackstate;               // state of tracking

	// debugging
	UINT8               m_cmdbuf[3];                // 3 bytes worth of commands
	UINT8               m_cmdcount;                 // number of command bytes seen
	INT16               m_advanced;                 // number of frames advanced
};


#endif
