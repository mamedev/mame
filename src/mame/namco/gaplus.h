// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Ernesto Corvi, Nicola Salmoria
#ifndef MAME_NAMCO_GAPLUS_H
#define MAME_NAMCO_GAPLUS_H

#pragma once

#include "sound/namco.h"
#include "sound/samples.h"
#include "namcoio.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class gaplus_base_state : public driver_device
{
public:
	static constexpr unsigned MAX_STARS = 250;

	struct star {
		float x = 0, y = 0;
		int col = 0, set = 0;
	};

	gaplus_base_state(const machine_config &mconfig, device_type type, const char *tag, const char *namco56xx_tag, const char *namco58xx_tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_subcpu2(*this, "sub2")
		, m_namco56xx(*this, namco56xx_tag)
		, m_namco58xx(*this, namco58xx_tag)
		, m_namco_15xx(*this, "namco")
		, m_samples(*this, "samples")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_proms_region(*this, "proms")
		, m_customio_3(*this, "customio_3")
		, m_videoram(*this, "videoram")
		, m_spriteram(*this, "spriteram")
		, m_gfx1_region(*this, "gfx1")
		, m_gfx2_region(*this, "gfx2")
	{ }

	void irq_1_ctrl_w(offs_t offset, uint8_t data);
	void irq_2_ctrl_w(offs_t offset, uint8_t data);
	void irq_3_ctrl_w(offs_t offset, uint8_t data);
	void sreset_w(offs_t offset, uint8_t data);
	void freset_w(offs_t offset, uint8_t data);
	void customio_3_w(offs_t offset, uint8_t data);
	uint8_t customio_3_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	void starfield_control_w(offs_t offset, uint8_t data);

	void gaplus_palette(palette_device &palette) const;

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);

	void vblank_irq(int state);
	TIMER_CALLBACK_MEMBER(namcoio0_run);
	TIMER_CALLBACK_MEMBER(namcoio1_run);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	void starfield_init();
	void starfield_render(bitmap_ind16 &bitmap);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect ) const;

	void gaplus_base(machine_config &config);
	void cpu1_map(address_map &map) ATTR_COLD;
	void cpu2_map(address_map &map) ATTR_COLD;
	void cpu3_map(address_map &map) ATTR_COLD;

	void driver_init();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	required_device<namco56xx_device> m_namco56xx;
	required_device<namco58xx_device> m_namco58xx;
	required_device<namco_15xx_device> m_namco_15xx;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_memory_region m_proms_region;

	required_shared_ptr<uint8_t> m_customio_3;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_memory_region m_gfx1_region;
	required_memory_region m_gfx2_region;

	int m_type = 0;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_starfield_control[4]{};
	int m_total_stars = 0;
	int m_starfield_framecount = 0;
	struct star m_stars[MAX_STARS];
	uint8_t m_main_irq_mask = 0;
	uint8_t m_sub_irq_mask = 0;
	uint8_t m_sub2_irq_mask = 0;
	emu_timer *m_namcoio0_run_timer = nullptr;
	emu_timer *m_namcoio1_run_timer = nullptr;
};

class gaplusd_state : public gaplus_base_state
{
public:
	gaplusd_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaplus_base_state(mconfig, type, tag, "namcoio_2", "namcoio_1")
	{
	}

	void gaplusd(machine_config &config);
};

class gapluso_state : public gaplus_base_state {
public:
	gapluso_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaplus_base_state(mconfig, type, tag, "namcoio_1", "namcoio_2") {
	}

	void gapluso(machine_config &config);

protected:
	void vblank_irq(int state);
};

class gaplus_state : public gaplus_base_state {
public:
	gaplus_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaplus_base_state(mconfig, type, tag, "namcoio_1", "namcoio_2")
		, m_lamps(*this, "lamp%u", 0U)
	{
	}

	void gaplus(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	void out_lamps0(uint8_t data);
	void out_lamps1(uint8_t data);

	output_finder<2> m_lamps;
};

#endif // MAME_NAMCO_GAPLUS_H
