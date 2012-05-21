/***************************************************************************

   Taito F3 Video emulation - Bryan McPhail, mish@tendril.co.uk

****************************************************************************

Brief overview:

    4 scrolling layers (512x512 or 1024x512) of 4/5/6 bpp tiles.
    1 scrolling text layer (512x512, characters generated in vram), 4bpp chars.
    1 scrolling pixel layer (512x256 pixels generated in pivot ram), 4bpp pixels.
    2 sprite banks (for double buffering of sprites)
    Sprites can be 4, 5 or 6 bpp
    Sprite scaling.
    Rowscroll on all playfields
    Line by line zoom on all playfields
    Column scroll on all playfields
    Line by line sprite and playfield priority mixing

Notes:
    All special effects are controlled by an area in 'line ram'.  Typically
    there are 256 values, one for each line of the screen (including clipped
    lines at the top of the screen).  For example, at 0x8000 in lineram,
    there are 4 sets of 256 values (one for each playfield) and each value
    is the scale control for that line in the destination bitmap (screen).
    Therefore each line can have a different zoom value for special effects.

    This also applies to playfield priority, rowscroll, column scroll, sprite
    priority and VRAM/pivot layers.

    However - at the start of line ram there are also sets of 256 values
    controlling each effect - effects can be selectively applied to individual
    playfields or only certain lines out of the 256 can be active - in which
    case the last allowed value can be latched (typically used so a game can
    use one zoom or row value over the whole playfield).

    The programmers of some of these games made strange use of flipscreen -
    some games have all their graphics flipped in ROM, and use the flipscreen
    bit to display them correctly!.

    Most games display 232 scanlines, but some use lineram effects to clip
    themselves to 224 or less.

****************************************************************************

Line ram memory map:

    Here 'playfield 1' refers to the first playfield in memory, etc

    0x0000: Column line control ram (256 lines)
        100x:   Where bit 0 of x enables effect on playfield 1
                Where bit 1 of x enables effect on playfield 2
                Where bit 2 of x enables effect on playfield 3
                Where bit 3 of x enables effect on playfield 4
    0x0200: Line control ram for 0x5000 section.
    0x0400: Line control ram for 0x6000 section.
        180x:   Where bit 0 of x latches sprite alpha value
                Where bit 1 of x latches tilemap alpha value
    0x0600: Sprite control ram
        1c0x:   Where x enables sprite control word for that line
    0x0800: Zoom line control ram (256 lines)
        200x:   Where bit 0 of x enables effect on playfield 1
                Where bit 1 of x enables effect on playfield 2
                Where bit 2 of x enables effect on playfield 3
                Where bit 3 of x enables effect on playfield 4
    0x0a00: Assumed unused.
    0x0c00: Rowscroll line control ram (256 lines)
        280x:   Where bit 0 of x enables effect on playfield 1
                Where bit 1 of x enables effect on playfield 2
                Where bit 2 of x enables effect on playfield 3
                Where bit 3 of x enables effect on playfield 4
    0x0e00: Priority line control ram (256 lines)
        2c0x:   Where bit 0 of x enables effect on playfield 1
                Where bit 1 of x enables effect on playfield 2
                Where bit 2 of x enables effect on playfield 3
                Where bit 3 of x enables effect on playfield 4

    0x4000: Column scroll & clipping info

    0x5000: Clip plane 0 (low bits)
    0x5200: Clip plane 1 (low bits)
    0x5400: Unused?
    0x5600: Unused?

    0x6000: Sync register
    0x6004: Sprite alpha control
        0xff00: VRAM/Pixel layer control
            0xa000 enables pixel layer for this line (Disables VRAM layer)
            0x2000 enables garbage pixels for this line (maybe another pixel bank?) [unemulated]
            0x0800 seems to set the vram layer to be opaque [unemulated]
            Effect of other bits is unknown.
        0x00c0: Alpha mode for sprites with pri value 0x00
        0x0030: Alpha mode for sprites with pri value 0x00
        0x000c: Alpha mode for sprites with pri value 0x00
        0x0003: Alpha mode for sprites with pri value 0x00
    0x6200: Alpha blending control

    0x6400 - controls X zoom of tile - each tile collapses to 16 single colour lines
        xxx1 - affects bottom playfield
        xxx2 -
        xxx4 -
        xxx8 - affects top playfield
        xxfx - x zoom - 0 = 1st pixel 16 times
                        1 = 1st pixel 8, then 2nd 8
                        8 = 2 per pixel

        (these effects only known to be used on spcinvdj title screen and riding fight - not emulated)

        1xxx = interpret palette ram for this playfield line as 15 bit and bilinear filter framebuffer(??)
        3xxx = interpret palette ram for this playfield line as 15 bit
        7xxx = interpret palette ram for this playfield line as 24 bit palette
        8xxx = interpret palette ram for this playfield line as 21 bit palette

        (effect not emulated)

    0x6600: Background - background palette entry (under all tilemaps) (takes part in alpha blending)

        (effect not emulated)

    0x7000: Pivot/vram layer enable
    0x7200: Cram layer priority
    0x7400: Sprite clipping
    0x7600: Sprite priority values
        0xf000: Relative priority for sprites with pri value 0xc0
        0x0f00: Relative priority for sprites with pri value 0x80
        0x00f0: Relative priority for sprites with pri value 0x40
        0x000f: Relative priority for sprites with pri value 0x00

    0x8000: Playfield 1 scale (1 word per line, 256 lines, 0x80 = default no scale)
    0x8200: Playfield 2/4 scale
    0x8400: Playfield 3 scale
    0x8600: Playfield 2/4 scale
        0x00ff = Y scale (0x80 = no scale, > 0x80 is zoom in, < 0x80 is zoom out [0xc0 is half height of 0x80])
        0xff00 = X scale (0 = no scale, > 0 = zoom in, [0x8000 would be double length])

        Playfield 2 & 4 registers seem to be interleaved, playfield 2 Y zoom is stored where you would
        expect playfield 4 y zoom to be and vice versa.

    0x9000: Palette add (can affect opacity) [ not emulated ]

    0xa000: Playfield 1 rowscroll (1 word per line, 256 lines)
    0xa200: Playfield 2 rowscroll
    0xa400: Playfield 3 rowscroll
    0xa600: Playfield 4 rowscroll

    0xb000: Playfield 1 priority (1 word per line, 256 lines)
    0xb200: Playfield 2 priority
    0xb400: Playfield 3 priority
    0xb600: Playfield 4 priority
        0xf000 = Blend mode, 0x3000 = Normal, 0x7000 = Alpha A, 0xb000 = Alpha B, others disable line
        0x0800 = Clip line (totally disable line)
        0x0400 = Assumed unused
        0x0200 = If set enable clip plane 1
        0x0100 = If set enable clip plane 0
        0x00c0 = Assumed unused
        0x0020 = Enable clip inverse mode for plane 1
        0x0010 = Enable clip inverse mode for plane 0
        0x000f = Layer priority

    0xc000 - 0xffff: Unused.

    When sprite priority==playfield priority sprite takes precedence (Bubble Symph title)

****************************************************************************

    F3 sprite format:

    Word 0: 0xffff      Tile number (LSB)
    Word 1: 0xff00      X zoom
            0x00ff      Y zoom
    Word 2: 0x03ff      X position
    Word 3: 0x03ff      Y position
    Word 4: 0xf000      Sprite block controls
            0x0800      Sprite block start
            0x0400      Use same colour on this sprite as block start
            0x0200      Y flip
            0x0100      X flip
            0x00ff      Colour
    Word 5: 0xffff      Tile number (MSB), probably only low bits used
    Word 6: 0x8000      If set, jump to sprite location in low bits
            0x03ff      Location to jump to.
    Word 7: 0xffff      Unused?  Always zero?

****************************************************************************

    Playfield control information (0x660000-1f):

    Word 0- 3: X scroll values for the 4 playfields.
    Word 4- 7: Y scroll values for the 4 playfields.
    Word 8-11: Unused.  Always zero.
    Word   12: Pixel + VRAM playfields X scroll
    Word   13: Pixel + VRAM playfields Y scroll
    Word   14: Unused. Always zero.
    Word   15: If set to 0x80, then 1024x512 playfields are used, else 512x512

    top bits unused - writing to low 4 bits is desync or blank tilemap

Playfield tile info:
    0xc000 0000 - X/Y Flip
    0x3000 ffff - Tile index
    0x0c00 0000 - Extra planes enable (00 = 4bpp, 01 = 5bpp, 10 = unused?, 11 = 6bpp)
    0x0200 0000 - Alpha blend mode
    0x01ff 0000 - Colour

***************************************************************************/

#include "emu.h"
#include "includes/taito_f3.h"
#include "ui.h"

#define VERBOSE 0
#define DARIUSG_KLUDGE
//#define DEBUG_F3 1


/* Game specific data, some of this can be
removed when the software values are figured out */
struct F3config
{
	int name;
	int extend;
	int sprite_lag;
};

static const struct F3config f3_config_table[] =
{
	/* Name    Extend  Lag */
	{ RINGRAGE,  0,     2 },
	{ ARABIANM,  0,     2 },
	{ RIDINGF,   1,     1 },
	{ GSEEKER,   0,     1 },
	{ COMMANDW,  1,     1 },
	{ SCFINALS,  0,     1 },
	{ TRSTAR,    1,     0 },
	{ GUNLOCK,   1,     2 },
	{ LIGHTBR,   1,     2 },
	{ KAISERKN,  0,     2 },
	{ DARIUSG,   0,     2 },
	{ BUBSYMPH,  1,     1 },
	{ SPCINVDX,  1,     1 },
	{ HTHERO95,  0,     1 },
	{ QTHEATER,  1,     1 },
	{ EACTION2,  1,     2 },
	{ RECALH,    1,     1 },
	{ SPCINV95,  0,     1 },
	{ TWINQIX,   1,     1 },
	{ QUIZHUHU,  1,     1 },
	{ PBOBBLE2,  0,     1 },
	{ GEKIRIDO,  0,     1 },
	{ KTIGER2,   0,     0 },
	{ BUBBLEM,   1,     1 },
	{ CLEOPATR,  0,     1 },
	{ PBOBBLE3,  0,     1 },
	{ ARKRETRN,  1,     1 },
	{ KIRAMEKI,  0,     1 },
	{ PUCHICAR,  1,     1 },
	{ PBOBBLE4,  0,     1 },
	{ POPNPOP,   1,     1 },
	{ LANDMAKR,  1,     1 },
	{ TMDRILL,   1,     0 },
	{0}
};


struct tempsprite
{
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int pri;
};

static void get_sprite_info(running_machine &machine, const UINT16 *spriteram16_ptr);

struct f3_playfield_line_inf
{
	int alpha_mode[256];
	int pri[256];

	/* use for draw_scanlines */
	UINT16 *src[256],*src_s[256],*src_e[256];
	UINT8 *tsrc[256],*tsrc_s[256];
	int x_count[256];
	UINT32 x_zoom[256];
	UINT32 clip0[256];
	UINT32 clip1[256];
};

struct f3_spritealpha_line_inf
{
	UINT16 alpha_level[256];
	UINT16 spri[256];
	UINT16 sprite_alpha[256];
	UINT32 sprite_clip0[256];
	UINT32 sprite_clip1[256];
	INT16 clip0_l[256];
	INT16 clip0_r[256];
	INT16 clip1_l[256];
	INT16 clip1_r[256];
};

/*
alpha_mode
---- --xx    0:disable 1:nomal 2:alpha 7000 3:alpha b000
---1 ----    alpha level a
--1- ----    alpha level b
1--------    opaque line
*/


/*
pri_alp_bitmap
---- ---1    sprite priority 0
---- --1-    sprite priority 1
---- -1--    sprite priority 2
---- 1---    sprite priority 3
---1 ----    alpha level a 7000
--1- ----    alpha level b 7000
-1-- ----    alpha level a b000
1--- ----    alpha level b b000
1111 1111    opaque pixel
*/

static void init_alpha_blend_func(running_machine &machine);

/******************************************************************************/

static void print_debug_info(running_machine &machine, bitmap_rgb32 &bitmap)
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	UINT16 *f3_line_ram = state->m_f3_line_ram;
	int l[16];
	char buf[64*16];
	char *bufptr = buf;

	bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n",state->m_f3_control_0[0]>>6,state->m_f3_control_0[1]>>6,state->m_f3_control_0[2]>>6,state->m_f3_control_0[3]>>6);
	bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n",state->m_f3_control_0[4]>>7,state->m_f3_control_0[5]>>7,state->m_f3_control_0[6]>>7,state->m_f3_control_0[7]>>7);
	bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n",state->m_f3_control_1[0],state->m_f3_control_1[1],state->m_f3_control_1[2],state->m_f3_control_1[3]);
	bufptr += sprintf(bufptr,"%04X %04X %04X %04X\n",state->m_f3_control_1[4],state->m_f3_control_1[5],state->m_f3_control_1[6],state->m_f3_control_1[7]);

	bufptr += sprintf(bufptr,"%04X %04X %04X %04X %04X %04X %04X %04X\n",state->m_spriteram16_buffered[0],state->m_spriteram16_buffered[1],state->m_spriteram16_buffered[2],state->m_spriteram16_buffered[3],state->m_spriteram16_buffered[4],state->m_spriteram16_buffered[5],state->m_spriteram16_buffered[6],state->m_spriteram16_buffered[7]);
	bufptr += sprintf(bufptr,"%04X %04X %04X %04X %04X %04X %04X %04X\n",state->m_spriteram16_buffered[8],state->m_spriteram16_buffered[9],state->m_spriteram16_buffered[10],state->m_spriteram16_buffered[11],state->m_spriteram16_buffered[12],state->m_spriteram16_buffered[13],state->m_spriteram16_buffered[14],state->m_spriteram16_buffered[15]);
	bufptr += sprintf(bufptr,"%04X %04X %04X %04X %04X %04X %04X %04X\n",state->m_spriteram16_buffered[16],state->m_spriteram16_buffered[17],state->m_spriteram16_buffered[18],state->m_spriteram16_buffered[19],state->m_spriteram16_buffered[20],state->m_spriteram16_buffered[21],state->m_spriteram16_buffered[22],state->m_spriteram16_buffered[23]);

	l[0]=f3_line_ram[0x0040*2]&0xffff;
	l[1]=f3_line_ram[0x00c0*2]&0xffff;
	l[2]=f3_line_ram[0x0140*2]&0xffff;
	l[3]=f3_line_ram[0x01c0*2]&0xffff;
	bufptr += sprintf(bufptr,"Ctr1: %04x %04x %04x %04x\n",l[0],l[1],l[2],l[3]);

	l[0]=f3_line_ram[0x0240*2]&0xffff;
	l[1]=f3_line_ram[0x02c0*2]&0xffff;
	l[2]=f3_line_ram[0x0340*2]&0xffff;
	l[3]=f3_line_ram[0x03c0*2]&0xffff;
	bufptr += sprintf(bufptr,"Ctr2: %04x %04x %04x %04x\n",l[0],l[1],l[2],l[3]);

	l[0]=f3_line_ram[0x2c60*2]&0xffff;
	l[1]=f3_line_ram[0x2ce0*2]&0xffff;
	l[2]=f3_line_ram[0x2d60*2]&0xffff;
	l[3]=f3_line_ram[0x2de0*2]&0xffff;
	bufptr += sprintf(bufptr,"Pri : %04x %04x %04x %04x\n",l[0],l[1],l[2],l[3]);

	l[0]=f3_line_ram[0x2060*2]&0xffff;
	l[1]=f3_line_ram[0x20e0*2]&0xffff;
	l[2]=f3_line_ram[0x2160*2]&0xffff;
	l[3]=f3_line_ram[0x21e0*2]&0xffff;
	bufptr += sprintf(bufptr,"Zoom: %04x %04x %04x %04x\n",l[0],l[1],l[2],l[3]);

	l[0]=f3_line_ram[0x2860*2]&0xffff;
	l[1]=f3_line_ram[0x28e0*2]&0xffff;
	l[2]=f3_line_ram[0x2960*2]&0xffff;
	l[3]=f3_line_ram[0x29e0*2]&0xffff;
	bufptr += sprintf(bufptr,"Line: %04x %04x %04x %04x\n",l[0],l[1],l[2],l[3]);

	l[0]=f3_line_ram[0x1c60*2]&0xffff;
	l[1]=f3_line_ram[0x1ce0*2]&0xffff;
	l[2]=f3_line_ram[0x1d60*2]&0xffff;
	l[3]=f3_line_ram[0x1de0*2]&0xffff;
	bufptr += sprintf(bufptr,"Sprt: %04x %04x %04x %04x\n",l[0],l[1],l[2],l[3]);

	l[0]=f3_line_ram[0x1860*2]&0xffff;
	l[1]=f3_line_ram[0x18e0*2]&0xffff;
	l[2]=f3_line_ram[0x1960*2]&0xffff;
	l[3]=f3_line_ram[0x19e0*2]&0xffff;
	bufptr += sprintf(bufptr,"Pivt: %04x %04x %04x %04x\n",l[0],l[1],l[2],l[3]);

	l[0]=f3_line_ram[0x1060*2]&0xffff;
	l[1]=f3_line_ram[0x10e0*2]&0xffff;
	l[2]=f3_line_ram[0x1160*2]&0xffff;
	l[3]=f3_line_ram[0x11e0*2]&0xffff;
	bufptr += sprintf(bufptr,"Colm: %04x %04x %04x %04x\n",l[0],l[1],l[2],l[3]);

	l[0]=f3_line_ram[0x1460*2]&0xffff;
	l[1]=f3_line_ram[0x14e0*2]&0xffff;
	l[2]=f3_line_ram[0x1560*2]&0xffff;
	l[3]=f3_line_ram[0x15e0*2]&0xffff;
	bufptr += sprintf(bufptr,"5000: %04x %04x %04x %04x\n",l[0],l[1],l[2],l[3]);

	ui_draw_text(&machine.render().ui_container(), buf, 60, 40);
}

