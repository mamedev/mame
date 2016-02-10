// license:BSD-3-Clause
// copyright-holders:Couriersud
/***************************************************************************

  Nintendo Donkey Kong hardware

****************************************************************************

TODO:

- write a shootgal palette_init
- Pestplce colors and origin
- Shooting Gallery: Shootgal.txt mentions speech pcb,
  mikesarcade.com dk conversion - Clarify
- 8ballact: read 1507 no mapped
- implement 74LS259 (8bit addressable latches),
  74LS175 (QUAD D FlipFlop), 74LS373 (Octal transparent latch)

Done:
- dkong3 dma (Z80 dma)
- dkongjr discrete interface
- when i am retired: implement 8257 DMA controller
- drakton - add dkongjr conversion
- for documentation: hook up speech interface still present on radarscp (TRS02)
  and early dkong (TKG02/TKG03) pcbs.
- radarscp_grid_color_w() is wrong, it probably isn't supposed to change
  the grid color. There are reports of the grid being constantly blue in
  the real game, the flyer confirms this.
  Couriersud: On the TRS01/TRS02 hardware the grid color is changed by
              writing to radarscp_grid_color_w. The available flyers
              show a green and a white grid.


  Couriersud: 05-06/2007
    - Implemented discrete sound from schematics: dkong, radarscp
    - Implemented discrete background from schematics for radarscp and various effects
    - Changed inputs for radarscp
    - Color generation from schematics (resistor mixer) for radarscp, dkong, dkongjr and dkong3
    - Added sprite ram bank switching from schematics
    - combined memory maps
    - first attempts on decoding m58815

  Couriersud: 11/2007

    - Added configuration switch to change palette between TKG02 (radarscp conversion) and TKG04 (dkong 2board)
    - Added speech support (samples) to radarscp1
    - Hooked up and written 8257 dma controller
        All dkong and dkongjr based games now use the 8257
        All epos and 2650 based games now use the 8257
    - Fixed 2650 games to use dkong audio as well - only cpu replaced by addon board ...
    - straightened memory maps
    - Fixed bug in dkong3b memory map
    - Externalized sound drivers to MACHINE_START in audio/dkong.c
    - DAC Filter now uses lowpass sallen key filter
    - Reorganized INPUT_PORTS
    - Fixed sound for drakton and strtheat
    - Added Dip-Switch locations to 8ballact
    - Fixed 8ballact according to conversion manual
    - Reorganized code
    - http://members.cox.net/rcolbert/mame_cocktail_project.htm mentions, that radarscp to dkong conversions
      have a sticker TKG-2 over the TRS-02 label.
    - PCB Info (Dkong.txt) mentions, that TKG-3 is a 4 board set. This seems to be based on the radarscp 4-board set trs-02.
      Schematics show, that these boards actually were based on TRS-02
    - TKG-4 is a 2 board set.

  Couriersud: 12/2007

    - Cosmetic changes
    - Fixed regression in radarscp sound
    - Added dipswitch locations to dkong3b
    - Added dkongjr discrete sound
    - Proper interface Z80 - I8035 for dkongjr
    - Changed discrete sound output factors
    - changed dkong/radarscp based games to use hardware-conformant I8035 memory maps
    - Added drakton clone drktnjr on dkongjr hardware
    - wrote M58817 sound driver and hooked it up
         Uses tms5110 speech synthesis. LPC format is identical, however coefficients
         seem to be different. Until coefficients are known, samples are used.
    - moved address remapping proms to "user1" (hunchbkd & co)
    - Service now adds credit
    - Hooked up coin_counters
    - remove MACHINE_NOT_WORKING MACHINE_WRONG_COLORS from hunchbkd
    - fixed shootgal0121u4red
    - dkongjr: mapped more interface lines between sound board and cpu board
    - tagged all inputs, all reads use tag names
    - moved more static vars into dkong_state

  General notes

  The dma8257 is responsible for copying sprite data from 0x6900 (or where ever it is located) to 0x7400 (sprite banks).
  2650 based games had a rom for address lookups. This is needed for proper operation of the 8257 code.
  Where this was missing, I copied the entry from hunchbkd and marked it as BAD_DUMP.
  Additional protection was done by pals. The only driver with a PAL entry is herbiedk.
  I added NO_DUMP entries for the other 2650 games.

  Thanks to Mike I was able to closer analyze TKG02/03/04 schematics. The radar scope to donkey kong conversions done
  should lead to the same discrete sound as TKG04 (2 board boards). However the colours are different.
  Red girders are back. Until the release of TKG04, the 2 board release, schematics were largely identical with radar scope.
  There were slight modifications from release to release in the analog video logic. Already at TKG2-02(03) the
  darlington amplifier for the blue channel was replaced with a emitter one by leaving away (OMIT in schematics)
  a resistor and transistor. Later on, TKG3-06, 4066 switches and background generation circuits for radarscope were removed.
  I have added a driver configuration switch to switch between TKG4 and TKG2-01 (radarscope) palettes.
  At a later stage, a TKG2-02 will be added as well.

  Mike Haaland provided the following information on startup sounds:

  For both of these boards [dkong, dkongjr], only the jump sound
  is distinguishable. I don't hear the pound of walk sound.

****************************************************************************

Donkey Kong and Donkey Kong Jr. memory map (preliminary) (DKong 3 follows)

0000-3fff ROM (Donkey Kong Jr.and Donkey Kong 3: 0000-5fff)
6000-6fff RAM
7000-73ff sprites
7400-77ff Video RAM
8000-9fff ROM (DK3 only)



memory mapped ports:

read:
7c00      IN0
7c80      IN1
7d00      IN2 (DK3: DSW2)
7d80      DSW1

*
 * IN0 (bits NOT inverted)
 * bit 7 : ?
 * bit 6 : reset (when player 1 active)
 * bit 5 : ?
 * bit 4 : JUMP player 1
 * bit 3 : DOWN player 1
 * bit 2 : UP player 1
 * bit 1 : LEFT player 1
 * bit 0 : RIGHT player 1
 *
*
 * IN1 (bits NOT inverted)
 * bit 7 : ?
 * bit 6 : reset (when player 2 active)
 * bit 5 : ?
 * bit 4 : JUMP player 2
 * bit 3 : DOWN player 2
 * bit 2 : UP player 2
 * bit 1 : LEFT player 2
 * bit 0 : RIGHT player 2
 *
*
 * IN2 (bits NOT inverted)
 * bit 7 : COIN (IS inverted in Radarscope)
 * bit 6 : ? Radarscope does some wizardry with this bit
 * bit 5 : ?
 * bit 4 : ?
 * bit 3 : START 2
 * bit 2 : START 1
 * bit 1 : ?
 * bit 0 : ? if this is 1, the code jumps to $4000, outside the rom space
 *
*
 * DSW1 (bits NOT inverted)
 * bit 7 : COCKTAIL or UPRIGHT cabinet (1 = UPRIGHT)
 * bit 6 : \ 000 = 1 coin 1 play   001 = 2 coins 1 play  010 = 1 coin 2 plays
 * bit 5 : | 011 = 3 coins 1 play  100 = 1 coin 3 plays  101 = 4 coins 1 play
 * bit 4 : / 110 = 1 coin 4 plays  111 = 5 coins 1 play
 * bit 3 : \bonus at
 * bit 2 : / 00 = 7000  01 = 10000  10 = 15000  11 = 20000
 * bit 1 : \ 00 = 3 lives  01 = 4 lives
 * bit 0 : / 10 = 5 lives  11 = 6 lives
 *

write:
7800-780F P8257 Control registers
7c00      Background sound/music select:
          00 - nothing
          01 - Intro tune
          02 - How High? (intermisson) tune
          03 - Out of time
          04 - Hammer
          05 - Rivet level 2 completed (end tune)
          06 - Hammer hit
          07 - Standard level end
          08 - Background 1 (barrels)
          09 - Background 4 (pie factory)
          0A - Background 3 (springs)
          0B - Background 2 (rivets)
          0C - Rivet level 1 completed (end tune)
          0D - Rivet removed
          0E - Rivet level completed
          0F - Gorilla roar
7c80      gfx bank select (Donkey Kong Jr. only)
7d00      digital sound trigger - walk
7d01      digital sound trigger - jump
7d02      digital sound trigger - boom (gorilla stomps foot)
7d03      digital sound trigger - coin input/spring
7d04      digital sound trigger - gorilla fall
7d05      digital sound trigger - barrel jump/prize
7d06      ?
7d07      ?
7d80      digital sound trigger - dead
7d82      flip screen
7d83      ?
7d84      interrupt enable
7d85      0/1 toggle
7d86-7d87 palette bank selector (only bit 0 is significant: 7d86 = bit 0 7d87 = bit 1)


8035 Memory Map:

0000-07ff ROM
0800-0fff Compressed sound sample (Gorilla roar in DKong)

Read ports:
0x20   Read current tune
P2.5   Active low when jumping
T0     Select sound for jump (Normal or Barrell?)
T1     Active low when gorilla is falling

Write ports:
P1     Digital out
P2.7   External decay
P2.6   Select second ROM reading (MOVX instruction will access area 800-fff)
P2.2-0 Select the bank of 256 bytes for second ROM



Donkey Kong 3 memory map (preliminary):

RAM and read ports same as above;

write:
7d00      ?
7d80      ?
7e00      ?
7e80
7e81      char bank selector
7e82      flipscreen
7e83      ?
7e84      interrupt enable
7e85      ?
7e86-7e87 palette bank selector (only bit 0 is significant: 7e86 = bit 0 7e87 = bit 1)


I/O ports

write:
00        ?

Changes:
    Apr 7 98 Howie Cohen
    * Added samples for the climb, jump, land and walking sounds

    Jul 27 99 Chad Hendrickson
    * Added cocktail mode flipscreen

Donkey Kong Junior Notes
========================

    DJR-03  Donkey Kong Modification Kits                 02-23-83
    --------------------------------------------------------------
    Nintendo Service Department Bulletin # DJR-03         02-23-83
    GAME: Donkey Kong Junior
    SUBJECT: Play Time

    It has come to our attention that play time on Donkey Kong
    Junior is, in some cases, excessive, while in other cases
    there is concern over the ability of players to learn the
    game and thereby retain an interest in playing.

    We are making available, through our Nintendo Distributors,
    two EPROM change kits, designated DJR1-E and DJR1-P.

    The E kit consists of four (4) chips and is an easier version
    of the program.  it presents the board sequence in a way which
    familiarizes the players with the game faster.  The board
    order is vines, springboard, Mario's hideout and keys.

    The P kit is a more difficult version consisting of two chips.
    The order remains the same while more and faster
    obstacles (snapjaws, nitpickers, etc.) are presented.
    --------------------------------------------------------------

    E-Kit - roms
    ------------------------------------------------
    Filename    Label          Type   Loc/PCB  CSum
    ----------- -------------- ------ -------  ---- --
    2732.5Ae    DJR1-C-5A e    2732   5A(CPU)  68E9
    2764.5Be    DJR1-C-5B e    2764   5B(CPU)  B0CF
    2764.5Ce    DJR1-C-5C e    2764   5C(CPU)  FC64
    2764.5Ee    DJR1-C-5E e    2764   5E(CPU)  7CC6
    ------------------------------------------------

    P-Kit roms (Still looking for these, suffix is
    more than likely P. :)
    ------------------------------------------------
    Filename    Label          Type   Loc/PCB  CSum
    ----------- -------------- ------ -------  ---- --
    empty       ---------      ----   5A(CPU)  ----
    2764.5Bp    DJR1-C-5B p    2764   5B(CPU)  1B58
    2764.5Cp    DJR1-C-5C p    2764   5C(CPU)  F4FE
    ------------------------------------------------


Donkey Kong Notes
=================

    Nintendo Service Department Bulletin # TKG-02 12-11-81
    GAME: Donkey Kong
    SUBJECT: Speed-up Kit #1

    TO prevent extremely long play times, we are making available
    a speed-up kit. This kit prevents players from waiting on top
    of ladders, on screen #1, while the barrels roll across
    instead of coming donw on top of him. Whit this kit, barrels
    will roll on top of the player on a ladder 70 to 80 percent of
    the time.

    Machines above serial #30,000 will have this kit installed at
    the factory, and all TKG4 board sets will come with this kit.

    To install this kit in a four-board set, follow these
    instructions:

    1) Remove P.C. Boards from game, leaving them
    attached to P.C. Board bracket.

    2) Using a pair of needle-nose pliers, separate the
    sound P.C. Board fromt he CPU P.C. Board, exposing
    the entire surface of the CPU Board.

    3) Remove the EPROMS's at location 5F, 5A, 5H and 5K
    from the CPU Board. (Note 5A should read 5G - MSH)

    4) Install the speed-up kit EPROM's in the corres-
    ponding locations.

    5) Reconnect the sound P.C. Board to the CPU P.C.
    Board and mount the P.C. Boards in the game.
    --------------------------------------------------------------

    That kit included the following 4 EPROM's

    USA (c) 1981 Nintendo of America set
    ------------------------------------

    Filename Label Type Loc/PCB *Label *Loc/PCB CSum
    --------- ---------- ------ ------- ---------- -------- ----
    2532.5K TKG4-C-5At 2532 5A(CPU) TKG3-C-5K 5K(CPU) A0F0
    2532.5H TKG4-C-5Bt 2532 5B(CPU) TKG3-C-5H 5H(CPU) B2BC
    2532.5G TKG4-C-5Ct 2532 5C(CPU) TKG3-C-5G 5G(CPU) 73BA
    2532.5F TKG4-C-5Et 2532 5E(CPU) TKG3-C-5F 5F(CPU) AA97

    Not only did the eprom fix the ladder bug, but it also changed
    the copyright screen to read "(C)1981 Nintendo of America".


    D2K Jumpman returns Notes
    =========================

    This is a DKong/Hack combo using a Braze Technologies High Score Save pcb.
    This pcb will be placed in the cpu socket and the Z80 together with an
    additional 64K rom, a 74LS245, an eeprom and a pal/gal. It looks like the
    "encryption" was a coincidence resulting from an easy pcb layout.
    The pal is also used to switch A15 on and off. This is done in locations
    6800 and E800.


    Donkey Kong "Hard" Kit
    ======================

    A yet "unconfirmed original" rom replacement kit which is a replacement for
    TKG-03 and -04 boards and greatly increases the speed and amount of fireballs
    showing on all levels.  Such behavior can be seen easily on the Rivet Board,
    where most of the fireballs have appeared even before removing the first rivet.

    Hopefully confirmation and information will come along later which confirms
    this is a legitimate Nintendo Kit.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/s2650/s2650.h"
#include "cpu/m6502/m6502.h"
#include "includes/dkong.h"
#include "machine/eepromser.h"

/*************************************
 *
 *  Defines
 *
 *************************************/

#define DEBUG_PROTECTION    (0)
#define DEBUG_DISC_SOUND    (0)

#define COMBINE_TYPE_PC(_tyn, _pc) ((_tyn)<<16 | (_pc))


/*************************************
 *
 *  statics
 *
 *************************************/

READ8_MEMBER(dkong_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(dkong_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(offset, data);
}

/*************************************
 *
 *  VBLANK and IRQ generation
 *
 *************************************/

INTERRUPT_GEN_MEMBER(dkong_state::s2650_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x03);
}

/*************************************
 *
 *  Machine setup
 *
 *************************************/

MACHINE_START_MEMBER(dkong_state,dkong2b)
{
	m_hardware_type = HARDWARE_TKG04;

	save_item(NAME(m_decrypt_counter));
	save_item(NAME(m_dma_latch));
}

MACHINE_START_MEMBER(dkong_state,s2650)
{
	UINT8   *p = memregion("user1")->base();
	const char *game_name = machine().system().name;
	int i;

	MACHINE_START_CALL_MEMBER(dkong2b);

	for (i = 0; i < 0x200; i++)
		m_rev_map[i] = -1;
	for (i = 0; i < 0x200; i++)
		m_rev_map[p[0x0000 + i]] = i;

	m_hunchloopback = 0;

	save_item(NAME(m_hunchloopback));
	save_item(NAME(m_prot_cnt));
	save_item(NAME(m_main_fo));

	if (strcmp(game_name,"herbiedk") == 0) m_protect_type = DK2650_HERBIEDK;
	else if (strcmp(game_name,"hunchbkd") == 0) m_protect_type = DK2650_HUNCHBKD;
	else if (strcmp(game_name,"sbdk") == 0) m_protect_type = DK2650_HUNCHBKD;
	else if (strcmp(game_name,"herodk") == 0) m_protect_type = DK2650_HUNCHBKD;
	else if (strcmp(game_name,"herodku") == 0) m_protect_type = DK2650_HUNCHBKD;
	else if (strcmp(game_name,"8ballact") == 0) m_protect_type = DK2650_EIGHTACT;
	else if (strcmp(game_name,"8ballact2") == 0) m_protect_type = DK2650_EIGHTACT;
	else if (strcmp(game_name,"shootgal") == 0) m_protect_type = DK2650_SHOOTGAL;
	else if (strcmp(game_name,"spclforc") == 0) m_protect_type = DK2650_SPCLFORC;
	else if (strcmp(game_name,"spcfrcii") == 0) m_protect_type = DK2650_SPCLFORC;
	else
		fatalerror("Unknown game <%s> in S2650 start.\n", game_name);
}

MACHINE_START_MEMBER(dkong_state,radarscp)
{
	MACHINE_START_CALL_MEMBER(dkong2b);
	m_hardware_type = HARDWARE_TRS02;
	m_vidhw = DKONG_BOARD;
}

MACHINE_START_MEMBER(dkong_state,radarscp1)
{
	MACHINE_START_CALL_MEMBER(dkong2b);
	m_hardware_type = HARDWARE_TRS01;
	m_vidhw = DKONG_BOARD;
}

MACHINE_START_MEMBER(dkong_state,dkong3)
{
	m_hardware_type = HARDWARE_TKG04;
}

MACHINE_RESET_MEMBER(dkong_state,dkong)
{
	/* nothing */
}

MACHINE_RESET_MEMBER(dkong_state,strtheat)
{
	UINT8 *ROM = memregion("maincpu")->base();

	MACHINE_RESET_CALL_MEMBER(dkong);

	/* The initial state of the counter is 0x08 */
	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);
	m_decrypt_counter = 0x08;
	membank("bank1")->set_entry(0);
}

MACHINE_RESET_MEMBER(dkong_state,drakton)
{
	UINT8 *ROM = memregion("maincpu")->base();

	MACHINE_RESET_CALL_MEMBER(dkong);

	/* The initial state of the counter is 0x09 */
	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);
	m_decrypt_counter = 0x09;
	membank("bank1")->set_entry(1);
}


/*************************************
 *
 *  DMA handling
 *
 *************************************/

READ8_MEMBER(dkong_state::hb_dma_read_byte)
{
	int   bucket = m_rev_map[(offset>>10) & 0x1ff];
	int   addr;

	if (bucket < 0)
		fatalerror("hb_dma_read_byte - unmapped access for 0x%02x - bucket 0x%02x\n", offset, bucket);

	addr = ((bucket << 7) & 0x7c00) | (offset & 0x3ff);
	address_space &prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(addr);
}

WRITE8_MEMBER(dkong_state::hb_dma_write_byte)
{
	int   bucket = m_rev_map[(offset>>10) & 0x1ff];
	int   addr;

	if (bucket < 0)
		fatalerror("hb_dma_read_byte - unmapped access for 0x%02x - bucket 0x%02x\n", offset, bucket);

	addr = ((bucket << 7) & 0x7c00) | (offset & 0x3ff);
	address_space &prog_space = m_maincpu->space(AS_PROGRAM);
	prog_space.write_byte(addr, data);
}

READ8_MEMBER(dkong_state::p8257_ctl_r)
{
	return m_dma_latch;
}

WRITE8_MEMBER(dkong_state::p8257_ctl_w)
{
	m_dma_latch = data;
}


/*************************************
 *
 *  Output ports
 *
 *************************************/

WRITE8_MEMBER(dkong_state::dkong3_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(offset, data & 0x01);
}

WRITE8_MEMBER(dkong_state::p8257_drq_w)
{
	m_dma8257->dreq0_w(data & 0x01);
	m_dma8257->dreq1_w(data & 0x01);
	machine().scheduler().abort_timeslice(); // transfer occurs immediately
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100)); // smooth things out a bit
}

READ8_MEMBER(dkong_state::dkong_in2_r)
{
	/* mcu status (sound feedback) is inverted bit4 from port B (8039) */
	UINT8 mcustatus = m_dev_vp2->bit4_q_r(space, 0);
	UINT8 r;

	r = (ioport("IN2")->read() & 0xBF) | (mcustatus << 6);
	machine().bookkeeping().coin_counter_w(offset, r >> 7);
	if (r & 0x10)
		r = (r & ~0x10) | 0x80; /* service ==> coin */
	return r;
}

READ8_MEMBER(dkong_state::dkongjr_in2_r)
{
	/* dkongjr does not have the mcu line connected */

	UINT8 r;

	r = (ioport("IN2")->read() & 0xBF) | 0x40;
	machine().bookkeeping().coin_counter_w(offset, r >> 7);
	if (r & 0x10)
		r = (r & ~0x10) | 0x80; /* service ==> coin */
	return r;
}

READ8_MEMBER(dkong_state::s2650_mirror_r)
{
	return space.read_byte(0x1000 + offset);
}


WRITE8_MEMBER(dkong_state::s2650_mirror_w)
{
	space.write_byte(0x1000 + offset, data);
}


