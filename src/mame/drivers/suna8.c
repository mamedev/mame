// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

                            -=  SunA 8 Bit Games =-

                    driver by   Luca Elia (l.elia@tin.it)


Main  CPU:      Encrypted Z80 (Epoxy Module)
Sound CPU:      Z80 [Music]  +  Z80 [8 Bit PCM, Optional]
Sound Chips:    AY8910  +  YM3812/YM2203  + DAC x 4 [Optional] + Samples [Optional]


--------------------------------------------------------------------------------------
Year + Game         Game     PCB         Epoxy CPU  Samples  Notes
--------------------------------------------------------------------------------------
88  Hard Head       KRB-14   60138-0083  S562008    Yes      Encryption + Protection
88  Rough Ranger    K030087  ?           S562008    Yes      Not Encrypted
89  Spark Man       KRB-16   60136-081   T568009    Yes      Encryption + Protection
90  Star Fighter    KRB-17   60484-0082  T568009    Yes      Encryption + Protection
91  Hard Head 2     ?        ?           T568009    -        Encryption + Protection
92  Brick Zone      KRB-19   70523-0084  Yes        -        Encryption + Protection
--------------------------------------------------------------------------------------

Notes:

- hardhea2: in test mode press P1&P2 button 2 to see a picture of each level
- Rough Ranger default dipswitch settings are based on the settings listed in
  the Sharp Image licensed Rough Ranger manaul / NOTICE sheet dated 8-5-88
- starfigh video: http://youtu.be/SIwV7wjvnHM -> missing starfield effect!
- brickzn  video: http://youtu.be/yfU1C7A3iZI (recorded from v6.0, Joystick version)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"
#include "sound/dac.h"
#include "sound/samples.h"
#include "includes/suna8.h"

#define SUNA8_MASTER_CLOCK      XTAL_24MHz

/***************************************************************************


                                ROMs Decryption


***************************************************************************/

/***************************************************************************
                                Hard Head
***************************************************************************/

DRIVER_INIT_MEMBER(suna8_state,hardhead)
{
	UINT8 *rom = memregion("maincpu")->base();
	int i;

	for (i = 0; i < 0x8000; i++)
	{
		static const UINT8 swaptable[8] =
		{
			1,1,0,1,1,1,1,0
		};
		int table = ((i & 0x0c00) >> 10) | ((i & 0x4000) >> 12);

		if (swaptable[table])
			rom[i] = BITSWAP8(rom[i], 7,6,5,3,4,2,1,0) ^ 0x58;
	}

	m_bank0d->set_base(memregion("maincpu")->base());
	m_bank1->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
}

/* Non encrypted bootleg */
DRIVER_INIT_MEMBER(suna8_state,hardhedb)
{
	m_bank0d->set_base(memregion("maincpu")->base() + 0x48000);
	m_bank1->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
}

/***************************************************************************
                                Brick Zone
***************************************************************************/

UINT8 *suna8_state::brickzn_decrypt()
{
	UINT8   *RAM    =   memregion("maincpu")->base();
	size_t  size    =   memregion("maincpu")->bytes();
	UINT8   *decrypt = auto_alloc_array(machine(), UINT8, size);
	int i;

	/* Opcodes and data */
	for (i = 0; i < 0x50000; i++)
	{
		static const UINT8 opcode_swaptable[8] =
		{
			1,1,1,0,0,1,1,0
		};
		static const UINT8 data_swaptable[16] =
		{
			1,1,1,0,0,1,1,1,1,0,1,1,1,1,1,1
		};
		int opcode_swap = opcode_swaptable[((i & 0x00c) >> 2) | ((i & 0x040) >> 4)];
		int data_swap = (i >= 0x8000) ? 0 : data_swaptable[(i & 0x003) | ((i & 0x008) >> 1) | ((i & 0x400) >> 7)];
		UINT8 x = RAM[i];

		if (data_swap)
		{
			x = BITSWAP8(x, 7,6,5,4,3,2,0,1);
			RAM[i] = BITSWAP8(x, 7,2,3,4,5,6,1,0) ^ 0x10;
		}

		if (opcode_swap)
			x ^= 0x80;

		if (opcode_swap || data_swap)
			x = BITSWAP8(x, 7,2,3,4,5,6,1,0) ^ 0x10;

		decrypt[i] = x;

		// Alternate data decryption, activated at run-time. Store in higher banks.
		if (i >= 0x10000)
			RAM[i+0x40000] = RAM[i] ^ 0x44;
	}

	return decrypt;
}

DRIVER_INIT_MEMBER(suna8_state, brickzn_common)
{
	m_decrypt = brickzn_decrypt();

	// Non-banked opcodes
	m_bank0d->set_base(m_decrypt);

	// Data banks: 00-0f normal data decryption, 10-1f alternate data decryption:
	m_bank1->configure_entries(0, 16*2, memregion("maincpu")->base() + 0x10000, 0x4000);
	// Opcode banks: 00-1f normal opcode decryption:
	m_bank1d->configure_entries(0, 16, m_decrypt + 0x10000, 0x4000);
	m_bank1d->configure_entries(16, 16, m_decrypt + 0x10000, 0x4000);
}

DRIVER_INIT_MEMBER(suna8_state,brickzn)
{
	DRIVER_INIT_CALL(brickzn_common);

	// !!!!!! PATCHES !!!!!!
	// To do: ROM banking should be disabled here
	m_decrypt[0x11cc] = 0x00; // LD ($C040),A -> NOP
	m_decrypt[0x11cd] = 0x00; // LD ($C040),A -> NOP
	m_decrypt[0x11ce] = 0x00; // LD ($C040),A -> NOP

	m_decrypt[0x335b] = 0xc9; // RET Z -> RET (to avoid: jp $C800)

	// NMI enable / source??
	m_decrypt[0x1442] = 0xc9; // HALT -> RET
	m_decrypt[0x24C6] = 0x00; // HALT -> NOP
	m_decrypt[0x25A4] = 0x00; // HALT -> NOP
}

DRIVER_INIT_MEMBER(suna8_state,brickznv5)
{
	DRIVER_INIT_CALL(brickzn_common);

	// !!!!!! PATCHES !!!!!!
	// To do: ROM banking should be disabled here
	m_decrypt[0x11bb] = 0x00; // LD ($C040),A -> NOP
	m_decrypt[0x11bc] = 0x00; // LD ($C040),A -> NOP
	m_decrypt[0x11bd] = 0x00; // LD ($C040),A -> NOP

	m_decrypt[0x3349] = 0xc9; // RET Z -> RET (to avoid: jp $C800)

	// NMI enable / source??
	m_decrypt[0x1431] = 0xc9; // HALT -> RET
	m_decrypt[0x24b5] = 0x00; // HALT -> NOP
	m_decrypt[0x2593] = 0x00; // HALT -> NOP
}

DRIVER_INIT_MEMBER(suna8_state,brickznv4)
{
	DRIVER_INIT_CALL(brickzn_common);

	// !!!!!! PATCHES !!!!!!
	// To do: ROM banking should be disabled here
	m_decrypt[0x1190] = 0x00; // LD ($C040),A -> NOP
	m_decrypt[0x1191] = 0x00; // LD ($C040),A -> NOP
	m_decrypt[0x1192] = 0x00; // LD ($C040),A -> NOP

	m_decrypt[0x3337] = 0xc9; // RET Z -> RET (to avoid: jp $C800)

	// NMI enable / source??
	m_decrypt[0x1406] = 0xc9; // HALT -> RET
	m_decrypt[0x2487] = 0x00; // HALT -> NOP
	m_decrypt[0x256c] = 0x00; // HALT -> NOP
}

DRIVER_INIT_MEMBER(suna8_state,brickzn11)
{
	m_bank1->configure_entries(0, 16*2, memregion("maincpu")->base() + 0x10000, 0x4000);
}


/***************************************************************************
                                Hard Head 2
***************************************************************************/

DRIVER_INIT_MEMBER(suna8_state,hardhea2)
{
	UINT8   *RAM    =   memregion("maincpu")->base();
	size_t  size    =   memregion("maincpu")->bytes();
	UINT8   *decrypt =  auto_alloc_array(machine(), UINT8, size);
	UINT8 x;
	int i;

	m_bank0d->set_base(decrypt);

	/* Address lines scrambling */
	memcpy(decrypt, RAM, size);
	for (i = 0x00000; i < 0x50000; i++)
	{
/*
0x1000 to scramble:
        dump                screen
rom10:  0y, 1y, 2n, 3n      0y,1y,2n,3n
        4n?,5n, 6n, 7n      4n,5n,6n,7n
        8?, 9n, an, bn      8n,9n,an,bn
        cy, dy, ey?,        cy,dy,en,fn
rom11:                      n
rom12:                      n
rom13:  0?, 1y, 2n, 3n      ?,?,?,? (palettes)
        4n, 5n, 6n, 7?      ?,?,n,n (intro anim)
        8?, 9n?,an, bn      y,y,?,? (player anims)
        cn, dy, en, fn      y,y,n,n
*/
		static const UINT8 swaptable[0x50] =
		{
			1,1,1,1,0,0,1,1,    0,0,0,0,0,0,0,0,    // 8000-ffff not used
			1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0
		};
		int addr = i;

		if (swaptable[(i & 0xff000) >> 12])
			addr = (addr & 0xf0000) | BITSWAP16(addr, 15,14,13,12,11,10,9,8,6,7,5,4,3,2,1,0);

		RAM[i] = decrypt[addr];
	}

	/* Opcodes */
	for (i = 0; i < 0x8000; i++)
	{
		static const UINT8 swaptable[32] =
		{
			1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,
			1,1,0,1,1,1,1,1,1,1,1,1,0,1,0,0
		};
		static const UINT8 xortable[32] =
		{
			0x04,0x04,0x00,0x04,0x00,0x04,0x00,0x00,0x04,0x45,0x00,0x04,0x00,0x04,0x00,0x00,
			0x04,0x45,0x00,0x04,0x00,0x04,0x00,0x00,0x04,0x04,0x00,0x04,0x00,0x04,0x00,0x00
		};
		int table = (i & 1) | ((i & 0x400) >> 9) | ((i & 0x7000) >> 10);

		x = RAM[i];

		x = BITSWAP8(x, 7,6,5,3,4,2,1,0) ^ 0x41 ^ xortable[table];
		if (swaptable[table])
			x = BITSWAP8(x, 5,6,7,4,3,2,1,0);

		decrypt[i] = x;
	}

	/* Data */
	for (i = 0; i < 0x8000; i++)
	{
		static const UINT8 swaptable[8] = { 1,1,0,1,0,1,1,0 };

		if (swaptable[(i & 0x7000) >> 12])
			RAM[i] = BITSWAP8(RAM[i], 5,6,7,4,3,2,1,0) ^ 0x41;
	}

	m_bank1->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
	membank("bank2")->configure_entries(0, 2, auto_alloc_array(machine(), UINT8, 0x2000 * 2), 0x2000);
}


/***************************************************************************
                                Star Fighter
***************************************************************************/

DRIVER_INIT_MEMBER(suna8_state,starfigh)
{
	UINT8   *RAM    =   memregion("maincpu")->base();
	size_t  size    =   memregion("maincpu")->bytes();
	UINT8   *decrypt =  auto_alloc_array(machine(), UINT8, size);
	UINT8 x;
	int i;

	m_bank0d->set_base(decrypt);

	/* Address lines scrambling */
	memcpy(decrypt, RAM, size);
	for (i = 0; i < 0x50000; i++)
	{
		static const UINT8 swaptable[0x50] =
		{
			1,1,1,1,    1,1,0,0,    0,0,0,0,    0,0,0,0,    // 8000-ffff not used
			0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
			0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
			0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
			0,0,0,0,    0,0,0,0,    1,1,0,0,    0,0,0,0     // bank $0e, 9c80 (boss 1) and 8350 (first wave)
		};
		int addr = i;

		if (swaptable[(i & 0xff000) >> 12])
			addr = BITSWAP24(addr, 23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,6,7,5,4,3,2,1,0);

		RAM[i] = decrypt[addr];
	}

	/* Opcodes */
	for (i = 0; i < 0x8000; i++)
	{
		static const UINT8 swaptable[32] =
		{
			0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,
			0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		};
		static const UINT8 xortable[32] =
		{
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x41,0x01,0x00,0x00,0x00,0x00,
			0x01,0x01,0x41,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		};
		int table = (i & 0x7c00) >> 10;

		x = RAM[i];

		x = BITSWAP8(x, 5,6,7,3,4,2,1,0) ^ 0x45 ^ xortable[table];
		if (swaptable[table])
			x = BITSWAP8(x, 5,6,7,4,3,2,1,0) ^ 0x04;

		decrypt[i] = x;
	}

	/* Data */
	for (i = 0; i < 0x8000; i++)
	{
		static const UINT8 swaptable[8] = { 1,1,0,1,0,1,1,0 };

		if (swaptable[(i & 0x7000) >> 12])
			RAM[i] = BITSWAP8(RAM[i], 5,6,7,4,3,2,1,0) ^ 0x45;
	}


	// !!!!!! PATCHES !!!!!!

	decrypt[0x07c0] = 0xc9; // c080 bit 7 protection check

//  decrypt[0x083e] = 0x00; // sound latch disabling
//  decrypt[0x083f] = 0x00; // ""
//  decrypt[0x0840] = 0x00; // ""

//  decrypt[0x0cef] = 0xc9; // rombank latch check, corrupt d12d

	decrypt[0x2696] = 0xc9; // work ram writes disable, corrupt next routine
	decrypt[0x4e9a] = 0x00; // work ram writes disable, flip background sprite

	m_bank1->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
}


/***************************************************************************
                                Spark Man
***************************************************************************/

DRIVER_INIT_MEMBER(suna8_state,sparkman)
{
	UINT8   *RAM    =   memregion("maincpu")->base();
	size_t  size    =   memregion("maincpu")->bytes();
	UINT8   *decrypt =  auto_alloc_array(machine(), UINT8, size);
	UINT8 x;
	int i;

	m_bank0d->set_base(decrypt);

	/* Address lines scrambling */
	memcpy(decrypt, RAM, size);
	for (i = 0; i < 0x50000; i++)
	{
		static const UINT8 swaptable[0x50] =
		{
			1,1,1,1,    0,0,1,1,    0,0,0,0,    0,0,0,0,    // 8000-ffff not used
			0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
			0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
			0,0,0,0,    0,0,0,0,    0,0,0,0,    0,0,0,0,
			0,0,0,0,    0,0,0,0,    1,1,0,0,    0,0,0,0     // bank $0e, $8xxx, $9xxx (hand in title screen)
		};
		int addr = i;

		if (swaptable[(i & 0xff000) >> 12])
			addr = BITSWAP24(addr, 23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,7,8,6,5,4,3,2,1,0);

		RAM[i] = decrypt[addr];
	}

	/* Opcodes */
	for (i = 0; i < 0x8000; i++)
	{
		static const UINT8 swaptable[32] =
		{
			0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,1,
			0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0
		};
		static const UINT8 xortable[32] =
		{
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00
		};
		int table = (i & 0x7c00) >> 10;

		x = RAM[i];

		x = BITSWAP8(x, 5,6,7,3,4,2,1,0) ^ 0x44 ^ xortable[table];
		if (swaptable[table])
			x = BITSWAP8(x, 5,6,7,4,3,2,1,0) ^ 0x04;

		decrypt[i] = x;
	}

	/* Data */
	for (i = 0; i < 0x8000; i++)
	{
		static const UINT8 swaptable[8] = { 1,1,1,0,1,1,0,1 };

		if (swaptable[(i & 0x7000) >> 12])
			RAM[i] = BITSWAP8(RAM[i], 5,6,7,4,3,2,1,0) ^ 0x44;
	}

	// !!!!!! PATCHES !!!!!!

	// c083 bit 7 protection
	decrypt[0x0ee0] = 0x00;
	decrypt[0x0ee1] = 0x00;
	decrypt[0x0ee2] = 0x00;

	// c083 bit 7 protection
	decrypt[0x1ac3] = 0x00;
	decrypt[0x1ac4] = 0x00;
	decrypt[0x1ac5] = 0x00;

	m_bank1->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
}

/***************************************************************************


                                Protection


***************************************************************************/

/***************************************************************************
                                Hard Head
***************************************************************************/

