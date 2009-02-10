/*

Go 2000 - Korean Card game

Newer PCB, very sparce with newer surface mounted CPUs

MC68EC000FU10
Z84C0006FEC
TM29F550ZX
OSC: 32.000MHz
2 8-way Dipswitch banks
Ram:
 2 UM61256FK-15 (near 3 & 4 (68k program roms))
 3 Windbond W24257AK-15 (near TM29F550ZX)
 2 UM61256AK-15 (near Z80)

P1, P2 & P3 4-pin connectors (unkown purpose)

2008-08:
Added Dips and Dip locations based on Service Mode.

Notes:
- Maybe SA stands for SunA? The z80 memory map matches the one seen in Ultra Balloon,
  and the only difference stands in the DAC used.

*/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"

static UINT16* go2000_video;
static UINT16* go2000_video2;

static WRITE16_HANDLER( sound_cmd_w )
{
	soundlatch_w(space,offset,data & 0xff);
	cpu_set_input_line(space->machine->cpu[1], 0, HOLD_LINE);
}

static ADDRESS_MAP_START( go2000_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x200000, 0x203fff) AM_RAM
	AM_RANGE(0x600000, 0x60ffff) AM_RAM AM_BASE(&go2000_video)
	AM_RANGE(0x610000, 0x61ffff) AM_RAM AM_BASE(&go2000_video2)
	AM_RANGE(0x800000, 0x800fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE(&paletteram16)
	AM_RANGE(0xa00000, 0xa00001) AM_READ_PORT("INPUTS")
	AM_RANGE(0xa00002, 0xa00003) AM_READ_PORT("DSW")
	AM_RANGE(0x620002, 0x620003) AM_WRITE(sound_cmd_w)
//	AM_RANGE(0xe00000, 0xe00001) AM_WRITENOP
//	AM_RANGE(0xe00010, 0xe00011) AM_WRITENOP
//	AM_RANGE(0xe00020, 0xe00021) AM_WRITENOP
ADDRESS_MAP_END

static WRITE8_HANDLER( go2000_dac_w )
{
	dac_data_w( 0 , data & 0xff );
}

static WRITE8_HANDLER( go2000_pcm_1_bankswitch_w )
{
	UINT8 *RAM = memory_region(space->machine, "sound");
	int bank = data & 7;

	memory_set_bankptr(space->machine, 1, &RAM[bank * 0x10000 + 0x400]);
}

static ADDRESS_MAP_START( go2000_sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0400, 0xffff) AM_ROMBANK(1)
ADDRESS_MAP_END

static ADDRESS_MAP_START( go2000_sound_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_r)
	AM_RANGE(0x00, 0x00) AM_WRITE(go2000_dac_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(go2000_pcm_1_bankswitch_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( go2000 )
	PORT_START("INPUTS")	/* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) // continue
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) // korean symbol
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON3 ) // out
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) // high
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // low
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED) // unused?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED) // unused?
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED) // unused?
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED) // unused?
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 M1")	// m1
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 M2")	// m2
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 M3")	// m3
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED) // unused?
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 ) // coin2

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, "Coin / Credits" ) PORT_DIPLOCATION("SW-1:1,2")
	PORT_DIPSETTING(      0x0000, "1 Coin / 50 Credits" )
	PORT_DIPSETTING(      0x0003, "1 Coin / 100 Credits" )
	PORT_DIPSETTING(      0x0002, "1 Coin / 125 Credits" )
	PORT_DIPSETTING(      0x0001, "1 Coin / 150 Credits" )
	PORT_DIPNAME( 0x000c, 0x000c, "Minimum Coin" ) PORT_DIPLOCATION("SW-1:3,4")
	PORT_DIPSETTING(      0x000c, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x0004, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0010, IP_ACTIVE_LOW, "SW-1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0020, IP_ACTIVE_LOW, "SW-1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x0040, IP_ACTIVE_LOW, "SW-1:7" )
	PORT_SERVICE_DIPLOC( 0x0080, IP_ACTIVE_LOW, "SW-1:8" )
	PORT_DIPNAME( 0x0700, 0x0700, "Difficult-1" ) PORT_DIPLOCATION("SW-2:1,2,3")
	PORT_DIPSETTING(      0x0700, "1" )
	PORT_DIPSETTING(      0x0600, "2" )
	PORT_DIPSETTING(      0x0500, "3" )
	PORT_DIPSETTING(      0x0400, "4" )
	PORT_DIPSETTING(      0x0300, "5" )
	PORT_DIPSETTING(      0x0200, "6" )
	PORT_DIPSETTING(      0x0100, "7" )
	PORT_DIPSETTING(      0x0000, "8" )
	PORT_DIPNAME( 0x1800, 0x1800, "M1 value" ) PORT_DIPLOCATION("SW-2:4,5")
	PORT_DIPSETTING(      0x0800, "3000" ) PORT_CONDITION("DSW", 0x0003, PORTCOND_EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, "3500" ) PORT_CONDITION("DSW", 0x0003, PORTCOND_EQUALS, 0x0000)
	PORT_DIPSETTING(      0x1800, "4000" ) PORT_CONDITION("DSW", 0x0003, PORTCOND_EQUALS, 0x0000)
	PORT_DIPSETTING(      0x1000, "4500" ) PORT_CONDITION("DSW", 0x0003, PORTCOND_EQUALS, 0x0000)
	PORT_DIPSETTING(      0x0800, "6000" ) PORT_CONDITION("DSW", 0x0003, PORTCOND_NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x0000, "7000" ) PORT_CONDITION("DSW", 0x0003, PORTCOND_NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x1800, "8000" ) PORT_CONDITION("DSW", 0x0003, PORTCOND_NOTEQUALS, 0x0000)
	PORT_DIPSETTING(      0x1000, "9000" ) PORT_CONDITION("DSW", 0x0003, PORTCOND_NOTEQUALS, 0x0000)
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, IP_ACTIVE_LOW, "SW-2:6" )
	PORT_DIPNAME( 0xc000, 0xc000, "Difficult-2" ) PORT_DIPLOCATION("SW-2:7,8")
	PORT_DIPSETTING(      0xc000, "1" )
	PORT_DIPSETTING(      0x8000, "2" )
	PORT_DIPSETTING(      0x4000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
INPUT_PORTS_END


static const gfx_layout go2000_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8,12,0,4 },
	{ 3,2,1,0, 19,18,17,16 },
	{ 0*32, 1*32, 2*32, 3*32,4*32,5*32,6*32,7*32 },
	8*32
};

static GFXDECODE_START( go2000 )
	GFXDECODE_ENTRY( "gfx1", 0, go2000_layout,   0x0, 0x80  ) /* tiles */
GFXDECODE_END

static VIDEO_START(go2000)
{
}

static VIDEO_UPDATE(go2000)
{
	int x,y;
	int count = 0;

	/* 0x600000 - 0x601fff / 0x610000 - 0x611fff */
	for (x=0;x<64;x++)
	{
		for (y=0;y<32;y++)
		{
			int tile = go2000_video[count];
			int attr = go2000_video2[count];
			drawgfx(bitmap,screen->machine->gfx[0],tile,attr,0,0,x*8,y*8,cliprect,TRANSPARENCY_NONE,0);
			count++;
		}
	}

	/* 0x602000 - 0x603fff / 0x612000 - 0x613fff */
	for (x=0;x<64;x++)
	{
		for (y=0;y<32;y++)
		{
			int tile = go2000_video[count];
			int attr = go2000_video2[count];
			drawgfx(bitmap,screen->machine->gfx[0],tile,attr,0,0,x*8,y*8,cliprect,TRANSPARENCY_PEN,0xf);
			count++;
		}
	}

	/*Spriteram, TODO*/
	{
		int tile,offs;

		for(offs=0;offs<(0x800/2);offs+=4)
		{
			x = go2000_video[(0xf800/2)+offs+1];
			y = go2000_video[(0xf800/2)+offs+2]-0x30;
			tile = go2000_video[(0xf800/2)+offs+0] & 0x1fff;
			drawgfx(bitmap,screen->machine->gfx[0],tile+0,0,0,0,x+0,y+0,cliprect,TRANSPARENCY_PEN,0xf);
			drawgfx(bitmap,screen->machine->gfx[0],tile+1,0,0,0,x+8,y+0,cliprect,TRANSPARENCY_PEN,0xf);
			drawgfx(bitmap,screen->machine->gfx[0],tile+2,0,0,0,x+0,y+8,cliprect,TRANSPARENCY_PEN,0xf);
			drawgfx(bitmap,screen->machine->gfx[0],tile+3,0,0,0,x+8,y+8,cliprect,TRANSPARENCY_PEN,0xf);

		}
	}

	return 0;
}


static MACHINE_RESET(go2000)
{
	const address_space *space = cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM);
	go2000_pcm_1_bankswitch_w(space, 0, 0);
}

static MACHINE_DRIVER_START( go2000 )
	MDRV_CPU_ADD("main", M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(go2000_map,0)
	MDRV_CPU_VBLANK_INT("main", irq1_line_hold)

	MDRV_CPU_ADD("sound", Z80, 4000000)
	MDRV_CPU_PROGRAM_MAP(go2000_sound_map,0)
	MDRV_CPU_IO_MAP(go2000_sound_io,0)

	MDRV_GFXDECODE(go2000)
	MDRV_MACHINE_RESET(go2000)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(go2000)
	MDRV_VIDEO_UPDATE(go2000)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(0, "left", 0.50)
	MDRV_SOUND_ROUTE(1, "right", 0.50)
MACHINE_DRIVER_END

ROM_START( go2000 )
	ROM_REGION( 0x80000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "3.bin", 0x00000, 0x20000, CRC(fe1fb269) SHA1(266b8acddfcfd960b8e44f8606bf0873da42b9f8) )
	ROM_LOAD16_BYTE( "4.bin", 0x00001, 0x20000, CRC(d6246ae3) SHA1(f2618dcabaa0c0a6e377e4acd1cdec8bea90bea8) )

	ROM_REGION( 0x080000, "sound", 0 ) /* Z80? */
	ROM_LOAD( "5.bin", 0x00000, 0x80000, CRC(a32676ee) SHA1(2dab73497c0818fce479be21ed589985db51560b) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD16_BYTE( "1.bin", 0x00000, 0x20000, CRC(96e50aba) SHA1(caa1aadab855c3a758378dc8c48eec859e8110a4) )
	ROM_LOAD16_BYTE( "2.bin", 0x00001, 0x20000, CRC(b0adf1cb) SHA1(2afb30691182dbf46be709f0d5b03b0f8ff52790) )
ROM_END


GAME( 2000, go2000,    0, go2000,    go2000,    0, ROT0,  "SA", "Go 2000", GAME_IMPERFECT_GRAPHICS )
