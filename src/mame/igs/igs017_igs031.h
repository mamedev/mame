// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Luca Elia
#ifndef MAME_IGS_IGS017_IGS031_H
#define MAME_IGS_IGS017_IGS031_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

class igs017_igs031_device :
		public device_t,
		public device_gfx_interface,
		public device_video_interface,
		public device_memory_interface
{
public:
	typedef device_delegate<u16 (u16)> palette_scramble_delegate;

	igs017_igs031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto in_pa_callback() { return m_input_port_cb[0].bind(); }
	auto in_pb_callback() { return m_input_port_cb[1].bind(); }
	auto in_pc_callback() { return m_input_port_cb[2].bind(); }
	template <typename... T> void set_palette_scramble_cb(T &&... args) { m_palette_scramble_cb.set(std::forward<T>(args)...); }

	void set_text_reverse_bits(bool revbits)
	{
		m_revbits = revbits;
	}

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	bool get_nmi_enable() { return m_nmi_enable; }
	bool get_irq_enable() { return m_irq_enable; }

private:
	u16 palette_callback_straight(u16 bgr) const;

	void map(address_map &map) ATTR_COLD;

	void palram_w(offs_t offset, u8 data);
	u8 input_port_r(offs_t offset);

	void video_disable_w(u8 data);
	void nmi_enable_w(u8 data);
	void irq_enable_w(u8 data);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void fg_w(offs_t offset, u8 data);
	void bg_w(offs_t offset, u8 data);

	void expand_sprites();
	void draw_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, int offsx, int offsy, int dimx, int dimy, int flipx, int flipy, u32 color, u32 addr);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int debug_viewer(bitmap_ind16 &bitmap, const rectangle &cliprect);

public:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void video_start();

	void lhzb2_decrypt_tiles();
	void mgcs_decrypt_tiles();
	void tarzan_decrypt_tiles(int address_xor);
	void slqz2_decrypt_tiles();
	void sdwx_gfx_decrypt();

	void mgcs_flip_sprites(size_t max_size);
	void lhzb2_decrypt_sprites();
	void tarzan_decrypt_sprites(size_t max_size, size_t flip_size);
	void spkrform_decrypt_sprites();
	void starzan_decrypt_sprites(size_t max_size, size_t flip_size);
	void tjsb_decrypt_sprites();

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	DECLARE_GFXDECODE_MEMBER(gfxinfo);

private:
	palette_scramble_delegate m_palette_scramble_cb;

	address_space_config        m_space_config;

	devcb_read8::array<3> m_input_port_cb;

	required_shared_ptr<u8> m_spriteram;
	required_shared_ptr<u8> m_fg_videoram;
	required_shared_ptr<u8> m_bg_videoram;
	required_shared_ptr<u8> m_palram;
	required_device<palette_device> m_palette;

	// the gfx roms were often hooked up with the bits backwards, allow us to handle it here to save doing it in every driver.
	bool m_revbits;

	u8 m_toggle = 0;
	int m_debug_addr = 0;
	int m_debug_width = 0;
	bool m_video_disable = false;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	std::unique_ptr<u8[]> m_sprites_gfx;
	u32 m_sprites_gfx_size = 0;

	bool m_nmi_enable = false;
	bool m_irq_enable = false;
};

DECLARE_DEVICE_TYPE(IGS017_IGS031, igs017_igs031_device)

#endif // MAME_IGS_IGS017_IGS031_H
