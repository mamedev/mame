// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "includes/scramble.h"


MACHINE_RESET_MEMBER(scramble_state,scramble)
{
	MACHINE_RESET_CALL_MEMBER(galaxold);

	if (m_audiocpu != NULL)
		sh_init();
}

MACHINE_RESET_MEMBER(scramble_state,explorer)
{
	UINT8 *RAM = memregion("maincpu")->base();
	RAM[0x47ff] = 0; /* If not set, it doesn't reset after the 1st time */

	MACHINE_RESET_CALL_MEMBER(galaxold);
}


CUSTOM_INPUT_MEMBER(scramble_state::darkplnt_custom_r)
{
	static const UINT8 remap[] = {0x03, 0x02, 0x00, 0x01, 0x21, 0x20, 0x22, 0x23,
								0x33, 0x32, 0x30, 0x31, 0x11, 0x10, 0x12, 0x13,
								0x17, 0x16, 0x14, 0x15, 0x35, 0x34, 0x36, 0x37,
								0x3f, 0x3e, 0x3c, 0x3d, 0x1d, 0x1c, 0x1e, 0x1f,
								0x1b, 0x1a, 0x18, 0x19, 0x39, 0x38, 0x3a, 0x3b,
								0x2b, 0x2a, 0x28, 0x29, 0x09, 0x08, 0x0a, 0x0b,
								0x0f, 0x0e, 0x0c, 0x0d, 0x2d, 0x2c, 0x2e, 0x2f,
								0x27, 0x26, 0x24, 0x25, 0x05, 0x04, 0x06, 0x07 };
	UINT8 val = ioport((const char *)param)->read();

	return remap[val >> 2];
}

/* state of the security PAL (6J) */


READ8_MEMBER(scramble_state::mariner_protection_1_r )
{
	return 7;
}

READ8_MEMBER(scramble_state::mariner_protection_2_r )
{
	return 3;
}


READ8_MEMBER(scramble_state::triplep_pip_r )
{
	logerror("PC %04x: triplep read port 2\n",space.device().safe_pc());
	if (space.device().safe_pc() == 0x015a) return 0xff;
	else if (space.device().safe_pc() == 0x0886) return 0x05;
	else return 0;
}

READ8_MEMBER(scramble_state::triplep_pap_r )
{
	logerror("PC %04x: triplep read port 3\n",space.device().safe_pc());
	if (space.device().safe_pc() == 0x015d) return 0x04;
	else return 0;
}



void scramble_state::cavelon_banksw()
{
	/* any read/write access in the 0x8000-0xffff region causes a bank switch.
	   Only the lower 0x2000 is switched but we switch the whole region
	   to keep the CPU core happy at the boundaries */

	m_cavelon_bank = !m_cavelon_bank;
	membank("bank1")->set_entry(m_cavelon_bank);
}

READ8_MEMBER(scramble_state::cavelon_banksw_r )
{
	cavelon_banksw();

	if ((offset >= 0x0100) && (offset <= 0x0103))
		return m_ppi8255_0->read(space, offset - 0x0100);
	else if ((offset >= 0x0200) && (offset <= 0x0203))
		return m_ppi8255_1->read(space, offset - 0x0200);

	return 0xff;
}

WRITE8_MEMBER(scramble_state::cavelon_banksw_w )
{
	cavelon_banksw();

	if ((offset >= 0x0100) && (offset <= 0x0103))
		m_ppi8255_0->write(space, offset - 0x0100, data);
	else if ((offset >= 0x0200) && (offset <= 0x0203))
		m_ppi8255_1->write(space, offset - 0x0200, data);
}


READ8_MEMBER(scramble_state::hunchbks_mirror_r )
{
	return space.read_byte(0x1000+offset);
}

WRITE8_MEMBER(scramble_state::hunchbks_mirror_w )
{
	space.write_byte(0x1000+offset,data);
}



DRIVER_INIT_MEMBER(scramble_state,scramble_ppi)
{
}

DRIVER_INIT_MEMBER(scramble_state,scobra)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xa803, 0xa803, write8_delegate(FUNC(scramble_state::scrambold_background_enable_w),this));
}

#ifdef UNUSED_FUNCTION
DRIVER_INIT_MEMBER(scramble_state,atlantis)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x6803, 0x6803, write8_delegate(FUNC(scramble_state::scrambold_background_enable_w),this));
}

