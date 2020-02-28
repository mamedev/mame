// license:BSD-3-Clause
// copyright-holders:Brad Oliver
#ifndef MAME_INCLUDES_TANKBATT_H
#define MAME_INCLUDES_TANKBATT_H

#pragma once

#include "sound/samples.h"
#include "emupal.h"
#include "tilemap.h"

class tankbatt_state : public driver_device
{
public:
	tankbatt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bulletsram(*this, "bulletsram"),
		m_videoram(*this, "videoram")
	{ }

	void tankbatt(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_bulletsram;
	required_shared_ptr<uint8_t> m_videoram;

	int m_nmi_enable;
	int m_sound_enable;
	tilemap_t *m_bg_tilemap;

	DECLARE_READ8_MEMBER(in0_r);
	DECLARE_READ8_MEMBER(in1_r);
	DECLARE_READ8_MEMBER(dsw_r);
	DECLARE_WRITE_LINE_MEMBER(interrupt_enable_w);
	DECLARE_WRITE_LINE_MEMBER(demo_interrupt_enable_w);
	DECLARE_WRITE_LINE_MEMBER(sh_expl_w);
	DECLARE_WRITE_LINE_MEMBER(sh_engine_w);
	DECLARE_WRITE_LINE_MEMBER(sh_fire_w);
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_WRITE_LINE_MEMBER(coincounter_w);
	DECLARE_WRITE_LINE_MEMBER(coinlockout_w);
	DECLARE_WRITE8_MEMBER(videoram_w);

	INTERRUPT_GEN_MEMBER(interrupt);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override;
	virtual void video_start() override;
	void tankbatt_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bullets(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_TANKBATT_H
