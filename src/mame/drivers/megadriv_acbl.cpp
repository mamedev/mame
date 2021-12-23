// license:BSD-3-Clause
// copyright-holders:David Haywood,Stephane Humbert
/***************************************************************************

    Sega Mega Drive/Genesis-based bootlegs

    Games supported:
        * Aladdin
        * Bare Knuckle II
        * Bare Knuckle III
        * Bare Knuckle III / Sunset Riders
        * Jurassic Park
        * Mortal Kombat 3
        * Sonic The Hedgehog 2
        * Sonic The Hedgehog 3
        * Super Street Fighter II - The New Challengers
        * Sunset Riders
        * Top Shooter
        * Twinkle Tale


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

2) Addresses notes

  - I can't tell what addresses 0xff7r12.l and 0xff7e16.l are supposed to be designed for :
    they are written once at the beginning of each level (code at 0x1a9030) but I haven't found
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

5) MegaDrive comparison ('g_aladj' in HazeMD)

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
    be hit 8 times before you lose a life, while in 'aladmdb', you lose a life as soon as you are hit !
    This is done via code change at 0x1aee3c and patched code at 0x1afc00 :

      diff aladmdb.asm g_aladj.asm

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

Sunset Riders info
====================

 - title raster effect is broken (bug in megadrive code, happens with normal set too)

****************************************************************************

Top Shooter PCB info
====================

 Sun Mixing board, looks like a hacked up Genesis clone.

 Original driver by David Haywood
 Inputs by Mariusz Wojcieszek

 Top Shooter - (c)1995  - older board, look more like an actual hacked cart system, has an MCU

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


  IC1 = Surface scratched out, don't know what it is
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
#include "includes/megadriv_acbl.h"


/************************************ Megadrive Bootlegs *************************************/

// smaller ROM region because some bootlegs check for RAM there (used by topshoot and hshavoc)
void md_boot_state::md_bootleg_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom(); // Cartridge Program ROM
	map(0x200000, 0x2023ff).ram(); // Tested

	map(0xa00000, 0xa01fff).rw(FUNC(md_boot_state::megadriv_68k_read_z80_ram), FUNC(md_boot_state::megadriv_68k_write_z80_ram));
	map(0xa02000, 0xa03fff).w(FUNC(md_boot_state::megadriv_68k_write_z80_ram));
	map(0xa04000, 0xa04003).rw(FUNC(md_boot_state::megadriv_68k_YM2612_read), FUNC(md_boot_state::megadriv_68k_YM2612_write));
	map(0xa06000, 0xa06001).w(FUNC(md_boot_state::megadriv_68k_z80_bank_write));

	map(0xa10000, 0xa1001f).rw(FUNC(md_boot_state::megadriv_68k_io_read), FUNC(md_boot_state::megadriv_68k_io_write));
	map(0xa11100, 0xa11101).rw(FUNC(md_boot_state::megadriv_68k_check_z80_bus), FUNC(md_boot_state::megadriv_68k_req_z80_bus));
	map(0xa11200, 0xa11201).w(FUNC(md_boot_state::megadriv_68k_req_z80_reset));

	map(0xc00000, 0xc0001f).rw(m_vdp, FUNC(sega315_5313_device::vdp_r), FUNC(sega315_5313_device::vdp_w));
	map(0xd00000, 0xd0001f).rw(m_vdp, FUNC(sega315_5313_device::vdp_r), FUNC(sega315_5313_device::vdp_w));

	map(0xe00000, 0xe0ffff).ram().mirror(0x1f0000).share("megadrive_ram");
}

void md_boot_state::md_bootleg(machine_config &config)
{
	md_ntsc(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &md_boot_state::md_bootleg_map);
}

/*************************************
 *
 *  Games memory handlers
 *
 *************************************/

void md_boot_state::aladmdb_w(uint16_t data)
{
	/*
	Values returned from the log file :
	  - aladmdb_w : 1b2a6c - data = 6600 (each time a coin is inserted)
	  - aladmdb_w : 1b2a82 - data = 0000 (each time a coin is inserted)
	  - aladmdb_w : 1b2d18 - data = aa00 (only once on reset)
	  - aladmdb_w : 1b2d42 - data = 0000 (only once on reset)
	*/
	logerror("aladmdb_w : %06x - data = %04x\n",m_maincpu->pc(),data);
}

uint16_t md_boot_state::aladmdb_r()
{
	if (m_maincpu->pc()==0x1b2a56)
	{
		m_aladmdb_mcu_port = ioport("MCU")->read();

		if (m_aladmdb_mcu_port & 0x100)
			return ((m_aladmdb_mcu_port & 0x0f) | 0x100); // coin inserted, calculate the number of coins
		else
			return (0x100); //MCU status, needed if you fall into a pitfall
	}
	if (m_maincpu->pc()==0x1b2a72) return 0x0000;
	if (m_maincpu->pc()==0x1b2d24) return (ioport("MCU")->read() & 0x00f0) | 0x1200;    // difficulty
	if (m_maincpu->pc()==0x1b2d4e) return 0x0000;

	logerror("aladbl_r : %06x\n",m_maincpu->pc());
	return 0x0000;
}

uint16_t md_boot_state::twinktmb_r()
{
	if (m_maincpu->pc()==0x02f81e)
		return ioport("COIN")->read(); // TODO: coins don't respond well

	if (m_maincpu->pc()==0x02f84e) return 0x0000; // what's this? dips?

	//logerror("twinktmb_r : %06x\n",m_maincpu->pc());

	return 0x0000;
}

