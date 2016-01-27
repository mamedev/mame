// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "machine/eepromser.h"

class xorworld_state : public driver_device
{
public:
	xorworld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_spriteram;

	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE16_MEMBER(irq2_ack_w);
	DECLARE_WRITE16_MEMBER(irq6_ack_w);
	DECLARE_WRITE16_MEMBER(videoram_w);
	DECLARE_WRITE16_MEMBER(eeprom_chip_select_w);
	DECLARE_WRITE16_MEMBER(eeprom_serial_clock_w);
	DECLARE_WRITE16_MEMBER(eeprom_data_w);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	DECLARE_DRIVER_INIT(xorworld);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(xorworld);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
