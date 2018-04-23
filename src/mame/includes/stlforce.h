// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "machine/eepromser.h"

class stlforce_state : public driver_device
{
public:
	stlforce_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
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
		m_spriteram(*this, "spriteram"),
		m_okibank(*this, "okibank") { }

	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_mlow_videoram;
	required_shared_ptr<uint16_t> m_mhigh_videoram;
	required_shared_ptr<uint16_t> m_tx_videoram;
	required_shared_ptr<uint16_t> m_bg_scrollram;
	required_shared_ptr<uint16_t> m_mlow_scrollram;
	required_shared_ptr<uint16_t> m_mhigh_scrollram;
	required_shared_ptr<uint16_t> m_vidattrram;
	required_shared_ptr<uint16_t> m_spriteram;

	optional_memory_bank m_okibank;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_mlow_tilemap;
	tilemap_t *m_mhigh_tilemap;
	tilemap_t *m_tx_tilemap;

	int m_sprxoffs;

	DECLARE_WRITE16_MEMBER(bg_videoram_w);
	DECLARE_WRITE16_MEMBER(mlow_videoram_w);
	DECLARE_WRITE16_MEMBER(mhigh_videoram_w);
	DECLARE_WRITE16_MEMBER(tx_videoram_w);
	DECLARE_WRITE8_MEMBER(eeprom_w);
	DECLARE_WRITE8_MEMBER(oki_bank_w);

	DECLARE_DRIVER_INIT(twinbrat);
	DECLARE_DRIVER_INIT(stlforce);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_mlow_tile_info);
	TILE_GET_INFO_MEMBER(get_mhigh_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
	void stlforce(machine_config &config);
	void twinbrat(machine_config &config);
	void stlforce_map(address_map &map);
	void twinbrat_oki_map(address_map &map);
};
