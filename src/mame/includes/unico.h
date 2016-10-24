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

	std::unique_ptr<uint16_t[]> m_vram;
	std::unique_ptr<uint16_t[]> m_scroll;
	tilemap_t *m_tilemap[3];
	int m_sprites_scrolldx;
	int m_sprites_scrolldy;
	std::unique_ptr<uint16_t[]> m_spriteram;
	void zeropnt_sound_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t unico_gunx_0_msb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t unico_guny_0_msb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t unico_gunx_1_msb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t unico_guny_1_msb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t zeropnt2_gunx_0_msb_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t zeropnt2_guny_0_msb_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t zeropnt2_gunx_1_msb_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t zeropnt2_guny_1_msb_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void zeropnt2_sound_bank_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void zeropnt2_leds_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void unico_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void unico_palette32_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint16_t unico_vram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void unico_vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t unico_scroll_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void unico_scroll_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t unico_spriteram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void unico_spriteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void burglarx_sound_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void zeropnt2_eeprom_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_reset_unico();
	void video_start_unico();
	void machine_reset_zeropt();
	uint32_t screen_update_unico(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void unico_draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	optional_device<okim6295_device> m_oki;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint16_t> m_generic_paletteram_16;
	optional_shared_ptr<uint32_t> m_generic_paletteram_32;
};
