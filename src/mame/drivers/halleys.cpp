// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Acho A. Tang
/*
Halley's Comet, 1986 Taito

    Halley's Comet was created by Fukio Mitsuji (MTJ),
    who also created Bubble Bobble, Rainbow Islands, Syvalion, etc.

  Issues:
    - Status reads from blitter RAM aren't well understood.
      They probably includes both completion and collision flags.
    - The blitter is missing many features, in particular certain sprites aren't
      erased properly.
    - It isn't known how many independent planes of graphics the blitter manages.
      The starfield, for example, probably involves at least two additional planes
      orthogonal to the sprites and scroll layer.
    - Background tiles inside the comet have wrong coordinates.
    - Score digits have wrong colors.
    - Blitter functions, especially those capable of screen warping, are unoptimized.
    - The starfields can probably be represented and rendered by two simple lists
      instead of costly bitmaps.
    - Ben Bero Beh has collision problems with the falling fireballs and a minor
      priority glitch with the "Elevator Action" baddies.
    - IRQ system needs a complete rewrite
    - De-achoize the blitter code

    * Halley's Comet's undocumented DIP switches only work if the player1 start button
      is depressed during boot-up.


    Special thanks to Phil Stroffolino for the creation of a solid framework,
    Jarek Burczynski for adding sound and providing awesome hand-drawn schematics,
    Jason MacFarlane for his continuous and selfless support. I'd never have come
    all this way without the invaluable videos and screen shots sent by Jason.

    To the Columbia crew and the forerunners who gave their lives to science and space exploration.
    Your bravery and devotion will not be forgotten.


CPU:    MC68B09EP Z80A(SOUND)
Sound:  YM2149F x 4
OSC:    19.968MHz 6MHz

J1100021    sound daughterboard

Z80A    6MHz
2016
A62-13
A62-14
YM2149
YM2149
YM2149

J1100018

      sw1       sw2  sw3  sw4

          A62-2  A62-15
68B09EP   A62-1
                 A62-4
                 2016    YM2149
                 2016

J1100020

 4116 4116 4116 4116     MB15511
 4116 4116 4116 4116
 4116 4116 4116 4116
 4116 4116 4116 4116
 4116 4116 4116 4116     MB15512
 4116 4116 4116 4116
 4116 4116 4116 4116
 4116 4116 4116 4116

 MB15510  MB15510
                          2016
                          2016

J1100019


                 A62-9  A62-10
 A26-13          A62-11 A62-12
 A26-13          A62-5  A62-16
 A26-13          A62-7  A62-8



                                 HALLEY'S COMET
                                   Taito 1986

                          Pinout and DIP switch settings
                    typed in by Pelle Einarsson, pelle@mensa.se

NOTE: I had to guess the meaning of some Japanese text concerning the DIP
      switch settings, but I think I got it right.

      * = default settings


              T                                       G
    PARTS          SOLDER                  PARTS              SOLDER
----------------------------------    ---------------------------------------
       GND   1 A   GND                         GND   1 A   GND
       GND   2 B   GND                               2 B
       GND   3 C   GND                               3 C
       GND   4 D   GND                               4 D
 Video GND   5 E   Video GND            Sound out+   5 E   Sound Out -
Video sync   6 F   Video sync                 Post   6 F   Post
      Post   7 H   Post                              7 H   Power on reset
   Video R   8 J   Video R               Coin sw A   8 J   Coin sw B
   Video G   9 K   Video G          Coin counter A   9 K   Coin counter B
   Video B  10 L   Video B          Coin lockout A  10 L   Coin lockout B
       -5V  11 M   -5V                  Service sw  11 M   Tilt sw
       -5V  12 N   -5V                    Select 1  12 N   Select 2
      +12V  13 P   +12V                      1P up  13 P   2P up
      +12V  14 R   +12V                    1P down  14 R   2P down
       +5V  15 S   +5V                    1P right  15 S   2P right
       +5V  16 T   +5V                     1P left  16 T   2P left
       +5V  17 U   +5V                              17 U
       +5V  18 V   +5V                              18 V
                                                    19 W
                                                    20 X
      H                                   1P shoot  21 Y   2p shoot
   --------                          1P hyperspace  22 Z   2P hyperspace
    1  GND
    2  GND
    3  GND
    4  GND
    5  +5V
    6  +5V
    7  +5V
    8  -5V
    9  +13V (a typo?)            DIP switch 1                Option    1-5   6
    10  Post                     ----------------------------------------------
    11  +12V                     (single/dual controls?)    *1-way     off  on
    12  +12V                                                 2-way          off



  color test layout:             (0,0) *ROT90
     +------+-------+------+------+
     |      |       |      |      |
     | White| Blue  |Green | Red  | 1 (brightest)
     |      |       |      |      |
     +------+-------+------+------+
     |      |       |      |      |
     |      |       |      |      | 2
     |      |       |      |      |
     +------+-------+------+------+
     |      |       |      |      |
     |      |       |      |      | 3
     |      |       |      |      |
     +------+-------+------+------+
     |      |       |      |      |
     |      |       |      |      | 4 (darkest)
     |      |       |      |      |
     +------+-------+------+------+
*/

//**************************************************************************
// Compiler Directives

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/taitoipt.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"

#define HALLEYS_DEBUG 0


//**************************************************************************
// Driver Constants and Variables

#define GAME_BENBEROB 0
#define GAME_HALLEYS  1

/* CAUTION: SENSITIVE VALUES */
#define SCREEN_WIDTH_L2 8
#define SCREEN_HEIGHT 256
#define VIS_MINX      0
#define VIS_MAXX      255
#define VIS_MINY      8
#define VIS_MAXY      (255-8)

#define MAX_LAYERS    6
#define MAX_SPRITES   256
#define MAX_SOUNDS    16

#define SP_2BACK     0x100
#define SP_ALPHA     0x200
#define SP_COLLD     0x300

#define BG_MONO      0x400
#define BG_RGB       0x500

#define PALETTE_SIZE 0x600

#define SCREEN_WIDTH        (1 << SCREEN_WIDTH_L2)
#define SCREEN_SIZE         (SCREEN_HEIGHT << SCREEN_WIDTH_L2)
#define SCREEN_BYTEWIDTH_L2 (SCREEN_WIDTH_L2 + 1)
#define SCREEN_BYTEWIDTH    (1 << SCREEN_BYTEWIDTH_L2)
#define SCREEN_BYTESIZE     (SCREEN_HEIGHT << SCREEN_BYTEWIDTH_L2)
#define CLIP_SKIP           (VIS_MINY * SCREEN_WIDTH + VIS_MINX)
#define CLIP_W              (VIS_MAXX - VIS_MINX + 1)
#define CLIP_H              (VIS_MAXY - VIS_MINY + 1)
#define CLIP_BYTEW          (CLIP_W << 1)


