// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
#ifndef MAME_INCLUDES_INUFUKU_H
#define MAME_INCLUDES_INUFUKU_H

#include "video/bufsprite.h"
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
		m_sprtileram(*this, "sprtileram"),
		m_audiobank(*this, "audiobank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spr(*this, "vsystem_spr"),
		m_soundlatch(*this, "soundlatch"),
		m_sprattrram(*this, "sprattrram") { }

	void inufuku(machine_config &config);
	void _3on3dunk(machine_config &config);

	DECLARE_READ_LINE_MEMBER(soundflag_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// memory pointers
	required_shared_ptr<u16> m_bg_videoram;
	required_shared_ptr<u16> m_bg_rasterram;
	required_shared_ptr<u16> m_tx_videoram;
	required_shared_ptr<u16> m_sprtileram;

	required_memory_bank m_audiobank;

	// video-related
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	int       m_bg_scrollx;
	int       m_bg_scrolly;
	int       m_tx_scrollx;
	int       m_tx_scrolly;
	bool      m_bg_raster;
	u8        m_bg_palettebank;
	u8        m_tx_palettebank;
	u32       tile_callback( u32 code );

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<vsystem_spr_device> m_spr;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<buffered_spriteram16_device> m_sprattrram;

	void soundrombank_w(u8 data);
	void palettereg_w(offs_t offset, u16 data);
	void scrollreg_w(offs_t offset, u16 data);
	void bg_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tx_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);
	void sound_io_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_INUFUKU_H
