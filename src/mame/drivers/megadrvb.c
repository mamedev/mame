/***************************************************************************

    Sega Mega Drive/Genesis-based bootlegs

    Games supported:
        * Aladdin
        * Mortal Kombat 3
        * Super Street Fighter II - The New Challengers
        * Top Shooter


Aladdin PCB info
================

CPU
Main CPU 68000P10
Work RAM 64kB (62256 x2)
Sound CPU Z80B
Sound RAM 8kB (76c88-6264 x1)
Sound IC YM2612 (identified by pins,code was been erased.Named on board as TA07)

Other ICs
Microchip PIC16C57 (probably it contains the MD modified bios)
Osc 50 MHz
There are present 3 flat-pack chips with code erased again and named TA04,TA05,TA06 on board,which i have
identified (generically) by looking the PCB as:
TA04-Intercommunication and sync generator chip
TA05-Input controller
TA06-VDP (probably MD clone) Uses 2x D41264 SIL package as video RAM

ROMs

M3,M4 main program
M1,M2 graphics
All EPROMs are 27C040

Notes:

Dip-switch 8 x1

------------------------

This ROMset comes from a bootleg PCB.The game is a coin-op conversion of the one developed for the Megadrive
console.I cannot know gameplay differences since PCB is faulty.

However,hardware is totally different.It seems to be based on Sega Mega Drive hardware with CPU clock increased,
and since exists an "unlicensed" porting of the game for this system probably the "producers" are the same.


Stephh's notes (based on the game M68000 code and some tests) :

1) Useful addresses

  - 0xffff88.b = number of credits (range 0x00-0xff but display limited to 9)
  - 0xff7e3c.b = lives (range 0x30-0x39)
  - 0xffeffa.b = energy (range 0x00-0x08)
  - 0xffefe0.w = apples (range 0x30-0x39 * 2 , MSB first)
  - 0xffefe2.w = jewels (range 0x30-0x39 * 2 , MSB first)
  - 0xff7e29.b to 0xff7e2e.b = score (range 0x30-0x39 * 6) - MSDigit first
  - 0xff7e12.b to 0xff7e15.b = ??? (range 0x30-0x33 * 3 + 0x00) - MSDigit first - see below
  - 0xff7e16.b to 0xff7e19.b = ??? (range 0x30-0x33 * 3 + 0x00) - MSDigit first - see below
  - 0xff7e21.b = difficulty (range 0x00-0x02) - see below

2) Adresses notes

  - I can't tell what adresses 0xff7r12.l and 0xff7e16.l are supposed to be designed for :
    they are written once at the begining of each level (code at 0x1a9030) but I haven't found
    when they were read back (I've only played the 2 first levels though as well as the bonus level,
    but I also watched all demo levels till the end after the games full credits).
    I guess they were originally designed for bonus lives (additional and first), but no evidence.
  - 0xff7e21.b affects contents of 0xff7e3c.b, 0xffefe0.w, 0xff7e12.l and 0xff7e16.l :

                         Easy             Normal             Hard
                    0xff7e21.b=0x00   0xff7e21.b=0x01   0xff7e21.b=0x02
      0xff7e3c.b      0x32              0x31              0x30
      0xffefe0.w      0x3035            0x3032            0x3030
      0xff7e12.l      0x30313000        0x30313200        0x30313400
      0xff7e16.l      0x30303900        0x30313200        0x30313500

3) MCU notes

  - As I don't know how it is on real hardware, MCU simulation is more a guess than anything;
    anyway, the game now runs correctly (coins are handled and settings change)
  - Difficulty Dip Switches are correct (see code at 0x1b2680)
  - Coinage Dip Switches might be wrong because I don't know what the possible values can be,
    but setting them the way I did isn't that bad (see code at 0x1b2a50)
  - It's possible that writes to 0x220000 are in fact a mask for what is read back from 0x330000,
    but I haven't found any formula (thus the "lame" read/write handlers you may investigate)

4) Controls notes

  - This game is a one player only game (same as the MegaDrive version);
    that's why I've "blanked" player 2 inputs which are never read.
  - I've labelled the buttons the same way as in 'g_aladj' with default options.

5) MegaDrive comparaison ('g_aladj' in HazeMD)

  - There is no "OPTIONS" menu as the difficulty is handled via the MCU / Dip Switches.
    Some code has been patched but most is still there (see the texts in the ROM ares);
    Unfortunately, there seems to be no way to access them (no "service" button).
  - Even with the same settings (same value for 0xff7e21.b), lives and apples,
    as well as contents of 0xff7e12.l and 0xff7e16.l are really different) !
    Here is the same data as above for 'g_aladj' :

                         Easy             Normal             Hard
                    0xff7e21.b=0x00   0xff7e21.b=0x01   0xff7e21.b=0x02
      0xff7e3c.b      0x35              0x33              0x32
      0xffefe0.w      0x3135            0x3130            0x3035
      0xff7e12.l      0x30303600        0x30303800        0x30313000
      0xff7e16.l      0x30303300        0x30303600        0x30303900

    But what makes the arcade version much harder is how energy is handled : in 'g_aladj', you can
    be hit 8 times before you lose a life, while in 'aladbl', you lose a life as soon as you are hit !
    This is done via code change at 0x1aee3c and patched code at 0x1afc00 :

      diff aladbl.asm g_aladj.asm

      < 1AEE3C: 4EB9 001A FC00             jsr     $1afc00.l
      > 1AEE3C: 5339 00FF EFFA             subq.b  #1, $ffeffa.l

      < 1AFC00: 0C39 0001 00FF F57C        cmpi.b  #$1, $fff57c.l
      < 1AFC08: 6700 000A                  beq     $1afc14
      < 1AFC0C: 4239 00FF EFFA             clr.b   $ffeffa.l
      < 1AFC12: 4E75                       rts
      < 1AFC14: 5339 00FF EFFA             subq.b  #1, $ffeffa.l
      < 1AFC1A: 4E75                       rts

    Surprisingly, when you are in "demo mode", player can be again be hit 8 times
    before losing a life (this is the purpose of the 0xfff57c "flag") !

****************************************************************************

Top Shooter PCB info
====================

 Sun Mixing board, looks like a hacked up Genesis clone.

 Original driver by David Haywood
 Inputs by Mariusz Wojcieszek

 Top Shooter - (c)1995  - older board, look more like an actual hacked cart system, has an MCU

 Not Dumped

 Super Bubble Bobble (a bubble bobble rip-off from Sun Mixing, looks like it may be on this hardware)

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
 DIPs = Fixed as: 00001000
 ROMs = Toshiba TC574000AD

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

****************************************************************************/

