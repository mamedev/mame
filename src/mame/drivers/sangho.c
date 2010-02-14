/*

Sang Ho Soft 'Puzzle Star' PCB

Driver by David Haywood, Tomasz Slanina and Mariusz Wojcieszek

Each board contains a custom FGPA on a sub-board with
a warning   "WARNING ! NO TOUCH..." printed on the PCB

A battery is connected to the underside of the sub-board
and if the battery dies the PCB is no-longer functional.

It is possible that important game code is stored within
the battery.

The ROMs for "Puzzle Star" don't appear to have code at 0
and all boards found so far have been dead.

The Sexy Boom board was working, but it may only be a
matter of time before that board dies too.

It is thought that these games are based on MSX hardware
as some of the Puzzle Star roms appear to be a hacked
MSX Bios.  If we're lucky then the FGPA may only contain
Sang Ho's MSX simulation, rather than any specific game code.

The FGPA is labeled 'Custom 3'

There is another covered chip on the PCB labeled 'Custom 2'
at U17.  It is unknown what this chip is.

Custom 1 is underneath the sub-board and is a UM3567 which
is a YM2413 compatible chip.

*** the custom chip with the warning appears to control banking etc.

*/

#include "emu.h"
#include "deprecat.h"
#include "cpu/z80/z80.h"
#include "video/v9938.h"
#include "sound/2413intf.h"

static UINT8* sangho_ram;
static UINT8 sexyboom_bank[8];

static WRITE8_HANDLER(sangho_ram_w)
{
	sangho_ram[offset]=data;
}

static ADDRESS_MAP_START( pzlestar_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_WRITE(sangho_ram_w)
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank2")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank3")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank4")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sexyboom_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank5")
	AM_RANGE(0x4000, 0x7fff) AM_READ_BANK("bank2") AM_WRITE_BANK("bank6")
	AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bank3") AM_WRITE_BANK("bank7")
	AM_RANGE(0xc000, 0xffff) AM_READ_BANK("bank4") AM_WRITE_BANK("bank8")
ADDRESS_MAP_END

/* Wrong ! */
static WRITE8_HANDLER(pzlestar_bank_w)
{
	memory_set_bankptr(space->machine, "bank2",&memory_region(space->machine, "user1")[0x20000+ ( ((0x8000*data)^0x10000))  ]);
	memory_set_bankptr(space->machine, "bank3",&memory_region(space->machine, "user1")[  0x18000  ]);
}

static void sexyboom_map_bank(running_machine *machine, int bank)
{
	UINT8 banknum, banktype;
	char read_bank_name[6], write_bank_name[6];

	banknum = sexyboom_bank[bank*2];
	banktype = sexyboom_bank[bank*2 + 1];
	sprintf(read_bank_name, "bank%d", bank+1);
	sprintf(write_bank_name, "bank%d", bank+1+4);

	if (banktype == 0)
	{
		if (banknum & 0x80)
		{
			// ram
			memory_set_bankptr(machine, read_bank_name, &sangho_ram[(banknum & 0x7f) * 0x4000]);
			memory_install_write_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), bank*0x4000, (bank+1)*0x4000 - 1, 0, 0, write_bank_name );
			memory_set_bankptr(machine, write_bank_name, &sangho_ram[(banknum & 0x7f) * 0x4000]);
		}
		else
		{
			// rom 0
			memory_set_bankptr(machine, read_bank_name, memory_region(machine, "user1")+0x4000*banknum);
			memory_unmap_write(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), bank*0x4000, (bank+1)*0x4000 - 1, 0, 0);
		}
	}
	else if (banktype == 0x82)
	{
		memory_set_bankptr(machine, read_bank_name, memory_region(machine, "user1")+0x20000+banknum*0x4000);
		memory_unmap_write(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), bank*0x4000, (bank+1)*0x4000 - 1, 0, 0);
	}
	else if (banktype == 0x80)
	{
		memory_set_bankptr(machine, read_bank_name, memory_region(machine, "user1")+0x120000+banknum*0x4000);
		memory_unmap_write(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), bank*0x4000, (bank+1)*0x4000 - 1, 0, 0);
	}
	else
	{
		logerror("Unknown bank type %02x\n", banktype);
	}
}

