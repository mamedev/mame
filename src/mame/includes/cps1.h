// license:BSD-3-Clause
// copyright-holders:Paul Leaman
/***************************************************************************

   Capcom CPS1/2 hardware

***************************************************************************/

#ifndef MAME_INCLUDES_CPS1_H
#define MAME_INCLUDES_CPS1_H

#pragma once

#include "sound/msm5205.h"
#include "sound/qsound.h"
#include "sound/okim6295.h"
#include "machine/gen_latch.h"
#include "machine/74157.h"
#include "machine/timekpr.h"
#include "machine/timer.h"
#include "cpu/m68000/m68000.h"
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
	int type;
	int start;
	int end;
	int bank;
};

struct CPS1config
{
	const char *name;             /* game driver name */

	/* Some games interrogate a couple of registers on bootup. */
	/* These are CPS1 board B self test checks. They wander from game to */
	/* game. */
	int cpsb_addr;        /* CPS board B test register address */
	int cpsb_value;       /* CPS board B test register expected value */

	/* some games use as a protection check the ability to do 16-bit multiplies */
	/* with a 32-bit result, by writing the factors to two ports and reading the */
	/* result from two other ports. */
	/* It looks like this feature was introduced with 3wonders (CPSB ID = 08xx) */
	int mult_factor1;
	int mult_factor2;
	int mult_result_lo;
	int mult_result_hi;

	/* unknown registers which might be related to the multiply protection */
	int unknown1;
	int unknown2;
	int unknown3;

	int layer_control;
	int priority[4];
	int palette_control;

	/* ideally, the layer enable masks should consist of only one bit, */
	/* but in many cases it is unknown which bit is which. */
	int layer_enable_mask[5];

	/* these depend on the B-board model and PAL */
	int bank_sizes[4];
	const struct gfx_range *bank_mapper;

	/* some C-boards have additional I/O for extra buttons/extra players */
	int in2_addr;
	int in3_addr;
	int out2_addr;

	int bootleg_kludge;
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
	
	void init_cps1();
	void init_sf2ee();
	void init_wof();
	void init_dino();
	void init_punisher();
	void init_slammast();
	void init_pang3();
	void init_ganbare();
	void init_pang3b();
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
	
	DECLARE_READ16_MEMBER(cps1_dsw_r);
	DECLARE_READ16_MEMBER(cps1_in1_r);
	DECLARE_READ16_MEMBER(cps1_in2_r);
	DECLARE_READ16_MEMBER(cps1_in3_r);
	DECLARE_WRITE16_MEMBER(cps1_coinctrl_w);
	DECLARE_WRITE16_MEMBER(cpsq_coinctrl2_w);
	DECLARE_WRITE16_MEMBER(cps1_cps_a_w);
	DECLARE_READ16_MEMBER(cps1_cps_b_r);
	DECLARE_WRITE16_MEMBER(cps1_cps_b_w);
	DECLARE_WRITE16_MEMBER(cps1_gfxram_w);
	DECLARE_WRITE16_MEMBER(cps1_soundlatch_w);
	DECLARE_WRITE16_MEMBER(cps1_soundlatch2_w);
	DECLARE_WRITE8_MEMBER(cps1_snd_bankswitch_w);
	DECLARE_WRITE8_MEMBER(cps1_oki_pin7_w);
	DECLARE_READ16_MEMBER(qsound_rom_r);
	DECLARE_READ16_MEMBER(qsound_sharedram1_r);
	DECLARE_WRITE16_MEMBER(qsound_sharedram1_w);
	DECLARE_READ16_MEMBER(qsound_sharedram2_r);
	DECLARE_WRITE16_MEMBER(qsound_sharedram2_w);
	DECLARE_WRITE8_MEMBER(qsound_banksw_w);
	DECLARE_READ16_MEMBER(ganbare_ram_r);
	DECLARE_WRITE16_MEMBER(ganbare_ram_w);
	DECLARE_READ16_MEMBER(cps1_hack_dsw_r);
	DECLARE_READ16_MEMBER(sf2rb_prot_r);
	DECLARE_READ16_MEMBER(sf2rb2_prot_r);
	DECLARE_READ16_MEMBER(sf2dongb_prot_r);
	DECLARE_READ16_MEMBER(sf2ceblp_prot_r);
	DECLARE_WRITE16_MEMBER(sf2ceblp_prot_w);
	DECLARE_WRITE16_MEMBER(sf2m3_layer_w);
	DECLARE_READ16_MEMBER(dinohunt_sound_r);
	
	TILEMAP_MAPPER_MEMBER(tilemap0_scan);
	TILEMAP_MAPPER_MEMBER(tilemap1_scan);
	TILEMAP_MAPPER_MEMBER(tilemap2_scan);
	TILE_GET_INFO_MEMBER(get_tile0_info);
	TILE_GET_INFO_MEMBER(get_tile1_info);
	TILE_GET_INFO_MEMBER(get_tile2_info);
	virtual void video_start() override;
	
	INTERRUPT_GEN_MEMBER(cps1_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(ganbare_interrupt);
	
	virtual void render_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cps1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_cps1);
	
	void kabuki_setup(void (*decode)(uint8_t *src, uint8_t *dst));
	
	/* maps */
	void cpu_space_map(address_map &map);
	void main_map(address_map &map);
	void forgottn_map(address_map &map);
	void qsound_main_map(address_map &map);
	void qsound_decrypted_opcodes_map(address_map &map);
	void sub_map(address_map &map);
	void qsound_sub_map(address_map &map);
	void sf2m3_map(address_map &map);
	void sf2cems6_map(address_map &map);
	void sf2m10_map(address_map &map);
	
