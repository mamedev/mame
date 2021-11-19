// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Slapstic decoding helper

**************************************************************************

    Atari Slapstic FAQ
    Version 1.12
    by Aaron Giles and Frank Palazzolo
    9/12/2002


    What is a slapstic?

    The slapstic was a security chip made by Atari, which was used for
    bank switching and security in several coin-operated video games from
    1984 through 1990.  It means "SLA Protection from Software Thievery IC",
    where SLA is a Storage/Logic Array, an early form of gate array.


    What is a SLOOP?

    The SLOOP (or "SLOOPstic") is a follow-on chip to the slapstic. It
    provides a similar type of security, but is programmed onto a GAL6001,
    rather than a custom part. It was created because Atari was running
    out of slapstics to use in their games, and the original masks for the
    slapstic had been lost by the company that manufactured them. A separate
    FAQ for this chip is planned for the future.


    How do I identify a slapstic chip on my board?

    Look for a small, socketed 20-pin DIP on the board. The number on
    the chip will be 137412-1xx.


    Are slapstic chips interchangeable?

    Sadly, no. They were designed to prevent operators from burning
    new EPROMs and "upgrading" their PCBs to a new game without buying
    the necessary kits from Atari. For example, the five System 1 games
    each used a different slapstic, so that you couldn't take, say,
    a <b>Marble Madness</b> machine, burn new EPROMs, and convert it into
    an <b>Indiana Jones</b>.

    That said, however, there are two pairs of the slapstics that appear
    to be functionally identical, despite the fact that they have
    different part numbers:

        137412-103 (<b>Marble Madness</b>) appears to be functionally identical
            to 137412-110 (<b>Road Blasters</b> & <b>APB</b>)

        137412-106 (<b>Gauntlet II</b>) appears to be functionally identical
            to 137412-109 (<b>Championship Sprint</b>)

    Note, however, that I have not tried these swaps to confirm that they
    work. Your mileage may vary.


    How many different slapstics are there?

    All told, a total of 13 actual slapstics have been found. However,
    there are gaps in the numbering which indicate that more may exist.


    Do all slapstics work the same?

    In general, yes. However, matters are complicated by the existence
    of multiple revisions of the chip design:

        SLAPSTIC    Part #137412-101 through 137412-110
        SLAPSTIC-2  Part #137412-111 through 137412-118

    In the simplest case, both revs act the same. However, they differ
    in how the more complex modes of operation are used.


    How is the slapstic connected to the game?

    The slapstic generally sits between the CPU's address bus and one
    of the program ROMs. Here's a pinout:

            A9   1 +-v-+ 20  A8
            A10  2 |   | 19  A7
            A11  3 |   | 18  A6
            A12  4 |   | 17  A5
            A13  5 |   | 16  A4
            /CS  6 |   | 15  A3
            CLK  7 |   | 14  A2
            VCC  8 |   | 13  A1
            BS1  9 |   | 12  A0
            BS0 10 +---+ 11 GND

    A0-A13 are the address lines from the CPU. CLK and /CS together
    trigger a state change. BS0 and BS1 are the bank select outputs,
    which usually connect to the protected program ROM in place of
    two address lines (traditionally A12 and A13).

    Most slapstics were used on 68000 or T-11 based games, which had
    a 16-bit address bus. This meant that A0-A13 on the slapstic were
    generally connected to A1-A14 on the CPU. However, two 8-bit
    games (Tetris and Empire Strikes Back) used the slapstic as well.
    This slapstic (#101) has a slightly different pinout, though it
    operates similarly to the others in its class.

            A8   1 +-v-+ 20  A7
            A9   2 |   | 19  A6
            A10  3 |   | 18  A5
            A11  4 |   | 17  A4
            A12  5 |   | 16  A3
            /CS  6 |   | 15  A2
            CLK  7 |   | 14  A1
            VCC  8 |   | 13  A0
            /BS1 9 |   | 12 GND
            BS1 10 +---+ 11 BS0


    Which games used slapstics?

        137412-101  Empire Strikes Back
        137412-101  Tetris
        137412-103  Marble Madness
        137412-104  Gauntlet
        137412-105  Paperboy
        137412-105  Indiana Jones & the Temple of Doom
        137412-106  Gauntlet II
        137412-107  2-Player Gauntlet
        137412-107  Peter Packrat
        137412-107  720 Degrees
        137412-107  Xybots
        137412-108  Road Runner
        137412-108  Super Sprint
        137412-109  Championship Sprint
        137412-109  Road Blasters (some versions)
        137412-110  Road Blasters
        137412-110  APB
        137412-111  Pit Fighter (Aug 09, 1990 to Aug 22, 1990)
        137412-112  Pit Fighter (Aug 22, 1990 to Oct 01, 1990)
        137412-113  Pit Fighter (Oct 09, 1990 to Oct 12, 1990)
        137412-114  Pit Fighter (Nov 01, 1990 and later)
        137412-115  Race Drivin' DSK board
        137412-116  Hydra
        137412-116  Tournament Cyberball 2072
        137412-117  Race Drivin' main board
        137412-118  Rampart
        137412-118  Vindicators Part II


    How does the slapstic work?

    On power-up, the slapstic starts by pointing to bank 0 or bank 3.
    After that, certain sequences of addresses will trigger a bankswitch.
    Each sequence begins with an access to location $0000, followed by one
    or more special addresses.

    Each slapstic has a 'simple' mode of bankswitching, consisting of an
    access to $0000 followed by an access to one of four bank addresses.
    Other accesses are allowed in between these two accesses without
    affecting the outcome.

    Additionally, each slapstic has a trickier variant of the
    bankswitching, which requires an access to $0000, followed by accesses
    to two specific addresses, followed by one of four alternate bank
    addresses. All three accesses following the $0000 must occur in
    sequence with no interruptions, or else the sequence is invalidated.

    Finally, each slapstic has a mechanism for modifying the value of the
    current bank. Earlier chips (101-110) allowed you to twiddle the
    specific bits of the bank number, clearing or setting bits 0 and 1
    independently. Later chips (111-118) provided a mechanism of adding
    1, 2, or 3 to the number of the current bank.

    One important detail is that some accesses must be done with CS=0,
    while others don't care.  CS=0 usually means the access is in the
    slapstic banked region.  Specifically:
      - on 101 and 102, the 2nd alt access must be done outside of
        the bank region
      - on 103 to 110, the 1st alt access can be done anywhere
      - on 111 to 118, the 1st and 3rd alt access can be done anywhere

    These out-of-range accesses pose technical difficulties we're not fully
    handling yet.  Similarly, accesses that must be done in sequence get
    broken by an out-of-range access.

    Surprisingly, the slapstic appears to have used DRAM cells to store
    the current bank. After 5 or 6 seconds without a clock, the chip
    reverts to the default bank, with the chip reset (bank select
    addresses are enabled). Typically, the clock is connnected to the
    cpu memory access line (AS on the 68000 for instance), accessing
    often enough to avoid the problem.

    For full details, see the MAME source code.

*************************************************************************/


#include "emu.h"
#include "machine/slapstic.h"

#include "cpu/m68000/m68000.h"

DEFINE_DEVICE_TYPE(SLAPSTIC, atari_slapstic_device, "slapstic", "Atari Slapstic")

atari_slapstic_device::atari_slapstic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SLAPSTIC, tag, owner, clock),
	m_bank(*this, finder_base::DUMMY_TAG),
	m_view(nullptr),
	m_space(*this, finder_base::DUMMY_TAG, -1),
	m_start(0),
	m_end(0),
	m_mirror(0),
	m_saved_state(S_IDLE),
	m_current_bank(0),
	m_loaded_bank(0)
{
}

