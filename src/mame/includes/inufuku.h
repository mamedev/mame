// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#ifndef MAME_INCLUDES_INUFUKU_H
#define MAME_INCLUDES_INUFUKU_H

#include "video/vsystem_spr.h"
#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class inufuku_state : public driver_device
{
public:
	inufuku_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_rasterram(*this, "bg_rasterram"),
		m_tx_videoram(*this, "tx_videoram"),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram2(*this, "spriteram2"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spr(*this, "vsystem_spr"),
		m_soundlatch(*this, "soundlatch") { }

	void inufuku(machine_config &config);
	void _3on3dunk(machine_config &config);

	DECLARE_READ_LINE_MEMBER(soundflag_r);

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_bg_rasterram;
	required_shared_ptr<uint16_t> m_tx_videoram;
	required_shared_ptr<uint16_t> m_spriteram1;
	required_shared_ptr<uint16_t> m_spriteram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_tx_tilemap;
	int       m_bg_scrollx;
	int       m_bg_scrolly;
	int       m_tx_scrollx;
	int       m_tx_scrolly;
	int       m_bg_raster;
	int       m_bg_palettebank;
	int       m_tx_palettebank;
	std::unique_ptr<uint16_t[]>     m_spriteram1_old;
	uint32_t  inufuku_tile_callback( uint32_t code );

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<vsystem_spr_device> m_spr;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE8_MEMBER(inufuku_soundrombank_w);
	DECLARE_WRITE16_MEMBER(inufuku_palettereg_w);
	DECLARE_WRITE16_MEMBER(inufuku_scrollreg_w);
	DECLARE_READ16_MEMBER(inufuku_bg_videoram_r);
	DECLARE_WRITE16_MEMBER(inufuku_bg_videoram_w);
	DECLARE_READ16_MEMBER(inufuku_tx_videoram_r);
	DECLARE_WRITE16_MEMBER(inufuku_tx_videoram_w);
	TILE_GET_INFO_MEMBER(get_inufuku_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_inufuku_tx_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_inufuku(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_inufuku);
	void inufuku_map(address_map &map);
	void inufuku_sound_io_map(address_map &map);
	void inufuku_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_INUFUKU_H
