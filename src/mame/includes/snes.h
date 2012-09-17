#ifndef _SNES_H_
#define _SNES_H_

#include "devlegcy.h"
#include "devcb.h"
#include "cpu/spc700/spc700.h"
#include "cpu/g65816/g65816.h"
#include "cpu/upd7725/upd7725.h"
#include "audio/snes_snd.h"

/*
    SNES timing theory:

    the master clock drives the CPU and PPU
    4  MC ticks = 1 PPU dot
    6  MC ticks = 1 65816 cycle for 3.58 MHz (3.579545)
    8  MC ticks = 1 65816 cycle for 2.68 MHz (2.684659)
    12 MC ticks = 1 65816 cycle for 1.78 MHz (1.789772)

    Each scanline has 341 readable positions and 342 actual dots.
    This is because 2 dots are "long" dots that last 6 MC ticks, resulting in 1 extra dot per line.
*/

#define MCLK_NTSC	(21477272)	/* verified */
#define MCLK_PAL	(21218370)	/* verified */

#define DOTCLK_NTSC	(MCLK_NTSC/4)
#define DOTCLK_PAL	(MCLK_PAL/4)

#define SNES_LAYER_DEBUG  0

#define SNES_DSP1B_OFFSET (0x0000)
#define SNES_DSP1_OFFSET  (0x3000)
#define SNES_DSP2_OFFSET  (0x6000)
#define SNES_DSP3_OFFSET  (0x9000)
#define SNES_DSP4_OFFSET  (0xc000)

/* Debug definitions */
#ifdef MAME_DEBUG
/* #define SNES_DBG_GENERAL*/		/* Display general debug info */
/* #define SNES_DBG_VIDEO */		/* Display video debug info */
/* #define SNES_DBG_DMA*/			/* Display DMA debug info */
/* #define SNES_DBG_HDMA*/			/* Display HDMA debug info */
/* #define SNES_DBG_REG_R*/			/* Display register read info */
/* #define SNES_DBG_REG_W*/			/* Display register write info */
#endif /* MAME_DEBUG */