uint16_t md_boot_state::jparkmb_r()
{
	if (m_maincpu->pc()==0x1e327a)
		return ioport("COIN")->read(); // TODO: coins don't respond well

	if (m_maincpu->pc()==0x1e3254) return 0x0000; // what's this? dips?

	//logerror("jparkmb_r : %06x\n",m_maincpu->pc());

	return 0x0000;
}

uint16_t md_boot_state::barek2mb_r()
{
	if (m_maincpu->pc()==0xfa40)
		return 0x0400; // TODO: what's this? Needed or the game doesn't boot

	if (m_maincpu->pc()==0xfa88)
		return 0x0ff0; // TODO: fix this, should probably read coin inputs, as is gives 9 credits at start up

	logerror("aladbl_r : %06x\n",m_maincpu->pc());
	return 0x0000;
}

uint16_t md_boot_state::sonic3mb_r()
{
	if (m_maincpu->pc() == 0x1688) return 0x0300; // TODO: should work but doesn't? debug: just put 0x0300 at 0xfffffc during the first startup check to succesfully boot. Coins are stored in the same location

	// logerror("sonic3mb_r : %06x\n", m_maincpu->pc());

	return 0x0000;
}

uint16_t md_boot_state::dsw_r(offs_t offset)
{
	static const char *const dswname[3] = { "DSWA", "DSWB", "DSWC" };
	return ioport(dswname[offset])->read();
}

uint16_t md_boot_state::topshoot_200051_r()
{
	return -0x5b;
}

uint16_t md_boot_state::sbubsm_400000_r()
{
	logerror("%s: sbubsm_400000_r\n", machine().describe_context().c_str());
	return 0x5500;
}

uint16_t md_boot_state::sbubsm_400002_r()
{
	logerror("%s: sbubsm_400002_r\n", machine().describe_context().c_str());
	return 0x0f00;
}

// jzth protection
void md_boot_state::bl_710000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int pc = m_maincpu->pc();

	logerror("%06x writing to bl_710000_w %04x %04x\n", pc, data, mem_mask);

	// protection value is read from  0x710000 after a series of writes.. and stored at ff0007
	// startup
	/*
	059ce0 writing to bl_710000_w ff08 ffff
	059d04 writing to bl_710000_w 000a ffff
	059d04 writing to bl_710000_w 000b ffff
	059d04 writing to bl_710000_w 000c ffff
	059d04 writing to bl_710000_w 000f ffff
	059d1c writing to bl_710000_w ff09 ffff
	059d2a reading from bl_710000_r  (wants 0xe)
	059ce0 writing to bl_710000_w ff08 ffff
	059d04 writing to bl_710000_w 000a ffff
	059d04 writing to bl_710000_w 000b ffff
	059d04 writing to bl_710000_w 000c ffff
	059d04 writing to bl_710000_w 000f ffff
	059d1c writing to bl_710000_w ff09 ffff
	059d2a reading from bl_710000_r  (wants 0xe)
	*/
	// before lv stage 3
	/*
	059ce0 writing to bl_710000_w 0008 ffff
	059d04 writing to bl_710000_w 000b ffff
	059d04 writing to bl_710000_w 000f ffff
	059d1c writing to bl_710000_w ff09 ffff
	059d2a reading from bl_710000_r  (wants 0x4)
	*/
	// start level 3
	/*
	059ce0 writing to bl_710000_w ff08 ffff
	059d04 writing to bl_710000_w 000b ffff
	059d04 writing to bl_710000_w 000c ffff
	059d04 writing to bl_710000_w 000e ffff
	059d1c writing to bl_710000_w ff09 ffff
	059d2a reading from bl_710000_r  (wants 0x5)

	// after end sequence
	059ce0 writing to bl_710000_w 0008 ffff
	059d04 writing to bl_710000_w 000a ffff
	059d04 writing to bl_710000_w 000b ffff
	059d04 writing to bl_710000_w 000c ffff
	059d04 writing to bl_710000_w 000f ffff
	059d1c writing to bl_710000_w ff09 ffff
	059d2a reading from bl_710000_r  (wants 0xe)

	*/
	m_protcount++;
}


uint16_t md_boot_state::bl_710000_r()
{
	uint16_t ret;
	int pc = m_maincpu->pc();
	logerror("%06x reading from bl_710000_r\n", pc);

	if (m_protcount==6) { ret = 0xe; }
	else if (m_protcount==5) { ret = 0x5; }
	else if (m_protcount==4) { ret = 0x4; }
	else ret = 0xf;

	m_protcount = 0;
	return ret;
}


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

// Verified from M68000 code
INPUT_PORTS_START( ssf2mdb )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("EXTRA1")    // Extra buttons for Joypad 1 (6 button + start + mode) NOT READ DIRECTLY
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA2")    // Extra buttons for Joypad 2 (6 button + start + mode) NOT READ DIRECTLY
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")       // 3rd I/O port
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, "0 (Easiest)" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7 (Hardest)" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x0f, 0x0b, "Speed" )
	PORT_DIPSETTING(    0x0f, "0 (Slowest)" )
	PORT_DIPSETTING(    0x0e, "1" )
	PORT_DIPSETTING(    0x0d, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x0b, "4" )
	PORT_DIPSETTING(    0x0a, "5" )
	PORT_DIPSETTING(    0x09, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x07, "8" )
	PORT_DIPSETTING(    0x06, "9" )
	PORT_DIPSETTING(    0x05, "10 (Fastest)" )
//  PORT_DIPSETTING(    0x04, "10 (Fastest)" )
//  PORT_DIPSETTING(    0x03, "10 (Fastest)" )
//  PORT_DIPSETTING(    0x02, "10 (Fastest)" )
//  PORT_DIPSETTING(    0x01, "10 (Fastest)" )
//  PORT_DIPSETTING(    0x00, "10 (Fastest)" )
INPUT_PORTS_END