static WRITE8_HANDLER(sexyboom_bank_w)
{
	sexyboom_bank[offset] = data;
	sexyboom_map_bank(space->machine, offset>>1);
}

/* Puzzle Star Ports */

static ADDRESS_MAP_START( pzlestar_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x7c, 0x7d) AM_DEVWRITE( "ymsnd", ym2413_w )
	AM_RANGE( 0x91, 0x91) AM_WRITE( pzlestar_bank_w )
	AM_RANGE( 0x98, 0x98) AM_READWRITE( v9938_0_vram_r, v9938_0_vram_w )
	AM_RANGE( 0x99, 0x99) AM_READWRITE( v9938_0_status_r, v9938_0_command_w )
	AM_RANGE( 0x9a, 0x9a) AM_WRITE( v9938_0_palette_w )
	AM_RANGE( 0x9b, 0x9b) AM_WRITE( v9938_0_register_w )
	AM_RANGE( 0xa0, 0xa0) AM_READ_PORT("P1")
	AM_RANGE( 0xa1, 0xa1) AM_READ_PORT("P2")
	AM_RANGE( 0xf7, 0xf7) AM_READ_PORT("DSW")
ADDRESS_MAP_END

/* Sexy Boom Ports */

static ADDRESS_MAP_START( sexyboom_io_map, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x7c, 0x7d) AM_DEVWRITE( "ymsnd", ym2413_w )
	AM_RANGE( 0xa0, 0xa0) AM_READ_PORT("P1")
	AM_RANGE( 0xa1, 0xa1) AM_READ_PORT("P2")
	AM_RANGE( 0xf0, 0xf0) AM_READWRITE( v9938_0_vram_r,v9938_0_vram_w )
	AM_RANGE( 0xf1, 0xf1) AM_READWRITE( v9938_0_status_r,v9938_0_command_w )
	AM_RANGE( 0xf2, 0xf2) AM_WRITE( v9938_0_palette_w )
	AM_RANGE( 0xf3, 0xf3) AM_WRITE( v9938_0_register_w )
	AM_RANGE( 0xf7, 0xf7) AM_READ_PORT("DSW")
	AM_RANGE( 0xf8, 0xff) AM_WRITE( sexyboom_bank_w )
ADDRESS_MAP_END


static INPUT_PORTS_START( sangho )
    PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
    PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

   PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
    PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("DSW")
    PORT_DIPNAME( 0x01, 0x01, "DIPS" ) /* coinage etc. */
    PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    PORT_DIPNAME( 0x40, 0x40, "Display Numbers on Tiles" )
    PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


static void sangho_common_machine_reset(running_machine *machine)
{
	memory_set_bankptr(machine, "bank1",&sangho_ram[0]);
	memory_set_bankptr(machine, "bank2",&sangho_ram[0x4000]);
	memory_set_bankptr(machine, "bank3",&sangho_ram[0x8000]);
	memory_set_bankptr(machine, "bank4",&sangho_ram[0xc000]);
	v9938_reset(0);
}


static MACHINE_RESET(pzlestar)
{
	/* give it some code to run, note this isn't at 0 in the rom! */
	memcpy(sangho_ram,&memory_region(machine, "user1")[0x10000],0x8000);

	/* patch out rom check (it fails, due to bad banking) */
	sangho_ram[0x25c1]=0xaf;
	sangho_ram[0x25c2]=0xc9;

	sangho_common_machine_reset(machine);
}

static MACHINE_RESET(sexyboom)
{
	sexyboom_bank[0] = 0x00;
	sexyboom_bank[1] = 0x00;
	sexyboom_bank[2] = 0x01;
	sexyboom_bank[3] = 0x00;
	sexyboom_bank[4] = 0x80;
	sexyboom_bank[5] = 0x00;
	sexyboom_bank[6] = 0x80;
	sexyboom_bank[7] = 0x01;
	sexyboom_map_bank(machine, 0);
	sexyboom_map_bank(machine, 1);
	sexyboom_map_bank(machine, 2);
	sexyboom_map_bank(machine, 3);

	v9938_reset(0);
}

