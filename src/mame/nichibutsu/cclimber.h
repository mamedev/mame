// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_NICHIBUTSU_CCLIMBER_H
#define MAME_NICHIBUTSU_CCLIMBER_H

#pragma once

#include "machine/segacrpt_device.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class cclimber_state : public driver_device
{
public:
	cclimber_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mainlatch(*this, "mainlatch"),
		m_bigsprite_videoram(*this, "bigspriteram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_bigsprite_control(*this, "bigspritectrl"),
		m_colorram(*this, "colorram"),
		m_column_scroll(*this, "column_scroll"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	void init_cannonb();
	void init_cannonb2();
	void init_cclimber();
	void init_cclimberj();
	void init_ckongb();
	void init_dking();
	void init_rpatrol();

	void root(machine_config &config);
	void bagmanf(machine_config &config);
	void cannonb(machine_config &config);
	void cclimber(machine_config &config);
	void cclimberx(machine_config &config);
	void ckongb(machine_config &config);
	void rpatrol(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero); }
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_mainlatch;
	required_shared_ptr<uint8_t> m_bigsprite_videoram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_bigsprite_control;
	required_shared_ptr<uint8_t> m_colorram;

	std::unique_ptr<uint8_t[]> m_opcodes;

	void nmi_mask_w(int state);
	void cclimber_colorram_w(offs_t offset, uint8_t data);
	void flip_screen_x_w(int state);
	void flip_screen_y_w(int state);

	void cclimber_portmap(address_map &map);

	void vblank_irq(int state);

	bool m_flip_x = false;
	bool m_flip_y = false;

	tilemap_t *m_pf_tilemap = nullptr;
	tilemap_t *m_bs_tilemap = nullptr;

	TILE_GET_INFO_MEMBER(cclimber_get_pf_tile_info);
	TILE_GET_INFO_MEMBER(cclimber_get_bs_tile_info);

	void draw_playfield(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cclimber_draw_bigsprite(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void toprollr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);

private:
	optional_shared_ptr<uint8_t> m_column_scroll;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	bool m_nmi_mask = false;

	uint8_t bagmanf_a000_r();
	void bagmanf_vblank_irq(int state);

	void cclimber_palette(palette_device &palette) const;

	uint32_t screen_update_cclimber(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cclimber_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element *gfx);
	void cclimber_decode(const uint8_t convtable[8][16]);

	void bagmanf_map(address_map &map);
	void cannonb_map(address_map &map);
	void cclimber_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
	void rpatrol_map(address_map &map);
	void rpatrol_portmap(address_map &map);
};

class swimmer_state : public cclimber_state
{
public:
	swimmer_state(const machine_config &mconfig, device_type type, const char* tag) :
		cclimber_state(mconfig, type, tag),
		m_swimmer_background_color(*this, "bgcolor"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void swimmer(machine_config &config);
	void au(machine_config &config);
	void guzzler(machine_config &config);

protected:
	virtual void video_start() override;

private:
	optional_shared_ptr<uint8_t> m_swimmer_background_color;
	optional_device<generic_latch_8_device> m_soundlatch;

	uint8_t soundlatch_read_and_clear();
	void swimmer_sh_soundlatch_w(uint8_t data);
	void sidebg_enable_w(int state);
	void palette_bank_w(int state);

	void swimmer_root_map(address_map &map);
	void swimmer_map(address_map &map);
	void swimmer_audio_map(address_map &map);
	void swimmer_audio_portmap(address_map &map);
	void au_map(address_map &map);
	void guzzler_map(address_map &map);

	void swimmer_palette(palette_device &palette) const;
	void swimmer_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, gfx_element* gfx);
	uint32_t screen_update_swimmer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_background_pen();
	void set_sidepen(uint16_t pen) { m_sidepen = pen; }
	TILE_GET_INFO_MEMBER(swimmer_get_pf_tile_info);

	bool m_side_background_enabled = false;
	bool m_palettebank = false;
	uint16_t m_sidepen = 0;

	static constexpr int SWIMMER_BG_SPLIT = 0x18 * 8;
};

class toprollr_state : public cclimber_state
{
public:
	toprollr_state(const machine_config &mconfig, device_type type, const char* tag) :
		cclimber_state(mconfig, type, tag),
		m_toprollr_bg_videoram(*this, "bg_videoram"),
		m_toprollr_bg_coloram(*this, "bg_coloram"),
		m_bank1(*this, "bank1"),
		m_bank1d(*this, "bank1d")
	{ }

	void toprollr(machine_config &config);

	void init_toprollr();

protected:
	virtual void video_start() override;

private:
	optional_shared_ptr<uint8_t> m_toprollr_bg_videoram;
	optional_shared_ptr<uint8_t> m_toprollr_bg_coloram;
	required_memory_bank m_bank1;
	required_memory_bank m_bank1d;

	void toprollr_decrypted_opcodes_map(address_map &map);
	void toprollr_map(address_map &map);

	void toprollr_palette(palette_device &palette) const;
	void toprollr_draw_bigsprite(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void toprollr_rombank_w(int state);
	uint32_t screen_update_toprollr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(toprollr_get_pf_tile_info);
	TILE_GET_INFO_MEMBER(toprollr_get_bs_tile_info);
	TILE_GET_INFO_MEMBER(toproller_get_bg_tile_info);

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_rombank = 0;
};

class yamato_state : public cclimber_state
{
public:
	yamato_state(const machine_config &mconfig, device_type type, const char* tag) :
		cclimber_state(mconfig, type, tag)
	{ }

	void yamato(machine_config &config);

	void init_yamato();

private:
	void yamato_p0_w(uint8_t data);
	void yamato_p1_w(uint8_t data);
	uint8_t yamato_p0_r();
	uint8_t yamato_p1_r();

	void yamato_map(address_map &map);
	void yamato_decrypted_opcodes_map(address_map &map);
	void yamato_portmap(address_map &map);
	void yamato_audio_map(address_map &map);
	void yamato_audio_portmap(address_map &map);

	void yamato_palette(palette_device &palette) const;
	uint32_t screen_update_yamato(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t m_yamato_p0 = 0;
	uint8_t m_yamato_p1 = 0;

	static constexpr int YAMATO_SKY_PEN_BASE = 0x60;
};

#endif // MAME_NICHIBUTSU_CCLIMBER_H