// Verified from M68000 code
INPUT_PORTS_START( mk3mdb )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("EXTRA1")    // Extra buttons for Joypad 1 (6 button + start + mode) NOT READ DIRECTLY
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("EXTRA2")    // Extra buttons for Joypad 2 (6 button + start + mode) NOT READ DIRECTLY
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x05, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( Hardest ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( Hardest ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Blood" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSWC")        // Not even read in this set
INPUT_PORTS_END

// Verified from M68000 code
INPUT_PORTS_START( aladmdb )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")     // Joypad 1 (3 button + start) NOT READ DIRECTLY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Throw") // a
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Sword") // b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Jump") // c
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 ) // start

	PORT_MODIFY("PAD2")     // Joypad 2 (3 button + start) NOT READ DIRECTLY - not used
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )

	// As I don't know how it is on real hardware, this is more a guess than anything
	PORT_START("MCU")
	PORT_DIPNAME( 0x07, 0x01, DEF_STR( Coinage ) )          // Code at 0x1b2a50 - unsure if there are so many settings
//  PORT_DIPSETTING(    0x00, "INVALID" )                   // Adds 0 credit
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )
//  PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM )         // To avoid it being changed and corrupting Coinage settings
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )       // Code at 0x1b2680
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )             // "PRACTICE"
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )           // "NORMAL"
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )             // "DIFFICULT"
//  PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1) // Needed to avoid credits getting mad
INPUT_PORTS_END

INPUT_PORTS_START( sonic2mb )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME(          0x0300, 0x0200, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(       0x0000, "1" )
	PORT_DIPSETTING(       0x0100, "2" )
	PORT_DIPSETTING(       0x0200, "3" )
	PORT_DIPSETTING(       0x0300, "4" )
	PORT_DIPNAME(  0x3c00, 0x2000, "Timer Speed" ) PORT_DIPLOCATION("SW1:3,4,5,6")
	PORT_DIPSETTING(       0x3c00, "0 (Slowest)" )
	PORT_DIPSETTING(       0x3800, "1" )
	PORT_DIPSETTING(       0x3400, "2" )
	PORT_DIPSETTING(       0x3000, "3" )
	PORT_DIPSETTING(       0x2c00, "4" )
	PORT_DIPSETTING(       0x2800, "5" )
	PORT_DIPSETTING(       0x2400, "6" )
	PORT_DIPSETTING(       0x2000, "7" )
	PORT_DIPSETTING(       0x1c00, "8" )
	PORT_DIPSETTING(       0x1800, "9" )
	PORT_DIPSETTING(       0x1400, "10" )
	PORT_DIPSETTING(       0x1000, "11" )
	PORT_DIPSETTING(       0x0c00, "12" )
	PORT_DIPSETTING(       0x0800, "13" )
	PORT_DIPSETTING(       0x0400, "14" )
	PORT_DIPSETTING(       0x0000, "15 (Fastest)" )
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x4000, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x8000, "SW1:8")
INPUT_PORTS_END

INPUT_PORTS_START( twinktmb )
	PORT_INCLUDE( aladmdb )

	// As I don't know how it is on real hardware, this is more a guess than anything

	PORT_MODIFY("MCU")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

// Verified from M68000 code
INPUT_PORTS_START( srmdb )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Shoot") // a
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Jump") // b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED ) // c (duplicate shoot button)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 ) // start

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Shoot") // a
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Jump") // b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED ) // c (duplicate shoot button)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN0")       // 3rd I/O port
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
INPUT_PORTS_END

static INPUT_PORTS_START( topshoot ) // Top Shooter Input Ports

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

static INPUT_PORTS_START( sbubsm )
	// the bit ordering in the ports is strange here because this is being read through shared RAM, the MCU presumably reads the real inputs then scrambles them in RAM for the 68k to sort out
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )  PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )  PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	// no service mode here?
INPUT_PORTS_END

INPUT_PORTS_START( barekch ) // TODO: identify dips. PCB has 3 x 8-dip banks, but probably most unused
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSWA")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSWC")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x00, "6" )
INPUT_PORTS_END

INPUT_PORTS_START( barek2ch )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 ) // also change character during gameplay

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 ) // also change character during gameplay

	PORT_START("IN0")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNUSED ) // apparently no use for these
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1") // at least some of the first 3 seem to control difficulty (enemies attack later / less frequently by switching these)
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xe0, 0xe0, "Starting Level" ) PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x00, "8" )

	PORT_START("DSWC") // present on PCB but there doesn't seem to be any read for them
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")
INPUT_PORTS_END

INPUT_PORTS_START( barek2 )
	PORT_INCLUDE( aladmdb )
	// TODO!
INPUT_PORTS_END

INPUT_PORTS_START( barek3 )
	PORT_INCLUDE( md_common )

	PORT_START("COINS")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
INPUT_PORTS_END

