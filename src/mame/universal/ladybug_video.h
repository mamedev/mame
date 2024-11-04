// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_UNIVERSAL_LADYBUG_H
#define MAME_UNIVERSAL_LADYBUG_H

#pragma once

#include "screen.h"
#include "tilemap.h"


// used by ladybug and sraider
class ladybug_video_device : public device_t
{
public:
	ladybug_video_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }

	u8 spr_r(offs_t offset) { return m_spr_ram[offset & 0x03ff]; }
	void spr_w(offs_t offset, u8 data) { m_spr_ram[offset & 0x03ff] = data; }
	u8 bg_r(offs_t offset) { return m_bg_ram[offset & 0x07ff]; }
	void bg_w(offs_t offset, u8 data);

	void draw(screen_device &screen, bitmap_ind16 &bitmap, rectangle const &cliprect, bool flip);

protected:
	virtual void device_start() override ATTR_COLD;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

private:
	void draw_sprites(bitmap_ind16 &bitmap, rectangle const &cliprect);

	required_device<gfxdecode_device>   m_gfxdecode;
	std::unique_ptr<u8 []>              m_spr_ram;
	std::unique_ptr<u8 []>              m_bg_ram;
	tilemap_t                           *m_bg_tilemap;
};


DECLARE_DEVICE_TYPE(LADYBUG_VIDEO, ladybug_video_device)

#endif // MAME_UNIVERSAL_LADYBUG_H
