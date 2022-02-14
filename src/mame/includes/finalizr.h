// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Konami Finalizer

***************************************************************************/

#ifndef MAME_INCLUDES_FINALIZR_H
#define MAME_INCLUDES_FINALIZR_H

#pragma once

#include "cpu/mcs48/mcs48.h"
#include "machine/timer.h"

#include "emupal.h"
#include "tilemap.h"

class finalizr_state : public driver_device
{
public:
	finalizr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram%u", 1U),
		m_videoram(*this, "videoram%u", 1U),
		m_spriteram(*this, "spriteram%u", 1U)
	{ }

	void finalizr(machine_config &config);
	void finalizrb(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<mcs48_cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr_array<uint8_t, 2> m_colorram;
	required_shared_ptr_array<uint8_t, 2> m_videoram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;

	// video-related
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	uint8_t m_spriterambank;
	uint8_t m_charbank;

	// misc
	uint8_t m_nmi_enable;
	uint8_t m_irq_enable;

	void coin_w(uint8_t data);
	void flipscreen_w(uint8_t data);
	void sound_irq_w(uint8_t data);
	void sound_irqen_w(uint8_t data);
	DECLARE_READ_LINE_MEMBER(bootleg_t1_r);
	void videoctrl_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void palette(palette_device &palette) const;
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void main_map(address_map &map);
	void sound_io_map(address_map &map);
};

#endif // MAME_INCLUDES_FINALIZR_H
