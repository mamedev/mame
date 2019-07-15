// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_AUDIO_ZACCARIA_H
#define MAME_AUDIO_ZACCARIA_H

#pragma once

#include "machine/6821pia.h"
#include "machine/netlist.h"
#include "sound/ay8910.h"
#include "sound/tms5220.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(ZACCARIA_1B11107, zac1b11107_audio_device)
DECLARE_DEVICE_TYPE(ZACCARIA_1B11142, zac1b11142_audio_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class zac1b111xx_melody_base : public device_t, public device_mixer_interface
{
protected:
	zac1b111xx_melody_base(
			machine_config const &mconfig,
			device_type devtype,
			char const *tag,
			device_t *owner,
			u32 clock);

	DECLARE_READ8_MEMBER(melodypia_porta_r);
	DECLARE_WRITE8_MEMBER(melodypia_porta_w);
	DECLARE_WRITE8_MEMBER(melodypia_portb_w);
	DECLARE_READ8_MEMBER(melodypsg1_portb_r);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<cpu_device>     m_melodycpu;
	required_device<pia6821_device> m_melodypia;
	required_device<ay8910_device>  m_melodypsg1;
	required_device<ay8910_device>  m_melodypsg2;

	u8  m_melody_command;
	void zac1b111xx_melody_base_map(address_map &map);
};


class zac1b11107_audio_device : public zac1b111xx_melody_base
{
public:
	zac1b11107_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// host interface
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE_LINE_MEMBER(reset_w);

	void zac1b11107_melody_map(address_map &map);
protected:
	// PSG output handlers
	DECLARE_WRITE8_MEMBER(melodypsg1_porta_w);
	DECLARE_WRITE8_MEMBER(melodypsg2_porta_w);

	virtual void device_add_mconfig(machine_config &config) override;
};


class zac1b11142_audio_device : public zac1b111xx_melody_base
{
public:
	auto acs_cb() { return m_acs_cb.bind(); }

	zac1b11142_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// host interface
	DECLARE_WRITE8_MEMBER(hs_w);
	DECLARE_READ_LINE_MEMBER(acs_r);
	DECLARE_WRITE_LINE_MEMBER(ressound_w);

	// master audio section handlers
	DECLARE_READ8_MEMBER(host_command_r);
	DECLARE_WRITE8_MEMBER(melody_command_w);
	DECLARE_INPUT_CHANGED_MEMBER(p1_changed);

	void zac1b11142_audio_map(address_map &map);
	void zac1b11142_melody_map(address_map &map);
protected:
	// melody section handlers
	DECLARE_WRITE8_MEMBER(ay_4g_porta_w);
	DECLARE_WRITE8_MEMBER(ay_4h_porta_w);
	DECLARE_WRITE8_MEMBER(ay_4h_portb_w);

	// master audio section handlers
	DECLARE_WRITE8_MEMBER(pia_1i_portb_w);

	// input ports don't push
	INTERRUPT_GEN_MEMBER(input_poll);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line                m_acs_cb;

	required_device<cpu_device>     m_audiocpu;
	required_device<pia6821_device> m_pia_1i;
	required_device<tms5220_device> m_speech;

	required_device_array<netlist_mame_logic_input_device, 5>   m_ioa;
	required_device<netlist_mame_logic_input_device>            m_level;
	required_device<netlist_mame_logic_input_device>            m_levelt;
	required_device<netlist_mame_logic_input_device>            m_sw1;

	required_ioport                 m_inputs;

	u8  m_host_command;
};

#endif // MAME_AUDIO_ZACCARIA_H
