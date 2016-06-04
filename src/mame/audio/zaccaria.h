// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#pragma once

#ifndef __AUDIO_ZACCARIA_H__
#define __AUDIO_ZACCARIA_H__

#include "emu.h"
#include "machine/6821pia.h"
#include "machine/netlist.h"
#include "sound/ay8910.h"
#include "sound/tms5220.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern device_type const ZACCARIA_1B11107;
extern device_type const ZACCARIA_1B11142;



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ZACCARIA_1B11107(_tag) \
	MCFG_DEVICE_ADD(_tag, ZACCARIA_1B11107, 0)

#define MCFG_ZACCARIA_1B11142(_tag) \
	MCFG_DEVICE_ADD(_tag, ZACCARIA_1B11142, 0)

#define MCFG_ZACCARIA_1B11142_SET_ACS_CALLBACK(_devcb) \
	devcb = &zac1b11142_audio_device::static_set_acs_cb(*device, DEVCB_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class zac1b111xx_melody_base : public device_t, public device_mixer_interface
{
public:
	zac1b111xx_melody_base(
			machine_config const &mconfig,
			device_type devtype,
			char const *name,
			char const *tag,
			device_t *owner,
			UINT32 clock,
			char const *shortname,
			char const *source);

	DECLARE_READ8_MEMBER(melodypia_porta_r);
	DECLARE_WRITE8_MEMBER(melodypia_porta_w);
	DECLARE_WRITE8_MEMBER(melodypia_portb_w);
	DECLARE_READ8_MEMBER(melodypsg1_portb_r);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<cpu_device>     m_melodycpu;
	required_device<pia6821_device> m_melodypia;
	required_device<ay8910_device>  m_melodypsg1;
	required_device<ay8910_device>  m_melodypsg2;

	UINT8   m_melody_command;
};


class zac1b11107_audio_device : public zac1b111xx_melody_base
{
public:
	zac1b11107_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, UINT32 clock);

	// host interface
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE_LINE_MEMBER(reset_w);

	// PSG output handlers
	DECLARE_WRITE8_MEMBER(melodypsg1_porta_w);
	DECLARE_WRITE8_MEMBER(melodypsg2_porta_w);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
};


class zac1b11142_audio_device : public zac1b111xx_melody_base
{
public:
	template<class _Object> static devcb_base &static_set_acs_cb(device_t &device, _Object object)
	{ return downcast<zac1b11142_audio_device &>(device).m_acs_cb.set_callback(object); }

	zac1b11142_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, UINT32 clock);

	// host interface
	DECLARE_WRITE8_MEMBER(hs_w);
	DECLARE_READ_LINE_MEMBER(acs_r);
	DECLARE_WRITE_LINE_MEMBER(ressound_w);

	// melody section handlers
	DECLARE_WRITE8_MEMBER(ay_4g_porta_w);
	DECLARE_WRITE8_MEMBER(ay_4h_porta_w);
	DECLARE_WRITE8_MEMBER(ay_4h_portb_w);

	// master audio section handlers
	DECLARE_READ8_MEMBER(host_command_r);
	DECLARE_WRITE8_MEMBER(melody_command_w);
	DECLARE_WRITE8_MEMBER(pia_1i_portb_w);

	// input ports don't push
	INTERRUPT_GEN_MEMBER(input_poll);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line    m_acs_cb;

	required_device<cpu_device>     m_audiocpu;
	required_device<pia6821_device> m_pia_1i;
	required_device<tms5220_device> m_speech;

	required_ioport m_inputs;

	UINT8   m_host_command;
};

#endif // __AUDIO_ZACCARIA_H__
