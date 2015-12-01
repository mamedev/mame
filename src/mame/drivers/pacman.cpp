// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Namco PuckMan

    driver by Nicola Salmoria and many others

    Games supported:
        * Puck Man
        * Pac-Man Plus
        * Ms. Pac-Man
        * Crush Roller
        * Ponpoko
        * Eyes
        * Mr. TNT
        * Gorkans
        * Lizard Wizard
        * The Glob
        * Dream Shopper
        * Van Van Car
        * Ali Baba and 40 Thieves
        * Jump Shot
        * Shoot the Bull
        * Big Bucks
        * Driving Force
        * Eight Ball Action
        * Porky
        * MTV Rock-N-Roll Trivia (Part 2)
        * Woodpecker
		* Pacman Club / Club Lambada


    Known issues:
        * Mystery items in Ali Baba don't work correctly because of protection.
		* Pacman Club controls need to be demultiplexed for 2-players simultaneous mode.
		  Also need 4-players extra inputs.

    Known to exist but dumps needed
        * Eeeek!
        * Ms Pac Plus
        * Ms Pac Man Twin
        * MTV Rock-N-Roll Trivia (Part 2), 1 bad rom. It's not a bad dump, the rom is bad.

****************************************************************************

    Pac-Man memory map (preliminary)

    0000-3fff ROM
    4000-43ff Video RAM
    4400-47ff Color RAM
    4800-4bff RAM Dream Shopper, Van Van Car only.  Pacman uses this block due to a bug in the routine to
         translate the internal pacman location to a screen address when in the right tunnel.
    4c00-4fff RAM
    8000-bfff ROM Ms Pac-Man, Ponpoko, Lizard Wizard, Dream Shopper, Van Van Car, Woodpecker, Ali Babba all use
         portions of the upper memory area.  Pacman and most bootlegs don't have an A15 line to the cpu so most
         boards that use upper memmory have an auxillary board that plugs into the cpu socket with a ribbon cable.
         There is also a common Ms Pacman hack for pacman bootlegs to wire A15 from the cpu to the address decoder
         in place of the refresh line. The extra eproms are stacked on lower eproms or placed in unused sockets for
         2k roms.


    memory mapped ports:

    read:
    5000      IN0
    5040      IN1
    5080      DSW 1
    50c0      DSW 2 (Ponpoko, mschamp, Van Van Car, Rock and Roll Trivia 2 only)
    see the input_ports definition below for details on the input bits

    write:
    4ff0-4fff 8 pairs of two bytes:
              the first byte contains the sprite image number (bits 2-7), Y flip (bit 0),
              X flip (bit 1); the second byte the color.  Note: Only Ponpoko has 8 sprites
              an original Midway Pacman board containls only the center 6 sprites.
    5000      interrupt enable
    5001      sound enable
    5002      latch at location 8K has no connection for this address
    5003      flip screen
    5004      1 player start lamp
    5005      2 players start lamp
    5006      coin lockout                      Note: no boards are known to contain the output
            transistors to drive a lamp or coin lockout.   The schematics and boards show no connection
            to the output of the latch at location 8k
    5007      coin counter
    5040-5044 sound voice 1 accumulator (nibbles) (used by the sound hardware only)
    5045      sound voice 1 waveform (nibble)
    5046-5049 sound voice 2 accumulator (nibbles) (used by the sound hardware only)
    504a      sound voice 2 waveform (nibble)
    504b-504e sound voice 3 accumulator (nibbles) (used by the sound hardware only)
    504f      sound voice 3 waveform (nibble)
    5050-5054 sound voice 1 frequency (nibbles)
    5055      sound voice 1 volume (nibble)
    5056-5059 sound voice 2 frequency (nibbles)
    505a      sound voice 2 volume (nibble)
    505b-505e sound voice 3 frequency (nibbles)
    505f      sound voice 3 volume (nibble)
    5060-506f Sprite coordinates, x/y pairs for 8 sprites
    50c0      Watchdog reset

    I/O ports:
    OUT on port $0 sets the interrupt vector


****************************************************************************

    Make Trax protection description:

    Make Trax has a "Special" chip that it uses for copy protection.
    The following chart shows when reads and writes may occur:

    AAAAAAAA AAAAAAAA
    11111100 00000000  <- address bits
    54321098 76543210
    xxx1xxxx 01xxxxxx - read data bits 4 and 7
    xxx1xxxx 10xxxxxx - read data bits 6 and 7
    xxx1xxxx 11xxxxxx - read data bits 0 through 5

    xxx1xxxx 00xxx100 - write to Special
    xxx1xxxx 00xxx101 - write to Special
    xxx1xxxx 00xxx110 - write to Special
    xxx1xxxx 00xxx111 - write to Special

    In practical terms, it reads from Special when it reads from
    location $5040-$50FF.  Note that these locations overlap our
    inputs and Dip Switches.  Yuk.

    I don't bother trapping the writes right now, because I don't
    know how to interpret them.  However, comparing against Crush
    Roller gives most of the values necessary on the reads.

    Instead of always reading from $5040, $5080, and $50C0, the Make
    Trax programmers chose to read from a wide variety of locations,
    probably to make debugging easier.  To us, it means that for the most
    part we can just assign a specific value to return for each address and
    we'll be OK.  This falls apart for the following addresses:  $50C0, $508E,
    $5090, and $5080.  These addresses should return multiple values.  The other
    ugly thing happening is in the ROMs at $3AE5.  It keeps checking for
    different values of $50C0 and $5080, and weird things happen if it gets
    the wrong values.  The only way I've found around these is to patch the
    ROMs using the same patches Crush Roller uses.  The only thing to watch
    with this is that changing the ROMs will break the beginning checksum.
    That's why we use the rom opcode decode function to do our patches.

    Incidentally, there are extremely few differences between Crush Roller
    and Make Trax.  About 98% of the differences appear to be either unused
    bytes, the name of the game, or code related to the protection.  I've
    only spotted two or three actual differences in the games, and they all
    seem minor.

    If anybody cares, here's a list of disassembled addresses for every
    read and write to the Special chip (not all of the reads are
    specifically for checking the Special bits, some are for checking
    player inputs and Dip Switches):

    Writes: $0084, $012F, $0178, $023C, $0C4C, $1426, $1802, $1817,
        $280C, $2C2E, $2E22, $3205, $3AB7, $3ACC, $3F3D, $3F40,
        $3F4E, $3F5E
    Reads:  $01C8, $01D2, $0260, $030E, $040E, $0416, $046E, $0474,
        $0560, $0568, $05B0, $05B8, $096D, $0972, $0981, $0C27,
        $0C2C, $0F0A, $10B8, $10BE, $111F, $1127, $1156, $115E,
        $11E3, $11E8, $18B7, $18BC, $18CA, $1973, $197A, $1BE7,
        $1C06, $1C9F, $1CAA, $1D79, $213D, $2142, $2389, $238F,
        $2AAE, $2BF4, $2E0A, $39D5, $39DA, $3AE2, $3AEA, $3EE0,
        $3EE9, $3F07, $3F0D


*****************************************************************************************
notes:
------
- Pacman: The Cabinet Type "dip switch" actually comes from the edge connector.  "Dip switches" #7(0x40) and #8(0x80)
  are really solder pads. The actual dip #7 is Rack advance from IN0 and dip #8 is video freeze which is handled in
  hardware.

- Pacman: Pacman contains a bug in the up direction of pacman animation. The circle animation is 1 pixel to low.
  Pucman set 1 has a patch to fix it at 1700-1704

- puckman set 3 (previously labeled(harder?):
  npacmod.6j and NAMCOPAC.6J
  00000031: AF 25    ;3031 is sub for fail rom check.
  00000032: C3 7C    ;301c is sub for pass rom check
  00000033: 1C E6    ;so it now clears the sum (reg A) and
  00000034: 30 F0    ;jumps to pass if it fails rom check.
  000007F8: 31 30  c 1981 / c 1980
  0000080B-12:   ghost / nickname
  00000FFF: 00 F1  checksum

- mspacmab: this is the equivalent of pacman with the ms pacman overlays permanently installed.  There are a few
  extra bytes that don't seemed to be used at all.  The Rom check is patched to never fail.  Many repair shops will
  install this code in all ms pacman boards.  Since the overlays move in and out during play on the original it is
  unknown if this version will play the same. It is possible to identify if a board is running this or the original
  code by observing the behavior on boot.

- mspacmnf,pacmanf: These sets are the results of replacing 6f with what is known as the speedup chip. These are more
  popular than the original with operators and players.  The pacman speedup breaks the attract mode and intermissions.
  There's a modern speedup chip that fixes this.

- Pacmod Harder compared to Pacman
  pacmanh.5E
  erase tiles $25:"."   $28-$2e:"NAMCO"   $5c:copyright C

  program code
  pacman/pacmod
  0843: 14 0F  Table of when ghosts come out of house.
  0844: 1E 14
  0845: 46 37
  0846: 00 04 pink,   level 1
  0847: 1E 18 blue
  0848: 3C 34 orange
  0849: 00 02         level 2
  084A: 00 06
  084B: 32 28
  084c: 00 00         level 3
  084D: 00 04
  084E: 00 08
  0FFF: 36 54  checksum
  2795: 29 00  pink ghost ai, points target to 2 tiles in front of pac instead of 4.
  281F: 40 24  orange ghost ai, change close/far breakpoint from $40 to $24.
  2FFF: 4C 91    checksum
  37F8,3d28,3d43:  change 1980 to 1981
  Notice the harder upgrade was 2 roms only. 6e,6h.  The others are from a newer set.

- joyman: Pacman maze hack, but they left the dots as is, the music has been mangled as well.

- ctrpllrp: Caterpillar is like Piranha in that it was sold as a unique game.  They were hoping no one would notice
  it uses pacman code for it's base.  Unlike Piranha as far as I know they got away with it.
  Differences include new music in the intermissions.

- mspacatk: This is the alternate maze chip. The mazes are new but the fruit paths are not changed, causing fruit
  to move through walls.  There are at least two different hacks of this chip to fix it.

- mspacpls: This romset is hacked.  mspacatk.2 is more commonly known as the "cheat chip" and is used on bootleg
  ms pacs  in place of boot2. It was created circa 1997 by Doc Cutlip and allows turbo speed and invulnerability.
  mspacatk.5 and .6 contain the decoded maze data for ms pac plus.  The actual dump is not available.
  I believe the actual version contains more than just new maze data.

- pacgal:  This is a common hack found on make trax and other boards.  Mostly they use 4k eproms but Make Trax allows
  4x2k for graphics. Boot5 and 6 are stacked on boot3 and 4, the refresh hack is used for addressing.   Usually
  they swap out the 4a color prom as well.  The bootlegger here was apparently too cheap to burn the prom and kept the
  make trax colors.  Other than that this is an insignificant set.

  Comparing files boot3 and PACMAN.7FH
  0000069F: 01 03   ;runs on boot only, probably garbage bit
  Comparing files boot4 and PACMAN.7HJ
  00000807-812:  ;Changes "MS PAC-MAN" to "PAC-GAL"
  00000D3E-D4C:  ;Erases  c MIDWAY MFG CO
  00000D97-DA0:  ;Changes MS PAC-MAN  to  PAC-GAL
  Comparing files 5f and 5F PacGal
  000007D5: 00 01 garbage bit in otherwise blank sprite $1f

- maketrax: All Make Trax boards are Crush Roller boards.  They have Crush Roller screen printed on them covered by
  a Williams sticker.  The roms also have another sticker under the Williams sticker.

- maketrxb:  This board looked slightly different than the standard make trax.  The usual Red and Yellow jumper wires
  for sync inversion were smaller and white and the stickers were slightly different.

- Pac and Paint: Even though this seems to be a unique game the marquee shows a paintbush.  The board has standard
  crush roller roms.  It is slightly different from a Make Trax board in that the sync inverter chip is missing and
  there are no jumper wires.

- jumpshotp: This board was acquired by Pokemame from a former midway employee.  The hardware is identical to normal boards
  except the daughterboard is missing the epoxy potting.  Board was labeled engineering sample. Code differences
  include dips, starting position and cheerleader text.

- sprglobp: This might be original or someone may have combined the program roms from the glob with roms from super.
  I believe there is a set that uses 1 more program rom than this set.

- Eyes, Mr. Tnt and Lizard Wizard share some code.

- Games that share code with Puckman: Ali Baba, Piranha, Caterpiller, Naughty Mouse, Pacman Plus, ms pacman.  From
  galaxian hardware steaking and the pacman bootlegs also.

- acitya contains a bug with the insurance in blackjack.  It's impossible to collect, so it's likely that acitya is
  earlier than bwcasino.

Easter eggs:
-----------
- Pacman, Ms Pacman, Piranha and similar display "MADE BY NAMCO" sideways in power pellets.  Hold start 1 and 2 and
  toggle the test switch to get to the convergence grid.  4xUp, 4xL, 4xR, 4xD.

- Ms. Pac-Man has a hidden message at the very end of ROM memory 0x97d0-0x97ff:

    000097d0: 4745 4e45 5241 4c20 434f 4d50 5554 4552  GENERAL COMPUTER
    000097e0: 2020 434f 5250 4f52 4154 494f 4e20 2020    CORPORATION
    000097f0: 4865 6c6c 6f2c 204e 616b 616d 7572 6121  Hello, Nakamura!

  Masaya Nakamura is the founder of Namco who originally produced Pac-Man in Japan.
  General Computer Corporation designed Ms. Pac-Man and licensed it to Midway for manufacture in North America.

- Super ABC shows developer credits at boot if IN1 is diconnected.

Boards:
-------
- puckman is the same as pacman except they are slotted to break a part and have ribbon cables to connect the halves.

- All ms pacman boards are pacman boards with an auxillary cpu board installed on a ribbon cable and 5e,5f changed.

- Pacman Plus, Shoot the Bull, Jumpshot are epoxy potted auxillary cpu boards.  The graphics and
  color proms are changed as well.  They are labeled Authorized Enhancement Kit Bally/Midway.   Jumpshot=B3172

- The Eyes boards are similar to pacman boards except for the data swapping encryption.  The custom chips are integrated
  into the board and there is no voltage regulator section.  There is an extra rom at row 6 so row 7 is equivilant to
  pacman row 6.  Program roms are 4k but graphics roms have unpopulated areas for optional 2k roms.

- Piranha and Naughty Mouse use a board known as the GDP-01 bootleg.  It is similar to an eyes board with an extra row
  of eproms in row 6 to enable 2k program roms. Thee GDP-01 does not requiree a SBC, but 5 SBC chips can be left unpopulated
  and a SBC card(GDP-02) can be installed.

- Make Trax/Crush Roller boards are similar to pacman boards, the chip positions are even mostly the same.  There is no
  voltage regulator section.  Make Trax has sync inverter at 1S, jumper wires run from the edge connector to the
  inverter and back.  Make Trax/CR boards are easily identified because the edge connector is on the long side,
  offset to one side.  Make trax and Eyes pinouts are similar enough to test each other but are not playable.

- Atlantic City Action, Boardwalk Casino, The Glob, Super Glob, Beastie Feastie, EEEK! all use identical Epos boards
  with different pals.  Street heat, Drakton on Donkey Kong also use this board.  It is a cpu auxillary board.  It is
  not potted but all the chip labels are removed.

- Porky and 8ball action use the same plug in board for pacman.  It is much larger than most and requires some soldering
  to install.  It includes a new cpu and sound hardware.  Driving force uses a similar board and requires a lot of
  soldering.

- Lizard Wizard is a potted auxillary cpu board.

- Big Bucks is an auxillary cpu board.

- MTV Trivia is an auxillary cpu board.

- Truco Clemente runs on a pac bootleg with a handwired cpu/graphics auxillary board.  This supports banking of the
  roms and completely bypasses color proms and video output section of pacman.

- Vanvanb was found on a low quality board assumed to be a bootleg of an original.  The 2 sound chips were on a
  daughterboard that plugged into a 40 pin socket. Physically it is very different from pacman, although the fact that
  it uses the same falcon pinout as every other pac boot is a little suspicious.

- Ponpoko uses it's own board.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/pacman.h"
#include "cpu/s2650/s2650.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/sn76496.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK        (XTAL_18_432MHz)

#define PIXEL_CLOCK         (MASTER_CLOCK/3)

/* H counts from 128->511, HBLANK starts at 144 and ends at 240 */
#define HTOTAL              (384)
#define HBEND               (0)     /*(96+16)*/
#define HBSTART             (288)   /*(16)*/

#define VTOTAL              (264)
#define VBEND               (0)     /*(16)*/
#define VBSTART             (224)   /*(224+16)*/


/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET_MEMBER(pacman_state,mschamp)
{
	UINT8 *rom = memregion("maincpu")->base() + 0x10000;
	int whichbank = ioport("GAME")->read() & 1;

	membank("bank1")->configure_entries(0, 2, &rom[0x0000], 0x8000);
	membank("bank2")->configure_entries(0, 2, &rom[0x4000], 0x8000);

	membank("bank1")->set_entry(whichbank);
	membank("bank2")->set_entry(whichbank);
}

MACHINE_RESET_MEMBER(pacman_state,superabc)
{
	superabc_bank_w(m_maincpu->space(AS_PROGRAM), 0, 0);
}



/*************************************
 *
 *  Interrupts
 *
 *************************************/

INTERRUPT_GEN_MEMBER(pacman_state::vblank_irq)
{
	if(m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(pacman_state::vblank_nmi)
{
	if(m_irq_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(pacman_state::irq_mask_w)
{
	m_irq_mask = data & 1;
}

WRITE8_MEMBER(pacman_state::pacman_interrupt_vector_w)
{
	m_maincpu->set_input_line_vector(0, data);
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


/*
   The piranha board has a sync bus controller card similar to Midway's pacman. It
   stores the LSB of the interrupt vector using port 00 but it alters the byte to prevent
   it from running on normal pacman hardware and vice versa. I wrote a program to print
   out the even numbers and the vectors they convert to.  Thanks to Dave France for
   burning the roms.  The numbers that didn't print here convert to odd numbers.  It's
   slightly possible some numbers listed converted to odd numbers and coincidentally
   printed a valid even number.  Since it only uses 2 vectors($fa,$fc) I didn't complete
   the table or attempt to find the algorithm.

   David Widel
   Pacman@mailblocks.com
  out vec  out vec  out vec  out vec
  c0 44    80 04    40 44    00 04
  c2 40    82 00    42 C4    02 84
  c4 C4    84 84    44 C4    04 00
  c6 40    86 00
  c8 4C    88 0C    48 4C    08 0C
  ca 48    8A 08    4A CC    0A 8C
  cc CC    8C 8C    4C 48    0C 08
  ce 48    8E 08
  d0 54    90 14    50 54    10 14
  d2 50    92 10    52 D4    12 94
  d4 D4    94 94    54 50    14 10
  d6 50    96 10
  d8 5C    98 1C    58 5C    18 1C
  da 58    9A 18    5A DC    1A 9C
  dc DC    9C 9C    5C 58    1C 18
  de 58    9E 18
  e0 64    a0 24    60 64    20 24
  e2 60    a2 20    62 E4    22 A4
  e4 E4    a4 A4    64 60    24 20
  e6 60    a6 20
  e8 6C    a8 2C    68 6C    28 2C
  ea 68    aA 28    6A EC    2A AC
  ec EC    aC AC    6C 68    2C 28
  ee 68    aE 28
  f0 74    b0 34    70 74    30 34
  f2 70    b2 30    72 F4    32 84
  f4 F4    b4 B4    74 70    34 30
  f6 70    b6 30
  f8 7C    b8 3C    78 7C    38 3C
  fa 78    bA 38    7A FC    3A BC
  fc FC    bC BC    7C 78    3C 38
  fe 78    bE 38


Naughty Mouse uses the same board as Piranha with a different pal to encrypt the vectors.
Incidentally we don't know the actual name of this game.  Other than the word naughty at
the top of the playfield there's no name.

I haven't examined the code thoroughly but what I
did look at(sprite buffer), was copied from Pacman.  The addresses for the variables seem
to be the same as well.
*/

WRITE8_MEMBER(pacman_state::piranha_interrupt_vector_w)
{
	if (data == 0xfa) data = 0x78;
	if (data == 0xfc) data = 0xfc;
	m_maincpu->set_input_line_vector(0, data );
}


WRITE8_MEMBER(pacman_state::nmouse_interrupt_vector_w)
{
	if (data == 0xbf) data = 0x3c;
	if (data == 0xc6) data = 0x40;
	if (data == 0xfc) data = 0xfc;
	m_maincpu->set_input_line_vector(0, data );
}



/*************************************
 *
 *  LEDs/coin counters
 *
 *************************************/

WRITE8_MEMBER(pacman_state::pacman_leds_w)
{
	set_led_status(machine(), offset,data & 1);
}


WRITE8_MEMBER(pacman_state::pacman_coin_counter_w)
{
	coin_counter_w(machine(), offset,data & 1);
}


WRITE8_MEMBER(pacman_state::pacman_coin_lockout_global_w)
{
	coin_lockout_global_w(machine(), ~data & 0x01);
}



/*************************************
 *
 *  Ali Baba sound
 *
 *************************************/

WRITE8_MEMBER(pacman_state::alibaba_sound_w)
{
	/* since the sound region in Ali Baba is not contiguous, translate the
	   offset into the 0-0x1f range */
	if (offset < 0x10)
		m_namco_sound->pacman_sound_w(space, offset, data);
	else if (offset < 0x20)
		m_spriteram2[offset - 0x10] = data;
	else
		m_namco_sound->pacman_sound_w(space, offset - 0x10, data);
}

READ8_MEMBER(pacman_state::alibaba_mystery_1_r)
{
	/* The return value determines what the mystery item is.  Each bit corresponds
	   to a question mark */
	return machine().rand() & 0x0f;
}


READ8_MEMBER(pacman_state::alibaba_mystery_2_r)
{
	/* The single bit return value determines when the mystery is lit up.
	   This is certainly wrong */
	m_mystery++;
	return (m_mystery >> 10) & 1;
}



/*************************************
 *
 *  Make Trax input handlers
 *
 *************************************/

READ8_MEMBER(pacman_state::maketrax_special_port2_r)
{
	int data = ioport("DSW1")->read();
	int pc = space.device().safe_pcbase();

	if ((pc == 0x1973) || (pc == 0x2389)) return data | 0x40;

	switch (offset)
	{
		case 0x01:
		case 0x04:
			data |= 0x40; break;
		case 0x05:
			data |= 0xc0; break;
		default:
			data &= 0x3f; break;
	}

	return data;
}

READ8_MEMBER(pacman_state::maketrax_special_port3_r)
{
	int pc = space.device().safe_pcbase();

	if (pc == 0x040e) return 0x20;

	if ((pc == 0x115e) || (pc == 0x3ae2)) return 0x00;

	switch (offset)
	{
		case 0x00:
			return 0x1f;
		case 0x09:
			return 0x30;
		case 0x0c:
			return 0x00;
		default:
			return 0x20;
	}
}

READ8_MEMBER(pacman_state::korosuke_special_port2_r)
{
	int data = ioport("DSW1")->read();
	int pc = space.device().safe_pcbase();

	if ((pc == 0x196e) || (pc == 0x2387)) return data | 0x40;

	switch (offset)
	{
		case 0x01:
		case 0x04:
			data |= 0x40; break;
		case 0x05:
			data |= 0xc0; break;
		default:
			data &= 0x3f; break;
	}

	return data;
}

READ8_MEMBER(pacman_state::korosuke_special_port3_r)
{
	int pc = space.device().safe_pcbase();

	if (pc == 0x0445) return 0x20;

	if ((pc == 0x115b) || (pc == 0x3ae6)) return 0x00;

	switch (offset)
	{
		case 0x00:
			return 0x1f;
		case 0x09:
			return 0x30;
		case 0x0c:
			return 0x00;
		default:
			return 0x20;
	}
}


/*************************************
 *
 *  Zola kludge
 *
 *************************************/

READ8_MEMBER(pacman_state::mschamp_kludge_r)
{
	return m_counter++;
}


/************************************
 *
 *  Big Bucks questions roms handlers
 *
 ************************************/

WRITE8_MEMBER(pacman_state::bigbucks_bank_w)
{
	m_bigbucks_bank = data;
}

READ8_MEMBER(pacman_state::bigbucks_question_r)
{
	UINT8 *question = memregion("user1")->base();
	UINT8 ret;

	ret = question[(m_bigbucks_bank << 16) | (offset ^ 0xffff)];

	return ret;
}


/************************************
 *
 *  S2650 cpu based games
 *
 ************************************/

INTERRUPT_GEN_MEMBER(pacman_state::s2650_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x03);
}

WRITE8_MEMBER(pacman_state::porky_banking_w)
{
	membank("bank1")->set_entry(data & 1);
	membank("bank2")->set_entry(data & 1);
	membank("bank3")->set_entry(data & 1);
	membank("bank4")->set_entry(data & 1);
}

READ8_MEMBER(pacman_state::drivfrcp_port1_r)
{
	switch (space.device().safe_pc())
	{
		case 0x0030:
		case 0x0291:
			return 0x01;
	}

	return 0;
}

READ8_MEMBER(pacman_state::_8bpm_port1_r)
{
	switch (space.device().safe_pc())
	{
		case 0x0030:
		case 0x0466:
			return 0x01;
	}

	return 0;
}

READ8_MEMBER(pacman_state::porky_port1_r)
{
	switch (space.device().safe_pc())
	{
		case 0x0034:
			return 0x01;
	}

	return 0;
}


/************************************
 *
 *  Rock-N-Roll Trivia (Part 2)
 *  questions roms and protection
 *  handlers
 *
 ************************************/

READ8_MEMBER(pacman_state::rocktrv2_prot1_data_r)
{
	return m_rocktrv2_prot_data[0] >> 4;
}

READ8_MEMBER(pacman_state::rocktrv2_prot2_data_r)
{
	return m_rocktrv2_prot_data[1] >> 4;
}

READ8_MEMBER(pacman_state::rocktrv2_prot3_data_r)
{
	return m_rocktrv2_prot_data[2] >> 4;
}

READ8_MEMBER(pacman_state::rocktrv2_prot4_data_r)
{
	return m_rocktrv2_prot_data[3] >> 4;
}

WRITE8_MEMBER(pacman_state::rocktrv2_prot_data_w)
{
	m_rocktrv2_prot_data[offset] = data;
}

WRITE8_MEMBER(pacman_state::rocktrv2_question_bank_w)
{
	m_rocktrv2_question_bank = data;
}

READ8_MEMBER(pacman_state::rocktrv2_question_r)
{
	UINT8 *question = memregion("user1")->base();

	return question[offset | (m_rocktrv2_question_bank * 0x8000)];
}


/************************************
 *
 *  Super ABC
 *
 ************************************/

/*
  This is a kit that upgrades an original Midway Pacman PCB to allow it to run many Pacman games.
  It includes two small PCBs and two PROMs. The large PCB plugs into the CPU socket at 6B and the
  Sync Bus Controller socket at 6D. The CPU that was in the socket at 6B plugs into the Super ABC PCB.

  The large PCB contains the following parts.....
  - Z80 CPU
  - 27C040 EPROM at U14
  - 82S123 PROM at U18
  - 28C16 2KB EEPROM at U17
  - several logic chips

  The small PCB contains just one 27C010 EPROM for the characters. There's a small cable that joins
  this PCB to the other large PCB.

  The two PROMs on the main board at 7F and 4A are also replaced with PROMs from the kit
*/

WRITE8_MEMBER(pacman_state::superabc_bank_w)
{
	// d4-d6: bank
	int bank = data >> 4 & 7;
	membank("bank1")->set_base(memregion("maincpu")->base() + bank * 0x10000 + 0x0000);
	membank("bank2")->set_base(memregion("maincpu")->base() + bank * 0x10000 + 0x4000);
	membank("bank3")->set_base(memregion("maincpu")->base() + bank * 0x10000 + 0xa000); // looks like a15 is not connected and a16-onwards is shifted 1 bit

	if (bank != m_charbank)
	{
		m_spritebank = bank;
		m_charbank = bank;
		m_bg_tilemap->mark_all_dirty();
	}
}


/************************************
 *
 *  Ms. Pac-Man
 *
 ************************************/

/*
  Ms. Pac-Man has an auxiliary PCB with ribbon cable that plugs into the Z-80 CPU socket of a Pac-Man main PCB. Also the
  graphics ROMs at 5E, 5F on the main board are replaced.

  The aux board contains three ROMs (two 2532 at U6, U7 and one 2716 at U5), a Z-80, and four PAL/HAL logic chips.

  The aux board logic decodes the Z-80 address and determines whether to enable the main board ROMs (containing Pac-Man
  code) or the aux board ROMs (containing Ms. Pac-Man code). Normally the Pac-Man ROMs reside at address 0x0000-0x3fff
  and are mirrored at 0x8000-0xbfff (Z-80 A15 is not used in Pac-Man). The aux board logic modifies the address map and
  enables the aux board ROMs for addresses 0x3000-0x3fff and 0x8000-0x97ff. Furthermore there are forty 8-byte "patch"
  regions which reside in the 0x0000-0x2fff address range. Any access to these patch addresses will disable the Pac-Man
  ROMs and enable the aux board ROM. Aux board ROM addresses 0x8000-0x81ef are mapped onto the patch regions. These
  patches typically insert jumps to new code above 0x8000.

  The aux board logic also acts as a software protection circuit which inhibits dumping of the ROMs (e.g., using a
  microprocessor emulator system). There are several "trap" address regions which enable and disable the decode
  functions. In order to properly operate as Ms. Pac-Man you must access one of the "latch set" trap addresses. This
  enables the decode. If a "latch clear" address is accessed then decode is disabled and all you get is Pac-Man. For
  more info see U.S. Patent 4,525,599 "Software protection methods and apparatus".

  The trap regions are 8 bytes in length starting on the following addresses:

  latch clear, decode disable
    0x0038
    0x03b0
    0x1600
    0x2120
    0x3ff0
    0x8000
    0x97f0

  latch set, decode enable
    0x3ff8

  Any memory access will trigger the trap behavior: instruction fetch, data read, data write. The latch clear addresses
  should never be accessed during normal Ms. Pac-Man operation, so when the circuitry detects an access it clears the
  latch and prevents any further dumping of the aux board ROMs.

  The Pac-Man self-test code does a checksum of the ROM 0x0000-0x2fff. This works because the checksum routine walks the
  ROM starting from the low address and hits the latch clear trap at 0x0038 prior to encountering any of the patch
  regions. The decode stays disabled for the rest of the checksum routine, and thus the checksum is calculated for the
  Pac-Man ROMs with no patches applied.

  During normal operation every VBLANK (60.6Hz) interrupt will fetch its interrupt vector from the 0x3ff8 trap region, so
  the latch is continually being enabled.

  David Widel points out that the Pac-Man pseudo-random number generator (RNG) routine at 0x2a23 might also access a
  trap region. This RNG is only used when the monsters are blue (after a power pellet has been eaten) and is used to
  select a "random" direction for the monsters to run away. The routine calculates a pointer address between
  0x0000-0x1fff and fetches the ROM value from that address. This value is the "random" number returned by the
  routine. During the blue mode only Pac-Man code is being executed, so a trap hit that clears the decode latch will
  have no effect on gameplay. Every VBLANK interrupt vector fetch re-enables the latch and you are back in Ms. Pac-Man
  mode.

  In a further attempt to thwart copying, the aux board ROMs have a simple encryption scheme: their address and data
  lines are bit flipped (i.e., wired in a nonstandard fashion). The specific bit flips were selected to minimize the
  vias required to lay out the aux PCB.


  History  (Originally contributed by Steve Golson of GCC, one of the original Developers of Ms. Pac-Man, with some
  -------   minor modifications to address availability of data.)

  During the summer of 1981, General Computer Corp. of Massachusetts developed a game called Crazy Otto. This game was
  intended to be sold as an enhancement kit for Pac-Man cabinets. The main character Crazy Otto had legs and blue
  eyes. Also the monsters have blue feet and antennae that bob up and down. New game play, mazes, music, and sounds were
  developed. Several new bonus characters (fruit) were added. Otto and his female counterpart appeared in three new
  animations, culminating in the arrival of JUNIOR, a baby Crazy Otto.

  In October 1981 this game was licensed to Midway, who owned the North American rights to produce Pac-Man. With Midway
  producing the game, the original Pac-Man character and name could be used. At first the game was called Super Pac-Man,
  but eventually the decision was made to use the female character as the protagonist, resulting in Ms. Pac-Man.

  The only differences between Crazy Otto and the final production Ms. Pac-Man are the characters themselves and related
  text strings. Game play, mazes, colors, fruits, sounds, music, animations are unchanged from original GCC Crazy Otto.
  Also the "marquee" attract mode was added to include the Midway logo and copyright string.

  The ROMs from several prototype versions of Crazy Otto have been recovered, documented and archived since that time but
  as of now are not available outside of the occasional public viewing and playing of a modified upright Ms. Pac-Man machine
  at a gaming convention, exposition or specific special industry events.

  Information shared regarding the known prototypes indicates they are dated from 10/12/81, 10/20/81 and 10/29/81.  Also
  two prototype versions of Super Pac-Man, one with old Pac-Man monsters and one with new Crazy Otto monsters are both
  dated 10/29/81.

  Early prototypes - those dated 10/12/81 and 10/20/81 - do not use the Ms. Pac-Man code patch scheme outlined above.
  Instead, all four Pac-Man ROMs are replaced, and one or two additional ROMs are provided at addresses above 0x8000.

  Later prototypes - those dated 10/29/81 - use the patch hardware, however the latch set/clear function is not implemented.
  Furthermore the ROM encryption bit flip is not used.
*/

#define mspacman_enable_decode_latch(m)  m.root_device().membank("bank1")->set_entry(1)
#define mspacman_disable_decode_latch(m) m.root_device().membank("bank1")->set_entry(0)

// any access to these ROM addresses disables the decoder, and all you see is the original Pac-Man code
READ8_MEMBER(pacman_state::mspacman_disable_decode_r_0x0038){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x0038]; }
READ8_MEMBER(pacman_state::mspacman_disable_decode_r_0x03b0){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x03b0]; }
READ8_MEMBER(pacman_state::mspacman_disable_decode_r_0x1600){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x1600]; }
READ8_MEMBER(pacman_state::mspacman_disable_decode_r_0x2120){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x2120]; }
READ8_MEMBER(pacman_state::mspacman_disable_decode_r_0x3ff0){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x3ff0]; }
READ8_MEMBER(pacman_state::mspacman_disable_decode_r_0x8000){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x8000]; }
READ8_MEMBER(pacman_state::mspacman_disable_decode_r_0x97f0){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x97f0]; }
WRITE8_MEMBER(pacman_state::mspacman_disable_decode_w){ mspacman_disable_decode_latch(machine()); }