/*************************************
 *
 *  Slapstic definitions
 *
 *************************************/

#define UNKNOWN 0xffff
#define NO_BITWISE          \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN }
#define NO_ADDITIVE         \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN },    \
	{ UNKNOWN,UNKNOWN }


/* slapstic 137412-101: Empire Strikes Back/Tetris */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic101 =
{
	/* basic banking */
	3,                              /* starting bank */
	{ 0x0080,0x0090,0x00a0,0x00b0 },/* bank select values */

	/* alternate banking */
	{ 0x1f00,0x1e00 },              /* 1st mask/value in sequence */
	{ 0x1fff,0x1fff },              /* 2nd mask/value in sequence, *outside* of the range */
	{ 0x1ffc,0x1b5c },              /* 3rd mask/value in sequence */
	{ 0x1fcf,0x0080 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x1ff0,0x1540 },              /* 1st mask/value */
	{ 0x1fcf,0x0080 },              /* 2nd mask/value */
	{ 0x1ff3,0x1540 },              /* clear bit 0 value on odd /   set bit 1 on even */
	{ 0x1ff3,0x1541 },              /*   set bit 0 value on odd / clear bit 1 on even */
	{ 0x1ff3,0x1542 },              /* clear bit 1 value on odd /   set bit 0 on even */
	{ 0x1ff3,0x1543 },              /*   set bit 1 value on odd / clear bit 0 on even */
	{ 0x1ff8,0x1550 },              /* final mask/value */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-103: Marble Madness */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic103 =
{
	/* basic banking */
	3,                              /* starting bank */
	{ 0x0040,0x0050,0x0060,0x0070 },/* bank select values */

	/* alternate banking */
	// Real values, to be worked on later
	{ 0x007f,0x002d },              /* 1st mask/value in sequence */
	{ 0x3fff,0x3d14 },              /* 2nd mask/value in sequence */
	{ 0x3ffc,0x3d24 },              /* 3rd mask/value in sequence */
	{ 0x3fcf,0x0040 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x34c0 },              /* 1st mask/value in sequence */
	{ 0x3fcf,0x0040 },              /* 2nd mask/value */
	{ 0x3ff3,0x34c0 },              /* clear bit 0 value on odd /   set bit 1 on even */
	{ 0x3ff3,0x34c1 },              /*   set bit 0 value on odd / clear bit 1 on even */
	{ 0x3ff3,0x34c2 },              /* clear bit 1 value on odd /   set bit 0 on even */
	{ 0x3ff3,0x34c3 },              /*   set bit 1 value on odd / clear bit 0 on even */
	{ 0x3ff8,0x34d0 },              /* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-104: Gauntlet */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic104 =
{
	/* basic banking */
	3,                              /* starting bank */
	{ 0x0020,0x0028,0x0030,0x0038 },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0069 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x3735 },              /* 2nd mask/value in sequence */
	{ 0x3ffc,0x3764 },              /* 3rd mask/value in sequence */
	{ 0x3fe7,0x0020 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x3d90 },              /* 1st mask/value in sequence */
	{ 0x3fe7,0x0020 },              /* 2nd mask/value */
	{ 0x3ff3,0x3d90 },              /* clear bit 0 value on odd /   set bit 1 on even */
	{ 0x3ff3,0x3d91 },              /*   set bit 0 value on odd / clear bit 1 on even */
	{ 0x3ff3,0x3d92 },              /* clear bit 1 value on odd /   set bit 0 on even */
	{ 0x3ff3,0x3d93 },              /*   set bit 1 value on odd / clear bit 0 on even */
	{ 0x3ff8,0x3da0 },              /* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-105: Indiana Jones/Paperboy */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic105 =
{
	/* basic banking */
	3,                              /* starting bank */
	{ 0x0010,0x0014,0x0018,0x001c },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x003d },              /* 1st mask/value in sequence */
	{ 0x3fff,0x0092 },              /* 2nd mask/value in sequence */
	{ 0x3ffc,0x00a4 },              /* 3rd mask/value in sequence */
	{ 0x3ff3,0x0010 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x35b0 },              /* 1st mask/value in sequence */
	{ 0x3ff3,0x0010 },              /* 2nd mask/value */
	{ 0x3ff3,0x35b0 },              /* clear bit 0 value on odd /   set bit 1 on even */
	{ 0x3ff3,0x35b1 },              /*   set bit 0 value on odd / clear bit 1 on even */
	{ 0x3ff3,0x35b2 },              /* clear bit 1 value on odd /   set bit 0 on even */
	{ 0x3ff3,0x35b3 },              /*   set bit 1 value on odd / clear bit 0 on even */
	{ 0x3ff8,0x35c0 },              /* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-106: Gauntlet II */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic106 =
{
	/* basic banking */
	3,                              /* starting bank */
	{ 0x0008,0x000a,0x000c,0x000e },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x002b },              /* 1st mask/value in sequence */
	{ 0x3fff,0x0052 },              /* 2nd mask/value in sequence */
	{ 0x3ffc,0x0064 },              /* 3rd mask/value in sequence */
	{ 0x3ff9,0x0008 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x3da0 },              /* 1st mask/value in sequence */
	{ 0x3ff9,0x0008 },              /* 2nd mask/value */
	{ 0x3ff3,0x3da0 },              /* clear bit 0 value on odd /   set bit 1 on even */
	{ 0x3ff3,0x3da1 },              /*   set bit 0 value on odd / clear bit 1 on even */
	{ 0x3ff3,0x3da2 },              /* clear bit 1 value on odd /   set bit 0 on even */
	{ 0x3ff3,0x3da3 },              /*   set bit 1 value on odd / clear bit 0 on even */
	{ 0x3ff8,0x3db0 },              /* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-107: Peter Packrat/Xybots/2p Gauntlet/720 */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic107 =
{
	/* basic banking */
	3,                              /* starting bank */
	{ 0x0018,0x001a,0x001c,0x001e },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x006b },              /* 1st mask/value in sequence */
	{ 0x3fff,0x3d52 },              /* 2nd mask/value in sequence */
	{ 0x3ffc,0x3d64 },              /* 3rd mask/value in sequence */
	{ 0x3ff9,0x0018 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x00a0 },              /* 1st mask/value in sequence */
	{ 0x3ff9,0x0018 },              /* 2nd mask/value */
	{ 0x3ff3,0x00a0 },              /* clear bit 0 value on odd /   set bit 1 on even */
	{ 0x3ff3,0x00a1 },              /*   set bit 0 value on odd / clear bit 1 on even */
	{ 0x3ff3,0x00a2 },              /* clear bit 1 value on odd /   set bit 0 on even */
	{ 0x3ff3,0x00a3 },              /*   set bit 1 value on odd / clear bit 0 on even */
	{ 0x3ff8,0x00b0 },              /* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-108: Road Runner/Super Sprint */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic108 =
{
	/* basic banking */
	3,                              /* starting bank */
	{ 0x0028,0x002a,0x002c,0x002e },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x001f },              /* 1st mask/value in sequence */
	{ 0x3fff,0x3772 },              /* 2nd mask/value in sequence */
	{ 0x3ffc,0x3764 },              /* 3rd mask/value in sequence */
	{ 0x3ff9,0x0028 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x0060 },              /* 1st mask/value in sequence */
	{ 0x3ff9,0x0028 },              /* 2nd mask/value */
	{ 0x3ff3,0x0060 },              /* clear bit 0 value on odd /   set bit 1 on even */
	{ 0x3ff3,0x0061 },              /*   set bit 0 value on odd / clear bit 1 on even */
	{ 0x3ff3,0x0062 },              /* clear bit 1 value on odd /   set bit 0 on even */
	{ 0x3ff3,0x0063 },              /*   set bit 1 value on odd / clear bit 0 on even */
	{ 0x3ff8,0x0070 },              /* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-109: Championship Sprint/Road Blasters */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic109 =
{
	/* basic banking */
	3,                              /* starting bank */
	{ 0x0008,0x000a,0x000c,0x000e },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x002b },              /* 1st mask/value in sequence */
	{ 0x3fff,0x0052 },              /* 2nd mask/value in sequence */
	{ 0x3ffc,0x0064 },              /* 3rd mask/value in sequence */
	{ 0x3ff9,0x0008 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x3da0 },              /* 1st mask/value in sequence */
	{ 0x3ff9,0x0008 },              /* 2nd mask/value */
	{ 0x3ff3,0x3da0 },              /* clear bit 0 value on odd /   set bit 1 on even */
	{ 0x3ff3,0x3da1 },              /*   set bit 0 value on odd / clear bit 1 on even */
	{ 0x3ff3,0x3da2 },              /* clear bit 1 value on odd /   set bit 0 on even */
	{ 0x3ff3,0x3da3 },              /*   set bit 1 value on odd / clear bit 0 on even */
	{ 0x3ff8,0x3db0 },              /* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-110: Road Blasters/APB */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic110 =
{
	/* basic banking */
	3,                              /* starting bank */
	{ 0x0040,0x0050,0x0060,0x0070 },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x002d },              /* 1st mask/value in sequence */
	{ 0x3fff,0x3d14 },              /* 2nd mask/value in sequence */
	{ 0x3ffc,0x3d24 },              /* 3rd mask/value in sequence */
	{ 0x3fcf,0x0040 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x34c0 },              /* 1st mask/value in sequence */
	{ 0x3fcf,0x0040 },              /* 2nd mask/value */
	{ 0x3ff3,0x34c0 },              /* clear bit 0 value on odd /   set bit 1 on even */
	{ 0x3ff3,0x34c1 },              /*   set bit 0 value on odd / clear bit 1 on even */
	{ 0x3ff3,0x34c2 },              /* clear bit 1 value on odd /   set bit 0 on even */
	{ 0x3ff3,0x34c3 },              /*   set bit 1 value on odd / clear bit 0 on even */
	{ 0x3ff8,0x34d0 },              /* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};



/*************************************
 *
 *  Slapstic-2 definitions
 *
 *************************************/

/* slapstic 137412-111: Pit Fighter (Aug 09, 1990 to Aug 22, 1990) */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic111 =
{
	/* basic banking */
	0,                              /* starting bank */
	{ 0x0042,0x0052,0x0062,0x0072 },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x000a },              /* 1st mask/value in sequence */
	{ 0x3fff,0x28a4 },              /* 2nd mask/value in sequence */
	{ 0x0784,0x0080 },              /* 3rd mask/value in sequence */
	{ 0x3fcf,0x0042 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x00a1 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x00a2 },              /* 2nd mask/value in sequence */
	{ 0x3c4f,0x284d },              /* +1 mask/value */
	{ 0x3a5f,0x285d },              /* +2 mask/value */
	{ 0x3ff8,0x2800 }               /* final mask/value in sequence */
};


/* slapstic 137412-112: Pit Fighter (Aug 22, 1990 to Oct 01, 1990) */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic112 =
{
	/* basic banking */
	0,                              /* starting bank */
	{ 0x002c,0x003c,0x006c,0x007c },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0014 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x29a0 },              /* 2nd mask/value in sequence */
	{ 0x0073,0x0010 },              /* 3rd mask/value in sequence */
	{ 0x3faf,0x002c },              /* 4th mask/value in sequence */
	2,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x2dce },              /* 1st mask/value in sequence */
	{ 0x3fff,0x2dcf },              /* 2nd mask/value in sequence */
	{ 0x3def,0x15e2 },              /* +1 mask/value */
	{ 0x3fbf,0x15a2 },              /* +2 mask/value */
	{ 0x3ffc,0x1450 }               /* final mask/value in sequence */
};


