/*
    Hal21


Change Log
----------

AT08XX03:

[Common]
 - added shadows and highlights

 * Cocktail mode involves changing tile offsets and sprite
   coordinates and is still unsupported.

 * Manuals show both boards have noise filters to smooth out
   rings and scratches which are especially audible in HAL21.

[HAL21]
 - installed NMI scheduler to prevent music trashing

[TODO]
 - verify color effects
*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "snk.h"
#include "sound/ay8910.h"
#include "sound/3812intf.h"

static UINT8 *hal21_vreg, *hal21_sndfifo;
static UINT8 *textram;
static int color[2];
static int snk_blink_parity;	// FIXME likely wrong
static int snk_sound_busy_bit;

/**************************************************************************/

static void hal21_sound_scheduler(running_machine *machine, int mode, int data)
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

	snk_sound_busy_bit = 0x01;
	soundlatch_w(machine, 0, data);
	cpunum_set_input_line(machine, 2, INPUT_LINE_NMI, PULSE_LINE);
}

/**************************************************************************/

static WRITE8_HANDLER( hal21_vreg0_w ){ hal21_vreg[0] = data; }
static WRITE8_HANDLER( hal21_vreg1_w ){ hal21_vreg[1] = data; }
static WRITE8_HANDLER( hal21_vreg2_w ){ hal21_vreg[2] = data; }
static WRITE8_HANDLER( hal21_vreg3_w ){ hal21_vreg[3] = data; }
static WRITE8_HANDLER( hal21_vreg4_w ){ hal21_vreg[4] = data; }
static WRITE8_HANDLER( hal21_vreg5_w ){ hal21_vreg[5] = data; }


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

static VIDEO_START( hal21 )
{
	snk_blink_parity = 0;
}


static VIDEO_RESET( aso )
{
	color[0] = 8;
	color[1] = 8;
}


