// license:BSD-3-Clause
// copyright-holders:Mike Harris
/***************************************************************************

    bally.h

    Functions to emulate the various Bally pinball sound boards.

***************************************************************************/
#ifndef MAME_AUDIO_BALLY_H
#define MAME_AUDIO_BALLY_H

#pragma once


#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(BALLY_AS3022,      bally_as3022_device)
DECLARE_DEVICE_TYPE(BALLY_SOUNDS_PLUS, bally_sounds_plus_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bally_as3022_device

class bally_as3022_device : public device_t, public device_mixer_interface
{
public:
	bally_as3022_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			uint32_t clock = 3'579'545) :
		bally_as3022_device(mconfig, BALLY_AS3022, tag, owner, clock)
	{ }

	// read/write
	DECLARE_INPUT_CHANGED_MEMBER(sw1);
	DECLARE_WRITE8_MEMBER(sound_select);
	DECLARE_WRITE_LINE_MEMBER(sound_int);

	void as3022_map(address_map &map);

protected:
	bally_as3022_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock) :
	        device_t(mconfig, type, tag, owner, clock),
                device_mixer_interface(mconfig, *this),
                m_cpu(*this, "cpu"),
                m_pia(*this, "pia"),
                m_ay(*this, "ay"),
		m_mc3417(*this, "mc3417")
	{ }

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;

	// devices
	// The schematics list an optional 555, but it never seemed to be used
	required_device<m6808_cpu_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<ay8910_device> m_ay;
	optional_device<mc3417_device> m_mc3417;

	// overwridden by children
	DECLARE_WRITE8_MEMBER(pia_portb_w);

private:
	bool m_bc1;
	bool m_bdir;
	uint8_t m_sound_select;
	uint8_t m_ay_data;

	// internal communications
	TIMER_CALLBACK_MEMBER(sound_select_sync);
	TIMER_CALLBACK_MEMBER(sound_int_sync);
	DECLARE_READ8_MEMBER(pia_porta_r);
	DECLARE_WRITE8_MEMBER(pia_porta_w);
	DECLARE_WRITE_LINE_MEMBER(pia_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_READ8_MEMBER(ay_io_r);

	void update_sound_selects();
};


// ======================> bally_sounds_plus_device
class bally_sounds_plus_device : public bally_as3022_device
{
public:
	bally_sounds_plus_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			uint32_t clock = 3'579'545) :
		bally_as3022_device(mconfig, BALLY_SOUNDS_PLUS, tag, owner, clock)
	{ }

	void sounds_plus_map(address_map &map);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	// internal communications
	DECLARE_WRITE8_MEMBER(vocalizer_pia_portb_w);
};

#endif // MAME_AUDIO_BALLY_H