READ8_MEMBER(dkong_state::epos_decrypt_rom)
{
	if (offset & 0x01)
	{
		m_decrypt_counter = m_decrypt_counter - 1;
		if (m_decrypt_counter < 0)
			m_decrypt_counter = 0x0F;
	}
	else
	{
		m_decrypt_counter = (m_decrypt_counter + 1) & 0x0F;
	}

	switch(m_decrypt_counter)
	{
		case 0x08:  membank("bank1")->set_entry(0);      break;
		case 0x09:  membank("bank1")->set_entry(1);      break;
		case 0x0A:  membank("bank1")->set_entry(2);      break;
		case 0x0B:  membank("bank1")->set_entry(3);      break;
		default:
			logerror("Invalid counter = %02X\n",m_decrypt_counter);
			break;
	}

	return 0;
}


WRITE8_MEMBER(dkong_state::s2650_data_w)
{
#if DEBUG_PROTECTION
	logerror("write : pc = %04x, loopback = %02x\n",space.device().safe_pc(), data);
#endif

	m_hunchloopback = data;
}

WRITE_LINE_MEMBER(dkong_state::s2650_fo_w)
{
#if DEBUG_PROTECTION
	logerror("write : pc = %04x, FO = %02x\n",space.device().safe_pc(), data);
#endif

	m_main_fo = state;

	if (m_main_fo)
		m_hunchloopback = 0xfb;
}

READ8_MEMBER(dkong_state::s2650_port0_r)
{
#if DEBUG_PROTECTION
	logerror("port 0 : pc = %04x, loopback = %02x fo=%d\n",space.device().safe_pc(), m_hunchloopback, m_main_fo);
#endif

	switch (m_protect_type)
	{
		case DK2650_SHOOTGAL:
		case DK2650_HUNCHBKD:
			if (m_main_fo)
				return m_hunchloopback;
			else
				return m_hunchloopback--;
		case DK2650_SPCLFORC:
			if (!m_main_fo)
				return m_hunchloopback;
			else
				return m_hunchloopback--;
	}
	fatalerror("Unhandled read from port 0 : pc = %4x\n",space.device().safe_pc());
}


READ8_MEMBER(dkong_state::s2650_port1_r)
{
#if DEBUG_PROTECTION
	logerror("port 1 : pc = %04x, loopback = %02x fo=%d\n",space.device().safe_pc(), m_hunchloopback, m_main_fo);
#endif

	switch (m_protect_type)
	{
		case DK2650_HUNCHBKD:
			return m_hunchloopback--;
		case DK2650_EIGHTACT:
		case DK2650_HERBIEDK:
			if (m_hunchloopback & 0x80)
				return m_prot_cnt;
			else
				return ++m_prot_cnt;
	}
	fatalerror("Unhandled read from port 1 : pc = %4x\n",space.device().safe_pc());
}


WRITE8_MEMBER(dkong_state::dkong3_2a03_reset_w)
{
	if (data & 1)
	{
		m_dev_n2a03a->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
		m_dev_n2a03b->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
	else
	{
		m_dev_n2a03a->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
		m_dev_n2a03b->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}
}

READ8_MEMBER(dkong_state::strtheat_inputport_0_r)
{
	if(ioport("DSW0")->read() & 0x40)
	{
		/* Joystick inputs */
		return ioport("IN0")->read();
	}
	else
	{
		/* Steering Wheel inputs */
		return (ioport("IN0")->read() & ~3) | (ioport("IN4")->read() & 3);
	}
}


READ8_MEMBER(dkong_state::strtheat_inputport_1_r)
{
	if(ioport("DSW0")->read() & 0x40)
	{
		/* Joystick inputs */
		return ioport("IN1")->read();
	}
	else
	{
		/* Steering Wheel inputs */
		return (ioport("IN1")->read() & ~3) | (ioport("IN5")->read() & 3);
	}
}

WRITE8_MEMBER(dkong_state::dkong_z80dma_rdy_w)
{
	m_z80dma->rdy_w(data & 0x01);
}

WRITE8_MEMBER(dkong_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( dkong_map, AS_PROGRAM, 8, dkong_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x6000, 0x6bff) AM_RAM
	AM_RANGE(0x7000, 0x73ff) AM_RAM AM_SHARE("sprite_ram") /* sprite set 1 */
	AM_RANGE(0x7400, 0x77ff) AM_RAM_WRITE(dkong_videoram_w) AM_SHARE("video_ram")
	AM_RANGE(0x7800, 0x780f) AM_DEVREADWRITE("dma8257", i8257_device, read, write)   /* P8257 control registers */
	AM_RANGE(0x7c00, 0x7c00) AM_READ_PORT("IN0") AM_LATCH8_WRITE("ls175.3d")    /* IN0, sound CPU intf */
	AM_RANGE(0x7c80, 0x7c80) AM_READ_PORT("IN1") AM_WRITE(radarscp_grid_color_w)/* IN1 */

	AM_RANGE(0x7d00, 0x7d00) AM_READ(dkong_in2_r)                               /* IN2 */
	AM_RANGE(0x7d00, 0x7d07) AM_DEVWRITE("ls259.6h", latch8_device, bit0_w)          /* Sound signals */

	AM_RANGE(0x7d80, 0x7d80) AM_READ_PORT("DSW0") AM_WRITE(dkong_audio_irq_w)   /* DSW0 */
	AM_RANGE(0x7d81, 0x7d81) AM_WRITE(radarscp_grid_enable_w)
	AM_RANGE(0x7d82, 0x7d82) AM_WRITE(dkong_flipscreen_w)
	AM_RANGE(0x7d83, 0x7d83) AM_WRITE(dkong_spritebank_w)                       /* 2 PSL Signal */
	AM_RANGE(0x7d84, 0x7d84) AM_WRITE(nmi_mask_w)
	AM_RANGE(0x7d85, 0x7d85) AM_WRITE(p8257_drq_w)          /* P8257 ==> /DRQ0 /DRQ1 */
	AM_RANGE(0x7d86, 0x7d87) AM_WRITE(dkong_palettebank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkongjr_map, AS_PROGRAM, 8, dkong_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x6bff) AM_RAM
	AM_RANGE(0x6c00, 0x6fff) AM_RAM                                              /* DK3 bootleg only */
	AM_RANGE(0x7000, 0x73ff) AM_RAM AM_SHARE("sprite_ram") /* sprite set 1 */
	AM_RANGE(0x7400, 0x77ff) AM_RAM_WRITE(dkong_videoram_w) AM_SHARE("video_ram")
	AM_RANGE(0x7800, 0x780f) AM_DEVREADWRITE("dma8257", i8257_device, read, write)   /* P8257 control registers */

	AM_RANGE(0x7c00, 0x7c00) AM_READ_PORT("IN0") AM_LATCH8_WRITE("ls174.3d")    /* IN0, sound interface */

	AM_RANGE(0x7c80, 0x7c80) AM_READ_PORT("IN1") AM_WRITE(dkongjr_gfxbank_w)
	AM_RANGE(0x7c80, 0x7c87) AM_DEVWRITE("ls259.4h", latch8_device, bit0_w)     /* latch for sound and signals above */

	AM_RANGE(0x7d00, 0x7d00) AM_READ(dkongjr_in2_r)                             /* IN2 */
	AM_RANGE(0x7d00, 0x7d07) AM_DEVWRITE("ls259.6h", latch8_device, bit0_w)      /* Sound addrs */

	AM_RANGE(0x7d80, 0x7d80) AM_READ_PORT("DSW0") AM_WRITE(dkong_audio_irq_w)   /* DSW0 */
	AM_RANGE(0x7d82, 0x7d82) AM_WRITE(dkong_flipscreen_w)
	AM_RANGE(0x7d83, 0x7d83) AM_WRITE(dkong_spritebank_w)                       /* 2 PSL Signal */
	AM_RANGE(0x7d84, 0x7d84) AM_WRITE(nmi_mask_w)
	AM_RANGE(0x7d85, 0x7d85) AM_WRITE(p8257_drq_w)        /* P8257 ==> /DRQ0 /DRQ1 */
	AM_RANGE(0x7d86, 0x7d87) AM_WRITE(dkong_palettebank_w)
	AM_RANGE(0x7d80, 0x7d87) AM_DEVWRITE("ls259.5h", latch8_device, bit0_w)     /* latch for sound and signals above*/

	AM_RANGE(0x8000, 0x9fff) AM_ROM                                             /* bootleg DKjr only */
	AM_RANGE(0xb000, 0xbfff) AM_ROM                                             /* pestplce only */
	AM_RANGE(0xd000, 0xdfff) AM_ROM                                             /* DK3 bootleg only */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkong3_map, AS_PROGRAM, 8, dkong_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0x6800, 0x6fff) AM_RAM
	AM_RANGE(0x7000, 0x73ff) AM_RAM AM_SHARE("sprite_ram") /* sprite set 1 */
	AM_RANGE(0x7400, 0x77ff) AM_RAM_WRITE(dkong_videoram_w) AM_SHARE("video_ram")
	AM_RANGE(0x7c00, 0x7c00) AM_READ_PORT("IN0")  AM_LATCH8_WRITE("latch1")
	AM_RANGE(0x7c80, 0x7c80) AM_READ_PORT("IN1")  AM_LATCH8_WRITE("latch2")
	AM_RANGE(0x7d00, 0x7d00) AM_READ_PORT("DSW0") AM_LATCH8_WRITE("latch3")
	AM_RANGE(0x7d80, 0x7d80) AM_READ_PORT("DSW1") AM_WRITE(dkong3_2a03_reset_w)
	AM_RANGE(0x7e80, 0x7e80) AM_WRITE(dkong3_coin_counter_w)
	AM_RANGE(0x7e81, 0x7e81) AM_WRITE(dkong3_gfxbank_w)
	AM_RANGE(0x7e82, 0x7e82) AM_WRITE(dkong_flipscreen_w)
	AM_RANGE(0x7e83, 0x7e83) AM_WRITE(dkong_spritebank_w)                 /* 2 PSL Signal */
	AM_RANGE(0x7e84, 0x7e84) AM_WRITE(nmi_mask_w)
	AM_RANGE(0x7e85, 0x7e85) AM_WRITE(dkong_z80dma_rdy_w)  /* ==> DMA Chip */
	AM_RANGE(0x7e86, 0x7e87) AM_WRITE(dkong_palettebank_w)
	AM_RANGE(0x8000, 0x9fff) AM_ROM                                       /* DK3 and bootleg DKjr only */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dkong3_io_map, AS_IO, 8, dkong_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("z80dma", z80dma_device, read, write)  /* dma controller */
ADDRESS_MAP_END

/* Epos conversions */

static ADDRESS_MAP_START( epos_readport, AS_IO, 8, dkong_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xff) AM_READ(epos_decrypt_rom)  /* Switch protection logic */
ADDRESS_MAP_END

/* S2650 conversions */

static ADDRESS_MAP_START( s2650_map, AS_PROGRAM, 8, dkong_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("sprite_ram")  /* 0x7000 */
	AM_RANGE(0x1400, 0x1400) AM_MIRROR(0x007f) AM_READ_PORT("IN0") AM_DEVWRITE("ls175.3d", latch8_device, write)
	AM_RANGE(0x1480, 0x1480) AM_READ_PORT("IN1")
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0x007f) AM_READ(dkong_in2_r)                                 /* IN2 */
	AM_RANGE(0x1500, 0x1507) AM_DEVWRITE("ls259.6h", latch8_device, bit0_w)       /* Sound signals */
	AM_RANGE(0x1580, 0x1580) AM_READ_PORT("DSW0") AM_WRITE(dkong_audio_irq_w)     /* DSW0 */
	AM_RANGE(0x1582, 0x1582) AM_WRITE(dkong_flipscreen_w)
	AM_RANGE(0x1583, 0x1583) AM_WRITE(dkong_spritebank_w)                         /* 2 PSL Signal */
	AM_RANGE(0x1584, 0x1584) AM_NOP                                               /* Possibly still interrupt enable */
	AM_RANGE(0x1585, 0x1585) AM_WRITE(p8257_drq_w)          /* P8257 ==> /DRQ0 /DRQ1 */
	AM_RANGE(0x1586, 0x1587) AM_WRITE(dkong_palettebank_w)
	AM_RANGE(0x1600, 0x17ff) AM_RAM                                               /* 0x6400  spriteram location */
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE(dkong_videoram_w) AM_SHARE("video_ram")        /* 0x7400 */
	AM_RANGE(0x1C00, 0x1f7f) AM_RAM                                               /* 0x6000 */
	AM_RANGE(0x1f80, 0x1f8f) AM_DEVREADWRITE("dma8257", i8257_device, read, write)   /* P8257 control registers */
	/* 0x6800 not remapped */
	AM_RANGE(0x2000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x3fff) AM_READWRITE(s2650_mirror_r, s2650_mirror_w)
	AM_RANGE(0x4000, 0x4fff) AM_ROM
	AM_RANGE(0x5000, 0x5fff) AM_READWRITE(s2650_mirror_r, s2650_mirror_w)
	AM_RANGE(0x6000, 0x6fff) AM_ROM
	AM_RANGE(0x7000, 0x7fff) AM_READWRITE(s2650_mirror_r, s2650_mirror_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( s2650_io_map, AS_IO, 8, dkong_state )
	AM_RANGE(0x00, 0x00) AM_READ(s2650_port0_r)
	AM_RANGE(0x01, 0x01) AM_READ(s2650_port1_r)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_WRITE(s2650_data_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( radarscp_in0_2 )
	PORT_START("IN0")      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not connected - held to high */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not connected - held to high */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not connected - held to high */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not connected - held to high */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not connected - held to high */
INPUT_PORTS_END

static INPUT_PORTS_START( radarscp_in1_2 )
	PORT_START("IN1")      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_COCKTAIL /* not connected - held to high */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) PORT_COCKTAIL /* not connected - held to high */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
INPUT_PORTS_END

static INPUT_PORTS_START( dkong_in0_4 )
	PORT_START("IN0")      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
INPUT_PORTS_END

static INPUT_PORTS_START( dkong_in1_4 )
	PORT_START("IN1")      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
INPUT_PORTS_END

static INPUT_PORTS_START( dkong_in0_8 )
	PORT_START("IN0")      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
INPUT_PORTS_END

static INPUT_PORTS_START( dkong_in1_8 )
	PORT_START("IN1")      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* not connected - held to high */
INPUT_PORTS_END

static INPUT_PORTS_START( dkong_in2 )
	/* Bit 0x80 is (SERVICE OR COIN) !
	 * Bit 0x01 is going to the connector but it is not labeled
	 * It should be a IPT_UNKNOWN. In fact, it will reset the game.
	 */
	PORT_START("IN2")      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN  ) /* connection not labeled in schematics - reset */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* connection not labeled in schematics - freeze or reset */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE )   /* not connected - held to high - used as service */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* not connected - held to high */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* status from sound cpu */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )
INPUT_PORTS_END

static INPUT_PORTS_START( dkong_dsw0 )
	PORT_START("DSW0")      /* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION( "SW1:!1,!2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION( "SW1:!3,!4" )
	PORT_DIPSETTING(    0x00, "7000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x08, "15000" )
	PORT_DIPSETTING(    0x0c, "20000" )
	PORT_DIPNAME( 0x70, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION( "SW1:!5,!6,!7" )
	PORT_DIPSETTING(    0x70, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )      PORT_DIPLOCATION( "SW1:!8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dkong_config )
	PORT_START("VR2")
	PORT_ADJUSTER( 90, "VR2 - DAC Volume" )

	PORT_START("VIDHW")
	PORT_CONFNAME( 0x01, 0x01, "Video Hardware" )
	PORT_CONFSETTING(    0x00, "TKG-02 (Radarscope Conversion)" )
	PORT_CONFSETTING(    0x01, "TKG-04 (Two board set)" )
INPUT_PORTS_END

static INPUT_PORTS_START( dkong )
	PORT_INCLUDE( dkong_in0_4 )
	PORT_INCLUDE( dkong_in1_4 )
	PORT_INCLUDE( dkong_in2 )
	PORT_INCLUDE( dkong_dsw0 )

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END

static INPUT_PORTS_START( dkongf )
	PORT_INCLUDE( dkong )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x70, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION( "SW1:!5,!6,!7" )
	PORT_DIPSETTING(    0x70, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dkongx )
	/* Supposedly the physical DIPS are read as defaults for the NVRAM when it's initially created.
	    The settings here match those from the default DSW0 settings.  Beyond the initial NVRAM
	    creation, DIPS (other than CABINET) can only be adjusted from the Service Mode */
	PORT_INCLUDE( dkong )

	PORT_MODIFY("DSW0")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW1:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW1:!2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW1:!3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW1:!4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW1:!5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW1:!7" )
INPUT_PORTS_END

static INPUT_PORTS_START( radarscp )
	PORT_INCLUDE( radarscp_in0_2 )
	PORT_INCLUDE( radarscp_in1_2 )

	PORT_INCLUDE( dkong_in2 )

	PORT_INCLUDE( dkong_dsw0 )

	PORT_START("VR2")
	PORT_ADJUSTER( 35, "VR2 - DAC Volume" )

INPUT_PORTS_END

static INPUT_PORTS_START( dkong3 )
	PORT_START("IN0")      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN3 )

	PORT_START("IN1")      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW0")      /* DSW0 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_6C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW2:!4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW2:!5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW2:!6" )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_HIGH, "SW2:!7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x08, "50000" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, "Additional Bonus" )          PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x10, "40000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dkong3b )
	PORT_INCLUDE( dkong_in0_4 )
	PORT_INCLUDE( dkong_in1_4 )

	PORT_INCLUDE( dkong_in2 )

	PORT_START("DSW0")      /* DSW0 */
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:!1,!2,!3")
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:!5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1:!6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1:!7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1:!8" )
INPUT_PORTS_END

static INPUT_PORTS_START( dkongjr )
	PORT_INCLUDE( dkong_in0_4 )
	PORT_INCLUDE( dkong_in1_4 )
	PORT_INCLUDE( dkong_in2 )

	PORT_INCLUDE( dkong_dsw0 )
	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION( "SW1:!3,!4" )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x0c, "25000" )

#if DEBUG_DISC_SOUND
	PORT_START("TST")      /* TST */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CODE(KEYCODE_B)
#endif
INPUT_PORTS_END

static INPUT_PORTS_START( hunchbkd )

	PORT_INCLUDE( dkong_in0_8 )
	PORT_INCLUDE( dkong_in1_8 )

	PORT_INCLUDE( dkong_in2 )
	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )

	PORT_INCLUDE( dkong_dsw0 )
	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION( "SW1:!1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION( "SW1:!2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION( "SW1:!3,!4" )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x0c, "80000" )

	PORT_START("SENSE") /* Sense */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END

static INPUT_PORTS_START( shootgal )
	PORT_START("IN0")      /* IN0 */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("IN1")      /* IN1 */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_INCLUDE( dkong_in2 )
	PORT_MODIFY("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Start 1 / P1 Button 1") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Start 2 / P1 Button 2") PORT_PLAYER(1)
	/* PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) */
	/* PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) */

	PORT_START("DSW0")      /* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, "1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Time" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPSETTING(    0x04, "100" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("SENSE") /* Sense */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END

static INPUT_PORTS_START( sbdk )
	PORT_INCLUDE( dkong_in0_8 )
	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not used by clone */

	PORT_INCLUDE( dkong_in1_8 )
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not used by clone */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not used by clone */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not used by clone */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not used by clone */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED ) /* not used by clone */

	PORT_INCLUDE( dkong_in2 )
	PORT_MODIFY("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Start 1 / P1 Button 1") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Start 2 / P1 Button 2") PORT_PLAYER(1)

	PORT_INCLUDE( dkong_dsw0 )
	PORT_MODIFY("DSW0")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW1:!1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION( "SW1:!2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:!4" )

	PORT_START("SENSE") /* Sense */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END

static INPUT_PORTS_START( herbiedk )
	PORT_INCLUDE( dkong_in0_8 )
	PORT_INCLUDE( dkong_in1_8 )

	PORT_INCLUDE( dkong_in2 )

	PORT_INCLUDE( dkong_dsw0 )
	PORT_MODIFY("DSW0")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x00, "SW1:!1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW1:!2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:!3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:!4" )

	PORT_START("SENSE") /* Sense */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END

/* Notes :
     - you ALWAYS get an extra life at 150000 points.
     - having more than 6 lives will reset the game.
*/

static INPUT_PORTS_START( herodk )
	PORT_INCLUDE( dkong_in0_8 )
	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )

	PORT_INCLUDE( dkong_in1_8 )
	PORT_MODIFY("IN1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_INCLUDE( dkong_in2 )

	PORT_INCLUDE( dkong_dsw0 )
	PORT_MODIFY("DSW0")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:!1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION( "SW1:!2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x0c, 0x00, "Difficulty?" )           PORT_DIPLOCATION( "SW1:!3,!4" ) /* Stored at 0x1c99 */
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )

	PORT_START("SENSE") /* Sense */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END


static INPUT_PORTS_START( pestplce )
	PORT_START("IN0")      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE( dkong_in2 )

	PORT_START("DSW0")      /* DSW0 */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "2 Players Game" )
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x20, "2 Credits" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x80, "40000" )
	PORT_DIPSETTING(    0xc0, DEF_STR ( None ) )

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END


static INPUT_PORTS_START( spclforc )
	PORT_INCLUDE( dkong_in0_8 )
	PORT_MODIFY("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE( dkong_in1_8 )
	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_INCLUDE( dkong_in2 )
	PORT_MODIFY("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Start 1 / P1 Button 1") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Start 2 / P1 Button 2") PORT_PLAYER(1)

	PORT_START("DSW0")      /* DSW0 */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPSETTING(    0x08, "50000" )
	PORT_DIPSETTING(    0x10, "60000" )
	PORT_DIPSETTING(    0x18, "70000" )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )

	PORT_START("SENSE") /* Sense */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END


