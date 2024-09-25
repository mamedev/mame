// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_SUNA_SUNA8_H
#define MAME_SUNA_SUNA8_H

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
		m_hardhea2_banked_protram(*this, "hardhea2_protram", 0x4000, ENDIANNESS_LITTLE),
		m_audiocpu(*this, "audiocpu"),
		m_samples(*this, "samples"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_mainbank(*this, "mainbank"),
		m_decrypted_opcodes_low(*this, "decrypted_opcodes_low"),
		m_decrypted_opcodes_high(*this, "decrypted_opcodes_high"),
		m_hardhea2_rambank(*this, "hardhea2_rambank"),
		m_inputs(*this, {"P1", "P2", "DSW1", "DSW2"}),
		m_cheats(*this, "CHEATS"),
		m_leds(*this, "led%u", 0U),
		m_prot_opcode_toggle(0),
		m_remap_sound(0)
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

protected:
	virtual void machine_start() override ATTR_COLD;

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
	template <uint8_t Which> void hardhea2_prot_spritebank_w(uint8_t data);
	template <uint8_t Which> void hardhea2_prot_rambank_w(uint8_t data);

	// starfigh
	void starfigh_rombank_latch_w(offs_t offset, uint8_t data);
	void starfigh_spritebank_latch_w(uint8_t data);
	void starfigh_sound_latch_w(uint8_t data);
	void starfigh_spritebank_w(uint8_t data);
	void starfigh_leds_w(uint8_t data);

	// sparkman
	void sparkman_rombank_latch_w(offs_t offset, uint8_t data);
	void sparkman_rombank_w(uint8_t data);
	void sparkman_spritebank_latch_w(uint8_t data);
	void sparkman_spritebank_w(uint8_t data);
	void sparkman_write_disable_w(uint8_t data);
	void wram_w(offs_t offset, uint8_t data);
	void sparkman_coin_counter_w(uint8_t data);
	uint8_t sparkman_c0a3_r();

	uint8_t banked_paletteram_r(offs_t offset);
	uint8_t banked_spriteram_r(offs_t offset);
	void banked_spriteram_w(offs_t offset, uint8_t data);

	void vh_start_common(bool has_text, GFXBANK_TYPE_T gfxbank_type);
	DECLARE_VIDEO_START(text);
	DECLARE_VIDEO_START(sparkman);
	DECLARE_VIDEO_START(brickzn);
	DECLARE_VIDEO_START(starfigh);

	DECLARE_MACHINE_RESET(brickzn);
	DECLARE_MACHINE_RESET(hardhea2);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(hardhea2_interrupt);

	// samples
	void play_samples_w(uint8_t data);
	void rranger_play_samples_w(uint8_t data);
	void samples_number_w(uint8_t data);
	void play_sample(int index);
	SAMPLES_START_CB_MEMBER(sh_start);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int which);
	void draw_text_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int ypos, bool write_mask);
	void brickzn_decrypt();

	void brickzn11_map(address_map &map) ATTR_COLD;
	void brickzn_io_map(address_map &map) ATTR_COLD;
	void brickzn_map(address_map &map) ATTR_COLD;
	void brickzn_pcm_io_map(address_map &map) ATTR_COLD;
	void brickzn_pcm_map(address_map &map) ATTR_COLD;
	void brickzn_sound_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void hardhea2_map(address_map &map) ATTR_COLD;
	void hardhea2b_map(address_map &map) ATTR_COLD;
	void hardhea2b_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void hardhead_io_map(address_map &map) ATTR_COLD;
	void hardhead_map(address_map &map) ATTR_COLD;
	void hardhead_sound_io_map(address_map &map) ATTR_COLD;
	void hardhead_sound_map(address_map &map) ATTR_COLD;
	void rranger_io_map(address_map &map) ATTR_COLD;
	void rranger_map(address_map &map) ATTR_COLD;
	void rranger_sound_map(address_map &map) ATTR_COLD;
	void sparkman_map(address_map &map) ATTR_COLD;
	void starfigh_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<uint8_t> m_hardhead_ip;
	memory_share_creator<uint8_t> m_banked_spriteram;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_wram;
	memory_share_creator<uint8_t> m_banked_paletteram;
	memory_share_creator<uint8_t> m_hardhea2_banked_protram;
	required_device<cpu_device> m_audiocpu;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_memory_bank m_mainbank;
	optional_memory_bank m_decrypted_opcodes_low;
	optional_memory_bank m_decrypted_opcodes_high;
	optional_memory_bank m_hardhea2_rambank;
	optional_ioport_array<4> m_inputs;
	optional_ioport m_cheats;
	output_finder<2> m_leds;

	std::unique_ptr<uint8_t[]> m_decrypt;

	uint8_t m_rombank = 0; // all
	uint8_t m_rombank_latch = 0; // sparkman and starfigh
	uint8_t m_spritebank = 0; // all
	uint8_t m_palettebank = 0; // all brickzn sets
	uint8_t m_paletteram_enab = 0; // only brickzn newer sets (no 1.1)
	uint8_t m_prot2 = 0; // only brickzn newer sets (no 1.1)
	uint8_t m_prot2_prev = 0; // only brickzn newer sets (no 1.1)

	uint8_t m_protection_val = 0; // hardhead and brickzn newer sets (no 1.1)
	uint8_t m_nmi_enable = 0; // hardhea2, sparkman and starfigh
	uint8_t m_spritebank_latch = 0; // sparkman and starfigh
	uint8_t m_write_disable = 0; // only sparkman
	uint8_t m_prot_opcode_toggle = 0; // only brickzn newer sets (no 1.1)
	uint8_t m_remap_sound = 0; // only brickzn newer sets (no 1.1)

	uint8_t m_gfxbank = 0; // only starfigh

	bool m_has_text = false; // has text sprites (older games)

	// samples
	std::unique_ptr<int16_t[]> m_samplebuf;
	uint8_t m_sample = 0, m_play = 0;
	int m_numsamples = 0;
};

#endif // MAME_SUNA_SUNA8_H