class halleys_state : public driver_device
{
public:
	halleys_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_blitter_ram(*this, "blitter_ram"),
		m_io_ram(*this, "io_ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_palette(*this, "palette") { }

	UINT16 *m_render_layer[MAX_LAYERS];
	UINT8 m_sound_fifo[MAX_SOUNDS];
	UINT8 *m_gfx_plane02;
	UINT8 *m_gfx_plane13;
	std::unique_ptr<UINT8[]> m_collision_list;
	UINT8 *m_scrolly0;
	UINT8 *m_scrollx0;
	UINT8 *m_scrolly1;
	UINT8 *m_scrollx1;
	std::unique_ptr<UINT32[]> m_internal_palette;
	std::unique_ptr<UINT32[]> m_alpha_table;
	UINT8 *m_cpu1_base;
	std::unique_ptr<UINT8[]> m_gfx1_base;
	required_shared_ptr<UINT8> m_blitter_ram;
	required_shared_ptr<UINT8> m_io_ram;
	int m_game_id;
	int m_blitter_busy;
	int m_collision_count;
	int m_stars_enabled;
	int m_bgcolor;
	int m_ffcount;
	int m_ffhead;
	int m_fftail;
	int m_mVectorType;
	int m_sndnmi_mask;
	int m_firq_level;
	emu_timer *m_blitter_reset_timer;
	offs_t m_collision_detection;
	int m_latch_delay;
	std::vector<UINT8> m_paletteram;

	DECLARE_WRITE8_MEMBER(bgtile_w);
	DECLARE_READ8_MEMBER(blitter_status_r);
	DECLARE_READ8_MEMBER(blitter_r);
	DECLARE_WRITE8_MEMBER(blitter_w);
	DECLARE_READ8_MEMBER(collision_id_r);
	DECLARE_READ8_MEMBER(paletteram_r);
	DECLARE_WRITE8_MEMBER(paletteram_w);
	DECLARE_READ8_MEMBER(vector_r);
	DECLARE_WRITE8_MEMBER(firq_ack_w);
	DECLARE_WRITE8_MEMBER(soundcommand_w);
	DECLARE_READ8_MEMBER(coin_lockout_r);
	DECLARE_READ8_MEMBER(io_mirror_r);
	void blit(int offset);
	DECLARE_WRITE8_MEMBER(sndnmi_msk_w);
	DECLARE_DRIVER_INIT(halley87);
	DECLARE_DRIVER_INIT(benberob);
	DECLARE_DRIVER_INIT(halleys);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(halleys);
	UINT32 screen_update_halleys(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_benberob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(blitter_reset);
	TIMER_DEVICE_CALLBACK_MEMBER(halleys_scanline);
	TIMER_DEVICE_CALLBACK_MEMBER(benberob_scanline);
	void halleys_decode_rgb(UINT32 *r, UINT32 *g, UINT32 *b, int addr, int data);
	void copy_scroll_op(bitmap_ind16 &bitmap, UINT16 *source, int sx, int sy);
	void copy_scroll_xp(bitmap_ind16 &bitmap, UINT16 *source, int sx, int sy);
	void copy_fixed_xp(bitmap_ind16 &bitmap, UINT16 *source);
	void copy_fixed_2b(bitmap_ind16 &bitmap, UINT16 *source);
	void filter_bitmap(bitmap_ind16 &bitmap, int mask);
	void init_common();
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<palette_device> m_palette;
};



//**************************************************************************
// MB1551x Blitter Functions

void halleys_state::blit(int offset)
{
/*
    The render layers can be converted to standard MAME bitmaps but
    I am not taking chances as long as the blitter has unknown draw
    modes which may go beyond MAME's capability.

    The function has grown tremendously as more features were added
    and become rather difficult to maintain.
*/

// status attributes
#define ACTIVE 0x02
#define DECAY  0x10
#define RETIRE 0x80

// mode attributes
#define IGNORE_0 0x01
#define MODE_02  0x02
#define COLOR_ON 0x04
#define MIRROR_X 0x08
#define MIRROR_Y 0x10
#define MODE_40  0x40
#define GROUP    0x80

// color attributes
#define PENCOLOR 0x0f
#define BANKBIT1 0x80

// code attributes
#define COMMAND  0x0f
#define BGLAYER  0x10
#define BANKBIT0 0x40
#define PLANE    0x80

// special draw commands
#define EFX1      0x8 // draws to back??
#define HORIZBAR  0xe // draws 8-bit RGB horizontal bars
#define STARPASS1 0x3 // draws the stars
#define STARPASS2 0xd // modifies stars color
#define TILEPASS1 0x5 // draws tiles(s1: embedded 8-bit RGB, s2: bit-packed pixel data)
#define TILEPASS2 0xb // modifies tiles color/alpha

// user flags
#define FLIP_X     0x0100
#define FLIP_Y     0x0200
#define SINGLE_PEN 0x0400
#define RGB_MASK   0x0800
#define AD_HIGH    0x1000
#define SY_HIGH    0x2000
#define S1_IDLE    0x4000
#define S1_REV     0x8000
#define S2_IDLE    0x10000
#define S2_REV     0x20000
#define BACKMODE   0x40000
#define ALPHAMODE  0x80000
#define PPCD_ON    0x100000

// hard-wired defs and interfaces
#define HALLEYS_SPLIT   0xf4
#define COLLISION_PORTA 0x67
#define COLLISION_PORTB 0x8b

// short cuts
#define XMASK (SCREEN_WIDTH-1)
#define YMASK (SCREEN_HEIGHT-1)

	static const UINT8 penxlat[16]={0x03,0x07,0x0b,0x0f,0x02,0x06,0x0a,0x0e,0x01,0x05,0x09,0x0d,0x00,0x04,0x08,0x0c};
	static const UINT8 rgbmask[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xff,0xff};
	static const UINT8 tyremap[ 8]={ 0, 5, 9,13,16,20,24,28};

	UINT8 *param, *src_base;
	UINT16 *dst_base;
	int stptr, mode, color, code, y, x, h, w, src1, src2;
	int status, flags, group, command, bank, layer, pen0, pen1;
	int yclip, xclip, src_yskip, src_xskip, dst_skip, hclip, wclip, src_dy, src_dx;
	UINT32 *pal_ptr;
	UINT8 *src1_ptr, *src2_ptr; // esi alias, ebx alias
	UINT16 *dst_ptr;             // edi alias
	void *edi;                 // scratch
	int eax, ebx, ecx, edx;    // scratch
	UINT16 ax; UINT8 al, ah;      // partial regs


	param = m_blitter_ram + offset;


#if HALLEYS_DEBUG
if (0) {
	logerror("%s:[%04x]", machine.describe_context(), offset);
	for (ecx=0; ecx<16; ecx++) logerror(" %02x", param[ecx]);
	logerror("\n");
}
#endif


	// init blitter controls
	stptr = (int)param[0x2]<<8 | (int)param[0x3];
	mode  = (int)param[0x5];
	color = (int)param[0x6];
	code  = (int)param[0x7];


	// update sprite status
	layer = (code>>3 & 2) | (code>>7 & 1);
	offset >>= 4;
	status = (stptr) ? m_cpu1_base[stptr] : ACTIVE;
	flags = mode;
	group = mode & GROUP;
	command = code & COMMAND;
	if (m_game_id == GAME_HALLEYS)
	{
		if (!layer) flags |= PPCD_ON;
		if (offset >= HALLEYS_SPLIT) flags |= AD_HIGH; else
		if (offset & 1) memcpy(param-0x10, param, 0x10); else group = param[0x15] & GROUP;

		// HACK: force engine flame in group zero to increase collision accuracy
		if (offset == 0x1a || offset == 0x1b) group = 0;
	}
	else if (offset & 1) memcpy(param-0x10, param, 0x10);


	// init draw parameters
	y = (int)param[0x8];
	x = (int)param[0x9];
	h = (int)param[0xa];
	w = (int)param[0xb] + 1;
	if (!h) return;


	// translate source addresses
	src1 = (int)param[0xc]<<8 | (int)param[0xd];
	src2 = (int)param[0xe]<<8 | (int)param[0xf];
	flags |= src1 & (S1_IDLE | S1_REV);
	flags |= src2<<2 & (S2_IDLE | S2_REV);
	src1 &= 0x3fff;
	src2 &= 0x3fff;
	bank = ((code & BANKBIT0) | (color & BANKBIT1)) << 8;
	pal_ptr = m_internal_palette.get();


	// the crossroad of fate
	if (code & BGLAYER || command & 7) goto COMMAND_MODE;


	// reject off-screen objects
	if (flags & MIRROR_Y) { flags |= FLIP_Y; y -= (h - 1); }
	if (flags & MIRROR_X) { flags |= FLIP_X; x -= (w - 1); }
	if (y > VIS_MAXY || (y + h) <= VIS_MINY) return;
	if (x > VIS_MAXX || (x + w) <= VIS_MINX) return;


	// clip objects against the visible area
	yclip = y; xclip = x; hclip = h; wclip = w;
	src_yskip = src_xskip = 0;
	if (yclip < VIS_MINY) { src_yskip = VIS_MINY - yclip; yclip = VIS_MINY; hclip -= src_yskip; }
	if (yclip + hclip > VIS_MAXY+1) { hclip = VIS_MAXY+1 - yclip; }
	if (xclip < VIS_MINX) { src_xskip = VIS_MINX - xclip; xclip = VIS_MINX; wclip -= src_xskip; }
	if (xclip + wclip > VIS_MAXX+1) { wclip = VIS_MAXX+1 - xclip; }
	dst_skip = (yclip << SCREEN_WIDTH_L2) + xclip;


	// adjust orientations
	eax = 0;
	if (flags & (S1_REV | S2_REV)) { flags ^= FLIP_Y | FLIP_X; eax -= w * h - 8; }

	if (flags & FLIP_Y)
	{
		eax += w * (h - 1);
		src_yskip = -src_yskip;
		src_dy = (flags & FLIP_X) ? -w + wclip : -w - wclip;

	}
	else src_dy = (flags & FLIP_X) ? w + wclip : w - wclip;

	if (flags & FLIP_X)
	{
		eax += w - 1;
		src_xskip = -src_xskip;
		src_dx = -1;
	}
	else src_dx = 1;


	// calculate entry points and loop constants
	src1_ptr = m_gfx_plane02 + ((bank + src1)<<3) + eax;
	src2_ptr = m_gfx_plane13 + ((bank + src2)<<3) + eax;

	if (!(flags & (S1_IDLE | S2_IDLE)))
	{
		eax = src_yskip * w + src_xskip;
		src1_ptr += eax;
		src2_ptr += eax;
	}
	else src_dy = src_dx = 0;

	dst_ptr = m_render_layer[layer] + dst_skip;


	// look up pen values and set rendering flags
	pen0 = code>>3 & 0x10;
	pen1 = 0;
	if (command == EFX1) { flags |= BACKMODE; pen0 |= SP_2BACK; }
	if (src1 == src2)
	{
		flags |= SINGLE_PEN;
		eax = (UINT32)penxlat[color & PENCOLOR];
		if (eax) pen1 = pen0 + eax;
	}
	else if (color & PENCOLOR) flags |= RGB_MASK;


//--------------------------------------------------------------------------

#define BLOCK_WIPE_COMMON { \
	eax = wclip<<1; \
	ecx = hclip; \
	edi = dst_ptr; \
	do { memset((UINT8*)edi, 0, eax); edi = (UINT8*)edi + SCREEN_BYTEWIDTH; } while (--ecx); \
}

//--------------------------------------------------------------------------

	// multi-pen block or transparent blit
	if ((flags & (SINGLE_PEN | RGB_MASK | COLOR_ON)) == COLOR_ON)
	{
		if (!(flags & IGNORE_0)) BLOCK_WIPE_COMMON

		dst_ptr += wclip;
		ecx = wclip = -wclip;
		edx = src_dx;

		if (flags & PPCD_ON) goto COLLISION_MODE;

		al = ah = (UINT8)pen0;

		if (!(flags & BACKMODE))
		{
			do {
				do {
					al |= *src1_ptr;
					src1_ptr += edx;
					al |= *src2_ptr;
					src2_ptr += edx;
					if (al & 0xf) { dst_ptr[ecx] = (UINT16)al;  al = ah;}
				}
				while (++ecx);
				ecx = wclip; src1_ptr += src_dy; src2_ptr += src_dy; dst_ptr += SCREEN_WIDTH;
			}
			while (--hclip);
		}
		else
		{
			do {
				do {
					al |= *src1_ptr;
					src1_ptr += edx;
					al |= *src2_ptr;
					src2_ptr += edx;
					if (al & 0xf) { dst_ptr[ecx] = (UINT16)al | SP_2BACK;  al = ah; }
				}
				while (++ecx);
				ecx = wclip; src1_ptr += src_dy; src2_ptr += src_dy; dst_ptr += SCREEN_WIDTH;
			}
			while (--hclip);
		}
		return;


		COLLISION_MODE:

		ax = 0;
		if (group)
		{
			do {
				do {
					al = *src1_ptr;
					src1_ptr += edx;
					al |= *src2_ptr;
					src2_ptr += edx;
					if (al & 0xf) { dst_ptr[ecx] = (UINT16)al | SP_COLLD; } // set collision flag on group one pixels
				}
				while (++ecx);
				ecx = wclip; src1_ptr += src_dy; src2_ptr += src_dy; dst_ptr += SCREEN_WIDTH;
			}
			while (--hclip);
		}
		else
		{
			do {
				do {
					al = *src1_ptr;
					src1_ptr += edx;
					al |= *src2_ptr;
					src2_ptr += edx;
					if (al & 0xf) { ax |= dst_ptr[ecx]; dst_ptr[ecx] = (UINT16)al; } // combine collision flags in ax
				}
				while (++ecx);
				ecx = wclip; src1_ptr += src_dy; src2_ptr += src_dy; dst_ptr += SCREEN_WIDTH;
			}
			while (--hclip);
		}

		// update collision list if object collided with the other group
		if (status & ACTIVE && ax & SP_COLLD)
		{
			m_collision_list[m_collision_count & (MAX_SPRITES-1)] = offset;
			m_collision_count++;

			#if HALLEYS_DEBUG
				popmessage("ID:%02x CC:%3d", offset, m_collision_count);
			#endif
		}

	} else

//--------------------------------------------------------------------------

	// multi-pen, RGB masked block or transparent blit
	if ((flags & (RGB_MASK | COLOR_ON)) == RGB_MASK + COLOR_ON)
	{
		if (!(flags & IGNORE_0)) BLOCK_WIPE_COMMON

		dst_ptr += wclip;
		ecx = wclip = -wclip;
		al = ah = (UINT8)pen0;
		ebx = rgbmask[color & PENCOLOR] | 0xffffff00;

		do {
			do {
				al |= *src1_ptr;
				src1_ptr += src_dx;
				al |= *src2_ptr;
				src2_ptr += src_dx;
				if (al & 0xf) { edx = (UINT32)al;  al = ah;  dst_ptr[ecx] = pal_ptr[edx] & ebx; }
			}
			while (++ecx);
			ecx = wclip; src1_ptr += src_dy; src2_ptr += src_dy; dst_ptr += SCREEN_WIDTH;
		}
		while (--hclip);

	} else

//--------------------------------------------------------------------------

	// single-pen block or transparent blit
	if ((flags & (SINGLE_PEN | COLOR_ON)) == SINGLE_PEN + COLOR_ON)
	{
		if (!(flags & IGNORE_0)) BLOCK_WIPE_COMMON

		dst_ptr += wclip;
		ebx = hclip;
		ecx = wclip = -wclip;
		edx = src_dx;
		ax = (UINT16)pen1;

		do {
			do {
				if (*src1_ptr) dst_ptr[ecx] = ax;
				src1_ptr += edx;
			}
			while (++ecx);

			ecx = wclip;
			src1_ptr += src_dy;
			dst_ptr  += SCREEN_WIDTH;
		}
		while (--ebx);

	} else

//--------------------------------------------------------------------------

	// transparent wipe
	if ((flags & (IGNORE_0 | COLOR_ON)) == IGNORE_0)
	{
		dst_ptr += wclip;
		wclip = -wclip;
		ecx = wclip;
		edx = src_dx;

		if (flags & PPCD_ON && !group)
		{
			// preserve collision flags when wiping group zero objects
			do {
				do {
					al = *src1_ptr;
					ah = *src2_ptr;
					src1_ptr += edx;
					src2_ptr += edx;
					if (al | ah) dst_ptr[ecx] &= SP_COLLD;
				}
				while (++ecx);

				ecx = wclip;
				src1_ptr += src_dy;
				src2_ptr += src_dy;
				dst_ptr  += SCREEN_WIDTH;
			}
			while (--hclip);
		}
		else
		{
			do {
				do {
					al = *src1_ptr;
					ah = *src2_ptr;
					src1_ptr += edx;
					src2_ptr += edx;
					if (al | ah) dst_ptr[ecx] = 0;
				}
				while (++ecx);

				ecx = wclip;
				src1_ptr += src_dy;
				src2_ptr += src_dy;
				dst_ptr  += SCREEN_WIDTH;
			}
			while (--hclip);
		}

	} else

//--------------------------------------------------------------------------

	// block wipe
	if ((flags & (IGNORE_0 | COLOR_ON)) == 0) BLOCK_WIPE_COMMON

//--------------------------------------------------------------------------

	// End of Standard Mode
	return;

//--------------------------------------------------------------------------


COMMAND_MODE:

#define GFX_HI 0x10000


	// reject illegal blits and adjust parameters
	if (command)
	{
		if (h > 8) return;
		if (y >= 0xf8) { y -= 0xf8; flags |= SY_HIGH; } else
		if (y >= 8) return;
		if (command != TILEPASS1) { h <<= 5; y <<= 5; }
	}


	// calculate entry points and loop constants
	if (flags & S1_IDLE) src_dx = 0; else src_dx = 1;
	if (flags & S1_REV ) src_dx = -src_dx;

	src_base = m_gfx1_base.get() + bank;

	if (command == STARPASS1 || command == STARPASS2) layer = (layer & 1) + 4;
	dst_base = m_render_layer[layer];


//--------------------------------------------------------------------------

#define WARP_WIPE_COMMON { \
	ebx = y + h; \
	ecx = (x + w - SCREEN_WIDTH) << 1; \
	for (edx=y; edx<ebx; edx++) { \
		dst_ptr = dst_base + ((edx & YMASK) << SCREEN_WIDTH_L2); \
		if (ecx > 0) { memset(dst_ptr, 0, ecx); eax = SCREEN_WIDTH - x; } else eax = w; \
		memset(dst_ptr+x, 0, eax<<1); \
	} \
}

//--------------------------------------------------------------------------