INPUT_PORTS_START( bk3ssrmb )
	PORT_INCLUDE( md_common )

	PORT_MODIFY("PAD1")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("PAD2")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN0")       // 3rd I/O port
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSWA") // PCB has three 8-dip banks, but only the first seems to influence game behaviour?
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DSWA:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) ) PORT_DIPLOCATION("DSWA:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSWA:6,7,8")
	PORT_DIPSETTING(    0x60, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWB:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWB:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWB:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWB:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWB:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWB:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWB:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSWB:8")

	PORT_START("DSWC")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWC:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWC:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWC:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWC:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWC:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWC:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWC:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSWC:8")
INPUT_PORTS_END

/*************************************
 *
 *  Machine Configuration
 *
 *************************************/

void md_boot_state::megadrvb(machine_config &config)
{
	md_ntsc(config);
}


void md_boot_6button_state::machine_start()
{
	md_base_state::machine_start();
	m_vdp->stop_timers();

	m_io_pad_6b[0] = ioport("EXTRA1");
	m_io_pad_6b[1] = ioport("EXTRA2");
	m_io_pad_6b[2] = ioport("IN0");
	m_io_pad_6b[3] = ioport("UNK");

	// Setup timers for 6 button pads
	for (int i = 0; i < 3; i++)
		m_io_timeout[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(md_base_state::io_timeout_timer_callback),this), (void*)(uintptr_t)i);
}

void md_boot_6button_state::megadrvb_6b(machine_config &config)
{
	md_ntsc(config);
}



/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

#define ENERGY_CONSOLE_MODE 0

void md_boot_state::init_aladmdb()
{
	// Game does a check @ 1afc00 with work RAM fff57c that makes it play like the original console version (i.e. 8 energy hits instead of 2)
	#if ENERGY_CONSOLE_MODE
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();
	rom[0x1afc08/2] = 0x6600;
	#endif

	// 220000 = writes to mcu? 330000 = reads?
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x220000, 0x220001, write16smo_delegate(*this, FUNC(md_boot_state::aladmdb_w)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x330000, 0x330001, read16smo_delegate(*this, FUNC(md_boot_state::aladmdb_r)));

	init_megadrij();
}

// This should be correct, the areas of the ROM that differ to the original
// after this decode look like intentional changes
void md_boot_state::init_mk3mdb()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int x = 0x000001; x < 0x100001; x += 2)
	{
		if (x & 0x80000)
		{
			rom[x] = rom[x] ^ 0xff;
			rom[x] = bitswap<8>(rom[x], 0,3,2,5,4,6,7,1);
		}
		else
		{
			rom[x] = rom[x] ^ 0xff;
			rom[x] = bitswap<8>(rom[x], 4,0,7,1,3,6,2,5);
		}
	}

	for (int x = 0x100001; x < 0x400000; x += 2)
	{
		if (x & 0x80000)
		{
			rom[x] = rom[x] ^ 0xff;
			rom[x] = bitswap<8>(rom[x], 2,7,5,4,1,0,3,6);
		}
		else
		{
			rom[x] = bitswap<8>(rom[x], 6,1,4,2,7,0,3,5);
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

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770070, 0x770075, read16sm_delegate(*this, FUNC(md_boot_state::dsw_r)));

	init_megadriv();
	// 6 button game, so overwrite 3 button io handlers
	m_megadrive_io_read_data_port_ptr = read8sm_delegate(*this, FUNC(md_base_state::megadrive_io_read_data_port_6button));
	m_megadrive_io_write_data_port_ptr = write16sm_delegate(*this, FUNC(md_base_state::megadrive_io_write_data_port_6button));
}

void md_boot_state::init_ssf2mdb()
{
	m_maincpu->space(AS_PROGRAM).nop_write(0xA130F0, 0xA130FF); // custom banking is disabled (!)
	m_maincpu->space(AS_PROGRAM).install_rom(0x400000, 0x5fffff, memregion( "maincpu" )->base() + 0x400000);
	m_maincpu->space(AS_PROGRAM).unmap_write(0x400000, 0x5fffff);
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770070, 0x770075, read16sm_delegate(*this, FUNC(md_boot_state::dsw_r)));

	init_megadrij();
	// 6 button game, so overwrite 3 button io handlers
	m_megadrive_io_read_data_port_ptr = read8sm_delegate(*this, FUNC(md_base_state::megadrive_io_read_data_port_6button));
	m_megadrive_io_write_data_port_ptr = write16sm_delegate(*this, FUNC(md_base_state::megadrive_io_write_data_port_6button));
}

void md_boot_state::init_srmdb()
{
	uint8_t* rom = memregion("maincpu")->base();

	for (int x = 0x00001; x < 0x40000; x += 2)
	{
		rom[x] = bitswap<8>(rom[x] ^ 0xff, 5,1,6,2,4,3,7,0);
	}

	for (int x = 0x40001; x < 0x80000; x += 2)
	{
		rom[x] = bitswap<8>(rom[x] ^ 0x00, 2,6,1,5,0,7,3,4);
	}

	// boot vectors don't seem to be valid, so they are patched...
	rom[0x01] = 0x01;
	rom[0x00] = 0x00;
	rom[0x03] = 0x00;
	rom[0x02] = 0x00;

	rom[0x06] = 0xd2;
	rom[0x07] = 0x00;

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770070, 0x770075, read16sm_delegate(*this, FUNC(md_boot_state::dsw_r)));

	init_megadriv();
}

void md_boot_state::init_topshoot()
{

	// these are shared RAM, MCU puts the inputs here
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200050, 0x200051, read16smo_delegate(*this, FUNC(md_boot_state::topshoot_200051_r)));
	m_maincpu->space(AS_PROGRAM).install_read_port(0x200042, 0x200043, "IN0");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x200044, 0x200045, "IN1");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x200046, 0x200047, "IN2");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x200048, 0x200049, "IN3");

	init_megadriv();
}


