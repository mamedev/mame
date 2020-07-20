// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_AUDIO_CINEMAT_H
#define MAME_AUDIO_CINEMAT_H

#pragma once

#include "machine/74259.h"
#include "machine/netlist.h"
#include "netlist/nl_setup.h"

// log to cinemat.csv for nltool playback/analysis
#define ENABLE_NETLIST_LOGGING		(1)


class cinemat_audio_device : public device_t
{
public:
	cinemat_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 inputs_mask, void (*netlist)(netlist::nlparse_t &), double output_scale);

	void configure_latch_inputs(ls259_device &latch, u8 mask = 0);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_stop() override;

	template<int _Index> DECLARE_WRITE_LINE_MEMBER(sound_w) { input_set(_Index, state); }
	void input_set(int bit, int state);

	optional_device_array<netlist_mame_logic_input_device, 8> m_out_input;

private:
	u8 m_inputs = 0xff;
	u8 m_inputs_mask = 0xff;
	void (*m_netlist)(netlist::nlparse_t &) = nullptr;
	double m_output_scale = 0;

#if ENABLE_NETLIST_LOGGING
	FILE *m_logfile = nullptr;
#endif

};


class spacewar_audio_device : public cinemat_audio_device
{
public:
	spacewar_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class barrier_audio_device : public cinemat_audio_device
{
public:
	barrier_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class speedfrk_audio_device : public cinemat_audio_device
{
public:
	speedfrk_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class starhawk_audio_device : public cinemat_audio_device
{
public:
	starhawk_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class sundance_audio_device : public cinemat_audio_device
{
public:
	sundance_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class tailg_audio_device : public cinemat_audio_device
{
public:
	tailg_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class warrior_audio_device : public cinemat_audio_device
{
public:
	warrior_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class armora_audio_device : public cinemat_audio_device
{
public:
	armora_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class ripoff_audio_device : public cinemat_audio_device
{
public:
	ripoff_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class starcas_audio_device : public cinemat_audio_device
{
public:
	starcas_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class solarq_audio_device : public cinemat_audio_device
{
public:
	solarq_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class boxingb_audio_device : public cinemat_audio_device
{
public:
	boxingb_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class wotw_audio_device : public cinemat_audio_device
{
public:
	wotw_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(SPACE_WARS_AUDIO, spacewar_audio_device)
DECLARE_DEVICE_TYPE(BARRIER_AUDIO, barrier_audio_device)
DECLARE_DEVICE_TYPE(STAR_HAWK_AUDIO, starhawk_audio_device)
DECLARE_DEVICE_TYPE(SUNDANCE_AUDIO, sundance_audio_device)
DECLARE_DEVICE_TYPE(SPEED_FREAK_AUDIO, speedfrk_audio_device)
DECLARE_DEVICE_TYPE(TAIL_GUNNER_AUDIO, tailg_audio_device)
DECLARE_DEVICE_TYPE(WARRIOR_AUDIO, warrior_audio_device)
DECLARE_DEVICE_TYPE(ARMOR_ATTACK_AUDIO, armora_audio_device)
DECLARE_DEVICE_TYPE(RIPOFF_AUDIO, ripoff_audio_device)
DECLARE_DEVICE_TYPE(SOLAR_QUEST_AUDIO, solarq_audio_device)
DECLARE_DEVICE_TYPE(BOXING_BUGS_AUDIO, boxingb_audio_device)
DECLARE_DEVICE_TYPE(STAR_CASTLE_AUDIO, starcas_audio_device)
DECLARE_DEVICE_TYPE(WAR_OF_THE_WORLDS_AUDIO, wotw_audio_device)

#endif // MAME_AUDIO_CINEMAT_H