READ8_MEMBER(suna8_state::hardhead_protection_r)
{
	UINT8 protection_val = m_protection_val;

	if (protection_val & 0x80)
		return  ((~offset & 0x20)           ?   0x20 : 0) |
				((protection_val & 0x04)    ?   0x80 : 0) |
				((protection_val & 0x01)    ?   0x04 : 0);
	else
		return  ((~offset & 0x20)                   ?   0x20 : 0) |
				(((offset ^ protection_val) & 0x01) ?   0x84 : 0);
}

WRITE8_MEMBER(suna8_state::hardhead_protection_w)
{
	if (data & 0x80)    m_protection_val = data;
	else                m_protection_val = offset & 1;
}


/***************************************************************************


                            Memory Maps - Main CPU


***************************************************************************/

/***************************************************************************
                                Hard Head
***************************************************************************/

READ8_MEMBER(suna8_state::hardhead_ip_r)
{
	switch (*m_hardhead_ip)
	{
		case 0: return ioport("P1")->read();
		case 1: return ioport("P2")->read();
		case 2: return ioport("DSW1")->read();
		case 3: return ioport("DSW2")->read();
		default:
			logerror("CPU #0 - PC %04X: Unknown IP read: %02X\n", space.device().safe_pc(), *m_hardhead_ip);
			return 0xff;
	}
}

/*
    765- ----   Unused (eg. they go into hardhead_flipscreen_w)
    ---4 ----
    ---- 3210   ROM Bank
*/
WRITE8_MEMBER(suna8_state::hardhead_bankswitch_w)
{
	int bank = data & 0x0f;

	if (data & ~0xef)   logerror("CPU #0 - PC %04X: unknown bank bits: %02X\n",space.device().safe_pc(),data);
	m_bank1->set_entry(bank);
}


/*
    765- ----
    ---4 3---   Coin Lockout
    ---- -2--   Flip Screen
    ---- --10
*/
WRITE8_MEMBER(suna8_state::hardhead_flipscreen_w)
{
	flip_screen_set(data & 0x04);
	coin_lockout_w ( machine(), 0,  data & 0x08);
	coin_lockout_w ( machine(), 1,  data & 0x10);
}

