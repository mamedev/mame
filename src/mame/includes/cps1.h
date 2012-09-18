#ifndef _CPS1_H_
#define _CPS1_H_

#include "sound/msm5205.h"

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
		m_gfxram(*this, "gfxram"),
		m_cps_a_regs(*this, "cps_a_regs"),
		m_cps_b_regs(*this, "cps_b_regs"),
		m_qsound_sharedram1(*this, "qsound_ram1"),
		m_qsound_sharedram2(*this, "qsound_ram2"),
		m_objram1(*this, "objram1"),
		m_objram2(*this, "objram2"),
		m_output(*this, "output") { }

	/* memory pointers */
	// cps1
	required_shared_ptr<UINT16> m_gfxram;
	required_shared_ptr<UINT16> m_cps_a_regs;
	required_shared_ptr<UINT16> m_cps_b_regs;
	UINT16 *     m_scroll1;
	UINT16 *     m_scroll2;
	UINT16 *     m_scroll3;
	UINT16 *     m_obj;
	UINT16 *     m_other;
	UINT16 *     m_buffered_obj;
	optional_shared_ptr<UINT8> m_qsound_sharedram1;
	optional_shared_ptr<UINT8> m_qsound_sharedram2;
	// cps2
	optional_shared_ptr<UINT16> m_objram1;
	optional_shared_ptr<UINT16> m_objram2;
	optional_shared_ptr<UINT16> m_output;
	UINT16 *     m_cps2_buffered_obj;
	// game-specific
	UINT16 *     m_gigaman2_dummyqsound_ram;

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

	int          m_stars_enabled[2];		/* Layer enabled [Y/N] */
	int          m_stars1x;
	int          m_stars1y;
	int          m_stars2x;
	int          m_stars2y;
	int          m_last_sprite_offset;		/* Offset of the last sprite */
	int          m_cps2_last_sprite_offset;	/* Offset of the last sprite */
	int          m_pri_ctrl;				/* Sprite layer priorities */
	int          m_objram_bank;

	/* misc */
	int          m_dial[2];		// forgottn
	int          m_readpaddle;	// pzloop2
	int          m_cps2networkpresent;

	/* fcrash sound hw */
	int          m_sample_buffer1;
	int          m_sample_buffer2;
	int          m_sample_select1;
	int          m_sample_select2;

	/* video config (never changed after VIDEO_START) */
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

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	msm5205_device *m_msm_1;	// fcrash
	msm5205_device *m_msm_2;	// fcrash
	DECLARE_READ16_MEMBER(cps1_hack_dsw_r);
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
	DECLARE_READ16_MEMBER(sf2mdt_r);
	DECLARE_READ16_MEMBER(cps1_dsw_r);
	DECLARE_WRITE16_MEMBER(cps1_coinctrl_w);
	DECLARE_READ16_MEMBER(qsound_sharedram1_r);
	DECLARE_WRITE16_MEMBER(qsound_sharedram1_w);
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
	DECLARE_DRIVER_INIT(sf2thndr);
	DECLARE_DRIVER_INIT(dinohunt);
	DECLARE_DRIVER_INIT(forgottn);
	DECLARE_DRIVER_INIT(sf2hack);
	DECLARE_DRIVER_INIT(slammast);
	DECLARE_DRIVER_INIT(pang3b);
	DECLARE_DRIVER_INIT(pang3);
	DECLARE_DRIVER_INIT(sf2ee);
	DECLARE_DRIVER_INIT(cps1);
	DECLARE_DRIVER_INIT(sf2mdt);
	DECLARE_DRIVER_INIT(dino);
	DECLARE_DRIVER_INIT(punisher);
	DECLARE_DRIVER_INIT(wof);
	DECLARE_DRIVER_INIT(cps2_video);
	DECLARE_DRIVER_INIT(cps2);
	DECLARE_DRIVER_INIT(cps2crpt);
	DECLARE_DRIVER_INIT(ssf2tb);
	DECLARE_DRIVER_INIT(pzloop2);
	DECLARE_DRIVER_INIT(gigaman2);
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
	DECLARE_MACHINE_START(fcrash);
	DECLARE_MACHINE_RESET(fcrash);
	DECLARE_MACHINE_START(kodb);
	DECLARE_MACHINE_START(qsound);
	DECLARE_MACHINE_RESET(cps);
	DECLARE_VIDEO_START(cps);
	UINT32 screen_update_cps1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_fcrash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kodb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_cps1(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(cps1_interrupt);
	INTERRUPT_GEN_MEMBER(cps1_qsound_interrupt);
};

/*----------- defined in drivers/cps1.c -----------*/

ADDRESS_MAP_EXTERN( qsound_sub_map, 8 );

GFXDECODE_EXTERN( cps1 );


/*----------- defined in video/cps1.c -----------*/
void cps1_get_video_base(running_machine &machine);
void cps2_set_sprite_priorities(running_machine &machine);
void cps2_objram_latch(running_machine &machine);

/*************************************
 *  Encryption
 *************************************/

/*----------- defined in machine/kabuki.c -----------*/

void mgakuen2_decode(running_machine &machine);
void pang_decode(running_machine &machine);
void cworld_decode(running_machine &machine);
void hatena_decode(running_machine &machine);
void spang_decode(running_machine &machine);
void spangj_decode(running_machine &machine);
void sbbros_decode(running_machine &machine);
void marukin_decode(running_machine &machine);
void qtono1_decode(running_machine &machine);
void qsangoku_decode(running_machine &machine);
void block_decode(running_machine &machine);

void wof_decode(running_machine &machine);
void dino_decode(running_machine &machine);
void punisher_decode(running_machine &machine);
void slammast_decode(running_machine &machine);

#endif
