// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_DATAEAST_LIBERATE_H
#define MAME_DATAEAST_LIBERATE_H

#pragma once

#include "machine/gen_latch.h"
#include "emupal.h"
#include "tilemap.h"

class liberate_state : public driver_device
{
public:
	liberate_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_vram(*this, "bg_vram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scratchram(*this, "scratchram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void liberate_base(machine_config &config);
	void liberate(machine_config &config);
	void liberatb(machine_config &config);
	void boomrang(machine_config &config);
	void prosoccr(machine_config &config);
	void prosport(machine_config &config);

	void init_yellowcb();
	void init_liberate();
	void init_prosport();

private:
	optional_shared_ptr<uint8_t> m_bg_vram; /* prosport */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_scratchram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	uint8_t *m_fg_gfx = nullptr;   /* prosoccr */
	std::unique_ptr<uint8_t[]> m_charram{};   /* prosoccr */
	uint8_t m_io_ram[16]{};

	int m_bank = 0;
	int m_latch = 0;
	uint8_t m_gfx_rom_readback = 0U;
	int m_background_color = 0;
	int m_background_disable = 0;

	tilemap_t *m_back_tilemap = nullptr;
	tilemap_t *m_fix_tilemap = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	uint8_t deco16_bank_r(offs_t offset);
	uint8_t deco16_io_r(offs_t offset);
	void deco16_bank_w(uint8_t data);
	uint8_t prosoccr_bank_r(offs_t offset);
	uint8_t prosoccr_charram_r(offs_t offset);
	void prosoccr_charram_w(offs_t offset, uint8_t data);
	void prosoccr_char_bank_w(uint8_t data);
	void prosoccr_io_bank_w(uint8_t data);
	uint8_t prosport_charram_r(offs_t offset);
	void prosport_charram_w(offs_t offset, uint8_t data);
	void deco16_io_w(offs_t offset, uint8_t data);
	void prosoccr_io_w(offs_t offset, uint8_t data);
	void prosport_io_w(offs_t offset, uint8_t data);
	void liberate_videoram_w(offs_t offset, uint8_t data);
	void liberate_colorram_w(offs_t offset, uint8_t data);
	void prosport_bg_vram_w(offs_t offset, uint8_t data);
	TILEMAP_MAPPER_MEMBER(back_scan);
	TILEMAP_MAPPER_MEMBER(fix_scan);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	TILE_GET_INFO_MEMBER(prosport_get_back_tile_info);
	DECLARE_MACHINE_START(liberate);
	DECLARE_MACHINE_RESET(liberate);
	DECLARE_VIDEO_START(liberate);
	void liberate_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(prosport);
	DECLARE_VIDEO_START(boomrang);
	DECLARE_VIDEO_START(prosoccr);
	uint32_t screen_update_liberate(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_prosport(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_boomrang(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_prosoccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void deco16_interrupt(int state);
	void liberate_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prosport_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void boomrang_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void prosoccr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void deco16_io_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void liberatb_map(address_map &map) ATTR_COLD;
	void liberate_map(address_map &map) ATTR_COLD;
	void liberate_sound_map(address_map &map) ATTR_COLD;
	void prosoccr_io_map(address_map &map) ATTR_COLD;
	void prosoccr_map(address_map &map) ATTR_COLD;
	void prosoccr_sound_map(address_map &map) ATTR_COLD;
	void prosport_map(address_map &map) ATTR_COLD;
};

#endif // MAME_DATAEAST_LIBERATE_H
