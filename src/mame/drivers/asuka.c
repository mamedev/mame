// license:BSD-3-Clause
// copyright-holders:David Graves, Brian Troha
/***************************************************************************

Asuka & Asuka  (+ Taito/Visco games on similar hardware)
=============

David Graves, Brian Troha

Made out of:    Rastan driver by Jarek Burczynski
                MAME Taito F2 driver
                Raine source - very special thanks to
                  Richard Bush and the Raine Team.
                two different drivers for Bonze Adventure that were
                  written at the same time by Yochizo and Frotz

    Bonze Adventure (c) 1988 Taito Corporation
    Asuka & Asuka   (c) 1988 Taito Corporation
    Maze of Flott   (c) 1989 Taito Corporation
    Galmedes        (c) 1992 Visco Corporation
    Earth Joker     (c) 1993 Visco Corporation
    Kokontouzai Eto Monogatari (c) 1994 Visco Corporation

Main CPU: MC68000 uses irq 5 (4 in bonze, 4&5 in cadash).
Sound   : Z80 & YM2151 + MSM5205 (YM2610 in bonze)
Chips   : TC0100SCN + TC0002OBJ + TC0110PCR (+ C-Chip in bonze)
(Bryan McPhail:  My Bonze uses TC0100SCN + PC0900J (OBJ) + TC0110PCR + TC0140SYT (SND))

Memory map for Asuka & Asuka
----------------------------

The other games seem identical but Eto is slightly different.

0x000000 - 0x0fffff : ROM (not all used for each game)
0x100000 - 0x103fff : 16k of RAM
0x200000 - 0x20000f : palette generator
0x400000 - 0x40000f : input ports and dipswitches
0x3a0000 - 0x3a0003 : sprite control
0x3e0000 - 0x3e0003 : communication with sound CPU
0xc00000 - 0xc2000f : TC0100SCN (see taitoic.c)
0xd00000 - 0xd007ff : sprite RAM


Cadashu Info (Malcor)
---------------------

Main PCB (JAMMA) K1100528A
Main processor  - 68000 12MHz
                - HD64180RP8 8MHz (8 bit processor, dual channel DMAC,
                             memory mapped I/O, used for multigame link)
Misc custom ICs including three PQFPs, one PGA, and one SIP


From "garmedes.txt"
-------------------

The following cord is written, on PCB:  K1100388A   J1100169A   M6100708A
There are the parts that were written as B68 on this PCB.
The original title of the game called B68 is unknown.
This PCB is the same as the one that is used with EARTH-JOKER.
<B68 is the verified Taito ROM id# for Asuka & Asuka - B.Troha>


Use of TC0100SCN
----------------

Asuka & Asuka: $e6a init code clearing TC0100SCN areas is erroneous.
It only clears 1/8 of the BG layers; then it clears too much of the
rowscroll areas [0xc000, 0xc400] causing overrun into next 64K block.

Asuka is one of the early Taito games using the TC0100SCN. (Ninja
Warriors was probably the first.) They didn't bother using its FG (text)
layer facility, instead placing text in the BG / sprite layers.

Maze of Flott [(c) one year later] and most other games with the
TC0100SCN do use the FG layer for text (Driftout is an exception).


Stephh's notes (based on the game M68000 code and some tests) :

1) 'bonzeadv', 'jigkmgri' and 'bonzeadvu'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'bonzeadv' : region = 0x0002
      * 'jigkmgri' : region = 0x0000
      * 'bonzeadvu': region = 0x0001
  - These 3 games are 100% the same, only region differs !
  - Coinage relies on the region (code at 0x02d344) :
      * 0x0000 (Japan) and 0x0001 (US) use TAITO_COINAGE_JAPAN_OLD_LOC()
      * 0x0002 (World) uses TAITO_COINAGE_WORLD_LOC()
  - Notice screen only if region = 0x0000
  - Texts and game name rely on the region :
      * 0x0000 : most texts in Japanese - game name is "Jigoku Meguri"
      * other : all texts in English - game name is "Bonze Adventure"
  - Bonus lives aren't awarded correctly due to bogus code at 0x00961e :

      00961E: 302D 0B7E                  move.w  ($b7e,A5), D0
      009622: 0240 0018                  andi.w  #$18, D0
      009626: E648                       lsr.w   #3, D0

    Here is what the correct code should be :

      00961E: 302D 0B7E                  move.w  ($b7e,A5), D0
      009622: 0240 0030                  andi.w  #$30, D0
      009626: E848                       lsr.w   #4, D0

  - DSWB bit 7 was previously used to allow map viewing (C-Chip test ?),
    but it is now unused due to "bra" instruction at 0x007572


2) 'bonzeadvo'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'bonzeadvo' : region = 0x0002
  - The only difference is that the following code is missing :

      00D218: 08AD 0004 15DE             bclr    #$4, ($15de,A5)

    So the "crouch" bit wasn't always reset, which may cause you
    to consume all your magic powers in less than 4 frames !
    See bonzeadv0107u1ora full report on MAME Testers site
  - Same other notes as for 'bonzeadv'


3) 'asuka*'

  - No region
  - BOTH sets use TAITO_COINAGE_JAPAN_OLD_LOC() for coinage,
    so I wonder if the World version isn't a US version
  - Additional notice screen in 'asukaj'


4) 'mofflott'

  - Region stored at 0x03fffe.w
  - Sets :
      * 'mofflott' : region = 0x0001
  - Coinage relies on the region (code at 0x0145ec) :
      * 0x0001 (Japan) and 0x0002 (US ?) use TAITO_COINAGE_JAPAN_OLD_LOC()
      * 0x0003 (World) uses TAITO_COINAGE_WORLD_LOC()
  - Notice screen only if region = 0x0001


5) 'cadash*'

  - Region stored at 0x07fffe.w
  - Sets :
      * 'cadash'   : region = 0x0003
      * 'cadashj'  : region = 0x0001
      * 'cadashu'  : region = 0x0002
      * 'cadashfr' : region = 0x0003
      * 'cadashit' : region = 0x0003
  - These 5 games are 100% the same, only region differs !
    However each version requires its specific texts
  - Coinage relies on the region (code at 0x0013d6) :
      * 0x0001 (Japan) uses TAITO_COINAGE_JAPAN_OLD_LOC()
      * 0x0002 (US) uses TAITO_COINAGE_US_LOC()
      * 0x0003 (World) uses TAITO_COINAGE_WORLD_LOC()
  - Notice screen only if region = 0x0001 or region = 0x0002
  - FBI logo only if region = 0x0002
  - I can't tell about the Italian and Japanese versions,
    but translation in the French version is really poor !


6) 'galmedes'

  - No region (not a Taito game anyway)
  - Coinage relies on "Coin Mode" Dip Switch (code at 0x0801c0) :
      * "Mode A" uses TAITO_COINAGE_JAPAN_OLD_LOC()
      * "Mode B" uses TAITO_COINAGE_WORLD_LOC()
  - Notice screen


7) 'earthjkr'

  - No region (not a Taito game anyway)
  - Game uses TAITO_COINAGE_JAPAN_OLD_LOC()
  - Notice screen only if "Copyright" Dip Switch set to "Visco"


8) 'eto'

  - No region (not a Taito game anyway)
  - Game uses TAITO_COINAGE_JAPAN_OLD_LOC()
  - No notice screen


TODO
----

Mofflot: $14c46 sub inits sound system: in a pause loop during this
it reads a dummy address.

Earthjkr: Wrong screen size? Left edge of green blueprints in
attract looks like it's incorrectly off screen.

Cadash: Hooks for twin arcade machine setup: will involve emulating an extra
microcontroller, the 07 rom might be the program for it. Cadash background
colors don't reinitialize properly with save states.

Galmedes: Test mode has select1/2 stuck at on.

Eto: $76d0 might be a protection check? It reads to and writes from
the prog rom. Doesn't seem to cause problems though.

DIP locations verified for:
    - bonzeadv (manual)
    - cadash (manual)
    - asuka (manual)
    - mofflott (manual)
    - galmedes (manual)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z180/z180.h"
#include "cpu/m68000/m68000.h"
#include "includes/taitoipt.h"
#include "audio/taitosnd.h"
#include "sound/2610intf.h"
#include "sound/2151intf.h"
#include "sound/msm5205.h"
#include "includes/asuka.h"


/***********************************************************
                INTERRUPTS
***********************************************************/

void asuka_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_CADASH_INTERRUPT5:
		m_maincpu->set_input_line(5, HOLD_LINE);
		break;
	default:
		assert_always(FALSE, "Unknown id in asuka_state::device_timer");
	}
}


INTERRUPT_GEN_MEMBER(asuka_state::cadash_interrupt)
{
	timer_set(downcast<cpu_device *>(&device)->cycles_to_attotime(500), TIMER_CADASH_INTERRUPT5);
	device.execute().set_input_line(4, HOLD_LINE);  /* interrupt vector 4 */
}


/************************************************
            SOUND
************************************************/

WRITE8_MEMBER(asuka_state::sound_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x03);
}

WRITE8_MEMBER(asuka_state::sound_bankswitch_2151_w)
{
	membank("bank1")->set_entry(data & 0x03);
}



