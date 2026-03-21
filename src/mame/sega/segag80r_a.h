// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega G-80 raster hardware

    Across these games, there's a mixture of discrete sound circuitry,
    speech boards, ADPCM samples, and a TMS3617 music chip.

***************************************************************************/
#ifndef MAME_SEGA_SEGAG80R_A_H
#define MAME_SEGA_SEGAG80R_A_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"
#include "machine/i8255.h"
#include "sound/samples.h"
#include "sound/tms36xx.h"

#include <utility>


class sega005_sound_device : public device_t, public device_sound_interface
{
public:
	sega005_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_sound_region(T &&tag) { m_sound_region.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_proms_region(T &&tag) { m_proms_region.set_tag(std::forward<T>(tag)); }

	void b_w(uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	TIMER_CALLBACK_MEMBER(auto_timer);

	void update_sound_data();

	required_region_ptr<u8> m_sound_region;
	required_region_ptr<u8> m_proms_region;
	emu_timer *m_sound_timer;
	sound_stream *m_stream;

	uint16_t m_sound_addr;
	uint8_t m_sound_data;
	uint8_t m_square_state;
	uint8_t m_square_count;
	uint8_t m_pb_state;
};


class monsterb_sound_device : public device_t
{
public:
	template <typename T> monsterb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&region_tag)
		: monsterb_sound_device(mconfig, tag, owner, clock)
	{
		m_audiocpu_region.set_tag(std::forward<T>(region_tag));
	}

	monsterb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t upd7751_status_r();
	void upd7751_command_w(uint8_t data);

	void sound_a_w(uint8_t data);
	void sound_b_w(uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	uint8_t upd7751_command_r();
	void upd7751_p2_w(uint8_t data);

	uint8_t upd7751_rom_r();
	template<int Shift> void upd7751_rom_addr_w(uint8_t data);
	void upd7751_rom_select_w(uint8_t data);

	required_device<upd7751_device> m_audiocpu;
	required_memory_region m_audiocpu_region;

	required_device<tms36xx_device> m_music;
	required_device<samples_device> m_samples;

	required_device<i8243_device> m_i8243;

	uint8_t m_upd7751_command;
	uint8_t m_upd7751_busy;
	uint8_t m_sound_state[2];
	uint16_t m_sound_addr;
};


DECLARE_DEVICE_TYPE(SEGA005, sega005_sound_device)
DECLARE_DEVICE_TYPE(MONSTERB_SOUND, monsterb_sound_device)

#endif // MAME_SEGA_SEGAG80R_A_H