/* slapstic 137412-113: Pit Fighter (Oct 09, 1990 to Oct 12, 1990) */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic113 =
{
	/* basic banking */
	0,                              /* starting bank */
	{ 0x0008,0x0018,0x0028,0x0038 },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0059 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x11a5 },              /* 2nd mask/value in sequence */
	{ 0x0860,0x0800 },              /* 3rd mask/value in sequence */
	{ 0x3fcf,0x0008 },              /* 4th mask/value in sequence */
	3,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x049b },              /* 1st mask/value in sequence */
	{ 0x3fff,0x049c },              /* 2nd mask/value in sequence */
	{ 0x3fcf,0x3ec7 },              /* +1 mask/value */
	{ 0x3edf,0x3ed7 },              /* +2 mask/value */
	{ 0x3fff,0x3fb2 }               /* final mask/value in sequence */
};


/* slapstic 137412-114: Pit Fighter (Nov 01, 1990 and later) */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic114 =
{
	/* basic banking */
	0,                              /* starting bank */
	{ 0x0040,0x0048,0x0050,0x0058 },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0016 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x24de },              /* 2nd mask/value in sequence */
	{ 0x3871,0x0000 },              /* 3rd mask/value in sequence */
	{ 0x3fe7,0x0040 },              /* 4th mask/value in sequence */
	1,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x0ab7 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x0ab8 },              /* 2nd mask/value in sequence */
	{ 0x3f63,0x0d40 },              /* +1 mask/value */
	{ 0x3fd9,0x0dc8 },              /* +2 mask/value */
	{ 0x3fff,0x0ab0 }               /* final mask/value in sequence */
};