WRITE_LINE_MEMBER(asuka_state::asuka_msm5205_vck)
{
	if (m_adpcm_data != -1)
	{
		m_msm->data_w(m_adpcm_data & 0x0f);
		m_adpcm_data = -1;
	}
	else
	{
		m_adpcm_data = memregion("ymsnd")->base()[m_adpcm_pos];
		m_adpcm_pos = (m_adpcm_pos + 1) & 0xffff;
		m_msm->data_w(m_adpcm_data >> 4);
	}
}

WRITE8_MEMBER(asuka_state::asuka_msm5205_address_w)
{
	m_adpcm_pos = (m_adpcm_pos & 0x00ff) | (data << 8);
}

WRITE8_MEMBER(asuka_state::asuka_msm5205_start_w)
{
	m_msm->reset_w(0);
}

WRITE8_MEMBER(asuka_state::asuka_msm5205_stop_w)
{
	m_msm->reset_w(1);
	m_adpcm_pos &= 0xff00;
}

READ16_MEMBER(asuka_state::cadash_share_r)
{
	return m_cadash_shared_ram[offset];
}

WRITE16_MEMBER(asuka_state::cadash_share_w)
{
	m_cadash_shared_ram[offset] = data & 0xff;
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

static ADDRESS_MAP_START( bonzeadv_map, AS_PROGRAM, 16, asuka_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x0fffff) AM_ROM
	AM_RANGE(0x10c000, 0x10ffff) AM_RAM
	AM_RANGE(0x200000, 0x200007) AM_DEVREADWRITE("tc0110pcr", tc0110pcr_device, word_r, step1_word_w)
	AM_RANGE(0x390000, 0x390001) AM_READ_PORT("DSWA")
	AM_RANGE(0x3a0000, 0x3a0001) AM_WRITE(asuka_spritectrl_w)
	AM_RANGE(0x3b0000, 0x3b0001) AM_READ_PORT("DSWB")
	AM_RANGE(0x3c0000, 0x3c0001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0x3d0000, 0x3d0001) AM_READNOP
	AM_RANGE(0x3e0000, 0x3e0001) AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x3e0002, 0x3e0003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0x800000, 0x8007ff) AM_READWRITE(bonzeadv_cchip_ram_r, bonzeadv_cchip_ram_w)
	AM_RANGE(0x800802, 0x800803) AM_READWRITE(bonzeadv_cchip_ctrl_r, bonzeadv_cchip_ctrl_w)
	AM_RANGE(0x800c00, 0x800c01) AM_WRITE(bonzeadv_cchip_bank_w)
	AM_RANGE(0xc00000, 0xc0ffff) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, word_r, word_w)    /* tilemaps */
	AM_RANGE(0xc20000, 0xc2000f) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, ctrl_word_r, ctrl_word_w)
	AM_RANGE(0xd00000, 0xd03fff) AM_DEVREADWRITE("pc090oj", pc090oj_device, word_r, word_w)  /* sprite ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( asuka_map, AS_PROGRAM, 16, asuka_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x1076f0, 0x1076f1) AM_READNOP /* Mofflott init does dummy reads here */
	AM_RANGE(0x200000, 0x20000f) AM_DEVREADWRITE("tc0110pcr", tc0110pcr_device, word_r, step1_word_w)
	AM_RANGE(0x3a0000, 0x3a0003) AM_WRITE(asuka_spritectrl_w)
	AM_RANGE(0x3e0000, 0x3e0001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x3e0002, 0x3e0003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0x400000, 0x40000f) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, read, write, 0x00ff)
	AM_RANGE(0xc00000, 0xc0ffff) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, word_r, word_w)    /* tilemaps */
	AM_RANGE(0xc10000, 0xc103ff) AM_WRITENOP    /* error in Asuka init code */
	AM_RANGE(0xc20000, 0xc2000f) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, ctrl_word_r, ctrl_word_w)
	AM_RANGE(0xd00000, 0xd03fff) AM_DEVREADWRITE("pc090oj", pc090oj_device, word_r, word_w)  /* sprite ram */
ADDRESS_MAP_END

static ADDRESS_MAP_START( cadash_map, AS_PROGRAM, 16, asuka_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080003) AM_WRITE(asuka_spritectrl_w)
	AM_RANGE(0x0c0000, 0x0c0001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x0c0002, 0x0c0003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0x100000, 0x107fff) AM_RAM
	AM_RANGE(0x800000, 0x800fff) AM_READWRITE(cadash_share_r,cadash_share_w)    /* network ram */
	AM_RANGE(0x900000, 0x90000f) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, read, write, 0x00ff)
	AM_RANGE(0xa00000, 0xa0000f) AM_DEVREADWRITE("tc0110pcr", tc0110pcr_device, word_r, step1_4bpg_word_w)
	AM_RANGE(0xb00000, 0xb03fff) AM_DEVREADWRITE("pc090oj", pc090oj_device, word_r, word_w)  /* sprite ram */
	AM_RANGE(0xc00000, 0xc0ffff) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, word_r, word_w)    /* tilemaps */
	AM_RANGE(0xc20000, 0xc2000f) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, ctrl_word_r, ctrl_word_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( eto_map, AS_PROGRAM, 16   /* N.B. tc100scn mirror overlaps spriteram */, asuka_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10000f) AM_DEVREADWRITE("tc0110pcr", tc0110pcr_device, word_r, step1_word_w)
	AM_RANGE(0x200000, 0x203fff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_DEVREADWRITE8("tc0220ioc", tc0220ioc_device, read, write, 0x00ff)
	AM_RANGE(0x400000, 0x40000f) AM_DEVREAD8("tc0220ioc", tc0220ioc_device, read, 0x00ff)   /* service mode mirror */
	AM_RANGE(0x4a0000, 0x4a0003) AM_WRITE(asuka_spritectrl_w)
	AM_RANGE(0x4e0000, 0x4e0001) AM_READNOP AM_DEVWRITE8("tc0140syt", tc0140syt_device, master_port_w, 0x00ff)
	AM_RANGE(0x4e0002, 0x4e0003) AM_DEVREADWRITE8("tc0140syt", tc0140syt_device, master_comm_r, master_comm_w, 0x00ff)
	AM_RANGE(0xc00000, 0xc03fff) AM_DEVREADWRITE("pc090oj", pc090oj_device, word_r, word_w)  /* sprite ram */
	AM_RANGE(0xc00000, 0xc0ffff) AM_DEVWRITE("tc0100scn", tc0100scn_device, word_w)
	AM_RANGE(0xd00000, 0xd0ffff) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, word_r, word_w)    /* tilemaps */
	AM_RANGE(0xd20000, 0xd2000f) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, ctrl_word_r, ctrl_word_w)
ADDRESS_MAP_END


/***************************************************************************/

