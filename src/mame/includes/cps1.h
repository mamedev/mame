#ifndef _CPS1_H_
#define _CPS1_H_

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


class cps_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, cps_state(machine)); }

	cps_state(running_machine &machine) { }

	/* memory pointers */
	// cps1
	UINT16 *     gfxram;
	UINT16 *     cps_a_regs;
	UINT16 *     cps_b_regs;
	UINT16 *     scroll1;
	UINT16 *     scroll2;
	UINT16 *     scroll3;
	UINT16 *     obj;
	UINT16 *     other;
	UINT16 *     buffered_obj;
	UINT8  *     qsound_sharedram1;
	UINT8  *     qsound_sharedram2;
	size_t       gfxram_size;
	// cps2
	UINT16 *     objram1;
	UINT16 *     objram2;
	UINT16 *     output;
	UINT16 *     cps2_buffered_obj;
	size_t       output_size;
	// game-specific
	UINT16 *     gigamn2_dummyqsound_ram;

	/* video-related */
	tilemap_t      *bg_tilemap[3];
	int          scanline1;
	int          scanline2;
	int          scancalls;
	int          scancount;

	int          scroll1x, scroll1y;
	int          scroll2x, scroll2y;
	int          scroll3x, scroll3y;

	int          stars_enabled[2];		/* Layer enabled [Y/N] */
	int          stars1x, stars1y, stars2x, stars2y;
	int          last_sprite_offset;		/* Offset of the last sprite */
	int          cps2_last_sprite_offset;	/* Offset of the last sprite */
	int          pri_ctrl;				/* Sprite layer priorities */
	int          objram_bank;

	/* misc */
	int          dial[2];		// forgottn
	int          readpaddle;	// pzloop2
	int          cps2networkpresent;

	/* fcrash sound hw */
	int          sample_buffer1, sample_buffer2;
	int          sample_select1, sample_select2;

	/* video config (never changed after VIDEO_START) */
	const struct CPS1config *game_config;
	int          scroll_size;
	int          obj_size;
	int          cps2_obj_size;
	int          other_size;
	int          palette_align;
	int          palette_size;
	int          stars_rom_size;
	UINT8        empty_tile8x8[8*8];
	UINT8        empty_tile[32*32/2];
	int          cps_version;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *msm_1;	// fcrash
	running_device *msm_2;	// fcrash
};

/*----------- defined in drivers/cps1.c -----------*/

ADDRESS_MAP_EXTERN( qsound_sub_map, 8 );

READ16_HANDLER( qsound_sharedram1_r );
WRITE16_HANDLER( qsound_sharedram1_w );

READ16_HANDLER( cps1_dsw_r );
WRITE16_HANDLER( cps1_coinctrl_w );
INTERRUPT_GEN( cps1_interrupt );

GFXDECODE_EXTERN( cps1 );


/*----------- defined in video/cps1.c -----------*/

WRITE16_HANDLER( cps1_cps_a_w );
WRITE16_HANDLER( cps1_cps_b_w );
READ16_HANDLER( cps1_cps_b_r );
WRITE16_HANDLER( cps1_gfxram_w );

DRIVER_INIT( cps1 );
DRIVER_INIT( cps2_video );

WRITE16_HANDLER( cps2_objram_bank_w );
READ16_HANDLER( cps2_objram1_r );
READ16_HANDLER( cps2_objram2_r );
WRITE16_HANDLER( cps2_objram1_w );
WRITE16_HANDLER( cps2_objram2_w );

VIDEO_START( cps1 );
VIDEO_START( cps2 );
VIDEO_UPDATE( cps1 );
VIDEO_EOF( cps1 );

void cps1_get_video_base(running_machine *machine);
void cps2_set_sprite_priorities(running_machine *machine);
void cps2_objram_latch(running_machine *machine);


/*************************************
 *  Encryption
 *************************************/

/*----------- defined in machine/cps2crpt.c -----------*/

DRIVER_INIT( cps2crpt );


/*----------- defined in machine/kabuki.c -----------*/

void mgakuen2_decode(running_machine *machine);
void pang_decode(running_machine *machine);
void cworld_decode(running_machine *machine);
void hatena_decode(running_machine *machine);
void spang_decode(running_machine *machine);
void spangj_decode(running_machine *machine);
void sbbros_decode(running_machine *machine);
void marukin_decode(running_machine *machine);
void qtono1_decode(running_machine *machine);
void qsangoku_decode(running_machine *machine);
void block_decode(running_machine *machine);

void wof_decode(running_machine *machine);
void dino_decode(running_machine *machine);
void punisher_decode(running_machine *machine);
void slammast_decode(running_machine *machine);

#endif