/******************************************************************************/

INLINE void get_tile_info(running_machine &machine, tile_data &tileinfo, int tile_index, UINT16 *gfx_base)
{
	UINT32 tile=(gfx_base[tile_index*2+0]<<16)|(gfx_base[tile_index*2+1]&0xffff);
	UINT8 abtype=(tile>>(16+9)) & 1;
	// tiles can be configured to use 4, 5, or 6 bpp data.
	// if tiles use more than 4bpp, the bottom bits of the color code must be masked out.
	// This fixes (at least) the rain in round 6 of Arabian Magic.
	UINT8 extra_planes = ((tile>>(16+10)) & 3);	// 0 = 4bpp, 1 = 5bpp, 2 = unused?, 3 = 6bpp

	SET_TILE_INFO(
			1,
			tile&0xffff,
			(tile>>16) & 0x1ff & (~extra_planes),
			TILE_FLIPYX( tile >> 30 ));
	tileinfo.category =  abtype&1;		/* alpha blending type */
	tileinfo.pen_mask = (extra_planes << 4) | 0x0f;
}

static TILE_GET_INFO( get_tile_info1 )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	get_tile_info(machine,tileinfo,tile_index,state->m_f3_pf_data_1);
}

static TILE_GET_INFO( get_tile_info2 )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	get_tile_info(machine,tileinfo,tile_index,state->m_f3_pf_data_2);
}

static TILE_GET_INFO( get_tile_info3 )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	get_tile_info(machine,tileinfo,tile_index,state->m_f3_pf_data_3);
}

static TILE_GET_INFO( get_tile_info4 )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	get_tile_info(machine,tileinfo,tile_index,state->m_f3_pf_data_4);
}

static TILE_GET_INFO( get_tile_info5 )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	get_tile_info(machine,tileinfo,tile_index,state->m_f3_pf_data_5);
}

static TILE_GET_INFO( get_tile_info6 )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	get_tile_info(machine,tileinfo,tile_index,state->m_f3_pf_data_6);
}

static TILE_GET_INFO( get_tile_info7 )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	get_tile_info(machine,tileinfo,tile_index,state->m_f3_pf_data_7);
}

static TILE_GET_INFO( get_tile_info8 )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	get_tile_info(machine,tileinfo,tile_index,state->m_f3_pf_data_8);
}


static TILE_GET_INFO( get_tile_info_vram )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	int vram_tile;
	int flags=0;

	vram_tile = (state->m_videoram[tile_index]&0xffff);

	if (vram_tile&0x0100) flags|=TILE_FLIPX;
	if (vram_tile&0x8000) flags|=TILE_FLIPY;

	SET_TILE_INFO(
			0,
			vram_tile&0xff,
			(vram_tile>>9)&0x3f,
			flags);
}

static TILE_GET_INFO( get_tile_info_pixel )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	int vram_tile,col_off;
	int flags=0;
	int y_offs=(state->m_f3_control_1[5]&0x1ff);
	if (state->m_flipscreen) y_offs+=0x100;

	/* Colour is shared with VRAM layer */
	if ((((tile_index%32)*8 + y_offs)&0x1ff)>0xff)
		col_off=0x800+((tile_index%32)*0x40)+((tile_index&0xfe0)>>5);
	else
		col_off=((tile_index%32)*0x40)+((tile_index&0xfe0)>>5);

	vram_tile = (state->m_videoram[col_off]&0xffff);

	if (vram_tile&0x0100) flags|=TILE_FLIPX;
	if (vram_tile&0x8000) flags|=TILE_FLIPY;

	SET_TILE_INFO(
			3,
			tile_index,
			(vram_tile>>9)&0x3f,
			flags);
}

/******************************************************************************/

SCREEN_VBLANK( f3 )
{
	// rising edge
	if (vblank_on)
	{
		taito_f3_state *state = screen.machine().driver_data<taito_f3_state>();
		if (state->m_sprite_lag==2)
		{
			if (screen.machine().video().skip_this_frame() == 0)
			{
				get_sprite_info(screen.machine(), state->m_spriteram16_buffered);
			}
			memcpy(state->m_spriteram16_buffered,state->m_spriteram,0x10000);
		}
		else if (state->m_sprite_lag==1)
		{
			if (screen.machine().video().skip_this_frame() == 0)
			{
				get_sprite_info(screen.machine(), state->m_spriteram);
			}
		}
	}
}

VIDEO_START( f3 )
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	const struct F3config *pCFG=&f3_config_table[0];
	int i;

	state->m_f3_alpha_level_2as=127;
	state->m_f3_alpha_level_2ad=127;
	state->m_f3_alpha_level_3as=127;
	state->m_f3_alpha_level_3ad=127;
	state->m_f3_alpha_level_2bs=127;
	state->m_f3_alpha_level_2bd=127;
	state->m_f3_alpha_level_3bs=127;
	state->m_f3_alpha_level_3bd=127;
	state->m_alpha_level_last = -1;

	state->m_pdest_2a = 0x10;
	state->m_pdest_2b = 0x20;
	state->m_tr_2a = 0;
	state->m_tr_2b = 1;
	state->m_pdest_3a = 0x40;
	state->m_pdest_3b = 0x80;
	state->m_tr_3a = 0;
	state->m_tr_3b = 1;

	state->m_spritelist=0;
	state->m_spriteram16_buffered=0;
	state->m_pf_line_inf=0;
	state->m_tile_opaque_sp=0;

	/* Setup individual game */
	do {
		if (pCFG->name==state->m_f3_game)
		{
			break;
		}
		pCFG++;
	} while(pCFG->name);

	state->m_f3_game_config=pCFG;

	state->m_f3_vram =      auto_alloc_array_clear(machine, UINT16, 0x2000/2);
	state->m_f3_pf_data =   auto_alloc_array_clear(machine, UINT16, 0xc000/2);
	state->m_videoram =     auto_alloc_array_clear(machine, UINT16, 0x2000/2);
	state->m_f3_line_ram =  auto_alloc_array_clear(machine, UINT16, 0x10000/2);
	state->m_f3_pivot_ram = auto_alloc_array_clear(machine, UINT16, 0x10000/2);
	state->m_spriteram =    auto_alloc_array_clear(machine, UINT16, 0x10000/2);

	if (state->m_f3_game_config->extend) {
		state->m_pf1_tilemap = tilemap_create(machine, get_tile_info1,tilemap_scan_rows,16,16,64,32);
		state->m_pf2_tilemap = tilemap_create(machine, get_tile_info2,tilemap_scan_rows,16,16,64,32);
		state->m_pf3_tilemap = tilemap_create(machine, get_tile_info3,tilemap_scan_rows,16,16,64,32);
		state->m_pf4_tilemap = tilemap_create(machine, get_tile_info4,tilemap_scan_rows,16,16,64,32);

		state->m_f3_pf_data_1=state->m_f3_pf_data+(0x0000/2);
		state->m_f3_pf_data_2=state->m_f3_pf_data+(0x2000/2);
		state->m_f3_pf_data_3=state->m_f3_pf_data+(0x4000/2);
		state->m_f3_pf_data_4=state->m_f3_pf_data+(0x6000/2);

		state->m_width_mask=0x3ff;
		state->m_twidth_mask=0x7f;
		state->m_twidth_mask_bit=7;

		state->m_pf1_tilemap->set_transparent_pen(0);
		state->m_pf2_tilemap->set_transparent_pen(0);
		state->m_pf3_tilemap->set_transparent_pen(0);
		state->m_pf4_tilemap->set_transparent_pen(0);


	} else {
		state->m_pf1_tilemap = tilemap_create(machine, get_tile_info1,tilemap_scan_rows,16,16,32,32);
		state->m_pf2_tilemap = tilemap_create(machine, get_tile_info2,tilemap_scan_rows,16,16,32,32);
		state->m_pf3_tilemap = tilemap_create(machine, get_tile_info3,tilemap_scan_rows,16,16,32,32);
		state->m_pf4_tilemap = tilemap_create(machine, get_tile_info4,tilemap_scan_rows,16,16,32,32);
		state->m_pf5_tilemap = tilemap_create(machine, get_tile_info5,tilemap_scan_rows,16,16,32,32);
		state->m_pf6_tilemap = tilemap_create(machine, get_tile_info6,tilemap_scan_rows,16,16,32,32);
		state->m_pf7_tilemap = tilemap_create(machine, get_tile_info7,tilemap_scan_rows,16,16,32,32);
		state->m_pf8_tilemap = tilemap_create(machine, get_tile_info8,tilemap_scan_rows,16,16,32,32);

		state->m_f3_pf_data_1=state->m_f3_pf_data+(0x0000/2);
		state->m_f3_pf_data_2=state->m_f3_pf_data+(0x1000/2);
		state->m_f3_pf_data_3=state->m_f3_pf_data+(0x2000/2);
		state->m_f3_pf_data_4=state->m_f3_pf_data+(0x3000/2);
		state->m_f3_pf_data_5=state->m_f3_pf_data+(0x4000/2);
		state->m_f3_pf_data_6=state->m_f3_pf_data+(0x5000/2);
		state->m_f3_pf_data_7=state->m_f3_pf_data+(0x6000/2);
		state->m_f3_pf_data_8=state->m_f3_pf_data+(0x7000/2);

		state->m_width_mask=0x1ff;
		state->m_twidth_mask=0x3f;
		state->m_twidth_mask_bit=6;

		state->m_pf1_tilemap->set_transparent_pen(0);
		state->m_pf2_tilemap->set_transparent_pen(0);
		state->m_pf3_tilemap->set_transparent_pen(0);
		state->m_pf4_tilemap->set_transparent_pen(0);
		state->m_pf5_tilemap->set_transparent_pen(0);
		state->m_pf6_tilemap->set_transparent_pen(0);
		state->m_pf7_tilemap->set_transparent_pen(0);
		state->m_pf8_tilemap->set_transparent_pen(0);
	}

	state->m_spriteram16_buffered = auto_alloc_array(machine, UINT16, 0x10000/2);
	state->m_spritelist = auto_alloc_array(machine, struct tempsprite, 0x400);
	state->m_sprite_end = state->m_spritelist;
	state->m_vram_layer = tilemap_create(machine, get_tile_info_vram,tilemap_scan_rows,8,8,64,64);
	state->m_pixel_layer = tilemap_create(machine, get_tile_info_pixel,tilemap_scan_cols,8,8,64,32);
	state->m_pf_line_inf = auto_alloc_array(machine, struct f3_playfield_line_inf, 5);
	state->m_sa_line_inf = auto_alloc_array(machine, struct f3_spritealpha_line_inf, 1);
	machine.primary_screen->register_screen_bitmap(state->m_pri_alp_bitmap);
	state->m_tile_opaque_sp = auto_alloc_array(machine, UINT8, machine.gfx[2]->total_elements);
	for (i=0; i<8; i++)
		state->m_tile_opaque_pf[i] = auto_alloc_array(machine, UINT8, machine.gfx[1]->total_elements);


	state->m_vram_layer->set_transparent_pen(0);
	state->m_pixel_layer->set_transparent_pen(0);

	/* Palettes have 4 bpp indexes despite up to 6 bpp data. The unused */
	/* top bits in the gfx data are cleared later.                      */
	machine.gfx[1]->color_granularity=16;
	machine.gfx[2]->color_granularity=16;

	state->m_flipscreen = 0;
	memset(state->m_spriteram16_buffered,0,0x10000);
	memset(state->m_spriteram,0,0x10000);

	state_save_register_global_array(machine, state->m_f3_control_0);
	state_save_register_global_array(machine, state->m_f3_control_1);

	gfx_element_set_source(machine.gfx[0], (UINT8 *)state->m_f3_vram);
	gfx_element_set_source(machine.gfx[3], (UINT8 *)state->m_f3_pivot_ram);

	state->m_f3_skip_this_frame=0;

	state->m_sprite_lag=state->m_f3_game_config->sprite_lag;

	init_alpha_blend_func(machine);

	{
		const gfx_element *sprite_gfx = machine.gfx[2];
		int c;

		for (c = 0;c < sprite_gfx->total_elements;c++)
		{
			int x,y;
			int chk_trans_or_opa=0;
			const UINT8 *dp = gfx_element_get_data(sprite_gfx, c);
			for (y = 0;y < sprite_gfx->height;y++)
			{
				for (x = 0;x < sprite_gfx->width;x++)
				{
					if(!dp[x]) chk_trans_or_opa|=2;
					else	   chk_trans_or_opa|=1;
				}
				dp += sprite_gfx->line_modulo;
			}
			if(chk_trans_or_opa==1) state->m_tile_opaque_sp[c]=1;
			else					state->m_tile_opaque_sp[c]=0;
		}
	}


	{
		const gfx_element *pf_gfx = machine.gfx[1];
		int c;

		for (c = 0;c < pf_gfx->total_elements;c++)
		{
			int x,y;
			int extra_planes; /* 0 = 4bpp, 1=5bpp, 2=?, 3=6bpp */

			for (extra_planes=0; extra_planes<4; extra_planes++)
			{
				int chk_trans_or_opa=0;
				UINT8 extra_mask = ((extra_planes << 4) | 0x0f);
				const UINT8 *dp = gfx_element_get_data(pf_gfx, c);

				for (y = 0;y < pf_gfx->height;y++)
				{
					for (x = 0;x < pf_gfx->width;x++)
					{
						if(!(dp[x] & extra_mask))
							chk_trans_or_opa|=2;
						else
							chk_trans_or_opa|=1;
					}
					dp += pf_gfx->line_modulo;
				}
				state->m_tile_opaque_pf[extra_planes][c]=chk_trans_or_opa;
			}
		}
	}
}

/******************************************************************************/

READ16_MEMBER(taito_f3_state::f3_pf_data_r)
{
	return m_f3_pf_data[offset];
}

WRITE16_MEMBER(taito_f3_state::f3_pf_data_w)
{
	COMBINE_DATA(&m_f3_pf_data[offset]);

	if (m_f3_game_config->extend) {
		if		(offset<0x1000) m_pf1_tilemap->mark_tile_dirty((offset & 0xfff) >> 1);
		else if (offset<0x2000) m_pf2_tilemap->mark_tile_dirty((offset & 0xfff) >> 1);
		else if (offset<0x3000) m_pf3_tilemap->mark_tile_dirty((offset & 0xfff) >> 1);
		else if (offset<0x4000) m_pf4_tilemap->mark_tile_dirty((offset & 0xfff) >> 1);
	} else {
		if		(offset<0x0800) m_pf1_tilemap->mark_tile_dirty((offset & 0x7ff) >> 1);
		else if (offset<0x1000) m_pf2_tilemap->mark_tile_dirty((offset & 0x7ff) >> 1);
		else if (offset<0x1800) m_pf3_tilemap->mark_tile_dirty((offset & 0x7ff) >> 1);
		else if (offset<0x2000) m_pf4_tilemap->mark_tile_dirty((offset & 0x7ff) >> 1);
		else if (offset<0x2800) m_pf5_tilemap->mark_tile_dirty((offset & 0x7ff) >> 1);
		else if (offset<0x3000) m_pf6_tilemap->mark_tile_dirty((offset & 0x7ff) >> 1);
		else if (offset<0x3800) m_pf7_tilemap->mark_tile_dirty((offset & 0x7ff) >> 1);
		else if (offset<0x4000) m_pf8_tilemap->mark_tile_dirty((offset & 0x7ff) >> 1);
	}
}

WRITE16_MEMBER(taito_f3_state::f3_control_0_w)
{
	COMBINE_DATA(&m_f3_control_0[offset]);
}

WRITE16_MEMBER(taito_f3_state::f3_control_1_w)
{
	COMBINE_DATA(&m_f3_control_1[offset]);
}