static void msx_vdp_interrupt(running_machine *machine, int i)
{
	cputag_set_input_line (machine, "maincpu", 0, (i ? HOLD_LINE : CLEAR_LINE));
}

static INTERRUPT_GEN( sangho_interrupt )
{
	v9938_set_sprite_limit(0, 0);
	v9938_set_resolution(0, 2);
	v9938_interrupt(device->machine, 0);
}


static VIDEO_START( sangho )
{
	VIDEO_START_CALL(generic_bitmapped);
	v9938_init (machine, 0, machine->primary_screen, machine->generic.tmpbitmap, MODEL_V9938, 0x20000, msx_vdp_interrupt);
}

static MACHINE_DRIVER_START(pzlestar)

	MDRV_CPU_ADD("maincpu", Z80,8000000) // ?
	MDRV_CPU_PROGRAM_MAP(pzlestar_map)
	MDRV_CPU_IO_MAP(pzlestar_io_map)
	MDRV_CPU_VBLANK_INT_HACK(sangho_interrupt,262)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512 + 32, (212 + 28) * 2)
	MDRV_SCREEN_VISIBLE_AREA(0, 512 + 32 - 1, 0, (212 + 28) * 2 - 1)

	MDRV_PALETTE_LENGTH(512)

	MDRV_MACHINE_RESET(pzlestar)

	MDRV_PALETTE_INIT( v9938 )

	MDRV_VIDEO_START( sangho )
	MDRV_VIDEO_UPDATE( generic_bitmapped )


	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ymsnd", YM2413, 3580000)

	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START(sexyboom )

	MDRV_CPU_ADD("maincpu", Z80,8000000) // ?
	MDRV_CPU_PROGRAM_MAP(sexyboom_map)
	MDRV_CPU_IO_MAP(sexyboom_io_map)
	MDRV_CPU_VBLANK_INT_HACK(sangho_interrupt,262)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512 + 32, (212 + 28) * 2)
	MDRV_SCREEN_VISIBLE_AREA(0, 512 + 32 - 1, 0, (212 + 28) * 2 - 1)

	MDRV_PALETTE_LENGTH(512)

	MDRV_MACHINE_RESET(sexyboom)

	MDRV_PALETTE_INIT( v9938 )

	MDRV_VIDEO_START( sangho )
	MDRV_VIDEO_UPDATE( generic_bitmapped )

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ymsnd", YM2413, 3580000)

	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

ROM_START( pzlestar )
	ROM_REGION( 0x20000*16, "user1", 0 ) // 15 sockets, 13 used
	ROM_LOAD( "rom01.bin", 0x000000, 0x20000, CRC(0b327a3b) SHA1(450fd27f9844b9f0b710c1886985bd67cac2716f) ) // seems to be some code at 0x10000
	ROM_LOAD( "rom02.bin", 0x020000, 0x20000, CRC(dc859437) SHA1(e9fe5aab48d80e8857fc679ff7e35298ce4d47c8) )
	ROM_LOAD( "rom03.bin", 0x040000, 0x20000, CRC(f92b5624) SHA1(80be9000fc4326d790801d02959550aada111548) )
	ROM_LOAD( "rom04.bin", 0x060000, 0x20000, CRC(929e7491) SHA1(fb700d3e1d50fefa9b85ccd3702a9854df53a210) )
	ROM_LOAD( "rom05.bin", 0x080000, 0x20000, CRC(8c6f71e5) SHA1(3597b03fe61216256437c56c583d55c7d59b5525) )
	ROM_LOAD( "rom06.bin", 0x0a0000, 0x20000, CRC(84599227) SHA1(d47c6cdbf3b64f83627c768059148e31f8de1f36) )
	ROM_LOAD( "rom07.bin", 0x0c0000, 0x20000, CRC(6f64cc35) SHA1(3e3270834ad31e8240748c2b61f9b8f138d22f68) )
	ROM_LOAD( "rom08.bin", 0x0e0000, 0x20000, CRC(18d2bfe2) SHA1(cb92ee51d061bc053e296fcba10708f69ba12a61) )
	ROM_LOAD( "rom09.bin", 0x100000, 0x20000, CRC(19a31115) SHA1(fa6ead5c8bf6be21d07797f74fcba13f0d041937) )
	ROM_LOAD( "rom10.bin", 0x120000, 0x20000, CRC(c003328b) SHA1(5172e2c48e118ac9f9b9dd4f4df8804245047b33) )
	ROM_LOAD( "rom11.bin", 0x140000, 0x20000, CRC(d36c1f92) SHA1(42b412c1ab99cb14f2e15bd80fede34c0df414b9) )
	ROM_LOAD( "rom12.bin", 0x160000, 0x20000, CRC(baa82727) SHA1(ed3dd1befa615003204f903472ef1af1eb702c38) )
	ROM_LOAD( "rom13.bin", 0x180000, 0x20000, CRC(8b4b6a2c) SHA1(4b9c188260617ccce94cbf6cccb45aab455af09b) )
	/* 14 empty */
	/* 15 empty */
