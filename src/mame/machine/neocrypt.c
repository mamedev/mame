// license:???
// copyright-holders:Bryan McPhail,Fuzz,Ernesto Corvi,Andrew Prime,Zsolt Vasvari

/***************************************************************************

    Neo-Geo hardware encryption devices

    NeoGeo 'C' (Graphics) Rom encryption
      CMC42 and CMC50 protection chips
      Also contains 'S' (Text Layer) data on these games
      M1 (Z80) rom is also encrypted for CMC50

     The M1 roms contain some additional data at 0xf800.  Some of this
     is said to be related to the C rom encryption.
     If CMC50 is used, data located at 0xff00 - 0xffff is required for
     m1 encryption checksum?.

      Later games use additional basic scrambling on top of the standard
      CMC scramble.

    NeoGeo 'P' Rom Encryption
      Used on various games

      kof98
        - unique early encryption
      kof99, garou, garouh, mslug3, kof2000
        - complex SMA chip which appears to contain part of the game rom
          internally and decrypts the 68k code on the board.  Also has a
          random number generator and  custom bankswitching
          (see machine/neoprot.c)
      kof2002, matrim, samsho5, samsh5p
        - some basic block / bank swapping
      svc, kof2003, mslug5
        - different scrambling with additional xor

    NeoGeo 'V' Rom encryption
      NEO-PCM2 chip used on various games
      type1 used on pnyaa, rotd, mslug4
      type2 used on kof2002, matrim, mslug5, svc,
                    samsho5, samsh5s, kof2003

***************************************************************************/

#include "emu.h"
#include "includes/neogeo.h"




/* ms5pcb and svcpcb have an additional scramble on top of the standard CMC scrambling */
void neogeo_noslot_state::svcpcb_gfx_decrypt()
{
	static const UINT8 xorval[ 4 ] = { 0x34, 0x21, 0xc4, 0xe9 };
	int i;
	int ofst;
	int rom_size = memregion( "sprites" )->bytes();
	UINT8 *rom = memregion( "sprites" )->base();
	dynamic_buffer buf( rom_size );

	for( i = 0; i < rom_size; i++ )
	{
		rom[ i ] ^= xorval[ (i % 4) ];
	}
	for( i = 0; i < rom_size; i += 4 )
	{
		UINT32 rom32 = rom[i] | rom[i+1]<<8 | rom[i+2]<<16 | rom[i+3]<<24;
		rom32 = BITSWAP32( rom32, 0x09, 0x0d, 0x13, 0x00, 0x17, 0x0f, 0x03, 0x05, 0x04, 0x0c, 0x11, 0x1e, 0x12, 0x15, 0x0b, 0x06, 0x1b, 0x0a, 0x1a, 0x1c, 0x14, 0x02, 0x0e, 0x1d, 0x18, 0x08, 0x01, 0x10, 0x19, 0x1f, 0x07, 0x16 );
		buf[i]   = rom32       & 0xff;
		buf[i+1] = (rom32>>8)  & 0xff;
		buf[i+2] = (rom32>>16) & 0xff;
		buf[i+3] = (rom32>>24) & 0xff;
	}
	for( i = 0; i < rom_size / 4; i++ )
	{
		ofst =  BITSWAP24( (i & 0x1fffff), 0x17, 0x16, 0x15, 0x04, 0x0b, 0x0e, 0x08, 0x0c, 0x10, 0x00, 0x0a, 0x13, 0x03, 0x06, 0x02, 0x07, 0x0d, 0x01, 0x11, 0x09, 0x14, 0x0f, 0x12, 0x05 );
		ofst ^= 0x0c8923;
		ofst += (i & 0xffe00000);
		memcpy( &rom[ i * 4 ], &buf[ ofst * 4 ], 0x04 );
	}
}


/* and a further swap on the s1 data */
void neogeo_noslot_state::svcpcb_s1data_decrypt()
{
	int i;
	UINT8 *s1 = memregion( "fixed" )->base();
	size_t s1_size = memregion( "fixed" )->bytes();

	for( i = 0; i < s1_size; i++ ) // Decrypt S
	{
		s1[ i ] = BITSWAP8( s1[ i ] ^ 0xd2, 4, 0, 7, 2, 5, 1, 6, 3 );
	}
}