READ16_MEMBER(taito_f3_state::f3_spriteram_r)
{
	return m_spriteram[offset];
}

WRITE16_MEMBER(taito_f3_state::f3_spriteram_w)
{
	COMBINE_DATA(&m_spriteram[offset]);
}

READ16_MEMBER(taito_f3_state::f3_videoram_r)
{
	return m_videoram[offset];
}

WRITE16_MEMBER(taito_f3_state::f3_videoram_w)
{
	int tile,col_off;
	COMBINE_DATA(&m_videoram[offset]);

	m_vram_layer->mark_tile_dirty(offset);
	//m_vram_layer->mark_tile_dirty(offset+1);

	if (offset>0x7ff) offset-=0x800;

	tile=offset;
	col_off=((tile&0x3f)*32)+((tile&0xfc0)>>6);

	m_pixel_layer->mark_tile_dirty(col_off);
	//m_pixel_layer->mark_tile_dirty(col_off+32);
}


READ16_MEMBER(taito_f3_state::f3_vram_r)
{

	return m_f3_vram[offset];
}

WRITE16_MEMBER(taito_f3_state::f3_vram_w)
{
	COMBINE_DATA(&m_f3_vram[offset]);
	gfx_element_mark_dirty(machine().gfx[0], offset/16);
}

READ16_MEMBER(taito_f3_state::f3_pivot_r)
{
	return m_f3_pivot_ram[offset];
}

WRITE16_MEMBER(taito_f3_state::f3_pivot_w)
{
	COMBINE_DATA(&m_f3_pivot_ram[offset]);
	gfx_element_mark_dirty(machine().gfx[3], offset/16);
}

READ16_MEMBER(taito_f3_state::f3_lineram_r)
{
	return m_f3_line_ram[offset];
}

WRITE16_MEMBER(taito_f3_state::f3_lineram_w)
{
	/* DariusGX has an interesting bug at the start of Round D - the clearing of lineram
    (0xa000->0x0xa7ff) overflows into priority RAM (0xb000) and creates garbage priority
    values.  I'm not sure what the real machine would do with these values, and this
    emulation certainly doesn't like it, so I've chosen to catch the bug here, and prevent
    the trashing of priority ram.  If anyone has information on what the real machine does,
    please let me know! */
	if (m_f3_game==DARIUSG) {
		if (m_f3_skip_this_frame)
			return;
		if (offset==0xb000/2 && data==0x003f) {
			m_f3_skip_this_frame=1;
			return;
		}
	}

	COMBINE_DATA(&m_f3_line_ram[offset]);
}

WRITE32_MEMBER(taito_f3_state::f3_palette_24bit_w)
{
	int r,g,b;

	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	/* 12 bit palette games - there has to be a palette select bit somewhere */
	if (m_f3_game==SPCINVDX || m_f3_game==RIDINGF || m_f3_game==ARABIANM || m_f3_game==RINGRAGE) {
		b = 15 * ((m_generic_paletteram_32[offset] >> 4) & 0xf);
		g = 15 * ((m_generic_paletteram_32[offset] >> 8) & 0xf);
		r = 15 * ((m_generic_paletteram_32[offset] >> 12) & 0xf);
	}

	/* This is weird - why are only the sprites and VRAM palettes 21 bit? */
	else if (m_f3_game==CLEOPATR) {
		if (offset<0x100 || offset>0x1000) {
			r = ((m_generic_paletteram_32[offset] >>16) & 0x7f)<<1;
			g = ((m_generic_paletteram_32[offset] >> 8) & 0x7f)<<1;
			b = ((m_generic_paletteram_32[offset] >> 0) & 0x7f)<<1;
		} else {
			r = (m_generic_paletteram_32[offset] >>16) & 0xff;
			g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
			b = (m_generic_paletteram_32[offset] >> 0) & 0xff;
		}
	}

	/* Another weird couple - perhaps this is alpha blending related? */
	else if (m_f3_game==TWINQIX || m_f3_game==RECALH) {
		if (offset>0x1c00) {
			r = ((m_generic_paletteram_32[offset] >>16) & 0x7f)<<1;
			g = ((m_generic_paletteram_32[offset] >> 8) & 0x7f)<<1;
			b = ((m_generic_paletteram_32[offset] >> 0) & 0x7f)<<1;
		} else {
			r = (m_generic_paletteram_32[offset] >>16) & 0xff;
			g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
			b = (m_generic_paletteram_32[offset] >> 0) & 0xff;
		}
	}

	/* All other games - standard 24 bit palette */
	else {
		r = (m_generic_paletteram_32[offset] >>16) & 0xff;
		g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
		b = (m_generic_paletteram_32[offset] >> 0) & 0xff;
	}

	palette_set_color(machine(),offset,MAKE_RGB(r,g,b));
}

/******************************************************************************/

/*============================================================================*/

#define SET_ALPHA_LEVEL(d,s)			\
{										\
	int level = s;						\
	if(level == 0) level = -1;			\
	d = level+1;						\
}

INLINE void f3_alpha_set_level(taito_f3_state *state)
{
//  SET_ALPHA_LEVEL(state->m_alpha_s_1_1, state->m_f3_alpha_level_2ad)
	SET_ALPHA_LEVEL(state->m_alpha_s_1_1, 255-state->m_f3_alpha_level_2as)
//  SET_ALPHA_LEVEL(state->m_alpha_s_1_2, state->m_f3_alpha_level_2bd)
	SET_ALPHA_LEVEL(state->m_alpha_s_1_2, 255-state->m_f3_alpha_level_2bs)
	SET_ALPHA_LEVEL(state->m_alpha_s_1_4, state->m_f3_alpha_level_3ad)
//  SET_ALPHA_LEVEL(state->m_alpha_s_1_5, state->m_f3_alpha_level_3ad*state->m_f3_alpha_level_2ad/255)
	SET_ALPHA_LEVEL(state->m_alpha_s_1_5, state->m_f3_alpha_level_3ad*(255-state->m_f3_alpha_level_2as)/255)
//  SET_ALPHA_LEVEL(state->m_alpha_s_1_6, state->m_f3_alpha_level_3ad*state->m_f3_alpha_level_2bd/255)
	SET_ALPHA_LEVEL(state->m_alpha_s_1_6, state->m_f3_alpha_level_3ad*(255-state->m_f3_alpha_level_2bs)/255)
	SET_ALPHA_LEVEL(state->m_alpha_s_1_8, state->m_f3_alpha_level_3bd)
//  SET_ALPHA_LEVEL(state->m_alpha_s_1_9, state->m_f3_alpha_level_3bd*state->m_f3_alpha_level_2ad/255)
	SET_ALPHA_LEVEL(state->m_alpha_s_1_9, state->m_f3_alpha_level_3bd*(255-state->m_f3_alpha_level_2as)/255)
//  SET_ALPHA_LEVEL(state->m_alpha_s_1_a, state->m_f3_alpha_level_3bd*state->m_f3_alpha_level_2bd/255)
	SET_ALPHA_LEVEL(state->m_alpha_s_1_a, state->m_f3_alpha_level_3bd*(255-state->m_f3_alpha_level_2bs)/255)

	SET_ALPHA_LEVEL(state->m_alpha_s_2a_0, state->m_f3_alpha_level_2as)
	SET_ALPHA_LEVEL(state->m_alpha_s_2a_4, state->m_f3_alpha_level_2as*state->m_f3_alpha_level_3ad/255)
	SET_ALPHA_LEVEL(state->m_alpha_s_2a_8, state->m_f3_alpha_level_2as*state->m_f3_alpha_level_3bd/255)

	SET_ALPHA_LEVEL(state->m_alpha_s_2b_0, state->m_f3_alpha_level_2bs)
	SET_ALPHA_LEVEL(state->m_alpha_s_2b_4, state->m_f3_alpha_level_2bs*state->m_f3_alpha_level_3ad/255)
	SET_ALPHA_LEVEL(state->m_alpha_s_2b_8, state->m_f3_alpha_level_2bs*state->m_f3_alpha_level_3bd/255)

	SET_ALPHA_LEVEL(state->m_alpha_s_3a_0, state->m_f3_alpha_level_3as)
	SET_ALPHA_LEVEL(state->m_alpha_s_3a_1, state->m_f3_alpha_level_3as*state->m_f3_alpha_level_2ad/255)
	SET_ALPHA_LEVEL(state->m_alpha_s_3a_2, state->m_f3_alpha_level_3as*state->m_f3_alpha_level_2bd/255)

	SET_ALPHA_LEVEL(state->m_alpha_s_3b_0, state->m_f3_alpha_level_3bs)
	SET_ALPHA_LEVEL(state->m_alpha_s_3b_1, state->m_f3_alpha_level_3bs*state->m_f3_alpha_level_2ad/255)
	SET_ALPHA_LEVEL(state->m_alpha_s_3b_2, state->m_f3_alpha_level_3bs*state->m_f3_alpha_level_2bd/255)
}
#undef SET_ALPHA_LEVEL

/*============================================================================*/

#define COLOR1 BYTE4_XOR_LE(0)
#define COLOR2 BYTE4_XOR_LE(1)
#define COLOR3 BYTE4_XOR_LE(2)



INLINE void f3_alpha_blend32_s( taito_f3_state *state, int alphas, UINT32 s )
{
	UINT8 *sc = (UINT8 *)&s;
	UINT8 *dc = (UINT8 *)&state->m_dval;
	dc[COLOR1] = (alphas * sc[COLOR1]) >> 8;
	dc[COLOR2] = (alphas * sc[COLOR2]) >> 8;
	dc[COLOR3] = (alphas * sc[COLOR3]) >> 8;
}

INLINE void f3_alpha_blend32_d( taito_f3_state *state, int alphas, UINT32 s )
{
	UINT8 *sc = (UINT8 *)&s;
	UINT8 *dc = (UINT8 *)&state->m_dval;
	dc[COLOR1] = state->m_add_sat[dc[COLOR1]][(alphas * sc[COLOR1]) >> 8];
	dc[COLOR2] = state->m_add_sat[dc[COLOR2]][(alphas * sc[COLOR2]) >> 8];
	dc[COLOR3] = state->m_add_sat[dc[COLOR3]][(alphas * sc[COLOR3]) >> 8];
}

/*============================================================================*/

INLINE void f3_alpha_blend_1_1( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_1_1,s);}
INLINE void f3_alpha_blend_1_2( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_1_2,s);}
INLINE void f3_alpha_blend_1_4( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_1_4,s);}
INLINE void f3_alpha_blend_1_5( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_1_5,s);}
INLINE void f3_alpha_blend_1_6( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_1_6,s);}
INLINE void f3_alpha_blend_1_8( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_1_8,s);}
INLINE void f3_alpha_blend_1_9( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_1_9,s);}
INLINE void f3_alpha_blend_1_a( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_1_a,s);}

INLINE void f3_alpha_blend_2a_0( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_s(state, state->m_alpha_s_2a_0,s);}
INLINE void f3_alpha_blend_2a_4( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_2a_4,s);}
INLINE void f3_alpha_blend_2a_8( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_2a_8,s);}

INLINE void f3_alpha_blend_2b_0( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_s(state, state->m_alpha_s_2b_0,s);}
INLINE void f3_alpha_blend_2b_4( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_2b_4,s);}
INLINE void f3_alpha_blend_2b_8( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_2b_8,s);}

INLINE void f3_alpha_blend_3a_0( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_s(state, state->m_alpha_s_3a_0,s);}
INLINE void f3_alpha_blend_3a_1( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_3a_1,s);}
INLINE void f3_alpha_blend_3a_2( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_3a_2,s);}

INLINE void f3_alpha_blend_3b_0( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_s(state, state->m_alpha_s_3b_0,s);}
INLINE void f3_alpha_blend_3b_1( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_3b_1,s);}
INLINE void f3_alpha_blend_3b_2( taito_f3_state *state, UINT32 s ){f3_alpha_blend32_d(state, state->m_alpha_s_3b_2,s);}

/*============================================================================*/

static int dpix_1_noalpha(taito_f3_state *state, UINT32 s_pix) {state->m_dval = s_pix; return 1;}
static int dpix_ret1(taito_f3_state *state, UINT32 s_pix) {return 1;}
static int dpix_ret0(taito_f3_state *state, UINT32 s_pix) {return 0;}
static int dpix_1_1(taito_f3_state *state, UINT32 s_pix) {if(s_pix) f3_alpha_blend_1_1(state, s_pix); return 1;}
static int dpix_1_2(taito_f3_state *state, UINT32 s_pix) {if(s_pix) f3_alpha_blend_1_2(state, s_pix); return 1;}
static int dpix_1_4(taito_f3_state *state, UINT32 s_pix) {if(s_pix) f3_alpha_blend_1_4(state, s_pix); return 1;}
static int dpix_1_5(taito_f3_state *state, UINT32 s_pix) {if(s_pix) f3_alpha_blend_1_5(state, s_pix); return 1;}
static int dpix_1_6(taito_f3_state *state, UINT32 s_pix) {if(s_pix) f3_alpha_blend_1_6(state, s_pix); return 1;}
static int dpix_1_8(taito_f3_state *state, UINT32 s_pix) {if(s_pix) f3_alpha_blend_1_8(state, s_pix); return 1;}
static int dpix_1_9(taito_f3_state *state, UINT32 s_pix) {if(s_pix) f3_alpha_blend_1_9(state, s_pix); return 1;}
static int dpix_1_a(taito_f3_state *state, UINT32 s_pix) {if(s_pix) f3_alpha_blend_1_a(state, s_pix); return 1;}

static int dpix_2a_0(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_2a_0(state, s_pix);
	else	  state->m_dval = 0;
	if(state->m_pdest_2a) {state->m_pval |= state->m_pdest_2a;return 0;}
	return 1;
}
static int dpix_2a_4(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_2a_4(state, s_pix);
	if(state->m_pdest_2a) {state->m_pval |= state->m_pdest_2a;return 0;}
	return 1;
}
static int dpix_2a_8(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_2a_8(state, s_pix);
	if(state->m_pdest_2a) {state->m_pval |= state->m_pdest_2a;return 0;}
	return 1;
}

static int dpix_3a_0(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_3a_0(state, s_pix);
	else	  state->m_dval = 0;
	if(state->m_pdest_3a) {state->m_pval |= state->m_pdest_3a;return 0;}
	return 1;
}
static int dpix_3a_1(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_3a_1(state, s_pix);
	if(state->m_pdest_3a) {state->m_pval |= state->m_pdest_3a;return 0;}
	return 1;
}
static int dpix_3a_2(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_3a_2(state, s_pix);
	if(state->m_pdest_3a) {state->m_pval |= state->m_pdest_3a;return 0;}
	return 1;
}

static int dpix_2b_0(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_2b_0(state, s_pix);
	else	  state->m_dval = 0;
	if(state->m_pdest_2b) {state->m_pval |= state->m_pdest_2b;return 0;}
	return 1;
}
static int dpix_2b_4(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_2b_4(state, s_pix);
	if(state->m_pdest_2b) {state->m_pval |= state->m_pdest_2b;return 0;}
	return 1;
}
static int dpix_2b_8(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_2b_8(state, s_pix);
	if(state->m_pdest_2b) {state->m_pval |= state->m_pdest_2b;return 0;}
	return 1;
}

static int dpix_3b_0(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_3b_0(state, s_pix);
	else	  state->m_dval = 0;
	if(state->m_pdest_3b) {state->m_pval |= state->m_pdest_3b;return 0;}
	return 1;
}
static int dpix_3b_1(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_3b_1(state, s_pix);
	if(state->m_pdest_3b) {state->m_pval |= state->m_pdest_3b;return 0;}
	return 1;
}
static int dpix_3b_2(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix) f3_alpha_blend_3b_2(state, s_pix);
	if(state->m_pdest_3b) {state->m_pval |= state->m_pdest_3b;return 0;}
	return 1;
}

