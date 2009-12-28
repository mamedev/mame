/*********************************************************************
 Erotictac/Tactic [Sisteme 1992]
 (title depends on "Sexy Views" DIP)

 driver by
  Tomasz Slanina
  Steve Ellenoff
    Nicola Salmoria

TODO:
 - sound
    my guess:
     - data (samples?) in mem range 0000-$0680 ($1f00000 - $1f00680)
     - 4 chn?? ( masks $000000ff, $0000ff00, $00ff0000, $ff000000)
     - INT7 (bit 1 in IRQRQB) = sound hw 'ready' flag (checked at $56c)
     - writes to 3600000 - 3ffffff related to snd hardware

 - game doesn't work with 'demo sound' disabled
 - get rid of dirty hack in DRIVER_INIT (bug in the ARM core?)
 - is screen double buffered ?
 - flashing text in test mode

  Poizone PCB appears to be a Jamam Acorn Archimedes based system,
   Poizone was also released on the AA computer.

 - Poizone controls are checked exactly once at bootup, breaking them.

**********************************************************************/
#include "driver.h"
#include "cpu/arm/arm.h"

#define NUM_PENS	(0x100)

static UINT32 *ertictac_mainram;
static UINT32 *ertictac_videoram;
static UINT32 IRQSTA, IRQMSKA, IRQMSKB, FIQMSK, T1low, T1high;
static UINT32 vidFIFO[256];
static pen_t pens[NUM_PENS];

static WRITE32_HANDLER(video_fifo_w)
{
	vidFIFO[data >> 24] = data & 0xffffff;
}

static READ32_HANDLER(IOCR_r)
{
	return (input_port_read(space->machine, "dummy") & 0x80) | 0x34;
}

static WRITE32_HANDLER(IOCR_w)
{
	//ignored
}


static READ32_HANDLER(IRQSTA_r)
{
	return (IRQSTA & (~2)) | 0x80;
}

static READ32_HANDLER(IRQRQA_r)
{
	return (IRQSTA & IRQMSKA) | 0x80;
}

static WRITE32_HANDLER(IRQRQA_w)
{
	if(ACCESSING_BITS_0_7)
		IRQSTA &= ~data;
}

static READ32_HANDLER(IRQMSKA_r)
{
	return IRQMSKA;
}

static WRITE32_HANDLER(IRQMSKA_w)
{
	if(ACCESSING_BITS_0_7)
		IRQMSKA = (data & (~2)) | 0x80;
}

static READ32_HANDLER(IRQRQB_r)
{
	return mame_rand(space->machine) & IRQMSKB; /* hack  0x20 - controls,  0x02 - ?sound? */
}

static READ32_HANDLER(IRQMSKB_r)
{
	return IRQMSKB;
}

static WRITE32_HANDLER(IRQMSKB_w)
{
	if(ACCESSING_BITS_0_7)
		IRQMSKB = data;
}

static READ32_HANDLER(FIQMSK_r)
{
	return FIQMSK;
}

static WRITE32_HANDLER(FIQMSK_w)
{
	if(ACCESSING_BITS_0_7)
		FIQMSK = (data & (~0x2c)) | 0x80;
}

static READ32_HANDLER(T1low_r)
{
	return T1low;
}

static WRITE32_HANDLER(T1low_w)
{
	if(ACCESSING_BITS_0_7)
		T1low = data;
}

static READ32_HANDLER(T1high_r)
{
	return T1high;
}

static WRITE32_HANDLER(T1high_w)
{
	if(ACCESSING_BITS_0_7)
		T1high = data;
}

static void startTimer(running_machine *machine);

static TIMER_CALLBACK( ertictacTimer )
{
	IRQSTA |= 0x40;
	if(IRQMSKA & 0x40)
	{
		cputag_set_input_line(machine, "maincpu", ARM_IRQ_LINE, HOLD_LINE);
	}
	startTimer(machine);
}

static void startTimer(running_machine *machine)
{
	timer_set(machine, ATTOTIME_IN_USEC(((T1low & 0xff) | ((T1high & 0xff) << 8)) >> 4), NULL, 0, ertictacTimer);
}

static WRITE32_HANDLER(T1GO_w)
{
	startTimer(space->machine);
}

