// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

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
  $B.  The counter's intial value is $A, which is set by jumpers on the
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

***************************************************************************/

#include "emu.h"
#include "includes/pacman.h"


void pacman_state::theglobp_decrypt_rom_8()
{
	int oldbyte,inverted_oldbyte,newbyte;
	int mem;
	UINT8 *RAM;

	RAM = memregion("maincpu")->base();


	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = RAM[mem];
		inverted_oldbyte = ~oldbyte;

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
		    connected to D0.  The other six data bits are converted by a
		    PAL10H8 driven by the counter. */

		/* Direct inversion */
		newbyte  = (inverted_oldbyte & 0x04) >> 1;
		newbyte |= (inverted_oldbyte & 0x20) >> 5;
		/* PAL */
		newbyte |= (oldbyte & 0x01) << 5;
		newbyte |= (oldbyte & 0x02) << 1;
		newbyte |= (inverted_oldbyte & 0x08) << 4;
		newbyte |= (inverted_oldbyte & 0x10) >> 1;
		newbyte |= (inverted_oldbyte & 0x40) >> 2;
		newbyte |= (inverted_oldbyte & 0x80) >> 1;

		RAM[mem + 0x10000] = newbyte;
	}

	return;
}


void pacman_state::theglobp_decrypt_rom_9()
{
	int oldbyte,inverted_oldbyte,newbyte;
	int mem;
	UINT8 *RAM;

	RAM = memregion("maincpu")->base();

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = RAM[mem];
		inverted_oldbyte = ~oldbyte;

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
		    connected to D0.  The other six data bits are converted by a
		    PAL10H8 driven by the counter. */

		/* Direct inversion */
		newbyte  = (inverted_oldbyte & 0x04) >> 1;
		newbyte |= (inverted_oldbyte & 0x20) >> 5;
		/* PAL */
		newbyte |= (oldbyte & 0x01) << 5;
		newbyte |= (inverted_oldbyte & 0x02) << 6;
		newbyte |= (oldbyte & 0x08) << 1;
		newbyte |= (inverted_oldbyte & 0x10) >> 1;
		newbyte |= (inverted_oldbyte & 0x40) >> 4;
		newbyte |= (inverted_oldbyte & 0x80) >> 1;

		RAM[mem + 0x14000] = newbyte;
	}

	return;
}

void pacman_state::theglobp_decrypt_rom_A()
{
	int oldbyte,inverted_oldbyte,newbyte;
	int mem;
	UINT8 *RAM;

	RAM = memregion("maincpu")->base();

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = RAM[mem];
		inverted_oldbyte = ~oldbyte;

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
		    connected to D0.  The other six data bits are converted by a
		    PAL10H8 driven by the counter. */

		/* Direct inversion */
		newbyte  = (inverted_oldbyte & 0x04) >> 1;
		newbyte |= (inverted_oldbyte & 0x20) >> 5;
		/* PAL */
		newbyte |= (inverted_oldbyte & 0x01) << 6;
		newbyte |= (oldbyte & 0x02) << 1;
		newbyte |= (inverted_oldbyte & 0x08) << 4;
		newbyte |= (inverted_oldbyte & 0x10) << 1;
		newbyte |= (inverted_oldbyte & 0x40) >> 2;
		newbyte |= (oldbyte & 0x80) >> 4;

		RAM[mem + 0x18000] = newbyte;
	}

	return;
}

void pacman_state::theglobp_decrypt_rom_B()
{
	int oldbyte,inverted_oldbyte,newbyte;
	int mem;
	UINT8 *RAM;

	RAM = memregion("maincpu")->base();

	for (mem=0;mem<0x4000;mem++)
	{
		oldbyte = RAM[mem];
		inverted_oldbyte = ~oldbyte;

		/*  Note: D2 is inverted and connected to D1, D5 is inverted and
		    connected to D0.  The other six data bits are converted by a
		    PAL10H8 driven by the counter. */

		/* Direct inversion */
		newbyte  = (inverted_oldbyte & 0x04) >> 1;
		newbyte |= (inverted_oldbyte & 0x20) >> 5;
		/* PAL */
		newbyte |= (inverted_oldbyte & 0x01) << 6;
		newbyte |= (inverted_oldbyte & 0x02) << 6;
		newbyte |= (oldbyte & 0x08) << 1;
		newbyte |= (inverted_oldbyte & 0x10) << 1;
		newbyte |= (inverted_oldbyte & 0x40) >> 4;
		newbyte |= (oldbyte & 0x80) >> 4;

		RAM[mem + 0x1C000] = newbyte;
	}

	return;
}


READ8_MEMBER(pacman_state::theglobp_decrypt_rom )
{
	if (offset & 0x01)
	{
		m_counter = (m_counter - 1) & 0x0F;
	}
	else
	{
		m_counter = (m_counter + 1) & 0x0F;
	}

	switch(m_counter)
	{
		case 0x08:  membank ("bank1")->set_entry (0);        break;
		case 0x09:  membank ("bank1")->set_entry (1);        break;
		case 0x0A:  membank ("bank1")->set_entry (2);        break;
		case 0x0B:  membank ("bank1")->set_entry (3);        break;
		default:
			logerror("Invalid counter = %02X\n",m_counter);
			break;
	}

	return 0;
}


MACHINE_START_MEMBER(pacman_state,theglobp)
{
	UINT8 *RAM = memregion("maincpu")->base();

	/* While the PAL supports up to 16 decryption methods, only four
	    are actually used in the PAL.  Therefore, we'll take a little
	    memory overhead and decrypt the ROMs using each method in advance. */
	theglobp_decrypt_rom_8();
	theglobp_decrypt_rom_9();
	theglobp_decrypt_rom_A();
	theglobp_decrypt_rom_B();

	membank("bank1")->configure_entries(0, 4, &RAM[0x10000], 0x4000);

	save_item(NAME(m_counter));
}


MACHINE_RESET_MEMBER(pacman_state,theglobp)
{
	/* The initial state of the counter is 0x0A */
	m_counter = 0x0A;
	membank("bank1")->set_entry(2);
}