// any access to these ROM addresses enables the decoder, and you'll see the Ms. Pac-Man code
READ8_MEMBER(pacman_state::mspacman_enable_decode_r_0x3ff8){ mspacman_enable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x3ff8+0x10000]; }
WRITE8_MEMBER(pacman_state::mspacman_enable_decode_w){ mspacman_enable_decode_latch(machine()); }


READ8_MEMBER(pacman_state::pacman_read_nop)
{
	// Return value of reading the bus with no devices enabled.
	// This seems to be common but more tests are needed. Ms Pacman reads bytes in sequence
	// until it hits a 0 for a delimiter, including empty areas.  It writes to "random"
	// addresses each time. This causes the maze to invert sometimes.  See code at $95c3 where
	// level($4e13)=134. DW
	// tests on exactly what determines the value returned have thus far proved inconclusive
	return 0xbf;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( pacman_map, AS_PROGRAM, 8, pacman_state )
	//A lot of games don't have an a15 at the cpu.  Generally only games with a cpu daughter board can access the full 32k of romspace.
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x8000) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READ(pacman_read_nop) AM_WRITENOP
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(irq_mask_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITENOP
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITENOP
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITENOP
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW2")
ADDRESS_MAP_END
// The Pacman code uses $5004 and $5005 for LED's and $5007 for coin lockout.  This hardware does not
// exist on any Pacman or Puckman board I have seen.  DW

static ADDRESS_MAP_START( patched_opcodes_map, AS_DECRYPTED_OPCODES, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x8000) AM_ROM AM_SHARE("patched_opcodes")
ADDRESS_MAP_END


static ADDRESS_MAP_START( birdiy_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x8000) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
//  AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READ(pacman_read_nop) AM_WRITENOP
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_WRITE(irq_mask_w)
//  AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
//  AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITENOP
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
//  AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_leds_w)
//  AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5080, 0x509f) AM_MIRROR(0xaf00) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x50a0, 0x50af) AM_MIRROR(0xaf00) AM_WRITEONLY AM_SHARE("spriteram2")
//  AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITENOP
//  AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITENOP
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( mspacman_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READ(pacman_read_nop) AM_WRITENOP
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(irq_mask_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITENOP
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITENOP
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITENOP
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW2")

	/* overlay decode enable/disable on top */
	AM_RANGE(0x0038, 0x003f) AM_READWRITE(mspacman_disable_decode_r_0x0038,mspacman_disable_decode_w)
	AM_RANGE(0x03b0, 0x03b7) AM_READWRITE(mspacman_disable_decode_r_0x03b0,mspacman_disable_decode_w)
	AM_RANGE(0x1600, 0x1607) AM_READWRITE(mspacman_disable_decode_r_0x1600,mspacman_disable_decode_w)
	AM_RANGE(0x2120, 0x2127) AM_READWRITE(mspacman_disable_decode_r_0x2120,mspacman_disable_decode_w)
	AM_RANGE(0x3ff0, 0x3ff7) AM_READWRITE(mspacman_disable_decode_r_0x3ff0,mspacman_disable_decode_w)
	AM_RANGE(0x3ff8, 0x3fff) AM_READWRITE(mspacman_enable_decode_r_0x3ff8,mspacman_enable_decode_w)
	AM_RANGE(0x8000, 0x8007) AM_READWRITE(mspacman_disable_decode_r_0x8000,mspacman_disable_decode_w)
	AM_RANGE(0x97f0, 0x97f7) AM_READWRITE(mspacman_disable_decode_r_0x97f0,mspacman_disable_decode_w)

	/* start with 0000-3fff and 8000-bfff mapped to the ROMs */
	AM_RANGE(0x4000, 0x7fff) AM_MIRROR(0x8000) AM_UNMAP
	AM_RANGE(0x0000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END


static ADDRESS_MAP_START( woodpek_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READ(pacman_read_nop) AM_WRITENOP
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(irq_mask_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITENOP
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITENOP
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITENOP
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW2")
	AM_RANGE(0x8000, 0xbfff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( alibaba_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READ(pacman_read_nop) AM_WRITENOP
	AM_RANGE(0x4c00, 0x4eef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ef0, 0x4eff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x4f00, 0x4fff) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x506f) AM_MIRROR(0xaf00) AM_WRITE(alibaba_sound_w)  /* the sound region is not contiguous */
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITEONLY AM_SHARE("spriteram2") /* actually at 5050-505f, here to point to free RAM */
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITENOP
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITENOP
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf00) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x50c1, 0x50c1) AM_MIRROR(0xaf00) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x50c2, 0x50c2) AM_MIRROR(0xaf00) AM_WRITE(irq_mask_w)
	AM_RANGE(0x50c3, 0x50ff) AM_MIRROR(0xaf00) AM_WRITENOP
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf00) AM_READ(alibaba_mystery_1_r)
	AM_RANGE(0x50c1, 0x50c1) AM_MIRROR(0xaf00) AM_READ(alibaba_mystery_2_r)
	AM_RANGE(0x50c2, 0x50ff) AM_MIRROR(0xaf00) AM_READ(pacman_read_nop)
	AM_RANGE(0x8000, 0x8fff) AM_ROM
	AM_RANGE(0x9000, 0x93ff) AM_MIRROR(0x0c00) AM_RAM
	AM_RANGE(0xa000, 0xa7ff) AM_MIRROR(0x1800) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( dremshpr_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4800, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(irq_mask_w)
//  AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITENOP /* unknown */
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
//  AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITENOP
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITENOP
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW2")
	AM_RANGE(0x8000, 0xbfff) AM_ROM

	/* vanvan: probably a leftover from development: the Sanritsu version writes
	   the color lookup table here, while the Karateko version writes garbage. */
	AM_RANGE(0xb800, 0xb87f) AM_WRITENOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( epos_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x8000) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READ(pacman_read_nop) AM_WRITENOP
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(irq_mask_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITENOP
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITENOP
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITENOP
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( s2650games_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x0fff) AM_MIRROR(0x8000) AM_ROMBANK("bank1")
	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0xe000) AM_WRITE(s2650games_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x1400, 0x141f) AM_MIRROR(0xe000) AM_WRITE(s2650games_scroll_w)
	AM_RANGE(0x1420, 0x148f) AM_MIRROR(0xe000) AM_WRITEONLY
	AM_RANGE(0x1490, 0x149f) AM_MIRROR(0xe000) AM_WRITEONLY AM_SHARE("s2650_spriteram")
	AM_RANGE(0x14a0, 0x14bf) AM_MIRROR(0xe000) AM_WRITE(s2650games_tilesbank_w) AM_SHARE("s2650_tileram")
	AM_RANGE(0x14c0, 0x14ff) AM_MIRROR(0xe000) AM_WRITEONLY
	AM_RANGE(0x1500, 0x1502) AM_MIRROR(0xe000) AM_WRITENOP
	AM_RANGE(0x1503, 0x1503) AM_MIRROR(0xe000) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x1504, 0x1506) AM_MIRROR(0xe000) AM_WRITENOP
	AM_RANGE(0x1507, 0x1507) AM_MIRROR(0xe000) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x1508, 0x155f) AM_MIRROR(0xe000) AM_WRITEONLY
	AM_RANGE(0x1560, 0x156f) AM_MIRROR(0xe000) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x1570, 0x157f) AM_MIRROR(0xe000) AM_WRITEONLY
	AM_RANGE(0x1586, 0x1587) AM_MIRROR(0xe000) AM_WRITENOP
	AM_RANGE(0x15c0, 0x15c0) AM_MIRROR(0xe000) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x15c7, 0x15c7) AM_MIRROR(0xe000) AM_WRITE(porky_banking_w)
	AM_RANGE(0x1500, 0x1500) AM_MIRROR(0xe000) AM_READ_PORT("IN0")
	AM_RANGE(0x1540, 0x1540) AM_MIRROR(0xe000) AM_READ_PORT("IN1")
	AM_RANGE(0x1580, 0x1580) AM_MIRROR(0xe000) AM_READ_PORT("DSW0")
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0xe000) AM_WRITE(s2650games_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x1c00, 0x1fef) AM_MIRROR(0xe000) AM_RAM
	AM_RANGE(0x1ff0, 0x1fff) AM_MIRROR(0xe000) AM_WRITEONLY AM_SHARE("spriteram")
	AM_RANGE(0x2000, 0x2fff) AM_MIRROR(0x8000) AM_ROMBANK("bank2")
	AM_RANGE(0x4000, 0x4fff) AM_MIRROR(0x8000) AM_ROMBANK("bank3")
	AM_RANGE(0x6000, 0x6fff) AM_MIRROR(0x8000) AM_ROMBANK("bank4")
ADDRESS_MAP_END


static ADDRESS_MAP_START( rocktrv2_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4c00, 0x4fff) AM_RAM
	AM_RANGE(0x5000, 0x5000) AM_WRITE(irq_mask_w)
	AM_RANGE(0x5001, 0x5001) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x5003, 0x5003) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5007, 0x5007) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x50c0, 0x50c0) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5fe0, 0x5fe3) AM_WRITE(rocktrv2_prot_data_w) AM_SHARE("rocktrv2_prot")
	AM_RANGE(0x5ff0, 0x5ff0) AM_WRITE(rocktrv2_question_bank_w)
	AM_RANGE(0x5000, 0x5000) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x507f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x5080) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50c0) AM_READ_PORT("DSW2")
	AM_RANGE(0x5fe0, 0x5fe0) AM_READ(rocktrv2_prot1_data_r)
	AM_RANGE(0x5fe4, 0x5fe4) AM_READ(rocktrv2_prot2_data_r)
	AM_RANGE(0x5fe8, 0x5fe8) AM_READ(rocktrv2_prot3_data_r)
	AM_RANGE(0x5fec, 0x5fec) AM_READ(rocktrv2_prot4_data_r)
	AM_RANGE(0x5fff, 0x5fff) AM_READ_PORT("DSW2")       /* DSW2 mirrored */
	AM_RANGE(0x6000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_READ(rocktrv2_question_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( bigbucks_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4c00, 0x4fff) AM_RAM
	AM_RANGE(0x5000, 0x5000) AM_WRITE(irq_mask_w)
	AM_RANGE(0x5001, 0x5001) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x5003, 0x5003) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5007, 0x5007) AM_WRITENOP /*?*/
	AM_RANGE(0x5040, 0x505f) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x50c0, 0x50c0) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x503f) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x507f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x50bf) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50ff) AM_READ_PORT("DSW2")
	AM_RANGE(0x5100, 0x5100) AM_WRITENOP /*?*/
	AM_RANGE(0x6000, 0x6000) AM_WRITE(bigbucks_bank_w)
	AM_RANGE(0x8000, 0x9fff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( mschamp_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READ(pacman_read_nop) AM_WRITENOP
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(irq_mask_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITENOP
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITENOP
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITENOP
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW2")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( superabc_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4800, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("28c16.u17") // nvram
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(irq_mask_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf3c) AM_WRITE(superabc_bank_w)
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITENOP
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITENOP
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ_PORT("IN0")
	AM_RANGE(0x5040, 0x5040) AM_MIRROR(0xaf3f) AM_READ_PORT("IN1")
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW1")
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_READ_PORT("DSW2")
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank2")
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank3")
ADDRESS_MAP_END


static ADDRESS_MAP_START( crushs_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x3fff) AM_MIRROR(0x8000) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x4400, 0x47ff) AM_MIRROR(0xa000) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x4800, 0x4bff) AM_MIRROR(0xa000) AM_READ(pacman_read_nop) AM_WRITENOP
	AM_RANGE(0x4c00, 0x4fef) AM_MIRROR(0xa000) AM_RAM
	AM_RANGE(0x4ff0, 0x4fff) AM_MIRROR(0xa000) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf38) AM_WRITE(irq_mask_w)
	AM_RANGE(0x5001, 0x5001) AM_MIRROR(0xaf38) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x5002, 0x5002) AM_MIRROR(0xaf38) AM_WRITENOP
	AM_RANGE(0x5003, 0x5003) AM_MIRROR(0xaf38) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x5004, 0x5005) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x5006, 0x5006) AM_MIRROR(0xaf38) AM_WRITENOP // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x5007, 0x5007) AM_MIRROR(0xaf38) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x5040, 0x505f) AM_MIRROR(0xaf00) AM_WRITENOP // doesn't use pacman sound hw
	AM_RANGE(0x5060, 0x506f) AM_MIRROR(0xaf00) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x5070, 0x507f) AM_MIRROR(0xaf00) AM_WRITENOP
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_WRITENOP
	AM_RANGE(0x50c0, 0x50c0) AM_MIRROR(0xaf3f) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x5000, 0x5000) AM_MIRROR(0xaf3f) AM_READ_PORT("IN0")
	AM_RANGE(0x5080, 0x5080) AM_MIRROR(0xaf3f) AM_READ_PORT("IN1")
ADDRESS_MAP_END



static ADDRESS_MAP_START( pengojpm_map, AS_PROGRAM, 8, pacman_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROM

	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(pacman_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8400, 0x87ff) AM_RAM_WRITE(pacman_colorram_w) AM_SHARE("colorram")

	AM_RANGE(0x8800, 0x8bff) AM_RAM
//  AM_RANGE(0x8800, 0x8bff) AM_READ(pacman_read_nop) AM_WRITENOP
	AM_RANGE(0x8c00, 0x8fef) AM_RAM
	AM_RANGE(0x8ff0, 0x8fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9000, 0x9000) AM_WRITE(irq_mask_w)
	AM_RANGE(0x9001, 0x9001) AM_DEVWRITE("namco", namco_device, pacman_sound_enable_w)
	AM_RANGE(0x9002, 0x9002) AM_WRITENOP
	AM_RANGE(0x9003, 0x9003) AM_WRITE(pacman_flipscreen_w)
	AM_RANGE(0x9004, 0x9005) AM_WRITENOP // AM_WRITE(pacman_leds_w)
	AM_RANGE(0x9006, 0x9006) AM_WRITENOP // AM_WRITE(pacman_coin_lockout_global_w)
	AM_RANGE(0x9007, 0x9007) AM_WRITE(pacman_coin_counter_w)
	AM_RANGE(0x9040, 0x905f) AM_DEVWRITE("namco", namco_device, pacman_sound_w)
	AM_RANGE(0x9060, 0x906f) AM_WRITEONLY AM_SHARE("spriteram2")
	AM_RANGE(0x9070, 0x907f) AM_WRITENOP
	AM_RANGE(0x9080, 0x9080) AM_WRITENOP
	AM_RANGE(0x90c0, 0x90c0) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x9000, 0x9000) AM_READ_PORT("IN0")
	AM_RANGE(0x9040, 0x9040) AM_READ_PORT("IN1")
	AM_RANGE(0x9080, 0x9080) AM_READ_PORT("DSW1")
	AM_RANGE(0x90c0, 0x90c0) AM_READ_PORT("DSW2")

	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END


/*************************************
 *
 *  Main CPU port handlers
 *
 *************************************/

static ADDRESS_MAP_START( writeport, AS_IO, 8, pacman_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(pacman_interrupt_vector_w)    /* Pac-Man only */
ADDRESS_MAP_END

static ADDRESS_MAP_START( vanvan_portmap, AS_IO, 8, pacman_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE("sn1", sn76496_device, write)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE("sn2", sn76496_device, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( dremshpr_portmap, AS_IO, 8, pacman_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x06, 0x07) AM_DEVWRITE("ay8910", ay8910_device, data_address_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( piranha_portmap, AS_IO, 8, pacman_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(piranha_interrupt_vector_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nmouse_portmap, AS_IO, 8, pacman_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(nmouse_interrupt_vector_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( theglobp_portmap, AS_IO, 8, pacman_state )
	AM_RANGE(0x00, 0xff) AM_READ(theglobp_decrypt_rom)   /* Switch protection logic */
	AM_IMPORT_FROM(writeport)
ADDRESS_MAP_END

static ADDRESS_MAP_START( acitya_portmap, AS_IO, 8, pacman_state )
	AM_RANGE(0x00, 0xff) AM_READ(acitya_decrypt_rom) /* Switch protection logic */
	AM_IMPORT_FROM(writeport)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mschamp_portmap, AS_IO, 8, pacman_state )
	AM_RANGE(0x00, 0x00) AM_READ(mschamp_kludge_r)
	AM_IMPORT_FROM(writeport)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bigbucks_portmap, AS_IO, 8, pacman_state )
	AM_RANGE(0x0000, 0xffff) AM_READ(bigbucks_question_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( s2650games_writeport, AS_IO, 8, pacman_state )
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_DEVWRITE("sn1", sn76496_device, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( drivfrcp_portmap, AS_IO, 8, pacman_state )
	AM_RANGE(0x00, 0x00) AM_READNOP
	AM_RANGE(0x01, 0x01) AM_READ(drivfrcp_port1_r)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
	AM_IMPORT_FROM(s2650games_writeport)
ADDRESS_MAP_END

static ADDRESS_MAP_START( _8bpm_portmap, AS_IO, 8, pacman_state )
	AM_RANGE(0x00, 0x00) AM_READNOP
	AM_RANGE(0x01, 0x01) AM_READ(_8bpm_port1_r)
	AM_RANGE(0xe0, 0xe0) AM_READNOP
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
	AM_IMPORT_FROM(s2650games_writeport)
ADDRESS_MAP_END

static ADDRESS_MAP_START( porky_portmap, AS_IO, 8, pacman_state )
	AM_RANGE(0x01, 0x01) AM_READ(porky_port1_r)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
	AM_IMPORT_FROM(s2650games_writeport)
ADDRESS_MAP_END

static ADDRESS_MAP_START( crushs_portmap, AS_IO, 8, pacman_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay8912", ay8912_device, data_address_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("DSW2")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("DSW1")
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( pacman )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_4WAY
	PORT_DIPNAME(0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(   0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(   0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(   0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW:7") // physical location for difficulty on puckman set is split-pad between R32 and C29
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Ghost Names" )           PORT_DIPLOCATION("SW:8") // physical location for ghostnames on puckman set is split-pad between C10 and C29
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Alternate ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


/* Ms. Pac-Man input ports are identical to Pac-Man, the only difference is */
/* the missing Ghost Names dip switch. */
static INPUT_PORTS_START( mspacman )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( pacmansp )
	PORT_INCLUDE( pacman )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Hard (Invalid)" ) // breaks the attract screen on this version
INPUT_PORTS_END

static INPUT_PORTS_START( pacuman )
	PORT_INCLUDE( pacman )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, "2C/1C, 1C/1C" )
	PORT_DIPSETTING(    0x01, "1C/2C, 1C/3C" )
	PORT_DIPSETTING(    0x02, "1C/2C, 1C/4C" )
	PORT_DIPSETTING(    0x00, "1C/1C, 1C/2C" )
INPUT_PORTS_END


static INPUT_PORTS_START( mspacpls )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) /* Also invincibility when playing */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) /* Also speed-up when playing */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( mschamp )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GAME")
	PORT_DIPNAME( 0x01, 0x01, "Game" )
	PORT_DIPSETTING(    0x01, "Champion Edition" )
	PORT_DIPSETTING(    0x00, "Super Zola Pac Gal" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( superabc )
	PORT_INCLUDE( pacman )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, "10000, Allow Continue" ) // also free game in Ultra Pac-Man
	PORT_DIPSETTING(    0x10, "15000, Allow Continue" ) // also free game in Ultra Pac-Man
	PORT_DIPSETTING(    0x20, "20000, No Continue" )    // also free life in Ultra Pac-Man
	PORT_DIPSETTING(    0x30, "None, No Continue" )     // also free life in Ultra Pac-Man
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( birdiy )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_4WAY
	PORT_DIPNAME(0x10, 0x10, "Rack Test (Cheat)" )  PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(2) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x20, "Skip Screen" )       PORT_DIPLOCATION("SW:7") /* Used to skip "Act" (AKA level)?? - How do you activate it? */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Stop Screen" )       PORT_DIPLOCATION("SW:8") /* Seems to have no function? */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( maketrax )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPNAME( 0x10, 0x10, "First Pattern" )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Teleport Holes" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( crush4 )
	PORT_INCLUDE( maketrax )

	PORT_START("GAME")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) // always select 2nd part of code
INPUT_PORTS_END

static INPUT_PORTS_START( korosuke )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPNAME( 0x10, 0x10, "First Pattern" )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x20, "Teleport Holes" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection */

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mbrush )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( paintrlr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )  /* Protection in Make Trax */

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( crushs )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* Audio Test? */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x10, 0x10, "Teleport Holes" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* These last dips seems doesn't works in the test input, why? */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 2C_8C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ponpoko )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	/* The 2nd player controls are used even in upright mode */
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "10000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x03, "50000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0f, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, "A 3/1 B 3/1" )
	PORT_DIPSETTING(    0x0e, "A 3/1 B 1/2" )
	PORT_DIPSETTING(    0x0f, "A 3/1 B 1/4" )
	PORT_DIPSETTING(    0x02, "A 2/1 B 2/1" )
	PORT_DIPSETTING(    0x0d, "A 2/1 B 1/1" )
	PORT_DIPSETTING(    0x07, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x0b, "A 2/1 B 1/5" )
	PORT_DIPSETTING(    0x0c, "A 2/1 B 1/6" )
	PORT_DIPSETTING(    0x01, "A 1/1 B 1/1" )
	PORT_DIPSETTING(    0x06, "A 1/1 B 4/5" )
	PORT_DIPSETTING(    0x05, "A 1/1 B 2/3" )
	PORT_DIPSETTING(    0x0a, "A 1/1 B 1/3" )
	PORT_DIPSETTING(    0x08, "A 1/1 B 1/5" )
	PORT_DIPSETTING(    0x09, "A 1/1 B 1/6" )
	PORT_DIPSETTING(    0x03, "A 1/2 B 1/2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  /* Most likely unused */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  /* Most likely unused */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  /* Most likely unused */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( eyes )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "50000" )
	PORT_DIPSETTING(    0x20, "75000" )
	PORT_DIPSETTING(    0x10, "100000" )
	PORT_DIPSETTING(    0x00, "125000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  /* Not accessed */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( mrtnt )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "75000" )
	PORT_DIPSETTING(    0x20, "100000" )
	PORT_DIPSETTING(    0x10, "125000" )
	PORT_DIPSETTING(    0x00, "150000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( lizwiz )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "75000" )
	PORT_DIPSETTING(    0x20, "100000" )
	PORT_DIPSETTING(    0x10, "125000" )
	PORT_DIPSETTING(    0x00, "150000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( theglobp )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // and start1
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) // and start2
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x14, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Very_Difficult ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( vanvan )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "20k and 100k" )
	PORT_DIPSETTING(    0x04, "40k and 140k" )
	PORT_DIPSETTING(    0x00, "70k and 200k" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )

	/* When all DSW2 are ON, there is no sprite collision detection */
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Invulnerability (Cheat)" ) PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      /* Missile effect */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )      /* Killer car is destroyed */
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )           /* Killer car is not destroyed */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( vanvank )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "20k and 100k" )
	PORT_DIPSETTING(    0x04, "40k and 140k" )
	PORT_DIPSETTING(    0x00, "70k and 200k" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_3C ) )

	/* When all DSW2 are ON, there is no sprite collision detection */
	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Invulnerability (Cheat)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      /* Missile effect */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )      /* Killer car is destroyed */
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )           /* Killer car is not destroyed */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( dremshpr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x00, "70000" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00,"Invulnerability (Cheat)")     /* turning this on crashes the emulated machine in an infinite loop once in a while */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( alibaba )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING( 0x10, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( jumpshot )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, "Time"  )
//  PORT_DIPSETTING(    0x00,  "2 Minutes"  )
	PORT_DIPSETTING(    0x02,  "2 Minutes" )
	PORT_DIPSETTING(    0x03,  "3 Minutes" )
	PORT_DIPSETTING(    0x01,  "4 Minutes"  )
	PORT_DIPNAME( 0x04, 0x04, "Player 1 Skin Tone" )
	PORT_DIPSETTING(    0x04, "Lighter" )
	PORT_DIPSETTING(    0x00, "Darker" )
	PORT_DIPNAME( 0x08, 0x08, "Player 2 Skin Tone" )
	PORT_DIPSETTING(    0x08, "Lighter" )
	PORT_DIPSETTING(    0x00, "Darker" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "2 Players Game" )
	PORT_DIPSETTING(    0x20, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( jumpshotp )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, "Time Limit"  )
//  PORT_DIPSETTING(    0x00,  "Short"  )
	PORT_DIPSETTING(    0x02,  "Short" )
	PORT_DIPSETTING(    0x03,  "Average" )
	PORT_DIPSETTING(    0x01,  "Above Average"   )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Players Skin Tone" )
	PORT_DIPSETTING(    0x08, "Lighter" )
	PORT_DIPSETTING(    0x00, "Darker" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "2 Players Game" )
	PORT_DIPSETTING(    0x20, "1 Credit" )
	PORT_DIPSETTING(    0x00, "2 Credits" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( shootbul )
	PORT_START("IN0")
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X  ) PORT_SENSITIVITY(50) PORT_KEYDELTA(25)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("IN1")
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Time"  )
	PORT_DIPSETTING(    0x01, "Short")
	PORT_DIPSETTING(    0x07, "Average" )
	PORT_DIPSETTING(    0x03, "Long" )
	PORT_DIPSETTING(    0x05, "Longer" )
	PORT_DIPSETTING(    0x06, "Longest" )
	PORT_DIPNAME( 0x08, 0x08, "Title Page Sounds"  )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* New Atlantic City Action / Board Walk Casino Inputs
   Annoyingly enough, you can't get into service mode on bwcasino if the
   cocktail mode is set. To test player 2's inputs, select Upright Mode on
   the dipswitches, and enter test mode. Now select cocktail mode and you
   can test everything. Weird. */

static INPUT_PORTS_START( bwcasino )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_COCKTAIL PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1e, 0x1e, "Hands per Game" )
	PORT_DIPSETTING(    0x1e, "3" )
	PORT_DIPSETTING(    0x1c, "4" )
	PORT_DIPSETTING(    0x1a, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPSETTING(    0x16, "7" )
	PORT_DIPSETTING(    0x14, "8" )
	PORT_DIPSETTING(    0x12, "9" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x0e, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0a, "13" )
	PORT_DIPSETTING(    0x08, "14" )
	PORT_DIPSETTING(    0x06, "15" )
	PORT_DIPSETTING(    0x04, "16" )
	PORT_DIPSETTING(    0x02, "17" )
	PORT_DIPSETTING(    0x00, "18" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* ATLANTIC CITY ACTION (acitya)
   Unlike "Boardwalk Casino", "Atlantic City Action" does not appear to
   have a cocktail mode, and uses service button connected differently to
   "Boardwalk" */

static INPUT_PORTS_START( acitya )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")  /* Test mode shows 6 selection DIP */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1e, 0x1e, "Hands per Game" )        PORT_DIPLOCATION("DSW1:2,3,4,5")
	PORT_DIPSETTING(    0x1e, "3" )
	PORT_DIPSETTING(    0x1c, "4" )
	PORT_DIPSETTING(    0x1a, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPSETTING(    0x16, "7" )
	PORT_DIPSETTING(    0x14, "8" )
	PORT_DIPSETTING(    0x12, "9" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x0e, "11" )
	PORT_DIPSETTING(    0x0c, "12" )
	PORT_DIPSETTING(    0x0a, "13" )
	PORT_DIPSETTING(    0x08, "14" )
	PORT_DIPSETTING(    0x06, "15" )
	PORT_DIPSETTING(    0x04, "16" )
	PORT_DIPSETTING(    0x02, "17" )
	PORT_DIPSETTING(    0x00, "18" )
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "DSW1:6")

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( nmouse )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME(0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(   0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "10000" )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

INPUT_PORTS_END

static INPUT_PORTS_START( woodpek )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "10000" )
	PORT_DIPSETTING(    0x20, "15000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( numcrash )
	PORT_INCLUDE( woodpek )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

INPUT_PORTS_END

static INPUT_PORTS_START( bigbucks )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Enable Category Adult Affairs" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Time to bet / answer" )
	PORT_DIPSETTING(    0x00, "15 sec. / 10 sec." )
	PORT_DIPSETTING(    0x01, "20 sec. / 15 sec." )
	PORT_DIPNAME( 0x02, 0x00, "Continue if player busts" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Show correct answer" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( drivfrcp )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


static INPUT_PORTS_START( 8bpm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Start 1 / P1 Button 1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Start 2 / P1 Button 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


static INPUT_PORTS_START( porky )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END


static INPUT_PORTS_START( rocktrv2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x1c, 0x10, "Questions Per Game" )
	PORT_DIPSETTING(    0x1c, "2" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x14, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x04, "8" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0x60, 0x60, "Clock Speed" )
	PORT_DIPSETTING(    0x60, "Beginner" )
	PORT_DIPSETTING(    0x40, "Intermed" )
	PORT_DIPSETTING(    0x20, "Pro" )
	PORT_DIPSETTING(    0x00, "Super - Pro" )
	PORT_DIPNAME( 0x80, 0x80,"Freeze Image" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Mode" )
	PORT_DIPSETTING(    0x01, "Amusement" )
	PORT_DIPSETTING(    0x00, "Credit" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "K.O. Switch" )
	PORT_DIPSETTING(    0x04, "Auto" )
	PORT_DIPSETTING(    0x00, "Manual" )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x70, "10000" )
	PORT_DIPSETTING(    0x60, "17500" )
	PORT_DIPSETTING(    0x50, "25000" )
	PORT_DIPSETTING(    0x40, "32500" )
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x20, "47500" )
	PORT_DIPSETTING(    0x10, "55000" )
	PORT_DIPSETTING(    0x00, "62500" )
	PORT_DIPNAME( 0x80, 0x80, "Music" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cannonbp )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME( "Select" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Display" )
	PORT_DIPSETTING(    0x03, "Scores and Progession Bars" )
	PORT_DIPSETTING(    0x01, "Scores only" )
	PORT_DIPSETTING(    0x02, "Progession Bars only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( pengojpm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout tilelayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,2),    /* 256 characters */
	2,  /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 }, /* bits are packed in groups of four */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 bytes */
};


static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,2),  /* 64 sprites */
	2,  /* 2 bits per pixel */
	{ 0, 4 },   /* the two bitplanes for 4 pixels are packed into one byte */
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    /* every sprite takes 64 bytes */
};


static const gfx_layout crush4_tilelayout =
{
	8,8, /* 8*8 characters */
	RGN_FRAC(1,4),
	2,  /* 2 bits per pixel */
	{ RGN_FRAC(1,2), RGN_FRAC(0,2)+4 },
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 }, /* bits are packed in groups of four */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8    /* every char takes 16 bytes */
};

static const gfx_layout crush4_spritelayout =
{
	16,16, /* 16*16 sprites */
	RGN_FRAC(1,4),
	2,  /* 2 bits per pixel */
	{ RGN_FRAC(1,2), RGN_FRAC(0,2)+4 },
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    /* every sprite takes 64 bytes */
};


static GFXDECODE_START( pacman )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, spritelayout, 0, 128 )
GFXDECODE_END


