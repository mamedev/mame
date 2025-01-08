// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_TAITO_TC0280GRD_H
#define MAME_TAITO_TC0280GRD_H

#pragma once

#include "tilemap.h"


class tc0280grd_device : public device_t, public device_gfx_interface
{
public:
	tc0280grd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	void set_color_base(u16 base) { m_colorbase = base; }

	u16 tc0280grd_word_r(offs_t offset);
	void tc0280grd_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tc0280grd_ctrl_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tc0280grd_tilemap_update(int base_color);
	void tc0280grd_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, u8 priority, u8 priority_mask = 0xff);

	u16 tc0430grw_word_r(offs_t offset);
	void tc0430grw_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tc0430grw_ctrl_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tc0430grw_tilemap_update(int base_color);
	void tc0430grw_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, u8 priority, u8 priority_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	std::unique_ptr<u16[]> m_ram;

	tilemap_t   *m_tilemap = nullptr;

	u16         m_ctrl[8];
	int         m_base_color;

	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	u16 m_colorbase;

	TILE_GET_INFO_MEMBER(get_tile_info);
	void zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, u8 priority, int xmultiply, u8 priority_mask = 0xff);
};

DECLARE_DEVICE_TYPE(TC0280GRD, tc0280grd_device)

#define TC0430GRW TC0280GRD

#endif // MAME_TAITO_TC0280GRD_H
