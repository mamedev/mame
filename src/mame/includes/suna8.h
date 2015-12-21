// license:BSD-3-Clause
// copyright-holders:Luca Elia
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
		m_bank0d(*this, "bank0d"),
		m_bank1(*this, "bank1"),
		m_bank1d(*this, "bank1d"),
		m_prot_opcode_toggle(0),
		m_remap_sound(0)
		{ }

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<UINT8> m_hardhead_ip;
	optional_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_wram;
	optional_shared_ptr<UINT8> m_banked_paletteram;
	required_device<cpu_device> m_audiocpu;
	optional_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_memory_bank m_bank0d;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank1d;

	UINT8 m_rombank;
	UINT8 m_rombank_latch;
	UINT8 m_spritebank;
	UINT8 m_palettebank;
	UINT8 m_paletteram_enab;
	UINT8 m_prot2;
	UINT8 m_prot2_prev;

	UINT8 m_protection_val;
	UINT8 m_nmi_enable;
	UINT8 m_spritebank_latch;
	UINT8 m_write_disable;
	UINT8 m_prot_opcode_toggle;
	UINT8 m_remap_sound;
	UINT8* m_decrypt;

	enum GFXBANK_TYPE_T
	{
		GFXBANK_TYPE_SPARKMAN,
		GFXBANK_TYPE_BRICKZN,
		GFXBANK_TYPE_STARFIGH
	}   m_gfxbank_type;
	UINT8 m_gfxbank;

	bool m_has_text; // has text sprites (older games)

	// samples
	std::unique_ptr<INT16[]> m_samplebuf;
	int m_sample, m_play;
	int m_numsamples;

#if TILEMAPS
	tilemap_t *m_bg_tilemap;
	int m_tiles;
	int m_trombank;
	int m_page;

	TILE_GET_INFO_MEMBER(get_tile_info);
#endif

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
	DECLARE_WRITE8_MEMBER(brickzn_pcm_w);
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
	DECLARE_DRIVER_INIT(brickzn_common);
	DECLARE_DRIVER_INIT(brickznv5);
	DECLARE_DRIVER_INIT(brickznv4);
	DECLARE_DRIVER_INIT(starfigh);
	DECLARE_DRIVER_INIT(hardhea2);
	DECLARE_DRIVER_INIT(hardhea2b);
	DECLARE_DRIVER_INIT(hardhedb);
	DECLARE_DRIVER_INIT(sparkman);
	DECLARE_DRIVER_INIT(brickzn);
	DECLARE_DRIVER_INIT(brickzn11);
	DECLARE_DRIVER_INIT(hardhead);
	DECLARE_DRIVER_INIT(suna8);

	void suna8_vh_start_common(bool has_text, GFXBANK_TYPE_T gfxbank_type);
	DECLARE_VIDEO_START(suna8_text);
	DECLARE_VIDEO_START(suna8_sparkman);
	DECLARE_VIDEO_START(suna8_brickzn);
	DECLARE_VIDEO_START(suna8_starfigh);

	DECLARE_MACHINE_RESET(brickzn);
	DECLARE_MACHINE_RESET(hardhea2);
	UINT32 screen_update_suna8(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(brickzn_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(hardhea2_interrupt);

	// samples
	DECLARE_WRITE8_MEMBER(suna8_play_samples_w);
	DECLARE_WRITE8_MEMBER(rranger_play_samples_w);
	DECLARE_WRITE8_MEMBER(suna8_samples_number_w);
	void play_sample(int index);
	SAMPLES_START_CB_MEMBER(sh_start);

	void draw_sprites     (screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int which);
	void draw_text_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int end, int ypos, bool write_mask);
	UINT8 *brickzn_decrypt();
};
