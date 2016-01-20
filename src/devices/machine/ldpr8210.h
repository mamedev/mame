// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldpr8210.h

    Pioneer PR-8210 laserdisc emulation.

*************************************************************************/

#pragma once

#ifndef __LDPR8210_H__
#define __LDPR8210_H__

#include "laserdsc.h"
#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LASERDISC_PR8210_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, PIONEER_PR8210, 0)
#define MCFG_LASERDISC_SIMUTREK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SIMUTREK_SPECIAL, 0)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
extern const device_type PIONEER_PR8210;
extern const device_type SIMUTREK_SPECIAL;



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// pioneer PIA subclass
class pioneer_pia
{
public:
	UINT8               frame[7];               // (20-26) 7 characters for the chapter/frame
	UINT8               text[17];               // (20-30) 17 characters for the display
	UINT8               control;                // (40) control lines
	UINT8               latchdisplay;           //   flag: set if the display was latched
	UINT8               portb;                  // (60) port B value (LEDs)
	UINT8               display;                // (80) display enable
	UINT8               porta;                  // (A0) port A value (from serial decoder)
	UINT8               vbi1;                   // (C0) VBI decoding state 1
	UINT8               vbi2;                   // (E0) VBI decoding state 2
};


// ======================> pioneer_pr8210_device

// base pr8210 class
class pioneer_pr8210_device : public laserdisc_device
{
public:
	// construction/destruction
	pioneer_pr8210_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	pioneer_pr8210_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// input and output
	void control_w(UINT8 data);

protected:
	// timer IDs
	enum
	{
		TID_VSYNC_OFF = TID_FIRST_PLAYER_TIMER,
		TID_VBI_DATA_FETCH,
		TID_FIRST_SUBCLASS_TIMER
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
	virtual void player_overlay(bitmap_yuy16 &bitmap) override;

	// internal helpers
	bool focus_on() const { return !(m_i8049_port1 & 0x08); }
	bool spdl_on() const { return !(m_i8049_port1 & 0x10); }
	bool laser_on() const { return !(m_i8049_port2 & 0x01); }
	virtual bool override_control() const { return false; }
	void update_video_squelch() { set_video_squelch((m_i8049_port1 & 0x20) != 0); }
	virtual void update_audio_squelch() { set_audio_squelch((m_i8049_port1 & 0x40) || !(m_pia.portb & 0x01), (m_i8049_port1 & 0x40) || !(m_pia.portb & 0x02)); }

public:
	// internal read/write handlers
	DECLARE_READ8_MEMBER( i8049_pia_r );
	DECLARE_WRITE8_MEMBER( i8049_pia_w );
	DECLARE_READ8_MEMBER( i8049_bus_r );
	DECLARE_WRITE8_MEMBER( i8049_port1_w );
	DECLARE_WRITE8_MEMBER( i8049_port2_w );
	DECLARE_READ8_MEMBER( i8049_t0_r );
	DECLARE_READ8_MEMBER( i8049_t1_r );

protected:
	// internal overlay helpers
	void overlay_draw_group(bitmap_yuy16 &bitmap, const UINT8 *text, int count, float xstart);
	void overlay_erase(bitmap_yuy16 &bitmap, float xstart, float xend);
	void overlay_draw_char(bitmap_yuy16 &bitmap, UINT8 ch, float xstart);

	// internal state
	UINT8               m_control;              // control line state
	UINT8               m_lastcommand;          // last command seen
	UINT16              m_accumulator;          // bit accumulator
	attotime            m_lastcommandtime;      // time of the last command
	attotime            m_lastbittime;          // time of last bit received
	attotime            m_firstbittime;         // time of first bit in command

	// low-level emulation data
	required_device<i8049_device> m_i8049_cpu;  // 8049 CPU device
	attotime            m_slowtrg;              // time of the last SLOW TRG
	pioneer_pia         m_pia;                  // PIA state
	bool                m_vsync;                // live VSYNC state
	UINT8               m_i8049_port1;          // 8049 port 1 state
	UINT8               m_i8049_port2;          // 8049 port 2 state
};


// ======================> simutrek_special_device

class simutrek_special_device : public pioneer_pr8210_device
{
public:
	// construction/destruction
	simutrek_special_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// input and output
	void data_w(UINT8 data);
	UINT8 ready_r() const { return !m_data_ready; }
	UINT8 status_r() const { return ((m_i8748_port2 & 0x03) == 0x03) ? ASSERT_LINE : CLEAR_LINE; }

	// external controls
	void set_external_audio_squelch(int state);

protected:
	// timer IDs
	enum
	{
		TID_IRQ_OFF = TID_FIRST_SUBCLASS_TIMER,
		TID_LATCH_DATA
	};

	// subclass overrides
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// internal helpers
	virtual bool override_control() const override { return m_controlthis; }
	virtual void update_audio_squelch() override { set_audio_squelch(m_audio_squelch, m_audio_squelch); }

public:
	// internal read/write handlers
	DECLARE_READ8_MEMBER( i8748_port2_r );
	DECLARE_WRITE8_MEMBER( i8748_port2_w );
	DECLARE_READ8_MEMBER( i8748_data_r );
	DECLARE_READ8_MEMBER( i8748_t0_r );

protected:
	// internal state
	required_device<i8748_device> m_i8748_cpu;
	UINT8               m_audio_squelch;            // audio squelch value
	UINT8               m_data;                 // parallel data for simutrek
	bool                m_data_ready;               // ready flag for simutrek data
	UINT8               m_i8748_port2;                  // 8748 port 2 state
	UINT8               m_controlnext;          // latch to control next pair of fields
	UINT8               m_controlthis;          // latched value for our control over the current pair of fields
};


#endif