	// specialized block wipe
	if ((flags & (IGNORE_0 | COLOR_ON)) == 0)
	{
		// wipe star layer when the following conditions are met:
		if (!command && y == 0xff && h == 1)
		{
			y = 0; h = SCREEN_HEIGHT;
			dst_base = m_render_layer[(layer&1)+4];
			WARP_WIPE_COMMON

			m_stars_enabled = ~layer & 1;
		}

		// wipe background and chain-wipe corresponding sprite layer when the command is zero
		else
		{
			WARP_WIPE_COMMON
			if (!command) { dst_base = m_render_layer[layer&1]; WARP_WIPE_COMMON }
		}

	} else

//--------------------------------------------------------------------------

	// gray shaded star map with 16x16 cells
	if (command == STARPASS1 && flags & COLOR_ON)
	{
		/*
		    Each star map is generated by two data sets pointed by the second source
		    address. The first 256-byte set has scattered bits to reflect the star
		    population while the second 256-byte set appears to have random values.
		    When the game runs the star fields are updated a small portion at a time
		    by a third data set containing gradient patterns which may indicate gray
		    shades or alpha levels.

		    Halley's Comet draws and clears the two star fields as if they are
		    independent from the backgrounds, making it a total of four scrollable
		    layers. However, only two pairs of scroll registers are in use and they
		    affect the stars and the backgrounds together - possibly afterthoughts
		    on the original Ben Bero Beh hardware.

		    This algorithm is based on speculation and deemed inaccurate.
		*/
		#define RORB(R,N) ( ((R>>N)|(R<<(8-N))) & 0xff )
		#define C2S(X,Y,O) ( (((Y+(O>>4))&YMASK)<<SCREEN_WIDTH_L2) + ((X+(O&0xf))&XMASK) )

		src2_ptr = src_base + src2 + GFX_HI;

		for (yclip=y+h; y<yclip; y+=16)
		for (xclip=x+w; x<xclip; x+=16)
		{
			edx = (UINT32)*src2_ptr;
			src2_ptr++;
			if (edx)
			{
				ax = (UINT16)*(src2_ptr-0x100 -1);
				ebx = (UINT32)ax;
				ax |= BG_MONO;
				if (edx & 0x01) {                    dst_base[C2S(x,y,ebx)] = ax; }
				if (edx & 0x02) { ecx = RORB(ebx,1); dst_base[C2S(x,y,ecx)] = ax; }
				if (edx & 0x04) { ecx = RORB(ebx,2); dst_base[C2S(x,y,ecx)] = ax; }
				if (edx & 0x08) { ecx = RORB(ebx,3); dst_base[C2S(x,y,ecx)] = ax; }
				if (edx & 0x10) { ecx = RORB(ebx,4); dst_base[C2S(x,y,ecx)] = ax; }
				if (edx & 0x20) { ecx = RORB(ebx,5); dst_base[C2S(x,y,ecx)] = ax; }
				if (edx & 0x40) { ecx = RORB(ebx,6); dst_base[C2S(x,y,ecx)] = ax; }
				if (edx & 0x80) { ecx = RORB(ebx,7); dst_base[C2S(x,y,ecx)] = ax; }
			}
		}

		m_stars_enabled = layer & 1;

		#undef RORB
		#undef C2S

	} else

//--------------------------------------------------------------------------

	// stars color modifier
	if (command == STARPASS2 && flags & COLOR_ON)
	{
		edx = SCREEN_WIDTH - x - w;
		w = -w;

		ax = (UINT16)(~src_base[src2] & 0xff);

		for (yclip=y+h; y<yclip; y++)
		{
			edi = dst_base + ((y & YMASK) << SCREEN_WIDTH_L2);

			if (edx < 0)
			{
				ecx = edx;
				dst_ptr = (UINT16*)edi - edx;
				do { if (dst_ptr[ecx]) dst_ptr[ecx] ^= ax; } while (++ecx);
				ecx = x - SCREEN_WIDTH;
			} else ecx = w;

			dst_ptr = (UINT16*)edi + x - ecx;
			do { if (dst_ptr[ecx]) dst_ptr[ecx] ^= ax; } while (++ecx);
		}

	} else

//--------------------------------------------------------------------------

	// RGB 8x8 bit-packed transparent tile
	if (command == TILEPASS1 && flags & COLOR_ON)
	{
		/*
		    Tile positioning in this mode is very cryptic and different from
		    others. The coordinate system seems banked and influenced by layer
		    number and whether the blit code is written to the upper or lower
		    off-screen area. These conditions may also imply height-doubling,
		    Y-flipping and draw-from-bottom attributes.

		    Pixel and color information is embedded but the game draws a
		    second pass at the same location with zeroes. In addition the
		    X and Y values passed to the blitter do not reflect the tiles true
		    locations. For example, tiles near the top or bottom of the screen
		    are positioned resonably close but those in the middle are oddly
		    shifted toward either side. The tiles also resemble predefined
		    patterns but I don't know if there are supposed to be lookup tables
		    in ROM or hard-wired to the blitter chips.
		*/
		if (y & 1) x -= 8;
		y = tyremap[y] << 3;

		if (flags & SY_HIGH) y += 8;
		if (y > 0xf8) return;

		if (code & PLANE) return; /* WARNING: UNEMULATED */


		if (flags & IGNORE_0) { w=8; h=8; WARP_WIPE_COMMON }

		src1_ptr = src_base + src1;
		dst_ptr = m_render_layer[2] + (y << SCREEN_WIDTH_L2);

		src1_ptr += 8;
		edx = -8;
		if (x <= 0xf8)
		{
			dst_ptr += x;
			do {
				ax = (UINT16)src1_ptr[edx];
				al = src1_ptr[edx+0x10000];
				ax |= BG_RGB;
				if (al & 0x01) *dst_ptr   = ax;
				if (al & 0x02) dst_ptr[1] = ax;
				if (al & 0x04) dst_ptr[2] = ax;
				if (al & 0x08) dst_ptr[3] = ax;
				if (al & 0x10) dst_ptr[4] = ax;
				if (al & 0x20) dst_ptr[5] = ax;
				if (al & 0x40) dst_ptr[6] = ax;
				if (al & 0x80) dst_ptr[7] = ax;
				dst_ptr += SCREEN_WIDTH;
			} while (++edx);
		}
		else
		{
			#define WARPMASK ((SCREEN_WIDTH<<1)-1)
			do {
				ecx = x & WARPMASK;
				ax = (UINT16)src1_ptr[edx];
				al = src1_ptr[edx+0x10000];
				ax |= BG_RGB;
				if (al & 0x01) dst_ptr[ecx] = ax;  ecx++; ecx &= WARPMASK;
				if (al & 0x02) dst_ptr[ecx] = ax;  ecx++; ecx &= WARPMASK;
				if (al & 0x04) dst_ptr[ecx] = ax;  ecx++; ecx &= WARPMASK;
				if (al & 0x08) dst_ptr[ecx] = ax;  ecx++; ecx &= WARPMASK;
				if (al & 0x10) dst_ptr[ecx] = ax;  ecx++; ecx &= WARPMASK;
				if (al & 0x20) dst_ptr[ecx] = ax;  ecx++; ecx &= WARPMASK;
				if (al & 0x40) dst_ptr[ecx] = ax;  ecx++; ecx &= WARPMASK;
				if (al & 0x80) dst_ptr[ecx] = ax;
				dst_ptr += SCREEN_WIDTH;
			} while (++edx);
			#undef WARPMASK
		}

	} else

//--------------------------------------------------------------------------

