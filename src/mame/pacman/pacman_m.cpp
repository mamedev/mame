// license:BSD-3-Clause
// copyright-holders:smf, Mike Balfour, David Widel

#include "emu.h"
#include "pacman.h"

uint8_t epospm_state::epos_decryption_w(offs_t offset)
{
	if (offset & 0x01)
	{
		m_counter = (m_counter - 1) & 0x0F;
	}
	else
	{
		m_counter = (m_counter + 1) & 0x0F;
	}

	switch (m_counter)
	{
	case 0x08:
	case 0x09:
	case 0x0A:
	case 0x0B:
		membank("bank1")->set_entry(m_counter & 3);
		break;

	default:
		logerror("Invalid counter = %02X\n", m_counter);
		break;
	}

	return 0;
}

static void epos_decrypt_rom(uint8_t *ROM, uint8_t invert, int offset, int *bs)
{
	for (int mem = 0; mem < 0x4000; mem++ )
	{
		ROM[mem + offset] = bitswap<8>(ROM[mem] ^ invert, bs[0], bs[1], bs[2], bs[3], bs[4], bs[5], bs[6], bs[7]);
	}
}


/*

The Glob protection description:

The Glob is designed to run on modified Pacman hardware.  It contains
two graphics ROMs at 5E and 5F, but contains code ROMs on a daughterboard
similar in concept to Ms. Pacman.  However, these code ROMs are decrypted
through additional circuitry.  The daughterboard was encased in epoxy.

Here's a description of the protection as best as I can give it.

1)  The decrypted D0 bit fed to the CPU is simply an inversion of the
D5 bit from the code ROMs.
2)  The decrypted D1 bit fed to the CPU is simply an inversion of the
D2 bit from the code ROMs.
3)  The other 6 data bits are decrypted by a 10H8 PAL.  The PAL also
takes as input a 4-bit counter.  The counter is incremented and
decremented as follows:
- the Z-80 command IN($xx) where xx is an odd number decrements the
counter; an even number increments the counter.
Ex:  IN($64) would increment the counter, IN($6B) would decrement
the counter.
4)  The PAL output also contains the two ROM enable lines used to enable
the two encrypted code ROMs.  As long as the system is working
correctly, these ROMs will always be enabled.

As it so happens, only four counter values are ever used, which is
fortunate because the PAL only contains signals to enable the ROMs for
those four counter values.  The valid counter values are $8, $9, $A, and
$B.  The counter's initial value is $A, which is set by jumpers on the
daughterboard.  Following is a description of the resulting decryptions
for these four counter states.

COUNTER       ENCRYPTED   DECRYPTED
VALUE       VALUE

DDDDDDDD    DDDDDDDD
76543210    76543210

Counter = 8:  abcdefgh    EAhBDgFC
Counter = 9:  abcdefgh    FAgeDBFC
Counter = A:  abcdefgh    EHDBagFC
Counter = B:  abcdefgh    GHDEaBFC

In the above diagram, capital letters represent inverted bits.  Notice
that bits D2 and D5 are the same independent of counter state, this is
because these bits are not decrypted by the PAL.


In the code below, all four of these decryption patterns are used to
decrypt the entire code ROMs before execution.  This is done for speed,
since we can then just bankswitch between the decrypted code sets on
each IN($xx) command, as opposed to dynamically decrypting every byte.

- Mike Balfour (mab22@po.cwru.edu)

*/

MACHINE_START_MEMBER(epospm_state, theglobp)
{
	/*  Note: D2 is inverted and connected to D1, D5 is inverted and
	connected to D0.  The other six data bits are converted by a
	PAL10H8 driven by the counter. */

	int bs[4][8] = {
		{ 3,7,0,6,4,1,2,5 },
		{ 1,7,0,3,4,6,2,5 },
		{ 3,0,4,6,7,1,2,5 },
		{ 1,0,4,3,7,6,2,5 },
	};

	/* While the PAL supports up to 16 decryption methods, only four
	are actually used in the PAL.  Therefore, we'll take a little
	memory overhead and decrypt the ROMs using each method in advance. */

	uint8_t *ROM = memregion("maincpu")->base();
	epos_decrypt_rom(ROM, 0xfc, 0x10000, bs[0]);
	epos_decrypt_rom(ROM, 0xf6, 0x14000, bs[1]);
	epos_decrypt_rom(ROM, 0x7d, 0x18000, bs[2]);
	epos_decrypt_rom(ROM, 0x77, 0x1c000, bs[3]);

	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);

	save_item(NAME(m_counter));
}

