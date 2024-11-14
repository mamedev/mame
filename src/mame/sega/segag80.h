// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SEGA_SEGAG80_H
#define MAME_SEGA_SEGAG80_H

#pragma once

#include "machine/netlist.h"
#include "netlist/nl_setup.h"
#include "sound/ay8910.h"


class segag80_audio_device_base : public device_t, public device_mixer_interface
{
	using netlist_ptr = void (*)(netlist::nlparse_t &);

public:
	void write(offs_t addr, uint8_t data);
	void write_ay(offs_t addr, uint8_t data);

protected:
	segag80_audio_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 lomask, u8 himask, bool haspsg, netlist_ptr netlist, double output_scale);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	optional_device_array<netlist_mame_logic_input_device, 8> m_lo_input;
	optional_device_array<netlist_mame_logic_input_device, 8> m_hi_input;
	optional_device<ay8912_device> m_psg;

private:
	u8 const m_lo_mask;
	u8 const m_hi_mask;
	bool const m_has_psg;
	netlist_ptr const m_netlist;
	double const m_output_scale;
};


class elim_audio_device : public segag80_audio_device_base
{
public:
	elim_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class zektor_audio_device : public segag80_audio_device_base
{
public:
	zektor_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class spacfury_audio_device : public segag80_audio_device_base
{
public:
	spacfury_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class astrob_audio_device : public segag80_audio_device_base
{
public:
	astrob_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(ELIMINATOR_AUDIO, elim_audio_device)
DECLARE_DEVICE_TYPE(ZEKTOR_AUDIO, zektor_audio_device)
DECLARE_DEVICE_TYPE(SPACE_FURY_AUDIO, spacfury_audio_device)
DECLARE_DEVICE_TYPE(ASTRO_BLASTER_AUDIO, astrob_audio_device)

#endif // MAME_SEGA_SEGAG80_H
