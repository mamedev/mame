// license:BSD-3-Clause
// copyright-holders:David Haywood,Nicola Salmoria,Paul Priest
#include "machine/eepromser.h"
#include "sound/okim6295.h"

class pirates_state : public driver_device
{
public:
	pirates_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_tx_tileram(*this, "tx_tileram"),
		m_fg_tileram(*this, "fg_tileram"),
		m_bg_tileram(*this, "bg_tileram") { }

	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_scroll;
	required_shared_ptr<UINT16> m_tx_tileram;
	required_shared_ptr<UINT16> m_fg_tileram;
	required_shared_ptr<UINT16> m_bg_tileram;

	tilemap_t *m_tx_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE16_MEMBER(out_w);
	DECLARE_WRITE16_MEMBER(tx_tileram_w);
	DECLARE_WRITE16_MEMBER(fg_tileram_w);
	DECLARE_WRITE16_MEMBER(bg_tileram_w);
	DECLARE_READ16_MEMBER(genix_prot_r);

	DECLARE_CUSTOM_INPUT_MEMBER(prot_r);

	DECLARE_DRIVER_INIT(pirates);
	DECLARE_DRIVER_INIT(genix);

	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void decrypt_68k();
	void decrypt_p();
	void decrypt_s();
	void decrypt_oki();
};