/* Useful definitions */
#define SNES_SCR_WIDTH        256		/* 32 characters 8 pixels wide */
#define SNES_SCR_HEIGHT_NTSC  225		/* Can be 224 or 240 height */
#define SNES_SCR_HEIGHT_PAL   240		/* ??? */
#define SNES_VTOTAL_NTSC      262		/* Maximum number of lines for NTSC systems */
#define SNES_VTOTAL_PAL       312		/* Maximum number of lines for PAL systems */
#define SNES_HTOTAL           341		/* Maximum number pixels per line (incl. hblank) */
#define SNES_DMA_BASE         0x4300	/* Base DMA register address */
#define SNES_MODE_20          0x01		/* Lo-ROM cart */
#define SNES_MODE_21          0x02		/* Hi-ROM cart */
#define SNES_MODE_22          0x04		/* Extended Lo-ROM cart - SDD-1 */
#define SNES_MODE_25          0x08		/* Extended Hi-ROM cart */
#define SNES_MODE_BSX         0x10
#define SNES_MODE_BSLO        0x20
#define SNES_MODE_BSHI        0x40
#define SNES_MODE_ST          0x80
#define SNES_NTSC             0x00
#define SNES_PAL              0x10
#define SNES_VRAM_SIZE        0x20000	/* 128kb of video ram */
#define SNES_CGRAM_SIZE       0x202		/* 256 16-bit colours + 1 tacked on 16-bit colour for fixed colour */
#define SNES_OAM_SIZE         0x440		/* 1088 bytes of Object Attribute Memory */
#define SNES_EXROM_START      0x1000000
#define FIXED_COLOUR          256		/* Position in cgram for fixed colour */
/* Definitions for PPU Memory-Mapped registers */
#define INIDISP        0x2100
#define OBSEL          0x2101
#define OAMADDL        0x2102
#define OAMADDH        0x2103
#define OAMDATA        0x2104
#define BGMODE         0x2105	/* abcdefff = abcd: bg4-1 tile size | e: BG3 high priority | f: mode */
#define MOSAIC         0x2106	/* xxxxabcd = x: pixel size | abcd: affects bg 1-4 */
#define BG1SC          0x2107
#define BG2SC          0x2108
#define BG3SC          0x2109
#define BG4SC          0x210A
#define BG12NBA        0x210B
#define BG34NBA        0x210C
#define BG1HOFS        0x210D
#define BG1VOFS        0x210E
#define BG2HOFS        0x210F
#define BG2VOFS        0x2110
#define BG3HOFS        0x2111
#define BG3VOFS        0x2112
#define BG4HOFS        0x2113
#define BG4VOFS        0x2114
#define VMAIN          0x2115	/* i---ffrr = i: Increment timing | f: Full graphic | r: increment rate */
#define VMADDL         0x2116	/* aaaaaaaa = a: LSB of vram address */
#define VMADDH         0x2117	/* aaaaaaaa = a: MSB of vram address */
#define VMDATAL        0x2118	/* dddddddd = d: data to be written */
#define VMDATAH        0x2119	/* dddddddd = d: data to be written */
#define M7SEL          0x211A	/* ab----yx = a: screen over | y: vertical flip | x: horizontal flip */
#define M7A            0x211B	/* aaaaaaaa = a: COSINE rotate angle / X expansion */
#define M7B            0x211C	/* aaaaaaaa = a: SINE rotate angle / X expansion */
#define M7C            0x211D	/* aaaaaaaa = a: SINE rotate angle / Y expansion */
#define M7D            0x211E	/* aaaaaaaa = a: COSINE rotate angle / Y expansion */
#define M7X            0x211F
#define M7Y            0x2120
#define CGADD          0x2121
#define CGDATA         0x2122
#define W12SEL         0x2123
#define W34SEL         0x2124
#define WOBJSEL        0x2125
#define WH0            0x2126	/* pppppppp = p: Left position of window 1 */
#define WH1            0x2127	/* pppppppp = p: Right position of window 1 */
#define WH2            0x2128	/* pppppppp = p: Left position of window 2 */
#define WH3            0x2129	/* pppppppp = p: Right position of window 2 */
#define WBGLOG         0x212A	/* aabbccdd = a: BG4 params | b: BG3 params | c: BG2 params | d: BG1 params */
#define WOBJLOG        0x212B	/* ----ccoo = c: Colour window params | o: Object window params */
#define TM             0x212C
#define TS             0x212D
#define TMW            0x212E
#define TSW            0x212F
#define CGWSEL         0x2130
#define CGADSUB        0x2131
#define COLDATA        0x2132
#define SETINI         0x2133
#define MPYL           0x2134
#define MPYM           0x2135
#define MPYH           0x2136
#define SLHV           0x2137
#define ROAMDATA       0x2138
#define RVMDATAL       0x2139
#define RVMDATAH       0x213A
#define RCGDATA        0x213B
#define OPHCT          0x213C
#define OPVCT          0x213D
#define STAT77         0x213E
#define STAT78         0x213F
#define APU00          0x2140
#define APU01          0x2141
#define APU02          0x2142
#define APU03          0x2143
#define WMDATA         0x2180
#define WMADDL         0x2181
#define WMADDM         0x2182
#define WMADDH         0x2183
/* Definitions for CPU Memory-Mapped registers */
#define OLDJOY1        0x4016
#define OLDJOY2        0x4017
#define NMITIMEN       0x4200
#define WRIO           0x4201
//#define WRMPYA         0x4202
//#define WRMPYB         0x4203
//#define WRDIVL         0x4204
//#define WRDIVH         0x4205
//#define WRDVDD         0x4206
#define HTIMEL         0x4207
#define HTIMEH         0x4208
#define VTIMEL         0x4209
#define VTIMEH         0x420A
#define MDMAEN         0x420B
#define HDMAEN         0x420C
//#define MEMSEL         0x420D
#define RDNMI          0x4210
#define TIMEUP         0x4211
#define HVBJOY         0x4212
#define RDIO           0x4213
//#define RDDIVL         0x4214
//#define RDDIVH         0x4215
//#define RDMPYL         0x4216
//#define RDMPYH         0x4217
#define JOY1L          0x4218
#define JOY1H          0x4219
#define JOY2L          0x421A
#define JOY2H          0x421B
#define JOY3L          0x421C
#define JOY3H          0x421D
#define JOY4L          0x421E
#define JOY4H          0x421F
/* DMA */
#define DMAP0          0x4300
#define BBAD0          0x4301
#define A1T0L          0x4302
#define A1T0H          0x4303
#define A1B0           0x4304
#define DAS0L          0x4305
#define DAS0H          0x4306
#define DSAB0          0x4307
#define A2A0L          0x4308
#define A2A0H          0x4309
#define NTRL0          0x430A
#define DMAP1          0x4310
#define BBAD1          0x4311
#define A1T1L          0x4312
#define A1T1H          0x4313
#define A1B1           0x4314
#define DAS1L          0x4315
#define DAS1H          0x4316
#define DSAB1          0x4317
#define A2A1L          0x4318
#define A2A1H          0x4319
#define NTRL1          0x431A
#define DMAP2          0x4320
#define BBAD2          0x4321
#define A1T2L          0x4322
#define A1T2H          0x4323
#define A1B2           0x4324
#define DAS2L          0x4325
#define DAS2H          0x4326
#define DSAB2          0x4327
#define A2A2L          0x4328
#define A2A2H          0x4329
#define NTRL2          0x432A
#define DMAP3          0x4330
#define BBAD3          0x4331
#define A1T3L          0x4332
#define A1T3H          0x4333
#define A1B3           0x4334
#define DAS3L          0x4335
#define DAS3H          0x4336
#define DSAB3          0x4337
#define A2A3L          0x4338
#define A2A3H          0x4339
#define NTRL3          0x433A
#define DMAP4          0x4340
#define BBAD4          0x4341
#define A1T4L          0x4342
#define A1T4H          0x4343
#define A1B4           0x4344
#define DAS4L          0x4345
#define DAS4H          0x4346
#define DSAB4          0x4347
#define A2A4L          0x4348
#define A2A4H          0x4349
#define NTRL4          0x434A
#define DMAP5          0x4350
#define BBAD5          0x4351
#define A1T5L          0x4352
#define A1T5H          0x4353
#define A1B5           0x4354
#define DAS5L          0x4355
#define DAS5H          0x4356
#define DSAB5          0x4357
#define A2A5L          0x4358
#define A2A5H          0x4359
#define NTRL5          0x435A
#define DMAP6          0x4360
#define BBAD6          0x4361
#define A1T6L          0x4362
#define A1T6H          0x4363
#define A1B6           0x4364
#define DAS6L          0x4365
#define DAS6H          0x4366
#define DSAB6          0x4367
#define A2A6L          0x4368
#define A2A6H          0x4369
#define NTRL6          0x436A
#define DMAP7          0x4370
#define BBAD7          0x4371
#define A1T7L          0x4372
#define A1T7H          0x4373
#define A1B7           0x4374
#define DAS7L          0x4375
#define DAS7H          0x4376
#define DSAB7          0x4377
#define A2A7L          0x4378
#define A2A7H          0x4379
#define NTRL7          0x437A
/* Definitions for sound DSP */
#define DSP_V0_VOLL		0x00
#define DSP_V0_VOLR		0x01
#define DSP_V0_PITCHL	0x02
#define DSP_V0_PITCHH	0x03
#define DSP_V0_SRCN		0x04
#define DSP_V0_ADSR1	0x05	/* gdddaaaa = g:gain enable | d:decay | a:attack */
#define DSP_V0_ADSR2	0x06	/* llllrrrr = l:sustain left | r:sustain right */
#define DSP_V0_GAIN		0x07
#define DSP_V0_ENVX		0x08
#define DSP_V0_OUTX		0x09
#define DSP_V1_VOLL		0x10
#define DSP_V1_VOLR		0x11
#define DSP_V1_PITCHL	0x12
#define DSP_V1_PITCHH	0x13
#define DSP_V1_SRCN		0x14
#define DSP_V1_ADSR1	0x15
#define DSP_V1_ADSR2	0x16
#define DSP_V1_GAIN		0x17
#define DSP_V1_ENVX		0x18
#define DSP_V1_OUTX		0x19
#define DSP_V2_VOLL		0x20
#define DSP_V2_VOLR		0x21
#define DSP_V2_PITCHL	0x22
#define DSP_V2_PITCHH	0x23
#define DSP_V2_SRCN		0x24
#define DSP_V2_ADSR1	0x25
#define DSP_V2_ADSR2	0x26
#define DSP_V2_GAIN		0x27
#define DSP_V2_ENVX		0x28
#define DSP_V2_OUTX		0x29
#define DSP_V3_VOLL		0x30
#define DSP_V3_VOLR		0x31
#define DSP_V3_PITCHL	0x32
#define DSP_V3_PITCHH	0x33
#define DSP_V3_SRCN		0x34
#define DSP_V3_ADSR1	0x35
#define DSP_V3_ADSR2	0x36
#define DSP_V3_GAIN		0x37
#define DSP_V3_ENVX		0x38
#define DSP_V3_OUTX		0x39
#define DSP_V4_VOLL		0x40
#define DSP_V4_VOLR		0x41
#define DSP_V4_PITCHL	0x42
#define DSP_V4_PITCHH	0x43
#define DSP_V4_SRCN		0x44
#define DSP_V4_ADSR1	0x45
#define DSP_V4_ADSR2	0x46
#define DSP_V4_GAIN		0x47
#define DSP_V4_ENVX		0x48
#define DSP_V4_OUTX		0x49
#define DSP_V5_VOLL		0x50
#define DSP_V5_VOLR		0x51
#define DSP_V5_PITCHL	0x52
#define DSP_V5_PITCHH	0x53
#define DSP_V5_SRCN		0x54
#define DSP_V5_ADSR1	0x55
#define DSP_V5_ADSR2	0x56
#define DSP_V5_GAIN		0x57
#define DSP_V5_ENVX		0x58
#define DSP_V5_OUTX		0x59
#define DSP_V6_VOLL		0x60
#define DSP_V6_VOLR		0x61
#define DSP_V6_PITCHL	0x62
#define DSP_V6_PITCHH	0x63
#define DSP_V6_SRCN		0x64
#define DSP_V6_ADSR1	0x65
#define DSP_V6_ADSR2	0x66
#define DSP_V6_GAIN		0x67
#define DSP_V6_ENVX		0x68
#define DSP_V6_OUTX		0x69
#define DSP_V7_VOLL		0x70
#define DSP_V7_VOLR		0x71
#define DSP_V7_PITCHL	0x72
#define DSP_V7_PITCHH	0x73
#define DSP_V7_SRCN		0x74
#define DSP_V7_ADSR1	0x75
#define DSP_V7_ADSR2	0x76
#define DSP_V7_GAIN		0x77
#define DSP_V7_ENVX		0x78
#define DSP_V7_OUTX		0x79
#define DSP_MVOLL		0x0C
#define DSP_MVOLR		0x1C
#define DSP_EVOLL		0x2C
#define DSP_EVOLR		0x3C
#define DSP_KON			0x4C	/* 01234567 = Key on for voices 0-7 */
#define DSP_KOF			0x5C	/* 01234567 = Key off for voices 0-7 */
#define DSP_FLG			0x6C	/* rme--n-- = r:Soft reset | m:Mute | e:External memory through echo | n:Clock of noise generator */
#define DSP_ENDX		0x7C
#define DSP_EFB			0x0D	/* sfffffff = s: sign bit | f: feedback */
#define DSP_PMOD		0x2D
#define DSP_NON			0x3D
#define DSP_EON			0x4D
#define DSP_DIR			0x5D
#define DSP_ESA			0x6D
#define DSP_EDL			0x7D	/* ----dddd = d: echo delay */
#define DSP_FIR_C0		0x0F
#define DSP_FIR_C1		0x1F
#define DSP_FIR_C2		0x2F
#define DSP_FIR_C3		0x3F
#define DSP_FIR_C4		0x4F
#define DSP_FIR_C5		0x5F
#define DSP_FIR_C6		0x6F
#define DSP_FIR_C7		0x7F