/* slapstic 137412-115: Race Drivin' DSK board */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic115 =
{
	/* basic banking */
	0,                              /* starting bank */
	{ 0x0020,0x0022,0x0024,0x0026 },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0054 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x3e01 },              /* 2nd mask/value in sequence */
	{ 0x3879,0x0029 },              /* 3rd mask/value in sequence */
	{ 0x3ff9,0x0020 },              /* 4th mask/value in sequence */
	1,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x2591 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x2592 },              /* 2nd mask/value in sequence */
	{ 0x3fe6,0x3402 },              /* +1 mask/value */
	{ 0x3fb4,0x3410 },              /* +2 mask/value */
	{ 0x3fff,0x34a2 }               /* final mask/value in sequence */
};


/* slapstic 137412-116: Hydra */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic116 =
{
	/* basic banking */
	0,                              /* starting bank */
	{ 0x0044,0x004c,0x0054,0x005c },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0069 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x2bab },              /* 2nd mask/value in sequence */
	{ 0x387c,0x0808 },              /* 3rd mask/value in sequence */
	{ 0x3fe7,0x0044 },              /* 4th mask/value in sequence */
	0,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x3f7c },              /* 1st mask/value in sequence */
	{ 0x3fff,0x3f7d },              /* 2nd mask/value in sequence */
	{ 0x3db2,0x3c12 },              /* +1 mask/value */
	{ 0x3fe3,0x3e43 },              /* +2 mask/value */
	{ 0x3fff,0x2ba8 }               /* final mask/value in sequence */
};


