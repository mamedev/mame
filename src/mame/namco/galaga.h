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


class galaga_state_base : public driver_device
{
public:
	galaga_state_base(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_subcpu2(*this, "sub2")
		, m_namco_sound(*this, "namco")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_io_dsw(*this, "DSW%c", 'A')
		, m_leds(*this, "led%u", 0U)
	{
	}

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t bosco_dsw_r(offs_t offset);
	void irq1_clear_w(int state);
	void irq2_clear_w(int state);
	void out(uint8_t data);
	void lockout(int state);
	void nmion_w(int state);
	void vblank_irq(int state);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	required_device<namco_wsg_device> m_namco_sound;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport_array<2> m_io_dsw;
	output_finder<2> m_leds;

private:
	TIMER_CALLBACK_MEMBER(cpu3_interrupt_callback);

	emu_timer *m_cpu3_interrupt_timer = nullptr;

	uint8_t m_main_irq_mask = 0;
	uint8_t m_sub_irq_mask = 0;
	uint8_t m_sub2_nmi_mask = 0;
};


class galaga_state : public galaga_state_base
{
public:
	galaga_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaga_state_base(mconfig, type, tag)
		, m_videoram(*this, "videoram")
		, m_galaga_ram1(*this, "galaga_ram1")
		, m_galaga_ram2(*this, "galaga_ram2")
		, m_galaga_ram3(*this, "galaga_ram3")
		, m_videolatch(*this, "videolatch")
		, m_starfield(*this, "starfield")
	{
	}

	void galaga(machine_config &config) ATTR_COLD;
	void gatsbee(machine_config &config) ATTR_COLD;
	void galagab(machine_config &config) ATTR_COLD;

	void init_galaga() ATTR_COLD;
	void init_gatsbee() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

	void videoram_w(offs_t offset, uint8_t data);
	void gatsbee_bank_w(int state);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void galaga_palette(palette_device &palette) const;
	uint32_t screen_update_galaga(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_galaga(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint16_t get_next_lfsr_state(uint16_t lfsr);
	void draw_stars(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void galaga_map(address_map &map) ATTR_COLD;
	void galaga_mem4(address_map &map) ATTR_COLD;
	void gatsbee_main_map(address_map &map) ATTR_COLD;

	/* memory pointers, devices */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_galaga_ram1;
	required_shared_ptr<uint8_t> m_galaga_ram2;
	required_shared_ptr<uint8_t> m_galaga_ram3;
	required_device<ls259_device> m_videolatch;
	required_device<starfield_05xx_device> m_starfield;

	tilemap_t *m_fg_tilemap = nullptr;

	uint32_t m_gfxbank = 0; // used by gatsbee
};

extern const gfx_layout spritelayout_galaga;

DISCRETE_SOUND_EXTERN( galaga_discrete );
DISCRETE_SOUND_EXTERN( bosco_discrete );

#endif // MAME_NAMCO_GALAGA_H