struct snes_cart_info
{
	UINT8  mode;		/* ROM memory mode */
	UINT32 sram;		/* Amount of sram in cart */
	UINT32 sram_max;	/* Maximum amount sram in cart (based on ROM mode) */
	int    small_sram;
	int    slot_in_use;	/* this is needed by Sufami Turbo slots (to check if SRAM has to be saved) */
};

struct snes_joypad
{
	UINT16 buttons;
};

struct snes_mouse
{
	INT16 x, y, oldx, oldy;
	UINT8 buttons;
	UINT8 deltax, deltay;
	int speed;
};

struct snes_superscope
{
	INT16 x, y;
	UINT8 buttons;
	int turbo_lock, pause_lock, fire_lock;
	int offscreen;
};

typedef void (*snes_io_read)(running_machine &machine);
typedef UINT8 (*snes_oldjoy_read)(running_machine &machine);

class snes_state : public driver_device
{
public:
	snes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		{ }


	/* misc */
	UINT16                m_htmult;		/* in 512 wide, we run HTOTAL double and halve it on latching */
	UINT16                m_cgram_address;	/* CGRAM address */
	UINT8                 m_vram_read_offset;	/* VRAM read offset */
	UINT8                 m_read_ophct;
	UINT8                 m_read_opvct;
	UINT16                m_hblank_offset;
	UINT16                m_vram_fgr_high;
	UINT16                m_vram_fgr_increment;
	UINT16                m_vram_fgr_count;
	UINT16                m_vram_fgr_mask;
	UINT16                m_vram_fgr_shift;
	UINT16                m_vram_read_buffer;
	UINT32                m_wram_address;
	UINT16                m_htime;
	UINT16                m_vtime;
	UINT16                m_vmadd;

