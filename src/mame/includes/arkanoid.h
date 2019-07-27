// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Stephane Humbert
#ifndef MAME_INCLUDES_ARKANOID_H
#define MAME_INCLUDES_ARKANOID_H

#pragma once

#include "machine/taito68705interface.h"
#include "emupal.h"

/* This it the best way to allow game specific kludges until the system is fully understood */
enum {
	ARKUNK = 0,  /* unknown bootlegs for inclusion of possible new sets */
	ARKANGC,
	ARKANGC2,
	BLOCK2,
	ARKBLOCK,
	ARKBLOC2,
	ARKGCBL,
	PADDLE2
};

class arkanoid_state : public driver_device
{
public:
	arkanoid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_spriteram(*this, "spriteram")
		, m_protram(*this, "protram")
		, m_muxports(*this, "P%u", 1)
		, m_maincpu(*this, "maincpu")
		, m_mcuintf(*this, "mcu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{
	}

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_protram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	uint8_t    m_gfxbank;
	uint8_t    m_palettebank;

	/* input-related */
	uint8_t    m_paddle_select;   // selected by d008 bit 2

	/* bootleg related */
	int      m_bootleg_id;
	uint8_t    m_bootleg_cmd;

	/* hexaa */
	uint8_t m_hexaa_from_main;
	uint8_t m_hexaa_from_sub;

	/* devices */
	optional_ioport_array<2> m_muxports;
	required_device<cpu_device> m_maincpu;
	optional_device<arkanoid_mcu_device_base> m_mcuintf;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;


	DECLARE_READ8_MEMBER(arkanoid_bootleg_f000_r);
	DECLARE_READ8_MEMBER(arkanoid_bootleg_f002_r);
	DECLARE_WRITE8_MEMBER(arkanoid_bootleg_d018_w);
	DECLARE_READ8_MEMBER(arkanoid_bootleg_d008_r);
	DECLARE_WRITE8_MEMBER(arkanoid_videoram_w);
	DECLARE_WRITE8_MEMBER(arkanoid_d008_w);
	DECLARE_WRITE8_MEMBER(tetrsark_d008_w);
	DECLARE_WRITE8_MEMBER(brixian_d008_w);
	DECLARE_WRITE8_MEMBER(hexa_d008_w);
	DECLARE_READ8_MEMBER(hexaa_f000_r);
	DECLARE_WRITE8_MEMBER(hexaa_f000_w);
	DECLARE_WRITE8_MEMBER(hexaa_sub_80_w);
	DECLARE_READ8_MEMBER(hexaa_sub_90_r);
	DECLARE_CUSTOM_INPUT_MEMBER(arkanoid_semaphore_input_r);
	DECLARE_CUSTOM_INPUT_MEMBER(arkanoid_input_mux);
	void init_block2();
	void init_arkblock();
	void init_hexa();
	void init_hexaa();
	void init_paddle2();
	void init_tetrsark();
	void init_tetrsark2();
	void init_arkgcbl();
	void init_arkangc2();
	void init_arkbloc2();
	void init_arkangc();
	void init_brixian();
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_arkanoid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hexa(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void arkanoid_bootleg_init(  );

	void bootleg(machine_config &config);
	void p3mcuay(machine_config &config);
	void aysnd(machine_config &config);
	void hexa(machine_config &config);
	void brixian(machine_config &config);
	void hexaa(machine_config &config);
	void p3mcu(machine_config &config);
	void arkanoid(machine_config &config);
	void arkanoid_map(address_map &map);
	void bootleg_map(address_map &map);
	void brixian_map(address_map &map);
	void hexa_map(address_map &map);
	void hexaa_map(address_map &map);
	void hexaa_sub_iomap(address_map &map);
	void hexaa_sub_map(address_map &map);
};

#endif // MAME_INCLUDES_ARKANOID_H
