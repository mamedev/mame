
/*

Sun Mixing board, looks like a hacked up Genesis clone.


Top Shooter - (c)1995  - older board, look more like an actual hacked cart system, has an MCU, doesn't
                         work in MAME due to VDP status etc.  boots in Kega Fusion but has no inputs
                         (MCU related?)

*/

#define MASTER_CLOCK		53693100


/* Not Dumped

 Super Bubble Bobble (a bubble bobble rip-off from Sun Mixing, looks like it may be on this hardware)

 */

/*

note:

inputs don't work.. (MCU related?)
no sprites in MAME (VDP related? they show in Kega Fusion)

*/


/*

TOP SHOOTER - Sun Mixing Co. Ltd. 1995

To me it seems like an original cartridge-based arcade board
hacked to use an external ROM board and a standard JAMMA
connector, but of course, I can be wrong.


   UPPER BOARD

   _________________________________________________________
   |            ___________  ___________  _____      __    |
   | 74LS245P  |U14 Empty | |U12 ROM1  |  |IC1|      |B|   |
   | 74LS245P  |__________| |__________|  |___|            |
   | 74LS245P   ___________  ___________    _____________  |
 __|           |U13 Empty | |U11 ROM2  |   | AT89C51    |  |
 |_ J          |__________| |__________|   |____________|  |_
 |_ A           ______________________              _____  |_ J
 |_ M          | U10 MC68000P10       |             |OSC|  |_ P
 |_ M          | Motorola             |                    |_ 2
 |_ A          |______________________|            74HC00P |_
 |_  74LS245P   ______________________           ________  |
 |_            | U9 Empty             |          |HM6116L  |
 |_            |                      |          |_______| |_ J
 |_            |______________________|                    |_ P
 |_  74LS245P                           TD62oo3AP 74LS373P |_ 3
 |_                                            __________  |
 |_  74LS245P                                  |GALv20V8B| |
 |_                                    ______              |
 |_               _____                |DIPS|              |_ P
   |             |U24  |                                   |_ 1
   | 74LS245P                                              |
   | TD62oo3AP                                             |
   |                                                       |
   |_            97              ____________         _____|
     |_|_|_|_|_|_|_|_|_|_|_|_|_|_|           |_|_|_|_|


  IC1 = Surface scracthed out, don't know what is it
  U24 = Surface scratched out, seems like a PROM
 DIPS = Fixed as: 00001000
 ROMS = Toshiba TC574000AD

  JP2, JP3 and P1 connects both boards, also another
  on-board connector is used, see notes for the 68K socket
  for the lower board.


   LOWER BOARD

   _________________________________________________________
   |                                     ____ ____         |
   |  ___                                | I| | I|         |
   |  |I|                                | C| | C|         |
   |  |C|                                | 3| | 2|         |
   |  |1|                                |__| |__|         |
   |  |3|                                                  |__
   |   _                _________________________           __|
   |  |_|               |||||||||||||||||||||||||           __|
   |  IC14              ---------- SLOT ---------           __|
   |               ______________________                   __|
   |              |                      |                  __|
   |  ___         | 68K (to upper board) |   _______        __|
   |  |I|         |______________________|   |SE-94|        __|
   |  |C|                                    |JDDB |      _|
   |  |1|           _______                  |_____|      |
   |  |2|           |SE-93|                    IC4        |
   |                |JDDA |                               |
   |                |_____|                ___________    |_
   |                  IC8                  |Z8400A PS|     |
   |                                       |_________|     |
   |                  ______         _________  _________  |
   |                  | OSC|         | IC11  |  | IC7   |  |
   |            _____________        |_______|  |_______|  |
   |    RST    |            |           CN5        CN6     |
   |___________|            |______________________________|


   IC3 = IC2 = Winbond W24257V
   IC7  = 6264LD 9440
   IC11 = SE-95 JDDC
   IC12 = Sony CXA1634P
   IC13 = Sony CXA1145P
   IC14 = GL358 N16

   RST is a reset button.

   OSC = 53.693175 MHz

   CN5 and CN6 are 9-pin connectors... serial ports?

   There are two wires soldered directly to two connectors
   of the slot, going to the upper board (via P1).

   The whole upper board is plugged using the 68000 socket,
   there is no 68K on the lower board.

   There is an edge connector, but it isn't JAMMA.

   "HK-986 (KINYO)" is written on the PCB, near the slot.

*/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "genesis.h"