#include "emu.h"
#include "includes/megadriv.h"


/*************************************
 *
 *  Games memory handlers
 *
 *************************************/

static WRITE16_HANDLER( aladbl_w )
{
    /*
    Values returned from the log file :
      - aladbl_w : 1b2a6c - data = 6600 (each time a coin is inserted)
      - aladbl_w : 1b2a82 - data = 0000 (each time a coin is inserted)
      - aladbl_w : 1b2d18 - data = aa00 (only once on reset)
      - aladbl_w : 1b2d42 - data = 0000 (only once on reset)
    */
	logerror("aladbl_w : %06x - data = %04x\n",cpu_get_pc(space->cpu),data);
}

static READ16_HANDLER( aladbl_r )
{
	if (cpu_get_pc(space->cpu)==0x1b2a56)
	{
		static UINT16 mcu_port;

		mcu_port = input_port_read(space->machine, "MCU");

		if(mcu_port & 0x100)
			return ((mcu_port & 0x0f) | 0x100); // coin inserted, calculate the number of coins
		else
			return (0x100); //MCU status, needed if you fall into a pitfall
	}
	if (cpu_get_pc(space->cpu)==0x1b2a72) return 0x0000;
	if (cpu_get_pc(space->cpu)==0x1b2d24) return (input_port_read(space->machine, "MCU") & 0x00f0) | 0x1200;    // difficulty
	if (cpu_get_pc(space->cpu)==0x1b2d4e) return 0x0000;

	logerror("aladbl_r : %06x\n",cpu_get_pc(space->cpu));

	return 0x0000;
}


static READ16_HANDLER( mk3ghw_dsw_r )
{
	static const char *const dswname[3] = { "DSWA", "DSWB", "DSWC" };
	return input_port_read(space->machine, dswname[offset]);
}

static READ16_HANDLER( ssf2ghw_dsw_r )
{
	static const char *const dswname[3] = { "DSWA", "DSWB", "DSWC" };
	return input_port_read(space->machine, dswname[offset]);
}

static READ16_HANDLER(topshoot_200051_r)
{
	return -0x5b;
}