	// RGB horizontal bar(foreground only)
	if (command == HORIZBAR && flags & COLOR_ON && !(layer & 1))
	{
		#define WARP_LINE_COMMON { \
			if (ecx & 1) { ecx--; *dst_ptr = (UINT16)eax; dst_ptr++; } \
			dst_ptr += ecx; ecx = -ecx; \
			while (ecx) { *(UINT32*)(dst_ptr+ecx) = eax; ecx += 2; } \
		}

		src1_ptr = src_base + src1;
		src2_ptr = src_base + src2;
		hclip = y + h;

		if (!(flags & S2_IDLE))
		{
			// double source mode
			src2_ptr += GFX_HI;
			wclip = x + w;
			w = 32;
		}
		else
		{
			// single source mode
			if (color & 4) src1_ptr += GFX_HI;
			wclip = 32;
		}

		for (xclip=x; xclip<wclip; xclip+=32)
		{
			xclip &= XMASK;
			edx = xclip + w - SCREEN_WIDTH;

			for (yclip=y; yclip<hclip; yclip++)
			{
				eax = (UINT32)*src1_ptr;
				src1_ptr += src_dx;
				if (!eax) continue;
				eax = eax | (eax<<16) | ((BG_RGB<<16)|BG_RGB);
				edi = dst_base + ((yclip & YMASK) << SCREEN_WIDTH_L2);

				if (edx > 0)
				{
					ecx = edx;
					dst_ptr = (UINT16*)edi;
					WARP_LINE_COMMON
					ecx = SCREEN_WIDTH - xclip;
				} else ecx = w;

				dst_ptr = (UINT16*)edi + xclip;
				WARP_LINE_COMMON
			}

			edi = src1_ptr; src1_ptr = src2_ptr; src2_ptr = (UINT8*)edi;
		}

		#undef WARP_LINE_COMMON
	}


//--------------------------------------------------------------------------

} // end of blit()


// draws Ben Bero Beh's color backdrop(verification required)
WRITE8_MEMBER(halleys_state::bgtile_w)
{
	int yskip, xskip, ecx;
	UINT16 *edi;
	UINT16 ax;

	m_cpu1_base[0x1f00+offset] = data;
	offset -= 0x18;

	if (offset >= 191) return;
	yskip = offset / 48;
	xskip = offset % 48;
	if (xskip > 43) return;

	yskip = yskip * 48 + 24;
	xskip = xskip * 5 + 2;

	edi = m_render_layer[2] + (yskip<<SCREEN_WIDTH_L2) + xskip + (48<<SCREEN_WIDTH_L2);
	ecx = -(48<<SCREEN_WIDTH_L2);
	ax = (UINT16)data | BG_RGB;

	do { edi[ecx] = edi[ecx+1] = edi[ecx+2] = edi[ecx+3] = edi[ecx+4] = ax; } while (ecx += SCREEN_WIDTH);
}


READ8_MEMBER(halleys_state::blitter_status_r)
{
	if (m_game_id==GAME_HALLEYS && space.device().safe_pc()==0x8017) return(0x55); // HACK: trick SRAM test on startup

	return(0);
}


READ8_MEMBER(halleys_state::blitter_r)
{
	int i = offset & 0xf;

	if (i==0 || i==4) return(1);

	return(m_blitter_ram[offset]);
}


TIMER_CALLBACK_MEMBER(halleys_state::blitter_reset)
{
	m_blitter_busy = 0;
}


WRITE8_MEMBER(halleys_state::blitter_w)
{
	int i = offset & 0xf;

	m_blitter_ram[offset] = data;

	if (i==0) blit(offset);

	if (m_game_id == GAME_BENBEROB)
	{
		if (i==0 || (i==4 && !data))
		{
			m_blitter_busy = 0;
			if (m_firq_level) m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE); // make up delayed FIRQ's
		}
		else
		{
			m_blitter_busy = 1;
			m_blitter_reset_timer->adjust(downcast<cpu_device *>(&space.device())->cycles_to_attotime(100)); // free blitter if no updates in 100 cycles
		}
	}
}


READ8_MEMBER(halleys_state::collision_id_r)
{
/*
    Collision detection abstract:

    The blitter hardware does per-pixel comparison between two sprite groups.
    When a collision occurs the ID's(memory offsets/16) of the intersecting pair
    are written to ff8b(certain) and ff67(?). Then the blitter causes an alternate
    IRQ to alert the CPU and a second check is performed in software to verify the
    collision.

    The trial emulation of the above was painfully slow and inaccurate because
    collisions are not verified immediately during IRQ. Instead, a dedicated loop
    is run between game logic and VBLANK to constantly check whether new collision
    ID's have been reported. The blitter somehow knows to IRQ at the right moment
    but I couldn't find any clear-cut triggers within the register area.

    It is possible to bypass blitter-side collision altogether by feeding a redundant
    sprite list to the main CPU, given that the processor is fast enough. This method
    works reliably at 5MHz or above and will certainly break under 4MHz.

    UPDATE: re-implemented pixel collision to accompany the hack method.
*/

	if (m_game_id==GAME_HALLEYS && space.device().safe_pc()==m_collision_detection) // HACK: collision detection bypass
	{
		if (m_collision_count) { m_collision_count--; return(m_collision_list[m_collision_count]); }

		return(0);
	}

	return(m_io_ram[0x66]);
}


//**************************************************************************
// Video Initializations and Updates

PALETTE_INIT_MEMBER(halleys_state, halleys)
{
	UINT32 d, r, g, b, i, j, count;
	// allocate memory for internal palette
	m_internal_palette = std::make_unique<UINT32[]>(PALETTE_SIZE);
	UINT32 *pal_ptr = m_internal_palette.get();

	for (count=0; count<1024; count++)
	{
		pal_ptr[count] = 0;
		palette.set_pen_color(count, rgb_t(0, 0, 0));
	}

	// 00-31: palette RAM(ffc0-ffdf)

	// 32-63: colors decoded through unknown PROMs

	// 64-255: unused

	// 256-1023: palette mirrors

	// 1024-1279: gray shades
	for (i=0; i<16; i++)
	{
		d = (i<<6&0xc0) | (i<<2&0x30) | (i&0x0c) | (i>>2&0x03) | BG_RGB;
		for (count=0; count<16; count++)
		{
			r = i << 4;
			g = r + count + BG_MONO;
			r += i;
			pal_ptr[g] = d;
			palette.set_pen_color(g, rgb_t(r, r, r));
		}
	}

	// 1280-1535: RGB
	for (d=0; d<256; d++)
	{
		j = d + BG_RGB;
		pal_ptr[j] = j;

		i = d>>6 & 0x03;
		r = d>>2 & 0x0c; r |= i;
		g = d    & 0x0c; g |= i;
		b = d<<2 & 0x0c; b |= i;

		palette.set_pen_color(j, pal4bit(r), pal4bit(g), pal4bit(b));
	}
}

void halleys_state::halleys_decode_rgb(UINT32 *r, UINT32 *g, UINT32 *b, int addr, int data)
{
/*
    proms contain:
        00 00 00 00 c5 0b f3 17 fd cf 1f ef df bf 7f ff
        00 00 00 00 01 61 29 26 0b f5 e2 17 57 fb cf f7
*/
	int latch1_273, latch2_273;
	UINT8 *sram_189;
	UINT8 *prom_6330;

	int bit0, bit1, bit2, bit3, bit4;

	// the four 16x4-bit SN74S189 SRAM chips are assumed be the game's 32-byte palette RAM
	sram_189 = &m_paletteram[0];

	// each of the three 32-byte 6330 PROM is wired to an RGB component output
	prom_6330 = memregion("proms")->base();

	// latch1 holds 8 bits from the selected palette RAM address
	latch1_273 = sram_189[addr];

	// latch2 holds another 8 bits from somewhere on the data bus in a bit-swapped manner(inaccurate)
	latch2_273 = (data>>4 & 0x0f)|(data<<1 & 0x10)|(data<<3 & 0x20)|(data<<5 & 0x40)|(data<<7 & 0x80);

	// data from latch1 and 2 are combined and then decoded through PROMs to form three 8-bit tripplets
	bit0 = latch1_273>>5 & 0x01;
	bit1 = latch1_273>>3 & 0x02;

	bit2 = latch1_273>>5 & 0x04;
	bit3 = latch1_273>>3 & 0x08;
	bit4 = latch2_273>>2 & 0x10;
	*r = prom_6330[0x00 + (bit0|bit1|bit2|bit3|bit4)];

	bit2 = latch1_273>>0 & 0x04;
	bit3 = latch1_273>>0 & 0x08;
	bit4 = latch2_273>>3 & 0x10;
	*g = prom_6330[0x20 + (bit0|bit1|bit2|bit3|bit4)];

	bit2 = latch1_273<<2 & 0x04;
	bit3 = latch1_273<<2 & 0x08;
	bit4 = latch2_273<<1 & 0x10;
	*b = prom_6330[0x40 + (bit0|bit1|bit2|bit3|bit4)];
}

READ8_MEMBER(halleys_state::paletteram_r)
{
	return m_paletteram[offset];
}

WRITE8_MEMBER(halleys_state::paletteram_w)
{
	UINT32 d, r, g, b, i, j;
	UINT32 *pal_ptr = m_internal_palette.get();

	m_paletteram[offset] = data;
	d = (UINT32)data;
	j = d | BG_RGB;
	pal_ptr[offset] = j;
	pal_ptr[offset+SP_2BACK] = j;
	pal_ptr[offset+SP_ALPHA] = j;
	pal_ptr[offset+SP_COLLD] = j;

	// 8-bit RGB format: IIRRGGBB
	i = d>>6 & 0x03;
	r = d>>2 & 0x0c; r |= i;  r = r<<4 | r;
	g = d    & 0x0c; g |= i;  g = g<<4 | g;
	b = d<<2 & 0x0c; b |= i;  b = b<<4 | b;

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
	m_palette->set_pen_color(offset+SP_2BACK, rgb_t(r, g, b));
	m_palette->set_pen_color(offset+SP_ALPHA, rgb_t(r, g, b));
	m_palette->set_pen_color(offset+SP_COLLD, rgb_t(r, g, b));

	halleys_decode_rgb(&r, &g, &b, offset, 0);
	m_palette->set_pen_color(offset+0x20, rgb_t(r, g, b));
}


