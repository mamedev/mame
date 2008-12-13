/*******************************************************************************************

Jangou (c) 1983 Nichibutsu

driver by David Haywood,Angelo Salese and Phil Bennett

TODO:
-convert the video hw to true 4bpp;
-some charset transparency pens are wrong,likely that the buffer trigger isn't correct;
-unemulated screen flipping;

============================================================================================
Debug cheats:

$c132 coin counter
$c088-$c095 player tiles

============================================================================================

JANGOU (C)1982 Nichibutsu

CPU:   Z80 *2
XTAL:  19.968MHz
SOUND: AY-3-8910

Location 2-P: HARRIS HCI-55536-5
Location 3-G: MB7051

*******************************************************************************************/

#include "driver.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"


#define MASTER_CLOCK	XTAL_19_968MHz

static UINT8 *blit_buffer;
static UINT8 pen_data[0x10];
static UINT8 blit_data[6];

static emu_timer *cvsd_bit_timer;
static UINT8 cvsd_shiftreg;
static int shift_cnt;


static VIDEO_START( jangou )
{
	blit_buffer = auto_malloc(256*256);
}

static VIDEO_UPDATE( jangou )
{
	int x, y;

	for (y = cliprect->min_y; y <= cliprect->max_y; ++y)
	{
		UINT8 *src = &blit_buffer[y * 512/2 + cliprect->min_x];
		UINT16 *dst = BITMAP_ADDR16(bitmap, y, cliprect->min_x);

		for (x = cliprect->min_x; x <= cliprect->max_x; x += 2)
		{
			UINT32 srcpix = *src++;
			*dst++ = screen->machine->pens[srcpix & 0xf];
			*dst++ = screen->machine->pens[(srcpix >> 4) & 0xf];
		}
	}

	return 0;
}

static READ8_HANDLER( blitter_status_r )
{
	/*if bit 7 is not high,game goes into a tight loop until this bit is set,likely to be a busy flag.*/
	return 0x80;
}

/*
Blitter Memory Map:

src lo word[$12]
src hi word[$13]
x [$14]
y [$15]
h [$16]
w [$17]
*/

static UINT8 jangou_gfx_nibble(running_machine *machine,UINT16 niboffset)
{
	UINT8 *blit_rom = memory_region(machine,"gfx");

	if (niboffset&1)
	{
		return (blit_rom[(niboffset>>1)&0xffff] & 0xf0)>>4;
	}
	else
	{
		return (blit_rom[(niboffset>>1)&0xffff] & 0x0f);
	}
}

static void plot_jangou_gfx_pixel(UINT8 pix, int x, int y)
{
	if (y>=512) return;
	if (x>=512) return;
	if (y<0) return;
	if (x<0) return;

	if (x&1)
	{
		blit_buffer[(y*256)+(x>>1)] = (blit_buffer[(y*256)+(x>>1)] & 0x0f) | ((pix<<4) & 0xf0);
	}
	else
	{
		blit_buffer[(y*256)+(x>>1)] = (blit_buffer[(y*256)+(x>>1)] & 0xf0) | (pix & 0x0f);
	}
}

