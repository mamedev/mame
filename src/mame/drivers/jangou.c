/*******************************************************************************************

Jangou (c) 1983 Nichibutsu

driver by David Haywood,Angelo Salese and Phil Bennett

TODO:
-convert the video hw to true 4bpp;
-some charset transparency pens are wrong,likely that the buffer trigger isn't correct;
-unemulated screen flipping;
-jngolady: if you do a soft reset sometimes the z80<->mcu communication fails,causing a
 black screen after the title screen.

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

---

Jangou Lady
(c)1984 Nihon Bussan

CPU:    Z80 x2 (#1,#2)
    (40pin unknown:#3)
SOUND:  AY-3-8910
    MSM5218RS
OSC:    19.968MHz
    400KHz


1.5N    chr.
2.5M
3.5L

4.9P    Z80#1 prg.
5.9N
6.9M
7.9L

8.9H    Z80#2 prg.
9.9G
10.9F
11.9E
12.9D

M13.13  CPU#3 prg. (?)

JL.3G   color

*******************************************************************************************/

#include "driver.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"
#include "sound/msm5205.h"

#define MASTER_CLOCK	XTAL_19_968MHz

static UINT8 *blit_buffer;
static UINT8 pen_data[0x10];
static UINT8 blit_data[6];

/* Jangou CVSD Sound */
static emu_timer *cvsd_bit_timer;
static UINT8 cvsd_shiftreg;
static int cvsd_shift_cnt;

/* Jangou Lady ADPCM Sound */
static UINT8 adpcm_byte;
static int msm5205_vclk_toggle;


/*************************************
 *
 *  Video Hardware
 *
 *************************************/

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
//      printf("%02x %02x %02x %02x %02x %02x\n",blit_data[0],blit_data[1],blit_data[2],blit_data[3],blit_data[4],blit_data[5]);
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


/*************************************
 *
 *  I/O
 *
 *************************************/

static UINT8 mux_data;

static WRITE8_HANDLER( mux_w )
{
	mux_data = ~data;
}

static WRITE8_HANDLER( output_w )
{
	/*
    --x- ---- ? (polls between high and low in irq routine,probably signals the vblank routine)
    ---- -x-- flip screen
    ---- ---x coin counter
    */
//  printf("%02x\n",data);
	coin_counter_w(0,data & 0x01);
//  flip_screen_set(data & 0x04);
//  coin_lockout_w(0,~data & 0x20);
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
//  printf("%04x\n",mux_data);

	return 0xff;
}

static READ8_HANDLER( input_system_r )
{
	return input_port_read(space->machine, "SYSTEM");
}


/*************************************
 *
 *  Sample Player CPU
 *
 *************************************/

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

/* Jangou HC-55516 CVSD */
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
	if ((++cvsd_shift_cnt & 7) == 0)
		cpu_set_input_line(machine->cpu[1], 0, HOLD_LINE);
}


/* Jangou Lady MSM5218 (MSM5205-compatible) ADPCM */
static WRITE8_HANDLER( adpcm_w )
{
	adpcm_byte = data;
}

static void jngolady_vclk_cb(const device_config *device)
{
	if (msm5205_vclk_toggle == 0)
	{
		msm5205_data_w(0, adpcm_byte >> 4);
	}
	else
	{
		msm5205_data_w(0, adpcm_byte & 0xf);
		cpu_set_input_line(device->machine->cpu[1], 0, HOLD_LINE);
	}

	msm5205_vclk_toggle ^= 1;
}


/*************************************
 *
 *  Jangou Lady NSC8105 CPU
 *
 *************************************/

static UINT8 nsc_latch;

static READ8_HANDLER( master_com_r )
{
	return nsc_latch;
}

static WRITE8_HANDLER( master_com_w )
{
	cpu_set_input_line(space->machine->cpu[2], 0, HOLD_LINE);
	nsc_latch = data;
}

static READ8_HANDLER( slave_com_r )
{
	return nsc_latch;
}

static WRITE8_HANDLER( slave_com_w )
{
	nsc_latch = data;
//  cpu_set_input_line(space->machine->cpu[0], 0, HOLD_LINE);
}

/*************************************
 *
 *  Jangou Memory Map
 *
 *************************************/

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


/*************************************
 *
 *  Jangou Lady Memory Map
 *
 *************************************/