static GFXDECODE_START( s2650games )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0x4000, spritelayout, 0, 128 )
GFXDECODE_END


static GFXDECODE_START( superabc )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0x8000, spritelayout, 0, 128 )
GFXDECODE_END


static GFXDECODE_START( crush4 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, crush4_tilelayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, crush4_spritelayout, 0, 128 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( pacman, pacman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/6)
	MCFG_CPU_PROGRAM_MAP(pacman_map)
	MCFG_CPU_IO_MAP(writeport)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pacman_state, vblank_irq)
	MCFG_WATCHDOG_VBLANK_INIT(16)

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pacman)
	MCFG_PALETTE_ADD("palette", 128*4)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(pacman_state,pacman)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(pacman_state, screen_update_pacman)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(pacman_state,pacman)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("namco", NAMCO, MASTER_CLOCK/6/32)
	MCFG_NAMCO_AUDIO_VOICES(3)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pacmanp, pacman )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(patched_opcodes_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pengojpm, pacman )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(pengojpm_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( birdiy, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(birdiy_map)
	MCFG_CPU_IO_MAP(0)

	MCFG_VIDEO_START_OVERRIDE(pacman_state,birdiy)
MACHINE_CONFIG_END




static MACHINE_CONFIG_DERIVED( piranha, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(piranha_portmap)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( nmouse, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(nmouse_portmap)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mspacman, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mspacman_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( woodpek, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(woodpek_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( alibaba, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(alibaba_map)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( dremshpr, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dremshpr_map)
	MCFG_CPU_IO_MAP(dremshpr_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pacman_state,  vblank_nmi)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("namco")
	MCFG_SOUND_ADD("ay8910", AY8910, 14318000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( theglobp, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(epos_map)
	MCFG_CPU_IO_MAP(theglobp_portmap)

	MCFG_MACHINE_START_OVERRIDE(pacman_state,theglobp)
	MCFG_MACHINE_RESET_OVERRIDE(pacman_state,theglobp)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( acitya, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(epos_map)
	MCFG_CPU_IO_MAP(acitya_portmap)

	MCFG_MACHINE_START_OVERRIDE(pacman_state,acitya)
	MCFG_MACHINE_RESET_OVERRIDE(pacman_state,acitya)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( vanvan, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(dremshpr_map)
	MCFG_CPU_IO_MAP(vanvan_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pacman_state,  vblank_nmi)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(2*8, 34*8-1, 0*8, 28*8-1)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("namco")
	MCFG_SOUND_ADD("sn1", SN76496, 1789750)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
	MCFG_SOUND_ADD("sn2", SN76496, 1789750)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( bigbucks, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(bigbucks_map)
	MCFG_CPU_IO_MAP(bigbucks_portmap)
	MCFG_CPU_PERIODIC_INT_DRIVER(pacman_state, vblank_irq, 20*60)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 0*8, 28*8-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( s2650games, pacman )

	/* basic machine hardware */
	MCFG_DEVICE_REMOVE("maincpu")
	MCFG_CPU_ADD("maincpu", S2650, MASTER_CLOCK/6/2)    /* 2H */
	MCFG_CPU_PROGRAM_MAP(s2650games_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pacman_state,  s2650_interrupt)

	MCFG_GFXDECODE_MODIFY("gfxdecode", s2650games)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_DRIVER(pacman_state, screen_update_s2650games)

	MCFG_VIDEO_START_OVERRIDE(pacman_state,s2650games)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("namco")
	MCFG_SOUND_ADD("sn1", SN76496, MASTER_CLOCK/6)    /* 1H */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( drivfrcp, s2650games )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(drivfrcp_portmap)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( 8bpm, s2650games )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(_8bpm_portmap)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( porky, s2650games )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(porky_portmap)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( rocktrv2, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(rocktrv2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pacman_state,  irq0_line_hold)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 0*8, 28*8-1)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mschamp, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(mschamp_map)
	MCFG_CPU_IO_MAP(mschamp_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pacman_state,  vblank_irq)

	MCFG_MACHINE_RESET_OVERRIDE(pacman_state,mschamp)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( superabc, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(superabc_map)

	MCFG_NVRAM_ADD_0FILL("28c16.u17")

	MCFG_MACHINE_RESET_OVERRIDE(pacman_state,superabc)

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", superabc)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( crush4, mschamp )

	/* basic machine hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", crush4)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( crushs, pacman )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(crushs_map)
	MCFG_CPU_IO_MAP(crushs_portmap)

	/* sound hardware */
	MCFG_SOUND_ADD("ay8912", AY8912, 1789750)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( puckman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pm1_prg1.6e",  0x0000, 0x0800, CRC(f36e88ab) SHA1(813cecf44bf5464b1aed64b36f5047e4c79ba176) )
	ROM_LOAD( "pm1_prg2.6k",  0x0800, 0x0800, CRC(618bd9b3) SHA1(b9ca52b63a49ddece768378d331deebbe34fe177) )
	ROM_LOAD( "pm1_prg3.6f",  0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "pm1_prg4.6m",  0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "pm1_prg5.6h",  0x2000, 0x0800, CRC(6bf4f625) SHA1(afe72fdfec66c145b53ed865f98734686b26e921) )
	ROM_LOAD( "pm1_prg6.6n",  0x2800, 0x0800, CRC(a948ce83) SHA1(08759833f7e0690b2ccae573c929e2a48e5bde7f) )
	ROM_LOAD( "pm1_prg7.6j",  0x3000, 0x0800, CRC(b6289b26) SHA1(d249fa9cdde774d5fee7258147cd25fa3f4dc2b3) )
	ROM_LOAD( "pm1_prg8.6p",  0x3800, 0x0800, CRC(17a88c13) SHA1(eb462de79f49b7aa8adb0cc6d31535b10550c0ce) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pm1_chg1.5e",  0x0000, 0x0800, CRC(2066a0b7) SHA1(6d4ccc27d6be185589e08aa9f18702b679e49a4a) )
	ROM_LOAD( "pm1_chg2.5h",  0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pm1_chg3.5f",  0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "pm1_chg4.5j",  0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pm1-1.7f",     0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) ) // 82s123
	ROM_LOAD( "pm1-4.4a",     0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // 82s126

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "pm1-3.1m",     0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // 82s126
	ROM_LOAD( "pm1-2.3m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // 82s126 - timing - not used
ROM_END


ROM_START( puckmanb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "namcopac.6e",  0x0000, 0x1000, CRC(fee263b3) SHA1(87117ba5082cd7a615b4ec7c02dd819003fbd669) )
	ROM_LOAD( "namcopac.6f",  0x1000, 0x1000, CRC(39d1fc83) SHA1(326dbbf94c6fa2e96613dedb53702f8832b47d59) )
	ROM_LOAD( "namcopac.6h",  0x2000, 0x1000, CRC(02083b03) SHA1(7e1945f6eb51f2e51806d0439f975f7a2889b9b8) )
	ROM_LOAD( "namcopac.6j",  0x3000, 0x1000, CRC(7a36fe55) SHA1(01b4c38108d9dc4e48da4f8d685248e1e6821377) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( puckmanf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "namcopac.6e",  0x0000, 0x1000, CRC(fee263b3) SHA1(87117ba5082cd7a615b4ec7c02dd819003fbd669) )
	ROM_LOAD( "nampfast.6f",  0x1000, 0x1000, CRC(51b38db9) SHA1(0a796f93462aec4758c2aa1c1f34cd05bb10a178) )
	ROM_LOAD( "namcopac.6h",  0x2000, 0x1000, CRC(02083b03) SHA1(7e1945f6eb51f2e51806d0439f975f7a2889b9b8) )
	ROM_LOAD( "namcopac.6j",  0x3000, 0x1000, CRC(7a36fe55) SHA1(01b4c38108d9dc4e48da4f8d685248e1e6821377) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( puckmod )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "namcopac.6e",  0x0000, 0x1000, CRC(fee263b3) SHA1(87117ba5082cd7a615b4ec7c02dd819003fbd669) )
	ROM_LOAD( "namcopac.6f",  0x1000, 0x1000, CRC(39d1fc83) SHA1(326dbbf94c6fa2e96613dedb53702f8832b47d59) )
	ROM_LOAD( "namcopac.6h",  0x2000, 0x1000, CRC(02083b03) SHA1(7e1945f6eb51f2e51806d0439f975f7a2889b9b8) )
	ROM_LOAD( "npacmod.6j",   0x3000, 0x1000, CRC(7d98d5f5) SHA1(39939bcd6fb785d0d06fd29f0287158ab1267dfc) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( pacman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( pacmanf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacfast.6f",   0x1000, 0x1000, CRC(720dc3ee) SHA1(7224d7acfa0144b681c71d7734a7337189835361) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( pacmod )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pacmanh.6e",   0x0000, 0x1000, CRC(3b2ec270) SHA1(48fc607ad8d86249948aa377c677ae44bb8ad3da) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacmanh.6h",   0x2000, 0x1000, CRC(18811780) SHA1(ab34acaa3dbcafe8b20c2197f36641e471984487) )
	ROM_LOAD( "pacmanh.6j",   0x3000, 0x1000, CRC(5c96a733) SHA1(22ae15a6f088e7296f77c7487a350c4bd102f00e) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacmanh.5e",   0x0000, 0x1000, CRC(299fb17a) SHA1(ad97adc2122482a9018bacd137df9d8f409ddf85) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


// more recent bootleg board running a Spanish version of the game with larger ROMs and 'MADE IN GREECE' marking
// game has a high score name entry feature, with the name displayed next to 'El Super' on the title screen
ROM_START( pacmansp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",  0x0000, 0x4000, CRC(f2404b4d) SHA1(c9707ace0632e745fb7f1bf58cd606be5c7ee000) )
	ROM_CONTINUE(0x8000,0x4000)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "2.bin",    0x0000, 0x0800, CRC(7a75b696) SHA1(d25179f3ce20277a20d7159ff47d8b364bf4a8a3) )
	ROM_CONTINUE(0x1000,0x800)
	ROM_CONTINUE(0x0800,0x800)
	ROM_CONTINUE(0x1800,0x800)

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*
Ms.Pacman Bootleg Later Jamma Board

The game looks like the original, but the board is smallest than the original and only use two bigger 27256 eproms.

There are 4 proms in the board, one of them is a color prom that is different from the other sets, this cause a little different colors, but almost the same.

82s123  different from other sets
82s129-1.a9 == Ms. Pacman prom 82s126.1m
82s129-2.c9 == Ms. Pacman prom 82s126.3m
82s129-3.d1 == Ms. Pacman prom 82s126.4a

If you need more info about the board please write contact ricky2001 at AUMAP
*/

ROM_START( mspacmanbg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9.g5",  0x0000, 0x4000, CRC(97c64918) SHA1(a46ca4822e6cd7b9a5603d5d06a78fd489dc0b96) )
	ROM_CONTINUE(0x8000,0x4000) // blocks 5+6 are repeated twice in here

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "10.e5",    0x0000, 0x0800, CRC(f2c5da43) SHA1(6a6de2ecc313a11ad12d8d1712c05f923984f668) )
	ROM_CONTINUE(0x1000,0x800)
	ROM_CONTINUE(0x0800,0x800)
	ROM_CONTINUE(0x1800,0x800)
	ROM_IGNORE(0x6000) // this also contains regular pacman gfx, ignore them for now at least

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.h7",    0x0000, 0x0020, CRC(3545e7e9) SHA1(b866b02579438afb11296e5c53a32c6425bd044d) ) // slightly different to original (verified)
	ROM_LOAD( "82s129-3.d1",  0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // == 82s126.4a

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // == 82s126.1m
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */ // == 82s126.3m
ROM_END

ROM_START( mspacmanbgd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27256.01",  0x0000, 0x4000, CRC(5bcc195e) SHA1(9a82536f3a33c406b8cc9e076ccf21c61a009305) )
	ROM_CONTINUE(0x8000,0x4000) // blocks 5+6 are repeated twice in here

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "10.e5",    0x0000, 0x0800, CRC(f2c5da43) SHA1(6a6de2ecc313a11ad12d8d1712c05f923984f668) )
	ROM_CONTINUE(0x1000,0x800)
	ROM_CONTINUE(0x0800,0x800)
	ROM_CONTINUE(0x1800,0x800)
	ROM_IGNORE(0x6000) // this also contains regular pacman gfx, ignore them for now at least

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.h7",    0x0000, 0x0020, CRC(3545e7e9) SHA1(b866b02579438afb11296e5c53a32c6425bd044d) ) // slightly different to original (verified)
	ROM_LOAD( "82s129-3.d1",  0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // == 82s126.4a

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // == 82s126.1m
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */ // == 82s126.3m
ROM_END

ROM_START( mspacmanblt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "triunvi.1.bin",  0x0000, 0x4000, CRC(d9da2917) SHA1(d0b5705d69cc513ad546a16001dcde9dcc6175d3) )
	ROM_CONTINUE(0x8000,0x4000)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "triunvi.2.bin",    0x0000, 0x0800, CRC(e6446f49) SHA1(572964721d48ac082d3be62a0d04491e9dddc9b8) )
	ROM_CONTINUE(0x1000,0x800)
	ROM_CONTINUE(0x0800,0x800)
	ROM_CONTINUE(0x1800,0x800)

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.h7",    0x0000, 0x0020, CRC(3545e7e9) SHA1(b866b02579438afb11296e5c53a32c6425bd044d) )
	ROM_LOAD( "82s129-3.d1",  0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END

ROM_START( mspacmanbcc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "comecocos.bin",  0x0000, 0x4000, CRC(220eccae) SHA1(30183601d27e95f1e7a60983eaf2056505ab13d2) )
	ROM_CONTINUE(0x8000,0x4000) // blocks 5+6 are repeated twice in here

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "10.e5",    0x0000, 0x0800, CRC(f2c5da43) SHA1(6a6de2ecc313a11ad12d8d1712c05f923984f668) )
	ROM_CONTINUE(0x1000,0x800)
	ROM_CONTINUE(0x0800,0x800)
	ROM_CONTINUE(0x1800,0x800)
	ROM_IGNORE(0x6000) // this also contains regular pacman gfx, ignore them for now at least

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.h7",    0x0000, 0x0020, CRC(3545e7e9) SHA1(b866b02579438afb11296e5c53a32c6425bd044d) ) // slightly different to original (verified)
	ROM_LOAD( "82s129-3.d1",  0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // == 82s126.4a

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // == 82s126.1m
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */ // == 82s126.3m
ROM_END

/*

  Double Command Pac-Man game.
  ----------------------------

  Manufactured by Miky SRL.

  Double board system.
  Silkscreened "G-GA-1" and "G-GB-2"

  1x Z80. @ 6a

  1x 27256 (program) @ 6f

  4x 2716 (gfx) @ 5e, 5f, 5h, 5j

  3x HM1-7611-5 PROMs @ 1m, 3m, 4a

  1x Xtal (no marks) @ 7b
  1x DIP switches bank @ 9d (2-3-5 ON, 1-4-6-7-8 OFF).

  1x 2x22 edge connector (converted to JAMMA).
  1x 2x18 edge connector.

  1x sticker warning about use 4.75 V. max.


  WIRES PATCH:

  The program ROM is a 27256 (28-pins) inserted into a 24-pins socket @ location 6f.
  Aligned at the base, the upper part is out of the socket. some other legs are out of the socket.

  OUT LEGS:

  01 (Vpp) --> Tied to +5V
  02 (A12) --> Tied to Z80 pin 02 (A12).
  20 (/E)  --> Tied to pin 06 of 74LS139 @ location 7m, and pin 12 of 74LS42P @ location 7n
  22 (/G)
  23 (A11) --> Tied to Z80 pin 01 (A11).
  26 (A13) --> Tied to Z80 pin 03 (A13).
  27 (A14) --> Tied to Z80 pin 05 (A15), and pin 13 of 74LS42P @ location 7n.
  28 (Vcc) --> Tied to +5V


  Data lines look straight...


   Z80  | 27256
  ------+-------
   07 <---> 16
   08 <---> 15
   09 <---> 17
   10 <---> 18
   11 <---> 01/28 (+5V)
   12 <---> 13
   13 <---> 19
   14 <---> 11
   15 <---> 12


  Because EPROM A14 goes to Z80 A15, and Z80 A14 isn't connected to anything,
  the EPROM data should be offset as follows:


  Z80 address | EPROM offset
  ------------+-------------
   0000-3FFF  |  0000-3FFF
   4000-7FFF  |  0000-3FFF
   8000-BFFF  |  4000-7FFF
   C000-FFFF  |  4000-7FFF


  That's how the data is arranged, but the EPROM only drives the bus when the
  enable pin (/E) is asserted.



  About the bipolar PROMs...

  Harris Semiconductor
  Search: HM1-7611-5
  Fuse-Programmable PROM
  Number of Words=256
  Bits Per Word=4
  t(a) Max. (s) Access Time=60n
  Output Config=3-State
  Number of Chip Selects=2
  Program Voltage (V)=14
  Nom. Supp (V)=5.0
  Status=Discontinued
  Package=DIP
  Pins=16
  Military=N
  Technology=TTL

  http://www.bg-electronics.de/datenblaetter/Schaltkreise/HM-7611.pdf



  256*4 = 1024 bit

     +--\/--+
  A6 |01  16| Vcc
  A5 |02  15| A7
  A4 |03  14| /CE2
  A3 |04  13| /CE1
  A0 |05  12| D0
  A1 |06  11| D1
  A2 |07  10| D2
  GND|08  09| D3
     +------+

   NEC   Fujitsu Fairchild Intersil Mitsubishi
  ------ ------- --------- -------- ----------
  uPB423 MB7052   93427    IM5623    M54700
         MB7114

  Signetics     MMI    TI       Harris  Raytheon  AMD   National Intel  OKI
  ------------ ------ -------- -------- -------- ------ -------- ----- --------
  82S129       6301-1 TBP24S10 HM7611-5 29661    27S21  74S287   3621  MBL8521A
  82S129A      63S141 TBP34S10 HM7611A           27S21A
  82S27                                                          27S11


  Dumps and docs by Robbie.
  Credits: Roberto Fresca, ytsejam

*/

ROM_START( clubpacm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prg.6f",  0x0000, 0x4000, CRC(9baa78a2) SHA1(0f153b047028e8a065fbedd2a67d6601a8a4e384) )
	ROM_CONTINUE(0x8000,0x4000)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "12.5e",   0x0000, 0x0800, CRC(93933d1d) SHA1(fa38d2cb87e872bb9a3158a4df98f38360dc85ec) )
	ROM_LOAD( "14.5h",   0x0800, 0x0800, CRC(7409fbec) SHA1(f440f08ba026ae6172666e1bdc0894ce33bba420) )
	ROM_LOAD( "13.5f",   0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "15.5j",   0x1800, 0x0800, CRC(50c7477d) SHA1(c04ec282a8cb528df5e38ad750d12ee71612695d) )

	// Color PROMs have been dumped. They match the pacman/mspacman ones 
	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "n82s123n.7f",  0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "m7611.4a",     0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "m7611.1m",     0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "m7611.3m",     0x0100, 0x0100, CRC(0e307106) SHA1(6140b5339008dd3110cd5be2e2fb4813779dfe28) )    /* timing - not used */
ROM_END


ROM_START( hangly )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hangly.6e",    0x0000, 0x1000, CRC(5fe8610a) SHA1(d63eaebd85e10aa6c27bb7f47642dd403eeb6934) )
	ROM_LOAD( "hangly.6f",    0x1000, 0x1000, CRC(73726586) SHA1(cedddc5194589039dd8b64f07ab6320d7d4f55f9) )
	ROM_LOAD( "hangly.6h",    0x2000, 0x1000, CRC(4e7ef99f) SHA1(bd42e68b29b4d654dc817782ba00db69b7d2dfe2) )
	ROM_LOAD( "hangly.6j",    0x3000, 0x1000, CRC(7f4147e6) SHA1(0a7ac0e59d4d26fe52a2f4196c9f19e5ab677c87) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( hangly2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hangly.6e",    0x0000, 0x1000, CRC(5fe8610a) SHA1(d63eaebd85e10aa6c27bb7f47642dd403eeb6934) )
	ROM_LOAD( "hangly2.6f",   0x1000, 0x0800, CRC(5ba228bb) SHA1(b0e902cdf98bee72d6ec8069eec96adce3245074) )
	ROM_LOAD( "hangly2.6m",   0x1800, 0x0800, CRC(baf5461e) SHA1(754586a6449fd54a342f260e572c1cd60ab70815) )
	ROM_LOAD( "hangly.6h",    0x2000, 0x1000, CRC(4e7ef99f) SHA1(bd42e68b29b4d654dc817782ba00db69b7d2dfe2) )
	ROM_LOAD( "hangly2.6j",   0x3000, 0x0800, CRC(51305374) SHA1(6197b606a0eedb11135d9f4f7a89aecc23fb2d33) )
	ROM_LOAD( "hangly2.6p",   0x3800, 0x0800, CRC(427c9d4d) SHA1(917bc3d571cbdd24d88327ecabfb5b3f6d39af0a) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacmanh.5e",   0x0000, 0x1000, CRC(299fb17a) SHA1(ad97adc2122482a9018bacd137df9d8f409ddf85) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( hangly3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hm1.6e",   0x0000, 0x0800, CRC(9d027c4a) SHA1(88e094880057451a75cdc2ce9477403021813982) )
	ROM_LOAD( "hm5.6k",   0x0800, 0x0800, CRC(194c7189) SHA1(fd423bac2810015313841c7b935054565390fbd0) )
	ROM_LOAD( "hangly2.6f",   0x1000, 0x0800, CRC(5ba228bb) SHA1(b0e902cdf98bee72d6ec8069eec96adce3245074) ) /* hm2.6f */
	ROM_LOAD( "hangly2.6m",   0x1800, 0x0800, CRC(baf5461e) SHA1(754586a6449fd54a342f260e572c1cd60ab70815) ) /* hm6.6m */
	ROM_LOAD( "hm3.6h",   0x2000, 0x0800, CRC(08419c4a) SHA1(7e5001adad401080c788737c1d2349f218750442) )
	ROM_LOAD( "hm7.6n",   0x2800, 0x0800, CRC(ab74b51f) SHA1(1bce8933ed7807eb7aca9670df8994f8d1a8b5b7) )
	ROM_LOAD( "hm4.6j",   0x3000, 0x0800, CRC(5039b082) SHA1(086a6ac4742734167d283b1121fce29d8ac4a6cd) )
	ROM_LOAD( "hm8.6p",   0x3800, 0x0800, CRC(931770d7) SHA1(78fcf88e07ec5126c12c3297b62ca388809e947c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "hm9.5e",     0x0000, 0x0800, CRC(5f4be3cc) SHA1(eeb0e1e44549b99eab481d9ac016b4359e19fe30) )
	ROM_LOAD( "hm11.5h",    0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "hm10.5f",    0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "hm12.5j",    0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( popeyeman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pop1.6e",      0x0000, 0x0800, CRC(9d027c4a) SHA1(88e094880057451a75cdc2ce9477403021813982) )
	ROM_LOAD( "pop5.6k",      0x0800, 0x0800, CRC(194c7189) SHA1(fd423bac2810015313841c7b935054565390fbd0) )
	ROM_LOAD( "pop2.6f",      0x1000, 0x0800, CRC(5ba228bb) SHA1(b0e902cdf98bee72d6ec8069eec96adce3245074) )
	ROM_LOAD( "pop6.6m",      0x1800, 0x0800, CRC(baf5461e) SHA1(754586a6449fd54a342f260e572c1cd60ab70815) )
	ROM_LOAD( "pop3.6h",      0x2000, 0x0800, CRC(08419c4a) SHA1(7e5001adad401080c788737c1d2349f218750442) )
	ROM_LOAD( "pop7.6n",      0x2800, 0x0800, CRC(ab74b51f) SHA1(1bce8933ed7807eb7aca9670df8994f8d1a8b5b7) )
	ROM_LOAD( "pop4.6j",      0x3000, 0x0800, CRC(5039b082) SHA1(086a6ac4742734167d283b1121fce29d8ac4a6cd) )
	ROM_LOAD( "pop8.6p",      0x3800, 0x0800, CRC(931770d7) SHA1(78fcf88e07ec5126c12c3297b62ca388809e947c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pop9.5e",      0x0000, 0x0800, CRC(b569c4c1) SHA1(34a09bcb6eb08375cd5e9ce0aa66b23d60489f92) )
	ROM_LOAD( "pop11.5h",     0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pop10.5f",     0x1000, 0x0800, CRC(014fb5a4) SHA1(7779f0f0d11027adf3b9175af26d53f1e3a54f29) )
	ROM_LOAD( "pop12.5j",     0x1800, 0x0800, CRC(21b91c64) SHA1(4818194ecc18b5df159f3799ea93c911fd4b9556) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( crockman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.6e",        0x0000, 0x0800, CRC(2c0fa0ab) SHA1(37680e4502771ae69d51d07ce43f65b9b2dd2a49) )
	ROM_LOAD( "p5.6k",        0x0800, 0x0800, CRC(afeca2f1) SHA1(1e6d6c75eeb3a354ce2dc88da62caf9e7d53d0cb) )
	ROM_LOAD( "p2.6f",        0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "p6.6m",        0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "p3.6h",        0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "p7.6n",        0x2800, 0x0800, CRC(93f344c5) SHA1(987c7fa18a774a47c045fa1dc7dff37457cb8983) )
	ROM_LOAD( "p4.6j",        0x3000, 0x0800, CRC(bed4a077) SHA1(39ac1d4d2acf4752ff7f9839f8f0d1974e023fab) )
	ROM_LOAD( "p8.6p",        0x3800, 0x0800, CRC(800be41e) SHA1(6f40e741d95c2cfe1b217f1061da3497b4c2a153) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "p9.5e",        0x0000, 0x0800, CRC(a10218c4) SHA1(fa48f780ddbef37d0ef58f301c8783668843eab6) )
	ROM_LOAD( "p11.5h",       0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "p10.5f",       0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "p12.5j",       0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( puckmanh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pm01.6e",      0x0000, 0x1000, CRC(5fe8610a) SHA1(d63eaebd85e10aa6c27bb7f47642dd403eeb6934) )
	ROM_LOAD( "pm02.6f",      0x1000, 0x1000, CRC(61d38c6c) SHA1(1406aacdc9c8a3776e5853d214380ad3124408f4) )
	ROM_LOAD( "pm03.6h",      0x2000, 0x1000, CRC(4e7ef99f) SHA1(bd42e68b29b4d654dc817782ba00db69b7d2dfe2) )
	ROM_LOAD( "pm04.6j",      0x3000, 0x1000, CRC(8939ddd2) SHA1(cf769bb34f711cfd0ee75328cd5dc07442f88607) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pm9.5e",       0x0000, 0x0800, CRC(2229ab07) SHA1(56000ed5009ae60c7f0498b5cac1b06da6ae270e) )
	ROM_LOAD( "pm11.5h",      0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pm10.5f",      0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "pm12.5j",      0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( newpuckx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "puckman.6e",   0x0000, 0x1000, CRC(a8ae23c5) SHA1(1481a4f083b563350744f9d25b1bcd28073875d6) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "puckman.6h",   0x2000, 0x1000, CRC(197443f8) SHA1(119aab12a9e1052c7b9a1f81e563740b41429a8c) )
	ROM_LOAD( "puckman.6j",   0x3000, 0x1000, CRC(2e64a3ba) SHA1(f86a921173f32211b18d023c2701664d13ae23be) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacman.5e",    0x0000, 0x1000, CRC(0c944964) SHA1(06ef227747a440831c9a3a613b76693d52a2f0a9) )
	ROM_LOAD( "pacman.5f",    0x1000, 0x1000, CRC(958fedf9) SHA1(4a937ac02216ea8c96477d4a15522070507fb599) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( pacheart )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pacheart1.6e", 0x0000, 0x0800, CRC(d844b679) SHA1(c4486198b3126bb8e05a308c53787e51065f77ae) )
	ROM_LOAD( "pacheart.pg2", 0x0800, 0x0800, CRC(b9152a38) SHA1(b6be2cb6bc7dd123503eb6bf1165dd1c99456813) )
	ROM_LOAD( "pacheart2.6f", 0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "pacheart.pg4", 0x1800, 0x0800, CRC(842d6574) SHA1(40e32d09cc8d701eb318716493a68cf3f95d3d6d) )
	ROM_LOAD( "pacheart3.6h", 0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "pacheart7.6n", 0x2800, 0x0800, CRC(888f3c3e) SHA1(c2b5917bf13071131dd53ea76f0da86706db2d80) )
	ROM_LOAD( "pacheart.pg7", 0x3000, 0x0800, CRC(f5265c10) SHA1(9a320790d7a03fd6192a92d30b3e9c754bbc6a9d) )
	ROM_LOAD( "pacheart.pg8", 0x3800, 0x0800, CRC(1a21a381) SHA1(d5367a327d19fb57ba5e484bd4fda1b10953c040) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacheart.ch1", 0x0000, 0x0800, CRC(c62bbabf) SHA1(f6f28ae33c2ab274105283b22b49ad243780a95e) )
	ROM_LOAD( "chg2",         0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pacheart.ch3", 0x1000, 0x0800, CRC(ca8c184c) SHA1(833aa845824ed80777b62f03df36a920ad7c3656) )
	ROM_LOAD( "pacheart.ch4", 0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END

ROM_START( pacmanjpm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jpm1",      0x0000, 0x0800, CRC(2c0fa0ab) SHA1(37680e4502771ae69d51d07ce43f65b9b2dd2a49) )
	ROM_LOAD( "jpm5",      0x0800, 0x0800, CRC(afeca2f1) SHA1(1e6d6c75eeb3a354ce2dc88da62caf9e7d53d0cb) )
	ROM_LOAD( "jpm2",      0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "jpm6",      0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "jpm3",      0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "jpm7",      0x2800, 0x0800, CRC(93f344c5) SHA1(987c7fa18a774a47c045fa1dc7dff37457cb8983) )
	ROM_LOAD( "jpm4",      0x3000, 0x0800, CRC(258580a2) SHA1(c594329963215971dba7cd857dfde5323f610dba) )
	ROM_LOAD( "jpm8",      0x3800, 0x0800, CRC(b4d7ee8c) SHA1(38dfea4443f13c15ed17e0eb8f7ca24ce14d6ca8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "jpm9",       0x0000, 0x0800, CRC(2066a0b7) SHA1(6d4ccc27d6be185589e08aa9f18702b679e49a4a) )
	ROM_LOAD( "jpm11",      0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "jpm10",      0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "jpm12",      0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END

// this bootleg on Pacman hardware has half as many tiles as the original and some gfx / animations
// have been reduced / removed to fit
ROM_START( pengojpm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pengo5.bin",      0x0800, 0x0800, CRC(7458f816) SHA1(bc5d3a4f374d5b93aefa7378eae1492956cca6af) )
	ROM_CONTINUE(0x000,0x800)
	ROM_LOAD( "pengo1.bin",      0x4000, 0x1000, CRC(1519d59b) SHA1(13b99780fcccac61b16201500e309c9b442406c8) )
	ROM_LOAD( "pengo2.bin",      0x5000, 0x1000, CRC(1b90c32c) SHA1(1761add93d71d29840b1462b9747a3d463b7148d) )
	ROM_LOAD( "pengo3.bin",      0x6000, 0x1000, CRC(aff4fba1) SHA1(8083352b3a2a4a70b2db778074826a55177e06ab) )
	ROM_LOAD( "pengo4.bin",      0x7000, 0x1000, CRC(1628eb6d) SHA1(44bd9d30828bb2440599fcd4a46f20fd798c24d5) )


	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pengoa.bin",      0x0000, 0x0800, CRC(ad88978a) SHA1(a568baf751753660223958b722980f031310eba1) )
	ROM_LOAD( "pengoc.bin",      0x0800, 0x0800, CRC(cb208b9f) SHA1(63b64b52c9c3e18b2d2823e79095160fb1a71f00) )
	ROM_LOAD( "pengob.bin",      0x1000, 0x0800, CRC(bae319a3) SHA1(88f0562ba2501f16ddfaffb12c4d1c00315f4225) )
	ROM_LOAD( "pengod.bin",      0x1800, 0x0800, CRC(5a5190e8) SHA1(caf49a348c649fbf959e97c632832bdb5bc068be) )

	// proms are unknown, using pengo ones
	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78",    0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) /* color palette */
	ROM_LOAD( "pr1634.88",    0x0020, 0x0400, CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) /* color lookup */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END

ROM_START( pengopac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pengopac.0",      0x0000, 0x0800, CRC(56f1718b) SHA1(016148f914117947b406bff8419390a1ffe92f7a) ) // 99.121094% to above set
	ROM_LOAD( "pengopac.1",      0x0800, 0x0800, CRC(013d450e) SHA1(f17313c5a458de2a4cb491707c5ddeef0fa40ef3) )
	ROM_LOAD( "pengopac.4",      0x4000, 0x1000, CRC(1519d59b) SHA1(13b99780fcccac61b16201500e309c9b442406c8) )
	ROM_LOAD( "pengopac.5",      0x5000, 0x1000, CRC(1b90c32c) SHA1(1761add93d71d29840b1462b9747a3d463b7148d) )
	ROM_LOAD( "pengopac.6",      0x6000, 0x2000, CRC(8d2994ee) SHA1(1f16b32c4574107a4a15d40113b966581b374a81) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pengopac.5e",      0x0000, 0x0800, CRC(ad88978a) SHA1(a568baf751753660223958b722980f031310eba1) )
	ROM_LOAD( "pengopac.5f",      0x0800, 0x0800, CRC(cb208b9f) SHA1(63b64b52c9c3e18b2d2823e79095160fb1a71f00) )
	ROM_LOAD( "pengopac.5h",      0x1000, 0x0800, CRC(bae319a3) SHA1(88f0562ba2501f16ddfaffb12c4d1c00315f4225) )
	ROM_LOAD( "pengopac.5j",      0x1800, 0x0800, CRC(5a5190e8) SHA1(caf49a348c649fbf959e97c632832bdb5bc068be) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78",    0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) /* color palette */
	ROM_LOAD( "pengopac.4a",    0x0020, 0x0100, CRC(ef283be2) SHA1(6d616348c06d08f3ffbe875a40036a2453cb45ad) ) /* color lookup */

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END

ROM_START( bucaner )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "buc1.6e",      0x0000, 0x0800, CRC(2c0fa0ab) SHA1(37680e4502771ae69d51d07ce43f65b9b2dd2a49) )
	ROM_LOAD( "buc5.6k",      0x0800, 0x0800, CRC(afeca2f1) SHA1(1e6d6c75eeb3a354ce2dc88da62caf9e7d53d0cb) )
	ROM_LOAD( "buc2.6f",      0x1000, 0x0800, CRC(6b53ada9) SHA1(a905688b389bfbc6792965d8f3d5bb1b9f0f4ec6) )
	ROM_LOAD( "buc6.6m",      0x1800, 0x0800, CRC(35f3ca84) SHA1(3da7336caa0742ea79f1e0e8f6b80f8560507a33) )
	ROM_LOAD( "buc3.6h",      0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "buc7.6n",      0x2800, 0x0800, CRC(888f3c3e) SHA1(c2b5917bf13071131dd53ea76f0da86706db2d80) )
	ROM_LOAD( "buc4.6j",      0x3000, 0x0800, CRC(292de161) SHA1(09b439c301d7bedb76c1590e937e9d8d5e24a048) )
	ROM_LOAD( "buc8.6p",      0x3800, 0x0800, CRC(884af858) SHA1(bad8b0dfdaf221dff0f84928f20873b01a168be5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "buc9.5e",      0x0000, 0x0800, CRC(4060c077) SHA1(78d4aa5243246f73533fc0886438dc1fa6f7ebe5) )
	ROM_LOAD( "buc11.5h",     0x0800, 0x0800, CRC(e3861283) SHA1(61cf8ed24902910e98438d9e2e2745f226ad2a13) )
	ROM_LOAD( "buc10.5f",     0x1000, 0x0800, CRC(09f66dec) SHA1(2d3649341fed19bac15ec274f7d747de46a3edb2) )
	ROM_LOAD( "buc12.5j",     0x1800, 0x0800, CRC(653314e7) SHA1(c466a421917b3502e9115ebda1b2d11f7f586de8) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END

ROM_START( joyman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "joy1.6e",      0x0000, 0x0800, CRC(d844b679) SHA1(c4486198b3126bb8e05a308c53787e51065f77ae) )
	ROM_LOAD( "joy5.6k",      0x0800, 0x0800, CRC(ab9c8f29) SHA1(3753b8609c30d85d89acf745cf9303b77be440fd) )
	ROM_LOAD( "joy2.6f",      0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "joy6.6m",      0x1800, 0x0800, CRC(b3c8d32e) SHA1(8b336fca1300820308cd5c4efc60bf2ba4199302) )
	ROM_LOAD( "joy3.6h",      0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "joy7.6n",      0x2800, 0x0800, CRC(888f3c3e) SHA1(c2b5917bf13071131dd53ea76f0da86706db2d80) )
	ROM_LOAD( "joy4.6j",      0x3000, 0x0800, CRC(00b553f8) SHA1(57f2e4a6da9f00935fead447b2123a8b95e5d672) )
	ROM_LOAD( "joy8.6p",      0x3800, 0x0800, CRC(5d5ce992) SHA1(ced7ed39cfc7ec7b2c0459e275577976109ee82f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "joy9.5e",      0x0000, 0x0800, CRC(39b557bc) SHA1(0f602ec84cb25fced89699e430b95b5ae93c83bd) )
	ROM_LOAD( "joy11.5h",     0x0800, 0x0800, CRC(33e0289e) SHA1(c1b910bdc61e560a8c34298deb11401f718e7330) )
	ROM_LOAD( "joy10.5f",     0x1000, 0x0800, CRC(338771a6) SHA1(7cd68cc428986255d0de29aae894900519e7fda5) )
	ROM_LOAD( "joy12.5j",     0x1800, 0x0800, CRC(f4f0add5) SHA1(d71c54ef55a755ec1316623d183b4f615ef7c055) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END


ROM_START( piranha )
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "pir1.bin", 0x0000, 0x0800, CRC(69a3e6ea) SHA1(c54e5d039a03d3cbee7a5e21bf1e23f4fd913ea6) )
	ROM_LOAD( "pir5.bin", 0x0800, 0x0800, CRC(245e753f) SHA1(4c1183b8449e4e7995f81079953fe0e251251c60) )
	ROM_LOAD( "pir2.bin", 0x1000, 0x0800, CRC(62cb6954) SHA1(0e01c8463b130ab5518ce23368ad028c86cd0a32) )
	ROM_LOAD( "pir6.bin", 0x1800, 0x0800, CRC(cb0700bc) SHA1(1f5e91791ea25eb58d26b9627e98e0b6c1d9becf) )
	ROM_LOAD( "pir3.bin", 0x2000, 0x0800, CRC(843fbfe5) SHA1(6671a3c55ef70447f2a127438e0c39857f8bf6b1) )
	ROM_LOAD( "pir7.bin", 0x2800, 0x0800, CRC(73084d5e) SHA1(cb04a4c9dbf1672ddf478d2fe92b0ffd0159bb9e) )
	ROM_LOAD( "pir4.bin", 0x3000, 0x0800, CRC(4cdf6704) SHA1(97af8bbd08896dffd73e359ec46843dd673c4c9c) )
	ROM_LOAD( "pir8.bin", 0x3800, 0x0800, CRC(b86fedb3) SHA1(f5eaf7ccc1ecaa2417bcc077561efca8e7cb691a) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "pir9.bin",  0x0000, 0x0800, CRC(0f19eb28) SHA1(0335189a06be01b97ca376d3682ed54df9b121e8) )
	ROM_LOAD( "pir11.bin", 0x0800, 0x0800, CRC(5f8bdabe) SHA1(eb6a0515a381a885b087d165aaefb0277a223715) )
	ROM_LOAD( "pir10.bin", 0x1000, 0x0800, CRC(d19399fb) SHA1(c0a75a08f77adb9d0010511c4b6ea99324c33c50) )
	ROM_LOAD( "pir12.bin", 0x1800, 0x0800, CRC(cfb4403d) SHA1(1642a4917be0621ebf5f705c7f68a2b75d1c78d3) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "piranha.4a", 0x0020, 0x0100, CRC(08c9447b) SHA1(5e4fbfcc7179fc4b1436af9bb709ffc381479315) )

	ROM_REGION( 0x0200, "namco", 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END

ROM_START( piranhao )
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "p1.bin", 0x0000, 0x0800, CRC(c6ce1bfc) SHA1(da145d67331cee292654a185fb09e773dd9d40cd) )
	ROM_LOAD( "p5.bin", 0x0800, 0x0800, CRC(a2655a33) SHA1(2253dcf5c8cbe278118aa1569cf456b13d8cf029) )
	ROM_LOAD( "pir2.bin", 0x1000, 0x0800, CRC(62cb6954) SHA1(0e01c8463b130ab5518ce23368ad028c86cd0a32) )
	ROM_LOAD( "pir6.bin", 0x1800, 0x0800, CRC(cb0700bc) SHA1(1f5e91791ea25eb58d26b9627e98e0b6c1d9becf) )
	ROM_LOAD( "pir3.bin", 0x2000, 0x0800, CRC(843fbfe5) SHA1(6671a3c55ef70447f2a127438e0c39857f8bf6b1) )
	ROM_LOAD( "pir7.bin", 0x2800, 0x0800, CRC(73084d5e) SHA1(cb04a4c9dbf1672ddf478d2fe92b0ffd0159bb9e) )
	ROM_LOAD( "p4.bin", 0x3000, 0x0800, CRC(9363a4d1) SHA1(4cb4a86d92a1f9bf233cac01aa266485a8bb7a34) )
	ROM_LOAD( "p8.bin", 0x3800, 0x0800, CRC(2769979c) SHA1(581592da26199b325de51791ddab66b474ab0413) )

	ROM_REGION( 0x2000, "gfx1" , 0 )
	ROM_LOAD( "p9.bin",  0x0000, 0x0800, CRC(94eb7563) SHA1(c99741ce1aebdfb89628fbfaecf5ae6b2719a0ca) )
	ROM_LOAD( "p11.bin", 0x0800, 0x0800, CRC(a3606973) SHA1(72297e1a33102c6a48b4c65f2a0b9bfc75a2df36) )
	ROM_LOAD( "p10.bin", 0x1000, 0x0800, CRC(84165a2c) SHA1(95b24620fbf9bd0ec4dd2aeeb6d9305bd475dce2) )
	ROM_LOAD( "p12.bin", 0x1800, 0x0800, CRC(2699ba9e) SHA1(b91ff586defe65b200bea5ade7374c2c7579cd80) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "piranha.4a", 0x0020, 0x0100, CRC(08c9447b) SHA1(5e4fbfcc7179fc4b1436af9bb709ffc381479315) )

	ROM_REGION( 0x0200, "namco", 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END

ROM_START( piranhah )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pr1.cpu",      0x0000, 0x1000, CRC(bc5ad024) SHA1(a3ed781b514a1068b24a7146a28f0a2adfaa2719) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pr3.cpu",      0x2000, 0x1000, CRC(473c379d) SHA1(6e7985367c3e544b4cb98ba8291908df88eafe7f) )
	ROM_LOAD( "pr4.cpu",      0x3000, 0x1000, CRC(63fbf895) SHA1(d328bf3b8f307fb774614834edec211117148e64) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pr5.cpu",      0x0000, 0x0800, CRC(3fc4030c) SHA1(5e45f0c19cf96daa17afd2fa1c628d7ac7f4a79c) )
	ROM_LOAD( "pr7.cpu",      0x0800, 0x0800, CRC(30b9a010) SHA1(b0ba8b6cd430feb32d11d092e1959b9f5d240f1b) )
	ROM_LOAD( "pr6.cpu",      0x1000, 0x0800, CRC(f3e9c9d5) SHA1(709a75b2457f21f0f1a3d9e7f4c8579468ee5cad) )
	ROM_LOAD( "pr8.cpu",      0x1800, 0x0800, CRC(133d720d) SHA1(8af75ed9e115a996379acedd44d0c09332ec5a03) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( abscam )
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "as0.bin", 0x0000, 0x0800, CRC(0b102302) SHA1(97f6399182db4f84efe482bf3a710aa45ca104ec) )
	ROM_LOAD( "as4.bin", 0x0800, 0x0800, CRC(3116a8ec) SHA1(259169bcc8fbe9fc73ca5100c3835a233351f530) )
	ROM_LOAD( "as1.bin", 0x1000, 0x0800, CRC(bc0281e0) SHA1(bcc6d63ede728d9b29f903489bfa80d94ec5cc00) )
	ROM_LOAD( "as5.bin", 0x1800, 0x0800, CRC(428ee2e8) SHA1(1477d1a86b32483ac0fdeea93512f517c9f66ce2) )
	ROM_LOAD( "as2.bin", 0x2000, 0x0800, CRC(e05d46ad) SHA1(87da57dbbe6ab5e1dd005fd68a982f1df917459c) )
	ROM_LOAD( "as6.bin", 0x2800, 0x0800, CRC(3ae9a8cb) SHA1(72896ad32cbdde90793788182958a943e35672f9) )
	ROM_LOAD( "as3.bin", 0x3000, 0x0800, CRC(b39eb940) SHA1(e144a1553c76ddee1c22ad1ed0cca241c2d03998) )
	ROM_LOAD( "as7.bin", 0x3800, 0x0800, CRC(16cf1c67) SHA1(0015fe64d476de87f1a030e7f2e735380dfcfd41) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "as8.bin",  0x0000, 0x0800, CRC(61daabe5) SHA1(00503916d1d1011afe68898e3416718c0e63a298) )
	ROM_LOAD( "as10.bin", 0x0800, 0x0800, CRC(81d50c98) SHA1(6b61c666f68b5948e4facb8bac1378f986f993a7) )
	ROM_LOAD( "as9.bin", 0x1000, 0x0800, CRC(a3bd1613) SHA1(c59bb0a4d1fa5cbe596f41ee7b1a4a661ab5614b) )
	ROM_LOAD( "as11.bin", 0x1800, 0x0800, CRC(9d802b68) SHA1(4e8f37c2faedcfce91221a34c14f6490d578c80a) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "as4a.bin", 0x0020, 0x0100, CRC(1605b324) SHA1(336fce22caedbe69bcba9cea2b43e00f6f8e8067) )

	ROM_REGION( 0x0200, "namco", 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END


ROM_START( ctrpllrp )
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "c1.bin", 0x0000, 0x0800, CRC(9d027c4a) SHA1(88e094880057451a75cdc2ce9477403021813982) )
	ROM_LOAD( "c5.bin", 0x0800, 0x0800, CRC(f39846d3) SHA1(bc1137a45898210523cf8da1e6a2425e7c322884) )
	ROM_LOAD( "c2.bin", 0x1000, 0x0800, CRC(afa149a8) SHA1(207b842854ac9e015e12a2dae41105438cda1df9) )
	ROM_LOAD( "c6.bin", 0x1800, 0x0800, CRC(baf5461e) SHA1(754586a6449fd54a342f260e572c1cd60ab70815) )
	ROM_LOAD( "c3.bin", 0x2000, 0x0800, CRC(6bb282a1) SHA1(a96f25dc0f49ebe7e528e3297a112d778c6c3030) )
	ROM_LOAD( "c7.bin", 0x2800, 0x0800, CRC(fa2140f5) SHA1(123d31e653e8af78c6153702eca2e136c427ed64) )
	ROM_LOAD( "c4.bin", 0x3000, 0x0800, CRC(86c91e0e) SHA1(52af6a3af5b1363859f790470ca5860ef2a08566) )
	ROM_LOAD( "c8.bin", 0x3800, 0x0800, CRC(3d28134e) SHA1(45a257a0aca74e2ab36dd70097220d8be29cc87b) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "c9.bin", 0x0000, 0x0800, CRC(1c4617be) SHA1(2b2b10f1256b4612e3e01ed1c8e2d7ccb6989f5d) )
	ROM_LOAD( "c11.bin", 0x0800, 0x0800, CRC(46f72fef) SHA1(daf334c78fdb73d43d524b733b763b290c602ae2) )
	ROM_LOAD( "c10.bin", 0x1000, 0x0800, CRC(ba9ec199) SHA1(626ab2eedf4c8d307dfad3b8863a67f8c34dda97) )
	ROM_LOAD( "c12.bin", 0x1800, 0x0800, CRC(41c09655) SHA1(cc639e660443b9dcb33f9aefe9af5d332591c466) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a", 0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */
ROM_END


ROM_START( pacplus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pacplus.6e",   0x0000, 0x1000, CRC(d611ef68) SHA1(8531c54ca6b0de0ea4ccc34e0e801ba9847e75bc) )
	ROM_LOAD( "pacplus.6f",   0x1000, 0x1000, CRC(c7207556) SHA1(8ba97215bdb75f0e70eb8d3223847efe4dc4fb48) )
	ROM_LOAD( "pacplus.6h",   0x2000, 0x1000, CRC(ae379430) SHA1(4e8613d51a80cf106f883db79685e1e22541da45) )
	ROM_LOAD( "pacplus.6j",   0x3000, 0x1000, CRC(5a6dff7b) SHA1(b956ae5d66683aab74b90469ad36b5bb361d677e) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacplus.5e",   0x0000, 0x1000, CRC(022c35da) SHA1(57d7d723c7b029e3415801f4ce83469ec97bb8a1) )
	ROM_LOAD( "pacplus.5f",   0x1000, 0x1000, CRC(4de65cdd) SHA1(9c0699204484be819b77f0b212c792fe9e9fae5d) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pacplus.7f",   0x0000, 0x0020, CRC(063dd53a) SHA1(2e43b46ec3b101d1babab87cdaddfa944116ec06) )
	ROM_LOAD( "pacplus.4a",   0x0020, 0x0100, CRC(e271a166) SHA1(cf006536215a7a1d488eebc1d8a2e2a8134ce1a6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mspacman )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k for code+64k for decrypted code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6",           0x9000, 0x1000, CRC(a90e7000) SHA1(e4df96f1db753533f7d770aa62ae1973349ea4cf) )
	ROM_LOAD( "u7",           0xb000, 0x1000, CRC(c82cd714) SHA1(1d8ac7ad03db2dc4c8c18ade466e12032673f874) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mspacmancr )  /* Bootleg on Crush Roller Board - Midway Graphics and Namco mentions are gone but Easter Egg still works */
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k for code+64k for decrypted code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) ) // a.6e
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) ) // b.6f
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) ) // c.6h
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) ) // d.6j
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) ) // 1.u5
	ROM_LOAD( "u6",           0x9000, 0x1000, CRC(a90e7000) SHA1(e4df96f1db753533f7d770aa62ae1973349ea4cf) ) // 2.u6
	ROM_LOAD( "3.u7",         0xb000, 0x1000, CRC(bd972c0c) SHA1(291bea78ba3cab862ab5c62488681f5848e50014) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5.5e",         0x0000, 0x1000, CRC(c45d4857) SHA1(06e8ceb7fcd745836b27b50495a4e9883a1ac88b) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) ) // 6.5f

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "mb7051.7f",    0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // m82s129n.4a

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // 7052.1m
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // 7052.3m   /* timing - not used */
ROM_END


ROM_START( mspacmnf )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k for code+64k for decrypted code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacfast.6f",   0x1000, 0x1000, CRC(720dc3ee) SHA1(7224d7acfa0144b681c71d7734a7337189835361) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6",           0x9000, 0x1000, CRC(a90e7000) SHA1(e4df96f1db753533f7d770aa62ae1973349ea4cf) )
	ROM_LOAD( "u7",           0xb000, 0x1000, CRC(c82cd714) SHA1(1d8ac7ad03db2dc4c8c18ade466e12032673f874) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mspacmab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "boot1",        0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "boot2",        0x1000, 0x1000, CRC(0d32de5e) SHA1(13ea0c343de072508908be885e6a2a217bbb3047) )
	ROM_LOAD( "boot3",        0x2000, 0x1000, CRC(1821ee0b) SHA1(5ea4d907dbb2690698db72c4e0b5be4d3e9a7786) )
	ROM_LOAD( "boot4",        0x3000, 0x1000, CRC(165a9dd8) SHA1(3022a408118fa7420060e32a760aeef15b8a96cf) )
	ROM_LOAD( "boot5",        0x8000, 0x1000, CRC(8c3e6de6) SHA1(fed6e9a2b210b07e7189a18574f6b8c4ec5bb49b) )
	ROM_LOAD( "boot6",        0x9000, 0x1000, CRC(368cb165) SHA1(387010a0c76319a1eab61b54c9bcb5c66c4b67a1) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mspacmbe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "boot1",        0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "2.bin",        0x1000, 0x1000, CRC(04e6c486) SHA1(63aa3e6c49d345cccfe87dd3fdcddc75ab4a570d) )
	ROM_LOAD( "boot3",        0x2000, 0x1000, CRC(1821ee0b) SHA1(5ea4d907dbb2690698db72c4e0b5be4d3e9a7786) )
	ROM_LOAD( "boot4",        0x3000, 0x1000, CRC(165a9dd8) SHA1(3022a408118fa7420060e32a760aeef15b8a96cf) )
	ROM_LOAD( "boot5",        0x8000, 0x1000, CRC(8c3e6de6) SHA1(fed6e9a2b210b07e7189a18574f6b8c4ec5bb49b) )
	ROM_LOAD( "6.bin",        0x9000, 0x1000, CRC(206a9623) SHA1(20006f945c1b7b0e3c0415eecc0b148e5a6a1dfa) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mspacii )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p3.6e",        0x0000, 0x1000, CRC(df673b57) SHA1(93ee4e8f9751db5f7d3c35988fcb8ae8037464ed) )
	ROM_LOAD( "p4.6f",        0x1000, 0x1000, CRC(7591f606) SHA1(0aead0ac5af602269df8732c5763147cdb543b8d) )
	ROM_LOAD( "p5.6h",        0x2000, 0x1000, CRC(c8ef1a7f) SHA1(0c9a28bbe63d7d44511a13316937a21b8846543e) )
	ROM_LOAD( "p6.6j",        0x3000, 0x1000, CRC(d498f435) SHA1(c041841b1349d8d5c2d560aed54c4aeb8adfddb8) )
	ROM_LOAD( "p7.s1",        0x8000, 0x1000, CRC(fbbc3d2e) SHA1(dc4337d7f0961e048a433021f670da6d314bd663) )
	ROM_LOAD( "p8.s2",        0x9000, 0x1000, CRC(aba3096d) SHA1(661e28785931fa329c2ebdc95d78072a42c512ff) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "p1.5e",        0x0000, 0x1000, CRC(04333722) SHA1(bb179d5302b26b815b5d7eff14865e7b4f8a6880) )
	ROM_LOAD( "p2.5f",        0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mspacii2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p3.bin",       0x0000, 0x1000, CRC(df673b57) SHA1(93ee4e8f9751db5f7d3c35988fcb8ae8037464ed) )
	ROM_LOAD( "p4.bin",       0x1000, 0x1000, CRC(7591f606) SHA1(0aead0ac5af602269df8732c5763147cdb543b8d) )
	ROM_LOAD( "p5.bin",       0x2000, 0x1000, CRC(c8ef1a7f) SHA1(0c9a28bbe63d7d44511a13316937a21b8846543e) )
	ROM_LOAD( "p6.bin",       0x3000, 0x1000, CRC(d498f435) SHA1(c041841b1349d8d5c2d560aed54c4aeb8adfddb8) )
	ROM_LOAD( "p2.bin",       0x8000, 0x1000, CRC(fbbc3d2e) SHA1(dc4337d7f0961e048a433021f670da6d314bd663) )
	ROM_LOAD( "p1.bin",       0x9000, 0x1000, CRC(aa3887c5) SHA1(0e20cb686383156a9883749568c8e57c15c3ae44) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "p7.bin",       0x0000, 0x1000, CRC(04333722) SHA1(bb179d5302b26b815b5d7eff14865e7b4f8a6880) )
	ROM_LOAD( "p8.bin",       0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mspacmat )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k for code+64k for decrypted code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6pacatk",     0x9000, 0x1000, CRC(f6d83f4d) SHA1(6135b187d6b968554d08f2ac00d3a3313efb8638) )
	ROM_LOAD( "u7",           0xb000, 0x1000, CRC(c82cd714) SHA1(1d8ac7ad03db2dc4c8c18ade466e12032673f874) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


// a graphics-only hack for Ms. Pac-Man that was sold as romkit by Two-Bit Score back in 1989
ROM_START( msheartb )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k for code+64k for decrypted code */
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6",           0x9000, 0x1000, CRC(a90e7000) SHA1(e4df96f1db753533f7d770aa62ae1973349ea4cf) )
	ROM_LOAD( "u7",           0xb000, 0x1000, CRC(c82cd714) SHA1(1d8ac7ad03db2dc4c8c18ade466e12032673f874) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5431d4c4) SHA1(34d45da44b4208e2774f5e2af08657a9086252e6) ) // sldh
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(ceb50654) SHA1(70dbe3cc715d3d52ee3d4f8dadbf5c59f87166a3) ) // sldh

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mspacpls )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "boot1",        0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "mspacatk.2",   0x1000, 0x1000, CRC(0af09d31) SHA1(6ff73e4da4910bcd2ca3aa299d8ffad23f8abf79) )
	ROM_LOAD( "boot3",        0x2000, 0x1000, CRC(1821ee0b) SHA1(5ea4d907dbb2690698db72c4e0b5be4d3e9a7786) )
	ROM_LOAD( "boot4",        0x3000, 0x1000, CRC(165a9dd8) SHA1(3022a408118fa7420060e32a760aeef15b8a96cf) )
	ROM_LOAD( "mspacatk.5",   0x8000, 0x1000, CRC(e6e06954) SHA1(ee5b266b1cc178df31fc1da5f66ef4911c653dda) )
	ROM_LOAD( "mspacatk.6",   0x9000, 0x1000, CRC(3b5db308) SHA1(c1ba630cb8fb665c4881a6cce9d3b0d4300bd0eb) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( pacgal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "boot1",        0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "boot2",        0x1000, 0x1000, CRC(0d32de5e) SHA1(13ea0c343de072508908be885e6a2a217bbb3047) )
	ROM_LOAD( "pacman.7fh",   0x2000, 0x1000, CRC(513f4d5c) SHA1(ae011b89422bd8cbb80389814500bc1427f6ecb2) )
	ROM_LOAD( "pacman.7hj",   0x3000, 0x1000, CRC(70694c8e) SHA1(d0d02f0997b44e1ba5ea27fc3f7af1b956e2a687) )
	ROM_LOAD( "boot5",        0x8000, 0x1000, CRC(8c3e6de6) SHA1(fed6e9a2b210b07e7189a18574f6b8c4ec5bb49b) )
	ROM_LOAD( "boot6",        0x9000, 0x1000, CRC(368cb165) SHA1(387010a0c76319a1eab61b54c9bcb5c66c4b67a1) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "pacman.5ef",   0x1000, 0x0800, CRC(65a3ee71) SHA1(cbbf700eefba2a5bf158983f2ca9688b7c6f5d2b) )
	ROM_LOAD( "pacman.5hj",   0x1800, 0x0800, CRC(50c7477d) SHA1(c04ec282a8cb528df5e38ad750d12ee71612695d) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mschamp ) /* "Original" Zola-Puc board.  FORCE ELECTRONICS KM-001 PCB copyright by RAYGLO MFG CO  1992/1993 */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "9fg.bin", 0x10000, 0x10000, CRC(04dba113) SHA1(6260fb58c47a506a60385fb7536fc4fbd8e02c7c) )   /* banked */

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "8e.bin",  0x0000, 0x0800, CRC(17435f53) SHA1(e844a7dfdb56a6f6cce5a3cf505d018434294470) )
	ROM_CONTINUE(        0x1000, 0x0800 )
	ROM_CONTINUE(        0x0800, 0x0800 )
	ROM_CONTINUE(        0x1800, 0x0800 )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END


ROM_START( mschamps ) /* Hack of hack???  Hack of the above "Rayglo" set??? */
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pm4.bin", 0x10000, 0x10000, CRC(7d6b6303) SHA1(65ad72a9188422653c02a48c07ed2661e1e36961) )   /* banked */

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pm5.bin", 0x0000, 0x0800, CRC(7fe6b9e2) SHA1(bfd0d84c7ef909ae078d8f60340682b3ff230aa6) )
	ROM_CONTINUE(        0x1000, 0x0800 )
	ROM_CONTINUE(        0x0800, 0x0800 )
	ROM_CONTINUE(        0x1800, 0x0800 )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END


ROM_START( superabc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "superabc.u14", 0x00000, 0x80000, CRC(a560efe6) SHA1(c7d43cc3bb3b1b10d06403462276231bfc8542dd) )  /* banked */

	ROM_REGION( 0x10000, "gfx1", ROMREGION_ERASE00 ) // descrambled rom goes here

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "char5e5f.u1",  0x00000, 0x20000, CRC(45caace0) SHA1(f850bd09ec68b0263ac8b30ae38c3878c7978ace) )

	ROM_REGION( 0x0120, "proms", 0 )    /* color PROMs */
	ROM_LOAD( "82s123.7f",  0x0000, 0x0020, CRC(3a188666) SHA1(067386e477ce48bbde3cf71f744a78a42238d236) )
	ROM_LOAD( "82s129.4a",  0x0020, 0x0100, CRC(4382c049) SHA1(5e535b1a6852260f38ae1e5cd57290a85cb6927f) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",  0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */

	ROM_REGION( 0x0020, "unknown", 0 )
	ROM_LOAD( "82s123.u18", 0x0000, 0x0020, CRC(23b2863c) SHA1(e62f87d2145e94be06dbd90fa8d9a79760bfcc4b) )  /* prom on daughterboard, unknown function */
ROM_END


ROM_START( superabco )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "superabco.u14", 0x00000, 0x80000, CRC(62565ad8) SHA1(cb434c608ee463788b73152d84ce6173bdfa350d) )  /* banked */

	ROM_REGION( 0x10000, "gfx1", ROMREGION_ERASE00 ) // descrambled rom goes here

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "char5e5f.u1",  0x00000, 0x20000, CRC(45caace0) SHA1(f850bd09ec68b0263ac8b30ae38c3878c7978ace) )

	ROM_REGION( 0x0120, "proms", 0 )    /* color PROMs */
	ROM_LOAD( "82s123.7f",  0x0000, 0x0020, CRC(3a188666) SHA1(067386e477ce48bbde3cf71f744a78a42238d236) )
	ROM_LOAD( "82s129.4a",  0x0020, 0x0100, CRC(4382c049) SHA1(5e535b1a6852260f38ae1e5cd57290a85cb6927f) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",  0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  /* timing - not used */

	ROM_REGION( 0x0020, "unknown", 0 )
	ROM_LOAD( "82s123.u18", 0x0000, 0x0020, CRC(23b2863c) SHA1(e62f87d2145e94be06dbd90fa8d9a79760bfcc4b) )  /* prom on daughterboard, unknown function */
ROM_END


ROM_START( crush )
	ROM_REGION( 2*0x10000, "maincpu", 0 )   /* 64k for code + 64k for opcode copy to hack protection */
	ROM_LOAD( "crushkrl.6e",  0x0000, 0x1000, CRC(a8dd8f54) SHA1(4e3a973ea74a9e145c6997513b98fc80aa478442) )
	ROM_LOAD( "crushkrl.6f",  0x1000, 0x1000, CRC(91387299) SHA1(3ad8c28e02c45667e32860953b157832445a82c8) )
	ROM_LOAD( "crushkrl.6h",  0x2000, 0x1000, CRC(d4455f27) SHA1(53f8ffc28be664fa8a2d756b4c70045a3f041bea) )
	ROM_LOAD( "crushkrl.6j",  0x3000, 0x1000, CRC(d59fc251) SHA1(024605e4485b0ac826217256e5356ed9a6c8ef34) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "maketrax.5e",  0x0000, 0x1000, CRC(91bad2da) SHA1(096197d0cb6d55bf72b5be045224f4bd6a9cfa1b) )
	ROM_LOAD( "maketrax.5f",  0x1000, 0x1000, CRC(aea79f55) SHA1(279021e6771dfa5bd0b7c557aae44434286d91b7) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( crushbl )
	ROM_REGION( 2*0x10000, "maincpu", 0 )   /* 64k for code + 64k for opcode copy to hack protection */
	ROM_LOAD( "cr1.bin",  0x0000, 0x1000, CRC(e2e84cd1) SHA1(3fc5a9aa3ee219b386a1d0622547c77aca27533d) )
	ROM_LOAD( "cr2.bin",  0x1000, 0x1000, CRC(ec020e6f) SHA1(eef9008c38a68ed20c1e3596016d97d4e72de9f2) )
	ROM_LOAD( "cr3.bin",  0x2000, 0x1000, CRC(d4455f27) SHA1(53f8ffc28be664fa8a2d756b4c70045a3f041bea) ) // matches original
	ROM_LOAD( "cr4.bin",  0x3000, 0x1000, CRC(9936ae06) SHA1(80aff9a12dab97e9d5818f7a7fac54dc52b579d4) )

	/* no other roms in this set */
	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "maketrax.5e",  0x0000, 0x1000, CRC(91bad2da) SHA1(096197d0cb6d55bf72b5be045224f4bd6a9cfa1b) )
	ROM_LOAD( "maketrax.5f",  0x1000, 0x1000, CRC(aea79f55) SHA1(279021e6771dfa5bd0b7c557aae44434286d91b7) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( crushbl2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cr5.7d",       0x0000, 0x1000, CRC(4954d51d) SHA1(ff973ed961a531f7fc6f45b2c27af317f4cc6a4d) )
	ROM_LOAD( "cr6.7e",       0x1000, 0x1000, CRC(27eb4299) SHA1(af2d7fdedcea766045fc2f20ae383024d1c35731) )
	ROM_LOAD( "cr7.7h",       0x2000, 0x1000, CRC(d297108e) SHA1(a5bd11f26ba82b66a93d07e8cbc838ad9bd01413) )
	ROM_LOAD( "cr8.7j",       0x3000, 0x1000, CRC(bcc40eaf) SHA1(50ffb3cf5cffd7618ceac207f60ca052fc13e38c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cr1.5e",       0x0000, 0x0800, CRC(c7617198) SHA1(95b204af0345163f93811cc770ee0ca2851a39c1) )
	ROM_LOAD( "cr3.5h",       0x0800, 0x0800, CRC(c15b6967) SHA1(d8f16e2d6af5bf0f610d1e23614c531f67490da9) )
	ROM_LOAD( "cr2.5f",       0x1000, 0x0800, CRC(d5bc5cb8) SHA1(269b82ae2b838c72ae06bff77412f22bb779ad2e) )  /* copyright sign was removed */
	ROM_LOAD( "cr4.5j",       0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "74s288.8a",    0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( crushbl3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cre.bin",       0x0000, 0x1000, CRC(e1ecc4da) SHA1(51f7d3db9bb2823d3528bf872fcf44bf7afd6d96) )
	ROM_LOAD( "crf.bin",       0x1000, 0x1000, CRC(b5193960) SHA1(4a5b743fcc178500ee8c7f2b30180daaee9057cc) )
	ROM_LOAD( "crg.bin",       0x2000, 0x1000, CRC(964b9f24) SHA1(bfb0585fecbef49b7445f3e6443835d673f552b2) )
	ROM_LOAD( "crh.bin",       0x3000, 0x1000, CRC(dfd2380e) SHA1(728a03b31a15739e325267e345e6e14c058e4b99) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cr1.5e",       0x0000, 0x0800, CRC(c7617198) SHA1(95b204af0345163f93811cc770ee0ca2851a39c1) )
	ROM_LOAD( "cr3.5h",       0x0800, 0x0800, CRC(c15b6967) SHA1(d8f16e2d6af5bf0f610d1e23614c531f67490da9) )
	ROM_LOAD( "cr2.5f",       0x1000, 0x0800, CRC(d5bc5cb8) SHA1(269b82ae2b838c72ae06bff77412f22bb779ad2e) )  /* copyright sign was removed */
	ROM_LOAD( "cr4.5j",       0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	// the set with the above 'crushbl3' program roms and these gfx roms just seems to be a bad dump (some bad maze tiles?)
//  ROM_REGION( 0x2000, "gfx1", 0 )
//  ROM_LOAD( "cr1.bin",       0x0000, 0x0800, CRC(cc31c649) SHA1(a0640d2abc21872b0e680e8e31e3bcb7e7a07953) )
//  ROM_LOAD( "cr3.bin",       0x0800, 0x0800, CRC(14c121d8) SHA1(05f900a2e2a67401ab357340c1fb36153f365f1b) )
//  ROM_LOAD( "cr2.bin",       0x1000, 0x0800, CRC(882dc667) SHA1(5ea01d9c692b3061a0e39e2227fbc6af4baaab11) )  /* copyright sign was removed */
//  ROM_LOAD( "cr4.bin",       0x1800, 0x0800, CRC(0d3877c4) SHA1(0a6f4098181480aa85225324129e37bba375252d) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "74s288.8a",    0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( crush2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tp1",          0x0000, 0x0800, CRC(f276592e) SHA1(68ebb7d9f70af868d99ec42c26bc55a54ba1f22c) )
	ROM_LOAD( "tp5a",         0x0800, 0x0800, CRC(3d302abe) SHA1(8ca5cd82d099b55e20f785489158231a1d99a430) )
	ROM_LOAD( "tp2",          0x1000, 0x0800, CRC(25f42e70) SHA1(66de8203c364fd90e8a2b5749c2e40665b2f5830) )
	ROM_LOAD( "tp6",          0x1800, 0x0800, CRC(98279cbe) SHA1(84b5e64bdbc25afab9b6f53e1719640e21a6feba) )
	ROM_LOAD( "tp3",          0x2000, 0x0800, CRC(8377b4cb) SHA1(f828a177f22db9093a00c31e39e16214ce0dc6de) )
	ROM_LOAD( "tp7",          0x2800, 0x0800, CRC(d8e76c8c) SHA1(7c3d7eb07b9256130141f71eba722f7823fd4c32) )
	ROM_LOAD( "tp4",          0x3000, 0x0800, CRC(90b28fa3) SHA1(ff58d2dfb016397daabe2996bc3a7b63d28a4cca) )
	ROM_LOAD( "tp8",          0x3800, 0x0800, CRC(10854e1b) SHA1(b3b9066d9a43796185c00ae12f7bb2bbf42e3a07) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tpa",          0x0000, 0x0800, CRC(c7617198) SHA1(95b204af0345163f93811cc770ee0ca2851a39c1) )
	ROM_LOAD( "tpc",          0x0800, 0x0800, CRC(e129d76a) SHA1(c9256795c6d0929ade1f24b372dadc2a2b88d897) )
	ROM_LOAD( "tpb",          0x1000, 0x0800, CRC(d1899f05) SHA1(dce755511b6262b984a2bca329f454892e486a09) )
	ROM_LOAD( "tpd",          0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( crushrlf )
	ROM_REGION( 0x10000, "maincpu", 0 )

	ROM_LOAD( "pin1cc_6e.bin",          0x0400, 0x0400, CRC(65e469cf) SHA1(baeb5ba0ca0d78bca07f7830269f9c079f36d425) )
	ROM_CONTINUE(0x0000,0x0400)
	ROM_LOAD( "pin5cc_6k.bin",          0x0c00, 0x0400, CRC(15f0415b) SHA1(90c663387a81ad206874a531d9fe631ac0175975) )
	ROM_CONTINUE(0x0800,0x0400)
	ROM_LOAD( "pin2cc_6f.bin",          0x1400, 0x0400, CRC(653f726d) SHA1(3121315cf3e8be86d29687f29fc514e29dc64a02) )
	ROM_CONTINUE(0x1000,0x400)
	ROM_LOAD( "pin6cc_6m.bin",          0x1c00, 0x0400, CRC(4536ea5b) SHA1(6e0b22dd05a76644b13f1c71f771d686cd411eea) )
	ROM_CONTINUE(0x1800,0x400)
	ROM_LOAD( "pin3cc_6h.bin",          0x2400, 0x0400, CRC(55e15863) SHA1(bcbf4e5a268739c906e5c400e639e0e055799d47) )
	ROM_CONTINUE(0x2000,0x400)
	ROM_LOAD( "pin7cc_6n.bin",          0x2c00, 0x0400, CRC(409111ec) SHA1(ba98cfc1cce8627d11fda4954c3776d0b90cb584) )
	ROM_CONTINUE(0x2800,0x400)
	ROM_LOAD( "pin4cc_6j.bin",          0x3400, 0x0400, CRC(4fc4b582) SHA1(cb73b5f9171ba493afdfced0baeef9bb6bdb428d) )
	ROM_CONTINUE(0x3000,0x400)
	ROM_LOAD( "pin8cc_6p.bin",          0x3c00, 0x0400, CRC(0d97a047) SHA1(d0024a87a7530246bfbef7d1603b599e2f168973) )
	ROM_CONTINUE(0x3800,0x400)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pin9cc_5e.bin",        0x0000, 0x0800, CRC(b6551507) SHA1(a544e6afda0dd1bea526cb94b9c456d923054698))
	ROM_LOAD( "pin11cc_5h.bin",       0x0800, 0x0800, CRC(e129d76a) SHA1(c9256795c6d0929ade1f24b372dadc2a2b88d897))
	ROM_LOAD( "pin10cc_5f.bin",       0x1000, 0x0800, CRC(d106da36) SHA1(a086e4874edf2f1a8bc945bed0e51424d92beaf2))
	ROM_LOAD( "pin12cc_5j.bin",       0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( crush3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "unkmol.4e",    0x0000, 0x0800, CRC(49150ddf) SHA1(5a20464a40d1d48606664779c85a7679073d7954) )
	ROM_LOAD( "unkmol.6e",    0x0800, 0x0800, CRC(21f47e17) SHA1(1194b5e8b0cce1f480acda3cb6c1fc65988bdc80) )
	ROM_LOAD( "unkmol.4f",    0x1000, 0x0800, CRC(9b6dd592) SHA1(6bb1b7ed95a7a8682a6ab58fa9f02c34beea8cd4) )
	ROM_LOAD( "unkmol.6f",    0x1800, 0x0800, CRC(755c1452) SHA1(a2da17ed0e526dad4d53d332467a3dfd3b2a8cab) )
	ROM_LOAD( "unkmol.4h",    0x2000, 0x0800, CRC(ed30a312) SHA1(15855904422eb603e5c5465bd038a3e8c666c10d) )
	ROM_LOAD( "unkmol.6h",    0x2800, 0x0800, CRC(fe4bb0eb) SHA1(70e480a75421ee0832456f1d30bf45a702192625) )
	ROM_LOAD( "unkmol.4j",    0x3000, 0x0800, CRC(072b91c9) SHA1(808df98c0cfd2367a39e06f30f920fd14887d922) )
	ROM_LOAD( "unkmol.6j",    0x3800, 0x0800, CRC(66fba07d) SHA1(4944d69a38fd823dad38b70433848017ae7027d7) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "unkmol.5e",    0x0000, 0x0800, CRC(338880a0) SHA1(beba1c71291394442b04fa5f4e1b833d7cf0fa8a) )
	ROM_LOAD( "unkmol.5h",    0x0800, 0x0800, CRC(4ce9c81f) SHA1(90a695ce4a45bde62bdbf09724a3ec6b45674660) )
	ROM_LOAD( "unkmol.5f",    0x1000, 0x0800, CRC(752e3780) SHA1(5730ebd8091eba5ac32ddd9db2f42d718b088753) )
	ROM_LOAD( "unkmol.5j",    0x1800, 0x0800, CRC(6e00d2ac) SHA1(aa3f1f3a3b6899bea717d97e4817b13159e552e5) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( crush4 )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* 64k for code+64k for decrypted code */
	ROM_LOAD( "crtwt.2", 0x10000, 0x10000, CRC(adbd21f7) SHA1(984b005cd7a73f697715ecb7a4d806024cb7596d) )   /* banked */

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "crtwt.1", 0x0000, 0x0800, CRC(4250a9ea) SHA1(496a368afcf09c09205f7d0882320d2022e6fc98) )
	ROM_CONTINUE(        0x1000, 0x0800 )
	ROM_CONTINUE(        0x0800, 0x0800 )
	ROM_CONTINUE(        0x1800, 0x0800 )
	ROM_CONTINUE(        0x2000, 0x0800 )
	ROM_CONTINUE(        0x3000, 0x0800 )
	ROM_CONTINUE(        0x2800, 0x0800 )
	ROM_CONTINUE(        0x3800, 0x0800 )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.bin",   0x0020, 0x0100, CRC(2bc5d339) SHA1(446e234df94d9ef34c3191877bb33dd775acfdf5) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( maketrax )
	ROM_REGION( 2*0x10000, "maincpu", 0 )   /* 64k for code + 64k for opcode copy to hack protection */
	ROM_LOAD( "maketrax.6e",  0x0000, 0x1000, CRC(0150fb4a) SHA1(ba41582d5432670654479b4bf6d938d2168858af) )
	ROM_LOAD( "maketrax.6f",  0x1000, 0x1000, CRC(77531691) SHA1(68a450bcc8d832368d0f1cb2815cb5c03451796e) )
	ROM_LOAD( "maketrax.6h",  0x2000, 0x1000, CRC(a2cdc51e) SHA1(80d80235cda3ce19c1dbafacf3d47b1325ad4728) )
	ROM_LOAD( "maketrax.6j",  0x3000, 0x1000, CRC(0b4b5e0a) SHA1(621aece612df612065f776696956ef3671421fac) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "maketrax.5e",  0x0000, 0x1000, CRC(91bad2da) SHA1(096197d0cb6d55bf72b5be045224f4bd6a9cfa1b) )
	ROM_LOAD( "maketrax.5f",  0x1000, 0x1000, CRC(aea79f55) SHA1(279021e6771dfa5bd0b7c557aae44434286d91b7) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( maketrxb )
	ROM_REGION( 2*0x10000, "maincpu", 0 )   /* 64k for code + 64k for opcode copy to hack protection */
	ROM_LOAD( "maketrax.6e",  0x0000, 0x1000, CRC(0150fb4a) SHA1(ba41582d5432670654479b4bf6d938d2168858af) )
	ROM_LOAD( "maketrax.6f",  0x1000, 0x1000, CRC(77531691) SHA1(68a450bcc8d832368d0f1cb2815cb5c03451796e) )
	ROM_LOAD( "maketrxb.6h",  0x2000, 0x1000, CRC(6ad342c9) SHA1(5469f3952adc682725a71602b4a00a7751e48a99) )
	ROM_LOAD( "maketrxb.6j",  0x3000, 0x1000, CRC(be27f729) SHA1(0f7b873d33f751fa2fc54f9eede0598cb7d7f3c8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "maketrax.5e",  0x0000, 0x1000, CRC(91bad2da) SHA1(096197d0cb6d55bf72b5be045224f4bd6a9cfa1b) )
	ROM_LOAD( "maketrax.5f",  0x1000, 0x1000, CRC(aea79f55) SHA1(279021e6771dfa5bd0b7c557aae44434286d91b7) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( korosuke )
	ROM_REGION( 2*0x10000, "maincpu", 0 )   /* 64k for code + 64k for opcode copy to hack protection */
	ROM_LOAD( "kr.6e",        0x0000, 0x1000, CRC(69f6e2da) SHA1(5f06523122d81a079bed080a16b44adb90aa95ad) )
	ROM_LOAD( "kr.6f",        0x1000, 0x1000, CRC(abf34d23) SHA1(6ae16fb8208037fd8b752076dd97e3da09e5cb8f) )
	ROM_LOAD( "kr.6h",        0x2000, 0x1000, CRC(76a2e2e2) SHA1(570aaed91279caab9274024e5a6176bdfe85bedd) )
	ROM_LOAD( "kr.6j",        0x3000, 0x1000, CRC(33e0e3bb) SHA1(43f5da486b9c44b0e4e8c909000786ee8ffee87f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "kr.5e",        0x0000, 0x1000, CRC(e0380be8) SHA1(96eb7c5ef91342be67bd2a6c4958412d2572ba2a) )
	ROM_LOAD( "kr.5f",        0x1000, 0x1000, CRC(63fec9ee) SHA1(7d136362e08cceba9395c2c469d8fec451c5e396) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mbrush )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mbrush.6e",    0x0000, 0x1000, CRC(750fbff7) SHA1(986d20010d4fdd4bac916ac6b3a01bcd09d695ea) )
	ROM_LOAD( "mbrush.6f",    0x1000, 0x1000, CRC(27eb4299) SHA1(af2d7fdedcea766045fc2f20ae383024d1c35731) )
	ROM_LOAD( "mbrush.6h",    0x2000, 0x1000, CRC(d297108e) SHA1(a5bd11f26ba82b66a93d07e8cbc838ad9bd01413) )
	ROM_LOAD( "mbrush.6j",    0x3000, 0x1000, CRC(6fd719d0) SHA1(3de00981264cef24dc2c6277192e071144da2a88) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tpa",          0x0000, 0x0800, CRC(c7617198) SHA1(95b204af0345163f93811cc770ee0ca2851a39c1) )
	ROM_LOAD( "mbrush.5h",    0x0800, 0x0800, CRC(c15b6967) SHA1(d8f16e2d6af5bf0f610d1e23614c531f67490da9) )
	ROM_LOAD( "mbrush.5f",    0x1000, 0x0800, CRC(d5bc5cb8) SHA1(269b82ae2b838c72ae06bff77412f22bb779ad2e) )  /* copyright sign was removed */
	ROM_LOAD( "tpd",          0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( paintrlr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "paintrlr.1",   0x0000, 0x0800, CRC(556d20b5) SHA1(c0a74def85bca108fc56726d22bbea1fc051e1ff) )
	ROM_LOAD( "paintrlr.5",   0x0800, 0x0800, CRC(4598a965) SHA1(866dbe7c0dbca10c5d5ec3efa3c79fb1ff1c5b56) )
	ROM_LOAD( "paintrlr.2",   0x1000, 0x0800, CRC(2da29c81) SHA1(e77f84e2f3136a116b75b40869e7f59404b0dbab) )
	ROM_LOAD( "paintrlr.6",   0x1800, 0x0800, CRC(1f561c54) SHA1(ef1159f2203ff6b5c17e3a79f32e8cafb12a49f7) )
	ROM_LOAD( "paintrlr.3",   0x2000, 0x0800, CRC(e695b785) SHA1(bc627a1a03d2e701fa4051acee469a4516cfb5bf) )
	ROM_LOAD( "paintrlr.7",   0x2800, 0x0800, CRC(00e6eec0) SHA1(e98850cf6e1762d08225a95f26a26766f8fa7303) )
	ROM_LOAD( "paintrlr.4",   0x3000, 0x0800, CRC(0fd5884b) SHA1(fa9614b625b3d71a6e9d5f883da625ad88e3eb5e) )
	ROM_LOAD( "paintrlr.8",   0x3800, 0x0800, CRC(4900114a) SHA1(47aee5bad136c19b203958b7ddac583d45018249) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tpa",          0x0000, 0x0800, CRC(c7617198) SHA1(95b204af0345163f93811cc770ee0ca2851a39c1) )
	ROM_LOAD( "mbrush.5h",    0x0800, 0x0800, CRC(c15b6967) SHA1(d8f16e2d6af5bf0f610d1e23614c531f67490da9) )
	ROM_LOAD( "mbrush.5f",    0x1000, 0x0800, CRC(d5bc5cb8) SHA1(269b82ae2b838c72ae06bff77412f22bb779ad2e) )  /* copyright sign was removed */
	ROM_LOAD( "tpd",          0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/*

Crush Roller (Sidam PCB)

CPU
1x SGS Z80ACPUB1 (main)
1x AY-3-8912 (sound)
1x oscillator (no speed markings)
ROMs
6x TMS2532
1x SN74S288N
1x TBP24S10N
Note
1x 22x2 edge connector
1x trimmer (volume)
2x 8 switches dip
1x 4 switches dip
1x red led
*/

ROM_START( crushs )
	ROM_REGION( 2*0x10000, "maincpu", 0 )   /* 64k for code + 64k for opcode copy to hack protection */
	ROM_LOAD( "11105-0.0j",  0x0000, 0x1000, CRC(dd425429) SHA1(dc3fd8b71384c25dc807caea9187a775144ad24e) )
	ROM_LOAD( "11105-1.1j",  0x1000, 0x1000, CRC(f9d89eef) SHA1(4de911b68cf6044d1e50fd3f455a61327483540b) )
	ROM_LOAD( "11105-2.2j",  0x2000, 0x1000, CRC(40c23a27) SHA1(b59586ec18446b866b9ef267bb876c410f2972b0) )
	ROM_LOAD( "11105-3.3j",  0x3000, 0x1000, CRC(5802644f) SHA1(3ba078c9ab8c6e251f6d1a3e8b6f8bf4820340a5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "11105-4.4j",  0x0000, 0x1000, CRC(91bad2da) SHA1(096197d0cb6d55bf72b5be045224f4bd6a9cfa1b) )
	ROM_LOAD( "11105-5.5j",  0x1000, 0x1000, CRC(b5c14376) SHA1(2c8c57f96c51f12f73daf65dc2a73e8185aaacea) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "74s288.8a",    0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "24s10.6b",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	/* No Sound Proms (not Namco Sound) */
ROM_END


ROM_START( ponpoko )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ppokoj1.bin",  0x0000, 0x1000, CRC(ffa3c004) SHA1(d9e3186dcd4eb94d02bd24ad56030b248721537f) )
	ROM_LOAD( "ppokoj2.bin",  0x1000, 0x1000, CRC(4a496866) SHA1(4b8bd13e58040c30ca032b54fb47d889677e8c6f) )
	ROM_LOAD( "ppokoj3.bin",  0x2000, 0x1000, CRC(17da6ca3) SHA1(1a57767557c13fa3d08e4451fb9fb1f7219b26ef) )
	ROM_LOAD( "ppokoj4.bin",  0x3000, 0x1000, CRC(9d39a565) SHA1(d4835ee97c9b3c63504d8b576a11f0a3a97057ec) )
	ROM_LOAD( "ppoko5.bin",   0x8000, 0x1000, CRC(54ca3d7d) SHA1(b54299b00573fbd6d3278586df0c12c09235615d) )
	ROM_LOAD( "ppoko6.bin",   0x9000, 0x1000, CRC(3055c7e0) SHA1(ab3fb9c8846effdcea0569d08a84c5fa19057a8f) )
	ROM_LOAD( "ppoko7.bin",   0xa000, 0x1000, CRC(3cbe47ca) SHA1(577c79c016be26a9fc7895cef0f30bf3f0b15097) )
	ROM_LOAD( "ppokoj8.bin",  0xb000, 0x1000, CRC(04b63fc6) SHA1(9b86ae34aaefa2813d29a4f7b24cee40eadcc6a1) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ppoko9.bin",   0x0000, 0x1000, CRC(b73e1a06) SHA1(f1229e804eb15827b71f0e769a8c9e496c6d1de7) )
	ROM_LOAD( "ppoko10.bin",  0x1000, 0x1000, CRC(62069b5d) SHA1(1b58ad1c2cc2d12f4e492fdd665b726d50c80364) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( ponpokov )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ppoko1.bin",   0x0000, 0x1000, CRC(49077667) SHA1(3e760cd4dbe5913e58d786caf510237ff635c775) )
	ROM_LOAD( "ppoko2.bin",   0x1000, 0x1000, CRC(5101781a) SHA1(a82fbd2418ac7866f9463092e9dd37fd7ba9b694) )
	ROM_LOAD( "ppoko3.bin",   0x2000, 0x1000, CRC(d790ed22) SHA1(2d32f91f6993232db40b44b35bd2503d85e5c874) )
	ROM_LOAD( "ppoko4.bin",   0x3000, 0x1000, CRC(4e449069) SHA1(d5e6e346f80e66eb0db530de9721d9b6f22e86ae) )
	ROM_LOAD( "ppoko5.bin",   0x8000, 0x1000, CRC(54ca3d7d) SHA1(b54299b00573fbd6d3278586df0c12c09235615d) )
	ROM_LOAD( "ppoko6.bin",   0x9000, 0x1000, CRC(3055c7e0) SHA1(ab3fb9c8846effdcea0569d08a84c5fa19057a8f) )
	ROM_LOAD( "ppoko7.bin",   0xa000, 0x1000, CRC(3cbe47ca) SHA1(577c79c016be26a9fc7895cef0f30bf3f0b15097) )
	ROM_LOAD( "ppoko8.bin",   0xb000, 0x1000, CRC(b39be27d) SHA1(c299d22d26da68bec8fc53c898523135ec4016fa) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ppoko9.bin",   0x0000, 0x1000, CRC(b73e1a06) SHA1(f1229e804eb15827b71f0e769a8c9e496c6d1de7) )
	ROM_LOAD( "ppoko10.bin",  0x1000, 0x1000, CRC(62069b5d) SHA1(1b58ad1c2cc2d12f4e492fdd665b726d50c80364) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( candory )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ppokoj1.bin",  0x0000, 0x1000, CRC(ffa3c004) SHA1(d9e3186dcd4eb94d02bd24ad56030b248721537f) ) // candory.1
	ROM_LOAD( "ppokoj2.bin",  0x1000, 0x1000, CRC(4a496866) SHA1(4b8bd13e58040c30ca032b54fb47d889677e8c6f) ) // candory.2
	ROM_LOAD( "ppokoj3.bin",  0x2000, 0x1000, CRC(17da6ca3) SHA1(1a57767557c13fa3d08e4451fb9fb1f7219b26ef) ) // candory.3
	ROM_LOAD( "ppokoj4.bin",  0x3000, 0x1000, CRC(9d39a565) SHA1(d4835ee97c9b3c63504d8b576a11f0a3a97057ec) ) // candory.4
	ROM_LOAD( "ppoko5.bin",   0x8000, 0x1000, CRC(54ca3d7d) SHA1(b54299b00573fbd6d3278586df0c12c09235615d) ) // candory.5
	ROM_LOAD( "ppoko6.bin",   0x9000, 0x1000, CRC(3055c7e0) SHA1(ab3fb9c8846effdcea0569d08a84c5fa19057a8f) ) // candory.6
	ROM_LOAD( "ppoko7.bin",   0xa000, 0x1000, CRC(3cbe47ca) SHA1(577c79c016be26a9fc7895cef0f30bf3f0b15097) ) // candory.7
	ROM_LOAD( "ppokoj8.bin",  0xb000, 0x1000, CRC(04b63fc6) SHA1(9b86ae34aaefa2813d29a4f7b24cee40eadcc6a1) ) // candory.8

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "candory.v2",   0x0000, 0x1000, CRC(7d16bdff) SHA1(36cce50b4f7b545efa4733ef9842919931a78353) )
	ROM_LOAD( "candory.v1",   0x1000, 0x1000, CRC(e08ac188) SHA1(68e66180d1c89c19d00320e5105373a70aab9dfb) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( eyes )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d7",           0x0000, 0x1000, CRC(3b09ac89) SHA1(a8f1c918da74495bb73172f39364dada38ae4713) )
	ROM_LOAD( "e7",           0x1000, 0x1000, CRC(97096855) SHA1(10d3b164bbbe5eee86e881a1434f0c114ee8adff) )
	ROM_LOAD( "f7",           0x2000, 0x1000, CRC(731e294e) SHA1(96c0308c146dbd85e244c4530af9ae8df78c86de) )
	ROM_LOAD( "h7",           0x3000, 0x1000, CRC(22f7a719) SHA1(eb000b606ecedd52bebbb232e661fb1ef205f8b0) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "d5",           0x0000, 0x1000, CRC(d6af0030) SHA1(652b779533e3f00e81cc102b78d367d503b06f33) )
	ROM_LOAD( "e5",           0x1000, 0x1000, CRC(a42b5201) SHA1(2e5cede3b6039c7bd5230de27d02aaa3f35a7b64) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( eyes2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g38201.7d",    0x0000, 0x1000, CRC(2cda7185) SHA1(7ec3ee9bb90e6a1d83ad3aa12fd62184e07b1399) )
	ROM_LOAD( "g38202.7e",    0x1000, 0x1000, CRC(b9fe4f59) SHA1(2d97dc1a0458b406ca0c50d6b8bd0dbe58d21464) )
	ROM_LOAD( "g38203.7f",    0x2000, 0x1000, CRC(d618ba66) SHA1(76d93d8bc09bafac464ebfd002869e21535a365b) )
	ROM_LOAD( "g38204.7h",    0x3000, 0x1000, CRC(cf038276) SHA1(bcf4e129a151e2245e630cf865ce6cb009b405a5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "g38205.5d",    0x0000, 0x1000, CRC(03b1b4c7) SHA1(a90b2fbaee2888ee4f0bcdf80a069c8594ef5ea1) )  /* this one has a (c) sign */
	ROM_LOAD( "g38206.5e",    0x1000, 0x1000, CRC(a42b5201) SHA1(2e5cede3b6039c7bd5230de27d02aaa3f35a7b64) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( eyesb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",           0x0000, 0x0800, CRC(339d279a) SHA1(bc2a7801b9f94782f260346255f84a0e84729f01) )
	ROM_LOAD( "5.bin",           0x0800, 0x0800, CRC(1b68a61d) SHA1(348ef7b19934b000b03c74bec4796f1089dfc5eb) )
	ROM_LOAD( "2.bin",           0x1000, 0x0800, CRC(d4f9aaf8) SHA1(1e450a7ecf42f9bc2d58823907930dd7aa454215) )
	ROM_LOAD( "6.bin",           0x1800, 0x0800, CRC(6b41bb80) SHA1(aa3555833a2e8e596e126a749fe12853e4aa05b1) )
	ROM_LOAD( "3.bin",           0x2000, 0x0800, CRC(748e0e48) SHA1(857de6f7fe79a2613a93fb3efd981edb64c0190e) )
	ROM_LOAD( "7.bin",           0x2800, 0x0800, CRC(7b7f4a74) SHA1(373229de4ed25d577fe15deb3b6bc36786988284) )
	ROM_LOAD( "4.bin",           0x3000, 0x0800, CRC(367a3884) SHA1(d55d7eca56412661adac1849a398298670e86d15) )
	ROM_LOAD( "8.bin",           0x3800, 0x0800, CRC(2baaadae) SHA1(7b20ed5935e9a4f793f690bab2a6bc0db44d12af) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "9.bin",           0x0000, 0x0800, CRC(342c0653) SHA1(d07e3d4528b72e54a1b5dbed009cce765a5a086f) )
	ROM_LOAD( "11.bin",          0x0800, 0x0800, CRC(aaa7a537) SHA1(571d981ed2aad62d7c7f2798e9084228d45523d4) )
	ROM_LOAD( "10.bin",          0x1000, 0x0800, CRC(b247b82c) SHA1(8c10a8ef5e79b0b5fefad6eb77bfa68a0ca18035) )
	ROM_LOAD( "12.bin",          0x1800, 0x0800, CRC(99af4b30) SHA1(6a0939ff2fa7ae39a960dd4d9f9b7c01f57647c5) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7051.bin",        0x0000, 0x0020, CRC(0dad2ccb) SHA1(f42c5ee7084e5702b5b0c8c1d86b0a41a6e1821d) )
	ROM_LOAD( "7051-3.bin",      0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",       0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // not dumped, taken from parent
	ROM_LOAD( "7051-2.bin",      0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // two of these?
ROM_END


ROM_START( eyeszac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.7d",         0x0000, 0x1000, BAD_DUMP CRC(568851aa) SHA1(a97963556a6d77400afaafd73bcc32cb7f3a54d2) ) // 2532 vs 2732 problem, (near)identical halves
	ROM_LOAD( "2.7f",         0x1000, 0x1000, BAD_DUMP CRC(9a0dba3b) SHA1(9f66f814bc2d483488df8918d872c7d6ce1bea3d) ) // 2532 vs 2732 problem, 1st half empty
	ROM_LOAD( "3.7h",         0x2000, 0x1000, BAD_DUMP CRC(5a12aa81) SHA1(1adb1b033066cabbd62a5d33012ed1c66b955943) ) // 2532 vs 2732 problem, (near)identical halves
	ROM_LOAD( "4.7j",         0x3000, 0x1000, BAD_DUMP CRC(b11958a1) SHA1(fa66fec80594f313f605e2b904dfe34693a1aa7d) ) // 2532 vs 2732 problem, (near)identical halves

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5.5d",         0x0000, 0x1000, BAD_DUMP CRC(7b2c4d53) SHA1(82e1a70c5cb76519dca252cfcea2a69c3601e36f) ) // 2532 vs 2732 problem, (near)identical halves
	ROM_LOAD( "6.5f",         0x1000, 0x1000, BAD_DUMP CRC(bccb4f1a) SHA1(0abf73b78a95b7e911480d41e0136dbc635b4a34) ) // 2532 vs 2732 problem, (near)identical halves

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) ) // taken from parent
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) ) // taken from parent

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // taken from parent
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // taken from parent
ROM_END

/* It's just a decrypted version of Eyes with the copyright changes...
 roms marked with a comment were in the set but we're not using them */
ROM_START( eyeszacb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "zacb_11.bin",  0x0000, 0x0800, CRC(69c1602a) SHA1(47b0935406b7ee2f414de58da1d4e81c6277a0c2) ) // "no diagnostics, bad custom??" (unused)
	ROM_LOAD( "zacb_1.bin",   0x0000, 0x0800, CRC(a4a9d7a0) SHA1(f0b807d2fa347e50df52971aa7539a88f342bad6) )
	ROM_LOAD( "zacb_5.bin",   0x0800, 0x0800, CRC(c32b3f73) SHA1(80d2e987f0318b984e5c7c4d0b5faa262eebeca4) )
	ROM_LOAD( "zacb_2.bin",   0x1000, 0x0800, CRC(195b9473) SHA1(62eb16af38cc9004787dc55433ed3db11af44a4b) )
	ROM_LOAD( "zacb_6.bin",   0x1800, 0x0800, CRC(292886cb) SHA1(e77c3724c7cd8cd95014194ba4bb2f7e04afb0dd) )
//  ROM_LOAD( "33.bin",          0x2000, 0x0800, CRC(df983e1d) SHA1(7c06fc69b7d0424f7b9348649d5587ff4d6dfc2d) ) // alt rom with copyright removed (unused)
	ROM_LOAD( "zacb_3.bin",   0x2000, 0x0800, CRC(ff94b015) SHA1(6d8f43db3c98cadb35f70e3bff788e653dc132cd) )
	ROM_LOAD( "zacb_7.bin",   0x2800, 0x0800, CRC(9271c58c) SHA1(e6b8f1807c5852ae4e822d80719a4e8f8b036c31) )
	ROM_LOAD( "zacb_4.bin",   0x3000, 0x0800, CRC(965cf32b) SHA1(68cc573a24c74f2ab417d0330fc9523e77fda961) )
	ROM_LOAD( "zacb_8.bin",   0x3800, 0x0800, CRC(c254e92e) SHA1(023b45403ebc69c29516d77950dc69f05a1a130c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "x.bin",           0x0000, 0x0800, CRC(59dce22e) SHA1(81eaef3e4d8299b5133b62d04460abfa519696f5) )
	ROM_LOAD( "c.bin",           0x0800, 0x0800, CRC(aaa7a537) SHA1(571d981ed2aad62d7c7f2798e9084228d45523d4) )
	ROM_LOAD( "b.bin",           0x1000, 0x0800, CRC(1969792b) SHA1(7c3e2ace75402ad227e6437785b7cfec4db88db8) )
	ROM_LOAD( "p.bin",           0x1800, 0x0800, CRC(99af4b30) SHA1(6a0939ff2fa7ae39a960dd4d9f9b7c01f57647c5) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


/*
Birdiy by Mama Top

Pcb marked Mama.Top MDK-13V-0  FCC  Made in japan

1x 18.432mhz OSC
1x Z80 LH0080 by Sharp running at 3.069mhz (18.432mhz/6)
6x dynamic rams HN472114P-3 near the CPU
1x dipswitch
6x HN462732 eproms
4x 82s129N proms
1x DIL 18 pin chip with markings scratched out in position 7M
1x HA1386 amplifier
No sound chip, probably made of discrete logic?

Note: the marking MDK-xxV-x is sometimes found on Nanao manufactured pcbs for Irem, so it very
likely that the board was manufactured by Nanao
(example: Major Title and Gussun Oyoyo are marked MDK-311V-0, M92 and M107 pcbs are marked MDK-321-V0)

Board supplied by Alberto Grego
Dumped by Corrado Tomaselli
*/

ROM_START( birdiy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a6.6a",      0x0000, 0x001000, CRC(3a58f8ad) SHA1(39e990ad4ee1fdec248665149bdb1072c8c01a9a) )
	ROM_LOAD( "c6.6c",      0x1000, 0x001000, CRC(fec61ea2) SHA1(f7ff528d2bcede2434d0a33ee0193b50113ef720) )
	ROM_LOAD( "a4.4a",      0x2000, 0x001000, CRC(3392783b) SHA1(e477f4284fd5b6c9f3619bd35cee6dbe8a2456b8) )
	ROM_LOAD( "c4.4c",      0x3000, 0x001000, CRC(2391d83d) SHA1(6933f1e11a7a84c26a3a45b240e55157a2400e9c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "c1.1c",      0x0000, 0x001000, CRC(8f6bf54f) SHA1(6e09a9c2b143908766837529b174f97dd5058b53) )
	ROM_LOAD( "c3.3c",      0x1000, 0x001000, CRC(10b55440) SHA1(4f3cf5d8954725cf791146abf8918c43138602e1) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "n82s123n.10n", 0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "n82s129n.9m",  0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "n82s129n.4k",  0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "n82s129n.6l",  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( mrtnt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tnt.1",        0x0000, 0x1000, CRC(0e836586) SHA1(5037b7c618f05bc3d6a33694729ae575b9aa7dbb) )
	ROM_LOAD( "tnt.2",        0x1000, 0x1000, CRC(779c4c5b) SHA1(5ecac4f5b64b306c73d8f57d5260b586789b3055) )
	ROM_LOAD( "tnt.3",        0x2000, 0x1000, CRC(ad6fc688) SHA1(e5729e4e42a5b9b3a26de8a44b3a78b49c8b1d8e) )
	ROM_LOAD( "tnt.4",        0x3000, 0x1000, CRC(d77557b3) SHA1(689746653b1e19fbcddd0d71db2b86d1019235aa) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tnt.5",        0x0000, 0x1000, CRC(3038cc0e) SHA1(f8f5927ea4cbfda8fa7546abd766ba2e8b020004) )
	ROM_LOAD( "tnt.6",        0x1000, 0x1000, CRC(97634d8b) SHA1(4c0fa4bc44bbb4b4614b5cc05e811c469c0e78e8) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( gorkans )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gorkans8.rom",        0x0000, 0x0800, CRC(55100b18) SHA1(8f657c1b2865987b60d95960c5297a82bb1cc6e0) )
	ROM_LOAD( "gorkans4.rom",        0x0800, 0x0800, CRC(b5c604bf) SHA1(0f3608d630fba9d4734a3ef30199a5d1a067cdff) )
	ROM_LOAD( "gorkans7.rom",        0x1000, 0x0800, CRC(b8c6def4) SHA1(58ac78fc5b3559ef771ca708a79089b7a00cf6b8) )
	ROM_LOAD( "gorkans3.rom",        0x1800, 0x0800, CRC(4602c840) SHA1(c77de0e991c44c2ee8a4537e264ac8fbb1b4b7db) )
	ROM_LOAD( "gorkans6.rom",        0x2000, 0x0800, CRC(21412a62) SHA1(ece44c3204cf182db23b594ebdc051b51340ba2b) )
	ROM_LOAD( "gorkans2.rom",        0x2800, 0x0800, CRC(a013310b) SHA1(847ba7ca033eaf49245bef49d6513619edec3472) )
	ROM_LOAD( "gorkans5.rom",        0x3000, 0x0800, CRC(122969b2) SHA1(0803e1ec5e5ed742ea83ff156ae75a2d48530f71) )
	ROM_LOAD( "gorkans1.rom",        0x3800, 0x0800, CRC(f2524b11) SHA1(1216b963e73c1de63cc323e361875f6810d83a05) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "gorkgfx4.rom",        0x0000, 0x0800, CRC(39cd0dbc) SHA1(8d6882dad94b26da8f0737e7f7f99946fe273f1b) )
	ROM_LOAD( "gorkgfx2.rom",        0x0800, 0x0800, CRC(33d52535) SHA1(e78ac5afa1ce996c41005c619ba2d2aa718497fc) )
	ROM_LOAD( "gorkgfx3.rom",        0x1000, 0x0800, CRC(4b6b7970) SHA1(1d8b65cad0b834fb920135fc907432042bc83db2) )
	ROM_LOAD( "gorkgfx1.rom",        0x1800, 0x0800, CRC(ed70bb3c) SHA1(7e51ddcf496f3b80fe186acc8bc6a0e574340346) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "gorkprom.4",   0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "gorkprom.1",   0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "gorkprom.3",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "gorkprom.2"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /* timing - not used */
ROM_END

ROM_START( eggor )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x0800, CRC(818ed154) SHA1(8c0f555a3ab1d20a2c284d721b31278a0ddf9e51) )
	ROM_LOAD( "5.bin",        0x0800, 0x0800, CRC(a4b21d93) SHA1(923b7a06f9146c7bcda4cdb16b15d2bbbec95eab) )
	ROM_LOAD( "2.bin",        0x1000, 0x0800, CRC(5d7a23ed) SHA1(242fd973b0bde91c38e1f5e7f6c53d737019ec9c) )
	ROM_LOAD( "6.bin",        0x1800, 0x0800, CRC(e9dbca8d) SHA1(b66783d68df778910cc190159aba07b476ff01af) )
	ROM_LOAD( "3.bin",        0x2000, 0x0800, CRC(4318ab85) SHA1(eda9bb1bb8102e1c2cf838d0682732a45609f430) )
	ROM_LOAD( "7.bin",        0x2800, 0x0800, CRC(03214d7f) SHA1(0e1b602fbdedfe81452109912fed006653bdc455) )
	ROM_LOAD( "4.bin",        0x3000, 0x0800, CRC(dc805be4) SHA1(18604b221cd8af23ff8a05c954a42c3aa9e1948a) )
	ROM_LOAD( "8.bin",        0x3800, 0x0800, CRC(f9ae204b) SHA1(53022d2d7b83f44c46fdcca454815cf1f65c34d1) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "9.bin",        0x0000, 0x0800, CRC(96ad8626) SHA1(f003a6e1b00a51bfe326eac18658fafd58c88f88) )
	ROM_LOAD( "11.bin",       0x0800, 0x0800, CRC(cc324017) SHA1(ea96572e3e24714033688fe7ca99af2fc707c1d3) )
	ROM_LOAD( "10.bin",       0x1000, 0x0800, CRC(7c97f513) SHA1(6f78c7cde321ea6ac51d08d0e3620653d0af87db) )
	ROM_LOAD( "12.bin",       0x1800, 0x0800, CRC(2e930602) SHA1(4012ec0cc542061b27b9b508bedde3f2ffc11838) )

	ROM_REGION( 0x0120, "proms", 0 )
	/* the board was stripped of its proms, these are the standard ones from Pacman, they look reasonable
	   but without another board its impossible to say if they are actually good */
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, BAD_DUMP CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

ROM_START( lizwiz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6e.cpu",       0x0000, 0x1000, CRC(32bc1990) SHA1(467c9d70e07f403b6b9118aebe4e6d0abb40a5c1) )
	ROM_LOAD( "6f.cpu",       0x1000, 0x1000, CRC(ef24b414) SHA1(12fce48008c4f9387df0c84f3b0d7c5a1b35d898) )
	ROM_LOAD( "6h.cpu",       0x2000, 0x1000, CRC(30bed83d) SHA1(8c2458f98320c6887580c71632b544da0a582ba2) )
	ROM_LOAD( "6j.cpu",       0x3000, 0x1000, CRC(dd09baeb) SHA1(f91447ec1f06bf95106e6872d80dcb82e1d42ffb) )
	ROM_LOAD( "wiza",         0x8000, 0x1000, CRC(f6dea3a6) SHA1(ec0b123fd2e6de6681ca14f35fda249b2c2ec44f) )
	ROM_LOAD( "wizb",         0x9000, 0x1000, CRC(f27fb5a8) SHA1(3ea384a1064302709d97fc16b347d3c012e90ac7) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e.cpu",       0x0000, 0x1000, CRC(45059e73) SHA1(c960cd5720bfa21db73e1000fe8be7d5baf2a3a1) )
	ROM_LOAD( "5f.cpu",       0x1000, 0x1000, CRC(d2469717) SHA1(194c8f816e5ff7614b3db4f355223667105738fa) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7f.cpu",       0x0000, 0x0020, CRC(7549a947) SHA1(4f2c3e7d6c38f0b9a90317f91feb3f86c9a0d0a5) )
	ROM_LOAD( "4a.cpu",       0x0020, 0x0100, CRC(5fdca536) SHA1(3a09b29374031aaa3722932aff974a467b3bb201) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( theglobp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "glob.u2",      0x0000, 0x2000, CRC(829d0bea) SHA1(89f52b459a03fb40b9bbd97ac8a292f7ead6faba) )
	ROM_LOAD( "glob.u3",      0x2000, 0x2000, CRC(31de6628) SHA1(35a47dcf34efd74b5b2fda137e06a3dcabd74854) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "glob.5e",      0x0000, 0x1000, CRC(53688260) SHA1(9ce0d1d67d12743b69e8190bf7506b00b2f02955) )
	ROM_LOAD( "glob.5f",      0x1000, 0x1000, CRC(051f59c7) SHA1(e1e1322686997e5bcdac164704b328cce352ae42) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "glob.7f",      0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "glob.4a",      0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


//Program roms same as the globp
ROM_START( sprglobp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "glob.u2",      0x0000, 0x2000, CRC(829d0bea) SHA1(89f52b459a03fb40b9bbd97ac8a292f7ead6faba) )
	ROM_LOAD( "glob.u3",      0x2000, 0x2000, CRC(31de6628) SHA1(35a47dcf34efd74b5b2fda137e06a3dcabd74854) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e_2532.dat",  0x0000, 0x1000, CRC(1aa16109) SHA1(ddc8606512d7ab7555b84146b9d793f65ad0a75f) )
	ROM_LOAD( "5f_2532.dat",  0x1000, 0x1000, CRC(afe72a89) SHA1(fb17632e2665c3cebc1865ef25fa310cc52725c4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "glob.7f",      0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "glob.4a",      0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END

/* This set is from a modified Pengo board.  Pengo and Pacman are functionally the same.
   The bad sound is probably correct as the sound data is part of the protection. */
ROM_START( sprglbpg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic8.1",      0x0000, 0x1000, CRC(a2df2073) SHA1(14c55186053b080de06cc3691111ede8b2ead231) )
	ROM_LOAD( "ic7.2",      0x1000, 0x1000, CRC(3d2c22d9) SHA1(2f1d27e49850f904d1f2256bfcf00557ed88bb16) )
	ROM_LOAD( "ic15.3",      0x2000, 0x1000, CRC(a252047f) SHA1(9fadbb098b86ee98e1a81da938316b833fc26912) )
	ROM_LOAD( "ic14.4",      0x3000, 0x1000, CRC(7efa81f1) SHA1(583999280623f02dcc318a6c7af5ee6fc46144b8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ic92.5",  0x0000, 0x2000, CRC(e54f484d) SHA1(4feb9ec917c2467a5ac531283cb00fe308be7775) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "ic78.prm",      0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "ic88.prm",      0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "ic51.prm",    0x0000, 0x0100, CRC(c29dea27) SHA1(563c9770028fe39188e62630711589d6ed242a66) )
	ROM_LOAD( "ic70.prm"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( beastf )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "bf-u2.bin",    0x0000, 0x2000, CRC(3afc517b) SHA1(5b74bca9e9cd4d8bcf94a340f8f0e53fe1dcfc1d) )
	ROM_LOAD( "bf-u3.bin",    0x2000, 0x2000, CRC(8dbd76d0) SHA1(058c01e87ad583eb99d5043a821e6c68f1b30267) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "beastf.5e",    0x0000, 0x1000, CRC(5654dc34) SHA1(fc2336b951a3ab48d4fc4f36a8dd80e79e8ca1a0) )
	ROM_LOAD( "beastf.5f",    0x1000, 0x1000, CRC(1b30ca61) SHA1(8495d8a280346246f00c4f1dc42ab5a2a02c5863) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "glob.7f",      0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "glob.4a",      0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( vanvan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "van-1.50",     0x0000, 0x1000, CRC(cf1b2df0) SHA1(938b4434c0129cf9151f829901d00e47dca68956) )
	ROM_LOAD( "van-2.51",     0x1000, 0x1000, CRC(df58e1cb) SHA1(5e0fc713b50d46c7650d6564c20882891864cdc5) )
	ROM_LOAD( "van-3.52",     0x2000, 0x1000, CRC(15571e24) SHA1(d259d81fce16e151b32ac81f94a13b7044fdef95) )
	ROM_LOAD( "van-4.53",     0x3000, 0x1000, CRC(b724cbe0) SHA1(5fe1d3b81d07c538c31daf6522b26bbf35cfc512) )
	ROM_LOAD( "van-5.39",     0x8000, 0x1000, CRC(db67414c) SHA1(19eba21dfea24507b386ea1b5ce737c5822b0696) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "van-20.18",    0x0000, 0x1000, CRC(60efbe66) SHA1(ac398f77bfeab3d18ffd496e117825bfbeed4b62) )
	ROM_LOAD( "van-21.19",    0x1000, 0x1000, CRC(5dd53723) SHA1(f75c869ac364f477d532e695347ceb5e281f9efa) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6331-1.6",     0x0000, 0x0020, CRC(ce1d9503) SHA1(b829bed78c02d9998c1aecb8f6813e90b417a7f2) )
	ROM_LOAD( "6301-1.37",    0x0020, 0x0100, CRC(4b803d9f) SHA1(59b7f2e22c4e0b20ac3b12d88996a6dfeebc5933) )
ROM_END

ROM_START( vanvank )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "van1.bin",     0x0000, 0x1000, CRC(00f48295) SHA1(703fab63760cadcce042b491d7d1d45301319158) )
	ROM_LOAD( "van-2.51",     0x1000, 0x1000, CRC(df58e1cb) SHA1(5e0fc713b50d46c7650d6564c20882891864cdc5) )
	ROM_LOAD( "van-3.52",     0x2000, 0x1000, CRC(15571e24) SHA1(d259d81fce16e151b32ac81f94a13b7044fdef95) )
	ROM_LOAD( "van4.bin",     0x3000, 0x1000, CRC(f8b37ed5) SHA1(34f844be891dfa5f6a1160de6f428e9dacd618a8) )
	ROM_LOAD( "van5.bin",     0x8000, 0x1000, CRC(b8c1e089) SHA1(c614fb9159210f6cf68f5085bfebd928caded91c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "van-20.18",    0x0000, 0x1000, CRC(60efbe66) SHA1(ac398f77bfeab3d18ffd496e117825bfbeed4b62) )
	ROM_LOAD( "van-21.19",    0x1000, 0x1000, CRC(5dd53723) SHA1(f75c869ac364f477d532e695347ceb5e281f9efa) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6331-1.6",     0x0000, 0x0020, CRC(ce1d9503) SHA1(b829bed78c02d9998c1aecb8f6813e90b417a7f2) )
	ROM_LOAD( "6301-1.37",    0x0020, 0x0100, CRC(4b803d9f) SHA1(59b7f2e22c4e0b20ac3b12d88996a6dfeebc5933) )
ROM_END


ROM_START( vanvanb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vv1.bin",     0x0000, 0x1000, CRC(cf1b2df0) SHA1(938b4434c0129cf9151f829901d00e47dca68956) )
	ROM_LOAD( "vv2.bin",     0x1000, 0x1000, CRC(80eca6a5) SHA1(e3e711e5c27f5effdae95222a019e427a754d505) )
	ROM_LOAD( "vv3.bin",     0x2000, 0x1000, CRC(15571e24) SHA1(d259d81fce16e151b32ac81f94a13b7044fdef95) )
	ROM_LOAD( "vv4.bin",     0x3000, 0x1000, CRC(b1f04006) SHA1(f0dccce9cc9871ff671e86947512f354ff5f4f13) )
	ROM_LOAD( "vv5.bin",     0x8000, 0x1000, CRC(db67414c) SHA1(19eba21dfea24507b386ea1b5ce737c5822b0696) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "vv20.bin",    0x0000, 0x1000, CRC(eb56cb51) SHA1(fa8c4f61acbbc0ea7b41aff2624dadcc581ddf8c) )
	ROM_LOAD( "vv21.bin",    0x1000, 0x1000, CRC(5dd53723) SHA1(f75c869ac364f477d532e695347ceb5e281f9efa) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6331-1.6",     0x0000, 0x0020, CRC(ce1d9503) SHA1(b829bed78c02d9998c1aecb8f6813e90b417a7f2) )
	ROM_LOAD( "6301-1.37",    0x0020, 0x0100, CRC(4b803d9f) SHA1(59b7f2e22c4e0b20ac3b12d88996a6dfeebc5933) )
ROM_END


ROM_START( dremshpr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "red_1.50",     0x0000, 0x1000, CRC(830c6361) SHA1(a28c517a9b7f509e0dedacea64b9740335315457) )
	ROM_LOAD( "red_2.51",     0x1000, 0x1000, CRC(d22551cc) SHA1(2c513908899b618f0c0a0c3e48c4a4aad90f627e) )
	ROM_LOAD( "red_3.52",     0x2000, 0x1000, CRC(0713a34a) SHA1(37733b557e6afe116f5d3c8bc918f59124a8229d) )
	ROM_LOAD( "red_4.53",     0x3000, 0x1000, CRC(f38bcaaa) SHA1(cdebeaf5b77ac5a8b4668cff97b6351e075b392b) )
	ROM_LOAD( "red_5.39",     0x8000, 0x1000, CRC(6a382267) SHA1(7d6a1c75de8a6eb714ba9a18dd3c497832145bcc) )
	ROM_LOAD( "red_6.40",     0x9000, 0x1000, CRC(4cf8b121) SHA1(04162b41e747dfa442b958bd360e49993c5c4162) )
	ROM_LOAD( "red_7.41",     0xa000, 0x1000, CRC(bd4fc4ba) SHA1(50a5858acde5fd4b3476f5502141e7d492c3af9f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "red-20.18",    0x0000, 0x1000, CRC(2d6698dc) SHA1(5f5e54fdcff53c6ba783d585cd994cf563c53613) )
	ROM_LOAD( "red-21.19",    0x1000, 0x1000, CRC(38c9ce9b) SHA1(c719bcd77549228e72ad9bcc42f5db0070ec5dca) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6331-1.6",     0x0000, 0x0020, CRC(ce1d9503) SHA1(b829bed78c02d9998c1aecb8f6813e90b417a7f2) )
	ROM_LOAD( "6301-1.37",    0x0020, 0x0100, CRC(39d6fb5c) SHA1(848f9cd02f90006e8a2aae3693b57ae391cf498b) )
ROM_END


ROM_START( alibaba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6e",           0x0000, 0x1000, CRC(38d701aa) SHA1(4e886a4a17f441f6d1d213c4c433b40dd38eefbc) )
	ROM_LOAD( "6f",           0x1000, 0x1000, CRC(3d0e35f3) SHA1(6b9a1fd11db9f521417566ae4c7065151aa272b5) )
	ROM_LOAD( "6h",           0x2000, 0x1000, CRC(823bee89) SHA1(5381a4fcbc9fa97574c6df2978c7500164df75e5) )
	ROM_LOAD( "6k",           0x3000, 0x1000, CRC(474d032f) SHA1(4516a60ec83e3c3388cd56f538f49afc86a50983) )
	ROM_LOAD( "6l",           0x8000, 0x1000, CRC(5ab315c1) SHA1(6f3507ad10432f9123150b8bc1d0fb52372a412b) )
	ROM_LOAD( "6m",           0xa000, 0x0800, CRC(438d0357) SHA1(7caaf668906b76d4947e988c444723b33f8e9726) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x0800, CRC(85bcb8f8) SHA1(986170627953582b1e6fbca59e5c15cf8c4de9c7) )
	ROM_LOAD( "5h",           0x0800, 0x0800, CRC(38e50862) SHA1(094d090bd0563f75d6ff1bfe2302cae941a89504) )
	ROM_LOAD( "5f",           0x1000, 0x0800, CRC(b5715c86) SHA1(ed6aee778295b0182d32846b5e41776b5b15420c) )
	ROM_LOAD( "5k",           0x1800, 0x0800, CRC(713086b3) SHA1(a1609bae637207a82920678f05bcc10a5ff096de) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.e7",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.a4",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	/* unknown, used for the mystery items ? */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "ab7.bin",      0x0000, 0x0800, CRC(52294ef5) SHA1(1d76e16c95cb2873d898a4151a902113fccafe1c) ) // 7.p6 dumped as 0x1000 - 1ST AND 2ND HALF IDENTICAL
ROM_END


ROM_START( alibabab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6e",           0x0000, 0x1000, CRC(38d701aa) SHA1(4e886a4a17f441f6d1d213c4c433b40dd38eefbc) ) // ab1.bin
	ROM_LOAD( "6f",           0x1000, 0x1000, CRC(3d0e35f3) SHA1(6b9a1fd11db9f521417566ae4c7065151aa272b5) ) // ab2.bin
	ROM_LOAD( "6h",           0x2000, 0x1000, CRC(823bee89) SHA1(5381a4fcbc9fa97574c6df2978c7500164df75e5) ) // ab3.bin
	ROM_LOAD( "6k",           0x3000, 0x1000, CRC(474d032f) SHA1(4516a60ec83e3c3388cd56f538f49afc86a50983) ) // ab4.bin
	ROM_LOAD( "ab5.bin",      0x8000, 0x1000, CRC(ae2f4aac) SHA1(8583514e4a876ecdb1bc2c65f9829f0bebfdee0d) )
	ROM_LOAD( "6m",           0xa000, 0x0800, CRC(438d0357) SHA1(7caaf668906b76d4947e988c444723b33f8e9726) ) // ab6.bin

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x0800, CRC(85bcb8f8) SHA1(986170627953582b1e6fbca59e5c15cf8c4de9c7) ) // ab8.bin
	ROM_LOAD( "5h",           0x0800, 0x0800, CRC(38e50862) SHA1(094d090bd0563f75d6ff1bfe2302cae941a89504) ) // ab10.bin
	ROM_LOAD( "5f",           0x1000, 0x0800, CRC(b5715c86) SHA1(ed6aee778295b0182d32846b5e41776b5b15420c) ) // ab9.bin
	ROM_LOAD( "5k",           0x1800, 0x0800, CRC(713086b3) SHA1(a1609bae637207a82920678f05bcc10a5ff096de) ) // ab11.bin

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.e7",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.a4",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	/* unknown, used for the mystery items ? */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "ab7.bin",      0x0000, 0x0800, CRC(52294ef5) SHA1(1d76e16c95cb2873d898a4151a902113fccafe1c) )
ROM_END


ROM_START( jumpshot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "6e",           0x0000, 0x1000, CRC(f00def9a) SHA1(465a7f368e61a1e6614d6eab0fa2c6319920eaa5) )
	ROM_LOAD( "6f",           0x1000, 0x1000, CRC(f70deae2) SHA1(a8a8369e865b62cb9ed66d3de2396c6a5fced549) )
	ROM_LOAD( "6h",           0x2000, 0x1000, CRC(894d6f68) SHA1(8693ffc29587cdd1be0b42cede53f8f450a2c7fa) )
	ROM_LOAD( "6j",           0x3000, 0x1000, CRC(f15a108a) SHA1(db5c8394f688c6f889cadddeeae4fbca63c29a4c) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(d9fa90f5) SHA1(3c37fe077a77baa802230dddbc4bb2c05985d2bb) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(2ec711c1) SHA1(fcc3169f48eb7d4af533ad0169701e4230ff5a1f) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "prom.7f",      0x0000, 0x0020, CRC(872b42f3) SHA1(bbcd392ba3d2a5715e92fa0f7a7cf1e7e6e655a2) )
	ROM_LOAD( "prom.4a",      0x0020, 0x0100, CRC(0399f39f) SHA1(e98f08da4666cab44e01acb760a1bd2fc858bc0d) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( jumpshotp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "js6e.bin",           0x0000, 0x1000, CRC(acc5e15e) SHA1(c9516f2d0862b29a3efe19eb13ab68decd670ca8) )
	ROM_LOAD( "js6f.bin",           0x1000, 0x1000, CRC(62b48ba4) SHA1(a17d8ca68de6116822cf4eff70eada04a3fbb4c3) )
	ROM_LOAD( "js6h.bin",           0x2000, 0x1000, CRC(7c9b5e30) SHA1(44c7694b8bd774550ec865d133f5660b90350428) )
	ROM_LOAD( "js6j.bin",           0x3000, 0x1000, CRC(9f0c39f6) SHA1(8714c9b0853206ec5d79155b4310195b46fafbf6) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(d9fa90f5) SHA1(3c37fe077a77baa802230dddbc4bb2c05985d2bb) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(2ec711c1) SHA1(fcc3169f48eb7d4af533ad0169701e4230ff5a1f) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "prom.7f",      0x0000, 0x0020, CRC(872b42f3) SHA1(bbcd392ba3d2a5715e92fa0f7a7cf1e7e6e655a2) )
	ROM_LOAD( "prom.4a",      0x0020, 0x0100, CRC(0399f39f) SHA1(e98f08da4666cab44e01acb760a1bd2fc858bc0d) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( shootbul )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sb6e.cpu",     0x0000, 0x1000, CRC(25daa5e9) SHA1(8257de5e0e62235d05d74b53e5fc716e85cb05b9) )
	ROM_LOAD( "sb6f.cpu",     0x1000, 0x1000, CRC(92144044) SHA1(905a354a806da47ab40577171acdac7db635d102) )
	ROM_LOAD( "sb6h.cpu",     0x2000, 0x1000, CRC(43b7f99d) SHA1(6372763fbbca3581376204c5e58ceedd3f47fc60) )
	ROM_LOAD( "sb6j.cpu",     0x3000, 0x1000, CRC(bc4d3bbf) SHA1(2fa15b339166b9a5bf711b58a1705bc0b9e528e2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "sb5e.cpu",     0x0000, 0x1000, CRC(07c6c5aa) SHA1(cbe99ece795f29fdeef374cbf9b1f45ff065e803) )
	ROM_LOAD( "sb5f.cpu",     0x1000, 0x1000, CRC(eaec6837) SHA1(ff21b0fd5381afb1ba7f5920132006ee8e6d10eb) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7f.rom",       0x0000, 0x0020, CRC(ec578b98) SHA1(196da49cc260f967ec5f01bc3c75b11077c85998) )
	ROM_LOAD( "4a.rom",       0x0020, 0x0100, CRC(81a6b30f) SHA1(60c767fd536c325151a2b759fdbce4ba41e0c78f) )

	ROM_REGION( 0x0200, "namco", 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( acitya )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "aca_u2.bin",   0x0000, 0x2000, CRC(261c2fdc) SHA1(b4e7e6c8d8e401c7e4673213074802a73b9886a2) )
	ROM_LOAD( "aca_u3.bin",   0x2000, 0x2000, CRC(05fab4ca) SHA1(5172229eda25920eeaa6d9f610f2bcfa674979b7) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "aca_5e.bin",   0x0000, 0x1000, CRC(7f2dd2c9) SHA1(aa7ea70355904989b99d568d1e055e8272cfa8ca) )
	ROM_RELOAD( 0x1000, 0x1000 ) /* Not Used?? */

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "aca_7f.bin",   0x0000, 0x0020, CRC(133bb744) SHA1(da4074f3ea30202973f0b6c9ad05a992bb44eafd) )
	ROM_LOAD( "aca_4a.bin",   0x0020, 0x0100, CRC(8e29208f) SHA1(a30a405fbd43d27a8d403f6c3545178564dede5d) )

	ROM_REGION( 0x0200, "namco", 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END

ROM_START( bwcasino )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "bwc_u2.bin",   0x0000, 0x2000, CRC(e2eea868) SHA1(9e9dae02ab746ef48981f42a75c192c5aae0ffee) )
	ROM_LOAD( "bwc_u3.bin",   0x2000, 0x2000, CRC(a935571e) SHA1(ab4f53be2544593fc8eb4c4bcccdec4191c0c626) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "bwc_5e.bin",   0x0000, 0x1000, CRC(e334c01e) SHA1(cc6e50e3cf51eb8b7b27aa7351733954da8128ff) )
	ROM_RELOAD( 0x1000, 0x1000 ) /* Not Used?? */

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "aca_7f.bin",   0x0000, 0x0020, CRC(133bb744) SHA1(da4074f3ea30202973f0b6c9ad05a992bb44eafd) )
	ROM_LOAD( "aca_4a.bin",   0x0020, 0x0100, CRC(8e29208f) SHA1(a30a405fbd43d27a8d403f6c3545178564dede5d) )

	ROM_REGION( 0x0200, "namco", 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */
ROM_END


ROM_START( newpuc2 )
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "6e.cpu", 0x0000, 0x0800, CRC(69496a98) SHA1(2934051d6305cc3654951bc1aacf2b8902f463fe) )
	ROM_LOAD( "6k.cpu", 0x0800, 0x0800, CRC(158fc01c) SHA1(2f7a1e24d259fdc716ef8e7354a87780595f3c4e) )
	ROM_LOAD( "6f.cpu", 0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "6m.cpu", 0x1800, 0x0800, CRC(70810ccf) SHA1(3941678606aab1e53356a6781e24d84e83cc88ce) )
	ROM_LOAD( "6h.cpu", 0x2000, 0x0800, CRC(81719de8) SHA1(e886d04ac0e20562a4bd2df7676bdf9aa98665d7) )
	ROM_LOAD( "6n.cpu", 0x2800, 0x0800, CRC(3f250c58) SHA1(53bf2270c26f10f7e97960cd4c96e09e16b9bdf3) )
	ROM_LOAD( "6j.cpu", 0x3000, 0x0800, CRC(e6675736) SHA1(85d0bb79bc96acbc67fcb70ff4d453c870a6c8ea) )
	ROM_LOAD( "6p.cpu", 0x3800, 0x0800, CRC(1f81e765) SHA1(442d8a82e79ae842f1ffb46369c632c1d0b83161) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "5e.cpu", 0x0000, 0x0800, CRC(2066a0b7) SHA1(6d4ccc27d6be185589e08aa9f18702b679e49a4a) )
	ROM_LOAD( "5h.cpu", 0x0800, 0x0800, CRC(777c70d3) SHA1(ed5ccbeb1102ec9f837577de3aa51317c32520d6) )
	ROM_LOAD( "5f.cpu", 0x1000, 0x0800, CRC(ca8c184c) SHA1(833aa845824ed80777b62f03df36a920ad7c3656) )
	ROM_LOAD( "5j.cpu", 0x1800, 0x0800, CRC(7dc75a81) SHA1(d3fe1cad3b594052d8367685febb2335b0ad62f4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /* timing - not used */
ROM_END


ROM_START( newpuc2b )
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "np2b1.bin", 0x0000, 0x0800, CRC(9d027c4a) SHA1(88e094880057451a75cdc2ce9477403021813982) )
	ROM_LOAD( "6k.cpu", 0x0800, 0x0800, CRC(158fc01c) SHA1(2f7a1e24d259fdc716ef8e7354a87780595f3c4e) )
	ROM_LOAD( "6f.cpu", 0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "6m.cpu", 0x1800, 0x0800, CRC(70810ccf) SHA1(3941678606aab1e53356a6781e24d84e83cc88ce) )
	ROM_LOAD( "np2b3.bin", 0x2000, 0x0800, CRC(f5e4b2b1) SHA1(68464f61cc50931f6cd4bb493dd703c139500825) )
	ROM_LOAD( "6n.cpu", 0x2800, 0x0800, CRC(3f250c58) SHA1(53bf2270c26f10f7e97960cd4c96e09e16b9bdf3) )
	ROM_LOAD( "np2b4.bin", 0x3000, 0x0800, CRC(f068e009) SHA1(a30763935e116559d535654827230bb21a5734bb) )
	ROM_LOAD( "np2b8.bin", 0x3800, 0x0800, CRC(1fadcc2f) SHA1(2d636cfc2b52b671ac5a26a03b1195e2cf8d4718) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "5e.cpu", 0x0000, 0x0800, CRC(2066a0b7) SHA1(6d4ccc27d6be185589e08aa9f18702b679e49a4a) )
	ROM_LOAD( "5h.cpu", 0x0800, 0x0800, CRC(777c70d3) SHA1(ed5ccbeb1102ec9f837577de3aa51317c32520d6) )
	ROM_LOAD( "5f.cpu", 0x1000, 0x0800, CRC(ca8c184c) SHA1(833aa845824ed80777b62f03df36a920ad7c3656) )
	ROM_LOAD( "5j.cpu", 0x1800, 0x0800, CRC(7dc75a81) SHA1(d3fe1cad3b594052d8367685febb2335b0ad62f4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /* timing - not used */
ROM_END

ROM_START( pacuman )
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "pacuman01.bin", 0x0000, 0x0800, CRC(ec8c1ed8) SHA1(8c8e1c60b8f4735d736d7ef94f2967cb84a078b1) )
	ROM_LOAD( "pacuman02.bin", 0x0800, 0x0800, CRC(40edaf56) SHA1(a53cdf0f2809e2de99507f22d2c4d0600416d6ea) )
	ROM_LOAD( "pacuman03.bin", 0x1000, 0x0800, CRC(db4f702b) SHA1(a7a22c3ee91ca28ffb20d1f8b54fc6caf48e8d0f) )
	ROM_LOAD( "pacuman04.bin", 0x1800, 0x0800, CRC(28f7257d) SHA1(fc8cbb74c18578b6245e987645327e50c7dd3012) )
	ROM_LOAD( "pacuman05.bin", 0x2000, 0x0800, CRC(212acb41) SHA1(6d784e139dfc7c772a970f3b65718a876a109660) )
	ROM_LOAD( "pacuman06.bin", 0x2800, 0x0800, CRC(e73dd1b9) SHA1(6fe69e96acc0dd638c1d51d32d7ea1e01cc21cb6) )
	ROM_LOAD( "pacuman07.bin", 0x3000, 0x0800, CRC(908a2cb2) SHA1(ea2a0f50878aaf27c46df11644180d224af3bd83) )
	ROM_LOAD( "pacuman08.bin", 0x3800, 0x0800, CRC(cf4ba26c) SHA1(b36b682cfb920b3247d80fe7c4e5fed845bf0272) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "pm1chg1.bin", 0x0000, 0x0800, CRC(2066a0b7) SHA1(6d4ccc27d6be185589e08aa9f18702b679e49a4a) )
	ROM_LOAD( "pm1chg2.bin", 0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pm1chg3.bin", 0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "pm1chg4.bin", 0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /* timing - not used */
ROM_END


ROM_START( nmouse )
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "naumouse.d7", 0x0000, 0x0800, CRC(e447ecfa) SHA1(45bce93f4a4e1c9994fb6b0c81691a14cae43ae5) )
	ROM_LOAD( "naumouse.d6", 0x0800, 0x0800, CRC(2e6f13d9) SHA1(1278bd1ddd84ac5b956cb4d25c151871fab2b1d9) )
	ROM_LOAD( "naumouse.e7", 0x1000, 0x0800, CRC(44a80f97) SHA1(d06ffd96c72c3c8a3c71df564e8f5f9fb289b398) )
	ROM_LOAD( "naumouse.e6", 0x1800, 0x0800, CRC(9c7a46bd) SHA1(04771a99295fc6d3c41807e2c4437ff4e7e4ba4a) )
	ROM_LOAD( "naumouse.h7", 0x2000, 0x0800, CRC(5bc94c5d) SHA1(9238add33bbde151532b7ce3917566d9b4f67c62) )
	ROM_LOAD( "naumouse.h6", 0x2800, 0x0800, CRC(1af29e22) SHA1(628291aa97f5f88793f624af66a0c2b021328ef9) )
	ROM_LOAD( "naumouse.j7", 0x3000, 0x0800, CRC(cc3be185) SHA1(92fdc87256d16c4e400da83e3ca2786012766767) )
	ROM_LOAD( "naumouse.j6", 0x3800, 0x0800, CRC(66b3e5dc) SHA1(0ca7e67ef0ff908bb9953399f024e8b1aaf74e55) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "naumouse.d5", 0x0000, 0x0800, CRC(2ea7cc3f) SHA1(ffeca1c382a7ae0cd898eab2905a0e8e96b95bee) )
	ROM_LOAD( "naumouse.h5", 0x0800, 0x0800, CRC(0511fcea) SHA1(52a498ca024b5c758ad0c978d3f67cdbbf2c56d3) )
	ROM_LOAD( "naumouse.e5", 0x1000, 0x0800, CRC(f5a627cd) SHA1(2b8bc6d29e2aead924423a232c130151c8a8ebe5) )
	ROM_LOAD( "naumouse.j5", 0x1800, 0x0800, CRC(65f2580e) SHA1(769905837b98736ef2bfcaafa7820083dad80c57) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "naumouse.a4", 0x0020, 0x0100, CRC(d8772167) SHA1(782fa53f0de7262924a92d75f12a42bc4e44c812) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /* timing - not used */
ROM_END

ROM_START( nmouseb )
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "naumouse.d7", 0x0000, 0x0800, CRC(e447ecfa) SHA1(45bce93f4a4e1c9994fb6b0c81691a14cae43ae5) )
	ROM_LOAD( "naumouse.d6", 0x0800, 0x0800, CRC(2e6f13d9) SHA1(1278bd1ddd84ac5b956cb4d25c151871fab2b1d9) )
	ROM_LOAD( "naumouse.e7", 0x1000, 0x0800, CRC(44a80f97) SHA1(d06ffd96c72c3c8a3c71df564e8f5f9fb289b398) )
	ROM_LOAD( "naumouse.e6", 0x1800, 0x0800, CRC(9c7a46bd) SHA1(04771a99295fc6d3c41807e2c4437ff4e7e4ba4a) )
	ROM_LOAD( "snatch2.bin", 0x2000, 0x0800, CRC(405aa389) SHA1(687c82d94309c4ed83b72d656dbe7068b1de1b44) )
	ROM_LOAD( "snatch6.bin", 0x2800, 0x0800, CRC(f58e7df4) SHA1(a0853374a2a8c3ab572154d12e2e6297c97bd8b9) )
	ROM_LOAD( "snatch3.bin", 0x3000, 0x0800, CRC(06fb18ec) SHA1(ad57ffdb0fc5acdddeb85c4ce3ad618124fd7a6d) )
	ROM_LOAD( "snatch7.bin", 0x3800, 0x0800, CRC(d187b82b) SHA1(db739d5894a7fbfbc2e384ee1bdfe170935b2df7) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "naumouse.d5",  0x0000, 0x0800, CRC(2ea7cc3f) SHA1(ffeca1c382a7ae0cd898eab2905a0e8e96b95bee) )
	ROM_LOAD( "naumouse.h5",  0x0800, 0x0800, CRC(0511fcea) SHA1(52a498ca024b5c758ad0c978d3f67cdbbf2c56d3) )
	ROM_LOAD( "naumouse.e5",  0x1000, 0x0800, CRC(f5a627cd) SHA1(2b8bc6d29e2aead924423a232c130151c8a8ebe5) )
	ROM_LOAD( "snatch11.bin", 0x1800, 0x0800, CRC(330230a5) SHA1(3de4e454dd51b2ef05b5e1c74c8d12f8cb3f42ef) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "naumouse.a4", 0x0020, 0x0100, CRC(d8772167) SHA1(782fa53f0de7262924a92d75f12a42bc4e44c812) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /* timing - not used */
ROM_END


/*
Wood Pecker
Amenip, 1981

This game uses a fairly large epoxy-filled plug-in security module which
connects to a DIP40 socket on the main board.
The module contains all of the program code and a Z80 plus some logic glue.

Main PCB Layout
---------------

|--------------|---------------|------------------------|
|    18.432MHz | DIP40         |     PR.4A              |
|              |               |                        |
|              |               |                        |
|              |       DIP28   |              6148      |
|-|            |               |                        |
  |            |DIP24  DIP24   |A.5E                    |
|-|            |               |                        |
|3             |DIP24  DIP24   |B.5F                    |
|6       DSW   |               |                        |
|W             |DIP24  DIP24   |C.5H  2114              |
|A     PR.8H   |               |                        |
|Y             |DIP24  DIP24   |D.5J  2114              |
|              |               |                        |
|-|            |---------------|      2114  PR.3K  PR.1K|
  |                                                     |
|-|                                   2114              |
|                                                       |
|                                     2114              |
|                                                       |
|                                     2114              |
|                                                       |
|-------------------------------------------------------|
Notes:
      DIP24      - Empty sockets
      DIP40      - Location where daughter board module connects to main board
      DIP28      - Empty socket
      2114/6148  - RAM
      A/B/C/D    - 2716 2K x8 EPROMs (DIP24)
      DSW        - 8-position Dip Switch
      PR.4A/3K/1K- 82S129 Bipolar PROMs (DIP16)
      PR.8H      - 82S123 Bipolar PROM (DIP16)


Daughter Board PCB Layout
-------------------------

|---------------|
|DIP40          |
|               |
|               |
|  Z80   74LS244|
|               |
|74LS244 74LS244|
|74LS04         |
|74LS10         |
|74LS138   ROM  |
|               |
|ROM       ROM  |
|               |
|ROM       ROM  |
|---------------|
Notes:
      All IC's shown
      Z80   - Sharp LH0080 Z80 CPU, running at 3.072MHz [18.432/6] (DIP40)
      ROM   - 2732 4K x8 EPROMs (DIP24)
      DIP40 - 40 pins extended from bottom of module for connection to main board
*/

ROM_START( woodpeck )
	ROM_REGION( 0x10000, "maincpu",0 )
	/* roms dumped from epoxy block */
	ROM_LOAD( "f.bin", 0x0000, 0x1000, CRC(37ea66ca) SHA1(1779e2af8ffc72ec454a401cf6fa93e77e28576a) )
	ROM_LOAD( "i.bin", 0x8000, 0x1000, CRC(cd115dba) SHA1(51dfa1966fa391654622cd4ffdd09007ec38ea02) )
	ROM_LOAD( "e.bin", 0x9000, 0x1000, CRC(d40b2321) SHA1(0418cb772e24b67fd1d04e06daed33e766c8bc3d) )
	ROM_LOAD( "g.bin", 0xa000, 0x1000, CRC(024092f4) SHA1(4b16a3ff101397af64fc89d9f93bbdb939c8e699) )
	ROM_LOAD( "h.bin", 0xb000, 0x1000, CRC(18ef0fc8) SHA1(4cf3854adbcdd4ca2d855c48acff39fce5be48f7) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "a.5e",  0x0000, 0x0800, CRC(15a87f62) SHA1(df6a6594ea8c6957e420b95e25ca33a9add13c09) )
	ROM_LOAD( "c.5h",  0x0800, 0x0800, CRC(ab4abd88) SHA1(04199a127556159878d719599d57a3548eb14a3c) )
	ROM_LOAD( "b.5f",  0x1000, 0x0800, CRC(5b9ba95b) SHA1(6d963da936c26830a614b69c663fc1e20b70f9dc) )
	ROM_LOAD( "d.5j",  0x1800, 0x0800, CRC(d7b80a45) SHA1(8f4ef319b960ae0e2cb30910b7efe6c0691df2bb) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pr.8h", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "pr.4a", 0x0020, 0x0100, CRC(d8772167) SHA1(782fa53f0de7262924a92d75f12a42bc4e44c812) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "pr.1k", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "pr.3k", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /* timing - not used */
ROM_END

ROM_START( woodpeca )
	ROM_REGION( 0x10000, "maincpu",0 )
	/* roms dumped from epoxy block */
	ROM_LOAD( "0", 0x0000, 0x1000, CRC(b5ee8bca) SHA1(b9a07dafa1b5ac26e28fd6520506c22b12881bc4) )
	ROM_LOAD( "1", 0x8000, 0x1000, CRC(c5ec2de6) SHA1(4014c99761c184466f586848ed3685c0e4bc272c) )
	ROM_LOAD( "2", 0x9000, 0x1000, CRC(07ea534e) SHA1(d93a9c35be21558b553ae8234b7d7e6e7e7e07f0) )
	ROM_LOAD( "3", 0xa000, 0x1000, CRC(a3a3253a) SHA1(e623bf11063570b7a8617a4590a6050cb73f61a2) )
	ROM_LOAD( "4", 0xb000, 0x1000, CRC(6c50546b) SHA1(1ca1c70a1722172036b30f99d7f6bf005dca9b79) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "10.5f", 0x0000, 0x1000, CRC(0bf52102) SHA1(dfd8bb56e25b5599a7fdc9d7db8f9f5f2d7c4b03) )
	ROM_LOAD( "11.5h", 0x1000, 0x1000, CRC(0ed8def8) SHA1(542a6615b45776104f3731a34a899850bb40b5e0) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pr.8h", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "pr.4a", 0x0020, 0x0100, CRC(d8772167) SHA1(782fa53f0de7262924a92d75f12a42bc4e44c812) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "pr.1k", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "pr.3k", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   /* timing - not used */
ROM_END


ROM_START( numcrash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nc-1.6e",      0x0000, 0x0800, CRC(c85a79ba) SHA1(938b6b2920b14a4f0fbcd09fe25873bfd8ddc245) )
	ROM_LOAD( "nc-5.6k",      0x0800, 0x0800, CRC(20e6c64d) SHA1(ff61ede279a46ea2bec534dd82ec89e41a3ec77b) )
	ROM_LOAD( "nc-2.6f",      0x1000, 0x0800, CRC(954a4630) SHA1(09bc3d8d1d4e79a805c5fb436739bc618faf7825) )
	ROM_RELOAD(               0x8000, 0x0800 )
	ROM_LOAD( "nc-6.6m",      0x1800, 0x0800, CRC(7b8de692) SHA1(7d5fe625ee9acf3cced2d98df99f5dee6c8122b1) )
	ROM_LOAD( "nc-3.6h",      0x2000, 0x0800, CRC(e47f7cf3) SHA1(47e513cf4fe80617547093210ca6582646a9b256) )
	ROM_RELOAD(               0x8800, 0x0800 )
	ROM_LOAD( "nc-7.6n",      0x2800, 0x0800, CRC(c6f5bd41) SHA1(854002c6c45f6993ada98b42ed6b814afac0c1f7) )
	ROM_LOAD( "nc-4.6j",      0x3000, 0x0800, CRC(c67450d2) SHA1(b940093c979795d0de4b203200c3f824d5d9ac7f) )
	ROM_LOAD( "nc-8.6p",      0x9000, 0x1000, CRC(7dabb3e5) SHA1(29dd5f6b7acc3abd292c903be6fef9fc4a135287) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "nc-9.5e",      0x0000, 0x0800, CRC(6a30b507) SHA1(e5760844cd051504e61774c57549c2697d473591) )
	ROM_LOAD( "nc-10.5f",     0x0800, 0x0800, CRC(bdd352db) SHA1(c4dd0bd009098a82dd5603046fa929f7395dd903) )
	ROM_LOAD( "nc-11.5h",     0x1000, 0x0800, CRC(5f54d8e6) SHA1(3a20df62f484846e5610b88c04db56f9b5b3029c) )
	ROM_LOAD( "nc-12.5j",     0x1800, 0x0800, CRC(b104a8a9) SHA1(ac7f9b0041a9be1d0ffa58d2f65a0d294e986357) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7051p1.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "7052.4a",      0x0020, 0x0100, CRC(2bc5d339) SHA1(446e234df94d9ef34c3191877bb33dd775acfdf5) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "7611p3.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "7611p2.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


ROM_START( bigbucks )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p.rom",        0x0000, 0x4000, CRC(eea6c1c9) SHA1(eaea4ffbcdfbb38364887830fd00ac87fe838006) )
	ROM_LOAD( "m.rom",        0x8000, 0x2000, CRC(bb8f7363) SHA1(11ebdb1a3c589515240d006646f2fb3ead06bdcf) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e.cpu",       0x0000, 0x1000, CRC(18442c37) SHA1(fac445d15731532364315852492b48470039c0ca) )
	ROM_RELOAD( 0x1000, 0x1000 ) /* Not Used?? */

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */

	ROM_REGION( 0x60000, "user1", 0 )   /* Question ROMs */
	ROM_LOAD( "rom1.rom",     0x00000, 0x8000, CRC(90b7785f) SHA1(7fc5aa2be868b87ffb9e957c204dabf1508e212e) )
	ROM_LOAD( "rom2.rom",     0x08000, 0x8000, CRC(60172d77) SHA1(92cb2312c6f3395f7ddb45e58695dd000d6ec756) )
	ROM_LOAD( "rom3.rom",     0x10000, 0x8000, CRC(a2207320) SHA1(18ad94b62e7e611ab8a1cbf2d2c6576b8840da2f) )
	ROM_LOAD( "rom4.rom",     0x18000, 0x8000, CRC(5a74c1f9) SHA1(0c55a27a492099ac98daefe0c199761ab1ccce82) )
	ROM_LOAD( "rom5.rom",     0x20000, 0x8000, CRC(93bc1080) SHA1(53e40b8bbc82b3be044f8a39b71fbb811e9263d8) )
	ROM_LOAD( "rom6.rom",     0x28000, 0x8000, CRC(eea2423f) SHA1(34de5495061be7d498773f9a723052c4f13d4a0c) )
	ROM_LOAD( "rom7.rom",     0x30000, 0x8000, CRC(96694055) SHA1(64ebbd85c2a60936f60345b5d573cd9eda196d3f) )
	ROM_LOAD( "rom8.rom",     0x38000, 0x8000, CRC(e68ebf8e) SHA1(cac17ac0231a0526e7f4a58bcb2cae3d05727ee6) )
	ROM_LOAD( "rom9.rom",     0x40000, 0x8000, CRC(fd20921d) SHA1(eedf93841b5ebe9afc4184e089d6694bbdb64445) )
	ROM_LOAD( "rom10.rom",    0x48000, 0x8000, CRC(5091b951) SHA1(224b65795d11599cdbd78e984ac2c71e8847041c) )
	ROM_LOAD( "rom11.rom",    0x50000, 0x8000, CRC(705128db) SHA1(92d45bfd09f61a1a3ac46c2e0ec1f634f04cf049) )
	ROM_LOAD( "rom12.rom",    0x58000, 0x8000, CRC(74c776e7) SHA1(03860d90461b01df4b734b784dddb20843ba811a) )
ROM_END


ROM_START( drivfrcp )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "drivforc.1",   0x0000, 0x1000, CRC(10b59d27) SHA1(fa09f3b95319a3487fa54b72198f41211663e087) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "drivforc.2",   0x0000, 0x1000, CRC(56331cb5) SHA1(520f2a18ebbdfb093c8e4d144749a3f5cbce19bf) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x1000, 0x1000 )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_RELOAD(               0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_CONTINUE(             0x5000, 0x1000 )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "drivforc.pr1", 0x0000, 0x0020, CRC(045aa47f) SHA1(ea9034f441937df43a7c0bdb502165fb27d06635) )
	ROM_LOAD( "drivforc.pr2", 0x0020, 0x0100, CRC(9e6d2f1d) SHA1(7bcbcd4c0a40264c3b0667fc6a39ed4f2a86cafe) )
ROM_END


ROM_START( 8bpm )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "8bpmp.bin",    0x0000, 0x1000, CRC(b4f7eba7) SHA1(9b15543895c70f5ee2b4f91b8af78a884453e4f1) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "8bpmc.bin",    0x0000, 0x1000, CRC(1c894a6d) SHA1(04e5c548290095d1d0f873b6c2e639e6dbe8ff35) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x1000, 0x1000 )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_RELOAD(               0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_CONTINUE(             0x5000, 0x1000 )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "8bpm.7f",      0x0000, 0x0020, CRC(4cf54241) SHA1(8d1db311941b8f821f949119d5ed2998a2fee80f) )
	ROM_LOAD( "8bpm.4a",      0x0020, 0x0100, CRC(618505a0) SHA1(6f7d4d54706b49d58427a60c2e2a48bd26d160d4) )
ROM_END


ROM_START( porky )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp",           0x0000, 0x1000, CRC(00592624) SHA1(41e554178a89b95bed1f570fab28e2a04f7a68d6) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_LOAD( "ps",           0x8000, 0x1000, CRC(2efb9861) SHA1(8c5a23ed15bd985af78a54d2121fe058e53703bb) )
	ROM_CONTINUE(             0xa000, 0x1000 )
	ROM_CONTINUE(             0xc000, 0x1000 )
	ROM_CONTINUE(             0xe000, 0x1000 )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "pc",           0x0000, 0x1000, CRC(a20e3d39) SHA1(3762289a495d597d6b9540ea7fa663128a9d543c) )
	ROM_CONTINUE(             0x2000, 0x1000 )
	ROM_CONTINUE(             0x1000, 0x1000 )
	ROM_CONTINUE(             0x3000, 0x1000 )
	ROM_RELOAD(               0x4000, 0x1000 )
	ROM_CONTINUE(             0x6000, 0x1000 )
	ROM_CONTINUE(             0x5000, 0x1000 )
	ROM_CONTINUE(             0x7000, 0x1000 )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7f",           0x0000, 0x0020, CRC(98bce7cc) SHA1(e461862ccaf7526421631ac6ebb9b09cd0bc9909) )
	ROM_LOAD( "4a",           0x0020, 0x0100, CRC(30fe0266) SHA1(5081a19ceaeb937ee1378f3374e9d5949d17c3e8) )
ROM_END


ROM_START( rocktrv2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.aux",        0x0000, 0x4000, CRC(d182947b) SHA1(b778658386b2ed7c9f518cf20d7805ea62ae727b) )
	ROM_LOAD( "2.aux",        0x6000, 0x2000, CRC(27a7461d) SHA1(0cbd4a03dcff352fbd6b9a9009dc908e34553ee2) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e.cpu",       0x0000, 0x1000, CRC(0a6cc43b) SHA1(a773bf3dda326797d63ceb908ad4d48f516bcea0) )
	ROM_RELOAD( 0x1000, 0x1000 ) /* Not Used?? */

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7f.cpu",       0x0000, 0x0020, CRC(7549a947) SHA1(4f2c3e7d6c38f0b9a90317f91feb3f86c9a0d0a5) )
	ROM_LOAD( "4a.cpu",       0x0020, 0x0100, CRC(ddd5d88e) SHA1(f28e1d90bb495001c30c63b0ef2eec45de568174) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) /* timing - not used */

	ROM_REGION( 0x40000, "user1", 0 )   /* Question ROMs */
	ROM_LOAD( "3.aux",        0x00000, 0x4000, CRC(5b117ca6) SHA1(08d625312a751b99e132b90dcf8274d0ff2aecf2) )
	ROM_LOAD( "4.aux",        0x04000, 0x4000, CRC(81bfd4c3) SHA1(300cb4a38d3a1234bfc793f0574527033697f5a2) )
	ROM_LOAD( "5.aux",        0x08000, 0x4000, CRC(e976423c) SHA1(53a7f100943313014285ce09c03bd3eabd1388b0) )
	ROM_LOAD( "6.aux",        0x0c000, 0x4000, CRC(425946bf) SHA1(c8b0ba85bbba2f2c33f4ba069bf2fbb9692281d8) )
	ROM_LOAD( "7.aux",        0x10000, 0x4000, CRC(7056fc8f) SHA1(99c18ba4cd4d45531066069d2fd5018177072d5b) )
	ROM_LOAD( "8.aux",        0x14000, 0x4000, CRC(8b86464f) SHA1(7827df4c763fe078d3844eafab728e9400275049) )
	ROM_LOAD( "9.aux",        0x18000, 0x4000, CRC(17d8eba4) SHA1(806593824868e266c776e2e49cebb60dd6f8302e) )
	ROM_LOAD( "10.aux",       0x1c000, 0x4000, CRC(398c8eb4) SHA1(2cbbb11e255b84a54621f5fccfa8354bf925f1df) )
	ROM_LOAD( "11.aux",       0x20000, 0x4000, CRC(7f376424) SHA1(72ba5b01053c0c568562ba7a1257252c47736a3c) )
	ROM_LOAD( "12.aux",       0x24000, 0x4000, BAD_DUMP CRC(8d5bbf81) SHA1(0ebc9afbe6df6d60cf8797e246dda45694dca89e) )
	ROM_LOAD( "13.aux",       0x28000, 0x4000, CRC(99fe2c21) SHA1(9ff29cb2b74a16f5249677172b9d96e11241032e) )
	ROM_LOAD( "14.aux",       0x2c000, 0x4000, CRC(df4cf5e7) SHA1(1228a31b9053ade416a33f699f3f5513d1e47b24) )
	ROM_LOAD( "15.aux",       0x30000, 0x4000, CRC(2a32de26) SHA1(5892d4aea590d109339a66d15ebedaa04629fa7e) )
	ROM_LOAD( "16.aux",       0x34000, 0x4000, CRC(fcd42187) SHA1(e99e1f281eff2f6f42440f30bcb7a5efe34590fd) )
	ROM_LOAD( "17.aux",       0x38000, 0x4000, CRC(24d5c388) SHA1(f7039d84b3cbf00884e87ea7221f1b608a7d879e) )
	ROM_LOAD( "18.aux",       0x3c000, 0x4000, CRC(feb195fd) SHA1(5677d31e526cc7752254e9af0d694f05bc6bc907) )
ROM_END

/* Special thanks to Rob Walmsley for dumping his PCB */
ROM_START( cannonbp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n1-6e",        0x0000, 0x0800, CRC(c68878c7) SHA1(4ab69f820e861a33fa7555286459e5953c126d33) )
	ROM_LOAD( "n2-6k",        0x0800, 0x0800, CRC(ff3951a5) SHA1(8c06526903f2ce3e0ac29ba0138e8c270732150a) )
	ROM_LOAD( "n3-6f",        0x1000, 0x0800, CRC(2329079d) SHA1(461d1258a03ea35ba33f4094c05f8ffda8cdfc47) )
	ROM_LOAD( "n4-6m",        0x1800, 0x0800, CRC(fcc57ecb) SHA1(9efafbb7f3d44652aded860e734332f47354e299) )
	ROM_LOAD( "n5-6h",        0x2000, 0x0800, CRC(52846c9d) SHA1(4a479ff8961c8865aea12976355d0201a8cb1b48) )
	ROM_LOAD( "n6-6n",        0x2800, 0x0800, CRC(59e890dd) SHA1(c148b71d71fb49a138ef1afe771be70bec21ad2b) )
	/* 3000 - 3fff = epoxy protection block */

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "z1-5e",        0x0000, 0x0800, CRC(125779e0) SHA1(c1ae7214e3ff1a941ae0c4948ec886bc84b6f040) )
	ROM_LOAD( "z3-5h",        0x0800, 0x0800, CRC(78f866c0) SHA1(171f0b5ed6c8b1f2ae8dc1b7e97ea9f0c3d8e18e) )
	ROM_LOAD( "z2-5f",        0x1000, 0x0800, CRC(fbd2c99d) SHA1(135a5d86d59010cc608b099054815dd31d443da7) )
	ROM_LOAD( "z4-5j",        0x1800, 0x0800, CRC(8734c904) SHA1(86d9dc72d3cfc863c558967860ac4d2dc8d2c07c) )

	ROM_REGION( 0x0120, "proms", 0 ) /* these give some ugly colours, but should be correct */
	ROM_LOAD( "colorprom_1",    0x0000, 0x0020, CRC(08f8ae7e) SHA1(cd1e26da5f214f4d9924a30e6d9cf312f91c2028) )
	ROM_LOAD( "colorprom_2",    0x0020, 0x0100, CRC(359a15dc) SHA1(e57ef15eb3baac70fe9e2db897c4165da3c00e20) )

	ROM_REGION( 0x0200, "namco", 0 )    /* sound PROMs */
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    /* timing - not used */
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void pacman_state::maketrax_rom_decode()
{
	UINT8 *rom = memregion("maincpu")->base();

	/* patch protection using a copy of the opcodes so ROM checksum */
	/* tests will not fail */

	memcpy(m_patched_opcodes,rom,0x4000);

	m_patched_opcodes[0x0415] = 0xc9;
	m_patched_opcodes[0x1978] = 0x18;
	m_patched_opcodes[0x238e] = 0xc9;
	m_patched_opcodes[0x3ae5] = 0xe6;
	m_patched_opcodes[0x3ae7] = 0x00;
	m_patched_opcodes[0x3ae8] = 0xc9;
	m_patched_opcodes[0x3aed] = 0x86;
	m_patched_opcodes[0x3aee] = 0xc0;
	m_patched_opcodes[0x3aef] = 0xb0;
}

DRIVER_INIT_MEMBER(pacman_state,maketrax)
{
	/* set up protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5080, 0x50bf, read8_delegate(FUNC(pacman_state::maketrax_special_port2_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x50c0, 0x50ff, read8_delegate(FUNC(pacman_state::maketrax_special_port3_r),this));

	maketrax_rom_decode();
}

void pacman_state::korosuke_rom_decode()
{
	UINT8 *rom = memregion("maincpu")->base();

	/* patch protection using a copy of the opcodes so ROM checksum */
	/* tests will not fail */

	memcpy(m_patched_opcodes,rom,0x4000);

	m_patched_opcodes[0x044c] = 0xc9;
	m_patched_opcodes[0x1973] = 0x18;
	m_patched_opcodes[0x238c] = 0xc9;
	m_patched_opcodes[0x3ae9] = 0xe6;   /* not changed */
	m_patched_opcodes[0x3aeb] = 0x00;
	m_patched_opcodes[0x3aec] = 0xc9;
	m_patched_opcodes[0x3af1] = 0x86;
	m_patched_opcodes[0x3af2] = 0xc0;
	m_patched_opcodes[0x3af3] = 0xb0;
}

DRIVER_INIT_MEMBER(pacman_state,korosuke)
{
	/* set up protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5080, 0x5080, read8_delegate(FUNC(pacman_state::korosuke_special_port2_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x50c0, 0x50ff, read8_delegate(FUNC(pacman_state::korosuke_special_port3_r),this));

	korosuke_rom_decode();
}

DRIVER_INIT_MEMBER(pacman_state,ponpoko)
{
	/* The gfx data is swapped wrt the other Pac-Man hardware games. */
	/* Here we revert it to the usual format. */

	int i, j;
	UINT8 *RAM, temp;
	int length = memregion("gfx1")->bytes()/2;

	/* Characters */
	RAM = memregion("gfx1")->base();
	for (i = 0;i < length;i += 0x10)
	{
		for (j = 0; j < 8; j++)
		{
			temp          = RAM[i+j+0x08];
			RAM[i+j+0x08] = RAM[i+j+0x00];
			RAM[i+j+0x00] = temp;
		}
	}

	/* Sprites */
	RAM = memregion("gfx1")->base()+length;
	for (i = 0;i < length;i += 0x20)
	{
		for (j = 0; j < 8; j++)
		{
			temp          = RAM[i+j+0x18];
			RAM[i+j+0x18] = RAM[i+j+0x10];
			RAM[i+j+0x10] = RAM[i+j+0x08];
			RAM[i+j+0x08] = RAM[i+j+0x00];
			RAM[i+j+0x00] = temp;
		}
	}
}

void pacman_state::eyes_decode(UINT8 *data)
{
	int j;
	UINT8 swapbuffer[8];

	for (j = 0; j < 8; j++)
	{
		swapbuffer[j] = data[BITSWAP16(j,15,14,13,12,11,10,9,8,7,6,5,4,3,0,1,2)];
	}

	for (j = 0; j < 8; j++)
	{
		data[j] = BITSWAP8(swapbuffer[j],7,4,5,6,3,2,1,0);
	}
}

DRIVER_INIT_MEMBER(pacman_state,eyes)
{
	int i, len;
	UINT8 *RAM;

	/* CPU ROMs */

	/* Data lines D3 and D5 swapped */
	RAM = memregion("maincpu")->base();
	for (i = 0; i < 0x4000; i++)
	{
		RAM[i] = BITSWAP8(RAM[i],7,6,3,4,5,2,1,0);
	}


	/* Graphics ROMs */

	/* Data lines D4 and D6 and address lines A0 and A2 are swapped */
	RAM = memregion("gfx1")->base();
	len = memregion("gfx1")->bytes();
	for (i = 0;i < len;i += 8)
		eyes_decode(&RAM[i]);
}


#define BITSWAP12(val,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
	BITSWAP16(val,15,14,13,12,B11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0)

#define BITSWAP11(val,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0) \
	BITSWAP16(val,15,14,13,12,11,B10,B9,B8,B7,B6,B5,B4,B3,B2,B1,B0)

void pacman_state::mspacman_install_patches(UINT8 *ROM)
{
	int i;

	/* copy forty 8-byte patches into Pac-Man code */
	for (i = 0; i < 8; i++)
	{
		ROM[0x0410+i] = ROM[0x8008+i];
		ROM[0x08E0+i] = ROM[0x81D8+i];
		ROM[0x0A30+i] = ROM[0x8118+i];
		ROM[0x0BD0+i] = ROM[0x80D8+i];
		ROM[0x0C20+i] = ROM[0x8120+i];
		ROM[0x0E58+i] = ROM[0x8168+i];
		ROM[0x0EA8+i] = ROM[0x8198+i];

		ROM[0x1000+i] = ROM[0x8020+i];
		ROM[0x1008+i] = ROM[0x8010+i];
		ROM[0x1288+i] = ROM[0x8098+i];
		ROM[0x1348+i] = ROM[0x8048+i];
		ROM[0x1688+i] = ROM[0x8088+i];
		ROM[0x16B0+i] = ROM[0x8188+i];
		ROM[0x16D8+i] = ROM[0x80C8+i];
		ROM[0x16F8+i] = ROM[0x81C8+i];
		ROM[0x19A8+i] = ROM[0x80A8+i];
		ROM[0x19B8+i] = ROM[0x81A8+i];

		ROM[0x2060+i] = ROM[0x8148+i];
		ROM[0x2108+i] = ROM[0x8018+i];
		ROM[0x21A0+i] = ROM[0x81A0+i];
		ROM[0x2298+i] = ROM[0x80A0+i];
		ROM[0x23E0+i] = ROM[0x80E8+i];
		ROM[0x2418+i] = ROM[0x8000+i];
		ROM[0x2448+i] = ROM[0x8058+i];
		ROM[0x2470+i] = ROM[0x8140+i];
		ROM[0x2488+i] = ROM[0x8080+i];
		ROM[0x24B0+i] = ROM[0x8180+i];
		ROM[0x24D8+i] = ROM[0x80C0+i];
		ROM[0x24F8+i] = ROM[0x81C0+i];
		ROM[0x2748+i] = ROM[0x8050+i];
		ROM[0x2780+i] = ROM[0x8090+i];
		ROM[0x27B8+i] = ROM[0x8190+i];
		ROM[0x2800+i] = ROM[0x8028+i];
		ROM[0x2B20+i] = ROM[0x8100+i];
		ROM[0x2B30+i] = ROM[0x8110+i];
		ROM[0x2BF0+i] = ROM[0x81D0+i];
		ROM[0x2CC0+i] = ROM[0x80D0+i];
		ROM[0x2CD8+i] = ROM[0x80E0+i];
		ROM[0x2CF0+i] = ROM[0x81E0+i];
		ROM[0x2D60+i] = ROM[0x8160+i];
	}
}

DRIVER_INIT_MEMBER(pacman_state,mspacman)
{
	int i;
	UINT8 *ROM, *DROM;

	/* CPU ROMs */

	/* Pac-Man code is in low bank */
	ROM = memregion("maincpu")->base();

	/* decrypted Ms. Pac-Man code is in high bank */
	DROM = &memregion("maincpu")->base()[0x10000];

	/* copy ROMs into decrypted bank */
	for (i = 0; i < 0x1000; i++)
	{
		DROM[0x0000+i] = ROM[0x0000+i]; /* pacman.6e */
		DROM[0x1000+i] = ROM[0x1000+i]; /* pacman.6f */
		DROM[0x2000+i] = ROM[0x2000+i]; /* pacman.6h */
		DROM[0x3000+i] = BITSWAP8(ROM[0xb000+BITSWAP12(i,11,3,7,9,10,8,6,5,4,2,1,0)],0,4,5,7,6,3,2,1);  /* decrypt u7 */
	}
	for (i = 0; i < 0x800; i++)
	{
		DROM[0x8000+i] = BITSWAP8(ROM[0x8000+BITSWAP11(i,   8,7,5,9,10,6,3,4,2,1,0)],0,4,5,7,6,3,2,1);  /* decrypt u5 */
		DROM[0x8800+i] = BITSWAP8(ROM[0x9800+BITSWAP12(i,11,3,7,9,10,8,6,5,4,2,1,0)],0,4,5,7,6,3,2,1);  /* decrypt half of u6 */
		DROM[0x9000+i] = BITSWAP8(ROM[0x9000+BITSWAP12(i,11,3,7,9,10,8,6,5,4,2,1,0)],0,4,5,7,6,3,2,1);  /* decrypt half of u6 */
		DROM[0x9800+i] = ROM[0x1800+i];     /* mirror of pacman.6f high */
	}
	for (i = 0; i < 0x1000; i++)
	{
		DROM[0xa000+i] = ROM[0x2000+i];     /* mirror of pacman.6h */
		DROM[0xb000+i] = ROM[0x3000+i];     /* mirror of pacman.6j */
	}
	/* install patches into decrypted bank */
	mspacman_install_patches(DROM);

	/* mirror Pac-Man ROMs into upper addresses of normal bank */
	for (i = 0; i < 0x1000; i++)
	{
		ROM[0x8000+i] = ROM[0x0000+i];
		ROM[0x9000+i] = ROM[0x1000+i];
		ROM[0xa000+i] = ROM[0x2000+i];
		ROM[0xb000+i] = ROM[0x3000+i];
	}

	/* initialize the banks */
	membank("bank1")->configure_entries(0, 2, &ROM[0x00000], 0x10000);
	membank("bank1")->set_entry(1);
}

DRIVER_INIT_MEMBER(pacman_state, mschamp)
{
	save_item(NAME(m_counter));
}

DRIVER_INIT_MEMBER(pacman_state,woodpek)
{
	int i, len;
	UINT8 *RAM;

	/* Graphics ROMs */

	/* Data lines D4 and D6 and address lines A0 and A2 are swapped */
	RAM = memregion("gfx1")->base();
	len = memregion("gfx1")->bytes();
	for (i = 0;i < len;i += 8)
		eyes_decode(&RAM[i]);
}

DRIVER_INIT_MEMBER(pacman_state,pacplus)
{
	pacplus_decode();
}

DRIVER_INIT_MEMBER(pacman_state,jumpshot)
{
	jumpshot_decode();
}

DRIVER_INIT_MEMBER(pacman_state,drivfrcp)
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->set_base(&ROM[0 * 0x2000]);
	membank("bank2")->set_base(&ROM[1 * 0x2000]);
	membank("bank3")->set_base(&ROM[2 * 0x2000]);
	membank("bank4")->set_base(&ROM[3 * 0x2000]);
}

DRIVER_INIT_MEMBER(pacman_state,8bpm)
{
	UINT8 *ROM = memregion("maincpu")->base();
	int i;

	/* Data lines D0 and D6 swapped */
	for( i = 0; i < 0x8000; i++ )
	{
		ROM[i] = BITSWAP8(ROM[i],7,0,5,4,3,2,1,6);
	}

	membank("bank1")->set_base(&ROM[0 * 0x2000]);
	membank("bank2")->set_base(&ROM[1 * 0x2000]);
	membank("bank3")->set_base(&ROM[2 * 0x2000]);
	membank("bank4")->set_base(&ROM[3 * 0x2000]);
}

DRIVER_INIT_MEMBER(pacman_state,porky)
{
	UINT8 *ROM = memregion("maincpu")->base();
	int i;

	/* Data lines D0 and D4 swapped */
	for(i = 0; i < 0x10000; i++)
	{
		ROM[i] = BITSWAP8(ROM[i],7,6,5,0,3,2,1,4);
	}

	membank("bank1")->configure_entries(0, 2, &ROM[0 * 0x2000], 0x8000);
	membank("bank2")->configure_entries(0, 2, &ROM[1 * 0x2000], 0x8000);
	membank("bank3")->configure_entries(0, 2, &ROM[2 * 0x2000], 0x8000);
	membank("bank4")->configure_entries(0, 2, &ROM[3 * 0x2000], 0x8000);

	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
	membank("bank3")->set_entry(0);
	membank("bank4")->set_entry(0);
}

DRIVER_INIT_MEMBER(pacman_state,rocktrv2)
{
	/* hack to pass the rom check for the bad rom */
	UINT8 *ROM = memregion("maincpu")->base();

	ROM[0x7ffe] = 0xa7;
	ROM[0x7fee] = 0x6d;

	save_item(NAME(m_rocktrv2_question_bank));
}

/* The encrpytion is provided by a 74298 sitting on top of the rom at 6f.
The select line is tied to a2; a0 and a1 of the eprom are are left out of
socket and run through the 74298.  Clock is tied to system clock.  */
DRIVER_INIT_MEMBER(pacman_state,mspacmbe)
{
	UINT8 temp;
	UINT8 *RAM = memregion("maincpu")->base();
	int i;

	/* Address lines A1 and A0 swapped if A2=0 */
	for(i = 0x1000; i < 0x2000; i+=4)
	{
		if (!(i & 8))
		{
				temp = RAM[i+1];
				RAM[i+1] = RAM[i+2];
				RAM[i+2] = temp;
		};
	}
}

READ8_MEMBER(pacman_state::mspacii_protection_r)
{
	/* used by extra routine at $3FE, bit 4 of 504d needs to be low, and of 504e to be high */
	UINT8 data = ioport("IN1")->read();
	return (data & 0xef) | (offset << 4 & 0x10);
}

DRIVER_INIT_MEMBER(pacman_state,mspacii)
{
	// protection
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x504d, 0x506f, read8_delegate(FUNC(pacman_state::mspacii_protection_r), this));
}

DRIVER_INIT_MEMBER(pacman_state,superabc)
{
	UINT8 *src = memregion("user1")->base();
	UINT8 *dest = memregion("gfx1")->base();

	// descramble gfx
	for (int i = 0; i < 0x10000; i++)
		dest[i] = src[BITSWAP24(i,23,22,21,20,19,18,17, 12,13,14,16,15, 11,10,9,8,7,6,5,4,3,2,1,0)];
}

READ8_MEMBER(pacman_state::cannonbp_protection_r)
{
	/* At 6p where a rom would usually be there is an epoxy resin chip with 'Novomatic Industrie' Cannon Ball tm 1984 label. */
	/* As I have no clue about what shall be in this chip, what follows is only a simulation which is enough to play the game. */
	switch (offset)
	{
		default:
			logerror("CPU0 %04x: Unhandled protection read, offset %04x\n", space.device().safe_pc(), offset);
			return 0x00;

		case 0x0000: // unknown
		case 0x0003: // unknown
		case 0x0012: // unknown
			return 0x00;

		/* code at 0x2b77 :
		     - after partial checksum (range 0x0000-0x1e7c), HL = 0x9d39
		     - then L += A and HL += 0x717c to determine jump address after bonus round
		       where A is the result of 8 reads from 0x3001
		     - as jump address shall be 0x0efb, A = 0x46
		     - as H after partial checksum is wrong in the bootlegs,
		       they will reset or hang after bonus round
		       unless you patch ROM at 0x2ba0 with this code :
		         2BA0: 21 FB 0E      ld   hl,$0EFB
		         2BA3: 00            nop
		*/
		case 0x0004:
			m_cannonb_bit_to_read = 7;
			return 0x00;
		case 0x0001: // affects the ball hitting the blocks as well as jump address after bonus round
			if (space.device().safe_pc() == 0x2b97)
				return (BIT(0x46, m_cannonb_bit_to_read--) << 7);
			else
				return 0xff;            /* value taken from the bootlegs */

		/* code at 0x2613 : HL += 0xd088 for start position - in the bootlegs, start position = 0x1088 */
		case 0x0105: // player start x position -> register L
			return 0x00;
		case 0x0107: // player start y position -> register H
			return 0x40;
	}
}


DRIVER_INIT_MEMBER(pacman_state,cannonbp)
{
	/* extra memory */
	m_maincpu->space(AS_PROGRAM).install_ram(0x4800, 0x4bff);

	/* protection? */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x3000, 0x3fff, read8_delegate(FUNC(pacman_state::cannonbp_protection_r),this));
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

/*          rom       parent    machine   inp       init */
GAME( 1980, puckman,  0,        pacman,   pacman,   driver_device, 0,        ROT90,  "Namco", "Puck Man (Japan set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, puckmanb, puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "bootleg", "Puck Man (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, puckmanf, puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Puck Man (speedup hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, puckmanh, puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "bootleg (Falcom?)", "Puck Man (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, pacman,   puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "Namco (Midway license)", "Pac-Man (Midway)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, pacmanf,  puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Pac-Man (Midway, speedup hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, puckmod,  puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "Namco", "Puck Man (Japan set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacmod,   puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "Namco (Midway license)", "Pac-Man (Midway, harder)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacmanjpm,puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "bootleg (JPM)", "Pac-Man (JPM bootleg)", MACHINE_SUPPORTS_SAVE ) // aka 'Muncher', UK bootleg, JPM later made fruit machines etc.
GAME( 1980, newpuc2,  puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Newpuc2 (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, newpuc2b, puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Newpuc2 (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, newpuckx, puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "New Puck-X", MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacheart, puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Pac-Man (Hearts)", MACHINE_SUPPORTS_SAVE )
GAME( 198?, bucaner,  puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Buccaneer", MACHINE_SUPPORTS_SAVE )
GAME( 1981, hangly,   puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Hangly-Man (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, hangly2,  puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Hangly-Man (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, hangly3,  puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Hangly-Man (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, popeyeman,puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Popeye-Man", MACHINE_SUPPORTS_SAVE )
GAME( 1980, pacuman,  puckman,  pacman,   pacuman,  driver_device, 0,        ROT90,  "bootleg (Recreativos Franco S.A.)", "Pacu-Man (Spanish bootleg of Puck Man)", MACHINE_SUPPORTS_SAVE ) // common bootleg in Spain, code is shifted a bit compared to the Puck Man sets. Title & Manufacturer info from cabinet/PCB, not displayed ingame
GAME( 1980, crockman, puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "bootleg (Rene Pierre)", "Crock-Man", MACHINE_SUPPORTS_SAVE )
GAME( 1982, joyman,   puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Joyman", MACHINE_SUPPORTS_SAVE )
GAME( 1982, ctrpllrp, puckman,  pacman,   pacman,   driver_device, 0,        ROT90,  "hack", "Caterpillar Pacman Hack", MACHINE_SUPPORTS_SAVE )
GAME( 1981, piranha,  puckman,  piranha,  mspacman, pacman_state,  eyes,     ROT90,  "GL (US Billiards license)", "Piranha", MACHINE_SUPPORTS_SAVE )
GAME( 1981, piranhao, puckman,  piranha,  mspacman, pacman_state,  eyes,     ROT90,  "GL (US Billiards license)", "Piranha (older)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, abscam,   puckman,  piranha,  mspacman, pacman_state,  eyes,     ROT90,  "GL (US Billiards license)", "Abscam", MACHINE_SUPPORTS_SAVE )
GAME( 1981, piranhah, puckman,  pacman,   mspacman, driver_device, 0,        ROT90,  "hack", "Piranha (hack)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, pacplus,  0,        pacman,   pacman,   pacman_state,  pacplus,  ROT90,  "Namco (Midway license)", "Pac-Man Plus", MACHINE_SUPPORTS_SAVE )

GAME( 1981, mspacman, 0,        mspacman, mspacman, pacman_state,  mspacman, ROT90,  "Midway / General Computer Corporation", "Ms. Pac-Man", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmnf, mspacman, mspacman, mspacman, pacman_state,  mspacman, ROT90,  "hack", "Ms. Pac-Man (speedup hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmat, mspacman, mspacman, mspacman, pacman_state,  mspacman, ROT90,  "hack", "Ms. Pac Attack", MACHINE_SUPPORTS_SAVE )
GAME( 1989, msheartb, mspacman, mspacman, mspacman, pacman_state,  mspacman, ROT90,  "hack (Two-Bit Score)", "Ms. Pac-Man Heart Burn", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmancr,mspacman,mspacman, mspacman, pacman_state,  mspacman, ROT90,  "bootleg", "Ms. Pac-Man (bootleg on Crush Roller Hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmab, mspacman, woodpek,  mspacman, driver_device, 0,        ROT90,  "bootleg", "Ms. Pac-Man (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmbe, mspacman, woodpek,  mspacman, pacman_state,  mspacmbe, ROT90,  "bootleg", "Ms. Pac-Man (bootleg, encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacii,  mspacman, woodpek,  mspacman, pacman_state,  mspacii,  ROT90,  "bootleg (Orca)", "Ms. Pac-Man II (Orca bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacii2, mspacman, woodpek,  mspacman, pacman_state,  mspacii,  ROT90,  "bootleg (Orca)", "Ms. Pac-Man II (Orca bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacgal,   mspacman, woodpek,  mspacman, driver_device, 0,        ROT90,  "hack", "Pac-Gal", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacpls, mspacman, woodpek,  mspacpls, driver_device, 0,        ROT90,  "hack", "Ms. Pac-Man Plus", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mschamp,  mspacman, mschamp,  mschamp,  pacman_state,  mschamp,  ROT90,  "hack", "Ms. Pacman Champion Edition / Zola-Puc Gal", MACHINE_SUPPORTS_SAVE ) /* Rayglo version */
GAME( 1995, mschamps, mspacman, mschamp,  mschamp,  pacman_state,  mschamp,  ROT90,  "hack", "Ms. Pacman Champion Edition / Super Zola-Puc Gal", MACHINE_SUPPORTS_SAVE )

// These bootlegs have MADE IN GREECE clearly visible and etched into the PCBs. They were very common in Spain with several operators having their own versions.
// Based on the PCBs and copyright dates shown they  were produced late 80s / early 90s. Usually they run a version of Ms. Pacman, but were sometimes converted back to regular Pac-Man
GAME( 198?, mspacmanbg, mspacman,woodpek, mspacman, driver_device, 0,        ROT90,  "bootleg",            "Ms. Pac-Man ('Made in Greece' bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mspacmanbgd,mspacman,woodpek, mspacman, driver_device, 0,        ROT90,  "bootleg (Datamat)",  "Miss Pukman ('Made in Greece' Datamat bootleg)", MACHINE_SUPPORTS_SAVE ) // shows 'Miss Pukman 1991/1992' but confirmed to be the bootleg distributed by Datamat
GAME( 1992, mspacmanblt,mspacman,woodpek, mspacman, driver_device, 0,        ROT90,  "bootleg (Triunvi)",  "Come-Cocos (Ms. Pac-Man) ('Made in Greece' Triunvi bootleg)", MACHINE_SUPPORTS_SAVE ) //
GAME( 1991, mspacmanbcc,mspacman,woodpek, mspacman, driver_device, 0,        ROT90,  "bootleg (Tecnausa)", "Come-Cocos (Ms. Pac-Man) ('Made in Greece' Tecnausa bootleg)", MACHINE_SUPPORTS_SAVE ) // ^ same PCB, also dated 1991, distributed by Tecnausa
GAME( 198?, pacmansp,   puckman, pacman,  pacmansp, driver_device, 0,        ROT90,  "bootleg",            "Puck Man (Spanish, 'Made in Greece' bootleg)", MACHINE_SUPPORTS_SAVE ) // probably a further conversion of the mspacmanbg bootleg, still has some MS Pacman code + extra features

GAME( 1989, clubpacm,  0,       woodpek,  mspacman, driver_device, 0,        ROT90,  "Miky SRL", "Pacman Club / Club Lambada (Argentina)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )

GAME( 1985, jumpshot, 0,        pacman,   jumpshot, pacman_state,  jumpshot, ROT90,  "Bally Midway", "Jump Shot", MACHINE_SUPPORTS_SAVE )
GAME( 1985, jumpshotp,jumpshot, pacman,   jumpshotp,pacman_state,  jumpshot, ROT90,  "Bally Midway", "Jump Shot Engineering Sample", MACHINE_SUPPORTS_SAVE )

GAME( 1985, shootbul, 0,        pacman,   shootbul, pacman_state,  jumpshot, ROT90,  "Bally Midway", "Shoot the Bull", MACHINE_SUPPORTS_SAVE )

GAME( 1981, crush,    0,        pacmanp,  maketrax, pacman_state,  maketrax, ROT90,  "Alpha Denshi Co. / Kural Samno Electric, Ltd.", "Crush Roller (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush2,   crush,    pacman,   maketrax, driver_device, 0,        ROT90,  "Alpha Denshi Co. / Kural Esco Electric, Ltd.", "Crush Roller (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush3,   crush,    pacman,   maketrax, pacman_state,  eyes,     ROT90,  "Alpha Denshi Co. / Kural Electric, Ltd.", "Crush Roller (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush4,   crush,    crush4,   crush4,   driver_device, 0,        ROT90,  "Alpha Denshi Co. / Kural TWT", "Crush Roller (set 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, maketrax, crush,    pacmanp,  maketrax, pacman_state,  maketrax, ROT270, "Alpha Denshi Co. / Kural (Williams license)", "Make Trax (US set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, maketrxb, crush,    pacmanp,  maketrax, pacman_state,  maketrax, ROT270, "Alpha Denshi Co. / Kural (Williams license)", "Make Trax (US set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, korosuke, crush,    pacmanp,  korosuke, pacman_state,  korosuke, ROT90,  "Alpha Denshi Co. / Kural Electric, Ltd.", "Korosuke Roller (Japan)", MACHINE_SUPPORTS_SAVE ) // ADK considers it a sequel?
GAME( 1981, crushrlf, crush,    pacman,   maketrax, driver_device, 0,        ROT90,  "bootleg", "Crush Roller (Famaresa PCB)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushbl,  crush,    pacman,   maketrax, driver_device, 0,        ROT90,  "bootleg", "Crush Roller (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushbl2, crush,    pacmanp,  mbrush,   pacman_state,  maketrax, ROT90,  "bootleg", "Crush Roller (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushbl3, crush,    pacmanp,  mbrush,   pacman_state,  maketrax, ROT90,  "bootleg", "Crush Roller (bootleg set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushs,   crush,    crushs,   crushs,   driver_device, 0,        ROT90,  "bootleg (Sidam)", "Crush Roller (bootleg set 4)", MACHINE_SUPPORTS_SAVE ) // Sidam PCB, no Sidam text
GAME( 1981, mbrush,   crush,    pacmanp,  mbrush,   pacman_state,  maketrax, ROT90,  "bootleg", "Magic Brush (bootleg of Crush Roller)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, paintrlr, crush,    pacman,   paintrlr, driver_device, 0,        ROT90,  "bootleg", "Paint Roller (bootleg of Crush Roller)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, eyes,     0,        pacman,   eyes,     pacman_state,  eyes,     ROT90,  "Techstar (Rock-Ola license)", "Eyes (US set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, eyes2,    eyes,     pacman,   eyes,     pacman_state,  eyes,     ROT90,  "Techstar (Rock-Ola license)", "Eyes (US set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, eyesb,    eyes,     pacman,   eyes,     pacman_state,  eyes,     ROT90,  "bootleg", "Eyes (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, eyeszac,  eyes,     pacman,   eyes,     pacman_state,  eyes,     ROT90,  "Techstar (Zaccaria license)", "Eyes (Italy)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // bad dump
GAME( 1982, eyeszacb, eyes,     pacman,   eyes,     driver_device, 0,        ROT90,  "bootleg", "Eyes (bootleg set 2, decrypted)", MACHINE_SUPPORTS_SAVE ) // based on Zaccaria version

GAME( 1983, mrtnt,    0,        pacman,   mrtnt,    pacman_state,  eyes,     ROT90,  "Techstar (Telko license)", "Mr. TNT", MACHINE_SUPPORTS_SAVE )
GAME( 1983, gorkans,  mrtnt,    pacman,   mrtnt,    driver_device, 0,        ROT90,  "Techstar", "Gorkans", MACHINE_SUPPORTS_SAVE )

GAME( 1985, lizwiz,   0,        woodpek,  lizwiz,   driver_device, 0,        ROT90,  "Techstar (Sunn license)", "Lizard Wizard", MACHINE_SUPPORTS_SAVE )

GAME( 1983, eggor,    0,        pacman,   mrtnt,    pacman_state,  eyes,     ROT90,  "Telko", "Eggor", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1983, birdiy,   0,        birdiy,   birdiy,   driver_device, 0,        ROT270, "Mama Top", "Birdiy", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1981, woodpeck, 0,        woodpek,  woodpek,  pacman_state,  woodpek,  ROT90,  "Amenip (Palcom Queen River)", "Woodpecker (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, woodpeca, woodpeck, woodpek,  woodpek,  pacman_state,  woodpek,  ROT90,  "Amenip", "Woodpecker (set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, nmouse,   0,        nmouse,   nmouse,   pacman_state,  eyes,     ROT90,  "Amenip (Palcom Queen River)", "Naughty Mouse (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, nmouseb,  nmouse,   nmouse,   nmouse,   pacman_state,  eyes,     ROT90,  "Amenip Nova Games Ltd.", "Naughty Mouse (set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, ponpoko,  0,        woodpek,  ponpoko,  pacman_state,  ponpoko,  ROT0,   "Sigma Enterprises Inc.", "Ponpoko", MACHINE_SUPPORTS_SAVE )
GAME( 1982, ponpokov, ponpoko,  woodpek,  ponpoko,  pacman_state,  ponpoko,  ROT0,   "Sigma Enterprises Inc. (Venture Line license)", "Ponpoko (Venture Line)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, candory,  ponpoko,  woodpek,  ponpoko,  pacman_state,  ponpoko,  ROT0,   "bootleg", "Candory (Ponpoko bootleg with Mario)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, alibaba,  0,        alibaba,  alibaba,  driver_device, 0,        ROT90,  "Sega", "Ali Baba and 40 Thieves", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1982, alibabab, alibaba,  alibaba,  alibaba,  driver_device, 0,        ROT90,  "bootleg", "Mustafa and 40 Thieves (bootleg)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )

GAME( 1982, dremshpr, 0,        dremshpr, dremshpr, driver_device, 0,        ROT270, "Sanritsu", "Dream Shopper", MACHINE_SUPPORTS_SAVE )

GAME( 1983, vanvan,   0,        vanvan,   vanvan,   driver_device, 0,        ROT270, "Sanritsu", "Van-Van Car", MACHINE_SUPPORTS_SAVE )
GAME( 1983, vanvank,  vanvan,   vanvan,   vanvank,  driver_device, 0,        ROT270, "Sanritsu (Karateco license?)", "Van-Van Car (Karateco set 1)", MACHINE_SUPPORTS_SAVE ) // or bootleg?
GAME( 1983, vanvanb,  vanvan,   vanvan,   vanvank,  driver_device, 0,        ROT270, "Sanritsu (Karateco license?)", "Van-Van Car (Karateco set 2)", MACHINE_SUPPORTS_SAVE ) // "

GAME( 1983, bwcasino, 0,        acitya,   bwcasino, driver_device, 0,        ROT90,  "Epos Corporation", "Boardwalk Casino", MACHINE_SUPPORTS_SAVE )
GAME( 1983, acitya,   bwcasino, acitya,   acitya,   driver_device, 0,        ROT90,  "Epos Corporation", "Atlantic City Action", MACHINE_SUPPORTS_SAVE )

GAME( 1983, theglobp, suprglob, theglobp, theglobp, driver_device, 0,        ROT90,  "Epos Corporation", "The Glob (Pac-Man hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, sprglobp, suprglob, theglobp, theglobp, driver_device, 0,        ROT90,  "Epos Corporation", "Super Glob (Pac-Man hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, sprglbpg, suprglob, pacman,   theglobp, driver_device, 0,        ROT90,  "bootleg", "Super Glob (Pac-Man hardware) (German bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, beastf,   suprglob, theglobp, theglobp, driver_device, 0,        ROT90,  "Epos Corporation", "Beastie Feastie", MACHINE_SUPPORTS_SAVE )

GAME( 1984, drivfrcp, 0,        drivfrcp, drivfrcp, pacman_state,  drivfrcp, ROT90,  "Shinkai Inc. (Magic Electronics Inc. license)", "Driving Force (Pac-Man conversion)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, 8bpm,     8ballact, 8bpm,     8bpm,     pacman_state,  8bpm,     ROT90,  "Seatongrove Ltd (Magic Electronics USA license)", "Eight Ball Action (Pac-Man conversion)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, porky,    0,        porky,    porky,    pacman_state,  porky,    ROT90,  "Shinkai Inc. (Magic Electronics Inc. license)", "Porky", MACHINE_SUPPORTS_SAVE )

GAME( 1986, rocktrv2, 0,        rocktrv2, rocktrv2, pacman_state,  rocktrv2, ROT90,  "Triumph Software Inc.", "MTV Rock-N-Roll Trivia (Part 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1986, bigbucks, 0,        bigbucks, bigbucks, driver_device, 0,        ROT90,  "Dynasoft Inc.", "Big Bucks", MACHINE_SUPPORTS_SAVE )

GAME( 1983, numcrash, 0,        woodpek,  numcrash, driver_device, 0,        ROT90,  "Hanshin Goraku", "Number Crash", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

GAME( 198?, cannonbp, 0,        pacman,   cannonbp, pacman_state,  cannonbp, ROT90,  "Novomatic", "Cannon Ball (Pac-Man Hardware)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )

GAME( 1999, superabc, 0,        superabc, superabc, pacman_state,  superabc, ROT90,  "hack (Two-Bit Score)", "Super ABC (Pac-Man multigame kit, Sep. 03 1999)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, superabco,superabc, superabc, superabc, pacman_state,  superabc, ROT90,  "hack (Two-Bit Score)", "Super ABC (Pac-Man multigame kit, Mar. 08 1999)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, pengojpm, pengo,    pengojpm, pengojpm, driver_device, 0,        ROT90,  "bootleg", "Pengo (bootleg on Pac-Man hardware, set 1)", MACHINE_SUPPORTS_SAVE ) // conversion of pacmanjpm board with wire mods
GAME( 1981, pengopac, pengo,    pengojpm, pengojpm, driver_device, 0,        ROT90,  "bootleg", "Pengo (bootleg on Pac-Man hardware, set 2)", MACHINE_SUPPORTS_SAVE ) // different conversion?
