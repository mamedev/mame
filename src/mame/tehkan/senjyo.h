// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
#ifndef MAME_INCLUDES_SENJYO_H
#define MAME_INCLUDES_SENJYO_H

#pragma once

#include "sound/dac.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "emupal.h"
#include "tilemap.h"

class senjyo_state : public driver_device
{
public:
	senjyo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio(*this, "z80pio"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_radar_palette(*this, "radar_palette"),
		m_spriteram(*this, "spriteram"),
		m_fgscroll(*this, "fgscroll"),
		m_scrollx1(*this, "scrollx1"),
		m_scrolly1(*this, "scrolly1"),
		m_scrollx2(*this, "scrollx2"),
		m_scrolly2(*this, "scrolly2"),
		m_scrollx3(*this, "scrollx3"),
		m_scrolly3(*this, "scrolly3"),
		m_fgvideoram(*this, "fgvideoram"),
		m_fgcolorram(*this, "fgcolorram"),
		m_bg1videoram(*this, "bg1videoram"),
		m_bg2videoram(*this, "bg2videoram"),
		m_bg3videoram(*this, "bg3videoram"),
		m_radarram(*this, "radarram"),
		m_bgstripesram(*this, "bgstripesram"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void senjyox_e(machine_config &config);
	void senjyo(machine_config &config);
	void starforb(machine_config &config);
	void senjyox_a(machine_config &config);

	void init_starfora();
	void init_senjyo();
	void init_starfore();
	void init_starforc();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<dac_byte_interface> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<palette_device> m_radar_palette;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_fgscroll;
	required_shared_ptr<uint8_t> m_scrollx1;
	required_shared_ptr<uint8_t> m_scrolly1;
	required_shared_ptr<uint8_t> m_scrollx2;
	required_shared_ptr<uint8_t> m_scrolly2;
	required_shared_ptr<uint8_t> m_scrollx3;
	required_shared_ptr<uint8_t> m_scrolly3;
	required_shared_ptr<uint8_t> m_fgvideoram;
	required_shared_ptr<uint8_t> m_fgcolorram;
	required_shared_ptr<uint8_t> m_bg1videoram;
	required_shared_ptr<uint8_t> m_bg2videoram;
	required_shared_ptr<uint8_t> m_bg3videoram;
	required_shared_ptr<uint8_t> m_radarram;
	required_shared_ptr<uint8_t> m_bgstripesram;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	// game specific initialization
	int m_is_senjyo = 0;
	int m_scrollhack = 0;

	uint8_t m_sound_cmd = 0;
	int m_single_volume = 0;
	int m_sound_state = 0;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg1_tilemap = nullptr;
	tilemap_t *m_bg2_tilemap = nullptr;
	tilemap_t *m_bg3_tilemap = nullptr;

	void flip_screen_w(uint8_t data);
	void starforb_scrolly2(offs_t offset, uint8_t data);
	void starforb_scrollx2(offs_t offset, uint8_t data);
	void fgvideoram_w(offs_t offset, uint8_t data);
	void fgcolorram_w(offs_t offset, uint8_t data);
	void bg1videoram_w(offs_t offset, uint8_t data);
	void bg2videoram_w(offs_t offset, uint8_t data);
	void bg3videoram_w(offs_t offset, uint8_t data);
	void volume_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(sound_line_clock);
	void sound_cmd_w(uint8_t data);
	void irq_ctrl_w(uint8_t data);
	uint8_t pio_pa_r();

	static rgb_t IIBBGGRR(uint32_t raw);
	void radar_palette(palette_device &palette) const;

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(senjyo_bg1_tile_info);
	TILE_GET_INFO_MEMBER(starforc_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg3_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_bgbitmap(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_radar(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect,int priority);

	void decrypted_opcodes_map(address_map &map);
	void senjyo_map(address_map &map);
	void senjyo_sound_io_map(address_map &map);
	void senjyo_sound_map(address_map &map);
	void starforb_map(address_map &map);
	void starforb_sound_map(address_map &map);
};

/*----------- defined in audio/senjyo.c -----------*/
extern const z80_daisy_config senjyo_daisy_chain[];

#endif // MAME_INCLUDES_SENJYO_H
