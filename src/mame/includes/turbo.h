// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Howie Cohen, Frank Palazzolo, Ernesto Corvi, Aaron Giles
/*************************************************************************

    Sega Z80-3D system

*************************************************************************/
#ifndef MAME_INCLUDES_TURBO_H
#define MAME_INCLUDES_TURBO_H

#pragma once

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/discrete.h"
#include "sound/samples.h"
#include "emupal.h"
#include "screen.h"

/* sprites are scaled in the analog domain; to give a better */
/* rendition of this, we scale in the X direction by this factor */
#define TURBO_X_SCALE       2



class turbo_state : public driver_device
{
public:
	turbo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_i8255_0(*this, "i8255_0")
		, m_i8255_1(*this, "i8255_1")
		, m_i8255_2(*this, "i8255_2")
		, m_i8255_3(*this, "i8255_3")
		, m_spriteroms(*this, "sprites")
		, m_proms(*this, "proms")
		, m_roadroms(*this, "road")
		, m_bgcolorrom(*this, "bgcolor")
		, m_videoram(*this, "videoram")
		, m_spriteram(*this, "spriteram")
		, m_sprite_position(*this, "spritepos")
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_samples(*this, "samples")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_digits(*this, "digit%u", 0U)
		, m_lamp(*this, "lamp")
	{ }

	void turbo(machine_config &config);
	void buckrog(machine_config &config);
	void buckroge(machine_config &config);
	void buckrogu(machine_config &config);
	void subroc3d(machine_config &config);
	void turbo_samples(machine_config &config);
	void subroc3d_samples(machine_config &config);
	void buckrog_samples(machine_config &config);

	void init_turbo_enc();
	void init_turbo_noenc();

private:
	/* device/memory pointers */
	required_device<z80_device> m_maincpu;
	optional_device<z80_device> m_subcpu;
	required_device<i8255_device> m_i8255_0;
	required_device<i8255_device> m_i8255_1;
	optional_device<i8255_device> m_i8255_2;
	optional_device<i8255_device> m_i8255_3;

	required_region_ptr<uint8_t> m_spriteroms;
	required_region_ptr<uint8_t> m_proms;
	optional_region_ptr<uint8_t> m_roadroms;
	optional_region_ptr<uint8_t> m_bgcolorrom;

	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_sprite_position;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	required_device<samples_device> m_samples;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	output_finder<32> m_digits;
	output_finder<> m_lamp;

	std::unique_ptr<uint8_t[]>     m_buckrog_bitmap_ram;
	virtual void machine_start() override { m_digits.resolve(); m_lamp.resolve(); }

	/* machine states */
	uint8_t       m_i8279_scanlines;
	uint8_t       m_alt_spriteram[0x80];

	/* sound state */
	uint8_t       m_turbo_osel;
	uint8_t       m_turbo_bsel;
	uint8_t       m_sound_state[3];

	/* video state */
	tilemap_t * m_fg_tilemap;

	/* Turbo-specific states */
	uint8_t       m_turbo_opa;
	uint8_t       m_turbo_opb;
	uint8_t       m_turbo_opc;
	uint8_t       m_turbo_ipa;
	uint8_t       m_turbo_ipb;
	uint8_t       m_turbo_ipc;
	uint8_t       m_turbo_fbpla;
	uint8_t       m_turbo_fbcol;
	uint8_t       m_turbo_speed;
	uint8_t       m_turbo_collision;
	uint8_t       m_turbo_last_analog;
	uint8_t       m_turbo_accel;

	/* Subroc-specific states */
	uint8_t       m_subroc3d_col;
	uint8_t       m_subroc3d_ply;
	uint8_t       m_subroc3d_flip;
	uint8_t       m_subroc3d_mdis;
	uint8_t       m_subroc3d_mdir;
	uint8_t       m_subroc3d_tdis;
	uint8_t       m_subroc3d_tdir;
	uint8_t       m_subroc3d_fdis;
	uint8_t       m_subroc3d_fdir;
	uint8_t       m_subroc3d_hdis;
	uint8_t       m_subroc3d_hdir;

	/* Buck Rogers-specific states */
	uint8_t       m_buckrog_fchg;
	uint8_t       m_buckrog_mov;
	uint8_t       m_buckrog_obch;
	uint8_t       m_buckrog_command;
	uint8_t       m_buckrog_myship;
	int m_last_sound_a;