static void hal21_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int scrollx, int scrolly, int attrs,
								const gfx_element *gfx )
{
	int bankbase, c, x, y, offsx, offsy, dx, dy, sx, sy, offs, tile_number;

	bankbase = attrs<<3 & 0x100;
	c = attrs & 0x0f;
	if (c > 11) { fillbitmap(bitmap,(c<<4)+8, cliprect); return; }
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

static void hal21_draw_sprites(bitmap_t *bitmap, const rectangle *cliprect, int scrollx, int scrolly,
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

static void tnk3_draw_text(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int bank, UINT8 *source )
{
	const gfx_element *gfx = machine->gfx[0];

	int tile_number, color, sx, sy;
	int x, y;

	for(x=0; x<32; x++) for(y=0; y<32; y++)
	{
		tile_number = source[(x<<5)+y];

		if(tile_number == 0x20 || tile_number == 0xff) continue;

		if(bank == -1) color = 8;
		else
		{
			color = tile_number >> 5;
			tile_number |= bank << 8;
		}
		sx = (x+2) << 3;
		sy = (y+1) << 3;

		drawgfx(bitmap,gfx,tile_number,color,0,0,sx,sy,cliprect,TRANSPARENCY_PEN,15);
	}
}

static void tnk3_draw_status_main(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int bank, UINT8 *source, int start )
{
	const gfx_element *gfx = machine->gfx[0];

	int tile_number, color, sx, sy;
	int x, y;

	for(x = start; x < start+2; x++) for(y = 0; y < 32; y++)
	{
		tile_number = source[(x<<5)+y];

		if(bank == -1) color = 8;
		else
		{
 			color = tile_number >> 5;
			tile_number |= (bank << 8);
		}
		sx = ((x+34)&0x3f) << 3;
		sy = (y+1) << 3;

		drawgfx(bitmap,gfx,tile_number,color,0,0,sx,sy,cliprect,TRANSPARENCY_NONE,0);
	}
}

static void tnk3_draw_status(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int bank, UINT8 *source )
{
	tnk3_draw_status_main(machine,bitmap,cliprect,bank,source, 0);
	tnk3_draw_status_main(machine,bitmap,cliprect,bank,source,30);
}

static VIDEO_UPDATE( hal21 )
{
	int attr, msbs, spsy, spsx, bgsy, bgsx, bank, i;

	attr = (int)hal21_vreg[0];
	msbs = (int)hal21_vreg[1];
	spsy = (int)hal21_vreg[2] + (msbs<<5 & 0x100) + 9;
	spsx = (int)hal21_vreg[3] + (msbs<<8 & 0x100) + 30;
	bgsy = (int)hal21_vreg[4] + (msbs<<4 & 0x100) - 8;
	bgsx = (int)hal21_vreg[5] - 16;

	hal21_draw_background(screen->machine, bitmap, cliprect, bgsx+(msbs<<7 & 0x100), bgsy, attr, screen->machine->gfx[1]);

	attr = snk_blink_parity;
	snk_blink_parity ^= 0xdf;
	for (i=6; i<0x80; i+=8) { palette_set_color(screen->machine, i, MAKE_RGB(attr, attr, attr)); }

	hal21_draw_sprites(bitmap, cliprect, spsx, spsy, screen->machine->gfx[2]);

	bank = msbs>>6 & 1;
	tnk3_draw_text(screen->machine, bitmap, cliprect, bank, &textram[0]);
	tnk3_draw_status(screen->machine, bitmap, cliprect, bank, &textram[0x400]);
	return 0;
}


static CUSTOM_INPUT( sound_status_r )
{
	return snk_sound_busy_bit;
}


static INPUT_PORTS_START( hal21 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(sound_status_r, NULL) /* sound CPU status */
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
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

	PORT_START("DSW2")
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
	GFXDECODE_ENTRY( "gfx1", 0, char256,    128*3,  8 ) /* colors 384..511 */
	GFXDECODE_ENTRY( "gfx2", 0, char1024,   128*1, 16 ) /* colors 128..383 */
	GFXDECODE_ENTRY( "gfx3", 0, sprite1024, 128*0, 16 ) /* colors   0..127 */
GFXDECODE_END

/**************************************************************************/

static READ8_HANDLER( CPUC_ready_r ) { snk_sound_busy_bit = 0; return 0; }

static WRITE8_HANDLER( hal21_soundcommand_w ) { hal21_sound_scheduler(machine, 1, data); }
static WRITE8_HANDLER( hal21_soundack_w ) { hal21_sound_scheduler(machine,2, data); }

static READ8_HANDLER( hal21_soundcommand_r )
{
	int data = soundlatch_r(machine, 0);
	soundlatch_clear_w(machine, 0, 0);
	return data;
}

static INTERRUPT_GEN( hal21_sound_interrupt )
{
	hal21_sound_scheduler(machine, 3, 0);
}

/**************************************************************************/

static ADDRESS_MAP_START( hal21_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(hal21_soundcommand_r)
	AM_RANGE(0xc000, 0xc000) AM_READ(CPUC_ready_r)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0xe001, 0xe001) AM_WRITE(ay8910_write_port_0_w)
	AM_RANGE(0xe002, 0xe002) AM_WRITE(hal21_soundack_w) // bitfielded(0-5) acknowledge write, details unknown
	AM_RANGE(0xe008, 0xe008) AM_WRITE(ay8910_control_port_1_w)
	AM_RANGE(0xe009, 0xe009) AM_WRITE(ay8910_write_port_1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( hal21_sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0x0000) AM_NOP				// external sound ROM detection?
ADDRESS_MAP_END

/**************************** HAL21 *************************/

static ADDRESS_MAP_START( hal21_cpuA_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("P1")
	AM_RANGE(0xc200, 0xc200) AM_READ_PORT("P2")
	AM_RANGE(0xc300, 0xc300) AM_WRITE(hal21_soundcommand_w)
	AM_RANGE(0xc400, 0xc400) AM_READ_PORT("DSW1")
	AM_RANGE(0xc500, 0xc500) AM_READ_PORT("DSW2")
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

static DRIVER_INIT( hal21 )
{
	hal21_vreg = auto_malloc(24);
	hal21_sndfifo = hal21_vreg + 8;
}

static MACHINE_RESET( aso )
{
	memset(hal21_vreg, 0, 8);
	hal21_sound_scheduler(machine, 0, 0);
	snk_sound_busy_bit = 0;
}

static MACHINE_DRIVER_START( hal21 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(hal21_cpuA_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD("sub", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(hal21_cpuB_map,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD("audio", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(hal21_sound_map,0)
	MDRV_CPU_IO_MAP(hal21_sound_portmap,0)
	MDRV_CPU_VBLANK_INT("main", hal21_sound_interrupt)
	MDRV_CPU_PERIODIC_INT(irq0_line_hold, 220) // music tempo, hand tuned

	MDRV_INTERLEAVE(100)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_HIGHLIGHTS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(36*8, 28*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 1*8, 28*8-1)

	MDRV_GFXDECODE(aso)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_MACHINE_RESET(aso)

	MDRV_PALETTE_INIT(aso)
	MDRV_VIDEO_START(hal21)
	MDRV_VIDEO_RESET(aso)
	MDRV_VIDEO_UPDATE(hal21)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MDRV_SOUND_ADD("ay2", AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_DRIVER_END

/**************************************************************************/

ROM_START( hal21 )
	ROM_REGION( 0x10000, "main", 0 )   /* 64k for CPUA code */
	ROM_LOAD( "hal21p1.bin",    0x0000, 0x2000, CRC(9d193830) SHA1(8e4e9c8bc774d7c7c0b68a5fa5cabdc6b5cfa41b) )
	ROM_LOAD( "hal21p2.bin",    0x2000, 0x2000, CRC(c1f00350) SHA1(8709455a980931565ccca60162a04c6c3133099b) )
	ROM_LOAD( "hal21p3.bin",    0x4000, 0x2000, CRC(881d22a6) SHA1(4b2a65dc18620f7f77532f791212fccfe1f0b245) )
	ROM_LOAD( "hal21p4.bin",    0x6000, 0x2000, CRC(ce692534) SHA1(e1d8e6948578ec9d0b6dc2aff17ad23b8ce46d6a) )

	ROM_REGION( 0x10000, "sub", 0 )   /* 64k for CPUB code */
	ROM_LOAD( "hal21p5.bin",    0x0000, 0x2000, CRC(3ce0684a) SHA1(5e76770a3252d5565a8f11a79ac3a9a6c31a43e2) )
	ROM_LOAD( "hal21p6.bin",    0x2000, 0x2000, CRC(878ef798) SHA1(0aae152947c9c6733b77dd1ac14f2f6d6bfabeaa) )
	ROM_LOAD( "hal21p7.bin",    0x4000, 0x2000, CRC(72ebbe95) SHA1(b1f7dc535e7670647500391d21dfa971d5e342a2) )
	ROM_LOAD( "hal21p8.bin",    0x6000, 0x2000, CRC(17e22ad3) SHA1(0e10a3c0f2e2ec284f4e0f1055397a8ccd1ff0f7) )
	ROM_LOAD( "hal21p9.bin",    0x8000, 0x2000, CRC(b146f891) SHA1(0b2db3e14b0401a7914002c6f7c26933a1cba162) )

	ROM_REGION( 0x10000, "audio", 0 )   /* 64k for sound code */
	ROM_LOAD( "hal21p10.bin",   0x0000, 0x4000, CRC(916f7ba0) SHA1(7b8bcd59d768c4cd226de96895d3b9755bb3ba79) )

	ROM_REGION( 0x2000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "hal21p12.bin", 0x0000, 0x2000, CRC(9839a7cd) SHA1(d3f9d964263a64aa3648faf5eb2e4fa532ae7852) ) /* char */

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE ) /* background tiles */
	ROM_LOAD( "hal21p11.bin", 0x0000, 0x4000, CRC(24abc57e) SHA1(1d7557a62adc059fb3fe20a09be18c2f40441581) )

	ROM_REGION( 0x18000, "gfx3", ROMREGION_DISPOSE ) /* 16x16 sprites */
	ROM_LOAD( "hal21p13.bin", 0x00000, 0x4000, CRC(052b4f4f) SHA1(032eb5771d33defce86e222f3e7aa22bc37db6db) )
	ROM_RELOAD(               0x04000, 0x4000 )
	ROM_LOAD( "hal21p14.bin", 0x08000, 0x4000, CRC(da0cb670) SHA1(1083bdd3488dfaa5094a2ef52cfc4206f35c9612) )
	ROM_RELOAD(               0x0c000, 0x4000 )
	ROM_LOAD( "hal21p15.bin", 0x10000, 0x4000, CRC(5c5ea945) SHA1(f9ce206cab4fad1f6478d731d4b096ec33e7b99f) )
	ROM_RELOAD(               0x14000, 0x4000 )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "hal21_3.prm",  0x000, 0x400, CRC(605afff8) SHA1(94e80ebd574b1580dac4a2aebd57e3e767890c0d) )
	ROM_LOAD( "hal21_2.prm",  0x400, 0x400, CRC(c5d84225) SHA1(cc2cd32f81ed7c1bcdd68e91d00f8081cb706ce7) )
	ROM_LOAD( "hal21_1.prm",  0x800, 0x400, CRC(195768fc) SHA1(c88bc9552d57d52fb4b030d118f48fedccf563f4) )
ROM_END

ROM_START( hal21j )
	ROM_REGION( 0x10000, "main", 0 )   /* 64k for CPUA code */
	ROM_LOAD( "hal21p1.bin",    0x0000, 0x2000, CRC(9d193830) SHA1(8e4e9c8bc774d7c7c0b68a5fa5cabdc6b5cfa41b) )
	ROM_LOAD( "hal21p2.bin",    0x2000, 0x2000, CRC(c1f00350) SHA1(8709455a980931565ccca60162a04c6c3133099b) )
	ROM_LOAD( "hal21p3.bin",    0x4000, 0x2000, CRC(881d22a6) SHA1(4b2a65dc18620f7f77532f791212fccfe1f0b245) )
	ROM_LOAD( "hal21p4.bin",    0x6000, 0x2000, CRC(ce692534) SHA1(e1d8e6948578ec9d0b6dc2aff17ad23b8ce46d6a) )

	ROM_REGION( 0x10000, "sub", 0 )   /* 64k for CPUB code */
	ROM_LOAD( "hal21p5.bin",    0x0000, 0x2000, CRC(3ce0684a) SHA1(5e76770a3252d5565a8f11a79ac3a9a6c31a43e2) )
	ROM_LOAD( "hal21p6.bin",    0x2000, 0x2000, CRC(878ef798) SHA1(0aae152947c9c6733b77dd1ac14f2f6d6bfabeaa) )
	ROM_LOAD( "hal21p7.bin",    0x4000, 0x2000, CRC(72ebbe95) SHA1(b1f7dc535e7670647500391d21dfa971d5e342a2) )
	ROM_LOAD( "hal21p8.bin",    0x6000, 0x2000, CRC(17e22ad3) SHA1(0e10a3c0f2e2ec284f4e0f1055397a8ccd1ff0f7) )
	ROM_LOAD( "hal21p9.bin",    0x8000, 0x2000, CRC(b146f891) SHA1(0b2db3e14b0401a7914002c6f7c26933a1cba162) )

	ROM_REGION( 0x10000, "audio", 0 )   /* 64k for sound code */
	ROM_LOAD( "hal21-10.bin",   0x0000, 0x4000, CRC(a182b3f0) SHA1(b76eff97a58a96467e9f3a74125a0a770e7678f8) )

	ROM_REGION( 0x2000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "hal21p12.bin", 0x0000, 0x2000, CRC(9839a7cd) SHA1(d3f9d964263a64aa3648faf5eb2e4fa532ae7852) ) /* char */

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE ) /* background tiles */
	ROM_LOAD( "hal21p11.bin", 0x0000, 0x4000, CRC(24abc57e) SHA1(1d7557a62adc059fb3fe20a09be18c2f40441581) )

	ROM_REGION( 0x18000, "gfx3", ROMREGION_DISPOSE ) /* 16x16 sprites */
	ROM_LOAD( "hal21p13.bin", 0x00000, 0x4000, CRC(052b4f4f) SHA1(032eb5771d33defce86e222f3e7aa22bc37db6db) )
	ROM_RELOAD(               0x04000, 0x4000 )
	ROM_LOAD( "hal21p14.bin", 0x08000, 0x4000, CRC(da0cb670) SHA1(1083bdd3488dfaa5094a2ef52cfc4206f35c9612) )
	ROM_RELOAD(               0x0c000, 0x4000 )
	ROM_LOAD( "hal21p15.bin", 0x10000, 0x4000, CRC(5c5ea945) SHA1(f9ce206cab4fad1f6478d731d4b096ec33e7b99f) )
	ROM_RELOAD(               0x14000, 0x4000 )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "hal21_3.prm",  0x000, 0x400, CRC(605afff8) SHA1(94e80ebd574b1580dac4a2aebd57e3e767890c0d) )
	ROM_LOAD( "hal21_2.prm",  0x400, 0x400, CRC(c5d84225) SHA1(cc2cd32f81ed7c1bcdd68e91d00f8081cb706ce7) )
	ROM_LOAD( "hal21_1.prm",  0x800, 0x400, CRC(195768fc) SHA1(c88bc9552d57d52fb4b030d118f48fedccf563f4) )
ROM_END




GAME( 1985, hal21,    0,     hal21, hal21, hal21, ROT270, "SNK", "HAL21", GAME_IMPERFECT_COLORS | GAME_NO_COCKTAIL )
GAME( 1985, hal21j,   hal21, hal21, hal21, hal21, ROT270, "SNK", "HAL21 (Japan)", GAME_IMPERFECT_COLORS | GAME_NO_COCKTAIL )
