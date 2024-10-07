// license:BSD-3-Clause
// copyright-holders:David Haywood
/* */


#ifndef MAME_MISC_INDER_SB_H
#define MAME_MISC_INDER_SB_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "sound/dac.h"


DECLARE_DEVICE_TYPE(INDER_AUDIO, inder_sb_device)


class inder_sb_device :  public device_t,
							public device_mixer_interface
{
public:
	// construction/destruction
	inder_sb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t megaphx_sound_sent_r();
	uint8_t megaphx_sound_cmd_r();
	void megaphx_sound_to_68k_w(uint8_t data);

	void dac0_rombank_write(uint8_t data);
	void dac1_rombank_write(uint8_t data);
	void dac2_rombank_write(uint8_t data);
	void dac3_rombank_write(uint8_t data);

	uint8_t vec_bankswitch_r(offs_t offset);


	uint16_t megaphx_0x050002_r(offs_t offset, uint16_t mem_mask = ~0);
	void megaphx_0x050000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void sound_io(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_audiocpu;
	required_device<z80ctc_device> m_ctc;
	required_region_ptr<uint8_t> m_audiocpu_rom;
	required_memory_bank m_sounddata_bank;

	uint8_t m_soundbank[4];

	int m_soundsent;
	uint8_t m_sounddata;
	uint8_t m_soundback;
};

#endif // MAME_MISC_INDER_SB_H
