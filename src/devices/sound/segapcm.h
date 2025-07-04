// license:BSD-3-Clause
// copyright-holders:Hiromitsu Shioya, Olivier Galibert
/*********************************************************/
/*    SEGA 8bit PCM                                      */
/*********************************************************/

#ifndef MAME_SOUND_SEGAPCM_H
#define MAME_SOUND_SEGAPCM_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class segapcm_device : public device_t,
					   public device_sound_interface,
					   public device_rom_interface<21>
{
public:
	static constexpr int BANK_256    = 11;
	static constexpr int BANK_512    = 12;
	static constexpr int BANK_12M    = 13;
	static constexpr int BANK_MASK7  = 0x70 << 16;
	static constexpr int BANK_MASKF  = 0xf0 << 16;
	static constexpr int BANK_MASKF8 = 0xf8 << 16;

	segapcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_bank(int bank) { m_bankshift = (bank & 0xf); m_bankmask = (0x70|((bank >> 16) & 0xfc)); }

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

private:
	std::unique_ptr<uint8_t[]> m_ram;
	uint8_t m_low[16];
	int m_bankshift;
	int m_bankmask;
	sound_stream* m_stream;
};

DECLARE_DEVICE_TYPE(SEGAPCM, segapcm_device)

#endif // MAME_SOUND_SEGAPCM_H