static int dpix_2_0(taito_f3_state *state, UINT32 s_pix)
{
	UINT8 tr2=state->m_tval&1;
	if(s_pix)
	{
		if(tr2==state->m_tr_2b)		{f3_alpha_blend_2b_0(state, s_pix);if(state->m_pdest_2b) state->m_pval |= state->m_pdest_2b;else return 1;}
		else if(tr2==state->m_tr_2a)	{f3_alpha_blend_2a_0(state, s_pix);if(state->m_pdest_2a) state->m_pval |= state->m_pdest_2a;else return 1;}
	}
	else
	{
		if(tr2==state->m_tr_2b)		{state->m_dval = 0;if(state->m_pdest_2b) state->m_pval |= state->m_pdest_2b;else return 1;}
		else if(tr2==state->m_tr_2a)	{state->m_dval = 0;if(state->m_pdest_2a) state->m_pval |= state->m_pdest_2a;else return 1;}
	}
	return 0;
}
static int dpix_2_4(taito_f3_state *state, UINT32 s_pix)
{
	UINT8 tr2=state->m_tval&1;
	if(s_pix)
	{
		if(tr2==state->m_tr_2b)		{f3_alpha_blend_2b_4(state, s_pix);if(state->m_pdest_2b) state->m_pval |= state->m_pdest_2b;else return 1;}
		else if(tr2==state->m_tr_2a)	{f3_alpha_blend_2a_4(state, s_pix);if(state->m_pdest_2a) state->m_pval |= state->m_pdest_2a;else return 1;}
	}
	else
	{
		if(tr2==state->m_tr_2b)		{if(state->m_pdest_2b) state->m_pval |= state->m_pdest_2b;else return 1;}
		else if(tr2==state->m_tr_2a)	{if(state->m_pdest_2a) state->m_pval |= state->m_pdest_2a;else return 1;}
	}
	return 0;
}
static int dpix_2_8(taito_f3_state *state, UINT32 s_pix)
{
	UINT8 tr2=state->m_tval&1;
	if(s_pix)
	{
		if(tr2==state->m_tr_2b)		{f3_alpha_blend_2b_8(state, s_pix);if(state->m_pdest_2b) state->m_pval |= state->m_pdest_2b;else return 1;}
		else if(tr2==state->m_tr_2a)	{f3_alpha_blend_2a_8(state, s_pix);if(state->m_pdest_2a) state->m_pval |= state->m_pdest_2a;else return 1;}
	}
	else
	{
		if(tr2==state->m_tr_2b)		{if(state->m_pdest_2b) state->m_pval |= state->m_pdest_2b;else return 1;}
		else if(tr2==state->m_tr_2a)	{if(state->m_pdest_2a) state->m_pval |= state->m_pdest_2a;else return 1;}
	}
	return 0;
}

static int dpix_3_0(taito_f3_state *state, UINT32 s_pix)
{
	UINT8 tr2=state->m_tval&1;
	if(s_pix)
	{
		if(tr2==state->m_tr_3b)		{f3_alpha_blend_3b_0(state, s_pix);if(state->m_pdest_3b) state->m_pval |= state->m_pdest_3b;else return 1;}
		else if(tr2==state->m_tr_3a)	{f3_alpha_blend_3a_0(state, s_pix);if(state->m_pdest_3a) state->m_pval |= state->m_pdest_3a;else return 1;}
	}
	else
	{
		if(tr2==state->m_tr_3b)		{state->m_dval = 0;if(state->m_pdest_3b) state->m_pval |= state->m_pdest_3b;else return 1;}
		else if(tr2==state->m_tr_3a)	{state->m_dval = 0;if(state->m_pdest_3a) state->m_pval |= state->m_pdest_3a;else return 1;}
	}
	return 0;
}
static int dpix_3_1(taito_f3_state *state, UINT32 s_pix)
{
	UINT8 tr2=state->m_tval&1;
	if(s_pix)
	{
		if(tr2==state->m_tr_3b)		{f3_alpha_blend_3b_1(state, s_pix);if(state->m_pdest_3b) state->m_pval |= state->m_pdest_3b;else return 1;}
		else if(tr2==state->m_tr_3a)	{f3_alpha_blend_3a_1(state, s_pix);if(state->m_pdest_3a) state->m_pval |= state->m_pdest_3a;else return 1;}
	}
	else
	{
		if(tr2==state->m_tr_3b)		{if(state->m_pdest_3b) state->m_pval |= state->m_pdest_3b;else return 1;}
		else if(tr2==state->m_tr_3a)	{if(state->m_pdest_3a) state->m_pval |= state->m_pdest_3a;else return 1;}
	}
	return 0;
}
static int dpix_3_2(taito_f3_state *state, UINT32 s_pix)
{
	UINT8 tr2=state->m_tval&1;
	if(s_pix)
	{
		if(tr2==state->m_tr_3b)		{f3_alpha_blend_3b_2(state, s_pix);if(state->m_pdest_3b) state->m_pval |= state->m_pdest_3b;else return 1;}
		else if(tr2==state->m_tr_3a)	{f3_alpha_blend_3a_2(state, s_pix);if(state->m_pdest_3a) state->m_pval |= state->m_pdest_3a;else return 1;}
	}
	else
	{
		if(tr2==state->m_tr_3b)		{if(state->m_pdest_3b) state->m_pval |= state->m_pdest_3b;else return 1;}
		else if(tr2==state->m_tr_3a)	{if(state->m_pdest_3a) state->m_pval |= state->m_pdest_3a;else return 1;}
	}
	return 0;
}

INLINE void dpix_1_sprite(taito_f3_state *state, UINT32 s_pix)
{
	if(s_pix)
	{
		UINT8 p1 = state->m_pval&0xf0;
		if     (p1==0x10)	f3_alpha_blend_1_1(state, s_pix);
		else if(p1==0x20)	f3_alpha_blend_1_2(state, s_pix);
		else if(p1==0x40)	f3_alpha_blend_1_4(state, s_pix);
		else if(p1==0x50)	f3_alpha_blend_1_5(state, s_pix);
		else if(p1==0x60)	f3_alpha_blend_1_6(state, s_pix);
		else if(p1==0x80)	f3_alpha_blend_1_8(state, s_pix);
		else if(p1==0x90)	f3_alpha_blend_1_9(state, s_pix);
		else if(p1==0xa0)	f3_alpha_blend_1_a(state, s_pix);
	}
}

INLINE void dpix_bg(taito_f3_state *state, UINT32 bgcolor)
{
	UINT8 p1 = state->m_pval&0xf0;
	if(!p1)			state->m_dval = bgcolor;
	else if(p1==0x10)	f3_alpha_blend_1_1(state, bgcolor);
	else if(p1==0x20)	f3_alpha_blend_1_2(state, bgcolor);
	else if(p1==0x40)	f3_alpha_blend_1_4(state, bgcolor);
	else if(p1==0x50)	f3_alpha_blend_1_5(state, bgcolor);
	else if(p1==0x60)	f3_alpha_blend_1_6(state, bgcolor);
	else if(p1==0x80)	f3_alpha_blend_1_8(state, bgcolor);
	else if(p1==0x90)	f3_alpha_blend_1_9(state, bgcolor);
	else if(p1==0xa0)	f3_alpha_blend_1_a(state, bgcolor);
}

/******************************************************************************/

static void init_alpha_blend_func(running_machine &machine)
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	int i,j;

	state->m_dpix_n[0][0x0]=dpix_1_noalpha;
	state->m_dpix_n[0][0x1]=dpix_1_noalpha;
	state->m_dpix_n[0][0x2]=dpix_1_noalpha;
	state->m_dpix_n[0][0x3]=dpix_1_noalpha;
	state->m_dpix_n[0][0x4]=dpix_1_noalpha;
	state->m_dpix_n[0][0x5]=dpix_1_noalpha;
	state->m_dpix_n[0][0x6]=dpix_1_noalpha;
	state->m_dpix_n[0][0x7]=dpix_1_noalpha;
	state->m_dpix_n[0][0x8]=dpix_1_noalpha;
	state->m_dpix_n[0][0x9]=dpix_1_noalpha;
	state->m_dpix_n[0][0xa]=dpix_1_noalpha;
	state->m_dpix_n[0][0xb]=dpix_1_noalpha;
	state->m_dpix_n[0][0xc]=dpix_1_noalpha;
	state->m_dpix_n[0][0xd]=dpix_1_noalpha;
	state->m_dpix_n[0][0xe]=dpix_1_noalpha;
	state->m_dpix_n[0][0xf]=dpix_1_noalpha;

	state->m_dpix_n[1][0x0]=dpix_1_noalpha;
	state->m_dpix_n[1][0x1]=dpix_1_1;
	state->m_dpix_n[1][0x2]=dpix_1_2;
	state->m_dpix_n[1][0x3]=dpix_ret1;
	state->m_dpix_n[1][0x4]=dpix_1_4;
	state->m_dpix_n[1][0x5]=dpix_1_5;
	state->m_dpix_n[1][0x6]=dpix_1_6;
	state->m_dpix_n[1][0x7]=dpix_ret1;
	state->m_dpix_n[1][0x8]=dpix_1_8;
	state->m_dpix_n[1][0x9]=dpix_1_9;
	state->m_dpix_n[1][0xa]=dpix_1_a;
	state->m_dpix_n[1][0xb]=dpix_ret1;
	state->m_dpix_n[1][0xc]=dpix_ret1;
	state->m_dpix_n[1][0xd]=dpix_ret1;
	state->m_dpix_n[1][0xe]=dpix_ret1;
	state->m_dpix_n[1][0xf]=dpix_ret1;

	state->m_dpix_n[2][0x0]=dpix_2a_0;
	state->m_dpix_n[2][0x1]=dpix_ret0;
	state->m_dpix_n[2][0x2]=dpix_ret0;
	state->m_dpix_n[2][0x3]=dpix_ret0;
	state->m_dpix_n[2][0x4]=dpix_2a_4;
	state->m_dpix_n[2][0x5]=dpix_ret0;
	state->m_dpix_n[2][0x6]=dpix_ret0;
	state->m_dpix_n[2][0x7]=dpix_ret0;
	state->m_dpix_n[2][0x8]=dpix_2a_8;
	state->m_dpix_n[2][0x9]=dpix_ret0;
	state->m_dpix_n[2][0xa]=dpix_ret0;
	state->m_dpix_n[2][0xb]=dpix_ret0;
	state->m_dpix_n[2][0xc]=dpix_ret0;
	state->m_dpix_n[2][0xd]=dpix_ret0;
	state->m_dpix_n[2][0xe]=dpix_ret0;
	state->m_dpix_n[2][0xf]=dpix_ret0;

	state->m_dpix_n[3][0x0]=dpix_3a_0;
	state->m_dpix_n[3][0x1]=dpix_3a_1;
	state->m_dpix_n[3][0x2]=dpix_3a_2;
	state->m_dpix_n[3][0x3]=dpix_ret0;
	state->m_dpix_n[3][0x4]=dpix_ret0;
	state->m_dpix_n[3][0x5]=dpix_ret0;
	state->m_dpix_n[3][0x6]=dpix_ret0;
	state->m_dpix_n[3][0x7]=dpix_ret0;
	state->m_dpix_n[3][0x8]=dpix_ret0;
	state->m_dpix_n[3][0x9]=dpix_ret0;
	state->m_dpix_n[3][0xa]=dpix_ret0;
	state->m_dpix_n[3][0xb]=dpix_ret0;
	state->m_dpix_n[3][0xc]=dpix_ret0;
	state->m_dpix_n[3][0xd]=dpix_ret0;
	state->m_dpix_n[3][0xe]=dpix_ret0;
	state->m_dpix_n[3][0xf]=dpix_ret0;

	state->m_dpix_n[4][0x0]=dpix_2b_0;
	state->m_dpix_n[4][0x1]=dpix_ret0;
	state->m_dpix_n[4][0x2]=dpix_ret0;
	state->m_dpix_n[4][0x3]=dpix_ret0;
	state->m_dpix_n[4][0x4]=dpix_2b_4;
	state->m_dpix_n[4][0x5]=dpix_ret0;
	state->m_dpix_n[4][0x6]=dpix_ret0;
	state->m_dpix_n[4][0x7]=dpix_ret0;
	state->m_dpix_n[4][0x8]=dpix_2b_8;
	state->m_dpix_n[4][0x9]=dpix_ret0;
	state->m_dpix_n[4][0xa]=dpix_ret0;
	state->m_dpix_n[4][0xb]=dpix_ret0;
	state->m_dpix_n[4][0xc]=dpix_ret0;
	state->m_dpix_n[4][0xd]=dpix_ret0;
	state->m_dpix_n[4][0xe]=dpix_ret0;
	state->m_dpix_n[4][0xf]=dpix_ret0;

	state->m_dpix_n[5][0x0]=dpix_3b_0;
	state->m_dpix_n[5][0x1]=dpix_3b_1;
	state->m_dpix_n[5][0x2]=dpix_3b_2;
	state->m_dpix_n[5][0x3]=dpix_ret0;
	state->m_dpix_n[5][0x4]=dpix_ret0;
	state->m_dpix_n[5][0x5]=dpix_ret0;
	state->m_dpix_n[5][0x6]=dpix_ret0;
	state->m_dpix_n[5][0x7]=dpix_ret0;
	state->m_dpix_n[5][0x8]=dpix_ret0;
	state->m_dpix_n[5][0x9]=dpix_ret0;
	state->m_dpix_n[5][0xa]=dpix_ret0;
	state->m_dpix_n[5][0xb]=dpix_ret0;
	state->m_dpix_n[5][0xc]=dpix_ret0;
	state->m_dpix_n[5][0xd]=dpix_ret0;
	state->m_dpix_n[5][0xe]=dpix_ret0;
	state->m_dpix_n[5][0xf]=dpix_ret0;

	state->m_dpix_n[6][0x0]=dpix_2_0;
	state->m_dpix_n[6][0x1]=dpix_ret0;
	state->m_dpix_n[6][0x2]=dpix_ret0;
	state->m_dpix_n[6][0x3]=dpix_ret0;
	state->m_dpix_n[6][0x4]=dpix_2_4;
	state->m_dpix_n[6][0x5]=dpix_ret0;
	state->m_dpix_n[6][0x6]=dpix_ret0;
	state->m_dpix_n[6][0x7]=dpix_ret0;
	state->m_dpix_n[6][0x8]=dpix_2_8;
	state->m_dpix_n[6][0x9]=dpix_ret0;
	state->m_dpix_n[6][0xa]=dpix_ret0;
	state->m_dpix_n[6][0xb]=dpix_ret0;
	state->m_dpix_n[6][0xc]=dpix_ret0;
	state->m_dpix_n[6][0xd]=dpix_ret0;
	state->m_dpix_n[6][0xe]=dpix_ret0;
	state->m_dpix_n[6][0xf]=dpix_ret0;

	state->m_dpix_n[7][0x0]=dpix_3_0;
	state->m_dpix_n[7][0x1]=dpix_3_1;
	state->m_dpix_n[7][0x2]=dpix_3_2;
	state->m_dpix_n[7][0x3]=dpix_ret0;
	state->m_dpix_n[7][0x4]=dpix_ret0;
	state->m_dpix_n[7][0x5]=dpix_ret0;
	state->m_dpix_n[7][0x6]=dpix_ret0;
	state->m_dpix_n[7][0x7]=dpix_ret0;
	state->m_dpix_n[7][0x8]=dpix_ret0;
	state->m_dpix_n[7][0x9]=dpix_ret0;
	state->m_dpix_n[7][0xa]=dpix_ret0;
	state->m_dpix_n[7][0xb]=dpix_ret0;
	state->m_dpix_n[7][0xc]=dpix_ret0;
	state->m_dpix_n[7][0xd]=dpix_ret0;
	state->m_dpix_n[7][0xe]=dpix_ret0;
	state->m_dpix_n[7][0xf]=dpix_ret0;

	for(i=0;i<256;i++)
		for(j=0;j<256;j++)
			state->m_add_sat[i][j] = (i + j < 256) ? i + j : 255;
}

/******************************************************************************/

#define GET_PIXMAP_POINTER(pf_num) \
{ \
	const struct f3_playfield_line_inf *line_tmp=line_t[pf_num]; \
	state->m_src##pf_num=line_tmp->src[y]; \
	state->m_src_s##pf_num=line_tmp->src_s[y]; \
	state->m_src_e##pf_num=line_tmp->src_e[y]; \
	state->m_tsrc##pf_num=line_tmp->tsrc[y]; \
	state->m_tsrc_s##pf_num=line_tmp->tsrc_s[y]; \
	state->m_x_count##pf_num=line_tmp->x_count[y]; \
	state->m_x_zoom##pf_num=line_tmp->x_zoom[y]; \
	state->m_clip_al##pf_num=line_tmp->clip0[y]&0xffff; \
	state->m_clip_ar##pf_num=line_tmp->clip0[y]>>16; \
	state->m_clip_bl##pf_num=line_tmp->clip1[y]&0xffff; \
	state->m_clip_br##pf_num=line_tmp->clip1[y]>>16; \
}