	/* non-SNES HW-specific flags / variables */
	UINT8                 m_is_nss;
	UINT8				  m_input_disabled;
	UINT8				  m_game_over_flag;
	UINT8                 m_joy_flag;
	UINT8                 m_is_sfcbox;

	/* timers */
	emu_timer             *m_scanline_timer;
	emu_timer             *m_hblank_timer;
	emu_timer             *m_nmi_timer;
	emu_timer             *m_hirq_timer;
//  emu_timer             *m_div_timer;
//  emu_timer             *m_mult_timer;
	emu_timer             *m_io_timer;

	/* DMA/HDMA-related */
	struct
	{
		UINT8  dmap;
		UINT8  dest_addr;
		UINT16 src_addr;
		UINT16 trans_size;
		UINT8  bank;
		UINT8  ibank;
		UINT16 hdma_addr;
		UINT16 hdma_iaddr;
		UINT8  hdma_line_counter;
		UINT8  unk;

		int    do_transfer;

		int    dma_disabled;	// used to stop DMA if HDMA is enabled (currently not implemented, see machine/snes.c)
	}m_dma_channel[8];
	UINT8                 m_hdmaen;	/* channels enabled for HDMA */

	/* input-related */
	UINT8                 m_joy1l;
	UINT8                 m_joy1h;
	UINT8                 m_joy2l;
	UINT8                 m_joy2h;
	UINT8                 m_joy3l;
	UINT8                 m_joy3h;
	UINT8                 m_joy4l;
	UINT8                 m_joy4h;
	UINT16                m_data1[2];
	UINT16                m_data2[2];
	UINT8                 m_read_idx[2];
	snes_joypad           m_joypad[2];
	snes_mouse            m_mouse[2];
	snes_superscope       m_scope[2];