void md_boot_state::init_sbubsm()
{
	// needed to boot, somme kind of hardware ident?
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x400000, 0x400001, read16smo_delegate(*this, FUNC(md_boot_state::sbubsm_400000_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x400002, 0x400003, read16smo_delegate(*this, FUNC(md_boot_state::sbubsm_400002_r)));

	// these are shared RAM, MCU puts the inputs here
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x200050, 0x200051, read16smo_delegate(*this, FUNC(md_boot_state::topshoot_200051_r))); // needed for coins to work
	m_maincpu->space(AS_PROGRAM).install_read_port(0x200042, 0x200043, "IN0");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x200044, 0x200045, "IN1");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x200046, 0x200047, "IN2");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x200048, 0x200049, "IN3");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x20007e, 0x20007f, "IN4");

	init_megadriv();
}


void md_boot_state::init_barek2()
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x220000, 0x220001, write16smo_delegate(*this, FUNC(md_boot_state::aladmdb_w)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x330000, 0x330001, read16smo_delegate(*this, FUNC(md_boot_state::barek2mb_r)));

	init_megadrij();
}

void md_boot_state::init_barekch()
{
	uint16_t *src = (uint16_t *)memregion("maincpu")->base();

	for (int i = 0x000000; i < 0x80000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0xff00, 15, 9, 12, 8, 14, 13, 11, 10, 7, 6, 5, 4, 3, 2, 1, 0);

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770070, 0x770075, read16sm_delegate(*this, FUNC(md_boot_state::dsw_r)));

	init_megadrij();
}

void md_boot_state::init_barek2ch()
{
	uint16_t *src = (uint16_t *)memregion("maincpu")->base();

	for (int i = 0x000000; i < 0x200000 / 2; i++)
		src[i] = bitswap<16>(src[i], 8, 11, 10, 13, 12, 14, 15, 9, 7, 6, 5, 4, 3, 2, 1, 0);

	src[0x06 / 2] = 0x0210; // TODO: why is this needed?

	m_maincpu->space(AS_PROGRAM).install_read_port(0x380070, 0x380071, "IN0");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x380078, 0x380079, "DSWA");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x38007a, 0x38007b, "DSWB");

	init_megadrij();
}

void md_boot_state::init_barek3()
{
	uint8_t* rom = memregion("maincpu")->base();

	for (int x = 0x00001; x < 0x300000; x += 2)
	{
		rom[x] = bitswap<8>(rom[x], 6,2,4,0,7,1,3,5);
	}

	m_maincpu->space(AS_PROGRAM).install_read_port(0x380070, 0x380071, "COINS");
	m_maincpu->space(AS_PROGRAM).install_read_port(0x380078, 0x380079, "DSW");

	init_megadrij();
}

void md_boot_state::init_bk3ssrmb()
{
	uint8_t* rom = memregion("maincpu")->base();

	for (int x = 0x00001; x < 0x80000; x += 2)
	{
		rom[x] = bitswap<8>(rom[x] ^ 0xff, 3, 1, 6, 4, 7, 0, 2, 5);
	}

	for (int x = 0x80001; x < 0x100000; x += 2)
	{
		rom[x] = bitswap<8>(rom[x], 3, 7, 0, 5, 1, 6, 2, 4);
	}

	for (int x = 0x100001; x < 0x300000; x += 2)
	{
		rom[x] = bitswap<8>(rom[x], 1, 7, 6, 4, 5, 2, 3, 0);
	}

	for (int x = 0x300001; x < 0x380000; x += 2)
	{
		rom[x] = bitswap<8>(rom[x] ^ 0xff, 3, 1, 6, 4, 7, 0, 2, 5);
	}

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x770070, 0x770075, read16sm_delegate(*this, FUNC(md_boot_state::dsw_r)));


	init_megadrij();
}

void md_boot_state::init_sonic2mb()
{
	// 100000 = writes to unpopulated MCU?
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x100000, 0x100001, write16smo_delegate(*this, FUNC(md_boot_state::aladmdb_w)));
	m_maincpu->space(AS_PROGRAM).install_read_port(0x300000, 0x300001, "DSW");

	init_megadrij();
}

void md_boot_state::init_sonic3mb()
{
	// m_maincpu->space(AS_PROGRAM).install_write_handler(0x200000, 0x200001, write16smo_delegate(*this, FUNC(md_boot_state::sonic3mb_w))); // seems to write to PIC from here
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x300000, 0x300001, read16smo_delegate(*this, FUNC(md_boot_state::sonic3mb_r))); // reads from PIC from here

	init_megadrij();
}

void md_boot_state::init_twinktmb()
{
	// boot vectors don't seem to be valid, so they are patched...
	uint8_t* rom = memregion("maincpu")->base();
	rom[0x01] = 0x00;

	rom[0x04] = 0x00;
	rom[0x07] = 0x46;
	rom[0x06] = 0xcc;

	init_megadrij();
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x100000, 0x100001, write16smo_delegate(*this, FUNC(md_boot_state::aladmdb_w)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x300000, 0x300001, read16smo_delegate(*this, FUNC(md_boot_state::twinktmb_r)));
}