DRIVER_INIT_MEMBER(scramble_state,scramble)
{
	DRIVER_INIT_CALL(atlantis);
}
#endif

DRIVER_INIT_MEMBER(scramble_state,stratgyx)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xb000, 0xb000, write8_delegate(FUNC(scramble_state::scrambold_background_green_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xb002, 0xb002, write8_delegate(FUNC(scramble_state::scrambold_background_blue_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xb00a, 0xb00a, write8_delegate(FUNC(scramble_state::scrambold_background_red_w),this));
}

DRIVER_INIT_MEMBER(scramble_state,tazmani2)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xb002, 0xb002, write8_delegate(FUNC(scramble_state::scrambold_background_enable_w),this));
}

DRIVER_INIT_MEMBER(scramble_state,ckongs)
{
}

DRIVER_INIT_MEMBER(scramble_state,mariner)
{
	/* extra ROM */
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x5800, 0x67ff, "bank1");
	m_maincpu->space(AS_PROGRAM).unmap_write(0x5800, 0x67ff);
	membank("bank1")->set_base(memregion("maincpu")->base() + 0x5800);

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x9008, 0x9008, read8_delegate(FUNC(scramble_state::mariner_protection_2_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xb401, 0xb401, read8_delegate(FUNC(scramble_state::mariner_protection_1_r),this));

	/* ??? (it's NOT a background enable) */
	/*m_maincpu->space(AS_PROGRAM).nop_write(0x6803, 0x6803);*/
}

#ifdef UNUSED_FUNCTION
DRIVER_INIT_MEMBER(scramble_state,frogger)
{
	offs_t A;
	UINT8 *ROM;

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	ROM = memregion("audiocpu")->base();
	for (A = 0;A < 0x0800;A++)
		ROM[A] = BITSWAP8(ROM[A],7,6,5,4,3,2,0,1);

	/* likewise, the 2nd gfx ROM has data lines D0 and D1 swapped. Decode it. */
	ROM = memregion("gfx1")->base();
	for (A = 0x0800;A < 0x1000;A++)
		ROM[A] = BITSWAP8(ROM[A],7,6,5,4,3,2,0,1);
}

DRIVER_INIT_MEMBER(scramble_state,froggers)
{
	offs_t A;
	UINT8 *ROM;

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	ROM = memregion("audiocpu")->base();
	for (A = 0;A < 0x0800;A++)
		ROM[A] = BITSWAP8(ROM[A],7,6,5,4,3,2,0,1);
}
#endif

DRIVER_INIT_MEMBER(scramble_state,devilfsh)
{
	offs_t i;
	UINT8 *RAM;

	/* Address lines are scrambled on the main CPU */

	/* A0 -> A2 */
	/* A1 -> A0 */
	/* A2 -> A3 */
	/* A3 -> A1 */

	RAM = memregion("maincpu")->base();
	for (i = 0; i < 0x10000; i += 16)
	{
		offs_t j;
		UINT8 swapbuffer[16];

		for (j = 0; j < 16; j++)
		{
			offs_t newval = BITSWAP8(j,7,6,5,4,2,0,3,1);

			swapbuffer[j] = RAM[i + newval];
		}

		memcpy(&RAM[i], swapbuffer, 16);
	}
}

DRIVER_INIT_MEMBER(scramble_state,mars)
{
	DRIVER_INIT_CALL(devilfsh);
}

DRIVER_INIT_MEMBER(scramble_state,hotshock)
{
	/* protection??? The game jumps into never-neverland here. I think
	   it just expects a RET there */
	memregion("maincpu")->base()[0x2ef9] = 0xc9;
}

DRIVER_INIT_MEMBER(scramble_state,cavelon)
{
	UINT8 *ROM = memregion("maincpu")->base();

	/* banked ROM */
	m_maincpu->space(AS_PROGRAM).install_read_bank(0x0000, 0x3fff, "bank1");
	membank("bank1")->configure_entries(0, 2, &ROM[0x00000], 0x10000);
	cavelon_banksw();

	/* A15 switches memory banks */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x8000, 0xffff, read8_delegate(FUNC(scramble_state::cavelon_banksw_r),this), write8_delegate(FUNC(scramble_state::cavelon_banksw_w),this));

	m_maincpu->space(AS_PROGRAM).nop_write(0x2000, 0x2000);  /* ??? */
	m_maincpu->space(AS_PROGRAM).nop_write(0x3800, 0x3801);  /* looks suspicously like
                                                               an AY8910, but not sure */
	save_item(NAME(m_cavelon_bank));
}