ROM_END

ROM_START( sexyboom )
	ROM_REGION( 0x20000*16, "user1", 0 ) // 15 sockets, 12 used
	ROM_LOAD( "rom1.bin",  0x000000, 0x20000, CRC(7827a079) SHA1(a48e7c7d16e50de24c8bf77883230075c1fad858) )
	ROM_LOAD( "rom2.bin",  0x020000, 0x20000, CRC(7028aa61) SHA1(77d5e5945b90d3e15ba2c1364b76f6643247592d) )
	ROM_LOAD( "rom3.bin",  0x040000, 0x20000, CRC(340edac9) SHA1(47ffc4553cb34ff932d3d54d5cefe82571c6ddbf) )
	ROM_LOAD( "rom4.bin",  0x060000, 0x20000, CRC(25f76d7f) SHA1(caff03ba4ca9ad44e488618141c4e1f43a0cdc48) )
	ROM_LOAD( "rom5.bin",  0x080000, 0x20000, CRC(3a3dda85) SHA1(b174cf87be5dd7a7430ff29c70c8093c577f4033) )
	ROM_LOAD( "rom6.bin",  0x0a0000, 0x20000, CRC(d0428a82) SHA1(4409c2ebd2f70828286769c9367cbccac483cdaf) )
	ROM_LOAD( "rom7.bin",  0x0c0000, 0x20000, CRC(2d2e4df2) SHA1(8ec36c8c021c2b9d9be7b61e09e31a7a18a06dad) )
	ROM_LOAD( "rom8.bin",  0x0e0000, 0x20000, CRC(283ba870) SHA1(98f35d95caf58595f002d57a4bafc39b6d67ed1a) )
	ROM_LOAD( "rom9.bin",  0x100000, 0x20000, CRC(a78310f4) SHA1(7a14cabd371d6ba4e335f0e00135de3dd8a4e642) )
	ROM_LOAD( "rom10.bin", 0x120000, 0x20000, CRC(b20fabd2) SHA1(a6a3bac1ac19e1ecd2fd0aeb17fbf16ffa07df13) )
	ROM_LOAD( "rom11.bin", 0x140000, 0x20000, CRC(e4aa16bc) SHA1(c5889b813ceb7a1c0deb8a9d4d006932b266a482) )
	ROM_LOAD( "rom12.bin", 0x160000, 0x20000, CRC(cd8b6b5d) SHA1(ffddc7781e13146e198ad12a9c89504f270857d8) )
	/* 13 empty */
	/* 14 empty */
	/* 15 empty */
ROM_END

static DRIVER_INIT(sangho)
{
	sangho_ram = auto_alloc_array(machine, UINT8, 0x20000);
}

GAME( 1991, pzlestar,  0,    pzlestar, sangho, sangho, ROT270, "Sang Ho Soft", "Puzzle Star (Sang Ho Soft)", GAME_NOT_WORKING )
GAME( 1992, sexyboom,  0,    sexyboom, sangho, sangho, ROT270, "Sang Ho Soft", "Sexy Boom", GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS )
