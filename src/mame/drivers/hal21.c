/*
    Hal21
    ASO
    Alpha Mission


Change Log
----------

AT08XX03:

[Common]
 - cleaned and consolidated VIDEO_UPDATE()
 - added shadows and highlights

 * A.S.O and HAL21 do a lot of palette cycling therefore
   conversion to tilemaps may be disadvantageous.
   Cocktail mode involves changing tile offsets and sprite
   coordinates and is still unsupported.

 * Manuals show both boards have noise filters to smooth out
   rings and scratches which are especially audible in HAL21.

[HAL21]
 - installed NMI scheduler to prevent music trashing

[ASO]
 - fixed music and sound effects being cut short
 - fixed service mode(hold P1 start during ROM test)
 - improved scrolling and color

 * Stage 5 boss' sky and the first half of stage 6's background
   appear to have consistent color as shown in Beep! magazine:

     http://qtq.hp.infoseek.co.jp/kouryaku/aso/aso2.jpg
     http://qtq.hp.infoseek.co.jp/kouryaku/aso/aso3.png

   Compared to MAME these areas are blacked out under pens
   0xf0-0xff. On the other hand pens 0x170-0x17f suit them
   perfectly but they are never used in the first two loops.
   (I played through the game and logged pen usage. Only four
   color codes have blue pen15 so it's not difficult to tell.)

   There are unknown bits embedded in RGB triplets and the whole
   upper half of the palette is simply unused. The fact that ASO's
   color PROMs are identical in every set dismissed bad dumps but
   increased the likelyhood of proprietary logic which is quite
   obvious in Touchdown Fever and HAL21.

[TODO]
 - find out what "really" messes up ASO's scrolling
 - verify color effects in both games
*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "snk.h"
#include "sound/ay8910.h"
#include "sound/3812intf.h"

static UINT8 *hal21_vreg, *hal21_sndfifo;
static UINT8 *textram;
static UINT8 *aso_scroll_sync;

/**************************************************************************/
// Test Handlers

static WRITE8_HANDLER( aso_scroll_sync_w )
{
	if (data == 0x7f && (program_read_byte(0xdcd2) & 1)) data++;
	aso_scroll_sync[offset] = data;
}

static void hal21_sound_scheduler(int mode, int data)
{
	static int busy, hold, ffcount, ffhead, fftail;

	switch (mode)
	{
		case 0: // init
			fftail = ffhead = ffcount = hold = busy = 0;
		return;

		case 1: // cut-through or capture
			if (data & ~0x1f) busy = 1; else
			if (data && busy)
			{
				if (ffcount < 16)
				{
					ffcount++;
					hal21_sndfifo[ffhead] = data;
					ffhead = (ffhead + 1) & 15;
				}
				return;
			}
		break;

		case 2: // acknowledge
			if (busy) { busy = 0; hold = 4; }
		return;

		case 3: // release
			if (!busy)
			{
				if (hold) hold--; else
				if (ffcount)
				{
					ffcount--;
					data = hal21_sndfifo[fftail];
					fftail = (fftail + 1) & 15;
					break;
				}
			}
		return;
	}

	snk_sound_busy_bit = 0x20;
	soundlatch_w(0, data);
	cpunum_set_input_line(Machine, 2, INPUT_LINE_NMI, PULSE_LINE);
}

/**************************************************************************/

static WRITE8_HANDLER( hal21_vreg0_w ){ hal21_vreg[0] = data; }
static WRITE8_HANDLER( hal21_vreg1_w ){ hal21_vreg[1] = data; }
static WRITE8_HANDLER( hal21_vreg2_w ){ hal21_vreg[2] = data; }
static WRITE8_HANDLER( hal21_vreg3_w ){ hal21_vreg[3] = data; }
static WRITE8_HANDLER( hal21_vreg4_w ){ hal21_vreg[4] = data; }
static WRITE8_HANDLER( hal21_vreg5_w ){ hal21_vreg[5] = data; }
static WRITE8_HANDLER( hal21_vreg6_w ){ hal21_vreg[6] = data; }
static WRITE8_HANDLER( hal21_vreg7_w ){ hal21_vreg[7] = data; }


PALETTE_INIT( aso )
{
	int i;
	int num_colors = 1024;

	/*
        palette format is RRRG GGBB B??? the three unknown bits are used but
        I'm not sure how, I'm currently using them as least significant bit but
        that's most likely wrong.
    */
	for( i=0; i<num_colors; i++ )
	{
		int bit0=0,bit1,bit2,bit3,r,g,b;

		bit0 = (color_prom[i + 2*num_colors] >> 2) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 2*num_colors] >> 1) & 0x01;
		bit1 = (color_prom[i + num_colors] >> 2) & 0x01;
		bit2 = (color_prom[i + num_colors] >> 3) & 0x01;
		bit3 = (color_prom[i] >> 0) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (color_prom[i + 2*num_colors] >> 0) & 0x01;
		bit1 = (color_prom[i + 2*num_colors] >> 3) & 0x01;
		bit2 = (color_prom[i + num_colors] >> 0) & 0x01;
		bit3 = (color_prom[i + num_colors] >> 1) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette_set_color(machine,i,MAKE_RGB(r,g,b));
	}

	/* prepare shadow draw table */
	for (i=0; i<=5; i++) gfx_drawmode_table[i] = DRAWMODE_SOURCE;

	gfx_drawmode_table[6] = DRAWMODE_SHADOW;
	gfx_drawmode_table[7] = DRAWMODE_NONE;
}

static VIDEO_START( aso )
{
	snk_blink_parity = 0;
}


