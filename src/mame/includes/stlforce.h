// license:BSD-3-Clause
// copyright-holders:David Haywood
#include "sound/okim6295.h"
#include "machine/eepromser.h"

class stlforce_state : public driver_device
{
public:
	stlforce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bg_videoram(*this, "bg_videoram"),
		m_mlow_videoram(*this, "mlow_videoram"),
		m_mhigh_videoram(*this, "mhigh_videoram"),
		m_tx_videoram(*this, "tx_videoram"),
		m_bg_scrollram(*this, "bg_scrollram"),
		m_mlow_scrollram(*this, "mlow_scrollram"),
		m_mhigh_scrollram(*this, "mhigh_scrollram"),
		m_vidattrram(*this, "vidattrram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_mlow_videoram;
	required_shared_ptr<UINT16> m_mhigh_videoram;
	required_shared_ptr<UINT16> m_tx_videoram;
	required_shared_ptr<UINT16> m_bg_scrollram;
	required_shared_ptr<UINT16> m_mlow_scrollram;
	required_shared_ptr<UINT16> m_mhigh_scrollram;
	required_shared_ptr<UINT16> m_vidattrram;
	required_shared_ptr<UINT16> m_spriteram;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_mlow_tilemap;
	tilemap_t *m_mhigh_tilemap;
	tilemap_t *m_tx_tilemap;

	int m_sprxoffs;

	DECLARE_WRITE16_MEMBER(stlforce_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(stlforce_mlow_videoram_w);
	DECLARE_WRITE16_MEMBER(stlforce_mhigh_videoram_w);
	DECLARE_WRITE16_MEMBER(stlforce_tx_videoram_w);
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_WRITE16_MEMBER(oki_bank_w);

	DECLARE_DRIVER_INIT(twinbrat);
	DECLARE_DRIVER_INIT(stlforce);

	TILE_GET_INFO_MEMBER(get_stlforce_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_stlforce_mlow_tile_info);
	TILE_GET_INFO_MEMBER(get_stlforce_mhigh_tile_info);
	TILE_GET_INFO_MEMBER(get_stlforce_tx_tile_info);

	virtual void video_start() override;
	UINT32 screen_update_stlforce(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