static ADDRESS_MAP_START( bonzeadv_z80_map, AS_PROGRAM, 8, asuka_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe003) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0xe200, 0xe200) AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xe201, 0xe201) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xe400, 0xe403) AM_WRITENOP /* pan */
	AM_RANGE(0xe600, 0xe600) AM_WRITENOP
	AM_RANGE(0xee00, 0xee00) AM_WRITENOP
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP
	AM_RANGE(0xf200, 0xf200) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( z80_map, AS_PROGRAM, 8, asuka_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
//  AM_RANGE(0x9002, 0x9100) AM_READNOP
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
	AM_RANGE(0xb000, 0xb000) AM_WRITE(asuka_msm5205_address_w)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(asuka_msm5205_start_w)
	AM_RANGE(0xd000, 0xd000) AM_WRITE(asuka_msm5205_stop_w)
ADDRESS_MAP_END

/* no MSM5205 */
static ADDRESS_MAP_START( cadash_z80_map, AS_PROGRAM, 8, asuka_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("tc0140syt", tc0140syt_device, slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_DEVREADWRITE("tc0140syt", tc0140syt_device, slave_comm_r, slave_comm_w)
ADDRESS_MAP_END

/*
Cadash communication CPU is a z180.
[0x8000]: at pc=31, z180 checks a byte ... if it's equal to 0x4d ("M") then the board is in master mode, otherwise it's in slave mode.
Right now, the z180 is too fast, so it never checks it properly ... maybe I'm missing a z180 halt line that's lying to somewhere on m68k side.
[0x8002]: puts T in master mode, R in slave mode ... looks a rather obvious flag that says the current Tx / Rx state
[0x8080-0x80ff]: slave data
[0x8100-0x817f]: master data

Internal I/O Asynchronous SCI regs are then checked ... we can't emulate this at the current time, needs two MAME instances.

m68k M communicates with z180 M through shared ram, then the z180 M communicates with z180 S through these ASCI regs ... finally, the z180 S
communicates with m68k S with its own shared ram. In short:

m68k M -> z180 M <-> z180 S <- m68k S
*/

static ADDRESS_MAP_START( cadash_sub_map, AS_PROGRAM, 8, asuka_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("sharedram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cadash_sub_io, AS_IO, 8, asuka_state )
	AM_RANGE(0x00, 0x3f) AM_RAM // z180 internal I/O regs
ADDRESS_MAP_END

/***********************************************************
             INPUT PORTS, DIPs
***********************************************************/

#define CADASH_PLAYERS_INPUT( player ) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) \
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(player) \
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(player) \
	INPUT_GENERIC_JOY_HIGH_NIBBLE(player, IP_ACTIVE_LOW, PORT_8WAY, RIGHT, LEFT, DOWN, UP)


/* different players and system inputs than 'asuka' */
static INPUT_PORTS_START( bonzeadv )
	/* 0x390000 -> 0x10cb7c ($b7c,A5) */
	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* 0x3b0000 -> 0x10cb7e ($b7e,A5) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SWB:3,4") /* see notes */
	PORT_DIPSETTING(    0x08, "40k 100k" )                  /* 300k 1000k 1500k 2000k 2500k 3000k 3500k 5000k */
	PORT_DIPSETTING(    0x0c, "50k 150k" )                  /* 500k 1000k 2000k 3000k 4000k 5000k 6000k 7000k */
	PORT_DIPSETTING(    0x04, "60k 200k" )                  /* 500k 1000k 2000k 3000k 4000k 5000k 6000k 7000k */
	PORT_DIPSETTING(    0x00, "80k 250k" )                  /* 500k 1000k 2000k 3000k 4000k 5000k 6000k 7000k */
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )            /* see notes */

	PORT_START("800007")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("800009")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("80000B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("80000D")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( jigkmgri )
	PORT_INCLUDE(bonzeadv)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END

static INPUT_PORTS_START( asuka )
	/* 0x400000 -> 0x103618 */
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)

	/* 0x400002 -> 0x10361c */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, "Bonus Points" )              PORT_DIPLOCATION("SWB:3,4") /* for each plane shot after each end of level boss */
	PORT_DIPSETTING(    0x0c, "500" )
	PORT_DIPSETTING(    0x08, "1500" )
	PORT_DIPSETTING(    0x04, "2000" )
	PORT_DIPSETTING(    0x00, "2500" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0xc0, "Up To Level 2" )
	PORT_DIPSETTING(    0x80, "Up To Level 3" )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )

	PORT_START("IN0")
	TAITO_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN1")
	TAITO_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_START2 )
INPUT_PORTS_END

static INPUT_PORTS_START( mofflott )
	PORT_INCLUDE(asuka)

	/* 0x400000 -> 0x100a92.b */
	PORT_MODIFY("DSWA")
	TAITO_MACHINE_COCKTAIL_LOC(SWA)

	/* 0x400002 -> 0x100a93.b */
	PORT_MODIFY("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "20k And Every 50k" )
	PORT_DIPSETTING(    0x08, "50k And Every 100k" )
	PORT_DIPSETTING(    0x04, "100k Only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )            PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Number Of Keys" )            PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, "B 14" )                      /* Hard */
	PORT_DIPSETTING(    0x80, "A 16" )                      /* Easy */
INPUT_PORTS_END

/* different players and system inputs than 'asuka' */
static INPUT_PORTS_START( cadash )
	/* 0x900000 -> 0x10317a ($317a,A5) */
	PORT_START("DSWA")
	TAITO_MACHINE_NO_COCKTAIL_LOC(SWA)
	TAITO_COINAGE_WORLD_LOC(SWA)

	/* 0x900002 -> 0x10317c ($317c,A5) */
	PORT_START("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x0c, "Starting Time" )         PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, "5:00" )
	PORT_DIPSETTING(    0x04, "6:00" )
	PORT_DIPSETTING(    0x0c, "7:00" )
	PORT_DIPSETTING(    0x08, "8:00" )
	/* Round cleared   Added time   */
	/*       1            8:00  */
	/*       2           10:00  */
	/*       3            8:00  */
	/*       4            7:00  */
	/*       5            9:00  */
	PORT_DIPNAME( 0x30, 0x30, "Added Time (after round clear)" ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, "Default - 2:00" )
	PORT_DIPSETTING(    0x10, "Default - 1:00" )
	PORT_DIPSETTING(    0x30, "Default" )
	PORT_DIPSETTING(    0x20, "Default + 1:00" )
	PORT_DIPNAME( 0xc0, 0xc0, "Communication Mode" )    PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0xc0, "Stand alone" )
	PORT_DIPSETTING(    0x80, "Master" )
	PORT_DIPSETTING(    0x00, "Slave" )
//  PORT_DIPSETTING(    0x40, "Stand alone" )

	PORT_START("IN0")
	CADASH_PLAYERS_INPUT( 1 )

	PORT_START("IN1")
	CADASH_PLAYERS_INPUT( 2 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cadashj )
	PORT_INCLUDE(cadash)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_OLD_LOC(SWA)
INPUT_PORTS_END

static INPUT_PORTS_START( cadashu )
	PORT_INCLUDE(cadash)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US_LOC(SWA)
INPUT_PORTS_END

static INPUT_PORTS_START( galmedes )
	PORT_INCLUDE(asuka)

	/* 0x400000 -> 0x100982 */
	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x80)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSWB",0x80,EQUALS,0x00)

	/* 0x400002 -> 0x100984 */
	PORT_MODIFY("DSWB")
	TAITO_DIFFICULTY_LOC(SWB)
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, "Every 100k" )
	PORT_DIPSETTING(    0x0c, "100k And Every 200k" )
	PORT_DIPSETTING(    0x04, "150k And Every 200k" )
	PORT_DIPSETTING(    0x00, "Every 200k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, "Coin Mode" )             PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, "Mode A (Japan)" )        /* Mode A is TAITO_COINAGE_JAPAN_OLD */
	PORT_DIPSETTING(    0x00, "Mode B (World)" )        /* Mode B is TAITO_COINAGE_WORLD */
INPUT_PORTS_END

static INPUT_PORTS_START( earthjkr )
	PORT_INCLUDE(asuka)
	/* DSWA: 0x400000 -> 0x100932 */

	/* 0x400002 -> 0x1009842 */
	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, "100k and 300k" )
	PORT_DIPSETTING(    0x08, "100k only" )
	PORT_DIPSETTING(    0x04, "200k only" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x40, 0x40, "Copyright" )             PORT_DIPLOCATION("SWB:7") /* code at 0x00b982 and 0x00dbce */
	PORT_DIPSETTING(    0x40, "Visco" )                          /* Japan notice screen ON */
	PORT_DIPSETTING(    0x00, "Visco (distributed by Romstar)" ) /* Japan notice screen OFF */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( eto )
	PORT_INCLUDE(asuka)
	/* DSWA: 0x300000 -> 0x200914 */

	/* 0x300002 -> 0x200916 */
	PORT_MODIFY("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SWB:6" )    /* value stored at 0x20090a but not read back */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END


/**************************************************************
                GFX DECODING
**************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
		10*4, 11*4, 8*4, 9*4, 14*4, 15*4, 12*4, 13*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static GFXDECODE_START( asuka )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  0, 256 )   /* OBJ */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0, 256 )   /* SCR */
GFXDECODE_END



/**************************************************************
                SOUND
**************************************************************/

WRITE_LINE_MEMBER(asuka_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

void asuka_state::machine_start()
{
	/* configure the banks */
	membank("bank1")->configure_entry(0, memregion("audiocpu")->base());
	membank("bank1")->configure_entries(1, 3, memregion("audiocpu")->base() + 0x10000, 0x04000);

	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_data));

	save_item(NAME(m_current_round));
	save_item(NAME(m_current_bank));
	save_item(NAME(m_video_ctrl));
	save_item(NAME(m_video_mask));
	save_item(NAME(m_cc_port));
	save_item(NAME(m_restart_status));
	save_item(NAME(m_cval));
}

void asuka_state::machine_reset()
{
	m_adpcm_pos = 0;
	m_adpcm_data = -1;
	m_current_round = 0;
	m_current_bank = 0;
	m_video_ctrl = 0;
	m_video_mask = 0;
	m_cc_port = 0;
	m_restart_status = 0;

	memset(m_cval, 0, 26);
}

void asuka_state::screen_eof_asuka(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		m_pc090oj->eof_callback();
	}
}

