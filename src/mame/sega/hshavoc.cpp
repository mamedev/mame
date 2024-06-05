// license:BSD-3-Clause
// copyright-holders:David Haywood
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
1x PIC16C55 (7a)
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

Thanks to DOX, the "mystery chip" has been identified (and verified) as a PIC.
It's read protected, but has been decapped and read out. Unfortunately it doesn't
help with figuring out the encryption on the coin-op parts of the program.

*/


#include "emu.h"
#include "megadriv_acbl.h"

#include "cpu/m68000/m68000.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/ymopn.h"


namespace {

class hshavoc_state : public md_boot_state
{
public:
	hshavoc_state(const machine_config &mconfig, device_type type, const char *tag)
		: md_boot_state(mconfig, type, tag)
	{ }

	void hshavoc(machine_config &config);

	void init_hshavoc();
};


INPUT_PORTS_START( hshavoc )
	PORT_START("IN0")   // 16 bit
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


void hshavoc_state::hshavoc(machine_config &config)
{
	md_bootleg(config);

	PIC16C55(config, "pic", 4'000'000); // clock unknown
}


ROM_START( hshavoc )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "d-25.11a", 0x000000, 0x080000, CRC(6a155060) SHA1(ecb47bd428786e50e300a062b5038f943419a389) )
	ROM_LOAD16_BYTE( "d-26.9a",  0x000001, 0x080000, CRC(1afa84fe) SHA1(041296e0360b7747aedc2d948c39e06ba03a7d08) )

	ROM_REGION( 0x400, "plds", 0 )
	ROM_LOAD( "peel18cv8s.4b.bin",  0x000, 0x155, CRC(b5fb1d5f) SHA1(f0ac80471d97f77f415b5a1f153e1fce66720963) )
	ROM_LOAD( "peel18cv8s.5b.bin",  0x000, 0x155, CRC(efc7ceea) SHA1(1c31a56bc4b83bfa708048b7de4cee7a24537500) )

	ROM_REGION( 0x407, "pic", 0 ) // pic16c55
	ROM_LOAD( "ms02-.7a",  0x000, 0x407, CRC(7163eb63) SHA1(d9c2cb7d24ce070d43597fabf012ebce47693954) ) // decapped
ROM_END


void hshavoc_state::init_hshavoc()
{
	uint16_t *src = (uint16_t *)memregion("maincpu")->base();

	static const uint16_t typedat[16] = {
		1,1,1,1, 1,1,1,1,
		1,0,0,1, 1,0,1,1  };

	/* this decryption is wrong / incomplete, maybe it uses slightly different decryption for opcodes / data */
	/* I think the PIC that exists on the PCB controls a state-based encryption... there is a large amount
	   of code encrypted using the same encryption as the data, but all the startup-code + vectors use additional
	   encryption.. maybe the PIC can also patch the code, I'm also concerned that we may decrypt it and find
	   that it runs as the Genesis (no insert coin etc.) version without the PIC, or the PIC supplies additional
	   code in RAM.. but as of yet we can't know */

	int rom_size = 0xe8000;
	for (int x = 0; x < rom_size / 2; x++)
	{
		src[x] = bitswap<16>(src[x],
								7, 15,6, 14,
								5, 2, 1, 10,
								13,4, 12,3,
								11,0, 8, 9 );


		if (typedat[x & 0xf] == 1)
			src[x] = src[x] ^ 0x0501;
		else
			src[x] = src[x] ^ 0x0406;


		if (src[x] & 0x0400)
			src[x] ^= 0x0200;

		if (typedat[x & 0xf] == 0)
		{
			if (src[x] & 0x0100)
				src[x] ^= 0x0004;

			src[x] = bitswap<16>(src[x], 15,14,13,12,
												11,9, 10,8,
												7, 6, 5, 4,
												3, 2, 1, 0 );
		}
	}

	// START e? from e80000 to end you need THIS ALONE to match the Genesis ROM
	for (int x = rom_size / 2; x < 0x100000 / 2; x++)
		src[x] = bitswap<16>(src[x],
								7, 15, 6, 14,
								5,  2, 1,  0,
								13, 4, 12, 3,
								11, 10, 9, 8 );
	// EMD e80000 - end

	src[0] ^= 0x0107;
	src[1] ^= 0x0107;
	src[2] ^= 0x0107;
	src[3] ^= 0x0707; //? 0701 not 0107 .. conditional 0x600 extra xor?, different 'typemap' ??

	/* I'm pretty sure c42 is where the startup code is located, comparing Genesis version
	   and there is at least one jump in the Genesis version to the startup code which
	   has been changed to this address in the arcade version.

	   there are several blocks of code like this, all appear to end with a normal rts instruction
	   tho...
	   */
	for (int x = 0xc42 / 2; x < 0xc9a / 2; x++)
	{
		src[x] ^= 0x0107; //? seems conditional..

		src[x] = bitswap<16>(src[x],
								15,13,14,12,
								11,10,9, 0,
								8, 6, 5, 4,
								3, 2, 1, 7 ); // probably wrong

		src[x] ^= 0x0001; // wrong..
	}

	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		space.nop_write(0x200000, 0x201fff);
	}

	init_megadriv();

	m_vdp->stop_timers();
}

} // anonymous namespace


GAME( 1993, hshavoc, 0, hshavoc, hshavoc, hshavoc_state, init_hshavoc, ROT0, "Data East Corporation", "High Seas Havoc", MACHINE_NOT_WORKING )
