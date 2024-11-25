// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_ROLAND_JX8P_SYNTH_H
#define MAME_ROLAND_JX8P_SYNTH_H

#pragma once

#include "cpu/mcs51/mcs51.h"
#include "machine/i8155.h"
#include "machine/pit8253.h"


class jx8p_synth_device : public device_t
{
public:
	jx8p_synth_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	jx8p_synth_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void prescale_w(u8 data);
	void adc_w(offs_t offset, u8 data);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_synthcpu;
	required_device<i8155_device> m_ramio;
	required_device_array<pit8253_device, 4> m_pit;
};

class superjx_synth_device : public jx8p_synth_device
{
public:
	superjx_synth_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(JX8P_SYNTH, jx8p_synth_device)
DECLARE_DEVICE_TYPE(SUPERJX_SYNTH, superjx_synth_device)

#endif // MAME_ROLAND_JX8P_SYNTH_H
