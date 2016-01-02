// license:???
// copyright-holders:Paul Leaman
/***************************************************************************

   Capcom CPS1/2 hardware

***************************************************************************/

#ifndef _CPS1_H_
#define _CPS1_H_

#include "sound/msm5205.h"
#include "sound/qsound.h"
#include "sound/okim6295.h"
#include "machine/timekpr.h"
#include "cpu/m68000/m68000.h"

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
#define CPS_PIXEL_CLOCK  (XTAL_16MHz/2)

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
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_gfxram(*this, "gfxram"),
		m_cps_a_regs(*this, "cps_a_regs"),
		m_cps_b_regs(*this, "cps_b_regs"),
		m_qsound_sharedram1(*this, "qsound_ram1"),
		m_qsound_sharedram2(*this, "qsound_ram2"),
		m_objram1(*this, "objram1"),
		m_objram2(*this, "objram2"),
		m_output(*this, "output"),
		m_io_in0(*this, "IN0"),
		m_io_in1(*this, "IN1"),
		m_cps2_dial_type(0),
		m_ecofghtr_dial_direction0(0),
		m_ecofghtr_dial_direction1(0),
		m_ecofghtr_dial_last0(0),
		m_ecofghtr_dial_last1(0),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_oki(*this, "oki"),
		m_m48t35(*this,"m48t35"),
		m_msm_1(*this, "msm1"),
		m_msm_2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes")
	{ }

	/* memory pointers */
	// cps1
	optional_shared_ptr<UINT16> m_mainram;
	required_shared_ptr<UINT16> m_gfxram;
	required_shared_ptr<UINT16> m_cps_a_regs;
	required_shared_ptr<UINT16> m_cps_b_regs;
	UINT16 *     m_scroll1;
	UINT16 *     m_scroll2;
	UINT16 *     m_scroll3;
	UINT16 *     m_obj;
	UINT16 *     m_other;
	std::unique_ptr<UINT16[]>     m_buffered_obj;
	optional_shared_ptr<UINT8> m_qsound_sharedram1;
	optional_shared_ptr<UINT8> m_qsound_sharedram2;
	std::unique_ptr<UINT8[]> m_decrypt_kabuki;
	// cps2
	optional_shared_ptr<UINT16> m_objram1;
	optional_shared_ptr<UINT16> m_objram2;
	optional_shared_ptr<UINT16> m_output;

	optional_ioport m_io_in0;
	optional_ioport m_io_in1;
	std::unique_ptr<UINT16[]>     m_cps2_buffered_obj;
	// game-specific
	std::unique_ptr<UINT16[]>    m_gigaman2_dummyqsound_ram;
	UINT16  sf2ceblp_prot;

	/* video-related */
	tilemap_t      *m_bg_tilemap[3];
	int          m_scanline1;
	int          m_scanline2;
	int          m_scancalls;

	int          m_scroll1x;
	int          m_scroll1y;
	int          m_scroll2x;
	int          m_scroll2y;
	int          m_scroll3x;
	int          m_scroll3y;

	int          m_stars_enabled[2];        /* Layer enabled [Y/N] */
	int          m_stars1x;
	int          m_stars1y;
	int          m_stars2x;
	int          m_stars2y;
	int          m_last_sprite_offset;      /* Offset of the last sprite */
	int          m_cps2_last_sprite_offset; /* Offset of the last sprite */
	int          m_pri_ctrl;                /* Sprite layer priorities */
	int          m_objram_bank;

	/* misc */
	int          m_dial[2];     // forgottn
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


	/* fcrash sound hw */
	int          m_sample_buffer1;
	int          m_sample_buffer2;
	int          m_sample_select1;
	int          m_sample_select2;

	/* video config (never changed after video_start) */
	const struct CPS1config *m_game_config;
	int          m_scroll_size;
	int          m_obj_size;
	int          m_cps2_obj_size;
	int          m_other_size;
	int          m_palette_align;
	int          m_palette_size;
	int          m_stars_rom_size;
	UINT8        m_empty_tile[32*32];
	int          m_cps_version;

	/* fcrash video config */
	UINT8        m_layer_enable_reg;
	UINT8        m_layer_mask_reg[4];
	int          m_layer_scroll1x_offset;
	int          m_layer_scroll2x_offset;
	int          m_layer_scroll3x_offset;
	int          m_sprite_base;
	int          m_sprite_list_end_marker;
	int          m_sprite_x_offset;
	UINT16       *m_bootleg_sprite_ram;
	UINT16       *m_bootleg_work_ram;

	/* devices */
	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<m48t35_device> m_m48t35;
	optional_device<msm5205_device> m_msm_1;    // fcrash
	optional_device<msm5205_device> m_msm_2;    // fcrash
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_shared_ptr<UINT16> m_decrypted_opcodes;

	DECLARE_READ16_MEMBER(cps1_hack_dsw_r);
	DECLARE_READ16_MEMBER(cps1_in1_r);
	DECLARE_READ16_MEMBER(cps1_in2_r);
	DECLARE_READ16_MEMBER(cps1_in3_r);
	DECLARE_READ16_MEMBER(forgottn_dial_0_r);
	DECLARE_READ16_MEMBER(forgottn_dial_1_r);
	DECLARE_WRITE16_MEMBER(forgottn_dial_0_reset_w);
	DECLARE_WRITE16_MEMBER(forgottn_dial_1_reset_w);
	DECLARE_WRITE8_MEMBER(cps1_snd_bankswitch_w);
	DECLARE_WRITE16_MEMBER(cps1_soundlatch_w);
	DECLARE_WRITE16_MEMBER(cps1_soundlatch2_w);
	DECLARE_WRITE16_MEMBER(cpsq_coinctrl2_w);
	DECLARE_READ16_MEMBER(qsound_rom_r);
	DECLARE_READ16_MEMBER(qsound_sharedram2_r);
	DECLARE_WRITE16_MEMBER(qsound_sharedram2_w);
	DECLARE_WRITE8_MEMBER(qsound_banksw_w);
	DECLARE_READ16_MEMBER(sf2rb_prot_r);
	DECLARE_READ16_MEMBER(sf2rb2_prot_r);
	DECLARE_READ16_MEMBER(sf2dongb_prot_r);
	DECLARE_READ16_MEMBER(sf2ceblp_prot_r);
	DECLARE_WRITE16_MEMBER(sf2ceblp_prot_w);
	DECLARE_READ16_MEMBER(cps1_dsw_r);
	DECLARE_WRITE16_MEMBER(cps1_coinctrl_w);
	DECLARE_READ16_MEMBER(qsound_sharedram1_r);
	DECLARE_WRITE16_MEMBER(qsound_sharedram1_w);
	DECLARE_READ16_MEMBER(ganbare_ram_r);
	DECLARE_WRITE16_MEMBER(ganbare_ram_w);
	DECLARE_WRITE16_MEMBER(cps1_cps_a_w);
	DECLARE_READ16_MEMBER(cps1_cps_b_r);
	DECLARE_WRITE16_MEMBER(cps1_cps_b_w);
	DECLARE_WRITE16_MEMBER(cps1_gfxram_w);
	DECLARE_WRITE16_MEMBER(cps2_objram_bank_w);
	DECLARE_READ16_MEMBER(cps2_objram1_r);
	DECLARE_READ16_MEMBER(cps2_objram2_r);
	DECLARE_WRITE16_MEMBER(cps2_objram1_w);
	DECLARE_WRITE16_MEMBER(cps2_objram2_w);
	DECLARE_WRITE8_MEMBER(cps1_oki_pin7_w);
	DECLARE_WRITE16_MEMBER(sf2m1_layer_w);
	DECLARE_WRITE16_MEMBER(sf2m3_layer_w);
	DECLARE_READ16_MEMBER(dinohunt_sound_r);
	DECLARE_DRIVER_INIT(sf2rb);
	DECLARE_DRIVER_INIT(sf2rb2);
	DECLARE_DRIVER_INIT(sf2thndr);
	DECLARE_DRIVER_INIT(dinohunt);
	DECLARE_DRIVER_INIT(forgottn);
	DECLARE_DRIVER_INIT(sf2hack);
	DECLARE_DRIVER_INIT(slammast);
	DECLARE_DRIVER_INIT(pang3b);
	DECLARE_DRIVER_INIT(pang3);
	DECLARE_DRIVER_INIT(sf2ee);
	DECLARE_DRIVER_INIT(sf2m8);
	DECLARE_DRIVER_INIT(cps1);
	DECLARE_DRIVER_INIT(dino);
	DECLARE_DRIVER_INIT(punisher);
	DECLARE_DRIVER_INIT(wof);
	DECLARE_DRIVER_INIT(ganbare);
	DECLARE_DRIVER_INIT(cps2_video);
	DECLARE_DRIVER_INIT(cps2);
	DECLARE_DRIVER_INIT(cps2nc);
	DECLARE_DRIVER_INIT(cps2crpt);
	DECLARE_DRIVER_INIT(ssf2tb);
	DECLARE_DRIVER_INIT(pzloop2);
	DECLARE_DRIVER_INIT(singbrd);
	DECLARE_DRIVER_INIT(gigaman2);
	DECLARE_DRIVER_INIT(ecofghtr);
	DECLARE_DRIVER_INIT(sf2dongb);
	DECLARE_DRIVER_INIT(sf2ceblp);
	TILEMAP_MAPPER_MEMBER(tilemap0_scan);
	TILEMAP_MAPPER_MEMBER(tilemap1_scan);
	TILEMAP_MAPPER_MEMBER(tilemap2_scan);
	TILE_GET_INFO_MEMBER(get_tile0_info);
	TILE_GET_INFO_MEMBER(get_tile1_info);
	TILE_GET_INFO_MEMBER(get_tile2_info);
	DECLARE_MACHINE_START(cps1);
	DECLARE_VIDEO_START(cps1);
	DECLARE_MACHINE_START(common);
	DECLARE_MACHINE_START(cps2);
	DECLARE_VIDEO_START(cps2);
	DECLARE_MACHINE_START(qsound);
	DECLARE_MACHINE_START(ganbare);
	DECLARE_MACHINE_RESET(cps);
	DECLARE_VIDEO_START(cps);
	DECLARE_MACHINE_START(sf2m1);
	UINT32 screen_update_cps1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_cps1(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(cps1_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(ganbare_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(cps2_interrupt);
	TIMER_CALLBACK_MEMBER(cps2_update_digital_volume);

	void kabuki_setup(void (*decode)(UINT8 *src, UINT8 *dst));

	/* fcrash handlers */
	DECLARE_DRIVER_INIT(kodb);
	DECLARE_DRIVER_INIT(cawingbl);
	DECLARE_DRIVER_INIT(dinopic);
	DECLARE_DRIVER_INIT(knightsb);
	DECLARE_DRIVER_INIT(punipic);
	DECLARE_DRIVER_INIT(punipic3);
	DECLARE_DRIVER_INIT(sf2m1);
	DECLARE_DRIVER_INIT(sf2mdt);
	DECLARE_DRIVER_INIT(sf2mdta);
	DECLARE_DRIVER_INIT(sf2mdtb);
	DECLARE_DRIVER_INIT(sf2b);
	DECLARE_DRIVER_INIT(slampic);
	DECLARE_MACHINE_START(fcrash);
	DECLARE_MACHINE_RESET(fcrash);
	DECLARE_MACHINE_START(cawingbl);
	DECLARE_MACHINE_START(dinopic);
	DECLARE_MACHINE_START(knightsb);
	DECLARE_MACHINE_START(kodb);
	DECLARE_MACHINE_START(punipic);
	DECLARE_MACHINE_START(sf2mdt);
	DECLARE_MACHINE_START(slampic);
	DECLARE_MACHINE_START(sgyxz);
	DECLARE_WRITE16_MEMBER(cawingbl_soundlatch_w);
	DECLARE_WRITE16_MEMBER(dinopic_layer_w);
	DECLARE_WRITE16_MEMBER(dinopic_layer2_w);
	DECLARE_WRITE16_MEMBER(knightsb_layer_w);
	DECLARE_WRITE16_MEMBER(kodb_layer_w);
	DECLARE_WRITE16_MEMBER(punipic_layer_w);
	DECLARE_WRITE16_MEMBER(sf2mdt_layer_w);
	DECLARE_WRITE16_MEMBER(sf2mdta_layer_w);
	DECLARE_WRITE16_MEMBER(slampic_layer_w);
	DECLARE_WRITE16_MEMBER(fcrash_soundlatch_w);
	DECLARE_WRITE8_MEMBER(fcrash_snd_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sf2mdt_snd_bankswitch_w);
	DECLARE_WRITE8_MEMBER(knightsb_snd_bankswitch_w);
	DECLARE_WRITE8_MEMBER(fcrash_msm5205_0_data_w);
	DECLARE_WRITE8_MEMBER(fcrash_msm5205_1_data_w);
	UINT32 screen_update_fcrash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fcrash_update_transmasks();
	void fcrash_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fcrash_render_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int primask);
	void fcrash_render_high_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);
	void fcrash_build_palette();


	/* cps video */
	inline UINT16 *cps1_base( int offset, int boundary );
	void cps1_get_video_base();
	void unshuffle(UINT64 *buf, int len);
	void cps2_gfx_decode();
	int gfxrom_bank_mapper(int type, int code);
	void cps1_update_transmasks();
	void cps1_build_palette(const UINT16* const palette_base);
	void cps1_find_last_sprite();
	void cps1_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cps2_find_last_sprite();
	void cps2_render_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int *primasks);
	void cps1_render_stars(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cps1_render_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int primask);
	void cps1_render_high_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);
	void cps2_set_sprite_priorities();
	void cps2_objram_latch();
	UINT16 *cps2_objbase();


	/* cps2 driver */
	void init_digital_volume();
	DECLARE_READ16_MEMBER(gigaman2_dummyqsound_r);
	DECLARE_WRITE16_MEMBER(gigaman2_dummyqsound_w);
	void gigaman2_gfx_reorder();
	DECLARE_WRITE16_MEMBER(cps2_eeprom_port_w);
	DECLARE_READ16_MEMBER(cps2_qsound_volume_r);
	DECLARE_READ16_MEMBER(kludge_r);
	DECLARE_READ16_MEMBER(joy_or_paddle_r);
	DECLARE_READ16_MEMBER(joy_or_paddle_ecofghtr_r);
	DECLARE_WRITE_LINE_MEMBER(m5205_int1);
	DECLARE_WRITE_LINE_MEMBER(m5205_int2);
};

/*----------- defined in drivers/cps1.c -----------*/

MACHINE_CONFIG_EXTERN(cps1_12MHz);

ADDRESS_MAP_EXTERN( qsound_sub_map, 8 );

GFXDECODE_EXTERN( cps1 );

INPUT_PORTS_EXTERN( dino );
INPUT_PORTS_EXTERN( knights );
INPUT_PORTS_EXTERN( punisher );
INPUT_PORTS_EXTERN( sf2 );
INPUT_PORTS_EXTERN( slammast );


#endif