#define CULC_PIXMAP_POINTER(pf_num) \
{ \
	state->m_x_count##pf_num += state->m_x_zoom##pf_num; \
	if(state->m_x_count##pf_num>>16) \
	{ \
		state->m_x_count##pf_num &= 0xffff; \
		state->m_src##pf_num++; \
		state->m_tsrc##pf_num++; \
		if(state->m_src##pf_num==state->m_src_e##pf_num) {state->m_src##pf_num=state->m_src_s##pf_num; state->m_tsrc##pf_num=state->m_tsrc_s##pf_num;} \
	} \
}

#define UPDATE_PIXMAP_SP(pf_num) \
	if(cx>=clip_als && cx<clip_ars && !(cx>=clip_bls && cx<clip_brs)) \
	{ \
		sprite_pri=sprite[pf_num]&state->m_pval; \
		if(sprite_pri) \
		{ \
			if(sprite[pf_num]&0x100) break; \
			if(!state->m_dpix_sp[sprite_pri]) \
			{ \
				if(!(state->m_pval&0xf0)) break; \
				else {dpix_1_sprite(state, *dsti);*dsti=state->m_dval;break;} \
			} \
			if(state->m_dpix_sp[sprite_pri][state->m_pval>>4](state, *dsti)) {*dsti=state->m_dval;break;} \
		} \
	}

#define UPDATE_PIXMAP_LP(pf_num) \
	if (cx>=state->m_clip_al##pf_num && cx<state->m_clip_ar##pf_num && !(cx>=state->m_clip_bl##pf_num && cx<state->m_clip_br##pf_num)) \
	{ \
		state->m_tval=*state->m_tsrc##pf_num; \
		if(state->m_tval&0xf0) \
			if(state->m_dpix_lp[pf_num][state->m_pval>>4](state, clut[*state->m_src##pf_num])) {*dsti=state->m_dval;break;} \
	}


/*============================================================================*/

INLINE void draw_scanlines(running_machine &machine,
		bitmap_rgb32 &bitmap,int xsize,INT16 *draw_line_num,
		const struct f3_playfield_line_inf **line_t,
		const int *sprite,
		UINT32 orient,
		int skip_layer_num)
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	const pen_t *clut = &machine.pens[0];
	UINT32 bgcolor=clut[0];
	int length;

	const int x=46;






	UINT16 clip_als=0, clip_ars=0, clip_bls=0, clip_brs=0;

	UINT8 *dstp0,*dstp;

	int yadv = bitmap.rowpixels();
	int yadvp = state->m_pri_alp_bitmap.rowpixels();
	int i=0,y=draw_line_num[0];
	int ty = y;

	if (orient & ORIENTATION_FLIP_Y)
	{
		ty = bitmap.height() - 1 - ty;
		yadv = -yadv;
		yadvp = -yadvp;
	}

	dstp0 = &state->m_pri_alp_bitmap.pix8(ty, x);

	state->m_pdest_2a = state->m_f3_alpha_level_2ad ? 0x10 : 0;
	state->m_pdest_2b = state->m_f3_alpha_level_2bd ? 0x20 : 0;
	state->m_tr_2a =(state->m_f3_alpha_level_2as==0 && state->m_f3_alpha_level_2ad==255) ? -1 : 0;
	state->m_tr_2b =(state->m_f3_alpha_level_2bs==0 && state->m_f3_alpha_level_2bd==255) ? -1 : 1;
	state->m_pdest_3a = state->m_f3_alpha_level_3ad ? 0x40 : 0;
	state->m_pdest_3b = state->m_f3_alpha_level_3bd ? 0x80 : 0;
	state->m_tr_3a =(state->m_f3_alpha_level_3as==0 && state->m_f3_alpha_level_3ad==255) ? -1 : 0;
	state->m_tr_3b =(state->m_f3_alpha_level_3bs==0 && state->m_f3_alpha_level_3bd==255) ? -1 : 1;

	{
		UINT32 *dsti0,*dsti;
		dsti0 = &bitmap.pix32(ty, x);
		while(1)
		{
			int cx=0;

			clip_als=state->m_sa_line_inf[0].sprite_clip0[y]&0xffff;
			clip_ars=state->m_sa_line_inf[0].sprite_clip0[y]>>16;
			clip_bls=state->m_sa_line_inf[0].sprite_clip1[y]&0xffff;
			clip_brs=state->m_sa_line_inf[0].sprite_clip1[y]>>16;

			length=xsize;
			dsti = dsti0;
			dstp = dstp0;

			switch(skip_layer_num)
			{
				case 0: GET_PIXMAP_POINTER(0)
				case 1: GET_PIXMAP_POINTER(1)
				case 2: GET_PIXMAP_POINTER(2)
				case 3: GET_PIXMAP_POINTER(3)
				case 4: GET_PIXMAP_POINTER(4)
			}

			while (1)
			{
				state->m_pval=*dstp;
				if (state->m_pval!=0xff)
				{
					UINT8 sprite_pri;
					switch(skip_layer_num)
					{
						case 0: UPDATE_PIXMAP_SP(0) UPDATE_PIXMAP_LP(0)
						case 1: UPDATE_PIXMAP_SP(1) UPDATE_PIXMAP_LP(1)
						case 2: UPDATE_PIXMAP_SP(2) UPDATE_PIXMAP_LP(2)
						case 3: UPDATE_PIXMAP_SP(3) UPDATE_PIXMAP_LP(3)
						case 4: UPDATE_PIXMAP_SP(4) UPDATE_PIXMAP_LP(4)
						case 5: UPDATE_PIXMAP_SP(5)
								if(!bgcolor) {if(!(state->m_pval&0xf0)) {*dsti=0;break;}}
								else dpix_bg(state, bgcolor);
								*dsti=state->m_dval;
					}
				}

				if(!(--length)) break;
				dsti++;
				dstp++;
				cx++;

				switch(skip_layer_num)
				{
					case 0: CULC_PIXMAP_POINTER(0)
					case 1: CULC_PIXMAP_POINTER(1)
					case 2: CULC_PIXMAP_POINTER(2)
					case 3: CULC_PIXMAP_POINTER(3)
					case 4: CULC_PIXMAP_POINTER(4)
				}
			}

			i++;
			if(draw_line_num[i]<0) break;
			if(draw_line_num[i]==y+1)
			{
				dsti0 += yadv;
				dstp0 += yadvp;
				y++;
				continue;
			}
			else
			{
				dsti0 += (draw_line_num[i]-y)*yadv;
				dstp0 += (draw_line_num[i]-y)*yadvp;
				y=draw_line_num[i];
			}
		}
	}
}
#undef GET_PIXMAP_POINTER
#undef CULC_PIXMAP_POINTER

/******************************************************************************/

static void visible_tile_check(running_machine &machine,
							   struct f3_playfield_line_inf *line_t,
								int line,
								UINT32 x_index_fx,UINT32 y_index,
								UINT16 *f3_pf_data_n)
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	UINT16 *pf_base;
	int i,trans_all,tile_index,tile_num;
	int alpha_type,alpha_mode;
	int opaque_all;
	int total_elements;

	alpha_mode=line_t->alpha_mode[line];
	if(!alpha_mode) return;

	total_elements=machine.gfx[1]->total_elements;

	tile_index=x_index_fx>>16;
	tile_num=(((line_t->x_zoom[line]*320+(x_index_fx & 0xffff)+0xffff)>>16)+(tile_index%16)+15)/16;
	tile_index/=16;

	if (state->m_flipscreen)
	{
		pf_base=f3_pf_data_n+((31-(y_index/16))<<state->m_twidth_mask_bit);
		tile_index=(state->m_twidth_mask-tile_index)-tile_num+1;
	}
	else pf_base=f3_pf_data_n+((y_index/16)<<state->m_twidth_mask_bit);


	trans_all=1;
	opaque_all=1;
	alpha_type=0;
	for(i=0;i<tile_num;i++)
	{
		UINT32 tile=(pf_base[(tile_index*2+0)&state->m_twidth_mask]<<16)|(pf_base[(tile_index*2+1)&state->m_twidth_mask]);
		UINT8  extra_planes = (tile>>(16+10)) & 3;
		if(tile&0xffff)
		{
			trans_all=0;
			if(opaque_all)
			{
				if(state->m_tile_opaque_pf[extra_planes][(tile&0xffff)%total_elements]!=1) opaque_all=0;
			}

			if(alpha_mode==1)
			{
				if(!opaque_all) return;
			}
			else
			{
				if(alpha_type!=3)
				{
					if((tile>>(16+9))&1) alpha_type|=2;
					else				 alpha_type|=1;
				}
				else if(!opaque_all) break;
			}
		}
		else if(opaque_all) opaque_all=0;

		tile_index++;
	}

	if(trans_all)	{line_t->alpha_mode[line]=0;return;}

	if(alpha_mode>1)
	{
		line_t->alpha_mode[line]|=alpha_type<<4;
	}

	if(opaque_all)
		line_t->alpha_mode[line]|=0x80;
}

/******************************************************************************/

static void calculate_clip(taito_f3_state *state, int y, UINT16 pri, UINT32* clip0, UINT32* clip1, int* line_enable)
{
	const struct f3_spritealpha_line_inf *sa_line_t=&state->m_sa_line_inf[0];

	switch (pri)
	{
	case 0x0100: /* Clip plane 1 enable */
		{
			if (sa_line_t->clip0_l[y] > sa_line_t->clip0_r[y])
				*line_enable=0;
			else
				*clip0=(sa_line_t->clip0_l[y]) | (sa_line_t->clip0_r[y]<<16);
			*clip1=0;
		}
		break;
	case 0x0110: /* Clip plane 1 enable, inverted */
		{
			*clip1=(sa_line_t->clip0_l[y]) | (sa_line_t->clip0_r[y]<<16);
			*clip0=0x7fff0000;
		}
		break;
	case 0x0200: /* Clip plane 2 enable */
		{
			if (sa_line_t->clip1_l[y] > sa_line_t->clip1_r[y])
				*line_enable=0;
			else
				*clip0=(sa_line_t->clip1_l[y]) | (sa_line_t->clip1_r[y]<<16);
			*clip1=0;
		}
		break;
	case 0x0220: /* Clip plane 2 enable, inverted */
		{
			*clip1=(sa_line_t->clip1_l[y]) | (sa_line_t->clip1_r[y]<<16);
			*clip0=0x7fff0000;
		}
		break;
	case 0x0300: /* Clip plane 1 & 2 enable */
		{
			int clipl=0,clipr=0;

			if (sa_line_t->clip1_l[y] > sa_line_t->clip0_l[y])
				clipl=sa_line_t->clip1_l[y];
			else
				clipl=sa_line_t->clip0_l[y];

			if (sa_line_t->clip1_r[y] < sa_line_t->clip0_r[y])
				clipr=sa_line_t->clip1_r[y];
			else
				clipr=sa_line_t->clip0_r[y];

			if (clipl > clipr)
				*line_enable=0;
			else
				*clip0=(clipl) | (clipr<<16);
			*clip1=0;
		}
		break;
	case 0x0310: /* Clip plane 1 & 2 enable, plane 1 inverted */
		{
			if (sa_line_t->clip1_l[y] > sa_line_t->clip1_r[y])
				line_enable=0;
			else
				*clip0=(sa_line_t->clip1_l[y]) | (sa_line_t->clip1_r[y]<<16);

			*clip1=(sa_line_t->clip0_l[y]) | (sa_line_t->clip0_r[y]<<16);
		}
		break;
	case 0x0320: /* Clip plane 1 & 2 enable, plane 2 inverted */
		{
			if (sa_line_t->clip0_l[y] > sa_line_t->clip0_r[y])
				line_enable=0;
			else
				*clip0=(sa_line_t->clip0_l[y]) | (sa_line_t->clip0_r[y]<<16);

			*clip1=(sa_line_t->clip1_l[y]) | (sa_line_t->clip1_r[y]<<16);
		}
		break;
	case 0x0330: /* Clip plane 1 & 2 enable, both inverted */
		{
			int clipl=0,clipr=0;

			if (sa_line_t->clip1_l[y] < sa_line_t->clip0_l[y])
				clipl=sa_line_t->clip1_l[y];
			else
				clipl=sa_line_t->clip0_l[y];

			if (sa_line_t->clip1_r[y] > sa_line_t->clip0_r[y])
				clipr=sa_line_t->clip1_r[y];
			else
				clipr=sa_line_t->clip0_r[y];

			if (clipl > clipr)
				*line_enable=0;
			else
				*clip1=(clipl) | (clipr<<16);
			*clip0=0x7fff0000;
		}
		break;
	default:
		// popmessage("Illegal clip mode");
		break;
	}
}

static void get_spritealphaclip_info(taito_f3_state *state)
{
	struct f3_spritealpha_line_inf *line_t=&state->m_sa_line_inf[0];

	int y,y_end,y_inc;

	int spri_base,clip_base_low,clip_base_high,inc;

	UINT16 spri=0;
	UINT16 sprite_clip=0;
	UINT16 clip0_low=0, clip0_high=0, clip1_low=0;
	int alpha_level=0;
	UINT16 sprite_alpha=0;

	if (state->m_flipscreen)
	{
		spri_base=0x77fe;
		clip_base_low=0x51fe;
		clip_base_high=0x45fe;
		inc=-2;
		y=255;
		y_end=-1;
		y_inc=-1;

	}
	else
	{
		spri_base=0x7600;
		clip_base_low=0x5000;
		clip_base_high=0x4400;
		inc=2;
		y=0;
		y_end=256;
		y_inc=1;
	}

	while(y!=y_end)
	{
		/* The zoom, column and row values can latch according to control ram */
		{
			if (state->m_f3_line_ram[0x100+(y)]&1)
				clip0_low=(state->m_f3_line_ram[clip_base_low/2]>> 0)&0xffff;
			if (state->m_f3_line_ram[0x000+(y)]&4)
				clip0_high=(state->m_f3_line_ram[clip_base_high/2]>> 0)&0xffff;
			if (state->m_f3_line_ram[0x100+(y)]&2)
				clip1_low=(state->m_f3_line_ram[(clip_base_low+0x200)/2]>> 0)&0xffff;

			if (state->m_f3_line_ram[(0x0600/2)+(y)]&0x8)
				spri=state->m_f3_line_ram[spri_base/2]&0xffff;
			if (state->m_f3_line_ram[(0x0600/2)+(y)]&0x4)
				sprite_clip=state->m_f3_line_ram[(spri_base-0x200)/2]&0xffff;
			if (state->m_f3_line_ram[(0x0400/2)+(y)]&0x1)
				sprite_alpha=state->m_f3_line_ram[(spri_base-0x1600)/2]&0xffff;
			if (state->m_f3_line_ram[(0x0400/2)+(y)]&0x2)
				alpha_level=state->m_f3_line_ram[(spri_base-0x1400)/2]&0xffff;
		}


		line_t->alpha_level[y]=alpha_level;
		line_t->spri[y]=spri;
		line_t->sprite_alpha[y]=sprite_alpha;
		line_t->clip0_l[y]=((clip0_low&0xff)|((clip0_high&0x1000)>>4)) - 47;
		line_t->clip0_r[y]=(((clip0_low&0xff00)>>8)|((clip0_high&0x2000)>>5)) - 47;
		line_t->clip1_l[y]=((clip1_low&0xff)|((clip0_high&0x4000)>>6)) - 47;
		line_t->clip1_r[y]=(((clip1_low&0xff00)>>8)|((clip0_high&0x8000)>>7)) - 47;
		if (line_t->clip0_l[y]<0) line_t->clip0_l[y]=0;
		if (line_t->clip0_r[y]<0) line_t->clip0_r[y]=0;
		if (line_t->clip1_l[y]<0) line_t->clip1_l[y]=0;
		if (line_t->clip1_r[y]<0) line_t->clip1_r[y]=0;

		/* Evaluate sprite clipping */
		if (sprite_clip&0x080)
		{
			line_t->sprite_clip0[y]=0x7fff7fff;
			line_t->sprite_clip1[y]=0;
		}
		else if (sprite_clip&0x33)
		{
			int line_enable=1;
			calculate_clip(state, y, ((sprite_clip&0x33)<<4), &line_t->sprite_clip0[y], &line_t->sprite_clip1[y], &line_enable);
			if (line_enable==0)
				line_t->sprite_clip0[y]=0x7fff7fff;
		}
		else
		{
			line_t->sprite_clip0[y]=0x7fff0000;
			line_t->sprite_clip1[y]=0;
		}

		spri_base+=inc;
		clip_base_low+=inc;
		clip_base_high+=inc;
		y +=y_inc;
	}
}

/* sx and sy are 16.16 fixed point numbers */
static void get_line_ram_info(running_machine &machine, tilemap_t *tmap, int sx, int sy, int pos, UINT16 *f3_pf_data_n)
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	struct f3_playfield_line_inf *line_t=&state->m_pf_line_inf[pos];

	int y,y_start,y_end,y_inc;
	int line_base,zoom_base,col_base,pri_base,inc;

	int line_enable;
	int colscroll=0,x_offset=0,line_zoom=0;
	UINT32 _y_zoom[256];
	UINT16 pri=0;
	int bit_select=1<<pos;

	int _colscroll[256];
	UINT32 _x_offset[256];
	int y_index_fx;

	sx+=((46<<16));

	if (state->m_flipscreen)
	{
		line_base=0xa1fe + (pos*0x200);
		zoom_base=0x81fe;// + (pos*0x200);
		col_base =0x41fe + (pos*0x200);
		pri_base =0xb1fe + (pos*0x200);
		inc=-2;
		y_start=255;
		y_end=-1;
		y_inc=-1;

		if (state->m_f3_game_config->extend)	sx=-sx+((188-512)<<16); else sx=-sx+(188<<16); /* Adjust for flipped scroll position */
		y_index_fx=-sy-(256<<16); /* Adjust for flipped scroll position */
	}
	else
	{
		line_base=0xa000 + (pos*0x200);
		zoom_base=0x8000;// + (pos*0x200);
		col_base =0x4000 + (pos*0x200);
		pri_base =0xb000 + (pos*0x200);
		inc=2;
		y_start=0;
		y_end=256;
		y_inc=1;

		y_index_fx=sy;
	}

	y=y_start;

	while(y!=y_end)
	{
		/* The zoom, column and row values can latch according to control ram */
		{
			if (state->m_f3_line_ram[0x600+(y)]&bit_select)
				x_offset=(state->m_f3_line_ram[line_base/2]&0xffff)<<10;
			if (state->m_f3_line_ram[0x700+(y)]&bit_select)
				pri=state->m_f3_line_ram[pri_base/2]&0xffff;

			// Zoom for playfields 1 & 3 is interleaved, as is the latch select
			switch (pos)
			{
			case 0:
				if (state->m_f3_line_ram[0x400+(y)]&bit_select)
					line_zoom=state->m_f3_line_ram[(zoom_base+0x000)/2]&0xffff;
				break;
			case 1:
				if (state->m_f3_line_ram[0x400+(y)]&0x2)
					line_zoom=((state->m_f3_line_ram[(zoom_base+0x200)/2]&0xffff)&0xff00) | (line_zoom&0x00ff);
				if (state->m_f3_line_ram[0x400+(y)]&0x8)
					line_zoom=((state->m_f3_line_ram[(zoom_base+0x600)/2]&0xffff)&0x00ff) | (line_zoom&0xff00);
				break;
			case 2:
				if (state->m_f3_line_ram[0x400+(y)]&bit_select)
					line_zoom=state->m_f3_line_ram[(zoom_base+0x400)/2]&0xffff;
				break;
			case 3:
				if (state->m_f3_line_ram[0x400+(y)]&0x8)
					line_zoom=((state->m_f3_line_ram[(zoom_base+0x600)/2]&0xffff)&0xff00) | (line_zoom&0x00ff);
				if (state->m_f3_line_ram[0x400+(y)]&0x2)
					line_zoom=((state->m_f3_line_ram[(zoom_base+0x200)/2]&0xffff)&0x00ff) | (line_zoom&0xff00);
				break;
			default:
				break;
			}

			// Column scroll only affects playfields 2 & 3
			if (pos>=2 && state->m_f3_line_ram[0x000+(y)]&bit_select)
				colscroll=(state->m_f3_line_ram[col_base/2]>> 0)&0x3ff;
		}

		if (!pri || (!state->m_flipscreen && y<24) || (state->m_flipscreen && y>231) ||
			(pri&0xc000)==0xc000 || !(pri&0x2000)/**/)
			line_enable=0;
		else if(pri&0x4000)	//alpha1
			line_enable=2;
		else if(pri&0x8000)	//alpha2
			line_enable=3;
		else
			line_enable=1;

		_colscroll[y]=colscroll;
		_x_offset[y]=(x_offset&0xffff0000) - (x_offset&0x0000ffff);
		_y_zoom[y] = (line_zoom&0xff) << 9;

		/* Evaluate clipping */
		if (pri&0x0800)
			line_enable=0;
		else if (pri&0x0330)
		{
			//fast path todo - remove line enable
			calculate_clip(state, y, pri&0x0330, &line_t->clip0[y], &line_t->clip1[y], &line_enable);
		}
		else
		{
			/* No clipping */
			line_t->clip0[y]=0x7fff0000;
			line_t->clip1[y]=0;
		}

		line_t->x_zoom[y]=0x10000 - (line_zoom&0xff00);
		line_t->alpha_mode[y]=line_enable;
		line_t->pri[y]=pri;

		zoom_base+=inc;
		line_base+=inc;
		col_base +=inc;
		pri_base +=inc;
		y +=y_inc;
	}


	tilemap_t* tm = tmap;

	y=y_start;
	while(y!=y_end)
	{
		UINT32 x_index_fx;
		UINT32 y_index;

		/* The football games use values in the range 0x200-0x3ff where the crowd should be drawn - !?

           This appears to cause it to reference outside of the normal tilemap RAM area into the unused
           area on the 32x32 tilemap configuration.. but exactly how isn't understood

            Until this is understood we're creating additional tilemaps for the otherwise unused area of
            RAM and forcing it to look up there.

            the crowd area still seems to 'lag' behind the pitch area however.. but these are the values
            in ram??
        */
		int cs = _colscroll[y];

		if (cs&0x200)
		{
			if (state->m_pf5_tilemap && state->m_pf6_tilemap)
			{
				if (tmap == state->m_pf3_tilemap) tmap = state->m_pf5_tilemap; // pitch -> crowd
				if (tmap == state->m_pf4_tilemap) tmap = state->m_pf6_tilemap; // corruption on goals -> blank (hthero94)
			}
		}
		else
		{
			tmap = tm;
		}

		/* set pixmap pointer */
		bitmap_ind16 &srcbitmap = tmap->pixmap();
		bitmap_ind8 &flagsbitmap = tmap->flagsmap();

		if(line_t->alpha_mode[y]!=0)
		{
			UINT16 *src_s;
			UINT8 *tsrc_s;

			x_index_fx = (sx+_x_offset[y]-(10*0x10000)+(10*line_t->x_zoom[y]))&((state->m_width_mask<<16)|0xffff);
			y_index = ((y_index_fx>>16)+_colscroll[y])&0x1ff;

			/* check tile status */
			visible_tile_check(machine, line_t,y,x_index_fx,y_index,f3_pf_data_n);

			/* If clipping enabled for this line have to disable 'all opaque' optimisation */
			if (line_t->clip0[y]!=0x7fff0000 || line_t->clip1[y]!=0)
				line_t->alpha_mode[y]&=~0x80;

			/* set pixmap index */
			line_t->x_count[y]=x_index_fx & 0xffff; // Fractional part
			line_t->src_s[y]=src_s=&srcbitmap.pix16(y_index);
			line_t->src_e[y]=&src_s[state->m_width_mask+1];
			line_t->src[y]=&src_s[x_index_fx>>16];

			line_t->tsrc_s[y]=tsrc_s=&flagsbitmap.pix8(y_index);
			line_t->tsrc[y]=&tsrc_s[x_index_fx>>16];
		}

		y_index_fx += _y_zoom[y];
		y +=y_inc;
	}
}

static void get_vram_info(running_machine &machine, tilemap_t *vram_tilemap, tilemap_t *pixel_tilemap, int sx, int sy)
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	const struct f3_spritealpha_line_inf *sprite_alpha_line_t=&state->m_sa_line_inf[0];
	struct f3_playfield_line_inf *line_t=&state->m_pf_line_inf[4];

	int y,y_start,y_end,y_inc;
	int pri_base,inc;

	int line_enable;

	UINT16 pri=0;

	const int vram_width_mask=0x3ff;

	if (state->m_flipscreen)
	{
		pri_base =0x73fe;
		inc=-2;
		y_start=255;
		y_end=-1;
		y_inc=-1;
	}
	else
	{
		pri_base =0x7200;
		inc=2;
		y_start=0;
		y_end=256;
		y_inc=1;

	}

	y=y_start;
	while(y!=y_end)
	{
		/* The zoom, column and row values can latch according to control ram */
		{
			if (state->m_f3_line_ram[(0x0600/2)+(y)]&0x2)
				pri=(state->m_f3_line_ram[pri_base/2]&0xffff);
		}


		if (!pri || (!state->m_flipscreen && y<24) || (state->m_flipscreen && y>231) ||
			(pri&0xc000)==0xc000 || !(pri&0x2000)/**/)
			line_enable=0;
		else if(pri&0x4000)	//alpha1
			line_enable=2;
		else if(pri&0x8000)	//alpha2
			line_enable=3;
		else
			line_enable=1;

		line_t->pri[y]=pri;

		/* Evaluate clipping */
		if (pri&0x0800)
			line_enable=0;
		else if (pri&0x0330)
		{
			//fast path todo - remove line enable
			calculate_clip(state, y, pri&0x0330, &line_t->clip0[y], &line_t->clip1[y], &line_enable);
		}
		else
		{
			/* No clipping */
			line_t->clip0[y]=0x7fff0000;
			line_t->clip1[y]=0;
		}

		line_t->x_zoom[y]=0x10000;
		line_t->alpha_mode[y]=line_enable;
		if (line_t->alpha_mode[y]>1)
			line_t->alpha_mode[y]|=0x10;

		pri_base +=inc;
		y +=y_inc;
	}

	sx&=0x1ff;

	/* set pixmap pointer */
	bitmap_ind16 &srcbitmap_pixel = pixel_tilemap->pixmap();
	bitmap_ind8 &flagsbitmap_pixel = pixel_tilemap->flagsmap();
	bitmap_ind16 &srcbitmap_vram = vram_tilemap->pixmap();
	bitmap_ind8 &flagsbitmap_vram = vram_tilemap->flagsmap();

	y=y_start;
	while(y!=y_end)
	{
		if(line_t->alpha_mode[y]!=0)
		{
			UINT16 *src_s;
			UINT8 *tsrc_s;

			// These bits in control ram indicate whether the line is taken from
			// the VRAM tilemap layer or pixel layer.
			const int usePixelLayer=((sprite_alpha_line_t->sprite_alpha[y]&0xa000)==0xa000);

			/* set pixmap index */
			line_t->x_count[y]=0xffff;
			if (usePixelLayer)
				line_t->src_s[y]=src_s=&srcbitmap_pixel.pix16(sy&0xff);
			else
				line_t->src_s[y]=src_s=&srcbitmap_vram.pix16(sy&0x1ff);
			line_t->src_e[y]=&src_s[vram_width_mask+1];
			line_t->src[y]=&src_s[sx];

			if (usePixelLayer)
				line_t->tsrc_s[y]=tsrc_s=&flagsbitmap_pixel.pix8(sy&0xff);
			else
				line_t->tsrc_s[y]=tsrc_s=&flagsbitmap_vram.pix8(sy&0x1ff);
			line_t->tsrc[y]=&tsrc_s[sx];
		}

		sy++;
		y += y_inc;
	}
}

/******************************************************************************/

static void scanline_draw(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	int i,j,y,ys,ye;
	int y_start,y_end,y_start_next,y_end_next;
	UINT8 draw_line[256];
	INT16 draw_line_num[256];

	UINT32 rot=0;

	if (state->m_flipscreen)
	{
		rot=ORIENTATION_FLIP_Y;
		ys=0;
		ye=232;
	}
	else
	{
		ys=24;
		ye=256;
	}

	y_start=ys;
	y_end=ye;
	memset(draw_line,0,256);

	while(1)
	{
		int pos;
		int pri[5],alpha_mode[5],alpha_mode_flag[5],alpha_level;
		UINT16 sprite_alpha;
		UINT8 sprite_alpha_check;
		UINT8 sprite_alpha_all_2a;
		int spri;
		int alpha;
		int layer_tmp[5];
		struct f3_playfield_line_inf *pf_line_inf = state->m_pf_line_inf;
		struct f3_spritealpha_line_inf *sa_line_inf = state->m_sa_line_inf;
		int count_skip_layer=0;
		int sprite[6]={0,0,0,0,0,0};
		const struct f3_playfield_line_inf *line_t[5];


		/* find same status of scanlines */
		pri[0]=pf_line_inf[0].pri[y_start];
		pri[1]=pf_line_inf[1].pri[y_start];
		pri[2]=pf_line_inf[2].pri[y_start];
		pri[3]=pf_line_inf[3].pri[y_start];
		pri[4]=pf_line_inf[4].pri[y_start];
		alpha_mode[0]=pf_line_inf[0].alpha_mode[y_start];
		alpha_mode[1]=pf_line_inf[1].alpha_mode[y_start];
		alpha_mode[2]=pf_line_inf[2].alpha_mode[y_start];
		alpha_mode[3]=pf_line_inf[3].alpha_mode[y_start];
		alpha_mode[4]=pf_line_inf[4].alpha_mode[y_start];
		alpha_level=sa_line_inf[0].alpha_level[y_start];
		spri=sa_line_inf[0].spri[y_start];
		sprite_alpha=sa_line_inf[0].sprite_alpha[y_start];

		draw_line[y_start]=1;
		draw_line_num[i=0]=y_start;
		y_start_next=-1;
		y_end_next=-1;
		for(y=y_start+1;y<y_end;y++)
		{
			if(!draw_line[y])
			{
				if(pri[0]!=pf_line_inf[0].pri[y]) y_end_next=y+1;
				else if(pri[1]!=pf_line_inf[1].pri[y]) y_end_next=y+1;
				else if(pri[2]!=pf_line_inf[2].pri[y]) y_end_next=y+1;
				else if(pri[3]!=pf_line_inf[3].pri[y]) y_end_next=y+1;
				else if(pri[4]!=pf_line_inf[4].pri[y]) y_end_next=y+1;
				else if(alpha_mode[0]!=pf_line_inf[0].alpha_mode[y]) y_end_next=y+1;
				else if(alpha_mode[1]!=pf_line_inf[1].alpha_mode[y]) y_end_next=y+1;
				else if(alpha_mode[2]!=pf_line_inf[2].alpha_mode[y]) y_end_next=y+1;
				else if(alpha_mode[3]!=pf_line_inf[3].alpha_mode[y]) y_end_next=y+1;
				else if(alpha_mode[4]!=pf_line_inf[4].alpha_mode[y]) y_end_next=y+1;
				else if(alpha_level!=sa_line_inf[0].alpha_level[y]) y_end_next=y+1;
				else if(spri!=sa_line_inf[0].spri[y]) y_end_next=y+1;
				else if(sprite_alpha!=sa_line_inf[0].sprite_alpha[y]) y_end_next=y+1;
				else
				{
					draw_line[y]=1;
					draw_line_num[++i]=y;
					continue;
				}

				if(y_start_next<0) y_start_next=y;
			}
		}
		y_end=y_end_next;
		y_start=y_start_next;
		draw_line_num[++i]=-1;

		/* alpha blend */
		alpha_mode_flag[0]=alpha_mode[0]&~3;
		alpha_mode_flag[1]=alpha_mode[1]&~3;
		alpha_mode_flag[2]=alpha_mode[2]&~3;
		alpha_mode_flag[3]=alpha_mode[3]&~3;
		alpha_mode_flag[4]=alpha_mode[4]&~3;
		alpha_mode[0]&=3;
		alpha_mode[1]&=3;
		alpha_mode[2]&=3;
		alpha_mode[3]&=3;
		alpha_mode[4]&=3;
		if( alpha_mode[0]>1 ||
			alpha_mode[1]>1 ||
			alpha_mode[2]>1 ||
			alpha_mode[3]>1 ||
			alpha_mode[4]>1 ||
			(sprite_alpha&0xff) != 0xff  )
		{
			/* set alpha level */
			if(alpha_level!=state->m_alpha_level_last)
			{
				int al_s,al_d;
				int a=alpha_level;
				int b=(a>>8)&0xf;
				int c=(a>>4)&0xf;
				int d=(a>>0)&0xf;
				a>>=12;

				/* b000 7000 */
				al_s = ( (15-d)*256) / 8;
				al_d = ( (15-b)*256) / 8;
				if(al_s>255) al_s = 255;
				if(al_d>255) al_d = 255;
				state->m_f3_alpha_level_3as = al_s;
				state->m_f3_alpha_level_3ad = al_d;
				state->m_f3_alpha_level_2as = al_d;
				state->m_f3_alpha_level_2ad = al_s;

				al_s = ( (15-c)*256) / 8;
				al_d = ( (15-a)*256) / 8;
				if(al_s>255) al_s = 255;
				if(al_d>255) al_d = 255;
				state->m_f3_alpha_level_3bs = al_s;
				state->m_f3_alpha_level_3bd = al_d;
				state->m_f3_alpha_level_2bs = al_d;
				state->m_f3_alpha_level_2bd = al_s;

				f3_alpha_set_level(state);
				state->m_alpha_level_last=alpha_level;
			}

			/* set sprite alpha mode */
			sprite_alpha_check=0;
			sprite_alpha_all_2a=1;
			state->m_dpix_sp[1]=0;
			state->m_dpix_sp[2]=0;
			state->m_dpix_sp[4]=0;
			state->m_dpix_sp[8]=0;
			for(i=0;i<4;i++)	/* i = sprite priority offset */
			{
				UINT8 sprite_alpha_mode=(sprite_alpha>>(i*2))&3;
				UINT8 sftbit=1<<i;
				if(state->m_sprite_pri_usage&sftbit)
				{
					if(sprite_alpha_mode==1)
					{
						if(state->m_f3_alpha_level_2as==0 && state->m_f3_alpha_level_2ad==255)
							state->m_sprite_pri_usage&=~sftbit;  // Disable sprite priority block
						else
						{
							state->m_dpix_sp[sftbit]=state->m_dpix_n[2];
							sprite_alpha_check|=sftbit;
						}
					}
					else if(sprite_alpha_mode==2)
					{
						if(sprite_alpha&0xff00)
						{
							if(state->m_f3_alpha_level_3as==0 && state->m_f3_alpha_level_3ad==255) state->m_sprite_pri_usage&=~sftbit;
							else
							{
								state->m_dpix_sp[sftbit]=state->m_dpix_n[3];
								sprite_alpha_check|=sftbit;
								sprite_alpha_all_2a=0;
							}
						}
						else
						{
							if(state->m_f3_alpha_level_3bs==0 && state->m_f3_alpha_level_3bd==255) state->m_sprite_pri_usage&=~sftbit;
							else
							{
								state->m_dpix_sp[sftbit]=state->m_dpix_n[5];
								sprite_alpha_check|=sftbit;
								sprite_alpha_all_2a=0;
							}
						}
					}
				}
			}


			/* check alpha level */
			for(i=0;i<5;i++)	/* i = playfield num (pos) */
			{
				int alpha_type = (alpha_mode_flag[i]>>4)&3;

				if(alpha_mode[i]==2)
				{
					if(alpha_type==1)
					{
						/* if (state->m_f3_alpha_level_2as==0   && state->m_f3_alpha_level_2ad==255)
                         *     alpha_mode[i]=3; alpha_mode_flag[i] |= 0x80;}
                         * will display continue screen in gseeker (mt 00026) */
						if     (state->m_f3_alpha_level_2as==0   && state->m_f3_alpha_level_2ad==255) alpha_mode[i]=0;
						else if(state->m_f3_alpha_level_2as==255 && state->m_f3_alpha_level_2ad==0  ) alpha_mode[i]=1;
					}
					else if(alpha_type==2)
					{
						if     (state->m_f3_alpha_level_2bs==0   && state->m_f3_alpha_level_2bd==255) alpha_mode[i]=0;
						else if(state->m_f3_alpha_level_2as==255 && state->m_f3_alpha_level_2ad==0 &&
								state->m_f3_alpha_level_2bs==255 && state->m_f3_alpha_level_2bd==0  ) alpha_mode[i]=1;
					}
					else if(alpha_type==3)
					{
						if     (state->m_f3_alpha_level_2as==0   && state->m_f3_alpha_level_2ad==255 &&
								state->m_f3_alpha_level_2bs==0   && state->m_f3_alpha_level_2bd==255) alpha_mode[i]=0;
						else if(state->m_f3_alpha_level_2as==255 && state->m_f3_alpha_level_2ad==0   &&
								state->m_f3_alpha_level_2bs==255 && state->m_f3_alpha_level_2bd==0  ) alpha_mode[i]=1;
					}
				}
				else if(alpha_mode[i]==3)
				{
					if(alpha_type==1)
					{
						if     (state->m_f3_alpha_level_3as==0   && state->m_f3_alpha_level_3ad==255) alpha_mode[i]=0;
						else if(state->m_f3_alpha_level_3as==255 && state->m_f3_alpha_level_3ad==0  ) alpha_mode[i]=1;
					}
					else if(alpha_type==2)
					{
						if     (state->m_f3_alpha_level_3bs==0   && state->m_f3_alpha_level_3bd==255) alpha_mode[i]=0;
						else if(state->m_f3_alpha_level_3as==255 && state->m_f3_alpha_level_3ad==0 &&
								state->m_f3_alpha_level_3bs==255 && state->m_f3_alpha_level_3bd==0  ) alpha_mode[i]=1;
					}
					else if(alpha_type==3)
					{
						if     (state->m_f3_alpha_level_3as==0   && state->m_f3_alpha_level_3ad==255 &&
								state->m_f3_alpha_level_3bs==0   && state->m_f3_alpha_level_3bd==255) alpha_mode[i]=0;
						else if(state->m_f3_alpha_level_3as==255 && state->m_f3_alpha_level_3ad==0   &&
								state->m_f3_alpha_level_3bs==255 && state->m_f3_alpha_level_3bd==0  ) alpha_mode[i]=1;
					}
				}
			}

			if (	(alpha_mode[0]==1 || alpha_mode[0]==2 || !alpha_mode[0]) &&
					(alpha_mode[1]==1 || alpha_mode[1]==2 || !alpha_mode[1]) &&
					(alpha_mode[2]==1 || alpha_mode[2]==2 || !alpha_mode[2]) &&
					(alpha_mode[3]==1 || alpha_mode[3]==2 || !alpha_mode[3]) &&
					(alpha_mode[4]==1 || alpha_mode[4]==2 || !alpha_mode[4]) &&
					sprite_alpha_all_2a						)
			{
				int alpha_type = (alpha_mode_flag[0] | alpha_mode_flag[1] | alpha_mode_flag[2] | alpha_mode_flag[3])&0x30;
				if(		(alpha_type==0x10 && state->m_f3_alpha_level_2as==255) ||
						(alpha_type==0x20 && state->m_f3_alpha_level_2as==255 && state->m_f3_alpha_level_2bs==255) ||
						(alpha_type==0x30 && state->m_f3_alpha_level_2as==255 && state->m_f3_alpha_level_2bs==255)	)
				{
					if(alpha_mode[0]>1) alpha_mode[0]=1;
					if(alpha_mode[1]>1) alpha_mode[1]=1;
					if(alpha_mode[2]>1) alpha_mode[2]=1;
					if(alpha_mode[3]>1) alpha_mode[3]=1;
					if(alpha_mode[4]>1) alpha_mode[4]=1;
					sprite_alpha_check=0;
					state->m_dpix_sp[1]=0;
					state->m_dpix_sp[2]=0;
					state->m_dpix_sp[4]=0;
					state->m_dpix_sp[8]=0;
				}
			}
		}
		else
		{
			sprite_alpha_check=0;
			state->m_dpix_sp[1]=0;
			state->m_dpix_sp[2]=0;
			state->m_dpix_sp[4]=0;
			state->m_dpix_sp[8]=0;
		}



		/* set scanline priority */
		{
			int pri_max_opa=-1;
			for(i=0;i<5;i++)	/* i = playfield num (pos) */
			{
				int p0=pri[i];
				int pri_sl1=p0&0x0f;

				layer_tmp[i]=i + (pri_sl1<<3);

				if(!alpha_mode[i])
				{
					layer_tmp[i]|=0x80;
					count_skip_layer++;
				}
				else if(alpha_mode[i]==1 && (alpha_mode_flag[i]&0x80))
				{
					if(layer_tmp[i]>pri_max_opa) pri_max_opa=layer_tmp[i];
				}
			}

			if(pri_max_opa!=-1)
			{
				if(pri_max_opa>layer_tmp[0]) {layer_tmp[0]|=0x80;count_skip_layer++;}
				if(pri_max_opa>layer_tmp[1]) {layer_tmp[1]|=0x80;count_skip_layer++;}
				if(pri_max_opa>layer_tmp[2]) {layer_tmp[2]|=0x80;count_skip_layer++;}
				if(pri_max_opa>layer_tmp[3]) {layer_tmp[3]|=0x80;count_skip_layer++;}
				if(pri_max_opa>layer_tmp[4]) {layer_tmp[4]|=0x80;count_skip_layer++;}
			}
		}


		/* sort layer_tmp */
		for(i=0;i<4;i++)
		{
			for(j=i+1;j<5;j++)
			{
				if(layer_tmp[i]<layer_tmp[j])
				{
					int temp = layer_tmp[i];
					layer_tmp[i] = layer_tmp[j];
					layer_tmp[j] = temp;
				}
			}
		}


		/* check sprite & layer priority */
		{
			int l0,l1,l2,l3,l4;
			int pri_sp[5];

			l0=layer_tmp[0]>>3;
			l1=layer_tmp[1]>>3;
			l2=layer_tmp[2]>>3;
			l3=layer_tmp[3]>>3;
			l4=layer_tmp[4]>>3;

			pri_sp[0]=spri&0xf;
			pri_sp[1]=(spri>>4)&0xf;
			pri_sp[2]=(spri>>8)&0xf;
			pri_sp[3]=spri>>12;

			for(i=0;i<4;i++)	/* i = sprite priority offset */
			{
				int sp,sflg=1<<i;
				if(!(state->m_sprite_pri_usage & sflg)) continue;
				sp=pri_sp[i];

				/*
                    sprite priority==playfield priority
                        GSEEKER (plane leaving hangar) --> sprite
                        BUBSYMPH (title)       ---> sprite
                        DARIUSG (ZONE V' BOSS) ---> playfield
                */

				if (state->m_f3_game == BUBSYMPH ) sp++;		//BUBSYMPH (title)
				if (state->m_f3_game == GSEEKER ) sp++;		//GSEEKER (plane leaving hangar)

					 if(		  sp>l0) sprite[0]|=sflg;
				else if(sp<=l0 && sp>l1) sprite[1]|=sflg;
				else if(sp<=l1 && sp>l2) sprite[2]|=sflg;
				else if(sp<=l2 && sp>l3) sprite[3]|=sflg;
				else if(sp<=l3 && sp>l4) sprite[4]|=sflg;
				else if(sp<=l4		   ) sprite[5]|=sflg;
			}
		}


		/* draw scanlines */
		alpha=0;
		for(i=count_skip_layer;i<5;i++)
		{
			pos=layer_tmp[i]&7;
			line_t[i]=&pf_line_inf[pos];

			if(sprite[i]&sprite_alpha_check) alpha=1;
			else if(!alpha) sprite[i]|=0x100;

			if(alpha_mode[pos]>1)
			{
				int alpha_type=(((alpha_mode_flag[pos]>>4)&3)-1)*2;
				state->m_dpix_lp[i]=state->m_dpix_n[alpha_mode[pos]+alpha_type];
				alpha=1;
			}
			else
			{
				if(alpha) state->m_dpix_lp[i]=state->m_dpix_n[1];
				else	  state->m_dpix_lp[i]=state->m_dpix_n[0];
			}
		}
		if(sprite[5]&sprite_alpha_check) alpha=1;
		else if(!alpha) sprite[5]|=0x100;

		draw_scanlines(machine, bitmap,320,draw_line_num,line_t,sprite,rot,count_skip_layer);
		if(y_start<0) break;
	}
}

/******************************************************************************/

#define PSET_T					\
	c = *source & state->m_sprite_pen_mask;	\
	if(c)						\
	{							\
		p=*pri;					\
		if(!p || p==0xff)		\
		{						\
			*dest = pal[c];		\
			*pri = pri_dst;		\
		}						\
	}

#define PSET_O					\
	p=*pri;						\
	if(!p || p==0xff)			\
	{							\
		*dest = pal[*source & state->m_sprite_pen_mask];	\
		*pri = pri_dst;			\
	}

#define NEXT_P					\
	source += dx;				\
	dest++;						\
	pri++;

INLINE void f3_drawgfx(
		bitmap_rgb32 &dest_bmp,const rectangle &clip,const gfx_element *gfx,
		int code,
		int color,
		int flipx,int flipy,
		int sx,int sy,
		UINT8 pri_dst)
{
	taito_f3_state *state = gfx->machine().driver_data<taito_f3_state>();
	rectangle myclip;

	pri_dst=1<<pri_dst;

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	myclip = clip;
	myclip &= dest_bmp.cliprect();


	if( gfx )
	{
		const pen_t *pal = &gfx->machine().pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
		const UINT8 *code_base = gfx_element_get_data(gfx, code % gfx->total_elements);

		{
			/* compute sprite increment per screen pixel */
			int dx = 1;
			int dy = 1;

			int ex = sx+16;
			int ey = sy+16;

			int x_index_base;
			int y_index;

			if( flipx )
			{
				x_index_base = 15;
				dx = -1;
			}
			else
			{
				x_index_base = 0;
			}

			if( flipy )
			{
				y_index = 15;
				dy = -1;
			}
			else
			{
				y_index = 0;
			}

			if( sx < myclip.min_x)
			{ /* clip left */
				int pixels = myclip.min_x-sx;
				sx += pixels;
				x_index_base += pixels*dx;
			}
			if( sy < myclip.min_y )
			{ /* clip top */
				int pixels = myclip.min_y-sy;
				sy += pixels;
				y_index += pixels*dy;
			}
			/* NS 980211 - fixed incorrect clipping */
			if( ex > myclip.max_x+1 )
			{ /* clip right */
				int pixels = ex-myclip.max_x-1;
				ex -= pixels;
			}
			if( ey > myclip.max_y+1 )
			{ /* clip bottom */
				int pixels = ey-myclip.max_y-1;
				ey -= pixels;
			}

			if( ex>sx && ey>sy)
			{ /* skip if inner loop doesn't draw anything */
//              if (dest_bmp.bpp == 32)
				{
					int y=ey-sy;
					int x=(ex-sx-1)|(state->m_tile_opaque_sp[code % gfx->total_elements]<<4);
					const UINT8 *source0 = code_base + y_index * 16 + x_index_base;
					UINT32 *dest0 = &dest_bmp.pix32(sy, sx);
					UINT8 *pri0 = &state->m_pri_alp_bitmap.pix8(sy, sx);
					int yadv = dest_bmp.rowpixels();
					int yadvp = state->m_pri_alp_bitmap.rowpixels();
					dy=dy*16;
					while(1)
					{
						const UINT8 *source = source0;
						UINT32 *dest = dest0;
						UINT8 *pri = pri0;

						switch(x)
						{
							int c;
							UINT8 p;
							case 31: PSET_O NEXT_P
							case 30: PSET_O NEXT_P
							case 29: PSET_O NEXT_P
							case 28: PSET_O NEXT_P
							case 27: PSET_O NEXT_P
							case 26: PSET_O NEXT_P
							case 25: PSET_O NEXT_P
							case 24: PSET_O NEXT_P
							case 23: PSET_O NEXT_P
							case 22: PSET_O NEXT_P
							case 21: PSET_O NEXT_P
							case 20: PSET_O NEXT_P
							case 19: PSET_O NEXT_P
							case 18: PSET_O NEXT_P
							case 17: PSET_O NEXT_P
							case 16: PSET_O break;

							case 15: PSET_T NEXT_P
							case 14: PSET_T NEXT_P
							case 13: PSET_T NEXT_P
							case 12: PSET_T NEXT_P
							case 11: PSET_T NEXT_P
							case 10: PSET_T NEXT_P
							case  9: PSET_T NEXT_P
							case  8: PSET_T NEXT_P
							case  7: PSET_T NEXT_P
							case  6: PSET_T NEXT_P
							case  5: PSET_T NEXT_P
							case  4: PSET_T NEXT_P
							case  3: PSET_T NEXT_P
							case  2: PSET_T NEXT_P
							case  1: PSET_T NEXT_P
							case  0: PSET_T
						}

						if(!(--y)) break;
						source0 += dy;
						dest0+=yadv;
						pri0+=yadvp;
					}
				}
			}
		}
	}
}
#undef PSET_T
#undef PSET_O
#undef NEXT_P


INLINE void f3_drawgfxzoom(
		bitmap_rgb32 &dest_bmp,const rectangle &clip,const gfx_element *gfx,
		int code,
		int color,
		int flipx,int flipy,
		int sx,int sy,
		int scalex, int scaley,
		UINT8 pri_dst)
{
	taito_f3_state *state = gfx->machine().driver_data<taito_f3_state>();
	rectangle myclip;

	pri_dst=1<<pri_dst;

	/* KW 991012 -- Added code to force clip to bitmap boundary */
	myclip = clip;
	myclip &= dest_bmp.cliprect();


	if( gfx )
	{
		const pen_t *pal = &gfx->machine().pens[gfx->color_base + gfx->color_granularity * (color % gfx->total_colors)];
		const UINT8 *code_base = gfx_element_get_data(gfx, code % gfx->total_elements);

		{
			/* compute sprite increment per screen pixel */
			int dx = (16<<16)/scalex;
			int dy = (16<<16)/scaley;

			int ex = sx+scalex;
			int ey = sy+scaley;

			int x_index_base;
			int y_index;

			if( flipx )
			{
				x_index_base = (scalex-1)*dx;
				dx = -dx;
			}
			else
			{
				x_index_base = 0;
			}

			if( flipy )
			{
				y_index = (scaley-1)*dy;
				dy = -dy;
			}
			else
			{
				y_index = 0;
			}

			if( sx < myclip.min_x)
			{ /* clip left */
				int pixels = myclip.min_x-sx;
				sx += pixels;
				x_index_base += pixels*dx;
			}
			if( sy < myclip.min_y )
			{ /* clip top */
				int pixels = myclip.min_y-sy;
				sy += pixels;
				y_index += pixels*dy;
			}
			/* NS 980211 - fixed incorrect clipping */
			if( ex > myclip.max_x+1 )
			{ /* clip right */
				int pixels = ex-myclip.max_x-1;
				ex -= pixels;
			}
			if( ey > myclip.max_y+1 )
			{ /* clip bottom */
				int pixels = ey-myclip.max_y-1;
				ey -= pixels;
			}

			if( ex>sx )
			{ /* skip if inner loop doesn't draw anything */
//              if (dest_bmp.bpp == 32)
				{
					int y;
					for( y=sy; y<ey; y++ )
					{
						const UINT8 *source = code_base + (y_index>>16) * 16;
						UINT32 *dest = &dest_bmp.pix32(y);
						UINT8 *pri = &state->m_pri_alp_bitmap.pix8(y);

						int x, x_index = x_index_base;
						for( x=sx; x<ex; x++ )
						{
							int c = source[x_index>>16] & state->m_sprite_pen_mask;
							if(c)
							{
								UINT8 p=pri[x];
								if (p == 0 || p == 0xff)
								{
									dest[x] = pal[c];
									pri[x] = pri_dst;
								}
							}
							x_index += dx;
						}
						y_index += dy;
					}
				}
			}
		}
	}
}


#define CALC_ZOOM(p)	{										\
	p##_addition = 0x100 - block_zoom_##p + p##_addition_left;	\
	p##_addition_left = p##_addition & 0xf;						\
	p##_addition = p##_addition >> 4;							\
	/*zoom##p = p##_addition << 12;*/							\
}

static void get_sprite_info(running_machine &machine, const UINT16 *spriteram16_ptr)
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	const rectangle &visarea = machine.primary_screen->visible_area();
	const int min_x=visarea.min_x,max_x=visarea.max_x;
	const int min_y=visarea.min_y,max_y=visarea.max_y;
	int offs,spritecont,flipx,flipy,/*old_x,*/color,x,y;
	int sprite,global_x=0,global_y=0,subglobal_x=0,subglobal_y=0;
	int block_x=0, block_y=0;
	int last_color=0,last_x=0,last_y=0,block_zoom_x=0,block_zoom_y=0;
	int this_x,this_y;
	int y_addition=16, x_addition=16;
	int multi=0;
	int sprite_top;

	int x_addition_left = 8, y_addition_left = 8;

	struct tempsprite *sprite_ptr = state->m_spritelist;

	int total_sprites=0;

	color=0;
	flipx=flipy=0;
	//old_x=0;
	y=x=0;

	sprite_top=0x2000;
	for (offs = 0; offs < sprite_top && (total_sprites < 0x400); offs += 8)
	{
		const int current_offs=offs; /* Offs can change during loop, current_offs cannot */

		/* Check if the sprite list jump command bit is set */
		if ((spriteram16_ptr[current_offs+6+0]) & 0x8000) {
			UINT32 jump = (spriteram16_ptr[current_offs+6+0])&0x3ff;

			UINT32 new_offs=((offs&0x4000)|((jump<<4)/2));
			if (new_offs==offs)
				break;
			offs=new_offs - 8;
		}

		/* Check if special command bit is set */
		if (spriteram16_ptr[current_offs+2+1] & 0x8000) {
			UINT32 cntrl=(spriteram16_ptr[current_offs+4+1])&0xffff;
			state->m_flipscreen=cntrl&0x2000;

			/*  cntrl&0x1000 = disabled?  (From F2 driver, doesn't seem used anywhere)
                cntrl&0x0010 = ???
                cntrl&0x0020 = ???
            */

			state->m_sprite_extra_planes = (cntrl & 0x0300) >> 8;	// 0 = 4bpp, 1 = 5bpp, 2 = unused?, 3 = 6bpp
			state->m_sprite_pen_mask = (state->m_sprite_extra_planes << 4) | 0x0f;

			/* Sprite bank select */
			if (cntrl&1) {
				offs=offs|0x4000;
				sprite_top=sprite_top|0x4000;
			}
		}

		/* Set global sprite scroll */
		if (((spriteram16_ptr[current_offs+2+0]) & 0xf000) == 0xa000) {
			global_x = (spriteram16_ptr[current_offs+2+0]) & 0xfff;
			if (global_x >= 0x800) global_x -= 0x1000;
			global_y = spriteram16_ptr[current_offs+2+1] & 0xfff;
			if (global_y >= 0x800) global_y -= 0x1000;
		}

		/* And sub-global sprite scroll */
		if (((spriteram16_ptr[current_offs+2+0]) & 0xf000) == 0x5000) {
			subglobal_x = (spriteram16_ptr[current_offs+2+0]) & 0xfff;
			if (subglobal_x >= 0x800) subglobal_x -= 0x1000;
			subglobal_y = spriteram16_ptr[current_offs+2+1] & 0xfff;
			if (subglobal_y >= 0x800) subglobal_y -= 0x1000;
		}

		if (((spriteram16_ptr[current_offs+2+0]) & 0xf000) == 0xb000) {
			subglobal_x = (spriteram16_ptr[current_offs+2+0]) & 0xfff;
			if (subglobal_x >= 0x800) subglobal_x -= 0x1000;
			subglobal_y = spriteram16_ptr[current_offs+2+1] & 0xfff;
			if (subglobal_y >= 0x800) subglobal_y -= 0x1000;
			global_y=subglobal_y;
			global_x=subglobal_x;
		}

		/* A real sprite to process! */
		sprite = (spriteram16_ptr[current_offs+0+0]) | ((spriteram16_ptr[current_offs+4+1]&1)<<16);
		spritecont = spriteram16_ptr[current_offs+4+0]>>8;

/* These games either don't set the XY control bits properly (68020 bug?), or
    have some different mode from the others */
#ifdef DARIUSG_KLUDGE
		if (state->m_f3_game==DARIUSG || state->m_f3_game==GEKIRIDO || state->m_f3_game==CLEOPATR || state->m_f3_game==RECALH)
			multi=spritecont&0xf0;
#endif

		/* Check if this sprite is part of a continued block */
		if (multi) {
			/* Bit 0x4 is 'use previous colour' for this block part */
			if (spritecont&0x4) color=last_color;
			else color=(spriteram16_ptr[current_offs+4+0])&0xff;

#ifdef DARIUSG_KLUDGE
			if (state->m_f3_game==DARIUSG || state->m_f3_game==GEKIRIDO || state->m_f3_game==CLEOPATR || state->m_f3_game==RECALH) {
				/* Adjust X Position */
				if ((spritecont & 0x40) == 0) {
					if (spritecont & 0x4) {
						x = block_x;
					} else {
						this_x = spriteram16_ptr[current_offs+2+0];
						if (this_x&0x800) this_x= 0 - (0x800 - (this_x&0x7ff)); else this_x&=0x7ff;

						if ((spriteram16_ptr[current_offs+2+0])&0x8000) {
							this_x+=0;
						} else if ((spriteram16_ptr[current_offs+2+0])&0x4000) {
							/* Ignore subglobal (but apply global) */
							this_x+=global_x;
						} else { /* Apply both scroll offsets */
							this_x+=global_x+subglobal_x;
						}

						x = block_x = this_x;
					}
					x_addition_left = 8;
					CALC_ZOOM(x)
				}
				else if ((spritecont & 0x80) != 0) {
					x = last_x+x_addition;
					CALC_ZOOM(x)
				}

				/* Adjust Y Position */
				if ((spritecont & 0x10) == 0) {
					if (spritecont & 0x4) {
						y = block_y;
					} else {
						this_y = spriteram16_ptr[current_offs+2+1]&0xffff;
						if (this_y&0x800) this_y= 0 - (0x800 - (this_y&0x7ff)); else this_y&=0x7ff;

						if ((spriteram16_ptr[current_offs+2+0])&0x8000) {
							this_y+=0;
						} else if ((spriteram16_ptr[current_offs+2+0])&0x4000) {
							/* Ignore subglobal (but apply global) */
							this_y+=global_y;
						} else { /* Apply both scroll offsets */
							this_y+=global_y+subglobal_y;
						}

						y = block_y = this_y;
					}
					y_addition_left = 8;
					CALC_ZOOM(y)
				}
				else if ((spritecont & 0x20) != 0) {
					y = last_y+y_addition;
					CALC_ZOOM(y)
				}
			} else
#endif
			{
				/* Adjust X Position */
				if ((spritecont & 0x40) == 0) {
					x = block_x;
					x_addition_left = 8;
					CALC_ZOOM(x)
				}
				else if ((spritecont & 0x80) != 0) {
					x = last_x+x_addition;
					CALC_ZOOM(x)
				}
				/* Adjust Y Position */
				if ((spritecont & 0x10) == 0) {
					y = block_y;
					y_addition_left = 8;
					CALC_ZOOM(y)
				}
				else if ((spritecont & 0x20) != 0) {
					y = last_y+y_addition;
					CALC_ZOOM(y)
				}
				/* Both zero = reread block latch? */
			}
		}
		/* Else this sprite is the possible start of a block */
		else {
			color = (spriteram16_ptr[current_offs+4+0])&0xff;
			last_color=color;

			/* Sprite positioning */
			this_y = spriteram16_ptr[current_offs+2+1]&0xffff;
			this_x = spriteram16_ptr[current_offs+2+0]&0xffff;
			if (this_y&0x800) this_y= 0 - (0x800 - (this_y&0x7ff)); else this_y&=0x7ff;
			if (this_x&0x800) this_x= 0 - (0x800 - (this_x&0x7ff)); else this_x&=0x7ff;

			/* Ignore both scroll offsets for this block */
			if ((spriteram16_ptr[current_offs+2+0])&0x8000) {
				this_x+=0;
				this_y+=0;
			} else if ((spriteram16_ptr[current_offs+2+0])&0x4000) {
				/* Ignore subglobal (but apply global) */
				this_x+=global_x;
				this_y+=global_y;
			} else { /* Apply both scroll offsets */
				this_x+=global_x+subglobal_x;
				this_y+=global_y+subglobal_y;
			}

	        block_y = y = this_y;
            block_x = x = this_x;

			block_zoom_x=spriteram16_ptr[current_offs+0+1];
			block_zoom_y=(block_zoom_x>>8)&0xff;
			block_zoom_x&=0xff;

			x_addition_left = 8;
			CALC_ZOOM(x)

			y_addition_left = 8;
			CALC_ZOOM(y)
		}

		/* These features are common to sprite and block parts */
		flipx = spritecont&0x1;
		flipy = spritecont&0x2;
		multi = spritecont&0x8;
		last_x=x;
		last_y=y;

		if (!sprite) continue;
		if (!x_addition || !y_addition) continue;

		if (state->m_flipscreen)
		{
			int tx,ty;

			tx = 512-x_addition-x;
			ty = 256-y_addition-y;

			if (tx+x_addition<=min_x || tx>max_x || ty+y_addition<=min_y || ty>max_y) continue;
			sprite_ptr->x = tx;
			sprite_ptr->y = ty;
			sprite_ptr->flipx = !flipx;
			sprite_ptr->flipy = !flipy;
		}
		else
		{
			if (x+x_addition<=min_x || x>max_x || y+y_addition<=min_y || y>max_y) continue;
			sprite_ptr->x = x;
			sprite_ptr->y = y;
			sprite_ptr->flipx = flipx;
			sprite_ptr->flipy = flipy;
		}


		sprite_ptr->code = sprite;
		sprite_ptr->color = color;
		sprite_ptr->zoomx = x_addition;
		sprite_ptr->zoomy = y_addition;
		sprite_ptr->pri = (color & 0xc0) >> 6;
		sprite_ptr++;
		total_sprites++;
	}
	state->m_sprite_end = sprite_ptr;
}
#undef CALC_ZOOM


