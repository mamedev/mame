/*

  High Seas Havoc
   (c)1993 Data East

   This is an unusual Data East PCB based on the Sega Genesis / Megadrive.  The Game was also released on the home system.


Produttore  Data East
N.revisione CG-2

CPU
1x MC68000P8 (main)(12c)
1x Z8400B (sound)(1a)
1x custom SEGA315-5660-FC1004 (QFP208)(5d)
1x PIC16C57 (7a)
1x oscillator 53.6931MHz (osc1)

ROMs
2x TMS27C040 (25,26)(11a,9a)
2x PEEL18CV8 (4b,5b)

Note
1x JAMMA edge connector
1x trimmer (volume)(vr1)
2x 6 switches dip (dsw1,dsw2)
-----------------------------------
PCB markings:"DE-0407-2 MADE IN JAPAN"
-----------------------------------

Thanks to DOX, the "mistery chip" has been identified (and verified) as a PIC.
Unfortunately it's read protected.

*/


#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2612intf.h"
#include "sound/sn76496.h"

#include "genesis.h"

#define MASTER_CLOCK		53693100


static INPUT_PORTS_START( hshavoc )
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

static ADDRESS_MAP_START( topshoot_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM														/* Cartridge Program Rom */
	AM_RANGE(0x200000, 0x201fff) AM_WRITENOP // tested
	AM_RANGE(0x202000, 0x2023ff) AM_RAM // tested
	AM_RANGE(0xa00000, 0xa0ffff) AM_READWRITE(genesis_68k_to_z80_r, genesis_68k_to_z80_w)
	AM_RANGE(0xa10000, 0xa1001f) AM_WRITE(genesis_io_w) AM_BASE(&genesis_io_ram)			/* Genesis Input */
	AM_RANGE(0xa11000, 0xa11203) AM_WRITE(genesis_ctrl_w)
	AM_RANGE(0xc00000, 0xc0001f) AM_READWRITE(genesis_vdp_r, genesis_vdp_w)					/* VDP Access */
	AM_RANGE(0xe00000, 0xe1ffff) AM_ROMBANK(3)
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAMBANK(4)
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_BASE(&genesis_68k_ram)							/* Main Ram */
ADDRESS_MAP_END


static ADDRESS_MAP_START( genesis_z80_map, ADDRESS_SPACE_PROGRAM, 8 )
 	AM_RANGE(0x0000, 0x1fff) AM_RAMBANK(1) AM_BASE(&genesis_z80_ram)
 	AM_RANGE(0x2000, 0x3fff) AM_RAMBANK(2) /* mirror */
	AM_RANGE(0x4000, 0x7fff) AM_READWRITE(genesis_z80_r, genesis_z80_w)
	AM_RANGE(0x8000, 0xffff) AM_READ(genesis_z80_bank_r)
 // AM_RANGE(0x8000, 0xffff) AM_WRITE(genesis_z80_bank_w)
ADDRESS_MAP_END


static MACHINE_DRIVER_START( genesis_base )
	/*basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, MASTER_CLOCK / 7)
	MDRV_CPU_VBLANK_INT("screen", genesis_vblank_interrupt)

	MDRV_CPU_ADD("genesis_snd_z80", Z80, MASTER_CLOCK / 15)
	MDRV_CPU_PROGRAM_MAP(genesis_z80_map)
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


static MACHINE_DRIVER_START( hshavoc )
	/* basic machine hardware */
	MDRV_IMPORT_FROM( genesis_base )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(topshoot_map)

	/* sound hardware */
	MDRV_SOUND_ADD("sn", SN76496, MASTER_CLOCK/15)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

ROM_START( hshavoc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "d-25.11a", 0x000000, 0x080000, CRC(6a155060) SHA1(ecb47bd428786e50e300a062b5038f943419a389) )
	ROM_LOAD16_BYTE( "d-26.9a",  0x000001, 0x080000, CRC(1afa84fe) SHA1(041296e0360b7747aedc2d948c39e06ba03a7d08) )

	ROM_REGION( 0x200000, "user1", 0 ) // other
	ROM_LOAD( "peel18cv8s.4b.bin",  0x000, 0x155, CRC(b5fb1d5f) SHA1(f0ac80471d97f77f415b5a1f153e1fce66720963) )
	ROM_LOAD( "peel18cv8s.5b.bin",  0x000, 0x155, CRC(efc7ceea) SHA1(1c31a56bc4b83bfa708048b7de4cee7a24537500) )

	ROM_REGION( 0x200000, "user2", 0 ) // other
	ROM_LOAD( "pic16c57",  0x00, 0x01, NO_DUMP ) // protected
ROM_END

#ifdef UNUSED_DEFINITION
ROM_START( hshavoc2 ) /* Genesis Version, for reference */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "hsh.rom", 0x000000, 0x100000, CRC(17be551c) SHA1(0dc1969098716ba332978b89356f62961417682b) )
ROM_END
#endif

