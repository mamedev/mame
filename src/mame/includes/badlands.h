// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Bad Lands hardware

*************************************************************************/

#ifndef MAME_INCLUDES_BADLANDS_H
#define MAME_INCLUDES_BADLANDS_H

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m6502.h"
#include "machine/eeprompar.h"
#include "machine/watchdog.h"
#include "machine/atarigen.h"
#include "machine/timer.h"
#include "sound/ym2151.h"
#include "video/atarimo.h"

#include "speaker.h"

/*----------- defined in machine/badlands.cpp -----------*/

//extern const gfx_layout badlands_molayout;

INPUT_PORTS_EXTERN(badlands);


class badlands_state : public atarigen_state
{
public:
	badlands_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_audiocpu(*this, "audiocpu"),
			m_soundcomm(*this, "soundcomm"),
			m_playfield_tilemap(*this, "playfield"),
			m_mob(*this, "mob"),
			m_b_sharedram(*this, "b_sharedram")
			{ }

	optional_device<cpu_device> m_audiocpu;
	optional_device<atari_sound_comm_device> m_soundcomm;

	required_device<tilemap_device> m_playfield_tilemap;
	required_device<atari_motion_objects_device> m_mob;
	optional_shared_ptr<uint8_t> m_b_sharedram;

	uint8_t           m_pedal_value[2];
	uint8_t           m_playfield_tile_bank;

	virtual void update_interrupts() override;
	virtual void scanline_update(screen_device &screen, int scanline) override;
	DECLARE_READ16_MEMBER(sound_busy_r);
	DECLARE_READ16_MEMBER(pedal_0_r);
	DECLARE_READ16_MEMBER(pedal_1_r);
	DECLARE_READ8_MEMBER(audio_io_r);
	DECLARE_WRITE8_MEMBER(audio_io_w);
	DECLARE_DRIVER_INIT(badlands);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(badlands);
	DECLARE_MACHINE_RESET(badlands);
	DECLARE_VIDEO_START(badlands);
	uint32_t screen_update_badlands(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_int);
	TIMER_DEVICE_CALLBACK_MEMBER(sound_scanline);
	DECLARE_WRITE16_MEMBER( badlands_pf_bank_w );

	static const atari_motion_objects_config s_mob_config;
	void badlands(machine_config &config);
	void audio_map(address_map &map);
	void main_map(address_map &map);
};

class badlandsbl_state : public badlands_state
{
public:
	badlandsbl_state(const machine_config &mconfig, device_type type, const char *tag)
		: badlands_state(mconfig, type, tag)
	{}
	
	DECLARE_READ8_MEMBER(bootleg_shared_r);
	DECLARE_WRITE8_MEMBER(bootleg_shared_w);
	DECLARE_WRITE8_MEMBER(bootleg_main_irq_w);
	DECLARE_READ16_MEMBER(badlandsb_unk_r);
	TIMER_DEVICE_CALLBACK_MEMBER(bootleg_sound_scanline);

	void badlandsb(machine_config &config);
	void bootleg_map(address_map &map);
	void bootleg_audio_map(address_map &map);
	
protected:
	virtual void machine_reset() override;
};


#endif // MAME_INCLUDES_BADLANDS_H