void halleys_state::video_start()
{
#define HALLEYS_Y0  0x8e
#define HALLEYS_X0  0x9a
#define HALLEYS_Y1  0xa2
#define HALLEYS_X1  0xa3

	int dst, src, c;

	// create short cuts to scroll registers
	m_scrolly0 = m_io_ram + HALLEYS_Y0;
	m_scrollx0 = m_io_ram + HALLEYS_X0;
	m_scrolly1 = m_io_ram + HALLEYS_Y1;
	m_scrollx1 = m_io_ram + HALLEYS_X1;

	// fill alpha table
	for (src=0; src<256; src++)
	for (dst=0; dst<256; dst++)
	{
		c  = (((src&0xc0)+(dst&0xc0))>>1) & 0xc0;
		c += (((src&0x30)+(dst&0x30))>>1) & 0x30;
		c += (((src&0x0c)+(dst&0x0c))>>1) & 0x0c;
		c += (((src&0x03)+(dst&0x03))>>1) & 0x03;

		m_alpha_table[(src<<8)+dst] = c | BG_RGB;
	}

	m_paletteram.resize(m_palette->entries());
	m_palette->basemem().set(m_paletteram, ENDIANNESS_LITTLE, 1);

	m_bgcolor         = m_palette->black_pen();
}


void halleys_state::copy_scroll_op(bitmap_ind16 &bitmap, UINT16 *source, int sx, int sy)
{
//--------------------------------------------------------------------------

#define OPCOPY_COMMON { \
	memcpy(edi, esi+sx, rcw); \
	memcpy((UINT8*)edi+rcw, esi, CLIP_BYTEW-rcw); \
	esi += SCREEN_WIDTH; \
	edi += edx; }

//--------------------------------------------------------------------------

	UINT16 *esi, *edi;
	int rcw, bch, ecx, edx;

	sx = -sx & 0xff;
	sy = -sy & 0xff;

	if ((rcw = CLIP_W - sx) < 0) rcw = 0; else rcw <<= 1;
	if ((bch = CLIP_H - sy) < 0) bch = 0;

	esi = source + CLIP_SKIP + (sy << SCREEN_WIDTH_L2);
	edi = &bitmap.pix16(VIS_MINY, VIS_MINX);
	edx = bitmap.rowpixels();

	// draw top split
	for (ecx=bch; ecx; ecx--) OPCOPY_COMMON

	esi = source + CLIP_SKIP;

	// draw bottom split
	for (ecx=CLIP_H-bch; ecx; ecx--) OPCOPY_COMMON

#undef OPCOPY_COMMON
}


void halleys_state::copy_scroll_xp(bitmap_ind16 &bitmap, UINT16 *source, int sx, int sy)
{
//--------------------------------------------------------------------------

#define XCOPY_COMMON \
	if (ecx) { \
		if (ecx & 1) { ecx--; ax = *esi; esi++; if (ax) *edi = ax; edi++; } \
		esi += ecx; edi += ecx; ecx = -ecx; \
		while (ecx) { \
			ax = esi[ecx]; \
			bx = esi[ecx+1]; \
			if (ax) edi[ecx] = ax; \
			if (bx) edi[ecx+1] = bx; \
			ecx += 2; \
		} \
	}

//--------------------------------------------------------------------------

#define YCOPY_COMMON { \
	esi = src_base + sx; ecx = rcw; XCOPY_COMMON \
	esi = src_base; ecx = CLIP_W - rcw; XCOPY_COMMON \
	src_base += SCREEN_WIDTH; \
	edi += dst_adv; \
}

//--------------------------------------------------------------------------

	int rcw, bch, dst_adv;

	UINT16 *src_base, *esi, *edi;
	int ecx, edx;
	UINT16 ax, bx;

	sx = -sx & 0xff;
	sy = -sy & 0xff;

	if ((rcw = CLIP_W - sx) < 0) rcw = 0;
	if ((bch = CLIP_H - sy) < 0) bch = 0;

	src_base = source + CLIP_SKIP + (sy << SCREEN_WIDTH_L2);
	edi = &bitmap.pix16(VIS_MINY, VIS_MINX);
	dst_adv = bitmap.rowpixels() - CLIP_W;

	// draw top split
	for (edx=bch; edx; edx--) YCOPY_COMMON

	src_base = source + CLIP_SKIP;

	// draw bottom split
	for (edx=CLIP_H-bch; edx; edx--) YCOPY_COMMON

#undef XCOPY_COMMON
#undef YCOPY_COMMON
}



void halleys_state::copy_fixed_xp(bitmap_ind16 &bitmap, UINT16 *source)
{
	UINT16 *esi, *edi;
	int dst_pitch, ecx, edx;
	UINT16 ax, bx;

	esi = source + CLIP_SKIP + CLIP_W;
	edi = &bitmap.pix16(VIS_MINY, VIS_MINX + CLIP_W);
	dst_pitch = bitmap.rowpixels();
	ecx = -CLIP_W;
	edx = CLIP_H;

	do {
		do {
			ax = esi[ecx];
			bx = esi[ecx+1];
			if (ax) edi[ecx  ] = ax; ax = esi[ecx+2];
			if (bx) edi[ecx+1] = bx; bx = esi[ecx+3];
			if (ax) edi[ecx+2] = ax; ax = esi[ecx+4];
			if (bx) edi[ecx+3] = bx; bx = esi[ecx+5];
			if (ax) edi[ecx+4] = ax; ax = esi[ecx+6];
			if (bx) edi[ecx+5] = bx; bx = esi[ecx+7];
			if (ax) edi[ecx+6] = ax;
			if (bx) edi[ecx+7] = bx;
		}
		while (ecx += 8);

		ecx = -CLIP_W;
		esi += SCREEN_WIDTH;
		edi += dst_pitch;
	}
	while (--edx);
}


void halleys_state::copy_fixed_2b(bitmap_ind16 &bitmap, UINT16 *source)
{
	UINT16 *esi, *edi;
	int dst_pitch, ecx, edx;
	UINT16 ax, bx;

	esi = source + CLIP_SKIP + CLIP_W;
	edi = &bitmap.pix16(VIS_MINY, VIS_MINX + CLIP_W);
	dst_pitch = bitmap.rowpixels();
	ecx = -CLIP_W;
	edx = CLIP_H;

	do {
		do {
			ax = esi[ecx];
			bx = esi[ecx+1];

			if (!(ax)) goto SKIP0; if (!(ax&SP_2BACK)) goto DRAW0; if (edi[ecx  ]) goto SKIP0;
			DRAW0: edi[ecx  ] = ax; SKIP0: ax = esi[ecx+2];
			if (!(bx)) goto SKIP1; if (!(bx&SP_2BACK)) goto DRAW1; if (edi[ecx+1]) goto SKIP1;
			DRAW1: edi[ecx+1] = bx; SKIP1: bx = esi[ecx+3];

			if (!(ax)) goto SKIP2; if (!(ax&SP_2BACK)) goto DRAW2; if (edi[ecx+2]) goto SKIP2;
			DRAW2: edi[ecx+2] = ax; SKIP2: ax = esi[ecx+4];
			if (!(bx)) goto SKIP3; if (!(bx&SP_2BACK)) goto DRAW3; if (edi[ecx+3]) goto SKIP3;
			DRAW3: edi[ecx+3] = bx; SKIP3: bx = esi[ecx+5];

			if (!(ax)) goto SKIP4; if (!(ax&SP_2BACK)) goto DRAW4; if (edi[ecx+4]) goto SKIP4;
			DRAW4: edi[ecx+4] = ax; SKIP4: ax = esi[ecx+6];
			if (!(bx)) goto SKIP5; if (!(bx&SP_2BACK)) goto DRAW5; if (edi[ecx+5]) goto SKIP5;
			DRAW5: edi[ecx+5] = bx; SKIP5: bx = esi[ecx+7];

			if (!(ax)) goto SKIP6; if (!(ax&SP_2BACK)) goto DRAW6; if (edi[ecx+6]) goto SKIP6;
			DRAW6: edi[ecx+6] = ax; SKIP6:
			if (!(bx)) continue;   if (!(bx&SP_2BACK)) goto DRAW7; if (edi[ecx+7]) continue;
			DRAW7: edi[ecx+7] = bx;
		}
		while (ecx += 8);

		ecx = -CLIP_W;
		esi += SCREEN_WIDTH;
		edi += dst_pitch;
	}
	while (--edx);
}


void halleys_state::filter_bitmap(bitmap_ind16 &bitmap, int mask)
{
	int dst_pitch;

	UINT32 *pal_ptr, *edi;
	int esi, eax, ebx, ecx, edx;

	pal_ptr = m_internal_palette.get();
	esi = mask | 0xffffff00;
	edi = (UINT32*)&bitmap.pix16(VIS_MINY, VIS_MINX + CLIP_W);
	dst_pitch = bitmap.rowpixels() >> 1;
	ecx = -(CLIP_W>>1);
	edx = CLIP_H;

	do {
		do {
			eax = edi[ecx];
			if (eax & 0x00ff00ff)
			{
				ebx = eax;
				eax >>= 16;
				ebx &= 0xffff;
				eax = pal_ptr[eax];
				ebx = pal_ptr[ebx];
				eax &= esi;
				ebx &= esi;
				eax <<= 16;
				ebx |= eax;
				edi[ecx] = ebx;
			}
		}
		while (++ecx);

		ecx = -(CLIP_W>>1);
		edi += dst_pitch;
	}
	while (--edx);
}


UINT32 halleys_state::screen_update_halleys(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i, j;

	if (m_stars_enabled)
	{
		copy_scroll_op(bitmap, m_render_layer[5], *m_scrollx0, *m_scrolly0);
		copy_scroll_xp(bitmap, m_render_layer[4], *m_scrollx1, *m_scrolly1);
	}
	else
		bitmap.fill(m_bgcolor, cliprect);

#ifdef MAME_DEBUG
	if (ioport("DEBUG")->read()) copy_scroll_xp(bitmap, m_render_layer[3], *m_scrollx0, *m_scrolly0); // not used???
#endif

	copy_scroll_xp(bitmap, m_render_layer[2], *m_scrollx1, *m_scrolly1);
	copy_fixed_2b (bitmap, m_render_layer[1]);
	copy_fixed_xp (bitmap, m_render_layer[0]);

	// HALF-HACK: apply RGB filter when the following conditions are met
	i = m_io_ram[0xa0];
	j = m_io_ram[0xa1];
	if (m_io_ram[0x2b] && (i>0xc6 && i<0xfe) && (j==0xc0 || j==0xed)) filter_bitmap(bitmap, i);
	return 0;
}


