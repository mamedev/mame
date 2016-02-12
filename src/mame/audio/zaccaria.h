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


extern device_type const ZACCARIA_1B11142;


#define MCFG_ZACCARIA_1B11142(_tag) \
	MCFG_DEVICE_ADD(_tag, ZACCARIA_1B11142, 0)

#define MCFG_ZACCARIA_1B11142_SET_ACS_CALLBACK(_devcb) \
	devcb = &zac1b11142_audio_device::static_set_acs_cb(*device, DEVCB_##_devcb);


class zac1b11142_audio_device : public device_t, public device_mixer_interface
{
public:
	template<class _Object> static devcb_base &static_set_acs_cb(device_t &device, _Object object)
	{ return downcast<zac1b11142_audio_device &>(device).m_acs_cb.set_callback(object); }

	zac1b11142_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, UINT32 clock);
	~zac1b11142_audio_device() { }

	// host interface
	DECLARE_WRITE8_MEMBER(hs_w);
	DECLARE_READ_LINE_MEMBER(acs_r);
	DECLARE_WRITE_LINE_MEMBER(ressound_w);

	// melody section handlers
	DECLARE_READ8_MEMBER(pia_4i_porta_r);
	DECLARE_WRITE8_MEMBER(pia_4i_porta_w);
	DECLARE_WRITE8_MEMBER(pia_4i_portb_w);
	DECLARE_WRITE8_MEMBER(ay_4g_porta_w);
	DECLARE_READ8_MEMBER(ay_4g_portb_r);
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
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line    m_acs_cb;

	required_device<cpu_device>     m_melodycpu;
	required_device<pia6821_device> m_pia_4i;
	required_device<ay8910_device>  m_ay_4g;
	required_device<ay8910_device>  m_ay_4h;

	required_device<cpu_device>     m_audiocpu;
	required_device<pia6821_device> m_pia_1i;
	required_device<tms5220_device> m_speech;

	required_ioport m_inputs;

	UINT8   m_host_command;
	UINT8   m_melody_command;
};

#endif // __AUDIO_ZACCARIA_H__