	/* input callbacks (to allow MESS to have its own input handlers) */
	snes_io_read          m_io_read;
	snes_oldjoy_read      m_oldjoy1_read;
	snes_oldjoy_read      m_oldjoy2_read;

	/* cart related */
	UINT8 m_has_addon_chip;
	UINT32 m_cart_size;
	snes_cart_info m_cart[2];	// the second one is used by MESS for Sufami Turbo and, eventually, BS-X

	/* devices */
	_5a22_device *m_maincpu;
	spc700_device *m_soundcpu;
	snes_sound_device *m_spc700;
	cpu_device *m_superfx;
	upd7725_device *m_upd7725;
	upd96050_device *m_upd96050;

	DECLARE_DIRECT_UPDATE_MEMBER(snes_spc_direct);
	DECLARE_DIRECT_UPDATE_MEMBER(snes_direct);
	DECLARE_DRIVER_INIT(snes);
	DECLARE_DRIVER_INIT(snes_hirom);
	DECLARE_DRIVER_INIT(snes_mess);
	DECLARE_DRIVER_INIT(snesst);

	inline UINT16 snes_get_bgcolor( UINT8 direct_colors, UINT16 palette, UINT8 color );
	inline void snes_set_scanline_pixel( int screen, INT16 x, UINT16 color, UINT8 priority, UINT8 layer, int blend );
	inline void snes_draw_bgtile_lores( UINT8 layer, INT16 ii, UINT8 colour, UINT16 pal, UINT8 direct_colors, UINT8 priority );
	inline void snes_draw_bgtile_hires( UINT8 layer, INT16 ii, UINT8 colour, UINT16 pal, UINT8 direct_colors, UINT8 priority );
	inline void snes_draw_oamtile( INT16 ii, UINT8 colour, UINT16 pal, UINT8 priority );
	inline void snes_draw_tile( UINT8 planes, UINT8 layer, UINT32 tileaddr, INT16 x, UINT8 priority, UINT8 flip, UINT8 direct_colors, UINT16 pal, UINT8 hires );
	inline UINT32 snes_get_tmap_addr( UINT8 layer, UINT8 tile_size, UINT32 base, UINT32 x, UINT32 y );
	inline void snes_update_line( UINT16 curline, UINT8 layer, UINT8 priority_b, UINT8 priority_a, UINT8 color_depth, UINT8 hires, UINT8 offset_per_tile, UINT8 direct_colors );
	void snes_update_line_mode7( UINT16 curline, UINT8 layer, UINT8 priority_b, UINT8 priority_a );
	void snes_update_obsel( void );
	void snes_oam_list_build( void );
	int is_sprite_on_scanline( UINT16 curline, UINT8 sprite );
	void snes_update_objects_rto( UINT16 curline );
	void snes_update_objects( UINT8 priority_oam0, UINT8 priority_oam1, UINT8 priority_oam2, UINT8 priority_oam3 );
	void snes_update_mode_0( UINT16 curline );
	void snes_update_mode_1( UINT16 curline );
	void snes_update_mode_2( UINT16 curline );
	void snes_update_mode_3( UINT16 curline );
	void snes_update_mode_4( UINT16 curline );
	void snes_update_mode_5( UINT16 curline );
	void snes_update_mode_6( UINT16 curline );
	void snes_update_mode_7( UINT16 curline );
	void snes_draw_screens( UINT16 curline );
	void snes_update_windowmasks( void );
	void snes_update_offsets( void );
	inline void snes_draw_blend( UINT16 offset, UINT16 *colour, UINT8 prevent_color_math, UINT8 black_pen_clip, int switch_screens );
	void snes_refresh_scanline( running_machine &machine, bitmap_rgb32 &bitmap, UINT16 curline );

