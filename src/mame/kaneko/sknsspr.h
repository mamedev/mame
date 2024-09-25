// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_KANEKO_SKNSSPR_H
#define MAME_KANEKO_SKNSSPR_H

#pragma once

#include "dirom.h"

 // TODO : Unknown address bits; maybe 27?
class sknsspr_device : public device_t, public device_video_interface, public device_rom_interface<27>
{
public:
	sknsspr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void skns_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, u32* spriteram_source, size_t spriteram_size, u32* sprite_regs);
	void skns_sprite_kludge(int x, int y);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr unsigned SUPRNOVA_DECODE_BUFFER_SIZE = 0x2000;
	int m_sprite_kludge_x = 0, m_sprite_kludge_y = 0;
	std::unique_ptr<u8[]> m_decodebuffer;
	int skns_rle_decode ( int romoffset, int size );
};

DECLARE_DEVICE_TYPE(SKNS_SPRITE, sknsspr_device)

#endif // MAME_KANEKO_SKNSSPR_H
