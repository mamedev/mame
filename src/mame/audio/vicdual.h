// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_AUDIO_VICDUAL_H
#define MAME_AUDIO_VICDUAL_H

#pragma once

#include "machine/netlist.h"
#include "netlist/nl_setup.h"
#include "audio/nl_frogs.h"


class vicdual_audio_device_base : public device_t, public device_mixer_interface
{
protected:
	vicdual_audio_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 inputs_mask, void (*netlist)(netlist::nlparse_t &), double output_scale);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

public:
	virtual void write(u8 data);

protected:
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

#if (FROGS_TEST_INDEPENDENT_NETLISTS)
DECLARE_DEVICE_TYPE(FROGS_AUDIO_COMPONENT, frogs_audio_component_device)

class frogs_audio_component_device : public device_t, public device_mixer_interface
{
public:
	frogs_audio_component_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, void (*netlist)(netlist::nlparse_t &) = nullptr);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

protected:
	void (*const m_netlist)(netlist::nlparse_t &);
	u8 m_dummy;
};
#endif

class frogs_audio_device : public vicdual_audio_device_base
{
public:
	frogs_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

#if (FROGS_TEST_INDEPENDENT_NETLISTS)
	virtual void write(u8 data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<netlist_mame_logic_input_device> m_jump;
	required_device<netlist_mame_logic_input_device> m_tongue;
	required_device<netlist_mame_logic_input_device> m_hop;
	required_device<netlist_mame_logic_input_device> m_capture;
	required_device<netlist_mame_logic_input_device> m_splash;
	required_device<netlist_mame_logic_input_device> m_fly;
#endif
};


DECLARE_DEVICE_TYPE(BORDERLINE_AUDIO, borderline_audio_device)
DECLARE_DEVICE_TYPE(FROGS_AUDIO, frogs_audio_device)

#endif // MAME_AUDIO_VICDUAL_H
