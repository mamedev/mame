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

protected:
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<k053252_device> m_k053252;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t *m_vram[2]{};
	uint8_t *m_unkram = nullptr;
	int m_bankctrl = 0;
	int m_rambank = 0;
	int m_pmcbank = 0;
	int m_gfxrom_select = 0;
	int m_ccu_int_time = 0;
	int m_ccu_int_time_count = 0;
	tilemap_t *m_bg_tilemap[2]{};

	void coincntr_w(uint8_t data);
	void bankswitch_w(uint8_t data);
	uint8_t bankedram_r(offs_t offset);
	void bankedram_w(offs_t offset, uint8_t data);
	void bankctrl_w(uint8_t data);
	void gfxrom_select_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(irq_ack_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_ack_w);
	void ccu_int_time_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tile_info(tile_data &tileinfo,int tile_index,uint8_t *ram);
	void hexion_map(address_map &map);
	void hexionb_map(address_map &map);
};

#endif // MAME_INCLUDES_HEXION_H
