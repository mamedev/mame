// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_AUDIO_VICDUAL_H
#define MAME_AUDIO_VICDUAL_H

#pragma once

#include "machine/netlist.h"
#include "netlist/nl_setup.h"


class borderline_audio_device : public device_t, public device_mixer_interface
{
public:
	borderline_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	void write(u8 data);

private:
	optional_device_array<netlist_mame_logic_input_device, 8> m_input_line;
	u8 m_input_state = 0xff;
};


DECLARE_DEVICE_TYPE(BORDERLINE_AUDIO, borderline_audio_device)

#endif // MAME_AUDIO_VICDUAL_H
