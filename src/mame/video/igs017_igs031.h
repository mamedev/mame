// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Luca Elia
#ifndef MAME_VIDEO_IGS017_IGS031_H
#define MAME_VIDEO_IGS017_IGS031_H

#pragma once

#include "machine/i8255.h"

typedef device_delegate<uint16_t (uint16_t)> igs017_igs031_palette_scramble_delegate;

#define MCFG_PALETTE_SCRAMBLE_CB( _class, _method) \
	igs017_igs031_device::set_palette_scramble_cb(*device, igs017_igs031_palette_scramble_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));

#define MCFG_REVERSE_TEXT_BITS \
	igs017_igs031_device::static_set_text_reverse_bits(*device);

class igs017_igs031_device : public device_t,
							public device_gfx_interface,
							public device_video_interface,
							public device_memory_interface
{
public:
	igs017_igs031_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


	static void set_palette_scramble_cb(device_t &device,igs017_igs031_palette_scramble_delegate newtilecb);

	static void static_set_text_reverse_bits(device_t &device)
	{
		igs017_igs031_device &dev = downcast<igs017_igs031_device &>(device);
		dev.m_revbits = 1;
	}

	uint16_t palette_callback_straight(uint16_t bgr);

	igs017_igs031_palette_scramble_delegate m_palette_scramble_cb;

	void map(address_map &map);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	int get_nmi_enable() { return m_nmi_enable; }
	int get_irq_enable() { return m_irq_enable; }


	DECLARE_WRITE8_MEMBER(palram_w);
	DECLARE_READ8_MEMBER(i8255_r);

	DECLARE_WRITE8_MEMBER(video_disable_w);


	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	DECLARE_WRITE8_MEMBER(fg_w);
	DECLARE_WRITE8_MEMBER(bg_w);

	void space_w(int offset, uint8_t data);
	uint8_t space_r(int offset);

	void expand_sprites();
	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int sx, int sy, int dimx, int dimy, int flipx, int flipy, int color, int addr);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int debug_viewer(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_igs017(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	virtual void video_start();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual space_config_vector memory_space_config() const override;

	DECLARE_GFXDECODE_MEMBER(gfxinfo);

private:
	address_space_config        m_space_config;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_palram;
	optional_device<i8255_device> m_i8255;
	required_device<palette_device> m_palette;

	// the gfx roms were often hooked up with the bits backwards, allow us to handle it here to save doing it in every driver.
	int m_revbits;

	int m_toggle;
	int m_debug_addr;
	int m_debug_width;
	uint8_t m_video_disable;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	std::unique_ptr<uint8_t[]> m_sprites_gfx;
	int m_sprites_gfx_size;

	int m_nmi_enable;
	int m_irq_enable;
};

DECLARE_DEVICE_TYPE(IGS017_IGS031, igs017_igs031_device)

#endif // MAME_VIDEO_IGS017_IGS031_H
