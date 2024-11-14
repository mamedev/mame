// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_KONAMI_K007342_H
#define MAME_KONAMI_K007342_H

#pragma once

#include "tilemap.h"


class k007342_device : public device_t, public device_gfx_interface
{
public:
	using tile_delegate = device_delegate<void (int layer, uint32_t bank, uint32_t &code, uint32_t &color, uint8_t &flags)>;

	k007342_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template<typename T> k007342_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: k007342_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	template <typename... T> void set_tile_callback(T &&... args) { m_callback.set(std::forward<T>(args)...); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t scroll_r(offs_t offset);
	void scroll_w(offs_t offset, uint8_t data);
	void vreg_w(offs_t offset, uint8_t data);

	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int num, int flags, uint32_t priority);
	int is_int_enabled();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	std::unique_ptr<uint8_t[]>  m_ram;
	std::unique_ptr<uint8_t[]>  m_scroll_ram;
	const uint8_t               *m_videoram[2];
	const uint8_t               *m_colorram[2];

	tilemap_t       *m_tilemap[2];
	bool            m_flipscreen, m_int_enabled;
	uint8_t         m_regs[8];
	uint16_t        m_scrollx[2];
	uint8_t         m_scrolly[2];
	tile_delegate   m_callback;

	TILEMAP_MAPPER_MEMBER(scan);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	void get_tile_info(tile_data &tileinfo, int tile_index, uint8_t layer);
};

DECLARE_DEVICE_TYPE(K007342, k007342_device)

#endif // MAME_KONAMI_K007342_H
