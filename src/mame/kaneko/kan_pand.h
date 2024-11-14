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
	kaneko_pandora_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	template <typename T> kaneko_pandora_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&palette_tag, const gfx_decode_entry *gfxinfo)
		: kaneko_pandora_device(mconfig, tag, owner, clock)
	{
		set_info(gfxinfo);
		set_palette(std::forward<T>(palette_tag));
	}

	// configuration
	void set_gfxinfo(const gfx_decode_entry *gfxinfo) { set_info(gfxinfo); }
	void set_offsets(int32_t x_offset, int32_t y_offset)
	{
		m_xoffset = x_offset;
		m_yoffset = y_offset;
	}

	void spriteram_w(offs_t offset, uint8_t data);
	uint8_t spriteram_r(offs_t offset);
	void spriteram_LSB_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t spriteram_LSB_r(offs_t offset);
	void update( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void set_clear_bitmap(int clear);
	void eof();
	void set_bg_pen(uint16_t pen);
	void flip_screen_set(bool flip) { m_flip_screen = flip; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void draw( bitmap_ind16 &bitmap, const rectangle &cliprect );

private:
	// internal state
	std::unique_ptr<uint8_t[]>    m_spriteram;
	std::unique_ptr<bitmap_ind16> m_sprites_bitmap; // bitmap to render sprites to, Pandora seems to be frame'buffered'
	bool            m_clear_bitmap;
	uint16_t        m_bg_pen; // might work some other way..
	int32_t         m_xoffset;
	int32_t         m_yoffset;
	bool            m_flip_screen;
};

DECLARE_DEVICE_TYPE(KANEKO_PANDORA, kaneko_pandora_device)

#endif // MAME_KANEKO_KAN_PAND_H