static ADDRESS_MAP_START( hardhead_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                             // ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")                        // Banked ROM
	AM_RANGE(0xc000, 0xd7ff) AM_RAM                             // RAM
	AM_RANGE(0xd800, 0xd9ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // Palette
	AM_RANGE(0xda00, 0xda00) AM_RAM_READ(hardhead_ip_r) AM_SHARE("hardhead_ip") // Input Port Select
	AM_RANGE(0xda80, 0xda80) AM_READ(soundlatch2_byte_r) AM_WRITE(hardhead_bankswitch_w )   // ROM Banking
	AM_RANGE(0xdb00, 0xdb00) AM_WRITE(soundlatch_byte_w         )   // To Sound CPU
	AM_RANGE(0xdb80, 0xdb80) AM_WRITE(hardhead_flipscreen_w )   // Flip Screen + Coin Lockout
	AM_RANGE(0xdc00, 0xdc00) AM_NOP                             // <- R (after bank select)
	AM_RANGE(0xdc80, 0xdc80) AM_NOP                             // <- R (after bank select)
	AM_RANGE(0xdd00, 0xdd00) AM_NOP                             // <- R (after ip select)
	AM_RANGE(0xdd80, 0xddff) AM_READWRITE(hardhead_protection_r, hardhead_protection_w  )   // Protection
	AM_RANGE(0xe000, 0xffff) AM_RAM_WRITE(suna8_spriteram_w) AM_SHARE("spriteram")  // Sprites
ADDRESS_MAP_END

static ADDRESS_MAP_START( hardhead_io_map, AS_IO, 8, suna8_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READNOP // ? IRQ Ack
ADDRESS_MAP_END

/***************************************************************************
                                Rough Ranger
***************************************************************************/

/*
    76-- ----   Coin Lockout
    --5- ----   Flip Screen
    ---4 ----   ROM Bank
    ---- 3---
    ---- -210   ROM Bank
*/
WRITE8_MEMBER(suna8_state::rranger_bankswitch_w)
{
	int bank = data & 0x07;
	if ((~data & 0x10) && (bank >= 4))  bank += 4;

	if (data & ~0xf7)   logerror("CPU #0 - PC %04X: unknown bank bits: %02X\n",space.device().safe_pc(),data);

	m_bank1->set_entry(bank);

	flip_screen_set(data & 0x20);
	coin_lockout_w ( machine(), 0,  data & 0x40);
	coin_lockout_w ( machine(), 1,  data & 0x80);
}

/*
    7--- ----   1 -> Garbled title (another romset?)
    -654 ----
    ---- 3---   1 -> No sound (soundlatch full?)
    ---- -2--
    ---- --1-   1 -> Interlude screens
    ---- ---0
*/
READ8_MEMBER(suna8_state::rranger_soundstatus_r)
{
	soundlatch2_byte_r(space, offset);
	return 0x02;
}

WRITE8_MEMBER(suna8_state::sranger_prot_w)
{
	/* check code at 0x2ce2 (in sranger), protection is so dire that I can't even exactly
	   estabilish if what I'm doing can be considered or not a kludge... -AS */
	space.write_byte(0xcd99,0xff);
}

static ADDRESS_MAP_START( rranger_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                             // ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")                        // Banked ROM
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(watchdog_reset_r, soundlatch_byte_w)  // To Sound CPU
	AM_RANGE(0xc002, 0xc002) AM_WRITE(rranger_bankswitch_w  )   // ROM Banking
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("P1")                 // P1 (Inputs)
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("P2")                 // P2
	AM_RANGE(0xc004, 0xc004) AM_READ(rranger_soundstatus_r  )   // Latch Status?
	AM_RANGE(0xc200, 0xc200) AM_READNOP AM_WRITE(sranger_prot_w)// Protection?
	AM_RANGE(0xc280, 0xc280) AM_WRITENOP    // ? NMI Ack
	AM_RANGE(0xc280, 0xc280) AM_READ_PORT("DSW1")               // DSW 1
	AM_RANGE(0xc2c0, 0xc2c0) AM_READ_PORT("DSW2")               // DSW 2
	AM_RANGE(0xc600, 0xc7ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // Palette
	AM_RANGE(0xc800, 0xdfff) AM_RAM                                                                     // Work RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM_WRITE(suna8_spriteram_w) AM_SHARE("spriteram")                      // Sprites
ADDRESS_MAP_END


static ADDRESS_MAP_START( rranger_io_map, AS_IO, 8, suna8_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READNOP // ? IRQ Ack
ADDRESS_MAP_END

/***************************************************************************
                                Brick Zone
***************************************************************************/

/*
  C140:  7--- ----   Finish Stage (Cheat)
         -654 3---
         ---- -2--   Must flip rapidly?
         ---- --1-
         ---- ---0   Use Cheat 1 and 2 (driver config)
*/
READ8_MEMBER(suna8_state::brickzn_cheats_r)
{
	static UINT8 bit2 = 0;
	bit2 = 1 - bit2;    // see code at 2b48
	return
		(ioport("CHEATS")->read() & (~(1 << 2))) |
		(bit2 << 2);
}

/*
 (C060 in newer sets)
  C040:  7654 32--   Protection (e.g. select output of multi_w, newer sets only)
         ---- --1-   Sprite RAM Bank
         ---- ---0   Flip Screen
*/
WRITE8_MEMBER(suna8_state::brickzn_sprbank_w)
{
	m_protection_val = data;

	flip_screen_set(data & 0x01);
	m_spritebank = (data >> 1) & 1;

	logerror("CPU #0 - PC %04X: protection_val = %02X\n",space.device().safe_pc(),data);
//  if (data & ~0x03)   logerror("CPU #0 - PC %04X: unknown spritebank bits: %02X\n",space.device().safe_pc(),data);
}

/*
 (C040 in newer sets)
  C060:  7654 ----
         ---- 3210   ROM Bank
*/
WRITE8_MEMBER(suna8_state::brickzn_rombank_w)
{
	int bank = data & 0x0f;

	if (data & ~0x0f)   logerror("CPU #0 - PC %04X: unknown rom bank bits: %02X\n",space.device().safe_pc(),data);

	m_bank1->set_entry(bank + (m_bank1->entry() & 0x10));
	if(m_bank1d)
		m_bank1d->set_entry(m_bank1->entry());

	m_rombank = data;
}

/*
 (C0A0 in newer sets)
  C080:  7654 3---
         ---- -2--   Coin Counter
         ---- --1-   Start 2 Led
         ---- ---0   Start 1 Led
*/
WRITE8_MEMBER(suna8_state::brickzn_leds_w)
{
	set_led_status(machine(), 0, data & 0x01);
	set_led_status(machine(), 1, data & 0x02);
	coin_counter_w(machine(), 0, data & 0x04);

	logerror("CPU #0 - PC %04X: leds = %02X\n",space.device().safe_pc(),data);
	if (data & ~0x07)   logerror("CPU #0 - PC %04X: unknown leds bits: %02X\n",space.device().safe_pc(),data);
}

/*
  C0A0:  7654 321-
         ---- ---0   Palette RAM Bank
*/
WRITE8_MEMBER(suna8_state::brickzn_palbank_w)
{
	m_palettebank = data & 0x01;

	logerror("CPU #0 - PC %04X: palettebank = %02X\n",space.device().safe_pc(),data);
	if (data & ~0x01)   logerror("CPU #0 - PC %04X: unknown palettebank bits: %02X\n",space.device().safe_pc(),data);
}

static ADDRESS_MAP_START( brickzn11_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                             // ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")                // Banked ROM

	AM_RANGE(0xc000, 0xc000) AM_WRITE(soundlatch_byte_w     )   // To Sound CPU
	AM_RANGE(0xc040, 0xc040) AM_WRITE(brickzn_sprbank_w     )   // Sprite RAM Bank + Flip Screen + Protection
	AM_RANGE(0xc060, 0xc060) AM_WRITE(brickzn_rombank_w     )   // ROM Bank
	AM_RANGE(0xc080, 0xc080) AM_WRITE(brickzn_leds_w        )   // Leds
	AM_RANGE(0xc0a0, 0xc0a0) AM_WRITE(brickzn_palbank_w     )   // Palette RAM Bank
//  AM_RANGE(0xc0c0, 0xc0c0) AM_WRITE(brickzn_prot2_w       )   // Protection 2

	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("P1")                 // P1 (Buttons)
	AM_RANGE(0xc101, 0xc101) AM_READ_PORT("P2")                 // P2 (Buttons)
	AM_RANGE(0xc102, 0xc102) AM_READ_PORT("DSW1")               // DSW 1
	AM_RANGE(0xc103, 0xc103) AM_READ_PORT("DSW2")               // DSW 2
	AM_RANGE(0xc108, 0xc108) AM_READ_PORT("SPIN1")              // P1 (Spinner)
	AM_RANGE(0xc10c, 0xc10c) AM_READ_PORT("SPIN2")              // P2 (Spinner)

	AM_RANGE(0xc140, 0xc140) AM_READ(brickzn_cheats_r)          // Cheats / Debugging Inputs

	AM_RANGE(0xc600, 0xc7ff) AM_READWRITE(banked_paletteram_r, brickzn_banked_paletteram_w) AM_SHARE("paletteram")      // Palette (Banked)
	AM_RANGE(0xc800, 0xdfff) AM_RAM AM_SHARE("wram")                                            // Work RAM
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(suna8_banked_spriteram_r, suna8_banked_spriteram_w)   // Sprites (Banked)
ADDRESS_MAP_END

/*
  (newer sets only)

  C0A0:  Palette RAM Bank

  C0A0:  Sound Latch (optionally scrambled)

  C0A0:  Leds
*/
WRITE8_MEMBER(suna8_state::brickzn_multi_w)
{
	int protselect = m_protection_val & 0xfc;

	if ((protselect == 0x88) || (protselect == 0x8c))
	{
		brickzn_palbank_w(space, offset, data, mem_mask);
	}
	else if (protselect == 0x90)
	{
		/*
		    0d  brick hit       NO!     25?
		    2c  side wall hit   OK
		    3b  paddle hit      OK
		    44  death           OK?
		    53  death           OK?
		    56  coin in         OK?
		    70  monster hit     NO?     58?
		*/
		UINT8 remap = (m_remap_sound ? BITSWAP8(data, 7,6,3,4,5,2,1,0) : data);

		soundlatch_byte_w(space, 0, remap);

		logerror("CPU #0 - PC %04X: soundlatch = %02X (->%02X)\n",space.device().safe_pc(),data,remap);
	}
	else if (protselect == 0x04)
	{
		brickzn_leds_w(space, offset, data, mem_mask);
	}
	else if (protselect == 0x80)
	{
		// disables rom banking?
		// see code at 11b1:

		logerror("CPU #0 - PC %04X: rombank_disable = %02X\n",space.device().safe_pc(),data);
	}
	else
	{
		logerror("CPU #0 - PC %04X: ignore = %02X\n",space.device().safe_pc(),data);
	}

	if ((m_protection_val & 0x1f) == 0x1c)
	{
		// controls opcode decryption
		// see code at 71b, 45b7, 7380, 7a6b
		//printf("CPU #0 - PC %04X: alt op-decrypt tog = %02X\n",space.device().safe_pc(),data);
		m_prot_opcode_toggle ^= 1;

		if (m_prot_opcode_toggle == 0)
		{
			m_bank0d->set_base(m_decrypt);
		}
		else
		{
			m_bank0d->set_base(memregion("maincpu")->base());
		}
	}
}

/*
  (newer sets only)
  C0C0: two protection values written in rapid succession
*/
WRITE8_MEMBER(suna8_state::brickzn_prot2_w)
{
	// Disable work RAM write, see code at 96a:
	if ((m_prot2 ^ data) == 0x24)
		space.unmap_write(0xc800, 0xdfff);
	else
		space.install_ram(0xc800, 0xdfff, m_wram);

	m_remap_sound = ((m_prot2 ^ data) == 0xf8) ? 1 : 0;

	// Select alternate data decryption, see code at 787e:
	m_bank1->set_entry((m_bank1->entry() & 0x0f) + ((m_prot2 == (data | 0xdc)) ? 0x10 : 0));
	if(m_bank1d)
		m_bank1d->set_entry(m_bank1->entry());

	m_prot2_prev = m_prot2;
	m_prot2 = data;

	logerror("CPU #0 - PC %04X: unknown = %02X\n",space.device().safe_pc(),data);
}

// (newer sets only) Disable palette RAM writes, see code at 4990:
WRITE8_MEMBER(suna8_state::brickzn_enab_palram_w)
{
	m_paletteram_enab = 1;
}
WRITE8_MEMBER(suna8_state::brickzn_disab_palram_w)
{
	m_paletteram_enab = 0;
}

static ADDRESS_MAP_START( brickzn_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                             // ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")                // Banked ROM

	// c000 writes before reading buttons
	// c010 writes?
	AM_RANGE(0xc040, 0xc040) AM_WRITE(brickzn_rombank_w     )   // ROM Bank
	AM_RANGE(0xc060, 0xc060) AM_WRITE(brickzn_sprbank_w     )   // Sprite RAM Bank + Flip Screen + Protection
	// c080 writes?
	// c090 writes?
	AM_RANGE(0xc0a0, 0xc0a0) AM_WRITE(brickzn_multi_w       )   // Palette RAM Bank / Sound Latch / ...
	AM_RANGE(0xc0c0, 0xc0c0) AM_WRITE(brickzn_prot2_w       )   // Protection 2

	AM_RANGE(0xc100, 0xc100) AM_READ_PORT("P1")                 // P1 (Buttons)
	AM_RANGE(0xc101, 0xc101) AM_READ_PORT("P2")                 // P2 (Buttons)
	AM_RANGE(0xc102, 0xc102) AM_READ_PORT("DSW1")               // DSW 1
	AM_RANGE(0xc103, 0xc103) AM_READ_PORT("DSW2")               // DSW 2
	AM_RANGE(0xc108, 0xc108) AM_READ_PORT("SPIN1")              // P1 (Spinner)
	AM_RANGE(0xc10c, 0xc10c) AM_READ_PORT("SPIN2")              // P2 (Spinner)

	AM_RANGE(0xc140, 0xc140) AM_READ(brickzn_cheats_r)          // Cheats / Debugging Inputs
	// c144 reads?
	// c14a reads?

	AM_RANGE(0xc600, 0xc7ff) AM_READWRITE(banked_paletteram_r, brickzn_banked_paletteram_w) AM_SHARE("paletteram")      // Palette (Banked)
	AM_RANGE(0xc800, 0xdfff) AM_RAM AM_SHARE("wram")                                            // Work RAM
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(suna8_banked_spriteram_r, suna8_banked_spriteram_w)   // Sprites (Banked)
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, suna8_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK("bank0d")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1d")
ADDRESS_MAP_END

static ADDRESS_MAP_START( brickzn_io_map, AS_IO, 8, suna8_state )
	AM_RANGE(0x0000, 0x0000) AM_WRITE(brickzn_disab_palram_w    )   // Disable Palette RAM
	AM_RANGE(0x00a1, 0x00a1) AM_WRITE(brickzn_enab_palram_w     )   // Enable Palette RAM
ADDRESS_MAP_END

/***************************************************************************
                                Hard Head 2
***************************************************************************/

/* Probably wrong: */
WRITE8_MEMBER(suna8_state::hardhea2_nmi_w)
{
	m_nmi_enable = data & 0x01;
//  if (data & ~0x01)   logerror("CPU #0 - PC %04X: unknown nmi bits: %02X\n",space.device().safe_pc(),data);
}

/*
    7654 321-
    ---- ---0   Flip Screen
*/
WRITE8_MEMBER(suna8_state::hardhea2_flipscreen_w)
{
	flip_screen_set(data & 0x01);
	if (data & ~0x01)   logerror("CPU #0 - PC %04X: unknown flipscreen bits: %02X\n",space.device().safe_pc(),data);
}

WRITE8_MEMBER(suna8_state::hardhea2_leds_w)
{
	set_led_status(machine(), 0, data & 0x01);
	set_led_status(machine(), 1, data & 0x02);
	coin_counter_w(machine(), 0, data & 0x04);
	if (data & ~0x07)   logerror("CPU #0 - PC %04X: unknown leds bits: %02X\n",space.device().safe_pc(),data);
}

/*
    7654 32--
    ---- --1-   Sprite RAM Bank
    ---- ---0   Sprite RAM Bank?
*/
WRITE8_MEMBER(suna8_state::hardhea2_spritebank_w)
{
	m_spritebank = (data >> 1) & 1;
	if (data & ~0x02)   logerror("CPU #0 - PC %04X: unknown spritebank bits: %02X\n",space.device().safe_pc(),data);
}

/*
    7654 ----
    ---- 3210   ROM Bank
*/
WRITE8_MEMBER(suna8_state::hardhea2_rombank_w)
{
	int bank = data & 0x0f;

	if (data & ~0x0f)   logerror("CPU #0 - PC %04X: unknown rom bank bits: %02X\n",space.device().safe_pc(),data);

	m_bank1->set_entry(bank);

	m_rombank = data;
}

WRITE8_MEMBER(suna8_state::hardhea2_spritebank_0_w)
{
	m_spritebank = 0;
}
WRITE8_MEMBER(suna8_state::hardhea2_spritebank_1_w)
{
	m_spritebank = 1;
}

WRITE8_MEMBER(suna8_state::hardhea2_rambank_0_w)
{
	membank("bank2")->set_entry(0);
}

WRITE8_MEMBER(suna8_state::hardhea2_rambank_1_w)
{
	membank("bank2")->set_entry(1);
}


static ADDRESS_MAP_START( hardhea2_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                                 // ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")                            // Banked ROM
	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("P1")                     // P1 (Inputs)
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("P2")                     // P2
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("DSW1")                   // DSW 1
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW2")                   // DSW 2
	AM_RANGE(0xc080, 0xc080) AM_READ_PORT("BUTTONS")                // vblank?
	AM_RANGE(0xc200, 0xc200) AM_WRITE(hardhea2_spritebank_w     )   // Sprite RAM Bank
	AM_RANGE(0xc280, 0xc280) AM_WRITE(hardhea2_rombank_w            )   // ROM Bank (?mirrored up to c2ff?)

	// *** Protection
	AM_RANGE(0xc28c, 0xc28c) AM_WRITE(hardhea2_rombank_w        )
	// Protection ***

	AM_RANGE(0xc300, 0xc300) AM_WRITE(hardhea2_flipscreen_w     )   // Flip Screen
	AM_RANGE(0xc380, 0xc380) AM_WRITE(hardhea2_nmi_w                )   // ? NMI related ?
	AM_RANGE(0xc400, 0xc400) AM_WRITE(hardhea2_leds_w               )   // Leds + Coin Counter
	AM_RANGE(0xc480, 0xc480) AM_WRITENOP    // ~ROM Bank
	AM_RANGE(0xc500, 0xc500) AM_WRITE(soundlatch_byte_w             )   // To Sound CPU

	// *** Protection
	AM_RANGE(0xc50f, 0xc50f) AM_WRITE(hardhea2_spritebank_1_w )
	AM_RANGE(0xc508, 0xc508) AM_WRITE(hardhea2_spritebank_0_w )

	AM_RANGE(0xc507, 0xc507) AM_WRITE(hardhea2_rambank_1_w )
	AM_RANGE(0xc522, 0xc522) AM_WRITE(hardhea2_rambank_0_w )

	AM_RANGE(0xc556, 0xc556) AM_WRITE(hardhea2_rambank_1_w )
	AM_RANGE(0xc528, 0xc528) AM_WRITE(hardhea2_rambank_0_w )

	AM_RANGE(0xc560, 0xc560) AM_WRITE(hardhea2_rambank_1_w )
	AM_RANGE(0xc533, 0xc533) AM_WRITE(hardhea2_rambank_0_w )
	// Protection ***

	AM_RANGE(0xc600, 0xc7ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // Palette
	AM_RANGE(0xc800, 0xdfff) AM_RAMBANK("bank2")                                                        // Work RAM (Banked)
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(suna8_banked_spriteram_r, suna8_banked_spriteram_w)           // Sprites (Banked)
ADDRESS_MAP_END


/***************************************************************************
                                Star Fighter
***************************************************************************/

/*
  C280-C2FF:  76-- ----
              --5- ----   Disable Sound Latch Writes
              ---4 ----
              ---- 3210   ROM Bank (Latched)
*/
WRITE8_MEMBER(suna8_state::starfigh_rombank_latch_w)
{
	logerror("CPU #0 - PC %04X: rom bank latch %04X = %02X\n",space.device().safe_pc(), 0xc280 + offset, data);
	m_rombank_latch = data;
}

/*
  C500:  Sound Latch
*/
WRITE8_MEMBER(suna8_state::starfigh_sound_latch_w)
{
	if ( !(m_rombank_latch & 0x20) )
		soundlatch_byte_w(space, 0, data);
}

/*
  C080:
*/
READ8_MEMBER(suna8_state::starfigh_cheats_r)
{
	return ioport("CHEATS")->read();
}

/*
  C380-C3FF:
*/
WRITE8_MEMBER(suna8_state::starfigh_spritebank_latch_w)
{
	// bit 1 = disable RAM writes. See code at 2696, 4e8f
	m_spritebank_latch  =   (data >> 2) & 1;
	m_nmi_enable        =   (data >> 5) & 1;    // see code at 1c2, 491, 4aa, 4e9b
	if (data & ~0x04)   logerror("CPU #0 - PC %04X: unknown spritebank bits: %02X\n",space.device().safe_pc(),data);
}

/*
  C200:
*/
WRITE8_MEMBER(suna8_state::starfigh_spritebank_w)
{
	m_spritebank = m_spritebank_latch;
}

/*
  C400:  7654 ----
         ---- 3---   Gfx banking (bosses)
         ---- -2--   Coin Counter
         ---- --1-   Start 2 Led
         ---- ---0   Start 1 Led

  Writes to C400 also set ROM bank from latch
*/
WRITE8_MEMBER(suna8_state::starfigh_leds_w)
{
	set_led_status(machine(), 0,     data & 0x01);
	set_led_status(machine(), 1,     data & 0x02);
	coin_counter_w(machine(), 0,     data & 0x04);
	m_gfxbank       =               (data & 0x08) ? 4 : 0;
	if (data & ~0x0f)   logerror("CPU #0 - PC %04X: unknown leds bits: %02X\n",space.device().safe_pc(),data);

	// ROM Bank:

	int bank = m_rombank_latch & 0x0f;

	m_bank1->set_entry(bank);

	m_rombank = m_rombank_latch;
	logerror("CPU #0 - PC %04X: rom bank = %02X\n",space.device().safe_pc(), m_rombank);
}

static ADDRESS_MAP_START( starfigh_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                                     // ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")                        // Banked ROM

	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("P1")                         // P1 (Inputs)
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("P2")                         // P2
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("DSW1")                       // DSW 1
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW2")                       // DSW 2
	AM_RANGE(0xc080, 0xc080) AM_READ(starfigh_cheats_r              )   // Cheats?

	AM_RANGE(0xc200, 0xc200) AM_WRITE(starfigh_spritebank_w         )   // Sprite RAM Bank
	AM_RANGE(0xc280, 0xc2ff) AM_WRITE(starfigh_rombank_latch_w      )   // ROM Bank Latch (?mirrored up to c2ff?)
	AM_RANGE(0xc300, 0xc300) AM_WRITE(hardhea2_flipscreen_w         )   // Flip Screen
	AM_RANGE(0xc380, 0xc3ff) AM_WRITE(starfigh_spritebank_latch_w   )   // Sprite RAM Bank Latch
	AM_RANGE(0xc400, 0xc47f) AM_WRITE(starfigh_leds_w               )   // Leds + Coin Counter + ROM Bank
//  c480 write?
	AM_RANGE(0xc500, 0xc500) AM_WRITE(starfigh_sound_latch_w        )   // To Sound CPU (can be disabled)
//  (c522 + R & 0x1f) write?

	AM_RANGE(0xc600, 0xc7ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // Palette
	AM_RANGE(0xc800, 0xdfff) AM_RAM                                                                     // Work RAM
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(suna8_banked_spriteram_r, suna8_banked_spriteram_w)           // Sprites (Banked)
ADDRESS_MAP_END


/***************************************************************************
                                Spark Man
***************************************************************************/

/*
    C200: 765432--
          ------1-   Sprite RAM Bank (Inverted by Sprite Bank Latch)
          -------0   Sprite "chip"   ""
*/
WRITE8_MEMBER(suna8_state::sparkman_spritebank_w)
{
	m_spritebank = ((data >> 1) & 0x01) | ((data << 1) & 0x02);
	if ((m_spritebank_latch >> 1) & 0x01)
		m_spritebank ^= 0x03;

	logerror("CPU #0 - PC %04X: spritebank = %02X (%X)\n",space.device().safe_pc(),data,m_spritebank);
}

/*
    C280:  76-- ----
           --5- ----   Disable Sound Latch Writes
           ---4 ----
           ---- 3210   ROM Bank (Latched)
*/
WRITE8_MEMBER(suna8_state::sparkman_rombank_latch_w)
{
	m_rombank_latch = data;
	logerror("CPU #0 - PC %04X: rom bank latch %04X = %02X\n",space.device().safe_pc(), 0xc280 + offset, data);
}

/*
    C300: 76-- ----
          --5- ----   Invert Sprite Chip and Bank
          ---4 ----   Almost Always On?
          ---- 321-
          ---- ---0   Flip Screen
*/
WRITE8_MEMBER(suna8_state::sparkman_spritebank_latch_w)
{
	flip_screen_set(data & 0x01);
	m_spritebank_latch  =   (data >> 4) & 0x03;
	logerror("CPU #0 - PC %04X: spritebank latch = %02X\n",space.device().safe_pc(),data);
}

/*
    C380: 76------
          --5-----   NMI Enable
          ---4321-
          -------0   Work RAM Writes Disable
*/
WRITE8_MEMBER(suna8_state::sparkman_write_disable_w)
{
	m_write_disable     =   (data >> 0) & 1;    // bit 0 = disable RAM writes. See code at b48, d4d
	m_nmi_enable        =   (data >> 5) & 1;    // see code at 66
	if (data & ~0x21)   logerror("CPU #0 - PC %04X: unknown spritebank bits: %02X\n",space.device().safe_pc(),data);
}

// RAM writes can be disabled
WRITE8_MEMBER(suna8_state::suna8_wram_w)
{
	if (!m_write_disable)
		m_wram[offset] = data;
}

/*
  C400:  7654 32--
         ---- --1-   Start 2 Led
         ---- ---0   Start 1 Led

  Writes to C400 also set ROM bank from latch
*/
WRITE8_MEMBER(suna8_state::sparkman_rombank_w)
{
	set_led_status(machine(), 0,     data & 0x01);
	set_led_status(machine(), 1,     data & 0x02);

	if (data & ~0x03)   logerror("CPU #0 - PC %04X: unknown leds bits: %02X\n",space.device().safe_pc(),data);

	// ROM Bank:

	int bank = m_rombank_latch & 0x0f;

	m_bank1->set_entry(bank);

	m_rombank = m_rombank_latch;
	logerror("CPU #0 - PC %04X: rom bank = %02X\n",space.device().safe_pc(), m_rombank);
}

/*
    C480: 7654321-
          -------0   Coin Counter
*/
WRITE8_MEMBER(suna8_state::sparkman_coin_counter_w)
{
	coin_counter_w(machine(), 0, data & 0x01);
}

// To do: implement this, affects the duration of copyright screen
READ8_MEMBER(suna8_state::sparkman_c0a3_r)
{
	return (m_screen->frame_number() & 1) ? 0x80 : 0;
}

static ADDRESS_MAP_START( sparkman_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM                                     // ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")                        // Banked ROM

	AM_RANGE(0xc000, 0xc000) AM_READ_PORT("P1")                         // P1 (Inputs)
	AM_RANGE(0xc001, 0xc001) AM_READ_PORT("P2")                         // P2
	AM_RANGE(0xc002, 0xc002) AM_READ_PORT("DSW1")                       // DSW 1
	AM_RANGE(0xc003, 0xc003) AM_READ_PORT("DSW2")                       // DSW 2
	AM_RANGE(0xc080, 0xc080) AM_READ_PORT("BUTTONS")                    // Buttons
	AM_RANGE(0xc0a3, 0xc0a3) AM_READ(sparkman_c0a3_r                )   // ???

	AM_RANGE(0xc200, 0xc27f) AM_WRITE(sparkman_spritebank_w         )   // Sprite RAM Bank
	AM_RANGE(0xc280, 0xc2ff) AM_WRITE(sparkman_rombank_latch_w      )   // ROM Bank Latch
	AM_RANGE(0xc300, 0xc37f) AM_WRITE(sparkman_spritebank_latch_w   )   // Sprite RAM Bank Latch (Invert) + Flip Screen
	AM_RANGE(0xc380, 0xc3ff) AM_WRITE(sparkman_write_disable_w      )   // Work RAM Writes Disable + NMI Enable
	AM_RANGE(0xc400, 0xc47f) AM_WRITE(sparkman_rombank_w            )   // ROM Bank + Leds
	AM_RANGE(0xc480, 0xc480) AM_WRITE(sparkman_coin_counter_w       )   // Coin Counter
	AM_RANGE(0xc500, 0xc57f) AM_WRITE(starfigh_sound_latch_w        )   // To Sound CPU (can be disabled)

	AM_RANGE(0xc600, 0xc7ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // Palette
	AM_RANGE(0xc800, 0xdfff) AM_RAM_WRITE(suna8_wram_w) AM_SHARE("wram")                        // RAM
	AM_RANGE(0xe000, 0xffff) AM_READWRITE(suna8_banked_spriteram_r, suna8_banked_spriteram_w)   // Sprites (Banked)
ADDRESS_MAP_END


/***************************************************************************


                            Memory Maps - Sound CPU(s)


***************************************************************************/

/***************************************************************************
                                Hard Head
***************************************************************************/

static ADDRESS_MAP_START( hardhead_sound_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM // ROM
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
	AM_RANGE(0xa002, 0xa003) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM // RAM
	AM_RANGE(0xc800, 0xc800) AM_DEVREAD("ymsnd", ym3812_device, status_port_r)   // ? unsure
	AM_RANGE(0xd000, 0xd000) AM_WRITE(soundlatch2_byte_w                )   //
	AM_RANGE(0xd800, 0xd800) AM_READ(soundlatch_byte_r              )   // From Main CPU
ADDRESS_MAP_END


static ADDRESS_MAP_START( hardhead_sound_io_map, AS_IO, 8, suna8_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READNOP // ? IRQ Ack
ADDRESS_MAP_END


/***************************************************************************
                                Rough Ranger
***************************************************************************/

static ADDRESS_MAP_START( rranger_sound_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM // ROM
	AM_RANGE(0xa000, 0xa001) AM_DEVWRITE("ym1", ym2203_device, write)   // Samples + Music
	AM_RANGE(0xa002, 0xa003) AM_DEVWRITE("ym2", ym2203_device, write)   // Music + FX
	AM_RANGE(0xc000, 0xc7ff) AM_RAM // RAM
	AM_RANGE(0xd000, 0xd000) AM_WRITE(soundlatch2_byte_w                )   // To Sound CPU
	AM_RANGE(0xd800, 0xd800) AM_READ(soundlatch_byte_r                  )   // From Main CPU
ADDRESS_MAP_END


/***************************************************************************
                                Brick Zone
***************************************************************************/

static ADDRESS_MAP_START( brickzn_sound_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM // ROM
	AM_RANGE(0xc000, 0xc001) AM_DEVWRITE("ymsnd", ym3812_device, write)
	AM_RANGE(0xc002, 0xc003) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM // RAM
	AM_RANGE(0xf000, 0xf000) AM_WRITE(soundlatch2_byte_w                )   // To PCM CPU
	AM_RANGE(0xf800, 0xf800) AM_READ(soundlatch_byte_r                  )   // From Main CPU
ADDRESS_MAP_END


/* PCM Z80, 4 DACs (4 bits per sample), NO RAM !! */

static ADDRESS_MAP_START( brickzn_pcm_map, AS_PROGRAM, 8, suna8_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM // ROM
ADDRESS_MAP_END


WRITE8_MEMBER(suna8_state::brickzn_pcm_w)
{
	static const char *const dacs[] = { "dac1", "dac2", "dac3", "dac4" };
	machine().device<dac_device>(dacs[offset & 3])->write_signed8( (data & 0xf) * 0x11 );
}


static ADDRESS_MAP_START( brickzn_pcm_io_map, AS_IO, 8, suna8_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch2_byte_r     )   // From Sound CPU
	AM_RANGE(0x00, 0x03) AM_WRITE(brickzn_pcm_w     )   // 4 x DAC
ADDRESS_MAP_END

/***************************************************************************


                                Input Ports


***************************************************************************/

#define JOY(_n_) \
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(_n_) \
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_START##_n_ ) \
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_COIN##_n_  )

/***************************************************************************
                                Hard Head
***************************************************************************/

static INPUT_PORTS_START( hardhead )

	PORT_START("P1")    // Player 1 - $da00 (ip = 0)
	JOY(1)

	PORT_START("P2")    // Player 2 - $da00 (ip = 1)
	JOY(2)

	PORT_START("DSW1")  // DSW 1 - $da00 (ip = 2)
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0e, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(    0x0e, "No Bonus" )
	PORT_DIPSETTING(    0x0c, "10K" )
	PORT_DIPSETTING(    0x0a, "20K" )
	PORT_DIPSETTING(    0x08, "50K" )
	PORT_DIPSETTING(    0x06, "50K, Every 50K" )
	PORT_DIPSETTING(    0x04, "100K, Every 50K" )
	PORT_DIPSETTING(    0x02, "100K, Every 100K" )
	PORT_DIPSETTING(    0x00, "200K, Every 100K" )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:5,6,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") //DSW 2 - $da00 (ip = 3)
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Play Together" )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0xe0, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7,8")
	PORT_DIPSETTING(    0xe0, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0xa0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, "Moderate" )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

/***************************************************************************
                                Rough Ranger
***************************************************************************/

static INPUT_PORTS_START( rranger )

	PORT_START("P1")    // Player 1 - $c002
	JOY(1)

	PORT_START("P2") // Player 2 - $c003
	JOY(2)

	PORT_START("DSW1") //DSW 1 - $c280
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x20, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x30, "10K" )
	PORT_DIPSETTING(    0x28, "30K" )
	PORT_DIPSETTING(    0x20, "50K" )
	PORT_DIPSETTING(    0x18, "50K, Every 50K" )
	PORT_DIPSETTING(    0x10, "100K, Every 50K" )
	PORT_DIPSETTING(    0x08, "100K, Every 100K" )
	PORT_DIPSETTING(    0x00, "100K, Every 200K" )
	PORT_DIPSETTING(    0x38, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("DSW2") // DSW 2 - $c2c0
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Play Together" )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability (Cheat)")    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/***************************************************************************
                                Brick Zone
***************************************************************************/

static INPUT_PORTS_START( brickzn )

	PORT_START("P1") // Player 1 Joystick - $c100
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1)
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_CONFNAME( 0x04, 0x04, "Select First Stage" )
	PORT_CONFSETTING(    0x04, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFNAME( 0x08, 0x08, "Cheat 1" )  // ???
	PORT_CONFSETTING(    0x08, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(1)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(1)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_START1         )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_COIN1          )

	PORT_START("P2") // Player 2 Joystick - $c101
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_CONFNAME( 0x04, 0x04, "Select Next Stage" )
	PORT_CONFSETTING(    0x04, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFNAME( 0x08, 0x08, "Cheat 2" )  // ???
	PORT_CONFSETTING(    0x08, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_START2         )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_COIN2          )

	PORT_START("DSW1") // DSW 1 - $c102
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW-A:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW-A:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x28, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, "Moderate" )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x40, IP_ACTIVE_LOW, "SW-A:7" ) PORT_NAME( "Service / Invulnerability" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW-A:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") // DSW 2 - $c103
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW-B:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW-B:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Play Together" )     PORT_DIPLOCATION("SW-B:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW-B:4,5,6")
	PORT_DIPSETTING(    0x30, "10K" )
	PORT_DIPSETTING(    0x28, "30K" )
	PORT_DIPSETTING(    0x18, "50K, Every 50K" )
	PORT_DIPSETTING(    0x20, "50K" )
	PORT_DIPSETTING(    0x10, "100K, Every 50K" )
	PORT_DIPSETTING(    0x08, "100K, Every 100K" )
	PORT_DIPSETTING(    0x00, "200K, Every 100K" )
	PORT_DIPSETTING(    0x38, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW-B:7,8")
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("SPIN1") // Player 1 Spinner - $c108
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("SPIN2") // Player 2 Spinner - $c10c
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("CHEATS") // Cheats / Debugging Inputs - $c140
	PORT_CONFNAME( 0x01, 0x00, "0: Use Cheat 1&2 Setting" )
	PORT_CONFSETTING(    0x01, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFNAME( 0x02, 0x02, "1: Unused?" )
	PORT_CONFSETTING(    0x02, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(  0x04, IP_ACTIVE_HIGH, IPT_SPECIAL )  // Must flip rapidly
	PORT_CONFNAME( 0x08, 0x08, "3: Unused?" )
	PORT_CONFSETTING(    0x08, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFNAME( 0x10, 0x10, "4: Unused?" )
	PORT_CONFSETTING(    0x10, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFNAME( 0x20, 0x20, "5: Unused?" )
	PORT_CONFSETTING(    0x20, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_CONFNAME( 0x40, 0x40, "6: Unused?" )
	PORT_CONFSETTING(    0x40, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Finish Stage (Cheat)")

INPUT_PORTS_END

static INPUT_PORTS_START( brickznv6 )
	PORT_INCLUDE(brickzn)

	PORT_MODIFY("DSW2") // DSW 2 - $c103
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW-B:4,5")
	PORT_DIPSETTING(    0x18, "None" )
	PORT_DIPSETTING(    0x10, "10K" )
	PORT_DIPSETTING(    0x08, "30K" )
	PORT_DIPSETTING(    0x00, "50K" )
	PORT_DIPNAME( 0x20, 0x20, "Display" )   PORT_DIPLOCATION("SW-B:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
INPUT_PORTS_END

/***************************************************************************
                                Hard Head 2
***************************************************************************/

static INPUT_PORTS_START( hardhea2 )

	PORT_START("P1") // Player 1 - $c000
	JOY(1)

	PORT_START("P2") // Player 2 - $c001
	JOY(2)

	PORT_START("DSW1") // DSW 1 - $c002
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x28, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, "Moderate" )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") // DSW 2 - $c003
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Play Together" )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x30, "10K" )
	PORT_DIPSETTING(    0x28, "30K" )
	PORT_DIPSETTING(    0x18, "50K, Every 50K" )
	PORT_DIPSETTING(    0x20, "50K" )
	PORT_DIPSETTING(    0x10, "100K, Every 50K" )
	PORT_DIPSETTING(    0x08, "100K, Every 100K" )
	PORT_DIPSETTING(    0x00, "200K, Every 100K" )
	PORT_DIPSETTING(    0x38, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("BUTTONS") // Buttons - $c080
	PORT_BIT(  0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT(  0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN )

INPUT_PORTS_END


/***************************************************************************
                                Star Fighter
***************************************************************************/

static INPUT_PORTS_START( starfigh )

	PORT_START("P1") // Player 1 - $c000
	JOY(1)

	PORT_START("P2") // Player 2 - $c001
	JOY(2)

	PORT_START("DSW1") // DSW 1 - $c002
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x28, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, "Moderate" )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") // DSW 2 - $c003
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Play Together" )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x30, "10K" )
	PORT_DIPSETTING(    0x28, "30K" )
	PORT_DIPSETTING(    0x18, "50K, Every 50K" )
	PORT_DIPSETTING(    0x20, "50K" )
	PORT_DIPSETTING(    0x10, "100K, Every 50K" )
	PORT_DIPSETTING(    0x08, "100K, Every 100K" )
	PORT_DIPSETTING(    0x00, "200K, Every 100K" )
	PORT_DIPSETTING(    0x38, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("CHEATS") // ??? - $c080
	PORT_BIT(  0x01, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x02, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_CONFNAME( 0x08, 0x08, "3: Copyright Screen Color + ?" )    // also changes a table
	PORT_CONFSETTING(    0x08, "Green" )
	PORT_CONFSETTING(    0x00, "Blue" )
	PORT_BIT(  0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT(  0x40, IP_ACTIVE_HIGH, IPT_CUSTOM  ) PORT_VBLANK("screen")    // 0 = skip color cycling (red)
	PORT_BIT(  0x80, IP_ACTIVE_LOW,  IPT_SPECIAL )  // read in protection check, see code at 787

INPUT_PORTS_END


/***************************************************************************
                                Spark Man
***************************************************************************/

static INPUT_PORTS_START( sparkman )

	PORT_START("P1") // Player 1 - $c000
	JOY(1)

	PORT_START("P2") // Player 2 - $c001
	JOY(2)

	PORT_START("DSW1") // DSW 1 - $c002
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x38, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x28, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x38, "Moderate" )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC(  0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") // DSW 2 - $c003
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, "Play Together" )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x38, "10K" )
	PORT_DIPSETTING(    0x28, "30K" )
	PORT_DIPSETTING(    0x18, "50K, Every 50K" )
	PORT_DIPSETTING(    0x20, "50K" )
	PORT_DIPSETTING(    0x10, "100K, Every 50K" )
	PORT_DIPSETTING(    0x08, "100K, Every 100K" )
	PORT_DIPSETTING(    0x00, "200K, Every 100K" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("BUTTONS") // Buttons - $c080
	PORT_BIT(  0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)    // P1 bomb
	PORT_BIT(  0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)    // P2 bomb
	PORT_BIT(  0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )                   // ?
	PORT_BIT(  0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(  0x80, IP_ACTIVE_LOW, IPT_SPECIAL )                   // protection

INPUT_PORTS_END


/***************************************************************************


                                Graphics Layouts


***************************************************************************/

/* 8x8x4 tiles (2 bitplanes per ROM) */
static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2) + 0, RGN_FRAC(1,2) + 4, 0, 4 },
	{ 3,2,1,0, 11,10,9,8},
	{ STEP8(0,16) },
	8*8*2
};

static GFXDECODE_START( suna8 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4, 0, 16*2 ) // [0] Sprites (brickzn has 2 palette RAMs)
GFXDECODE_END

static GFXDECODE_START( suna8_x2 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_8x8x4, 0, 16*2 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_8x8x4, 0, 16*2 ) // [1] Sprites (sparkman has 2 sprite "chips")
GFXDECODE_END



/***************************************************************************


                                Machine Drivers


***************************************************************************/

/* In games with only 2 CPUs, port A&B of the AY8910 are used
   for sample playing. */

/***************************************************************************
                                Hard Head
***************************************************************************/

/* 1 x 24 MHz crystal */


static MACHINE_CONFIG_START( hardhead, suna8_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, SUNA8_MASTER_CLOCK / 4)    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(hardhead_map)
	MCFG_CPU_IO_MAP(hardhead_io_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", suna8_state,  irq0_line_hold)      /* No NMI */

	MCFG_CPU_ADD("audiocpu", Z80, SUNA8_MASTER_CLOCK / 8)   /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(hardhead_sound_map)
	MCFG_CPU_IO_MAP(hardhead_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(suna8_state, irq0_line_hold, 4*60)     /* No NMI */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.10)  /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(suna8_state, screen_update_suna8)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", suna8)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	MCFG_VIDEO_START_OVERRIDE(suna8_state,suna8_textdim12)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, SUNA8_MASTER_CLOCK / 8)     /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("aysnd", AY8910, SUNA8_MASTER_CLOCK / 16)    /* verified on pcb */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(suna8_state, suna8_play_samples_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(suna8_state, suna8_samples_number_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_START_CB(suna8_state, sh_start)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/***************************************************************************
                                Rough Ranger
***************************************************************************/

/* 1 x 24 MHz crystal */

/* 2203 + 8910 */
static MACHINE_CONFIG_START( rranger, suna8_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, SUNA8_MASTER_CLOCK / 4)                    /* ? */
	MCFG_CPU_PROGRAM_MAP(rranger_map)
	MCFG_CPU_IO_MAP(rranger_io_map)
//  MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", suna8_state,  irq0_line_hold)  /* IRQ & NMI ! */

	MCFG_CPU_ADD("audiocpu", Z80, SUNA8_MASTER_CLOCK / 8)   /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(rranger_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(suna8_state, irq0_line_hold, 4*60) /* NMI = retn */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.1)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(suna8_state, screen_update_suna8)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", suna8)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	MCFG_VIDEO_START_OVERRIDE(suna8_state,suna8_textdim8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, SUNA8_MASTER_CLOCK / 16)  /* verified on pcb */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(suna8_state, rranger_play_samples_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(suna8_state, suna8_samples_number_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)

	MCFG_SOUND_ADD("ym2", YM2203, SUNA8_MASTER_CLOCK / 16)  /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.90)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_START_CB(suna8_state, sh_start)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/***************************************************************************
                                Brick Zone
***************************************************************************/

/* 1 x 24 MHz crystal */

MACHINE_RESET_MEMBER(suna8_state,brickzn)
{
	m_protection_val = m_prot2 = m_prot2_prev = 0xff;
	m_paletteram_enab = 1;  // for brickzn11
	m_remap_sound = 0;
	m_bank1->set_entry(0);
	if(m_bank1d)
		m_bank1d->set_entry(0);
}

static MACHINE_CONFIG_START( brickzn11, suna8_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, SUNA8_MASTER_CLOCK / 4)        /* SUNA PROTECTION BLOCK */
	MCFG_CPU_PROGRAM_MAP(brickzn11_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", suna8_state,  irq0_line_hold)  // nmi breaks ramtest but is needed!

	MCFG_CPU_ADD("audiocpu", Z80, SUNA8_MASTER_CLOCK / 4)   /* Z0840006PSC - 6MHz (measured) */
	MCFG_CPU_PROGRAM_MAP(brickzn_sound_map)

	MCFG_CPU_ADD("pcm", Z80, SUNA8_MASTER_CLOCK / 4)    /* Z0840006PSC - 6MHz (measured) */
	MCFG_CPU_PROGRAM_MAP(brickzn_pcm_map)
	MCFG_CPU_IO_MAP(brickzn_pcm_io_map)

	MCFG_MACHINE_RESET_OVERRIDE(suna8_state, brickzn )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)   // we're using PORT_VBLANK
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(suna8_state, screen_update_suna8)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", suna8)
	MCFG_PALETTE_ADD("palette", 256 * 2)    // 2 x Palette RAM
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	MCFG_VIDEO_START_OVERRIDE(suna8_state,suna8_brickzn)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, SUNA8_MASTER_CLOCK / 8)     // 3MHz (measured)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("aysnd", AY8910, SUNA8_MASTER_CLOCK / 16)    // 1.5MHz (measured)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.17)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.17)

	MCFG_DAC_ADD("dac3")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.17)

	MCFG_DAC_ADD("dac4")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.17)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( brickzn, brickzn11 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(brickzn_map)
	MCFG_CPU_IO_MAP(brickzn_io_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END


/***************************************************************************
                                Hard Head 2
***************************************************************************/

/* 1 x 24 MHz crystal */

TIMER_DEVICE_CALLBACK_MEMBER(suna8_state::hardhea2_interrupt)
{
	int scanline = param;

	if(scanline == 240)
		m_maincpu->set_input_line(0, HOLD_LINE);
	if(scanline == 112)
		if (m_nmi_enable)   m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

MACHINE_RESET_MEMBER(suna8_state,hardhea2)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	hardhea2_rambank_0_w(space,0,0);
}

static MACHINE_CONFIG_DERIVED( hardhea2, brickzn )
	MCFG_DEVICE_REMOVE("maincpu")

	MCFG_CPU_ADD("maincpu", Z80, SUNA8_MASTER_CLOCK / 4)        /* SUNA T568009 */
	MCFG_CPU_PROGRAM_MAP(hardhea2_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", suna8_state, hardhea2_interrupt, "screen", 0, 1)

	MCFG_MACHINE_RESET_OVERRIDE(suna8_state,hardhea2)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(256)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)
MACHINE_CONFIG_END


/***************************************************************************
                                Star Fighter
***************************************************************************/

static MACHINE_CONFIG_START( starfigh, suna8_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, SUNA8_MASTER_CLOCK / 4)                    /* ? */
	MCFG_CPU_PROGRAM_MAP(starfigh_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", suna8_state, hardhea2_interrupt, "screen", 0, 1)

	/* The sound section is identical to that of hardhead */
	MCFG_CPU_ADD("audiocpu", Z80, SUNA8_MASTER_CLOCK / 4)                   /* ? */
	MCFG_CPU_PROGRAM_MAP(hardhead_sound_map)
	MCFG_CPU_IO_MAP(hardhead_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(suna8_state, irq0_line_hold, 4*60) /* No NMI */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(suna8_state, screen_update_suna8)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", suna8)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	MCFG_VIDEO_START_OVERRIDE(suna8_state,suna8_starfigh)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, SUNA8_MASTER_CLOCK / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("aysnd", AY8910, SUNA8_MASTER_CLOCK / 16)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(suna8_state, suna8_play_samples_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(suna8_state, suna8_samples_number_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_START_CB(suna8_state, sh_start)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/***************************************************************************
                                Spark Man
***************************************************************************/

static MACHINE_CONFIG_START( sparkman, suna8_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, SUNA8_MASTER_CLOCK / 4)                    /* ? */
	MCFG_CPU_PROGRAM_MAP(sparkman_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", suna8_state, hardhea2_interrupt, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, SUNA8_MASTER_CLOCK / 4)               /* ? */
	MCFG_CPU_PROGRAM_MAP(hardhead_sound_map)
	MCFG_CPU_IO_MAP(hardhead_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(suna8_state, irq0_line_hold, 4*60) /* No NMI */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0+16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(suna8_state, screen_update_suna8)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", suna8_x2)    // 2 sprite "chips"
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	MCFG_VIDEO_START_OVERRIDE(suna8_state,suna8_sparkman)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, SUNA8_MASTER_CLOCK / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("aysnd", AY8910, SUNA8_MASTER_CLOCK / 16)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(suna8_state, suna8_play_samples_w))  // two sample roms
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(suna8_state, suna8_samples_number_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_START_CB(suna8_state, sh_start)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/***************************************************************************


                                ROMs Loading


***************************************************************************/

/***************************************************************************

                                    Hard Head

Location  Type    File ID  Checksum
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
L5       27C256     P1       1327   [ main program ]
K5       27C256     P2       50B1   [ main program ]
J5       27C256     P3       CF73   [ main program ]
I5       27C256     P4       DE86   [ main program ]
D5       27C256     P5       94D1   [  background  ]
A5       27C256     P6       C3C7   [ motion obj.  ]
L7       27C256     P7       A7B8   [ main program ]
K7       27C256     P8       5E53   [ main program ]
J7       27C256     P9       35FC   [ main program ]
I7       27C256     P10      8F9A   [ main program ]
D7       27C256     P11      931C   [  background  ]
A7       27C256     P12      2EED   [ motion obj.  ]
H9       27C256     P13      5CD2   [ snd program  ]
M9       27C256     P14      5576   [  sound data  ]

Note:  Game   No. KRB-14
       PCB    No. 60138-0083

Main processor  -  Custom security block (battery backed) CPU No. S562008

Sound processor -  Z80
                -  YM3812
                -  AY-3-8910

24 MHz crystal

***************************************************************************/

// The sample rom is from srange with 1 byte changed (first byte is FF here, instead of 77)
// (laugh sound used when scoring a goal at the end of level 1)

ROM_START( hardhead )
	ROM_REGION( 0x48000, "maincpu", 0 ) /* Main Z80 Code */
	ROM_LOAD( "p1",  0x00000, 0x8000, CRC(c6147926) SHA1(8d1609aaeac344c6aec102e92d34caab22a8ec64) )    // 1988,9,14
	ROM_LOAD( "p2",  0x10000, 0x8000, CRC(faa2cf9a) SHA1(5987f146b58fcbc3aaa9c010d86022b5172bcfb4) )
	ROM_LOAD( "p3",  0x18000, 0x8000, CRC(3d24755e) SHA1(519a179594956f7c3ddfaca362c42b453c928e25) )
	ROM_LOAD( "p4",  0x20000, 0x8000, CRC(0241ac79) SHA1(b3c3b98fb29836cbc9fd35ac49e02bfefd3b0c79) )
	ROM_LOAD( "p7",  0x28000, 0x8000, CRC(beba8313) SHA1(20aa4e07ec560a89d07ec73cc93311ceaed899a3) )
	ROM_LOAD( "p8",  0x30000, 0x8000, CRC(211a9342) SHA1(85bdafe1a2c683eea391cc663caabd958fdf5197) )
	ROM_LOAD( "p9",  0x38000, 0x8000, CRC(2ad430c4) SHA1(286a5b1042e077c3ae741d01311d4c91f8f87054) )
	ROM_LOAD( "p10", 0x40000, 0x8000, CRC(b6894517) SHA1(e114a5f92b83d98215aab6e2cd943a110d118f56) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "p13", 0x0000, 0x8000, CRC(493c0b41) SHA1(994a334253e905c39ec912765e8b0f4b1be900bc) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "p5",  0x00000, 0x8000, CRC(e9aa6fba) SHA1(f286727541f08b136a7d45e13975652bdc8fd663) )
	ROM_RELOAD(      0x08000, 0x8000             )
	ROM_LOAD( "p6",  0x10000, 0x8000, CRC(15d5f5dd) SHA1(4441344701fcdb2be55bdd76a8a5fd59f5de813c) )
	ROM_RELOAD(      0x18000, 0x8000             )
	ROM_LOAD( "p11", 0x20000, 0x8000, CRC(055f4c29) SHA1(0eee5db50504a3d37d9291ccd29863ba71da85e1) )
	ROM_RELOAD(      0x28000, 0x8000             )
	ROM_LOAD( "p12", 0x30000, 0x8000, CRC(9582e6db) SHA1(a2b34d740e07bd35a3184365e7f3ab7476075d70) )
	ROM_RELOAD(      0x38000, 0x8000             )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "p14", 0x0000, 0x8000, CRC(41314ac1) SHA1(1ac9213b0ac4ce9fe6256e93875672e128a5d069) )
ROM_END

ROM_START( hardheadb )
	ROM_REGION( 0x48000+0x8000, "maincpu", 0 ) /* Main Z80 Code */
	ROM_LOAD( "1_27512.l6",  0x48000, 0x8000, CRC(bb4aa9ac) SHA1(da6310a1034cf610139d74fc30dd13e5fbd1d8dd) ) // 1988,9,14 (already decrypted)
	ROM_CONTINUE(            0x00000, 0x8000 )
	ROM_LOAD( "p2",          0x10000, 0x8000, CRC(faa2cf9a) SHA1(5987f146b58fcbc3aaa9c010d86022b5172bcfb4) )
	ROM_LOAD( "p3",          0x18000, 0x8000, CRC(3d24755e) SHA1(519a179594956f7c3ddfaca362c42b453c928e25) )
	ROM_LOAD( "p4",          0x20000, 0x8000, CRC(0241ac79) SHA1(b3c3b98fb29836cbc9fd35ac49e02bfefd3b0c79) )
	ROM_LOAD( "p7",          0x28000, 0x8000, CRC(beba8313) SHA1(20aa4e07ec560a89d07ec73cc93311ceaed899a3) )
	ROM_LOAD( "p8",          0x30000, 0x8000, CRC(211a9342) SHA1(85bdafe1a2c683eea391cc663caabd958fdf5197) )
	ROM_LOAD( "p9",          0x38000, 0x8000, CRC(2ad430c4) SHA1(286a5b1042e077c3ae741d01311d4c91f8f87054) )
	ROM_LOAD( "p10",         0x40000, 0x8000, CRC(b6894517) SHA1(e114a5f92b83d98215aab6e2cd943a110d118f56) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "p13", 0x0000, 0x8000, CRC(493c0b41) SHA1(994a334253e905c39ec912765e8b0f4b1be900bc) )
//  ROM_LOAD( "2_13_9h.rom", 0x00000, 0x8000, CRC(1b20e5ec) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "p5",  0x00000, 0x8000, CRC(e9aa6fba) SHA1(f286727541f08b136a7d45e13975652bdc8fd663) )
	ROM_RELOAD(      0x08000, 0x8000             )
	ROM_LOAD( "p6",  0x10000, 0x8000, CRC(15d5f5dd) SHA1(4441344701fcdb2be55bdd76a8a5fd59f5de813c) )
	ROM_RELOAD(      0x18000, 0x8000             )
	ROM_LOAD( "p11", 0x20000, 0x8000, CRC(055f4c29) SHA1(0eee5db50504a3d37d9291ccd29863ba71da85e1) )
	ROM_RELOAD(      0x28000, 0x8000             )
	ROM_LOAD( "p12", 0x30000, 0x8000, CRC(9582e6db) SHA1(a2b34d740e07bd35a3184365e7f3ab7476075d70) )
	ROM_RELOAD(      0x38000, 0x8000             )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "p14", 0x0000, 0x8000, CRC(41314ac1) SHA1(1ac9213b0ac4ce9fe6256e93875672e128a5d069) )
ROM_END

ROM_START( pop_hh )
	ROM_REGION( 0x48000+0x8000, "maincpu", 0 ) /* Main Z80 Code */
	ROM_LOAD( "1_27512.l6",  0x48000, 0x8000, CRC(bb4aa9ac) SHA1(da6310a1034cf610139d74fc30dd13e5fbd1d8dd) ) // 1988,9,14 (already decrypted)
	ROM_CONTINUE(            0x00000, 0x8000 )
	ROM_LOAD( "2_27256.k6",  0x10000, 0x8000, CRC(8fcc1248) SHA1(5da0b7dc63f7bc00e81e9e5bac02ee6b0076ffaa) )
	ROM_LOAD( "p3",          0x18000, 0x8000, CRC(3d24755e) SHA1(519a179594956f7c3ddfaca362c42b453c928e25) ) // 3_27256.j6
	ROM_LOAD( "p4",          0x20000, 0x8000, CRC(0241ac79) SHA1(b3c3b98fb29836cbc9fd35ac49e02bfefd3b0c79) ) // 4_27256.i6
	ROM_LOAD( "p7",          0x28000, 0x8000, CRC(beba8313) SHA1(20aa4e07ec560a89d07ec73cc93311ceaed899a3) ) // 7_27256.l8
	ROM_LOAD( "8_27256.k8",  0x30000, 0x8000, CRC(87a8b4b4) SHA1(83d30cf184c5dccdf2666c0ef9e078541d6a146e) )
	ROM_LOAD( "p9",          0x38000, 0x8000, CRC(2ad430c4) SHA1(286a5b1042e077c3ae741d01311d4c91f8f87054) ) // 9_27256.j8
	ROM_LOAD( "10_27256.i8", 0x40000, 0x8000, CRC(84fc6574) SHA1(ab33e6c656f25e65bb08d0a2689693df83cab43d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "p13", 0x0000, 0x8000, CRC(493c0b41) SHA1(994a334253e905c39ec912765e8b0f4b1be900bc) ) // 13_27256.i10

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "p5",  0x00000, 0x8000, CRC(e9aa6fba) SHA1(f286727541f08b136a7d45e13975652bdc8fd663) ) // 5_27256.d6
	ROM_RELOAD(      0x08000, 0x8000             )
	ROM_LOAD( "p6",  0x10000, 0x8000, CRC(15d5f5dd) SHA1(4441344701fcdb2be55bdd76a8a5fd59f5de813c) ) // 6_27256.a6
	ROM_RELOAD(      0x18000, 0x8000             )
	ROM_LOAD( "11_27256.d8", 0x20000, 0x8000, CRC(3751b99d) SHA1(dc4082e481a79f0389e59b4b38698df8f7b94053) )
	ROM_RELOAD(      0x28000, 0x8000             )
	ROM_LOAD( "p12", 0x30000, 0x8000, CRC(9582e6db) SHA1(a2b34d740e07bd35a3184365e7f3ab7476075d70) ) // 12_27256.a8
	ROM_RELOAD(      0x38000, 0x8000             )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "p14", 0x0000, 0x8000, CRC(41314ac1) SHA1(1ac9213b0ac4ce9fe6256e93875672e128a5d069) ) // 14_27256.m11
ROM_END


/***************************************************************************

                            Rough Ranger / Super Ranger

(SunA 1988)
K030087

 24MHz    6  7  8  9  - 10 11 12 13   sw1  sw2



   6264
   5    6116
   4    6116                         6116
   3    6116                         14
   2    6116                         Z80A
   1                        6116     8910
                 6116  6116          2203
                                     15
 Epoxy CPU
                            6116


---------------------------
Super Ranger by SUNA (1988)
---------------------------

Location   Type    File ID  Checksum
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
E2        27C256      1      28C0    [ main program ]
F2        27C256      2      73AD    [ main program ]
H2        27C256      3      8B7A    [ main program ]
I2        27C512      4      77BE    [ main program ]
J2        27C512      5      6121    [ main program ]
P5        27C256      6      BE0E    [  background  ]
P6        27C256      7      BD5A    [  background  ]
P7        27C256      8      4605    [ motion obj.  ]
P8        27C256      9      7097    [ motion obj.  ]
P9        27C256      10     3B9F    [  background  ]
P10       27C256      11     2AE8    [  background  ]
P11       27C256      12     8B6D    [ motion obj.  ]
P12       27C256      13     927E    [ motion obj.  ]
J13       27C256      14     E817    [ snd program  ]
E13       27C256      15     54EE    [ sound data   ]

Note:  Game model number K030087

Hardware:

Main processor  -  Custom security block (battery backed)  CPU No. S562008

Sound processor - Z80
                - YM2203C
                - AY-3-8910

All versions come with roms simply numbered 1 through 15 even if the data
  is different per version.

***************************************************************************/

ROM_START( rranger )
	// Sharp Image License but distributed by CAPCOM U.S.A. Inc.  PCB came with ROM 1 labeled as 01 CAPCOM
	// PCB have been see with ROM 1 simply labled as 1 in either RED and BLUE print.
	ROM_REGION( 0x48000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "01_capcom.e2", 0x00000, 0x8000, CRC(ff1868cf) SHA1(54175111d8e39894ff11a779057f0bc061d63912) ) // V 2.0 1988,4,15
	ROM_LOAD( "2.f2", 0x10000, 0x8000, CRC(ff65af29) SHA1(90f9a0c862e2a9da0343446a325961ab29d26b4b) )
	ROM_LOAD( "3.h2", 0x18000, 0x8000, CRC(64e09436) SHA1(077f0d38d489562532d5f7678434a85ca04d373c) )
	ROM_LOAD( "4.i2", 0x30000, 0x8000, CRC(4346fae6) SHA1(a9f000e4427a1e9902627402dce14dc8ee04dbf8) )
	ROM_CONTINUE(     0x20000, 0x8000 )
	ROM_LOAD( "5.j2", 0x38000, 0x8000, CRC(6a7ca1c3) SHA1(0f0b508e9b20909e9efa07b42d67732082b6940b) )
	ROM_CONTINUE(     0x28000, 0x8000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "14.j13", 0x0000, 0x8000, CRC(11c83aa1) SHA1(d1f75096528b220a3f858eac62e3b4111fa013de) )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "15.e13", 0x0000, 0x8000, CRC(28c2c87e) SHA1(ec0d92140ef44df822f2229e79b915e051caa033) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "6.p5",   0x00000, 0x8000, CRC(57543643) SHA1(59c7717321314678e61b50767e168eb2a73147d3) ) // sldh - Sharp Image license
	ROM_LOAD( "7.p6",   0x08000, 0x8000, CRC(9f35dbfa) SHA1(8a8f158ad7f0bc6b43eaa95959af3ab58cf14d6d) )
	ROM_LOAD( "8.p7",   0x10000, 0x8000, CRC(f400db89) SHA1(a07b226af40cac5a20739bb8f4226909724fda86) )
	ROM_LOAD( "9.p8",   0x18000, 0x8000, CRC(fa2a11ea) SHA1(ea29ade1254caa2a3bd4b4816fe338f238025284) )
	ROM_LOAD( "10.p9",  0x20000, 0x8000, CRC(42c4fdbf) SHA1(fd8b267d5098b640e731942b922149866ece1dc6) ) // sldh - Sharp Image license
	ROM_LOAD( "11.p10", 0x28000, 0x8000, CRC(19037a7b) SHA1(a6843b0220bab5c47307a0c761d5bd638716aef0) )
	ROM_LOAD( "12.p11", 0x30000, 0x8000, CRC(c59c0ec7) SHA1(80003f3e33610a84f6e194918276d5f60145b00e) )
	ROM_LOAD( "13.p12", 0x38000, 0x8000, CRC(9809fee8) SHA1(b7e0664702d0c1f77247d7c76a381b24687a09ea) )
ROM_END

ROM_START( rrangerb ) // protection is patched out in this set.
	ROM_REGION( 0x48000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "1.e2", 0x00000, 0x8000, CRC(4fb4f096) SHA1(c5ac3e04080cdcf570769918587e8cf8d455fc30) ) // sldh - V 2.0 1988,4,15
	ROM_LOAD( "2.f2", 0x10000, 0x8000, CRC(ff65af29) SHA1(90f9a0c862e2a9da0343446a325961ab29d26b4b) )
	ROM_LOAD( "3.h2", 0x18000, 0x8000, CRC(64e09436) SHA1(077f0d38d489562532d5f7678434a85ca04d373c) )
	ROM_LOAD( "4.i2", 0x30000, 0x8000, CRC(4346fae6) SHA1(a9f000e4427a1e9902627402dce14dc8ee04dbf8) )
	ROM_CONTINUE(     0x20000, 0x8000 )
	ROM_LOAD( "5.j2", 0x38000, 0x8000, CRC(6a7ca1c3) SHA1(0f0b508e9b20909e9efa07b42d67732082b6940b) )
	ROM_CONTINUE(     0x28000, 0x8000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "14.j13", 0x0000, 0x8000, CRC(11c83aa1) SHA1(d1f75096528b220a3f858eac62e3b4111fa013de) )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "15.e13", 0x0000, 0x8000, CRC(28c2c87e) SHA1(ec0d92140ef44df822f2229e79b915e051caa033) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "6.p5",   0x00000, 0x8000, CRC(57543643) SHA1(59c7717321314678e61b50767e168eb2a73147d3) ) // sldh - Sharp Image license
	ROM_LOAD( "7.p6",   0x08000, 0x8000, CRC(9f35dbfa) SHA1(8a8f158ad7f0bc6b43eaa95959af3ab58cf14d6d) )
	ROM_LOAD( "8.p7",   0x10000, 0x8000, CRC(f400db89) SHA1(a07b226af40cac5a20739bb8f4226909724fda86) )
	ROM_LOAD( "9.p8",   0x18000, 0x8000, CRC(fa2a11ea) SHA1(ea29ade1254caa2a3bd4b4816fe338f238025284) )
	ROM_LOAD( "10.p9",  0x20000, 0x8000, CRC(42c4fdbf) SHA1(fd8b267d5098b640e731942b922149866ece1dc6) ) // sldh - Sharp Image license
	ROM_LOAD( "11.p10", 0x28000, 0x8000, CRC(19037a7b) SHA1(a6843b0220bab5c47307a0c761d5bd638716aef0) )
	ROM_LOAD( "12.p11", 0x30000, 0x8000, CRC(c59c0ec7) SHA1(80003f3e33610a84f6e194918276d5f60145b00e) )
	ROM_LOAD( "13.p12", 0x38000, 0x8000, CRC(9809fee8) SHA1(b7e0664702d0c1f77247d7c76a381b24687a09ea) )
ROM_END

ROM_START( sranger )
	ROM_REGION( 0x48000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "1.e2", 0x00000, 0x8000, CRC(4eef1ede) SHA1(713074400e27f6983f97ce73e522a1d687961317) ) // sldh - V 2.0 1988,4,15
	ROM_LOAD( "2.f2", 0x10000, 0x8000, CRC(ff65af29) SHA1(90f9a0c862e2a9da0343446a325961ab29d26b4b) )
	ROM_LOAD( "3.h2", 0x18000, 0x8000, CRC(64e09436) SHA1(077f0d38d489562532d5f7678434a85ca04d373c) )
	ROM_LOAD( "4.i2", 0x30000, 0x8000, CRC(4346fae6) SHA1(a9f000e4427a1e9902627402dce14dc8ee04dbf8) )
	ROM_CONTINUE(     0x20000, 0x8000 )
	ROM_LOAD( "5.j2", 0x38000, 0x8000, CRC(6a7ca1c3) SHA1(0f0b508e9b20909e9efa07b42d67732082b6940b) )
	ROM_CONTINUE(     0x28000, 0x8000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "14.j13", 0x0000, 0x8000, CRC(11c83aa1) SHA1(d1f75096528b220a3f858eac62e3b4111fa013de) )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "15.e13", 0x0000, 0x8000, CRC(28c2c87e) SHA1(ec0d92140ef44df822f2229e79b915e051caa033) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "6.p5",   0x00000, 0x8000, CRC(4f11fef3) SHA1(f48f3051a5ab681da0fd0a7107ea0c833993247a) ) // sldh - Newer / updated graphics ??
	ROM_LOAD( "7.p6",   0x08000, 0x8000, CRC(9f35dbfa) SHA1(8a8f158ad7f0bc6b43eaa95959af3ab58cf14d6d) )
	ROM_LOAD( "8.p7",   0x10000, 0x8000, CRC(f400db89) SHA1(a07b226af40cac5a20739bb8f4226909724fda86) )
	ROM_LOAD( "9.p8",   0x18000, 0x8000, CRC(fa2a11ea) SHA1(ea29ade1254caa2a3bd4b4816fe338f238025284) )
	ROM_LOAD( "10.p9",  0x20000, 0x8000, CRC(1b204d6b) SHA1(8649d552dff08bb01ac7ca6cb873124e05646041) ) // sldh - Newer / updated graphics ??
	ROM_LOAD( "11.p10", 0x28000, 0x8000, CRC(19037a7b) SHA1(a6843b0220bab5c47307a0c761d5bd638716aef0) )
	ROM_LOAD( "12.p11", 0x30000, 0x8000, CRC(c59c0ec7) SHA1(80003f3e33610a84f6e194918276d5f60145b00e) )
	ROM_LOAD( "13.p12", 0x38000, 0x8000, CRC(9809fee8) SHA1(b7e0664702d0c1f77247d7c76a381b24687a09ea) )
ROM_END

ROM_START( srangerb )
	ROM_REGION( 0x48000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "r1bt", 0x00000, 0x8000, CRC(40635e7c) SHA1(741290ad640e941774d496a329cd29198ab83463) )   // NYWACORPORATION LTD 88-1-07
	ROM_LOAD( "2.f2", 0x10000, 0x8000, CRC(ff65af29) SHA1(90f9a0c862e2a9da0343446a325961ab29d26b4b) )
	ROM_LOAD( "3.h2", 0x18000, 0x8000, CRC(64e09436) SHA1(077f0d38d489562532d5f7678434a85ca04d373c) )
	ROM_LOAD( "4.i2", 0x30000, 0x8000, CRC(4346fae6) SHA1(a9f000e4427a1e9902627402dce14dc8ee04dbf8) )
	ROM_CONTINUE(     0x20000, 0x8000 )
	ROM_LOAD( "5.j2", 0x38000, 0x8000, CRC(6a7ca1c3) SHA1(0f0b508e9b20909e9efa07b42d67732082b6940b) )
	ROM_CONTINUE(     0x28000, 0x8000 )
	ROM_LOAD( "r5bt", 0x28000, 0x8000, BAD_DUMP CRC(f7f391b5) SHA1(a0a8de1d9d7876f5c4b26e34d5e54ec79529c2da) )  // wrong length

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "14.j13", 0x0000, 0x8000, CRC(11c83aa1) SHA1(d1f75096528b220a3f858eac62e3b4111fa013de) )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "15.e13", 0x0000, 0x8000, CRC(28c2c87e) SHA1(ec0d92140ef44df822f2229e79b915e051caa033) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "6.p5",   0x00000, 0x8000, CRC(4f11fef3) SHA1(f48f3051a5ab681da0fd0a7107ea0c833993247a) ) // sldh - Newer / updated graphics ??
	ROM_LOAD( "7.p6",   0x08000, 0x8000, CRC(9f35dbfa) SHA1(8a8f158ad7f0bc6b43eaa95959af3ab58cf14d6d) )
	ROM_LOAD( "8.p7",   0x10000, 0x8000, CRC(f400db89) SHA1(a07b226af40cac5a20739bb8f4226909724fda86) )
	ROM_LOAD( "9.p8",   0x18000, 0x8000, CRC(fa2a11ea) SHA1(ea29ade1254caa2a3bd4b4816fe338f238025284) )
	ROM_LOAD( "10.p9",  0x20000, 0x8000, CRC(1b204d6b) SHA1(8649d552dff08bb01ac7ca6cb873124e05646041) ) // sldh - Newer / updated graphics ??
	ROM_LOAD( "11.p10", 0x28000, 0x8000, CRC(19037a7b) SHA1(a6843b0220bab5c47307a0c761d5bd638716aef0) )
	ROM_LOAD( "12.p11", 0x30000, 0x8000, CRC(c59c0ec7) SHA1(80003f3e33610a84f6e194918276d5f60145b00e) )
	ROM_LOAD( "13.p12", 0x38000, 0x8000, CRC(9809fee8) SHA1(b7e0664702d0c1f77247d7c76a381b24687a09ea) )
ROM_END

ROM_START( srangero )
	ROM_REGION( 0x48000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "1.e2", 0x00000, 0x8000, CRC(2287d3fc) SHA1(cc2dab587ca50fc4371d2861ac842cd81370f868) ) // 88,2,28 RANGER
	ROM_LOAD( "2.f2", 0x10000, 0x8000, CRC(ff65af29) SHA1(90f9a0c862e2a9da0343446a325961ab29d26b4b) )
	ROM_LOAD( "3.h2", 0x18000, 0x8000, CRC(64e09436) SHA1(077f0d38d489562532d5f7678434a85ca04d373c) )
	ROM_LOAD( "4.i2", 0x30000, 0x8000, CRC(4346fae6) SHA1(a9f000e4427a1e9902627402dce14dc8ee04dbf8) )
	ROM_CONTINUE(     0x20000, 0x8000 )
	ROM_LOAD( "5.j2", 0x38000, 0x8000, CRC(6a7ca1c3) SHA1(0f0b508e9b20909e9efa07b42d67732082b6940b) )
	ROM_CONTINUE(     0x28000, 0x8000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "14.j13", 0x0000, 0x8000, CRC(11c83aa1) SHA1(d1f75096528b220a3f858eac62e3b4111fa013de) )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "15.e13", 0x0000, 0x8000, CRC(28c2c87e) SHA1(ec0d92140ef44df822f2229e79b915e051caa033) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "6.p5",   0x00000, 0x8000, CRC(ffe13cc4) SHA1(811a749fcb4f9a575374af593b79d3927f74d301) )
	ROM_LOAD( "7.p6",   0x08000, 0x8000, CRC(9f35dbfa) SHA1(8a8f158ad7f0bc6b43eaa95959af3ab58cf14d6d) )
	ROM_LOAD( "8.p7",   0x10000, 0x8000, CRC(f400db89) SHA1(a07b226af40cac5a20739bb8f4226909724fda86) )
	ROM_LOAD( "9.p8",   0x18000, 0x8000, CRC(fa2a11ea) SHA1(ea29ade1254caa2a3bd4b4816fe338f238025284) )
	ROM_LOAD( "10.p9",  0x20000, 0x8000, CRC(13f1faab) SHA1(901df4bd1d42483679edae5e1a0b2b8b3a429d05) )
	ROM_LOAD( "11.p10", 0x28000, 0x8000, CRC(19037a7b) SHA1(a6843b0220bab5c47307a0c761d5bd638716aef0) )
	ROM_LOAD( "12.p11", 0x30000, 0x8000, CRC(c59c0ec7) SHA1(80003f3e33610a84f6e194918276d5f60145b00e) )
	ROM_LOAD( "13.p12", 0x38000, 0x8000, CRC(9809fee8) SHA1(b7e0664702d0c1f77247d7c76a381b24687a09ea) )
ROM_END

ROM_START( srangerw )   // same program as srangero, 2 different gfx roms
	ROM_REGION( 0x48000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "1.e2", 0x00000, 0x8000, CRC(2287d3fc) SHA1(cc2dab587ca50fc4371d2861ac842cd81370f868) ) // 88,2,28 RANGER
	ROM_LOAD( "2.f2", 0x10000, 0x8000, CRC(ff65af29) SHA1(90f9a0c862e2a9da0343446a325961ab29d26b4b) )
	ROM_LOAD( "3.h2", 0x18000, 0x8000, CRC(64e09436) SHA1(077f0d38d489562532d5f7678434a85ca04d373c) )
	ROM_LOAD( "4.i2", 0x30000, 0x8000, CRC(4346fae6) SHA1(a9f000e4427a1e9902627402dce14dc8ee04dbf8) )
	ROM_CONTINUE(     0x20000, 0x8000 )
	ROM_LOAD( "5.j2", 0x38000, 0x8000, CRC(6a7ca1c3) SHA1(0f0b508e9b20909e9efa07b42d67732082b6940b) )
	ROM_CONTINUE(     0x28000, 0x8000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "14.j13", 0x0000, 0x8000, CRC(11c83aa1) SHA1(d1f75096528b220a3f858eac62e3b4111fa013de) )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "15.e13", 0x0000, 0x8000, CRC(28c2c87e) SHA1(ec0d92140ef44df822f2229e79b915e051caa033) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "6.p5",   0x00000, 0x8000, CRC(312ecda6) SHA1(db11259e10da5f7f2b7b306482a08835597dbde4) ) // sldh - WDK license
	ROM_LOAD( "7.p6",   0x08000, 0x8000, CRC(9f35dbfa) SHA1(8a8f158ad7f0bc6b43eaa95959af3ab58cf14d6d) )
	ROM_LOAD( "8.p7",   0x10000, 0x8000, CRC(f400db89) SHA1(a07b226af40cac5a20739bb8f4226909724fda86) )
	ROM_LOAD( "9.p8",   0x18000, 0x8000, CRC(fa2a11ea) SHA1(ea29ade1254caa2a3bd4b4816fe338f238025284) )
	ROM_LOAD( "10.p9",  0x20000, 0x8000, CRC(8731abc6) SHA1(05c13b359106b4ead1326f2e53d0585a2f0019ac) ) // sldh - WDK license
	ROM_LOAD( "11.p10", 0x28000, 0x8000, CRC(19037a7b) SHA1(a6843b0220bab5c47307a0c761d5bd638716aef0) )
	ROM_LOAD( "12.p11", 0x30000, 0x8000, CRC(c59c0ec7) SHA1(80003f3e33610a84f6e194918276d5f60145b00e) )
	ROM_LOAD( "13.p12", 0x38000, 0x8000, CRC(9809fee8) SHA1(b7e0664702d0c1f77247d7c76a381b24687a09ea) )
ROM_END

ROM_START( srangern )   // same program as srangero, 2 different gfx roms
	ROM_REGION( 0x48000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "1.e2", 0x00000, 0x8000, CRC(2287d3fc) SHA1(cc2dab587ca50fc4371d2861ac842cd81370f868) ) // 88,2,28 RANGER
	ROM_LOAD( "2.f2", 0x10000, 0x8000, CRC(ff65af29) SHA1(90f9a0c862e2a9da0343446a325961ab29d26b4b) )
	ROM_LOAD( "3.h2", 0x18000, 0x8000, CRC(64e09436) SHA1(077f0d38d489562532d5f7678434a85ca04d373c) )
	ROM_LOAD( "4.i2", 0x30000, 0x8000, CRC(4346fae6) SHA1(a9f000e4427a1e9902627402dce14dc8ee04dbf8) )
	ROM_CONTINUE(     0x20000, 0x8000 )
	ROM_LOAD( "5.j2", 0x38000, 0x8000, CRC(6a7ca1c3) SHA1(0f0b508e9b20909e9efa07b42d67732082b6940b) )
	ROM_CONTINUE(     0x28000, 0x8000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound Z80 Code */
	ROM_LOAD( "14.j13", 0x0000, 0x8000, CRC(11c83aa1) SHA1(d1f75096528b220a3f858eac62e3b4111fa013de) )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "15.e13", 0x0000, 0x8000, CRC(28c2c87e) SHA1(ec0d92140ef44df822f2229e79b915e051caa033) )

	ROM_REGION( 0x40000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "6.p5",   0x00000, 0x8000, CRC(af534075) SHA1(ce6e927702666d2588d6cdb3991463065a4e8084) ) // sldh - NOVA license
	ROM_LOAD( "7.p6",   0x08000, 0x8000, CRC(9f35dbfa) SHA1(8a8f158ad7f0bc6b43eaa95959af3ab58cf14d6d) )
	ROM_LOAD( "8.p7",   0x10000, 0x8000, CRC(f400db89) SHA1(a07b226af40cac5a20739bb8f4226909724fda86) )
	ROM_LOAD( "9.p8",   0x18000, 0x8000, CRC(fa2a11ea) SHA1(ea29ade1254caa2a3bd4b4816fe338f238025284) )
	ROM_LOAD( "10.p9",  0x20000, 0x8000, CRC(a4916537) SHA1(9defc8b22ba5119d8c3efb3eb1e28e835adffec2) ) // sldh - NOVA license
	ROM_LOAD( "11.p10", 0x28000, 0x8000, CRC(19037a7b) SHA1(a6843b0220bab5c47307a0c761d5bd638716aef0) )
	ROM_LOAD( "12.p11", 0x30000, 0x8000, CRC(c59c0ec7) SHA1(80003f3e33610a84f6e194918276d5f60145b00e) )
	ROM_LOAD( "13.p12", 0x38000, 0x8000, CRC(9809fee8) SHA1(b7e0664702d0c1f77247d7c76a381b24687a09ea) )
ROM_END



/***************************************************************************

                                    Brick Zone

SUNA ELECTRONICS IND CO., LTD

CPU Z0840006PSC (ZILOG)

Chrystal : 24.000 MHz

Sound CPU : Z084006PSC (ZILOG) + AY3-8910A

Warning ! This game has a 'SUNA' protection block :-(

-

(c) 1992 Suna Electronics

2 * Z80B

AY-3-8910
YM3812

24 MHz crystal

Large epoxy(?) module near the cpu's.

***************************************************************************/

ROM_START( brickzn )
	ROM_REGION( 0x50000 + 0x40000, "maincpu", 0 )       /* Main Z80 Code */
	ROM_LOAD( "p9.m7", 0x00000, 0x08000, CRC(bd7a3c01) SHA1(05fb2836f1c8d8818847ccb76e7b477f13a9929b) )  // V6.0 1992,3,16
	ROM_LOAD( "p8.k7", 0x10000, 0x20000, CRC(ec3e266d) SHA1(4441a5ae88e51353f6d1eb22c00becb0a7ecea6e) )
	ROM_LOAD( "p7.i7", 0x30000, 0x20000, CRC(4dd88631) SHA1(0dbcaf3420dad82c3ed94d231948fe69b044b786) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Music Z80 Code */
	ROM_LOAD( "10.o8", 0x00000, 0x10000, CRC(4eba8178) SHA1(9a214a1acacdc124529bc9dde73a8e884fc70293) )  // BRICK MUSIC XILINX PROGRAM 3020 1991,11,14 MUSIC PROGRAM V 2,0 1990.12.14

	ROM_REGION( 0x10000, "pcm", 0 )     /* PCM Z80 Code */
	ROM_LOAD( "11.n10", 0x00000, 0x10000, CRC(6c54161a) SHA1(ea216d9f45b441acd56b9fed81a83e3bfe299fbd) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "p5.m5", 0x00000, 0x20000, CRC(ca59e2f7) SHA1(dbb9f2b316a44f760768f0430798e0c4e9e8f3ff) )
	ROM_LOAD( "p4.l5", 0x20000, 0x20000, CRC(cc8fb330) SHA1(fd263f65b81acbfc00fe339c461068ab160c04af) )
	ROM_LOAD( "p3.k5", 0x40000, 0x20000, CRC(2e4f194b) SHA1(86da1a582ea274f2af96d3e3e2ac72bcaf3638cb) )
	ROM_LOAD( "p2.i5", 0x60000, 0x20000, CRC(592d45ce) SHA1(8ce9236b7deba6cf00999680314ac04eff624a6d) )
	ROM_LOAD( "p1.h5", 0x80000, 0x20000, CRC(7a6bb583) SHA1(ff7018c07182fce0ff6954bbe3b08fa5105f6be0) )
	ROM_LOAD( "p6.h7", 0xa0000, 0x20000, CRC(bbf31081) SHA1(1fdbd0e0853082345225e18df340184a7a604b78) )
ROM_END


ROM_START( brickznv5 )
	ROM_REGION( 0x50000 + 0x40000, "maincpu", 0 )       /* Main Z80 Code */
	ROM_LOAD( "brickzon.009", 0x00000, 0x08000, CRC(1ea68dea) SHA1(427152a26b062c5e77089de49c1da69369d4d557) )  // V5.0 1992,3,3
	ROM_LOAD( "brickzon.008", 0x10000, 0x20000, CRC(c61540ba) SHA1(08c0ede591b229427b910ca6bb904a6146110be8) )
	ROM_LOAD( "brickzon.007", 0x30000, 0x20000, CRC(ceed12f1) SHA1(9006726b75a65455afb1194298bade8fa2207b4a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Music Z80 Code */
	ROM_LOAD( "brickzon.010", 0x00000, 0x10000, CRC(4eba8178) SHA1(9a214a1acacdc124529bc9dde73a8e884fc70293) )  // BRICK MUSIC XILINX PROGRAM 3020 1991,11,14 MUSIC PROGRAM V 2,0 1990.12.14

	ROM_REGION( 0x10000, "pcm", 0 )     /* PCM Z80 Code */
	ROM_LOAD( "brickzon.011", 0x00000, 0x10000, CRC(6c54161a) SHA1(ea216d9f45b441acd56b9fed81a83e3bfe299fbd) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "brickzon.005", 0x00000, 0x20000, CRC(118f8392) SHA1(598fa4df3ae348ec9796cd6d90c3045bec546da3) )
	ROM_LOAD( "brickzon.004", 0x20000, 0x20000, CRC(2be5f335) SHA1(dc870a3c5303cb2ea1fea4a25f53db016ed5ecee) )
	ROM_LOAD( "brickzon.003", 0x40000, 0x20000, CRC(2e4f194b) SHA1(86da1a582ea274f2af96d3e3e2ac72bcaf3638cb) )
	ROM_LOAD( "brickzon.002", 0x60000, 0x20000, CRC(241f0659) SHA1(71b577bf7097b3b367d068df42f991d515f9003a) )
	ROM_LOAD( "brickzon.001", 0x80000, 0x20000, CRC(6970ada9) SHA1(5cfe5dcf25af7aff67ee5d78eb963d598251025f) )
	ROM_LOAD( "brickzon.006", 0xa0000, 0x20000, CRC(bbf31081) SHA1(1fdbd0e0853082345225e18df340184a7a604b78) )
ROM_END


ROM_START( brickznv4 )
	ROM_REGION( 0x50000 + 0x40000, "maincpu", 0 )       /* Main Z80 Code */
	ROM_LOAD( "39",           0x00000, 0x08000, CRC(043380bd) SHA1(7eea7cc7d754815df233879b4a9d3d88eac5b28d) )  // V3.0 1992,1,23
	ROM_LOAD( "38",           0x10000, 0x20000, CRC(e16216e8) SHA1(e88ae97e8a632823d5f1fe500954b6f6542407d5) )
	ROM_LOAD( "brickzon.007", 0x30000, 0x20000, CRC(ceed12f1) SHA1(9006726b75a65455afb1194298bade8fa2207b4a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Music Z80 Code */
	ROM_LOAD( "brickzon.010", 0x00000, 0x10000, CRC(4eba8178) SHA1(9a214a1acacdc124529bc9dde73a8e884fc70293) )  // BRICK MUSIC XILINX PROGRAM 3020 1991,11,14 MUSIC PROGRAM V 2,0 1990.12.14

	ROM_REGION( 0x10000, "pcm", 0 )     /* PCM Z80 Code */
	ROM_LOAD( "brickzon.011", 0x00000, 0x10000, CRC(6c54161a) SHA1(ea216d9f45b441acd56b9fed81a83e3bfe299fbd) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "35",           0x00000, 0x20000, CRC(b463dfcf) SHA1(35c8a4a0c5b62a087a2cb70bc5b7815f5bb2d973) )
	ROM_LOAD( "brickzon.004", 0x20000, 0x20000, CRC(2be5f335) SHA1(dc870a3c5303cb2ea1fea4a25f53db016ed5ecee) )
	ROM_LOAD( "brickzon.003", 0x40000, 0x20000, CRC(2e4f194b) SHA1(86da1a582ea274f2af96d3e3e2ac72bcaf3638cb) )
	ROM_LOAD( "32",           0x60000, 0x20000, CRC(32dbf2dd) SHA1(b9ab8b93a062b871b7f824e4be2f214cc832d585) )
	ROM_LOAD( "brickzon.001", 0x80000, 0x20000, CRC(6970ada9) SHA1(5cfe5dcf25af7aff67ee5d78eb963d598251025f) )
	ROM_LOAD( "brickzon.006", 0xa0000, 0x20000, CRC(bbf31081) SHA1(1fdbd0e0853082345225e18df340184a7a604b78) )
ROM_END

ROM_START( brickzn11 )
	ROM_REGION( 0x50000 + 0x40000, "maincpu", 0 )       /* Main Z80 Code */
	ROM_LOAD( "9.bin", 0x00000, 0x08000, CRC(24f88cfd) SHA1(dfa7313ab6696042bab2e6cc8ff97b331d526c6b) )  // V1.1 1992,1,13
	ROM_LOAD( "8.bin", 0x10000, 0x20000, CRC(e2c7f7ac) SHA1(43377daf6957829ef9bb7a81708c2f18f5d7ced6) )
	ROM_LOAD( "7.bin", 0x30000, 0x20000, CRC(7af5b25c) SHA1(9e98e99bdc5be1602144c83f40b2ccf6b90a729a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Music Z80 Code */
	ROM_LOAD( "10.bin", 0x00000, 0x10000, CRC(494adf0f) SHA1(eb28ccf0c5f38c2299f55e379ff73ba84bb793c6) )  // NO PROGRAM MUSIC PROGRAM V 2.4 1990.12.14

	ROM_REGION( 0x10000, "pcm", 0 )     /* PCM Z80 Code */
	ROM_LOAD( "11.bin", 0x00000, 0x10000, CRC(6c54161a) SHA1(ea216d9f45b441acd56b9fed81a83e3bfe299fbd) )

	ROM_REGION( 0xc0000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "5.bin", 0x00000, 0x20000, CRC(e9f73ba1) SHA1(4b5e294ae160ba3ca28b8956a797330234ace576) )
	ROM_LOAD( "4.bin", 0x20000, 0x20000, CRC(2be5f335) SHA1(dc870a3c5303cb2ea1fea4a25f53db016ed5ecee) )
	ROM_LOAD( "3.bin", 0x40000, 0x20000, CRC(2e4f194b) SHA1(86da1a582ea274f2af96d3e3e2ac72bcaf3638cb) )
	ROM_LOAD( "2.bin", 0x60000, 0x20000, CRC(0e994fbf) SHA1(62e059a5ca5f7199e597841f94519a466affe098) )
	ROM_LOAD( "1.bin", 0x80000, 0x20000, CRC(6970ada9) SHA1(5cfe5dcf25af7aff67ee5d78eb963d598251025f) )
	ROM_LOAD( "6.bin", 0xa0000, 0x20000, CRC(bbf31081) SHA1(1fdbd0e0853082345225e18df340184a7a604b78) )
ROM_END

/***************************************************************************

                                Hard Head 2

These ROMS are all 27C512

ROM 1 is at Location 1N
ROM 2 ..............1o
ROM 3 ..............1Q
ROM 4...............3N
ROM 5.............. 4N
ROM 6...............4o
ROM 7...............4Q
ROM 8...............6N
ROM 10..............H5
ROM 11..............i5
ROM 12 .............F7
ROM 13..............H7
ROM 15..............N10

These ROMs are 27C256

ROM 9...............F5
ROM 14..............C8

Game uses 2 Z80B processors and a Custom Sealed processor (assumed)
Labeled "SUNA T568009"

Sound is a Yamaha YM3812 and a  AY-3-8910A

3 RAMS are 6264LP- 10   and 5) HM6116K-90 rams  (small package)

24 MHz

***************************************************************************/

ROM_START( hardhea2 )
	ROM_REGION( 0x50000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "hrd-hd9",  0x00000, 0x08000, CRC(69c4c307) SHA1(0dfde1dcda51b5b1740aff9e96cb877a428a3e04) )  // V 2.0 1991,2,12
	ROM_LOAD( "hrd-hd10", 0x10000, 0x10000, CRC(77ec5b0a) SHA1(2d3e24c208904a7884e585e08e5818fd9f8b5391) )
	ROM_LOAD( "hrd-hd11", 0x20000, 0x10000, CRC(12af8f8e) SHA1(1b33a060b70900042fdae00f7dec325228d566f5) )
	ROM_LOAD( "hrd-hd12", 0x30000, 0x10000, CRC(35d13212) SHA1(2fd03077b89ec9e55d2758b7f9cada970f0bdd91) )
	ROM_LOAD( "hrd-hd13", 0x40000, 0x10000, CRC(3225e7d7) SHA1(2da9d1ce182dab8d9e09772e6899676b84c7458c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Music Z80 Code */
	ROM_LOAD( "hrd-hd14", 0x00000, 0x08000, CRC(79a3be51) SHA1(30bc67cd3a936615c6931f8e15953425dff59611) )

	ROM_REGION( 0x10000, "pcm", 0 )     /* PCM Z80 Code */
	ROM_LOAD( "hrd-hd15", 0x00000, 0x10000, CRC(bcbd88c3) SHA1(79782d598d9d764de70c54fc07ff9bf0f7d13d62) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT ) /* Sprites */
	ROM_LOAD( "hrd-hd1",  0x00000, 0x10000, CRC(7e7b7a58) SHA1(1a74260dda64aafcb046c8add92a54655bbc74e4) )
	ROM_LOAD( "hrd-hd2",  0x10000, 0x10000, CRC(303ec802) SHA1(533c29d9bb54415410c5d3c5af234b8b040190de) )
	ROM_LOAD( "hrd-hd3",  0x20000, 0x10000, CRC(3353b2c7) SHA1(a3ec0fc2a97e7e0bc72fafd5897cb1dd4cd32197) )
	ROM_LOAD( "hrd-hd4",  0x30000, 0x10000, CRC(dbc1f9c1) SHA1(720c729d7825635584632d033b4b46eea2fb1291) )
	ROM_LOAD( "hrd-hd5",  0x40000, 0x10000, CRC(f738c0af) SHA1(7dda657acd1d6fb7064e8dbd5ce386e9eae3d36a) )
	ROM_LOAD( "hrd-hd6",  0x50000, 0x10000, CRC(bf90d3ca) SHA1(2d0533d93fc5155fe879c1890bc7bc4581308e16) )
	ROM_LOAD( "hrd-hd7",  0x60000, 0x10000, CRC(992ce8cb) SHA1(21c0dd227138ec64003c7cb090855ec27d41719e) )
	ROM_LOAD( "hrd-hd8",  0x70000, 0x10000, CRC(359597a4) SHA1(ae024dd61c5d12813a661abe8ea63ae6112ddc9c) )
ROM_END


/***************************************************************************

                                Star Fighter

***************************************************************************/

ROM_START( starfigh )
	ROM_REGION( 0x50000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "starfgtr.l1", 0x00000, 0x08000, CRC(f93802c6) SHA1(4005b06b69dd440dfb6385766386a1168e73288f) )   // V.1
	ROM_LOAD( "starfgtr.j1", 0x10000, 0x10000, CRC(fcfcf08a) SHA1(65fe1666aa5092f820b337bcbcbed7accdec440d) )
	ROM_LOAD( "starfgtr.i1", 0x20000, 0x10000, CRC(6935fcdb) SHA1(f47812f6716ccf52dd7ab8522c29e059f1e38f31) )
	ROM_LOAD( "starfgtr.l3", 0x30000, 0x10000, CRC(50c072a4) SHA1(e48ec5a786ef245e5b2b72390824b6b7c449a74b) )   // 0xxxxxxxxxxxxxxx = 0xFF (ROM Test: OK)
	ROM_LOAD( "starfgtr.j3", 0x40000, 0x10000, CRC(3fe3c714) SHA1(ccc9a33cf29c0e43ae8ab91f08438a89c777c186) )   // clear text here

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Music Z80 Code */
	ROM_LOAD( "starfgtr.m8", 0x0000, 0x8000, CRC(ae3b0691) SHA1(41e004d09522cf7ddce6e4adc68841ad5553264a) )

	ROM_REGION( 0x8000, "samples", 0 )  /* Samples */
	ROM_LOAD( "starfgtr.q10", 0x0000, 0x8000, CRC(fa510e94) SHA1(e2742385a4ba152dbc89534e4350d1d9ad49730f) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT )    /* Sprites */
	// bitplanes 0-1
	ROM_LOAD( "starfgtr.e4",   0x00000, 0x10000, CRC(54c0ca3d) SHA1(87f785502beb8a52d47bd48275d695ee303054f8) ) // banks 00-03
	ROM_LOAD( "starfgtr.d4",   0x10000, 0x10000, CRC(4313ba40) SHA1(3c41f99dc40136517f172b3525987d8909f877c3) ) // banks 04-07
	ROM_COPY( "gfx1", 0x00000, 0x20000, 0x20000 )                                                               // banks 08-0f == 00-07
	ROM_LOAD( "starfgtr.b4",   0x40000, 0x10000, CRC(ad8d0f21) SHA1(ffdb407c7fe76b5f290de6bbed2fec34e40daf3f) ) // banks 10-13
	ROM_LOAD( "starfgtr.a4",   0x50000, 0x10000, CRC(6d8f74c8) SHA1(c40b77e27bd29d6c3a9b4d43189933c10543786b) ) // banks 14-17
	ROM_COPY( "gfx1", 0x40000, 0x60000, 0x20000 )                                                               // banks 18-1f == 10-17

	// bitplanes 2-3
	ROM_LOAD( "starfgtr.e6",   0x80000, 0x10000, CRC(ceff00ff) SHA1(5e7df7f33f36f4bc511be48266eaec274dfb8706) )
	ROM_LOAD( "starfgtr.d6",   0x90000, 0x10000, CRC(7aaa358a) SHA1(56d75f4abe626de7923d5bcc9ad18c02ce162907) )
	ROM_COPY( "gfx1", 0x80000, 0xa0000, 0x20000 )
	ROM_LOAD( "starfgtr.b6",   0xc0000, 0x10000, CRC(47d6049c) SHA1(cae0795a19cb6bb8bdabc10c200aa6f8d78dd347) )
	ROM_LOAD( "starfgtr.a6",   0xd0000, 0x10000, CRC(4a33f6f3) SHA1(daa0a1a43b1b60e2f05b9934fdd6b5f285a0b93a) )
	ROM_COPY( "gfx1", 0xc0000, 0xe0000, 0x20000 )
ROM_END


/***************************************************************************

                                Spark Man

Suna Electronics IND. CO., LTD 1989 KRB-16 60136-0081  Pinout = JAMMA

Game uses a Custom Sealed processor labeled "SUNA T568009" and a z80 processor for sound

Sound is a Yamaha YM3812 and a  AY-3-8910A

24mhz crystal

***************************************************************************/

ROM_START( sparkman )
	ROM_REGION( 0x50000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "sparkman.e7", 0x00000, 0x08000, CRC(d89c5780) SHA1(177f0ae21c00575a7eb078e86f3a790fc95211e4) )   // "SPARK MAN MAIN PROGRAM 1989,8,12 K.H.T (SUNA ELECTRPNICS) V 2.0 SOULE KOREA"
	ROM_LOAD( "10.g7",       0x10000, 0x10000, CRC(48b4a31e) SHA1(771d1f1a2ce950ce2b661a4081471e98a7a7d53e) )
	ROM_LOAD( "12.g8",       0x20000, 0x10000, CRC(b8a4a557) SHA1(10251b49fb44fb1e7c71fde8fe9544df29d27346) )
	ROM_LOAD( "11.i7",       0x30000, 0x10000, CRC(f5f38e1f) SHA1(25f0abbac1298fad1f8e7202db05e48c3598bc88) )
	ROM_LOAD( "13.i8",       0x40000, 0x10000, CRC(e54eea25) SHA1(b8ea884ee1a24953b6406f2d1edf103700f542d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Music Z80 Code */
	ROM_LOAD( "14.h11", 0x00000, 0x08000, CRC(06822f3d) SHA1(d30592cecbcd4dbf67e5a8d9c151d60b3232a54d) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT | ROMREGION_ERASEFF) /* Sprites (0) */
	// bitplanes 0-1
	ROM_LOAD( "p3.u1", 0x00000, 0x10000, CRC(39dbd414) SHA1(03fe938ed1191329b6a2f7ed54c6ef69273998df) ) // banks 00-03
	ROM_LOAD( "p2.t1", 0x10000, 0x10000, CRC(2e474203) SHA1(a407126d92e529568129d5246f89d51330ff5d32) ) // banks 04-07
	ROM_FILL(          0x20000, 0x10000, 0xFF )
	ROM_FILL(          0x30000, 0x10000, 0xFF )
	ROM_LOAD( "p1.r1", 0x40000, 0x08000, CRC(7115cfe7) SHA1(05fde6279a1edc97e79b1ff3f72b2da400a6a409) ) // banks 10,11
	ROM_FILL(          0x50000, 0x10000, 0xFF )
	ROM_FILL(          0x60000, 0x10000, 0xFF )
	ROM_FILL(          0x70000, 0x10000, 0xFF )
	// bitplanes 2-3
	ROM_LOAD( "p6.u2", 0x80000, 0x10000, CRC(e6551db9) SHA1(bed2a9ba72895f3ba876b4e0a41c33ea8a3c5af2) )
	ROM_LOAD( "p5.t2", 0x90000, 0x10000, CRC(0df5da2a) SHA1(abbd5ba22b30f17d203ecece7afafa0cbe78352c) )
	ROM_FILL(          0xa0000, 0x10000, 0xFF )
	ROM_FILL(          0xb0000, 0x10000, 0xFF )
	ROM_LOAD( "p4.r2", 0xc0000, 0x08000, CRC(6904bde2) SHA1(c426fa0c29b1874c729b981467f219c422f863aa) )
	ROM_FILL(          0xd0000, 0x10000, 0xFF )
	ROM_FILL(          0xe0000, 0x10000, 0xFF )
	ROM_FILL(          0xf0000, 0x10000, 0xFF )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT | ROMREGION_ERASEFF) /* Sprites (1) */
	// bitplanes 0-1
	ROM_LOAD( "p7.u4",         0x00000, 0x10000, CRC(17c16ce4) SHA1(b4127e9aedab69193bef1d85e68003e225913417) ) // banks 00-03 (alt gfx)
	ROM_COPY( "gfx2", 0x00000, 0x10000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x20000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x30000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x40000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x50000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x60000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x70000, 0x10000 )
	// bitplanes 2-3
	ROM_LOAD( "p8.u6",         0x80000, 0x10000, CRC(414222ea) SHA1(e05f0504c6e735c73027312a85cc55fc98728e53) )
	ROM_COPY( "gfx2", 0x80000, 0x90000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xa0000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xb0000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xc0000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xd0000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xe0000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xf0000, 0x10000 )

	ROM_REGION( 0x8000 * 2, "samples", 0 )      /* Samples */
	ROM_LOAD( "15.b10", 0x0000, 0x8000, CRC(46c7d4d8) SHA1(99f38cc044390ee4646498667ad2bf536ce91e8f) )
	ROM_LOAD( "16.b11", 0x8000, 0x8000, CRC(d6823a62) SHA1(f8ce748aa7bdc9c95799dd111fd872717e46d416) )
ROM_END

ROM_START( sparkmana )
	ROM_REGION( 0x50000, "maincpu", 0 )     /* Main Z80 Code */
	ROM_LOAD( "p9.7f", 0x00000, 0x08000, CRC(b114cb2b) SHA1(4f79bf65ef17147004f7a8d1d6a58dac0293cdc7) ) // sparkman.e7 99.972534% (9 bytes differ, version string is the same)
	ROM_LOAD( "10.g7", 0x10000, 0x10000, CRC(48b4a31e) SHA1(771d1f1a2ce950ce2b661a4081471e98a7a7d53e) )
	ROM_LOAD( "12.g8", 0x20000, 0x10000, CRC(b8a4a557) SHA1(10251b49fb44fb1e7c71fde8fe9544df29d27346) )
	ROM_LOAD( "11.i7", 0x30000, 0x10000, CRC(f5f38e1f) SHA1(25f0abbac1298fad1f8e7202db05e48c3598bc88) )
	ROM_LOAD( "13.i8", 0x40000, 0x10000, CRC(e54eea25) SHA1(b8ea884ee1a24953b6406f2d1edf103700f542d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Music Z80 Code */
	ROM_LOAD( "14.h11", 0x00000, 0x08000, CRC(06822f3d) SHA1(d30592cecbcd4dbf67e5a8d9c151d60b3232a54d) )

	ROM_REGION( 0x100000, "gfx1", ROMREGION_INVERT | ROMREGION_ERASEFF) /* Sprites (0) */
	// bitplanes 0-1
	ROM_LOAD( "p3.u1", 0x00000, 0x10000, CRC(39dbd414) SHA1(03fe938ed1191329b6a2f7ed54c6ef69273998df) ) // banks 00-03
	ROM_LOAD( "p2.t1", 0x10000, 0x10000, CRC(2e474203) SHA1(a407126d92e529568129d5246f89d51330ff5d32) ) // banks 04-07
	ROM_FILL(          0x20000, 0x10000, 0xFF )
	ROM_FILL(          0x30000, 0x10000, 0xFF )
	ROM_LOAD( "p1.r1", 0x40000, 0x08000, CRC(7115cfe7) SHA1(05fde6279a1edc97e79b1ff3f72b2da400a6a409) ) // banks 10,11
	ROM_FILL(          0x50000, 0x10000, 0xFF )
	ROM_FILL(          0x60000, 0x10000, 0xFF )
	ROM_FILL(          0x70000, 0x10000, 0xFF )
	// bitplanes 2-3
	ROM_LOAD( "p6.u2", 0x80000, 0x10000, CRC(e6551db9) SHA1(bed2a9ba72895f3ba876b4e0a41c33ea8a3c5af2) )
	ROM_LOAD( "p5.t2", 0x90000, 0x10000, CRC(0df5da2a) SHA1(abbd5ba22b30f17d203ecece7afafa0cbe78352c) )
	ROM_FILL(          0xa0000, 0x10000, 0xFF )
	ROM_FILL(          0xb0000, 0x10000, 0xFF )
	ROM_LOAD( "p4.r2", 0xc0000, 0x08000, CRC(6904bde2) SHA1(c426fa0c29b1874c729b981467f219c422f863aa) )
	ROM_FILL(          0xd0000, 0x10000, 0xFF )
	ROM_FILL(          0xe0000, 0x10000, 0xFF )
	ROM_FILL(          0xf0000, 0x10000, 0xFF )

	ROM_REGION( 0x100000, "gfx2", ROMREGION_INVERT | ROMREGION_ERASEFF) /* Sprites (1) */
	// bitplanes 0-1
	ROM_LOAD( "p7.u4",         0x00000, 0x10000, CRC(17c16ce4) SHA1(b4127e9aedab69193bef1d85e68003e225913417) ) // banks 00-03 (alt gfx)
	ROM_COPY( "gfx2", 0x00000, 0x10000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x20000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x30000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x40000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x50000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x60000, 0x10000 )
	ROM_COPY( "gfx2", 0x00000, 0x70000, 0x10000 )
	// bitplanes 2-3
	ROM_LOAD( "p8.u6",         0x80000, 0x10000, CRC(414222ea) SHA1(e05f0504c6e735c73027312a85cc55fc98728e53) )
	ROM_COPY( "gfx2", 0x80000, 0x90000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xa0000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xb0000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xc0000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xd0000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xe0000, 0x10000 )
	ROM_COPY( "gfx2", 0x80000, 0xf0000, 0x10000 )

	ROM_REGION( 0x8000 * 2, "samples", 0 )      /* Samples */
	ROM_LOAD( "15.b10", 0x0000, 0x8000, CRC(46c7d4d8) SHA1(99f38cc044390ee4646498667ad2bf536ce91e8f) )
	ROM_LOAD( "16.b11", 0x8000, 0x8000, CRC(d6823a62) SHA1(f8ce748aa7bdc9c95799dd111fd872717e46d416) )
ROM_END

/***************************************************************************


                                Games Drivers


***************************************************************************/

DRIVER_INIT_MEMBER(suna8_state,suna8)
{
	m_bank1->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);
}

GAME( 1988, sranger,   0,        rranger,  rranger,  suna8_state, suna8,     ROT0,  "SunA",                       "Super Ranger (v2.0)",                0 )
GAME( 1988, rranger,   sranger,  rranger,  rranger,  suna8_state, suna8,     ROT0,  "SunA (Sharp Image license)", "Rough Ranger (v2.0)",                0 )
GAME( 1988, rrangerb,  sranger,  rranger,  rranger,  suna8_state, suna8,     ROT0,  "bootleg",                    "Rough Ranger (v2.0, bootleg)",       0 )
GAME( 1988, srangero,  sranger,  rranger,  rranger,  suna8_state, suna8,     ROT0,  "SunA",                       "Super Ranger (older)",               0 )
GAME( 1988, srangern,  sranger,  rranger,  rranger,  suna8_state, suna8,     ROT0,  "SunA (NOVA license)",        "Super Ranger (older, NOVA license)", 0 )
GAME( 1988, srangerw,  sranger,  rranger,  rranger,  suna8_state, suna8,     ROT0,  "SunA (WDK license)",         "Super Ranger (older, WDK license)",  0 )
GAME( 1988, srangerb,  sranger,  rranger,  rranger,  suna8_state, suna8,     ROT0,  "bootleg (NYWA)",             "Super Ranger (older, bootleg)",      0 )

GAME( 1988, hardhead,  0,        hardhead, hardhead, suna8_state, hardhead,  ROT0,  "SunA",                       "Hard Head",                   0 )
GAME( 1988, hardheadb, hardhead, hardhead, hardhead, suna8_state, hardhedb,  ROT0,  "bootleg",                    "Hard Head (bootleg)",         0 )
GAME( 1988, pop_hh,    hardhead, hardhead, hardhead, suna8_state, hardhedb,  ROT0,  "bootleg",                    "Popper (Hard Head bootleg)",  0 )

GAME( 1989, sparkman,  0,        sparkman, sparkman, suna8_state, sparkman,  ROT0,  "SunA",                       "Spark Man (v2.0, set 1)",     0 )
GAME( 1989, sparkmana, sparkman, sparkman, sparkman, suna8_state, sparkman,  ROT0,  "SunA",                       "Spark Man (v2.0, set 2)",     0 )

GAME( 1990, starfigh,  0,        starfigh, starfigh, suna8_state, starfigh,  ROT90, "SunA",                       "Star Fighter (v1)",           MACHINE_IMPERFECT_GRAPHICS )

GAME( 1991, hardhea2,  0,        hardhea2, hardhea2, suna8_state, hardhea2,  ROT0,  "SunA",                       "Hard Head 2 (v2.0)",          0 )

GAME( 1992, brickzn,   0,        brickzn,  brickznv6,suna8_state, brickzn,   ROT90, "SunA",                       "Brick Zone (v6.0, Joystick)", 0 )
GAME( 1992, brickznv5, brickzn,  brickzn,  brickzn,  suna8_state, brickznv5, ROT90, "SunA",                       "Brick Zone (v5.0, Joystick)", 0 )
GAME( 1992, brickznv4, brickzn,  brickzn,  brickzn,  suna8_state, brickznv4, ROT90, "SunA",                       "Brick Zone (v4.0, Spinner)",  0 )
GAME( 1992, brickzn11, brickzn,  brickzn11,brickzn,  suna8_state, brickzn11, ROT90, "SunA",                       "Brick Zone (v1.1, Spinner)",  0 )
