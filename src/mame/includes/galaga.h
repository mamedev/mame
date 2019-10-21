// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_GALAGA_H
#define MAME_INCLUDES_GALAGA_H

#pragma once

#include "machine/74259.h"
#include "sound/discrete.h"
#include "sound/namco.h"
#include "sound/samples.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class galaga_state : public driver_device
{
public:
	galaga_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_galaga_ram1(*this, "galaga_ram1")
		, m_galaga_ram2(*this, "galaga_ram2")
		, m_galaga_ram3(*this, "galaga_ram3")
		, m_videolatch(*this, "videolatch")
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_subcpu2(*this, "sub2")
		, m_namco_sound(*this, "namco")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_leds(*this, "led%u", 0U)
	{ }

	DECLARE_READ8_MEMBER(bosco_dsw_r);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_w);
	DECLARE_WRITE_LINE_MEMBER(irq1_clear_w);
	DECLARE_WRITE_LINE_MEMBER(irq2_clear_w);
	DECLARE_WRITE_LINE_MEMBER(nmion_w);
	DECLARE_WRITE8_MEMBER(galaga_videoram_w);
	DECLARE_WRITE_LINE_MEMBER(gatsbee_bank_w);
	DECLARE_WRITE8_MEMBER(out_0);
	DECLARE_WRITE8_MEMBER(out_1);
	DECLARE_READ8_MEMBER(namco_52xx_rom_r);
	DECLARE_READ8_MEMBER(namco_52xx_si_r);
	void init_galaga();
	void init_gatsbee();
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_VIDEO_START(galaga);
	void galaga_palette(palette_device &palette) const;
	uint32_t screen_update_galaga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_galaga);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	TIMER_CALLBACK_MEMBER(cpu3_interrupt_callback);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void galaga(machine_config &config);
	void gatsbee(machine_config &config);
	void galagab(machine_config &config);
	void dzigzag_mem4(address_map &map);
	void galaga_map(address_map &map);
	void galaga_mem4(address_map &map);
	void gatsbee_main_map(address_map &map);

	void starfield_init();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	/* memory pointers */
	optional_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_galaga_ram1;
	optional_shared_ptr<uint8_t> m_galaga_ram2;
	optional_shared_ptr<uint8_t> m_galaga_ram3;
	optional_device<ls259_device> m_videolatch; // not present on Xevious
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	required_device<namco_device> m_namco_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	output_finder<2> m_leds;
	emu_timer *m_cpu3_interrupt_timer;

	/* machine state */
	uint32_t m_stars_scrollx;
	uint32_t m_stars_scrolly;

	uint32_t m_galaga_gfxbank; // used by catsbee

	/* devices */

	/* bank support */

	/* shared */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	uint8_t m_main_irq_mask;
	uint8_t m_sub_irq_mask;
	uint8_t m_sub2_nmi_mask;

	struct star
	{
		uint16_t x,y;
		uint8_t col,set;
	};

	static star const s_star_seed_tab[];

};

DISCRETE_SOUND_EXTERN( galaga_discrete );
DISCRETE_SOUND_EXTERN( bosco_discrete );

#endif // MAME_INCLUDES_GALAGA_H