static MACHINE_CONFIG_START( bonzeadv, asuka_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)    /* checked on PCB */
	MCFG_CPU_PROGRAM_MAP(bonzeadv_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", asuka_state,  irq4_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,4000000)    /* sound CPU, also required for test mode */
	MCFG_CPU_PROGRAM_MAP(bonzeadv_z80_map)


	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 3*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(asuka_state, screen_update_bonzeadv)
	MCFG_SCREEN_VBLANK_DRIVER(asuka_state, screen_eof_asuka)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", asuka)
	MCFG_PALETTE_ADD("palette", 4096)

	MCFG_DEVICE_ADD("pc090oj", PC090OJ, 0)
	MCFG_PC090OJ_OFFSETS(0, 8)
	MCFG_PC090OJ_GFXDECODE("gfxdecode")
	MCFG_PC090OJ_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0100scn", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(1)
	MCFG_TC0100SCN_TX_REGION(2)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette")

	MCFG_TC0110PCR_ADD("tc0110pcr")
	MCFG_TC0110PCR_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, 8000000)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(asuka_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( asuka, asuka_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz/2)   /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(asuka_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", asuka_state,  irq5_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_16MHz/4) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(z80_map)


	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(asuka_state, screen_update_asuka)
	MCFG_SCREEN_VBLANK_DRIVER(asuka_state, screen_eof_asuka)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", asuka)
	MCFG_PALETTE_ADD("palette", 4096)

	MCFG_DEVICE_ADD("pc090oj", PC090OJ, 0)
	MCFG_PC090OJ_OFFSETS(0, 8)
	MCFG_PC090OJ_USEBUFFER(1)
	MCFG_PC090OJ_GFXDECODE("gfxdecode")
	MCFG_PC090OJ_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0100scn", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(1)
	MCFG_TC0100SCN_TX_REGION(2)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette")

	MCFG_TC0110PCR_ADD("tc0110pcr")
	MCFG_TC0110PCR_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL_16MHz/4) /* verified on pcb */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(asuka_state,sound_bankswitch_2151_w))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_384kHz) /* verified on pcb */
	MCFG_MSM5205_VCLK_CB(WRITELINE(asuka_state, asuka_msm5205_vck))  /* VCK function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( cadash, asuka_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_32MHz/2)   /* 68000p12 running at 16Mhz, verified on pcb  */
	MCFG_CPU_PROGRAM_MAP(cadash_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", asuka_state,  cadash_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_8MHz/2)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(cadash_z80_map)

	MCFG_CPU_ADD("subcpu", Z180, 4000000)   /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(cadash_sub_map)
	MCFG_CPU_IO_MAP(cadash_sub_io)


	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(asuka_state, screen_update_bonzeadv)
	MCFG_SCREEN_VBLANK_DRIVER(asuka_state, screen_eof_asuka)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", asuka)
	MCFG_PALETTE_ADD("palette", 4096)

	MCFG_DEVICE_ADD("pc090oj", PC090OJ, 0)
	MCFG_PC090OJ_OFFSETS(0, 8)
	MCFG_PC090OJ_USEBUFFER(1)
	MCFG_PC090OJ_GFXDECODE("gfxdecode")
	MCFG_PC090OJ_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0100scn", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(1)
	MCFG_TC0100SCN_TX_REGION(2)
	MCFG_TC0100SCN_OFFSETS(1, 0)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette")

	MCFG_TC0110PCR_ADD("tc0110pcr")
	MCFG_TC0110PCR_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL_8MHz/2)   /* verified on pcb */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(asuka_state,sound_bankswitch_2151_w))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mofflott, asuka_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)    /* 8 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(asuka_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", asuka_state,  irq5_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)  /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(z80_map)


	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(asuka_state, screen_update_asuka)
	MCFG_SCREEN_VBLANK_DRIVER(asuka_state, screen_eof_asuka)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", asuka)
	MCFG_PALETTE_ADD("palette", 4096)   /* only Mofflott uses full palette space */

	MCFG_DEVICE_ADD("pc090oj", PC090OJ, 0)
	MCFG_PC090OJ_OFFSETS(0, 8)
	MCFG_PC090OJ_GFXDECODE("gfxdecode")
	MCFG_PC090OJ_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0100scn", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(1)
	MCFG_TC0100SCN_TX_REGION(2)
	MCFG_TC0100SCN_OFFSETS(1, 0)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette")

	MCFG_TC0110PCR_ADD("tc0110pcr")
	MCFG_TC0110PCR_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(asuka_state,sound_bankswitch_2151_w))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_SOUND_ADD("msm", MSM5205, 384000)
	MCFG_MSM5205_VCLK_CB(WRITELINE(asuka_state, asuka_msm5205_vck))  /* VCK function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8 kHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( galmedes, asuka_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)    /* 8 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(asuka_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", asuka_state,  irq5_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)  /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(cadash_z80_map)


	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(asuka_state, screen_update_asuka)
	MCFG_SCREEN_VBLANK_DRIVER(asuka_state, screen_eof_asuka)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", asuka)
	MCFG_PALETTE_ADD("palette", 4096)   /* only Mofflott uses full palette space */

	MCFG_DEVICE_ADD("pc090oj", PC090OJ, 0)
	MCFG_PC090OJ_OFFSETS(0, 8)
	MCFG_PC090OJ_GFXDECODE("gfxdecode")
	MCFG_PC090OJ_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0100scn", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(1)
	MCFG_TC0100SCN_TX_REGION(2)
	MCFG_TC0100SCN_OFFSETS(1, 0)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette")

	MCFG_TC0110PCR_ADD("tc0110pcr")
	MCFG_TC0110PCR_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(asuka_state,sound_bankswitch_2151_w))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( eto, asuka_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)    /* 8 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(eto_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", asuka_state,  irq5_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)  /* 4 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(cadash_z80_map)


	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	MCFG_DEVICE_ADD("tc0220ioc", TC0220IOC, 0)
	MCFG_TC0220IOC_READ_0_CB(IOPORT("DSWA"))
	MCFG_TC0220IOC_READ_1_CB(IOPORT("DSWB"))
	MCFG_TC0220IOC_READ_2_CB(IOPORT("IN0"))
	MCFG_TC0220IOC_READ_3_CB(IOPORT("IN1"))
	MCFG_TC0220IOC_READ_7_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(asuka_state, screen_update_asuka)
	MCFG_SCREEN_VBLANK_DRIVER(asuka_state, screen_eof_asuka)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", asuka)
	MCFG_PALETTE_ADD("palette", 4096)

	MCFG_DEVICE_ADD("pc090oj", PC090OJ, 0)
	MCFG_PC090OJ_OFFSETS(0, 8)
	MCFG_PC090OJ_GFXDECODE("gfxdecode")
	MCFG_PC090OJ_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0100scn", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(1)
	MCFG_TC0100SCN_TX_REGION(2)
	MCFG_TC0100SCN_OFFSETS(1, 0)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette")

	MCFG_TC0110PCR_ADD("tc0110pcr")
	MCFG_TC0110PCR_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_YM2151_PORT_WRITE_HANDLER(WRITE8(asuka_state,sound_bankswitch_2151_w))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 0.50)

	MCFG_DEVICE_ADD("tc0140syt", TC0140SYT, 0)
	MCFG_TC0140SYT_MASTER_CPU("maincpu")
	MCFG_TC0140SYT_SLAVE_CPU("audiocpu")
MACHINE_CONFIG_END


/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( bonzeadv )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "b41-09-1.17", 0x00000, 0x10000, CRC(af821fbc) SHA1(55bc13742033a31c92d6268d6b8344062ca78633) )
	ROM_LOAD16_BYTE( "b41-11-1.26", 0x00001, 0x10000, CRC(823fff00) SHA1(b8b8cafbe860136c202d8d9f3ed5a54e2f4df363) )
	ROM_LOAD16_BYTE( "b41-10.16",   0x20000, 0x10000, CRC(4ca94d77) SHA1(69a9f6bcb6d5e4132eed50860bdfe8d6b6d914cd) )
	ROM_LOAD16_BYTE( "b41-15.25",   0x20001, 0x10000, CRC(aed7a0d0) SHA1(99ffc0b0e88b81231756610bf48df5365e12603b) )
	/* 0x040000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD_SWAP( "b41-01.15", 0x80000, 0x80000, CRC(5d072fa4) SHA1(6ffe1b8531381eb6dd3f1fec18c91294a6aca9f6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b41-03.1",  0x00000, 0x80000, CRC(736d35d0) SHA1(7d41a7d71e117714bbd2cdda2953589cda6e763a) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b41-02.7",  0x00000, 0x80000, CRC(29f205d9) SHA1(9e9f0c2755a9aa5acfe2601911bfa07d8d61164c) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "b41-13.20",  0x00000, 0x04000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) )
	ROM_CONTINUE(        0x10000, 0x0c000 )   /* banked stuff */

	/* CPU3 - CCHIP aka TC0030CMD marked b41-05.43 */

	ROM_REGION( 0x80000, "ymsnd", 0 )     /* ADPCM samples */
	ROM_LOAD( "b41-04.48",  0x00000, 0x80000, CRC(c668638f) SHA1(07238a6cb4d93ffaf6351657163b5d80f0dbf688) )
ROM_END

