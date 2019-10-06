// license:BSD-3-Clause
// copyright-holders:Couriersud
#ifndef MAME_AUDIO_IREM_H
#define MAME_AUDIO_IREM_H

#pragma once

#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "machine/netlist.h"
#include "audio/nl_kidniki.h"

class irem_audio_device : public device_t
{
public:
	DECLARE_WRITE8_MEMBER( cmd_w );
	DECLARE_WRITE8_MEMBER( m6803_port1_w );
	DECLARE_WRITE8_MEMBER( m6803_port2_w );
	DECLARE_READ8_MEMBER( m6803_port1_r );
	DECLARE_READ8_MEMBER( m6803_port2_r );
	DECLARE_WRITE8_MEMBER( sound_irq_ack_w );
	DECLARE_WRITE8_MEMBER( m52_adpcm_w );
	DECLARE_WRITE8_MEMBER( m62_adpcm_w );

	DECLARE_READ8_MEMBER( soundlatch_r );
	DECLARE_WRITE8_MEMBER( ay8910_45M_portb_w );
	DECLARE_WRITE8_MEMBER( ay8910_45L_porta_w );

	void irem_sound_portmap(address_map &map);
	void m52_large_sound_map(address_map &map);
	void m52_small_sound_map(address_map &map);
	void m62_sound_map(address_map &map);

	optional_device<netlist_mame_logic_input_device> m_audio_SINH;

protected:
	irem_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<cpu_device> m_cpu;
	required_device<msm5205_device> m_adpcm1;
	optional_device<msm5205_device> m_adpcm2;
	required_device<ay8910_device> m_ay_45L;
	required_device<ay8910_device> m_ay_45M;

private:
	// internal state
	uint8_t           m_port1;
	uint8_t           m_port2;

	uint8_t           m_soundlatch;

	optional_device<netlist_mame_logic_input_device> m_audio_BD;
	optional_device<netlist_mame_logic_input_device> m_audio_SD;
	optional_device<netlist_mame_logic_input_device> m_audio_OH;
	optional_device<netlist_mame_logic_input_device> m_audio_CH;
};

class m62_audio_device : public irem_audio_device
{
public:
	m62_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

class m52_soundc_audio_device : public irem_audio_device
{
public:
	m52_soundc_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

class m52_large_audio_device : public irem_audio_device
{
public:
	m52_large_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
};


DECLARE_DEVICE_TYPE(IREM_M62_AUDIO,        m62_audio_device)
DECLARE_DEVICE_TYPE(IREM_M52_SOUNDC_AUDIO, m52_soundc_audio_device)
DECLARE_DEVICE_TYPE(IREM_M52_LARGE_AUDIO,  m52_large_audio_device)

#endif // MAME_AUDIO_IREM_H