/* slapstic 137412-117: Race Drivin' main board */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic117 =
{
	/* basic banking */
	0,                              /* starting bank */
	{ 0x0008,0x001a,0x002c,0x003e },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x007d },              /* 1st mask/value in sequence */
	{ 0x3fff,0x3580 },              /* 2nd mask/value in sequence */
	{ 0x0079,0x0020 },              /* 3rd mask/value in sequence */
	{ 0x3fc9,0x0008 },              /* 4th mask/value in sequence */
	1,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x0676 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x0677 },              /* 2nd mask/value in sequence */
	{ 0x3e62,0x1a42 },              /* +1 mask/value */
	{ 0x3e35,0x1a11 },              /* +2 mask/value */
	{ 0x3fff,0x1a42 }               /* final mask/value in sequence */
};


/* slapstic 137412-118: Rampart/Vindicators II */
const atari_slapstic_device::slapstic_data atari_slapstic_device::slapstic118 =
{
	/* basic banking */
	0,                              /* starting bank */
	{ 0x0014,0x0034,0x0054,0x0074 },/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0002 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x1950 },              /* 2nd mask/value in sequence */
	{ 0x0067,0x0020 },              /* 3rd mask/value in sequence */
	{ 0x3f9f,0x0014 },              /* 4th mask/value in sequence */
	3,                              /* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x1958 },              /* 1st mask/value in sequence */
	{ 0x3fff,0x1959 },              /* 2nd mask/value in sequence */
	{ 0x3f73,0x3052 },              /* +1 mask/value */
	{ 0x3f67,0x3042 },              /* +2 mask/value */
	{ 0x3ff8,0x30e0 }               /* final mask/value in sequence */
};



/*************************************
 *
 *  Master slapstic table
 *
 *************************************/

/* master table */
const atari_slapstic_device::slapstic_data *const atari_slapstic_device::slapstic_table[] =
{
	&slapstic101,
	nullptr,      /* never seen */
	&slapstic103,
	&slapstic104,
	&slapstic105,
	&slapstic106,
	&slapstic107,
	&slapstic108,
	&slapstic109,
	&slapstic110,
	&slapstic111,
	&slapstic112,
	&slapstic113,
	&slapstic114,
	&slapstic115,
	&slapstic116,
	&slapstic117,
	&slapstic118
};


void atari_slapstic_device::device_validity_check(validity_checker &valid) const
{
	// only a small number of chips are known to exist
	if (m_chipnum < 101 || m_chipnum > 118 || !slapstic_table[m_chipnum - 101])
		osd_printf_error("Unknown slapstic number: %d\n", m_chipnum);
}


/*************************************
 *
 *  Initialization
 *
 *************************************/

void atari_slapstic_device::device_start()
{
	/* save state */
	save_item(NAME(m_current_bank));
	save_item(NAME(m_loaded_bank));
	save_item(NAME(m_saved_state));

	/* Address space tap installation */
	if(m_space->data_width() == 16)
		m_space->install_readwrite_tap(0, m_space->addrmask(), 0, "slapstic",
									   [this](offs_t offset, u16 &, u16) { if(!machine().side_effects_disabled()) m_state->test(offset); },
									   [this](offs_t offset, u16 &, u16) { if(!machine().side_effects_disabled()) m_state->test(offset); });
	else
		m_space->install_readwrite_tap(0, m_space->addrmask(), 0, "slapstic",
									   [this](offs_t offset, u8 &, u8) { if(!machine().side_effects_disabled()) m_state->test(offset); },
									   [this](offs_t offset, u8 &, u8) { if(!machine().side_effects_disabled()) m_state->test(offset); });

	checker check(m_start, m_end, m_mirror, m_space->data_width(), m_chipnum == 101 ? 13 : 14);
	const auto *info = slapstic_table[m_chipnum - 101];

	m_s_idle = std::make_unique<idle>(this, check, info);

	if(m_chipnum <= 102) {
		m_s_active = std::make_unique<active_101_102>(this, check, info);
		m_s_alt_valid = std::make_unique<alt_valid_101_102>(this, check, info);

	} else if(m_chipnum <= 110) {
		m_s_active = std::make_unique<active_103_110>(this, check, info);
		m_s_alt_valid = std::make_unique<alt_valid_103_110>(this, check, info);

	} else {
		m_s_active = std::make_unique<active_111_118>(this, check, info);
		m_s_alt_valid = std::make_unique<alt_valid_111_118>(this, check, info);
	}

	if(m_chipnum <= 110)
		m_s_alt_select = std::make_unique<alt_select_101_110>(this, check, info, m_space->data_width() == 16 ? 1 : 0);
	else
		m_s_alt_select = std::make_unique<alt_select_111_118>(this, check, info, m_space->data_width() == 16 ? 1 : 0);

	m_s_alt_commit = std::make_unique<alt_commit>(this, check, info);

	if(m_chipnum <= 110) {
		m_s_bit_load = std::make_unique<bit_load>(this, check, info);
		m_s_bit_set_odd  = std::make_unique<bit_set>(this, check, info, true);
		m_s_bit_set_even = std::make_unique<bit_set>(this, check, info, false);
	}

	if(m_chipnum >= 111) {
		m_s_add_load = std::make_unique<add_load>(this, check, info);
		m_s_add_set  = std::make_unique<add_set> (this, check, info);
	}

	m_state = m_s_idle.get();
}


