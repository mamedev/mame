// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_SUNA8_H
#define MAME_INCLUDES_SUNA8_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/timer.h"
#include "sound/samples.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

#define TILEMAPS 0

class suna8_state : public driver_device
{
public:
	suna8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_hardhead_ip(*this, "hardhead_ip"),
		m_spriteram(*this, "spriteram"),
		m_wram(*this, "wram"),
		m_banked_paletteram(*this, "paletteram"),
		m_audiocpu(*this, "audiocpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_bank0d(*this, "bank0d"),
		m_bank1(*this, "bank1"),
		m_bank1d(*this, "bank1d"),
		m_prot_opcode_toggle(0),
		m_remap_sound(0),
		m_leds(*this, "led%u", 0U)
	{ }

	void brickzn(machine_config &config);
	void starfigh(machine_config &config);
	void sparkman(machine_config &config);
	void hardhea2b(machine_config &config);
	void brickzn11(machine_config &config);
	void hardhead(machine_config &config);
	void hardhea2(machine_config &config);
	void rranger(machine_config &config);

	void init_brickzn_common();
	void init_brickznv5();
	void init_brickznv4();
	void init_starfigh();
	void init_hardhea2();
	void init_hardhea2b();
	void init_hardhedb();
	void init_sparkman();
	void init_brickzn();
	void init_brickzn11();
	void init_hardhead();
	void init_suna8();

private:
	enum GFXBANK_TYPE_T
	{
		GFXBANK_TYPE_SPARKMAN,
		GFXBANK_TYPE_BRICKZN,
		GFXBANK_TYPE_STARFIGH
	}   m_gfxbank_type;

	DECLARE_READ8_MEMBER(hardhead_protection_r);
	DECLARE_WRITE8_MEMBER(hardhead_protection_w);
	DECLARE_READ8_MEMBER(hardhead_ip_r);
	DECLARE_WRITE8_MEMBER(hardhead_bankswitch_w);
	DECLARE_WRITE8_MEMBER(hardhead_flipscreen_w);
	DECLARE_WRITE8_MEMBER(rranger_bankswitch_w);
	DECLARE_READ8_MEMBER(rranger_soundstatus_r);
	DECLARE_WRITE8_MEMBER(sranger_prot_w);

	// brickzn
	DECLARE_READ8_MEMBER(brickzn_cheats_r);
	DECLARE_WRITE8_MEMBER(brickzn_leds_w);
	DECLARE_WRITE8_MEMBER(brickzn_palbank_w);
	DECLARE_WRITE8_MEMBER(brickzn_sprbank_w);
	DECLARE_WRITE8_MEMBER(brickzn_rombank_w);
	DECLARE_WRITE8_MEMBER(brickzn_banked_paletteram_w);
	// brickzn (newer sets)
	DECLARE_WRITE8_MEMBER(brickzn_prot2_w);
	DECLARE_WRITE8_MEMBER(brickzn_multi_w);
	DECLARE_WRITE8_MEMBER(brickzn_enab_palram_w);
	DECLARE_WRITE8_MEMBER(brickzn_disab_palram_w);

	// hardhea2
	DECLARE_WRITE8_MEMBER(hardhea2_nmi_w);
	DECLARE_WRITE8_MEMBER(hardhea2_flipscreen_w);
	DECLARE_WRITE8_MEMBER(hardhea2_leds_w);
	DECLARE_WRITE8_MEMBER(hardhea2_spritebank_w);
	DECLARE_WRITE8_MEMBER(hardhea2_rombank_w);
	DECLARE_WRITE8_MEMBER(hardhea2_spritebank_0_w);
	DECLARE_WRITE8_MEMBER(hardhea2_spritebank_1_w);
	DECLARE_WRITE8_MEMBER(hardhea2_rambank_0_w);
	DECLARE_WRITE8_MEMBER(hardhea2_rambank_1_w);

	// starfigh
	DECLARE_WRITE8_MEMBER(starfigh_rombank_latch_w);
	DECLARE_WRITE8_MEMBER(starfigh_spritebank_latch_w);
	DECLARE_WRITE8_MEMBER(starfigh_sound_latch_w);
	DECLARE_WRITE8_MEMBER(starfigh_spritebank_w);
	DECLARE_READ8_MEMBER(starfigh_cheats_r);
	DECLARE_WRITE8_MEMBER(starfigh_leds_w);

	// sparkman
	DECLARE_WRITE8_MEMBER(sparkman_rombank_latch_w);
	DECLARE_WRITE8_MEMBER(sparkman_rombank_w);
	DECLARE_WRITE8_MEMBER(sparkman_spritebank_latch_w);
	DECLARE_WRITE8_MEMBER(sparkman_spritebank_w);
	DECLARE_WRITE8_MEMBER(sparkman_write_disable_w);
	DECLARE_WRITE8_MEMBER(suna8_wram_w);
	DECLARE_WRITE8_MEMBER(sparkman_coin_counter_w);
	DECLARE_READ8_MEMBER(sparkman_c0a3_r);

	DECLARE_READ8_MEMBER(banked_paletteram_r);
	DECLARE_READ8_MEMBER(suna8_banked_spriteram_r);
	DECLARE_WRITE8_MEMBER(suna8_spriteram_w);
	DECLARE_WRITE8_MEMBER(suna8_banked_spriteram_w);

	void suna8_vh_start_common(bool has_text, GFXBANK_TYPE_T gfxbank_type);
	DECLARE_VIDEO_START(suna8_text);
	DECLARE_VIDEO_START(suna8_sparkman);
	DECLARE_VIDEO_START(suna8_brickzn);
	DECLARE_VIDEO_START(suna8_starfigh);

	DECLARE_MACHINE_RESET(brickzn);
	DECLARE_MACHINE_RESET(hardhea2);
	uint32_t screen_update_suna8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(hardhea2_interrupt);

	// samples
	DECLARE_WRITE8_MEMBER(suna8_play_samples_w);
	DECLARE_WRITE8_MEMBER(rranger_play_samples_w);
	DECLARE_WRITE8_MEMBER(suna8_samples_number_w);
	void play_sample(int index);
	SAMPLES_START_CB_MEMBER(sh_start);

	void draw_sprites     (screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int which);
	void draw_text_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int ypos, bool write_mask);
	uint8_t *brickzn_decrypt();

