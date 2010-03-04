#include "sound/discrete.h"

/*
 * From the schematics:
 *
 * XTAL is 61,44 MHZ. There is some oscillator logic around it. The oscillating circuit
 * transfers the signal with a transformator. Onwards, it is fed through a M(B/C)10136. This
 * is a programmable counter which is used as a divisor by 5.
 * Cascaded 74LS161 further divide the signal. The following signals are generated:
 * 1/2H: 61,44MHZ/5/2 - pixel clock
 * 1H  : 61,44MHZ/5/4 - cpu-clock
 * 2H  : 61,44MHZ/5/8
 * ....
 * 128H: 61,44MHZ/5/512
 * The horizontal circuit counts till 384=256+128, thus 256H only being high for 128H/2
 *
 * Signal 16H,32H,64H and 256H are combined using a LS00, LS04 and a D-Flipflop to produce
 * a signal with Freq 16H/12. This is only possible because a 220pf capacitor with the
 * impedance of the LS-Family of 10K delays the 16H signal by about half a cycle.
 * This signal is divided by two by another D-Flipflop(74LS74) to give:
 * 1VF: 61,44MHZ/5/64/12/2 = 8KHZ
 * 2VF: 1VF/2 - Noise frequency: 4Khz
 * ...
 * The vertical circuit counts from 248 till 512 giving 264 lines.
 * 256VF is not being used, so counting is from 248...255, 0...255, ....
 */

#define MASTER_CLOCK			XTAL_61_44MHz
#define CLOCK_1H				(MASTER_CLOCK / 5 / 4)
#define CLOCK_16H				(CLOCK_1H / 16)
#define CLOCK_1VF				((CLOCK_16H) / 12 / 2)
#define CLOCK_2VF				((CLOCK_1VF) / 2)

#define PIXEL_CLOCK				(MASTER_CLOCK/10)
#define HTOTAL					(384)
#define HBSTART					(256)
#define HBEND					(0)
#define VTOTAL					(264)
#define VBSTART					(240)
#define VBEND					(16)

#define I8035_CLOCK				(XTAL_6MHz)

/****************************************************************************
 * CONSTANTS
 ****************************************************************************/

#define HARDWARE_TYPE_TAG		"HARDWARE_TYPE"

enum
{
	HARDWARE_TKG04 = 0,
	HARDWARE_TRS01,
	HARDWARE_TRS02,
	HARDWARE_TKG02
};

enum
{
	DK2650_HERBIEDK = 0,
	DK2650_HUNCHBKD,
	DK2650_EIGHTACT,
	DK2650_SHOOTGAL,
	DK2650_SPCLFORC
};

#define DK2B_PALETTE_LENGTH		(256+256+8+1) /*  (256) */
#define DK4B_PALETTE_LENGTH		(256+256+8+1) /*  (256) */
#define DK3_PALETTE_LENGTH		(256+256+8+1) /*  (256) */
#define RS_PALETTE_LENGTH		(256+256+8+1)

class dkong_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, dkong_state(machine)); }

	dkong_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *           video_ram;
	UINT8 *           sprite_ram;
	size_t            sprite_ram_size;

	/* devices */
	running_device *dev_n2a03a;
	running_device *dev_n2a03b;
	running_device *dev_vp2;		/* virtual port 2 */
	running_device *dev_6h;

#if 0
	/* machine states */
	UINT8	hardware_type;

	/* sound state */
	const UINT8 *			snd_rom;

	/* video state */
	tilemap_t *bg_tilemap;

	bitmap_t *		bg_bits;
	const UINT8 *	color_codes;
	emu_timer *		scanline_timer;
	INT8			vidhw;			/* Selected video hardware RS Conversion / TKG04 */

	/* radar scope */

	UINT8 *			gfx4;
	UINT8 *			gfx3;
	int				gfx3_len;

	UINT8	sig30Hz;
	UINT8	grid_sig;
	UINT8	rflip_sig;
	UINT8	star_ff;
	UINT8	blue_level;
	double	cd4049_a;
	double	cd4049_b;

	/* Specific states */
	INT8 decrypt_counter;

	/* 2650 protection */
	UINT8 protect_type;
	UINT8 hunchloopback;
	UINT8 prot_cnt;
	UINT8 main_fo;

	/* Save state relevant */
	UINT8	gfx_bank, palette_bank;
	UINT8	grid_on;
	UINT16	grid_col;
	UINT8	sprite_bank;
	UINT8	dma_latch;
	UINT8	flip;

	/* reverse address lookup map - hunchbkd */
	INT16 rev_map[0x200];
#endif
	/* machine states */
	UINT8	            hardware_type;

	/* sound state */
	const UINT8       *snd_rom;

	/* video state */
	tilemap_t           *bg_tilemap;

	bitmap_t          *bg_bits;
	const UINT8 *     color_codes;
	emu_timer *       scanline_timer;
	INT8              vidhw;			/* Selected video hardware RS Conversion / TKG04 */

	/* radar scope */

	UINT8 *           gfx4;
	UINT8 *           gfx3;
	int               gfx3_len;

	UINT8             sig30Hz;
	UINT8             grid_sig;
	UINT8             rflip_sig;
	UINT8             star_ff;
	UINT8             blue_level;
	double            cd4049_a;
	double            cd4049_b;

	/* Specific states */
	INT8              decrypt_counter;

	/* 2650 protection */
	UINT8             protect_type;
	UINT8             hunchloopback;
	UINT8             prot_cnt;
	UINT8             main_fo;

	/* Save state relevant */
	UINT8             gfx_bank, palette_bank;
	UINT8             grid_on;
	UINT16	      grid_col;
	UINT8             sprite_bank;
	UINT8             dma_latch;
	UINT8             flip;

	/* reverse address lookup map - hunchbkd */
	INT16             rev_map[0x200];
};

/*----------- defined in video/dkong.c -----------*/

WRITE8_HANDLER( radarscp_grid_enable_w );
WRITE8_HANDLER( radarscp_grid_color_w );
WRITE8_HANDLER( dkong_flipscreen_w );
WRITE8_HANDLER( dkongjr_gfxbank_w );
WRITE8_HANDLER( dkong3_gfxbank_w );
WRITE8_HANDLER( dkong_spritebank_w );
WRITE8_HANDLER( dkong_palettebank_w );

WRITE8_HANDLER( dkong_videoram_w );

PALETTE_INIT( dkong2b );
PALETTE_INIT( radarscp );
PALETTE_INIT( radarscp1 );
PALETTE_INIT( dkong3 );

VIDEO_START( dkong );
VIDEO_UPDATE( dkong );
VIDEO_UPDATE( pestplce );
VIDEO_UPDATE( spclforc );

/*----------- defined in audio/dkong.c -----------*/

WRITE8_HANDLER( dkong_audio_irq_w );

MACHINE_DRIVER_EXTERN( radarscp_audio );
MACHINE_DRIVER_EXTERN( dkong2b_audio );
MACHINE_DRIVER_EXTERN( dkongjr_audio );
MACHINE_DRIVER_EXTERN( dkong3_audio );
MACHINE_DRIVER_EXTERN( radarscp1_audio );