static void hal21_draw_background(running_machine *machine, mame_bitmap *bitmap, const rectangle *cliprect, int scrollx, int scrolly, int attrs,
								const gfx_element *gfx )
{
	static int color[2] = {8, 8};
	int bankbase, c, x, y, offsx, offsy, dx, dy, sx, sy, offs, tile_number;

	bankbase = attrs<<3 & 0x100;
	c = attrs & 0x0f;
	if (c > 11) { fillbitmap(bitmap,machine->pens[(c<<4)+8], cliprect); return; }
	if (c<8 || color[0]<14 || bankbase)
	{
		c ^= 0x08;
		color[0] = c;
		color[1] = (c & 0x08) ? c : 8;
	}

	offsx = ((scrollx>>3) + 0) & 0x3f;
	dx = -(scrollx & 7) + 0;
	offsy = ((scrolly>>3) + 0) & 0x3f;
	dy = -(scrolly & 7) + 0;

	for (x=2; x<35; x++)
		for (y=0; y<28; y++)
		{
			offs = (((offsx+x)&0x3f)<<6) + ((offsy+y)&0x3f);
			sx = (x<<3) + dx;
			sy = (y<<3) + dy;
			tile_number = bankbase + videoram[offs];
			c = (tile_number & ~0x3f) ? color[0] : color[1];

			drawgfx(bitmap, gfx,
				tile_number, c,
				0, 0,
				sx, sy,
				cliprect, TRANSPARENCY_NONE, 0);
		}
}

static void hal21_draw_sprites(mame_bitmap *bitmap, const rectangle *cliprect, int scrollx, int scrolly,
								const gfx_element *gfx )
{
	UINT8 *sprptr, *endptr;
	int attrs, tile, x, y, color, fy;

	sprptr = spriteram;
	endptr = spriteram + 0x100;

	for (; sprptr<endptr; sprptr+=4)
	{
		if (*(UINT32*)sprptr == 0 || *(UINT32*)sprptr == -1) continue;

		attrs = sprptr[3];
		tile  = sprptr[1] + (attrs<<2 & 0x100);
		color = attrs & 0x0f;
		fy    = attrs & 0x20;
		y     = (sprptr[0] + (attrs<<4 & 0x100) - scrolly) & 0x1ff;
		x     = (0x100 - (sprptr[2] + (attrs<<1 & 0x100) - scrollx)) & 0x1ff;
		if (y > 512-16) y -= 512;
		if (x > 512-16) x -= 512;

		drawgfx(bitmap, gfx,
				tile, color,
				0, fy,
				x, y,
				cliprect, TRANSPARENCY_PEN, 7);
	}
}

static void aso_draw_background(mame_bitmap *bitmap, const rectangle *cliprect, int scrollx, int scrolly, int attrs,
								const gfx_element *gfx )
{
	int bankbase, c, x, y, offsx, offsy, dx, dy, sx, sy, offs, tile_number;

	bankbase = attrs<<4 & 0x300;
	c = attrs & 0x0f;
	if (c == 7) c = 15;

	offsx = ((scrollx>>3) + 0) & 0x3f;
	dx = -(scrollx & 7) + 0;
	offsy = ((scrolly>>3) + 0) & 0x3f;
	dy = -(scrolly & 7) + 0;

	for (x=2; x<35; x++)
		for (y=0; y<28; y++)
		{
			offs = (((offsx+x)&0x3f)<<6) + ((offsy+y)&0x3f);
			sx = (x<<3) + dx;
			sy = (y<<3) + dy;
			tile_number = bankbase + videoram[offs];

			drawgfx(bitmap, gfx,
				tile_number, c,
				0, 0,
				sx, sy,
				cliprect, TRANSPARENCY_NONE, 0);
		}
}

static void aso_draw_sprites(mame_bitmap *bitmap, const rectangle *cliprect, int scrollx, int scrolly,
								const gfx_element *gfx )
{
	UINT8 *sprptr, *endptr;
	int attrs, tile, x, y, color;

	sprptr = spriteram;
	endptr = spriteram + 0x100;

	for (; sprptr<endptr; sprptr+=4)
	{
		if (*(UINT32*)sprptr == 0 || *(UINT32*)sprptr == -1) continue;

		attrs = sprptr[3]; /* YBBX.CCCC */
		tile  = sprptr[1] + (attrs<<2 & 0x100) + (~attrs<<4 & 0x200);
		color = attrs & 0x0f;
		y     = (sprptr[0] + (attrs<<4 & 0x100) - scrolly) & 0x1ff;
		x     = (0x100 - (sprptr[2] + (attrs<<1 & 0x100) - scrollx)) & 0x1ff;
		if (y > 512-16) y -= 512;
		if (x > 512-16) x -= 512;

		drawgfx(bitmap, gfx,
				tile, color,
				0, 0,
				x, y,
				cliprect, TRANSPARENCY_PEN_TABLE, 7);
	}
}

