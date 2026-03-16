// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia
/*************************************************************************

    kan_pand.h

    Implementation of Kaneko Pandora sprite chip

**************************************************************************/
#ifndef MAME_KANEKO_KAN_PAND_H
#define MAME_KANEKO_KAN_PAND_H

#pragma once

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class kaneko_pandora_device : public device_t,
								public device_video_interface,
								public device_gfx_interface
{
public:
	// constructor/destructor
	kaneko_pandora_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	template <typename T> kaneko_pandora_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: kaneko_pandora_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	void set_gfxinfo(const gfx_decode_entry *gfxinfo) { set_info(gfxinfo); }
	void set_offsets(s32 x_offset, s32 y_offset)
	{
		m_xoffset = x_offset;
		m_yoffset = y_offset;
	}

	void spriteram_w(offs_t offset, u8 data);
	u8 spriteram_r(offs_t offset);
	void spriteram_lsb_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 spriteram_lsb_r(offs_t offset);
	void update(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_clear_bitmap(int clear);
	void eof();
	void set_bg_pen(u16 pen);
	void flip_screen_set(bool flip) { m_flip_screen = flip; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void draw(bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	// internal state
	std::unique_ptr<u8[]> m_spriteram;
	bitmap_ind16          m_sprites_bitmap[2]; // bitmap to render sprites to, Pandora seems to be frame'buffered'
	u8                    m_buffer;
	bool                  m_clear_bitmap;
	u16                   m_bg_pen; // might work some other way..
	s32                   m_xoffset;
	s32                   m_yoffset;
	bool                  m_flip_screen;
};

DECLARE_DEVICE_TYPE(KANEKO_PANDORA, kaneko_pandora_device)

#endif // MAME_KANEKO_KAN_PAND_H