	UINT32 snes_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER( snes_oam_read );
	DECLARE_WRITE8_MEMBER( snes_oam_write );
	DECLARE_READ8_MEMBER( snes_cgram_read );
	DECLARE_WRITE8_MEMBER( snes_cgram_write );
	DECLARE_READ8_MEMBER( snes_vram_read );
	DECLARE_WRITE8_MEMBER( snes_vram_write );
	UINT16 *m_snes_oam;		/* Object Attribute Memory */
	UINT16 *m_snes_cgram;	/* Palette RAM */
	UINT8  *m_snes_vram;	/* Video RAM (TODO: Should be 16-bit, but it's easier this way) */

};

/* Special chips, checked at init and used in memory handlers */
enum
{
	HAS_NONE = 0,
	HAS_DSP1,
	HAS_DSP2,
	HAS_DSP3,
	HAS_DSP4,
	HAS_SUPERFX,
	HAS_SA1,
	HAS_SDD1,
	HAS_OBC1,
	HAS_RTC,
	HAS_Z80GB,
	HAS_CX4,
	HAS_ST010,
	HAS_ST011,
	HAS_ST018,
	HAS_SPC7110,
	HAS_SPC7110_RTC,
	HAS_UNK
};

/* offset-per-tile modes */
enum
{
	SNES_OPT_NONE = 0,
	SNES_OPT_MODE2,
	SNES_OPT_MODE4,
	SNES_OPT_MODE6
};