static VIDEO_UPDATE( aso )
{
	int attr, msbs, spsy, spsx, bgsy, bgsx, bank, i;

	attr = (int)hal21_vreg[0];
	msbs = (int)hal21_vreg[1];
	spsy = (int)hal21_vreg[2] + (msbs<<5 & 0x100) + 9;
	spsx = (int)hal21_vreg[3] + (msbs<<8 & 0x100) + 30;
	bgsy = (int)hal21_vreg[4] + (msbs<<4 & 0x100) - 8;
	bgsx = (int)hal21_vreg[5] - 16;

	if (snk_gamegroup)
	{
		hal21_draw_background(machine, bitmap, cliprect, bgsx+(msbs<<7 & 0x100), bgsy, attr, machine->gfx[1]);

		attr = snk_blink_parity;
		snk_blink_parity ^= 0xdf;
		for (i=6; i<0x80; i+=8) { palette_set_color(machine, i, MAKE_RGB(attr, attr, attr)); }

		hal21_draw_sprites(bitmap, cliprect, spsx, spsy, machine->gfx[2]);
	}
	else
	{
		aso_draw_background(bitmap, cliprect, bgsx+(~msbs<<7 & 0x100), bgsy, attr, machine->gfx[1]);
		aso_draw_sprites(bitmap, cliprect, spsx, spsy, machine->gfx[2]);
	}

	bank = msbs>>6 & 1;
	tnk3_draw_text(machine, bitmap, cliprect, bank, &textram[0]);
	tnk3_draw_status(machine, bitmap, cliprect, bank, &textram[0x400]);
	return 0;
}


static INPUT_PORTS_START( hal21 )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* sound CPU status */
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN1") /* P1 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN2") /* P2 controls */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0xc0, "20000 60000" )
	PORT_DIPSETTING(    0x80, "40000 90000" )
	PORT_DIPSETTING(    0x40, "50000 120000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Bonus Type" )
	PORT_DIPSETTING(    0x01, "Every Bonus Set" )
	PORT_DIPSETTING(    0x00, "Second Bonus Set" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x18, 0x18, "Special" )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Demo_Sounds) )
	PORT_DIPSETTING(    0x08, "Infinite Lives" )
	PORT_DIPSETTING(    0x00, "Freeze" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) // 0x20 -> fe65
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/**************************************************************************/