static WRITE8_HANDLER( blitter_process_w )
{
	int src,x,y,h,w, flipx;
	blit_data[offset] = data;

	/*trigger blitter write to ram,might not be correct...*/
	if(offset == 5)
	{
//		printf("%02x %02x %02x %02x %02x %02x\n",blit_data[0],blit_data[1],blit_data[2],blit_data[3],blit_data[4],blit_data[5]);
		w = (blit_data[4] & 0xff)+1;
		h = (blit_data[5] & 0xff)+1;
		src = ((blit_data[1]<<8)|(blit_data[0]<<0));
		x = (blit_data[2] & 0xff);
		y = (blit_data[3] & 0xff);

		// lowest bit of src controls flipping / draw direction?
		flipx=(blit_data[0] & 1);

		if (!flipx) src += (w*h)-1;
		else src -= (w*h)-1;

		{
			int count = 0;
			int xcount,ycount;
			for(ycount=0;ycount<h;ycount++)
			{
				for(xcount=0;xcount<w;xcount++)
				{
					int drawx = (x+xcount) & 0xff;
					int drawy = (y+ycount) & 0xff;
					UINT8 dat = jangou_gfx_nibble(space->machine,src+count);
					UINT8 cur_pen_hi = pen_data[(dat & 0xf0)>>4];
					UINT8 cur_pen_lo = pen_data[(dat & 0x0f)>>0];

					/*TODO: simplify this when blitter internal ram is converted to 4bpp*/
					/*don't use hi & lo user-selected bits*/
					if((cur_pen_hi & 0x20) && (cur_pen_lo & 0x20))
						plot_jangou_gfx_pixel(dat, drawx,drawy);
					else if(cur_pen_lo & 0x20)//use hi pen user-selected bit
					{
						dat = (pen_data[cur_pen_hi]<<4) | (dat & 0xf);
						plot_jangou_gfx_pixel(dat, drawx,drawy);
					}
					else if(cur_pen_hi & 0x20)//use lo pen user-selected bit
					{
						dat = (pen_data[cur_pen_lo]) | (dat & 0xf0);
						plot_jangou_gfx_pixel(dat, drawx,drawy);
					}
					else //use hi & lo pens user-selected bits
					{
						dat = (pen_data[cur_pen_lo]) | (pen_data[cur_pen_hi]<<4);
						plot_jangou_gfx_pixel(dat, drawx,drawy);
					}

					if (!flipx)	count--;
					else count++;
				}
			}
		}
	}
}

/*Every offset of these registers controls a pen.
  If the bit 6 (0x20) is high,use the pen from the internal gfx rom.
  If the bit 6 is low,use an user-selected pen,defined by this register.*/
static WRITE8_HANDLER( blit_vregs_w )
{
	pen_data[offset] = data;
}

static UINT8 mux_data;

static WRITE8_HANDLER( mux_w )
{
	mux_data = ~data;
}

static WRITE8_HANDLER( output_w )
{
	/*
	--x- ---- ? (polls between high and low in irq routine)
	---- -x-- flip screen
	---- ---x coin counter
	*/
//	printf("%02x\n",data);
	coin_counter_w(0,data & 0x01);
//	flip_screen_set(data & 0x04);
//	coin_lockout_w(0,~data & 0x20);
}

static READ8_HANDLER( input_mux_r )
{
	switch(mux_data)
	{
		case 0x01: return input_port_read(space->machine, "PL1_1");
		case 0x02: return input_port_read(space->machine, "PL1_2");
		case 0x04: return input_port_read(space->machine, "PL2_1");
		case 0x08: return input_port_read(space->machine, "PL2_2");
		case 0x10: return input_port_read(space->machine, "PL1_3");
		case 0x20: return input_port_read(space->machine, "PL2_3");
	}
//	printf("%04x\n",mux_data);

	return 0xff;
}

static READ8_HANDLER( input_system_r )
{
	return input_port_read(space->machine, "SYSTEM");
}

/*
	Sound CPU and CVSD
*/
static WRITE8_HANDLER( sound_latch_w )
{
	soundlatch_w(space, 0, data & 0xff);
	cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_NMI, ASSERT_LINE);
}

static READ8_HANDLER( sound_latch_r )
{
	cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_NMI, CLEAR_LINE);
	return soundlatch_r(space, 0);
}

static WRITE8_HANDLER( cvsd_w )
{
	cvsd_shiftreg = data;
}

static TIMER_CALLBACK( cvsd_bit_timer_callback )
{
	/* Data is shifted out at the MSB */
	hc55516_digit_w(0, (cvsd_shiftreg >> 7) & 1);
	cvsd_shiftreg <<= 1;

	/* Trigger an IRQ for every 8 shifted bits */
	if ((++shift_cnt & 7) == 0)
		cpu_set_input_line(machine->cpu[1], 0, HOLD_LINE);
}