static void draw_sprites(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	taito_f3_state *state = machine.driver_data<taito_f3_state>();
	const struct tempsprite *sprite_ptr;
	const gfx_element *sprite_gfx = machine.gfx[2];

	sprite_ptr = state->m_sprite_end;
	state->m_sprite_pri_usage=0;

	// if sprites use more than 4bpp, the bottom bits of the color code must be masked out.
	// This fixes (at least) stage 1 battle ships and attract mode explosions in Ray Force.

	while (sprite_ptr != state->m_spritelist)
	{
		int pri;
		sprite_ptr--;

		pri=sprite_ptr->pri;
		state->m_sprite_pri_usage|=1<<pri;

		if(sprite_ptr->zoomx==16 && sprite_ptr->zoomy==16)
			f3_drawgfx(
					bitmap,cliprect,sprite_gfx,
					sprite_ptr->code,
					sprite_ptr->color & (~state->m_sprite_extra_planes),
					sprite_ptr->flipx,sprite_ptr->flipy,
					sprite_ptr->x,sprite_ptr->y,
					pri);
		else
			f3_drawgfxzoom(
					bitmap,cliprect,sprite_gfx,
					sprite_ptr->code,
					sprite_ptr->color & (~state->m_sprite_extra_planes),
					sprite_ptr->flipx,sprite_ptr->flipy,
					sprite_ptr->x,sprite_ptr->y,
					sprite_ptr->zoomx,sprite_ptr->zoomy,
					pri);
	}
}