DRIVER_INIT_MEMBER(scramble_state,darkplnt)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xb00a, 0xb00a, write8_delegate(FUNC(scramble_state::darkplnt_bullet_color_w),this));
}

DRIVER_INIT_MEMBER(scramble_state,mimonkey)
{
	static const UINT8 xortable[16][16] =
	{
		{ 0x03,0x03,0x05,0x07,0x85,0x00,0x85,0x85,0x80,0x80,0x06,0x03,0x03,0x00,0x00,0x81 },
		{ 0x83,0x87,0x03,0x87,0x06,0x00,0x06,0x04,0x02,0x00,0x84,0x84,0x04,0x00,0x01,0x83 },
		{ 0x82,0x82,0x84,0x02,0x04,0x00,0x00,0x03,0x82,0x00,0x06,0x80,0x03,0x00,0x81,0x07 },
		{ 0x06,0x06,0x82,0x81,0x85,0x00,0x04,0x07,0x81,0x05,0x04,0x00,0x03,0x00,0x82,0x84 },
		{ 0x07,0x07,0x80,0x07,0x07,0x00,0x85,0x86,0x00,0x07,0x06,0x04,0x85,0x00,0x86,0x85 },
		{ 0x81,0x83,0x02,0x02,0x87,0x00,0x86,0x03,0x04,0x06,0x80,0x05,0x87,0x00,0x81,0x81 },
		{ 0x01,0x01,0x00,0x07,0x07,0x00,0x01,0x01,0x07,0x07,0x06,0x00,0x06,0x00,0x07,0x07 },
		{ 0x80,0x87,0x81,0x87,0x83,0x00,0x84,0x01,0x01,0x86,0x86,0x80,0x86,0x00,0x86,0x86 },
		{ 0x03,0x03,0x05,0x07,0x85,0x00,0x85,0x85,0x80,0x80,0x06,0x03,0x03,0x00,0x00,0x81 },
		{ 0x83,0x87,0x03,0x87,0x06,0x00,0x06,0x04,0x02,0x00,0x84,0x84,0x04,0x00,0x01,0x83 },
		{ 0x82,0x82,0x84,0x02,0x04,0x00,0x00,0x03,0x82,0x00,0x06,0x80,0x03,0x00,0x81,0x07 },
		{ 0x06,0x06,0x82,0x81,0x85,0x00,0x04,0x07,0x81,0x05,0x04,0x00,0x03,0x00,0x82,0x84 },
		{ 0x07,0x07,0x80,0x07,0x07,0x00,0x85,0x86,0x00,0x07,0x06,0x04,0x85,0x00,0x86,0x85 },
		{ 0x81,0x83,0x02,0x02,0x87,0x00,0x86,0x03,0x04,0x06,0x80,0x05,0x87,0x00,0x81,0x81 },
		{ 0x01,0x01,0x00,0x07,0x07,0x00,0x01,0x01,0x07,0x07,0x06,0x00,0x06,0x00,0x07,0x07 },
		{ 0x80,0x87,0x81,0x87,0x83,0x00,0x84,0x01,0x01,0x86,0x86,0x80,0x86,0x00,0x86,0x86 }
	};

	UINT8 *ROM = memregion("maincpu")->base();
	int A, ctr = 0, line, col;

	for( A = 0; A < 0x4000; A++ )
	{
		line = (ctr & 0x07) | ((ctr & 0x200) >> 6);
		col = ((ROM[A] & 0x80) >> 4) | (ROM[A] & 0x07);
		ROM[A] = ROM[A] ^ xortable[line][col];
		ctr++;
	}
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xa804, 0xa804, write8_delegate(FUNC(scramble_state::scrambold_background_enable_w),this));
}

DRIVER_INIT_MEMBER(scramble_state,mimonsco)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xa804, 0xa804, write8_delegate(FUNC(scramble_state::scrambold_background_enable_w),this));
}

DRIVER_INIT_MEMBER(scramble_state,mimonscr)
{
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x6804, 0x6804, write8_delegate(FUNC(scramble_state::scrambold_background_enable_w),this));
}