void md_boot_state::init_jparkmb()
{
	init_megadrij();
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x100000, 0x100001, write16smo_delegate(*this, FUNC(md_boot_state::aladmdb_w)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x300000, 0x300001, read16smo_delegate(*this, FUNC(md_boot_state::jparkmb_r)));
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( aladmdb )
	ROM_REGION( 0x400000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "m1.bin", 0x000001, 0x080000,  CRC(5e2671e4) SHA1(54705c7614fc7b5a1065478fa41f51dd1d8045b7) )
	ROM_LOAD16_BYTE( "m2.bin", 0x000000, 0x080000,  CRC(142a0366) SHA1(6c94aa9936cd11ccda503b52019a6721e64a32f0) )
	ROM_LOAD16_BYTE( "m3.bin", 0x100001, 0x080000,  CRC(0feeeb19) SHA1(bd567a33077ab9997871d21736066140d50e3d70) )
	ROM_LOAD16_BYTE( "m4.bin", 0x100000, 0x080000,  CRC(bc712661) SHA1(dfd554d000399e17b4ddc69761e572195ed4e1f0) )

	ROM_REGION( 0x1000, "pic", ROMREGION_ERASE00 )
	ROM_LOAD( "pic16c57xtp", 0x0000, 0x1000, NO_DUMP )
ROM_END

ROM_START( mk3mdb ) // ROMs are scrambled, we take care of the address descramble in the ROM load, and the data descramble in the init
					// This is bootlegged from  "Mortal Kombat 3 (4) [!].bin"
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 ) // 68000 Code
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

ROM_START( ssf2mdb )
	ROM_REGION( 0x1400000, "maincpu", 0 ) // 68000 Code
	// Special Case, custom PCB, linear ROM mapping of 5meg
	ROM_LOAD16_BYTE( "rom_a", 0x000000, 0x200000,  CRC(59726521) SHA1(3120bac17f56c01ffb9d3f9e31efa0263e3774af) )
	ROM_LOAD16_BYTE( "rom_b", 0x000001, 0x200000,  CRC(7dad5540) SHA1(9279068b2218d239fdd557dd959ac70e74853178) )
	ROM_LOAD16_BYTE( "rom_c", 0x400000, 0x080000,  CRC(deb48624) SHA1(39ffa7de7b808e0b95cb039bb381705d77420933) )
	ROM_LOAD16_BYTE( "rom_d", 0x400001, 0x080000,  CRC(b99f6a5b) SHA1(adbe28a7522024bc66328ac86fecf9ded3310e8e) )
ROM_END

ROM_START( srmdb )
	ROM_REGION( 0x400000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "u1", 0x000001, 0x020000,  CRC(c59f33bd) SHA1(bd5bce7698a70ea005b79ab34bcdb056872ef980) )
	ROM_LOAD16_BYTE( "u2", 0x000000, 0x020000,  CRC(9125c054) SHA1(c73bdeb6b11c59d2b5f5968959b02697957ca894) )
	ROM_LOAD16_BYTE( "u3", 0x040001, 0x020000,  CRC(0fee0fbe) SHA1(001e0fda12707512aad537e533acf28e726e6107) )
	ROM_LOAD16_BYTE( "u4", 0x040000, 0x020000,  CRC(fc2aed41) SHA1(27eb3957f5ed26ee5276523b1df46fa7eb298e1f) )
ROM_END

ROM_START( topshoot ) // Top Shooter (c)1995 Sun Mixing
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tc574000ad_u11_2.bin", 0x000000, 0x080000, CRC(b235c4d9) SHA1(fbb308a5f6e769f3277824cb6a3b50c308969ac2) )
	ROM_LOAD16_BYTE( "tc574000ad_u12_1.bin", 0x000001, 0x080000, CRC(e826f6ad) SHA1(23ec8bb608f954d3b915f061e7076c0c63b8259e) )

	// Not hooked up yet
	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "89c51.bin", 0x0000, 0x1000, CRC(595475c8) SHA1(8313819ba06cc92b54f88c1ca9f34be8d1ec94d0) )
ROM_END

ROM_START( sbubsm )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u11.bin", 0x000000, 0x080000, CRC(4f9337ea) SHA1(b245eb615f80afd25e29b2efdddb7f61c1deff6b) )
	ROM_LOAD16_BYTE( "u12.bin", 0x000001, 0x080000, CRC(f5374835) SHA1(3a97910f5f7327ec7ad6425dfdfa72c86196ed33) )

	ROM_REGION( 0x1000, "mcu", 0 ) // could be the same as topshoot (same PCB)
	ROM_LOAD( "89c51.bin", 0x0000, 0x1000, NO_DUMP )
ROM_END

ROM_START( sonic2mb )
	ROM_REGION( 0x400000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "m1", 0x000001, 0x080000,  CRC(7b40aa24) SHA1(247882cd1f412366d61aeb4d85bbeefd5f108e1d) )
	ROM_LOAD16_BYTE( "m2", 0x000000, 0x080000,  CRC(84b3f758) SHA1(19846b9d951db6f78f3e155d33f1b6349fb87f1a) )
ROM_END

ROM_START( sonic3mb )
	ROM_REGION( 0x400000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "sonic3-4.bin", 0x000000, 0x080000, CRC(b7318bb8) SHA1(1707b563794c3ab4a1f04cb449efdd6f817317fb) )
	ROM_LOAD16_BYTE( "sonic3-3.bin", 0x000001, 0x080000, CRC(1898479f) SHA1(5f1c581157959e11979882d2180ae4b98c6a89d5) )
	ROM_LOAD16_BYTE( "sonic3-2.bin", 0x100000, 0x080000, CRC(02232f45) SHA1(8cdcb156603108ac9d3ef888f75adb5327abce1a) )
	ROM_LOAD16_BYTE( "sonic3-1.bin", 0x100001, 0x080000, CRC(cee2f679) SHA1(4cc7a8a228f7fc4f7a38c69a65585765751a49e5) )

	ROM_REGION( 0x1000, "pic", ROMREGION_ERASE00 )
	ROM_LOAD( "pic16c57xtp", 0x0000, 0x1000, NO_DUMP )
ROM_END