/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( topshoot ) /* Top Shooter Input Ports */

	PORT_START("IN0")
	PORT_BIT( 0x4f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet") PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Start") PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Fire") PORT_IMPULSE(1)

	PORT_START("IN1")
	PORT_BIT( 0xe7, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Test mode down") PORT_IMPULSE(1)

	PORT_START("IN2")
	PORT_BIT( 0xfd, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( aladbl )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "m1.bin", 0x000001, 0x080000,  CRC(5e2671e4) SHA1(54705c7614fc7b5a1065478fa41f51dd1d8045b7) )
	ROM_LOAD16_BYTE( "m2.bin", 0x000000, 0x080000,  CRC(142a0366) SHA1(6c94aa9936cd11ccda503b52019a6721e64a32f0) )
	ROM_LOAD16_BYTE( "m3.bin", 0x100001, 0x080000,  CRC(0feeeb19) SHA1(bd567a33077ab9997871d21736066140d50e3d70) )
	ROM_LOAD16_BYTE( "m4.bin", 0x100000, 0x080000,  CRC(bc712661) SHA1(dfd554d000399e17b4ddc69761e572195ed4e1f0))
ROM_END

ROM_START( mk3ghw ) // roms are scrambled, we take care of the address descramble in the ROM load, and the data descramble in the init
                    // this is bootlegged from  "Mortal Kombat 3 (4) [!].bin"
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "1.u1", 0x080001, 0x020000,  CRC(0dc01b23) SHA1(f1aa7ac88c8e3deb5a0a065862722e9d27b87b4c) )
	ROM_CONTINUE(            0x000001, 0x020000)
	ROM_CONTINUE(            0x0c0001, 0x020000)
	ROM_CONTINUE(            0x040001, 0x020000)
	ROM_LOAD16_BYTE( "2.u3", 0x180001, 0x040000,  CRC(50250235) SHA1(9f9e06f26163b92c76397fde43b38b3536bcb637) )
	ROM_CONTINUE(            0x100001, 0x040000)
	ROM_LOAD16_BYTE( "3.u9", 0x280001, 0x040000,  CRC(493404c1) SHA1(73f4bd1eeeee3f175f4378ab406a97f94f88880b) )
	ROM_CONTINUE(            0x200001, 0x040000)
	ROM_LOAD16_BYTE( "4.u11",0x380001, 0x040000,  CRC(a52156b8) SHA1(0990ef1fb3427a5d3c262e264feb25c1db75ed33) )
	ROM_CONTINUE(            0x300001, 0x040000)
	ROM_LOAD16_BYTE( "6.u2", 0x080000, 0x020000,  CRC(9852fd6f) SHA1(348befeca5129c5ea2c142760ec93511f98f23cc) )
	ROM_CONTINUE(            0x000000, 0x020000)
	ROM_CONTINUE(            0x0c0000, 0x020000)
	ROM_CONTINUE(            0x040000, 0x020000)
	ROM_LOAD16_BYTE( "5.u4", 0x180000, 0x040000,  CRC(ed6a6d13) SHA1(eaab912ee035ece03f7cfceb1b546004399daad5) )
	ROM_CONTINUE(            0x100000, 0x040000)
	ROM_LOAD16_BYTE( "7.u10",0x280000, 0x040000,  CRC(a124d8d1) SHA1(d391b130992701d0fae7e827ba314b8368d809de) )
	ROM_CONTINUE(            0x200000, 0x040000)
	ROM_LOAD16_BYTE( "8.u12",0x380000, 0x040000,  CRC(8176f7cc) SHA1(375e1e982b97ba709fb160b04f56f6aa2d580104) )
	ROM_CONTINUE(            0x300000, 0x040000)
ROM_END

ROM_START( ssf2ghw )
	ROM_REGION( 0x1400000, "maincpu", 0 ) /* 68000 Code */
	/* Special Case, custom PCB, linear ROM mapping of 5meg */
	ROM_LOAD16_BYTE( "rom_a", 0x000000, 0x200000,  CRC(59726521) SHA1(3120bac17f56c01ffb9d3f9e31efa0263e3774af) )
	ROM_LOAD16_BYTE( "rom_b", 0x000001, 0x200000,  CRC(7dad5540) SHA1(9279068b2218d239fdd557dd959ac70e74853178) )
	ROM_LOAD16_BYTE( "rom_c", 0x400000, 0x080000,  CRC(deb48624) SHA1(39ffa7de7b808e0b95cb039bb381705d77420933) )
	ROM_LOAD16_BYTE( "rom_d", 0x400001, 0x080000,  CRC(b99f6a5b) SHA1(adbe28a7522024bc66328ac86fecf9ded3310e8e) )
ROM_END