inline int scramble_state::bit(int i,int n)
{
	return ((i >> n) & 1);
}


#ifdef UNUSED_FUNCTION
DRIVER_INIT_MEMBER(scramble_state,anteater)
{
	offs_t i, len;
	UINT8 *RAM;
	UINT8 *scratch;


	DRIVER_INIT_CALL(scobra);

	/*
	*   Code To Decode Lost Tomb by Mirko Buffoni
	*   Optimizations done by Fabio Buffoni
	*/

	RAM = memregion("gfx1")->base();
	len = memregion("gfx1")->bytes();

	scratch = alloc_array_or_die(UINT8, len);

		memcpy(scratch, RAM, len);

		for (i = 0; i < len; i++)
		{
			int j;


			j = i & 0x9bf;
			j |= ( bit(i,4) ^ bit(i,9) ^ ( bit(i,2) & bit(i,10) ) ) << 6;
			j |= ( bit(i,2) ^ bit(i,10) ) << 9;
			j |= ( bit(i,0) ^ bit(i,6) ^ 1 ) << 10;

			RAM[i] = scratch[j];
		}

		free(scratch);
}
#endif

DRIVER_INIT_MEMBER(scramble_state,rescue)
{
	offs_t i, len;
	UINT8 *RAM;


	DRIVER_INIT_CALL(scobra);

	/*
	*   Code To Decode Lost Tomb by Mirko Buffoni
	*   Optimizations done by Fabio Buffoni
	*/

	RAM = memregion("gfx1")->base();
	len = memregion("gfx1")->bytes();

	dynamic_buffer scratch(len);

	memcpy(&scratch[0], RAM, len);

	for (i = 0; i < len; i++)
	{
		int j;


		j = i & 0xa7f;
		j |= ( bit(i,3) ^ bit(i,10) ) << 7;
		j |= ( bit(i,1) ^ bit(i,7) ) << 8;
		j |= ( bit(i,0) ^ bit(i,8) ) << 10;

		RAM[i] = scratch[j];
	}
}

DRIVER_INIT_MEMBER(scramble_state,minefld)
{
	offs_t i, len;
	UINT8 *RAM;


	DRIVER_INIT_CALL(scobra);

	/*
	*   Code To Decode Minefield by Mike Balfour and Nicola Salmoria
	*/

	RAM = memregion("gfx1")->base();
	len = memregion("gfx1")->bytes();

	dynamic_buffer scratch(len);

	memcpy(&scratch[0], RAM, len);

	for (i = 0; i < len; i++)
	{
		int j;


		j  = i & 0xd5f;
		j |= ( bit(i,3) ^ bit(i,7) ) << 5;
		j |= ( bit(i,2) ^ bit(i,9) ^ ( bit(i,0) & bit(i,5) ) ^
				( bit(i,3) & bit(i,7) & ( bit(i,0) ^ bit(i,5) ))) << 7;
		j |= ( bit(i,0) ^ bit(i,5) ^ ( bit(i,3) & bit(i,7) ) ) << 9;

		RAM[i] = scratch[j];
	}
}

#ifdef UNUSED_FUNCTION
DRIVER_INIT_MEMBER(scramble_state,losttomb)
{
	offs_t i, len;
	UINT8 *RAM;
	UINT8 *scratch;


	DRIVER_INIT_CALL(scramble);

	/*
	*   Code To Decode Lost Tomb by Mirko Buffoni
	*   Optimizations done by Fabio Buffoni
	*/

	RAM = memregion("gfx1")->base();
	len = memregion("gfx1")->bytes();

	scratch = alloc_array_or_die(UINT8, len);

		memcpy(scratch, RAM, len);

		for (i = 0; i < len; i++)
		{
			int j;


			j = i & 0xa7f;
			j |= ( (bit(i,1) & bit(i,8)) | ((1 ^ bit(i,1)) & (bit(i,10)))) << 7;
			j |= ( bit(i,7) ^ (bit(i,1) & ( bit(i,7) ^ bit(i,10) ))) << 8;
			j |= ( (bit(i,1) & bit(i,7)) | ((1 ^ bit(i,1)) & (bit(i,8)))) << 10;

			RAM[i] = scratch[j];
		}

		free(scratch);
}
#endif

DRIVER_INIT_MEMBER(scramble_state,hustler)
{
	offs_t A;
	UINT8 *rom = memregion("maincpu")->base();


	for (A = 0;A < 0x4000;A++)
	{
		UINT8 xormask;
		int bits[8];
		int i;


		for (i = 0;i < 8;i++)
			bits[i] = (A >> i) & 1;

		xormask = 0xff;
		if (bits[0] ^ bits[1]) xormask ^= 0x01;
		if (bits[3] ^ bits[6]) xormask ^= 0x02;
		if (bits[4] ^ bits[5]) xormask ^= 0x04;
		if (bits[0] ^ bits[2]) xormask ^= 0x08;
		if (bits[2] ^ bits[3]) xormask ^= 0x10;
		if (bits[1] ^ bits[5]) xormask ^= 0x20;
		if (bits[0] ^ bits[7]) xormask ^= 0x40;
		if (bits[4] ^ bits[6]) xormask ^= 0x80;

		rom[A] ^= xormask;
	}

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	{
		rom = memregion("audiocpu")->base();


		for (A = 0;A < 0x0800;A++)
			rom[A] = BITSWAP8(rom[A],7,6,5,4,3,2,0,1);
	}
}

DRIVER_INIT_MEMBER(scramble_state,hustlerd)
{
	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	offs_t A;
	UINT8 *rom = memregion("audiocpu")->base();


	for (A = 0;A < 0x0800;A++)
		rom[A] = BITSWAP8(rom[A],7,6,5,4,3,2,0,1);
}

DRIVER_INIT_MEMBER(scramble_state,billiard)
{
	offs_t A;
	UINT8 *rom = memregion("maincpu")->base();


	for (A = 0;A < 0x4000;A++)
	{
		UINT8 xormask;
		int bits[8];
		int i;


		for (i = 0;i < 8;i++)
			bits[i] = (A >> i) & 1;

		xormask = 0x55;
		if (bits[2] ^ (( bits[3]) & ( bits[6]))) xormask ^= 0x01;
		if (bits[4] ^ (( bits[5]) & ( bits[7]))) xormask ^= 0x02;
		if (bits[0] ^ (( bits[7]) & (!bits[3]))) xormask ^= 0x04;
		if (bits[3] ^ ((!bits[0]) & ( bits[2]))) xormask ^= 0x08;
		if (bits[5] ^ ((!bits[4]) & ( bits[1]))) xormask ^= 0x10;
		if (bits[6] ^ ((!bits[2]) & (!bits[5]))) xormask ^= 0x20;
		if (bits[1] ^ ((!bits[6]) & (!bits[4]))) xormask ^= 0x40;
		if (bits[7] ^ ((!bits[1]) & ( bits[0]))) xormask ^= 0x80;

		rom[A] ^= xormask;

		rom[A] = BITSWAP8(rom[A],6,1,2,5,4,3,0,7);
	}

	/* the first ROM of the second CPU has data lines D0 and D1 swapped. Decode it. */
	{
		rom = memregion("audiocpu")->base();


		for (A = 0;A < 0x0800;A++)
			rom[A] = BITSWAP8(rom[A],7,6,5,4,3,2,0,1);
	}
}

/************************************************************
 mr kougar protected main cpu - by HIGHWAYMAN
 mr kougar contains a steel module at location S7,
 this module contains a Z80c cpu with the following changes:
 IOREQ pin cut, RD & WR pins swapped and the following
 address lines swapped - a0-a2,a1-a0,a2-a3,a3-a1.
*************************************************************/

DRIVER_INIT_MEMBER(scramble_state,mrkougar)
{
	DRIVER_INIT_CALL(devilfsh);
}

DRIVER_INIT_MEMBER(scramble_state,mrkougb)
{
}

DRIVER_INIT_MEMBER(scramble_state,ad2083)
{
	UINT8 c;
	int i, len = memregion("maincpu")->bytes();
	UINT8 *ROM = memregion("maincpu")->base();

	for (i=0; i<len; i++)
	{
		c = ROM[i] ^ 0x35;
		c = BITSWAP8(c, 6,2,5,1,7,3,4,0); /* also swapped inside of the bigger module */
		ROM[i] = c;
	}
}

/************************************************************
 Harem run-time decryption
*************************************************************/