	void brickzn11_map(address_map &map);
	void brickzn_io_map(address_map &map);
	void brickzn_map(address_map &map);
	void brickzn_pcm_io_map(address_map &map);
	void brickzn_pcm_map(address_map &map);
	void brickzn_sound_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
	void hardhea2_map(address_map &map);
	void hardhea2b_map(address_map &map);
	void hardhea2b_decrypted_opcodes_map(address_map &map);
	void hardhead_io_map(address_map &map);
	void hardhead_map(address_map &map);
	void hardhead_sound_io_map(address_map &map);
	void hardhead_sound_map(address_map &map);
	void rranger_io_map(address_map &map);
	void rranger_map(address_map &map);
	void rranger_sound_map(address_map &map);
	void sparkman_map(address_map &map);
	void starfigh_map(address_map &map);

	virtual void machine_start() override { m_leds.resolve(); }

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<uint8_t> m_hardhead_ip;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_wram;
	optional_shared_ptr<uint8_t> m_banked_paletteram;
	required_device<cpu_device> m_audiocpu;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	optional_memory_bank m_bank0d;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank1d;

	uint8_t m_rombank;
	uint8_t m_rombank_latch;
	uint8_t m_spritebank;
	uint8_t m_palettebank;
	uint8_t m_paletteram_enab;
	uint8_t m_prot2;
	uint8_t m_prot2_prev;

	uint8_t m_protection_val;
	uint8_t m_nmi_enable;
	uint8_t m_spritebank_latch;
	uint8_t m_write_disable;
	uint8_t m_prot_opcode_toggle;
	uint8_t m_remap_sound;
	uint8_t* m_decrypt;

	uint8_t m_gfxbank;

	bool m_has_text; // has text sprites (older games)

	// samples
	std::unique_ptr<int16_t[]> m_samplebuf;
	int m_sample, m_play;
	int m_numsamples;

#if TILEMAPS
	tilemap_t *m_bg_tilemap;
	int m_tiles;
	int m_trombank;
	int m_page;

	TILE_GET_INFO_MEMBER(get_tile_info);
#endif
	output_finder<2> m_leds;
};

#endif // MAME_INCLUDES_SUNA8_H