void atari_slapstic_device::device_reset(void)
{
	/* reset the chip */
	m_state = m_s_idle.get();

	/* the 111 and later chips seem to reset to bank 0 */
	change_bank(slapstic_table[m_chipnum - 101]->bankstart);
}

void atari_slapstic_device::change_bank(int bank)
{
	logerror("current bank %d\n", bank);
	m_current_bank = bank;
	if(m_bank)
		m_bank->set_entry(m_current_bank);
	if(m_view)
		m_view->select(m_current_bank);
}

void atari_slapstic_device::commit_bank()
{
	change_bank(m_loaded_bank);
}


atari_slapstic_device::checker::checker(offs_t start, offs_t end, offs_t mirror, int data_width, int address_lines)
{
	m_range_mask = ~((end - start) | mirror);
	m_range_value = start;
	if(m_range_value & ~m_range_mask)
		fatalerror("The slapstic range %x-%x mirror %x is not masking friendly", start, end, mirror);
	m_shift = data_width == 16 ? 1 : 0;
	m_input_mask = util::make_bitmask<offs_t>(address_lines) << m_shift;
}

atari_slapstic_device::test atari_slapstic_device::checker::test_in(const mask_value &mv) const
{
	return test(m_range_mask | (mv.mask << m_shift), m_range_value | (mv.value << m_shift));
}

atari_slapstic_device::test atari_slapstic_device::checker::test_any(const mask_value &mv) const
{
	return test(mv.mask << m_shift, mv.value << m_shift);
}

atari_slapstic_device::test atari_slapstic_device::checker::test_inside() const
{
	return test(m_range_mask, m_range_value);
}

atari_slapstic_device::test atari_slapstic_device::checker::test_reset() const
{
	return test(m_range_mask | m_input_mask, m_range_value);
}

atari_slapstic_device::test atari_slapstic_device::checker::test_bank(u16 b) const
{
	return test(m_range_mask | m_input_mask, m_range_value | (b << m_shift));
}


// Idle state, waits for a reset to go to active

atari_slapstic_device::idle::idle(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	m_reset = check.test_reset();
}

void atari_slapstic_device::idle::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	}
}


// Active state, 101-102, has direct, alt and bitwise, and alt must be done in-range

atari_slapstic_device::active_101_102::active_101_102(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	for(int i=0; i != 4; i++)
		m_bank[i] = check.test_bank(data->bank[i]);
	m_alt = check.test_in(data->alt1);
	m_bit = check.test_in(data->bit1);
}