/* layers */
enum
{
	SNES_BG1 = 0,
	SNES_BG2,
	SNES_BG3,
	SNES_BG4,
	SNES_OAM,
	SNES_COLOR
};

/*----------- defined in machine/snes.c -----------*/


extern MACHINE_START( snes );
extern MACHINE_RESET( snes );

DECLARE_READ8_HANDLER( snes_open_bus_r );

extern DECLARE_READ8_HANDLER( snes_r_io );
extern DECLARE_WRITE8_HANDLER( snes_w_io );

extern DECLARE_READ8_HANDLER( snes_r_bank1 );
extern DECLARE_READ8_HANDLER( snes_r_bank2 );
extern DECLARE_READ8_HANDLER( snes_r_bank3 );
extern DECLARE_READ8_HANDLER( snes_r_bank4 );
extern DECLARE_READ8_HANDLER( snes_r_bank5 );
extern DECLARE_READ8_HANDLER( snes_r_bank6 );
extern DECLARE_READ8_HANDLER( snes_r_bank7 );
extern DECLARE_WRITE8_HANDLER( snes_w_bank1 );
extern DECLARE_WRITE8_HANDLER( snes_w_bank2 );
extern DECLARE_WRITE8_HANDLER( snes_w_bank4 );
extern DECLARE_WRITE8_HANDLER( snes_w_bank5 );
extern DECLARE_WRITE8_HANDLER( snes_w_bank6 );
extern DECLARE_WRITE8_HANDLER( snes_w_bank7 );

extern DECLARE_READ8_HANDLER( superfx_r_bank1 );
extern DECLARE_READ8_HANDLER( superfx_r_bank2 );
extern DECLARE_READ8_HANDLER( superfx_r_bank3 );
extern DECLARE_WRITE8_HANDLER( superfx_w_bank1 );
extern DECLARE_WRITE8_HANDLER( superfx_w_bank2 );
extern DECLARE_WRITE8_HANDLER( superfx_w_bank3 );

WRITE_LINE_DEVICE_HANDLER( snes_extern_irq_w );


