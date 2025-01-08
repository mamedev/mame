// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_NAMCO_GALAGA_H
#define MAME_NAMCO_GALAGA_H

#pragma once

#include "starfield_05xx.h"
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
		, m_starfield(*this, "starfield")
		, m_galaga_gfxbank(0)
		, m_main_irq_mask(0)
		, m_sub_irq_mask(0)
		, m_sub2_nmi_mask(0)
	{ }

	uint8_t bosco_dsw_r(offs_t offset);
	void irq1_clear_w(int state);
	void irq2_clear_w(int state);
	void nmion_w(int state);
	void galaga_videoram_w(offs_t offset, uint8_t data);
	void gatsbee_bank_w(int state);
	void out(uint8_t data);
	void lockout(int state);
	uint8_t namco_52xx_rom_r(offs_t offset);
	uint8_t namco_52xx_si_r();
	void init_galaga();
	void init_gatsbee();
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void galaga_palette(palette_device &palette) const;
	uint32_t screen_update_galaga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_galaga(int state);
	void vblank_irq(int state);
	TIMER_CALLBACK_MEMBER(cpu3_interrupt_callback);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	uint16_t get_next_lfsr_state(uint16_t lfsr);
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void galaga(machine_config &config);
	void gatsbee(machine_config &config);
	void galagab(machine_config &config);
	void dzigzag_mem4(address_map &map) ATTR_COLD;
	void galaga_map(address_map &map) ATTR_COLD;
	void galaga_mem4(address_map &map) ATTR_COLD;
	void gatsbee_main_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	/* memory pointers, devices */
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
	optional_device<starfield_05xx_device> m_starfield; // not present on battles, digdug, xevious
	emu_timer *m_cpu3_interrupt_timer = nullptr;

	uint32_t m_galaga_gfxbank; // used by gatsbee

	/* shared */
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t m_main_irq_mask;
	uint8_t m_sub_irq_mask;
	uint8_t m_sub2_nmi_mask;
};

DISCRETE_SOUND_EXTERN( galaga_discrete );
DISCRETE_SOUND_EXTERN( bosco_discrete );

#endif // MAME_NAMCO_GALAGA_H