static INPUT_PORTS_START( aso )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* sound CPU status */
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0xc0, "50k 100k" )
	PORT_DIPSETTING(    0x80, "60k 120k" )
	PORT_DIPSETTING(    0x40, "100k 200k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Bonus Occurrence" )
	PORT_DIPSETTING(    0x01, "1st & every 2nd" )
	PORT_DIPSETTING(    0x00, "1st & 2nd only" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Effect of some kind (Cheat)")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Start Area" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END


/**************************************************************************/

static const gfx_layout char256 = {
	8,8,
	0x100,
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	256
};

static const gfx_layout char1024 = {
	8,8,
	0x400,
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	256
};

static const gfx_layout sprite1024 = {
	16,16,
	0x400,
	3,
	{ 2*1024*256,1*1024*256,0*1024*256 },
	{
		7,6,5,4,3,2,1,0,
		15,14,13,12,11,10,9,8
	},
	{
		0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16
	},
	256
};

static GFXDECODE_START( aso )
	/* colors 512-1023 are currently unused, I think they are a second bank */
	GFXDECODE_ENTRY( REGION_GFX1, 0, char256,    128*3,  8 ) /* colors 384..511 */
	GFXDECODE_ENTRY( REGION_GFX2, 0, char1024,   128*1, 16 ) /* colors 128..383 */
	GFXDECODE_ENTRY( REGION_GFX3, 0, sprite1024, 128*0, 16 ) /* colors   0..127 */
GFXDECODE_END

/**************************************************************************/

static READ8_HANDLER( CPUC_ready_r ) { snk_sound_busy_bit = 0; return 0; }

static READ8_HANDLER( hal21_input_port_0_r ) { return input_port_0_r(0) | snk_sound_busy_bit; }

static WRITE8_HANDLER( hal21_soundcommand_w ) { hal21_sound_scheduler(1, data); }
static WRITE8_HANDLER( hal21_soundack_w ) { hal21_sound_scheduler(2, data); }

static READ8_HANDLER( hal21_soundcommand_r )
{
	int data = soundlatch_r(0);
	soundlatch_clear_w(0, 0);
	return data;
}

static WRITE8_HANDLER( aso_soundcommand_w )
{
	snk_sound_busy_bit = 0x20;
	soundlatch_w(0, data);
	cpunum_set_input_line(Machine, 2, 0, HOLD_LINE );
}

static INTERRUPT_GEN( hal21_sound_interrupt )
{
	hal21_sound_scheduler(3, 0);
}

/**************************************************************************/

static ADDRESS_MAP_START( aso_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd000) AM_READ(hal21_soundcommand_r)
	AM_RANGE(0xe000, 0xe000) AM_READ(CPUC_ready_r)
	AM_RANGE(0xf000, 0xf000) AM_READWRITE(YM3526_status_port_0_r, YM3526_control_port_0_w) /* YM3526 #1 control port? */
	AM_RANGE(0xf001, 0xf001) AM_WRITE(YM3526_write_port_0_w)   /* YM3526 #1 write port?  */
	AM_RANGE(0xf002, 0xf002) AM_READNOP // unknown read
	AM_RANGE(0xf004, 0xf004) AM_READNOP // unknown read
	AM_RANGE(0xf006, 0xf006) AM_READNOP // unknown read
ADDRESS_MAP_END

/**************************************************************************/

static ADDRESS_MAP_START( hal21_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(hal21_soundcommand_r)
	AM_RANGE(0xc000, 0xc000) AM_READ(CPUC_ready_r)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0xe001, 0xe001) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0xe002, 0xe002) AM_WRITE(hal21_soundack_w) // bitfielded(0-5) acknowledge write, details unknown
	AM_RANGE(0xe008, 0xe008) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0xe009, 0xe009) AM_WRITE(AY8910_write_port_1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hal21_sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_FLAGS( AMEF_ABITS(8) )
	AM_RANGE(0x0000, 0x0000) AM_NOP				// external sound ROM detection?
ADDRESS_MAP_END

/**************************** ASO/Alpha Mission *************************/

static ADDRESS_MAP_START( aso_cpuA_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ(hal21_input_port_0_r) /* coin, start */
	AM_RANGE(0xc100, 0xc100) AM_READ(input_port_1_r) /* P1 */
	AM_RANGE(0xc200, 0xc200) AM_READ(input_port_2_r) /* P2 */
	AM_RANGE(0xc400, 0xc400) AM_WRITE(aso_soundcommand_w)
	AM_RANGE(0xc500, 0xc500) AM_READ(input_port_3_r) /* DSW1 */
	AM_RANGE(0xc600, 0xc600) AM_READ(input_port_4_r) /* DSW2 */
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(hal21_vreg1_w)
	AM_RANGE(0xc900, 0xc900) AM_WRITE(hal21_vreg2_w)
	AM_RANGE(0xca00, 0xca00) AM_WRITE(hal21_vreg3_w)
	AM_RANGE(0xcb00, 0xcb00) AM_WRITE(hal21_vreg4_w)
	AM_RANGE(0xcc00, 0xcc00) AM_WRITE(hal21_vreg5_w)
	AM_RANGE(0xcd00, 0xcd00) AM_WRITE(hal21_vreg6_w)
	AM_RANGE(0xce00, 0xce00) AM_WRITE(hal21_vreg7_w)
	AM_RANGE(0xcf00, 0xcf00) AM_WRITE(hal21_vreg0_w)
	AM_RANGE(0xdcf8, 0xdcf8) AM_WRITE(aso_scroll_sync_w) AM_BASE(&aso_scroll_sync)
	AM_RANGE(0xd800, 0xe7ff) AM_RAM AM_SHARE(2)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_BASE(&spriteram)
	AM_RANGE(0xe800, 0xf7ff) AM_RAM AM_SHARE(3) AM_BASE(&videoram)
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE(1) AM_BASE(&textram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( aso_cpuB_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(snk_cpuA_nmi_trigger_r, snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc800, 0xd7ff) AM_RAM AM_SHARE(2)
	AM_RANGE(0xd800, 0xe7ff) AM_RAM AM_SHARE(3)
	AM_RANGE(0xe800, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE(1)
ADDRESS_MAP_END

/**************************** HAL21 *************************/

static ADDRESS_MAP_START( hal21_cpuA_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ(hal21_input_port_0_r) /* coin, start */
	AM_RANGE(0xc100, 0xc100) AM_READ(input_port_1_r) /* P1 */
	AM_RANGE(0xc200, 0xc200) AM_READ(input_port_2_r) /* P2 */
	AM_RANGE(0xc300, 0xc300) AM_WRITE(hal21_soundcommand_w)
	AM_RANGE(0xc400, 0xc400) AM_READ(input_port_3_r) /* DSW1 */
	AM_RANGE(0xc500, 0xc500) AM_READ(input_port_4_r) /* DSW2 */
	AM_RANGE(0xc600, 0xc600) AM_WRITE(hal21_vreg0_w)
	AM_RANGE(0xc700, 0xc700) AM_READWRITE(snk_cpuB_nmi_trigger_r, snk_cpuA_nmi_ack_w)
	AM_RANGE(0xd300, 0xd300) AM_WRITE(hal21_vreg1_w)
	AM_RANGE(0xd400, 0xd400) AM_WRITE(hal21_vreg2_w)
	AM_RANGE(0xd500, 0xd500) AM_WRITE(hal21_vreg3_w)
	AM_RANGE(0xd600, 0xd600) AM_WRITE(hal21_vreg4_w)
	AM_RANGE(0xd700, 0xd700) AM_WRITE(hal21_vreg5_w)
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE(2) AM_BASE(&spriteram)
	AM_RANGE(0xf000, 0xffff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_BASE(&textram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hal21_cpuB_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xa000) AM_WRITE(snk_cpuB_nmi_ack_w)
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_SHARE(2)
	AM_RANGE(0xd000, 0xdfff) AM_RAM AM_BASE(&videoram)
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE(1)
ADDRESS_MAP_END

/**************************************************************************/

static DRIVER_INIT( aso )
{
	hal21_vreg = auto_malloc(16);
	snk_gamegroup = 0;
}

static DRIVER_INIT( hal21 )
{
	hal21_vreg = auto_malloc(24);
	hal21_sndfifo = hal21_vreg + 8;
	snk_gamegroup = 1;
}

static MACHINE_RESET( aso )
{
	memset(hal21_vreg, 0, 8);
	hal21_sound_scheduler(0, 0);
	snk_sound_busy_bit = 0;
}

static MACHINE_DRIVER_START( aso )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(aso_cpuA_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(aso_cpuB_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(Z80, 4000000)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(aso_sound_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(100)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_HAS_SHADOWS)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(36*8, 28*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 1*8, 28*8-1)
	MDRV_GFXDECODE(aso)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_MACHINE_RESET(aso)

	MDRV_PALETTE_INIT(aso)
	MDRV_VIDEO_START(aso)
	MDRV_VIDEO_UPDATE(aso)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(YM3526, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( hal21 )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(hal21_cpuA_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(hal21_cpuB_map,0)
	MDRV_CPU_VBLANK_INT(irq0_line_hold,1)

	MDRV_CPU_ADD(Z80, 4000000)
	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(hal21_sound_map,0)
	MDRV_CPU_IO_MAP(hal21_sound_portmap,0)
	MDRV_CPU_VBLANK_INT(hal21_sound_interrupt,1)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, 220) // music tempo, hand tuned

	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)
	MDRV_INTERLEAVE(100)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_HAS_HIGHLIGHTS)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(36*8, 28*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 1*8, 28*8-1)
	MDRV_GFXDECODE(aso)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_MACHINE_RESET(aso)

	MDRV_PALETTE_INIT(aso)
	MDRV_VIDEO_START(aso)
	MDRV_VIDEO_UPDATE(aso)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END

/**************************************************************************/

ROM_START( hal21 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )   /* 64k for CPUA code */
	ROM_LOAD( "hal21p1.bin",    0x0000, 0x2000, CRC(9d193830) SHA1(8e4e9c8bc774d7c7c0b68a5fa5cabdc6b5cfa41b) )
	ROM_LOAD( "hal21p2.bin",    0x2000, 0x2000, CRC(c1f00350) SHA1(8709455a980931565ccca60162a04c6c3133099b) )
	ROM_LOAD( "hal21p3.bin",    0x4000, 0x2000, CRC(881d22a6) SHA1(4b2a65dc18620f7f77532f791212fccfe1f0b245) )
	ROM_LOAD( "hal21p4.bin",    0x6000, 0x2000, CRC(ce692534) SHA1(e1d8e6948578ec9d0b6dc2aff17ad23b8ce46d6a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )   /* 64k for CPUB code */
	ROM_LOAD( "hal21p5.bin",    0x0000, 0x2000, CRC(3ce0684a) SHA1(5e76770a3252d5565a8f11a79ac3a9a6c31a43e2) )
	ROM_LOAD( "hal21p6.bin",    0x2000, 0x2000, CRC(878ef798) SHA1(0aae152947c9c6733b77dd1ac14f2f6d6bfabeaa) )
	ROM_LOAD( "hal21p7.bin",    0x4000, 0x2000, CRC(72ebbe95) SHA1(b1f7dc535e7670647500391d21dfa971d5e342a2) )
	ROM_LOAD( "hal21p8.bin",    0x6000, 0x2000, CRC(17e22ad3) SHA1(0e10a3c0f2e2ec284f4e0f1055397a8ccd1ff0f7) )
	ROM_LOAD( "hal21p9.bin",    0x8000, 0x2000, CRC(b146f891) SHA1(0b2db3e14b0401a7914002c6f7c26933a1cba162) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )   /* 64k for sound code */
	ROM_LOAD( "hal21p10.bin",   0x0000, 0x4000, CRC(916f7ba0) SHA1(7b8bcd59d768c4cd226de96895d3b9755bb3ba79) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "hal21p12.bin", 0x0000, 0x2000, CRC(9839a7cd) SHA1(d3f9d964263a64aa3648faf5eb2e4fa532ae7852) ) /* char */

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE ) /* background tiles */
	ROM_LOAD( "hal21p11.bin", 0x0000, 0x4000, CRC(24abc57e) SHA1(1d7557a62adc059fb3fe20a09be18c2f40441581) )

	ROM_REGION( 0x18000, REGION_GFX3, ROMREGION_DISPOSE ) /* 16x16 sprites */
	ROM_LOAD( "hal21p13.bin", 0x00000, 0x4000, CRC(052b4f4f) SHA1(032eb5771d33defce86e222f3e7aa22bc37db6db) )
	ROM_RELOAD(               0x04000, 0x4000 )
	ROM_LOAD( "hal21p14.bin", 0x08000, 0x4000, CRC(da0cb670) SHA1(1083bdd3488dfaa5094a2ef52cfc4206f35c9612) )
	ROM_RELOAD(               0x0c000, 0x4000 )
	ROM_LOAD( "hal21p15.bin", 0x10000, 0x4000, CRC(5c5ea945) SHA1(f9ce206cab4fad1f6478d731d4b096ec33e7b99f) )
	ROM_RELOAD(               0x14000, 0x4000 )

	ROM_REGION( 0x0c00, REGION_PROMS, 0 )
	ROM_LOAD( "hal21_3.prm",  0x000, 0x400, CRC(605afff8) SHA1(94e80ebd574b1580dac4a2aebd57e3e767890c0d) )
	ROM_LOAD( "hal21_2.prm",  0x400, 0x400, CRC(c5d84225) SHA1(cc2cd32f81ed7c1bcdd68e91d00f8081cb706ce7) )
	ROM_LOAD( "hal21_1.prm",  0x800, 0x400, CRC(195768fc) SHA1(c88bc9552d57d52fb4b030d118f48fedccf563f4) )
ROM_END

ROM_START( hal21j )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )   /* 64k for CPUA code */
	ROM_LOAD( "hal21p1.bin",    0x0000, 0x2000, CRC(9d193830) SHA1(8e4e9c8bc774d7c7c0b68a5fa5cabdc6b5cfa41b) )
	ROM_LOAD( "hal21p2.bin",    0x2000, 0x2000, CRC(c1f00350) SHA1(8709455a980931565ccca60162a04c6c3133099b) )
	ROM_LOAD( "hal21p3.bin",    0x4000, 0x2000, CRC(881d22a6) SHA1(4b2a65dc18620f7f77532f791212fccfe1f0b245) )
	ROM_LOAD( "hal21p4.bin",    0x6000, 0x2000, CRC(ce692534) SHA1(e1d8e6948578ec9d0b6dc2aff17ad23b8ce46d6a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )   /* 64k for CPUB code */
	ROM_LOAD( "hal21p5.bin",    0x0000, 0x2000, CRC(3ce0684a) SHA1(5e76770a3252d5565a8f11a79ac3a9a6c31a43e2) )
	ROM_LOAD( "hal21p6.bin",    0x2000, 0x2000, CRC(878ef798) SHA1(0aae152947c9c6733b77dd1ac14f2f6d6bfabeaa) )
	ROM_LOAD( "hal21p7.bin",    0x4000, 0x2000, CRC(72ebbe95) SHA1(b1f7dc535e7670647500391d21dfa971d5e342a2) )
	ROM_LOAD( "hal21p8.bin",    0x6000, 0x2000, CRC(17e22ad3) SHA1(0e10a3c0f2e2ec284f4e0f1055397a8ccd1ff0f7) )
	ROM_LOAD( "hal21p9.bin",    0x8000, 0x2000, CRC(b146f891) SHA1(0b2db3e14b0401a7914002c6f7c26933a1cba162) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )   /* 64k for sound code */
	ROM_LOAD( "hal21-10.bin",   0x0000, 0x4000, CRC(a182b3f0) SHA1(b76eff97a58a96467e9f3a74125a0a770e7678f8) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "hal21p12.bin", 0x0000, 0x2000, CRC(9839a7cd) SHA1(d3f9d964263a64aa3648faf5eb2e4fa532ae7852) ) /* char */

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE ) /* background tiles */
	ROM_LOAD( "hal21p11.bin", 0x0000, 0x4000, CRC(24abc57e) SHA1(1d7557a62adc059fb3fe20a09be18c2f40441581) )

	ROM_REGION( 0x18000, REGION_GFX3, ROMREGION_DISPOSE ) /* 16x16 sprites */
	ROM_LOAD( "hal21p13.bin", 0x00000, 0x4000, CRC(052b4f4f) SHA1(032eb5771d33defce86e222f3e7aa22bc37db6db) )
	ROM_RELOAD(               0x04000, 0x4000 )
	ROM_LOAD( "hal21p14.bin", 0x08000, 0x4000, CRC(da0cb670) SHA1(1083bdd3488dfaa5094a2ef52cfc4206f35c9612) )
	ROM_RELOAD(               0x0c000, 0x4000 )
	ROM_LOAD( "hal21p15.bin", 0x10000, 0x4000, CRC(5c5ea945) SHA1(f9ce206cab4fad1f6478d731d4b096ec33e7b99f) )
	ROM_RELOAD(               0x14000, 0x4000 )

	ROM_REGION( 0x0c00, REGION_PROMS, 0 )
	ROM_LOAD( "hal21_3.prm",  0x000, 0x400, CRC(605afff8) SHA1(94e80ebd574b1580dac4a2aebd57e3e767890c0d) )
	ROM_LOAD( "hal21_2.prm",  0x400, 0x400, CRC(c5d84225) SHA1(cc2cd32f81ed7c1bcdd68e91d00f8081cb706ce7) )
	ROM_LOAD( "hal21_1.prm",  0x800, 0x400, CRC(195768fc) SHA1(c88bc9552d57d52fb4b030d118f48fedccf563f4) )
ROM_END

ROM_START( aso )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )   /* 64k for cpuA code */
	ROM_LOAD( "p1.bin",  0x0000, 0x8000, CRC(3fc9d5e4) SHA1(1318904d3d896affd5affd8e475ac9ee6929b955) )
	ROM_LOAD( "p3.bin",  0x8000, 0x4000, CRC(39a666d2) SHA1(b5426520eb600d44bc5566d742d7b88194076494) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )   /* 64k for cpuB code */
	ROM_LOAD( "p4.bin",  0x0000, 0x8000, CRC(2429792b) SHA1(674e81880f359f7e8d34d0ad9074267360afadbf) )
	ROM_LOAD( "p6.bin",  0x8000, 0x4000, CRC(c0bfdf1f) SHA1(65b15ce9c2e78df79cb603c58639421d29701633) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )   /* 64k for sound code */
	ROM_LOAD( "p7.bin",  0x0000, 0x8000, CRC(49258162) SHA1(c265b79d012be1e065389f910f7b4ce61f5b27ce) )  /* YM3526 */
	ROM_LOAD( "p9.bin",  0x8000, 0x4000, CRC(aef5a4f4) SHA1(e908e79e27ff892fe75d1ba5cb0bc9dc6b7b4268) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE ) /* characters */
	ROM_LOAD( "p14.bin", 0x0000, 0x2000, CRC(8baa2253) SHA1(e6e4a5aa005e89744c4e2a19a080cf322edc6b52) )

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE ) /* background tiles */
	ROM_LOAD( "p10.bin", 0x0000, 0x8000, CRC(00dff996) SHA1(4f6ce4c0f2da0d2a711bcbf9aa998b4e31d0d9bf) )

	ROM_REGION( 0x18000, REGION_GFX3, ROMREGION_DISPOSE ) /* 16x16 sprites */
	ROM_LOAD( "p11.bin", 0x00000, 0x8000, CRC(7feac86c) SHA1(13b81f006ec587583416c1e7432da4c3f0375924) )
	ROM_LOAD( "p12.bin", 0x08000, 0x8000, CRC(6895990b) SHA1(e84554cae9a768021c3dc7183bc3d28e2dd768ee) )
	ROM_LOAD( "p13.bin", 0x10000, 0x8000, CRC(87a81ce1) SHA1(28c1069e6c08ecd579f99620c1cb6df01ad1aa74) )

	ROM_REGION( 0x0c00, REGION_PROMS, 0 )
	ROM_LOAD( "mb7122h.f12",  0x000, 0x00400, CRC(5b0a0059) SHA1(f61e17c8959f1cd6cc12b38f2fb7c6190ebd0e0c) )
	ROM_LOAD( "mb7122h.f13",  0x400, 0x00400, CRC(37e28dd8) SHA1(681726e490872a574dd0295823a44d64ef3a7b45) )
	ROM_LOAD( "mb7122h.f14",  0x800, 0x00400, CRC(c3fd1dd3) SHA1(c48030cc458f0bebea0ffccf3d3c43260da6a7fb) )

	ROM_REGION( 0x0600, REGION_PLDS, ROMREGION_DISPOSE )
	ROM_LOAD( "pal16l8a-1.bin", 0x0000, 0x0104, CRC(4e3f9e0d) SHA1(de448d50c0d1cdef159a8c4028846142210eba0b) )
	ROM_LOAD( "pal16l8a-2.bin", 0x0200, 0x0104, CRC(2a681f9e) SHA1(b26eb631d3e4fa6850a109a9a63d377cf86923bc) )
	ROM_LOAD( "pal16r6a.bin",   0x0400, 0x0104, CRC(59c03681) SHA1(d21090b35596c28d44862782386e84dfc1feff0c) )
ROM_END

ROM_START( alphamis )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )   /* 64k for cpuA code */
	ROM_LOAD( "p1.rom",  0x0000, 0x4000, CRC(69af874b) SHA1(11a13574614e7e3b9e33c2b2827571946a805376) )
	ROM_LOAD( "p2.rom",  0x4000, 0x4000, CRC(7707bfe3) SHA1(fb1f4ef862f6762d2479e537fc67a819d11ace76) )
	ROM_LOAD( "p3.rom",  0x8000, 0x4000, CRC(b970d642) SHA1(d3a8045f05f001e5e2fae8ef7900cf87ab17fc74) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )   /* 64k for cpuB code */
	ROM_LOAD( "p4.rom",  0x0000, 0x4000, CRC(91a89d3c) SHA1(46ef8718c81aac2f09dd1884538750edf9662760) )
	ROM_LOAD( "p5.rom",  0x4000, 0x4000, CRC(9879e506) SHA1(0bce5fcb9d05ce77cd8e9ad1cac04ef617928db0) )
	ROM_LOAD( "p6.bin",  0x8000, 0x4000, CRC(c0bfdf1f) SHA1(65b15ce9c2e78df79cb603c58639421d29701633) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )   /* 64k for sound code */
	ROM_LOAD( "p7.rom",  0x0000, 0x4000, CRC(dbc19736) SHA1(fe365d70ead8243374979d2162c395fed9870405) )  /* YM3526 */
	ROM_LOAD( "p8.rom",  0x4000, 0x4000, CRC(537726a9) SHA1(ddf66946be71d2e6ab2cc53150e3b36d45dde2eb) )
	ROM_LOAD( "p9.bin",  0x8000, 0x4000, CRC(aef5a4f4) SHA1(e908e79e27ff892fe75d1ba5cb0bc9dc6b7b4268) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE ) /* characters */
	ROM_LOAD( "p14.rom", 0x0000, 0x2000, CRC(acbe29b2) SHA1(e304c6d30888fa7549d25e6329ba94d5088bd8b7) )

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE ) /* background tiles */
	ROM_LOAD( "p10.bin", 0x0000, 0x8000, CRC(00dff996) SHA1(4f6ce4c0f2da0d2a711bcbf9aa998b4e31d0d9bf) )

	ROM_REGION( 0x18000, REGION_GFX3, ROMREGION_DISPOSE ) /* 16x16 sprites */
	ROM_LOAD( "p11.bin", 0x00000, 0x8000, CRC(7feac86c) SHA1(13b81f006ec587583416c1e7432da4c3f0375924) )
	ROM_LOAD( "p12.bin", 0x08000, 0x8000, CRC(6895990b) SHA1(e84554cae9a768021c3dc7183bc3d28e2dd768ee) )
	ROM_LOAD( "p13.bin", 0x10000, 0x8000, CRC(87a81ce1) SHA1(28c1069e6c08ecd579f99620c1cb6df01ad1aa74) )

	ROM_REGION( 0x0c00, REGION_PROMS, 0 )
	ROM_LOAD( "mb7122h.f12",  0x000, 0x00400, CRC(5b0a0059) SHA1(f61e17c8959f1cd6cc12b38f2fb7c6190ebd0e0c) )
	ROM_LOAD( "mb7122h.f13",  0x400, 0x00400, CRC(37e28dd8) SHA1(681726e490872a574dd0295823a44d64ef3a7b45) )
	ROM_LOAD( "mb7122h.f14",  0x800, 0x00400, CRC(c3fd1dd3) SHA1(c48030cc458f0bebea0ffccf3d3c43260da6a7fb) )

	ROM_REGION( 0x0600, REGION_PLDS, ROMREGION_DISPOSE )
	ROM_LOAD( "pal16l8a-1.bin", 0x0000, 0x0104, CRC(4e3f9e0d) SHA1(de448d50c0d1cdef159a8c4028846142210eba0b) )
	ROM_LOAD( "pal16l8a-2.bin", 0x0200, 0x0104, CRC(2a681f9e) SHA1(b26eb631d3e4fa6850a109a9a63d377cf86923bc) )
	ROM_LOAD( "pal16r6a.bin",   0x0400, 0x0104, CRC(59c03681) SHA1(d21090b35596c28d44862782386e84dfc1feff0c) )
ROM_END

ROM_START( arian )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )   /* 64k for cpuA code */
	ROM_LOAD( "p1.d8",   0x0000, 0x4000, CRC(0ca89307) SHA1(d0ecb97b1e147a4001a4383fd5709394e2358a45) ) /* roms that differ from above sets all had a red stripe on the label */
	ROM_LOAD( "p2.d7",   0x4000, 0x4000, CRC(724518c3) SHA1(debbfe2a485af5f452d208a04705dbd48d47d90f) ) /* IE: P1 through P4 and P14 */
	ROM_LOAD( "p3.d6",   0x8000, 0x4000, CRC(4d8db650) SHA1(184141847d38077737ee7140861d94832018e2e2) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )   /* 64k for cpuB code */
	ROM_LOAD( "p4.d3",   0x0000, 0x4000, CRC(47baf1db) SHA1(3947a679745811e5499d690f2b73b4f28b1d47f9) )
	ROM_LOAD( "p5.rom",  0x4000, 0x4000, CRC(9879e506) SHA1(0bce5fcb9d05ce77cd8e9ad1cac04ef617928db0) )
	ROM_LOAD( "p6.bin",  0x8000, 0x4000, CRC(c0bfdf1f) SHA1(65b15ce9c2e78df79cb603c58639421d29701633) )

	ROM_REGION( 0x10000, REGION_CPU3, 0 )   /* 64k for sound code */
	ROM_LOAD( "p7.rom",  0x0000, 0x4000, CRC(dbc19736) SHA1(fe365d70ead8243374979d2162c395fed9870405) )  /* YM3526 */
	ROM_LOAD( "p8.rom",  0x4000, 0x4000, CRC(537726a9) SHA1(ddf66946be71d2e6ab2cc53150e3b36d45dde2eb) )
	ROM_LOAD( "p9.bin",  0x8000, 0x4000, CRC(aef5a4f4) SHA1(e908e79e27ff892fe75d1ba5cb0bc9dc6b7b4268) )

	ROM_REGION( 0x2000, REGION_GFX1, ROMREGION_DISPOSE ) /* characters */
	ROM_LOAD( "p14.h1",  0x0000, 0x2000, CRC(e599bd30) SHA1(bf70aae9a15d548bb532ca1fc8d7220dfa150d6e) )

	ROM_REGION( 0x8000, REGION_GFX2, ROMREGION_DISPOSE ) /* background tiles */
	ROM_LOAD( "p10.bin", 0x0000, 0x8000, CRC(00dff996) SHA1(4f6ce4c0f2da0d2a711bcbf9aa998b4e31d0d9bf) )

	ROM_REGION( 0x18000, REGION_GFX3, ROMREGION_DISPOSE ) /* 16x16 sprites */
	ROM_LOAD( "p11.bin", 0x00000, 0x8000, CRC(7feac86c) SHA1(13b81f006ec587583416c1e7432da4c3f0375924) )
	ROM_LOAD( "p12.bin", 0x08000, 0x8000, CRC(6895990b) SHA1(e84554cae9a768021c3dc7183bc3d28e2dd768ee) )
	ROM_LOAD( "p13.bin", 0x10000, 0x8000, CRC(87a81ce1) SHA1(28c1069e6c08ecd579f99620c1cb6df01ad1aa74) )

	ROM_REGION( 0x0c00, REGION_PROMS, 0 )
	ROM_LOAD( "mb7122h.f12",  0x000, 0x00400, CRC(5b0a0059) SHA1(f61e17c8959f1cd6cc12b38f2fb7c6190ebd0e0c) )
	ROM_LOAD( "mb7122h.f13",  0x400, 0x00400, CRC(37e28dd8) SHA1(681726e490872a574dd0295823a44d64ef3a7b45) )
	ROM_LOAD( "mb7122h.f14",  0x800, 0x00400, CRC(c3fd1dd3) SHA1(c48030cc458f0bebea0ffccf3d3c43260da6a7fb) )

	ROM_REGION( 0x0600, REGION_PLDS, ROMREGION_DISPOSE )
	ROM_LOAD( "pal16l8a-1.bin", 0x0000, 0x0104, CRC(4e3f9e0d) SHA1(de448d50c0d1cdef159a8c4028846142210eba0b) )
	ROM_LOAD( "pal16l8a-2.bin", 0x0200, 0x0104, CRC(2a681f9e) SHA1(b26eb631d3e4fa6850a109a9a63d377cf86923bc) )
	ROM_LOAD( "pal16r6a.bin",   0x0400, 0x0104, CRC(59c03681) SHA1(d21090b35596c28d44862782386e84dfc1feff0c) )
ROM_END

GAME( 1985, aso,      0,     aso,   aso,   aso,   ROT270, "SNK", "ASO - Armored Scrum Object", GAME_NO_COCKTAIL )
GAME( 1985, alphamis, aso,   aso,   aso,   aso,   ROT270, "SNK", "Alpha Mission", GAME_NO_COCKTAIL )
GAME( 1985, arian,    aso,   aso,   aso,   aso,   ROT270, "SNK", "Arian Mission", GAME_NO_COCKTAIL )
GAME( 1985, hal21,    0,     hal21, hal21, hal21, ROT270, "SNK", "HAL21", GAME_IMPERFECT_COLORS | GAME_NO_COCKTAIL )
GAME( 1985, hal21j,   hal21, hal21, hal21, hal21, ROT270, "SNK", "HAL21 (Japan)", GAME_IMPERFECT_COLORS | GAME_NO_COCKTAIL )