static READ16_HANDLER( vdp_fake_r )
{
	return mame_rand(space->machine);
}

static DRIVER_INIT(genesis)
{
	/* hack -- fix vdp emulation instead */
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xC00004, 0xC00005, 0, 0, vdp_fake_r);

	memory_set_bankptr(machine, 3, memory_region(machine, "maincpu") );
	memory_set_bankptr(machine, 4, genesis_68k_ram );
}

static DRIVER_INIT(hshavoc)
{

	int x;
	UINT16 *src = (UINT16 *)memory_region(machine, "maincpu");

	static const UINT16 typedat[16] = {
		1,1,1,1, 1,1,1,1,
		1,0,0,1, 1,0,1,1
		};

	/* this decryption is wrong / incomplete, maybe it uses slightly different decryption for opcodes / data */
	/* I think the PIC that exists on the PCB controls a state-based encryption... there is a large amount
       of code encrypted using the same encryption as the data, but all the startup-code + vectors use additional
       encryption.. maybe the PIC can also patch the code, I'm also concerned that we may decrypt it and find
       that it runs as the genesis (no insert coin etc.) version without the PIC, or the PIC supplies additonal
       code in RAM.. but as of yet we can't know */

	int rom_size = 0xe8000;

	for (x=0;x<rom_size/2;x++)
	{
		src[x] = BITSWAP16(src[x],
								7, 15,6, 14,
                                5, 2, 1, 10,
                                13,4, 12,3,
                                11,0, 8, 9
							   );




		if (typedat[x&0xf]==1) src[x] = src[x]^0x0501;
		else src[x] = src[x]^0x0406;


		if(src[x]&0x0400) src[x]^=0x0200;


		if (typedat[x&0xf]==0)
		{
			if(src[x]&0x0100) src[x]^=0x0004;

			src[x] = BITSWAP16(src[x], 15,14,13,12,
								       11,9, 10,8,
								       7, 6, 5, 4,
									   3, 2, 1, 0);
		}

	}

	/* START e? from e80000 to end you need THIS ALONE to match the genesis rom */
	for (x=rom_size/2;x<0x100000/2;x++)
	{
		src[x] = BITSWAP16(src[x],
								7, 15,6, 14,
                                5, 2, 1, 10,
                                13,4, 12,3,
                                11,0, 8, 9
							   );

		src[x] = BITSWAP16(src[x],
								15, 14,13, 12,
                                11, 10, 9, 2,
                                7,6, 5,4,
                                3,8, 0, 1
							   );

	}
	/* EMD e80000 - end */

	src[0]^=0x0107;
	src[1]^=0x0107;
	src[2]^=0x0107;
	src[3]^=0x0707; //? 0701 not 0107 .. conditional 0x600 extra xor?, different 'typemap' ??

	/* I'm pretty sure c42 is where the startup code is located, comparing genesis version
       and this there is at least one jump in the genesis version to the startup code which
       has been changed to this address in the arcade version.

       there are several blocks of code like this, all appear to end with a normal rts instruction
       tho...
       */
	for (x=0xc42/2;x<0xc9a/2;x++)
	{
		src[x]^=0x0107; //? seems conditional..

		src[x] = BITSWAP16(src[x],
								15, 13,14, 12,
                                11, 10, 9, 0,
                                8,6, 5,4,
                                3,2, 1, 7
							   ); // probably wrong

		src[x]^=0x0001; // wrong..

	}

/* Uncommented until actively worked on
    {
        FILE*FP;

        FP=fopen("hshavoc.dump","wb");

        fwrite(src, rom_size/2, 2, FP);
        fclose(FP);
    }
*/


	DRIVER_INIT_CALL(genesis);

}



GAME( 1993, hshavoc,   0,        hshavoc, hshavoc, hshavoc, ROT0, "Data East",                  "High Seas Havoc",GAME_NOT_WORKING )
//GAME( 1993, hshavoc2,  hshavoc,  hshavoc, hshavoc, genesis, ROT0, "Data East",                  "High Seas Havoc (Genesis ROM)",GAME_NOT_WORKING )
