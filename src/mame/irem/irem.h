// license:BSD-3-Clause
// copyright-holders:Couriersud
#ifndef MAME_IREM_IREM_H
#define MAME_IREM_IREM_H

#pragma once

#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "machine/netlist.h"
#include "nl_kidniki.h"

class irem_audio_device : public device_t
{
public:
	void cmd_w(uint8_t data);
	uint8_t soundlatch_r();
	void ay8910_45M_portb_w(uint8_t data);
	void ay8910_45L_porta_w(uint8_t data);

	void irem_sound_portmap(address_map &map) ATTR_COLD;
	void m52_large_sound_map(address_map &map) ATTR_COLD;
	void m52_small_sound_map(address_map &map) ATTR_COLD;
	void m62_sound_map(address_map &map) ATTR_COLD;

	optional_device<netlist_mame_logic_input_device> m_audio_SINH;

protected:
	irem_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<cpu_device> m_cpu;
	required_device<msm5205_device> m_adpcm1;
	optional_device<msm5205_device> m_adpcm2;
	required_device<ay8910_device> m_ay_45L;
	required_device<ay8910_device> m_ay_45M;

	void m6803_port1_w(uint8_t data);
	void m6803_port2_w(uint8_t data);
	uint8_t m6803_port1_r();
	uint8_t m6803_port2_r();
	void sound_irq_ack_w(uint8_t data);
	void m52_adpcm_w(offs_t offset, uint8_t data);
	void m62_adpcm_w(offs_t offset, uint8_t data);

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
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class m52_soundc_audio_device : public irem_audio_device
{
public:
	m52_soundc_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class m52_large_audio_device : public irem_audio_device
{
public:
	m52_large_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(IREM_M62_AUDIO,        m62_audio_device)
DECLARE_DEVICE_TYPE(IREM_M52_SOUNDC_AUDIO, m52_soundc_audio_device)
DECLARE_DEVICE_TYPE(IREM_M52_LARGE_AUDIO,  m52_large_audio_device)

#endif // MAME_IREM_IREM_H