UINT32 halleys_state::screen_update_benberob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_io_ram[0xa0] & 0x80)
		copy_scroll_op(bitmap, m_render_layer[2], *m_scrollx1, *m_scrolly1);
	else
		bitmap.fill(m_bgcolor, cliprect);

	copy_fixed_xp (bitmap, m_render_layer[1]);
	copy_fixed_xp (bitmap, m_render_layer[0]);
	return 0;
}


//**************************************************************************
// Debug and Test Handlers

#if HALLEYS_DEBUG

READ8_MEMBER(halleys_state::zero_r){ return(0); }

READ8_MEMBER(halleys_state::debug_r)
{
	return(m_io_ram[offset]);
}

#endif


//**************************************************************************
// Interrupt and Hardware Handlers


TIMER_DEVICE_CALLBACK_MEMBER(halleys_state::halleys_scanline)
{
	int scanline = param;

	/* TODO: fix this */
	switch (scanline)
	{
		case 248:
			// clear collision list of this frame unconditionally
			m_collision_count = 0;
			break;

		// In Halley's Comet, NMI is used exclusively to handle coin input
		case 56*3:
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
			break;

		// FIRQ drives gameplay; we need both types of NMI each frame.
		case 56*2:
			m_mVectorType = 1; m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
			break;

		case 56:
			m_mVectorType = 0; m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
			break;
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(halleys_state::benberob_scanline)
{
	int scanline = param;

	/* TODO: fix this */
	switch (scanline)
	{
		case 248:
			break;

		case 56*3:
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
			break;

		case 56*2:
		case 56*1:
			// FIRQ must not happen when the blitter is being updated or it'll cause serious screen artifacts
			if (!m_blitter_busy) m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE); else m_firq_level++;
			break;
	}
}


READ8_MEMBER(halleys_state::vector_r)
{
	return(m_cpu1_base[0xffe0 + (offset^(m_mVectorType<<4))]);
}


WRITE8_MEMBER(halleys_state::firq_ack_w)
{
	m_io_ram[0x9c] = data;

	if (m_firq_level) m_firq_level--;
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}


WRITE8_MEMBER(halleys_state::sndnmi_msk_w)
{
	m_sndnmi_mask = data & 1;
}


WRITE8_MEMBER(halleys_state::soundcommand_w)
{
	m_io_ram[0x8a] = data;
	soundlatch_byte_w(space,offset,data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


READ8_MEMBER(halleys_state::coin_lockout_r)
{
	// This is a hack, but it lets you coin up when COIN1 or COIN2 are signaled.
	// See NMI for the twisted logic that is involved in handling coin input :
	//   0x8599 : 'benberob'
	//   0x83e2 : 'halleys', 'halleysc', 'halleycj'
	//   0x83df : 'halley87'
	int inp = ioport("IN0")->read();
	int result = ((ioport("DSW4")->read()) & 0x20) >> 5;

	if (inp & 0x80) result |= 0x02;
	if (inp & 0x40) result |= 0x04;

	return(result);
}


READ8_MEMBER(halleys_state::io_mirror_r)
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "IN3" };

	return ioport(portnames[offset])->read();
}


//**************************************************************************
// Memory Maps

static ADDRESS_MAP_START( halleys_map, AS_PROGRAM, 8, halleys_state )
	AM_RANGE(0x0000, 0x0fff) AM_READWRITE(blitter_r, blitter_w) AM_SHARE("blitter_ram")
	AM_RANGE(0x1f00, 0x1fff) AM_WRITE(bgtile_w)     // background tiles?(Ben Bero Beh only)
	AM_RANGE(0x1000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xfeff) AM_RAM                 // work ram

	AM_RANGE(0xff66, 0xff66) AM_READ(collision_id_r) // HACK: collision detection bypass(Halley's Comet only)
	AM_RANGE(0xff71, 0xff71) AM_READ(blitter_status_r)
	AM_RANGE(0xff80, 0xff83) AM_READ(io_mirror_r)
	AM_RANGE(0xff8a, 0xff8a) AM_WRITE(soundcommand_w)
	AM_RANGE(0xff90, 0xff90) AM_READ_PORT("IN0")    // coin/start
	AM_RANGE(0xff91, 0xff91) AM_READ_PORT("IN1")    // player 1
	AM_RANGE(0xff92, 0xff92) AM_READ_PORT("IN2")    // player 2
	AM_RANGE(0xff93, 0xff93) AM_READ_PORT("IN3")    // unused?
	AM_RANGE(0xff94, 0xff94) AM_READ(coin_lockout_r)
	AM_RANGE(0xff95, 0xff95) AM_READ_PORT("DSW1")   // dipswitch 4
	AM_RANGE(0xff96, 0xff96) AM_READ_PORT("DSW2")   // dipswitch 3
	AM_RANGE(0xff97, 0xff97) AM_READ_PORT("DSW3")   // dipswitch 2
	AM_RANGE(0xff9c, 0xff9c) AM_WRITE(firq_ack_w)
	AM_RANGE(0xff00, 0xffbf) AM_RAM AM_SHARE("io_ram")  // I/O write fall-through

	AM_RANGE(0xffc0, 0xffdf) AM_READWRITE(paletteram_r, paletteram_w)
	AM_RANGE(0xffe0, 0xffff) AM_READ(vector_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, halleys_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x4800, 0x4801) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x4801, 0x4801) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0x4802, 0x4803) AM_DEVWRITE("ay3", ay8910_device, address_data_w)
	AM_RANGE(0x4803, 0x4803) AM_DEVREAD("ay3", ay8910_device, data_r)
	AM_RANGE(0x4804, 0x4805) AM_DEVWRITE("ay4", ay8910_device, address_data_w)
	AM_RANGE(0x4805, 0x4805) AM_DEVREAD("ay4", ay8910_device, data_r)
	AM_RANGE(0x5000, 0x5000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe000, 0xefff) AM_ROM // space for diagnostic ROM
ADDRESS_MAP_END


//**************************************************************************
// Port Maps

static INPUT_PORTS_START( benberob )
	PORT_START("DSW1")  /* 0xff95 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )       /* code at 0xb00e */
	PORT_DIPSETTING(    0x02, "Every 100k" )
	PORT_DIPSETTING(    0x03, "100k 300k 200k+" )
	PORT_DIPSETTING(    0x00, "100k 200k" )
	PORT_DIPSETTING(    0x01, "150k 300k" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPUNKNOWN( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )      /* TO DO : confirm OFF/ON when screen flipping emulated */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")  /* 0xff96 */
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSW3")  /* 0xff97 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, "Use Both Buttons" )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Timer Speed" )               /* table at 0xb10e */
	PORT_DIPSETTING(    0x0c, "Slowest" )
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x04, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x10, 0x10, "Show Coinage" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Show Year" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "No Hit" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Maximum Credits" )
	PORT_DIPSETTING(    0x80, "9" )
	PORT_DIPSETTING(    0x00, "16" )

	PORT_START("DSW4")  /* 0xff94 - read by coin_lockout_r */
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x20, "Coin Slots" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN0")   /* 0xff90 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_IMPULSE(12)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")   /* 0xff91 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")   /* 0xff92 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN3")   /* 0xff93 */
INPUT_PORTS_END


static INPUT_PORTS_START( halleys )
	PORT_START("DSW1")  /* 0xff95 */
	TAITO_MACHINE_COCKTAIL_LOC(SW4)
	TAITO_COINAGE_JAPAN_OLD_LOC(SW4)

	PORT_START("DSW2")  /* 0xff96 */
	TAITO_DIFFICULTY_LOC(SW3)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW3:3,4")   /* index of tables at 0x1a2f or 0x19b5 */
	PORT_DIPSETTING(    0x00, "100k 600k 500k+" )           /* last bonus life at 5600k : max. 12 bonus lives */
	PORT_DIPSETTING(    0x0c, "200k 800k 600k+" )           /* last bonus life at 6800k : max. 12 bonus lives */
	PORT_DIPSETTING(    0x08, "200k 1000k 800k+" )          /* last bonus life at 9000k : max. 12 bonus lives */
	PORT_DIPSETTING(    0x04, "200k 1200k 1000k+" )         /* last bonus life at 9200k : max. 10 bonus lives */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "4 (No Points Bonus Lives)" ) /* code at 0xa509 or 0xa502 - 3 in the US manual */
	PORT_DIPNAME( 0x40, 0x40, "Operation Data Recorder" )   PORT_DIPLOCATION("SW3:7")     /* from the US manual - what is this ? */
	PORT_DIPSETTING(    0x40, "Not fixed" )
	PORT_DIPSETTING(    0x00, "When fixed" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:8" )

	/* From US manual : "DIP SW 2 is not used and all contacts should be set off."
	   However, they enable debug features if you press START1 during the boot sequence. */
	PORT_START("DSW3")  /* 0xff97 */
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )         PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, "Start Area" )        PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING( 0x1c, "1 (Earth)" )
	PORT_DIPSETTING( 0x18, "4 (Venus)" )
	PORT_DIPSETTING( 0x14, "7 (Mercury)" )
	PORT_DIPSETTING( 0x10, "10 (Sun)" )
	PORT_DIPSETTING( 0x0c, "13 (Pluto)" )                   /* spelled "Plato" */
	PORT_DIPSETTING( 0x08, "16 (Neptune)" )
	PORT_DIPSETTING( 0x04, "19 (Uranus)" )
	PORT_DIPSETTING( 0x00, "22 (Saturn)" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPNAME( 0x40, 0x40, "Invincibility")      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Infinite Lives" )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	/* From US manual : "Coin mechs system can be optioned by setting DIP SW 1.
	   Position 6 on for single coin selector.  Position 6 off for twin coin selector." */
	PORT_START("DSW4")  /* 0xff94 - read by coin_lockout_r */
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPNAME( 0x20, 0x20, "Coin Slots" )                PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )

	PORT_START("IN0")   /* 0xff90 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_SERVICE1 ) PORT_IMPULSE(12)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")   /* 0xff91 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN2")   /* 0xff92 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN3")   /* 0xff93 */