static ADDRESS_MAP_START( ertictac_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0007ffff) AM_RAM AM_BASE (&ertictac_mainram)
	AM_RANGE(0x01f00000, 0x01ffffff) AM_RAM AM_BASE (&ertictac_videoram)
	AM_RANGE(0x03200000, 0x03200003) AM_READWRITE(IOCR_r, IOCR_w)
	AM_RANGE(0x03200010, 0x03200013) AM_READ(IRQSTA_r)
	AM_RANGE(0x03200014, 0x03200017) AM_READWRITE(IRQRQA_r, IRQRQA_w)
	AM_RANGE(0x03200018, 0x0320001b) AM_READWRITE(IRQMSKA_r, IRQMSKA_w)
	AM_RANGE(0x03200024, 0x03200027) AM_READ(IRQRQB_r)
	AM_RANGE(0x03200028, 0x0320002b) AM_READWRITE(IRQMSKB_r, IRQMSKB_w)
	AM_RANGE(0x03200038, 0x0320003b) AM_READWRITE(FIQMSK_r, FIQMSK_w)

	AM_RANGE(0x03200050, 0x03200053) AM_READWRITE(T1low_r, T1low_w)
	AM_RANGE(0x03200054, 0x03200057) AM_READWRITE(T1high_r, T1high_w)
	AM_RANGE(0x03200058, 0x0320005b) AM_WRITE( T1GO_w )

	AM_RANGE(0x03340000, 0x03340003) AM_NOP
	AM_RANGE(0x03340010, 0x03340013) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x03340014, 0x03340017) AM_READ_PORT("P2")
	AM_RANGE(0x03340018, 0x0334001b) AM_READ_PORT("P1")

	AM_RANGE(0x033c0004, 0x033c0007) AM_READ_PORT("DSW1")
	AM_RANGE(0x033c0008, 0x033c000b) AM_READ_PORT("DSW2")
	AM_RANGE(0x033c0010, 0x033c0013) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x033c0014, 0x033c0017) AM_READ_PORT("P2")
	AM_RANGE(0x033c0018, 0x033c001b) AM_READ_PORT("P1")

	AM_RANGE(0x03400000, 0x03400003) AM_WRITE(video_fifo_w)
	AM_RANGE(0x03800000, 0x03ffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( ertictac )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPNAME( 0x02, 0x02, "Demo Sound" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Test Mode" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Music" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x30, "Game Timing" )
	PORT_DIPSETTING(    0x30, "Normal Game" )
	PORT_DIPSETTING(    0x20, "3.0 Minutes" )
	PORT_DIPSETTING(    0x10, "2.5 Minutes" )
	PORT_DIPSETTING(    0x00, "2.0 Minutes" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x05, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_DIPNAME( 0x0a, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_4C ) )

	PORT_DIPNAME( 0x10, 0x00, "Sexy Views" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("dummy")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END

static INPUT_PORTS_START( poizone )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Language ) ) // 01
	PORT_DIPSETTING(    0x01, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( French ) )
	PORT_DIPNAME( 0x02, 0x02, "1-2" ) // 02
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Service_Mode ) ) // 04
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Demo Sound" ) // 08
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x40, "Coinage 1 (1-5, 1-6)" ) // 10 20
	PORT_DIPSETTING(	0x30, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(	0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(	0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xC0, 0x00, "Coinage 2 (1-7, 1-8)" ) // 40 80
	PORT_DIPSETTING(	0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(	0xC0, DEF_STR( 1C_4C ) )

	PORT_START("DSW2") /* DSW 2 doesn't work, may not be hooked up properly */
//  PORT_DIPNAME( 0x01, 0x01, "2-1" )
//  PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x01, 0x02, "2-2" )
//  PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x01, 0x04, "2-3" )
//  PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x01, 0x08, "2-4" )
//  PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x01, 0x10, "2-5" )
//  PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x01, 0x20, "2-6" )
//  PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x01, 0x40, "2-7" )
//  PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )
//  PORT_DIPNAME( 0x01, 0x80, "2-8" )
//  PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("dummy")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END

static MACHINE_RESET( ertictac )
{
	ertictac_mainram[0]=0xeae00007; //reset vector
}

static INTERRUPT_GEN( ertictac_interrupt )
{
	IRQSTA|=0x08;
	if(IRQMSKA&0x08)
	{
		cpu_set_input_line(device, ARM_IRQ_LINE, HOLD_LINE);
	}
}


static VIDEO_START( ertictac )
{
	int color;

	for (color = 0; color < NUM_PENS; color++)
	{
		UINT8 i = color & 0x03;
		UINT8 r = ((color & 0x04) >> 0) | ((color & 0x10) >> 1) | i;
		UINT8 g = ((color & 0x20) >> 3) | ((color & 0x40) >> 3) | i;
		UINT8 b = ((color & 0x08) >> 1) | ((color & 0x80) >> 4) | i;

		pens[color] = MAKE_RGB(pal4bit(r), pal4bit(g), pal4bit(b));
	}
}


static VIDEO_UPDATE( ertictac )
{
	int y, x;

	for (y = cliprect->min_y; y <= cliprect->max_y; y++)
		for (x = 0; x < 0x140; x += 4)
		{
			offs_t offs = (y * 0x50) + (x >> 2) + (vidFIFO[0x88] >> 2);

			*BITMAP_ADDR32(bitmap, y, x + 0) = pens[(ertictac_videoram[offs] >>  0) & 0xff];
			*BITMAP_ADDR32(bitmap, y, x + 1) = pens[(ertictac_videoram[offs] >>  8) & 0xff];
			*BITMAP_ADDR32(bitmap, y, x + 2) = pens[(ertictac_videoram[offs] >> 16) & 0xff];
			*BITMAP_ADDR32(bitmap, y, x + 3) = pens[(ertictac_videoram[offs] >> 24) & 0xff];
		}

	return 0;
}


static MACHINE_DRIVER_START( ertictac )

	MDRV_CPU_ADD("maincpu", ARM, 16000000) /* guess */
	MDRV_CPU_PROGRAM_MAP(ertictac_map)
	MDRV_CPU_VBLANK_INT("screen", ertictac_interrupt)

	MDRV_MACHINE_RESET(ertictac)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(320, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 255)

	MDRV_VIDEO_START(ertictac)
	MDRV_VIDEO_UPDATE(ertictac)
MACHINE_DRIVER_END

ROM_START( ertictac )
	ROM_REGION(0x800000, "user1", 0 )
	ROM_LOAD32_BYTE( "01", 0x00000, 0x10000, CRC(8dce677c) SHA1(9f12b1febe796038caa1ecad1d17864dc546cfd8) )
	ROM_LOAD32_BYTE( "02", 0x00001, 0x10000, CRC(7c5c916c) SHA1(d4ed5fc3a7b27253551e7d9176ed9e6513092e60) )
	ROM_LOAD32_BYTE( "03", 0x00002, 0x10000, CRC(edca5ac6) SHA1(f6c4b8030f3c1c93922c5f7232f2159e0471b93a) )
	ROM_LOAD32_BYTE( "04", 0x00003, 0x10000, CRC(959be81c) SHA1(3e8eaacf8809863fd712ad605d23fdda4e904aee) )
	ROM_LOAD32_BYTE( "05", 0x40000, 0x10000, CRC(d08a6c89) SHA1(17b0f5fb2094126146b09d69c91bf413737b0c9e) )
	ROM_LOAD32_BYTE( "06", 0x40001, 0x10000, CRC(d727bcd8) SHA1(4627f8d4d6e6f323445b3ffcfc0d9c699a9a4f89) )
	ROM_LOAD32_BYTE( "07", 0x40002, 0x10000, CRC(23b75de2) SHA1(e18f5339ca2dd362298784ff3e5479d780d823f8) )
	ROM_LOAD32_BYTE( "08", 0x40003, 0x10000, CRC(9448ccdf) SHA1(75fe3c31625f8ba1eedd806b52993e92b1f585b6) )
	ROM_LOAD32_BYTE( "09", 0x80000, 0x10000, CRC(2bfb312e) SHA1(af7a4a1926c9c3d0b3ad41a4729a17383581c070) )
	ROM_LOAD32_BYTE( "10", 0x80001, 0x10000, CRC(e7a05477) SHA1(0ec6f94a35b87e1e4529dea194fce1fde9a5b0ad) )
	ROM_LOAD32_BYTE( "11", 0x80002, 0x10000, CRC(3722591c) SHA1(e0c4075bc4b1c90a6decba3005a1f8298bd61bd1) )
	ROM_LOAD32_BYTE( "12", 0x80003, 0x10000, CRC(fe022b7e) SHA1(056f7283bc54eff555fd1db91410c020fd905063) )
	ROM_LOAD32_BYTE( "13", 0xc0000, 0x10000, CRC(83550842) SHA1(0fee78dbf13ba970e0544c48f8939550f9347822) )
	ROM_LOAD32_BYTE( "14", 0xc0001, 0x10000, CRC(3029567c) SHA1(6d49bea3a3f6f11f4182a602d37b53f1f896c154) )
	ROM_LOAD32_BYTE( "15", 0xc0002, 0x10000, CRC(500997ab) SHA1(028c7b3ca03141e5b596ab1e2ab98d0ccd9bf93a) )
	ROM_LOAD32_BYTE( "16", 0xc0003, 0x10000, CRC(70a8d136) SHA1(50b11f5701ed5b79a5d59c9a3c7d5b7528e66a4d) )
ROM_END

ROM_START( poizone )
	ROM_REGION(0x800000, "user1", 0 )
	ROM_LOAD32_BYTE( "p_son01.bin", 0x00000, 0x10000, CRC(28793c9f) SHA1(2d9f7d667203e745b47cd2cc97501ae961ae1a66) )
	ROM_LOAD32_BYTE( "p_son02.bin", 0x00001, 0x10000, CRC(2d4b6f4b) SHA1(8df2680d6e5dc41787b3a72e594f01f5e732d0ec) )
	ROM_LOAD32_BYTE( "p_son03.bin", 0x00002, 0x10000, CRC(0834d46e) SHA1(bf1cc9b47759ef39ed8fd8f334ed8f2902be3bf8) )
	ROM_LOAD32_BYTE( "p_son04.bin", 0x00003, 0x10000, CRC(9e9b1f6e) SHA1(d95067f3ecca3c079a67bd0b80e3b45c5b42151e) )
	ROM_LOAD32_BYTE( "p_son05.bin", 0x40000, 0x10000, CRC(be62ad42) SHA1(5eb51ad277ec7b7f1b5995bcdea35114f805baae) )
	ROM_LOAD32_BYTE( "p_son06.bin", 0x40001, 0x10000, CRC(c2f9141c) SHA1(e910fefcd6f0b99ab299b3a5f099b9ef84e1cc23) )
	ROM_LOAD32_BYTE( "p_son07.bin", 0x40002, 0x10000, CRC(8929c748) SHA1(35c108170590fbe97fdd4a1db7d660b4ee0adac8) )
	ROM_LOAD32_BYTE( "p_son08.bin", 0x40003, 0x10000, CRC(0ef5b14f) SHA1(425f130b2a94a4152fab763e0734e71f2913b25f) )
	ROM_LOAD32_BYTE( "p_son09.bin", 0x80000, 0x10000, CRC(e8cd75a6) SHA1(386a4ff576574e49711e72640dd3f33c8b7e04b3) )
	ROM_LOAD32_BYTE( "p_son10.bin", 0x80001, 0x10000, CRC(1dc01da7) SHA1(d37456e3407cab5eff5bbd9735c3a54e73b27545) )
	ROM_LOAD32_BYTE( "p_son11.bin", 0x80002, 0x10000, CRC(85e973ad) SHA1(850cd0dbda42eab78625038c6ea1f5b31674018a) )
	ROM_LOAD32_BYTE( "p_son12.bin", 0x80003, 0x10000, CRC(b89376d1) SHA1(cff29c2a8db88d4d104bae19a90de034158fe9e7) )

	ROM_LOAD32_BYTE( "p_son21.bin", 0x140000, 0x10000, CRC(a0c06c1e) SHA1(8d065117788e96ecd147d3d7ffdd273d4b69bb7a) )
	ROM_LOAD32_BYTE( "p_son22.bin", 0x140001, 0x10000, CRC(16f0bb52) SHA1(893ab1e72b84de7a38f88f9d713769968ebd4553) )
	ROM_LOAD32_BYTE( "p_son23.bin", 0x140002, 0x10000, CRC(e9c118b2) SHA1(110d9a204e701b9b54d89f027f8892c3f3a819c7) )
	ROM_LOAD32_BYTE( "p_son24.bin", 0x140003, 0x10000, CRC(a09d7f55) SHA1(e0d562c655c16034b40db93de801b98b7948beb2) )
ROM_END

GAME( 1990, ertictac, 0, ertictac, ertictac, 0, ROT0, "Sisteme", "Erotictac/Tactic" ,GAME_NO_SOUND)
GAME( 1991, poizone,  0, ertictac, poizone, 0,  ROT0, "Eterna" ,"Poizone" ,GAME_NO_SOUND|GAME_NOT_WORKING)

