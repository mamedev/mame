// license:BSD-3-Clause
// copyright-holders:Luca Elia

#include "machine/gen_latch.h"
#include "sound/samples.h"

#define TILEMAPS 0

class suna8_state : public driver_device
{
public:
	suna8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
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
		m_remap_sound(0)
		{ }

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

	enum GFXBANK_TYPE_T
	{
		GFXBANK_TYPE_SPARKMAN,
		GFXBANK_TYPE_BRICKZN,
		GFXBANK_TYPE_STARFIGH
	}   m_gfxbank_type;
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

	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
#endif

	uint8_t hardhead_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hardhead_protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hardhead_ip_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hardhead_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardhead_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rranger_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t rranger_soundstatus_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sranger_prot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// brickzn
	uint8_t brickzn_cheats_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void brickzn_leds_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brickzn_palbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brickzn_sprbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brickzn_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brickzn_banked_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	// brickzn (newer sets)
	void brickzn_prot2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brickzn_multi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brickzn_enab_palram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brickzn_disab_palram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// hardhea2
	void hardhea2_nmi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardhea2_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardhea2_leds_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardhea2_spritebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardhea2_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardhea2_spritebank_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardhea2_spritebank_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardhea2_rambank_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hardhea2_rambank_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// starfigh
	void starfigh_rombank_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void starfigh_spritebank_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void starfigh_sound_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void starfigh_spritebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t starfigh_cheats_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void starfigh_leds_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// sparkman
	void sparkman_rombank_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sparkman_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sparkman_spritebank_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sparkman_spritebank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sparkman_write_disable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void suna8_wram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sparkman_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sparkman_c0a3_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint8_t banked_paletteram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t suna8_banked_spriteram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void suna8_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void suna8_banked_spriteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
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

	void suna8_vh_start_common(bool has_text, GFXBANK_TYPE_T gfxbank_type);
	void video_start_suna8_text();
	void video_start_suna8_sparkman();
	void video_start_suna8_brickzn();
	void video_start_suna8_starfigh();

	void machine_reset_brickzn();
	void machine_reset_hardhea2();
	uint32_t screen_update_suna8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void hardhea2_interrupt(timer_device &timer, void *ptr, int32_t param);

	// samples
	void suna8_play_samples_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void rranger_play_samples_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void suna8_samples_number_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void play_sample(int index);
	SAMPLES_START_CB_MEMBER(sh_start);

	void draw_sprites     (screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int which);
	void draw_text_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int ypos, bool write_mask);
	uint8_t *brickzn_decrypt();
};