WRITE8_MEMBER(scramble_state::harem_decrypt_bit_w)
{
	m_harem_decrypt_bit = data;
}

WRITE8_MEMBER(scramble_state::harem_decrypt_clk_w)
{
	if ((data & 1) && !(m_harem_decrypt_clk & 1))
	{
		m_harem_decrypt_mode = ((m_harem_decrypt_mode >> 1) | ((m_harem_decrypt_bit & 1) << 3)) & 0x0f;
		m_harem_decrypt_count++;

//      logerror("%s: decrypt mode = %02x, count = %x\n", machine().describe_context(), m_harem_decrypt_mode, m_harem_decrypt_count);
	}

	m_harem_decrypt_clk = data;

	if (m_harem_decrypt_count == 4)
	{
		int bank;
		switch (m_harem_decrypt_mode)
		{
			case 0x03:  bank = 0;   break;
			case 0x09:  bank = 1;   break;
			case 0x0a:  bank = 2;   break;
			default:
				logerror("%s: warning, unknown decrypt mode = %02x\n", machine().describe_context(), m_harem_decrypt_mode);
				bank = 0;
		}

		membank("rombank")->set_base            (m_harem_decrypted_data     + 0x2000 * bank);
		membank("rombank_decrypted")->set_base  (m_harem_decrypted_opcodes  + 0x2000 * bank);

//      logerror("%s: decrypt mode = %02x (bank %x) active\n", machine().describe_context(), m_harem_decrypt_mode, bank);

		m_harem_decrypt_mode = 0;
		m_harem_decrypt_count = 0;
	}
}

WRITE8_MEMBER(scramble_state::harem_decrypt_rst_w)
{
	m_harem_decrypt_mode = 0;
	m_harem_decrypt_count = 0;

//  logerror("%s: decrypt mode reset\n", machine().describe_context());
}

DRIVER_INIT_MEMBER(scramble_state,harem)
{
	UINT8 *ROM      =   memregion("maincpu")->base() + 0x8000;
	size_t size     =   0x2000;

	UINT8 *data     =   m_harem_decrypted_data      = auto_alloc_array(machine(), UINT8, size * 3);
	UINT8 *opcodes  =   m_harem_decrypted_opcodes   = auto_alloc_array(machine(), UINT8, size * 3);

	// decryption 03
	for (int i = 0; i < size; i++)
	{
		UINT8 x = ROM[i];
		opcodes[size * 0 + i]   =   BITSWAP8(x, 7,0,5,2,3,4,1,6);
		data   [size * 0 + i]   =   BITSWAP8(x, 7,6,5,0,3,4,1,2);
	}

	// decryption 09
	for (int i = 0; i < size; i++)
	{
		UINT8 x = ROM[i];
		opcodes[size * 1 + i]   =   BITSWAP8(x, 7,0,5,6,3,2,1,4);
		data   [size * 1 + i]   =   BITSWAP8(x, 7,4,5,0,3,6,1,2);
	}

	// decryption 0a
	for (int i = 0; i < size; i++)
	{
		UINT8 x = ROM[i];
		opcodes[size * 2 + i]   =   BITSWAP8(x, 7,2,5,6,3,0,1,4);
		data   [size * 2 + i]   =   BITSWAP8(x, 7,2,5,4,3,0,1,6);
	}

	membank("rombank")->set_base            (m_harem_decrypted_data);
	membank("rombank_decrypted")->set_base  (m_harem_decrypted_opcodes);

	save_item(NAME(m_harem_decrypt_mode));
	save_item(NAME(m_harem_decrypt_bit));
	save_item(NAME(m_harem_decrypt_clk));
	save_item(NAME(m_harem_decrypt_count));
}

DRIVER_INIT_MEMBER(scramble_state,newsin7a)
{
	DRIVER_INIT_CALL(devilfsh); // decrypt

//  UINT8 *ROM = memregion("maincpu")->base();
//  ROM[0x0067] ^= 0x22;          /* rst $00         - should be push hl - the NMI routine is corrupt in this set, but the IRQ routine bypasses it? intentional? */

	// attempts to access port at c20x instead of 820x in one location, mirror or bitrot?
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xc200, 0xc20f, read8_delegate(FUNC(scramble_state::mars_ppi8255_1_r),this), write8_delegate(FUNC(scramble_state::mars_ppi8255_1_w),this));

}