#ifdef MAME_DEBUG
	PORT_START("DEBUG") /* just to be safe */
	PORT_DIPNAME( 0x01, 0x00, "Show Unused Layer" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
#endif
INPUT_PORTS_END


//**************************************************************************
// Machine Definitions and Initializations

void halleys_state::machine_reset()
{
	m_mVectorType     = 0;
	m_firq_level      = 0;
	m_blitter_busy    = 0;
	m_collision_count = 0;
	m_stars_enabled   = 0;
	m_fftail = m_ffhead = m_ffcount = 0;

	memset(m_io_ram, 0xff, m_io_ram.bytes());
	memset(m_render_layer[0], 0, SCREEN_BYTESIZE * MAX_LAYERS);
}


static MACHINE_CONFIG_START( halleys, halleys_state )
	MCFG_CPU_ADD("maincpu", M6809, XTAL_19_968MHz/12) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(halleys_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", halleys_state, halleys_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_6MHz/2) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(halleys_state, irq0_line_hold,  (double)6000000/(4*16*16*10*16))


	// video hardware

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.50) /* verified on PCB */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(SCREEN_WIDTH, SCREEN_HEIGHT)
	MCFG_SCREEN_VISIBLE_AREA(VIS_MINX, VIS_MAXX, VIS_MINY, VIS_MAXY)
	MCFG_SCREEN_UPDATE_DRIVER(halleys_state, screen_update_halleys)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", PALETTE_SIZE)
	MCFG_PALETTE_INIT_OWNER(halleys_state, halleys)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_6MHz/4) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_6MHz/4) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay3", AY8910, XTAL_6MHz/4) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay4", AY8910, XTAL_6MHz/4) /* verified on pcb */
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(halleys_state, sndnmi_msk_w)) // port Bwrite
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( benberob, halleys )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(XTAL_19_968MHz/12) /* not verified but pcb identical to halley's comet */
	MCFG_TIMER_MODIFY("scantimer")
	MCFG_TIMER_DRIVER_CALLBACK(halleys_state, benberob_scanline)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(halleys_state, screen_update_benberob)
MACHINE_CONFIG_END


//**************************************************************************
// ROM Definitions

