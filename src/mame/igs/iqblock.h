// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Ernesto Corvi
#ifndef MAME_INCLUDES_IQBLOCK_H
#define MAME_INCLUDES_IQBLOCK_H

#pragma once

#include "machine/timer.h"
#include "tilemap.h"

class iqblock_state : public driver_device
{
public:
	iqblock_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_rambase(*this, "rambase"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram")
	{ }

	void iqblock(machine_config &config);

	void init_grndtour();
	void init_iqblock();

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_rambase;
	required_shared_ptr<uint8_t> m_bgvideoram;
	required_shared_ptr<uint8_t> m_fgvideoram;

	int m_videoenable = 0;
	int m_video_type = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	void iqblock_prot_w(uint8_t data);
	void grndtour_prot_w(uint8_t data);
	void irqack_w(uint8_t data);
	void fgvideoram_w(offs_t offset, uint8_t data);
	void bgvideoram_w(offs_t offset, uint8_t data);
	void fgscroll_w(offs_t offset, uint8_t data);
	void port_C_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(irq);

	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map);
	void main_portmap(address_map &map);
};

#endif // MAME_INCLUDES_IQBLOCK_H
