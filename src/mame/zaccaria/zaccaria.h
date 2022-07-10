// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_INCLUDES_ZACCARIA_H
#define MAME_INCLUDES_ZACCARIA_H

#pragma once

#include "zaccaria_a.h"
#include "emupal.h"
#include "tilemap.h"

class zaccaria_state : public driver_device
{
public:
	zaccaria_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_audiopcb(*this, "audiopcb")
		, m_videoram(*this, "videoram")
		, m_attributesram(*this, "attributesram")
		, m_spriteram(*this, "spriteram%u", 1U)
		, m_dsw_port(*this, "DSW.%u", 0)
		, m_coins(*this, "COINS")
	{ }

	void zaccaria(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	uint8_t dsw_r();
	uint8_t prot1_r(offs_t offset);
	uint8_t prot2_r(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(coin_w);
	DECLARE_WRITE_LINE_MEMBER(nmi_mask_w);
	void videoram_w(offs_t offset, uint8_t data);
	void attributes_w(offs_t offset, uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_x_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_y_w);
	void dsw_sel_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *spriteram, int color, int section);

	void main_map(address_map &map);

	required_device<cpu_device>                 m_maincpu;
	required_device<gfxdecode_device>           m_gfxdecode;
	required_device<palette_device>             m_palette;
	required_device<zac1b11142_audio_device>    m_audiopcb;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_attributesram;
	required_shared_ptr_array<uint8_t, 2> m_spriteram;

	required_ioport_array<3> m_dsw_port;
	required_ioport m_coins;

	uint8_t m_dsw_sel = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_nmi_mask = 0;
};

#endif // MAME_INCLUDES_ZACCARIA_H
