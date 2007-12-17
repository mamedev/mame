/***************************************************************************

Super Tank
(c) 1981 Video Games GmbH

Reverse-engineering and MAME driver by Norbert Kehrer (December 2003).

****************************************************************************


Hardware:
---------

CPU     TMS9980 (Texas Instruments)
Sound chip  AY-3-8910


Memory map:
-----------

>0000 - >07ff   Fixed ROM
>0800 - >17ff   Bank-switched ROM
>1800 - >1bff   RAM
>1efc       Input port 1
>1efd       Input port 2
>1efe       Read:  DIP switch port 1
        Write: Control port of AY-3-8910 sound chip
>1eff       Read: DIP switch port 2
        Write: Data port of AY-3-8910
>2000 - >3fff   Video RAM (CPU view of it, in reality there are 24 KB on the video board)


Input ports:
------------

Input port 0, mapped to memory address 0x1efc:
7654 3210
0          Player 2 Right
 0         Player 2 Left
  0        Player 2 Down
   0       Player 2 Up
     0     Player 1 Right
      0    Player 1 Left
       0   Player 1 Down
        0  Player 1 Up

Input port 1, mapped to memory address 0x1efd:
7654 3210
0          Player 2 Fire
 0         Player 1 Fire
  0        ??
   0       ??
     0     Start 1 player game
      0    Start 2 players game
       0   Coin (strobe)
        0  ??


DIP switch ports:
-----------------

DIP switch port 1, mapped to memory address 0x1efe:
7654 3210
0          Not used (?)
 0         Not used (?)
  0        Tanks per player: 1 = 5 tanks, 0 = 3 tanks
   0       Extra tank: 1 = at 10,000, 0 = at 15,000 pts.
     000   Coinage
        0  Not used (?)

DIP switch port 2, mapped to memory address 0x1eff:
7654 3210
1          ??
 1         ??
  1        ??
   1       ??
     1     ??
      1    ??
       1   ??
        1  ??


CRU lines:
----------

>400    Select bitplane for writes into video RAM (bit 0)
>401    Select bitplane for writes into video RAM (bit 1)
>402    ROM bank selector (bit 0)
>404    ROM bank selector (bit 1)
>406    Interrupt acknowledge (clears interrupt line)
>407    Watchdog reset (?)
>b12    Unknown, maybe some special-hardware sound effect or lights blinking (?)
>b13    Unknown, maybe some special-hardware sound effect or lights blinking (?)

***************************************************************************/


#include "driver.h"
#include "sound/ay8910.h"



/* the color PROM is 32 bytes, but it is a repeating
   every 8 bytes */
#define NUM_PENS	(8)


static UINT8 *supertnk_videoram[3];
static size_t supertnk_videoram_size;
static UINT8 supertnk_rom_bank;
static UINT8 supertnk_bitplane_select;



/*************************************
 *
 *  Memory banking
 *
 *************************************/

static WRITE8_HANDLER( supertnk_bankswitch_0_w )
{
	offs_t bank_address;

	supertnk_rom_bank = (supertnk_rom_bank & 0x02) | ((data << 0) & 0x01);

	bank_address = 0x10000 + (supertnk_rom_bank * 0x1000);

	memory_set_bankptr(1, &memory_region(REGION_CPU1)[bank_address]);
}


