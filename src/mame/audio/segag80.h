// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_AUDIO_SEGAG80V_H
#define MAME_AUDIO_SEGAG80V_H

#pragma once

#include "machine/i8255.h"
#include "machine/netlist.h"
#include "netlist/nl_setup.h"
#include "sound/ay8910.h"


class segag80_audio_device : public device_t, public device_mixer_interface
{
public:
	segag80_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 lomask, u8 himask, bool haspsg, void (*netlist)(netlist::nlparse_t &), double output_scale);

	void write(offs_t addr, uint8_t data);
	void write_ay(offs_t addr, uint8_t data);

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	optional_device_array<netlist_mame_logic_input_device, 8> m_lo_input;
	optional_device_array<netlist_mame_logic_input_device, 8> m_hi_input;
	optional_device<ay8912_device> m_psg;

private:
	u8 m_lo_vals;
	u8 m_hi_vals;
	u8 m_lo_mask;
	u8 m_hi_mask;
	bool m_has_psg;
	void (*m_netlist)(netlist::nlparse_t &) = nullptr;
	double m_output_scale = 0;
};


class elim_audio_device : public segag80_audio_device
{
public:
	elim_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class zektor_audio_device : public segag80_audio_device
{
public:
	zektor_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class spacfury_audio_device : public segag80_audio_device
{
public:
	spacfury_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class astrob_audio_device : public segag80_audio_device
{
public:
	astrob_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class spaceod_audio_device : public segag80_audio_device
{
public:
	spaceod_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class sega005_audio_device : public device_t, public device_mixer_interface
{
public:
	sega005_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void write(offs_t addr, uint8_t data) { m_ppi->write(addr, data); }

protected:
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	optional_device_array<netlist_mame_logic_input_device, 8> m_a_input;
	optional_device_array<netlist_mame_logic_input_device, 8> m_b_input;
	required_device<i8255_device> m_ppi;

private:
	void sound_a_w(u8 data);
	void sound_b_w(u8 data);
};


DECLARE_DEVICE_TYPE(ELIMINATOR_AUDIO, elim_audio_device)
DECLARE_DEVICE_TYPE(ZEKTOR_AUDIO, zektor_audio_device)
DECLARE_DEVICE_TYPE(SPACE_FURY_AUDIO, spacfury_audio_device)
DECLARE_DEVICE_TYPE(ASTRO_BLASTER_AUDIO, astrob_audio_device)
DECLARE_DEVICE_TYPE(SPACE_ODYSSEY_AUDIO, spaceod_audio_device)
DECLARE_DEVICE_TYPE(SEGA_005_AUDIO, sega005_audio_device)

#endif // MAME_AUDIO_SEGAG80V_H