void atari_slapstic_device::active_101_102::test(offs_t addr) const
{
	if(m_bank[0](addr)) {
		m_sl->logerror("direct switch bank 0 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(0);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_bank[1](addr)) {
		m_sl->logerror("direct switch bank 1 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(1);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_bank[2](addr)) {
		m_sl->logerror("direct switch bank 2 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(2);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_bank[3](addr)) {
		m_sl->logerror("direct switch bank 3 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(3);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_alt(addr)) {
		m_sl->logerror("alt start (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_alt_valid.get();
	} else if(m_bit(addr)) {
		m_sl->logerror("bitwise start (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_bit_load.get();
	}
}


// Active state, 103-110, has direct, alt and bitwise, and alt can be done anywhere

atari_slapstic_device::active_103_110::active_103_110(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	for(int i=0; i != 4; i++)
		m_bank[i] = check.test_bank(data->bank[i]);
	m_alt = check.test_any(data->alt1);
	m_bit = check.test_in(data->bit1);
}

void atari_slapstic_device::active_103_110::test(offs_t addr) const
{
	if(m_bank[0](addr)) {
		m_sl->logerror("direct switch bank 0 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(0);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_bank[1](addr)) {
		m_sl->logerror("direct switch bank 1 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(1);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_bank[2](addr)) {
		m_sl->logerror("direct switch bank 2 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(2);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_bank[3](addr)) {
		m_sl->logerror("direct switch bank 3 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(3);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_alt(addr)) {
		m_sl->logerror("alt start (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_alt_valid.get();
	} else if(m_bit(addr)) {
		m_sl->logerror("bitwise start (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_bit_load.get();
	}
}


// Active state, 111-118, has direct, alt and add, and alt can be done anywhere

atari_slapstic_device::active_111_118::active_111_118(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	for(int i=0; i != 4; i++)
		m_bank[i] = check.test_bank(data->bank[i]);
	m_alt = check.test_any(data->alt1);
	m_add = check.test_in(data->add1);
}

void atari_slapstic_device::active_111_118::test(offs_t addr) const
{
	if(m_bank[0](addr)) {
		m_sl->logerror("direct switch bank 0 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(0);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_bank[1](addr)) {
		m_sl->logerror("direct switch bank 1 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(1);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_bank[2](addr)) {
		m_sl->logerror("direct switch bank 2 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(2);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_bank[3](addr)) {
		m_sl->logerror("direct switch bank 3 (%s)\n", m_sl->machine().describe_context());
		m_sl->change_bank(3);
		m_sl->m_state = m_sl->m_s_idle.get();
	} else if(m_alt(addr)) {
		m_sl->logerror("alt start (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_alt_valid.get();
	} else if(m_add(addr)) {
		m_sl->logerror("add start (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_add_load.get();
	}
}


// alt validation, 101-102 required to be outside of the range (hits a 6809 dummy vma access in practive)

atari_slapstic_device::alt_valid_101_102::alt_valid_101_102(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	m_reset = check.test_reset();
	m_inside = check.test_inside();
	m_valid = check.test_any(data->alt2);
}

void atari_slapstic_device::alt_valid_101_102::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	} else if(!m_inside(addr) && m_valid(addr)) {
		m_sl->logerror("alt valid (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_alt_select.get();
	} else {
		m_sl->logerror("alt sequence break at valid (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	}
}


// alt validation, 103-110, in-range

atari_slapstic_device::alt_valid_103_110::alt_valid_103_110(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	m_reset = check.test_reset();
	m_valid = check.test_in(data->alt2);
}

void atari_slapstic_device::alt_valid_103_110::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	} else if(m_valid(addr)) {
		m_sl->logerror("alt valid (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_alt_select.get();
	} else {
		m_sl->logerror("alt sequence break at valid (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	}
}


// alt validation, 111-118, in-range, can also switch to add

atari_slapstic_device::alt_valid_111_118::alt_valid_111_118(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	m_reset = check.test_reset();
	m_valid = check.test_in(data->alt2);
	m_add   = check.test_in(data->add1);
}

void atari_slapstic_device::alt_valid_111_118::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	} else if(m_valid(addr)) {
		m_sl->logerror("alt valid (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_alt_select.get();
	} else if(m_add(addr)) {
		m_sl->logerror("alt switch to add (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_add_load.get();
	} else {
		m_sl->logerror("alt sequence break at valid (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	}
}


// alt selection, 101-110, access must be done in-range

atari_slapstic_device::alt_select_101_110::alt_select_101_110(atari_slapstic_device *sl, const checker &check, const slapstic_data *data, int shift) : state(sl)
{
	m_reset  = check.test_reset();
	m_select = check.test_in(data->alt3);
	m_shift  = shift + data->altshift;
}

void atari_slapstic_device::alt_select_101_110::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	} else if(m_select(addr)) {
		m_sl->logerror("alt select (%s)\n", m_sl->machine().describe_context());
		m_sl->m_loaded_bank = (addr >> m_shift) & 3;
		m_sl->m_state = m_sl->m_s_alt_commit.get();
	} else {
		m_sl->logerror("alt sequence break at select (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	}
}


// alt selection, 111-118, access can be done anywhere

atari_slapstic_device::alt_select_111_118::alt_select_111_118(atari_slapstic_device *sl, const checker &check, const slapstic_data *data, int shift) : state(sl)
{
	m_reset  = check.test_reset();
	m_select = check.test_any(data->alt3);
	m_shift  = shift + data->altshift;
}

void atari_slapstic_device::alt_select_111_118::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	} else if(m_select(addr)) {
		m_sl->logerror("alt select (%s)\n", m_sl->machine().describe_context());
		m_sl->m_loaded_bank = (addr >> m_shift) & 3;
		m_sl->m_state = m_sl->m_s_alt_commit.get();
	} else {
		m_sl->logerror("alt sequence break at select (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	}
}


// alt commit

atari_slapstic_device::alt_commit::alt_commit(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	m_reset  = check.test_reset();
	m_commit = check.test_in(data->alt4);
}

void atari_slapstic_device::alt_commit::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	} else if(m_commit(addr)) {
		m_sl->logerror("alt/add commit (%s)\n", m_sl->machine().describe_context());
		m_sl->commit_bank();
		m_sl->m_state = m_sl->m_s_idle.get();
	}
}


// bitwise, load the current bank state

atari_slapstic_device::bit_load::bit_load(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	m_reset = check.test_reset();
	m_load  = check.test_in(data->bit2);
}

void atari_slapstic_device::bit_load::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	} else if(m_load(addr)) {
		m_sl->logerror("bitwise load (%s)\n", m_sl->machine().describe_context());
		m_sl->m_loaded_bank = m_sl->m_current_bank;
		m_sl->m_state = m_sl->m_s_bit_set_odd.get();
	}
}

// bitwise, change one bit

atari_slapstic_device::bit_set::bit_set(atari_slapstic_device *sl, const checker &check, const slapstic_data *data, bool is_odd) : state(sl), m_is_odd(is_odd)
{
	m_reset  = check.test_reset();
	m_clear0 = check.test_in(is_odd ? data->bit3c0 : data->bit3s1);
	m_set0   = check.test_in(is_odd ? data->bit3s0 : data->bit3c1);
	m_clear1 = check.test_in(is_odd ? data->bit3c1 : data->bit3s0);
	m_set1   = check.test_in(is_odd ? data->bit3s1 : data->bit3c0);
	m_commit = check.test_in(data->bit4);
}

void atari_slapstic_device::bit_set::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	} else if(m_clear0(addr)) {
		m_sl->logerror("bitwise clear 0 (%s)\n", m_sl->machine().describe_context());
		m_sl->m_loaded_bank &= ~1;
		m_sl->m_state = m_is_odd ? m_sl->m_s_bit_set_even.get() : m_sl->m_s_bit_set_odd.get();
	} else if(m_set0(addr)) {
		m_sl->logerror("bitwise set 0 (%s)\n", m_sl->machine().describe_context());
		m_sl->m_loaded_bank |= 1;
		m_sl->m_state = m_is_odd ? m_sl->m_s_bit_set_even.get() : m_sl->m_s_bit_set_odd.get();
	} else if(m_clear1(addr)) {
		m_sl->logerror("bitwise clear 1 (%s)\n", m_sl->machine().describe_context());
		m_sl->m_loaded_bank &= ~2;
		m_sl->m_state = m_is_odd ? m_sl->m_s_bit_set_even.get() : m_sl->m_s_bit_set_odd.get();
	} else if(m_set1(addr)) {
		m_sl->logerror("bitwise set 1 (%s)\n", m_sl->machine().describe_context());
		m_sl->m_loaded_bank |= 2;
		m_sl->m_state = m_is_odd ? m_sl->m_s_bit_set_even.get() : m_sl->m_s_bit_set_odd.get();
	} else if(m_commit(addr)) {
		m_sl->logerror("bitwise commit %d (%s)\n", m_sl->m_loaded_bank, m_sl->machine().describe_context());
		m_sl->commit_bank();
		m_sl->m_state = m_sl->m_s_idle.get();
	}
}


// add, load the current bank state

atari_slapstic_device::add_load::add_load(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	m_reset = check.test_reset();
	m_load  = check.test_in(data->add2);
}

void atari_slapstic_device::add_load::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	} else if(m_load(addr)) {
		m_sl->logerror("add load (%s)\n", m_sl->machine().describe_context());
		m_sl->m_loaded_bank = m_sl->m_current_bank;
		m_sl->m_state = m_sl->m_s_add_set.get();
	} else {
		m_sl->logerror("add sequence break at load (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	}
}


// add, change the bank number

atari_slapstic_device::add_set::add_set(atari_slapstic_device *sl, const checker &check, const slapstic_data *data) : state(sl)
{
	m_reset = check.test_reset();
	m_add1  = check.test_in(data->addplus1);
	m_add2  = check.test_in(data->addplus2);
	m_end   = check.test_in(data->add3);
}

void atari_slapstic_device::add_set::test(offs_t addr) const
{
	if(m_reset(addr)) {
		m_sl->logerror("reset (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_active.get();
	} else if(m_add1(addr)) {
		m_sl->logerror("add +1 (%s)\n", m_sl->machine().describe_context());
		m_sl->m_loaded_bank = (m_sl->m_loaded_bank + 1) & 3;
	} else if(m_add2(addr)) {
		m_sl->logerror("add +2 (%s)\n", m_sl->machine().describe_context());
		m_sl->m_loaded_bank = (m_sl->m_loaded_bank + 2) & 3;
	} else if(m_end(addr)) {
		m_sl->logerror("add end (%s)\n", m_sl->machine().describe_context());
		m_sl->m_state = m_sl->m_s_alt_commit.get();
	}
}


// state saving management

void atari_slapstic_device::device_pre_save()
{
	m_saved_state = m_state->state_id();
}

u8 atari_slapstic_device::idle              ::state_id() const { return S_IDLE; }
u8 atari_slapstic_device::active_101_102    ::state_id() const { return S_ACTIVE; }
u8 atari_slapstic_device::active_103_110    ::state_id() const { return S_ACTIVE; }
u8 atari_slapstic_device::active_111_118    ::state_id() const { return S_ACTIVE; }
u8 atari_slapstic_device::alt_valid_101_102 ::state_id() const { return S_ALT_VALID; }
u8 atari_slapstic_device::alt_valid_103_110 ::state_id() const { return S_ALT_VALID; }
u8 atari_slapstic_device::alt_valid_111_118 ::state_id() const { return S_ALT_VALID; }
u8 atari_slapstic_device::alt_select_101_110::state_id() const { return S_ALT_SELECT; }
u8 atari_slapstic_device::alt_select_111_118::state_id() const { return S_ALT_SELECT; }
u8 atari_slapstic_device::alt_commit        ::state_id() const { return S_ALT_COMMIT; }
u8 atari_slapstic_device::bit_load          ::state_id() const { return S_BIT_LOAD; }
u8 atari_slapstic_device::bit_set           ::state_id() const { return m_is_odd ? S_BIT_SET_ODD : S_BIT_SET_EVEN; }
u8 atari_slapstic_device::add_load          ::state_id() const { return S_ADD_LOAD; }
u8 atari_slapstic_device::add_set           ::state_id() const { return S_ADD_SET; }

void atari_slapstic_device::device_post_load()
{
	switch(m_saved_state) {
	case S_IDLE:         m_state = m_s_idle.get(); break;
	case S_ACTIVE:       m_state = m_s_active.get(); break;
	case S_ALT_VALID:    m_state = m_s_alt_valid.get(); break;
	case S_ALT_SELECT:   m_state = m_s_alt_select.get(); break;
	case S_ALT_COMMIT:   m_state = m_s_alt_commit.get(); break;
	case S_BIT_LOAD:     m_state = m_s_bit_load.get(); break;
	case S_BIT_SET_ODD:  m_state = m_s_bit_set_odd.get(); break;
	case S_BIT_SET_EVEN: m_state = m_s_bit_set_even.get(); break;
	case S_ADD_LOAD:     m_state = m_s_add_load.get(); break;
	case S_ADD_SET:      m_state = m_s_add_set.get(); break;
	}
}
