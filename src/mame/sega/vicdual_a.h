// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SEGA_VICDUAL_A_H
#define MAME_SEGA_VICDUAL_A_H

#pragma once

#include "machine/netlist.h"
#include "netlist/nl_setup.h"


class vicdual_audio_device_base : public device_t, public device_mixer_interface
{
protected:
	vicdual_audio_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 inputs_mask, void (*netlist)(netlist::nlparse_t &), double output_scale);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

public:
	void write(u8 data);

private:
	optional_device_array<netlist_mame_logic_input_device, 8> m_input_line;
	u8 m_input_state = 0xff;
	u8 const m_inputs_mask;
	void (*const m_netlist)(netlist::nlparse_t &);
	double const m_output_scale;
};


class borderline_audio_device : public vicdual_audio_device_base
{
public:
	borderline_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class frogs_audio_device : public vicdual_audio_device_base
{
public:
	frogs_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(BORDERLINE_AUDIO, borderline_audio_device)
DECLARE_DEVICE_TYPE(FROGS_AUDIO, frogs_audio_device)

#endif // MAME_SEGA_VICDUAL_A_H
