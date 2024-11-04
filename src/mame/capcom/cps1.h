// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

   Capcom CPS1/2 hardware

***************************************************************************/

#ifndef MAME_CAPCOM_CPS1_H
#define MAME_CAPCOM_CPS1_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "machine/timekpr.h"
#include "machine/timer.h"
#include "sound/msm5205.h"
#include "sound/okim6295.h"
#include "sound/qsound.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


// Video raw params
// measured clocks:
// CPS2(Guru): V = 59.6376Hz, H = 15,4445kHz *H is probably measured too low!
// CPS1 GNG: V = 59.61Hz
/* CPS1(Charles MacDonald):
    Pixel clock: 8.00 MHz
    Total pixel clocks per scanline: 512 clocks
    Horizontal sync pulse width : 36 clocks
    Horizontal display and blanking period: 476 clocks
    Frame size: 262 scanlines
    Refresh rate: 59.63 MHz.
*/
#define CPS_PIXEL_CLOCK  (XTAL(16'000'000)/2)

#define CPS_HTOTAL       (512)
#define CPS_HBEND        (64)
#define CPS_HBSTART      (448)

#define CPS_VTOTAL       (262)
#define CPS_VBEND        (16)
#define CPS_VBSTART      (240)


struct gfx_range
{
	// start and end are as passed by the game (shift adjusted to be all
	// in the same scale a 8x8 tiles): they don't necessarily match the
	// position in ROM.
	int type = 0;
	int start = 0;
	int end = 0;
	int bank = 0;
};

struct CPS1config
{
	const char *name = nullptr;             /* game driver name */

	/* Some games interrogate a couple of registers on bootup. */
	/* These are CPS1 board B self test checks. They wander from game to */
	/* game. */
	int cpsb_addr = 0;        /* CPS board B test register address */
	int cpsb_value = 0;       /* CPS board B test register expected value */

	/* some games use as a protection check the ability to do 16-bit multiplies */
	/* with a 32-bit result, by writing the factors to two ports and reading the */
	/* result from two other ports. */
	/* It looks like this feature was introduced with 3wonders (CPSB ID = 08xx) */
	int mult_factor1 = 0;
	int mult_factor2 = 0;
	int mult_result_lo = 0;
	int mult_result_hi = 0;

	/* unknown registers which might be related to the multiply protection */
	int unknown1 = 0;
	int unknown2 = 0;
	int unknown3 = 0;

	int layer_control = 0;
	int priority[4]{};
	int palette_control = 0;

	/* ideally, the layer enable masks should consist of only one bit, */
	/* but in many cases it is unknown which bit is which. */
	int layer_enable_mask[5]{};

	/* these depend on the B-board model and PAL */
	int bank_sizes[4]{};
	const struct gfx_range *bank_mapper;

	/* some C-boards have additional I/O for extra buttons/extra players */
	int in2_addr = 0;
	int in3_addr = 0;
	int out2_addr = 0;

	int bootleg_kludge = 0;
};


class cps_state : public driver_device
{
public:
	cps_state(const machine_config &mconfig, device_type type, const char *tag)
		: cps_state(mconfig, type, tag, 1)
	{ }

protected:
	cps_state(const machine_config &mconfig, device_type type, const char *tag, int version)
		: driver_device(mconfig, type, tag)
		, m_mainram(*this, "mainram")
		, m_gfxram(*this, "gfxram")
		, m_cps_a_regs(*this, "cps_a_regs")
		, m_cps_b_regs(*this, "cps_b_regs")
		, m_qsound_sharedram1(*this, "qsound_ram1")
		, m_qsound_sharedram2(*this, "qsound_ram2")
		, m_cps_version(version)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_m48t35(*this,"m48t35")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch")
		, m_soundlatch2(*this, "soundlatch2")
		, m_region_stars(*this, "stars")
		, m_led_cboard(*this, "led_cboard%u", 0U)
	{ }

public:
	void cps1_10MHz(machine_config &config);
	void forgottn(machine_config &config);
	void cps1_12MHz(machine_config &config);
	void pang3(machine_config &config);
	void ganbare(machine_config &config);
	void qsound(machine_config &config);
	void wofhfh(machine_config &config);
	void sf2m3(machine_config &config);
	void sf2cems6(machine_config &config);
	void sf2m10(machine_config &config);
	void varthb2(machine_config &config);
	void varthb3(machine_config &config);

	void init_cps1();
	void init_sf2ee();
	void init_wof();
	void init_dino();
	void init_punisher();
	void init_slammast();
	void init_pang3();
	void init_ganbare();
	void init_pang3b();
	void init_pang3b4();
	void init_sf2rb();
	void init_sf2rb2();
	void init_sf2thndr();
	void init_sf2hack();
	void init_sf2rk();
	void init_sf2dongb();
	void init_sf2ceblp();
	void init_sf2m8();
	void init_dinohunt();

protected:
	DECLARE_MACHINE_START(common);
	DECLARE_MACHINE_START(cps1);
	DECLARE_MACHINE_START(qsound);
	DECLARE_MACHINE_START(ganbare);
	DECLARE_MACHINE_RESET(cps);

