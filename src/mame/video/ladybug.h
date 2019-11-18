// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_LADYBUG_H
#define MAME_VIDEO_LADYBUG_H

#pragma once

#include "screen.h"
#include "tilemap.h"


// used by ladybug and sraider
class ladybug_video_device : public device_t
{
public:
	ladybug_video_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }

	DECLARE_READ8_MEMBER(spr_r) { return m_spr_ram[offset & 0x03ff]; }
	DECLARE_WRITE8_MEMBER(spr_w) { m_spr_ram[offset & 0x03ff] = data; }
	DECLARE_READ8_MEMBER(bg_r) { return m_bg_ram[offset & 0x07ff]; }
	DECLARE_WRITE8_MEMBER(bg_w);

	void draw(screen_device &screen, bitmap_ind16 &bitmap, rectangle const &cliprect, bool flip);

protected:
	virtual void device_start() override;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

private:
	required_device<gfxdecode_device>   m_gfxdecode;
	std::unique_ptr<u8 []>              m_spr_ram;
	std::unique_ptr<u8 []>              m_bg_ram;
	tilemap_t                           *m_bg_tilemap;
};


// used by zerohour, redclash and sraider
class zerohour_stars_device : public device_t
{
public:
	zerohour_stars_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	void set_enable(bool on);
	void update_state();
	void set_speed(u8 speed, u8 mask);
	void draw(bitmap_ind16 &bitmap, rectangle const &cliprect, u8 pal_offs, bool has_va, u8 firstx, u8 lastx);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	u8  m_enable;
	u8  m_speed;
	u32 m_state;
	u16 m_offset;
	u8  m_count;
};


DECLARE_DEVICE_TYPE(LADYBUG_VIDEO, ladybug_video_device)
DECLARE_DEVICE_TYPE(ZEROHOUR_STARS, zerohour_stars_device)

#endif // MAME_VIDEO_LADYBUG_H