static INPUT_PORTS_START( 8ballact )
	PORT_INCLUDE( dkong_in0_8 )
	PORT_INCLUDE( dkong_in1_8 )

	PORT_INCLUDE( dkong_in2 )

	PORT_START("DSW0")      /* DSW0 */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW1:!1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW1:!2" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION( "SW1:!3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION( "SW1:!4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x70, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION( "SW1:!5,!6,!7" )
	PORT_DIPSETTING(    0x70, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:!8" )

	PORT_START("SENSE") /* Sense */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END

static INPUT_PORTS_START( drakton )
	PORT_INCLUDE( dkong_in0_4 )
	PORT_INCLUDE( dkong_in1_4 )

	PORT_INCLUDE( dkong_in2 )

	PORT_START("DSW0")      /* DSW0 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x70, 0x10, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "20000" )
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPSETTING(    0x50, "60000" )
	PORT_DIPSETTING(    0x60, "70000" )
	PORT_DIPSETTING(    0x70, "80000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END


static INPUT_PORTS_START( strtheat )
	PORT_START("IN0")      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_INCLUDE( dkong_in2 )

	PORT_START("DSW0")      /* DSW0 */
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x18, "40000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x28, "60000" )
	PORT_DIPSETTING(    0x30, "70000" )
	PORT_DIPSETTING(    0x38, "80000" )
	PORT_DIPNAME( 0x40, 0x00,"Control type" )
	PORT_DIPSETTING(    0x00, "Steering Wheel" )
	PORT_DIPSETTING(    0x40, DEF_STR ( Joystick ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("IN4")      /* IN4 */
	PORT_BIT( 0x03, 0x00, IPT_DIAL ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("IN5")      /* IN5 */
	PORT_BIT( 0x03, 0x00, IPT_DIAL ) PORT_SENSITIVITY(40) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL

	PORT_INCLUDE( dkong_config )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,                                  /* 16*16 sprites */
	RGN_FRAC(1,4),                          /* 128 sprites */
	2,                                      /* 2 bits per pixel */
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },       /* the two bitplanes are separated */
	{ STEP8(0,1), STEP8(RGN_FRAC(1,4),1) }, /* the two halves of the sprite are separated */
	{ STEP16(0,8) },
	16*8                                    /* every sprite takes 16 consecutive bytes */
};

static const gfx_layout pestplce_spritelayout =
{
	16,16,                                  /* 16*16 sprites */
	RGN_FRAC(1,4),                          /* 256 sprites */
	2,                                      /* 2 bits per pixel */
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },       /* the two bitplanes are separated */
	{ STEP8(0,1), STEP8(RGN_FRAC(1,4),1) }, /* the two halves of the sprite are separated */
	{ STEP16(0,8) },
	16*8                                    /* every sprite takes 16 consecutive bytes */
};

static GFXDECODE_START( dkong )
	GFXDECODE_ENTRY( "gfx1", 0x0000, gfx_8x8x2_planar,   0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout,       0, 64 )
GFXDECODE_END

static GFXDECODE_START( pestplce )
	GFXDECODE_ENTRY( "gfx1", 0x0000, gfx_8x8x2_planar,         0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, pestplce_spritelayout,   0, 64 )
GFXDECODE_END


/*************************************
 *
 *  Braze Tech Addon boards
 *
 *************************************/

READ8_MEMBER(dkong_state::braze_eeprom_r)
{
	return m_eeprom->do_read();
}

WRITE8_MEMBER(dkong_state::braze_a15_w)
{
	membank("bank1")->set_entry(data & 0x01);
	membank("bank2")->set_entry(data & 0x01);
}

WRITE8_MEMBER(dkong_state::braze_eeprom_w)
{
	m_eeprom->di_write(data & 0x01);
	m_eeprom->cs_write(data & 0x04 ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->clk_write(data & 0x02 ? ASSERT_LINE : CLEAR_LINE);
}

void dkong_state::braze_decrypt_rom(UINT8 *dest)
{
	UINT8 oldbyte,newbyte;
	UINT8 *ROM;
	UINT32 mem;
	UINT32 newmem;

	ROM = memregion("braze")->base();

	for (mem=0;mem<0x10000;mem++)
	{
		oldbyte = ROM[mem];

		newmem = ((BITSWAP8((mem >> 8),7,2,3,1,0,6,4,5))<<8) | (mem & 0xff);
		newbyte = BITSWAP8(oldbyte, 1,4,5,7,6,0,3,2);

		dest[newmem] = newbyte;
	}
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

INTERRUPT_GEN_MEMBER(dkong_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE_LINE_MEMBER(dkong_state::busreq_w )
{
	// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_maincpu->set_input_line(INPUT_LINE_HALT, state); // do we need this?
	if(m_z80dma)
		m_z80dma->bai_w(state); // tell dma that bus has been granted
	else if(m_dma8257)
		m_dma8257->hlda_w(state);
}

static MACHINE_CONFIG_START( dkong_base, dkong_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CLOCK_1H)
	MCFG_CPU_PROGRAM_MAP(dkong_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dkong_state,  vblank_irq)

	MCFG_MACHINE_START_OVERRIDE(dkong_state,dkong2b)
	MCFG_MACHINE_RESET_OVERRIDE(dkong_state,dkong)

	MCFG_DEVICE_ADD("dma8257", I8257, CLOCK_1H)
	MCFG_I8257_OUT_HRQ_CB(WRITELINE(dkong_state, busreq_w))
	MCFG_I8257_IN_MEMR_CB(READ8(dkong_state, memory_read_byte))
	MCFG_I8257_OUT_MEMW_CB(WRITE8(dkong_state, memory_write_byte))
	MCFG_I8257_IN_IOR_1_CB(READ8(dkong_state, p8257_ctl_r))
	MCFG_I8257_OUT_IOW_0_CB(WRITE8(dkong_state, p8257_ctl_w))
	MCFG_I8257_REVERSE_RW_MODE(1) // why?

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(dkong_state, screen_update_dkong)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dkong)
	MCFG_PALETTE_ADD("palette", DK2B_PALETTE_LENGTH)

	MCFG_PALETTE_INIT_OWNER(dkong_state,dkong2b)
	MCFG_VIDEO_START_OVERRIDE(dkong_state,dkong)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( radarscp, dkong_base )

	/* basic machine hardware */
	MCFG_MACHINE_START_OVERRIDE(dkong_state,radarscp)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(RS_PALETTE_LENGTH)
	MCFG_PALETTE_INIT_OWNER(dkong_state,radarscp)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(radarscp_audio)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( radarscp1, dkong_base )

	/* basic machine hardware */
	MCFG_MACHINE_START_OVERRIDE(dkong_state,radarscp1)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(RS_PALETTE_LENGTH)
	MCFG_PALETTE_INIT_OWNER(dkong_state,radarscp1)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(radarscp1_audio)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( dkong2b, dkong_base )

	/* basic machine hardware */
	MCFG_MACHINE_START_OVERRIDE(dkong_state,dkong2b)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(DK2B_PALETTE_LENGTH)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(dkong2b_audio)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( braze, dkong2b )

	MCFG_EEPROM_SERIAL_93C46_8BIT_ADD("eeprom")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( dkong3, dkong_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_8MHz / 2) /* verified in schematics */
	MCFG_CPU_PROGRAM_MAP(dkong3_map)
	MCFG_CPU_IO_MAP(dkong3_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dkong_state, vblank_irq)

	MCFG_MACHINE_START_OVERRIDE(dkong_state, dkong3)

	MCFG_DEVICE_ADD("z80dma", Z80DMA, CLOCK_1H)
	MCFG_Z80DMA_OUT_BUSREQ_CB(INPUTLINE("maincpu", INPUT_LINE_HALT))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(dkong_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(dkong_state, memory_write_byte))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(dkong_state, screen_update_dkong)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dkong)
	MCFG_PALETTE_ADD("palette", DK3_PALETTE_LENGTH)

	MCFG_PALETTE_INIT_OWNER(dkong_state,dkong3)
	MCFG_VIDEO_START_OVERRIDE(dkong_state,dkong)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(dkong3_audio)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dkongjr, dkong_base )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dkongjr_map)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(dkongjr_audio)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pestplce, dkongjr )

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", pestplce)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(DK2B_PALETTE_LENGTH)
	MCFG_PALETTE_INIT_OWNER(dkong_state,dkong2b)  /* wrong! */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(dkong_state, screen_update_pestplce)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( dkong3b, dkongjr )

	/* basic machine hardware */
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(dkong_state,dkong3)
MACHINE_CONFIG_END

/*************************************
 *
 * S2650 Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_DERIVED( s2650, dkong2b )

	/* basic machine hardware */
	MCFG_CPU_REPLACE("maincpu", S2650, CLOCK_1H / 2)    /* ??? */
	MCFG_CPU_PROGRAM_MAP(s2650_map)
	MCFG_CPU_IO_MAP(s2650_io_map)
	MCFG_S2650_FLAG_HANDLER(WRITELINE(dkong_state, s2650_fo_w))
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dkong_state,  s2650_interrupt)

	MCFG_DEVICE_MODIFY("dma8257")
	MCFG_I8257_IN_MEMR_CB(READ8(dkong_state, hb_dma_read_byte))
	MCFG_I8257_OUT_MEMW_CB(WRITE8(dkong_state, hb_dma_write_byte))

	MCFG_MACHINE_START_OVERRIDE(dkong_state,s2650)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( spclforc, s2650 )

	/* basic machine hardware */
	MCFG_DEVICE_REMOVE("soundcpu")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(dkong_state, screen_update_spclforc)
MACHINE_CONFIG_END