static WRITE8_HANDLER( supertnk_bankswitch_1_w )
{
	offs_t bank_address;

	supertnk_rom_bank = (supertnk_rom_bank & 0x01) | ((data << 1) & 0x02);

	bank_address = 0x10000 + (supertnk_rom_bank * 0x1000);

	memory_set_bankptr(1, &memory_region(REGION_CPU1)[bank_address]);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static INTERRUPT_GEN( supertnk_interrupt )
{
	/* On a TMS9980, a 6 on the interrupt bus means a level 4 interrupt */
	cpunum_set_input_line_and_vector(0, 0, ASSERT_LINE, 6);
}


static WRITE8_HANDLER( supertnk_interrupt_ack_w )
{
	cpunum_set_input_line(0, 0, CLEAR_LINE);
}



/*************************************
 *
 *  Video system
 *
 *************************************/

static VIDEO_START( supertnk )
{
	supertnk_videoram[0] = auto_malloc(supertnk_videoram_size);
	supertnk_videoram[1] = auto_malloc(supertnk_videoram_size);
	supertnk_videoram[2] = auto_malloc(supertnk_videoram_size);
}


static WRITE8_HANDLER( supertnk_videoram_w )
{
	if (supertnk_bitplane_select > 2)
	{
		supertnk_videoram[0][offset] = 0;
		supertnk_videoram[1][offset] = 0;
		supertnk_videoram[2][offset] = 0;
	}
	else
	{
		supertnk_videoram[supertnk_bitplane_select][offset] = data;
	}
}


static READ8_HANDLER( supertnk_videoram_r )
{
	UINT8 ret = 0x00;

	if (supertnk_bitplane_select < 3)
		ret = supertnk_videoram[supertnk_bitplane_select][offset];

	return ret;
}


static WRITE8_HANDLER( supertnk_bitplane_select_0_w )
{
	supertnk_bitplane_select = (supertnk_bitplane_select & 0x02) | ((data << 0) & 0x01);
}


static WRITE8_HANDLER( supertnk_bitplane_select_1_w )
{
	supertnk_bitplane_select = (supertnk_bitplane_select & 0x01) | ((data << 1) & 0x02);
}


static void get_pens(pen_t *pens)
{
	offs_t i;

	for (i = 0; i < NUM_PENS; i++)
	{
		UINT8 data = memory_region(REGION_PROMS)[i];

		pens[i] = MAKE_RGB(pal1bit(data >> 2), pal1bit(data >> 5), pal1bit(data >> 6));
	}
}


static VIDEO_UPDATE( supertnk )
{
	pen_t pens[NUM_PENS];
	offs_t offs;

	get_pens(pens);

	for (offs = 0; offs < supertnk_videoram_size; offs++)
	{
		int i;

		UINT8 y = offs >> 5;
		UINT8 x = offs << 3;

		UINT8 data0 = supertnk_videoram[0][offs];
		UINT8 data1 = supertnk_videoram[1][offs];
		UINT8 data2 = supertnk_videoram[2][offs];

		for (i = 0; i < 8; i++)
		{
			UINT8 color = ((data0 & 0x80) >> 5) | ((data1 & 0x80) >> 6) | ((data2 & 0x80) >> 7);
			*BITMAP_ADDR32(bitmap, y, x) = pens[color];

			data0 = data0 << 1;
			data1 = data1 << 1;
			data2 = data2 << 1;

			x = x + 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

static MACHINE_RESET( supertnk )
{
	supertnk_bankswitch_0_w(0, 0);
	supertnk_bankswitch_1_w(0, 0);

	supertnk_bitplane_select_0_w(0, 0);
	supertnk_bitplane_select_1_w(0, 0);
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( supertnk_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x17ff) AM_ROMBANK(1)
	AM_RANGE(0x1800, 0x1bff) AM_RAM
	AM_RANGE(0x1efc, 0x1efc) AM_READ(input_port_0_r)
	AM_RANGE(0x1efd, 0x1efd) AM_READ(input_port_1_r)
	AM_RANGE(0x1efe, 0x1efe) AM_READWRITE(input_port_2_r, AY8910_control_port_0_w)
	AM_RANGE(0x1eff, 0x1eff) AM_READWRITE(input_port_3_r, AY8910_write_port_0_w)
	AM_RANGE(0x2000, 0x3fff) AM_READWRITE(supertnk_videoram_r, supertnk_videoram_w) AM_SIZE(&supertnk_videoram_size)
ADDRESS_MAP_END



/*************************************
 *
 *  Port handlers
 *
 *************************************/

static ADDRESS_MAP_START( supertnk_io_map, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x0000) AM_WRITE(MWA8_NOP)
	AM_RANGE(0x0400, 0x0400) AM_WRITE(supertnk_bitplane_select_0_w)
	AM_RANGE(0x0401, 0x0401) AM_WRITE(supertnk_bitplane_select_1_w)
	AM_RANGE(0x0402, 0x0402) AM_WRITE(supertnk_bankswitch_0_w)
	AM_RANGE(0x0404, 0x0404) AM_WRITE(supertnk_bankswitch_1_w)
	AM_RANGE(0x0406, 0x0406) AM_WRITE(supertnk_interrupt_ack_w)
	AM_RANGE(0x0407, 0x0407) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( supertnk )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x02, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0x0a, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(	0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(	0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(	0x04, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(	0x00, "at 15,000 points" )
	PORT_DIPSETTING(	0x10, "at 10,000 points" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING( 	0x00, "3" )
	PORT_DIPSETTING(	0x20, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80, DEF_STR( On ) )

	PORT_START
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x80, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( supertnk )

	/* basic machine hardware */
	MDRV_CPU_ADD(TMS9980, 2598750) /* ? to which frequency is the 20.79 Mhz crystal mapped down? */
	MDRV_CPU_PROGRAM_MAP(supertnk_map,0)
	MDRV_CPU_IO_MAP(supertnk_io_map,0)
	MDRV_CPU_VBLANK_INT(supertnk_interrupt,1)

	MDRV_MACHINE_RESET(supertnk)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_VIDEO_START(supertnk)
	MDRV_VIDEO_UPDATE(supertnk)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_60HZ_VBLANK_DURATION)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 2000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( supertnk )
	ROM_REGION( 0x14000, REGION_CPU1, 0 ) /* 64k for TMS9980 code + 16k of ROM */
	ROM_LOAD( "supertan.2d",  0x00000, 0x0800, CRC(1656a2c1) SHA1(1d49945aed105003a051cfbf646af7a4be1b7e86) )
	ROM_LOAD( "supertnk.3d",  0x10800, 0x0800, CRC(8b023a9a) SHA1(1afdc8d75f2ca04153bac20c0e3e123e2a7acdb7) )
	ROM_CONTINUE(			  0x10000, 0x0800)
	ROM_LOAD( "supertnk.4d",  0x11800, 0x0800, CRC(b8249e5c) SHA1(ef4bb714b0c1b97890a067f05fc50ab3426ce37f) )
	ROM_CONTINUE(			  0x11000, 0x0800)
	ROM_LOAD( "supertnk.8d",  0x12800, 0x0800, CRC(d8175a4f) SHA1(cba7b426773ac86c81a9eac81087a2db268cd0f9) )
	ROM_CONTINUE(			  0x12000, 0x0800)
	ROM_LOAD( "supertnk.9d",  0x13800, 0x0800, CRC(a34a494a) SHA1(9b7f0560e9d569ee25eae56f31886d50a3153dcc) )
	ROM_CONTINUE(			  0x13000, 0x0800)

	ROM_REGION( 0x0060, REGION_PROMS, 0 )
	 /* color PROM */
	ROM_LOAD( "supertnk.clr",  0x0000, 0x0020, CRC(9ae1faee) SHA1(19de4bb8bc389d98c8f8e35c755fad96e1a6a0cd) )
 	/* unknown - sync? */
	ROM_LOAD( "supertnk.s",	   0x0020, 0x0020, CRC(91722fcf) SHA1(f77386014b459cc151d2990ac823b91c04e8d319) )
	/* unknown - sync? */
	ROM_LOAD( "supertnk.t",	   0x0040, 0x0020, CRC(154390bd) SHA1(4dc0fd7bd8999d2670c8d93aaada835d2a84d4db) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

static DRIVER_INIT( supertnk )
{
	/* decode the TMS9980 ROMs */
	offs_t offs;

	for (offs = 0; offs < memory_region_length(REGION_CPU1); offs++)
	{
		memory_region(REGION_CPU1)[offs] = BITSWAP8(memory_region(REGION_CPU1)[offs],0,1,2,3,4,5,6,7);
	};
}


GAME( 1981, supertnk, 0, supertnk, supertnk, supertnk, ROT90, "Video Games GmbH", "Super Tank", 0 )
