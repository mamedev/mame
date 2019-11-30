// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_HEXION_H
#define MAME_INCLUDES_HEXION_H

#pragma once

#include "machine/k053252.h"
#include "machine/timer.h"
#include "emupal.h"
#include "tilemap.h"

class hexion_state : public driver_device
{
public:
	hexion_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_k053252(*this, "k053252"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void hexion(machine_config &config);
	void hexionb(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<k053252_device> m_k053252;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t *m_vram[2];
	uint8_t *m_unkram;
	int m_bankctrl;
	int m_rambank;
	int m_pmcbank;
	int m_gfxrom_select;
	int m_ccu_int_time;
	int m_ccu_int_time_count;
	tilemap_t *m_bg_tilemap[2];

	DECLARE_WRITE8_MEMBER(coincntr_w);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_READ8_MEMBER(bankedram_r);
	DECLARE_WRITE8_MEMBER(bankedram_w);
	DECLARE_WRITE8_MEMBER(bankctrl_w);
	DECLARE_WRITE8_MEMBER(gfxrom_select_w);
	DECLARE_WRITE_LINE_MEMBER(irq_ack_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_ack_w);
	DECLARE_WRITE8_MEMBER(ccu_int_time_w);

	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tile_info(tile_data &tileinfo,int tile_index,uint8_t *ram);
	void hexion_map(address_map &map);
	void hexionb_map(address_map &map);
};

#endif // MAME_INCLUDES_HEXION_H