/*************************************
 *
 * EPOS Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_DERIVED( strtheat, dkong2b )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(epos_readport)

	MCFG_MACHINE_RESET_OVERRIDE(dkong_state,strtheat)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( drakton, dkong2b )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(epos_readport)

	MCFG_MACHINE_RESET_OVERRIDE(dkong_state,drakton)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( drktnjr, dkongjr )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(epos_readport)

	MCFG_MACHINE_RESET_OVERRIDE(dkong_state,drakton)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( radarscp ) /* unclear which boardset this comes from; there existed a 5 pcb stack with trs-03 (no voice) sound board on top, and a 4 board as well as a 2 board pcb stack */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trs2c5fc",     0x0000, 0x1000, CRC(40949e0d) SHA1(94717b9d027600e25b863e89900df41325875961) )
	ROM_LOAD( "trs2c5gc",     0x1000, 0x1000, CRC(afa8c49f) SHA1(25880e9dcf2dc8862f7f3c38687f01dfe2424293) )
	ROM_LOAD( "trs2c5hc",     0x2000, 0x1000, CRC(51b8263d) SHA1(09687f2c40cf09ffc2aeddde4a4fa32800847f01) )
	ROM_LOAD( "trs2c5kc",     0x3000, 0x1000, CRC(1f0101f7) SHA1(b9f988847fdefa64dfeae06c2244215cb0d64dbe) )
	/* space for diagnostic ROM */

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "trs2s3i",      0x0000, 0x0800, CRC(78034f14) SHA1(548b44ac69f39df6687da1c0f60968009b1e0767) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_FILL(                 0x1000, 0x0800, 0x00 )
	/* socket 3J is empty */

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "trs2v3gc",     0x0000, 0x0800, CRC(f095330e) SHA1(dd3de744f28ff108630d3336bd246d3323fa34af) )
	ROM_LOAD( "trs2v3hc",     0x0800, 0x0800, CRC(15a316f0) SHA1(8785a996c6433882a0a7150693c329a4247bb77e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trs2v3dc",     0x0000, 0x0800, CRC(e0bb0db9) SHA1(b570439ea1b5d34d0ac938ac9157f22f319b786d) )
	ROM_LOAD( "trs2v3cc",     0x0800, 0x0800, CRC(6c4e7dad) SHA1(54e6a5005c44261dc4ba845dcd5ff62ea1402d26) )
	ROM_LOAD( "trs2v3bc",     0x1000, 0x0800, CRC(6fdd63f1) SHA1(2eb09ab0759e4c8df9188fb833440d8fc94f6172) )
	ROM_LOAD( "trs2v3ac",     0x1800, 0x0800, CRC(bbf62755) SHA1(cb4ca8d4fe689ca0011a4b6c0a2dbd4c764ac70a) )

	ROM_REGION( 0x0800, "gfx3", 0 ) /* radar/star timing table */
	ROM_LOAD( "trs2v3ec",     0x0000, 0x0800, CRC(0eca8d6b) SHA1(8358b5131d082b2fb8dd793d2e5382daeef6f75c) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "rs2-x.xxx",    0x0000, 0x0100, CRC(54609d61) SHA1(586620ecc61f3e55258fe6360bcacad5f570f29c) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "rs2-c.xxx",    0x0100, 0x0100, CRC(79a7d831) SHA1(475ec991929d43b2bcd4b5aee144249f487d0b5b) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "rs2-v.1hc",    0x0200, 0x0100, CRC(1b828315) SHA1(00c9f8c5ae86b68d38c66f9071b5f1ef421c1005) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( radarscp1 ) /* TRS01 5-pcb stack with TRS01 "Voice" pcb on top containing the sound cpu and the m58817 speech chip and the m58819 speech serial rom emulator chip */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trs01_5f",     0x0000, 0x1000, CRC(40949e0d) SHA1(94717b9d027600e25b863e89900df41325875961) )
	ROM_LOAD( "trs01_5g",     0x1000, 0x1000, CRC(afa8c49f) SHA1(25880e9dcf2dc8862f7f3c38687f01dfe2424293) )
	ROM_LOAD( "trs01_5h",     0x2000, 0x1000, CRC(51b8263d) SHA1(09687f2c40cf09ffc2aeddde4a4fa32800847f01) )
	ROM_LOAD( "trs01_5k",     0x3000, 0x1000, CRC(1f0101f7) SHA1(b9f988847fdefa64dfeae06c2244215cb0d64dbe) )
	/* space for diagnostic ROM */

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "trs-s__5a.5a", 0x0000, 0x0800, CRC(5166554c) SHA1(00bf501ca448929f6187598da6fdbc1ea488745a) ) // 5A on the 'voice' board on top of the 5-pcb stack; eprom label is "TRS-S ['S' overstrikes a '1'] // 5A [stamped '8' or 'a' in red ink]"
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_FILL(                 0x1000, 0x0800, 0xFF )

	ROM_REGION( 0x0800, "m58819", 0 )  /* speech rom */
	ROM_LOAD( "trs-s__4h.4h",      0x0000, 0x0800, CRC(d1f1b48c) SHA1(ee5584368d2e9f7bde271f5004585b53f5ff5c3f) ) // 4H on the 'voice' board on top of the 5-pcb stack; eprom label is "TRS-S ['S' overstrikes a '1'] // 4H [stamped '8' or 'a' in red ink]"

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "trs01v3f",     0x0000, 0x0800, CRC(f095330e) SHA1(dd3de744f28ff108630d3336bd246d3323fa34af) )
	ROM_LOAD( "trs01v3g",     0x0800, 0x0800, CRC(15a316f0) SHA1(8785a996c6433882a0a7150693c329a4247bb77e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "trs01v3d",     0x0000, 0x0800, CRC(e0bb0db9) SHA1(b570439ea1b5d34d0ac938ac9157f22f319b786d) )
	ROM_LOAD( "trs01v3c",     0x0800, 0x0800, CRC(6c4e7dad) SHA1(54e6a5005c44261dc4ba845dcd5ff62ea1402d26) )
	ROM_LOAD( "trs01v3b",     0x1000, 0x0800, CRC(6fdd63f1) SHA1(2eb09ab0759e4c8df9188fb833440d8fc94f6172) )
	ROM_LOAD( "trs01v3a",     0x1800, 0x0800, CRC(bbf62755) SHA1(cb4ca8d4fe689ca0011a4b6c0a2dbd4c764ac70a) )

	ROM_REGION( 0x0800, "gfx3", 0 ) /* radar/star timing table */
	ROM_LOAD( "trs011ha.bin",    0x0000, 0x0800, CRC(dbcc50c2) SHA1(1e438057d4d93ba22794ab0a9bf41bb49ac28a35) ) /* star /grid */

	ROM_REGION( 0x0100, "gfx4", 0 ) /* priority based on hor. pos */
	ROM_LOAD( "trs01e3k.bin",    0x0000, 0x0100, CRC(6c6f989c) SHA1(d4b90e43d93ef141a8002b88ce5e33411b870ced) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "trs01c2j.bin",    0x0000, 0x0100, CRC(2a087c87) SHA1(dbf0c6173583dc4fa5d3f34d2f42cbaf2bd4b167) ) /* blue */
	ROM_LOAD( "trs01c2k.bin",    0x0100, 0x0100, CRC(650c5daf) SHA1(72f91ee2fab9eee58ee42881327e6345aa70b7f9) ) /* green */
	ROM_LOAD( "trs01c2l.bin",    0x0200, 0x0100, CRC(23087910) SHA1(afc05c322b11fefaf0af857fee06a5afd0d4593e) ) /* red */
	/* Hack! The prom at pos 1D on video board has not been dumped
	 * Rom 1D is a MB7051, only 5 address lines
	 * Rom below from TRS02 dump: rs2-v.1hc
	 */
	ROM_LOAD( "trs01v1d.bin",    0x0300, 0x0100, BAD_DUMP CRC(1b828315) SHA1(00c9f8c5ae86b68d38c66f9071b5f1ef421c1005) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( dkong ) /* Confirmed TKG-04 Upgrade as mentioned in Nintendo Service Department Bulletin # TKG-02 12-11-81 */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c_5et_g.bin",  0x0000, 0x1000, CRC(ba70b88b) SHA1(d76ebecfea1af098d843ee7e578e480cd658ac1a) )
	ROM_LOAD( "c_5ct_g.bin",  0x1000, 0x1000, CRC(5ec461ec) SHA1(acb11a8fbdbb3ab46068385fe465f681e3c824bd) )
	ROM_LOAD( "c_5bt_g.bin",  0x2000, 0x1000, CRC(1c97d324) SHA1(c7966261f3a1d3296927e0b6ee1c58039fc53c1f) )
	ROM_LOAD( "c_5at_g.bin",  0x3000, 0x1000, CRC(b9005ac0) SHA1(3fe3599f6fa7c496f782053ddf7bacb453d197c4) )
	/* space for diagnostic ROM */

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "s_3i_b.bin",   0x0000, 0x0800, CRC(45a4ed06) SHA1(144d24464c1f9f01894eb12f846952290e6e32ef) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "s_3j_b.bin",   0x1000, 0x0800, CRC(4743fe92) SHA1(6c82b57637c0212a580591397e6a5a1718f19fd2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "v_5h_b.bin",   0x0000, 0x0800, CRC(12c8c95d) SHA1(a57ff5a231c45252a63b354137c920a1379b70a3) )
	ROM_LOAD( "v_3pt.bin",    0x0800, 0x0800, CRC(15e9c5e9) SHA1(976eb1e18c74018193a35aa86cff482ebfc5cc4e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "l_4m_b.bin",   0x0000, 0x0800, CRC(59f8054d) SHA1(793dba9bf5a5fe76328acdfb90815c243d2a65f1) )
	ROM_LOAD( "l_4n_b.bin",   0x0800, 0x0800, CRC(672e4714) SHA1(92e5d379f4838ac1fa44d448ce7d142dae42102f) )
	ROM_LOAD( "l_4r_b.bin",   0x1000, 0x0800, CRC(feaa59ee) SHA1(ecf95db5a20098804fc8bd59232c66e2e0ed3db4) )
	ROM_LOAD( "l_4s_b.bin",   0x1800, 0x0800, CRC(20f2ef7e) SHA1(3bc482a38bf579033f50082748ee95205b0f673d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2k.bpr",     0x0000, 0x0100, CRC(e273ede5) SHA1(b50ec9e1837c00c20fb2a4369ec7dd0358321127) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2j.bpr",     0x0100, 0x0100, CRC(d6412358) SHA1(f9c872da2fe8e800574ae3bf483fb3ccacc92eb3) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-5e.bpr",     0x0200, 0x0100, CRC(b869b8f5) SHA1(c2bdccbf2654b64ea55cd589fd21323a9178a660) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( dkonghrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dk5ehard.bin", 0x0000, 0x1000, CRC(a9445215) SHA1(9f2ca30cc0a26f5294395c204c6213769628cdfc) )
	ROM_LOAD( "c_5ct_g.bin",  0x1000, 0x1000, CRC(5ec461ec) SHA1(acb11a8fbdbb3ab46068385fe465f681e3c824bd) )
	ROM_LOAD( "c_5bt_g.bin",  0x2000, 0x1000, CRC(1c97d324) SHA1(c7966261f3a1d3296927e0b6ee1c58039fc53c1f) )
	ROM_LOAD( "dk5ahard.bin", 0x3000, 0x1000, CRC(a990729b) SHA1(ebb050ce2504fe5f2bbf1fc56018f85a083fe4d0) )
	/* space for diagnostic ROM */

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "s_3i_b.bin",   0x0000, 0x0800, CRC(45a4ed06) SHA1(144d24464c1f9f01894eb12f846952290e6e32ef) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "s_3j_b.bin",   0x1000, 0x0800, CRC(4743fe92) SHA1(6c82b57637c0212a580591397e6a5a1718f19fd2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "v_5h_b.bin",   0x0000, 0x0800, CRC(12c8c95d) SHA1(a57ff5a231c45252a63b354137c920a1379b70a3) )
	ROM_LOAD( "v_3pt.bin",    0x0800, 0x0800, CRC(15e9c5e9) SHA1(976eb1e18c74018193a35aa86cff482ebfc5cc4e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "l_4m_b.bin",   0x0000, 0x0800, CRC(59f8054d) SHA1(793dba9bf5a5fe76328acdfb90815c243d2a65f1) )
	ROM_LOAD( "l_4n_b.bin",   0x0800, 0x0800, CRC(672e4714) SHA1(92e5d379f4838ac1fa44d448ce7d142dae42102f) )
	ROM_LOAD( "l_4r_b.bin",   0x1000, 0x0800, CRC(feaa59ee) SHA1(ecf95db5a20098804fc8bd59232c66e2e0ed3db4) )
	ROM_LOAD( "l_4s_b.bin",   0x1800, 0x0800, CRC(20f2ef7e) SHA1(3bc482a38bf579033f50082748ee95205b0f673d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2k.bpr",     0x0000, 0x0100, CRC(e273ede5) SHA1(b50ec9e1837c00c20fb2a4369ec7dd0358321127) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2j.bpr",     0x0100, 0x0100, CRC(d6412358) SHA1(f9c872da2fe8e800574ae3bf483fb3ccacc92eb3) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-5e.bpr",     0x0200, 0x0100, CRC(b869b8f5) SHA1(c2bdccbf2654b64ea55cd589fd21323a9178a660) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c_5f_b.bin",   0x0000, 0x1000, CRC(424f2b11) SHA1(e4f096f2bbd37281f42a5f8e083738f55c07f3dd) )    /* tkg3c.5f */
	ROM_LOAD( "c_5ct_g.bin",  0x1000, 0x1000, CRC(5ec461ec) SHA1(acb11a8fbdbb3ab46068385fe465f681e3c824bd) )    /* tkg3c.5g */
	ROM_LOAD( "c_5h_b.bin",   0x2000, 0x1000, CRC(1d28895d) SHA1(63792cab215fc2a7b0e8ee61d8115045571e9d42) )    /* tkg3c.5h */
	ROM_LOAD( "tkg3c.5k",     0x3000, 0x1000, CRC(553b89bb) SHA1(61611df9e2748fdcd31821038dcc0e16dc933873) )
	/* space for diagnostic ROM */

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "s_3i_b.bin",   0x0000, 0x0800, CRC(45a4ed06) SHA1(144d24464c1f9f01894eb12f846952290e6e32ef) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "s_3j_b.bin",   0x1000, 0x0800, CRC(4743fe92) SHA1(6c82b57637c0212a580591397e6a5a1718f19fd2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "v_5h_b.bin",   0x0000, 0x0800, CRC(12c8c95d) SHA1(a57ff5a231c45252a63b354137c920a1379b70a3) )
	ROM_LOAD( "v_3pt.bin",    0x0800, 0x0800, CRC(15e9c5e9) SHA1(976eb1e18c74018193a35aa86cff482ebfc5cc4e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "l_4m_b.bin",   0x0000, 0x0800, CRC(59f8054d) SHA1(793dba9bf5a5fe76328acdfb90815c243d2a65f1) )
	ROM_LOAD( "l_4n_b.bin",   0x0800, 0x0800, CRC(672e4714) SHA1(92e5d379f4838ac1fa44d448ce7d142dae42102f) )
	ROM_LOAD( "l_4r_b.bin",   0x1000, 0x0800, CRC(feaa59ee) SHA1(ecf95db5a20098804fc8bd59232c66e2e0ed3db4) )
	ROM_LOAD( "l_4s_b.bin",   0x1800, 0x0800, CRC(20f2ef7e) SHA1(3bc482a38bf579033f50082748ee95205b0f673d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2k.bpr",     0x0000, 0x0100, CRC(e273ede5) SHA1(b50ec9e1837c00c20fb2a4369ec7dd0358321127) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2j.bpr",     0x0100, 0x0100, CRC(d6412358) SHA1(f9c872da2fe8e800574ae3bf483fb3ccacc92eb3) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-5e.bpr",     0x0200, 0x0100, CRC(b869b8f5) SHA1(c2bdccbf2654b64ea55cd589fd21323a9178a660) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c_5f_b.bin",   0x0000, 0x1000, CRC(424f2b11) SHA1(e4f096f2bbd37281f42a5f8e083738f55c07f3dd) )
	ROM_LOAD( "5g.cpu",       0x1000, 0x1000, CRC(d326599b) SHA1(94c7382604d0a123a442d53f9641f366dfbb7631) )
	ROM_LOAD( "5h.cpu",       0x2000, 0x1000, CRC(ff31ac89) SHA1(9626a9e6df0d1b0ff273dbbe986f670200f91f75) )
	ROM_LOAD( "c_5k_b.bin",   0x3000, 0x1000, CRC(394d6007) SHA1(57e5ae76ef5d4a2fa9cd860b6c6be03b6d5ed5ba) )

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "s_3i_b.bin",   0x0000, 0x0800, CRC(45a4ed06) SHA1(144d24464c1f9f01894eb12f846952290e6e32ef) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "s_3j_b.bin",   0x1000, 0x0800, CRC(4743fe92) SHA1(6c82b57637c0212a580591397e6a5a1718f19fd2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "v_5h_b.bin",   0x0000, 0x0800, CRC(12c8c95d) SHA1(a57ff5a231c45252a63b354137c920a1379b70a3) )
	ROM_LOAD( "v_5k_b.bin",   0x0800, 0x0800, CRC(3684f914) SHA1(882ae48ec1eabf5d350438dfec37ab20f7ee155d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "l_4m_b.bin",   0x0000, 0x0800, CRC(59f8054d) SHA1(793dba9bf5a5fe76328acdfb90815c243d2a65f1) )
	ROM_LOAD( "l_4n_b.bin",   0x0800, 0x0800, CRC(672e4714) SHA1(92e5d379f4838ac1fa44d448ce7d142dae42102f) )
	ROM_LOAD( "l_4r_b.bin",   0x1000, 0x0800, CRC(feaa59ee) SHA1(ecf95db5a20098804fc8bd59232c66e2e0ed3db4) )
	ROM_LOAD( "l_4s_b.bin",   0x1800, 0x0800, CRC(20f2ef7e) SHA1(3bc482a38bf579033f50082748ee95205b0f673d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2k.bpr",     0x0000, 0x0100, CRC(e273ede5) SHA1(b50ec9e1837c00c20fb2a4369ec7dd0358321127) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2j.bpr",     0x0100, 0x0100, CRC(d6412358) SHA1(f9c872da2fe8e800574ae3bf483fb3ccacc92eb3) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-5e.bpr",     0x0200, 0x0100, CRC(b869b8f5) SHA1(c2bdccbf2654b64ea55cd589fd21323a9178a660) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongjo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c_5f_b.bin",   0x0000, 0x1000, CRC(424f2b11) SHA1(e4f096f2bbd37281f42a5f8e083738f55c07f3dd) )
	ROM_LOAD( "c_5g_b.bin",   0x1000, 0x1000, CRC(3b2a6635) SHA1(32c62e00863ab99c6f263587d9d5bb775a68f3de) )
	ROM_LOAD( "c_5h_b.bin",   0x2000, 0x1000, CRC(1d28895d) SHA1(63792cab215fc2a7b0e8ee61d8115045571e9d42) )
	ROM_LOAD( "c_5k_b.bin",   0x3000, 0x1000, CRC(394d6007) SHA1(57e5ae76ef5d4a2fa9cd860b6c6be03b6d5ed5ba) )

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "s_3i_b.bin",   0x0000, 0x0800, CRC(45a4ed06) SHA1(144d24464c1f9f01894eb12f846952290e6e32ef) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "s_3j_b.bin",   0x1000, 0x0800, CRC(4743fe92) SHA1(6c82b57637c0212a580591397e6a5a1718f19fd2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "v_5h_b.bin",   0x0000, 0x0800, CRC(12c8c95d) SHA1(a57ff5a231c45252a63b354137c920a1379b70a3) )
	ROM_LOAD( "v_5k_b.bin",   0x0800, 0x0800, CRC(3684f914) SHA1(882ae48ec1eabf5d350438dfec37ab20f7ee155d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "l_4m_b.bin",   0x0000, 0x0800, CRC(59f8054d) SHA1(793dba9bf5a5fe76328acdfb90815c243d2a65f1) )
	ROM_LOAD( "l_4n_b.bin",   0x0800, 0x0800, CRC(672e4714) SHA1(92e5d379f4838ac1fa44d448ce7d142dae42102f) )
	ROM_LOAD( "l_4r_b.bin",   0x1000, 0x0800, CRC(feaa59ee) SHA1(ecf95db5a20098804fc8bd59232c66e2e0ed3db4) )
	ROM_LOAD( "l_4s_b.bin",   0x1800, 0x0800, CRC(20f2ef7e) SHA1(3bc482a38bf579033f50082748ee95205b0f673d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2k.bpr",     0x0000, 0x0100, CRC(e273ede5) SHA1(b50ec9e1837c00c20fb2a4369ec7dd0358321127) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2j.bpr",     0x0100, 0x0100, CRC(d6412358) SHA1(f9c872da2fe8e800574ae3bf483fb3ccacc92eb3) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-5e.bpr",     0x0200, 0x0100, CRC(b869b8f5) SHA1(c2bdccbf2654b64ea55cd589fd21323a9178a660) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongjo1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c_5f_b.bin",   0x0000, 0x1000, CRC(424f2b11) SHA1(e4f096f2bbd37281f42a5f8e083738f55c07f3dd) )
	ROM_LOAD( "5g.cpu",       0x1000, 0x1000, CRC(d326599b) SHA1(94c7382604d0a123a442d53f9641f366dfbb7631) )
	ROM_LOAD( "c_5h_b.bin",   0x2000, 0x1000, CRC(1d28895d) SHA1(63792cab215fc2a7b0e8ee61d8115045571e9d42) )
	ROM_LOAD( "5k.bin",       0x3000, 0x1000, CRC(7961599c) SHA1(698a4c2b8d67840dca7526efb1ac0d3370a86925) )

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "s_3i_b.bin",   0x0000, 0x0800, CRC(45a4ed06) SHA1(144d24464c1f9f01894eb12f846952290e6e32ef) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "s_3j_b.bin",   0x1000, 0x0800, CRC(4743fe92) SHA1(6c82b57637c0212a580591397e6a5a1718f19fd2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "v_5h_b.bin",   0x0000, 0x0800, CRC(12c8c95d) SHA1(a57ff5a231c45252a63b354137c920a1379b70a3) )
	ROM_LOAD( "v_5k_b.bin",   0x0800, 0x0800, CRC(3684f914) SHA1(882ae48ec1eabf5d350438dfec37ab20f7ee155d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "l_4m_b.bin",   0x0000, 0x0800, CRC(59f8054d) SHA1(793dba9bf5a5fe76328acdfb90815c243d2a65f1) )
	ROM_LOAD( "l_4n_b.bin",   0x0800, 0x0800, CRC(672e4714) SHA1(92e5d379f4838ac1fa44d448ce7d142dae42102f) )
	ROM_LOAD( "l_4r_b.bin",   0x1000, 0x0800, CRC(feaa59ee) SHA1(ecf95db5a20098804fc8bd59232c66e2e0ed3db4) )
	ROM_LOAD( "l_4s_b.bin",   0x1800, 0x0800, CRC(20f2ef7e) SHA1(3bc482a38bf579033f50082748ee95205b0f673d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2k.bpr",     0x0000, 0x0100, CRC(e273ede5) SHA1(b50ec9e1837c00c20fb2a4369ec7dd0358321127) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2j.bpr",     0x0100, 0x0100, CRC(d6412358) SHA1(f9c872da2fe8e800574ae3bf483fb3ccacc92eb3) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-5e.bpr",     0x0200, 0x0100, CRC(b869b8f5) SHA1(c2bdccbf2654b64ea55cd589fd21323a9178a660) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongpe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c_5et_g.bin",  0x0000, 0x1000, CRC(ba70b88b) SHA1(d76ebecfea1af098d843ee7e578e480cd658ac1a) )
	ROM_LOAD( "c_5ct_gp.bin", 0x1000, 0x1000, CRC(45af403e) SHA1(6030a4af7df98bfdf5b35a9a42541566f7d12901) )
	ROM_LOAD( "c_5bt_gp.bin", 0x2000, 0x1000, CRC(3a9783b7) SHA1(e98d757c048f2180ba22c774e0e425ddc661ba8c) )
	ROM_LOAD( "c_5at_gp.bin", 0x3000, 0x1000, CRC(32bc20ff) SHA1(ef141f437912923625722b83a33ea182eaa31427) )

	ROM_REGION( 0x1800, "soundcpu", 0 )
	ROM_LOAD( "s_3i_b.bin",   0x0000, 0x0800, CRC(45a4ed06) SHA1(144d24464c1f9f01894eb12f846952290e6e32ef) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "s_3j_b.bin",   0x1000, 0x0800, CRC(4743fe92) SHA1(6c82b57637c0212a580591397e6a5a1718f19fd2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "v_5h_bp.bin",  0x0000, 0x0800, CRC(007aa348) SHA1(ff2ae583fef6da9d260fda8f4a896dd0414c3388) )
	ROM_LOAD( "v_3ptp.bin",   0x0800, 0x0800, CRC(a967aff0) SHA1(7bcfdbeb0a5cdfec604eb8450664bc4b789526be) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "l_4m_bp.bin",  0x0000, 0x0800, CRC(766ae006) SHA1(0ec53798aa2c30b2c5c8b2f99b811a187faa2549) )
	ROM_LOAD( "l_4n_bp.bin",  0x0800, 0x0800, CRC(39e7ca4b) SHA1(b77ddd39608d08013fa8bb764c8e5aa4e03181dc) )
	ROM_LOAD( "l_4r_bp.bin",  0x1000, 0x0800, CRC(012f2f25) SHA1(836709192a249b00ded783be542ee844eb930c7a) )
	ROM_LOAD( "l_4s_bp.bin",  0x1800, 0x0800, CRC(84eb5bfb) SHA1(c1f38efb8670f1a489275eb8ff576a95d140cfb9) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2k.bpr",     0x0000, 0x0100, CRC(e273ede5) SHA1(b50ec9e1837c00c20fb2a4369ec7dd0358321127) )
	ROM_LOAD( "c-2j.bpr",     0x0100, 0x0100, CRC(d6412358) SHA1(f9c872da2fe8e800574ae3bf483fb3ccacc92eb3) )
	ROM_LOAD( "v-5e.bpr",     0x0200, 0x0100, CRC(b869b8f5) SHA1(c2bdccbf2654b64ea55cd589fd21323a9178a660) )
ROM_END

ROM_START( dkongf ) /* Donkey Kong Foundry (hack) from Jeff's Romhack */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dk_f.5et",  0x0000, 0x1000, CRC(00b7efaf) SHA1(97ed5930eb5d0cb98a9008b1d329ba7f3b8b8dbf) )
	ROM_LOAD( "dk_f.5ct",  0x1000, 0x1000, CRC(88af9b69) SHA1(c5621dd8198c333b3fa32fdece60ee5a3d8b2276) )
	ROM_LOAD( "dk_f.5bt",  0x2000, 0x1000, CRC(de74ad91) SHA1(c80227361bdbc565e9f5764e6364b52d40ca778a) )
	ROM_LOAD( "dk_f.5at",  0x3000, 0x1000, CRC(6a6bd420) SHA1(f012e2d21d906a2993af9cf8b2912ea6c928e94b) )
	/* space for diagnostic ROM */

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "s_3i_b.bin",   0x0000, 0x0800, CRC(45a4ed06) SHA1(144d24464c1f9f01894eb12f846952290e6e32ef) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "s_3j_b.bin",   0x1000, 0x0800, CRC(4743fe92) SHA1(6c82b57637c0212a580591397e6a5a1718f19fd2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "v_5h_b.bin",   0x0000, 0x0800, CRC(12c8c95d) SHA1(a57ff5a231c45252a63b354137c920a1379b70a3) )
	ROM_LOAD( "v_3pt.bin",    0x0800, 0x0800, CRC(15e9c5e9) SHA1(976eb1e18c74018193a35aa86cff482ebfc5cc4e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "l_4m_b.bin",   0x0000, 0x0800, CRC(59f8054d) SHA1(793dba9bf5a5fe76328acdfb90815c243d2a65f1) )
	ROM_LOAD( "l_4n_b.bin",   0x0800, 0x0800, CRC(672e4714) SHA1(92e5d379f4838ac1fa44d448ce7d142dae42102f) )
	ROM_LOAD( "l_4r_b.bin",   0x1000, 0x0800, CRC(feaa59ee) SHA1(ecf95db5a20098804fc8bd59232c66e2e0ed3db4) )
	ROM_LOAD( "l_4s_b.bin",   0x1800, 0x0800, CRC(20f2ef7e) SHA1(3bc482a38bf579033f50082748ee95205b0f673d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2k.bpr",     0x0000, 0x0100, CRC(e273ede5) SHA1(b50ec9e1837c00c20fb2a4369ec7dd0358321127) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2j.bpr",     0x0100, 0x0100, CRC(d6412358) SHA1(f9c872da2fe8e800574ae3bf483fb3ccacc92eb3) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-5e.bpr",     0x0200, 0x0100, CRC(b869b8f5) SHA1(c2bdccbf2654b64ea55cd589fd21323a9178a660) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c_5et_g.bin",  0x0000, 0x1000, CRC(ba70b88b) SHA1(d76ebecfea1af098d843ee7e578e480cd658ac1a) )
	ROM_LOAD( "c_5ct_g.bin",  0x1000, 0x1000, CRC(5ec461ec) SHA1(acb11a8fbdbb3ab46068385fe465f681e3c824bd) )
	ROM_LOAD( "c_5bt_g.bin",  0x2000, 0x1000, CRC(1c97d324) SHA1(c7966261f3a1d3296927e0b6ee1c58039fc53c1f) )
	ROM_LOAD( "c_5at_g.bin",  0x3000, 0x1000, CRC(b9005ac0) SHA1(3fe3599f6fa7c496f782053ddf7bacb453d197c4) )
	/* space for diagnostic ROM */

	ROM_REGION( 0x10000, "braze", 0 )
	ROM_LOAD( "d2k12.bin",  0x0000, 0x10000,  CRC(6e95ca0d) SHA1(c058add0f146d577e3df0ba60828fe1734e78d01) ) /* Version 1.2 */

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "s_3i_b.bin",   0x0000, 0x0800, CRC(45a4ed06) SHA1(144d24464c1f9f01894eb12f846952290e6e32ef) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "s_3j_b.bin",   0x1000, 0x0800, CRC(4743fe92) SHA1(6c82b57637c0212a580591397e6a5a1718f19fd2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "v_5h_b.bin",   0x0000, 0x0800, CRC(12c8c95d) SHA1(a57ff5a231c45252a63b354137c920a1379b70a3) )
	ROM_LOAD( "v_3pt.bin",    0x0800, 0x0800, CRC(15e9c5e9) SHA1(976eb1e18c74018193a35aa86cff482ebfc5cc4e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "l_4m_b.bin",   0x0000, 0x0800, CRC(59f8054d) SHA1(793dba9bf5a5fe76328acdfb90815c243d2a65f1) )
	ROM_LOAD( "l_4n_b.bin",   0x0800, 0x0800, CRC(672e4714) SHA1(92e5d379f4838ac1fa44d448ce7d142dae42102f) )
	ROM_LOAD( "l_4r_b.bin",   0x1000, 0x0800, CRC(feaa59ee) SHA1(ecf95db5a20098804fc8bd59232c66e2e0ed3db4) )
	ROM_LOAD( "l_4s_b.bin",   0x1800, 0x0800, CRC(20f2ef7e) SHA1(3bc482a38bf579033f50082748ee95205b0f673d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2k.bpr",     0x0000, 0x0100, CRC(e273ede5) SHA1(b50ec9e1837c00c20fb2a4369ec7dd0358321127) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2j.bpr",     0x0100, 0x0100, CRC(d6412358) SHA1(f9c872da2fe8e800574ae3bf483fb3ccacc92eb3) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-5e.bpr",     0x0200, 0x0100, CRC(b869b8f5) SHA1(c2bdccbf2654b64ea55cd589fd21323a9178a660) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongx11 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c_5et_g.bin",  0x0000, 0x1000, CRC(ba70b88b) SHA1(d76ebecfea1af098d843ee7e578e480cd658ac1a) )
	ROM_LOAD( "c_5ct_g.bin",  0x1000, 0x1000, CRC(5ec461ec) SHA1(acb11a8fbdbb3ab46068385fe465f681e3c824bd) )
	ROM_LOAD( "c_5bt_g.bin",  0x2000, 0x1000, CRC(1c97d324) SHA1(c7966261f3a1d3296927e0b6ee1c58039fc53c1f) )
	ROM_LOAD( "c_5at_g.bin",  0x3000, 0x1000, CRC(b9005ac0) SHA1(3fe3599f6fa7c496f782053ddf7bacb453d197c4) )
	/* space for diagnostic ROM */

	ROM_REGION( 0x10000, "braze", 0 )
	ROM_LOAD( "d2k11.bin",    0x00000, 0x10000, CRC(2048fc42) SHA1(e427a09ed8e792ee8ce01cd0b07c6a0d5a7c5536) ) /* Version 1.1 */

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "s_3i_b.bin",   0x0000, 0x0800, CRC(45a4ed06) SHA1(144d24464c1f9f01894eb12f846952290e6e32ef) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_LOAD( "s_3j_b.bin",   0x1000, 0x0800, CRC(4743fe92) SHA1(6c82b57637c0212a580591397e6a5a1718f19fd2) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "v_5h_b.bin",   0x0000, 0x0800, CRC(12c8c95d) SHA1(a57ff5a231c45252a63b354137c920a1379b70a3) )
	ROM_LOAD( "v_3pt.bin",    0x0800, 0x0800, CRC(15e9c5e9) SHA1(976eb1e18c74018193a35aa86cff482ebfc5cc4e) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "l_4m_b.bin",   0x0000, 0x0800, CRC(59f8054d) SHA1(793dba9bf5a5fe76328acdfb90815c243d2a65f1) )
	ROM_LOAD( "l_4n_b.bin",   0x0800, 0x0800, CRC(672e4714) SHA1(92e5d379f4838ac1fa44d448ce7d142dae42102f) )
	ROM_LOAD( "l_4r_b.bin",   0x1000, 0x0800, CRC(feaa59ee) SHA1(ecf95db5a20098804fc8bd59232c66e2e0ed3db4) )
	ROM_LOAD( "l_4s_b.bin",   0x1800, 0x0800, CRC(20f2ef7e) SHA1(3bc482a38bf579033f50082748ee95205b0f673d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2k.bpr",     0x0000, 0x0100, CRC(e273ede5) SHA1(b50ec9e1837c00c20fb2a4369ec7dd0358321127) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2j.bpr",     0x0100, 0x0100, CRC(d6412358) SHA1(f9c872da2fe8e800574ae3bf483fb3ccacc92eb3) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-5e.bpr",     0x0200, 0x0100, CRC(b869b8f5) SHA1(c2bdccbf2654b64ea55cd589fd21323a9178a660) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongjr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "djr1-c_5b_f-2.5b", 0x0000, 0x1000, CRC(dea28158) SHA1(08baf84ae6f9b40a2c743fe1d8c158c74a40e95a) )
	ROM_CONTINUE(                 0x3000, 0x1000 )
	ROM_LOAD( "djr1-c_5c_f-2.5c", 0x2000, 0x0800, CRC(6fb5faf6) SHA1(ce1cfde71a9e2a8b5896a6301d386f72869a1d2e) )
	ROM_CONTINUE(                 0x4800, 0x0800 )
	ROM_CONTINUE(                 0x1000, 0x0800 )
	ROM_CONTINUE(                 0x5800, 0x0800 )
	ROM_LOAD( "djr1-c_5e_f-2.5e", 0x4000, 0x0800, CRC(d042b6a8) SHA1(57ac237d273496b44220b4437118115ef11dbd9f) )
	ROM_CONTINUE(                 0x2800, 0x0800 )
	ROM_CONTINUE(                 0x5000, 0x0800 )
	ROM_CONTINUE(                 0x1800, 0x0800 )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "djr1-c_3h.3h",     0x0000, 0x1000, CRC(715da5f8) SHA1(f708c3fd374da65cbd9fe2e191152f5d865414a0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "djr1-v.3n",        0x0000, 0x1000, CRC(8d51aca9) SHA1(64887564b079d98e98aafa53835e398f34fe4e3f) )
	ROM_LOAD( "djr1-v.3p",        0x1000, 0x1000, CRC(4ef64ba5) SHA1(41a7a4005087951f57f62c9751d62a8c495e6bb3) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "djr1-v_7c.7c",     0x0000, 0x0800, CRC(dc7f4164) SHA1(07a6242e95b5c3b8dfdcd4b4950f463dba16dd77) )
	ROM_LOAD( "djr1-v_7d.7d",     0x0800, 0x0800, CRC(0ce7dcf6) SHA1(0654b77526c49f0dfa077ac4f1f69cf5cb2e2f64) )
	ROM_LOAD( "djr1-v_7e.7e",     0x1000, 0x0800, CRC(24d1ff17) SHA1(696854bf3dc5447d33b4815db357e6ce3834d867) )
	ROM_LOAD( "djr1-v_7f.7f",     0x1800, 0x0800, CRC(0f8c083f) SHA1(0b688ae9da296b2447fffa5e135fd6a56ec3e790) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "djr1-c-2e.2e",     0x0000, 0x0100, CRC(463dc7ad) SHA1(b2c9f22facc8885be2d953b056eb8dcddd4f34cb) )   /* palette low 4 bits (inverted) */
	ROM_LOAD( "djr1-c-2f.2f",     0x0100, 0x0100, CRC(47ba0042) SHA1(dbec3f4b8013628c5b8f83162e5f8b1f82f6ee5f) )   /* palette high 4 bits (inverted) */
	ROM_LOAD( "djr1-v-2n.2n",     0x0200, 0x0100, CRC(dbf185bf) SHA1(2697a991a4afdf079dd0b7e732f71c7618f43b70) )   /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongjrj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "c_5ba.bin",    0x0000, 0x1000, CRC(50a015ce) SHA1(edcafdf8f989dd25bb142817084d270a6942577a) )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_LOAD( "c_5ca.bin",    0x2000, 0x0800, CRC(c0a18f0d) SHA1(6d7396b98c0a7fa508dc233f90e5a8359439c97b) )
	ROM_CONTINUE(             0x4800, 0x0800 )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_CONTINUE(             0x5800, 0x0800 )
	ROM_LOAD( "c_5ea.bin",    0x4000, 0x0800, CRC(a81dd00c) SHA1(ec507d963151bb8fcee13a47d7f93aa4cd089b7e) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_CONTINUE(             0x5000, 0x0800 )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "c_3h.bin",     0x0000, 0x1000, CRC(715da5f8) SHA1(f708c3fd374da65cbd9fe2e191152f5d865414a0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "v_3na.bin",    0x0000, 0x1000, CRC(a95c4c63) SHA1(75e312b6872958f3bfc7bafd0743efdf7a74e8f0) )
	ROM_LOAD( "v_3pa.bin",    0x1000, 0x1000, CRC(4974ffef) SHA1(7bb1e207dd3c5214e405bf32c57ec1b048061050) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "v_7c.bin",     0x0000, 0x0800, CRC(dc7f4164) SHA1(07a6242e95b5c3b8dfdcd4b4950f463dba16dd77) )
	ROM_LOAD( "v_7d.bin",     0x0800, 0x0800, CRC(0ce7dcf6) SHA1(0654b77526c49f0dfa077ac4f1f69cf5cb2e2f64) )
	ROM_LOAD( "v_7e.bin",     0x1000, 0x0800, CRC(24d1ff17) SHA1(696854bf3dc5447d33b4815db357e6ce3834d867) )
	ROM_LOAD( "v_7f.bin",     0x1800, 0x0800, CRC(0f8c083f) SHA1(0b688ae9da296b2447fffa5e135fd6a56ec3e790) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2e.bpr",  0x0000, 0x0100, CRC(463dc7ad) SHA1(b2c9f22facc8885be2d953b056eb8dcddd4f34cb) )   /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2f.bpr",  0x0100, 0x0100, CRC(47ba0042) SHA1(dbec3f4b8013628c5b8f83162e5f8b1f82f6ee5f) )   /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-2n.bpr",  0x0200, 0x0100, CRC(dbf185bf) SHA1(2697a991a4afdf079dd0b7e732f71c7618f43b70) )   /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongjnrj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dkjp.5b",      0x0000, 0x1000, CRC(7b48870b) SHA1(4f737559e2bf5cc28824220417d7a2827361221f) )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_LOAD( "dkjp.5c",      0x2000, 0x0800, CRC(12391665) SHA1(3141ed5096097c48ac128636330ab6837a665d40) )
	ROM_CONTINUE(             0x4800, 0x0800 )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_CONTINUE(             0x5800, 0x0800 )
	ROM_LOAD( "dkjp.5e",      0x4000, 0x0800, CRC(6c9f9103) SHA1(2d595e13c4ecb74b18e92b00efcc90c1e841b478) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_CONTINUE(             0x5000, 0x0800 )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "c_3h.bin",       0x0000, 0x1000, CRC(715da5f8) SHA1(f708c3fd374da65cbd9fe2e191152f5d865414a0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "dkj.3n",       0x0000, 0x1000, CRC(8d51aca9) SHA1(64887564b079d98e98aafa53835e398f34fe4e3f) )
	ROM_LOAD( "dkj.3p",       0x1000, 0x1000, CRC(4ef64ba5) SHA1(41a7a4005087951f57f62c9751d62a8c495e6bb3) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "v_7c.bin",     0x0000, 0x0800, CRC(dc7f4164) SHA1(07a6242e95b5c3b8dfdcd4b4950f463dba16dd77) )
	ROM_LOAD( "v_7d.bin",     0x0800, 0x0800, CRC(0ce7dcf6) SHA1(0654b77526c49f0dfa077ac4f1f69cf5cb2e2f64) )
	ROM_LOAD( "v_7e.bin",     0x1000, 0x0800, CRC(24d1ff17) SHA1(696854bf3dc5447d33b4815db357e6ce3834d867) )
	ROM_LOAD( "v_7f.bin",     0x1800, 0x0800, CRC(0f8c083f) SHA1(0b688ae9da296b2447fffa5e135fd6a56ec3e790) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2e.bpr",  0x0000, 0x0100, CRC(463dc7ad) SHA1(b2c9f22facc8885be2d953b056eb8dcddd4f34cb) )   /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2f.bpr",  0x0100, 0x0100, CRC(47ba0042) SHA1(dbec3f4b8013628c5b8f83162e5f8b1f82f6ee5f) )   /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-2n.bpr",  0x0200, 0x0100, CRC(dbf185bf) SHA1(2697a991a4afdf079dd0b7e732f71c7618f43b70) )   /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongjre )  /* Confirmed E-Kit set mentioned in Nintendo Service Department Bulletin # DJR-03 (02-23-83) */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "djr1-c.5b",    0x0000, 0x1000, CRC(ffe9e1a5) SHA1(715dc79d85169b4c1faf43458592e69b434afefd) )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_LOAD( "djr1-c.5c",    0x2000, 0x0800, CRC(982e30e8) SHA1(4d93d79e6ab1cad678af509cb3be4166b239bfa6) )
	ROM_CONTINUE(             0x4800, 0x0800 )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_CONTINUE(             0x5800, 0x0800 )
	ROM_LOAD( "djr1-c.5e",    0x4000, 0x0800, CRC(24c3d325) SHA1(98b0354cddf2cb5e21a3aa8387b86e8606e51d55) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_CONTINUE(             0x5000, 0x0800 )
	ROM_CONTINUE(             0x1800, 0x0800 )
	ROM_LOAD( "djr1-c.5a",    0x8000, 0x1000, CRC(bb5f5180) SHA1(1ef6236b7204432cfd17c689760943ab603c6fb7) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "c_3h.bin",       0x0000, 0x1000, CRC(715da5f8) SHA1(f708c3fd374da65cbd9fe2e191152f5d865414a0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "dkj.3n",       0x0000, 0x1000, CRC(8d51aca9) SHA1(64887564b079d98e98aafa53835e398f34fe4e3f) )
	ROM_LOAD( "dkj.3p",       0x1000, 0x1000, CRC(4ef64ba5) SHA1(41a7a4005087951f57f62c9751d62a8c495e6bb3) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "v_7c.bin",     0x0000, 0x0800, CRC(dc7f4164) SHA1(07a6242e95b5c3b8dfdcd4b4950f463dba16dd77) )
	ROM_LOAD( "v_7d.bin",     0x0800, 0x0800, CRC(0ce7dcf6) SHA1(0654b77526c49f0dfa077ac4f1f69cf5cb2e2f64) )
	ROM_LOAD( "v_7e.bin",     0x1000, 0x0800, CRC(24d1ff17) SHA1(696854bf3dc5447d33b4815db357e6ce3834d867) )
	ROM_LOAD( "v_7f.bin",     0x1800, 0x0800, CRC(0f8c083f) SHA1(0b688ae9da296b2447fffa5e135fd6a56ec3e790) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2e.bpr",  0x0000, 0x0100, CRC(463dc7ad) SHA1(b2c9f22facc8885be2d953b056eb8dcddd4f34cb) )   /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2f.bpr",  0x0100, 0x0100, CRC(47ba0042) SHA1(dbec3f4b8013628c5b8f83162e5f8b1f82f6ee5f) )   /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-2n.bpr",  0x0200, 0x0100, CRC(dbf185bf) SHA1(2697a991a4afdf079dd0b7e732f71c7618f43b70) )   /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongjrpb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dkjr1-c.5b-p", 0x0000, 0x1000, CRC(8d99b3e0) SHA1(311a9f353e62d9d07c678e45baa2efec575a8f3b) ) // does not match SUM16 of bulletin (see notes), definitely not from Nintendo
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_LOAD( "dkjr1-c.5c-p", 0x2000, 0x0800, CRC(b92d258c) SHA1(793483e249d08cbbbefe06d3ddc4c2eda5428ee8) )
	ROM_CONTINUE(             0x4800, 0x0800 )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_CONTINUE(             0x5800, 0x0800 )
	ROM_LOAD( "dkjr1-c.5e",   0x4000, 0x0800, CRC(d042b6a8) SHA1(57ac237d273496b44220b4437118115ef11dbd9f) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_CONTINUE(             0x5000, 0x0800 )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "c_3h.bin",       0x0000, 0x1000, CRC(715da5f8) SHA1(f708c3fd374da65cbd9fe2e191152f5d865414a0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "dkj.3n",       0x0000, 0x1000, CRC(8d51aca9) SHA1(64887564b079d98e98aafa53835e398f34fe4e3f) )
	ROM_LOAD( "dkj.3p",       0x1000, 0x1000, CRC(4ef64ba5) SHA1(41a7a4005087951f57f62c9751d62a8c495e6bb3) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "v_7c.bin",     0x0000, 0x0800, CRC(dc7f4164) SHA1(07a6242e95b5c3b8dfdcd4b4950f463dba16dd77) )
	ROM_LOAD( "v_7d.bin",     0x0800, 0x0800, CRC(0ce7dcf6) SHA1(0654b77526c49f0dfa077ac4f1f69cf5cb2e2f64) )
	ROM_LOAD( "v_7e.bin",     0x1000, 0x0800, CRC(24d1ff17) SHA1(696854bf3dc5447d33b4815db357e6ce3834d867) )
	ROM_LOAD( "v_7f.bin",     0x1800, 0x0800, CRC(0f8c083f) SHA1(0b688ae9da296b2447fffa5e135fd6a56ec3e790) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2e.bpr",  0x0000, 0x0100, CRC(463dc7ad) SHA1(b2c9f22facc8885be2d953b056eb8dcddd4f34cb) )   /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2f.bpr",  0x0100, 0x0100, CRC(47ba0042) SHA1(dbec3f4b8013628c5b8f83162e5f8b1f82f6ee5f) )   /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-2n.bpr",  0x0200, 0x0100, CRC(dbf185bf) SHA1(2697a991a4afdf079dd0b7e732f71c7618f43b70) )   /* character color codes on a per-column basis */
ROM_END

ROM_START( dkongjrb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dkjr1",        0x0000, 0x1000, CRC(ec7e097f) SHA1(c10885d8724434030094a106c5b6de7fa6976d0f) )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_LOAD( "c_5ca.bin",    0x2000, 0x0800, CRC(c0a18f0d) SHA1(6d7396b98c0a7fa508dc233f90e5a8359439c97b) )
	ROM_CONTINUE(             0x4800, 0x0800 )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_CONTINUE(             0x5800, 0x0800 )
	ROM_LOAD( "c_5ea.bin",    0x4000, 0x0800, CRC(a81dd00c) SHA1(ec507d963151bb8fcee13a47d7f93aa4cd089b7e) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_CONTINUE(             0x5000, 0x0800 )
	ROM_CONTINUE(             0x1800, 0x0800 )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "c_3h.bin",       0x0000, 0x1000, CRC(715da5f8) SHA1(f708c3fd374da65cbd9fe2e191152f5d865414a0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "v_3na.bin",    0x0000, 0x1000, CRC(a95c4c63) SHA1(75e312b6872958f3bfc7bafd0743efdf7a74e8f0) )
	ROM_LOAD( "dkjr10",       0x1000, 0x1000, CRC(adc11322) SHA1(01c13213e413c269cf8d9e391209b32b18747c8d) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "v_7c.bin",     0x0000, 0x0800, CRC(dc7f4164) SHA1(07a6242e95b5c3b8dfdcd4b4950f463dba16dd77) )
	ROM_LOAD( "v_7d.bin",     0x0800, 0x0800, CRC(0ce7dcf6) SHA1(0654b77526c49f0dfa077ac4f1f69cf5cb2e2f64) )
	ROM_LOAD( "v_7e.bin",     0x1000, 0x0800, CRC(24d1ff17) SHA1(696854bf3dc5447d33b4815db357e6ce3834d867) )
	ROM_LOAD( "v_7f.bin",     0x1800, 0x0800, CRC(0f8c083f) SHA1(0b688ae9da296b2447fffa5e135fd6a56ec3e790) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2e.bpr",  0x0000, 0x0100, CRC(463dc7ad) SHA1(b2c9f22facc8885be2d953b056eb8dcddd4f34cb) )   /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2f.bpr",  0x0100, 0x0100, CRC(47ba0042) SHA1(dbec3f4b8013628c5b8f83162e5f8b1f82f6ee5f) )   /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-2n.bpr",  0x0200, 0x0100, CRC(dbf185bf) SHA1(2697a991a4afdf079dd0b7e732f71c7618f43b70) )   /* character color codes on a per-column basis */
ROM_END

/* only the graphic roms differ from dkongjrb but it's a common bootleg */
ROM_START( jrking )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b5.bin",    0x0000, 0x1000, CRC(ec7e097f) SHA1(c10885d8724434030094a106c5b6de7fa6976d0f) )
	ROM_CONTINUE(          0x3000, 0x1000 )
	ROM_LOAD( "c5.bin",    0x2000, 0x0800, CRC(c0a18f0d) SHA1(6d7396b98c0a7fa508dc233f90e5a8359439c97b) )
	ROM_CONTINUE(          0x4800, 0x0800 )
	ROM_CONTINUE(          0x1000, 0x0800 )
	ROM_CONTINUE(          0x5800, 0x0800 )
	ROM_LOAD( "e5.bin",    0x4000, 0x0800, CRC(a81dd00c) SHA1(ec507d963151bb8fcee13a47d7f93aa4cd089b7e) )
	ROM_CONTINUE(          0x2800, 0x0800 )
	ROM_CONTINUE(          0x5000, 0x0800 )
	ROM_CONTINUE(          0x1800, 0x0800 )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "h3.bin",       0x0000, 0x1000, CRC(715da5f8) SHA1(f708c3fd374da65cbd9fe2e191152f5d865414a0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "n3.bin",    0x0000, 0x1000, CRC(7110715d) SHA1(6d848563bf01b347e374d5cae23e09806e28ea6d) )
	ROM_LOAD( "p3.bin",    0x1000, 0x1000, CRC(46476016) SHA1(5af29f816e53fd344d114ebf2369e29359e08ce0) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "c7.bin",     0x0000, 0x1000, CRC(9f531527) SHA1(6b426cea91135ae1924506f964d856fc475c08e1) )
	ROM_LOAD( "d7.bin",     0x1000, 0x1000, CRC(32fbd41b) SHA1(1f98facc2c20dc4b8722085f269f123601382994) )
	ROM_LOAD( "e7.bin",     0x2000, 0x1000, CRC(2286bf8e) SHA1(7511e83aa5a4de988048fc2db4de7b3d5aabee8c) )
	ROM_LOAD( "f7.bin",     0x3000, 0x1000, CRC(627007a0) SHA1(f71091b65978f7f386ba3c30a3ffe824412bbe67) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "c-2e.bpr",  0x0000, 0x0100, CRC(463dc7ad) SHA1(b2c9f22facc8885be2d953b056eb8dcddd4f34cb) )   /* palette low 4 bits (inverted) */
	ROM_LOAD( "c-2f.bpr",  0x0100, 0x0100, CRC(47ba0042) SHA1(dbec3f4b8013628c5b8f83162e5f8b1f82f6ee5f) )   /* palette high 4 bits (inverted) */
	ROM_LOAD( "v-2n.bpr",  0x0200, 0x0100, CRC(dbf185bf) SHA1(2697a991a4afdf079dd0b7e732f71c7618f43b70) )   /* character color codes on a per-column basis */
ROM_END

/*
Donkey King Jr. PCB Layout

DJR-00
|-------------------------------------------------|
|DSW(8)                  MB7052.6B        12.263MHz
|                                                 |
|                    2114             6148        |
|  2114 Z80A         2114             6148        |
|  2114                                           |
|1 2114 D8257            ROM6              N82S09 |
|8 2114       MB7051.8J  ROM5                     |
|W 2114  MB7052.9K   ROM1                         |
|A 2114  MB7052.9L   ROM2    MC10124              |
|Y                   ROM3    MC10124              |
|                    ROM4    MC10125              |
|                    HM10422 MC10125              |
|6MHZ    I8035       HM10422                ROM10 |
|                            MC10124        ROM9  |
|HA1368                      MC10124        ROM8  |
|    VOL             V-POS  H-POS           ROM7  |
|-------------------------------------------------|
Notes:
 Z80 clock   - 3.0659 MHz
 D8257 clock - 3.0659 MHz
 8035 clock  - 6.000 MHz
 N82S09      - 576-bit BIPOLAR RAM (64 X9)
 6148        - 1024-bit x4 SRAM
 2114        - 1024-bit x4 SRAM
 MC10124     - Quad TTL to MECL Translator
 MC10125     - Quad MECL to TTL Translator
 HM10422     - 256 x 4 ECL RAM
 V-POS/H-POS - Pot to adjust horizontal/vertical screen position
 Vsync       - 60.4862Hz
 HSync       - 15.848kHz
 Xtal 1      - 12.263 MHz
 Xtal 2      - 6.000 MHz
*/

ROM_START( dkingjr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.7g",       0x0000, 0x1000, CRC(bd07bb8d) SHA1(a6d18f993c0469ad5db5bd546afab9a45677643a) )
	ROM_CONTINUE(           0x3000, 0x1000 )
	ROM_LOAD( "2.7h",       0x2000, 0x0800, CRC(01fbec11) SHA1(cf1aa88529c6c266ee5e924f83fd49d4a2766557) )
	ROM_CONTINUE(           0x4800, 0x0800 )
	ROM_CONTINUE(           0x1000, 0x0800 )
	ROM_CONTINUE(           0x5800, 0x0800 )
	ROM_LOAD( "3.7k",       0x4000, 0x0800, CRC(a81dd00c) SHA1(ec507d963151bb8fcee13a47d7f93aa4cd089b7e) )
	ROM_CONTINUE(           0x2800, 0x0800 )
	ROM_CONTINUE(           0x5000, 0x0800 )
	ROM_CONTINUE(           0x1800, 0x0800 )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "4.7l",       0x0000, 0x1000, CRC(715da5f8) SHA1(f708c3fd374da65cbd9fe2e191152f5d865414a0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5.6g",       0x0000, 0x1000, CRC(cf14669d) SHA1(1ab9ceba49bff6d7bd00c89dae7018093e860eeb) )
	ROM_LOAD( "6.6e",       0x1000, 0x1000, CRC(cefed15e) SHA1(7077951a3d239b8b34eb45ab959228cb93a957c5) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "7.2t",       0x0000, 0x0800, CRC(dc7f4164) SHA1(07a6242e95b5c3b8dfdcd4b4950f463dba16dd77) )
	ROM_LOAD( "8.2r",       0x0800, 0x0800, CRC(0ce7dcf6) SHA1(0654b77526c49f0dfa077ac4f1f69cf5cb2e2f64) )
	ROM_LOAD( "9.2p",       0x1000, 0x0800, CRC(24d1ff17) SHA1(696854bf3dc5447d33b4815db357e6ce3834d867) )
	ROM_LOAD( "10.2m",      0x1800, 0x0800, CRC(0f8c083f) SHA1(0b688ae9da296b2447fffa5e135fd6a56ec3e790) )

	ROM_REGION( 0x0320, "proms", 0 )
	ROM_LOAD( "mb7052.9k",  0x0000, 0x0100, CRC(49f2d444) SHA1(6995d73222f71f880ab3ce6d54577802a6ef53ab) )   /* palette low 4 bits */
	ROM_LOAD( "mb7052.9l",  0x0100, 0x0100, CRC(487513ab) SHA1(e686021bbd41ea8c9d1fd3a277333173ba50afdd) )   /* palette high 4 bits */
	ROM_LOAD( "mb7052.6b",  0x0200, 0x0100, CRC(dbf185bf) SHA1(2697a991a4afdf079dd0b7e732f71c7618f43b70) )   /* character color codes on a per-column basis */
	ROM_LOAD( "mb7051.8j",  0x0300, 0x0020, CRC(a5a6f2ca) SHA1(5507fb6f5c8845c4421c2996e9f76c818d987623) )   /* unknown */
ROM_END

ROM_START( pestplce )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pest.1p",      0x0000, 0x1000, CRC(80d50721) SHA1(9c0e7571b1664dce741595a2d13dc9d7709b35a9) )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_LOAD( "pest.2p",      0x2000, 0x0800, CRC(9c3681cc) SHA1(c12e8e7ab79c9fde92cca2c589904f68cf52cbf1) )
	ROM_CONTINUE(             0x4800, 0x0800 )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_CONTINUE(             0x5800, 0x0800 )
	ROM_LOAD( "pest.3p",      0x4000, 0x0800, CRC(49853922) SHA1(1e8a29fdb1af52a39c07ef214f5e7c2d56b35ea5) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_CONTINUE(             0x5000, 0x0800 )
	ROM_CONTINUE(             0x1800, 0x0800 )
	ROM_LOAD( "pest.0",       0xb000, 0x1000, CRC(28952b56) SHA1(fa8abe594a88a61e85f074d03822d7e0dcd52fb2) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "pest.4",       0x0000, 0x1000, CRC(715da5f8) SHA1(f708c3fd374da65cbd9fe2e191152f5d865414a0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pest.o",       0x0000, 0x1000, CRC(03939ece) SHA1(a776558eba2f8a2bc16933555d41a4532b627bff) )
	ROM_LOAD( "pest.k",       0x1000, 0x1000, CRC(2acacedf) SHA1(f91863f46aeb8986226b0b0854bac00217d6e7cf) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "pest.b",        0x0000, 0x1000, CRC(e760073e) SHA1(917e74a4efa62b7404a03f094f3f4047dda8feda) )
	ROM_LOAD( "pest.a",        0x1000, 0x1000, CRC(1958346e) SHA1(c4053dafc904b5e202e4a1acc48dd3e22db05c74) )
	ROM_LOAD( "pest.d",        0x2000, 0x1000, CRC(3a993c17) SHA1(af7048576aa3185b051518663693802ec9014a74) )
	ROM_LOAD( "pest.c",        0x3000, 0x1000, CRC(bf08f2a3) SHA1(c755f7463ac46054c65248d91b8e8da9cd379bf5) )

	/* not standard dkong layout */
	ROM_REGION( 0x0300, "proms", ROMREGION_INVERT )
	ROM_LOAD( "n82s129a.bin",  0x0000, 0x0100, CRC(0330f35f) SHA1(5bd50cdd738b258dd3cfcd0e1dd8d37c927edc4b) )
	ROM_LOAD( "n82s129b.bin",  0x0100, 0x0100, CRC(ba88311b) SHA1(b4388ebd3984bdb966d850cfb7d34c3ebce230b7) )
	ROM_LOAD( "sn74s288n.bin", 0x0200, 0x0020, CRC(a5a6f2ca) SHA1(5507fb6f5c8845c4421c2996e9f76c818d987623) )
ROM_END

ROM_START( dkong3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dk3c.7b",      0x0000, 0x2000, CRC(38d5f38e) SHA1(5a6bb0e5070211515e3d56bd7d4c2d1655ac1621) )
	ROM_LOAD( "dk3c.7c",      0x2000, 0x2000, CRC(c9134379) SHA1(ecddb3694b93cb3dc98c3b1aeeee928e27529aba) )
	ROM_LOAD( "dk3c.7d",      0x4000, 0x2000, CRC(d22e2921) SHA1(59a4a1a36aaca19ee0a7255d832df9d042ba34fb) )
	ROM_LOAD( "dk3c.7e",      0x8000, 0x2000, CRC(615f14b7) SHA1(145674073e95d97c9131b6f2b03303eadb57ca78) )

	ROM_REGION( 0x10000, "n2a03a", 0 )  /* sound #1 */
	ROM_LOAD( "dk3c.5l",      0xe000, 0x2000, CRC(7ff88885) SHA1(d530581778aab260e21f04c38e57ba34edea7c64) )

	ROM_REGION( 0x10000, "n2a03b", 0 )  /* sound #2 */
	ROM_LOAD( "dk3c.6h",      0xe000, 0x2000, CRC(36d7200c) SHA1(7965fcb9bc1c0fdcae8a8e79df9c7b7439c506d8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "dk3v.3n",      0x0000, 0x1000, CRC(415a99c7) SHA1(e0855b03bb1dc0d8ae46da9fe33ca30ecf6a2e96) )
	ROM_LOAD( "dk3v.3p",      0x1000, 0x1000, CRC(25744ea0) SHA1(4866e43e80b010ccf2c8cc94c232786521f9e26e) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "dk3v.7c",      0x0000, 0x1000, CRC(8ffa1737) SHA1(fa5896124227d412fbdf83f129ddffa32cf2053b) )
	ROM_LOAD( "dk3v.7d",      0x1000, 0x1000, CRC(9ac84686) SHA1(a089376b9c23094490703152ad98ed27f519402d) )
	ROM_LOAD( "dk3v.7e",      0x2000, 0x1000, CRC(0c0af3fb) SHA1(03e0c3f51bc3c20f95cb02f76f2d80188d5dbe36) )
	ROM_LOAD( "dk3v.7f",      0x3000, 0x1000, CRC(55c58662) SHA1(7f3d5a1b386cc37d466e42392ffefc928666a8dc) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "dkc1-c.1d",    0x0000, 0x0200, CRC(df54befc) SHA1(7912dbf0a0c8ef68f4ae0f95e55ab164da80e4a1) ) /* palette red & green component */
	ROM_LOAD( "dkc1-c.1c",    0x0200, 0x0200, CRC(66a77f40) SHA1(c408d65990f0edd78c4590c447426f383fcd2d88) ) /* palette blue component */
	ROM_LOAD( "dkc1-v.2n",    0x0400, 0x0100, CRC(50e33434) SHA1(b63da9bed9dc4c7da78e4c26d4ba14b65f2b7e72) ) /* character color codes on a per-column basis */

	ROM_REGION( 0x0020, "adrdecode", 0 )
	/* address decode prom 18s030 - this has inverted outputs. The dump does not reflect this. */
	ROM_LOAD( "dkc1-v.5e",    0x0000, 0x0020, CRC(d3e2eaf8) SHA1(87bb298137c26570dafb4ac495c87e82441e70e5) )
ROM_END

ROM_START( dkong3j )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dk3c.7b",      0x0000, 0x2000, CRC(38d5f38e) SHA1(5a6bb0e5070211515e3d56bd7d4c2d1655ac1621) )
	ROM_LOAD( "dk3c.7c",      0x2000, 0x2000, CRC(c9134379) SHA1(ecddb3694b93cb3dc98c3b1aeeee928e27529aba) )
	ROM_LOAD( "dk3c.7d",      0x4000, 0x2000, CRC(d22e2921) SHA1(59a4a1a36aaca19ee0a7255d832df9d042ba34fb) )
	ROM_LOAD( "dk3cj.7e",     0x8000, 0x2000, CRC(25b5be23) SHA1(43cf2a676922e60d9d637777a7721ab7582129fc) )

	ROM_REGION( 0x10000, "n2a03a", 0 )  /* sound #1 */
	ROM_LOAD( "dk3c.5l",      0xe000, 0x2000, CRC(7ff88885) SHA1(d530581778aab260e21f04c38e57ba34edea7c64) )

	ROM_REGION( 0x10000, "n2a03b", 0 )  /* sound #2 */
	ROM_LOAD( "dk3c.6h",      0xe000, 0x2000, CRC(36d7200c) SHA1(7965fcb9bc1c0fdcae8a8e79df9c7b7439c506d8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "dk3v.3n",      0x0000, 0x1000, CRC(415a99c7) SHA1(e0855b03bb1dc0d8ae46da9fe33ca30ecf6a2e96) )
	ROM_LOAD( "dk3v.3p",      0x1000, 0x1000, CRC(25744ea0) SHA1(4866e43e80b010ccf2c8cc94c232786521f9e26e) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "dk3v.7c",      0x0000, 0x1000, CRC(8ffa1737) SHA1(fa5896124227d412fbdf83f129ddffa32cf2053b) )
	ROM_LOAD( "dk3v.7d",      0x1000, 0x1000, CRC(9ac84686) SHA1(a089376b9c23094490703152ad98ed27f519402d) )
	ROM_LOAD( "dk3v.7e",      0x2000, 0x1000, CRC(0c0af3fb) SHA1(03e0c3f51bc3c20f95cb02f76f2d80188d5dbe36) )
	ROM_LOAD( "dk3v.7f",      0x3000, 0x1000, CRC(55c58662) SHA1(7f3d5a1b386cc37d466e42392ffefc928666a8dc) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "dkc1-c.1d",    0x0000, 0x0200, CRC(df54befc) SHA1(7912dbf0a0c8ef68f4ae0f95e55ab164da80e4a1) ) /* palette red & green component */
	ROM_LOAD( "dkc1-c.1c",    0x0200, 0x0200, CRC(66a77f40) SHA1(c408d65990f0edd78c4590c447426f383fcd2d88) ) /* palette blue component */
	ROM_LOAD( "dkc1-v.2n",    0x0400, 0x0100, CRC(50e33434) SHA1(b63da9bed9dc4c7da78e4c26d4ba14b65f2b7e72) )    /* character color codes on a per-column basis */

	ROM_REGION( 0x0020, "adrdecode", 0 )
	/* address decode prom 18s030 - this has inverted outputs. The dump does not reflect this. */
	ROM_LOAD( "dkc1-v.5e",    0x0000, 0x0020, CRC(d3e2eaf8) SHA1(87bb298137c26570dafb4ac495c87e82441e70e5) )
ROM_END

ROM_START( dkong3b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5b.bin",       0x0000, 0x1000, CRC(549979bc) SHA1(58532f39285db0b081089e54a23041d83bec49aa) )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_LOAD( "5c-2.bin",     0x2000, 0x0800, CRC(b9dcbae6) SHA1(a7a7a3d79cb1eed93e54dff508c61cbc24797007) )
	ROM_CONTINUE(             0x4800, 0x0800 )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_CONTINUE(             0x5800, 0x0800 )
	ROM_LOAD( "5e-2.bin",     0x4000, 0x0800, CRC(5a61868f) SHA1(25c57969c1fbf457d223c4186ed291a1e0f75e14) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_CONTINUE(             0x5000, 0x0800 )
	ROM_CONTINUE(             0x1800, 0x0800 )
	ROM_LOAD( "5c-1.bin",     0x9000, 0x1000, CRC(77a012d6) SHA1(334ae2c213acd50eda71b4102d0803bc596973ec) )
	ROM_LOAD( "5e-1.bin",     0xd000, 0x1000, CRC(745ed767) SHA1(32f4678f3eea9dc88f4c99509719a42292d6833a) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "3h.bin",       0x0000, 0x1000, CRC(715da5f8) SHA1(f708c3fd374da65cbd9fe2e191152f5d865414a0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "3n.bin",       0x0000, 0x1000, CRC(fed67d35) SHA1(472a378abff96a76aac0e2da06c8d7c3ce172b60) )
	ROM_LOAD( "3p.bin",       0x1000, 0x1000, CRC(3d1b87ce) SHA1(e0cbeebf8dc291302ab1f039e51e9b409cced340) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "7c.bin",       0x0000, 0x1000, CRC(8ffa1737) SHA1(fa5896124227d412fbdf83f129ddffa32cf2053b) )
	ROM_LOAD( "7d.bin",       0x1000, 0x1000, CRC(9ac84686) SHA1(a089376b9c23094490703152ad98ed27f519402d) )
	ROM_LOAD( "7e.bin",       0x2000, 0x1000, CRC(0c0af3fb) SHA1(03e0c3f51bc3c20f95cb02f76f2d80188d5dbe36) )
	ROM_LOAD( "7f.bin",       0x3000, 0x1000, CRC(55c58662) SHA1(7f3d5a1b386cc37d466e42392ffefc928666a8dc) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "dk3b-c.1d",    0x0000, 0x0200, CRC(df54befc) SHA1(7912dbf0a0c8ef68f4ae0f95e55ab164da80e4a1) ) /* palette red & green component */
	ROM_LOAD( "dk3b-c.1c",    0x0200, 0x0200, CRC(66a77f40) SHA1(c408d65990f0edd78c4590c447426f383fcd2d88) ) /* palette blue component */
	ROM_LOAD( "dk3b-v.2n",    0x0400, 0x0100, CRC(50e33434) SHA1(b63da9bed9dc4c7da78e4c26d4ba14b65f2b7e72) ) /* character color codes on a per-column basis */
ROM_END

ROM_START( hunchbkd )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "hb.5e",        0x0000, 0x1000, CRC(4c3ac070) SHA1(636843b33f1b7e994b112fa29e65038098528b8c) )
	ROM_LOAD( "hbsc-1.5c",    0x2000, 0x1000, CRC(9b0e6234) SHA1(a7405451e5cd42bc276c659ec5a2136dbb7b6aba) )
	ROM_LOAD( "hb.5b",        0x4000, 0x1000, CRC(4cde80f3) SHA1(3d93d8e454b2c517971a99c5700b6e943f975a11) )
	ROM_LOAD( "hb.5a",        0x6000, 0x1000, CRC(d60ef5b2) SHA1(b2b5528cb837d58ef632d7670820ad8b07e5af1b) )

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "hb.3h",        0x0000, 0x0800, CRC(a3c240d4) SHA1(8cb6057ca617909c73b09988ba65a1176696cb5d) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_FILL(                 0x1000, 0x0800, 0xFF )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "hb.3n",        0x0000, 0x0800, CRC(443ed5ac) SHA1(febed689e03abf25452aab6eff85ea01883e929c) )
	ROM_LOAD( "hb.3p",        0x0800, 0x0800, CRC(073e7b0c) SHA1(659cd3b1827bf6b7f0c9bef3cd83e69c2b2193ff) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "hb.7c",        0x0000, 0x0800, CRC(3ba71686) SHA1(34c2ceadea1026de6157df1e7a1c2f6b86fd3c82) )
	ROM_LOAD( "hb.7d",        0x0800, 0x0800, CRC(5786948d) SHA1(7e8bc953195cc9a07a8429b547e1fab6cd487b51) )
	ROM_LOAD( "hb.7e",        0x1000, 0x0800, CRC(f845e8ca) SHA1(4bedbbc74a637f6d60b3b2dbf41efc7390ee9091) )
	ROM_LOAD( "hb.7f",        0x1800, 0x0800, CRC(52d20fea) SHA1(e3825f75f312d1e256f78a89098e328e8f307577) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "hbprom.2e",    0x0000, 0x0100, CRC(37aab98f) SHA1(0b002ab82158854bdd4a9db05eee037711017313) )    /* palette low 4 bits (inverted) */
	ROM_LOAD( "hbprom.2f",    0x0100, 0x0100, CRC(845b8dcc) SHA1(eebd0c024172e54b509f1f99d9159438d5f3a905) )    /* palette high 4 bits (inverted) */
	ROM_LOAD( "hbprom.2n",    0x0200, 0x0100, CRC(dff9070a) SHA1(307b95749343b5106247d842f773b2b445faa156) )    /* character color codes on a per-column basis */

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "82s147.prm",   0x0000, 0x0200, CRC(46e5bc92) SHA1(f4171f8650818c017d58ad7131a7aff100b1b99c) )    /* address mapping */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pls153h.bin", 0x0000, 0x00eb, NO_DUMP ) /* missing - pls153 ??? */
ROM_END

ROM_START( sbdk )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "sb-dk.ap",     0x0000, 0x1000, CRC(fef0ef9c) SHA1(8d3de7f96354672d906b2e124f3fb355f3201ed2) )
	ROM_LOAD( "sb-dk.ay",     0x2000, 0x1000, CRC(2e9dade2) SHA1(74e5770fd362fd0242b8174b0ea5383fdf893cb3) )
	ROM_LOAD( "sb-dk.as",     0x4000, 0x1000, CRC(e6d200f3) SHA1(3787334df76e629baa9ef5362495cd3af7777358) )
	ROM_LOAD( "sb-dk.5a",     0x6000, 0x1000, CRC(ca41ca56) SHA1(d862172b1cc6639d540efc140b63d1a598f75656) )

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "sb-dk.3h",     0x0000, 0x0800, CRC(13e60b6e) SHA1(f5dca15db0f1a225ff0116726bb055bb7b9655cc) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_FILL(                 0x1000, 0x0800, 0xFF )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "sb-dk.3n",     0x0000, 0x0800, CRC(b1d76b59) SHA1(aed57ec67d80abdff1a4bfc3a713fa01c0dd15a2) )
	ROM_LOAD( "sb-dk.3p",     0x0800, 0x0800, CRC(ea5f9f88) SHA1(5742d3554d967ed1e90f7c6f73dafbd302f0f244) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sb-dk.7c",     0x0000, 0x0800, CRC(c12c18f2) SHA1(77e99d80e05108ceec21ae299645139f83a389b1) )
	ROM_LOAD( "sb-dk.7d",     0x0800, 0x0800, CRC(f7a32d23) SHA1(5782e8f8744481a931c629579ae6f4fff7e2f838) )
	ROM_LOAD( "sb-dk.7e",     0x1000, 0x0800, CRC(8e48b13e) SHA1(b4589685a60a8463f656a4f5b0dedfb265c3b3e4) )
	ROM_LOAD( "sb-dk.7f",     0x1800, 0x0800, CRC(989969f3) SHA1(de641082476ac3da3872461263566dfb398ea43a) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "sb.2e",        0x0000, 0x0100, CRC(4f06f789) SHA1(0b2775dd8da1c20121639871ed291a015a34e1f6) )
	ROM_LOAD( "sb.2f",        0x0100, 0x0100, CRC(2c15b1b2) SHA1(7c80eb77ba47e2f4d889fc10663a0391d4329a1d) )
	ROM_LOAD( "sb.2n",        0x0200, 0x0100, CRC(dff9070a) SHA1(307b95749343b5106247d842f773b2b445faa156) )

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "82s147.prm",   0x0000, 0x0200, CRC(46e5bc92) SHA1(f4171f8650818c017d58ad7131a7aff100b1b99c) )    /* address mapping */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pls153h.bin", 0x0000, 0x00eb, NO_DUMP ) /* missing - pls153 ??? */
ROM_END

ROM_START( herbiedk )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "5f.cpu",        0x0000, 0x1000, CRC(c7ab3ac6) SHA1(5ef8c0ac1acd09a0f6c1536d0525cc27bb87b167) )
	ROM_LOAD( "5g.cpu",        0x2000, 0x1000, CRC(d1031aa6) SHA1(6f5eadf43f1a59333833b3ee72d8d3043ac8c899) )
	ROM_LOAD( "5h.cpu",        0x4000, 0x1000, CRC(c0daf551) SHA1(f39058fa05ad69e839e7c0281cb1fad80cfa3134) )
	ROM_LOAD( "5k.cpu",        0x6000, 0x1000, CRC(67442242) SHA1(0241281e8cc721f7fe22822f2cf168c2eed7983d) )

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "3i.snd",        0x0000, 0x0800, CRC(20e30406) SHA1(e2b9c6b731e53651d26455c2753a6dc3d5e9d066) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_FILL(                 0x1000, 0x0800, 0xFF )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "5h.vid",        0x0000, 0x0800, CRC(ea2a2547) SHA1(ec714abe43ab86ef615e1105688bf3df209c8f5f) )
	ROM_LOAD( "5k.vid",        0x0800, 0x0800, CRC(a8d421c9) SHA1(b733246d8674450ef00ed81b7d5e2ca09b3731d8) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "7c.clk",        0x0000, 0x0800, CRC(af646166) SHA1(c935051697f559fa8dea647e976d35b607c931d5) )
	ROM_LOAD( "7d.clk",        0x0800, 0x0800, CRC(d8e15832) SHA1(d11983d7a3ff71c6bc75607453080d554ae15df2) )
	ROM_LOAD( "7e.clk",        0x1000, 0x0800, CRC(2f7e65fa) SHA1(ff4d03020f9ad423fcebca28395964cb01b19b31) )
	ROM_LOAD( "7f.clk",        0x1800, 0x0800, CRC(ad32d5ae) SHA1(578e703ca07b9a0284d1c9c7f260a52e4f4dac0e) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "74s287.2k",     0x0000, 0x0100, CRC(7dc0a381) SHA1(7d974b2249392160e3b800e7113d4899c3600b7f) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "74s287.2j",     0x0100, 0x0100, CRC(0a440c00) SHA1(e3249a646cd8aa50739e09ae101e796ea3aac37a) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "74s287.vid",    0x0200, 0x0100, CRC(5a3446cc) SHA1(158de015006e6c400cb7ee758fda7ff760eb5835) ) /* character color codes on a per-column basis */

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "82s147.hh",     0x0000, 0x0200, CRC(46e5bc92) SHA1(f4171f8650818c017d58ad7131a7aff100b1b99c) )   /* address mapping */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pls153h.bin", 0x0000, 0x00eb, CRC(d6a04bcc) SHA1(ef1ed4311869d46dec95c2bfb31875c2f022da4f) )
ROM_END

ROM_START( herodk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "red-dot.rgt",  0x0c00, 0x0400, CRC(9c4af229) SHA1(8b7330457acdd8d92f2853f3e5f8c18f8991c5c9) )    /* encrypted */
	ROM_CONTINUE(             0x0800, 0x0400 )
	ROM_CONTINUE(             0x0400, 0x0400 )
	ROM_CONTINUE(             0x0000, 0x0400 )
	ROM_CONTINUE(             0x2000, 0x0e00 )
	ROM_CONTINUE(             0x6e00, 0x0200 )
	ROM_LOAD( "wht-dot.lft",  0x4000, 0x1000, CRC(c10f9235) SHA1(42dbf01e5da80cd8bdd18a27c3fbdf4cb5110d9a) )    /* encrypted */
	ROM_CONTINUE(             0x6000, 0x0e00 )
	ROM_CONTINUE(             0x2e00, 0x0200 )

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "silver.3h",    0x0000, 0x0800, CRC(67863ce9) SHA1(2b78e3d32a64cdef34afc476fed7ff0ab6a0277c) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_FILL(                 0x1000, 0x0800, 0xFF )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "pnk.3n",       0x0000, 0x0800, CRC(574dfd7a) SHA1(78bbe4ea83fdaec14ca92ceae03e8a3d0877d14b) )
	ROM_LOAD( "blk.3p",       0x0800, 0x0800, CRC(16f7c040) SHA1(d1bd1b5f3c66ac6e71637ef42962adabacd79340) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gold.7c",      0x0000, 0x0800, CRC(5f5282ed) SHA1(fcd467e404fab89addd55bd7c5c07d28551a4c8e) )
	ROM_LOAD( "orange.7d",    0x0800, 0x0800, CRC(075d99f5) SHA1(ff6f85a50179e0599b39871be1739080768fc475) )
	ROM_LOAD( "yellow.7e",    0x1000, 0x0800, CRC(f6272e96) SHA1(a9608966613aedb36cfb04f85730efed9a44d17c) )
	ROM_LOAD( "violet.7f",    0x1800, 0x0800, CRC(ca020685) SHA1(fe0d8d85c3bf244384e9c94f6a7f17db31083245) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "82s129.2e",    0x0000, 0x0100, CRC(da4b47e6) SHA1(2cfc7d489002113eb91048cc29d24831dadbfabb) )    /* palette low 4 bits (inverted) */
	ROM_LOAD( "82s129.2f",    0x0100, 0x0100, CRC(96e213a4) SHA1(38f21e7bce96fd2159aa61e64d66aa574d85873c) )    /* palette high 4 bits (inverted) */
	ROM_LOAD( "82s126.2n",    0x0200, 0x0100, CRC(37aece4b) SHA1(08dbb470644278132b8126649fe41d70e7750bee) )    /* character color codes on a per-column basis */

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "82s147.prm",  0x0000, 0x0200, BAD_DUMP CRC(46e5bc92) SHA1(f4171f8650818c017d58ad7131a7aff100b1b99c) )    /* no dump - taken from hunchbkd */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pls153h.bin", 0x0000, 0x00eb, NO_DUMP ) /* missing - pls153 ??? */
ROM_END

ROM_START( herodku )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2764.8h",      0x0c00, 0x0400, CRC(989ce053) SHA1(852401856a2d91118d1bd0b3db892b57d0ac949c) )
	ROM_CONTINUE(             0x0800, 0x0400 )
	ROM_CONTINUE(             0x0400, 0x0400 )
	ROM_CONTINUE(             0x0000, 0x0400 )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_LOAD( "2764.8f",      0x4000, 0x1000, CRC(835e0074) SHA1(187358973f595033a4745759f554a3dfd398889b) )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "2716.3h",      0x0000, 0x0800, CRC(caf57bef) SHA1(60c19c65bf312b36c68631ccea5434ad8cf0f3df) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_FILL(                 0x1000, 0x0800, 0xFF )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "pnk.3n",       0x0000, 0x0800, CRC(574dfd7a) SHA1(78bbe4ea83fdaec14ca92ceae03e8a3d0877d14b) )
	ROM_LOAD( "blk.3p",       0x0800, 0x0800, CRC(16f7c040) SHA1(d1bd1b5f3c66ac6e71637ef42962adabacd79340) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "gold.7c",      0x0000, 0x0800, CRC(5f5282ed) SHA1(fcd467e404fab89addd55bd7c5c07d28551a4c8e) )
	ROM_LOAD( "orange.7d",    0x0800, 0x0800, CRC(075d99f5) SHA1(ff6f85a50179e0599b39871be1739080768fc475) )
	ROM_LOAD( "yellow.7e",    0x1000, 0x0800, CRC(f6272e96) SHA1(a9608966613aedb36cfb04f85730efed9a44d17c) )
	ROM_LOAD( "violet.7f",    0x1800, 0x0800, CRC(ca020685) SHA1(fe0d8d85c3bf244384e9c94f6a7f17db31083245) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "82s129.2e",    0x0000, 0x0100, CRC(da4b47e6) SHA1(2cfc7d489002113eb91048cc29d24831dadbfabb) )    /* palette low 4 bits (inverted) */
	ROM_LOAD( "82s129.2f",    0x0100, 0x0100, CRC(96e213a4) SHA1(38f21e7bce96fd2159aa61e64d66aa574d85873c) )    /* palette high 4 bits (inverted) */
	ROM_LOAD( "82s126.2n",    0x0200, 0x0100, CRC(37aece4b) SHA1(08dbb470644278132b8126649fe41d70e7750bee) )    /* character color codes on a per-column basis */

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "82s147.prm",  0x0000, 0x0200, BAD_DUMP CRC(46e5bc92) SHA1(f4171f8650818c017d58ad7131a7aff100b1b99c) )    /* no dump - taken from hunchbkd */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pls153h.bin", 0x0000, 0x00eb, NO_DUMP ) /* missing - pls153 ??? */
ROM_END

ROM_START( spclforc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "27128.8f",     0x0000, 0x1000, CRC(1e9b8d26) SHA1(783e733cfb5d8fa560a6e6a7b49f782abc60bb58) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2716.3n",      0x0000, 0x0800, CRC(68c28bcb) SHA1(12a12cd4d639fea649f4baf40c60994fba303cf0) )
	ROM_LOAD( "2716.3p",      0x0800, 0x0800, CRC(dfb18c81) SHA1(a7a17b14f0c1194da1e771cd1291b59d330e4307) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "2732.7c",      0x0000, 0x1000, CRC(c1ea2b17) SHA1(6e95b15e7fdd04023622e8c521560e47e98acf28) )
	ROM_LOAD( "2732.7d",      0x1000, 0x1000, CRC(fe5501f0) SHA1(ab7d1e02400659ebedfed2837823f228af017a94) )
	ROM_LOAD( "2732.7e",      0x2000, 0x1000, CRC(f6a113bd) SHA1(2f8776780284081f7858334766be6a6fde3a3371) )
	ROM_LOAD( "2732.7f",      0x3000, 0x1000, CRC(42857a7a) SHA1(267dd954480bc87f14c758803cb26b7812d323b8) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "82s126.2e",    0x0000, 0x0100, CRC(b3751a25) SHA1(4b444e8fd02ac8674ecaba2fee083cb9feb99fa0) )
	ROM_LOAD( "82s126.2f",    0x0100, 0x0100, CRC(1026d438) SHA1(927009e6ed520c39c36c1d7966589c6778df1a3a) )
	ROM_LOAD( "82s126.2n",    0x0200, 0x0100, CRC(9735998d) SHA1(c3f50f97369547b1fd25da64507a5c8b725de6d0) )

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "82s147.prm",  0x0000, 0x0200, BAD_DUMP CRC(46e5bc92) SHA1(f4171f8650818c017d58ad7131a7aff100b1b99c) )    /* no dump - taken from hunchbkd */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pls153h.bin", 0x0000, 0x00eb, NO_DUMP ) /* missing - pls153 ??? */
ROM_END

ROM_START( spcfrcii )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "spfc2.8f",     0x0000, 0x1000, CRC(87f9bb6c) SHA1(f432ff205336280ae11ef4b3061be48e19e07d76) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "spfc2.3n",     0x0000, 0x0800, CRC(7aba051a) SHA1(d9c9f9932991fbc8a2130d3025fdc027e7f89511) )
	ROM_LOAD( "spfc2.3p",     0x0800, 0x0800, CRC(efeed826) SHA1(d62c5c75f9c77f7313cb4a60c22368a85807a39a) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "spfc2.7c",     0x0000, 0x1000, CRC(c1ea2b17) SHA1(6e95b15e7fdd04023622e8c521560e47e98acf28) )
	ROM_LOAD( "spfc2.7d",     0x1000, 0x1000, CRC(fe5501f0) SHA1(ab7d1e02400659ebedfed2837823f228af017a94) )
	ROM_LOAD( "spfc2.7e",     0x2000, 0x1000, CRC(f6a113bd) SHA1(2f8776780284081f7858334766be6a6fde3a3371) )
	ROM_LOAD( "spfc2.7f",     0x3000, 0x1000, CRC(42857a7a) SHA1(267dd954480bc87f14c758803cb26b7812d323b8) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "spfc2.2e",     0x0000, 0x0100, CRC(b3751a25) SHA1(4b444e8fd02ac8674ecaba2fee083cb9feb99fa0) )
	ROM_LOAD( "spfc2.2f",     0x0100, 0x0100, CRC(1026d438) SHA1(927009e6ed520c39c36c1d7966589c6778df1a3a) )
	ROM_LOAD( "spfc2.2n",     0x0200, 0x0100, CRC(9735998d) SHA1(c3f50f97369547b1fd25da64507a5c8b725de6d0) )

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "82s147.prm",  0x0000, 0x0200, BAD_DUMP CRC(46e5bc92) SHA1(f4171f8650818c017d58ad7131a7aff100b1b99c) )    /* no dump - taken from hunchbkd */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pls153h.bin", 0x0000, 0x00eb, NO_DUMP ) /* missing - pls153 ??? */
ROM_END

ROM_START( 8ballact )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "8b-dk.5e",     0x0400, 0x0400, CRC(166c1c9b) SHA1(fd5661dbb4617a1daff7949ef030b8572bdebb85) )
	ROM_CONTINUE(             0x0000, 0x0400 )
	ROM_CONTINUE(             0x0c00, 0x0400 )
	ROM_CONTINUE(             0x0800, 0x0400 )
	ROM_LOAD( "8b-dk.5c",     0x2000, 0x1000, CRC(9ec87baa) SHA1(79fc89d9474ac23cda56f6af27db77aba964f395) )
	ROM_LOAD( "8b-dk.5b",     0x4000, 0x1000, CRC(f836a962) SHA1(5a45514ea59cd92092523d116b0dc4a1f8fc46b7) )
	ROM_LOAD( "8b-dk.5a",     0x6000, 0x1000, CRC(d45866d4) SHA1(5dfb121aa87bc5e6efadd9412b9f8d360c3dabd3) )

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "8b-dk.3h",     0x0000, 0x0800, CRC(a8752c60) SHA1(0d7d35fd271d796e884a33071b83c000b91208a0) )
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_FILL(                 0x1000, 0x0800, 0xFF )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "8b-dk.3n",     0x0000, 0x0800, CRC(44830867) SHA1(29d34792b9193edcdac427367c360d6f01e1e094) )
	ROM_LOAD( "8b-dk.3p",     0x0800, 0x0800, CRC(6148c6f2) SHA1(7e6ddb8999f07888ac49441ed8cf6496e8db3caa) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "8b-dk.7c",     0x0000, 0x0800, CRC(e34409f5) SHA1(81fd6f038988843d7ef1c57af75253e497a6f57b) )
	ROM_LOAD( "8b-dk.7d",     0x0800, 0x0800, CRC(b4dc37ca) SHA1(6c469f8edbc6dd02e1821972a503e634f920e221) )
	ROM_LOAD( "8b-dk.7e",     0x1000, 0x0800, CRC(655af8a8) SHA1(d434efc89226d28d24e858186fab9aff0e476deb) )
	ROM_LOAD( "8b-dk.7f",     0x1800, 0x0800, CRC(a29b2763) SHA1(6b2ee88e96a1b74193f12f4fa64a6705f17557b1) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "8b.2e",        0x0000, 0x0100, CRC(c7379a12) SHA1(e128e7d7c71ec61b934651c29648d0d2fc69e306) )
	ROM_LOAD( "8b.2f",        0x0100, 0x0100, CRC(116612b4) SHA1(9a7c5329f211b13d5a757fdac761d7096d78b65a) )
	ROM_LOAD( "8b.2n",        0x0200, 0x0100, CRC(30586988) SHA1(a9c246fd01cb3ff371ad33b55d5b2fe4898c4d1b) )

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "82s147.prm",  0x0000, 0x0200, BAD_DUMP CRC(46e5bc92) SHA1(f4171f8650818c017d58ad7131a7aff100b1b99c) )    /* no dump - taken from hunchbkd */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pls153h.bin", 0x0000, 0x00eb, NO_DUMP ) /* missing - pls153 ??? */
ROM_END

ROM_START( 8ballact2 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "8b-jr.5b",     0x0400, 0x0400, CRC(579cd634) SHA1(93d81539459f7198d8cbf05b3e66a40466aee2d9) )
	ROM_CONTINUE(             0x0000, 0x0400 )
	ROM_CONTINUE(             0x0c00, 0x0400 )
	ROM_CONTINUE(             0x0800, 0x0400 )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "8b-jr.5c",     0x4000, 0x1000, CRC(9bccbe93) SHA1(dec4e1d41e1df36359f205bf090c4290311e4141) )
	ROM_CONTINUE(             0x2000, 0x1000 )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "8b-jr.3h",     0x0000, 0x1000, CRC(7f5c19fa) SHA1(9cde134137ee8e34bb745a72e67981c581561428) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "8b-jr.3n",     0x0000, 0x1000, CRC(9ec0edac) SHA1(ffa1ae2236995527d72917c45be337f642134cf8) )
	ROM_LOAD( "8b-jr.3p",     0x1000, 0x1000, CRC(2978a88b) SHA1(15c21a3b3fb879996c13ce56791828e170c771d1) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "8b-jr.7c",     0x0000, 0x0800, CRC(e34409f5) SHA1(81fd6f038988843d7ef1c57af75253e497a6f57b) )
	ROM_LOAD( "8b-jr.7d",     0x0800, 0x0800, CRC(b4dc37ca) SHA1(6c469f8edbc6dd02e1821972a503e634f920e221) )
	ROM_LOAD( "8b-jr.7e",     0x1000, 0x0800, CRC(655af8a8) SHA1(d434efc89226d28d24e858186fab9aff0e476deb) )
	ROM_LOAD( "8b-jr.7f",     0x1800, 0x0800, CRC(a29b2763) SHA1(6b2ee88e96a1b74193f12f4fa64a6705f17557b1) )

	ROM_REGION( 0x0500, "proms", 0 )
	ROM_LOAD( "8b.2e",        0x0000, 0x0100, CRC(c7379a12) SHA1(e128e7d7c71ec61b934651c29648d0d2fc69e306) )
	ROM_LOAD( "8b.2f",        0x0100, 0x0100, CRC(116612b4) SHA1(9a7c5329f211b13d5a757fdac761d7096d78b65a) )
	ROM_LOAD( "8b.2n",        0x0200, 0x0100, CRC(30586988) SHA1(a9c246fd01cb3ff371ad33b55d5b2fe4898c4d1b) )

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "82s147.prm",  0x0000, 0x0200, BAD_DUMP CRC(46e5bc92) SHA1(f4171f8650818c017d58ad7131a7aff100b1b99c) )    /* no dump - taken from hunchbkd */

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "pls153h.bin", 0x0000, 0x00eb, NO_DUMP ) /* missing - pls153 ??? */
ROM_END

/* encrypted */
ROM_START( drakton )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k for code + 4*16k for decrypted code */
	ROM_LOAD( "2764.u2",      0x0000, 0x2000, CRC(d9a33205) SHA1(06dc96412e7162fd8a4f6ef4d14d1510c06b1d00) )
	ROM_LOAD( "2764.u3",      0x2000, 0x2000, CRC(69583a35) SHA1(061271be4e9ddfd8dff4217f1434215ad35ba505) )

	/* one is used for dkong conversions, the other one for dkongjr conversions */
	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "2716.3h",      0x0000, 0x0800, CRC(3489a35b) SHA1(9ebcf4b20b212d54e6b1a6d9abbda3109298631b) ) /* dkong */
	ROM_RELOAD(               0x0800, 0x0800 )
	ROM_FILL(                 0x1000, 0x0800, 0x00 )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2716.3n",      0x0000, 0x0800, CRC(ea0e7f9a) SHA1(a8e2b43e15281d45e414eaae98e5248bad79c41b) )
	ROM_LOAD( "2716.3p",      0x0800, 0x0800, CRC(46f51b68) SHA1(7d1c3a61cdd0ad471cb0064c0cbaf758325fc267) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "2716.7c",      0x0000, 0x0800, CRC(2925dc2d) SHA1(721748031714ba488191eb074643093c906e8ce2) )
	ROM_LOAD( "2716.7d",      0x0800, 0x0800, CRC(bdf6b1b4) SHA1(ea9076a2bba909bfae8a10a92d857e8f0644fc8b) )
	ROM_LOAD( "2716.7e",      0x1000, 0x0800, CRC(4d62e62f) SHA1(01e757110edcb24600a27b1505f54e3bd04b9e58) )
	ROM_LOAD( "2716.7f",      0x1800, 0x0800, CRC(81d200e5) SHA1(5eb74f319756ba3fbc6d0d918799337f911e9419) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s126.2e",    0x0000, 0x0100, CRC(3ff45f76) SHA1(4068b5568f9e22e54f0df8a9e02bfed0bfb00db7) )
	ROM_LOAD( "82s126.2f",    0x0100, 0x0100, CRC(38f905be) SHA1(a963aea9a92ac95850c90c43085376cb4e06696b) )
	ROM_LOAD( "82s126.2n",    0x0200, 0x0100, CRC(3c343b9b) SHA1(f84f5fddcccc8499a2511877f5d706b37ddc7db8) )
ROM_END

/* encrypted */
ROM_START( drktnjr )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k for code + 4*16k for decrypted code */
	ROM_LOAD( "2764.u2",      0x0000, 0x2000, CRC(d9a33205) SHA1(06dc96412e7162fd8a4f6ef4d14d1510c06b1d00) )
	ROM_LOAD( "2764.u3",      0x2000, 0x2000, CRC(69583a35) SHA1(061271be4e9ddfd8dff4217f1434215ad35ba505) )

	ROM_REGION( 0x1000, "soundcpu", 0 ) /* sound */
	/* one is used for dkong conversions, the other one for dkongjr conversions */
	ROM_LOAD( "2716.3h1",     0x0000, 0x0800, CRC(2a6ec016) SHA1(c95e185a39c8029f00798ce0a00759a4deb45677) ) /* dkongjr */

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "2716.3n",      0x0000, 0x0800, CRC(ea0e7f9a) SHA1(a8e2b43e15281d45e414eaae98e5248bad79c41b) )
	ROM_LOAD( "2716.3p",      0x0800, 0x0800, CRC(46f51b68) SHA1(7d1c3a61cdd0ad471cb0064c0cbaf758325fc267) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "2716.7c",      0x0000, 0x0800, CRC(2925dc2d) SHA1(721748031714ba488191eb074643093c906e8ce2) )
	ROM_LOAD( "2716.7d",      0x0800, 0x0800, CRC(bdf6b1b4) SHA1(ea9076a2bba909bfae8a10a92d857e8f0644fc8b) )
	ROM_LOAD( "2716.7e",      0x1000, 0x0800, CRC(4d62e62f) SHA1(01e757110edcb24600a27b1505f54e3bd04b9e58) )
	ROM_LOAD( "2716.7f",      0x1800, 0x0800, CRC(81d200e5) SHA1(5eb74f319756ba3fbc6d0d918799337f911e9419) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s126.2e",    0x0000, 0x0100, CRC(3ff45f76) SHA1(4068b5568f9e22e54f0df8a9e02bfed0bfb00db7) )
	ROM_LOAD( "82s126.2f",    0x0100, 0x0100, CRC(38f905be) SHA1(a963aea9a92ac95850c90c43085376cb4e06696b) )
	ROM_LOAD( "82s126.2n",    0x0200, 0x0100, CRC(3c343b9b) SHA1(f84f5fddcccc8499a2511877f5d706b37ddc7db8) )
ROM_END

/* encrypted */
ROM_START( strtheat )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k for code + 4*16k for decrypted code */
	ROM_LOAD( "2764.u2",   0x0000, 0x2000, CRC(8d3e82c3) SHA1(ec26fb1c6015721da1f61eca76a4b3390d8dcc76) )
	ROM_LOAD( "2764.u3",   0x2000, 0x2000, CRC(f0759e76) SHA1(e086f02d1861269194c4cd2ada71696b48ed1a1d) )

	ROM_REGION( 0x1800, "soundcpu", 0 ) /* sound */
	ROM_LOAD( "2716.3h",   0x0000, 0x0800, CRC(4cd17174) SHA1(5ed9b5275b0779d1ca05d6e62d3ad8a682ebde37) )
	ROM_RELOAD(            0x0800, 0x0800 )
	ROM_FILL(              0x1000, 0x0800, 0xFF )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "2716.3n",   0x0000, 0x0800, CRC(29e57678) SHA1(cbbb980c44c7f5c45d5f0b85209658f53b7ba4a7) )
	ROM_RELOAD(            0x0800, 0x0800 )
	ROM_LOAD( "2716.3p",   0x1000, 0x0800, CRC(31171146) SHA1(e26b22e73b528810b566b2b9f6d81e2d7856523d) )
	ROM_RELOAD(            0x1800, 0x0800 )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "2716.7c",   0x0000, 0x0800, CRC(a8238e9c) SHA1(947bbe48ce1c705ef974e37b138929f1c846ed79) )
	ROM_LOAD( "2716.7d",   0x0800, 0x0800, CRC(71202138) SHA1(b4edc77ed2844ef46aee4a492282e4785bdb7224) )
	ROM_LOAD( "2716.7e",   0x1000, 0x0800, CRC(dc7785ac) SHA1(4ccb3f9f938fd1d9bd20f1601a16a2780c84588b) )
	ROM_LOAD( "2716.7f",   0x1800, 0x0800, CRC(ede71d86) SHA1(0bce1b1d4180173537685a08055ce44b9dedc76a) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "82s129.2e",     0x0000, 0x0100, CRC(1311ba28) SHA1(d9b5bc07c8943d83592833e8b1c2ff57e4accb55) ) /* palette low 4 bits (inverted) */
	ROM_LOAD( "82s129.2f",     0x0100, 0x0100, CRC(18d90d4f) SHA1(b956fd652dcc5eb50eaec8729762d19cfd475bc7) ) /* palette high 4 bits (inverted) */
	ROM_LOAD( "82s129.2n",     0x0200, 0x0100, CRC(a515d59b) SHA1(930616c4bcd819c2a4432a6619a8c6da74f3e8c5) ) /* character color codes on a per-column basis */
ROM_END

/*
------------------------------------------------
Shooting Gallery by SEATON GROVE/ZACCARIA (1984)
------------------------------------------------

Location      Device      File ID      Checksum
-----------------------------------------------
SSB 5B         2716       SG-01-0        81B9
CPU 3H         2732       SG-01-3H       3F58
CPU 5B         2764       SG-01-5B       C2A9
GFX 3N         2732       SG-01-3N       C8AC
GFX 3P         2732       SG-01-3P       715A
GFX 7C         2716       SG-01-7C       D73C
GFX 7D         2716       SG-01-7D       1545
GFX 7E         2716       SG-01-7E       48A3
GFX 7F         2716       SG-01-7F       97C5
SND IC6        2532       SG-01-SND      6AE1
SND IC8        2716       SG-01-SPK      852B
CPU 2E         6306       SG-01-2E       127E
CPU 2F         6306       SG-01-2F       1167
GFX 2N         6306       SG-01-2N       1000
GIB          82S147       SG-01          1554
GIB          82S153       SG-02          D966 *


Note:  CPU  -  CPU PCB
       GFX  -  GFX/Video PCB
       SSB  -  (Century) Speech/Sound PCB   500-38 Iss3 (1981)
       SND  -  Sound PCB
       GIB  -  Gun Interface PCB  (C) Seatongrove (1984)

         *  -  JEDEC fusemap checksum, not the file checksum
            -  Some ROM images have duplicate halves, this is not an error



Hardware information:

CPU PCB:          MBL8035N  (MCU, RAM 64byte, ROM N/A)
                  upD8257C  (DMA Controller)
                  ROM  SG-01-3H
                  ROM  SG-01-5B
                  PROM SG-01-2E
                  PROM SG-01-2F

Gun Interface:    2650A (8-bit CPU)
                  PROM SG-1
                  PLD  SG-2

Sound PCB:     2x AY8910
                  ROM SG-01-SND
                  ROM SG-01-SPK

Speech PCB:    2x 2650A  (8-bit CPU)
                  MC6810 (128x8 SRAM)
                  TMS5100ANL (Speech)
                  ROM SG-01-0
*/

ROM_START( shootgal )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "cpu.5e",       0x0000, 0x1000, CRC(c6d61b9c) SHA1(de3f4cf559313c4e5c5f7019aa6f68b3334e05c1) )
	ROM_LOAD( "cpu.5a",       0x6000, 0x1000, CRC(5dabe5e3) SHA1(7a36d97b2604aa61fe468863651cf67b851bbfca) )

	ROM_REGION( 0x1000, "soundcpu", 0 )
	ROM_LOAD( "sg-01-3h",    0x0000, 0x1000, CRC(80be5915) SHA1(a4f7d6a8319065a7a712df0195a3f8695d8f99f9) )

	ROM_REGION( 0x0800, "cpu2", 0 )
	ROM_LOAD( "sg-01-0",     0x0000, 0x0800, CRC(f055a624) SHA1(5dfe89d7271092e665cdd5cd59d15a2b70f92f43) )

	ROM_REGION( 0x1800, "tms", 0 )
	ROM_LOAD( "sg-01snd",    0x0000, 0x1000, CRC(644a0728) SHA1(e249fd57bc49572a2246aaf7c68a547f319f51bc) ) /* sg-01-snd */
	ROM_LOAD( "sg-01spk",    0x1000, 0x0800, CRC(aacaf730) SHA1(cd562093ab8931d165cb0877e332474fce131c67) ) /* sg-01-spk */

	ROM_REGION( 0x2000, "user2", 0 ) /* gun proms? */
	ROM_LOAD( "sg-1",        0x0000, 0x0200, CRC(fda82517) SHA1(b36bac69b6f8218b280aae59133ea0d22d7a99f6) )
	ROM_LOAD( "sg-2",        0x0200, 0x091d, CRC(6e065613) SHA1(26d048af5c302f921de8e2c1bc7c7bf48dc21b5a) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sg-01-3n",    0x0000, 0x1000, CRC(72987a57) SHA1(8b972cfccb43023aca905c51182f8d3c06f7d0bb) )
	ROM_LOAD( "sg-01-3p",    0x1000, 0x1000, CRC(1ae9434e) SHA1(26228bf0aba99f48366544772693b35788084a6b) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "sg-01-7c",    0x0000, 0x0800, CRC(3315658e) SHA1(613387aaa58f5df75d42d38b07237c0dc8aaba36) )
	ROM_LOAD( "sg-01-7d",    0x0800, 0x0800, CRC(76ad4143) SHA1(4ccc32656de9d5142e539b84d4c73bb14d32f8bf) )
	ROM_LOAD( "sg-01-7e",    0x1000, 0x0800, CRC(65d11685) SHA1(4e4b0b60ca4c16e26d842e142002887456d98ea4) )
	ROM_LOAD( "sg-01-7f",    0x1800, 0x0800, CRC(44fe71a2) SHA1(ff4442a5601ac2ed63c57e22977299b5b5499c93) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "sg-01-2e",    0x0000, 0x0200, CRC(34fb23ea) SHA1(6bd6de791c9e0a5f9c833c287663e9755e01c573) )
	ROM_LOAD( "sg-01-2f",    0x0100, 0x0200, CRC(c29b880a) SHA1(950017a0298f91e41db9865ed8ce388f4095f6cf) )
	ROM_LOAD( "sg-01-2n",    0x0200, 0x0200, CRC(e08ed788) SHA1(6982f6bcc70dbf4c75ff538a5df70da11bc89bb4) )

	ROM_REGION( 0x0200, "user1", 0 )
	ROM_LOAD( "82s147.prm",  0x0000, 0x0200, BAD_DUMP CRC(46e5bc92) SHA1(f4171f8650818c017d58ad7131a7aff100b1b99c) )    /* no dump - taken from hunchbkd */
ROM_END

/*************************************
 *
 *  Decryption code
 *
 *************************************/

void dkong_state::drakton_decrypt_rom(UINT8 mod, int offs, int *bs)
{
	UINT8 oldbyte,newbyte;
	UINT8 *ROM;
	int mem;

	ROM = memregion("maincpu")->base();

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = ROM[mem];

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
		    connected to D0.  The other six data bits are converted by a
		    PAL10H8 driven by the counter. */

		newbyte = (oldbyte & mod) | (~oldbyte & ~mod);
		newbyte = BITSWAP8(newbyte, bs[0], bs[1], bs[2], bs[3], bs[4], bs[5], bs[6], bs[7]);

		ROM[mem + offs] = newbyte;
	}
}

/*************************************
 *
 *  Driver Init Code
 *
 *************************************/

DRIVER_INIT_MEMBER(dkong_state,herodk)
{
	int A;
	UINT8 *rom = memregion("maincpu")->base();

	/* swap data lines D3 and D4 */
	for (A = 0;A < 0x8000;A++)
	{
		if ((A & 0x1000) == 0)
		{
			int v;

			v = rom[A];
			rom[A] = (v & 0xe7) | ((v & 0x10) >> 1) | ((v & 0x08) << 1);
		}
	}
}


DRIVER_INIT_MEMBER(dkong_state,drakton)
{
	int bs[4][8] = {
			{7,6,1,3,0,4,2,5},
			{7,1,4,3,0,6,2,5},
			{7,6,1,0,3,4,2,5},
			{7,1,4,0,3,6,2,5},
	};

	m_maincpu->space(AS_PROGRAM).install_read_bank(0x0000, 0x3fff, "bank1" );

	/* While the PAL supports up to 16 decryption methods, only four
	    are actually used in the PAL.  Therefore, we'll take a little
	    memory overhead and decrypt the ROMs using each method in advance. */

	drakton_decrypt_rom(0x02, 0x10000, bs[0]);
	drakton_decrypt_rom(0x40, 0x14000, bs[1]);
	drakton_decrypt_rom(0x8a, 0x18000, bs[2]);
	drakton_decrypt_rom(0xc8, 0x1c000, bs[3]);
}


DRIVER_INIT_MEMBER(dkong_state,strtheat)
{
	int bs[4][8] = {
			{0,6,1,7,3,4,2,5},
			{0,6,4,1,3,7,2,5},
			{6,3,1,7,0,4,2,5},
			{6,3,4,1,0,7,2,5},
	};

	m_maincpu->space(AS_PROGRAM).install_read_bank(0x0000, 0x3fff, "bank1" );

	/* While the PAL supports up to 16 decryption methods, only four
	    are actually used in the PAL.  Therefore, we'll take a little
	    memory overhead and decrypt the ROMs using each method in advance. */
	drakton_decrypt_rom(0x03, 0x10000, bs[0]);
	drakton_decrypt_rom(0x81, 0x14000, bs[1]);
	drakton_decrypt_rom(0x0a, 0x18000, bs[2]);
	drakton_decrypt_rom(0x88, 0x1c000, bs[3]);

	/* custom handlers supporting Joystick or Steering Wheel */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x7c00, 0x7c00, read8_delegate(FUNC(dkong_state::strtheat_inputport_0_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x7c80, 0x7c80, read8_delegate(FUNC(dkong_state::strtheat_inputport_1_r),this));
}


DRIVER_INIT_MEMBER(dkong_state,dkongx)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	m_decrypted = std::make_unique<UINT8[]>(0x10000);

	m_maincpu->space(AS_PROGRAM).install_read_bank(0x0000, 0x5fff, "bank1" );
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x8000, 0xffff, "bank2" );

	space.install_write_handler(0xe000, 0xe000, write8_delegate(FUNC(dkong_state::braze_a15_w),this));

	space.install_read_handler(0xc800, 0xc800, read8_delegate(FUNC(dkong_state::braze_eeprom_r),this));
	space.install_write_handler(0xc800, 0xc800, write8_delegate(FUNC(dkong_state::braze_eeprom_w),this));

	braze_decrypt_rom(m_decrypted.get());

	membank("bank1")->configure_entries(0, 2, m_decrypted.get(), 0x8000);
	membank("bank1")->set_entry(0);
	membank("bank2")->configure_entries(0, 2, m_decrypted.get(), 0x8000);
	membank("bank2")->set_entry(0);
}

DRIVER_INIT_MEMBER(dkong_state,dkingjr)
{
	UINT8 *prom = memregion("proms")->base();
	for( int i=0; i<0x200; ++i)
	{
		prom[i]^=0xff; // invert color data
	}
	m_palette->update();
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1980, radarscp,  0,        radarscp,  radarscp, driver_device, 0,        ROT90,  "Nintendo", "Radar Scope", MACHINE_SUPPORTS_SAVE )
GAME( 1980, radarscp1, radarscp, radarscp1, radarscp, driver_device, 0,        ROT90,  "Nintendo", "Radar Scope (TRS01)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, dkong,     0,        dkong2b,   dkong,    driver_device, 0,        ROT90,  "Nintendo of America", "Donkey Kong (US set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, dkonghrd,  dkong,    dkong2b,   dkong,    driver_device, 0,        ROT90,  "Nintendo of America", "Donkey Kong (hard kit)", MACHINE_SUPPORTS_SAVE ) // not sure if original or bootleg (see notes on top of driver file)
GAME( 1981, dkongo,    dkong,    dkong2b,   dkong,    driver_device, 0,        ROT90,  "Nintendo", "Donkey Kong (US set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, dkongj,    dkong,    dkong2b,   dkong,    driver_device, 0,        ROT90,  "Nintendo", "Donkey Kong (Japan set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, dkongjo,   dkong,    dkong2b,   dkong,    driver_device, 0,        ROT90,  "Nintendo", "Donkey Kong (Japan set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, dkongjo1,  dkong,    dkong2b,   dkong,    driver_device, 0,        ROT90,  "Nintendo", "Donkey Kong (Japan set 3)", MACHINE_SUPPORTS_SAVE )

GAME( 2004, dkongf,    dkong,    dkong2b,   dkongf,   driver_device, 0,        ROT90,  "hack (Jeff Kulczycki)", "Donkey Kong Foundry (hack)", MACHINE_SUPPORTS_SAVE ) /* from Jeff's Romhack */
GAME( 2006, dkongx,    dkong,    braze,     dkongx,   dkong_state,   dkongx,   ROT90,  "hack (Braze Technologies)", "Donkey Kong II: Jumpman Returns (hack, V1.2)", MACHINE_SUPPORTS_SAVE )
GAME( 2006, dkongx11,  dkong,    braze,     dkongx,   dkong_state,   dkongx,   ROT90,  "hack (Braze Technologies)", "Donkey Kong II: Jumpman Returns (hack, V1.1)", MACHINE_SUPPORTS_SAVE )
GAME( 2013, dkongpe,   dkong,    dkong2b,   dkong,    driver_device, 0,        ROT90,  "hack (Clay Cowgill and Mike Mika)", "Donkey Kong: Pauline Edition Rev 5 (2013-04-22)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, dkongjr,   0,        dkongjr,   dkongjr,  driver_device, 0,        ROT90,  "Nintendo of America", "Donkey Kong Junior (US set F-2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, dkongjrj,  dkongjr,  dkongjr,   dkongjr,  driver_device, 0,        ROT90,  "Nintendo", "Donkey Kong Jr. (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, dkongjnrj, dkongjr,  dkongjr,   dkongjr,  driver_device, 0,        ROT90,  "Nintendo", "Donkey Kong Junior (Japan?)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, dkongjrb,  dkongjr,  dkongjr,   dkongjr,  driver_device, 0,        ROT90,  "bootleg", "Donkey Kong Jr. (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, dkongjre,  dkongjr,  dkongjr,   dkongjr,  driver_device, 0,        ROT90,  "Nintendo of America", "Donkey Kong Junior (E kit)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, dkongjrpb, dkongjr,  dkongjr,   dkongjr,  driver_device, 0,        ROT90,  "bootleg", "Donkey Kong Junior (P kit, bootleg)", MACHINE_SUPPORTS_SAVE ) // definitely not issued by Nintendo
GAME( 1982, jrking,    dkongjr,  dkongjr,   dkongjr,  driver_device, 0,        ROT90,  "bootleg", "Junior King (bootleg of Donkey Kong Jr.)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, dkingjr,   dkongjr,  dkongjr,   dkongjr,  dkong_state,   dkingjr,  ROT90,  "bootleg", "Donkey King Jr. (bootleg of Donkey Kong Jr.)", MACHINE_SUPPORTS_SAVE )

GAME( 1983, dkong3,    0,        dkong3,    dkong3,   driver_device, 0,        ROT90,  "Nintendo of America", "Donkey Kong 3 (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, dkong3j,   dkong3,   dkong3,    dkong3,   driver_device, 0,        ROT90,  "Nintendo", "Donkey Kong 3 (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, dkong3b,   dkong3,   dkong3b,   dkong3b,  driver_device, 0,        ROT90,  "bootleg", "Donkey Kong 3 (bootleg on Donkey Kong Jr. hardware)", MACHINE_SUPPORTS_SAVE )

GAME( 1983, pestplce,  mario,    pestplce,  pestplce, driver_device, 0,        ROT180, "bootleg", "Pest Place", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

/* 2650 based */
GAME( 1984, herbiedk,  huncholy, s2650,     herbiedk, driver_device, 0,        ROT90,  "Century Electronics / Seatongrove Ltd", "Herbie at the Olympics (DK conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, hunchbkd,  hunchbak, s2650,     hunchbkd, driver_device, 0,        ROT90,  "Century Electronics", "Hunchback (DK conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, sbdk,      superbik, s2650,     sbdk,     driver_device, 0,        ROT90,  "Century Electronics", "Super Bike (DK conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, herodk,    hero,     s2650,     herodk,   dkong_state,   herodk,   ROT90,  "Seatongrove Ltd (Crown license)", "Hero in the Castle of Doom (DK conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, herodku,   hero,     s2650,     herodk,   driver_device, 0,        ROT90,  "Seatongrove Ltd (Crown license)", "Hero in the Castle of Doom (DK conversion not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, 8ballact,  0,        s2650,     8ballact, driver_device, 0,        ROT90,  "Seatongrove Ltd (Magic Electronics USA license)", "Eight Ball Action (DK conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, 8ballact2, 8ballact, s2650,     8ballact, driver_device, 0,        ROT90,  "Seatongrove Ltd (Magic Electronics USA license)", "Eight Ball Action (DKJr conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, shootgal,  0,        s2650,     shootgal, driver_device, 0,        ROT180, "Seatongrove Ltd (Zaccaria license)", "Shooting Gallery", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, spclforc,  0,        spclforc,  spclforc, driver_device, 0,        ROT90,  "Senko Industries (Magic Electronics Inc. license)", "Special Forces", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1985, spcfrcii,  0,        spclforc,  spclforc, driver_device, 0,        ROT90,  "Senko Industries (Magic Electronics Inc. license)", "Special Forces II", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

/* EPOS */
GAME( 1984, drakton,   0,        drakton,   drakton,  dkong_state,   drakton,  ROT90,  "Epos Corporation", "Drakton (DK conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, drktnjr,   drakton,  drktnjr,   drakton,  dkong_state,   drakton,  ROT90,  "Epos Corporation", "Drakton (DKJr conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, strtheat,  0,        strtheat,  strtheat, dkong_state,   strtheat, ROT90,  "Epos Corporation", "Street Heat", MACHINE_SUPPORTS_SAVE ) // distributed by Cardinal Amusements Products (a division of Epos Corporation)