ROM_START( bonzeadvo )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "b41-09.17",   0x00000, 0x10000, CRC(06818710) SHA1(b8045f4e15246231a5645d22bb965953f7fb47a3) )
	ROM_LOAD16_BYTE( "b41-11.26",   0x00001, 0x10000, CRC(33c4c2f4) SHA1(3f1e76932d8f7e06e976b968a711177d25254bef) )
	ROM_LOAD16_BYTE( "b41-10.16",   0x20000, 0x10000, CRC(4ca94d77) SHA1(69a9f6bcb6d5e4132eed50860bdfe8d6b6d914cd) )
	ROM_LOAD16_BYTE( "b41-15.25",   0x20001, 0x10000, CRC(aed7a0d0) SHA1(99ffc0b0e88b81231756610bf48df5365e12603b) )
	/* 0x040000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD_SWAP( "b41-01.15", 0x80000, 0x80000, CRC(5d072fa4) SHA1(6ffe1b8531381eb6dd3f1fec18c91294a6aca9f6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b41-03.1",  0x00000, 0x80000, CRC(736d35d0) SHA1(7d41a7d71e117714bbd2cdda2953589cda6e763a) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b41-02.7",  0x00000, 0x80000, CRC(29f205d9) SHA1(9e9f0c2755a9aa5acfe2601911bfa07d8d61164c) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "b41-13.20",  0x00000, 0x04000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) )
	ROM_CONTINUE(        0x10000, 0x0c000 )   /* banked stuff */

	/* CPU3 - CCHIP aka TC0030CMD marked b41-05.43 */

	ROM_REGION( 0x80000, "ymsnd", 0 )     /* ADPCM samples */
	ROM_LOAD( "b41-04.48",  0x00000, 0x80000, CRC(c668638f) SHA1(07238a6cb4d93ffaf6351657163b5d80f0dbf688) )
ROM_END

ROM_START( bonzeadvu )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "b41-09-1.17", 0x00000, 0x10000, CRC(af821fbc) SHA1(55bc13742033a31c92d6268d6b8344062ca78633) )
	ROM_LOAD16_BYTE( "b41-11-1.26", 0x00001, 0x10000, CRC(823fff00) SHA1(b8b8cafbe860136c202d8d9f3ed5a54e2f4df363) )
	ROM_LOAD16_BYTE( "b41-10.16",   0x20000, 0x10000, CRC(4ca94d77) SHA1(69a9f6bcb6d5e4132eed50860bdfe8d6b6d914cd) )
	ROM_LOAD16_BYTE( "b41-14.25",   0x20001, 0x10000, CRC(37def16a) SHA1(b0a3b7206db55e29454672fffadf4e2a64eed873) )
	/* 0x040000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD_SWAP( "b41-01.15", 0x80000, 0x80000, CRC(5d072fa4) SHA1(6ffe1b8531381eb6dd3f1fec18c91294a6aca9f6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b41-03.1",  0x00000, 0x80000, CRC(736d35d0) SHA1(7d41a7d71e117714bbd2cdda2953589cda6e763a) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b41-02.7",  0x00000, 0x80000, CRC(29f205d9) SHA1(9e9f0c2755a9aa5acfe2601911bfa07d8d61164c) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "b41-13.20",  0x00000, 0x04000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) )
	ROM_CONTINUE(        0x10000, 0x0c000 )   /* banked stuff */

	/* CPU3 - CCHIP aka TC0030CMD marked b41-05.43 */

	ROM_REGION( 0x80000, "ymsnd", 0 )     /* ADPCM samples */
	ROM_LOAD( "b41-04.48",  0x00000, 0x80000, CRC(c668638f) SHA1(07238a6cb4d93ffaf6351657163b5d80f0dbf688) )
ROM_END

ROM_START( jigkmgri )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "b41-09-1.17", 0x00000, 0x10000, CRC(af821fbc) SHA1(55bc13742033a31c92d6268d6b8344062ca78633) )
	ROM_LOAD16_BYTE( "b41-11-1.26", 0x00001, 0x10000, CRC(823fff00) SHA1(b8b8cafbe860136c202d8d9f3ed5a54e2f4df363) )
	ROM_LOAD16_BYTE( "b41-10.16",   0x20000, 0x10000, CRC(4ca94d77) SHA1(69a9f6bcb6d5e4132eed50860bdfe8d6b6d914cd) )
	ROM_LOAD16_BYTE( "b41-12.25",   0x20001, 0x10000, CRC(40d9c1fc) SHA1(6f03d263e10559988aaa2be00d9bbf55f2fb864e) )
	/* 0x040000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD_SWAP( "b41-01.15", 0x80000, 0x80000, CRC(5d072fa4) SHA1(6ffe1b8531381eb6dd3f1fec18c91294a6aca9f6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b41-03.1",  0x00000, 0x80000, CRC(736d35d0) SHA1(7d41a7d71e117714bbd2cdda2953589cda6e763a) ) /* Tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b41-02.7",  0x00000, 0x80000, CRC(29f205d9) SHA1(9e9f0c2755a9aa5acfe2601911bfa07d8d61164c) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "b41-13.20",  0x00000, 0x04000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) )
	ROM_CONTINUE(        0x10000, 0x0c000 )   /* banked stuff */

	/* CPU3 - CCHIP aka TC0030CMD marked b41-05.43 */

	ROM_REGION( 0x80000, "ymsnd", 0 )     /* ADPCM samples */
	ROM_LOAD( "b41-04.48",  0x00000, 0x80000, CRC(c668638f) SHA1(07238a6cb4d93ffaf6351657163b5d80f0dbf688) )
ROM_END

ROM_START( bonzeadvp ) /* Labels consists of hand written checksum values of the ROMs */
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 68000 code */
	ROM_LOAD16_BYTE( "0l.ic17", 0x00000, 0x10000, CRC(9e046e6f) SHA1(a05ed46930bcfa8f59fda6f1d370b841ad261258) )
	ROM_LOAD16_BYTE( "0h.ic26", 0x00001, 0x10000, CRC(3e2b2628) SHA1(66ee0e5d2c38c467edc3f22b83b73643764ae8f0) )
	ROM_LOAD16_BYTE( "1h.ic16", 0x20000, 0x10000, CRC(52f31b98) SHA1(8a20a79350073438522361d3f598afa42f0f62ed) )
	ROM_LOAD16_BYTE( "1l.ic25", 0x20001, 0x10000, CRC(c7e79b98) SHA1(1d92861c6337362cdd9d31a2da944d8eb3171170) )
	/* 0x040000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_BYTE( "fd65.ic20",  0x80001, 0x20000, CRC(c32f3bd5) SHA1(d9db14ec26cac5504a61058e87da4e404647ca94) ) // these 4 == b41-01.15 but split
	ROM_LOAD16_BYTE( "49eb.ic26",  0x80000, 0x20000, CRC(c747650b) SHA1(ef03931d233ec1f9e61d45d02abb23e69edd8c15) ) // ^
	ROM_LOAD16_BYTE( "a418.ic23",  0xc0001, 0x20000, CRC(51b02be6) SHA1(20e3423aea359f3ca92dd24f4f87351d93c279b6) ) // ^
	ROM_LOAD16_BYTE( "0e7e.ic28",  0xc0000, 0x20000, CRC(dc1f9fd0) SHA1(88addfa2c3bb854efd88a3556d23a7607b6ec848) ) // ^

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "abbe.ic9",   0x00000, 0x20000, CRC(50e6581c) SHA1(230724d65c9b1ea5d72117dca077464dd599ad68) ) // first 2 == first half b41-03.1 but split
	ROM_LOAD16_BYTE( "0ac8.ic15",  0x00001, 0x20000, CRC(29002fc4) SHA1(5ddbefc0d173865802362990e99a3b542c096412) ) // ^
	ROM_LOAD16_BYTE( "5ebf.ic5",   0x40000, 0x20000, CRC(dac6f11f) SHA1(8c79d05ca539ebfbec35c7426c207937745c1949) ) // these 2 have differences
	ROM_LOAD16_BYTE( "77c8.ic12",  0x40001, 0x20000, CRC(d8aaae12) SHA1(240dda7d7e74ffc6a084c39ca19903fd35ad0157) ) // ^

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "9369.ic19",  0x00000, 0x20000, CRC(a9dd7f90) SHA1(c3acf2dcd9325b9a74967d4b9cfff59bdb4045c6) ) // these 4 == b41-02.7 but split
	ROM_LOAD16_BYTE( "e3ed.ic25",  0x00001, 0x20000, CRC(7cc66ee2) SHA1(145d3bd0e3ef765874fc679e709391d516e74ef0) ) // ^
	ROM_LOAD16_BYTE( "03eb.ic16",  0x40000, 0x20000, CRC(39f32715) SHA1(5c555fde1ae0bb1e796e0122157bc694392122f3) ) // ^
	ROM_LOAD16_BYTE( "b8e1.ic22",  0x40001, 0x20000, CRC(15b836cf) SHA1(0f7e5cb6a57c336125909e28af664fe7387947d4) ) // ^

	ROM_REGION( 0x1c000, "audiocpu", 0 )     /* sound cpu */
	ROM_LOAD( "b41-13.20",  0x00000, 0x04000, CRC(9e464254) SHA1(b6f6126b54c15320ecaa652d0eeabaa4cd94bd26) ) // missing from dump
	ROM_CONTINUE(           0x10000, 0x0c000)   /* banked stuff */

	/* is the C-Chip the same as the final? */

	ROM_REGION( 0x80000, "ymsnd", 0 )     /* ADPCM samples */
	ROM_LOAD( "6089.ic17",  0x00000, 0x20000, CRC(b092783c) SHA1(e13f765e2884b6194926bf982595de18376ffef9) ) // these 4 == b41-04.48 but split
	ROM_LOAD( "2e1f.ic14",  0x20000, 0x20000, CRC(df1f87c0) SHA1(ad3df38c22f1bb7bdc449922bd3c2a5c78aa87f8) ) // ^
	ROM_LOAD( "f66e.ic11",  0x40000, 0x20000, CRC(c6df1b3e) SHA1(84d6ad3e3af565060aa4324c6e3e91e4dc5089b6) ) // ^
	ROM_LOAD( "49d7.ic7",   0x60000, 0x20000, CRC(5584c02c) SHA1(00402df66debb257c97a609a37de0f8eeeb6e9f0) ) // ^
