// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega G-80 raster hardware

    Across these games, there's a mixture of discrete sound circuitry,
    speech boards, ADPCM samples, and a TMS3617 music chip.

***************************************************************************/

#ifndef MAME_AUDIO_SEGAG80R
#define MAME_AUDIO_SEGAG80R

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"
#include "machine/i8255.h"
#include "sound/samples.h"
#include "sound/tms36xx.h"

class monsterb_sound_device : public device_t
{
public:
	template <typename T> monsterb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&region_tag)
		: monsterb_sound_device(mconfig, tag, owner, clock)
	{
		m_audiocpu_region.set_tag(std::forward<T>(region_tag));
	}

	monsterb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(n7751_status_r);
	DECLARE_WRITE8_MEMBER(n7751_command_w);

	DECLARE_WRITE8_MEMBER(sound_a_w);
	DECLARE_WRITE8_MEMBER(sound_b_w);


protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	DECLARE_READ8_MEMBER(n7751_command_r);
	DECLARE_WRITE8_MEMBER(n7751_p2_w);

	DECLARE_READ8_MEMBER(n7751_rom_r);
	template<int Shift> void n7751_rom_addr_w(uint8_t data);
	void n7751_rom_select_w(uint8_t data);

	required_device<n7751_device> m_audiocpu;
	required_memory_region m_audiocpu_region;

	required_device<tms36xx_device> m_music;
	required_device<samples_device> m_samples;

	required_device<i8243_device> m_i8243;

	uint8_t m_n7751_command;
	uint8_t m_n7751_busy;
	uint8_t m_sound_state[2];
	uint16_t m_sound_addr;
};

DECLARE_DEVICE_TYPE(MONSTERB_SOUND, monsterb_sound_device)

#endif // MAME_AUDIO_SEGAG80R