static ADDRESS_MAP_START( jngolady_cpu0_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x9fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xe000, 0xe000) AM_READWRITE(master_com_r,master_com_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( jngolady_cpu1_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_WRITENOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( jngolady_cpu1_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x00) AM_READ(sound_latch_r)
	AM_RANGE(0x01,0x01) AM_WRITE(adpcm_w)
	AM_RANGE(0x02,0x02) AM_WRITE(SMH_NOP)
ADDRESS_MAP_END


static ADDRESS_MAP_START( nsc_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_RAM //internal ram for irq etc.
	AM_RANGE(0x8000, 0x8000) AM_WRITENOP //write-only,irq related?
	AM_RANGE(0x9000, 0x9000) AM_READWRITE(slave_com_r,slave_com_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE(1)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Input Port Definitions
 *
 *************************************/

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

static INPUT_PORTS_START( jngolady )
	PORT_INCLUDE( jangou )

	PORT_MODIFY("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start / P1 Mahjong Flip Flop")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("P1 Ready")

	PORT_MODIFY("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P2 Start / P2 Mahjong Flip Flop")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_NAME("P2 Ready")
INPUT_PORTS_END

/*************************************
 *
 *  Sound HW Config
 *
 *************************************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_mux_r,
	input_system_r,
	NULL,
	NULL
};

static const msm5205_interface msm5205_config =
{
	jngolady_vclk_cb,
	MSM5205_S96_4B
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

static MACHINE_DRIVER_START( jngolady )
	/* basic machine hardware */
	MDRV_IMPORT_FROM(jangou)

	MDRV_CPU_MODIFY("cpu0")
	MDRV_CPU_PROGRAM_MAP(0, jngolady_cpu0_map)

	MDRV_CPU_MODIFY("cpu1")
	MDRV_CPU_PROGRAM_MAP(0, jngolady_cpu1_map)
	MDRV_CPU_IO_MAP(0, jngolady_cpu1_io)

	MDRV_CPU_ADD("nsc", NSC8105, MASTER_CLOCK / 8)
	MDRV_CPU_PROGRAM_MAP(nsc_map, 0)

	/* sound hardware */
	MDRV_SOUND_START(NULL)
	MDRV_SOUND_REMOVE("cvsd")

	MDRV_SOUND_ADD("msm", MSM5205, XTAL_400kHz)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
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

ROM_START( jngolady )
	ROM_REGION( 0xa000, "cpu0", 0 )
	ROM_LOAD( "8.9h",  0x08000, 0x02000, CRC(69e31165) SHA1(81b166c101136ed453a4f4cd88445eb1da5dd0aa) )
	ROM_LOAD( "9.9g",  0x06000, 0x02000, CRC(2faba771) SHA1(d88d0673c9b8cf3783b23c7290253475c9bf397e) )
	ROM_LOAD( "10.9f", 0x04000, 0x02000, CRC(dd311ff9) SHA1(be39ed25343796dc062a612fe82ca19ceb06a9e7) )
	ROM_LOAD( "11.9e", 0x02000, 0x02000, CRC(66cad038) SHA1(c60713615d58a9888e21bfec62fee53558a98eaa) )
	ROM_LOAD( "12.9d", 0x00000, 0x02000, CRC(99c5cc06) SHA1(3a9b3810bb542e252521923ec3026f10f176fa82) )

	ROM_REGION( 0x10000, "cpu1", 0 )
	ROM_LOAD( "4.9p", 0x00000, 0x02000, CRC(34cc2c71) SHA1(b407fed069baf3df316f0006a559a6c5e0be5bd0) )
	ROM_LOAD( "5.9n", 0x02000, 0x02000, CRC(42ed7832) SHA1(2681a532049fee494e1d1779d9dc08b17ce6e134) )
	ROM_LOAD( "6.9m", 0x04000, 0x02000, CRC(9e0e7ef4) SHA1(c68d30e60377c1027f4f053c528a80df09b8ee08) )
	ROM_LOAD( "7.9l", 0x06000, 0x02000, CRC(048615d9) SHA1(3c79830db8792ae0746513ed9849cc5d43051ed6) )

	ROM_REGION( 0x10000, "nsc", 0 )
	ROM_LOAD( "m13.13", 0x0f000, 0x01000, CRC(5b20b0e2) SHA1(228d2d931e6daab3572a1f128b5686f84b6a5a29) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "jl.3g", 0x00, 0x20, CRC(15ffff8c) SHA1(5782697f9c9a6bb04bbf7824cd49033c962899f0) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "1.5n", 0x00000, 0x02000, CRC(54027dee) SHA1(0616c12dbf3a0515cf4312fc5e238a61c97f8084) )
	ROM_LOAD( "2.5m", 0x02000, 0x02000, CRC(323dfad5) SHA1(5908acbf80e4b609ee8e5c313ac99717860dd19c) )
	ROM_LOAD( "3.5l", 0x04000, 0x04000, CRC(14688574) SHA1(241eaf1838239e38d11dff3556fb0a609a4b46aa) )
ROM_END

GAME( 1983, jangou,    0,    jangou,   jangou,    0, ROT0, "Nichibutsu", "Jangou",      GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL )
GAME( 1984, jngolady,  0,    jngolady, jngolady,  0, ROT0, "Nichibutsu", "Jangou Lady", GAME_IMPERFECT_GRAPHICS | GAME_NO_COCKTAIL )
