// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_AUDIO_SEGAG80V_H
#define MAME_AUDIO_SEGAG80V_H

#pragma once

#include "machine/netlist.h"
#include "netlist/nl_setup.h"
#include "sound/ay8910.h"

// log to netlist.csv for nltool playback/analysis
#define ENABLE_NETLIST_LOGGING		(1)


class segag80_audio_device : public device_t, public device_mixer_interface
{
public:
	segag80_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, void (*netlist)(netlist::nlparse_t &), double output_scale);

	void write(offs_t addr, uint8_t data);
	void write_ay(offs_t addr, uint8_t data);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_stop() override;

	optional_device_array<netlist_mame_logic_input_device, 8> m_lo_input;
	optional_device_array<netlist_mame_logic_input_device, 8> m_hi_input;
	optional_device<ay8912_device> m_psg;

private:
	u8 m_lo_vals = 0xff;
	u8 m_hi_vals = 0xff;
	void (*m_netlist)(netlist::nlparse_t &) = nullptr;
	double m_output_scale = 0;

#if ENABLE_NETLIST_LOGGING
	FILE *m_logfile = nullptr;
#endif

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

protected:
	void device_add_mconfig(machine_config &config) override;
};


class spcfury_audio_device : public segag80_audio_device
{
public:
	spcfury_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};



DECLARE_DEVICE_TYPE(ELIMINATOR_AUDIO, elim_audio_device)
DECLARE_DEVICE_TYPE(ZEKTOR_AUDIO, zektor_audio_device)
DECLARE_DEVICE_TYPE(SPACE_FURY_AUDIO, spcfury_audio_device)

#endif // MAME_AUDIO_SEGAG80V_H
