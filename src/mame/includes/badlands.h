// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Bad Lands hardware

*************************************************************************/

#include "machine/atarigen.h"
#include "machine/timer.h"
#include "video/atarimo.h"

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
	DECLARE_READ16_MEMBER(badlandsb_unk_r);
	DECLARE_DRIVER_INIT(badlands);
	TILE_GET_INFO_MEMBER(get_playfield_tile_info);
	DECLARE_MACHINE_START(badlands);
	DECLARE_MACHINE_RESET(badlands);
	DECLARE_VIDEO_START(badlands);
	DECLARE_MACHINE_RESET(badlandsb);
	uint32_t screen_update_badlands(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_int);
	TIMER_DEVICE_CALLBACK_MEMBER(sound_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(bootleg_sound_scanline);
	DECLARE_WRITE16_MEMBER( badlands_pf_bank_w );
	DECLARE_READ8_MEMBER(bootleg_shared_r);
	DECLARE_WRITE8_MEMBER(bootleg_shared_w);
	DECLARE_WRITE8_MEMBER(bootleg_main_irq_w);

	static const atari_motion_objects_config s_mob_config;
};