ROM_START( topshoot ) /* Top Shooter (c)1995 Sun Mixing */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tc574000ad_u11_2.bin", 0x000000, 0x080000, CRC(b235c4d9) SHA1(fbb308a5f6e769f3277824cb6a3b50c308969ac2) )
	ROM_LOAD16_BYTE( "tc574000ad_u12_1.bin", 0x000001, 0x080000, CRC(e826f6ad) SHA1(23ec8bb608f954d3b915f061e7076c0c63b8259e) )

	// not hooked up yet
	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "89c51.bin", 0x0000, 0x1000, CRC(595475c8) SHA1(8313819ba06cc92b54f88c1ca9f34be8d1ec94d0) )
ROM_END

/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

#define ENERGY_CONSOLE_MODE 0

static DRIVER_INIT( aladbl )
{
	/*
     * Game does a check @ 1afc00 with work RAM fff57c that makes it play like the original console version (i.e. 8 energy hits instead of 2)
     */
	#if ENERGY_CONSOLE_MODE
	UINT16 *rom = (UINT16 *)memory_region(machine, "maincpu");
	rom[0x1afc08/2] = 0x6600;
	#endif

	// 220000 = writes to mcu? 330000 = reads?
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x220000, 0x220001, 0, 0, aladbl_w);
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x330000, 0x330001, 0, 0, aladbl_r);

	DRIVER_INIT_CALL(megadrij);
}

// this should be correct, the areas of the ROM that differ to the original
// after this decode look like intentional changes
static DRIVER_INIT( mk3ghw )
{
	int x;
	UINT8 *rom = memory_region(machine, "maincpu");

	for (x=0x000001;x<0x100001;x+=2)
	{
		if (x&0x80000)
		{
			rom[x] = rom[x]^0xff;
			rom[x] = BITSWAP8(rom[x], 0,3,2,5,4,6,7,1);
		}
		else
		{
			rom[x] = rom[x]^0xff;
			rom[x] = BITSWAP8(rom[x], 4,0,7,1,3,6,2,5);
		}
	}

	for (x=0x100001;x<0x400000;x+=2)
	{
		if (x&0x80000)
		{
			rom[x] = rom[x]^0xff;
			rom[x] = BITSWAP8(rom[x], 2,7,5,4,1,0,3,6);
		}
		else
		{
			rom[x] = BITSWAP8(rom[x], 6,1,4,2,7,0,3,5);
		}
	}

	// boot vectors don't seem to be valid, so they are patched...
	rom[0x01] = 0x01;
	rom[0x00] = 0x00;
	rom[0x03] = 0x00;
	rom[0x02] = 0x00;
	rom[0x05] = 0x00;
	rom[0x04] = 0x00;
	rom[0x07] = 0x02;
	rom[0x06] = 0x10;

	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x770070, 0x770075, 0, 0, mk3ghw_dsw_r );

	DRIVER_INIT_CALL(megadriv);
}

static DRIVER_INIT( ssf2ghw )
{
	memory_nop_write(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xA130F0, 0xA130FF, 0, 0); // custom banking is disabled (!)
	memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x5fffff, 0, 0, "bank5");
	memory_unmap_write(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x400000, 0x5fffff, 0, 0);

	memory_set_bankptr(machine,  "bank5", memory_region( machine, "maincpu" ) + 0x400000 );

	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x770070, 0x770075, 0, 0, ssf2ghw_dsw_r );

	DRIVER_INIT_CALL(megadrij);

}

static DRIVER_INIT(topshoot)
{
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200050, 0x200051, 0, 0, topshoot_200051_r );
	memory_install_read_port(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200042, 0x200043, 0, 0, "IN0");
	memory_install_read_port(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200044, 0x200045, 0, 0, "IN1");
	memory_install_read_port(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200046, 0x200047, 0, 0, "IN2");
	memory_install_read_port(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200048, 0x200049, 0, 0, "IN3");

	DRIVER_INIT_CALL(megadriv);
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1993, aladbl,   0, megadriv,   aladbl,   aladbl,   ROT0, "bootleg / Sega",   "Aladdin (bootleg of Japanese Megadrive version)", 0)
GAME( 1996, mk3ghw,   0, megadriv,   mk3ghw,   mk3ghw,   ROT0, "bootleg / Midway", "Mortal Kombat 3 (bootleg of Megadrive version)", 0)
GAME( 1994, ssf2ghw,  0, megadriv,   ssf2ghw,  ssf2ghw,  ROT0, "bootleg / Capcom", "Super Street Fighter II - The New Challengers (Arcade bootleg of Japanese MegaDrive version)", 0)
GAME( 1995, topshoot, 0, md_bootleg, topshoot, topshoot, ROT0, "Sun Mixing",       "Top Shooter", 0)
