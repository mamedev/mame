// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_KANEKO_KANEKO_RLESPR_H
#define MAME_KANEKO_KANEKO_RLESPR_H

#pragma once

#include "dirom.h"

 // TODO : Unknown address bits; maybe 27?
class kaneko_rle_sprites_device : public device_t, public device_video_interface, public device_rom_interface<27>
{
public:
	kaneko_rle_sprites_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configurations
	void set_sprite_kludge(int x, int y);

	void draw_sprites(const rectangle &cliprect, u32* spriteram_source, size_t spriteram_size, u32* sprite_regs);

	// getters
	bitmap_ind16 &bitmap() { return m_sprite_bitmap[m_buffer]; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr unsigned DECODE_BUFFER_SIZE = 0x2000;
	static constexpr unsigned DECODE_BUFFER_MASK = DECODE_BUFFER_SIZE - 1;
	static constexpr size_t ROM_ADDRESS_MASK = (1 << 27) - 1;

	int m_sprite_kludge_x, m_sprite_kludge_y;
	std::unique_ptr<u8[]> m_decodebuffer;
	bitmap_ind16 m_sprite_bitmap[2];
	u8 m_buffer;

	int rle_decode(int romoffset, int size);
};

DECLARE_DEVICE_TYPE(KANEKO_RLE_SPRITES, kaneko_rle_sprites_device)

#endif // MAME_KANEKO_KANEKO_RLESPR_H