ROM_END

ROM_START( asuka )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "b68-13.bin",  0x00000, 0x20000, CRC(855efb3e) SHA1(644e02e207adeaec7839c824688d88ab8d046418) )
	ROM_LOAD16_BYTE( "b68-12.bin",  0x00001, 0x20000, CRC(271eeee9) SHA1(c08e347be4aae929c0ab95ff7618edaa1a7d6da9) )
	/* 0x040000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD( "b68-03.bin",  0x80000, 0x80000, CRC(d3a59b10) SHA1(35a2ff18b64e73ac5e17484354c0cc58bc2cd7fc) )    /* Fix ROM */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b68-01.bin",  0x00000, 0x80000, CRC(89f32c94) SHA1(74fbb699e05e2336509cb5ac06ed94335ff870d5) )   /* SCR tiles (8 x 8) */

	ROM_REGION( 0xa0000, "gfx2", 0 )
	ROM_LOAD       ( "b68-02.bin", 0x00000, 0x80000, CRC(f5018cd3) SHA1(860ce140ae369556d03d5d78987b87c0d6070df5) ) /* Sprites (16 x 16) */
	ROM_LOAD16_BYTE( "b68-07.bin", 0x80000, 0x10000, CRC(c113acc8) SHA1(613c61a78df73dcb0b9c9018ae829e865baac772) )
	ROM_LOAD16_BYTE( "b68-06.bin", 0x80001, 0x10000, CRC(f517e64d) SHA1(8be491bfe0f7eed58521de9d31da677acf635c23) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b68-11.bin", 0x00000, 0x04000, CRC(c378b508) SHA1(1b145fe736b924f298e02532cf9f26cc18b42ca7) )
	ROM_CONTINUE(             0x10000, 0x0c000 )    /* banked stuff */

	ROM_REGION( 0x10000, "ymsnd", 0 )   /* ADPCM samples */
	ROM_LOAD( "b68-10.bin", 0x00000, 0x10000, CRC(387aaf40) SHA1(47c583564ef1d49ece15f97221b2e073e8fb0544) )

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.bin", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.bin", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( asukaj )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "b68-09.bin",  0x00000, 0x20000, CRC(1eaa1bbb) SHA1(01ca6a5f3c47dab49654b84601119714eb329cc5) )
	ROM_LOAD16_BYTE( "b68-08.bin",  0x00001, 0x20000, CRC(8cc96e60) SHA1(dc94f3fd48c0407ec72e8330bc688e9e16d39213) )
	/* 0x040000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD( "b68-03.bin",  0x80000, 0x80000, CRC(d3a59b10) SHA1(35a2ff18b64e73ac5e17484354c0cc58bc2cd7fc) )    /* Fix ROM */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b68-01.bin",  0x00000, 0x80000, CRC(89f32c94) SHA1(74fbb699e05e2336509cb5ac06ed94335ff870d5) )   /* SCR tiles (8 x 8) */

	ROM_REGION( 0xa0000, "gfx2", 0 )
	ROM_LOAD       ( "b68-02.bin", 0x00000, 0x80000, CRC(f5018cd3) SHA1(860ce140ae369556d03d5d78987b87c0d6070df5) ) /* Sprites (16 x 16) */
	ROM_LOAD16_BYTE( "b68-07.bin", 0x80000, 0x10000, CRC(c113acc8) SHA1(613c61a78df73dcb0b9c9018ae829e865baac772) )
	ROM_LOAD16_BYTE( "b68-06.bin", 0x80001, 0x10000, CRC(f517e64d) SHA1(8be491bfe0f7eed58521de9d31da677acf635c23) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "b68-11.bin", 0x00000, 0x04000, CRC(c378b508) SHA1(1b145fe736b924f298e02532cf9f26cc18b42ca7) )
	ROM_CONTINUE(             0x10000, 0x0c000 )    /* banked stuff */

	ROM_REGION( 0x10000, "ymsnd", 0 )   /* ADPCM samples */
	ROM_LOAD( "b68-10.bin", 0x00000, 0x10000, CRC(387aaf40) SHA1(47c583564ef1d49ece15f97221b2e073e8fb0544) )

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.bin", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.bin", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( mofflott )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "c17-09.bin",  0x00000, 0x20000, CRC(05ee110f) SHA1(8cedd911d3fdcca1e409260d12dd03a2fb35ef86) )
	ROM_LOAD16_BYTE( "c17-08.bin",  0x00001, 0x20000, CRC(d0aacffd) SHA1(2c5ec4020aad2c1cd3a004dc70a12e0d77eb6aa7) )
	/* 0x40000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD( "c17-03.bin",  0x80000, 0x80000, CRC(27047fc3) SHA1(1f88a7a42a94bac0e164a69896ae168ab821fbb3) )    /* Fix ROM */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c17-01.bin",  0x00000, 0x80000, CRC(e9466d42) SHA1(93d533a9a992e3ff537e914577ede41729235826) )   /* SCR tiles (8 x 8) */

	ROM_REGION( 0xa0000, "gfx2", 0 )
	ROM_LOAD       ( "c17-02.bin", 0x00000, 0x80000, CRC(8860a8db) SHA1(372adea8835a9524ece30ab71181ef9d05b120e9) ) /* Sprites (16 x 16) */
	ROM_LOAD16_BYTE( "c17-05.bin", 0x80000, 0x10000, CRC(57ac4741) SHA1(3188ff0866324c68fba8e9745a0cb186784cb53d) )
	ROM_LOAD16_BYTE( "c17-04.bin", 0x80001, 0x10000, CRC(f4250410) SHA1(1f5f6baca4aa695ce2ae5c65adcb460da872a239) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c17-07.bin", 0x00000, 0x04000, CRC(cdb7bc2c) SHA1(5113055c954a39918436db75cc06b53c29c60728) )
	ROM_CONTINUE(           0x10000, 0x0c000 )  /* banked stuff */

	ROM_REGION( 0x10000, "ymsnd", 0 )   /* ADPCM samples */
	ROM_LOAD( "c17-06.bin", 0x00000, 0x10000, CRC(5c332125) SHA1(408f42df18b38347c8a4e177a9484162a66877e1) )
ROM_END

ROM_START( cadash )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c21_14.ic11",  0x00000, 0x20000, CRC(5daf13fb) SHA1(c2be42b2cdc90b6463ce87211cf711c951b17fab) )
	ROM_LOAD16_BYTE( "c21_16.ic15",  0x00001, 0x20000, CRC(cbaa2e75) SHA1(c41ea71f2b0e72bf993dfcfd30f1994cae9f52a0) )
	ROM_LOAD16_BYTE( "c21_13.ic10",  0x40000, 0x20000, CRC(6b9e0ee9) SHA1(06314b9c0be19314e6b6ecb5274a63eb36b642f5) )
	ROM_LOAD16_BYTE( "c21_17.ic14",  0x40001, 0x20000, CRC(bf9a578a) SHA1(42bde46081db6be2f61eaf171438ecc9264d18be) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c21-08.38",   0x00000, 0x04000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x08000, "subcpu", 0 )  /* HD64180RP8 code (link) */
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashp )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "euro main h.ic11",  0x00000, 0x20000, CRC(9dae00ca) SHA1(e80a069d1afbc624fa3e9cbe9c18bcd0364b3889) )
	ROM_LOAD16_BYTE( "euro main l.ic15",  0x00001, 0x20000, CRC(ba66b6a5) SHA1(26040c847209c2fd25805eefb99c280b12564a17) )
	ROM_LOAD16_BYTE( "euro data h.bin",   0x40000, 0x20000, CRC(bcce9d44) SHA1(e20a79e1e1c3367f92d05a2313cbeee122c1d3c5) )
	ROM_LOAD16_BYTE( "euro data l.bin",   0x40001, 0x20000, CRC(21f5b591) SHA1(6ff70f79bca705407ab9a4825466826bc2dbab32) )

	ROM_REGION( 0x08000, "subcpu", 0 )  /* HD64180RP8 code (link) */
	ROM_LOAD( "com.ic57",   0x00000, 0x08000, CRC(bae1a92f) SHA1(dbe10a02a294dfa7d6052a692c3a49aad85d6ffd) )

	// all other roms are under some kind of epoxy, assuming to be the same..
	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c21-08.38",   0x00000, 0x04000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashj )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c21_04-2.ic11",  0x00000, 0x20000, CRC(7a9c1828) SHA1(491eea29efc47159ad904e734a980c444bfbd8aa) )
	ROM_LOAD16_BYTE( "c21_06-2.ic15",  0x00001, 0x20000, CRC(c9d6440a) SHA1(2555af4c4043811a53e9f069d97571672237c18e) )
	ROM_LOAD16_BYTE( "c21_03-2.ic10",  0x40000, 0x20000, CRC(30afc320) SHA1(d4c1d1ef30be633244c6b71b24491d6eb3562cef) )
	ROM_LOAD16_BYTE( "c21_05-2.ic14",  0x40001, 0x20000, CRC(2bc93209) SHA1(3352659ea9364ca9462343f03e26dd10087d6834) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c21-08.38",   0x00000, 0x04000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x08000, "subcpu", ROMREGION_ERASE00 )  /* HD64180RP8 code (link) */
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashj1 )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c21_04-1.ic11",  0x00000, 0x20000, CRC(cc22ebe5) SHA1(170787e7ab2055af593f3f2596cab44feb53b060) )
	ROM_LOAD16_BYTE( "c21_06-1.ic15",  0x00001, 0x20000, CRC(26e03304) SHA1(c8b271e455dde312c8871dc8dd4d3f0f063fa894) )
	ROM_LOAD16_BYTE( "c21_03-1.ic10",  0x40000, 0x20000, CRC(c54888ed) SHA1(8a58da25eb8986a1c6496290e82344840badef0a) )
	ROM_LOAD16_BYTE( "c21_05-1.ic14",  0x40001, 0x20000, CRC(834018d2) SHA1(0b1a29316f90a98478b47d7fa3f05c68e5ddd9b3) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c21-08.38",   0x00000, 0x04000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x08000, "subcpu", ROMREGION_ERASE00 )  /* HD64180RP8 code (link) */ // the board this set was from did not have the link section populated
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashjo )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c21_04.ic11",  0x00000, 0x20000, CRC(be7d3f12) SHA1(16e445317d053a19fc430625743f4afa54ce1d8e) )
	ROM_LOAD16_BYTE( "c21_06.ic15",  0x00001, 0x20000, CRC(1db3fe02) SHA1(3abb341596eed8f991ed2002d2e7b71fa2dd099d) )
	ROM_LOAD16_BYTE( "c21_03.ic10",  0x40000, 0x20000, CRC(7e31c5a3) SHA1(a0abc5862d594800934a4792de4ec655f60c1f23) )
	ROM_LOAD16_BYTE( "c21_05.ic14",  0x40001, 0x20000, CRC(a4f4901d) SHA1(a3e8d9ad033e6fb1c8383669e6e59f2f79386e32) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c21-08.38",   0x00000, 0x04000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x08000, "subcpu", ROMREGION_ERASE00 )  /* HD64180RP8 code (link) */ // the board this set was from did not have the link section populated
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashu )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c21_14-2.ic11",  0x00000, 0x20000, CRC(f823d418) SHA1(5b4a0b42fb5a2e1ba1e25465762cdc24c41b33f8) )
	ROM_LOAD16_BYTE( "c21_16-2.ic15",  0x00001, 0x20000, CRC(90165577) SHA1(b8e163cf60933aaaa53873fbc866d8d1750240ab) )
	ROM_LOAD16_BYTE( "c21_13-2.ic10",  0x40000, 0x20000, CRC(92dcc3ae) SHA1(7d11c6d8b54468f0c56b4f58adc176e4d46a62eb) )
	ROM_LOAD16_BYTE( "c21_15-2.ic14",  0x40001, 0x20000, CRC(f915d26a) SHA1(cdc7e6a35077ebff937350aee1eee332352e9383) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c21-08.38",   0x00000, 0x04000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x08000, "subcpu", 0 )  /* HD64180RP8 code (link) */
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashi )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c21-14it",  0x00000, 0x20000, CRC(d1d9e613) SHA1(296c188daec962bdb4e78e20f1cc4c7d1f4dda09) ) /* need correct Taito ID #s for these roms */
	ROM_LOAD16_BYTE( "c21-16it",  0x00001, 0x20000, CRC(142256ef) SHA1(9ffc64d7c900bfa0300de9e6d18c7458f4c76ed7) ) /* ID numbers should be at least 26 or higher */
	ROM_LOAD16_BYTE( "c21-13it",  0x40000, 0x20000, CRC(c9cf6e30) SHA1(872c871cd60e0aa7149660277f67f90748d82743) )
	ROM_LOAD16_BYTE( "c21-17it",  0x40001, 0x20000, CRC(641fc9dd) SHA1(1497e39f6b250de39ef2785aaca7e68a803612fa) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c21-08.38",   0x00000, 0x04000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x08000, "subcpu", 0 )  /* HD64180RP8 code (link) */
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashf )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c21_19.ic11",  0x00000, 0x20000, CRC(4d70543b) SHA1(4fc8d4a9f978232a484af3d91bf8eea2afc839a7) )
	ROM_LOAD16_BYTE( "c21_21.ic15",  0x00001, 0x20000, CRC(0e5b9950) SHA1(872919bab057fc9e5baffe5dfe35b1b8c1ed0105) )
	ROM_LOAD16_BYTE( "c21_18.ic10",  0x40000, 0x20000, CRC(8a19e59b) SHA1(b42a0c8273ca6f202a5dc6e33965423da3b074d8) )
	ROM_LOAD16_BYTE( "c21_20.ic14",  0x40001, 0x20000, CRC(b96acfd9) SHA1(d05b55fd5bbf8fd0e5a7272d1951f27a4900371f) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c21-08.38",   0x00000, 0x04000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x08000, "subcpu", 0 )  /* HD64180RP8 code (link) */
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( cadashg )
	ROM_REGION( 0x80000, "maincpu", 0 )     /* 512k for 68000 code */
	ROM_LOAD16_BYTE( "c21_23-1.ic11",  0x00000, 0x20000, CRC(30ddbabe) SHA1(f48ea6fe36c4d9fe291232fd7adddb8f3547270f) )
	ROM_LOAD16_BYTE( "c21_25-1.ic15",  0x00001, 0x20000, CRC(24e10611) SHA1(6f406267777dd693a3869ccb34fe3f2f8dea857d) )
	ROM_LOAD16_BYTE( "c21_22-1.ic10",  0x40000, 0x20000, CRC(daf58b2d) SHA1(7a64df848f46f27bb6f9757ce0cc81311c2f172f) )
	ROM_LOAD16_BYTE( "c21_24-1.ic14",  0x40001, 0x20000, CRC(2359b93e) SHA1(9a5ce34dd8667a987ab8b6e6246f0ad032af868f) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c21-02.9",  0x00000, 0x80000, CRC(205883b9) SHA1(5aafee8cab3f949a7db91bcc26912f331041b51e) ) /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "c21-01.1",  0x00000, 0x80000, CRC(1ff6f39c) SHA1(742f296efc8073fafa73da2c8d7d26ca9514b6bf) ) /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "c21-08.38",   0x00000, 0x04000, CRC(dca495a0) SHA1(4e0f401f1b967da75f33fd7294860ad0b4bf2dce) )
	ROM_CONTINUE(            0x10000, 0x0c000 ) /* banked stuff */

	ROM_REGION( 0x08000, "subcpu", 0 )  /* HD64180RP8 code (link) */
	ROM_LOAD( "c21-07.57",   0x00000, 0x08000, CRC(f02292bd) SHA1(0a5c06a048ad67f90e0d766b504582e9eef035f7) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "pal16l8b-c21-09.ic34",   0x0000, 0x0104, CRC(4b296700) SHA1(79d6c8fb13e30795d9c1f49885ada658f9722b68) )
	ROM_LOAD( "pal16l8b-c21-10.ic45",   0x0200, 0x0104, CRC(35642f00) SHA1(a04403536b0ef7e8e7251dfc47274a6c8772fd2d) )
	ROM_LOAD( "pal16l8b-c21-11-1.ic46", 0x0400, 0x0104, CRC(f4791e24) SHA1(7e3bbffec7b8f9171e6e09706e5622fef3c99ca0) )
	ROM_LOAD( "pal20l8b-c21-12.ic47",   0x0600, 0x0144, CRC(bbc2cc97) SHA1(d4a68f28e0d3f5a3b39ecc25640bc9197ad0260b) )
ROM_END

ROM_START( galmedes )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "gm-prg1.bin",  0x00000, 0x20000, CRC(32a70753) SHA1(3bd094b7ae600dbc87ba74e8b2d6b86a68346f4f) )
	ROM_LOAD16_BYTE( "gm-prg0.bin",  0x00001, 0x20000, CRC(fae546a4) SHA1(484cad5287daa495b347f6b5b065f3b3d02d8f0e) )
	/* 0x40000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD( "gm-30.rom",    0x80000, 0x80000, CRC(4da2a407) SHA1(7bd0eb629dd7022a16e328612c786c544267f7bc) )   /* Fix ROM */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "gm-scn.bin", 0x00000, 0x80000, CRC(3bab0581) SHA1(56b79a4ffd9f4880a63450b7d1b79f029de75e20) )    /* SCR tiles (8 x 8) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "gm-obj.bin", 0x00000, 0x80000, CRC(7a4a1315) SHA1(e2010ee4222415fd55ba3102003be4151d29e39b) )    /* Sprites (16 x 16) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "gm-snd.bin", 0x00000, 0x04000, CRC(d6f56c21) SHA1(ff9743448ac8ce57a2f8c33a26145e7b92cbe3c3) )
	ROM_CONTINUE(           0x10000, 0x0c000 )  /* banked stuff */

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.bin", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.bin", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( earthjkr )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "ej_3b.rom",  0x00000, 0x20000, CRC(bdd86fc2) SHA1(96578860ed03718f8a68847b367eac6c81b79ca2) )
	ROM_LOAD16_BYTE( "ej_3a.rom",  0x00001, 0x20000, CRC(9c8050c6) SHA1(076c882f75787e8120de66ff0dcd2cb820513c45) )
	/* 0x40000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD( "ej_30e.rom", 0x80000, 0x80000, CRC(49d1f77f) SHA1(f6c9b2fc88b77cc9baa5be48da5c3eb72310e471) ) /* Fix ROM */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "ej_chr.rom", 0x00000, 0x80000, CRC(ac675297) SHA1(2a34e1eae3a4be84dbf709053f5e8a781b1073fc) )    /* SCR tiles (8 x 8) */

	ROM_REGION( 0xa0000, "gfx2", 0 )
	ROM_LOAD       ( "ej_obj.rom", 0x00000, 0x80000, CRC(5f21ac47) SHA1(45c94ffb53ee9b822b0676f6fb151fed4ce6d967) ) /* Sprites (16 x 16) */
	ROM_LOAD16_BYTE( "ej_1.rom",   0x80000, 0x10000, CRC(cb4891db) SHA1(af1112608cdd897ef6028ef617f5ca69d7964861) )
	ROM_LOAD16_BYTE( "ej_0.rom",   0x80001, 0x10000, CRC(b612086f) SHA1(625748fcb698ec57b7b3ce46019cf85de99aaaa1) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "ej_2.rom", 0x00000, 0x04000, CRC(42ba2566) SHA1(c437388684b565c7504d6bad6accd73aa000faca) )
	ROM_CONTINUE(         0x10000, 0x0c000 )    /* banked stuff */

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.bin", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.bin", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( earthjkrp ) // was production PCB complete with MASK rom, could just be an early revision, not proto
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "3.bin",  0x00001, 0x20000, CRC(26c33225) SHA1(b039c47d0776c90813ab52c867e95989cab2c567) )
	ROM_LOAD16_BYTE( "4.bin",  0x00000, 0x20000, CRC(e9b1ef0c) SHA1(5e104146d37922a8c7e93696c2c156223653025b) )
	/* 0x40000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD( "5.bin", 0x80000, 0x80000, CRC(bf760b2d) SHA1(4aff36623e5a31ab86c77461fa93e40e77f08edd) ) /* Fix ROM */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "ej_chr.rom", 0x00000, 0x80000, CRC(ac675297) SHA1(2a34e1eae3a4be84dbf709053f5e8a781b1073fc) )    /* SCR tiles (8 x 8) */

	ROM_REGION( 0xa0000, "gfx2", 0 )
	ROM_LOAD       ( "ej_obj.rom", 0x00000, 0x80000, CRC(5f21ac47) SHA1(45c94ffb53ee9b822b0676f6fb151fed4ce6d967) ) /* Sprites (16 x 16) */
	ROM_LOAD16_BYTE( "ej_1.rom",   0x80000, 0x10000, CRC(cb4891db) SHA1(af1112608cdd897ef6028ef617f5ca69d7964861) )
	ROM_LOAD16_BYTE( "ej_0.rom",   0x80001, 0x10000, CRC(b612086f) SHA1(625748fcb698ec57b7b3ce46019cf85de99aaaa1) )

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "ej_2.rom", 0x00000, 0x04000, CRC(42ba2566) SHA1(c437388684b565c7504d6bad6accd73aa000faca) )
	ROM_CONTINUE(         0x10000, 0x0c000 )    /* banked stuff */

	ROM_REGION( 0x144, "pals", 0 )
	ROM_LOAD( "b68-04.bin", 0x00000, 0x144, CRC(9be618d1) SHA1(61ee33c3db448a05ff8f455e77fe17d51106baec) )
	ROM_LOAD( "b68-05.bin", 0x00000, 0x104, CRC(d6524ccc) SHA1(f3b56253692aebb63278d47832fc27b8b212b59c) )
