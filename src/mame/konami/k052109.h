// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_KONAMI_K052109_H
#define MAME_KONAMI_K052109_H

#pragma once

#include "tilemap.h"

#define K052109_CB_MEMBER(_name)   void _name(int layer, int bank, int *code, int *color, int *flags, int *priority)


class k052109_device : public device_t, public device_gfx_interface, public device_video_interface
{
	static const gfx_layout charlayout;
	static const gfx_layout charlayout_ram;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_ram);

public:
	using tile_delegate = device_delegate<void (int layer, int bank, int *code, int *color, int *flags, int *priority)>;

	k052109_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k052109_device() {}

	auto irq_handler() { return m_irq_handler.bind(); }
	auto firq_handler() { return m_firq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }
	template <typename... T> void set_tile_callback(T &&... args) { m_k052109_cb.set(std::forward<T>(args)...); }
	void set_char_ram(bool ram);

	/*
	The callback is passed:
	- layer number (0 = FIX, 1 = A, 2 = B)
	- bank (range 0-3, output of the pins CAB1 and CAB2)
	- code (range 00-FF, output of the pins VC3-VC10)
	NOTE: code is in the range 0000-FFFF for X-Men, which uses extra RAM
	- color (range 00-FF, output of the pins COL0-COL7)
	The callback must put:
	- in code the resulting tile number
	- in color the resulting color index
	- if necessary, put flags and/or priority for the TileMap code in the tile_info
	structure (e.g. TILE_FLIPX). Note that TILE_FLIPY is handled internally by the
	chip so it must not be set by the callback.
	*/

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void set_rmrd_line(int state);
	int get_rmrd_line();
	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, uint32_t flags, uint8_t priority);

	void vblank_callback(screen_device &screen, bool state);

	void set_xy_offset(int dx, int dy);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	// internal state
	std::unique_ptr<uint8_t[]>   m_ram;
	uint8_t    *m_videoram_F;
	uint8_t    *m_videoram_A;
	uint8_t    *m_videoram_B;
	uint8_t    *m_videoram2_F;
	uint8_t    *m_videoram2_A;
	uint8_t    *m_videoram2_B;
	uint8_t    *m_colorram_F;
	uint8_t    *m_colorram_A;
	uint8_t    *m_colorram_B;

	tilemap_t  *m_tilemap[3];
	int      m_tileflip_enable;
	uint8_t    m_charrombank[4];
	uint8_t    m_charrombank_2[4];
	uint8_t    m_has_extra_video_ram;
	int32_t    m_rmrd_line;
	uint8_t    m_irq_enabled;
	uint8_t    m_romsubbank, m_scrollctrl, m_addrmap;

	int        m_dx, m_dy;

	optional_region_ptr<uint8_t> m_char_rom;

	tile_delegate m_k052109_cb;

	devcb_write_line m_irq_handler;
	devcb_write_line m_firq_handler;
	devcb_write_line m_nmi_handler;

	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	void get_tile_info( tile_data &tileinfo, int tile_index, int layer, uint8_t *cram, uint8_t *vram1, uint8_t *vram2 );
	void tileflip_reset();
};

DECLARE_DEVICE_TYPE(K052109, k052109_device)

#endif // MAME_KONAMI_K052109_H
