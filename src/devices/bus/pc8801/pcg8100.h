// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC8801_PCG8100_H
#define MAME_BUS_PC8801_PCG8100_H

#pragma once

#include "pc8801_exp.h"
#include "machine/pit8253.h"
#include "sound/spkrdev.h"

class pcg8100_device : public pc8801_exp_device
{
public:
	pcg8100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	u8 m_pcg_data = 0;
	u16 m_pcg_address = 0;
	u8 m_pcg_latch = 0;

	void pcg_latch_w(u8 data);

	required_device<pit8253_device> m_pit;
	required_device_array<speaker_sound_device, 3> m_dac1bit;

	required_region_ptr<u8> m_cg_rom;
	std::array<u8, 0x400> m_original_rom;
	void audio_channel(u8 ch, bool keyon);
};

DECLARE_DEVICE_TYPE(PCG8100, pcg8100_device)


#endif // MAME_BUS_PC8801_JMBX1_H