ROM_START( barek2mb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "m1.bin", 0x000001, 0x080000,  CRC(1c1fa718) SHA1(393488f7747478728eb4f20c10b0cfce3b188719) )
	ROM_LOAD16_BYTE( "m2.bin", 0x000000, 0x080000,  CRC(59ee0905) SHA1(0e9f1f6e17aae2dd99bf9d7f640568b48ba699c7) )
	ROM_LOAD16_BYTE( "m3.bin", 0x100001, 0x080000,  CRC(6ec5af5d) SHA1(9088a2d4cff5e7eb439ebaa91ad3bfff11366127) )
	ROM_LOAD16_BYTE( "m4.bin", 0x100000, 0x080000,  CRC(d8c61e0d) SHA1(3d06e656f6621bb0741211f80c1ecff1669475ee) )

	// Not hooked up yet
	ROM_REGION( 0x1000, "pic", ROMREGION_ERASE00 )
	ROM_LOAD( "bk_pic16c57rcp.bin", 0x0000, 0x1000, CRC(434ad1b7) SHA1(9241554793c7375cf58239e762481a4b80a51df6) ) // Unprotected
ROM_END

ROM_START( barek3mb )
	ROM_REGION( 0x400000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "6.u19", 0x000000, 0x080000,  CRC(2de19519) SHA1(f5fcef1da8b5370e399f0451382e3c6e7754c9c8) )
	ROM_LOAD16_BYTE( "3.u18", 0x000001, 0x080000,  CRC(db900e82) SHA1(172a4fe01a0ffd1ea3aed74f2c58234fd55b876d) )
	ROM_LOAD16_BYTE( "4.u15", 0x100000, 0x080000,  CRC(6353b4b1) SHA1(9f89a2f02170496ca798b89e37e1f2bae0e9155d) )
	ROM_LOAD16_BYTE( "1.u14", 0x100001, 0x080000,  CRC(24d31e12) SHA1(64c1b968e1ee5d0355d902e280f33e4466f27b07) )
	ROM_LOAD16_BYTE( "5.u17", 0x200000, 0x080000,  CRC(0feb974f) SHA1(ed1a25b6f1669dc6061d519985b6373fa89176c7) )
	ROM_LOAD16_BYTE( "2.u16", 0x200001, 0x080000,  CRC(bba4a585) SHA1(32c59729943d7b4c1a39f2a2b0dae9ce16991e9c) )
ROM_END

ROM_START( bk3ssrmb )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.u15", 0x000000, 0x080000,  CRC(120a4b64) SHA1(7084fde0d08143f48f83d2afef30522d75c7889c) )
	ROM_LOAD16_BYTE( "5.u14", 0x000001, 0x080000,  CRC(1eb625d3) SHA1(8f67ab874643b3eafe91768df84ea4a3b8a5fa82) )
	ROM_LOAD16_BYTE( "2.u13", 0x100000, 0x080000,  CRC(af439685) SHA1(5cc55608355e11096c9fadb8d2460cf66704deec) )
	ROM_LOAD16_BYTE( "6.u12", 0x100001, 0x080000,  CRC(24d31e12) SHA1(64c1b968e1ee5d0355d902e280f33e4466f27b07) )
	ROM_LOAD16_BYTE( "3.u11", 0x200000, 0x080000,  CRC(dfa5c478) SHA1(079a9cad5c2252b2d65aa3c4dc9cba331078eeb9) )
	ROM_LOAD16_BYTE( "7.u10", 0x200001, 0x080000,  CRC(bba4a585) SHA1(32c59729943d7b4c1a39f2a2b0dae9ce16991e9c) )
	ROM_LOAD16_BYTE( "4.u9",  0x300000, 0x040000,  CRC(e5f1ab97) SHA1(0f4c527043f1272e75a996f4f7270c6ea4ed3c4d) )
	ROM_LOAD16_BYTE( "8.u8",  0x300001, 0x040000,  CRC(32ee1048) SHA1(1b135c200b4440e95a7d1766b4b404ddd238872d) )
ROM_END

ROM_START( twinktmb ) // Same PCB as sonic2mb, but in this one the PIC is populated
	ROM_REGION( 0x400000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "m2.bin", 0x000000, 0x080000,  CRC(44424f8f) SHA1(e16318bfdf869765c821c264cf9a7e6c728f7073) )
	ROM_LOAD16_BYTE( "m1.bin", 0x000001, 0x080000,  CRC(69aa916e) SHA1(7ea6b571fd0b6494051d5846ee9b4564b7692766) )

	ROM_REGION( 0x2000, "pic", ROMREGION_ERASE00 )
	ROM_LOAD( "pic16c57xtp", 0x0000, 0x2000, NO_DUMP )
ROM_END