static ADDRESS_MAP_START( cpu0_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu0_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01,0x01) AM_READ(ay8910_read_port_0_r)
	AM_RANGE(0x02,0x02) AM_WRITE(ay8910_write_port_0_w)
	AM_RANGE(0x03,0x03) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0x10,0x10) AM_READ(blitter_status_r)
	AM_RANGE(0x10,0x10) AM_WRITE(output_w)
	AM_RANGE(0x11,0x11) AM_WRITE(mux_w)
	AM_RANGE(0x12,0x17) AM_WRITE(blitter_process_w)
	AM_RANGE(0x20,0x2f) AM_WRITE(blit_vregs_w)
	AM_RANGE(0x30,0x30) AM_WRITENOP //? polls 0x03 continously
	AM_RANGE(0x31,0x31) AM_WRITE(sound_latch_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( cpu1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu1_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x00) AM_READ(sound_latch_r)
	AM_RANGE(0x01,0x01) AM_WRITE(cvsd_w)
	AM_RANGE(0x02,0x02) AM_WRITE(SMH_NOP) // Echoes sound command - acknowledge?
ADDRESS_MAP_END

static INPUT_PORTS_START( jangou )
	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	/*The "unknown" bits for this port might be actually unused*/
	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static PALETTE_INIT( jangou )
{
	int	bit0, bit1, bit2 , r, g, b;
	int	i;

	for (i = 0; i < 0x10; ++i)
	{
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 4) & 0x01;
		bit2 = (color_prom[0] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
		color_prom++;
	}
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_mux_r,
	input_system_r,
	NULL,
	NULL
};

static SOUND_START( jangou )
{
	/* Create a timer to feed the CVSD DAC with sample bits */
	cvsd_bit_timer = timer_alloc(machine, cvsd_bit_timer_callback, NULL);
	timer_adjust_periodic(cvsd_bit_timer, ATTOTIME_IN_HZ(MASTER_CLOCK / 1024), 0, ATTOTIME_IN_HZ(MASTER_CLOCK / 1024));
}


/* Note: All frequencies and dividers are unverified */
static MACHINE_DRIVER_START( jangou )
	/* basic machine hardware */
	MDRV_CPU_ADD("cpu0", Z80, MASTER_CLOCK / 8)
	MDRV_CPU_PROGRAM_MAP(0, cpu0_map)
	MDRV_CPU_IO_MAP(0, cpu0_io)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD("cpu1", Z80, MASTER_CLOCK / 8)
	MDRV_CPU_PROGRAM_MAP(0, cpu1_map)
	MDRV_CPU_IO_MAP(0, cpu1_io)

	/* video hardware */
	MDRV_PALETTE_INIT(jangou)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 16, 240-1)

	MDRV_PALETTE_LENGTH(16)

	MDRV_VIDEO_START(jangou)
	MDRV_VIDEO_UPDATE(jangou)

	/* sound hardware */
	MDRV_SOUND_START(jangou)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay", AY8910, MASTER_CLOCK / 16)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("cvsd", HC55516, MASTER_CLOCK / 1024)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_DRIVER_END


ROM_START( jangou )
	ROM_REGION( 0x10000, "cpu0", 0 )
	ROM_LOAD( "jg05.bin", 0x00000, 0x02000, CRC(a3cfe33f) SHA1(9ad34a2167568316d242c990ea6fe42dadd4ac30) )
	ROM_LOAD( "jg06.bin", 0x02000, 0x02000, CRC(d8523478) SHA1(f32c2e866c6aeae29f25f0947b07d725ce61d89d) )
	ROM_LOAD( "jg07.bin", 0x04000, 0x02000, CRC(4b30d1fc) SHA1(6f240aa4b7a343f180446581fe95cf7da0fba57b) )
	ROM_LOAD( "jg08.bin", 0x06000, 0x02000, CRC(bb078813) SHA1(a3b7df84629337c83307f49f52338aa983e531ba) )

	ROM_REGION( 0x4000, "cpu1", 0 )
	ROM_LOAD( "jg03.bin", 0x00000, 0x02000, CRC(5a113e90) SHA1(7d9ae481680fc640e03f6836f60bccb933bbef31) )
	ROM_LOAD( "jg04.bin", 0x02000, 0x02000, CRC(accd3ab5) SHA1(46a502801da7a56d73a984614f10b20897e340e8) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "jg01.bin", 0x00000, 0x02000, CRC(5034a744) SHA1(b83212b6ff12aaf730c6d3e3d1470d613bbe0d1d) )
	ROM_LOAD( "jg02.bin", 0x02000, 0x02000, CRC(10e7abfe) SHA1(3f5e0c5911baac19c381686e55f207166fe67d44) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "jg3_g.bin", 0x00, 0x20,  CRC(d389549d) SHA1(763486052b34f8a38247820109b114118a9f829f) )
ROM_END

GAME( 1983, jangou, 0, jangou, jangou, 0, ROT0, "Nichibutsu", "Jangou", GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL )
