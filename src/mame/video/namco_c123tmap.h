// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_VIDEO_NAMCO_C123TMAP_H
#define MAME_VIDEO_NAMCO_C123TMAP_H

#pragma once

#include "screen.h"
#include "emupal.h"

class namco_c123tmap_device : public device_t
{
public:
	// construction/destruction
	namco_c123tmap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }

	DECLARE_WRITE16_MEMBER(videoram_w);
	DECLARE_READ16_MEMBER(videoram_r);
	DECLARE_WRITE16_MEMBER(control_w);
	DECLARE_READ16_MEMBER(control_r);

	typedef delegate<void(uint16_t, int*, int*)> c123_tilemap_delegate;

	void init(int gfxbank, void *pMaskROM, c123_tilemap_delegate tilemap_cb);
	void mark_all_dirty(void);
	void draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	template<int Offset> TILE_GET_INFO_MEMBER(get_tile_info);
	void set_tilemap_videoram(int offset, uint16_t newword);
	void set_tilemap_control(int offset, uint16_t newword);

	struct info
	{
		uint16_t control[0x40 / 2];
		/**
		 * [0x1] 0x02/2 tilemap#0.scrollx
		 * [0x3] 0x06/2 tilemap#0.scrolly
		 * [0x5] 0x0a/2 tilemap#1.scrollx
		 * [0x7] 0x0e/2 tilemap#1.scrolly
		 * [0x9] 0x12/2 tilemap#2.scrollx
		 * [0xb] 0x16/2 tilemap#2.scrolly
		 * [0xd] 0x1a/2 tilemap#3.scrollx
		 * [0xf] 0x1e/2 tilemap#3.scrolly
		 * 0x20/2 priority
		 * 0x30/2 color
		 */
		tilemap_t *tmap[6];
		std::unique_ptr<uint16_t[]> videoram;
		int gfxbank;
		uint8_t *maskBaseAddr;
		c123_tilemap_delegate cb;
	};

	info m_tilemapinfo;

	required_device<gfxdecode_device> m_gfxdecode;
};

// device type definition
DECLARE_DEVICE_TYPE(NAMCO_C123TMAP, namco_c123tmap_device)

#endif // MAME_VIDEO_NAMCO_C123TMAP_H