ROM_START( benberob )
	ROM_REGION( 0x10000, "maincpu", 0 ) //MAIN PRG
	ROM_LOAD( "a26_01.31",   0x4000, 0x4000, CRC(9ed566ba) SHA1(15c042e727b00b1dc6f24c72226d1a361fc0fa58) )
	ROM_LOAD( "a26_02.52",   0x8000, 0x4000, CRC(a563a033) SHA1(c2c4a73f190303b7101e7849a638d35a80e4c36b) )
	ROM_LOAD( "a26_03.50",   0xc000, 0x4000, CRC(975849ef) SHA1(2724c3b7cca724bee5ae8331037529bfd8285011) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) //SOUND
	ROM_LOAD( "a26_12.5",    0x0000, 0x4000, CRC(7fd728f3) SHA1(772c14d9254e43a56f1e67ad4dd5d7840df50e34) )

	ROM_REGION( 0x20000, "gfx1", 0 ) //CHR
	ROM_LOAD( "a26_06.78",   0x00000, 0x4000, CRC(79ae2e58) SHA1(726dbaf01dcb330a8a7bd69578b67ec48db01ae1) )
	ROM_LOAD( "a26_07.77",   0x04000, 0x4000, CRC(fe976343) SHA1(68b655d00b1e7e72ab81974b39033f6c130dc829) )
	ROM_LOAD( "a26_04.80",   0x08000, 0x4000, CRC(77d10723) SHA1(fd284de30c7740c56e7192e0d73a6939b8386538) )
	ROM_LOAD( "a26_05.79",   0x0c000, 0x4000, CRC(098eebcc) SHA1(3dd82d92273b4d1ebbe0f81f6d367c8bc642815f) )
	ROM_LOAD( "a26_10.89",   0x10000, 0x4000, CRC(3d406060) SHA1(f02df4957ad7ba59ac14fd0134d856b2a80c9342) )
	ROM_LOAD( "a26_11.88",   0x14000, 0x4000, CRC(4561836a) SHA1(6bd582ddb980a1c0b1ed4981118d5a9c862ff89c) )
	ROM_LOAD( "a26_08.91",   0x18000, 0x4000, CRC(7e63059d) SHA1(01e1e805fa2e05058ebd83aae95e26879b4a275d) )
	ROM_LOAD( "a26_09.90",   0x1c000, 0x4000, CRC(ebd9a16c) SHA1(0230892f451cac310d5ad0d328cea0556b96157f) )

	ROM_REGION( 0x0060, "proms", 0 ) //COLOR (all identical!)
	ROM_LOAD( "a26_13.r",    0x0000, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
	ROM_LOAD( "a26_13.g",    0x0020, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
	ROM_LOAD( "a26_13.b",    0x0040, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
ROM_END


ROM_START( halleys )
	ROM_REGION( 0x10000, "maincpu", 0 ) //MAIN PRG
	ROM_LOAD( "a62_01.30",   0x0000, 0x4000, CRC(a5e82b3e) SHA1(c16c6a6c23a579454b8a2be4b951c35b04f2a856) )
	ROM_LOAD( "a62_02.31",   0x4000, 0x4000, CRC(25f5bcd3) SHA1(9d72afe866df363d2ac33dab3ed6c3913f4de12d) )
	ROM_LOAD( "a62-15.52",   0x8000, 0x4000, CRC(e65d8312) SHA1(29870fe0dbb30d23970a8a816849dc5807d70675) )
	ROM_LOAD( "a62_04.50",   0xc000, 0x4000, CRC(fad74dfe) SHA1(92c0d42c5e186bc07c168ad581e52a5ae340c2b2) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) //SOUND
	ROM_LOAD( "a62_13.5",    0x0000, 0x2000, CRC(7ce290db) SHA1(e3c72ba5d97cb07f0f72d2765a068af6fb5cca29) )
	ROM_LOAD( "a62_14.4",    0x2000, 0x2000, CRC(ea74b1a2) SHA1(7be3b9e9d51cfa753ce97e92f7eebd9723fe5821) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "a62_12.78",   0x00000, 0x4000, CRC(c5834a7a) SHA1(4a24b3fa707cde89ad5a52d4e994412fcf28e81f) )
	ROM_LOAD( "a62_10.77",   0x04000, 0x4000, CRC(3ae7231e) SHA1(277f12570001d82104c79d3d0a58a0b57ed18778) )
	ROM_LOAD( "a62_08.80",   0x08000, 0x4000, CRC(b9210dbe) SHA1(f72f2307e9acd2dd622a3efce71bd334b68a9b60) )
	ROM_LOAD( "a62-16.79",   0x0c000, 0x4000, CRC(1165a622) SHA1(6a0b4a3eadf157f89c69b5202bd4f895f92a3ef1) ) //bad?
	ROM_LOAD( "a62_11.89",   0x10000, 0x4000, CRC(d0e9974e) SHA1(6826cfb4fbf098ed7b9d8b00e2684d7c85a13c11) )
	ROM_LOAD( "a62_09.88",   0x14000, 0x4000, CRC(e93ef281) SHA1(8bfe1ecce1c7107a5bd1b43b531594c8cfc0719d) )
	ROM_LOAD( "a62_07.91",   0x18000, 0x4000, CRC(64c95e8b) SHA1(4c3320a764b13a5751c0019c9fafb899ea2f908f) )
	ROM_LOAD( "a62_05.90",   0x1c000, 0x4000, CRC(c3c877ef) SHA1(23180b106e50b7a2a230c5e9948832e5631972ae) )

	ROM_REGION( 0x0060, "proms", 0 ) //COLOR (all identical!)
	ROM_LOAD( "a26-13.109",  0x0000, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
	ROM_LOAD( "a26-13.110",  0x0020, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
	ROM_LOAD( "a26-13.111",  0x0040, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
ROM_END


ROM_START( halleycj )
	ROM_REGION( 0x10000, "maincpu", 0 ) //MAIN PRG
	ROM_LOAD( "a62_01.30",   0x0000, 0x4000, CRC(a5e82b3e) SHA1(c16c6a6c23a579454b8a2be4b951c35b04f2a856) )
	ROM_LOAD( "a62_02.31",   0x4000, 0x4000, CRC(25f5bcd3) SHA1(9d72afe866df363d2ac33dab3ed6c3913f4de12d) )
	ROM_LOAD( "a62_03-1.52", 0x8000, 0x4000, CRC(e2fffbe4) SHA1(a10ced7103a26fd6753765bf11b00ca018f49a48) )
	ROM_LOAD( "a62_04.50",   0xc000, 0x4000, CRC(fad74dfe) SHA1(92c0d42c5e186bc07c168ad581e52a5ae340c2b2) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) //SOUND
	ROM_LOAD( "a62_13.5",    0x0000, 0x2000, CRC(7ce290db) SHA1(e3c72ba5d97cb07f0f72d2765a068af6fb5cca29) )
	ROM_LOAD( "a62_14.4",    0x2000, 0x2000, CRC(ea74b1a2) SHA1(7be3b9e9d51cfa753ce97e92f7eebd9723fe5821) )

	ROM_REGION( 0x20000, "gfx1", 0 ) //CHR
	ROM_LOAD( "a62_12.78",   0x00000, 0x4000, CRC(c5834a7a) SHA1(4a24b3fa707cde89ad5a52d4e994412fcf28e81f) )
	ROM_LOAD( "a62_10.77",   0x04000, 0x4000, CRC(3ae7231e) SHA1(277f12570001d82104c79d3d0a58a0b57ed18778) )
	ROM_LOAD( "a62_08.80",   0x08000, 0x4000, CRC(b9210dbe) SHA1(f72f2307e9acd2dd622a3efce71bd334b68a9b60) )
	ROM_LOAD( "a62_06.79",   0x0c000, 0x4000, CRC(600be9ca) SHA1(a705b10be37ee93908b1bbaf806cfe7955aa3ffc) )
	ROM_LOAD( "a62_11.89",   0x10000, 0x4000, CRC(d0e9974e) SHA1(6826cfb4fbf098ed7b9d8b00e2684d7c85a13c11) )
	ROM_LOAD( "a62_09.88",   0x14000, 0x4000, CRC(e93ef281) SHA1(8bfe1ecce1c7107a5bd1b43b531594c8cfc0719d) )
	ROM_LOAD( "a62_07.91",   0x18000, 0x4000, CRC(64c95e8b) SHA1(4c3320a764b13a5751c0019c9fafb899ea2f908f) )
	ROM_LOAD( "a62_05.90",   0x1c000, 0x4000, CRC(c3c877ef) SHA1(23180b106e50b7a2a230c5e9948832e5631972ae) )

	ROM_REGION( 0x0060, "proms", 0 ) //COLOR (all identical!)
	ROM_LOAD( "a26-13.109",  0x0000, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
	ROM_LOAD( "a26-13.110",  0x0020, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
	ROM_LOAD( "a26-13.111",  0x0040, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
ROM_END


ROM_START( halleysc )
	ROM_REGION( 0x10000, "maincpu", 0 ) //MAIN PRG
	ROM_LOAD( "a62_01.30",   0x0000, 0x4000, CRC(a5e82b3e) SHA1(c16c6a6c23a579454b8a2be4b951c35b04f2a856) )
	ROM_LOAD( "a62_02.31",   0x4000, 0x4000, CRC(25f5bcd3) SHA1(9d72afe866df363d2ac33dab3ed6c3913f4de12d) )
	ROM_LOAD( "a62_03.52",   0x8000, 0x4000, CRC(8e90a97b) SHA1(9199628ad5353a88e4478a13c48df1ccb5d2b538) )
	ROM_LOAD( "a62_04.50",   0xc000, 0x4000, CRC(fad74dfe) SHA1(92c0d42c5e186bc07c168ad581e52a5ae340c2b2) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) //SOUND
	ROM_LOAD( "a62_13.5",    0x0000, 0x2000, CRC(7ce290db) SHA1(e3c72ba5d97cb07f0f72d2765a068af6fb5cca29) )
	ROM_LOAD( "a62_14.4",    0x2000, 0x2000, CRC(ea74b1a2) SHA1(7be3b9e9d51cfa753ce97e92f7eebd9723fe5821) )

	ROM_REGION( 0x20000, "gfx1", 0 ) //CHR
	ROM_LOAD( "a62_12.78",   0x00000, 0x4000, CRC(c5834a7a) SHA1(4a24b3fa707cde89ad5a52d4e994412fcf28e81f) )
	ROM_LOAD( "a62_10.77",   0x04000, 0x4000, CRC(3ae7231e) SHA1(277f12570001d82104c79d3d0a58a0b57ed18778) )
	ROM_LOAD( "a62_08.80",   0x08000, 0x4000, CRC(b9210dbe) SHA1(f72f2307e9acd2dd622a3efce71bd334b68a9b60) )
	ROM_LOAD( "a62_06.79",   0x0c000, 0x4000, CRC(600be9ca) SHA1(a705b10be37ee93908b1bbaf806cfe7955aa3ffc) )
	ROM_LOAD( "a62_11.89",   0x10000, 0x4000, CRC(d0e9974e) SHA1(6826cfb4fbf098ed7b9d8b00e2684d7c85a13c11) )
	ROM_LOAD( "a62_09.88",   0x14000, 0x4000, CRC(e93ef281) SHA1(8bfe1ecce1c7107a5bd1b43b531594c8cfc0719d) )
	ROM_LOAD( "a62_07.91",   0x18000, 0x4000, CRC(64c95e8b) SHA1(4c3320a764b13a5751c0019c9fafb899ea2f908f) )
	ROM_LOAD( "a62_05.90",   0x1c000, 0x4000, CRC(c3c877ef) SHA1(23180b106e50b7a2a230c5e9948832e5631972ae) )

	ROM_REGION( 0x0060, "proms", 0 ) //COLOR (all identical!)
	ROM_LOAD( "a26-13.109",  0x0000, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
	ROM_LOAD( "a26-13.110",  0x0020, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
	ROM_LOAD( "a26-13.111",  0x0040, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
ROM_END


ROM_START( halley87 )
	ROM_REGION( 0x10000, "maincpu", 0 ) //MAIN PRG
	ROM_LOAD( "a62-17.30",   0x0000, 0x4000, CRC(fa2a58a6) SHA1(42cb587aad166ff74ece987f275aa7ad16d58300) )
	ROM_LOAD( "a62-18.31",   0x4000, 0x4000, CRC(f3a078e6) SHA1(f8fa548b5814276d1ae2d575b9a5d3f0cc2f54fa) )
	ROM_LOAD( "a62-19.52",   0x8000, 0x4000, CRC(e8bb695c) SHA1(066558ec48a38db03ae10ba873ff05ab5f911658) )
	ROM_LOAD( "a62-20.50",   0xc000, 0x4000, CRC(59ed52cd) SHA1(059ee01610949a654741b853eed0235eabe0e313) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) //SOUND
	ROM_LOAD( "a62_13.5",    0x0000, 0x2000, CRC(7ce290db) SHA1(e3c72ba5d97cb07f0f72d2765a068af6fb5cca29) )
	ROM_LOAD( "a62_14.4",    0x2000, 0x2000, CRC(ea74b1a2) SHA1(7be3b9e9d51cfa753ce97e92f7eebd9723fe5821) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "a62_12.78",   0x00000, 0x4000, CRC(c5834a7a) SHA1(4a24b3fa707cde89ad5a52d4e994412fcf28e81f) )
	ROM_LOAD( "a62_10.77",   0x04000, 0x4000, CRC(3ae7231e) SHA1(277f12570001d82104c79d3d0a58a0b57ed18778) )
	ROM_LOAD( "a62_08.80",   0x08000, 0x4000, CRC(b9210dbe) SHA1(f72f2307e9acd2dd622a3efce71bd334b68a9b60) )
	ROM_LOAD( "a62_06.79",   0x0c000, 0x4000, CRC(600be9ca) SHA1(a705b10be37ee93908b1bbaf806cfe7955aa3ffc) )
	ROM_LOAD( "a62_11.89",   0x10000, 0x4000, CRC(d0e9974e) SHA1(6826cfb4fbf098ed7b9d8b00e2684d7c85a13c11) )
	ROM_LOAD( "a62_09.88",   0x14000, 0x4000, CRC(e93ef281) SHA1(8bfe1ecce1c7107a5bd1b43b531594c8cfc0719d) )
	ROM_LOAD( "a62_07.91",   0x18000, 0x4000, CRC(64c95e8b) SHA1(4c3320a764b13a5751c0019c9fafb899ea2f908f) )
	ROM_LOAD( "a62_05.90",   0x1c000, 0x4000, CRC(c3c877ef) SHA1(23180b106e50b7a2a230c5e9948832e5631972ae) )

	ROM_REGION( 0x0060, "proms", 0 ) //COLOR (all identical!)
	ROM_LOAD( "a26-13.109",  0x0000, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
	ROM_LOAD( "a26-13.110",  0x0020, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
	ROM_LOAD( "a26-13.111",  0x0040, 0x0020, CRC(ec449aee) SHA1(aa33e82b592276d5ffd540d9a73d1b48d7d4accf) )
ROM_END


//**************************************************************************
// Driver Initializations

void halleys_state::init_common()
{
	UINT8 *buf, *rom;
	int addr, i;
	UINT8 al, ah, dl, dh;


	// allocate memory for unpacked graphics
	buf = auto_alloc_array(machine(), UINT8, 0x100000);
	m_gfx_plane02 = buf;
	m_gfx_plane13 = buf + 0x80000;


	// allocate memory for render layers
	buf = auto_alloc_array(machine(), UINT8, SCREEN_BYTESIZE * MAX_LAYERS);
	for (i=0; i<MAX_LAYERS; buf+=SCREEN_BYTESIZE, i++) m_render_layer[i] = (UINT16*)buf;


	// allocate memory for pre-processed ROMs
	m_gfx1_base = std::make_unique<UINT8[]>(0x20000);


	// allocate memory for alpha table
	m_alpha_table = std::make_unique<UINT32[]>(0x10000);


	// allocate memory for hardware collision list
	m_collision_list = std::make_unique<UINT8[]>(MAX_SPRITES);


	// decrypt main program ROM
	rom = m_cpu1_base = memregion("maincpu")->base();
	buf = m_gfx1_base.get();

	for (i=0; i<0x10000; i++)
	{
		addr = BITSWAP16(i,15,14,13,12,11,10,1,0,4,5,6,3,7,8,9,2);
		buf[i] = BITSWAP8(rom[addr],0,7,6,5,1,4,2,3);
	}

	memcpy(rom, buf, 0x10000);


	// swap graphics ROM addresses and unpack each pixel
	rom = memregion("gfx1")->base();
	buf = m_gfx_plane02;

	for (i=0xffff; i>=0; i--)
	{
		al = rom[i];
		ah = rom[i+0x10000];
		m_gfx1_base[0xffff-i] = al;
		m_gfx1_base[0x1ffff-i] = ah;

		buf[0] = dl = (al    & 1) | (ah<<2 & 4);  dl <<= 1;
		buf[1] = dh = (al>>1 & 1) | (ah<<1 & 4);  dh <<= 1;
		buf[0+0x80000] = dl;
		buf[1+0x80000] = dh;
		buf[2] = dl = (al>>2 & 1) | (ah    & 4);  dl <<= 1;
		buf[3] = dh = (al>>3 & 1) | (ah>>1 & 4);  dh <<= 1;
		buf[2+0x80000] = dl;
		buf[3+0x80000] = dh;
		buf[4] = dl = (al>>4 & 1) | (ah>>2 & 4);  dl <<= 1;
		buf[5] = dh = (al>>5 & 1) | (ah>>3 & 4);  dh <<= 1;
		buf[4+0x80000] = dl;
		buf[5+0x80000] = dh;
		buf[6] = dl = (al>>6 & 1) | (ah>>4 & 4);  dl <<= 1;
		buf[7] = dh = (al>>7    ) | (ah>>5 & 4);  dh <<= 1;
		buf[6+0x80000] = dl;
		buf[7+0x80000] = dh;

		buf += 8;
	}
}


DRIVER_INIT_MEMBER(halleys_state,benberob)
{
	m_game_id = GAME_BENBEROB;

	init_common();

	m_blitter_reset_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(halleys_state::blitter_reset),this));
}


DRIVER_INIT_MEMBER(halleys_state,halleys)
{
	m_game_id = GAME_HALLEYS;
	m_collision_detection = 0xb114;

	init_common();
}

DRIVER_INIT_MEMBER(halleys_state,halley87)
{
	m_game_id = GAME_HALLEYS;
	m_collision_detection = 0xb10d;

	init_common();
}


//**************************************************************************
// Game Definitions

GAME( 1984, benberob, 0,       benberob, benberob, halleys_state, benberob,  ROT0,  "Taito", "Ben Bero Beh (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_NO_COCKTAIL )
GAME( 1986, halleys,  0,       halleys,  halleys, halleys_state,  halleys,   ROT90, "Taito America Corporation (Coin-It license)", "Halley's Comet (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 1986, halleysc, halleys, halleys,  halleys, halleys_state,  halleys,   ROT90, "Taito Corporation", "Halley's Comet (Japan, Newer)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 1986, halleycj, halleys, halleys,  halleys, halleys_state,  halleys,   ROT90, "Taito Corporation", "Halley's Comet (Japan, Older)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
GAME( 1986, halley87, halleys, halleys,  halleys, halleys_state,  halley87,  ROT90, "Taito Corporation", "Halley's Comet '87", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL )
