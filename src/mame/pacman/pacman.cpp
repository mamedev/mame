// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Stephane Humbert
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
        * Eeekk!

    TODO:
        * Mystery items in Ali Baba don't work correctly because of protection.
        * mspactwin shows a green "0" in the corner on the PCB at tilescreen, but "18" on MAME.
        * mspactwin_map supposed ROM 0x2000 mirroring implementation doesn't make much sense, there's a bus conflict now

    Known to exist but dumps needed
        * MTV Rock-N-Roll Trivia (Part 2), 1 bad rom. It's not a bad dump, the rom is bad.

****************************************************************************

    Pac-Man memory map (preliminary)

    0000-3fff ROM
    4000-43ff Video RAM
    4400-47ff Color RAM
    4800-4bff RAM Dream Shopper, Van Van Car only.  Pacman uses this block due to a bug in the routine to
         translate the internal pacman location to a screen address when in the right tunnel.
    4c00-4fff RAM
    8000-bfff ROM Ms Pac-Man, Ponpoko, Lizard Wizard, Dream Shopper, Van Van Car, Woodpecker, Ali Baba all use
         portions of the upper memory area.  Pacman and most bootlegs don't have an A15 line to the cpu so most
         boards that use upper memory have an auxiliary board that plugs into the cpu socket with a ribbon cable.
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
              an original Midway Pacman board contains only the center 6 sprites.
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
  mspacatk.5 and .6 contain the decoded maze data for ms pac plus.  The maze data was obtained from the mspackpls
  romset.

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

- Pac and Paint: Even though this seems to be a unique game the marquee shows a paintbrush.  The board has standard
  crush roller roms.  It is slightly different from a Make Trax board in that the sync inverter chip is missing and
  there are no jumper wires.

- jumpshotp: This board was acquired by Pokemame from a former Midway employee.  The hardware is identical to normal boards
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

- Super ABC shows developer credits at boot if IN1 is disconnected.

Boards:
-------
- puckman is the same as pacman except they are slotted to break a part and have ribbon cables to connect the halves.

- All ms pacman boards are pacman boards with an auxiliary cpu board installed on a ribbon cable and 5e,5f changed.

- Pacman Plus, Shoot the Bull, Jumpshot are epoxy potted auxiliary cpu boards.  The graphics and
  color proms are changed as well.  They are labeled Authorized Enhancement Kit Bally/Midway.   Jumpshot=B3172

- The Eyes boards are similar to pacman boards except for the data swapping encryption.  The custom chips are integrated
  into the board and there is no voltage regulator section.  There is an extra rom at row 6 so row 7 is equivalent to
  pacman row 6.  Program roms are 4k but graphics roms have unpopulated areas for optional 2k roms.

- Piranha and Naughty Mouse use a board known as the GDP-01 bootleg.  It is similar to an eyes board with an extra row
  of eproms in row 6 to enable 2k program roms. The GDP-01 does not require a SBC, but 5 SBC chips can be left unpopulated
  and a SBC card(GDP-02) can be installed.

- Some of the pacman hacks may have a cut trace under color PROM 4A, causing green dots and blue inner walls.
  Seen on a bucanera PCB, and a baracuda PCB(though old magazine photos show normal colors). To show them in MAME,
  do attr|1 in pacman_get_tile_info.

- Make Trax/Crush Roller boards are similar to pacman boards, the chip positions are even mostly the same.  There is no
  voltage regulator section.  Make Trax has sync inverter at 1S, jumper wires run from the edge connector to the
  inverter and back.  Make Trax/CR boards are easily identified because the edge connector is on the long side,
  offset to one side.  Make trax and Eyes pinouts are similar enough to test each other but are not playable.

- Atlantic City Action, Boardwalk Casino, The Glob, Super Glob, Beastie Feastie, EEEK! all use identical Epos boards
  with different pals.  Street heat, Drakton on Donkey Kong also use this board.  It is a cpu auxiliary board.  It is
  not potted but all the chip labels are removed.

- Porky and 8ball action use the same plug in board for pacman.  It is much larger than most and requires some soldering
  to install.  It includes a new cpu and sound hardware.  Driving force uses a similar board and requires a lot of
  soldering.

- Lizard Wizard is a potted auxiliary cpu board.

- Big Bucks is an auxiliary cpu board.

- MTV Trivia is an auxiliary cpu board.

- Truco Clemente runs on a pac bootleg with a handwired cpu/graphics auxiliary board.  This supports banking of the
  roms and completely bypasses color proms and video output section of pacman.

- Vanvanb was found on a low quality board assumed to be a bootleg of an original.  The 2 sound chips were on a
  daughterboard that plugged into a 40 pin socket. Physically it is very different from pacman, although the fact that
  it uses the same falcon pinout as every other pac boot is a little suspicious.

- Ponpoko uses its own board.

****************************************************************************/

#include "emu.h"
#include "pacman.h"
#include "jumpshot.h"
#include "pacplus.h"

#include "cpu/s2650/s2650.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/sn76496.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define MASTER_CLOCK        (XTAL(18'432'000))

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
	uint8_t *rom = memregion("maincpu")->base() + 0x10000;
	int whichbank = ioport("GAME")->read() & 1;

	membank("bank1")->configure_entries(0, 2, &rom[0x0000], 0x8000);
	membank("bank2")->configure_entries(0, 2, &rom[0x4000], 0x8000);

	membank("bank1")->set_entry(whichbank);
	membank("bank2")->set_entry(whichbank);
}

MACHINE_RESET_MEMBER(pacman_state,superabc)
{
	superabc_bank_w(0);
}

MACHINE_RESET_MEMBER(pacman_state,maketrax)
{
	m_maketrax_counter = 0;
	m_maketrax_offset = 0;
	m_maketrax_disable_protection = 0;
}

/*************************************
 *
 *  Interrupts
 *
 *************************************/

void pacman_state::vblank_irq(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

INTERRUPT_GEN_MEMBER(pacman_state::periodic_irq)
{
	if (m_irq_mask)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

void pacman_state::vblank_nmi(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void pacman_state::irq_mask_w(int state)
{
	m_irq_mask = state;
	if (!state)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

void pacman_state::nmi_mask_w(int state)
{
	m_irq_mask = state;
	if (!state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void pacman_state::pacman_interrupt_vector_w(uint8_t data)
{
	m_interrupt_vector = data;
}

IRQ_CALLBACK_MEMBER(pacman_state::interrupt_vector_r)
{
	return m_interrupt_vector;
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

void pacman_state::piranha_interrupt_vector_w(uint8_t data)
{
	if (data == 0xfa) data = 0x78;
	if (data == 0xfc) data = 0xfc;
	m_interrupt_vector = data;
}


void pacman_state::nmouse_interrupt_vector_w(uint8_t data)
{
	if (data == 0xbf) data = 0x3c;
	if (data == 0xc6) data = 0x40;
	if (data == 0xfc) data = 0xfc;
	m_interrupt_vector = data;
}


void pacman_state::mspacii_interrupt_vector_w(uint8_t data)
{
	if (data == 0xfb) data = 0xfe;
	if (data == 0xfc) data = 0xfc;
	m_interrupt_vector = data;
}



/*************************************
 *
 *  LEDs/coin counters
 *
 *************************************/

void pacman_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}


void pacman_state::coin_lockout_global_w(int state)
{
	machine().bookkeeping().coin_lockout_global_w(!state);
}



/*************************************
 *
 *  Ali Baba sound
 *
 *************************************/

void pacman_state::alibaba_sound_w(offs_t offset, uint8_t data)
{
	/* since the sound region in Ali Baba is not contiguous, translate the
	   offset into the 0-0x1f range */
	offset = (offset >> 1 & 0x10) | (offset & 0x0f);
	m_namco_sound->pacman_sound_w(offset, data);
}

uint8_t pacman_state::alibaba_mystery_1_r()
{
	/* The return value determines what the mystery item is.  Each bit corresponds
	   to a question mark */
	return machine().rand() & 0x0f;
//  return m_maincpu->state_int(Z80_R) & 0x0f;
}


uint8_t pacman_state::alibaba_mystery_2_r()
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

void pacman_state::maketrax_protection_w(uint8_t data)
{
	if (data == 0) // disable protection / reset?
	{
		m_maketrax_counter = 0;
		m_maketrax_offset = 0;
		m_maketrax_disable_protection = 1;
		return;
	}

	if (data == 1)
	{
		m_maketrax_disable_protection = 0;

		m_maketrax_counter++;
		if (m_maketrax_counter == 0x3c)
		{
			m_maketrax_counter = 0;
			m_maketrax_offset++;

			if (m_maketrax_offset == 0x1e)
				m_maketrax_offset = 0;
		}
	}
}

uint8_t pacman_state::maketrax_special_port2_r(offs_t offset)
{
	const uint8_t protdata[0x1e] = { // table at $ebd (odd entries)
		0x00, 0xc0, 0x00, 0x40, 0xc0, 0x40, 0x00, 0xc0, 0x00, 0x40, 0x00, 0xc0, 0x00, 0x40, 0xc0, 0x40,
		0x00, 0xc0, 0x00, 0x40, 0x00, 0xc0, 0x00, 0x40, 0xc0, 0x40, 0x00, 0xc0, 0x00, 0x40
	};

	uint8_t data = ioport("DSW1")->read() & 0x3f;

	if (m_maketrax_disable_protection == 0)
		return protdata[m_maketrax_offset] | data;

	switch (offset)
	{
		case 0x01:
		case 0x04:
			data |= 0x40; break;
		case 0x05:
		case 0x0e: // korosuke
		case 0x10: // korosuke
			data |= 0xc0; break;
		default:
			data &= 0x3f; break;
	}

	return data;
}

uint8_t pacman_state::maketrax_special_port3_r(offs_t offset)
{
	const uint8_t protdata[0x1e] = { // table at $ebd (even entries)
		0x1f, 0x3f, 0x2f, 0x2f, 0x0f, 0x0f, 0x0f, 0x3f, 0x0f, 0x0f, 0x1c, 0x3c, 0x2c, 0x2c, 0x0c, 0x0c,
		0x0c, 0x3c, 0x0c, 0x0c, 0x11, 0x31, 0x21, 0x21, 0x01, 0x01, 0x01, 0x31, 0x01, 0x01
	};

	if (m_maketrax_disable_protection == 0)
		return protdata[m_maketrax_offset];

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

uint8_t pacman_state::mbrush_prot_r(offs_t offset)
{
	uint8_t data = ioport("DSW1")->read() & 0x3f;

	switch (offset)
	{
		case 0x00:
		case 0x04:
		case 0x07:
		case 0x0e:
		case 0x0f:
			return 0xc0 | data;
		case 0x40:
		case 0x41:
		case 0x47:
		case 0x49:
		case 0x4d:
		case 0x5d:
			return 0x00;
		case 0x42:
		case 0x43:
		case 0x46:
		case 0x4c:
		case 0x50:
			return 0x02;
		case 0x7f:
			return 0x10;
	}

	return data;
}


/*************************************
 *
 *  Zola kludge
 *
 *************************************/

uint8_t pacman_state::mschamp_kludge_r()
{
	return m_counter++;
}


/************************************
 *
 *  Big Bucks questions roms handlers
 *
 ************************************/

void pacman_state::bigbucks_bank_w(uint8_t data)
{
	m_bigbucks_bank = data;
}

uint8_t pacman_state::bigbucks_question_r(offs_t offset)
{
	uint8_t *question = memregion("user1")->base();
	uint8_t ret;

	ret = question[(m_bigbucks_bank << 16) | (offset ^ 0xffff)];

	return ret;
}


/************************************
 *
 *  S2650 cpu based games
 *
 ************************************/

void pacman_state::s2650_interrupt(int state)
{
	if (state)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}

void pacman_state::porky_banking_w(uint8_t data)
{
	membank("bank1")->set_entry(data & 1);
	membank("bank2")->set_entry(data & 1);
	membank("bank3")->set_entry(data & 1);
	membank("bank4")->set_entry(data & 1);
}

uint8_t pacman_state::drivfrcp_port1_r()
{
	switch (m_maincpu->pc())
	{
		case 0x0030:
		case 0x0291:
			return 0x01;
	}

	return 0;
}

uint8_t pacman_state::_8bpm_port1_r()
{
	switch (m_maincpu->pc())
	{
		case 0x0030:
		case 0x0466:
			return 0x01;
	}

	return 0;
}

uint8_t pacman_state::porky_port1_r()
{
	switch (m_maincpu->pc())
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

uint8_t pacman_state::rocktrv2_prot1_data_r()
{
	return m_rocktrv2_prot_data[0] >> 4;
}

uint8_t pacman_state::rocktrv2_prot2_data_r()
{
	return m_rocktrv2_prot_data[1] >> 4;
}

uint8_t pacman_state::rocktrv2_prot3_data_r()
{
	return m_rocktrv2_prot_data[2] >> 4;
}

uint8_t pacman_state::rocktrv2_prot4_data_r()
{
	return m_rocktrv2_prot_data[3] >> 4;
}

void pacman_state::rocktrv2_prot_data_w(offs_t offset, uint8_t data)
{
	m_rocktrv2_prot_data[offset] = data;
}

void pacman_state::rocktrv2_question_bank_w(uint8_t data)
{
	m_rocktrv2_question_bank = data;
}

uint8_t pacman_state::rocktrv2_question_r(offs_t offset)
{
	uint8_t *question = memregion("user1")->base();

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

void pacman_state::superabc_bank_w(uint8_t data)
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
uint8_t pacman_state::mspacman_disable_decode_r_0x0038(offs_t offset){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x0038]; }
uint8_t pacman_state::mspacman_disable_decode_r_0x03b0(offs_t offset){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x03b0]; }
uint8_t pacman_state::mspacman_disable_decode_r_0x1600(offs_t offset){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x1600]; }
uint8_t pacman_state::mspacman_disable_decode_r_0x2120(offs_t offset){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x2120]; }
uint8_t pacman_state::mspacman_disable_decode_r_0x3ff0(offs_t offset){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x3ff0]; }
uint8_t pacman_state::mspacman_disable_decode_r_0x8000(offs_t offset){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x8000]; }
uint8_t pacman_state::mspacman_disable_decode_r_0x97f0(offs_t offset){ mspacman_disable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x97f0]; }
void pacman_state::mspacman_disable_decode_w(uint8_t data){ mspacman_disable_decode_latch(machine()); }

// any access to these ROM addresses enables the decoder, and you'll see the Ms. Pac-Man code
uint8_t pacman_state::mspacman_enable_decode_r_0x3ff8(offs_t offset){ mspacman_enable_decode_latch(machine()); return memregion("maincpu")->base()[offset+0x3ff8+0x10000]; }
void pacman_state::mspacman_enable_decode_w(uint8_t data){ mspacman_enable_decode_latch(machine()); }


uint8_t pacman_state::pacman_read_nop()
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

void pacman_state::pacman_map(address_map &map)
{
	//A lot of games don't have an a15 at the cpu.  Generally only games with a cpu daughter board can access the full 32k of romspace.
	map(0x0000, 0x3fff).mirror(0x8000).rom();
	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4bff).mirror(0xa000).r(FUNC(pacman_state::pacman_read_nop)).nopw();
	map(0x4c00, 0x4fef).mirror(0xa000).ram();
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0xaf38).w(m_mainlatch, FUNC(addressable_latch_device::write_d0));
	map(0x5040, 0x505f).mirror(0xaf00).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).nopw();
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf3f).portr("DSW2");
}

void pacman_state::cannonbp_map(address_map &map)
{
	pacman_map(map);
	map(0x4800, 0x4bff).ram();
	map(0x3000, 0x3fff).r(FUNC(pacman_state::cannonbp_protection_r));
}

void pacman_state::birdiy_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x8000).rom();
	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
//  map(0x4800, 0x4bff).mirror(0xa000).r(FUNC(pacman_state::pacman_read_nop)).nopw();
	map(0x4c00, 0x4fef).mirror(0xa000).ram();
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0xaf38).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5080, 0x509f).mirror(0xaf00).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x50a0, 0x50af).mirror(0xaf00).writeonly().share("spriteram2");
//  map(0x5070, 0x507f).mirror(0xaf00).nopw();
//  map(0x5080, 0x5080).mirror(0xaf3f).nopw();
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf3f).portr("DSW2");
}


void pacman_state::mspacman_map(address_map &map)
{
	/* start with 0000-3fff and 8000-bfff mapped to the ROMs */
	map(0x0000, 0xffff).bankr("bank1");
	map(0x4000, 0x7fff).mirror(0x8000).unmaprw();

	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4bff).mirror(0xa000).r(FUNC(pacman_state::pacman_read_nop)).nopw();
	map(0x4c00, 0x4fef).mirror(0xa000).ram();
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0xaf38).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5040, 0x505f).mirror(0xaf00).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).nopw();
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf3f).portr("DSW2");

	/* overlay decode enable/disable on top */
	map(0x0038, 0x003f).rw(FUNC(pacman_state::mspacman_disable_decode_r_0x0038), FUNC(pacman_state::mspacman_disable_decode_w));
	map(0x03b0, 0x03b7).rw(FUNC(pacman_state::mspacman_disable_decode_r_0x03b0), FUNC(pacman_state::mspacman_disable_decode_w));
	map(0x1600, 0x1607).rw(FUNC(pacman_state::mspacman_disable_decode_r_0x1600), FUNC(pacman_state::mspacman_disable_decode_w));
	map(0x2120, 0x2127).rw(FUNC(pacman_state::mspacman_disable_decode_r_0x2120), FUNC(pacman_state::mspacman_disable_decode_w));
	map(0x3ff0, 0x3ff7).rw(FUNC(pacman_state::mspacman_disable_decode_r_0x3ff0), FUNC(pacman_state::mspacman_disable_decode_w));
	map(0x3ff8, 0x3fff).rw(FUNC(pacman_state::mspacman_enable_decode_r_0x3ff8), FUNC(pacman_state::mspacman_enable_decode_w));
	map(0x8000, 0x8007).rw(FUNC(pacman_state::mspacman_disable_decode_r_0x8000), FUNC(pacman_state::mspacman_disable_decode_w));
	map(0x97f0, 0x97f7).rw(FUNC(pacman_state::mspacman_disable_decode_r_0x97f0), FUNC(pacman_state::mspacman_disable_decode_w));
}


void pacman_state::woodpek_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4bff).mirror(0xa000).r(FUNC(pacman_state::pacman_read_nop)).nopw();
	map(0x4c00, 0x4fef).mirror(0xa000).ram();
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0xaf38).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5040, 0x505f).mirror(0xaf00).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).nopw();
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf3f).portr("DSW2");
	map(0x8000, 0xbfff).rom();
}


void clubpacm_state::clubpacm_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(clubpacm_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(clubpacm_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4bff).mirror(0xa000).r(FUNC(clubpacm_state::pacman_read_nop)).nopw();
	map(0x4c00, 0x4fef).mirror(0xa000).ram();
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0xaf38).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5040, 0x505f).mirror(0xaf00).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).w(m_sublatch, FUNC(generic_latch_8_device::write));
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf3f).r(m_sublatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xbfff).rom();
}

void mspactwin_state::mspactwin_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).mirror(0x4000).rom();
	map(0x4000, 0x43ff).mirror(0x8000).ram().w(FUNC(mspactwin_state::mspactwin_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0x8000).ram().w(FUNC(mspactwin_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4bff).mirror(0xa000).r(FUNC(mspactwin_state::pacman_read_nop)).nopw();
	map(0x4c00, 0x4fef).mirror(0xa000).ram();
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0x0030).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5040, 0x505f).mirror(0xaf00).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).w(m_sublatch, FUNC(generic_latch_8_device::write));
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf3f).r(m_sublatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0xbfff).rom();
}

void mspactwin_state::mspactwin_decrypted_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x4000).rom().share("decrypted_opcodes");
	map(0x4000, 0x5fff).unmapr(); // mirror only 6000-7fff
	map(0x8000, 0xbfff).rom().share("decrypted_opcodes_high");
}


void pacman_state::numcrash_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x27ff).rom();
	/* 0x2800 - 0x2fff unmapped? */
	map(0x3000, 0x37ff).rom();
	/* 0x3800 - 0x3fff unmapped? */
	map(0x8000, 0x9fff).rom();

	map(0x4000, 0x43ff).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");

	map(0x4c00, 0x4fef).ram();
	map(0x4ff0, 0x4fff).ram().share("spriteram");
	map(0x5000, 0x5007).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5040, 0x505f).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).writeonly().share("spriteram2");
//  map(0x5070, 0x507f).nopw();
//  map(0x5080, 0x5080).nopw();
	map(0x50c0, 0x50c0).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).portr("IN0");
	map(0x5040, 0x5040).portr("IN1");
	map(0x5080, 0x5080).portr("DSW1");
//  map(0x50c0, 0x50c0).portr("DSW2"); // only one DSW on the PCB
}


void pacman_state::alibaba_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4bff).mirror(0xa000).r(FUNC(pacman_state::pacman_read_nop)).nopw();
	map(0x4c00, 0x4eef).mirror(0xa000).ram();
	map(0x4ef0, 0x4eff).mirror(0xa000).ram().share("spriteram");
	map(0x4f00, 0x4fff).mirror(0xa000).ram();
	map(0x5000, 0x5007).mirror(0xaf38).w("latch1", FUNC(ls259_device::write_d0));
	map(0x5000, 0x5000).mirror(0xaf38).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5040, 0x506f).mirror(0xaf00).w(FUNC(pacman_state::alibaba_sound_w)); /* the sound region is not contiguous */
	map(0x5050, 0x505f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).nopw();
	map(0x50c0, 0x50c7).mirror(0xaf00).w("latch2", FUNC(ls259_device::write_d0));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf00).r(FUNC(pacman_state::alibaba_mystery_1_r));
	map(0x50c1, 0x50c1).mirror(0xaf00).r(FUNC(pacman_state::alibaba_mystery_2_r));
	map(0x50c2, 0x50ff).mirror(0xaf00).r(FUNC(pacman_state::pacman_read_nop));
	map(0x8000, 0x8fff).rom();
	map(0x9000, 0x93ff).mirror(0x0c00).ram();
	map(0xa000, 0xa7ff).mirror(0x1800).rom();
}


void pacman_state::dremshpr_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4fef).mirror(0xa000).ram();
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0xaf38).w(m_mainlatch, FUNC(ls259_device::write_d0));
//  map(0x5040, 0x505f).mirror(0xaf00).w("namco", FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).nopw();
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf3f).portr("DSW2");
	map(0x8000, 0xbfff).rom();

	/* vanvan: probably a leftover from development: the Sanritsu version writes
	   the color lookup table here, while the Karateco version writes garbage. */
	map(0xb800, 0xb87f).nopw();
}


void epospm_state::epos_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x8000).bankr("bank1");
	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(epospm_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(epospm_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4bff).mirror(0xa000).r(FUNC(epospm_state::pacman_read_nop)).nopw();
	map(0x4c00, 0x4fef).mirror(0xa000).ram();
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0xaf38).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5040, 0x505f).mirror(0xaf00).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).nopw();
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf3f).portr("DSW2");
}


void pacman_state::s2650games_map(address_map &map)
{
	map(0x0000, 0x0fff).bankr("bank1");
	map(0x1000, 0x13ff).mirror(0x6000).w(FUNC(pacman_state::s2650games_colorram_w)).share("colorram");
	map(0x1400, 0x141f).mirror(0x6000).w(FUNC(pacman_state::s2650games_scroll_w));
	map(0x1420, 0x148f).mirror(0x6000).writeonly().share("unk_1420");
	map(0x1490, 0x149f).mirror(0x6000).writeonly().share("s2650_spriteram");
	map(0x14a0, 0x14bf).mirror(0x6000).w(FUNC(pacman_state::s2650games_tilesbank_w)).share("s2650_tileram");
	map(0x14c0, 0x14ff).mirror(0x6000).writeonly().share("unk_14c0");
	map(0x1500, 0x1507).mirror(0x6000).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x1508, 0x155f).mirror(0x6000).writeonly().share("unk_1508");
	map(0x1560, 0x156f).mirror(0x6000).writeonly().share("spriteram2");
	map(0x1570, 0x157f).mirror(0x6000).writeonly().share("unk_1570");
	map(0x1586, 0x1587).mirror(0x6000).nopw();
	map(0x15c0, 0x15c0).mirror(0x6000).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x15c7, 0x15c7).mirror(0x6000).w(FUNC(pacman_state::porky_banking_w));
	map(0x1500, 0x1500).mirror(0x6000).portr("IN0");
	map(0x1540, 0x1540).mirror(0x6000).portr("IN1");
	map(0x1580, 0x1580).mirror(0x6000).portr("DSW0");
	map(0x1800, 0x1bff).mirror(0x6000).w(FUNC(pacman_state::s2650games_videoram_w)).share("videoram");
	map(0x1c00, 0x1fef).mirror(0x6000).ram();
	map(0x1ff0, 0x1fff).mirror(0x6000).writeonly().share("spriteram");
	map(0x2000, 0x2fff).bankr("bank2");
	map(0x4000, 0x4fff).bankr("bank3");
	map(0x6000, 0x6fff).bankr("bank4");
}


void pacman_state::rocktrv2_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
	map(0x4c00, 0x4fff).ram();
	map(0x5000, 0x5007).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5040, 0x505f).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x50c0, 0x50c0).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5fe0, 0x5fe3).w(FUNC(pacman_state::rocktrv2_prot_data_w)).share("rocktrv2_prot");
	map(0x5ff0, 0x5ff0).w(FUNC(pacman_state::rocktrv2_question_bank_w));
	map(0x5000, 0x5000).portr("IN0");
	map(0x5040, 0x507f).portr("IN1");
	map(0x5080, 0x5080).portr("DSW1");
	map(0x50c0, 0x50c0).portr("DSW2");
	map(0x5fe0, 0x5fe0).r(FUNC(pacman_state::rocktrv2_prot1_data_r));
	map(0x5fe4, 0x5fe4).r(FUNC(pacman_state::rocktrv2_prot2_data_r));
	map(0x5fe8, 0x5fe8).r(FUNC(pacman_state::rocktrv2_prot3_data_r));
	map(0x5fec, 0x5fec).r(FUNC(pacman_state::rocktrv2_prot4_data_r));
	map(0x5fff, 0x5fff).portr("DSW2");       /* DSW2 mirrored */
	map(0x6000, 0x7fff).rom();
	map(0x8000, 0xffff).r(FUNC(pacman_state::rocktrv2_question_r));
}


void pacman_state::bigbucks_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
	map(0x4c00, 0x4fff).ram();
	map(0x5000, 0x5007).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5040, 0x505f).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x50c0, 0x50c0).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x503f).portr("IN0");
	map(0x5040, 0x507f).portr("IN1");
	map(0x5080, 0x50bf).portr("DSW1");
	map(0x50c0, 0x50ff).portr("DSW2");
	map(0x5100, 0x5100).nopw(); /*?*/
	map(0x6000, 0x6000).w(FUNC(pacman_state::bigbucks_bank_w));
	map(0x8000, 0x9fff).rom();
}


void pacman_state::mschamp_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bank1");
	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4bff).mirror(0xa000).r(FUNC(pacman_state::pacman_read_nop)).nopw();
	map(0x4c00, 0x4fef).mirror(0xa000).ram();
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0xaf38).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5040, 0x505f).mirror(0xaf00).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).nopw();
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf3f).portr("DSW2");
	map(0x8000, 0xbfff).bankr("bank2");
}


void pacman_state::superabc_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr("bank1");
	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4fef).mirror(0xa000).ram().share("28c16.u17"); // nvram
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0xaf38).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5002, 0x5002).mirror(0xaf3c).w(FUNC(pacman_state::superabc_bank_w));
	map(0x5040, 0x505f).mirror(0xaf00).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x5060, 0x506f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).nopw();
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5040, 0x5040).mirror(0xaf3f).portr("IN1");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("DSW1");
	map(0x50c0, 0x50c0).mirror(0xaf3f).portr("DSW2");
	map(0x8000, 0x9fff).bankr("bank2");
	map(0xa000, 0xbfff).bankr("bank3");
}


void pacman_state::crushs_map(address_map &map)
{
	map(0x0000, 0x3fff).mirror(0x8000).rom();
	map(0x4000, 0x43ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).mirror(0xa000).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");
	map(0x4800, 0x4bff).mirror(0xa000).r(FUNC(pacman_state::pacman_read_nop)).nopw();
	map(0x4c00, 0x4fef).mirror(0xa000).ram();
	map(0x4ff0, 0x4fff).mirror(0xa000).ram().share("spriteram");
	map(0x5000, 0x5007).mirror(0xaf38).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x5040, 0x505f).mirror(0xaf00).nopw(); // doesn't use pacman sound hw
	map(0x5060, 0x506f).mirror(0xaf00).writeonly().share("spriteram2");
	map(0x5070, 0x507f).mirror(0xaf00).nopw();
	map(0x5080, 0x5080).mirror(0xaf3f).nopw();
	map(0x50c0, 0x50c0).mirror(0xaf3f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x5000, 0x5000).mirror(0xaf3f).portr("IN0");
	map(0x5080, 0x5080).mirror(0xaf3f).portr("IN1");
}



void pacman_state::pengojpm_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
//  map(0x1000, 0x1fff) // header check for 0x55aa at POST, diagnostic ROM?
	map(0x4000, 0x7fff).rom();

	map(0x8000, 0x83ff).ram().w(FUNC(pacman_state::pacman_videoram_w)).share("videoram");
	map(0x8400, 0x87ff).ram().w(FUNC(pacman_state::pacman_colorram_w)).share("colorram");

	map(0x8800, 0x8bff).ram();
//  map(0x8800, 0x8bff).r(FUNC(pacman_state::pacman_read_nop)).nopw();
	map(0x8c00, 0x8fef).ram();
	map(0x8ff0, 0x8fff).ram().share("spriteram");
	map(0x9000, 0x9007).w(m_mainlatch, FUNC(ls259_device::write_d0));
	map(0x9040, 0x905f).w(m_namco_sound, FUNC(namco_device::pacman_sound_w));
	map(0x9060, 0x906f).writeonly().share("spriteram2");
	map(0x9070, 0x907f).nopw();
	map(0x9080, 0x9080).nopw();
	map(0x90c0, 0x90c0).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x9000, 0x9000).portr("IN0");
	map(0x9040, 0x9040).portr("IN1");
	map(0x9080, 0x9080).portr("DSW1");
	map(0x90c0, 0x90c0).portr("DSW2");

	map(0xf000, 0xffff).ram();
}


/*************************************
 *
 *  Main CPU port handlers
 *
 *************************************/

void pacman_state::writeport(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(pacman_state::pacman_interrupt_vector_w));    /* Pac-Man only */
}

void pacman_state::vanvan_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).w("sn1", FUNC(sn76496_device::write));
	map(0x02, 0x02).w("sn2", FUNC(sn76496_device::write));
}

void pacman_state::dremshpr_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x06, 0x07).w("ay8910", FUNC(ay8910_device::data_address_w));
}

void pacman_state::piranha_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(pacman_state::piranha_interrupt_vector_w));
}

void pacman_state::nmouse_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(pacman_state::nmouse_interrupt_vector_w));
}

void epospm_state::epos_portmap(address_map &map)
{
	writeport(map);
	map(0x00, 0xff).r(FUNC(epospm_state::epos_decryption_w));   /* Switch protection logic */
}

void pacman_state::mspacii_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(0xff).w(FUNC(pacman_state::mspacii_interrupt_vector_w));
}

void pacman_state::mschamp_portmap(address_map &map)
{
	writeport(map);
	map(0x00, 0x00).r(FUNC(pacman_state::mschamp_kludge_r));
}

void pacman_state::bigbucks_portmap(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(pacman_state::bigbucks_question_r));
}

void pacman_state::s2650games_dataport(address_map &map)
{
	map(S2650_DATA_PORT, S2650_DATA_PORT).w("sn1", FUNC(sn76496_device::write));
}

void pacman_state::drivfrcp_portmap(address_map &map)
{
	map(0x00, 0x00).nopr();
	map(0x01, 0x01).r(FUNC(pacman_state::drivfrcp_port1_r));
}

void pacman_state::_8bpm_portmap(address_map &map)
{
	map(0x00, 0x00).nopr();
	map(0x01, 0x01).r(FUNC(pacman_state::_8bpm_port1_r));
	map(0xe0, 0xe0).nopr();
}

void pacman_state::porky_portmap(address_map &map)
{
	map(0x01, 0x01).r(FUNC(pacman_state::porky_port1_r));
}

void pacman_state::crushs_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay8912", FUNC(ay8912_device::data_address_w));
	map(0x01, 0x01).r("ay8912", FUNC(ay8912_device::data_r));
	map(0x02, 0x02).portr("DSW1");
}



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
	PORT_DIPNAME(0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1) PORT_DIPLOCATION("SW:7") PORT_TOGGLE
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
	PORT_CONFNAME(0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_CONFSETTING(   0x80, DEF_STR( Upright ) )
	PORT_CONFSETTING(   0x00, DEF_STR( Cocktail ) )

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
	PORT_CONFNAME(0x40, 0x40, DEF_STR( Difficulty ) )   // physical location for difficulty on puckman set is split-pad between R32 and C29
	PORT_CONFSETTING(   0x40, DEF_STR( Normal ) )
	PORT_CONFSETTING(   0x00, DEF_STR( Hard ) )
	PORT_CONFNAME(0x80, 0x80, "Ghost Names" )           // physical location for ghostnames on puckman set is split-pad between C10 and C29
	PORT_CONFSETTING(   0x80, DEF_STR( Normal ) )
	PORT_CONFSETTING(   0x00, DEF_STR( Alternate ) )

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
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1) PORT_DIPLOCATION("SW:7") PORT_TOGGLE
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
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )  // Also invincibility in mspacpls
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )  // Also speed-up in mspacpls
	PORT_CONFNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_CONFSETTING(   0x80, DEF_STR( Upright ) )
	PORT_CONFSETTING(   0x00, DEF_STR( Cocktail ) )

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
	PORT_CONFNAME( 0x40, 0x40, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(    0x40, DEF_STR( Normal ) )
	PORT_CONFSETTING(    0x00, DEF_STR( Hard ) )
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

static INPUT_PORTS_START( pacmanpe )
	PORT_INCLUDE( pacman )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) // the coin slots seem reverse based on service text
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, "2C/1C, 1C/1C" )
	PORT_DIPSETTING(    0x01, "1C/2C, 1C/3C" )
	PORT_DIPSETTING(    0x02, "1C/1C, 1C/3C" )
	PORT_DIPSETTING(    0x00, "1C/1C, 1C/2C" )

	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x04, "3" )

	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pacuman )
	PORT_INCLUDE( pacman )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, "1C/2C, 1C/5C" )
	PORT_DIPSETTING(    0x01, "1C/2C, 1C/4C" )
	PORT_DIPSETTING(    0x02, "1C/2C, 1C/4C" )
	PORT_DIPSETTING(    0x00, "1C/1C, 1C/2C" )
INPUT_PORTS_END


static INPUT_PORTS_START( mschamp )
	PORT_INCLUDE( mspacman )

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

/* Pacman Club inputs are similar to Ms. Pac-Man, except:
    - P1/P2 joystick inputs are multiplexed via $5004/5005 to allow 2P simultaneous play in "double command" mode
    - no rack test switch
    - different service mode inputs (bit 3 of DSW1 enables the test screen, bit 4 of IN1 just resets the game)
    - different bonus life values and only two lives options
    - difficulty switch is read, but has no effect. instead, higher difficulty is enabled in double command mode
    - free play mode is bugged; game is supposed to set up pointers to $5080/50c0 in RAM for later,
      but this only happens during the attract mode, which is skipped over if free play is enabled
*/
static INPUT_PORTS_START( clubpacm )
	PORT_START("IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(clubpacm_state, clubpacm_input_r)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(clubpacm_state, clubpacm_input_r)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING( 0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Cocktail ) )

	/* multiplexed player inputs */
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, "Free Play (Invalid)" ) /* causes watchdog reset at title screen, see comments above */
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "40000" ) /* service mode incorrectly says 20000 */
	PORT_DIPSETTING(    0x10, "60000" ) /* service mode incorrectly says 40000 */
	PORT_DIPSETTING(    0x20, "80000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Same as clubpacm, but Double Command mode is removed and normal inputs are used */
static INPUT_PORTS_START( clubpacma )
	PORT_INCLUDE( clubpacm )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mspactwin )
	PORT_INCLUDE( clubpacm )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Speed" ) // Jama
	PORT_DIPSETTING(    0x10, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_DIPNAME( 0x80, 0x80, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0C, "5" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x10, "15000" )
	PORT_DIPSETTING(    0x20, "20000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
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
	PORT_DIPNAME( 0x10, 0x10, "Test mode?" ) // Some kind of test/debug mode
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_PLAYER(2)

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
	PORT_DIPNAME( 0x20, 0x20, "Skip Screen" )           PORT_DIPLOCATION("SW:7") // End level after the first worm fed to your baby
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )       PORT_DIPLOCATION("SW:8")
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) // always select 2nd part of code
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

static INPUT_PORTS_START( crushbl3 )
	PORT_INCLUDE(mbrush)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	/* The 2nd player controls are used even in upright mode */
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
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


static INPUT_PORTS_START( eeekkp )
	PORT_INCLUDE(pacman)
	PORT_MODIFY("IN0")
	PORT_SERVICE_NO_TOGGLE( 0x10, 0x10 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME( "P2 Attack" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME( "P1 Attack/2 Player Start" )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR(Lives) ) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x1c, 0x10, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW:3,4,5")
	PORT_DIPSETTING(    0x00, "8 (Hard)" )
	PORT_DIPSETTING(    0x04, "7" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x14, "3" )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x1c, "1 (Easy)" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR(Demo_Sounds) ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPSETTING(    0x20, DEF_STR(Off) )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW:7")
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW:8")
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
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
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
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
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
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
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
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_DIPNAME( 0x10, 0x10, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // all ipt_unknown probably ipt_unused, game is one player only
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_CUSTOM ) // or it won't boot "I CAN NOT RUN"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )    PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x01, "50000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPNAME( 0x06, 0x04, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1) PORT_TOGGLE
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


static GFXDECODE_START( gfx_pacman )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, spritelayout, 0, 128 )
GFXDECODE_END


static GFXDECODE_START( gfx_s2650games )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0x4000, spritelayout, 0, 128 )
GFXDECODE_END


static GFXDECODE_START( gfx_superabc )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0x8000, spritelayout, 0, 128 )
GFXDECODE_END


static GFXDECODE_START( gfx_crush4 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, crush4_tilelayout,   0, 128 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, crush4_spritelayout, 0, 128 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void pacman_state::pacman(machine_config &config, bool latch)
{
	// Basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::pacman_map);
	m_maincpu->set_addrmap(AS_IO, &pacman_state::writeport);
	m_maincpu->set_irq_acknowledge_callback(FUNC(pacman_state::interrupt_vector_r));

	if (latch)
	{
		LS259(config, m_mainlatch); // 74LS259 at 8K or 4099 at 7K
		m_mainlatch->q_out_cb<0>().set(FUNC(pacman_state::irq_mask_w));
		m_mainlatch->q_out_cb<1>().set("namco", FUNC(namco_device::sound_enable_w));
		m_mainlatch->q_out_cb<3>().set(FUNC(pacman_state::flipscreen_w));
		m_mainlatch->q_out_cb<7>().set(FUNC(pacman_state::coin_counter_w));

		// NOTE(dwidel): The Pacman code uses $5004 and $5005 for LEDs and $5007 for coin lockout.  This hardware does not
		// exist on any Pacman or Puckman board I have seen.
		//m_mainlatch->q_out_cb<4>().set_output("led0");
		//m_mainlatch->q_out_cb<5>().set_output("led1");
		//m_mainlatch->q_out_cb<6>().set(FUNC(pacman_state::coin_lockout_global_w));
	}

	WATCHDOG_TIMER(config, m_watchdog);
	m_watchdog->set_vblank_count("screen", 16);

	// Video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pacman);

	PALETTE(config, m_palette, FUNC(pacman_state::pacman_palette), 128*4, 32);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(pacman_state::screen_update_pacman));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(pacman_state::vblank_irq));

	MCFG_VIDEO_START_OVERRIDE(pacman_state,pacman)

	// Sound hardware
	SPEAKER(config, "mono").front_center();

	NAMCO(config, m_namco_sound, MASTER_CLOCK/6/32);
	m_namco_sound->set_voices(3);
	m_namco_sound->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void pacman_state::crush2(machine_config &config)
{
	pacman(config);

	m_mainlatch->q_out_cb<7>().set_nop(); // coin counter not hooked up here
}

void pacman_state::korosuke(machine_config &config)
{
	pacman(config);

	// 8K on original boards
	m_mainlatch->q_out_cb<7>().set_nop(); // outputs 4-7 go to protection chip at 6P

	MCFG_MACHINE_RESET_OVERRIDE(pacman_state,maketrax)
}

void pacman_state::pengojpm(machine_config &config)
{
	pacman(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::pengojpm_map);
}


void pacman_state::birdiy(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	Z80(config.replace(), m_maincpu, MASTER_CLOCK/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::birdiy_map);

	m_mainlatch->q_out_cb<0>().set_nop();
	m_mainlatch->q_out_cb<1>().set(FUNC(pacman_state::irq_mask_w));

	MCFG_VIDEO_START_OVERRIDE(pacman_state,birdiy)
}


void pacman_state::piranha(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::woodpek_map);
	m_maincpu->set_addrmap(AS_IO, &pacman_state::piranha_portmap);
}


void pacman_state::nmouse(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &pacman_state::nmouse_portmap);
}


void pacman_state::mspacman(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::mspacman_map);

	m_mainlatch->q_out_cb<6>().set(FUNC(pacman_state::coin_lockout_global_w));
}


void pacman_state::woodpek(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::woodpek_map);
}


void pacman_state::woodpek_rbg(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::woodpek_map);

	// Video hardware
	m_palette->set_init(FUNC(pacman_state::pacman_rbg_palette));
}


void clubpacm_state::clubpacm(machine_config &config)
{
	mspacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &clubpacm_state::clubpacm_map);

	GENERIC_LATCH_8(config, m_sublatch);
}


void mspactwin_state::mspactwin(machine_config &config)
{
	mspacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &mspactwin_state::mspactwin_map);
	m_maincpu->set_addrmap(AS_OPCODES, &mspactwin_state::mspactwin_decrypted_map);

	m_mainlatch->q_out_cb<3>().set(FUNC(mspactwin_state::flipscreen_w));

	GENERIC_LATCH_8(config, m_sublatch);
}


void pacman_state::numcrash(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::numcrash_map);

	m_mainlatch->q_out_cb<3>().set_nop(); // ???
	m_mainlatch->q_out_cb<7>().set_nop(); // ???
}

void pacman_state::alibaba(machine_config &config)
{
	pacman(config, false);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::alibaba_map);

	ls259_device &latch1(LS259(config, "latch1"));
	latch1.q_out_cb<4>().set_output("led0");
	latch1.q_out_cb<5>().set_output("led1");
	latch1.q_out_cb<6>().set(FUNC(pacman_state::coin_lockout_global_w));
	latch1.q_out_cb<7>().set(FUNC(pacman_state::coin_counter_w));

	ls259_device &latch2(LS259(config, "latch2"));
	latch2.q_out_cb<0>().set("namco", FUNC(namco_device::sound_enable_w));
	latch2.q_out_cb<1>().set(FUNC(pacman_state::flipscreen_w));
	latch2.q_out_cb<2>().set(FUNC(pacman_state::irq_mask_w));
}

void pacman_state::dremshpr(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::dremshpr_map);
	m_maincpu->set_addrmap(AS_IO, &pacman_state::dremshpr_portmap);
	m_maincpu->remove_irq_acknowledge_callback();

	subdevice<screen_device>("screen")->screen_vblank().set(FUNC(pacman_state::vblank_nmi));

	// Sound hardware
	config.device_remove("namco");
	AY8910(config, "ay8910", 14318000/8).add_route(ALL_OUTPUTS, "mono", 0.50);

	m_mainlatch->q_out_cb<0>().set(FUNC(pacman_state::nmi_mask_w));
	m_mainlatch->q_out_cb<1>().set_nop();
}


void epospm_state::theglobp(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &epospm_state::epos_map);
	m_maincpu->set_addrmap(AS_IO, &epospm_state::epos_portmap);

	MCFG_MACHINE_START_OVERRIDE(epospm_state,theglobp)
	MCFG_MACHINE_RESET_OVERRIDE(epospm_state,theglobp)
}


void epospm_state::acitya(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &epospm_state::epos_map);
	m_maincpu->set_addrmap(AS_IO, &epospm_state::epos_portmap);

	MCFG_MACHINE_START_OVERRIDE(epospm_state,acitya)
	MCFG_MACHINE_RESET_OVERRIDE(epospm_state,acitya)
}


void epospm_state::eeekkp(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &epospm_state::epos_map);
	m_maincpu->set_addrmap(AS_IO, &epospm_state::epos_portmap);

	MCFG_MACHINE_START_OVERRIDE(epospm_state,eeekkp)
	MCFG_MACHINE_RESET_OVERRIDE(epospm_state,eeekkp)
}


void pacman_state::vanvan(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::dremshpr_map);
	m_maincpu->set_addrmap(AS_IO, &pacman_state::vanvan_portmap);
	m_maincpu->remove_irq_acknowledge_callback();

	// Video hardware
	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_visarea(2*8, 34*8-1, 0*8, 28*8-1);
	screen.screen_vblank().set(FUNC(pacman_state::vblank_nmi));

	// Sound hardware
	config.device_remove("namco");

	SN76496(config, "sn1", 1789750).add_route(ALL_OUTPUTS, "mono", 0.75);

	SN76496(config, "sn2", 1789750).add_route(ALL_OUTPUTS, "mono", 0.75);

	m_mainlatch->q_out_cb<0>().set(FUNC(pacman_state::nmi_mask_w));
	m_mainlatch->q_out_cb<1>().set_nop();
}


void pacman_state::bigbucks(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::bigbucks_map);
	m_maincpu->set_addrmap(AS_IO, &pacman_state::bigbucks_portmap);
	m_maincpu->set_periodic_int(FUNC(pacman_state::periodic_irq), attotime::from_hz(20*60));
	m_maincpu->remove_irq_acknowledge_callback();

	subdevice<screen_device>("screen")->set_visarea(0*8, 36*8-1, 0*8, 28*8-1);

	m_mainlatch->q_out_cb<7>().set_nop(); /*?*/
}


void pacman_state::s2650games(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	s2650_device &maincpu(S2650(config.replace(), m_maincpu, MASTER_CLOCK/6/2));    /* 2H */
	maincpu.set_addrmap(AS_PROGRAM, &pacman_state::s2650games_map);
	maincpu.set_addrmap(AS_DATA, &pacman_state::s2650games_dataport);
	maincpu.sense_handler().set("screen", FUNC(screen_device::vblank)).invert();
	maincpu.intack_handler().set([this]() { m_maincpu->set_input_line(0, CLEAR_LINE); return 0x03; });

	m_mainlatch->q_out_cb<0>().set_nop();
	m_mainlatch->q_out_cb<1>().set_nop();
	m_mainlatch->q_out_cb<2>().set_nop();
	m_mainlatch->q_out_cb<3>().set(FUNC(pacman_state::flipscreen_w));
	m_mainlatch->q_out_cb<4>().set_nop();
	m_mainlatch->q_out_cb<5>().set_nop();
	m_mainlatch->q_out_cb<6>().set_nop();
	m_mainlatch->q_out_cb<7>().set(FUNC(pacman_state::coin_counter_w));

	m_gfxdecode->set_info(gfx_s2650games);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(pacman_state::screen_update_s2650games));
	screen.screen_vblank().set(FUNC(pacman_state::s2650_interrupt));

	MCFG_VIDEO_START_OVERRIDE(pacman_state,s2650games)

	// Sound hardware
	config.device_remove("namco");
	SN76496(config, "sn1", MASTER_CLOCK/6).add_route(ALL_OUTPUTS, "mono", 0.75);    /* 1H */
}


void pacman_state::drivfrcp(machine_config &config)
{
	s2650games(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &pacman_state::drivfrcp_portmap);
}


void pacman_state::_8bpm(machine_config &config)
{
	s2650games(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &pacman_state::_8bpm_portmap);
}


void pacman_state::porky(machine_config &config)
{
	s2650games(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &pacman_state::porky_portmap);
}


void pacman_state::rocktrv2(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::rocktrv2_map);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_visarea(0*8, 36*8-1, 0*8, 28*8-1);
}


void pacman_state::mspacii(machine_config &config)
{
	woodpek(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_IO, &pacman_state::mspacii_portmap);
}


void pacman_state::mschamp(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::mschamp_map);
	m_maincpu->set_addrmap(AS_IO, &pacman_state::mschamp_portmap);

	MCFG_MACHINE_RESET_OVERRIDE(pacman_state,mschamp)
}


void pacman_state::superabc(machine_config &config)
{
	pacman(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::superabc_map);

	NVRAM(config, "28c16.u17", nvram_device::DEFAULT_ALL_0);

	MCFG_MACHINE_RESET_OVERRIDE(pacman_state,superabc)

	// Video hardware
	m_gfxdecode->set_info(gfx_superabc);
}


void pacman_state::crush4(machine_config &config)
{
	mschamp(config);

	// Basic machine hardware
	m_gfxdecode->set_info(gfx_crush4);

	m_mainlatch->q_out_cb<7>().set_nop(); // coin counter is not hooked up here
}

void pacman_state::crushs(machine_config &config)
{
	crush2(config);

	// Basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::crushs_map);
	m_maincpu->set_addrmap(AS_IO, &pacman_state::crushs_portmap);
	m_maincpu->remove_irq_acknowledge_callback();

	// Sound hardware
	ay8912_device &ay8912(AY8912(config, "ay8912", 1789750));
	ay8912.port_a_read_callback().set_ioport("DSW2");
	ay8912.add_route(ALL_OUTPUTS, "mono", 0.75);
}

void pacman_state::cannonbp(machine_config &config)
{
	pacman(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pacman_state::cannonbp_map);
}


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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "pm1-3.1m",     0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // 82s126
	ROM_LOAD( "pm1-2.3m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // 82s126 - timing - not used
ROM_END



ROM_START( pacmanso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pm-1r.6e",  0x0000, 0x0800, CRC(76dbed21) SHA1(400e5821aef77c9bb7117b7703afff911786d55a) ) // different
	ROM_LOAD( "pm-5r.6k",  0x0800, 0x0800, CRC(965bb9b2) SHA1(0865d15fc6fa2f1398672f2195285e65d8401423) ) // ^
	ROM_LOAD( "pm-2r.6f",  0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "pm-6r.6m",  0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "pm-3r.6h",  0x2000, 0x0800, CRC(a5af382c) SHA1(c5e2d89ce565b02a4e21c3afe8add87df63e3d90) ) // ^
	ROM_LOAD( "pm-7r.6n",  0x2800, 0x0800, CRC(a948ce83) SHA1(08759833f7e0690b2ccae573c929e2a48e5bde7f) )
	ROM_LOAD( "pm-4r.6j",  0x3000, 0x0800, CRC(cd03135a) SHA1(ccde7d62154778a45f2ce1d12b55fb8531e747db) ) // ^
	ROM_LOAD( "pm-8r.6p",  0x3800, 0x0800, CRC(fb397ced) SHA1(33722a9a4f77a08ebf05f832ba3393212597a2bd) ) // ^

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pm-9s.5e",   0x0000, 0x0800, CRC(2ee076d2) SHA1(64defe73a89e348db55a23446aa64017334ffdec) ) // replace NAMCO logo with Sonic logo
	ROM_LOAD( "pm-11s.5h",  0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pm-10s.5f",  0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "pm-12s.5j",  0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pm1-1.7f",     0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) ) // 82s123
	ROM_LOAD( "pm1-4.4a",     0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // 82s126

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "pm1-3.1m",     0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // 82s126
	ROM_LOAD( "pm1-2.3m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // 82s126 - timing - not used
ROM_END


ROM_START( pacmanvg ) // very similar to the pacmanso set, it has an accelerator feature
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pm-1r.6e",  0x0000, 0x0800, CRC(76dbed21) SHA1(400e5821aef77c9bb7117b7703afff911786d55a) )
	ROM_LOAD( "pm-5r.6k",  0x0800, 0x0800, CRC(965bb9b2) SHA1(0865d15fc6fa2f1398672f2195285e65d8401423) )
	ROM_LOAD( "pm-2r.6f",  0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "pm-6r.6m",  0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "pm-3r.6h",  0x2000, 0x0800, CRC(a5af382c) SHA1(c5e2d89ce565b02a4e21c3afe8add87df63e3d90) )
	ROM_LOAD( "pm-7r.6n",  0x2800, 0x0800, CRC(a948ce83) SHA1(08759833f7e0690b2ccae573c929e2a48e5bde7f) )
	ROM_LOAD( "pm-4r.6j",  0x3000, 0x0800, CRC(7c42d9be) SHA1(aa24271ee088d608da8c97220dcf661e0eb89c44) ) // different from pacmanso
	ROM_LOAD( "pm-8r.6p",  0x3800, 0x0800, CRC(68a7300d) SHA1(f0d37a56d51ef4a961cde35e8281a3b7792aea4b) ) // ^

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pm-9s.5e",   0x0000, 0x0800, CRC(2229ab07) SHA1(56000ed5009ae60c7f0498b5cac1b06da6ae270e) ) // blank the NAMCO logo
	ROM_LOAD( "pm-11s.5h",  0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pm-10s.5f",  0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "pm-12s.5j",  0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "pm1-1.7f",     0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) ) // 82s123
	ROM_LOAD( "pm1-4.4a",     0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // 82s126

	ROM_REGION( 0x0200, "namco", 0 ) // not dumped for this set
	ROM_LOAD( "pm1-3.1m",     0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // 82s126
	ROM_LOAD( "pm1-2.3m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // 82s126 - timing - not used
ROM_END


ROM_START( pacmanpe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "come1.6e",  0x0000, 0x0800, CRC(183d235a) SHA1(e428a91be6756f20e2bd8000cdaeba790d62a408) ) // different
	ROM_LOAD( "come5.6k",  0x0800, 0x0800, CRC(2771c530) SHA1(48926b46ff43b73d780a51b1249bf373384cb3a4) ) // ^
	ROM_LOAD( "come2.6f",  0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "come6.6m",  0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "come3.6h",  0x2000, 0x0800, CRC(cc92abb1) SHA1(260266615c8431f2dd1d667c3ebdee29bee11c73) ) // ^
	ROM_LOAD( "come7.6n",  0x2800, 0x0800, CRC(8810b38e) SHA1(c9b4e7ea070ebc80ebefdde6ef0583078ea378ae) ) // ^
	ROM_LOAD( "come4.6j",  0x3000, 0x0800, CRC(a02ce07f) SHA1(a59b83dc38063dbe4fc52819b268f3b287f745c7) ) // ^
	ROM_LOAD( "come8.6p",  0x3800, 0x0800, CRC(34e9566d) SHA1(a2ae925e23215d451bdbf77ada3e063f213eadb1) ) // ^

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "come9.5e",   0x0000, 0x0800, CRC(2229ab07) SHA1(56000ed5009ae60c7f0498b5cac1b06da6ae270e) )
	ROM_LOAD( "come11.5h",  0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "come10.5f",  0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "come12.5j",  0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pm1-1.7f",     0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) ) // 82s123
	ROM_LOAD( "pm1-4.4a",     0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // 82s126

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


// More recent bootleg board running a Spanish version of the game with larger ROMs and 'MADE IN GREECE' marking.
// Game has a high score name entry feature, with the name displayed next to 'El Super' on the title screen.
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // == 82s126.1m
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used // == 82s126.3m
ROM_END

ROM_START( mspacmanbg2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11-prg.bin",  0x0000, 0x4000,  CRC(e11d4132) SHA1(9ab6b9e1ec8ad183ccdddb971e8be3eb3f59db01) )
	ROM_CONTINUE(0x8000,0x4000) // blocks 5+6 are repeated twice in here

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "13-chr.bin",    0x0000, 0x0800, CRC(8ee4a3b0) SHA1(01e3453c99f7a5d78ab083c49c650e898c0dd2ee) )
	ROM_CONTINUE(0x1000,0x800)
	ROM_CONTINUE(0x0800,0x800)
	ROM_CONTINUE(0x1800,0x800)
	ROM_IGNORE(0x2000)

	ROM_REGION( 0x0120, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "82s123.h7",    0x0000, 0x0020, BAD_DUMP CRC(3545e7e9) SHA1(b866b02579438afb11296e5c53a32c6425bd044d) )
	ROM_LOAD( "82s129-3.d1",  0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs, not dumped for this set
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END

ROM_START( mspacmanbi ) // very similar to mspacmanbg
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2.bin",  0x0000, 0x4000,  BAD_DUMP CRC(03905a76) SHA1(1780ef598c6150ffa44bf467479670f7ca50d512) )
	ROM_CONTINUE(       0x8000, 0x4000) // blocks 5+6 are repeated twice in here, but for 0xac0d that differs from 0x8c0d
	ROM_FILL( 0x18b5, 0x01, 0x4e ) // bad dump, has 0x04 instead of 0x4e. Causes Ms. Pacman not responding to controls / moving autonomously
	ROM_FILL( 0x197d, 0x01, 0xda ) // bad dump, has 0x92 instead of 0xda. Causes Ms. Pacman misplacements / teleportations inside and outside of the maze
	ROM_FILL( 0x1a61, 0x01, 0x21 ) // bad dump, has 0x00 instead of 0x21, resulting in illegal opcode
	ROM_FILL( 0xac0d, 0x01, 0xeb ) // bad dump, has 0xea instead of 0xeb. No ill effect observed, but better safe than sorry
	// other differences are due to copyright and year change

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "1.bin",    0x0000, 0x0800, CRC(8ee4a3b0) SHA1(01e3453c99f7a5d78ab083c49c650e898c0dd2ee) )
	ROM_CONTINUE(0x1000,0x800)
	ROM_CONTINUE(0x0800,0x800)
	ROM_CONTINUE(0x1800,0x800)
	ROM_IGNORE(0x2000)

	ROM_REGION( 0x0120, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "82s123.h7",    0x0000, 0x0020, BAD_DUMP CRC(3545e7e9) SHA1(b866b02579438afb11296e5c53a32c6425bd044d) )
	ROM_LOAD( "82s129-3.d1",  0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs, not dumped for this set
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // == 82s126.1m
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used // == 82s126.3m
ROM_END

ROM_START( mspacmanbco )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cs.11",  0x0000, 0x4000, CRC(1ba81f43) SHA1(db1407231f760a0c213d29d1126dd22dd4fcc5b9) )
	ROM_CONTINUE(0x8000,0x4000) // blocks 5+6 are repeated twice in here

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "c13.13",    0x0000, 0x0800, CRC(f2c5da43) SHA1(6a6de2ecc313a11ad12d8d1712c05f923984f668) )
	ROM_CONTINUE(0x1000,0x800)
	ROM_CONTINUE(0x0800,0x800)
	ROM_CONTINUE(0x1800,0x800)
	ROM_IGNORE(0x6000) // this also contains regular pacman gfx, ignore them for now at least

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.h7",    0x0000, 0x0020, CRC(3545e7e9) SHA1(b866b02579438afb11296e5c53a32c6425bd044d) ) // slightly different to original (verified)
	ROM_LOAD( "82s129-3.d1",  0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // == 82s126.4a

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // == 82s126.1m
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used // == 82s126.3m
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END

ROM_START( mspacmanblt2 ) // very small differences to the above
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11.bin", 0x0000, 0x4000, CRC(763c2abb) SHA1(d9428ca04fc7b26f83cee4629be021c1c806e001) )
	ROM_CONTINUE(       0x8000, 0x4000 )

	ROM_REGION( 0x8000, "gfx1", 0 ) // first 0x2000 are identical to mspacmanblt
	ROM_LOAD( "13.bin", 0x0000, 0x800, CRC(f2c5da43) SHA1(6a6de2ecc313a11ad12d8d1712c05f923984f668) )
	ROM_CONTINUE(       0x1000, 0x800 )
	ROM_CONTINUE(       0x0800, 0x800 )
	ROM_CONTINUE(       0x1800, 0x800 )
	ROM_IGNORE(         0x6000 )

	ROM_REGION( 0x0120, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "82s123.h7",   0x0000, 0x0020, BAD_DUMP CRC(3545e7e9) SHA1(b866b02579438afb11296e5c53a32c6425bd044d) )
	ROM_LOAD( "82s129-3.d1", 0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs, not dumped for this set
	ROM_LOAD( "82s129-1.a9", 0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129-2.c9", 0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // == 82s126.1m
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used // == 82s126.3m
ROM_END

ROM_START( mspacmanbgf ) // Mr Pac-Turbo. Argentine bootleg with turbo speed
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic11.bin", 0x0000, 0x4000, CRC(6573a470) SHA1(d66ed9e79d12bd6e61816a974f2500dee69b9ad6) )
	ROM_CONTINUE(       0x8000, 0x4000 )

	ROM_REGION( 0x8000, "gfx1", 0 ) // first 0x2000 are identical to mspacmanblt
	ROM_LOAD( "ic13.bin", 0x0000, 0x800, CRC(8ee4a3b0) SHA1(01e3453c99f7a5d78ab083c49c650e898c0dd2ee) )
	ROM_CONTINUE(       0x1000, 0x800 )
	ROM_CONTINUE(       0x0800, 0x800 )
	ROM_CONTINUE(       0x1800, 0x800 )
	ROM_IGNORE(         0x2000 )

	ROM_REGION( 0x0120, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "82s123.h7",   0x0000, 0x0020, BAD_DUMP CRC(3545e7e9) SHA1(b866b02579438afb11296e5c53a32c6425bd044d) )
	ROM_LOAD( "82s129-3.d1", 0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs, not dumped for this set
	ROM_LOAD( "82s129-1.a9", 0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129-2.c9", 0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END

ROM_START( mspacmanbhe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11.g5",  0x0000, 0x4000,  CRC(b256540f) SHA1(2ffdb9a9af98606793e9cb8b151370070be45091) )
	ROM_CONTINUE(0x8000,0x4000) // blocks 5+6 are repeated twice in here

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "13.e5",    0x0000, 0x0800, CRC(8ee4a3b0) SHA1(01e3453c99f7a5d78ab083c49c650e898c0dd2ee) )
	ROM_CONTINUE(0x1000,0x800)
	ROM_CONTINUE(0x0800,0x800)
	ROM_CONTINUE(0x1800,0x800)
	ROM_IGNORE(0x2000)

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.h7",    0x0000, 0x0020, CRC(3545e7e9) SHA1(b866b02579438afb11296e5c53a32c6425bd044d) ) // slightly different to original (verified)
	ROM_LOAD( "82s129-3.d1",  0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // == 82s126.4a

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // == 82s126.1m
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used // == 82s126.3m
ROM_END

ROM_START( mspacmanbgc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9cl.g5",  0x0000, 0x4000, CRC(a846bd10) SHA1(d585462d99a99014051f2dbbefee197127d9ec9e) )
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s129-1.a9",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // == 82s126.1m
	ROM_LOAD( "82s129-2.c9",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used // == 82s126.3m
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "m7611.1m",     0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "m7611.3m",     0x0100, 0x0100, CRC(0e307106) SHA1(6140b5339008dd3110cd5be2e2fb4813779dfe28) )    // Timing - not used
ROM_END

ROM_START( clubpacma )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "new_club.bin",  0x0000, 0x4000, CRC(6d79d3f2) SHA1(61ac436b4dc30b074e6c95f13efbf66f9aa1e2f7) )
	ROM_CONTINUE(0x8000,0x4000)
	// there is also a bad dump with the following hashes
	// CRC(3c8ce103) SHA1(1d3fd485f68c2c1ed6f19dcee51456a4d0ab0a01)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "12.5e",   0x0000, 0x0800, CRC(93933d1d) SHA1(fa38d2cb87e872bb9a3158a4df98f38360dc85ec) )
	ROM_LOAD( "14.5h",   0x0800, 0x0800, CRC(7409fbec) SHA1(f440f08ba026ae6172666e1bdc0894ce33bba420) )
	ROM_LOAD( "13.5f",   0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "15.5j",   0x1800, 0x0800, CRC(50c7477d) SHA1(c04ec282a8cb528df5e38ad750d12ee71612695d) )

	// Color PROMs have been dumped. They match the pacman/mspacman ones
	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "n82s123n.7f",  0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "m7611.4a",     0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "m7611.1m",     0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "m7611.3m",     0x0100, 0x0100, CRC(0e307106) SHA1(6140b5339008dd3110cd5be2e2fb4813779dfe28) )    // Timing - not used
ROM_END


/*
  Super Ms Pac-Man (turbo hack)
  -----------------------------

  This Ms. Pac-Man turbo game has all the info in a 27256 EPROM.
  It runs on a hardware with NVC284 and NVC285 Namco customs

  Also the PCB has a lot of hacks involving high addressing lines
  with different TTL components.


   EPROM 27256   CPU addressing
  -------------+----------------
    0000-0fff  |  0000-0fff
    1000-1fff  |  8000-8fff
    2000-2fff  |  1000-1fff
    3000-37ff  |  9000-97ff
    3800-3fff  |  9800-9fff (empty)
               |
    6000-6fff  |  2000-2fff
    7000-7fff  |  3000-3fff


  There is a complete graphics set at 4000-5fff of the 27256 EPROM.
  Still don't know if the game is using this set, or the one stored in the original 2732 EPROMs.
  Both GFX sets are identical.

   EPROM 27256   Graphics
  -------------+----------------
    4000-47ff  |  0000-07ff (GFX ROM @5e)
    4800-4fff  |  0800-0fff (GFX ROM @5h)
    5000-57ff  |  1000-17ff (GFX ROM @5f)
    5800-5fff  |  1800-1fff (GFX ROM @5j)

*/
ROM_START( mspacmanhnc )
	ROM_REGION( 0x8000, "bigeprom", 0 )
	ROM_LOAD( "6f.bin",  0x0000, 0x8000, CRC(db164116) SHA1(e5b16b37e765ee46681b1d565c67d3eda94cd0f1) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_COPY( "bigeprom",  0x0000, 0x0000, 0x1000 )   // copy segment to 0000-0fff
	ROM_COPY( "bigeprom",  0x2000, 0x1000, 0x1000 )   // copy segment to 1000-1fff
	ROM_COPY( "bigeprom",  0x6000, 0x2000, 0x1000 )   // copy segment to 2000-2fff
	ROM_COPY( "bigeprom",  0x7000, 0x3000, 0x1000 )   // copy segment to 3000-3fff
	ROM_COPY( "bigeprom",  0x1000, 0x8000, 0x1000 )   // copy segment to 8000-8fff
	ROM_COPY( "bigeprom",  0x3000, 0x9000, 0x0800 )   // copy segment to 9000-97ff

	ROM_REGION( 0x2000, "gfx1", 0 )
//  ROM_COPY( "bigeprom",  0x4000, 0x0000, 0x2000 )   // copy segments to 0000-1fff, same GFX set of the GFX EPROMs
	ROM_LOAD( "5e.bin",   0x0000, 0x0800, CRC(93933d1d) SHA1(fa38d2cb87e872bb9a3158a4df98f38360dc85ec) )
	ROM_LOAD( "5h.bin",   0x0800, 0x0800, CRC(7409fbec) SHA1(f440f08ba026ae6172666e1bdc0894ce33bba420) )
	ROM_LOAD( "5f.bin",   0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "5j.bin",   0x1800, 0x0800, CRC(50c7477d) SHA1(c04ec282a8cb528df5e38ad750d12ee71612695d) )

	// from parent set...
	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123-cpu.7f",    0x0000, 0x0020, BAD_DUMP CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129-vid.4a",    0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s129-vid.1m",    0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129-vid.3m",    0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


/*****************************************************************************

  Ms Pac Man Twin (SUSILU)
  ------------------------

  Argentine official hack of Ms. Pac-Man that allows 2 players simultaneously.
  Very rare PCB. Was avoiding us for more of 22 years...
  The mainboard is a PacMan bootleg PCB, with a mini daughterboard
  (normal in mspacman boards) plugged in the Z80 socket.


  Here the specs:

  Mini daughterboard 3" x 5.5" aprox.

  In the upper-left corner is vertically printed "SUSILU" and "NMPT008",
  and across: "MOD REG 294535" and "INDUSTRIA ARGENTINA".

  .----------------------------.
  |      .----------.          |
  |      | 74LS245N < U5       |
  |      '----------'          |
  |                            |
  |   .--------------------.   |
  |   |                    |   |
  |   |  UNKNOWN DIP40 IC  <   |
  |   |                    |U3 |
  |   '--------------------'   |
  |                            |
  |    oooooooooooooooooooo    |
  |                       1    |
  |             U1             |
  |                       40   |
  |    oooooooooooooooooooo    |
  |                            |
  |   .--------------------.   |
  |   |                    |   |
  |   |     NEC D780C      <   |
  |   |                    |U2 |
  |   '--------------------'   |
  |                            |
  |        .--------------.    |
  |        |              |    |
  |        |    M27256    <    |
  |        |              | U4 |
  |        '--------------'    |
  '----------------------------'

  Viewing from the top you can find: U5 U3 U1 U2 U4

  U5: 74LS245N (Octal Bus Tranceivers with 3-state outputs, DIP20),
      with a decoupling cap soldered from leg 10 to leg 20.

  U3: Unknown 40 pins IC, scratched to avoid recognition. Could be either
      a big PLD/CPLD/EPLD, or a kind of protection device.

  U1: 40 pins male connector, to plug the kit into Z80's mainboard socket.

  U2: NEC D780C (Z80).

  U4: ST M27256 ROM (program).
      NOTE: There was a TOSHIBA TMM24512AP-20 (28 pins), 512 Kbit OTPROM
      in the first board I found, but remains undumped.


  ---------------------------------------------------------

  40-pins connector @ U1:

                              40-pins connector
                         +-------------------------+
     Z80 pin 01 (A11) ---o 01 (A11)       (A10) 40 o--- Z80 pin 40 (A10)
     Z80 pin 02 (A12) ---o 02 (A12)       (A09) 39 o--- Z80 pin 39 (A09)
     Z80 pin 03 (A13) ---o 03 (A13)       (A08) 38 o--- Z80 pin 38 (A08)
     Z80 pin 04 (A14) ---o 04 (A14)       (A07) 37 o--- Z80 pin 37 (A07)
     Z80 pin 05 (A15) ---o 05 (A15)       (A06) 36 o--- Z80 pin 36 (A06)
     Z80 pin 06 (CLK) ---o 06 (CLK)       (A05) 35 o--- Z80 pin 35 (A05)
  74LS245 pin 02 (A1) ---o 07 (D4)        (A04) 34 o--- Z80 pin 34 (A04)
  74LS245 pin 03 (A2) ---o 08 (D3)        (A03) 33 o--- Z80 pin 33 (A03)
  74LS245 pin 04 (A3) ---o 09 (D5)        (A02) 32 o--- Z80 pin 32 (A02)
  74LS245 pin 05 (A4) ---o 10 (D6)        (A01) 31 o--- Z80 pin 31 (A01)
     Z80 pin 11 (VCC) ---o 11 (VCC)       (A00) 30 o--- Z80 pin 30 (A00)
  74LS245 pin 06 (A5) ---o 12 (D2)        (GND) 29 o--- Z80 pin 29 (GND)
  74LS245 pin 07 (A6) ---o 13 (D7)      (/RFSH) 28 o--- Z80 pin 28 (/RFSH)
  74LS245 pin 08 (A7) ---o 14 (D0)        (/M1) 27 o--- Z80 pin 27 (/M1)
  74LS245 pin 09 (A8) ---o 15 (D1)     (/RESET) 26 o--- Z80 pin 26 (/RESET)
    Z80 pin 16 (/INT) ---o 16 (/INT)   (/BUSRQ) 25 o--- N/C
                  N/C ---o 17 (/NMI)    (/WAIT) 24 o--- Z80 pin 24 (/WAIT)
                  N/C ---o 18 (/HALT)  (/BUSAK) 23 o--- N/C
   Z80 pin 19 (/MREQ) ---o 19 (/MREQ)     (/WR) 22 o--- Z80 pin 22 (/WR)
   Z80 pin 20 (/IORQ) ---o 20 (/IORQ)     (/RD) 21 o--- Z80 pin 21 (/RD)
                         +-------------------------+


  NEC D780C (Z80) @ U2:

                                   NEC D780C (Z80)
                              .-----------v-----------.
    connector pin 01 (A11) ---|01 (A11)       (A10) 40|--- connector pin 40 (A10)
    connector pin 02 (A12) ---|02 (A12)       (A09) 39|--- connector pin 39 (A09)
    connector pin 03 (A13) ---|03 (A13)       (A08) 38|--- connector pin 38 (A08)
    connector pin 04 (A14) ---|04 (A14)       (A07) 37|--- connector pin 37 (A07)
    connector pin 05 (A15) ---|05 (A15)       (A06) 36|--- connector pin 36 (A06)
    connector pin 06 (CLK) ---|06 (CLK)       (A05) 35|--- connector pin 35 (A05)
      unknown DIP40 pin 32 ---|07 (D4)        (A04) 34|--- connector pin 34 (A04)
      unknown DIP40 pin 31 ---|08 (D3)        (A03) 33|--- connector pin 33 (A03)
      unknown DIP40 pin 30 ---|09 (D5)        (A02) 32|--- connector pin 32 (A02)
      unknown DIP40 pin 29 ---|10 (D6)        (A01) 31|--- connector pin 31 (A01)
    connector pin 11 (VCC) ---|11 (VCC)       (A00) 30|--- connector pin 30 (A00)
      unknown DIP40 pin 28 ---|12 (D2)        (GND) 29|--- connector pin 29 (GND)
      unknown DIP40 pin 27 ---|13 (D7)      (/RFSH) 28|--- connector pin 28 (/RFSH)
      unknown DIP40 pin 26 ---|14 (D0)        (/M1) 27|--- connector pin 27 (/M1)
      unknown DIP40 pin 25 ---|15 (D1)     (/RESET) 26|--- connector pin 26 (/RESET)
   connector pin 16 (/INT) ---|16 (/INT)   (/BUSRQ) 25|--- VCC
                       VCC ---|17 (/NMI)    (/WAIT) 24|--- connector pin 24 (/WAIT)
                       N/C ---|18 (/HALT)  (/BUSAK) 23|--- N/C
  connector pin 19 (/MREQ) ---|19 (/MREQ)     (/WR) 22|--- connector pin 22 (/WR)
  connector pin 20 (/IORQ) ---|20 (/IORQ)     (/RD) 21|--- connector pin 21 (/RD)
                              '-----------------------'


  Unknown DIP40 @ U3:

  Is this a big PLD?
  Like Altera EP900 (24 macrocell EPLD),
  or Intel 5C090 (24 macrocell CMOS PLD)?


                              unknown DIL40
                              .-----v-----.
                         GND -|01   ?   40|- VCC
  connector/Z80 pin 21 (/RD) -|02       39|- GND
        74LS245 pin 19 (/OE) -|03-.     38|- GND
  connector/Z80 pin 27 (/M1) -|04 |     37|- GND
                         N/C -|05 |     36|- GND
        74LS245 pin 19 (/OE) -|06-'     35|- GND
                         N/C -|07       34|- M27256 pin 20 (/CE)
  connector/Z80 pin 03 (A13) -|08       33|- connector/Z80 pin 30 (A0)
         74LS245 pin 02 (D4) -|09       32|- Z80 pin 07 (D4)
         74LS245 pin 03 (D3) -|10       31|- Z80 pin 08 (D3)
         74LS245 pin 04 (D5) -|11       30|- Z80 pin 09 (D5)
         74LS245 pin 05 (D6) -|12       29|- Z80 pin 10 (D6)
         74LS245 pin 06 (D2) -|13       28|- Z80 pin 12 (D2)
         74LS245 pin 07 (D7) -|14       27|- Z80 pin 13 (D7)
         74LS245 pin 08 (D0) -|15       26|- Z80 pin 14 (D0)
         74LS245 pin 09 (D1) -|16       25|- Z80 pin 15 (D1)
  connector/Z80 pin 04 (A14) -|17       24|- GND
  connector/Z80 pin 05 (A15) -|18       23|- GND
                         GND -|19       22|- GND
                         GND -|20       21|- GND
                              '-----------'


  M27256 @ U4:

                                   M27256
                              .-------v-------.
                         VCC -|01 VPP   VCC 28|- VCC
  connector/Z80 pin 02 (A12) -|02 A12   A14 27|- connector/Z80 pin 05 (A15) & unknown DIP40 pin 18
  connector/Z80 pin 37 (A07) -|03 A07   A13 26|- connector/Z80 pin 03 (A13)
  connector/Z80 pin 36 (A06) -|04 A06   A08 25|- connector/Z80 pin 38 (A08)
  connector/Z80 pin 35 (A05) -|05 A05   A09 24|- connector/Z80 pin 39 (A09)
  connector/Z80 pin 34 (A04) -|06 A04   A11 23|- connector/Z80 pin 01 (A11)
  connector/Z80 pin 33 (A03) -|07 A03   /OE 22|- connector/Z80 pin 21 (/RD) & unknown DIP40 pin 02
  connector/Z80 pin 32 (A02) -|08 A02   A10 21|- connector/Z80 pin 40 (A10)
  connector/Z80 pin 31 (A01) -|09 A01   /CE 20|- unknown DIP40 pin 34
  connector/Z80 pin 30 (A00) -|10 A00    O7 19|- 74LS245 pin 07 (D7)
         74LS245 pin 08 (D0) -|11 O0     O6 18|- 74LS245 pin 05 (D6)
         74LS245 pin 09 (D1) -|12 O1     O5 17|- 74LS245 pin 04 (D5)
         74LS245 pin 06 (D2) -|13 O2     O4 16|- 74LS245 pin 02 (D4)
                         GND -|14 GND    O3 15|- 74LS245 pin 03 (D3)
                              '---------------'


  74LS245N @ U5:

                              74LS245
                         .-------v-------.
                    GND -|01 DIR   VCC 20|- VCC
  connector pin 07 (D4) -|02 A1    /OE 19|- unknown DIP40 pins 03 & 06
  connector pin 08 (D3) -|03 A2     B1 18|- Z80 pin 07 (D4)
  connector pin 09 (D5) -|04 A3     B2 17|- Z80 pin 08 (D3)
  connector pin 10 (D6) -|05 A4     B3 16|- Z80 pin 09 (D5)
  connector pin 12 (D2) -|06 A5     B4 15|- Z80 pin 10 (D6)
  connector pin 13 (D7) -|07 A6     B5 14|- Z80 pin 12 (D2)
  connector pin 14 (D0) -|08 A7     B6 13|- Z80 pin 13 (D7)
  connector pin 15 (D1) -|09 A8     B7 12|- Z80 pin 14 (D0)
                    GND -|10 GND    B8 11|- Z80 pin 15 (D1)
                         '---------------'


  Because program EPROM A14 goes to Z80 A15, and Z80 A14 isn't connected to anything,
  the EPROM data should be offset as follows:

  Z80 address | EPROM offset
  ------------+-------------
   0000-3FFF  |  0000-3FFF
   4000-7FFF  |  0000-3FFF
   8000-BFFF  |  4000-7FFF
   C000-FFFF  |  4000-7FFF

  All the graphics ROMs and bipolar PROMs are located
  in the mainboard, at normal positions.



  ********** Second PCB *********

  A second PCB was found!
  The hardware base is totally different from the first set.

  It's a two PCB system with the original SUSILU Ms PacMan Twin daughterboard replacing the Z80 at location 6B,
  plus two daughterboards replacing the original Namco customs NVC284 at location 5S (VRAM addresser),
  and NVC285 at location 6D (Z80 sync bus controller).

  Program ROM is way different and need to be anlyzed deeply to find behaviour differences in the game.
  Besides, the graphics ROM 3 (@ location 5h) has extra bitmapped strings "Push Start" and "Insert Coins"
  that are not present in the parent set.


  PCB Layout...

   G-HB
  .---------------------------------------------------------------------------------.
  |         1             2              3              4                 5         |
  |                  .----------.   .----------.                  .--------------.  |
  | S                | 74LS161  |   | 74LS161  |                  |   PKN00004   |  |
  |                  '----------'   '----------'                  |    socket    |  |
  |                  .----------.   .----------.   .----------.   '--------------'  |
  | R                | 74LS161  |   | 74LS161  |   |  2114-2  |                     |
  |                  '----------'   '----------'   '----------'                     |
  |                  .----------.   .----------.   .----------.                     |
  | P   RESNET 1     |  74LS02  |   |  74LS10  |   |  2114-2  |                     |
  |                  '----------'   '----------'   '----------'                     |
  |   .----------.                  .----------.   .----------.      .----------.   |
  | N |  CD4066  |     RESNET 2     |  74LS74  |   |  2114-2  |      |  74LS08  |   |
  |   '----------'                  '----------'   '----------'      '----------'   |
  |   .----------.   .----------.   .----------.   .----------.      .----------.   |
  | M |  IM5623  |   | 74LS273  |   |  IM5623  |   |  2114-2  |      |  74LS74  |   |
  |   '----------'   '----------'   '----------'   '----------'      '----------'   |
  |   .----------.   .----------.   .----------.   .----------.      .----------.   |
  | L | 74LS174  |   |  74S89   |   | 74LS157  |   |  2114-2  |      | 74LS139  |   |-----.
  |   '----------'   '----------'   '----------'   '----------'      '----------'   |-----| R
  |   .----------.   .----------.   .----------.   .----------.                     |-----| I
  | K | 74LS283  |   |  74S89   |   | 74LS158  |   |  2114-2  |                     |-----| B
  |   '----------'   '----------'   '----------'   '----------'   .--------------.  |-----| B
  |                                                               |    MB5816    |  |-----| O
  |                                                               |              |  |-----| N
  | J                                                             '--------------'  |-----|
  |                                                               .--------------.  |-----| C
  |                                                               |    MB5816    |  |-----| A
  |   .----------.   .----------.   .----------.   .----------.   |              |  |-----| B
  | H | 74LS174  |   | 74LS86   |   |  74S89   |   | 74LS245  |   '--------------'  |-----| L
  |   '----------'   '----------'   '----------'   '----------'   .--------------.  |-----| E
  |   .----------.   .----------.   .----------.   .----------.   |    MB5816    |  |-----|
  | F | 74LS283  |   | 74LS283  |   |  74S89   |   |  74LS86  |   |              |  |-----'
  |   '----------'   '----------'   '----------'   '----------'   '--------------'  |
  |                                                               .--------------.  |
  |   .----------.   .----------.   .----------.   .----------.   |    MB5816    |  |
  | E | 74LS161  |   | 74LS161  |   |  74LS20  |   | 74LS157  |   |              |  |
  |   '----------'   '----------'   '----------'   '----------'   '--------------'  |
  |                  .----------.                  .----------.                     |
  | D                |  2115A   |                  | 74LS273  |                     |
  |                  '----------'                  '----------'                     |
  |                  .----------.   .----------.   .----------.      .----------.   |
  | C                |  2115A   |   | 74LS375  |   |  74LS00  |      | 74LS194  |   |
  |                  '----------'   '----------'   '----------'      '----------'   |
  |                  .----------.   .----------.   .----------.      .----------.   |
  | B                |  2115A   |   | 74LS157  |   | 74LS377  |      | 74LS194  |   |
  |                  '----------'   '----------'   '----------'      '----------'   |
  |                  .----------.   .----------.   .----------.      .----------.   |
  | A                |  2115A   |   | 74LS158  |   | 74LS287  |      | 74LS157  |   |
  |                  '----------'   '----------'   '----------'      '----------'   |
  |                                                                                 |
  '---------------------------------------------------------------------------------'


           G-HA
          .----------------------------------------------------------------------------------.
          |           6               7              8              9                        |
          |       .----------.                                                               |
          | S     | 74LS367  |                                                               |
          |       '----------'                                                               |
          |       .----------.                                                               |
          | R     | 74LS367  |                                                               |
          |       '----------'                                                               |
          |   .--------------.                                                               |
          |   |              |                                                               |
          | P |    socket    |                                                               |
          |   '--------------'                                                               |
          |   .--------------.   .----------.                                                |
          | N |              |   |  74LS42  |                                                |
          |   |    socket    |   '----------'                                                |
          |   '--------------'                                                               |
          |   .--------------.   .----------.                                                |
          | M |              |   | 74LS139  |                                             .--'
          |   |    socket    |   '----------'                                             |
          |   '--------------'   .----------.                                          01 '--.
    .-----| L .--------------.   |  74LS02  |                                             ---|
  R |-----|   |              |   '----------'                                             ---|
  I |-----|   |    socket    |                                                            ---|
  B |-----|   '--------------'                  .----------.                              ---|
  B |-----| K .--------------.                  | 74LS259  |                              ---|
  O |-----|   |              |                  '----------'                        2x22  ---|
  N |-----|   |    socket    |   .----------.                                       edge  ---|
    |-----| J '--------------'   | 741LS38  |                                       conn  ---|
  C |-----|   .--------------.   '----------'                                             ---|
  A |-----|   |              |   .----------.   .----------.                              ---|
  B |-----| H |    socket    |   |  74LS08  |   | 74LS367  |                              ---|
  L |-----|   '--------------'   '----------'   '----------'                              ---|
  E |-----|   .--------------.                                                            ---|
    |-----|   |              |   .----------.   .----------.                              ---|
    '-----| F |    socket    |   |   7603   |   | 74LS367  |                              ---|
          |   '--------------'   '----------'   '----------'                           22 .--'
          |   .--------------.   .----------.   .----------.                              |
          | E |              |   |          |   | 74LS367  |                              '--.
          |   |    socket    |   '----------'   '----------'                                 |
          |   '--------------'                                                               |
          |       .--------------.              .----------.   .--------------.              |
          | D     |   PKN00003   |              | 74LS367  |   | DIP switches |              |
          |       |    socket    |              '----------'   '--------------'              |
          |       '--------------'              .----------.   .----------.                  |
          | C                                   |  74LS74  |   | 74LS161  |                  |
          |     .-------------------. .------.  '----------'   '----------'                  |
          |     |      SUSILU       | |18.432|  .----------.                                 |
          | B   |      socket       | |  MHZ |  |  74258   |                                 |
          |     '-------------------' '------'  '----------'                  .----------.   |
          |                             Xtal    .----------.                  |          |   |
          | A                                   |  74107   |                  |  MB3712  |   |
          |                                     '----------'                  '----------'   |
          |   .--.                        .--.                                               |
          |   |  ||||||||||||||||||||||||||  |                                               |
          '---'  '------------------------'  '-----------------------------------------------'
                    2x25 edge connector


  Custom NVC284 replacement PCB (at location 5S):
  .----------------------------------------------.
  |                 .----------.       PKN00004  |
  |                 | 74LS367  |                 |
  |                 '----------'                 |
  |  .----------.   . . . . . . . . . . . . . .  |
  |  |  74LS32  |                                |
  |  '----------'       C O N N E C T O R        |
  |  .----------.                                |
  |  | 74LS138  |   . . . . . . . . . . . . . .  |
  |  '----------'                                |
  |  .----------.   .----------.   .----------.  |
  |  |  74LS86  |   | 74LS257  |   | 74LS257  |  |
  |  '----------'   '----------'   '----------'  |
  |  .----------.   .----------.   .----------.  |
  |  |  74LS86  |   | 74LS257  |   | 74LS257  |  |
  |  '----------'   '----------'   '----------'  |
  |  .----------.   .----------.   .----------.  |
  |  |  74LS86  |   | 74LS257  |   | 74LS257  |  |
  |  '----------'   '----------'   '----------'  |
  |  .----------.   .----------.   .----------.  |
  |  |  74LS08  |   |  74LS04  |   | 74LS148  |  |
  |  '----------'   '----------'   '----------'  |
  '----------------------------------------------'


  Custom NVC285 replacement PCB (at location 6D):
  .-----------------------------------------------.
  |                   .  C   .          PKN00003  |
  |  .----------.     .  O   .                    |
  |  | 74LS139  |     .  N   .                    |
  |  '----------'     .  N   .                    |
  |  .----------.     .  E   .     .-----------.  |
  |  | 74LS139  |     .  C   .     |  74LS373  |  |
  |  '----------'     .  T   .     '-----------'  |
  |  .----------.     .  O   .     .-----------.  |
  |  |  74LS08  |     .  R   .     |  74LS373  |  |
  |  '----------'                  '-----------'  |
  |  .----------.   .----------.   .-----------.  |
  |  |  74LS32  |   |  74LS04  |   |  74LS373  |  |
  |  '----------'   '----------'   '-----------'  |
  |  .----------.   .----------.   .-----------.  |
  |  |  74LS14  |   | 74LS109  |   |  74LS109  |  |
  |  '----------'   '----------'   '-----------'  |
  |                                               |
  '-----------------------------------------------'


  The SUSILU PCB soldered below the Z80 socket, is exactly
  the same documented above, containing a Z80 CPU, a M27256 EPROM,
  a 74LS254 and an unknown DIL40 IC.


  Docs by Roberto Fresca.


*****************************************************************************/

ROM_START( mspactwin )
	ROM_REGION( 0x10000, "maincpu", 0 )  // 64k for encrypted code
	ROM_LOAD( "m27256.bin",  0x0000, 0x4000, CRC(77a99184) SHA1(9dcb1a1b78994aa401d653bec571cb3e6f9d900b) )
	ROM_CONTINUE(0x8000,0x4000)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "4__2716.5d",  0x0000, 0x0800, CRC(483c1d1c) SHA1(d3b967c6a71cf02b825d800f56d5268f2e0e60eb) )
	ROM_LOAD( "2__2716.5g",  0x0800, 0x0800, CRC(c08d73a2) SHA1(072e57641ac5ae3c47b4f8d9c55e3da5b35489ea) )
	ROM_LOAD( "3__2516.5f",  0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "1__2516.5j",  0x1800, 0x0800, CRC(0a8c46a0) SHA1(e38e9e3258ab26fcbc6fdf258844e364f4b165ab) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "mb7051.8h",  0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "82s129.4a",  0x0020, 0x0100, CRC(a8202d0d) SHA1(2a615211c33f3ef75af14e4bbedd2a700100be29) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "mb7052.1k",  0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129.3k",  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END

/*  Second set...

    m27256.bin   [2/4]      6_db.u4      [2/4]      87.792969%
    m27256.bin   [4/4]      6_db.u4      [4/4]      87.500000%
    m27256.bin   [3/4]      6_db.u4      [3/4]      67.150879%
    m27256.bin   [1/4]      6_db.u4      [1/4]      16.503906%
*/
ROM_START( mspactwina )
	ROM_REGION( 0x10000, "maincpu", 0 )  // 64k for encrypted code
	ROM_LOAD( "6_db.u4",  0x0000, 0x4000, CRC(a0fb55ba) SHA1(ad591aa6511600f4687b7c4e70882d87386c9fb9) )
	ROM_CONTINUE(0x8000,0x4000)

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "1.5e",  0x0000, 0x0800, CRC(483c1d1c) SHA1(d3b967c6a71cf02b825d800f56d5268f2e0e60eb) )
	ROM_LOAD( "3.5h",  0x0800, 0x0800, CRC(703912f5) SHA1(03f5d7b30bacabf388fdcfa13fe6a5b0e3027fe7) )  // this ROM has additional tiles
	ROM_LOAD( "2.5f",  0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "4.5j",  0x1800, 0x0800, CRC(0a8c46a0) SHA1(e38e9e3258ab26fcbc6fdf258844e364f4b165ab) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "mb7051.8h",  0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "82s129.4a",  0x0020, 0x0100, CRC(a8202d0d) SHA1(2a615211c33f3ef75af14e4bbedd2a700100be29) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "mb7052.1k",  0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129.3k",  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( hangly3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hm1.6e",   0x0000, 0x0800, CRC(9d027c4a) SHA1(88e094880057451a75cdc2ce9477403021813982) )
	ROM_LOAD( "hm5.6k",   0x0800, 0x0800, CRC(194c7189) SHA1(fd423bac2810015313841c7b935054565390fbd0) )
	ROM_LOAD( "hangly2.6f",   0x1000, 0x0800, CRC(5ba228bb) SHA1(b0e902cdf98bee72d6ec8069eec96adce3245074) ) // hm2.6f
	ROM_LOAD( "hangly2.6m",   0x1800, 0x0800, CRC(baf5461e) SHA1(754586a6449fd54a342f260e572c1cd60ab70815) ) // hm6.6m
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

// PCB marked KPM-II
ROM_START( baracuda )
	ROM_REGION( 0x10000, "maincpu", 0 ) // all 2732
	ROM_LOAD( "bcuda_prg1.bin", 0x0000, 0x1000, CRC(5fe8610a) SHA1(d63eaebd85e10aa6c27bb7f47642dd403eeb6934) )
	ROM_LOAD( "bcuda_prg2.bin", 0x1000, 0x1000, CRC(61d38c6c) SHA1(1406aacdc9c8a3776e5853d214380ad3124408f4) )
	ROM_LOAD( "bcuda_prg3.bin", 0x2000, 0x1000, CRC(4e7ef99f) SHA1(bd42e68b29b4d654dc817782ba00db69b7d2dfe2) )
	ROM_LOAD( "bcuda_prg4.bin", 0x3000, 0x1000, CRC(55e86c2b) SHA1(ddbd98a585e38abda868c2ebc4494231aef00382) )

	ROM_REGION( 0x2000, "gfx1", 0 ) // all 2716
	ROM_LOAD( "bcuda_gfx1.bin", 0x0000, 0x0800, CRC(3fc4030c) SHA1(5e45f0c19cf96daa17afd2fa1c628d7ac7f4a79c) )
	ROM_LOAD( "bcuda_gfx2.bin", 0x1000, 0x0800, CRC(f3e9c9d5) SHA1(709a75b2457f21f0f1a3d9e7f4c8579468ee5cad) )
	ROM_LOAD( "bcuda_gfx3.bin", 0x0800, 0x0800, CRC(ea7fba5e) SHA1(3e9fde897037309e8dedda95e7bbdd1f0885f3ca) )
	ROM_LOAD( "bcuda_gfx4.bin", 0x1800, 0x0800, CRC(133d720d) SHA1(8af75ed9e115a996379acedd44d0c09332ec5a03) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a", 0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

/* Bootleg from Marti Colls (Falgas) of Crock-Man.
   It's mainly a hack to remove Rene Pierre logo, but it was on an original Marti Colls bootleg hardware */
ROM_START( crockmnf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crockmc_01.bin", 0x0000, 0x0800, CRC(2c0fa0ab) SHA1(37680e4502771ae69d51d07ce43f65b9b2dd2a49) )
	ROM_LOAD( "crockmc_05.bin", 0x0800, 0x0800, CRC(afeca2f1) SHA1(1e6d6c75eeb3a354ce2dc88da62caf9e7d53d0cb) )
	ROM_LOAD( "crockmc_02.bin", 0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "crockmc_06.bin", 0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "crockmc_03.bin", 0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "crockmc_07.bin", 0x2800, 0x0800, CRC(93f344c5) SHA1(987c7fa18a774a47c045fa1dc7dff37457cb8983) )
	ROM_LOAD( "crockmc_04.bin", 0x3000, 0x0800, CRC(bed4a077) SHA1(39ac1d4d2acf4752ff7f9839f8f0d1974e023fab) )
	ROM_LOAD( "crockmc_08.bin", 0x3800, 0x0800, CRC(800be41e) SHA1(6f40e741d95c2cfe1b217f1061da3497b4c2a153) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "crockmc_09.bin", 0x0000, 0x0800, CRC(581d0c11) SHA1(82f39459bab2547d7bb70fd08e74da5c23590dfc) )
	ROM_LOAD( "crockmc_11.bin", 0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "crockmc_10.bin", 0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "crockmc_12.bin", 0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	// Undumped on the Marti Colls PCB, taken from the parent set
	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, BAD_DUMP CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a", 0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	// Undumped on the Marti Colls PCB, taken from the parent set
	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( puckmana ) // Alca bootleg
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.6e",        0x0000, 0x0800, CRC(2c0fa0ab) SHA1(37680e4502771ae69d51d07ce43f65b9b2dd2a49) )
	ROM_LOAD( "5.6k",        0x0800, 0x0800, CRC(afeca2f1) SHA1(1e6d6c75eeb3a354ce2dc88da62caf9e7d53d0cb) )
	ROM_LOAD( "2.6f",        0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "6.6m",        0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "3.6h",        0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "7.6n",        0x2800, 0x0800, CRC(93f344c5) SHA1(987c7fa18a774a47c045fa1dc7dff37457cb8983) )
	ROM_LOAD( "4.6j",        0x3000, 0x0800, CRC(bed4a077) SHA1(39ac1d4d2acf4752ff7f9839f8f0d1974e023fab) )
	ROM_LOAD( "8.6p",        0x3800, 0x0800, CRC(800be41e) SHA1(6f40e741d95c2cfe1b217f1061da3497b4c2a153) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "9a.5e",       0x0000, 0x0800, CRC(35de2118) SHA1(9db4376727cf381e3613f1e09bb1996ad7c11ee0) )
	ROM_LOAD( "11.5h",       0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "10.5f",       0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "12.5j",       0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "mb7051.7f",  0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "6301.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "6301.1m",      0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "6301.3m",      0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
ROM_END

ROM_START( pacmanmr ) // PCB is marked "PAC/M" on component side
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pacm.7f",   0x0000, 0x0800, CRC(2c0fa0ab) SHA1(37680e4502771ae69d51d07ce43f65b9b2dd2a49) )
	ROM_LOAD( "pacm.7l",   0x0800, 0x0800, CRC(afeca2f1) SHA1(1e6d6c75eeb3a354ce2dc88da62caf9e7d53d0cb) )
	ROM_LOAD( "pacm.7h",   0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "pacm.7m",   0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "pacm.7j",   0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "pacm.7n-1", 0x2800, 0x0800, CRC(93f344c5) SHA1(987c7fa18a774a47c045fa1dc7dff37457cb8983) )
	ROM_LOAD( "pacm.7k",   0x3000, 0x0800, CRC(cd9f1fa7) SHA1(d8c5bbd488ad193bdb32e745d9e8164c45f33bce) )
	ROM_LOAD( "pacm.7p",   0x3800, 0x0800, CRC(124d8ddf) SHA1(0a661bab79482c35c3fa4edb3b9376f49c3f3c87) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacm-0.5e", 0x0000, 0x0800, CRC(3ed40275) SHA1(bf56f9a21bc1dacee438d88eb5bc59f20906b00c) )
	ROM_LOAD( "pacm-1.5h", 0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pacm-0.5f", 0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "pacm-1.5j", 0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pacm.8h", 0x0000, 0x0020, CRC(2c3cc909) SHA1(32d68d4cfdf9f3e7351353428d268c763e809c63) )
	ROM_LOAD( "pacm.4a", 0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "pacm.1m", 0x0000, 0x0100, CRC(3cb61034) SHA1(2f24b88839aee107a0ac1064f8bc4853933f5205) )
	ROM_LOAD( "pacm.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
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
	ROM_LOAD( "pr1633.78",    0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) // Color palette
	ROM_LOAD( "pr1634.88",    0x0020, 0x0400, CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) // Color lookup

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
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
	ROM_LOAD( "pr1633.78",      0x0000, 0x0020, CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) // Color palette
	ROM_LOAD( "pengopac.4a",    0x0020, 0x0100, CRC(ef283be2) SHA1(6d616348c06d08f3ffbe875a40036a2453cb45ad) ) // Color lookup

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
ROM_END

// Marti Colls bootleg (set 1)
ROM_START( pengomc1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pengomc_01.bin", 0x4000, 0x0200, CRC(ea415ede) SHA1(da005876d96361c5b807a3cf1b82ec066c678c87) )
	ROM_CONTINUE( 0x0200, 0x0e00 )
	ROM_LOAD( "pengomc_02.bin", 0x0000, 0x0200, CRC(dfe9640e) SHA1(c0366399092cafecba1e407b256a62c0c3b538c0) )
	ROM_CONTINUE( 0x4200, 0x0e00 )
	ROM_LOAD( "pengomc_03.bin", 0x5000, 0x1000, CRC(0ac08ae4) SHA1(29eadd0ef0d1f584785e4e6f6cc9561c7dbfb70c) )
	ROM_LOAD( "pengomc_04.bin", 0x6000, 0x1000, CRC(1717a7d9) SHA1(d550a7644be3712ddbcbf008d45f99c0743976bb) )
	ROM_LOAD( "pengomc_05.bin", 0x7000, 0x1000, CRC(cb6eb19c) SHA1(c66256e0fc7ccdcac997775ae9186aef1e0819d9) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pengomc_09.bin", 0x0000, 0x0800, CRC(ad88978a) SHA1(a568baf751753660223958b722980f031310eba1) ) // same as pengoa.bin on "pengojpm"
	ROM_LOAD( "pengomc_11.bin", 0x0800, 0x0800, CRC(cb208b9f) SHA1(63b64b52c9c3e18b2d2823e79095160fb1a71f00) ) // same as pengoc.bin on "pengojpm"
	ROM_LOAD( "pengomc_10.bin", 0x1000, 0x0800, CRC(bae319a3) SHA1(88f0562ba2501f16ddfaffb12c4d1c00315f4225) ) // same as pengob.bin on "pengojpm"
	ROM_LOAD( "pengomc_12.bin", 0x1800, 0x0800, CRC(5a5190e8) SHA1(caf49a348c649fbf959e97c632832bdb5bc068be) ) // same as pengod.bin on "pengojpm"

	// Undumped on the Marti Colls PCB, taken from "pengojpm" set
	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78", 0x0000, 0x0020, BAD_DUMP CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) // Color palette
	ROM_LOAD( "pr1634.88", 0x0020, 0x0400, BAD_DUMP CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) // Color lookup

	// Undumped on the Marti Colls PCB, taken from "pengojpm" set
	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used
ROM_END

// Marti Colls bootleg (set 2)
ROM_START( pengomc2 ) // identical to pengojpm, but maincpu ROMs content was rearranged probably as some sort of protection
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pengomc2_02.bin", 0x0000, 0x0200, CRC(41dccc7c) SHA1(e018806bdac8a9ca53821f65f53cf5a4e1ddd45c) )
	ROM_CONTINUE( 0x4200, 0x0e00 )
	ROM_LOAD( "pengomc2_01.bin", 0x4000, 0x0200, CRC(4ebc5a8b) SHA1(2208288f68f0fc0fc010760290db2857f420bd16) )
	ROM_CONTINUE( 0x0200, 0x0e00 )
	ROM_LOAD( "pengomc2_03.bin", 0x5000, 0x1000, CRC(1b90c32c) SHA1(1761add93d71d29840b1462b9747a3d463b7148d) ) // same as pengo2.bin on "pengojpm"
	ROM_LOAD( "pengomc2_04.bin", 0x6000, 0x1000, CRC(aff4fba1) SHA1(8083352b3a2a4a70b2db778074826a55177e06ab) ) // same as pengo3.bin on "pengojpm"
	ROM_LOAD( "pengomc2_05.bin", 0x7000, 0x1000, CRC(1628eb6d) SHA1(44bd9d30828bb2440599fcd4a46f20fd798c24d5) ) // same as pengo4.bin on "pengojpm"

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pengomc_09.bin", 0x0000, 0x0800, CRC(ad88978a) SHA1(a568baf751753660223958b722980f031310eba1) ) // same as pengoa.bin on "pengojpm"
	ROM_LOAD( "pengomc_11.bin", 0x0800, 0x0800, CRC(cb208b9f) SHA1(63b64b52c9c3e18b2d2823e79095160fb1a71f00) ) // same as pengoc.bin on "pengojpm"
	ROM_LOAD( "pengomc_10.bin", 0x1000, 0x0800, CRC(bae319a3) SHA1(88f0562ba2501f16ddfaffb12c4d1c00315f4225) ) // same as pengob.bin on "pengojpm"
	ROM_LOAD( "pengomc_12.bin", 0x1800, 0x0800, CRC(5a5190e8) SHA1(caf49a348c649fbf959e97c632832bdb5bc068be) ) // same as pengod.bin on "pengojpm"

	// Undumped on the Marti Colls PCB, taken from "pengojpm" set
	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78", 0x0000, 0x0020, BAD_DUMP CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) // Color palette
	ROM_LOAD( "pr1634.88", 0x0020, 0x0400, BAD_DUMP CRC(766b139b) SHA1(3fcd66610fcaee814953a115bf5e04788923181f) ) // Color lookup

	// Undumped on the Marti Colls PCB, taken from "pengojpm" set
	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used
ROM_END

ROM_START( pinguinos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pg0_2732.bin", 0x0000, 0x1000, CRC(1a79436c) SHA1(341a0c86784b794c1206842d03da8111487099b0) )
	ROM_LOAD( "pg4_2732.bin", 0x4000, 0x1000, CRC(6210c06f) SHA1(995f63402720a337a2754eef6207d5c3331a0123) )
	ROM_LOAD( "pg5_2732.bin", 0x5000, 0x1000, CRC(7c83d678) SHA1(08fec6ba0f75684f1f5eb6d2ce3bf50b5b350a34) )
	ROM_LOAD( "pg6_2732.bin", 0x6000, 0x1000, CRC(ae646d36) SHA1(ac2621383607fa8eb81d44bba507e9fcf90c53bb) )
	ROM_LOAD( "pg7_2732.bin", 0x7000, 0x1000, CRC(1628eb6d) SHA1(44bd9d30828bb2440599fcd4a46f20fd798c24d5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pg9_mb8516.e6",  0x0000, 0x0800, CRC(ad88978a) SHA1(a568baf751753660223958b722980f031310eba1) )
	ROM_LOAD( "pg11.h6",        0x0800, 0x0800, CRC(cee7cb9a) SHA1(718aacdb61e8a82e2fd6c753787b39b6e45e60b5) )
	ROM_LOAD( "pg10_mb8516.f6", 0x1000, 0x0800, CRC(bae319a3) SHA1(88f0562ba2501f16ddfaffb12c4d1c00315f4225) )
	ROM_LOAD( "pg12_2716.j6",   0x1800, 0x0800, CRC(5a5190e8) SHA1(caf49a348c649fbf959e97c632832bdb5bc068be) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "pr1633.78",      0x0000, 0x0020, BAD_DUMP CRC(3a5844ec) SHA1(680eab0e1204c9b74adc11588461651b474021bb) ) // Color palette
	ROM_LOAD( "pengopac.4a",    0x0020, 0x0100, BAD_DUMP CRC(ef283be2) SHA1(6d616348c06d08f3ffbe875a40036a2453cb45ad) ) // Color lookup

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
ROM_END

ROM_START( bucanera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.6e",         0x0000, 0x0800, CRC(2c0fa0ab) SHA1(37680e4502771ae69d51d07ce43f65b9b2dd2a49) )
	ROM_LOAD( "5.6k",         0x0800, 0x0800, CRC(afeca2f1) SHA1(1e6d6c75eeb3a354ce2dc88da62caf9e7d53d0cb) )
	ROM_LOAD( "2.6f",         0x1000, 0x0800, CRC(6b53ada9) SHA1(a905688b389bfbc6792965d8f3d5bb1b9f0f4ec6) )
	ROM_LOAD( "6.6m",         0x1800, 0x0800, CRC(35f3ca84) SHA1(3da7336caa0742ea79f1e0e8f6b80f8560507a33) )
	ROM_LOAD( "3.6h",         0x2000, 0x0800, CRC(9045a44c) SHA1(a97d7016effbd2ace9a7d92ceb04a6ce18fb42f9) )
	ROM_LOAD( "7.6n",         0x2800, 0x0800, CRC(888f3c3e) SHA1(c2b5917bf13071131dd53ea76f0da86706db2d80) )
	ROM_LOAD( "4.6j",         0x3000, 0x0800, CRC(292de161) SHA1(09b439c301d7bedb76c1590e937e9d8d5e24a048) )
	ROM_LOAD( "8.6p",         0x3800, 0x0800, CRC(e037834d) SHA1(98e68e7f8d32dcebfb03cf41802c1b0763d7c1c6) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "9.5e",         0x0000, 0x0800, CRC(f814796f) SHA1(0bf4fefca235d3b079861e78298508ee03e1b029) )
	ROM_LOAD( "11.5h",        0x0800, 0x0800, CRC(e3861283) SHA1(61cf8ed24902910e98438d9e2e2745f226ad2a13) )
	ROM_LOAD( "10.5f",        0x1000, 0x0800, CRC(09f66dec) SHA1(2d3649341fed19bac15ec274f7d747de46a3edb2) )
	ROM_LOAD( "12.5j",        0x1800, 0x0800, CRC(653314e7) SHA1(c466a421917b3502e9115ebda1b2d11f7f586de8) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
ROM_END

// There is a 10 page Service Manual with basic wiring schematics for Titan in German claiming copyright by NSM - Apparatebau GmbH & Co.
// Flyer claims copyright by NSM/Löwen
ROM_START( titanpac ) // GDP-01 main PCB with GDP-02 auxiliary card (same as Piranha)
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "t101.7e", 0x0000, 0x0800, CRC(5538c288) SHA1(12d97dbaa000e85d4601e4d2c5a19821f7a4da09) )
	ROM_LOAD( "t105.7e", 0x0800, 0x0800, CRC(095f5a5f) SHA1(ed2b11f9e9b27fd26349ee66bd866a1768ea2002) )
	ROM_LOAD( "t102.7f", 0x1000, 0x0800, CRC(8117a6a2) SHA1(d9867a301176bd681aac290e9e17d99a2e5c6ae3) )
	ROM_LOAD( "t106.6f", 0x1800, 0x0800, CRC(cb0700bc) SHA1(1f5e91791ea25eb58d26b9627e98e0b6c1d9becf) )
	ROM_LOAD( "t103.7h", 0x2000, 0x0800, CRC(060e514e) SHA1(9ed1bd9430fc7c70a52f14cbd5429887f8c6c11b) )
	ROM_LOAD( "t107.6h", 0x2800, 0x0800, CRC(9209882a) SHA1(db71d68ce9ff059e0d4a2d10cf8882fef637b38b) )
	ROM_LOAD( "t104.7j", 0x3000, 0x0800, CRC(2c8c7299) SHA1(b3c904869842bcf29434b1554b646e2018d9a5a3) )
	ROM_LOAD( "t108.6j", 0x3800, 0x0800, CRC(ff943d70) SHA1(7d080f0645227a2f61b2e5c95cad03ff1347aff3) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "t109.5e", 0x0000, 0x0800, CRC(412e723e) SHA1(ecadc3cc179e0ccb875f693d2f7cbadcca0a3c86) )
	ROM_LOAD( "t111.5h", 0x0800, 0x0800, CRC(87d28931) SHA1(6b10cd76a88b81d96cf1a8a69cb8ee0cc0289a93) )
	ROM_LOAD( "t110.5f", 0x1000, 0x0800, CRC(3be1601b) SHA1(4a4ad4b6794d8e5d0cdd0d48b832a147d5f17eda) )
	ROM_LOAD( "t112.5j", 0x1800, 0x0800, CRC(f773cb8b) SHA1(eae5bf3da8d89452c53e4d73657d63201ebe1e65) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "titan.4a",  0x0020, 0x0100, CRC(b67a0c10) SHA1(3d4f4033f88648d397f780856b59b9d3f25f1a2d) )

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
ROM_END

/* Bootleg from Spanish company "FAMARE S.A.". Board labeled "FAMARESA 560-002"
   It's mainly a hack to remove Namco logo, but it was on an original Famaresa bootleg hardware */
ROM_START( pacmanfm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pacfama_01.bin",  0x0000, 0x0800, CRC(f36e88ab) SHA1(813cecf44bf5464b1aed64b36f5047e4c79ba176) )
	ROM_LOAD( "pacfama_05.bin",  0x0800, 0x0800, CRC(618bd9b3) SHA1(b9ca52b63a49ddece768378d331deebbe34fe177) )
	ROM_LOAD( "pacfama_02.bin",  0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "pacfama_06.bin",  0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "pacfama_03.bin",  0x2000, 0x0800, CRC(6bf4f625) SHA1(afe72fdfec66c145b53ed865f98734686b26e921) )
	ROM_LOAD( "pacfama_07.bin",  0x2800, 0x0800, CRC(a948ce83) SHA1(08759833f7e0690b2ccae573c929e2a48e5bde7f) )
	ROM_LOAD( "pacfama_04.bin",  0x3000, 0x0800, CRC(b6289b26) SHA1(d249fa9cdde774d5fee7258147cd25fa3f4dc2b3) )
	ROM_LOAD( "pacfama_08.bin",  0x3800, 0x0800, CRC(17a88c13) SHA1(eb462de79f49b7aa8adb0cc6d31535b10550c0ce) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pacfama_09.bin",  0x0000, 0x0800, CRC(7a7b48b3) SHA1(a12f08d76f9aee3c1fc8401d0aa087d2187c1803) )
	ROM_LOAD( "pacfama_11.bin",  0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "pacfama_10.bin",  0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "pacfama_12.bin",  0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	// Undumped on the Famaresa PCB, taken from the parent set
	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "pm1-1.7f", 0x0000, 0x0020, BAD_DUMP CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) ) // 82s123
	ROM_LOAD( "pm1-4.4a", 0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) ) // 82s126

	// Undumped on the Famaresa PCB, taken from the parent set
	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "pm1-3.1m", 0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // 82s126
	ROM_LOAD( "pm1-2.3m", 0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // 82s126 - timing - not used
ROM_END

// PCB is marked "PUK" both on component side and on solder side. Code is identical to pacmanfm, only differences are in p9bfz.5e (adds U.G.) and in mmi63s141j.3m
ROM_START( pacmanug )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1p.6e",  0x0000, 0x0800, CRC(f36e88ab) SHA1(813cecf44bf5464b1aed64b36f5047e4c79ba176) )
	ROM_LOAD( "5p.6k",  0x0800, 0x0800, CRC(618bd9b3) SHA1(b9ca52b63a49ddece768378d331deebbe34fe177) )
	ROM_LOAD( "2p.6f",  0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "6p.6m",  0x1800, 0x0800, CRC(d3e8914c) SHA1(c2f00e1773c6864435f29c8b7f44f2ef85d227d3) )
	ROM_LOAD( "3p.6h",  0x2000, 0x0800, CRC(6bf4f625) SHA1(afe72fdfec66c145b53ed865f98734686b26e921) )
	ROM_LOAD( "7p.6n",  0x2800, 0x0800, CRC(a948ce83) SHA1(08759833f7e0690b2ccae573c929e2a48e5bde7f) )
	ROM_LOAD( "4p.6j",  0x3000, 0x0800, CRC(b6289b26) SHA1(d249fa9cdde774d5fee7258147cd25fa3f4dc2b3) )
	ROM_LOAD( "8p.6p",  0x3800, 0x0800, CRC(17a88c13) SHA1(eb462de79f49b7aa8adb0cc6d31535b10550c0ce) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "p9bfz.5e", 0x0000, 0x0800, CRC(dc9f2a7b) SHA1(511a620abddaf40c8a578afaaddb2d832a74797b) )
	ROM_LOAD( "11p.5h",   0x0800, 0x0800, CRC(3591b89d) SHA1(79bb456be6c39c1ccd7d077fbe181523131fb300) )
	ROM_LOAD( "10p.5f",   0x1000, 0x0800, CRC(9e39323a) SHA1(be933e691df4dbe7d12123913c3b7b7b585b7a35) )
	ROM_LOAD( "12p.5j",   0x1800, 0x0800, CRC(1b1d9096) SHA1(53771c573051db43e7185b1d188533056290a620) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "sig82s123.7f",  0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "mmi6301-1j.4a", 0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "mmi6301-1j.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "mmi63s141j.3m", 0x0100, 0x0100, CRC(deadc015) SHA1(751029630dcfef61dc834203eaffaf6afc7d83fc) )
ROM_END

ROM_START( piranha ) // GDP-01 main PCB with GDP-02 auxiliary card
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "pir1.7e", 0x0000, 0x0800, CRC(69a3e6ea) SHA1(c54e5d039a03d3cbee7a5e21bf1e23f4fd913ea6) )
	ROM_LOAD( "pir5.6e", 0x0800, 0x0800, CRC(245e753f) SHA1(4c1183b8449e4e7995f81079953fe0e251251c60) )
	ROM_LOAD( "pir2.7f", 0x1000, 0x0800, CRC(62cb6954) SHA1(0e01c8463b130ab5518ce23368ad028c86cd0a32) )
	ROM_LOAD( "pir6.6f", 0x1800, 0x0800, CRC(cb0700bc) SHA1(1f5e91791ea25eb58d26b9627e98e0b6c1d9becf) )
	ROM_LOAD( "pir3.7h", 0x2000, 0x0800, CRC(843fbfe5) SHA1(6671a3c55ef70447f2a127438e0c39857f8bf6b1) )
	ROM_LOAD( "pir7.6h", 0x2800, 0x0800, CRC(73084d5e) SHA1(cb04a4c9dbf1672ddf478d2fe92b0ffd0159bb9e) )
	ROM_LOAD( "pir4.7j", 0x3000, 0x0800, CRC(4cdf6704) SHA1(97af8bbd08896dffd73e359ec46843dd673c4c9c) )
	ROM_LOAD( "pir8.6j", 0x3800, 0x0800, CRC(b86fedb3) SHA1(f5eaf7ccc1ecaa2417bcc077561efca8e7cb691a) )

	ROM_REGION( 0x2000, "gfx1" , 0)
	ROM_LOAD( "pir9.5e",  0x0000, 0x0800, CRC(0f19eb28) SHA1(0335189a06be01b97ca376d3682ed54df9b121e8) )
	ROM_LOAD( "pir11.5h", 0x0800, 0x0800, CRC(5f8bdabe) SHA1(eb6a0515a381a885b087d165aaefb0277a223715) )
	ROM_LOAD( "pir10.5f", 0x1000, 0x0800, CRC(d19399fb) SHA1(c0a75a08f77adb9d0010511c4b6ea99324c33c50) )
	ROM_LOAD( "pir12.5j", 0x1800, 0x0800, CRC(cfb4403d) SHA1(1642a4917be0621ebf5f705c7f68a2b75d1c78d3) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",  0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "piranha.4a", 0x0020, 0x0100, CRC(08c9447b) SHA1(5e4fbfcc7179fc4b1436af9bb709ffc381479315) )

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
ROM_END

ROM_START( piranhao ) // GDP-01 main PCB with GDP-02 auxiliary card
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "p1.7e",   0x0000, 0x0800, CRC(c6ce1bfc) SHA1(da145d67331cee292654a185fb09e773dd9d40cd) )
	ROM_LOAD( "p5.6e",   0x0800, 0x0800, CRC(a2655a33) SHA1(2253dcf5c8cbe278118aa1569cf456b13d8cf029) )
	ROM_LOAD( "pir2.7f", 0x1000, 0x0800, CRC(62cb6954) SHA1(0e01c8463b130ab5518ce23368ad028c86cd0a32) )
	ROM_LOAD( "pir6.6f", 0x1800, 0x0800, CRC(cb0700bc) SHA1(1f5e91791ea25eb58d26b9627e98e0b6c1d9becf) )
	ROM_LOAD( "pir3.7h", 0x2000, 0x0800, CRC(843fbfe5) SHA1(6671a3c55ef70447f2a127438e0c39857f8bf6b1) )
	ROM_LOAD( "pir7.6h", 0x2800, 0x0800, CRC(73084d5e) SHA1(cb04a4c9dbf1672ddf478d2fe92b0ffd0159bb9e) )
	ROM_LOAD( "p4.7j",   0x3000, 0x0800, CRC(9363a4d1) SHA1(4cb4a86d92a1f9bf233cac01aa266485a8bb7a34) )
	ROM_LOAD( "p8.6j",   0x3800, 0x0800, CRC(2769979c) SHA1(581592da26199b325de51791ddab66b474ab0413) )

	ROM_REGION( 0x2000, "gfx1" , 0 )
	ROM_LOAD( "p9.5e",  0x0000, 0x0800, CRC(94eb7563) SHA1(c99741ce1aebdfb89628fbfaecf5ae6b2719a0ca) )
	ROM_LOAD( "p11.5h", 0x0800, 0x0800, CRC(a3606973) SHA1(72297e1a33102c6a48b4c65f2a0b9bfc75a2df36) )
	ROM_LOAD( "p10.5f", 0x1000, 0x0800, CRC(84165a2c) SHA1(95b24620fbf9bd0ec4dd2aeeb6d9305bd475dce2) )
	ROM_LOAD( "p12.5j", 0x1800, 0x0800, CRC(2699ba9e) SHA1(b91ff586defe65b200bea5ade7374c2c7579cd80) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",  0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "piranha.4a", 0x0020, 0x0100, CRC(08c9447b) SHA1(5e4fbfcc7179fc4b1436af9bb709ffc381479315) )

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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
	ROM_LOAD( "as9.bin",  0x1000, 0x0800, CRC(a3bd1613) SHA1(c59bb0a4d1fa5cbe596f41ee7b1a4a661ab5614b) )
	ROM_LOAD( "as11.bin", 0x1800, 0x0800, CRC(9d802b68) SHA1(4e8f37c2faedcfce91221a34c14f6490d578c80a) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "as4a.bin",  0x0020, 0x0100, CRC(1605b324) SHA1(336fce22caedbe69bcba9cea2b43e00f6f8e8067) )

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
ROM_END

ROM_START( mspacmab3 ) // main PCB with GDP-02 auxiliary card
	ROM_REGION( 0x10000, "maincpu",0 )
	ROM_LOAD( "p1.6ef",  0x0000, 0x1000, CRC(50b38941) SHA1(c798fe6efe7ec6d15add74986d72d9ea8676d14a) ) // could also be d instead of p
	ROM_LOAD( "p2.7ef",  0x1000, 0x1000, CRC(195883b8) SHA1(3395fd2f9963b1809fc55a4f1c998673de75793b) ) // could also be d instead of p
	ROM_LOAD( "p3.bin",  0x2000, 0x1000, CRC(124a4507) SHA1(a7c82970ac53129c7a642322214adee4206298ff) ) // could also be d instead of p
	ROM_LOAD( "p4.bin",  0x3000, 0x1000, CRC(08ac65da) SHA1(745d9d054c33df96a7f27a1f4575f8770d92ac10) ) // could also be d instead of p
	ROM_LOAD( "d5.6fh",  0x8000, 0x1000, CRC(50b29f09) SHA1(27ca2cdf57b96d628b1811210d254b107e2f324e) )
	ROM_LOAD( "d6.6j",   0x9000, 0x0800, CRC(33b09ed9) SHA1(a2a3e069cce442c3b179315e90476cd431c604c2) )

	ROM_REGION( 0x2000, "gfx1" , 0 )
	ROM_LOAD( "d7.5de", 0x0000, 0x0800, CRC(b5d8c872) SHA1(d137b0cff8635a6a02f9334b7fa72dc1a623fe9b) )
	ROM_LOAD( "d9.5fh", 0x0800, 0x0800, CRC(9b2b936c) SHA1(d6f57d0ce6fba37d4838cdcace7e2e02c94b1ba1) )
	ROM_LOAD( "d8.5ef", 0x1000, 0x0800, CRC(a70a6ac4) SHA1(81b0c56697bb671cc43928133446da74e74b4982) )
	ROM_LOAD( "d10.5j", 0x1800, 0x0800, CRC(53368498) SHA1(0409288fe59c2bbf2730c31e4c7817392a544165) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "6331.8h", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "6301.4a", 0x0020, 0x0100, CRC(720528b4) SHA1(1f8fee1b8dec02cb19e706ca807438cec512479c) ) // 11xxxxxxx = 0x00
	ROM_IGNORE(                  0x0100 )

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs
	ROM_LOAD( "63s141.1k", 0x0000, 0x0100, CRC(459d2618) SHA1(14ba61caada575909b4dbd57e7342dc84722325d) )
	ROM_IGNORE(                    0x0100 ) // 1xxxxxxxx = 0x00
	ROM_LOAD( "63s141.3k", 0x0100, 0x0100, CRC(fcc24d5d) SHA1(7ae2523f92cccdbd8db8bda80c613a2f90220807) )  // timing - not used
	ROM_IGNORE(                    0x0100 ) // 11xxxxxxx = 0x00

	ROM_REGION( 0x0200, "gdp02_prom", 0 ) // currently not used by the emulation
	ROM_LOAD( "82s141.i14", 0x0000, 0x0200, CRC(8d43d0a6) SHA1(be4e0d86ce4c6f1833cc0efc18277b4e04dc458f) ) // 1ST AND 2ND HALF IDENTICAL
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

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( mspacman )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 64k for code+64k for decrypted code
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( mspacmancr )  // Bootleg on Crush Roller Board - Midway Graphics and Namco mentions are gone but Easter Egg still works
	ROM_REGION( 0x20000, "maincpu", 0 ) // 64k for code+64k for decrypted code
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // 7052.1m
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // 7052.3m   // Timing - not used
ROM_END


ROM_START( mspacmnf )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 64k for code+64k for decrypted code
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacfast.6f",   0x1000, 0x1000, CRC(720dc3ee) SHA1(7224d7acfa0144b681c71d7734a7337189835361) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6",           0x9000, 0x1000, CRC(a90e7000) SHA1(e4df96f1db753533f7d770aa62ae1973349ea4cf) )
	ROM_LOAD( "u7",           0xb000, 0x1000, CRC(c82cd714) SHA1(1d8ac7ad03db2dc4c8c18ade466e12032673f874) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) ) // sldh w/msheartb

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( mspacmab2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2764_a.a",       0x0000, 0x2000, CRC(39ae7b16) SHA1(8ed2a01877d7b32894bd932f6a292ef21a2876ab) )
	ROM_LOAD( "2764_b.b",       0x2000, 0x2000, CRC(09d86ef8) SHA1(c373d34250f29778be45122db666c4522e44e3a6) )
	ROM_LOAD( "2764_c.c",       0x8000, 0x2000, CRC(9921d46f) SHA1(8ed2bfdde3d1d1558532e2b9411535a5b5ff37cd) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "2732_e.5e",      0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "2732_f.5f",      0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "tbp18s030_p1.7f", 0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "pac.4a",          0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "63s141.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used

	ROM_REGION( 0x0100, "plds", 0 )
	ROM_LOAD( "82s153.d",     0x0000, 0x00eb, CRC(0294d8bc) SHA1(7b66d39c464ee2a3f7a659bf066d2ebb487605fd) )
ROM_END


ROM_START( mspacmab4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sub1.bin", 0x0000, 0x2000, CRC(3ed9d3ca) SHA1(2091224bbe0bab79df9e26e97581df6784171d81) )
	ROM_LOAD( "sub2.bin", 0x2000, 0x2000, CRC(988db4af) SHA1(9258e199a96f811f281d196ede369c144834c589) )
	ROM_LOAD( "sub3.bin", 0x8000, 0x2000, CRC(9921d46f) SHA1(8ed2bfdde3d1d1558532e2b9411535a5b5ff37cd) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e", 0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f", 0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x000, 0x020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a", 0x020, 0x100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x200, "namco", 0 ) // sound PROMs
	ROM_LOAD( "82s126.1m", 0x000, 0x100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x100, 0x100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // timing - not used

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "subpal16l8d.bin", 0x000, 0x104, CRC(5f852ffa) SHA1(244a28261b4004264cc20851062c7f3e659862c9) )
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

// Marti Colls (Falgas) bootleg
ROM_START( mspacmbmc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "misspacmanfalgas_0.bin", 0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "misspacmanfalgas_1.bin", 0x1000, 0x1000, CRC(0d32de5e) SHA1(13ea0c343de072508908be885e6a2a217bbb3047) )
	ROM_LOAD( "misspacmanfalgas_2.bin", 0x2000, 0x1000, CRC(1821ee0b) SHA1(5ea4d907dbb2690698db72c4e0b5be4d3e9a7786) )
	ROM_LOAD( "misspacmanfalgas_3.bin", 0x3000, 0x1000, CRC(e086219d) SHA1(6802cf425be878f442716d792946a85fc13e7413) )
	ROM_LOAD( "misspacmanfalgas_4.bin", 0x8000, 0x1000, CRC(8c3e6de6) SHA1(fed6e9a2b210b07e7189a18574f6b8c4ec5bb49b) )
	ROM_LOAD( "misspacmanfalgas_5.bin", 0x9000, 0x1000, CRC(206a9623) SHA1(20006f945c1b7b0e3c0415eecc0b148e5a6a1dfa) )

	// Undumped on the Marti Colls PCB, taken from the parent set
	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e", 0x0000, 0x1000, BAD_DUMP CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f", 0x1000, 0x1000, BAD_DUMP CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	// Undumped on the Marti Colls PCB, taken from the parent set
	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f", 0x0000, 0x0020, BAD_DUMP CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a", 0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	// Undumped on the Marti Colls PCB, taken from the parent set
	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( mspacmat )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 64k for code+64k for decrypted code
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6pacatk",     0x9000, 0x1000, CRC(f6d83f4d) SHA1(6135b187d6b968554d08f2ac00d3a3313efb8638) )
	ROM_LOAD( "u7",           0xb000, 0x1000, CRC(c82cd714) SHA1(1d8ac7ad03db2dc4c8c18ade466e12032673f874) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) ) // sldh w/msheartb
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


// a graphics-only hack for Ms. Pac-Man that was sold as romkit by Two-Bit Score back in 1989
ROM_START( msheartb )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 64k for code+64k for decrypted code
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

// This is a bootleg Ms. Pac-Man auxiliary board that displays "Miss Packman Plus" at the title screen and
// contains alternate mazes. These are the mazes that were later hacked into the mspacpls romset.
//
// The auxiliary board contains a Z80, a PAL with "MTS" handwritten on it, and 3 2764 eproms labelled "E",
// "F", and "H". The eproms have data bits 3 and 4 swapped, and the PAL scrambles the addressing in 0x0800
// chunks.
ROM_START( mspackpls )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom-h.bin",    0x0000, 0x0800, CRC(88c89824) SHA1(8fa49483864c79c50b4734896fc7bd3cc6531afb) )
	ROM_CONTINUE(             0x8800, 0x0800 )
	ROM_CONTINUE(             0x3000, 0x0800 )
	ROM_CONTINUE(             0x9800, 0x0800 )
	ROM_LOAD( "rom-f.bin",    0x2000, 0x0800, CRC(19620d5d) SHA1(3ab7dbdd6d72dd395b1d37b27e70f9a106e87971) )
	ROM_CONTINUE(             0x0800, 0x0800 )
	ROM_CONTINUE(             0x9000, 0x0800 )
	ROM_CONTINUE(             0x1800, 0x0800 )
	ROM_LOAD( "rom-e.bin",    0x8000, 0x0800, CRC(59cb7ea0) SHA1(bf6f84ea7c08be88ad14e13a6b45ae788804d850) )
	ROM_CONTINUE(             0x2800, 0x0800 )
	ROM_CONTINUE(             0x1000, 0x0800 )
	ROM_CONTINUE(             0x3800, 0x0800 )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( mspacmanlai )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1-cpu.6e", 0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "2-cpu.6f", 0x1000, 0x1000, CRC(0d32de5e) SHA1(13ea0c343de072508908be885e6a2a217bbb3047) )
	ROM_LOAD( "3-cpu.6h", 0x2000, 0x1000, CRC(1821ee0b) SHA1(5ea4d907dbb2690698db72c4e0b5be4d3e9a7786) )
	ROM_LOAD( "4-cpu.6j", 0x3000, 0x1000, CRC(e30f2dae) SHA1(74f696f05771fb77e7afb45575ce107ead2c7780) )
	ROM_LOAD( "5-cpu.6k", 0x8000, 0x1000, CRC(8c3e6de6) SHA1(fed6e9a2b210b07e7189a18574f6b8c4ec5bb49b) )
	ROM_LOAD( "6-cpu.6m", 0x9000, 0x0800, CRC(286041cf) SHA1(5a5fc97ea66a59895b3403b2982940b755076667) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "4-vid.5e", 0x0000, 0x0800, CRC(93933d1d) SHA1(fa38d2cb87e872bb9a3158a4df98f38360dc85ec) )
	ROM_LOAD( "2-vid.5h", 0x0800, 0x0800, CRC(ec7caeba) SHA1(51b95bbd52028b91b5da947cb053231b2729983a) )
	ROM_LOAD( "3-vid.5f", 0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "1-vid.5j", 0x1800, 0x0800, CRC(50c7477d) SHA1(c04ec282a8cb528df5e38ad750d12ee71612695d) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123-cpu.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129-vid.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s129-vid.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129-vid.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( mspacmane )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7f", 0x0000, 0x1000, CRC(d16b31b7) SHA1(bc2247ec946b639dd1f00bfc603fa157d0baaa97) )
	ROM_LOAD( "7h", 0x1000, 0x1000, CRC(0d32de5e) SHA1(13ea0c343de072508908be885e6a2a217bbb3047) )
	ROM_LOAD( "7j", 0x2000, 0x1000, CRC(1821ee0b) SHA1(5ea4d907dbb2690698db72c4e0b5be4d3e9a7786) )
	ROM_LOAD( "7k", 0x3000, 0x1000, CRC(629c4399) SHA1(cd469a5ab04d4f21237bb5c04e469f250f94e9a2) )
	ROM_LOAD( "7m", 0x8000, 0x1000, CRC(8c3e6de6) SHA1(fed6e9a2b210b07e7189a18574f6b8c4ec5bb49b) )
	ROM_LOAD( "7n", 0x9000, 0x0800, CRC(286041cf) SHA1(5a5fc97ea66a59895b3403b2982940b755076667) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e", 0x0000, 0x0800, BAD_DUMP CRC(93933d1d) SHA1(fa38d2cb87e872bb9a3158a4df98f38360dc85ec) ) // broken ROM, couldn't be read
	ROM_LOAD( "5h", 0x0800, 0x0800, CRC(7409fbec) SHA1(f440f08ba026ae6172666e1bdc0894ce33bba420) )
	ROM_LOAD( "5f", 0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "5l", 0x1800, 0x0800, CRC(50c7477d) SHA1(c04ec282a8cb528df5e38ad750d12ee71612695d) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "8h",    0x0000, 0x0020, CRC(2c3cc909) SHA1(32d68d4cfdf9f3e7351353428d268c763e809c63) )
	ROM_LOAD( "4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "1m",    0x0000, 0x0100, CRC(0922b031) SHA1(1eb9e1f8e6b027ca80a0ee0b391d4e904e9ea49b) )
	ROM_LOAD( "3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( mspacmane2 ) // G-GA-2 + G-GB-2 PCBs
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mpe1.6e", 0x0000, 0x1000, CRC(3ad0ae2f) SHA1(8431a2f4a89935a9f9898bef48f92f456d316aeb) )
	ROM_LOAD( "mpe2.6f", 0x1000, 0x1000, CRC(0d32de5e) SHA1(13ea0c343de072508908be885e6a2a217bbb3047) )
	ROM_LOAD( "mpe3.6h", 0x2000, 0x1000, CRC(1821ee0b) SHA1(5ea4d907dbb2690698db72c4e0b5be4d3e9a7786) )
	ROM_LOAD( "mpe4.6j", 0x3000, 0x1000, CRC(d5e5d2aa) SHA1(ffb0d701d6143e9a7c431e13bbe15db6b51eb49c) )
	ROM_LOAD( "mpe5.6l", 0x8000, 0x1000, CRC(8c3e6de6) SHA1(fed6e9a2b210b07e7189a18574f6b8c4ec5bb49b) )
	ROM_LOAD( "mpe6.6m", 0x9000, 0x1000, CRC(375f0693) SHA1(8ad53c966289e9cd402c7df6b3c04ec07aa89717) ) // 1xxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "mpe7.5e",  0x0000, 0x0800, CRC(93933d1d) SHA1(fa38d2cb87e872bb9a3158a4df98f38360dc85ec) )
	ROM_LOAD( "mpe9.5h",  0x0800, 0x0800, BAD_DUMP CRC(7409fbec) SHA1(f440f08ba026ae6172666e1bdc0894ce33bba420) ) // dump was bitrot, taken from other sets with same other ROMs
	ROM_LOAD( "mpe8.5f",  0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "mpe10.5j", 0x1800, 0x0800, CRC(50c7477d) SHA1(c04ec282a8cb528df5e38ad750d12ee71612695d) )

	ROM_REGION( 0x0120, "proms", 0 ) // not provided for this set
	ROM_LOAD( "8h", 0x0000, 0x0020, BAD_DUMP CRC(2c3cc909) SHA1(32d68d4cfdf9f3e7351353428d268c763e809c63) )
	ROM_LOAD( "4a", 0x0020, 0x0100, BAD_DUMP CRC(4c8e83a4) SHA1(e522cbc6c14bc481f2e97f1a7224c66bb283f553) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs, not provided for this set
	ROM_LOAD( "1m", 0x0000, 0x0100, BAD_DUMP CRC(7b1f9b71) SHA1(5ef72bbdfb72db3eb1175fed652a761938eeb6cd) )
	ROM_LOAD( "3m", 0x0100, 0x0100, BAD_DUMP CRC(05197026) SHA1(9b71fb175331bbc12e43441ecfad75b633e2f953) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( pacgal2 )
	ROM_REGION( 0x20000, "maincpu", 0 ) // 64k for code+64k for decrypted code
	ROM_LOAD( "pmg1.bin",    0x0000, 0x0800, CRC(8167fffc) SHA1(adc26e8baaf9b12c382b5606f2438f7ee6d2f7f5) )
	ROM_LOAD( "pmg5.bin",    0x0800, 0x0800, CRC(618bd9b3) SHA1(b9ca52b63a49ddece768378d331deebbe34fe177) )
	ROM_LOAD( "pmg2.bin",    0x1000, 0x0800, CRC(7d177853) SHA1(9b5ddaaa8b564654f97af193dbcc29f81f230a25) )
	ROM_LOAD( "pmg6.bin",    0x1800, 0x0800, CRC(4f91d245) SHA1(496f3d913a0abf6fa441615aedf60180e71866e8) )
	ROM_LOAD( "pmg3.bin",    0x2000, 0x0800, CRC(d0f20748) SHA1(7673d9744903f58d70ecc0ee817b57c3ce09dd80) )
	ROM_LOAD( "pmg7.bin",    0x2800, 0x0800, CRC(a948ce83) SHA1(08759833f7e0690b2ccae573c929e2a48e5bde7f) )
	ROM_LOAD( "pmg4.bin",    0x3000, 0x0800, CRC(4c842da6) SHA1(dbc84516e44412f863ffb678347929043fab852e) )
	ROM_LOAD( "pmg8.bin",    0x3800, 0x0800, CRC(022764dc) SHA1(0ab0da0834d88f3b1808755aec5c38a3ddd8184b) )
	ROM_LOAD( "u5.bin",      0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6a.bin",     0x9000, 0x1000, CRC(3fdcb271) SHA1(4d604b98c6981a469041c0038345dbd5dcf4cd1d) )
	ROM_LOAD( "u7a.bin",     0xb000, 0x1000, CRC(5fafec7c) SHA1(1c6a85e5c348a69b8d51bcea4f8bdebb24825770) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pmg9.bin",   0x0000, 0x0800, CRC(93933d1d) SHA1(fa38d2cb87e872bb9a3158a4df98f38360dc85ec) )
	ROM_LOAD( "pmg11.bin",  0x0800, 0x0800, CRC(7409fbec) SHA1(f440f08ba026ae6172666e1bdc0894ce33bba420) )
	ROM_LOAD( "pmg10.bin",  0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "pmg12.bin",  0x1800, 0x0800, CRC(50c7477d) SHA1(c04ec282a8cb528df5e38ad750d12ee71612695d) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )    // 7611p4.4a

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( mspacmbn ) // Main PCB H-P1, sub PCB with main CPU ROMs marked Novatronic and 1982
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1_sub.bin",       0x2000, 0x0200, CRC(0839a86e) SHA1(b9f3e477bd2e7ffd98156d5fa14f5ea12468811f) )
	ROM_CONTINUE(                0x0200, 0x0200 )
	ROM_CONTINUE(                0x2400, 0x0200 )
	ROM_CONTINUE(                0x0600, 0x0a00 )
	ROM_LOAD( "2_sub.bin",       0x3000, 0x0200, CRC(f40b1ce9) SHA1(0a00ef0873b60cbc8dcf5aac88fee4023f6f4087) )
	ROM_CONTINUE(                0x1200, 0x0200 )
	ROM_CONTINUE(                0x3400, 0x0200 )
	ROM_CONTINUE(                0x1600, 0x0a00 )
	ROM_LOAD( "3_sub.bin",       0x0000, 0x0200, CRC(40257a62) SHA1(89da3a47345902d58695fabd456099c1b1661dc4) )
	ROM_CONTINUE(                0x2200, 0x0200 )
	ROM_CONTINUE(                0x0400, 0x0200 )
	ROM_CONTINUE(                0x2600, 0x0a00 )
	ROM_LOAD( "4_sub.bin",       0x1000, 0x0200, CRC(269e0bdb) SHA1(51882b6bedc6488e4634f58bcaca4bb434e85d04) )
	ROM_CONTINUE(                0x3200, 0x0200 )
	ROM_CONTINUE(                0x1400, 0x0200 )
	ROM_CONTINUE(                0x3600, 0x0a00 )
	ROM_LOAD( "5_sub.bin",       0x9000, 0x0200, CRC(370b55a4) SHA1(10daebf5c858bec44d50d22348be3ffaa209e829) )
	ROM_CONTINUE(                0x8200, 0x0200 )
	ROM_CONTINUE(                0x9400, 0x0200 )
	ROM_CONTINUE(                0x8600, 0x0a00 )
	ROM_LOAD( "6_sub.bin",       0x8000, 0x0200, CRC(d0c0f66b) SHA1(b91859c363178dc21326305131f045c3f0e0a8d7) )
	ROM_CONTINUE(                0x9200, 0x0200 )
	ROM_CONTINUE(                0x8400, 0x0200 )
	ROM_CONTINUE(                0x9600, 0x0a00 )
	ROM_COPY( "maincpu", 0x2000, 0xa000, 0x2000 )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "7.bin",  0x0000, 0x0800, CRC(93933d1d) SHA1(fa38d2cb87e872bb9a3158a4df98f38360dc85ec) )
	ROM_LOAD( "8.bin",  0x0800, 0x0800, CRC(7409fbec) SHA1(f440f08ba026ae6172666e1bdc0894ce33bba420) )
	ROM_LOAD( "9.bin",  0x1000, 0x0800, CRC(22b0188a) SHA1(a9ed9ca8b36a60081fd364abc9bc23963932cc0b) )
	ROM_LOAD( "10.bin", 0x1800, 0x0800, CRC(50c7477d) SHA1(c04ec282a8cb528df5e38ad750d12ee71612695d) )

	ROM_REGION( 0x0120, "proms", 0 ) // not dumped for this set, taken from the very similar pacgal2
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, BAD_DUMP CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, BAD_DUMP CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) // sound PROMs,not dumped for this set, taken from the very similar pacgal2
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END

ROM_START( mschamp ) // "Original" Zola-Puc board.  FORCE ELECTRONICS KM-001 PCB copyright by RAYGLO MFG CO  1992/1993
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "9fg.bin", 0x10000, 0x10000, CRC(04dba113) SHA1(6260fb58c47a506a60385fb7536fc4fbd8e02c7c) )   // Banked

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


ROM_START( mschamps ) // Hack of hack???  Hack of the above "Rayglo" set???
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "pm4.bin", 0x10000, 0x10000, CRC(7d6b6303) SHA1(65ad72a9188422653c02a48c07ed2661e1e36961) )   // Banked

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
	ROM_LOAD( "superabc.u14", 0x00000, 0x80000, CRC(a560efe6) SHA1(c7d43cc3bb3b1b10d06403462276231bfc8542dd) )  // Banked

	ROM_REGION( 0x10000, "gfx1", ROMREGION_ERASE00 ) // descrambled rom goes here

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "char5e5f.u1",  0x00000, 0x20000, CRC(45caace0) SHA1(f850bd09ec68b0263ac8b30ae38c3878c7978ace) )

	ROM_REGION( 0x0120, "proms", 0 )    // Color PROMs
	ROM_LOAD( "82s123.7f",  0x0000, 0x0020, CRC(3a188666) SHA1(067386e477ce48bbde3cf71f744a78a42238d236) )
	ROM_LOAD( "82s129.4a",  0x0020, 0x0100, CRC(4382c049) SHA1(5e535b1a6852260f38ae1e5cd57290a85cb6927f) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",  0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used

	ROM_REGION( 0x0020, "unknown", 0 )
	ROM_LOAD( "82s123.u18", 0x0000, 0x0020, CRC(23b2863c) SHA1(e62f87d2145e94be06dbd90fa8d9a79760bfcc4b) )  // PROM on daughterboard, unknown function
ROM_END


ROM_START( superabco )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "superabco.u14", 0x00000, 0x80000, CRC(62565ad8) SHA1(cb434c608ee463788b73152d84ce6173bdfa350d) )  // Banked

	ROM_REGION( 0x10000, "gfx1", ROMREGION_ERASE00 ) // descrambled rom goes here

	ROM_REGION( 0x20000, "user1", 0 )
	ROM_LOAD( "char5e5f.u1",  0x00000, 0x20000, CRC(45caace0) SHA1(f850bd09ec68b0263ac8b30ae38c3878c7978ace) )

	ROM_REGION( 0x0120, "proms", 0 )    // Color PROMs
	ROM_LOAD( "82s123.7f",  0x0000, 0x0020, CRC(3a188666) SHA1(067386e477ce48bbde3cf71f744a78a42238d236) )
	ROM_LOAD( "82s129.4a",  0x0020, 0x0100, CRC(4382c049) SHA1(5e535b1a6852260f38ae1e5cd57290a85cb6927f) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",  0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )  // Timing - not used

	ROM_REGION( 0x0020, "unknown", 0 )
	ROM_LOAD( "82s123.u18", 0x0000, 0x0020, CRC(23b2863c) SHA1(e62f87d2145e94be06dbd90fa8d9a79760bfcc4b) )  // PROM on daughterboard, unknown function
ROM_END


ROM_START( crush )
	ROM_REGION( 0x10000, "maincpu", 0 )
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( crushbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cr1.bin",  0x0000, 0x1000, CRC(e2e84cd1) SHA1(3fc5a9aa3ee219b386a1d0622547c77aca27533d) )
	ROM_LOAD( "cr2.bin",  0x1000, 0x1000, CRC(ec020e6f) SHA1(eef9008c38a68ed20c1e3596016d97d4e72de9f2) )
	ROM_LOAD( "cr3.bin",  0x2000, 0x1000, CRC(d4455f27) SHA1(53f8ffc28be664fa8a2d756b4c70045a3f041bea) ) // matches original
	ROM_LOAD( "cr4.bin",  0x3000, 0x1000, CRC(9936ae06) SHA1(80aff9a12dab97e9d5818f7a7fac54dc52b579d4) )

	// No other ROMs in this set
	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "maketrax.5e",  0x0000, 0x1000, CRC(91bad2da) SHA1(096197d0cb6d55bf72b5be045224f4bd6a9cfa1b) )
	ROM_LOAD( "maketrax.5f",  0x1000, 0x1000, CRC(aea79f55) SHA1(279021e6771dfa5bd0b7c557aae44434286d91b7) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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
	ROM_LOAD( "cr2.5f",       0x1000, 0x0800, CRC(d5bc5cb8) SHA1(269b82ae2b838c72ae06bff77412f22bb779ad2e) )  // Copyright sign was removed
	ROM_LOAD( "cr4.5j",       0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "74s288.8a",    0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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
	ROM_LOAD( "cr2.5f",       0x1000, 0x0800, CRC(d5bc5cb8) SHA1(269b82ae2b838c72ae06bff77412f22bb779ad2e) )  // Copyright sign was removed
	ROM_LOAD( "cr4.5j",       0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	// the set with the above 'crushbl3' program roms and these gfx roms just seems to be a bad dump (some bad maze tiles?)
//  ROM_REGION( 0x2000, "gfx1", 0 )
//  ROM_LOAD( "cr1.bin",       0x0000, 0x0800, CRC(cc31c649) SHA1(a0640d2abc21872b0e680e8e31e3bcb7e7a07953) )
//  ROM_LOAD( "cr3.bin",       0x0800, 0x0800, CRC(14c121d8) SHA1(05f900a2e2a67401ab357340c1fb36153f365f1b) )
//  ROM_LOAD( "cr2.bin",       0x1000, 0x0800, CRC(882dc667) SHA1(5ea01d9c692b3061a0e39e2227fbc6af4baaab11) )  // Copyright sign was removed
//  ROM_LOAD( "cr4.bin",       0x1800, 0x0800, CRC(0d3877c4) SHA1(0a6f4098181480aa85225324129e37bba375252d) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "74s288.8a",    0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( crush3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.6e",  0x0000, 0x1000, CRC(50a1a776) SHA1(bdd5c2b0ce76744dedc3ff527a1f4fcfa38d193a) )
	ROM_LOAD( "2.6f",  0x1000, 0x1000, CRC(5b03c1f8) SHA1(11fffe37817739bfe4524278a6ec4b3555b088a0) )
	ROM_LOAD( "3.6h",  0x2000, 0x1000, CRC(ae5b39fb) SHA1(bf144f14baa3db5fc407488750183749d2b1ca8d) )
	ROM_LOAD( "4.6j",  0x3000, 0x1000, CRC(ddf63743) SHA1(34c8338bc7b14b200453febc7a90af8cb9416527) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5.5e",  0x0000, 0x1000, CRC(b1c86cd7) SHA1(4e9fdd37e99426bfb9da19ca41f1001af06b9ea2) )
	ROM_LOAD( "6.5f",  0x1000, 0x1000, CRC(b5c14376) SHA1(2c8c57f96c51f12f73daf65dc2a73e8185aaacea) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) ) // sldh w/crush4
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s129.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s129.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( crush4 )
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
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) ) // sldh w/crush3
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( crush5 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "crtwt.2", 0x10000, 0x10000, CRC(adbd21f7) SHA1(984b005cd7a73f697715ecb7a4d806024cb7596d) )   // Banked

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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( maketrax )
	ROM_REGION( 0x10000, "maincpu", 0 )
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( maketrxb )
	ROM_REGION( 0x10000, "maincpu", 0 )
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( korosuke )
	ROM_REGION( 0x10000, "maincpu", 0 )
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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
	ROM_LOAD( "mbrush.5f",    0x1000, 0x0800, CRC(d5bc5cb8) SHA1(269b82ae2b838c72ae06bff77412f22bb779ad2e) )  // Copyright sign was removed
	ROM_LOAD( "tpd",          0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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
	ROM_LOAD( "mbrush.5f",    0x1000, 0x0800, CRC(d5bc5cb8) SHA1(269b82ae2b838c72ae06bff77412f22bb779ad2e) )  // Copyright sign was removed
	ROM_LOAD( "tpd",          0x1800, 0x0800, CRC(d35d1caf) SHA1(65dd7861e05651485626465dc97215fed58db551) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "2s140.4a",     0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

// PCB is marked: "CRUSH ROLLER" on component side
// PCB is labelled: "PENNELLO 2" on component side ("pennello" is the Italian word for "brush")
// Bootleg of a hack? Shows Painter (c) 1984 Monshine Ent. Co. before coining up, Painter (c) 1984 Elsys Software after coining up.
// Interesting hack: it mixes Crush Roller elements with Pacman elements
ROM_START( painter )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pain1.6e",           0x0000, 0x1000, CRC(fb2eb6dc) SHA1(377075cb64c7ccc0bd2b0185848d4798428bceb2) )
	ROM_LOAD( "pain2.6f",           0x1000, 0x1000, CRC(39f92fb0) SHA1(2b58429664eadb8bb5100289182b274cc7e4688c) )
	ROM_LOAD( "pain3.6h",           0x2000, 0x1000, CRC(f80435b1) SHA1(7f41ecc2a91f8bdf0969f4f3fe96b48adb1010ba) )
	ROM_LOAD( "pain4-pennello2.6j", 0x3000, 0x1000, CRC(0cb678dc) SHA1(b1b9eadf22dc7985b39d414438092a7fc6c58955) ) // has strange strings in the second half, but seems to work fine

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "pain5.5e", 0x0000, 0x1000, CRC(bd819afc) SHA1(2e8762c3c480aa669f7e87651ddfdbb965ea4211) )
	ROM_LOAD( "pain6.5f", 0x1000, 0x1000, BAD_DUMP CRC(014e5ed3) SHA1(8e01c640457515da89723215b19684ceb4556997) ) // BADADDR            xx-xxxxxxxxx, dumped with 3 different programmers with same result, but probably damaged ROM

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "mb7051.7f",   0x0000, 0x0020, CRC(ff344446) SHA1(45eb37533da8912645a089b014f3b3384702114a) )
	ROM_LOAD( "n82s129n.4a", 0x0020, 0x0100, CRC(63efb927) SHA1(5c144a613fc4960a1dfd7ead89e7fee258a63171) )

	ROM_REGION( 0x0200, "namco", 0 )
	ROM_LOAD( "mb7052.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "mb7052.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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
	ROM_REGION( 0x10000, "maincpu", 0 )
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

	// No Sound Proms (not Namco Sound)
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( eyes2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g38201.7d",    0x0000, 0x1000, CRC(2cda7185) SHA1(7ec3ee9bb90e6a1d83ad3aa12fd62184e07b1399) )
	ROM_LOAD( "g38202.7e",    0x1000, 0x1000, CRC(b9fe4f59) SHA1(2d97dc1a0458b406ca0c50d6b8bd0dbe58d21464) )
	ROM_LOAD( "g38203.7f",    0x2000, 0x1000, CRC(d618ba66) SHA1(76d93d8bc09bafac464ebfd002869e21535a365b) )
	ROM_LOAD( "g38204.7h",    0x3000, 0x1000, CRC(cf038276) SHA1(bcf4e129a151e2245e630cf865ce6cb009b405a5) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "g38205.5d",    0x0000, 0x1000, CRC(03b1b4c7) SHA1(a90b2fbaee2888ee4f0bcdf80a069c8594ef5ea1) )  // This one has a (c) sign
	ROM_LOAD( "g38206.5e",    0x1000, 0x1000, CRC(a42b5201) SHA1(2e5cede3b6039c7bd5230de27d02aaa3f35a7b64) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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
	ROM_LOAD( "12.bin",          0x0800, 0x0800, CRC(99af4b30) SHA1(6a0939ff2fa7ae39a960dd4d9f9b7c01f57647c5) )
	ROM_LOAD( "10.bin",          0x1000, 0x0800, CRC(b247b82c) SHA1(8c10a8ef5e79b0b5fefad6eb77bfa68a0ca18035) )
	ROM_LOAD( "11.bin",          0x1800, 0x0800, CRC(aaa7a537) SHA1(571d981ed2aad62d7c7f2798e9084228d45523d4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7051.bin",        0x0000, 0x0020, CRC(2c3cc909) SHA1(32d68d4cfdf9f3e7351353428d268c763e809c63) ) // fixed 3x bytes with inverse second half
	ROM_LOAD( "7051-3.bin",      0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",       0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // not dumped, taken from parent
	ROM_LOAD( "7051-2.bin",      0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // two of these?
ROM_END


ROM_START( eyeszac ) // All ROMs / PROMs dumped and verified from actual PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.7e",         0x0000, 0x1000, CRC(e555b265) SHA1(ef3138d3d52b678bf26e8c2299719cca08eef4bf) ) // 2532
	ROM_LOAD( "2.7f",         0x1000, 0x1000, CRC(d6d73eb5) SHA1(b0c51afc09dd62bdda70710d57ae5b90a5e981ac) ) // 2532
	ROM_LOAD( "3.7h",         0x2000, 0x1000, CRC(604c940c) SHA1(a611c30e42492fc35d2a215dfc8c3ebda82909f7) ) // 2532
	ROM_LOAD( "4.7i",         0x3000, 0x1000, CRC(acc9cd8b) SHA1(d7fcf1b4b3466ee2187f82080634346a5427385e) ) // 2532

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5.5d",         0x0000, 0x1000, CRC(d6af0030) SHA1(652b779533e3f00e81cc102b78d367d503b06f33) ) // 2532
	ROM_LOAD( "6.5f",         0x1000, 0x1000, CRC(a42b5201) SHA1(2e5cede3b6039c7bd5230de27d02aaa3f35a7b64) ) // 2532

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(d8d78829) SHA1(19820d1651423210083a087fb70ebea73ad34951) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )
ROM_END

/* It's just a decrypted version of Eyes with the copyright changes...
 ROMs marked with a comment were in the set but we're not using them */
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "n82s129n.4k",  0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "n82s129n.6l",  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "gorkprom.3",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "gorkprom.2"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


// Distributed by:  Eagle Conversions, 25 Eagle St., Bldg #5, Providence, RI 02908
ROM_START( theglobp ) // Pac-Man PCB conversion kit. Includes a small daughtercard (2 roms + 4 PLDs, plugs in through the Z80 socket), 2 roms + 2 BPROMs
	ROM_REGION( 0x20000, "maincpu", 0 ) // these 2 ROMs are encrypted & located on daughtercard
	ROM_LOAD( "u 2 the glob pg02284 eagle.u2", 0x0000, 0x2000, CRC(829d0bea) SHA1(89f52b459a03fb40b9bbd97ac8a292f7ead6faba) ) // Actual label: U 2 THE GLOB PG02284 EAGLE
	ROM_LOAD( "u 3 the glob pg02284 eagle.u3", 0x2000, 0x2000, CRC(31de6628) SHA1(35a47dcf34efd74b5b2fda137e06a3dcabd74854) ) // Actual label: U 3 THE GLOB PG02284 EAGLE

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5 e the glob pg02284 eagle.5e", 0x0000, 0x1000, CRC(53688260) SHA1(9ce0d1d67d12743b69e8190bf7506b00b2f02955) ) // Actual label: 5 E THE GLOB PG02284 EAGLE
	ROM_LOAD( "5 f the glob pg02284 eagle.5f", 0x1000, 0x1000, CRC(051f59c7) SHA1(e1e1322686997e5bcdac164704b328cce352ae42) ) // Actual label: 5 F THE GLOB PG02284 EAGLE

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7 f the glob.7f", 0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) ) // Actual label: 7 F THE GLOB  (black dot preceeds "THE")
	ROM_LOAD( "4 a the glob.4a", 0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) ) // Actual label: 7 F THE GLOB  (black dot preceeds "THE")

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

ROM_START( theglobpa )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1-2516.bin",   0x0000, 0x0800, CRC(760f4764) SHA1(fc29aa0a1d5da28a40590ef37bbd1255713fb1a4) )
	ROM_LOAD( "2-2516.bin",   0x0800, 0x0800, CRC(7d556bc6) SHA1(aef2b6799bd2a51e3de8282361d0c03c5bedb8ad) )
	ROM_LOAD( "3-2516.bin",   0x1000, 0x0800, CRC(ca9dafca) SHA1(41cb4313f9a46cb40c7b41ea8c7c522c1d4b5d54) )
	ROM_LOAD( "4-2516.bin",   0x1800, 0x0800, CRC(fff64f47) SHA1(a9b8a5e1641626eff312b4588d8afae8f8811e6d) )
	ROM_LOAD( "5-2716.bin",   0x2000, 0x0800, CRC(3c352e0f) SHA1(5bc30414da27a96f9e96e3dccccc0a2d66c92731) )
	ROM_LOAD( "6-2716.bin",   0x2800, 0x0800, CRC(5a7ba8b0) SHA1(d6372ff05ade84957acd25dfc37adcfd47927358) )
	ROM_LOAD( "7-2716.bin",   0x3000, 0x0800, CRC(09f6b061) SHA1(7a39b8ad3f17f04aa908930ccc340627f2147216) )
	ROM_LOAD( "8-2716.bin",   0x3800, 0x0800, CRC(192b6d61) SHA1(30324859c7e0acd001b29c95b29ebf2156f2a802) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "9-2716.bin",   0x0000, 0x0800, CRC(36408c76) SHA1(f5bb18e38de57adc2aed6211048d9f0ee0e58df7) )
	ROM_LOAD( "11-2716.bin",  0x0800, 0x0800, CRC(b8ba069c) SHA1(f8d8e40afd8214a6d951af8de2761703b0651f79) )
	ROM_LOAD( "10-2716.bin",  0x1000, 0x0800, CRC(e0478b4e) SHA1(9697c7fd92752d052aea4c46292b1b7cae28f606) )
	ROM_LOAD( "12-2716.bin",  0x1800, 0x0800, CRC(ffb30caf) SHA1(ecdadd8207bc54548dae751e3e08c6647cd1f25e) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "tbp18s030.8h", 0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "82s129.4a",    0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "63s141.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "63s141.3m",    0x0100, 0x0100, CRC(2ee34ade) SHA1(7cd43283b9648feb9a15466212b7a480fad20a39) ) // Timing - not used

	ROM_REGION( 0x0800, "extra", 0 )
	ROM_LOAD( "top-2716.bin", 0x0000, 0x0800, CRC(25e74cd5) SHA1(dcee1fda9abe7fdeac3a87ef7897afda946efcb2) ) // EPROM on a subboard configured to replace a BPROM
ROM_END

// Program ROMs same as the globp
ROM_START( sprglobp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "u 2 the glob pg02284 eagle.u2", 0x0000, 0x2000, CRC(829d0bea) SHA1(89f52b459a03fb40b9bbd97ac8a292f7ead6faba) )
	ROM_LOAD( "u 3 the glob pg02284 eagle.u3", 0x2000, 0x2000, CRC(31de6628) SHA1(35a47dcf34efd74b5b2fda137e06a3dcabd74854) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e_2532.dat",  0x0000, 0x1000, CRC(1aa16109) SHA1(ddc8606512d7ab7555b84146b9d793f65ad0a75f) )
	ROM_LOAD( "5f_2532.dat",  0x1000, 0x1000, CRC(afe72a89) SHA1(fb17632e2665c3cebc1865ef25fa310cc52725c4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7 f the glob.7f", 0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "4 a the glob.4a", 0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

// A second dump exists. It has half sized program ROMs. The blister was missing 2 ROMs.
// What's available is identical to the below dump but for 1 single byte at 0x88c which is 0x9a below and 0x92 in the other dump.
ROM_START( sprglobp2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x0000, 0x1000, CRC(ac5bd172) SHA1(8c74ba7611e58e677f384ccd1fc1022b84cc2190) )
	ROM_LOAD( "2.bin", 0x1000, 0x1000, CRC(35c7fcf1) SHA1(efb2efd51fb5643ad4f4df11593197097c5cad3f) )
	ROM_LOAD( "3.bin", 0x2000, 0x1000, CRC(c10aae4b) SHA1(e40f6066c2eeefcf60553360eb424b875ef007b3) )
	ROM_LOAD( "4.bin", 0x3000, 0x1000, CRC(b8fd4eb2) SHA1(9bd003b20af0fcaa27780cb7764795b6597f1156) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5,bin",  0x0000, 0x1000, CRC(1aa16109) SHA1(ddc8606512d7ab7555b84146b9d793f65ad0a75f) )
	ROM_LOAD( "6.bin",  0x1000, 0x1000, CRC(afe72a89) SHA1(fb17632e2665c3cebc1865ef25fa310cc52725c4) )

	ROM_REGION( 0x0120, "proms", 0 ) // not dumped for this set
	ROM_LOAD( "7 f the glob.7f", 0x0000, 0x0020, BAD_DUMP CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "4 a the glob.4a", 0x0020, 0x0100, BAD_DUMP CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs, not dumped for this set
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, BAD_DUMP CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, BAD_DUMP CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END

/* This set is from a modified Pengo board.  Pengo and Pacman are functionally the same.
   The bad sound is probably correct as the sound data is part of the protection. */
ROM_START( sprglbpg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic8.1",      0x0000, 0x1000, CRC(a2df2073) SHA1(14c55186053b080de06cc3691111ede8b2ead231) )
	ROM_LOAD( "ic7.2",      0x1000, 0x1000, CRC(3d2c22d9) SHA1(2f1d27e49850f904d1f2256bfcf00557ed88bb16) )
	ROM_LOAD( "ic15.3",     0x2000, 0x1000, CRC(a252047f) SHA1(9fadbb098b86ee98e1a81da938316b833fc26912) )
	ROM_LOAD( "ic14.4",     0x3000, 0x1000, CRC(7efa81f1) SHA1(583999280623f02dcc318a6c7af5ee6fc46144b8) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "ic92.5",  0x0000, 0x2000, CRC(e54f484d) SHA1(4feb9ec917c2467a5ac531283cb00fe308be7775) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "ic78.prm",      0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "ic88.prm",      0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "ic51.prm",    0x0000, 0x0100, CRC(c29dea27) SHA1(563c9770028fe39188e62630711589d6ed242a66) )
	ROM_LOAD( "ic70.prm"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used
ROM_END

// 2 PCB set (G-GA-2 and G-GB-2). It was modified to use one 27128 instead of eight 2716 for the program ROMs.
ROM_START( theglobme )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "globo_1-8_a.6e",  0x0000, 0x4000, CRC(57252220) SHA1(ee02a1f8817cb5c55d67653391f4509bb5a30403) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "globo-10_b.5e",   0x0000, 0x0800, CRC(36408c76) SHA1(f5bb18e38de57adc2aed6211048d9f0ee0e58df7) )
	ROM_LOAD( "globo-12_b.5h",   0x0800, 0x0800, CRC(b8ba069c) SHA1(f8d8e40afd8214a6d951af8de2761703b0651f79) )
	ROM_LOAD( "globo-11_b.5f",   0x1000, 0x0800, CRC(890b8ebf) SHA1(1bf64f4ca1fca8efd35ac3d414d2bb755c5e44cc) )
	ROM_LOAD( "globo-13_b.5j",   0x1800, 0x0800, CRC(7c4456a4) SHA1(74f55ae921cdf8f1f7a866d75a63244187426f17) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "n82s123an_a.7f",  0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "n82s129n_b.4a",   0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs, Harris 63S141J
	ROM_LOAD( "63s141_b.1m",     0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "63s141_b.3m",     0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( beastfp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "bf-u2.bin",    0x0000, 0x2000, CRC(3afc517b) SHA1(5b74bca9e9cd4d8bcf94a340f8f0e53fe1dcfc1d) )
	ROM_LOAD( "bf-u3.bin",    0x2000, 0x2000, CRC(8dbd76d0) SHA1(058c01e87ad583eb99d5043a821e6c68f1b30267) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "beastf.5e",    0x0000, 0x1000, CRC(5654dc34) SHA1(fc2336b951a3ab48d4fc4f36a8dd80e79e8ca1a0) )
	ROM_LOAD( "beastf.5f",    0x1000, 0x1000, CRC(1b30ca61) SHA1(8495d8a280346246f00c4f1dc42ab5a2a02c5863) )

	ROM_REGION( 0x002c, "epos_pal10h8", 0)
	ROM_LOAD( "beastf.ic4",   0x0000, 0x002c, CRC(a6ab926d) SHA1(a8c2ddce2d595cecf057b37916f2f897de8a0e4e) ) // pal10h8

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7_f_the_glob.7f", 0x0000, 0x0020, CRC(1f617527) SHA1(448845cab63800a05fcb106897503d994377f78f) )
	ROM_LOAD( "4_a_the_glob.4a", 0x0020, 0x0100, CRC(28faa769) SHA1(7588889f3102d4e0ca7918f536556209b2490ea1) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( eeekkp ) // Pac-Man PCB conversion kit. Includes a small daughtercard (2 roms + 4 PLDs, plugs in through the Z80 socket), 2 roms + 2 BPROMs
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "u_2_eeekk_pg03094.u2",   0x0000, 0x2000, CRC(701e37f2) SHA1(15bbd983e9112ce15dd229f126f2bccfa8b9807c) ) // encrypted  - located on daughtercard
	ROM_LOAD( "u_3_eeekk_pg03094.u3",   0x2000, 0x2000, CRC(bcf524ae) SHA1(be2a1a2984ea1439c63d8c353e4ae85bf42c8a55) ) // encrypted  - located on daughtercard

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5_e_eeekk_pg03094.5e",   0x0000, 0x1000, CRC(780183a8) SHA1(8466d75d79b3c87db143f5c2e8b58dad1d2e501c) )
	ROM_LOAD( "5_f_eeekk_pg03094.5f",   0x1000, 0x1000, CRC(418526e4) SHA1(a95764e216ccdaca0631604e1101ba91884effde) )

	ROM_REGION( 0x002c, "epos_pal10h8", 0)
	ROM_LOAD( "eeekk.ic4",    0x0000, 0x002c, CRC(f588ba4e) SHA1(07d74da02172789ccd1fead2b5afe8ce72c5149d) ) // pal10h8

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7_f_eeekk.7f",     0x0000, 0x0020, CRC(c64c8a53) SHA1(55e7b88cb1ce129e8154722a489d76c38924d3f1) ) // 82s123
	ROM_LOAD( "4_a_eeekk.4a",     0x0020, 0x0100, CRC(a5044ded) SHA1(566bd06674bf8069dc633102493c9991b64e4379) ) // 82s126

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) ) // 82s126
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // 82s126 - timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used

	// Unknown, used for the mystery items?
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used

	// Unknown, used for the mystery items?
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used
ROM_END


ROM_START( acitya )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "aca_u2.bin",   0x0000, 0x2000, CRC(261c2fdc) SHA1(b4e7e6c8d8e401c7e4673213074802a73b9886a2) )
	ROM_LOAD( "aca_u3.bin",   0x2000, 0x2000, CRC(05fab4ca) SHA1(5172229eda25920eeaa6d9f610f2bcfa674979b7) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "aca_5e.bin",   0x0000, 0x1000, CRC(7f2dd2c9) SHA1(aa7ea70355904989b99d568d1e055e8272cfa8ca) )
	ROM_RELOAD( 0x1000, 0x1000 ) // Not Used??

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "aca_7f.bin",   0x0000, 0x0020, CRC(133bb744) SHA1(da4074f3ea30202973f0b6c9ad05a992bb44eafd) )
	ROM_LOAD( "aca_4a.bin",   0x0020, 0x0100, CRC(8e29208f) SHA1(a30a405fbd43d27a8d403f6c3545178564dede5d) )

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used
ROM_END

ROM_START( bwcasino ) // Pac-Man PCB conversion kit. Includes a small daughtercard (2 roms + 4 PLDs, plugs in through the Z80 socket), 1 rom + 2 BPROMs
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "u_2_boardwalk_casino_pp09144.u2",   0x0000, 0x2000, CRC(e2eea868) SHA1(9e9dae02ab746ef48981f42a75c192c5aae0ffee) ) // labeled U 2 BOARDWALK CASINO PP09144  - located on daughtercard
	ROM_LOAD( "u_3_boardwalk_casino_pp09144.u3",   0x2000, 0x2000, CRC(a935571e) SHA1(ab4f53be2544593fc8eb4c4bcccdec4191c0c626) ) // labeled U 3 BOARDWALK CASINO PP09144  - located on daughtercard

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5_e_boardwalk_casino_pp09144.5e",   0x0000, 0x1000, CRC(e334c01e) SHA1(cc6e50e3cf51eb8b7b27aa7351733954da8128ff) ) // labeled 5 E BOARDWALK CASINO PP09144
	ROM_RELOAD( 0x1000, 0x1000 ) // Not Used??

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7_f_b.w.c.7f",   0x0000, 0x0020, CRC(133bb744) SHA1(da4074f3ea30202973f0b6c9ad05a992bb44eafd) ) // labeled 7 F B.W.C. with single red dot
	ROM_LOAD( "4_a_b.w.c.4a",   0x0020, 0x0100, CRC(8e29208f) SHA1(a30a405fbd43d27a8d403f6c3545178564dede5d) ) // labeled 4 A B.W.C. with single red dot

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used
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
	ROM_LOAD( "82s126.4a", 0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   // Timing - not used
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
	ROM_LOAD( "82s126.4a", 0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   // Timing - not used
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
	ROM_LOAD( "82s126.4a", 0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 ) // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   // Timing - not used
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   // Timing - not used
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
	// ROMs dumped from epoxy block
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "pr.1k", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "pr.3k", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   // Timing - not used
ROM_END

ROM_START( woodpeca )
	ROM_REGION( 0x10000, "maincpu",0 )
	// ROMs dumped from epoxy block
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

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "pr.1k", 0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "pr.3k", 0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )   // Timing - not used
ROM_END


/*
Guru-Readme for Number Crash (Hanshin Goraku 1983)

|----------------------------------------------------------------------|
|        74107   Z80A          LS157  MB7052.4A LS158                  |
|        LS368 18.432MHz                LS377   LS157                  |
| LS16   LS367   DIP28A        LS194    LS00    7475            93422  |
|        LS367                 LS194    LS273   LS20                   |
| DSW(8) LS367 P1.7E  NC-1.6E  NC-9.5E  LS157   7489  7461       LS161 |
|2       LS367 LS08   NC-2.6F  NC-10.5F LS86    7489  LS283      LS283 |
|2             LS138  NC-3.6H  NC-11.5H                          LS174 |
|W             4099            NC-12.5J LS245   LS158 LS86             |
|A             LS02   NC-4.6K                   LS157             LS283|
|Y             LS139  NC-5.6L  LS139                  AM27S03.2K  LS174|
|              LS42                     2114    P2.3M AM27S03.2L  P3.1M|
|              LS367  NC-6.6M  LS74     2114    LS74                   |
|     MB3713                   LS08     2114          LS273      4066  |
|                     NC-7.6N           2114    LS10  LS02             |
|                     NC-8.6P  DIP28B   2114                     7416  |
|         VOLUME      LS367             2114    7416  7416       7416  |
|----------------------------------------------------------------------|
Notes: (All IC's shown)
      Z80A   - Clock input 3.072MHz [18.432/6]
      7489   - Texas Instruments 7489 64-bit Random Access Read-Write Memory
      2114   - 1k x 4-bit SRAM
      AM27S03- AMD AM27S03 (74S189 / 3101 compatible) 64-bit (16word x 4-bit) Inverting-Output Bipolar RAM
      93422  - AMD 93422 256 x 4-bit TTL Bipolar RAM
      NC-*   - 2716 EPROM
      P1     - Fujitsu MB7051 Bipolar PROM
      P2/3   - Harris 7611 Bipolar PROM
      MB7052 - Fujitsu MB7052 Bipolar PROM
      DIP28A - Plug-in daughterboard containing logic
               |----------|
               |   LS373  |
               |   LS245  |
               |LS02  LS00|
               |LS74  LS04|
               |----------|

      DIP28B - Plug-in daughterboard containing logic
               |----------------------------|
               |LS257 LS253 LS253 LS253 LS00|
               |                            |
               |LS86 LS86 LS86 LS02 LS368   |
               |----------------------------|

      VSync  - 60.5721Hz
      HSync  - 15.3892kHz
*/

ROM_START( numcrash )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nc-1.6e",      0x0000, 0x0800, CRC(c85a79ba) SHA1(938b6b2920b14a4f0fbcd09fe25873bfd8ddc245) )
	ROM_LOAD( "nc-5.6k",      0x0800, 0x0800, CRC(20e6c64d) SHA1(ff61ede279a46ea2bec534dd82ec89e41a3ec77b) )
	ROM_LOAD( "nc-2.6f",      0x1000, 0x0800, CRC(a9d50002) SHA1(048a6aca5a7cc5126cd143b444a4241c7e9d4ac6) )
	ROM_LOAD( "nc-6.6m",      0x1800, 0x0800, CRC(7b8de692) SHA1(7d5fe625ee9acf3cced2d98df99f5dee6c8122b1) )
	ROM_LOAD( "nc-3.6h",      0x2000, 0x0800, CRC(e47f7cf3) SHA1(47e513cf4fe80617547093210ca6582646a9b256) )
	// 0x2800 - 0x2fff unpopulated? would usually be 6n
	ROM_LOAD( "nc-4.6j",      0x3000, 0x0800, CRC(d9dee8cd) SHA1(b577e51b6b90673be10528ba4528e4d1c150a266) )
	// 0x3800 - 0x3fff unpopulated? would usually be 6p
	ROM_LOAD( "nc-7.6n",      0x8000, 0x1000, CRC(440c67e4) SHA1(a29385b4263e61bc8d48e531d0f7b161f8d66f7a) )
	ROM_LOAD( "nc-8.6p",      0x9000, 0x1000, CRC(7dabb3e5) SHA1(29dd5f6b7acc3abd292c903be6fef9fc4a135287) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "nc-9.5e",      0x0000, 0x0800, CRC(6a30b507) SHA1(e5760844cd051504e61774c57549c2697d473591) )
	ROM_LOAD( "nc-11.5h",     0x0800, 0x0800, CRC(5f54d8e6) SHA1(3a20df62f484846e5610b88c04db56f9b5b3029c) )
	ROM_LOAD( "nc-10.5f",     0x1000, 0x0800, CRC(bdd352db) SHA1(c4dd0bd009098a82dd5603046fa929f7395dd903) )
	ROM_LOAD( "nc-12.5j",     0x1800, 0x0800, CRC(b104a8a9) SHA1(ac7f9b0041a9be1d0ffa58d2f65a0d294e986357) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7051p1.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "7052.4a",      0x0020, 0x0100, CRC(2bc5d339) SHA1(446e234df94d9ef34c3191877bb33dd775acfdf5) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "7611p3.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "7611p2.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


ROM_START( bigbucks )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p.rom",        0x0000, 0x4000, CRC(eea6c1c9) SHA1(eaea4ffbcdfbb38364887830fd00ac87fe838006) )
	ROM_LOAD( "m.rom",        0x8000, 0x2000, CRC(bb8f7363) SHA1(11ebdb1a3c589515240d006646f2fb3ead06bdcf) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e.cpu",       0x0000, 0x1000, CRC(18442c37) SHA1(fac445d15731532364315852492b48470039c0ca) )
	ROM_RELOAD( 0x1000, 0x1000 ) // Not Used??

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used

	ROM_REGION( 0x60000, "user1", 0 )   // Question ROMs
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
	ROM_RELOAD( 0x1000, 0x1000 ) // Not Used??

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "7f.cpu",       0x0000, 0x0020, CRC(7549a947) SHA1(4f2c3e7d6c38f0b9a90317f91feb3f86c9a0d0a5) )
	ROM_LOAD( "4a.cpu",       0x0020, 0x0100, CRC(ddd5d88e) SHA1(f28e1d90bb495001c30c63b0ef2eec45de568174) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m"  ,  0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) ) // Timing - not used

	ROM_REGION( 0x40000, "user1", 0 )   // Question ROMs
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

// Special thanks to Rob Walmsley for dumping his PCB
ROM_START( cannonbp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "n1-6e",        0x0000, 0x0800, CRC(c68878c7) SHA1(4ab69f820e861a33fa7555286459e5953c126d33) )
	ROM_LOAD( "n2-6k",        0x0800, 0x0800, CRC(ff3951a5) SHA1(8c06526903f2ce3e0ac29ba0138e8c270732150a) )
	ROM_LOAD( "n3-6f",        0x1000, 0x0800, CRC(2329079d) SHA1(461d1258a03ea35ba33f4094c05f8ffda8cdfc47) )
	ROM_LOAD( "n4-6m",        0x1800, 0x0800, CRC(fcc57ecb) SHA1(9efafbb7f3d44652aded860e734332f47354e299) )
	ROM_LOAD( "n5-6h",        0x2000, 0x0800, CRC(52846c9d) SHA1(4a479ff8961c8865aea12976355d0201a8cb1b48) )
	ROM_LOAD( "n6-6n",        0x2800, 0x0800, CRC(59e890dd) SHA1(c148b71d71fb49a138ef1afe771be70bec21ad2b) )
	// 3000 - 3fff = epoxy protection block

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "z1-5e",        0x0000, 0x0800, CRC(125779e0) SHA1(c1ae7214e3ff1a941ae0c4948ec886bc84b6f040) )
	ROM_LOAD( "z3-5h",        0x0800, 0x0800, CRC(78f866c0) SHA1(171f0b5ed6c8b1f2ae8dc1b7e97ea9f0c3d8e18e) )
	ROM_LOAD( "z2-5f",        0x1000, 0x0800, CRC(fbd2c99d) SHA1(135a5d86d59010cc608b099054815dd31d443da7) )
	ROM_LOAD( "z4-5j",        0x1800, 0x0800, CRC(8734c904) SHA1(86d9dc72d3cfc863c558967860ac4d2dc8d2c07c) )

	ROM_REGION( 0x0120, "proms", 0 ) // These give some ugly colours, but should be correct
	ROM_LOAD( "colorprom_1",    0x0000, 0x0020, CRC(08f8ae7e) SHA1(cd1e26da5f214f4d9924a30e6d9cf312f91c2028) )
	ROM_LOAD( "colorprom_2",    0x0020, 0x0100, CRC(359a15dc) SHA1(e57ef15eb3baac70fe9e2db897c4165da3c00e20) )

	ROM_REGION( 0x0200, "namco", 0 )    // Sound PROMs
	ROM_LOAD( "82s126.1m",    0x0000, 0x0100, CRC(a9cc86bf) SHA1(bbcec0570aeceb582ff8238a4bc8546a23430081) )
	ROM_LOAD( "82s126.3m",    0x0100, 0x0100, CRC(77245b66) SHA1(0c4d0bee858b97632411c440bea6948a74759746) )    // Timing - not used
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void pacman_state::init_maketrax()
{
	// Set up protection handlers
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x5004, 0x5004, write8smo_delegate(*this, FUNC(pacman_state::maketrax_protection_w)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5080, 0x50bf, read8sm_delegate(*this, FUNC(pacman_state::maketrax_special_port2_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x50c0, 0x50ff, read8sm_delegate(*this, FUNC(pacman_state::maketrax_special_port3_r)));

	save_item(NAME(m_maketrax_disable_protection));
	save_item(NAME(m_maketrax_offset));
	save_item(NAME(m_maketrax_counter));
}

void pacman_state::init_mbrush()
{
	// Set up protection handlers
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x5080, 0x50ff, read8sm_delegate(*this, FUNC(pacman_state::mbrush_prot_r)));
}

void pacman_state::init_ponpoko()
{
	/* The gfx data is swapped wrt the other Pac-Man hardware games. */
	/* Here we revert it to the usual format. */

	int i, j;
	uint8_t *RAM, temp;
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

void pacman_state::eyes_decode(uint8_t *data)
{
	int j;
	uint8_t swapbuffer[8];

	for (j = 0; j < 8; j++)
	{
		swapbuffer[j] = data[bitswap<16>(j,15,14,13,12,11,10,9,8,7,6,5,4,3,0,1,2)];
	}

	for (j = 0; j < 8; j++)
	{
		data[j] = bitswap<8>(swapbuffer[j],7,4,5,6,3,2,1,0);
	}
}

void pacman_state::init_eyes()
{
	/* CPU ROMs */

	/* Data lines D3 and D5 swapped */
	uint8_t *RAM = memregion("maincpu")->base();
	for (int i = 0; i < 0xc000; i++)
	{
		RAM[i] = bitswap<8>(RAM[i],7,6,3,4,5,2,1,0);
	}


	/* Graphics ROMs */

	/* Data lines D4 and D6 and address lines A0 and A2 are swapped */
	RAM = memregion("gfx1")->base();
	int len = memregion("gfx1")->bytes();
	for (int i = 0; i < len; i += 8)
		eyes_decode(&RAM[i]);
}

void pacman_state::mspacman_install_patches(uint8_t *ROM)
{
	/* copy forty 8-byte patches into Pac-Man code */
	for (int i = 0; i < 8; i++)
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

void pacman_state::init_mspacman()
{
	/* CPU ROMs */

	/* Pac-Man code is in low bank */
	uint8_t *ROM = memregion("maincpu")->base();

	/* decrypted Ms. Pac-Man code is in high bank */
	uint8_t *DROM = &memregion("maincpu")->base()[0x10000];

	/* copy ROMs into decrypted bank */
	for (int i = 0; i < 0x1000; i++)
	{
		DROM[0x0000+i] = ROM[0x0000+i]; /* pacman.6e */
		DROM[0x1000+i] = ROM[0x1000+i]; /* pacman.6f */
		DROM[0x2000+i] = ROM[0x2000+i]; /* pacman.6h */
		DROM[0x3000+i] = bitswap<8>(ROM[0xb000+bitswap<12>(i,11,3,7,9,10,8,6,5,4,2,1,0)],0,4,5,7,6,3,2,1);  /* decrypt u7 */
	}
	for (int i = 0; i < 0x800; i++)
	{
		DROM[0x8000+i] = bitswap<8>(ROM[0x8000+bitswap<11>(i,8,7,5,9,10,6,3,4,2,1,0)],0,4,5,7,6,3,2,1);  /* decrypt u5 */
		DROM[0x8800+i] = bitswap<8>(ROM[0x9800+bitswap<11>(i,3,7,9,10,8,6,5,4,2,1,0)],0,4,5,7,6,3,2,1);  /* decrypt half of u6 */
		DROM[0x9000+i] = bitswap<8>(ROM[0x9000+bitswap<11>(i,3,7,9,10,8,6,5,4,2,1,0)],0,4,5,7,6,3,2,1);  /* decrypt half of u6 */
		DROM[0x9800+i] = ROM[0x1800+i];     /* mirror of pacman.6f high */
	}
	for (int i = 0; i < 0x1000; i++)
	{
		DROM[0xa000+i] = ROM[0x2000+i];     /* mirror of pacman.6h */
		DROM[0xb000+i] = ROM[0x3000+i];     /* mirror of pacman.6j */
	}
	/* install patches into decrypted bank */
	mspacman_install_patches(DROM);

	/* mirror Pac-Man ROMs into upper addresses of normal bank */
	for (int i = 0; i < 0x1000; i++)
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

void pacman_state::init_mschamp()
{
	save_item(NAME(m_counter));
	m_counter = 0;
}

void pacman_state::init_woodpek()
{
	/* Graphics ROMs */

	/* Data lines D4 and D6 and address lines A0 and A2 are swapped */
	uint8_t *RAM = memregion("gfx1")->base();
	int len = memregion("gfx1")->bytes();
	for (int i = 0;i < len;i += 8)
		eyes_decode(&RAM[i]);
}

void pacman_state::init_pacplus()
{
	pacplus_decode();
}

void pacman_state::init_jumpshot()
{
	jumpshot_decode();
}

void pacman_state::init_drivfrcp()
{
	uint8_t *ROM = memregion("maincpu")->base();
	membank("bank1")->set_base(&ROM[0 * 0x2000]);
	membank("bank2")->set_base(&ROM[1 * 0x2000]);
	membank("bank3")->set_base(&ROM[2 * 0x2000]);
	membank("bank4")->set_base(&ROM[3 * 0x2000]);
}

void pacman_state::init_8bpm()
{
	/* Data lines D0 and D6 swapped */
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x8000; i++)
	{
		ROM[i] = bitswap<8>(ROM[i],7,0,5,4,3,2,1,6);
	}

	membank("bank1")->set_base(&ROM[0 * 0x2000]);
	membank("bank2")->set_base(&ROM[1 * 0x2000]);
	membank("bank3")->set_base(&ROM[2 * 0x2000]);
	membank("bank4")->set_base(&ROM[3 * 0x2000]);
}

void pacman_state::init_porky()
{
	/* Data lines D0 and D4 swapped */
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
	{
		ROM[i] = bitswap<8>(ROM[i],7,6,5,0,3,2,1,4);
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

void pacman_state::init_rocktrv2()
{
	/* hack to pass the rom check for the bad rom */
	uint8_t *ROM = memregion("maincpu")->base();

	ROM[0x7ffe] = 0xa7;
	ROM[0x7fee] = 0x6d;

	save_item(NAME(m_rocktrv2_question_bank));
}

/* The encryption is provided by a 74298 sitting on top of the rom at 6f.
The select line is tied to a2; a0 and a1 of the eprom are are left out of
socket and run through the 74298.  Clock is tied to system clock.  */
void pacman_state::init_mspacmbe()
{
	/* Address lines A1 and A0 swapped if A2=0 */
	uint8_t *ROM = memregion("maincpu")->base();
	for (int i = 0x1000; i < 0x2000; i += 4)
	{
		if (!(i & 8))
		{
			uint8_t temp = ROM[i+1];
			ROM[i+1] = ROM[i+2];
			ROM[i+2] = temp;
		}
	}
}

uint8_t pacman_state::mspacii_protection_r(offs_t offset)
{
	/* used by extra routine at $3FE, bit 4 of 504d needs to be low, and of 504e to be high */
	uint8_t data = ioport("IN1")->read();
	return (data & 0xef) | (offset << 4 & 0x10);
}

void pacman_state::init_mspacii()
{
	// protection
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x504d, 0x506f, read8sm_delegate(*this, FUNC(pacman_state::mspacii_protection_r)));
}

void pacman_state::init_superabc()
{
	uint8_t *src = memregion("user1")->base();
	uint8_t *dest = memregion("gfx1")->base();

	// descramble gfx
	for (int i = 0; i < 0x10000; i++)
		dest[i] = src[bitswap<24>(i,23,22,21,20,19,18,17, 12,13,14,16,15, 11,10,9,8,7,6,5,4,3,2,1,0)];
}

uint8_t pacman_state::cannonbp_protection_r(offs_t offset)
{
	/* At 6p where a rom would usually be there is an epoxy resin chip with 'Novomatic Industrie' Cannon Ball tm 1984 label. */
	/* As I have no clue about what shall be in this chip, what follows is only a simulation which is enough to play the game. */
	switch (offset)
	{
		default:
			logerror("CPU0 %04x: Unhandled protection read, offset %04x\n", m_maincpu->pc(), offset);
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
			if (m_maincpu->pc() == 0x2b97)
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

void pacman_state::init_pengomc1()
{
	uint8_t *romdata = memregion("maincpu")->base();
	uint8_t buf[0xc000];
	memcpy(buf, romdata, 0xc000);

	// some sort of weak protection?
	for (int i = 0; i < 0xc000; i++)
		romdata[i] = buf[i^0xff];
}

ioport_value clubpacm_state::clubpacm_input_r()
{
	ioport_value data = 0x0f;

	if (!m_mainlatch->q5_r())
		data &= m_players[0]->read();
	if (!m_mainlatch->q4_r())
		data &= m_players[1]->read();

	return data ^ 0x0f;
}

void clubpacm_state::init_clubpacma()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0x0000; i < 0xc000; i++)
		rom[i] = bitswap<8>(rom[i], 6, 7, 5, 4, 3, 2, 1, 0);
}

void mspactwin_state::init_mspactwin()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int A = 0x0000; A < 0x4000; A+=2)
	{
		// decode opcode
		m_decrypted_opcodes     [A  ] = bitswap<8>(rom[       A  ]       , 4, 5, 6, 7, 0, 1, 2, 3);
		m_decrypted_opcodes     [A+1] = bitswap<8>(rom[       A+1] ^ 0x9a, 6, 4, 5, 7, 2, 0, 3, 1);
		m_decrypted_opcodes_high[A  ] = bitswap<8>(rom[0x8000+A  ]       , 4, 5, 6, 7, 0, 1, 2, 3);
		m_decrypted_opcodes_high[A+1] = bitswap<8>(rom[0x8000+A+1] ^ 0x9a, 6, 4, 5, 7, 2, 0, 3, 1);

		// decode operand
		rom[       A  ] = bitswap<8>(rom[       A  ]       , 0, 1, 2, 3, 4, 5, 6, 7);
		rom[       A+1] = bitswap<8>(rom[       A+1] ^ 0xa3, 2, 4, 6, 3, 7, 0, 5, 1);
		rom[0x8000+A  ] = bitswap<8>(rom[0x8000+A  ]       , 0, 1, 2, 3, 4, 5, 6, 7);
		rom[0x8000+A+1] = bitswap<8>(rom[0x8000+A+1] ^ 0xa3, 2, 4, 6, 3, 7, 0, 5, 1);
	}
}

void pacman_state::init_mspackpls()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0x0000; i < 0xa000; i++)
		rom[i] = bitswap<8>(rom[i], 7, 6, 5, 3, 4, 2, 1, 0);
}

void epospm_state::init_sprglobp2()
{
	// this set is very similar to the unencrypted sprglbpg set
	// for some reason the following doesn't work for some ranges
	// (opcodes are unencrypted there).

	static const uint8_t data_xortable[16][8] =
	{
		{ 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, },    // 0x0000
		{ 0xa0, 0xa0, 0x88, 0x88, 0x88, 0x88, 0xa0, 0xa0, },    // 0x0001
		{ 0x00, 0x00, 0x88, 0x88, 0x00, 0x00, 0x88, 0x88, },    // 0x0010
		{ 0xa0, 0xa0, 0x88, 0x88, 0x88, 0x88, 0xa0, 0xa0, },    // 0x0011
		{ 0x88, 0x88, 0xa0, 0xa0, 0x28, 0x28, 0x00, 0x00, },    // 0x0100
		{ 0xa0, 0xa0, 0x88, 0x88, 0x88, 0x88, 0xa0, 0xa0, },    // 0x0101
		{ 0x20, 0x20, 0x20, 0x20, 0x80, 0x80, 0x80, 0x80, },    // 0x0110
		{ 0xa0, 0xa0, 0x88, 0x88, 0x88, 0x88, 0xa0, 0xa0, },    // 0x0111
		{ 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, 0xa8, },    // 0x1000
		{ 0x28, 0x28, 0xa0, 0xa0, 0x00, 0x00, 0x88, 0x88, },    // 0x1001
		{ 0x00, 0x00, 0x88, 0x88, 0x00, 0x00, 0x88, 0x88, },    // 0x1010
		{ 0x28, 0x28, 0xa0, 0xa0, 0x00, 0x00, 0x88, 0x88, },    // 0x1011
		{ 0x88, 0x88, 0xa0, 0xa0, 0x28, 0x28, 0x00, 0x00, },    // 0x1100
		{ 0x28, 0x28, 0xa0, 0xa0, 0x00, 0x00, 0x88, 0x88, },    // 0x1101
		{ 0x20, 0x20, 0x20, 0x20, 0x80, 0x80, 0x80, 0x80, },    // 0x1110
		{ 0x28, 0x28, 0xa0, 0xa0, 0x00, 0x00, 0x88, 0x88, }     // 0x1111
	};

	uint8_t *rom = memregion("maincpu")->base();

	for (int a = 0; a < 0x4000; a++)
	{
		uint8_t src = rom[a];

		// pick the translation table from bits 0, 4, 8 and 12 of the address
		int i = BIT(a, 0) + (BIT(a, 4) << 1) + (BIT(a, 8) << 2) + (BIT(a, 12) << 3);

		// pick the offset in the table from bits 1, 3 and 5 of the source data
		int j = BIT(src, 1) + (BIT(src, 3) << 1) + (BIT(src, 5) << 2);

		// the bottom half of the translation table is the mirror image of the top
		if (BIT(src, 7)) j = 7 - j;

		// decode the ROM data
		rom[a] = src ^ data_xortable[i][j];
	}
}

/*************************************
 *
 *  Game drivers
 *
 *************************************/

//          rom       parent    machine   inp       state          init
GAME( 1980, puckman,  0,        pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "Namco",                             "Puck Man (Japan set 1)",                                   MACHINE_SUPPORTS_SAVE )
GAME( 1980, puckmanb, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "bootleg",                           "Puck Man (bootleg set 1)",                                 MACHINE_SUPPORTS_SAVE )
GAME( 1980, puckmanf, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack",                              "Puck Man (speedup hack)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1980, puckmanh, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "bootleg (Falcom?)",                 "Puck Man (bootleg set 2)",                                 MACHINE_SUPPORTS_SAVE )
GAME( 1980, pacman,   puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "Namco (Midway license)",            "Pac-Man (Midway)",                                         MACHINE_SUPPORTS_SAVE )
GAME( 1980, pacmanso, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "Namco (Sonic license)",             "Pac-Man (SegaSA / Sonic)",                                 MACHINE_SUPPORTS_SAVE ) // from SegaSA / Sonic, could be licensed, could be bootleg - it ignores the service mode credit settings despite listing them which is suspicious
GAME( 1980, pacmanvg, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "bootleg (Video Game SA)",           "Pac-Man (Video Game SA bootleg)",                          MACHINE_SUPPORTS_SAVE )
GAME( 1980, pacmanf,  puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack",                              "Pac-Man (Midway, speedup hack)",                           MACHINE_SUPPORTS_SAVE )
GAME( 1981, puckmod,  puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "Namco",                             "Puck Man (Japan set 2)",                                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacmod,   puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "Namco (Midway license)",            "Pac-Man (Midway, harder)",                                 MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacmanjpm,puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "bootleg (JPM)",                     "Pac-Man (JPM bootleg)",                                    MACHINE_SUPPORTS_SAVE ) // aka 'Muncher', UK bootleg, JPM later made fruit machines etc.
GAME( 1981, pacmanmr, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "bootleg (Model Racing)",            "Pac-Man (Model Racing bootleg)",                           MACHINE_SUPPORTS_SAVE )
GAME( 1980, pacmanpe, puckman,  pacman,   pacmanpe, pacman_state,  empty_init,    ROT90,  "bootleg (Petaco SA)",               "Come Come (Petaco SA bootleg of Puck Man)",                MACHINE_SUPPORTS_SAVE ) // might have a speed-up button, check
GAME( 1980, newpuc2,  puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack",                              "Newpuc2 (set 1)",                                          MACHINE_SUPPORTS_SAVE )
GAME( 1980, newpuc2b, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack",                              "Newpuc2 (set 2)",                                          MACHINE_SUPPORTS_SAVE )
GAME( 1980, newpuckx, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack",                              "New Puck-X",                                               MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacheart, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack",                              "Pac-Man (Hearts)",                                         MACHINE_SUPPORTS_SAVE )
GAME( 1981, bucaner,  puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack (Video Research)",             "Buccaneer (set 1)",                                        MACHINE_SUPPORTS_SAVE )
GAME( 1981, bucanera, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack (Video Research)",             "Buccaneer (set 2)",                                        MACHINE_SUPPORTS_SAVE )
GAME( 1981, hangly,   puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack (Igleck)",                     "Hangly-Man (set 1)",                                       MACHINE_SUPPORTS_SAVE )
GAME( 1981, hangly2,  puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack (Igleck)",                     "Hangly-Man (set 2)",                                       MACHINE_SUPPORTS_SAVE )
GAME( 1981, hangly3,  puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack (Igleck)",                     "Hangly-Man (set 3)",                                       MACHINE_SUPPORTS_SAVE )
GAME( 1981, baracuda, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack (Coinex)",                     "Barracuda",                                                MACHINE_SUPPORTS_SAVE )
GAME( 1981, popeyeman,puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack",                              "Popeye-Man",                                               MACHINE_SUPPORTS_SAVE )
GAME( 1980, pacuman,  puckman,  pacman,   pacuman,  pacman_state,  empty_init,    ROT90,  "bootleg (Recreativos Franco S.A.)", "Pacu-Man (Spanish bootleg of Puck Man)",                   MACHINE_SUPPORTS_SAVE ) // common bootleg in Spain, code is shifted a bit compared to the Puck Man sets. Title & Manufacturer info from cabinet/PCB, not displayed ingame
GAME( 1980, crockman, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "bootleg (Rene Pierre)",             "Crock-Man",                                                MACHINE_SUPPORTS_SAVE )
GAME( 1980, crockmnf, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "bootleg (Marti Colls)",             "Crock-Man (Marti Colls bootleg of Rene Pierre Crock-Man)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, puckmana, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "bootleg (Alca)",                    "Puck Man (Alca bootleg)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1982, joyman,   puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack",                              "Joyman",                                                   MACHINE_SUPPORTS_SAVE )
GAME( 1982, ctrpllrp, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "hack",                              "Caterpillar Pacman Hack",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1981, piranha,  puckman,  piranha,  mspacman, pacman_state,  init_eyes,     ROT90,  "GL (US Billiards license)",         "Piranha",                                                  MACHINE_SUPPORTS_SAVE )
GAME( 1981, piranhao, puckman,  piranha,  mspacman, pacman_state,  init_eyes,     ROT90,  "GL (US Billiards license)",         "Piranha (older)",                                          MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmab3,puckman,  piranha,  mspacman, pacman_state,  init_eyes,     ROT90,  "bootleg",                           "Ms. Pac-Man (bootleg, set 3)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1981, abscam,   puckman,  piranha,  mspacman, pacman_state,  init_eyes,     ROT90,  "GL (US Billiards license)",         "Abscam",                                                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, piranhah, puckman,  pacman,   mspacman, pacman_state,  empty_init,    ROT90,  "hack",                              "Piranha (hack)",                                           MACHINE_SUPPORTS_SAVE )
GAME( 1981, titanpac, puckman,  piranha,  mspacman, pacman_state,  init_eyes,     ROT90,  "hack (NSM)",                        "Titan (hack of Pac-Man)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1980, pacmanfm, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "bootleg (FAMARE S.A.)",             "Pac Man (FAMARE S.A. bootleg of Puck Man)",                MACHINE_SUPPORTS_SAVE )
GAME( 1980, pacmanug, puckman,  pacman,   pacman,   pacman_state,  empty_init,    ROT90,  "bootleg (U.Games)",                 "Pac Man (U.Games bootleg of Puck Man)",                    MACHINE_SUPPORTS_SAVE )

GAME( 1982, pacplus,  0,        pacman,   pacman,   pacman_state,  init_pacplus,  ROT90,  "Namco (Midway license)", "Pac-Man Plus", MACHINE_SUPPORTS_SAVE )

GAME( 1981, mspacman,    0,        mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "Midway / General Computer Corporation", "Ms. Pac-Man",                                      MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmnf,    mspacman, mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "hack",                                  "Ms. Pac-Man (speedup hack)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmat,    mspacman, mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "hack",                                  "Ms. Pac Attack",                                   MACHINE_SUPPORTS_SAVE )
GAME( 1989, msheartb,    mspacman, mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "hack (Two-Bit Score)",                  "Ms. Pac-Man Heart Burn",                           MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacgal2,     mspacman, mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "bootleg",                               "Pac-Gal (set 2)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmancr,  mspacman, mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "bootleg",                               "Ms. Pac-Man (bootleg on Crush Roller Hardware)",   MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmab,    mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg",                               "Ms. Pac-Man (bootleg, set 1)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmab2,   mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg",                               "Ms. Pac-Man (bootleg, set 2)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmab4,   mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg",                               "Ms. Pac-Man (bootleg, set 4)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmbe,    mspacman, woodpek,  mspacman, pacman_state,  init_mspacmbe,  ROT90,  "bootleg",                               "Ms. Pac-Man (bootleg, encrypted)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1982, mspacmbmc,   mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg (Marti Colls)",                 "Ms. Pac-Man (Marti Colls bootleg)",                MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmbn,    mspacman, woodpek,  mspacman, pacman_state,  init_pengomc1,  ROT90,  "bootleg (Novatronic)",                  "Ms. Pac-Man (Novatronic bootleg)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1982, mspacmanlai, mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg (Leisure and Allied)",          "Ms. Pac-Man (Leisure and Allied bootleg)",         MACHINE_SUPPORTS_SAVE )
GAME( 1983, mspacmane,   mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg (Elmac)",                       "Ms. Pac-Man (Elmac bootleg, earlier)",             MACHINE_SUPPORTS_SAVE )
GAME( 1984, mspacmane2,  mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg (Elmac)",                       "Ms. Pac-Man (Elmac bootleg, later)",               MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacii,     mspacman, mspacii,  mspacman, pacman_state,  init_mspacii,   ROT90,  "bootleg (Orca)",                        "Ms. Pac-Man II (Orca bootleg set 1)",              MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacii2,    mspacman, mspacii,  mspacman, pacman_state,  init_mspacii,   ROT90,  "bootleg (Orca)",                        "Ms. Pac-Man II (Orca bootleg set 2)",              MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacgal,      mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "hack",                                  "Pac-Gal (set 1)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacpls,    mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "hack",                                  "Ms. Pac-Man Plus",                                 MACHINE_SUPPORTS_SAVE )
GAME( 1992, mschamp,     mspacman, mschamp,  mschamp,  pacman_state,  init_mschamp,   ROT90,  "hack",                                  "Ms. Pacman Champion Edition / Zola-Puc Gal",       MACHINE_SUPPORTS_SAVE ) // Rayglo version
GAME( 1995, mschamps,    mspacman, mschamp,  mschamp,  pacman_state,  init_mschamp,   ROT90,  "hack",                                  "Ms. Pacman Champion Edition / Super Zola-Puc Gal", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspackpls,   mspacman, woodpek,  mspacman, pacman_state,  init_mspackpls, ROT90,  "hack",                                  "Miss Packman Plus",                                MACHINE_SUPPORTS_SAVE )
GAME( 1986, mspacmanhnc, mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "hack",                                  "Super Ms. Pac-Man (turbo hack, NVC284/NVC285 hardware)", MACHINE_SUPPORTS_SAVE )

// These bootlegs have MADE IN GREECE clearly visible and etched into the PCBs. They were very common in Spain with several operators having their own versions.
// Based on the PCBs and copyright dates shown they  were produced late 80s / early 90s. Usually they run a version of Ms. Pacman, but were sometimes converted back to regular Pac-Man
GAME( 198?, mspacmanbg,   mspacman, woodpek,     mspacman, pacman_state,  empty_init,   ROT90,  "bootleg",                 "Ms. Pac-Man ('Made in Greece' bootleg, set 1)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1997, mspacmanbg2,  mspacman, woodpek,     mspacman, pacman_state,  empty_init,   ROT90,  "bootleg",                 "Ms. Pac-Man ('Made in Greece' bootleg, set 2)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1992, mspacmanbgd,  mspacman, woodpek,     mspacman, pacman_state,  empty_init,   ROT90,  "bootleg (Datamat)",       "Miss Pukman ('Made in Greece' Datamat bootleg)",                     MACHINE_SUPPORTS_SAVE ) // shows 'Miss Pukman 1991/1992' but confirmed to be the bootleg distributed by Datamat
GAME( 1988, mspacmanbgf,  mspacman, woodpek_rbg, mspacman, pacman_state,  empty_init,   ROT90,  "bootleg (Fermin)",        "Mr Pac-Turbo ('Made in Greece' Fermin bootleg)",                     MACHINE_SUPPORTS_SAVE ) // Argentine bootleg with turbo speed. B-G color lines are intended swapped, showing a Ms PacMan purple/pink.
GAME( 1992, mspacmanblt,  mspacman, woodpek,     mspacman, pacman_state,  empty_init,   ROT90,  "bootleg (Triunvi)",       "Come-Cocos (Ms. Pac-Man) ('Made in Greece' Triunvi bootleg, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, mspacmanblt2, mspacman, woodpek,     mspacman, pacman_state,  empty_init,   ROT90,  "bootleg (Triunvi)",       "Come-Cocos (Ms. Pac-Man) ('Made in Greece' Triunvi bootleg, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, mspacmanbcc,  mspacman, woodpek,     mspacman, pacman_state,  empty_init,   ROT90,  "bootleg (Tecnausa)",      "Come-Cocos (Ms. Pac-Man) ('Made in Greece' Tecnausa bootleg)",       MACHINE_SUPPORTS_SAVE ) // ^ same PCB, also dated 1991, distributed by Tecnausa
GAME( 1991, mspacmanbhe,  mspacman, woodpek,     mspacman, pacman_state,  empty_init,   ROT90,  "bootleg (Herle SA)",      "Come-Cocos (Ms. Pac-Man) ('Made in Greece' Herle SA bootleg)",       MACHINE_SUPPORTS_SAVE ) // ^ same PCB
GAME( 1992, mspacmanbco,  mspacman, woodpek_rbg, mspacman, pacman_state,  empty_init,   ROT90,  "bootleg (Cocamatic)",     "Come-Cocos (Ms. Pac-Man) (Cocamatic bootleg)",                       MACHINE_SUPPORTS_SAVE ) // this PCB have swapped Blue and Green color lines (Ms.Pac-Man sprite should be pink), no "MADE IN GREECE" text at PCB
GAME( 1993, mspacmanbi,   mspacman, woodpek,     mspacman, pacman_state,  empty_init,   ROT90,  "bootleg (Impeuropex)",    "Ms. Pac-Man (Impeuropex bootleg)",                                   MACHINE_SUPPORTS_SAVE )
GAME( 1992, mspacmanbgc,  mspacman, woodpek,     mspacman, pacman_state,  empty_init,   ROT90,  "bootleg (Enavi)",         "Ms. Pac-Man ('Made in Greece' Enavi bootleg)",                       MACHINE_SUPPORTS_SAVE )
GAME( 198?, pacmansp,     puckman,  pacman,      pacmansp, pacman_state,  empty_init,   ROT90,  "bootleg (Video Game SA)", "Puck Man (Spanish, 'Made in Greece' bootleg)",                       MACHINE_SUPPORTS_SAVE ) // probably a further conversion of the mspacmanbg bootleg, still has some MS Pacman code + extra features

GAME( 1992, mspactwin,   0,         mspactwin, mspactwin, mspactwin_state, init_mspactwin, ROT90,  "hack (Susilu)",   "Ms Pac Man Twin (Argentina, set 1)",     MACHINE_SUPPORTS_SAVE )
GAME( 1992, mspactwina,  mspactwin, mspactwin, mspactwin, mspactwin_state, init_mspactwin, ROT90,  "hack (Susilu)",   "Ms Pac Man Twin (Argentina, set 2)",     MACHINE_SUPPORTS_SAVE )

GAME( 1989, clubpacm,    0,         clubpacm,  clubpacm,  clubpacm_state,  empty_init,     ROT90,  "hack (Miky SRL)", "Pacman Club / Club Lambada (Argentina)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, clubpacma,   clubpacm,  clubpacm,  clubpacma, clubpacm_state,  init_clubpacma, ROT90,  "hack (Miky SRL)", "Pacman Club (Argentina)",                MACHINE_SUPPORTS_SAVE )

GAME( 1985, jumpshot, 0,        pacman,   jumpshot, pacman_state,  init_jumpshot, ROT90,  "Bally Midway", "Jump Shot",                    MACHINE_SUPPORTS_SAVE )
GAME( 1985, jumpshotp,jumpshot, pacman,   jumpshotp,pacman_state,  init_jumpshot, ROT90,  "Bally Midway", "Jump Shot Engineering Sample", MACHINE_SUPPORTS_SAVE )

GAME( 1985, shootbul, 0,        pacman,   shootbul, pacman_state,  init_jumpshot, ROT90,  "Bally Midway", "Shoot the Bull", MACHINE_SUPPORTS_SAVE )

GAME( 1981, crush,    0,        korosuke, maketrax, pacman_state,  init_maketrax, ROT90,  "Alpha Denshi Co. / Kural Samno Electric, Ltd.", "Crush Roller (set 1)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush2,   crush,    crush2,   maketrax, pacman_state,  empty_init,    ROT90,  "Alpha Denshi Co. / Kural Esco Electric, Ltd.",  "Crush Roller (set 2)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush3,   crush,    korosuke, maketrax, pacman_state,  init_maketrax, ROT90,  "Alpha Denshi Co. / Kural Electric, Ltd.",       "Crush Roller (set 3)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush4,   crush,    crush2,   maketrax, pacman_state,  init_eyes,     ROT90,  "Alpha Denshi Co. / Kural Electric, Ltd.",       "Crush Roller (set 4)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush5,   crush,    crush4,   crush4,   pacman_state,  empty_init,    ROT90,  "Alpha Denshi Co. / Kural TWT",                  "Crush Roller (set 5)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, maketrax, crush,    korosuke, maketrax, pacman_state,  init_maketrax, ROT270, "Alpha Denshi Co. / Kural (Williams license)",   "Make Trax (US set 1)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, maketrxb, crush,    korosuke, maketrax, pacman_state,  init_maketrax, ROT270, "Alpha Denshi Co. / Kural (Williams license)",   "Make Trax (US set 2)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, korosuke, crush,    korosuke, korosuke, pacman_state,  init_maketrax, ROT90,  "Alpha Denshi Co. / Kural Electric, Ltd.",       "Korosuke Roller (Japan)",                MACHINE_SUPPORTS_SAVE ) // ADK considers it a sequel?
GAME( 1981, crushrlf, crush,    crush2,   maketrax, pacman_state,  empty_init,    ROT90,  "bootleg",                                       "Crush Roller (Famare SA PCB)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushbl,  crush,    crush2,   maketrax, pacman_state,  empty_init,    ROT90,  "bootleg",                                       "Crush Roller (bootleg set 1)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushbl2, crush,    korosuke, mbrush,   pacman_state,  init_mbrush,   ROT90,  "bootleg",                                       "Crush Roller (bootleg set 2)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushbl3, crush,    korosuke, crushbl3, pacman_state,  init_maketrax, ROT90,  "bootleg",                                       "Crush Roller (bootleg set 3)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushs,   crush,    crushs,   crushs,   pacman_state,  empty_init,    ROT90,  "bootleg (Sidam)",                               "Crush Roller (bootleg set 4)",           MACHINE_SUPPORTS_SAVE ) // Sidam PCB, no Sidam text
GAME( 1981, mbrush,   crush,    korosuke, mbrush,   pacman_state,  init_mbrush,   ROT90,  "bootleg (Olympia)",                             "Magic Brush (bootleg of Crush Roller)",  MACHINE_SUPPORTS_SAVE )
GAME( 1981, paintrlr, crush,    crush2,   paintrlr, pacman_state,  empty_init,    ROT90,  "bootleg",                                       "Paint Roller (bootleg of Crush Roller)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, painter,  crush,    crush2,   paintrlr, pacman_state,  empty_init,    ROT90,  "hack (Monshine Ent. Co.)",                      "Painter (hack of Crush Roller)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // currently shows Paintei due to bad sprite ROM

GAME( 1982, eyes,     0,        pacman,   eyes,     pacman_state,  init_eyes,     ROT90,  "Techstar (Rock-Ola license)", "Eyes (US set 1)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1982, eyes2,    eyes,     pacman,   eyes,     pacman_state,  init_eyes,     ROT90,  "Techstar (Rock-Ola license)", "Eyes (US set 2)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1982, eyesb,    eyes,     pacman,   eyes,     pacman_state,  init_eyes,     ROT90,  "bootleg",                     "Eyes (bootleg set 1)",            MACHINE_SUPPORTS_SAVE )
GAME( 1982, eyeszac,  eyes,     pacman,   eyes,     pacman_state,  init_eyes,     ROT90,  "Techstar (Zaccaria license)", "Eyes (Italy)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1982, eyeszacb, eyes,     pacman,   eyes,     pacman_state,  empty_init,    ROT90,  "bootleg",                     "Eyes (bootleg set 2, decrypted)", MACHINE_SUPPORTS_SAVE ) // based on Zaccaria version

GAME( 1983, mrtnt,    0,        pacman,   mrtnt,    pacman_state,  init_eyes,     ROT90,  "Techstar (Telko license)", "Mr. TNT", MACHINE_SUPPORTS_SAVE )
GAME( 1983, gorkans,  mrtnt,    pacman,   mrtnt,    pacman_state,  empty_init,    ROT90,  "Techstar",                 "Gorkans", MACHINE_SUPPORTS_SAVE )

GAME( 1985, lizwiz,   0,        woodpek,  lizwiz,   pacman_state,  empty_init,    ROT90,  "Techstar (Sunn license)", "Lizard Wizard", MACHINE_SUPPORTS_SAVE )

GAME( 1983, eggor,    0,        pacman,   mrtnt,    pacman_state,  init_eyes,     ROT90,  "Telko", "Eggor", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1983, birdiy,   0,        birdiy,   birdiy,   pacman_state,  empty_init,    ROT270, "Mama Top", "Birdiy", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1981, woodpeck, 0,        woodpek,  woodpek,  pacman_state,  init_woodpek,  ROT90,  "Amenip (Palcom Queen River)", "Woodpecker (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, woodpeca, woodpeck, woodpek,  woodpek,  pacman_state,  init_woodpek,  ROT90,  "Amenip",                      "Woodpecker (set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, nmouse,   0,        nmouse,   nmouse,   pacman_state,  init_eyes,     ROT90,  "Amenip (Palcom Queen River)", "Naughty Mouse (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, nmouseb,  nmouse,   nmouse,   nmouse,   pacman_state,  init_eyes,     ROT90,  "Amenip Nova Games Ltd.",      "Naughty Mouse (set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, ponpoko,  0,        woodpek,  ponpoko,  pacman_state,  init_ponpoko,  ROT0,   "Sigma Enterprises Inc.",                        "Ponpoko",                              MACHINE_SUPPORTS_SAVE )
GAME( 1982, ponpokov, ponpoko,  woodpek,  ponpoko,  pacman_state,  init_ponpoko,  ROT0,   "Sigma Enterprises Inc. (Venture Line license)", "Ponpoko (Venture Line)",               MACHINE_SUPPORTS_SAVE )
GAME( 1982, candory,  ponpoko,  woodpek,  ponpoko,  pacman_state,  init_ponpoko,  ROT0,   "bootleg",                                       "Candory (Ponpoko bootleg with Mario)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, alibaba,  0,        alibaba,  alibaba,  pacman_state,  empty_init,    ROT90,  "Sega",    "Ali Baba and 40 Thieves",          MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1982, alibabab, alibaba,  alibaba,  alibaba,  pacman_state,  empty_init,    ROT90,  "bootleg", "Mustafa and 40 Thieves (bootleg)", MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )

GAME( 1982, dremshpr, 0,        dremshpr, dremshpr, pacman_state,  empty_init,    ROT270, "Sanritsu", "Dream Shopper", MACHINE_SUPPORTS_SAVE )

GAME( 1983, vanvan,   0,        vanvan,   vanvan,   pacman_state,  empty_init,    ROT270, "Sanritsu",                     "Van-Van Car",                  MACHINE_SUPPORTS_SAVE )
GAME( 1983, vanvank,  vanvan,   vanvan,   vanvank,  pacman_state,  empty_init,    ROT270, "Sanritsu (Karateco license?)", "Van-Van Car (Karateco set 1)", MACHINE_SUPPORTS_SAVE ) // or bootleg?
GAME( 1983, vanvanb,  vanvan,   vanvan,   vanvank,  pacman_state,  empty_init,    ROT270, "Sanritsu (Karateco license?)", "Van-Van Car (Karateco set 2)", MACHINE_SUPPORTS_SAVE ) // "

GAME( 1983, bwcasino, 0,        acitya,   bwcasino, epospm_state,  empty_init,    ROT90,  "Epos Corporation", "Boardwalk Casino",     MACHINE_SUPPORTS_SAVE )
GAME( 1983, acitya,   bwcasino, acitya,   acitya,   epospm_state,  empty_init,    ROT90,  "Epos Corporation", "Atlantic City Action", MACHINE_SUPPORTS_SAVE )

GAME( 1983, theglobp, suprglob, theglobp, theglobp, epospm_state,  empty_init,    ROT90,  "Epos Corporation",         "The Glob (Pac-Man hardware, set 1)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1983, theglobpa,suprglob, theglobp, theglobp, epospm_state,  empty_init,    ROT90,  "Epos Corporation",         "The Glob (Pac-Man hardware, set 2)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1983, sprglobp, suprglob, theglobp, theglobp, epospm_state,  empty_init,    ROT90,  "Epos Corporation",         "Super Glob (Pac-Man hardware)",                              MACHINE_SUPPORTS_SAVE )
GAME( 1985, sprglobp2,suprglob, pacman,   theglobp, epospm_state,  init_sprglobp2,ROT90,  "bootleg (Elsys Software)", "Super Glob (Pac-Man hardware, bootleg)",                     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // encrypted
GAME( 1984, sprglbpg, suprglob, pacman,   theglobp, epospm_state,  empty_init,    ROT90,  "bootleg (Software Labor)", "Super Glob (Pac-Man hardware, German bootleg)",              MACHINE_SUPPORTS_SAVE )
GAME( 1983, theglobme,suprglob, woodpek,  theglobp, epospm_state,  empty_init,    ROT90,  "Magic Electronics Inc.",   "The Glob (Pacman hardware, Magic Electronics Inc. license)", MACHINE_SUPPORTS_SAVE )

GAME( 1984, beastfp,  suprglob, theglobp, theglobp, epospm_state,  empty_init,    ROT90,  "Epos Corporation",         "Beastie Feastie (Pac-Man conversion)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1984, eeekkp,   eeekk,    eeekkp,   eeekkp,   epospm_state,  empty_init,    ROT90,  "Epos Corporation",         "Eeekk! (Pac-Man conversion)",                                MACHINE_SUPPORTS_SAVE )

GAME( 1984, drivfrcp, 0,        drivfrcp, drivfrcp, pacman_state,  init_drivfrcp, ROT90,  "Shinkai Inc. (Magic Electronics Inc. license)", "Driving Force (Pac-Man conversion)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, 8bpm,     8ballact, _8bpm,    8bpm,     pacman_state,  init_8bpm,     ROT90,  "Seatongrove UK, Ltd. (Magic Electronics USA license)", "Eight Ball Action (Pac-Man conversion)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, porky,    0,        porky,    porky,    pacman_state,  init_porky,    ROT90,  "Shinkai Inc. (Magic Electronics Inc. license)", "Porky", MACHINE_SUPPORTS_SAVE )

GAME( 1986, rocktrv2, 0,        rocktrv2, rocktrv2, pacman_state,  init_rocktrv2, ROT90,  "Triumph Software Inc.", "MTV Rock-N-Roll Trivia (Part 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1986, bigbucks, 0,        bigbucks, bigbucks, pacman_state,  empty_init,    ROT90,  "Dynasoft Inc.", "Big Bucks", MACHINE_SUPPORTS_SAVE )

GAME( 1983, numcrash, 0,        numcrash, numcrash, pacman_state,  empty_init,    ROT90,  "Hanshin Goraku / Peni", "Number Crash", MACHINE_SUPPORTS_SAVE ) // "Peni soft" related?

GAME( 1985, cannonbp, 0,        cannonbp, cannonbp, pacman_state,  empty_init,    ROT90,  "Novomatic", "Cannon Ball (Pac-Man Hardware)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )

GAME( 1999, superabc, 0,        superabc, superabc, pacman_state,  init_superabc, ROT90,  "hack (Two-Bit Score)", "Super ABC (Pac-Man multigame kit, Sep. 03 1999)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, superabco,superabc, superabc, superabc, pacman_state,  init_superabc, ROT90,  "hack (Two-Bit Score)", "Super ABC (Pac-Man multigame kit, Mar. 08 1999)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, pengojpm, pengo,    pengojpm, pengojpm, pacman_state,  empty_init,    ROT90,  "bootleg",               "Pengo (bootleg on Pac-Man hardware, set 1)",             MACHINE_SUPPORTS_SAVE ) // conversion of pacmanjpm board with wire mods
GAME( 1981, pengopac, pengo,    pengojpm, pengojpm, pacman_state,  empty_init,    ROT90,  "bootleg",               "Pengo (bootleg on Pac-Man hardware, set 2)",             MACHINE_SUPPORTS_SAVE ) // different conversion?
GAME( 1982, pengomc1, pengo,    pengojpm, pengojpm, pacman_state,  init_pengomc1, ROT90,  "bootleg (Marti Colls)", "Pengo (Marti Colls bootleg on Pac-Man hardware, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, pengomc2, pengo,    pengojpm, pengojpm, pacman_state,  empty_init,    ROT90,  "bootleg (Marti Colls)", "Pengo (Marti Colls bootleg on Pac-Man hardware, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, pinguinos,pengo,    pengojpm, pengojpm, pacman_state,  empty_init,    ROT90,  "bootleg (Aincar)",      "Pinguinos (Spanish bootleg on Pac-Man hardware)",        MACHINE_SUPPORTS_SAVE )
