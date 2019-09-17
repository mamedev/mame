// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Jarek Parchanski, Nicola Salmoria, hap
/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/
#ifndef MAME_INCLUDES_CHAMPBAS_H
#define MAME_INCLUDES_CHAMPBAS_H

#pragma once

#include "machine/74259.h"
#include "machine/alpha8201.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "emupal.h"
#include "tilemap.h"


class champbas_state : public driver_device
{
public:
	champbas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_alpha_8201(*this, "alpha_8201"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2")
	{
	}

	DECLARE_READ_LINE_MEMBER(watchdog_bit2);

	void init_champbas();

	void champbas(machine_config &config);
	void champbb2(machine_config &config);
	void champbb2j(machine_config &config);
	void talbot(machine_config &config);
	void tbasebal(machine_config &config);
	void champbasjb(machine_config &config);
	void champbasj(machine_config &config);
	void champbasja(machine_config &config);

protected:
	// handlers
	DECLARE_WRITE_LINE_MEMBER(irq_enable_w);
	DECLARE_WRITE_LINE_MEMBER(mcu_switch_w);
	DECLARE_WRITE_LINE_MEMBER(mcu_start_w);
	DECLARE_READ8_MEMBER(champbja_protection_r);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	DECLARE_WRITE8_MEMBER(tilemap_w);
	DECLARE_WRITE_LINE_MEMBER(gfxbank_w);
	DECLARE_WRITE_LINE_MEMBER(palette_bank_w);
	DECLARE_WRITE_LINE_MEMBER(flipscreen_w);

	void champbas_palette(palette_device &palette) const;
	TILE_GET_INFO_MEMBER(champbas_get_bg_tile_info);

	uint32_t screen_update_champbas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void champbas_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void video_start() override;

	void champbas_map(address_map &map);
	void champbasj_map(address_map &map);
	void champbasja_map(address_map &map);
	void champbasjb_map(address_map &map);
	void champbb2_map(address_map &map);
	void champbb2j_map(address_map &map);
	void tbasebal_map(address_map &map);
	void champbas_sound_map(address_map &map);

	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_mainlatch;
	optional_device<alpha_8201_device> m_alpha_8201;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_spriteram2;

	// internal state
	uint8_t m_irq_mask;
	tilemap_t *m_bg_tilemap;
	uint8_t m_gfx_bank;
	uint8_t m_palette_bank;
};

class exctsccr_state : public champbas_state
{
public:
	exctsccr_state(const machine_config &mconfig, device_type type, const char *tag) :
		champbas_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu")
	{
	}

	void init_exctsccr();

	void exctsccr(machine_config &config);
	void exctsccrb(machine_config &config);

protected:
	TIMER_DEVICE_CALLBACK_MEMBER(exctsccr_sound_irq);

	void exctsccr_palette(palette_device &palette) const;
	TILE_GET_INFO_MEMBER(exctsccr_get_bg_tile_info);

	uint32_t screen_update_exctsccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void exctsccr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void video_start() override;

	void exctsccr_map(address_map &map);
	void exctsccrb_map(address_map &map);
	void exctsccr_sound_map(address_map &map);
	void exctsccr_sound_io_map(address_map &map);

private:
	required_device<cpu_device> m_audiocpu;
};

#endif // MAME_INCLUDES_CHAMPBAS_H
