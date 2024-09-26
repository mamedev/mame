// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_CINEMATRONICS_CINEMAT_A_H
#define MAME_CINEMATRONICS_CINEMAT_A_H

#pragma once

#include "machine/74259.h"
#include "machine/netlist.h"
#include "netlist/nl_setup.h"


class cinemat_audio_device_base : public device_t, public device_mixer_interface
{
public:
	cinemat_audio_device_base &configure_latch_inputs(ls259_device &latch, u8 mask = 0);

protected:
	cinemat_audio_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 inputs_mask, void (*netlist)(netlist::nlparse_t &), double output_scale);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	template <int Index> void sound_w(int state) { input_set(Index, state); }
	void input_set(int bit, int state);

	optional_device_array<netlist_mame_logic_input_device, 8> m_out_input;

private:
	u8 m_inputs = 0xff;
	u8 const m_inputs_mask;
	void (*const m_netlist)(netlist::nlparse_t &);
	double const m_output_scale;
};


class spacewar_audio_device : public cinemat_audio_device_base
{
public:
	spacewar_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class barrier_audio_device : public cinemat_audio_device_base
{
public:
	barrier_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class speedfrk_audio_device : public cinemat_audio_device_base
{
public:
	speedfrk_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class starhawk_audio_device : public cinemat_audio_device_base
{
public:
	starhawk_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class sundance_audio_device : public cinemat_audio_device_base
{
public:
	sundance_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class tailg_audio_device : public cinemat_audio_device_base
{
public:
	tailg_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class warrior_audio_device : public cinemat_audio_device_base
{
public:
	warrior_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class armora_audio_device : public cinemat_audio_device_base
{
public:
	armora_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class ripoff_audio_device : public cinemat_audio_device_base
{
public:
	ripoff_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class starcas_audio_device : public cinemat_audio_device_base
{
public:
	starcas_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class solarq_audio_device : public cinemat_audio_device_base
{
public:
	solarq_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class boxingb_audio_device : public cinemat_audio_device_base
{
public:
	boxingb_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class wotw_audio_device : public cinemat_audio_device_base
{
public:
	wotw_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
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

#endif // MAME_CINEMATRONICS_CINEMAT_A_H