	uint16_t cps1_dsw_r(offs_t offset);
	uint16_t cps1_in1_r();
	uint16_t cps1_in2_r();
	uint16_t cps1_in3_r();
	void cps1_coinctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cpsq_coinctrl2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cps1_cps_a_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cps1_cps_b_r(offs_t offset);
	void cps1_cps_b_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cps1_gfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cps1_soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cps1_soundlatch2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void cps1_snd_bankswitch_w(uint8_t data);
	void cps1_oki_pin7_w(uint8_t data);
	uint16_t qsound_rom_r(offs_t offset);
	uint16_t qsound_sharedram1_r(offs_t offset);
	void qsound_sharedram1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t qsound_sharedram2_r(offs_t offset);
	void qsound_sharedram2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void qsound_banksw_w(uint8_t data);
	uint16_t ganbare_ram_r(offs_t offset, uint16_t mem_mask = ~0);
	void ganbare_ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cps1_hack_dsw_r(offs_t offset);
	uint16_t sf2rb_prot_r(offs_t offset);
	uint16_t sf2rb2_prot_r(offs_t offset);
	uint16_t sf2dongb_prot_r(offs_t offset);
	uint16_t sf2ceblp_prot_r();
	void sf2ceblp_prot_w(uint16_t data);
	void sf2m3_layer_w(offs_t offset, uint16_t data);
	uint16_t dinohunt_sound_r();
	void varthb2_cps_a_w(offs_t offset, uint16_t data);
	uint16_t pang3b4_prot_r();
	void pang3b4_prot_w(uint16_t data);

	TILEMAP_MAPPER_MEMBER(tilemap0_scan);
	TILEMAP_MAPPER_MEMBER(tilemap1_scan);
	TILEMAP_MAPPER_MEMBER(tilemap2_scan);
	TILE_GET_INFO_MEMBER(get_tile0_info);
	TILE_GET_INFO_MEMBER(get_tile1_info);
	TILE_GET_INFO_MEMBER(get_tile2_info);
	virtual void video_start() override ATTR_COLD;

	INTERRUPT_GEN_MEMBER(cps1_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(ganbare_interrupt);

	virtual void render_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cps1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_cps1(int state);

	void kabuki_setup(void (*decode)(uint8_t *src, uint8_t *dst));

	/* maps */
	void cpu_space_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void forgottn_map(address_map &map) ATTR_COLD;
	void qsound_main_map(address_map &map) ATTR_COLD;
	void qsound_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void qsound_sub_map(address_map &map) ATTR_COLD;
	void sf2m3_map(address_map &map) ATTR_COLD;
	void sf2cems6_map(address_map &map) ATTR_COLD;
	void sf2m10_map(address_map &map) ATTR_COLD;
	void varthb2_map(address_map &map) ATTR_COLD;
	void varthb3_map(address_map &map) ATTR_COLD;

	// game-specific
	uint16_t m_sf2ceblp_prot = 0;
	uint16_t m_pang3b4_prot = 0;

	/* video-related */
	tilemap_t *m_bg_tilemap[3]{};
	int m_scanline1 = 0;
	int m_scanline2 = 0;
	int m_scancalls = 0;

	int m_scroll1x = 0;
	int m_scroll1y = 0;
	int m_scroll2x = 0;
	int m_scroll2y = 0;
	int m_scroll3x = 0;
	int m_scroll3y = 0;

	int m_stars_enabled[2]{};        /* Layer enabled [Y/N] */
	int m_stars1x = 0;
	int m_stars1y = 0;
	int m_stars2x = 0;
	int m_stars2y = 0;
	int m_last_sprite_offset = 0;      /* Offset of the last sprite */

	bitmap_ind16 m_dummy_bitmap;

	/* video config (never changed after video_start) */
	const struct CPS1config *m_game_config;
	int m_scroll_size = 0;
	int m_obj_size = 0;
	int m_other_size = 0;
	int m_palette_align = 0;
	int m_palette_size = 0;
	int m_stars_rom_size = 0;
	uint8_t m_empty_tile[32*32]{};

	/* video/cps1.cpp */
	inline uint16_t *cps1_base( int offset, int boundary );
	void cps1_get_video_base();
	int gfxrom_bank_mapper(int type, int code);
	void cps1_update_transmasks();
	void cps1_build_palette(const uint16_t* const palette_base);
	virtual void find_last_sprite();
	void cps1_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cps1_render_stars(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cps1_render_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int primask);
	void cps1_render_high_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);

	/* memory pointers */
	optional_shared_ptr<uint16_t> m_mainram;
	required_shared_ptr<uint16_t> m_gfxram;
	required_shared_ptr<uint16_t> m_cps_a_regs;
	required_shared_ptr<uint16_t> m_cps_b_regs;
	uint16_t * m_scroll1 = nullptr;
	uint16_t * m_scroll2 = nullptr;
	uint16_t * m_scroll3 = nullptr;
	uint16_t * m_obj = nullptr;
	uint16_t * m_other = nullptr;
	std::unique_ptr<uint16_t[]> m_buffered_obj;
	optional_shared_ptr<uint8_t> m_qsound_sharedram1;
	optional_shared_ptr<uint8_t> m_qsound_sharedram2;
	std::unique_ptr<uint8_t[]> m_decrypt_kabuki;
	int m_cps_version = 0;

	/* devices */
	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<m48t35_device> m_m48t35;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<generic_latch_8_device> m_soundlatch2;
	optional_memory_region m_region_stars;
	output_finder<3> m_led_cboard;
};


/*----------- defined in drivers/cps1.cpp -----------*/

extern gfx_decode_entry const gfx_cps1[];

INPUT_PORTS_EXTERN( dino );
INPUT_PORTS_EXTERN( knights );
INPUT_PORTS_EXTERN( mtwins );
INPUT_PORTS_EXTERN( punisher );
INPUT_PORTS_EXTERN( sf2 );
INPUT_PORTS_EXTERN( slammast );
INPUT_PORTS_EXTERN( varth );
INPUT_PORTS_EXTERN( captcomm );
INPUT_PORTS_EXTERN( wof );

#endif // MAME_CAPCOM_CPS1_H
