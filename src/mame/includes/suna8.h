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
		m_banked_spriteram(*this, "banked_spriteram", 0x8000, ENDIANNESS_LITTLE),
		m_spriteram(*this, "spriteram"),
		m_wram(*this, "wram"),
		m_banked_paletteram(*this, "paletteram", 0x400, ENDIANNESS_LITTLE),
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

	uint8_t hardhead_protection_r(offs_t offset);
	void hardhead_protection_w(offs_t offset, uint8_t data);
	uint8_t hardhead_ip_r();
	void hardhead_bankswitch_w(uint8_t data);
	void hardhead_flipscreen_w(uint8_t data);
	void rranger_bankswitch_w(uint8_t data);
	uint8_t rranger_soundstatus_r();
	void sranger_prot_w(uint8_t data);

	// brickzn
	uint8_t brickzn_cheats_r();
	void brickzn_leds_w(uint8_t data);
	void brickzn_palbank_w(uint8_t data);
	void brickzn_sprbank_w(uint8_t data);
	void brickzn_rombank_w(uint8_t data);
	void brickzn_banked_paletteram_w(offs_t offset, uint8_t data);
	// brickzn (newer sets)
	void brickzn_prot2_w(uint8_t data);
	void brickzn_multi_w(uint8_t data);
	void brickzn_enab_palram_w(uint8_t data);
	void brickzn_disab_palram_w(uint8_t data);

	// hardhea2
	void hardhea2_nmi_w(uint8_t data);
	void hardhea2_flipscreen_w(uint8_t data);
	void hardhea2_leds_w(uint8_t data);
	void hardhea2_spritebank_w(uint8_t data);
	void hardhea2_rombank_w(uint8_t data);
	void hardhea2_spritebank_0_w(uint8_t data);
	void hardhea2_spritebank_1_w(uint8_t data);
	void hardhea2_rambank_0_w(uint8_t data);
	void hardhea2_rambank_1_w(uint8_t data);

	// starfigh
	void starfigh_rombank_latch_w(offs_t offset, uint8_t data);
	void starfigh_spritebank_latch_w(uint8_t data);
	void starfigh_sound_latch_w(uint8_t data);
	void starfigh_spritebank_w(uint8_t data);
	uint8_t starfigh_cheats_r();
	void starfigh_leds_w(uint8_t data);

	// sparkman
	void sparkman_rombank_latch_w(offs_t offset, uint8_t data);
	void sparkman_rombank_w(uint8_t data);
	void sparkman_spritebank_latch_w(uint8_t data);
	void sparkman_spritebank_w(uint8_t data);
	void sparkman_write_disable_w(uint8_t data);
	void suna8_wram_w(offs_t offset, uint8_t data);
	void sparkman_coin_counter_w(uint8_t data);
	uint8_t sparkman_c0a3_r();

	uint8_t banked_paletteram_r(offs_t offset);
	uint8_t suna8_banked_spriteram_r(offs_t offset);
	void suna8_spriteram_w(offs_t offset, uint8_t data);
	void suna8_banked_spriteram_w(offs_t offset, uint8_t data);

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
	void suna8_play_samples_w(uint8_t data);
	void rranger_play_samples_w(uint8_t data);
	void suna8_samples_number_w(uint8_t data);
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
	memory_share_creator<uint8_t> m_banked_spriteram;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_wram;
	memory_share_creator<uint8_t> m_banked_paletteram;
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
