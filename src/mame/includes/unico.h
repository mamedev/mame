// license:BSD-3-Clause
// copyright-holders:Luca Elia
#include "sound/okim6295.h"
#include "machine/eepromser.h"

class unico_state : public driver_device
{
public:
	unico_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_generic_paletteram_16(*this, "paletteram"),
		m_generic_paletteram_32(*this, "paletteram") { }

	std::unique_ptr<UINT16[]> m_vram;
	std::unique_ptr<UINT16[]> m_scroll;
	tilemap_t *m_tilemap[3];
	int m_sprites_scrolldx;
	int m_sprites_scrolldy;
	std::unique_ptr<UINT16[]> m_spriteram;
	DECLARE_WRITE16_MEMBER(zeropnt_sound_bank_w);
	DECLARE_READ16_MEMBER(unico_gunx_0_msb_r);
	DECLARE_READ16_MEMBER(unico_guny_0_msb_r);
	DECLARE_READ16_MEMBER(unico_gunx_1_msb_r);
	DECLARE_READ16_MEMBER(unico_guny_1_msb_r);
	DECLARE_READ32_MEMBER(zeropnt2_gunx_0_msb_r);
	DECLARE_READ32_MEMBER(zeropnt2_guny_0_msb_r);
	DECLARE_READ32_MEMBER(zeropnt2_gunx_1_msb_r);
	DECLARE_READ32_MEMBER(zeropnt2_guny_1_msb_r);
	DECLARE_WRITE32_MEMBER(zeropnt2_sound_bank_w);
	DECLARE_WRITE32_MEMBER(zeropnt2_leds_w);
	DECLARE_WRITE16_MEMBER(unico_palette_w);
	DECLARE_WRITE32_MEMBER(unico_palette32_w);
	DECLARE_READ16_MEMBER(unico_vram_r);
	DECLARE_WRITE16_MEMBER(unico_vram_w);
	DECLARE_READ16_MEMBER(unico_scroll_r);
	DECLARE_WRITE16_MEMBER(unico_scroll_w);
	DECLARE_READ16_MEMBER(unico_spriteram_r);
	DECLARE_WRITE16_MEMBER(unico_spriteram_w);

	DECLARE_WRITE16_MEMBER(burglarx_sound_bank_w);
	DECLARE_WRITE32_MEMBER(zeropnt2_eeprom_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_MACHINE_RESET(unico);
	DECLARE_VIDEO_START(unico);
	DECLARE_MACHINE_RESET(zeropt);
	UINT32 screen_update_unico(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void unico_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT16> m_generic_paletteram_16;
	optional_shared_ptr<UINT32> m_generic_paletteram_32;
};