static INPUT_PORTS_START( topshoot ) /* Top Shooter Input Ports */
	PORT_START("IN0")	/* 16bit */
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static ADDRESS_MAP_START( topshoot_readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_READ(SMH_ROM)					/* Cartridge Program Rom */
//  AM_RANGE(0x200000, 0x20007f) AM_READ(SMH_RAM)
//  AM_RANGE(0x200040, 0x200041) AM_READ_PORT("IN0")        // ??
//  AM_RANGE(0x200050, 0x200051) AM_READ_PORT("IN0")        // ??
	AM_RANGE(0x202000, 0x2023ff) AM_READ(SMH_RAM)
	AM_RANGE(0x400004, 0x400005) AM_READ_PORT("IN0")		// ??
	AM_RANGE(0xa10000, 0xa1001f) AM_READ_PORT("IN0")
	AM_RANGE(0xa11100, 0xa11101) AM_READ_PORT("IN0")		// ??


	AM_RANGE(0xa00000, 0xa0ffff) AM_READ(genesis_68k_to_z80_r)
	AM_RANGE(0xc00000, 0xc0001f) AM_READ(genesis_vdp_r)				/* VDP Access */
	AM_RANGE(0xe00000, 0xe1ffff) AM_READ(SMH_BANK3)
	AM_RANGE(0xfe0000, 0xfeffff) AM_READ(SMH_BANK4)
	AM_RANGE(0xff0000, 0xffffff) AM_READ(SMH_RAM)					/* Main Ram */
ADDRESS_MAP_END


static ADDRESS_MAP_START( topshoot_writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_WRITE(SMH_ROM)					/* Cartridge Program Rom */
//  AM_RANGE(0x200000, 0x20007f) AM_WRITE(SMH_RAM)
	AM_RANGE(0x200000, 0x2023ff) AM_WRITE(SMH_RAM) // tested
	AM_RANGE(0xa10000, 0xa1001f) AM_WRITE(genesis_io_w) AM_BASE(&genesis_io_ram)				/* Genesis Input */
	AM_RANGE(0xa11000, 0xa11203) AM_WRITE(genesis_ctrl_w)
	AM_RANGE(0xa00000, 0xa0ffff) AM_WRITE(genesis_68k_to_z80_w)
	AM_RANGE(0xc00000, 0xc0001f) AM_WRITE(genesis_vdp_w)				/* VDP Access */
	AM_RANGE(0xfe0000, 0xfeffff) AM_WRITE(SMH_BANK4)
	AM_RANGE(0xff0000, 0xffffff) AM_WRITE(SMH_RAM) AM_BASE(&genesis_68k_ram)/* Main Ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( genesis_z80_readmem, ADDRESS_SPACE_PROGRAM, 8 )
 	AM_RANGE(0x0000, 0x1fff) AM_READ(SMH_BANK1)
 	AM_RANGE(0x2000, 0x3fff) AM_READ(SMH_BANK2) /* mirror */
	AM_RANGE(0x4000, 0x7fff) AM_READ(genesis_z80_r)
	AM_RANGE(0x8000, 0xffff) AM_READ(genesis_z80_bank_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( genesis_z80_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_WRITE(SMH_BANK1) AM_BASE(&genesis_z80_ram)
 	AM_RANGE(0x2000, 0x3fff) AM_WRITE(SMH_BANK2) /* mirror */
	AM_RANGE(0x4000, 0x7fff) AM_WRITE(genesis_z80_w)
 // AM_RANGE(0x8000, 0xffff) AM_WRITE(genesis_z80_bank_w)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( genesis_base )
	/*basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK / 7)
	MDRV_CPU_VBLANK_INT("screen", genesis_vblank_interrupt)

	MDRV_CPU_ADD("soundcpu", Z80, MASTER_CLOCK / 15)
	MDRV_CPU_PROGRAM_MAP(genesis_z80_readmem, genesis_z80_writemem)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold) /* from vdp at scanline 0xe0 */

	MDRV_QUANTUM_TIME(HZ(6000))

	MDRV_MACHINE_START(genesis)
	MDRV_MACHINE_RESET(genesis)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(342,262)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 223)

	MDRV_PALETTE_LENGTH(64)

	MDRV_VIDEO_START(genesis)
	MDRV_VIDEO_UPDATE(genesis)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM3438, MASTER_CLOCK/7)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( topshoot )
	/* basic machine hardware */
	MDRV_IMPORT_FROM( genesis_base )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(topshoot_readmem,topshoot_writemem)

	/* video hardware */
	MDRV_VIDEO_START(genesis)
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 223)

	/* sound hardware */
	MDRV_SOUND_ADD("sn", SN76496, MASTER_CLOCK/15)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

ROM_START( topshoot ) /* Top Shooter (c)1995 Sun Mixing */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tc574000ad_u11_2.bin", 0x000000, 0x080000, CRC(b235c4d9) SHA1(fbb308a5f6e769f3277824cb6a3b50c308969ac2) )
	ROM_LOAD16_BYTE( "tc574000ad_u12_1.bin", 0x000001, 0x080000, CRC(e826f6ad) SHA1(23ec8bb608f954d3b915f061e7076c0c63b8259e) )
ROM_END

static READ16_HANDLER( vdp_fake_r )
{
	return mame_rand(space->machine);
}

static DRIVER_INIT(topshoot)
{
	/* hack -- fix vdp emulation instead */
	memory_install_read16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0xC00004, 0xC00005, 0, 0, vdp_fake_r);

	memory_set_bankptr(machine, 3, memory_region(machine, "maincpu") );
	memory_set_bankptr(machine, 4, genesis_68k_ram );
}


/* Sun Mixing Hardware, very close to actual Genesis */
GAME( 1995, topshoot,  0,        topshoot, topshoot, topshoot, ROT0, "Sun Mixing",                  "Top Shooter",GAME_NOT_WORKING )