extern UINT8  *snes_ram;			/* Main memory */


/* (PPU) Video related */
struct SNES_PPU_STRUCT	/* once all the regs are saved in this structure, it would be better to reorganize it a bit... */
{
	struct
	{
		/* clipmasks */
		UINT8 window1_enabled, window1_invert;
		UINT8 window2_enabled, window2_invert;
		UINT8 wlog_mask;
		/* color math enabled */
		UINT8 color_math;

		UINT8 charmap;
		UINT8 tilemap;
		UINT8 tilemap_size;

		UINT8 tile_size;
		UINT8 mosaic_enabled;	// actually used only for layers 0->3!

		UINT8 main_window_enabled;
		UINT8 sub_window_enabled;
		UINT8 main_bg_enabled;
		UINT8 sub_bg_enabled;

		UINT16 hoffs;
		UINT16 voffs;
	} layer[6];	// this is for the BG1 - BG2 - BG3 - BG4 - OBJ - color layers

	struct
	{
		UINT8 address_low;
		UINT8 address_high;
		UINT8 saved_address_low;
		UINT8 saved_address_high;
		UINT16 address;
		UINT16 priority_rotation;
		UINT8 next_charmap;
		UINT8 next_size;
		UINT8 size;
		UINT32 next_name_select;
		UINT32 name_select;
		UINT8 first_sprite;
		UINT8 flip;
		UINT16 write_latch;
	} oam;

	struct
	{
		UINT16 horizontal[4];
		UINT16 vertical[4];
	} bgd_offset;

	struct
	{
		UINT16 latch_horz;
		UINT16 latch_vert;
		UINT16 current_horz;
		UINT16 current_vert;
		UINT8 last_visible_line;
		UINT8 interlace_count;
	} beam;

	struct
	{
		UINT8 repeat;
		UINT8 hflip;
		UINT8 vflip;
		INT16 matrix_a;
		INT16 matrix_b;
		INT16 matrix_c;
		INT16 matrix_d;
		INT16 origin_x;
		INT16 origin_y;
		UINT16 hor_offset;
		UINT16 ver_offset;
		UINT8 extbg;
	} mode7;

	UINT8 mosaic_size;
	UINT8 clip_to_black;
	UINT8 prevent_color_math;
	UINT8 sub_add_mode;
	UINT8 bg3_priority_bit;
	UINT8 direct_color;
	UINT8 ppu_last_scroll;		/* as per Anomie's doc and Theme Park, all scroll regs shares (but mode 7 ones) the same
                                    'previous' scroll value */
	UINT8 mode7_last_scroll;	/* as per Anomie's doc mode 7 scroll regs use a different value, shared with mode 7 matrix! */

	UINT8 ppu1_open_bus, ppu2_open_bus;
	UINT8 ppu1_version, ppu2_version;
	UINT8 window1_left, window1_right, window2_left, window2_right;

	UINT16 mosaic_table[16][4096];
	UINT8 clipmasks[6][SNES_SCR_WIDTH];
	UINT8 update_windows;
	UINT8 update_offsets;
	UINT8 update_oam_list;
	UINT8 mode;
	UINT8 interlace; //doubles the visible resolution
	UINT8 obj_interlace;
	UINT8 screen_brightness;
	UINT8 screen_disabled;
	UINT8 pseudo_hires;
	UINT8 color_modes;
	UINT8 stat77_flags;
};

extern struct snes_cart_info snes_cart;

/*----------- defined in video/snes.c -----------*/

extern struct SNES_PPU_STRUCT snes_ppu;

extern void snes_latch_counters(running_machine &machine);

extern VIDEO_START( snes );
extern SCREEN_UPDATE_RGB32( snes );

extern DECLARE_READ8_HANDLER( snes_ppu_read );
extern DECLARE_WRITE8_HANDLER( snes_ppu_write );

#endif /* _SNES_H_ */