ROM_START( jparkmb ) // Same PCB as twinktmb, JPA-028 label
	ROM_REGION( 0x400000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_BYTE( "f24.bin", 0x000000, 0x080000,  CRC(bdd851d2) SHA1(1a75922e848fd5c7cd4ab102c99effcfcf382097) )
	ROM_LOAD16_BYTE( "f23.bin", 0x000001, 0x080000,  CRC(8dc66c71) SHA1(a2741ffa583a4b779b7be3e3ae628e97f792ee3d) )
	ROM_LOAD16_BYTE( "f22.bin", 0x100000, 0x080000,  CRC(36337d06) SHA1(d537cff2c8ed58da146faf390c09252be359ccd1) )
	ROM_LOAD16_BYTE( "f21.bin", 0x100001, 0x080000,  CRC(6ede6b6b) SHA1(cf29300d9278ea03f54cf54ea582bdd8b9bbdbbd) )

	ROM_REGION( 0x1000, "pic", ROMREGION_ERASE00 )
	ROM_LOAD( "pic16c57xtp", 0x0000, 0x1000, NO_DUMP )
ROM_END

ROM_START( barekch ) // all 27c010
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.u1", 0x000001, 0x020000, CRC(a119b5ef) SHA1(710ef6dc340a2c3741af69cd9a3d16e5fdd73be6) )
	ROM_LOAD16_BYTE( "2.u2", 0x000000, 0x020000, CRC(7d4ad276) SHA1(9ab2a28356cc5c36eee8dba40c04a64cf5d2cfde) )
	ROM_LOAD16_BYTE( "3.u3", 0x040001, 0x020000, CRC(af6a9122) SHA1(0f2bac1ad20f5918b04dd5a503121445029e4c84) )
	ROM_LOAD16_BYTE( "4.u4", 0x040000, 0x020000, CRC(98245384) SHA1(f4f96f369764a7d204ec414f053b25da662ff401) )
ROM_END

ROM_START( barek2ch ) // all 27c4001
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u14", 0x000001, 0x080000, CRC(b0ee177f) SHA1(d63e6ee30fe7f4aaab098d3920eabc456730b2c5) )
	ROM_LOAD16_BYTE( "u15", 0x000000, 0x080000, CRC(09264195) SHA1(c5439731d932c90a57d68c4d82c9ebed8a01bd53) )
	ROM_LOAD16_BYTE( "u16", 0x100001, 0x080000, CRC(6c814fc4) SHA1(edaf5117b19d3fb40218c5f7c4b5099c9189f1be) )
	ROM_LOAD16_BYTE( "u17", 0x100000, 0x080000, CRC(cae1922e) SHA1(811c2164b6c467a49af4b0d22f151cd13c9efbc9) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1993, aladmdb,  0, megadrvb,     aladmdb,  md_boot_state, init_aladmdb,  ROT0, "bootleg / Sega",   "Aladdin (bootleg of Japanese Megadrive version)",                                       0 )
GAME( 1996, mk3mdb,   0, megadrvb_6b,  mk3mdb,   md_boot_6button_state, init_mk3mdb,   ROT0, "bootleg / Midway", "Mortal Kombat 3 (bootleg of Megadrive version)",                                        0 )
GAME( 1994, ssf2mdb,  0, megadrvb_6b,  ssf2mdb,  md_boot_6button_state, init_ssf2mdb,  ROT0, "bootleg / Capcom", "Super Street Fighter II - The New Challengers (bootleg of Japanese MegaDrive version)", 0 )
GAME( 1993, srmdb,    0, megadrvb,     srmdb,    md_boot_state, init_srmdb,    ROT0, "bootleg / Konami", "Sunset Riders (bootleg of Megadrive version)",                                          0 )
GAME( 1995, topshoot, 0, md_bootleg,   topshoot, md_boot_state, init_topshoot, ROT0, "Sun Mixing",       "Top Shooter",                                                                           0 )
GAME( 1996, sbubsm,   0, md_bootleg,   sbubsm,   md_boot_state, init_sbubsm,   ROT0, "Sun Mixing",       "Super Bubble Bobble (Sun Mixing, Megadrive clone hardware)",                            0 )
GAME( 1993, sonic2mb, 0, md_bootleg,   sonic2mb, md_boot_state, init_sonic2mb, ROT0, "bootleg / Sega",   "Sonic The Hedgehog 2 (bootleg of Megadrive version)",                                   0 ) // Flying wires going through the empty PIC space aren't completely understood
GAME( 1993, sonic3mb, 0, md_bootleg,   twinktmb, md_boot_state, init_sonic3mb, ROT0, "bootleg / Sega",   "Sonic The Hedgehog 3 (bootleg of Megadrive version)",                                   MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // undumped PIC
GAME( 1994, barek2mb, 0, md_bootleg,   barek2,   md_boot_state, init_barek2,   ROT0, "bootleg / Sega",   "Bare Knuckle II (bootleg of Megadrive version)",                                        MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // Needs PIC hook up
GAME( 1994, barek3mb, 0, megadrvb,     barek3,   md_boot_state, init_barek3,   ROT0, "bootleg / Sega",   "Bare Knuckle III (bootleg of Megadrive version)",                                       0 )
GAME( 1994, bk3ssrmb, 0, megadrvb_6b,  bk3ssrmb, md_boot_6button_state, init_bk3ssrmb, ROT0, "bootleg / Sega",   "Bare Knuckle III / Sunset Riders (bootleg of Megadrive versions)",                      MACHINE_NOT_WORKING ) // Currently boots as Bare Knuckle III, mechanism to switch game not found yet
GAME( 1993, twinktmb, 0, md_bootleg,   twinktmb, md_boot_state, init_twinktmb, ROT0, "bootleg / Sega",   "Twinkle Tale (bootleg of Megadrive version)",                                           MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // Needs PIC decap or simulation
GAME( 1993, jparkmb,  0, md_bootleg,   twinktmb, md_boot_state, init_jparkmb,  ROT0, "bootleg / Sega",   "Jurassic Park (bootleg of Megadrive version)",                                          MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING ) // Needs PIC decap or simulation

// Chinese bootlegs. Very clean looking with custom chips marked TA-04, TA-05 and TA-06.
GAME( 1994, barekch,  0, megadrvb_6b,  barekch,  md_boot_6button_state, init_barekch,  ROT0, "bootleg",          "Bare Knuckle (Chinese bootleg of Megadrive version)",                           0 )
GAME( 1994, barek2ch, 0, md_bootleg,   barek2ch, md_boot_state,         init_barek2ch, ROT0, "bootleg",          "Bare Knuckle II (Chinese bootleg of Megadrive version)",                        0 )
