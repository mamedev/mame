// license:BSD-3-Clause
// copyright-holders:Luca Elia,Olivier Galibert,Paul Priest
/*************************************************************************

    Psikyo Games

*************************************************************************/
#ifndef MAME_INCLUDES_PSIKYO_H
#define MAME_INCLUDES_PSIKYO_H

#pragma once

#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"

#include <algorithm>

class psikyo_state : public driver_device
{
public:
	psikyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spriteram(*this, "spriteram")
		, m_vram(*this, "vram_%u", 0)
		, m_vregs(*this, "vregs")
		, m_bootleg_spritebuffer(*this, "boot_spritebuf")
		, m_spritelut(*this, "spritelut")
		, m_audiobank(*this, "audiobank")
		, m_okibank(*this, "okibank")
		, m_in_dsw(*this, "DSW")
		, m_in_p1_p2(*this, "P1_P2")
		, m_in_coin(*this, "COIN")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_soundlatch(*this, "soundlatch")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{
		std::fill(std::begin(m_old_linescroll), std::end(m_old_linescroll), 0);
	}

	void sngkace(machine_config &config);
	void s1945jn(machine_config &config);
	void gunbird(machine_config &config);
	void s1945(machine_config &config);
	void s1945bl(machine_config &config);

	void init_s1945a();
	void init_s1945j();
	void init_sngkace();
	void init_s1945();
	void init_s1945bl();
	void init_tengai();
	void init_gunbird();

	DECLARE_CUSTOM_INPUT_MEMBER(z80_nmi_r);
	DECLARE_CUSTOM_INPUT_MEMBER(mcu_status_r);

private:
	/* memory pointers */
	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr_array<uint32_t, 2> m_vram;
	required_shared_ptr<uint32_t> m_vregs;
	optional_shared_ptr<uint32_t> m_bootleg_spritebuffer;
	std::unique_ptr<uint32_t[]>       m_spritebuf[2];

	required_memory_region m_spritelut;
	optional_memory_bank m_audiobank;
	optional_memory_bank m_okibank;
	optional_ioport m_in_dsw;
	optional_ioport m_in_p1_p2;
	optional_ioport m_in_coin;

	/* video-related */
	tilemap_t        *m_tilemap[2][4];
	int            m_tilemap_bank[2];
	int            m_ka302c_banking;
	int            m_old_linescroll[2];

	/* game-specific */
	// 1945 MCU
	int            m_mcu_status;
	uint8_t          m_s1945_mcu_direction;
	uint8_t          m_s1945_mcu_latch1;
	uint8_t          m_s1945_mcu_latch2;
	uint8_t          m_s1945_mcu_inlatch;
	uint8_t          m_s1945_mcu_index;
	uint8_t          m_s1945_mcu_latching;
	uint8_t          m_s1945_mcu_mode;
	uint8_t          m_s1945_mcu_control;
	uint8_t          m_s1945_mcu_bctrl;
	const uint8_t    *m_s1945_mcu_table;

	DECLARE_READ32_MEMBER(sngkace_input_r);
	DECLARE_READ32_MEMBER(gunbird_input_r);
	DECLARE_WRITE32_MEMBER(s1945_mcu_w);
	DECLARE_READ32_MEMBER(s1945_mcu_r);
	DECLARE_READ32_MEMBER(s1945_input_r);
	DECLARE_READ32_MEMBER(s1945bl_oki_r);
	DECLARE_WRITE32_MEMBER(s1945bl_oki_w);
	template<int Shift> DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	template<int Layer> DECLARE_WRITE32_MEMBER(vram_w);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_VIDEO_START(sngkace);
	DECLARE_VIDEO_START(psikyo);
	uint32_t screen_update_psikyo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_psikyo_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_psikyo);
	void psikyo_switch_banks( int tmap, int bank );
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int trans_pen );
	void draw_sprites_bootleg( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int trans_pen );
	int tilemap_width( int size );
	void s1945_mcu_init(  );

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<generic_latch_8_device> m_soundlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void gunbird_map(address_map &map);
	void gunbird_sound_io_map(address_map &map);
	void gunbird_sound_map(address_map &map);
	void psikyo_bootleg_map(address_map &map);
	void psikyo_map(address_map &map);
	void s1945_map(address_map &map);
	void s1945_sound_io_map(address_map &map);
	void s1945bl_oki_map(address_map &map);
	void s1945jn_map(address_map &map);
	void sngkace_map(address_map &map);
	void sngkace_sound_io_map(address_map &map);
	void sngkace_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_PSIKYO_H
