// license:BSD-3-Clause
// copyright-holders:Luca Elia,Olivier Galibert,Paul Priest
/*************************************************************************

    Psikyo Games

*************************************************************************/
#ifndef MAME_PSIKYO_PSIKYO_H
#define MAME_PSIKYO_PSIKYO_H

#pragma once

#include "machine/gen_latch.h"
#include "video/bufsprite.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#include <algorithm>

class psikyo_state : public driver_device
{
public:
	psikyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_vram(*this, "vram_%u", 0U, 0x2000U, ENDIANNESS_BIG)
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
		, m_spriteram(*this, "spriteram")
	{
		std::fill(std::begin(m_old_linescroll), std::end(m_old_linescroll), ~u32(0));
		std::fill(std::begin(m_old_tmapsize), std::end(m_old_tmapsize), ~u32(0));
		std::fill(std::begin(m_tmapsize), std::end(m_tmapsize), ~u32(0));
	}

	void sngkace(machine_config &config);
	void s1945n(machine_config &config);
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

	int z80_nmi_r();
	int mcu_status_r();

private:
	/* memory pointers */
	memory_share_array_creator<u16, 2> m_vram;
	required_shared_ptr<u32> m_vregs;
	optional_shared_ptr<u32> m_bootleg_spritebuffer;

	required_region_ptr<u16> m_spritelut;
	optional_memory_bank m_audiobank;
	optional_memory_bank m_okibank;
	optional_ioport m_in_dsw;
	optional_ioport m_in_p1_p2;
	optional_ioport m_in_coin;

	/* video-related */
	struct sprite_t
	{
		u8 gfx = 0;
		u32 code = 0, color = 0;
		bool flipx = false, flipy = false;
		s32 x = 0, y = 0;
		u32 zoomx = 0, zoomy = 0;
		u32 primask = 0;
	};

	tilemap_t   *m_tilemap[2]{};
	u8          m_tilemap_bank[2]{};
	bool        m_ka302c_banking = false;
	u32         m_old_linescroll[2]{};
	u32         m_old_tmapsize[2]{};
	u32         m_tmapsize[2]{};
	std::unique_ptr<sprite_t[]> m_spritelist;
	struct sprite_t *m_sprite_ptr_pre;
	u16         m_sprite_ctrl = 0;

	/* game-specific */
	// 1945 MCU
	int         m_mcu_status = 0;
	u8          m_s1945_mcu_direction = 0;
	u8          m_s1945_mcu_latch1 = 0;
	u8          m_s1945_mcu_latch2 = 0;
	u8          m_s1945_mcu_inlatch = 0;
	u8          m_s1945_mcu_index = 0;
	u8          m_s1945_mcu_latching = 0;
	u8          m_s1945_mcu_mode = 0;
	u8          m_s1945_mcu_control = 0;
	u8          m_s1945_mcu_bctrl = 0;
	const u8    *m_s1945_mcu_table = nullptr;

	u32 sngkace_input_r(offs_t offset);
	u32 gunbird_input_r(offs_t offset);
	void s1945_mcu_data_w(uint8_t data);
	void s1945_mcu_control_w(uint8_t data);
	void s1945_mcu_direction_w(uint8_t data);
	void s1945_mcu_bctrl_w(uint8_t data);
	void s1945_mcu_command_w(uint8_t data);
	u32 s1945_mcu_data_r();
	uint8_t s1945_mcu_control_r();
	u32 s1945_input_r(offs_t offset);
	void s1945bl_okibank_w(u8 data);
	template<int Shift> void sound_bankswitch_w(u8 data);
	template<int Layer> u16 vram_r(offs_t offset);
	template<int Layer> void vram_w(offs_t offset, u16 data, u16 mem_mask);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	template<int Layer> TILEMAP_MAPPER_MEMBER(tile_scan);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	DECLARE_VIDEO_START(sngkace);
	DECLARE_VIDEO_START(psikyo);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_bootleg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void screen_vblank_bootleg(int state);
	void switch_bgbanks(u8 tmap, u8 bank);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void get_sprites();
	void get_sprites_bootleg();
	void s1945_mcu_init();

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<generic_latch_8_device> m_soundlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram32_device> m_spriteram;

	void gunbird_map(address_map &map) ATTR_COLD;
	void gunbird_sound_io_map(address_map &map) ATTR_COLD;
	void gunbird_sound_map(address_map &map) ATTR_COLD;
	void psikyo_bootleg_map(address_map &map) ATTR_COLD;
	void psikyo_map(address_map &map) ATTR_COLD;
	void s1945_map(address_map &map) ATTR_COLD;
	void s1945_sound_io_map(address_map &map) ATTR_COLD;
	void s1945bl_oki_map(address_map &map) ATTR_COLD;
	void s1945n_map(address_map &map) ATTR_COLD;
	void sngkace_map(address_map &map) ATTR_COLD;
	void sngkace_sound_io_map(address_map &map) ATTR_COLD;
	void sngkace_sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_PSIKYO_PSIKYO_H