	struct sprite_info
	{
		uint16_t  ve;                 /* VE0-15 signals for this row */
		uint8_t   lst;                /* LST0-7 signals for this row */
		uint32_t  latched[8];         /* latched pixel data */
		uint8_t   plb[8];             /* latched PLB state */
		uint32_t  offset[8];          /* current offset for this row */
		uint32_t  frac[8];            /* leftover fraction */
		uint32_t  step[8];            /* stepping value */
	};

	DECLARE_WRITE8_MEMBER(scanlines_w);
	DECLARE_WRITE8_MEMBER(digit_w);
	DECLARE_READ8_MEMBER(turbo_collision_r);
	DECLARE_WRITE8_MEMBER(turbo_collision_clear_w);
	DECLARE_WRITE8_MEMBER(turbo_analog_reset_w);
	DECLARE_WRITE_LINE_MEMBER(coin_meter_1_w);
	DECLARE_WRITE_LINE_MEMBER(coin_meter_2_w);
	DECLARE_WRITE_LINE_MEMBER(start_lamp_w);
	DECLARE_READ8_MEMBER(buckrog_cpu2_command_r);
	DECLARE_READ8_MEMBER(buckrog_port_2_r);
	DECLARE_READ8_MEMBER(buckrog_port_3_r);
	DECLARE_WRITE8_MEMBER(turbo_videoram_w);
	DECLARE_WRITE8_MEMBER(buckrog_bitmap_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi0a_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi0b_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi0c_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi1a_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi1b_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi1c_w);
	DECLARE_WRITE8_MEMBER(turbo_ppi3c_w);
	DECLARE_WRITE8_MEMBER(subroc3d_ppi0a_w);
	DECLARE_WRITE8_MEMBER(subroc3d_ppi0c_w);
	DECLARE_WRITE8_MEMBER(subroc3d_ppi0b_w);
	DECLARE_WRITE8_MEMBER(buckrog_ppi0a_w);
	DECLARE_WRITE8_MEMBER(buckrog_ppi0b_w);
	DECLARE_WRITE8_MEMBER(buckrog_ppi0c_w);
	DECLARE_WRITE8_MEMBER(buckrog_ppi1c_w);
	DECLARE_READ8_MEMBER(turbo_analog_r);
	DECLARE_WRITE8_MEMBER(buckrog_i8255_0_w);
	DECLARE_READ8_MEMBER(spriteram_r);
	DECLARE_WRITE8_MEMBER(spriteram_w);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	DECLARE_VIDEO_START(turbo);
	void turbo_palette(palette_device &palette) const;
	void subroc3d_palette(palette_device &palette) const;
	DECLARE_MACHINE_RESET(buckrog);
	DECLARE_VIDEO_START(buckrog);
	void buckrog_palette(palette_device &palette) const;
	uint32_t screen_update_turbo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_subroc3d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_buckrog(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(delayed_i8255_w);
	DECLARE_WRITE8_MEMBER(turbo_sound_a_w);
	DECLARE_WRITE8_MEMBER(turbo_sound_b_w);
	DECLARE_WRITE8_MEMBER(turbo_sound_c_w);
	DECLARE_WRITE8_MEMBER(subroc3d_sound_a_w);
	DECLARE_WRITE8_MEMBER(subroc3d_sound_b_w);
	DECLARE_WRITE8_MEMBER(subroc3d_sound_c_w);
	DECLARE_WRITE8_MEMBER(buckrog_sound_a_w);
	DECLARE_WRITE8_MEMBER(buckrog_sound_b_w);
	inline uint32_t sprite_xscale(uint8_t dacinput, double vr1, double vr2, double cext);
	void turbo_prepare_sprites(uint8_t y, sprite_info *info);
	uint32_t turbo_get_sprite_bits(uint8_t road, sprite_info *sprinfo);
	void subroc3d_prepare_sprites(uint8_t y, sprite_info *info);
	uint32_t subroc3d_get_sprite_bits(sprite_info *sprinfo, uint8_t *plb);
	void buckrog_prepare_sprites(uint8_t y, sprite_info *info);
	uint32_t buckrog_get_sprite_bits(sprite_info *sprinfo, uint8_t *plb);
	void turbo_rom_decode();
	void turbo_update_samples();
	inline void subroc3d_update_volume(int leftchan, uint8_t dis, uint8_t dir);
	void buckrog_update_samples();

	void buckrog_cpu2_map(address_map &map);
	void buckrog_cpu2_portmap(address_map &map);
	void buckrog_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
	void subroc3d_map(address_map &map);
	void turbo_map(address_map &map);
};

#endif // MAME_INCLUDES_TURBO_H