ROM_END

ROM_START( eto )
	ROM_REGION( 0x100000, "maincpu", 0 )     /* 1024k for 68000 code */
	ROM_LOAD16_BYTE( "eto-1.23",  0x00000, 0x20000, CRC(44286597) SHA1(ac37e5edbf9d187f60232adc5e9ebed45b3d2fe2) )
	ROM_LOAD16_BYTE( "eto-0.8",   0x00001, 0x20000, CRC(57b79370) SHA1(25f83eada982ef654260fe92016d42a90005a05c) )
	/* 0x40000 - 0x7ffff is intentionally empty */
	ROM_LOAD16_WORD( "eto-2.30",    0x80000, 0x80000, CRC(12f46fb5) SHA1(04db8b6ccd0051668bd2930275efa0265c0cfd2b) )    /* Fix ROM */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "eto-4.3", 0x00000, 0x80000, CRC(a8768939) SHA1(a2cbbd3e10ed48ba32a680b2e40ea03900cf33fa) )   /* Sprites (16 x 16) */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "eto-3.6", 0x00000, 0x80000, CRC(dd247397) SHA1(53a7bf877fd7e5f3daf295a698f4012447b6f113) )   /* SCR tiles (8 x 8) */

	ROM_REGION( 0x1c000, "audiocpu", 0 )    /* sound cpu */
	ROM_LOAD( "eto-5.27", 0x00000, 0x04000, CRC(b3689da0) SHA1(812d2e0a794403df9f0a5035784f14cd070ea080) )
	ROM_CONTINUE(         0x10000, 0x0c000 )    /* banked stuff */