/******************************************************************************/

SCREEN_UPDATE_RGB32( f3 )
{
	taito_f3_state *state = screen.machine().driver_data<taito_f3_state>();
	UINT32 sy_fix[5],sx_fix[5];

	state->m_f3_skip_this_frame=0;
	screen.machine().tilemap().set_flip_all(state->m_flipscreen ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	/* Setup scroll */
	sy_fix[0]=((state->m_f3_control_0[4]&0xffff)<< 9) + (1<<16);
	sy_fix[1]=((state->m_f3_control_0[5]&0xffff)<< 9) + (1<<16);
	sy_fix[2]=((state->m_f3_control_0[6]&0xffff)<< 9) + (1<<16);
	sy_fix[3]=((state->m_f3_control_0[7]&0xffff)<< 9) + (1<<16);
	sx_fix[0]=((state->m_f3_control_0[0]&0xffc0)<<10) - (6<<16);
	sx_fix[1]=((state->m_f3_control_0[1]&0xffc0)<<10) - (10<<16);
	sx_fix[2]=((state->m_f3_control_0[2]&0xffc0)<<10) - (14<<16);
	sx_fix[3]=((state->m_f3_control_0[3]&0xffc0)<<10) - (18<<16);
	sx_fix[4]=-(state->m_f3_control_1[4])+41;
	sy_fix[4]=-(state->m_f3_control_1[5]&0x1ff);

	sx_fix[0]-=((state->m_f3_control_0[0]&0x003f)<<10)+0x0400-0x10000;
	sx_fix[1]-=((state->m_f3_control_0[1]&0x003f)<<10)+0x0400-0x10000;
	sx_fix[2]-=((state->m_f3_control_0[2]&0x003f)<<10)+0x0400-0x10000;
	sx_fix[3]-=((state->m_f3_control_0[3]&0x003f)<<10)+0x0400-0x10000;

	if (state->m_flipscreen)
	{
		sy_fix[0]= 0x3000000-sy_fix[0];
		sy_fix[1]= 0x3000000-sy_fix[1];
		sy_fix[2]= 0x3000000-sy_fix[2];
		sy_fix[3]= 0x3000000-sy_fix[3];
		sx_fix[0]=-0x1a00000-sx_fix[0];
		sx_fix[1]=-0x1a00000-sx_fix[1];
		sx_fix[2]=-0x1a00000-sx_fix[2];
		sx_fix[3]=-0x1a00000-sx_fix[3];
		sx_fix[4]=-sx_fix[4] + 75;
		sy_fix[4]=-sy_fix[4];
	}

	state->m_pri_alp_bitmap.fill(0, cliprect);

	/* sprites */
	if (state->m_sprite_lag==0)
		get_sprite_info(screen.machine(), state->m_spriteram);

	/* Update sprite buffer */
	draw_sprites(screen.machine(), bitmap,cliprect);

	/* Parse sprite, alpha & clipping parts of lineram */
	get_spritealphaclip_info(state);

	/* Parse playfield effects */
	get_line_ram_info(screen.machine(), state->m_pf1_tilemap,sx_fix[0],sy_fix[0],0,state->m_f3_pf_data_1);
	get_line_ram_info(screen.machine(), state->m_pf2_tilemap,sx_fix[1],sy_fix[1],1,state->m_f3_pf_data_2);
	get_line_ram_info(screen.machine(), state->m_pf3_tilemap,sx_fix[2],sy_fix[2],2,state->m_f3_pf_data_3);
	get_line_ram_info(screen.machine(), state->m_pf4_tilemap,sx_fix[3],sy_fix[3],3,state->m_f3_pf_data_4);
	get_vram_info(screen.machine(), state->m_vram_layer,state->m_pixel_layer,sx_fix[4],sy_fix[4]);

	/* Draw final framebuffer */
	scanline_draw(screen.machine(), bitmap,cliprect);

	if (VERBOSE)
		print_debug_info(screen.machine(), bitmap);
	return 0;
}