MACHINE_RESET_MEMBER(epospm_state, theglobp)
{
	m_counter = 0x0A;
	membank("bank1")->set_entry(m_counter & 3);
}

/*

Same Epos board as usual(theglobp,beastf,street heat). This is fairly easy to decrypt since it has consecutive bytes with the same algorithm.
There are 4 different algorithms.  One consists almost entirely of text, one contains the majority of code and the remaining 2 are not used
much and are therefore the most difficult. It is however difficult to decrypt to ROM.
The data for the coin sound is actually program code in a different phase. You need to move the sound tables and add a ROM to get it to run
without the daughterboard.

acitya contains a bug with the insurance in blackjack.  It's impossible to collect, so it's likely that acitya is earlier than bwcasino.

I don't think this game is a gambling game.  For one thing there's no real output hardware on a pacman board and the epos daughterboard doesn't contain any either.

David Widel d_widel@hotmail.com

*/

MACHINE_START_MEMBER(epospm_state, acitya)
{
	/*  Note: D2 is inverted and connected to D1, D5 is inverted and
	connected to D0.  The other six data bits are converted by a
	PAL10H8 driven by the counter. */

	int bs[4][8] = {
		{ 1,6,7,3,4,0,2,5 },
		{ 7,6,1,3,4,0,2,5 },
		{ 1,0,7,6,4,3,2,5 },
		{ 7,0,1,6,4,3,2,5 },
	};

	/* While the PAL supports up to 16 decryption methods, only four
	are actually used in the PAL.  Therefore, we'll take a little
	memory overhead and decrypt the ROMs using each method in advance. */

	uint8_t *ROM = memregion("maincpu")->base();
	epos_decrypt_rom(ROM, 0xb5, 0x10000, bs[0]);
	epos_decrypt_rom(ROM, 0xa7, 0x14000, bs[1]);
	epos_decrypt_rom(ROM, 0xfc, 0x18000, bs[2]);
	epos_decrypt_rom(ROM, 0xee, 0x1c000, bs[3]);

	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);

	save_item(NAME(m_counter));
}

MACHINE_RESET_MEMBER(epospm_state, acitya)
{
	m_counter = 0x0B;
	membank("bank1")->set_entry(m_counter & 3);
}


MACHINE_START_MEMBER(epospm_state, eeekkp)
{
	/*  Note: D2 is inverted and connected to D1, D5 is inverted and
	connected to D0.  The other six data bits are converted by a
	PAL10H8 driven by the counter. */

	int bs[4][8] = {
		{ 7,6,1,3,0,4,2,5 },
		{ 7,1,4,3,0,6,2,5 },
		{ 7,6,1,0,3,4,2,5 },
		{ 7,1,4,0,3,6,2,5 },
	};

	/* While the PAL supports up to 16 decryption methods, only four
	are actually used in the PAL.  Therefore, we'll take a little
	memory overhead and decrypt the ROMs using each method in advance. */

	uint8_t *ROM = memregion("maincpu")->base();
	epos_decrypt_rom(ROM, 0xfd, 0x10000, bs[0]);
	epos_decrypt_rom(ROM, 0xbf, 0x14000, bs[1]);
	epos_decrypt_rom(ROM, 0x75, 0x18000, bs[2]);
	epos_decrypt_rom(ROM, 0x37, 0x1c000, bs[3]);

	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);

	save_item(NAME(m_counter));
}

MACHINE_RESET_MEMBER(epospm_state, eeekkp)
{
	m_counter = 0x09;
	membank("bank1")->set_entry(m_counter & 3);
}
