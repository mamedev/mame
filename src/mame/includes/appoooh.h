// license:BSD-3-Clause
// copyright-holders:Tatsuyuki Satoh
#ifndef MAME_INCLUDES_APPOOOH_H
#define MAME_INCLUDES_APPOOOH_H

#pragma once

#include "sound/msm5205.h"
#include "emupal.h"
#include "tilemap.h"

class appoooh_state : public driver_device
{
public:
	appoooh_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_fg_colorram(*this, "fg_colorram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_colorram(*this, "bg_colorram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_msm(*this, "msm")
	{ }

	void init_robowresb();
	void appoooh(machine_config &config);
	void robowres(machine_config &config);
	void robowrese(machine_config &config);

protected:
	void adpcm_w(uint8_t data);
	void scroll_w(uint8_t data);
	void fg_videoram_w(offs_t offset, uint8_t data);
	void fg_colorram_w(offs_t offset, uint8_t data);
	void bg_videoram_w(offs_t offset, uint8_t data);
	void bg_colorram_w(offs_t offset, uint8_t data);
	void out_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void appoooh_palette(palette_device &palette) const;
	void robowres_palette(palette_device &palette) const;
	uint32_t screen_update_appoooh(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_robowres(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void appoooh_draw_sprites( bitmap_ind16 &dest_bmp, const rectangle &cliprect, gfx_element *gfx, uint8_t *sprite );
	void robowres_draw_sprites( bitmap_ind16 &dest_bmp, const rectangle &cliprect, gfx_element *gfx, uint8_t *sprite );
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	void appoooh_common(machine_config &config);
	void decrypted_opcodes_map(address_map &map);
	void main_map(address_map &map);
	void main_portmap(address_map &map);

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fg_videoram;
	required_shared_ptr<uint8_t> m_fg_colorram;
	required_shared_ptr<uint8_t> m_spriteram_2;
	required_shared_ptr<uint8_t> m_bg_videoram;
	required_shared_ptr<uint8_t> m_bg_colorram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	/* video-related */
	tilemap_t  *m_fg_tilemap = nullptr;
	tilemap_t  *m_bg_tilemap = nullptr;
	int m_scroll_x = 0;
	int m_priority = 0;

	/* sound-related */
	uint32_t   m_adpcm_data = 0U;
	uint32_t   m_adpcm_address = 0U;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<msm5205_device> m_msm;

	uint8_t m_nmi_mask = 0U;
};

#endif // MAME_INCLUDES_APPOOOH_H
