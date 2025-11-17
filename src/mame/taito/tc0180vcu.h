// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_TAITO_TC0180VCU_H
#define MAME_TAITO_TC0180VCU_H

#pragma once

#include "tilemap.h"


class tc0180vcu_device : public device_t, public device_gfx_interface, public device_video_interface
{
public:
	tc0180vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_fb_colorbase(int color) { m_fb_color_base = color * 16; }
	void set_bg_colorbase(int color) { m_bg_color_base = color; }
	void set_fg_colorbase(int color) { m_fg_color_base = color; }
	void set_tx_colorbase(int color) { m_tx_color_base = color; }
	auto inth_callback() { return m_inth_callback.bind(); }
	auto intl_callback() { return m_intl_callback.bind(); }

	uint8_t get_videoctrl() { return m_video_control; }
	void ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t framebuffer_word_r(offs_t offset);
	void framebuffer_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, int plane);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void draw_framebuffer( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );

	void tc0180vcu_memrw(address_map &map) ATTR_COLD;
protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_intl);

private:
	void vblank_callback(screen_device &screen, bool state);
	void vblank_update();

	// internal state

	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_vram;
	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_ctrl;
	/* framebuffer is a raw bitmap, remapped as a last step */
	bitmap_ind16 m_framebuffer[2];

	tilemap_t    *m_tilemap[3]{};

	uint16_t     m_bg_rambank[2], m_fg_rambank[2], m_tx_rambank;
	uint8_t      m_framebuffer_page;
	uint8_t      m_video_control;

	int          m_fb_color_base;
	int          m_bg_color_base;
	int          m_fg_color_base;
	int          m_tx_color_base;

	static const gfx_layout charlayout, tilelayout;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	devcb_write_line m_inth_callback;
	devcb_write_line m_intl_callback;
	emu_timer *m_intl_timer;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void video_control(uint8_t data);
};

DECLARE_DEVICE_TYPE(TC0180VCU, tc0180vcu_device)

#endif // MAME_TAITO_TC0180VCU_H