/* kf2k3pcb has an additional scramble on top of the standard CMC scrambling */
/* Thanks to Razoola & Halrin for the info */
void neogeo_noslot_state::kf2k3pcb_gfx_decrypt()
{
	static const UINT8 xorval[ 4 ] = { 0x34, 0x21, 0xc4, 0xe9 };
	int i;
	int ofst;
	int rom_size = memregion( "sprites" )->bytes();
	UINT8 *rom = memregion( "sprites" )->base();
	dynamic_buffer buf( rom_size );

	for ( i = 0; i < rom_size; i++ )
	{
		rom[ i ] ^= xorval[ (i % 4) ];
	}
	for ( i = 0; i < rom_size; i +=4 )
	{
		UINT32 rom32 = rom[i] | rom[i+1]<<8 | rom[i+2]<<16 | rom[i+3]<<24;
		rom32 = BITSWAP32( rom32, 0x09, 0x0d, 0x13, 0x00, 0x17, 0x0f, 0x03, 0x05, 0x04, 0x0c, 0x11, 0x1e, 0x12, 0x15, 0x0b, 0x06, 0x1b, 0x0a, 0x1a, 0x1c, 0x14, 0x02, 0x0e, 0x1d, 0x18, 0x08, 0x01, 0x10, 0x19, 0x1f, 0x07, 0x16 );
		buf[i]   =  rom32      & 0xff;
		buf[i+1] = (rom32>>8)  & 0xff;
		buf[i+2] = (rom32>>16) & 0xff;
		buf[i+3] = (rom32>>24) & 0xff;
	}
	for ( i = 0; i < rom_size; i+=4 )
	{
		ofst = BITSWAP24( (i & 0x7fffff), 0x17, 0x15, 0x0a, 0x14, 0x13, 0x16, 0x12, 0x11, 0x10, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 );
		ofst ^= 0x000000;
		ofst += (i & 0xff800000);
		memcpy( &rom[ ofst ], &buf[ i ], 0x04 );
	}
}


/* and a further swap on the s1 data */
void neogeo_noslot_state::kf2k3pcb_decrypt_s1data()
{
	UINT8 *src;
	UINT8 *dst;
	int i;
	int tx_size = memregion( "fixed" )->bytes();
	int srom_size = memregion( "sprites" )->bytes();

	src = memregion( "sprites" )->base() + srom_size - 0x1000000 - 0x80000; // Decrypt S
	dst = memregion( "fixed" )->base();

	for( i = 0; i < tx_size / 2; i++ )
	{
		dst[ i ] = src[ (i & ~0x1f) + ((i & 7) << 2) + ((~i & 8) >> 2) + ((i & 0x10) >> 4) ];
	}

	src = memregion( "sprites" )->base() + srom_size - 0x80000;
	dst = memregion( "fixed" )->base() + 0x80000;

	for( i = 0; i < tx_size / 2; i++ )
	{
		dst[ i ] = src[ (i & ~0x1f) + ((i & 7) << 2) + ((~i & 8) >> 2) + ((i & 0x10) >> 4) ];
	}

	dst = memregion( "fixed" )->base();

	for( i = 0; i < tx_size; i++ )
	{
		dst[ i ] = BITSWAP8( dst[ i ] ^ 0xd2, 4, 0, 7, 2, 5, 1, 6, 3 );
	}
}





/***************************************************************************

NeoGeo 'SP1' (BIOS) ROM encryption

***************************************************************************/


/* only found on kf2k3pcb */
void neogeo_noslot_state::kf2k3pcb_sp1_decrypt()
{
	static const UINT8 address[0x40] = {
		0x04,0x0a,0x04,0x0a,0x04,0x0a,0x04,0x0a,
		0x0a,0x04,0x0a,0x04,0x0a,0x04,0x0a,0x04,
		0x09,0x07,0x09,0x07,0x09,0x07,0x09,0x07,
		0x09,0x09,0x04,0x04,0x09,0x09,0x04,0x04,
		0x0b,0x0d,0x0b,0x0d,0x03,0x05,0x03,0x05,
		0x0e,0x0e,0x03,0x03,0x0e,0x0e,0x03,0x03,
		0x03,0x05,0x0b,0x0d,0x03,0x05,0x0b,0x0d,
		0x04,0x00,0x04,0x00,0x0e,0x0a,0x0e,0x0a
	};

	UINT16 *rom = (UINT16 *)memregion("mainbios")->base();
	std::vector<UINT16> buf(0x80000/2);
	int i, addr;

	for (i = 0; i < 0x80000/2; i++)
	{
		// address xor
		addr = i ^ 0x0020;
		if ( i & 0x00020) addr ^= 0x0010;
		if (~i & 0x00010) addr ^= 0x0040;
		if (~i & 0x00004) addr ^= 0x0080;
		if ( i & 0x00200) addr ^= 0x0100;
		if (~i & 0x02000) addr ^= 0x0400;
		if (~i & 0x10000) addr ^= 0x1000;
		if ( i & 0x02000) addr ^= 0x8000;
		addr ^= address[((i >> 1) & 0x38) | (i & 7)];
		buf[i] = rom[addr];

		// data xor
		if (buf[i] & 0x0004) buf[i] ^= 0x0001;
		if (buf[i] & 0x0010) buf[i] ^= 0x0002;
		if (buf[i] & 0x0020) buf[i] ^= 0x0008;
	}

	memcpy(rom, &buf[0], 0x80000);
}
