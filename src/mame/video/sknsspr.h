// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_SKNSSPR_H
#define MAME_VIDEO_SKNSSPR_H

#pragma once

class sknsspr_device : public device_t, public device_video_interface, public device_rom_interface
{
public:
	sknsspr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void skns_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t* spriteram_source, size_t spriteram_size, uint32_t* sprite_regs);
	void skns_sprite_kludge(int x, int y);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void rom_bank_updated() override;
private:
	static constexpr unsigned SUPRNOVA_DECODE_BUFFER_SIZE = 0x2000;
	int m_sprite_kludge_x, m_sprite_kludge_y;
	std::unique_ptr<uint8_t[]> m_decodebuffer;
	int skns_rle_decode ( int romoffset, int size );
};

DECLARE_DEVICE_TYPE(SKNS_SPRITE, sknsspr_device)

#endif // MAME_VIDEO_SKNSSPR_H