ROM_END


GAME( 1988, bonzeadv,  0,        bonzeadv, bonzeadv, driver_device, 0, ROT0,   "Taito Corporation Japan",   "Bonze Adventure (World, Newer)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bonzeadvo, bonzeadv, bonzeadv, bonzeadv, driver_device, 0, ROT0,   "Taito Corporation Japan",   "Bonze Adventure (World, Older)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bonzeadvu, bonzeadv, bonzeadv, jigkmgri, driver_device, 0, ROT0,   "Taito America Corporation", "Bonze Adventure (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, jigkmgri,  bonzeadv, bonzeadv, jigkmgri, driver_device, 0, ROT0,   "Taito Corporation",         "Jigoku Meguri (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, bonzeadvp, bonzeadv, bonzeadv, jigkmgri, driver_device, 0, ROT0,   "Taito Corporation Japan",   "Bonze Adventure (World, prototype)", MACHINE_SUPPORTS_SAVE )

GAME( 1988, asuka,     0,        asuka,    asuka, driver_device,    0, ROT270, "Taito Corporation",         "Asuka & Asuka (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, asukaj,    asuka,    asuka,    asuka, driver_device,    0, ROT270, "Taito Corporation",         "Asuka & Asuka (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, mofflott,  0,        mofflott, mofflott, driver_device, 0, ROT270, "Taito Corporation",         "Maze of Flott (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1989, cadash,    0,        cadash,   cadash, driver_device,   0, ROT0,   "Taito Corporation Japan",   "Cadash (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cadashj,   cadash,   cadash,   cadashj, driver_device,  0, ROT0,   "Taito Corporation",         "Cadash (Japan, version 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cadashj1,  cadash,   cadash,   cadashj, driver_device,  0, ROT0,   "Taito Corporation",         "Cadash (Japan, version 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cadashjo,  cadash,   cadash,   cadashj, driver_device,  0, ROT0,   "Taito Corporation",         "Cadash (Japan, oldest version)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cadashu,   cadash,   cadash,   cadashu, driver_device,  0, ROT0,   "Taito America Corporation", "Cadash (US, version 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cadashi,   cadash,   cadash,   cadash, driver_device,   0, ROT0,   "Taito Corporation Japan",   "Cadash (Italy)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cadashf,   cadash,   cadash,   cadash, driver_device,   0, ROT0,   "Taito Corporation Japan",   "Cadash (France)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cadashg,   cadash,   cadash,   cadash, driver_device,   0, ROT0,   "Taito Corporation Japan",   "Cadash (Germany, version 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, cadashp,   cadash,   cadash,   cadashj, driver_device,  0, ROT0,   "Taito Corporation Japan",   "Cadash (World, prototype)", MACHINE_SUPPORTS_SAVE )

GAME( 1992, galmedes,  0,        galmedes, galmedes, driver_device, 0, ROT270, "Visco",                     "Galmedes (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, earthjkr,  0,        galmedes, earthjkr, driver_device, 0, ROT270, "Visco",                     "U.N. Defense Force: Earth Joker (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, earthjkrp, earthjkr, galmedes, earthjkr, driver_device, 0, ROT270, "Visco",                     "U.N. Defense Force: Earth Joker (Japan, prototype?)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, eto,       0,        eto,      eto, driver_device,      0, ROT0,   "Visco",                     "Kokontouzai Eto Monogatari (Japan)", MACHINE_SUPPORTS_SAVE )
