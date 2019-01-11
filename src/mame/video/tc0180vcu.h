// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_TC0180VCU_H
#define MAME_VIDEO_TC0180VCU_H

#pragma once

class tc0180vcu_device : public device_t
{
public:
	tc0180vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_gfxdecode_tag(const char *tag) { m_gfxdecode.set_tag(tag); }
	void set_bg_colorbase(int color) { m_bg_color_base = color; }
	void set_fg_colorbase(int color) { m_fg_color_base = color; }
	void set_tx_colorbase(int color) { m_tx_color_base = color; }

	DECLARE_READ8_MEMBER( get_fb_page );
	DECLARE_WRITE8_MEMBER( set_fb_page );
	DECLARE_READ8_MEMBER( get_videoctrl );
	DECLARE_READ16_MEMBER( ctrl_r );
	DECLARE_WRITE16_MEMBER( ctrl_w );
	DECLARE_READ16_MEMBER( scroll_r );
	DECLARE_WRITE16_MEMBER( scroll_w );
	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, int plane);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint16_t         m_ctrl[0x10];

	std::unique_ptr<uint16_t[]>       m_ram;
	std::unique_ptr<uint16_t[]>       m_scrollram;

	tilemap_t      *m_tilemap[3];

	uint16_t         m_bg_rambank[2], m_fg_rambank[2], m_tx_rambank;
	uint8_t          m_framebuffer_page;
	uint8_t          m_video_control;

	int            m_bg_color_base;
	int            m_fg_color_base;
	int            m_tx_color_base;

	required_device<gfxdecode_device> m_gfxdecode;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void video_control( uint8_t data );
};

DECLARE_DEVICE_TYPE(TC0180VCU, tc0180vcu_device)

#define MCFG_TC0180VCU_BG_COLORBASE(_color) \
	downcast<tc0180vcu_device &>(*device).set_bg_colorbase(_color);

#define MCFG_TC0180VCU_FG_COLORBASE(_color) \
	downcast<tc0180vcu_device &>(*device).set_fg_colorbase(_color);

#define MCFG_TC0180VCU_TX_COLORBASE(_color) \
	downcast<tc0180vcu_device &>(*device).set_tx_colorbase(_color);

#define MCFG_TC0180VCU_GFXDECODE(_gfxtag) \
	downcast<tc0180vcu_device &>(*device).set_gfxdecode_tag(_gfxtag);

#endif // MAME_VIDEO_TC0180VCU_H