	// game-specific
	uint16_t sf2ceblp_prot;

	/* video-related */
	tilemap_t *m_bg_tilemap[3];
	int m_scanline1;
	int m_scanline2;
	int m_scancalls;

	int m_scroll1x;
	int m_scroll1y;
	int m_scroll2x;
	int m_scroll2y;
	int m_scroll3x;
	int m_scroll3y;

	int m_stars_enabled[2];        /* Layer enabled [Y/N] */
	int m_stars1x;
	int m_stars1y;
	int m_stars2x;
	int m_stars2y;
	int m_last_sprite_offset;      /* Offset of the last sprite */

	bitmap_ind16 m_dummy_bitmap;

	/* video config (never changed after video_start) */
	const struct CPS1config *m_game_config;
	int m_scroll_size;
	int m_obj_size;
	int m_other_size;
	int m_palette_align;
	int m_palette_size;
	int m_stars_rom_size;
	uint8_t m_empty_tile[32*32];
	
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
	uint16_t * m_scroll1;
	uint16_t * m_scroll2;
	uint16_t * m_scroll3;
	uint16_t * m_obj;
	uint16_t * m_other;
	std::unique_ptr<uint16_t[]> m_buffered_obj;
	optional_shared_ptr<uint8_t> m_qsound_sharedram1;
	optional_shared_ptr<uint8_t> m_qsound_sharedram2;
	std::unique_ptr<uint8_t[]> m_decrypt_kabuki;
	int m_cps_version;

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

class cps2_state : public cps_state
{
public:
	cps2_state(const machine_config &mconfig, device_type type, const char *tag)
		: cps_state(mconfig, type, tag, 2)
		, m_decrypted_opcodes(*this, "decrypted_opcodes")
		, m_region_key(*this, "key")
		, m_qsound(*this, "qsound")
		, m_objram1(*this, "objram1")
		, m_objram2(*this, "objram2")
		, m_output(*this, "output")
		, m_io_in0(*this, "IN0")
		, m_io_in1(*this, "IN1")
		, m_cps2_dial_type(0)
		, m_ecofghtr_dial_direction0(0)
		, m_ecofghtr_dial_direction1(0)
		, m_ecofghtr_dial_last0(0)
		, m_ecofghtr_dial_last1(0)
	{ }

	void cps2(machine_config &config);
	void gigaman2(machine_config &config);
	void dead_cps2(machine_config &config);
	void init_cps2_video();
	void init_cps2();
	void init_cps2nc();
	void init_cps2crypt();
	void init_gigaman2();
	void init_ssf2tb();
	void init_pzloop2();
	void init_singbrd();
	void init_ecofghtr();

private:
	void init_digital_volume();
	DECLARE_READ16_MEMBER(gigaman2_dummyqsound_r);
	DECLARE_WRITE16_MEMBER(gigaman2_dummyqsound_w);
	void gigaman2_gfx_reorder();
	DECLARE_WRITE16_MEMBER(cps2_eeprom_port_w);
	DECLARE_READ16_MEMBER(cps2_qsound_volume_r);
	DECLARE_READ16_MEMBER(kludge_r);
	DECLARE_READ16_MEMBER(joy_or_paddle_r);
	DECLARE_READ16_MEMBER(joy_or_paddle_ecofghtr_r);
	TIMER_DEVICE_CALLBACK_MEMBER(cps2_interrupt);
	TIMER_CALLBACK_MEMBER(cps2_update_digital_volume);

	DECLARE_WRITE16_MEMBER(cps2_objram_bank_w);
	DECLARE_READ16_MEMBER(cps2_objram1_r);
	DECLARE_READ16_MEMBER(cps2_objram2_r);
	DECLARE_WRITE16_MEMBER(cps2_objram1_w);
	DECLARE_WRITE16_MEMBER(cps2_objram2_w);

	void unshuffle(uint64_t *buf, int len);
	void cps2_gfx_decode();
	virtual void find_last_sprite() override;
	void cps2_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int *primasks);
	void cps2_set_sprite_priorities();
	void cps2_objram_latch();
	uint16_t *cps2_objbase();
	virtual void render_layers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	uint32_t screen_update_cps2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_MACHINE_START(cps2);
	virtual void video_start() override;

	void cps2_map(address_map &map);
	void dead_cps2_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);

	optional_shared_ptr<uint16_t> m_decrypted_opcodes;
	optional_memory_region m_region_key;

	optional_device<qsound_device> m_qsound;

	required_shared_ptr<uint16_t> m_objram1;
	required_shared_ptr<uint16_t> m_objram2;
	required_shared_ptr<uint16_t> m_output;

	optional_ioport m_io_in0;
	optional_ioport m_io_in1;

	std::unique_ptr<uint16_t[]> m_cps2_buffered_obj;
	std::unique_ptr<uint16_t[]> m_gigaman2_dummyqsound_ram;

	/* video-related */
	int          m_cps2_last_sprite_offset; /* Offset of the last sprite */
	int          m_pri_ctrl;                /* Sprite layer priorities */
	int          m_objram_bank;
	int          m_cps2_obj_size;

	/* misc */
	int          m_readpaddle;  // pzloop2
	int          m_cps2networkpresent;
	int          m_cps2digitalvolumelevel;
	int          m_cps2disabledigitalvolume;
	emu_timer    *m_digital_volume_timer;
	int          m_cps2_dial_type;
	int          m_ecofghtr_dial_direction0;
	int          m_ecofghtr_dial_direction1;
	int          m_ecofghtr_dial_last0;
	int          m_ecofghtr_dial_last1;
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

#endif // MAME_INCLUDES_CPS1_H
