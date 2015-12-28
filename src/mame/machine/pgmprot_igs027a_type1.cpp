// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi, Xing Xing
/***********************************************************************
 PGM IGS027A ARM protection simulations & emulation - type 1

 these are simulations of the 'kov' type ARM device
 used by

 Knights of Valor (kov) + bootlegs
 Knights of Valor Plus (kovplus)
 Puzzli 2 / Puzzli 2 Super (puzzli2s)
 Photo Y2k2 (py2k2)
 Photo Y2k2 - Flash 3-in-1 (pgm3in1)
 Puzzle Star (puzlstar)

 These are implemented in 55857E type chips

 the following appear to have the same basic behavior as the
 early '55857E' type chips, but are actually using the '55857G'
 chips, which execute only area (confirmed on ddpdoj at least)

 DoDonPachi Dai-ou-jou (ddpdoj)
 Espgaluda (espgal)
 Ketsui (ket)
 Oriental Legend Super Plus (oldsplus)
 Knights of Valor Super Plus (kovshp) + bootlegs

 the following also use the 55857E type and we emulate the
 internal ROM

 Photo Y2k (photoy2k)
 Knights of Valor Superheros (kovsh) + bootlegs

 ----

 Many of the simulations are preliminary.  If functions to access
 internal tables exist it is sometimes possible to extract the ROM
 via these functions if the buffers are unchecked, however many
 games have no table accesses.

 ----

 The basic protection communication is the same between all games
 however the commands differ

 None of these games have an external ARM rom, although it appears
 the program code does check for the possibility of one existing.

 The 68k ROM gets checksummed by the ARM, the code doesn't even
 get decrytped if it fails.

 68k code is encrypted on these, decryption table is uploaded to
 ARM space.

 Game Region is supplied by internal ARM rom.


 ***********************************************************************/

#include "emu.h"
#include "includes/pgm.h"

/**************************** EMULATION *******************************/
/* used by photoy2k, kovsh */

READ32_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_protlatch_r )
{
	machine().scheduler().synchronize(); // force resync

	return (m_pgm_arm_type1_highlatch_68k_w << 16) | (m_pgm_arm_type1_lowlatch_68k_w);
}

WRITE32_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_protlatch_w )
{
	machine().scheduler().synchronize(); // force resync

	if (ACCESSING_BITS_16_31)
	{
		m_pgm_arm_type1_highlatch_arm_w = data >> 16;
		m_pgm_arm_type1_highlatch_68k_w = 0;
	}
	if (ACCESSING_BITS_0_15)
	{
		m_pgm_arm_type1_lowlatch_arm_w = data;
		m_pgm_arm_type1_lowlatch_68k_w = 0;
	}
}

READ16_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_68k_protlatch_r )
{
	machine().scheduler().synchronize(); // force resync

	switch (offset)
	{
		case 1: return m_pgm_arm_type1_highlatch_arm_w;
		case 0: return m_pgm_arm_type1_lowlatch_arm_w;
	}
	return -1;
}

WRITE16_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_68k_protlatch_w )
{
	machine().scheduler().synchronize(); // force resync

	switch (offset)
	{
		case 1:
			m_pgm_arm_type1_highlatch_68k_w = data;
			break;

		case 0:
			m_pgm_arm_type1_lowlatch_68k_w = data;
			break;
	}
}

READ16_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_ram_r )
{
	UINT16 *share16 = reinterpret_cast<UINT16 *>(m_arm7_shareram.target());

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Read: %04x = %04x (%08x) (%06x)\n", BYTE_XOR_LE(offset), share16[BYTE_XOR_LE(offset)], mem_mask, space.device().safe_pc());
	return share16[BYTE_XOR_LE(offset << 1)];
}

WRITE16_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_ram_w )
{
	UINT16 *share16 = reinterpret_cast<UINT16 *>(m_arm7_shareram.target());

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Write: %04x = %04x (%04x) (%06x)\n", BYTE_XOR_LE(offset), data, mem_mask, space.device().safe_pc());
	COMBINE_DATA(&share16[BYTE_XOR_LE(offset << 1)]);
}




READ32_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_unk_r )
{
	return m_pgm_arm_type1_counter++;
}

READ32_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_exrom_r )
{
	return 0x00000000;
}

READ32_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_shareram_r )
{
	if (PGMARM7LOGERROR)
		logerror("ARM7: ARM7 Shared RAM Read: %04x = %08x (%08x) (%06x)\n", offset << 2, m_arm7_shareram[offset], mem_mask, space.device().safe_pc());
	return m_arm7_shareram[offset];
}

WRITE32_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_shareram_w )
{
	if (PGMARM7LOGERROR)
		logerror("ARM7: ARM7 Shared RAM Write: %04x = %08x (%08x) (%06x)\n", offset << 2, data, mem_mask, space.device().safe_pc());
	COMBINE_DATA(&m_arm7_shareram[offset]);
}

/* 55857E? */
/* Knights of Valor, Photo Y2k */
/*  no execute only space? */
static ADDRESS_MAP_START( kov_map, AS_PROGRAM, 16, pgm_arm_type1_state )
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x4effff) AM_ROMBANK("bank1") /* Game ROM */
	AM_RANGE(0x4f0000, 0x4f003f) AM_READWRITE(pgm_arm7_type1_ram_r, pgm_arm7_type1_ram_w) /* ARM7 Shared RAM */
	AM_RANGE(0x500000, 0x500005) AM_READWRITE(pgm_arm7_type1_68k_protlatch_r, pgm_arm7_type1_68k_protlatch_w) /* ARM7 Latch */
ADDRESS_MAP_END

static ADDRESS_MAP_START( 55857E_arm7_map, AS_PROGRAM, 32, pgm_arm_type1_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08100000, 0x083fffff) AM_READ(pgm_arm7_type1_exrom_r) // unpopulated, returns 0 to keep checksum happy
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM // internal ram for asic
	AM_RANGE(0x40000000, 0x40000003) AM_READWRITE(pgm_arm7_type1_protlatch_r, pgm_arm7_type1_protlatch_w)
	AM_RANGE(0x40000008, 0x4000000b) AM_WRITENOP // ?
	AM_RANGE(0x4000000c, 0x4000000f) AM_READ(pgm_arm7_type1_unk_r)
	AM_RANGE(0x50800000, 0x5080003f) AM_READWRITE(pgm_arm7_type1_shareram_r, pgm_arm7_type1_shareram_w) AM_SHARE("arm7_shareram")
	AM_RANGE(0x50000000, 0x500003ff) AM_RAM // uploads xor table to decrypt 68k rom here
ADDRESS_MAP_END




/**************************** SIMULATIONS *****************************/

static ADDRESS_MAP_START( kov_sim_map, AS_PROGRAM, 16, pgm_arm_type1_state )
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x4effff) AM_ROMBANK("bank1") /* Game ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( cavepgm_mem, AS_PROGRAM, 16, pgm_arm_type1_state )
	AM_IMPORT_FROM(pgm_base_mem)
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	/* protection devices installed (simulated) later */
ADDRESS_MAP_END


MACHINE_START_MEMBER(pgm_arm_type1_state,pgm_arm_type1)
{
	MACHINE_START_CALL_MEMBER(pgm);
	save_item(NAME(m_value0));
	save_item(NAME(m_value1));
	save_item(NAME(m_valuekey));
	save_item(NAME(m_valueresponse));
	save_item(NAME(m_curslots));
	save_item(NAME(m_slots));
}

MACHINE_CONFIG_START( pgm_arm_type1_cave, pgm_arm_type1_state )
	MCFG_FRAGMENT_ADD(pgmbase)

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(cavepgm_mem)

	MCFG_MACHINE_START_OVERRIDE(pgm_arm_type1_state, pgm_arm_type1 )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(59.17) // verified on pcb
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( pgm_arm_type1_sim, pgm_arm_type1_cave )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kov_sim_map)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000 )   // 55857E?
	MCFG_CPU_PROGRAM_MAP(55857E_arm7_map)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( pgm_arm_type1, pgm_arm_type1_cave )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kov_map)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)    // 55857E?
	MCFG_CPU_PROGRAM_MAP(55857E_arm7_map)
MACHINE_CONFIG_END

void pgm_arm_type1_state::pgm_arm7_type1_latch_init()
{
	m_pgm_arm_type1_highlatch_arm_w = 0;
	m_pgm_arm_type1_lowlatch_arm_w = 0;
	m_pgm_arm_type1_highlatch_68k_w = 0;
	m_pgm_arm_type1_lowlatch_68k_w = 0;
	m_pgm_arm_type1_counter = 1;

	save_item(NAME(m_pgm_arm_type1_highlatch_arm_w));
	save_item(NAME(m_pgm_arm_type1_lowlatch_arm_w));
	save_item(NAME(m_pgm_arm_type1_highlatch_68k_w));
	save_item(NAME(m_pgm_arm_type1_lowlatch_68k_w));
	save_item(NAME(m_pgm_arm_type1_counter));
}

READ16_MEMBER(pgm_arm_type1_state::kovsh_fake_region_r )
{
	int regionhack = ioport("RegionHack")->read();
	if (regionhack != 0xff) return regionhack;

	offset = 0x4;
	UINT16 *share16 = reinterpret_cast<UINT16 *>(m_arm7_shareram.target());
	return share16[BYTE_XOR_LE(offset << 1)];
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,photoy2k)
{
	pgm_basic_init();
	pgm_photoy2k_decrypt(machine());
	pgm_arm7_type1_latch_init();
	/* we only have a china internal ROM dumped for now.. allow region to be changed for debugging (to ensure all alt titles / regions can be seen) */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0008, 0x4f0009, read16_delegate(FUNC(pgm_arm_type1_state::kovsh_fake_region_r),this));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovsh)
{
	pgm_basic_init();
	pgm_kovsh_decrypt(machine());
	pgm_arm7_type1_latch_init();
	/* we only have a china internal ROM dumped for now.. allow region to be changed for debugging (to ensure all alt titles / regions can be seen) */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0008, 0x4f0009, read16_delegate(FUNC(pgm_arm_type1_state::kovsh_fake_region_r),this));
}

/* Fake remapping of ASIC commands to the ones used by KOVSH due to the lack of the real ARM rom for this set */
WRITE16_MEMBER(pgm_arm_type1_state::kovshp_asic27a_write_word )
{
	switch (offset)
	{
		case 0:
			m_pgm_arm_type1_lowlatch_68k_w = data;
		return;

		case 1:
		{
			unsigned char asic_key = data >> 8;
			unsigned char asic_cmd = (data & 0xff) ^ asic_key;

			switch (asic_cmd)
			{
				case 0x9a: asic_cmd = 0x99; break; // kovshxas

				case 0x38: asic_cmd = 0xad; break;
				case 0x43: asic_cmd = 0xca; break;
				case 0x56: asic_cmd = 0xac; break;
				case 0x73: asic_cmd = 0x93; break;
				case 0x84: asic_cmd = 0xb3; break;
				case 0x87: asic_cmd = 0xb1; break;
				case 0x89: asic_cmd = 0xb6; break;
				case 0x93: asic_cmd = 0x73; break;
				case 0xa5: asic_cmd = 0xa9; break;
				case 0xac: asic_cmd = 0x56; break;
				case 0xad: asic_cmd = 0x38; break;
				case 0xb1: asic_cmd = 0x87; break;
				case 0xb3: asic_cmd = 0x84; break;
				case 0xb4: asic_cmd = 0x90; break;
				case 0xb6: asic_cmd = 0x89; break;
				case 0xc5: asic_cmd = 0x8c; break;
				case 0xca: asic_cmd = 0x43; break;
				case 0xcc: asic_cmd = 0xf0; break;
				case 0xd0: asic_cmd = 0xe0; break;
				case 0xe0: asic_cmd = 0xd0; break;
				case 0xe7: asic_cmd = 0x70; break;
				case 0xed: asic_cmd = 0xcb; break;
				case 0xf0: asic_cmd = 0xcc; break;
				case 0xf1: asic_cmd = 0xf5; break;
				case 0xf2: asic_cmd = 0xf1; break;
				case 0xf4: asic_cmd = 0xf2; break;
				case 0xf5: asic_cmd = 0xf4; break;
				case 0xfc: asic_cmd = 0xc0; break;
				case 0xfe: asic_cmd = 0xc3; break;

				case 0xa6: asic_cmd = 0xa9; break;
				case 0xaa: asic_cmd = 0x56; break;
				case 0xf8: asic_cmd = 0xf3; break;
			}

			m_pgm_arm_type1_highlatch_68k_w = asic_cmd ^ (asic_key | (asic_key << 8));
		}
		return;
	}
}


DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovshp)
{
	pgm_basic_init();
	pgm_kovshp_decrypt(machine());
	pgm_arm7_type1_latch_init();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0008, 0x4f0009, read16_delegate(FUNC(pgm_arm_type1_state::kovsh_fake_region_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x500000, 0x500005, write16_delegate(FUNC(pgm_arm_type1_state::kovshp_asic27a_write_word),this));
}



/* bootleg inits */

DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovshxas)
{
	pgm_basic_init();
//  pgm_kovshp_decrypt(machine());
	pgm_arm7_type1_latch_init();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0008, 0x4f0009, read16_delegate(FUNC(pgm_arm_type1_state::kovsh_fake_region_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x500000, 0x500005, write16_delegate(FUNC(pgm_arm_type1_state::kovshp_asic27a_write_word),this));
}

void pgm_arm_type1_state::pgm_decode_kovlsqh2_tiles()
{
	int i, j;
	UINT16 *src = (UINT16 *)(memregion("tiles")->base() + 0x180000);
	std::vector<UINT16> dst(0x800000);

	for (i = 0; i < 0x800000 / 2; i++)
	{
		j = BITSWAP24(i, 23, 22, 9, 8, 21, 18, 0, 1, 2, 3, 16, 15, 14, 13, 12, 11, 10, 19, 20, 17, 7, 6, 5, 4);

		dst[j] = BITSWAP16(src[i], 1, 14, 8, 7, 0, 15, 6, 9, 13, 2, 5, 10, 12, 3, 4, 11);
	}

	memcpy( src, &dst[0], 0x800000 );
}

void pgm_arm_type1_state::pgm_decode_kovlsqh2_sprites( UINT8 *src )
{
	int i, j;
	dynamic_buffer dst(0x800000);

	for (i = 0; i < 0x800000; i++)
	{
		j = BITSWAP24(i, 23, 10, 9, 22, 19, 18, 20, 21, 17, 16, 15, 14, 13, 12, 11, 8, 7, 6, 5, 4, 3, 2, 1, 0);

		dst[j] = src[i];
	}

	memcpy( src, &dst[0], 0x800000 );
}

void pgm_arm_type1_state::pgm_decode_kovlsqh2_samples()
{
	int i;
	UINT8 *src = (UINT8 *)(memregion("ics")->base() + 0x400000);

	for (i = 0; i < 0x400000; i+=2) {
		src[i + 0x000001] = src[i + 0x400001];
	}

	memcpy( src + 0x400000, src, 0x400000 );
}

void pgm_arm_type1_state::pgm_decode_kovqhsgs_program()
{
	int i;
	UINT16 *src = (UINT16 *)(memregion("maincpu")->base() + 0x100000);
	std::vector<UINT16> dst(0x400000);

	for (i = 0; i < 0x400000 / 2; i++)
	{
		int j = BITSWAP24(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 6, 7, 5, 4, 3, 2, 1, 0);

		dst[j] = BITSWAP16(src[i], 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 4, 5, 3, 2, 1, 0);
	}

	memcpy( src, &dst[0], 0x400000 );
}

void pgm_arm_type1_state::pgm_decode_kovqhsgs2_program()
{
	int i;
	UINT16 *src = (UINT16 *)(memregion("maincpu")->base() + 0x100000);
	std::vector<UINT16> dst(0x400000);

	for (i = 0; i < 0x400000 / 2; i++)
	{
		int j = BITSWAP24(i, 23, 22, 21, 20, 19, 16, 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 18, 17, 7);

		dst[j] = src[i];
	}

	memcpy( src, &dst[0], 0x400000 );
}


DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovlsqh2)
{
	pgm_decode_kovqhsgs2_program();
	pgm_decode_kovlsqh2_tiles();

	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x0800000);
	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x1000000);
	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x1800000);
	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x2000000);
	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x2800000);
	pgm_decode_kovlsqh2_sprites(memregion("sprmask")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(memregion("sprmask")->base() + 0x0800000);

	pgm_decode_kovlsqh2_samples();
	pgm_basic_init();
	pgm_arm7_type1_latch_init();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0008, 0x4f0009, read16_delegate(FUNC(pgm_arm_type1_state::kovsh_fake_region_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x500000, 0x500005, write16_delegate(FUNC(pgm_arm_type1_state::kovshp_asic27a_write_word),this));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovqhsgs)
{
	pgm_decode_kovqhsgs_program();
	pgm_decode_kovlsqh2_tiles();

	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x0800000);
	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x1000000);
	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x1800000);
	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x2000000);
	pgm_decode_kovlsqh2_sprites(memregion("sprcol")->base() + 0x2800000);
	pgm_decode_kovlsqh2_sprites(memregion("sprmask")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(memregion("sprmask")->base() + 0x0800000);

	pgm_decode_kovlsqh2_samples();
	pgm_basic_init();
	pgm_arm7_type1_latch_init();
	/* we only have a china internal ROM dumped for now.. allow region to be changed for debugging (to ensure all alt titles / regions can be seen) */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0008, 0x4f0009, read16_delegate(FUNC(pgm_arm_type1_state::kovsh_fake_region_r),this));
}

/*
 in Ketsui (ket) @ 000A719C (move.w)

 if you change D0 to 0x12
 the game will runs to "Asic27 Test" mode

 bp A71A0,1,{d0=0x12;g}
*/

READ16_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_sim_r )
{
	if (offset == 0)
	{
		UINT16 d = m_valueresponse & 0xffff;
		UINT16 realkey = m_valuekey >> 8;
		realkey |= m_valuekey;
		d ^= realkey;

		return d;

	}
	else if (offset == 1)
	{
		UINT16 d = m_valueresponse >> 16;
		UINT16 realkey = m_valuekey >> 8;
		realkey |= m_valuekey;
		d ^= realkey;
		return d;

	}
	return 0xffff;
}

/* working */
void pgm_arm_type1_state::command_handler_ddp3(int pc)
{
	switch (m_ddp3lastcommand)
	{
		default:
			printf("%06x command %02x | %04x\n", pc, m_ddp3lastcommand, m_value0);
			m_valueresponse = 0x880000;
			break;

		case 0x40:
			m_valueresponse = 0x880000;
			m_slots[(m_value0>>10)&0x1F]=
				(m_slots[(m_value0>>5)&0x1F]+
					m_slots[(m_value0>>0)&0x1F])&0xffffff;
			break;

		case 0x67: // set high bits
	//      printf("%06x command %02x | %04x\n", space.device().safe_pc(), m_ddp3lastcommand, m_value0);
			m_valueresponse = 0x880000;
			m_curslots = (m_value0 & 0xff00)>>8;
			m_slots[m_curslots] = (m_value0 & 0x00ff) << 16;
			break;

		case 0xe5: // set low bits for operation?
		//  printf("%06x command %02x | %04x\n", space.device().safe_pc(), m_ddp3lastcommand, m_value0);
			m_valueresponse = 0x880000;
			m_slots[m_curslots] |= (m_value0 & 0xffff);
			break;


		case 0x8e: // read back result of operations
	//      printf("%06x command %02x | %04x\n", space.device().safe_pc(), m_ddp3lastcommand, m_value0);
			m_valueresponse = m_slots[m_value0&0xff];
			break;


		case 0x99: // reset?
			m_simregion = 0;//ioport("Region")->read();
			m_valuekey = 0x100;
			m_valueresponse = 0x00880000 | m_simregion << 8;
			break;

	}
}

/* preliminary */

// should be correct, note each value only appears once
UINT8 puzzli2_level_decode[256] = {
	// 0  ,  1  ,  2  ,  3  ,  4  ,  5   , 6  ,  7  ,  8  ,  9  ,  a  ,  b  ,  c  ,  d  ,  e  ,  f  ,
	0x32, 0x3e, 0xb2, 0x37, 0x31, 0x22, 0xd6, 0x0d, 0x35, 0x5c, 0x8d, 0x3c, 0x7a, 0x5f, 0xd7, 0xac, // 0x0
//   0  ,  0  ,  0  ,  0  ,  0  ,  1  ,  1  ,  0  ,  1  ,  1  ,  0  ,  0  ,  0  ,  0  ,  x  ,  x  ,
	0x53, 0xff, 0xeb, 0x44, 0xe8, 0x11, 0x69, 0x77, 0xd9, 0x34, 0x36, 0x45, 0xa6, 0xe9, 0x1c, 0xc6, // 0x1
//   0  ,  0  ,  x  ,  x  ,  x  ,  0  ,  x  ,  0  ,  x  ,  0  ,  0  ,  0  ,  0  ,  x  ,  0  ,  x  ,
	0x3b, 0xbd, 0xad, 0x2e, 0x18, 0xdf, 0xa1, 0xab, 0xdd, 0x52, 0x57, 0xc2, 0xe5, 0x0a, 0x00, 0x6d, // 0x2
//   0  ,  0  ,  0  ,  1  ,  1  ,  1  ,  1  ,  x  ,  1  ,  1  ,  0  ,  0  ,  1  ,  1  ,  x  ,  0  ,
	0x67, 0x64, 0x15, 0x70, 0xb6, 0x39, 0x27, 0x78, 0x82, 0xd2, 0x71, 0xb9, 0x13, 0xf5, 0x93, 0x92, // 0x3
//   0  ,  x  ,  1  ,  1  ,  x  ,  1  ,  1  ,  1  ,  1  ,  1  ,  1  ,  1  ,  x  ,  0  ,  x  ,  x  ,
	0xfa, 0xe7, 0x5e, 0xb0, 0xf6, 0xaf, 0x95, 0x8a, 0x7c, 0x73, 0xf9, 0x63, 0x86, 0xcb, 0x1a, 0x56, // 0x4
//   0  ,  1  ,  1  ,  0  ,  0  ,  0  ,  0  ,  1  ,  1  ,  1  ,  1  ,  0  ,  1  ,  1  ,  1  ,  0  ,
	0xf1, 0x3a, 0xae, 0x61, 0x01, 0x29, 0x97, 0x23, 0x8e, 0x5d, 0x9a, 0x65, 0x74, 0x21, 0x20, 0x40, // 0x5
//   0  ,  1  ,  1  ,  1  ,  1  ,  0  ,  x  ,  x  ,  x  ,  0  ,  0  ,  1  ,  1  ,  1  ,  1  ,  1  ,
	0xd3, 0x05, 0xa2, 0xe1, 0xbc, 0x9e, 0x1e, 0x10, 0x14, 0x0c, 0x88, 0x9c, 0xec, 0x38, 0xb5, 0x9d, // 0x6
//   1  ,  0  ,  0  ,  x  ,  1  ,  1  ,  0  ,  0  ,  x  ,  0  ,  x  ,  0  ,  0  ,  1  ,  1  ,  1  ,
	0x2d, 0xf7, 0x17, 0x0e, 0x84, 0xc7, 0x7d, 0xce, 0x94, 0x16, 0x48, 0xa8, 0x81, 0x6e, 0x7b, 0xd8, // 0x7
//   1  ,  1  ,  1  ,  1  ,  x  ,  0  ,  x  ,  0  ,  1  ,  1  ,  1  ,  x  ,  x  ,  1  ,  1  ,  1  ,
	0xa7, 0x7f, 0x42, 0xe6, 0xa0, 0x2a, 0xef, 0xee, 0x24, 0xba, 0xb8, 0x7e, 0xc9, 0x2b, 0x90, 0xcc, // 0x8
//   1  ,  x  ,  1  ,  1  ,  1  ,  1  ,  1  ,  1  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,
	0x5b, 0xd1, 0xf3, 0xe2, 0x6f, 0xed, 0x9f, 0xf0, 0x4b, 0x54, 0x8c, 0x08, 0xf8, 0x51, 0x68, 0xc8, // 0x9
//   x  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  x  ,  0  ,  x  ,  0  ,  0  ,  0  ,  0  ,  1  ,
	0x03, 0x0b, 0xbb, 0xc1, 0xe3, 0x4d, 0x04, 0xc5, 0x8f, 0x09, 0x0f, 0xbf, 0x62, 0x49, 0x76, 0x59, // 0xa
//   1  ,  1  ,  1  ,  1  ,  1  ,  x  ,  0  ,  1  ,  1  ,  0  ,  x  ,  1  ,  1  ,  1  ,  1  ,  0  ,
	0x1d, 0x80, 0xde, 0x60, 0x07, 0xe0, 0x1b, 0x66, 0xa5, 0xbe, 0xcd, 0x87, 0xdc, 0xc3, 0x6b, 0x4e, // 0xb
//   0  ,  1  ,  1  ,  1  ,  1  ,  x  ,  0  ,  x  ,  0  ,  x  ,  0  ,  1  ,  1  ,  0  ,  1  ,  1  ,
	0xd0, 0xfd, 0xd4, 0x3f, 0x98, 0x96, 0x2f, 0x4c, 0xb3, 0xea, 0x2c, 0x75, 0xe4, 0xc0, 0x6c, 0x6a, // 0xc
//   0  ,  x  ,  1  ,  1  ,  0  ,  1  ,  1  ,  1  ,  1  ,  0  ,  1  ,  1  ,  x  ,  1  ,  1  ,  1  ,
	0x9b, 0xb7, 0x43, 0x8b, 0x41, 0x47, 0x02, 0xdb, 0x99, 0x3d, 0xa3, 0x79, 0x50, 0x4f, 0xb4, 0x55, // 0xd
//   1  ,  0  ,  0  ,  0  ,  1  ,  0  ,  0  ,  x  ,  x  ,  1  ,  1  ,  1  ,  0  ,  1  ,  1  ,  1  ,
	0x5a, 0x25, 0xf4, 0xca, 0x58, 0x30, 0xc4, 0x12, 0xa9, 0x46, 0xda, 0x91, 0xa4, 0xaa, 0xfc, 0x85, // 0xe
//   1  ,  1  ,  0  ,  1  ,  1  ,  1  ,  1  ,  0  ,  0  ,  1  ,  1  ,  1  ,  1  ,  0  ,  0  ,  x  ,
	0xfb, 0x89, 0x06, 0xcf, 0xfe, 0x33, 0xd5, 0x28, 0x1f, 0x19, 0x4a, 0xb1, 0x83, 0xf2, 0x72, 0x26, // 0xf
//   x  ,  x  ,  1  ,  1  ,  1  ,  1  ,  1  ,  1  ,  x  ,  0  ,  1  ,  1  ,  1  ,  1  ,  1  ,  1  ,
};


#define puzzli2_printf logerror

void pgm_arm_type1_state::command_handler_puzzli2(int pc)
{
	switch (m_ddp3lastcommand)
	{
		case 0x31:
		{
			// how is this selected? command 54?


			/* writes the following sequence before how to play
			 each level has a different sequence written before it, size of sequence doesn't seem directly connected to level size (unlike the reads)
			 so it's probably compressed somehow as well as scrambled?  68k doesnt know in advance how big each lot of data is either, it only stops
			 writing when it gets a difference response from the MCU.


			00138278: 31 00fd | (set xor table offset)
		UNKNOWN - related to depth / number of columns?
			00138278: 31 0087 | value 87, after xor is 75 (table address,value fd,f2)
		COLUMN 1
			00138278: 31 0032 | value 32, after xor is 40 (table address,value fe,72) << 4 is the number of entries in this column
			00138278: 31 0029 | value 29, after xor is 0f (table address,value ff,26) << 0x0f is a mask of 4 bits..

			00138278: 31 0031 | value 31, after xor is 03 (table address,value 00,32)  -> 0x0103
			00138278: 31 003f | value 3f, after xor is 01 (table address,value 01,3e)  -> 0x0101
			00138278: 31 00b0 | value b0, after xor is 02 (table address,value 02,b2)  -> 0x0102
			00138278: 31 0035 | value 35, after xor is 02 (table address,value 03,37)  -> 0x0102
		COLUMN 2
			00138278: 31 0071 | value 71, after xor is 40 (table address,value 04,31) << 4 is the number of entries in this column
			00138278: 31 002d | value 2d, after xor is 0f (table address,value 05,22) << 0x0f is a mask of 4 bits..

			00138278: 31 00d5 | value d5, after xor is 03 (table address,value 06,d6)  -> 0x0103
			00138278: 31 000d | value 0d, after xor is 00 (table address,value 07,0d)  -> 0x0100
			00138278: 31 0034 | value 34, after xor is 01 (table address,value 08,35)  -> 0x0101
			00138278: 31 0059 | value 59, after xor is 05 (table address,value 09,5c)  -> 0x0105
		COLUMN 3
			00138278: 31 00dd | value dd, after xor is 50 (table address,value 0a,8d) << 5 is the number of entries in this column
			00138278: 31 0023 | value 23, after xor is 1f (table address,value 0b,3c) << 0x1f is a mask of 5 bits..

			00138278: 31 007a | value 7a, after xor is 00 (table address,value 0c,7a)  -> 0x0100
			00138278: 31 00f3 | value f3, after xor is 01 (table address,value fd,f2)  -> 0x0101
			00138278: 31 0077 | value 77, after xor is 05 (table address,value fe,72)  -> 0x0105
			00138278: 31 0022 | value 22, after xor is 04 (table address,value ff,26)  -> 0x0104
			00138278: 31 0036 | value 36, after xor is 04 (table address,value 00,32)  -> 0x0104
		COLUMN 4
			00138278: 31 002e | value 2e, after xor is 10 (table address,value 01,3e) << 1 is the number of entries in this column
			00138278: 31 00b3 | value b3, after xor is 01 (table address,value 02,b2) << 0x01 is a mask of 1 bit..

			00138278: 31 0035 | value 35, after xor is 02 (table address,value 03,37)  -> 0x0102
		COLUMN 5
			00138278: 31 0041 | value 41, after xor is 70 (table address,value 04,31) << 7 is the number of entries in this column
			00138278: 31 005d | value 5d, after xor is 7f (table address,value 05,22) << 0x7f is a mask of 7 bits..

			00138278: 31 00d6 | value d6, after xor is 00 (table address,value 06,d6)  -> 0x0100
			00138278: 31 000c | value 0c, after xor is 01 (table address,value 07,0d)  -> 0x0101
			00138278: 31 0036 | value 36, after xor is 03 (table address,value 08,35)  -> 0x0103
			00138278: 31 005e | value 5e, after xor is 02 (table address,value 09,5c)  -> 0x0102
			00138278: 31 0089 | value 89, after xor is 04 (table address,value 0a,8d)  -> 0x0104
			00138278: 31 003c | value 3c, after xor is 00 (table address,value 0b,3c)  -> 0x0100
			00138278: 31 007a | value 7a, after xor is 00 (table address,value 0c,7a)  -> 0x0100
		COLUMN 6
			00138278: 31 00a2 | value a2, after xor is 50 (table address,value fd,f2) << 5 is the number of entries in this column
			00138278: 31 006d | value 6d, after xor is 1f (table address,value fe,72) << 0x1f is a mask of 5 bits..

			00138278: 31 0023 | value 23, after xor is 05 (table address,value ff,26)  -> 0x0105
			00138278: 31 0037 | value 37, after xor is 05 (table address,value 00,32)  -> 0x0105
			00138278: 31 003f | value 3f, after xor is 01 (table address,value 01,3e)  -> 0x0101
			00138278: 31 00b3 | value b3, after xor is 01 (table address,value 02,b2)  -> 0x0101
			00138278: 31 0034 | value 34, after xor is 03 (table address,value 03,37)  -> 0x0103
			 ^ (end, returning 630006 as playfield width)

			*/

			if (command_31_write_type==2)
			{
				puzzli2_printf("%08x: %02x %04x | ",pc, m_ddp3lastcommand, m_value0);

				// this shouldn't apply to the stuff written on startup, only the level data..

				if (hackcount2==0)
				{
					puzzli2_take_leveldata_value(m_value0&0xff);

					hack_31_table_offset = m_value0 & 0xff;
					hack_31_table_offset2 = 0;
					hackcount2++;
					m_valueresponse = 0x00d20000;

					//puzzli2_printf("(set xor table offset)\n");
				}
				else  // how do we decide end?
				{
					int end = puzzli2_take_leveldata_value(m_value0&0xff);

					if (!end)
					{
						// always d2 0000 when writing doing level data
						// but different for the writes on startup?
						m_valueresponse = 0x00d20000;

						//UINT8 tableaddr = (hack_31_table_offset + (hack_31_table_offset2&0xf))&0xff;
						//UINT8 xoredval = m_value0 ^ puzzli2_level_decode[tableaddr];
						//puzzli2_printf("value %02x, after xor is %02x (table address,value %02x,%02x)\n", m_value0, xoredval, tableaddr, puzzli2_level_decode[tableaddr]);

						hackcount2++;
						hack_31_table_offset2++;
					}
					else
					{
						hackcount2=0;

						// when the ARM detects the end of the stream has been reached it returns a 0x63 status with the number of columns in the data word
						m_valueresponse = 0x00630000 | numbercolumns;

						//UINT8 tableaddr = (hack_31_table_offset + (hack_31_table_offset2&0xf))&0xff;
						//UINT8 xoredval = m_value0 ^ puzzli2_level_decode[tableaddr];
						//puzzli2_printf("value %02x, after xor is %02x (table address,value %02x,%02x) (end, returning %02x as playfield width)\n", m_value0, xoredval, tableaddr, puzzli2_level_decode[tableaddr], m_valueresponse);


					}
				}


			}
			else
			{
				// todo, responses when uploading the startup values are different
				puzzli2_printf("%08x: %02x %04x (for z80 address?)\n ",pc, m_ddp3lastcommand, m_value0);

				m_valueresponse = 0x00d20000 | p2_31_retcounter;
				p2_31_retcounter++; // returns 0xc for the first one, 0x19 for the last one
			}

		}
		break;


		// after writing the compressed and scrambled data stream for the level (copied from ROM) with command 0x31
		// the game expects to read back a fully formed level structure from the ARM
		case 0x13:
		{
			puzzli2_printf("%08x: %02x %04x (READ LEVEL DATA) | ",pc, m_ddp3lastcommand, m_value0);

			// this is the how to play screen, correctly returned with current code
			/*
			UINT16 retvals[61] =
			{ 0x0008, // depth (-2?)
			  0x0103, 0x0101, 0x0102, 0x0102, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // first column
			  0x0103, 0x0100, 0x0101, 0x0105, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			  0x0100, 0x0101, 0x0105, 0x0104, 0x0104, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			  0x0102, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
			  0x0100, 0x0101, 0x0103, 0x0102, 0x0104, 0x0100 ,0x0100, 0x0000, 0x0000, 0x0000,
			  0x0105, 0x0105, 0x0101, 0x0101, 0x0103, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000  // last column
			};
			*/


			UINT16* leveldata = &level_structure[0][0];
			if (hackcount==0)
			{
				m_valueresponse = 0x002d0000 | ((depth>>4)+1); // this *seems* to come from upper bits of the first real value written to the device during the level stream (verify, seems wrong for some levels because you get a black bar on the bottom of the screen, but might be bad xors)
				puzzli2_printf("level depth returning %08x\n", m_valueresponse );
			}
			else if (hackcount<((10*numbercolumns)+1))
			{
				m_valueresponse = 0x002d0000 | leveldata[hackcount-1];
				puzzli2_printf("level data returning %08x\n", m_valueresponse );
			}
			else
			{
				hackcount=0;
				m_valueresponse = 0x00740054;  // 0x0074 0054 is returned after how to play reads above.. where does 0x54 come from?
				puzzli2_printf("END returning %08x\n", m_valueresponse );

			}

			hackcount++;


			// 2d seems to be used when there is more data available
			// 74 seems to be used when there isn't.. (end of buffer reached?)
			// 2d or 74! (based on?)

		}
		break;



		case 0x38: // Reset
			puzzli2_printf("%08x: %02x %04x (RESET)\n",pc, m_ddp3lastcommand, m_value0);
			m_simregion = ioport("Region")->read();
			m_valueresponse = 0x780000 | m_simregion<<8; // this must also return the cart region or the game will act in odd ways when inserting a coin on continue, or during the game on later levels
			m_valuekey = 0x100;
			m_puzzli_54_trigger = 0;

		break;




		// 47 and 52 are used to get the images during the intro sequence, different each loop
		// also some other gfx?
		// logic here seems correct, not sure where the 0x19 and 0x5 etc. come from tho!
		case 0x47:
			puzzli2_printf("%08x: %02x %04x (GFX OFF PART 1)\n",pc, m_ddp3lastcommand, m_value0);

			hack_47_value = m_value0;

			//hack_47_value = ((m_value0 & 0x0700)>>8) * 0x19;
			//hack_47_value +=((m_value0 & 0x0007)>>0) * 0x05;
			if (m_value0 & 0xf0f0) puzzli2_printf("unhandled 0x47 bits %04x\n", m_value0);

			//puzzli2_printf("0x47 value was %04x - using %04x\n", m_value0, hack_47_value);

			m_valueresponse = 0x00740047;

		break;

		case 0x52:
			puzzli2_printf("%08x: %02x %04x (GFX OFF PART 2)\n",pc, m_ddp3lastcommand, m_value0);

			//puzzli2_printf("which %04x\n", m_value0);
			if (m_value0 & 0xfff0) puzzli2_printf("unhandled 0x52 bits %04x\n", m_value0);

			// it writes a value of 0x0000 then expects to read back like
			// this for the lower part of the game backgrounds
			if (m_value0==0x0000)
			{
				int val = ((hack_47_value & 0x0f00)>>8) * 0x19;
				m_valueresponse = 0x00740000 | (val & 0xffff);
			}
			else
			{
				int val = ((hack_47_value & 0x0f00)>>8) * 0x19;
				val +=((hack_47_value & 0x000f)>>0) * 0x05;
				val += m_value0 & 0x000f;
				m_valueresponse = 0x00740000 | (val & 0xffff);

			}



		break;



		case 0x61: // ??
			puzzli2_printf("%08x: %02x %04x\n",pc, m_ddp3lastcommand, m_value0);

			// this command is written before the values used to decrypt the z80 addresses (assumed) are uploaded with command 31
			command_31_write_type = 1;

			m_valueresponse = 0x36<<16;
			p2_31_retcounter = 0xc;
		break;

		case 0x41: // ASIC status?
			puzzli2_printf("%08x: %02x %04x (UNK)\n",pc, m_ddp3lastcommand, m_value0);

			// this command is written after the values used to decrypt the z80 addresses (assumed) are uploaded with command 31
			command_31_write_type = 0;

			//m_valueresponse = 0x74<<16;
			m_valueresponse = 0x740061;
		break;

		case 0x54: // ??
			puzzli2_printf("%08x: %02x %04x\n",pc, m_ddp3lastcommand, m_value0);

			// this command is written before uploading the compressed level data stream with command 31

			command_31_write_type = 2;
			stage = -1;
			m_puzzli_54_trigger = 1;
			hackcount2 = 0;
			hackcount = 0;
			m_valueresponse = 0x36<<16;

			//  clear the return structure
			for (auto & elem : level_structure)
				for (int rows=0;rows<10;rows++)
					elem[rows] = 0x0000;

		break;

	/*

	puzzli2 on startup (00148a84)         puzzli2s on startup (0014cf58)

	    (001489f6: 61 0202  0014ceca: 61 0202)
	                                           always
	    : 31 004e a6f7 | : 31 | 0051 14c9       + 26DD2
	    : 31 279e 534f | : 31 | 27a0 c121
	    : 31 ab5c a7cf | : 31 | ab5f 15a1
	    : 31 145f 7054 | : 31 | 1461 de26
	    : 31 85a0 7b7f | : 31 | 85a2 e951
	    : 31 7003 c5ab | : 31 | 7006 337d
	    : 31 456d f3aa | : 31 | 4570 617c

	    (00148b34: 41 e2bb  0014d008: 41 706d)

	    actual values needed       always
	    0x001694a8 / 0x0019027a  + 26DD2
	    0x0016cfae / 0x00193D80
	    0x0016ebf2 / 0x001959c4
	    0x0016faa8 / 0x0019687a
	    0x00174416 / 0x0019b1e8

	    0x00166178 / 0x0018cf4a
	    0x00166e72 / 0x0018dc44

	     as you can see the difference between the values written is always 26dd2, as is the difference between offsets expected
	     this makes it impossible to know which value is for which address without further tests!


	*/

		// I think the values returned here must be connected to the values written to command 31 on startup
		// 63/67 are used on startup to get the z80 music at least
		case 0x63: // used as a read address by the 68k code (related to previous uploaded values like cave?) should point at a table of ~0x80 in size? seems to use values as further pointers?
			puzzli2_printf("%08x: %02x %04x (Z80 ADDR PART 1)\n",pc, m_ddp3lastcommand, m_value0);

			if (!strcmp(machine().system().name,"puzzli2"))
			{
				if (m_value0==0x0000)
				{
					m_valueresponse = 0x001694a8;
				}
				else if (m_value0==0x0001)
				{
					m_valueresponse = 0x0016cfae;
				}
				else if (m_value0==0x0002)
				{
					m_valueresponse = 0x0016ebf2; // right for puzzli2 , wrong for puzzli2s, probably calculated from the writes then?
				}
				else if (m_value0==0x0003) // before 'cast' screen
				{
					m_valueresponse = 0x0016faa8;
				}
				else if (m_value0==0x0004) // 2 player demo
				{
					m_valueresponse = 0x00174416;
				}
				else
				{
					puzzli2_printf("unk case x63\n");
					m_valueresponse = 0x00600000; // wrong

				}
			}
			else // puzzli2 super
			{
				if (m_value0==0x0000)
				{
					m_valueresponse = 0x19027a;
				}
				else if (m_value0==0x0001)
				{
					m_valueresponse = 0x193D80;
				}
				else if (m_value0==0x0002)
				{
					m_valueresponse = 0x1959c4;
				}
				else if (m_value0==0x0003)
				{
					m_valueresponse = 0x19687a;
				}
				else if (m_value0==0x0004)
				{
					m_valueresponse = 0x19b1e8;
				}
				else
				{
					puzzli2_printf("unk case x63\n");
					m_valueresponse = 0x00600000; // wrong
				}
			}
		break;

		case 0x67: // used as a read address by the 68k code (related to previous uploaded values like cave?) directly reads ~0xDBE from the address..
			puzzli2_printf("%08x: %02x %04x (Z80 ADDR PART 2)\n",pc, m_ddp3lastcommand, m_value0);

			if (!strcmp(machine().system().name,"puzzli2"))
			{
				if ( (m_value0==0x0000) || (m_value0==0x0001) || (m_value0==0x0002) || (m_value0==0x0003) )
				{
					m_valueresponse = 0x00166178; // right for puzzli2 , wrong for puzzli2s, probably calculated from the writes then?
				}
				else if ( m_value0==0x0004 ) // 2 player demo
				{
					m_valueresponse = 0x00166e72;
				}
				else
				{
					puzzli2_printf("unk case x67\n");
					m_valueresponse = 0x00400000; // wrong
				}
			}
			else // puzzli2 super
			{
				if ((m_value0==0x0000) || (m_value0==0x0001) || (m_value0==0x0002) ||  (m_value0==0x0003))
				{
					m_valueresponse = 0x18cf4a;
				}
				else if ( m_value0==0x0004 ) // 2 player demo
				{
					m_valueresponse = 0x0018dc44;
				}
				else
				{
					puzzli2_printf("unk case x67\n");
					m_valueresponse = 0x00600000; // wrong
				}
			}
		break;

		default:
			puzzli2_printf("%08x: %02x %04x\n",pc, m_ddp3lastcommand, m_value0);

			m_valueresponse = 0x74<<16;
		break;
	}
}

/* preliminary */
void pgm_arm_type1_state::command_handler_py2k2(int pc)
{
	switch (m_ddp3lastcommand)
	{
		default:
			puzzli2_printf("%06x command %02x | %04x\n", pc, m_ddp3lastcommand, m_value0);
			m_valueresponse = 0x880000;
			break;

		case 0xc0:
			puzzli2_printf("%06x command %02x | %04x\n", pc, m_ddp3lastcommand, m_value0);
			m_valueresponse = 0x880000;
			break;

		case 0xcb: // Background layer 'x' select (pgm3in1, same as kov)
			m_valueresponse = 0x880000;
			m_kov_cb_value = m_value0;
		break;

		case 0xcc: // Background layer offset (pgm3in1, same as kov)
		{
			int y = m_value0;
			if (y & 0x400) y = -(0x400 - (y & 0x3ff));
			m_valueresponse = 0x900000 + ((m_kov_cb_value + (y * 0x40)) * 4);
		}
		break;

		case 0x99: // reset?
			m_simregion = ioport("Region")->read();
			m_valuekey = 0x100;
			m_valueresponse = 0x00880000 | m_simregion<<8;

			break;
	}
}

/* preliminary */


static const int pstar_ba[0x1E]={
	0x02,0x00,0x00,0x01,0x00,0x03,0x00,0x00, //0
	0x02,0x00,0x06,0x00,0x22,0x04,0x00,0x03, //8
	0x00,0x00,0x06,0x00,0x20,0x07,0x00,0x03, //10
	0x00,0x21,0x01,0x00,0x00,0x63
};

static const int pstar_b0[0x10]={
	0x09,0x0A,0x0B,0x00,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x00,0x00,0x00,0x00
};

static const int pstar_ae[0x10]={
	0x5D,0x86,0x8C ,0x8B,0xE0,0x8B,0x62,0xAF,
	0xB6,0xAF,0x10A,0xAF,0x00,0x00,0x00,0x00
};

static const int pstar_a0[0x10]={
	0x02,0x03,0x04,0x05,0x06,0x01,0x0A,0x0B,
	0x0C,0x0D,0x0E,0x09,0x00,0x00,0x00,0x00,
};

static const int pstar_9d[0x10]={
	0x05,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

static const int pstar_90[0x10]={
	0x0C,0x10,0x0E,0x0C,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
static const int pstar_8c[0x23]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,
	0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x04,
	0x03,0x03,0x03
};

static const int pstar_80[0x1a3]={
	0x03,0x03,0x04,0x04,0x04,0x04,0x05,0x05,
	0x05,0x05,0x06,0x06,0x03,0x03,0x04,0x04,
	0x05,0x05,0x05,0x05,0x06,0x06,0x07,0x07,
	0x03,0x03,0x04,0x04,0x05,0x05,0x05,0x05,
	0x06,0x06,0x07,0x07,0x06,0x06,0x06,0x06,
	0x06,0x06,0x06,0x07,0x07,0x07,0x07,0x07,
	0x06,0x06,0x06,0x06,0x06,0x06,0x07,0x07,
	0x07,0x07,0x08,0x08,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x06,0x06,0x06,0x07,0x07,
	0x06,0x06,0x06,0x07,0x07,0x07,0x08,0x08,
	0x09,0x09,0x09,0x09,0x07,0x07,0x07,0x07,
	0x07,0x08,0x08,0x08,0x08,0x09,0x09,0x09,
	0x06,0x06,0x07,0x07,0x07,0x08,0x08,0x08,
	0x08,0x08,0x09,0x09,0x05,0x05,0x06,0x06,
	0x06,0x07,0x07,0x08,0x08,0x08,0x08,0x09,
	0x07,0x07,0x07,0x07,0x07,0x08,0x08,0x08,
	0x08,0x09,0x09,0x09,0x06,0x06,0x07,0x03,
	0x07,0x06,0x07,0x07,0x08,0x07,0x05,0x04,
	0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,
	0x06,0x06,0x06,0x06,0x03,0x04,0x04,0x04,
	0x04,0x05,0x05,0x06,0x06,0x06,0x06,0x07,
	0x04,0x04,0x05,0x05,0x06,0x06,0x06,0x06,
	0x06,0x07,0x07,0x08,0x05,0x05,0x06,0x07,
	0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x05,0x05,0x05,0x07,0x07,0x07,0x07,0x07,
	0x07,0x08,0x08,0x08,0x08,0x08,0x09,0x09,
	0x09,0x09,0x03,0x04,0x04,0x05,0x05,0x05,
	0x06,0x06,0x07,0x07,0x07,0x07,0x08,0x08,
	0x08,0x09,0x09,0x09,0x03,0x04,0x05,0x05,
	0x04,0x03,0x04,0x04,0x04,0x05,0x05,0x04,
	0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
	0x03,0x03,0x03,0x04,0x04,0x04,0x04,0x04,
	0x04,0x04,0x04,0x04,0x04,0x03,0x03,0x03,
	0x03,0x03,0x03,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
	0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00
};

void pgm_arm_type1_state::command_handler_pstars(int pc)
{
	switch (m_ddp3lastcommand)
	{
		case 0x99:
			m_simregion = ioport("Region")->read();
			m_valuekey = 0x100;
			m_valueresponse = 0x00880000 | m_simregion<<8;

			break;

		case 0xe0:
			m_valueresponse = 0xa00000 + (m_value0 << 6);
			break;

		case 0xdc:
			m_valueresponse = 0xa00800 + (m_value0 << 6);
			break;

		case 0xd0:
			m_valueresponse = 0xa01000 + (m_value0 << 5);
			break;

		case 0xb1:
			m_pstar_b1_value = m_value0;
			m_valueresponse = 0x890000;
			break;

		case 0xbf:
			m_valueresponse = m_pstar_b1_value * m_value0;
			break;

		case 0xc1: //TODO:TIMER  0,1,2,FIX TO 0 should be OK?
			m_valueresponse = 0;
			break;

		case 0xce: //TODO:TIMER  0,1,2
			m_pstar_ce_value = m_value0;
			m_valueresponse=0x890000;
			break;

		case 0xcf: //TODO:TIMER  0,1,2
			m_extra_ram[m_pstar_ce_value] = m_value0;
			m_valueresponse = 0x890000;
			break;

		case 0xe7:
			m_pstar_e7_value = (m_value0 >> 12) & 0xf;
			m_slots[m_pstar_e7_value] &= 0xffff;
			m_slots[m_pstar_e7_value] |= (m_value0 & 0xff) << 16;
			m_valueresponse = 0x890000;
			break;

		case 0xe5:
			m_slots[m_pstar_e7_value] &= 0xff0000;
			m_slots[m_pstar_e7_value] |= m_value0;
			m_valueresponse = 0x890000;
			break;

		case 0xf8: //@73C
			m_valueresponse = m_slots[m_value0 & 0xf] & 0xffffff;
			break;

		case 0xba:
			m_valueresponse = pstar_ba[m_value0];
			break;

		case 0xb0:
			m_valueresponse = pstar_b0[m_value0];
			break;

		case 0xae:
			m_valueresponse = pstar_ae[m_value0];
			break;

		case 0xa0:
			m_valueresponse = pstar_a0[m_value0];
			break;

		case 0x9d:
			m_valueresponse = pstar_9d[m_value0];
			break;

		case 0x90:
			m_valueresponse = pstar_90[m_value0];
			break;

		case 0x8c:
			m_valueresponse = pstar_8c[m_value0];
			break;

		case 0x80:
			m_valueresponse = pstar_80[m_value0];
			break;

		default:
			m_valueresponse = 0x890000;
			logerror("PSTARS PC(%06x) UNKNOWN %4X %4X\n", pc, m_value1, m_value0);
	}
}

/* Old KOV and bootlegs sim ... really these should be read out... */

static const UINT8 kov_BATABLE[0x40] = {
	0x00,0x29,0x2c,0x35,0x3a,0x41,0x4a,0x4e,0x57,0x5e,0x77,0x79,0x7a,0x7b,0x7c,0x7d,
	0x7e,0x7f,0x80,0x81,0x82,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x90,
	0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9e,0xa3,0xd4,0xa9,0xaf,0xb5,0xbb,0xc1
};

static const UINT8 kov_B0TABLE[16] = { 2, 0, 1, 4, 3 }; // Maps char portraits to tables


void pgm_arm_type1_state::command_handler_kov(int pc)
{
	switch (m_ddp3lastcommand)
	{
		case 0x67: // unknown or status check?
		case 0x8e:
		case 0xa3:
		case 0x33: // kovsgqyz (a3)
		case 0x3a: // kovplus
		case 0xc5: // kovplus
			m_valueresponse = 0x880000;
		break;

		case 0x99: // Reset
			m_simregion = ioport("Region")->read();
			m_valueresponse = 0x880000 | m_simregion<<8;
			m_valuekey = 0x100;
		break;

		case 0x9d: // Sprite palette offset
			m_valueresponse = 0xa00000 + ((m_value0 & 0x1f) * 0x40);
		break;

		case 0xb0: // Read from data table
			m_valueresponse = kov_B0TABLE[m_value0 & 0x0f];
		break;

		case 0xb4: // Copy slot 'a' to slot 'b'
		case 0xb7: // kovsgqyz (b4)
		{
			m_valueresponse = 0x880000;

			if (m_value0 == 0x0102) m_value0 = 0x0100; // why?

			m_slots[(m_value0 >> 8) & 0x0f] = m_slots[(m_value0 >> 0) & 0x0f];
		}
		break;

		case 0xba: // Read from data table
			m_valueresponse = kov_BATABLE[m_value0 & 0x3f];
		break;

		case 0xc0: // Text layer 'x' select
			m_valueresponse = 0x880000;
			m_kov_c0_value = m_value0;
		break;

		case 0xc3: // Text layer offset
			m_valueresponse = 0x904000 + ((m_kov_c0_value + (m_value0 * 0x40)) * 4);
		break;

		case 0xcb: // Background layer 'x' select
			m_valueresponse = 0x880000;
			m_kov_cb_value = m_value0;
		break;

		case 0xcc: // Background layer offset
		{
			int y = m_value0;
			if (y & 0x400) y = -(0x400 - (y & 0x3ff));
			m_valueresponse = 0x900000 + ((m_kov_cb_value + (y * 0x40)) * 4);
		}
		break;

		case 0xd0: // Text palette offset
		case 0xcd: // kovsgqyz (d0)
			m_valueresponse = 0xa01000 + (m_value0 * 0x20);
		break;

		case 0xd6: // Copy slot to slot 0
			m_valueresponse = 0x880000;
			m_slots[0] = m_slots[m_value0 & 0x0f];
		break;

		case 0xdc: // Background palette offset
		case 0x11: // kovsgqyz (dc)
			m_valueresponse = 0xa00800 + (m_value0 * 0x40);
		break;

		case 0xe0: // Sprite palette offset
		case 0x9e: // kovsgqyz (e0)
			m_valueresponse = 0xa00000 + ((m_value0 & 0x1f) * 0x40);
		break;

		case 0xe5: // Write slot (low)
		{
			m_valueresponse = 0x880000;

			INT32 sel = (m_curslots >> 12) & 0x0f;
			m_slots[sel] = (m_slots[sel] & 0x00ff0000) | ((m_value0 & 0xffff) <<  0);
		}
		break;

		case 0xe7: // Write slot (and slot select) (high)
		{
			m_valueresponse = 0x880000;
			m_curslots = m_value0;

			INT32 sel = (m_curslots >> 12) & 0x0f;
			m_slots[sel] = (m_slots[sel] & 0x0000ffff) | ((m_value0 & 0x00ff) << 16);
		}
		break;

		case 0xf0: // Some sort of status read?
			m_valueresponse = 0x00c000;
		break;

		case 0xf8: // Read slot
		case 0xab: // kovsgqyz (f8)
			m_valueresponse = m_slots[m_value0 & 0x0f] & 0x00ffffff;
		break;

		case 0xfc: // Adjust damage level to char experience level
			m_valueresponse = (m_value0 * m_kov_fe_value) >> 6;
		break;

		case 0xfe: // Damage level adjust
			m_valueresponse = 0x880000;
			m_kov_fe_value = m_value0;
		break;

		default:
			m_valueresponse = 0x880000;
//                  logerror("Unknown ASIC27 command: %2.2x data: %4.4x\n", (data ^ m_valuekey) & 0xff, m_value0);
		break;
	}
}


/* Oriental Legend Super Plus ARM simulation */

static const int oldsplus_80[0x5]={
	0xbb8,0x1770,0x2328,0x2ee0,0xf4240
};

static const int oldsplus_fc[0x20]={
	0x00,0x00,0x0a,0x3a,0x4e,0x2e,0x03,0x40,
	0x33,0x43,0x26,0x2c,0x00,0x00,0x00,0x00,
	0x00,0x00,0x44,0x4d,0xb,0x27,0x3d,0x0f,
	0x37,0x2b,0x02,0x2f,0x15,0x45,0x0e,0x30
};

static const int oldsplus_a0[0x20]={
	0x000,0x023,0x046,0x069,0x08c,0x0af,0x0d2,0x0f5,
	0x118,0x13b,0x15e,0x181,0x1a4,0x1c7,0x1ea,0x20d,
	0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,
	0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,
};

static const int oldsplus_90[0x7]={
	0x50,0xa0,0xc8,0xf0,0x190,0x1f4,0x258
};

static const int oldsplus_5e[0x20]={
	0x04,0x04,0x04,0x04,0x04,0x03,0x03,0x03,
	0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const int oldsplus_b0[0xe0]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,
	0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
	0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,

	0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,
	0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,
	0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,

	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
	0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,

	0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,
	0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,
	0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,
	0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f
};

static const int oldsplus_ae[0xe0]={
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,

	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,

	0x1E,0x1F,0x20,0x21,0x22,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,

	0x1F,0x20,0x21,0x22,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,

	0x20,0x21,0x22,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,

	0x21,0x22,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,

	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23
};

static const int oldsplus_ba[0x4]={
	0x3138,0x2328,0x1C20,0x1518
};

static const int oldsplus_9d[0x111]={
	0x0000,0x0064,0x00c8,0x012c,0x0190,0x01f4,0x0258,0x02bc,
	0x02f8,0x0334,0x0370,0x03ac,0x03e8,0x0424,0x0460,0x049c,
	0x04d8,0x0514,0x0550,0x058c,0x05c8,0x0604,0x0640,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x0000,
	0x0064,0x00c8,0x012c,0x0190,0x01f4,0x0258,0x02bc,0x0302,
	0x0348,0x038e,0x03d4,0x041a,0x0460,0x04a6,0x04ec,0x0532,
	0x0578,0x05be,0x0604,0x064a,0x0690,0x06d6,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x0000,0x0064,
	0x00c8,0x012c,0x0190,0x01f4,0x0258,0x02bc,0x0316,0x0370,
	0x03ca,0x0424,0x047e,0x04d8,0x0532,0x058c,0x05e6,0x0640,
	0x069a,0x06f4,0x074e,0x07a8,0x0802,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x0000,0x0064,0x00c8,
	0x012c,0x0190,0x01f4,0x0258,0x02bc,0x032a,0x0398,0x0406,
	0x0474,0x04e2,0x0550,0x05be,0x062c,0x069a,0x0708,0x0776,
	0x07e4,0x0852,0x08c0,0x092e,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x0000,0x0064,0x00c8,0x012c,
	0x0190,0x01f4,0x0258,0x02bc,0x0348,0x03d4,0x0460,0x04ec,
	0x0578,0x0604,0x0690,0x071c,0x07a8,0x0834,0x08c0,0x094c,
	0x09d8,0x0a64,0x0af0,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x0000,0x0064,0x00c8,0x012c,0x0190,
	0x01f4,0x0258,0x02bc,0x0384,0x044c,0x0514,0x05dc,0x06a4,
	0x076c,0x0834,0x08fc,0x09c4,0x0a8c,0x0b54,0x0c1c,0x0ce4,
	0x0dac,0x0e74,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x0000,0x0064,0x00c8,0x012c,0x0190,0x01f4,
	0x0258,0x02bc,0x030c,0x035c,0x03ac,0x03fc,0x044c,0x049c,
	0x04ec,0x053c,0x058c,0x05dc,0x062c,0x067c,0x06cc,0x071c,
	0x076c,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc
};

static const int oldsplus_8c[0x20]={
	0x0032,0x0032,0x0064,0x0096,0x0096,0x00fa,0x012c,0x015e,
	0x0032,0x0064,0x0096,0x00c8,0x00c8,0x012c,0x015e,0x0190,
	0x0064,0x0096,0x00c8,0x00fa,0x00fa,0x015e,0x0190,0x01c2,
	0x0096,0x00c8,0x00fa,0x012c,0x012c,0x0190,0x01c2,0x01f4
};


void pgm_arm_type1_state::command_handler_oldsplus(int pc)
{
	switch (m_ddp3lastcommand)
	{
		case 0x88:
			m_simregion = ioport("Region")->read();
			m_valuekey = 0x100;
			m_valueresponse = 0x00990000 | m_simregion<<8;

			break;

		case 0xd0:
			m_valueresponse = 0xa01000 + (m_value0 << 5);
			break;

		case 0xc0:
			m_valueresponse = 0xa00000 + (m_value0 << 6);
			break;

		case 0xc3:
			m_valueresponse = 0xa00800 + (m_value0 << 6);
			break;

		case 0x36:
			m_extra_ram[0x36] = m_value0;
			m_valueresponse = 0x990000;
			break;

		case 0x33:
			m_extra_ram[0x33] = m_value0;
			m_valueresponse = 0x990000;
			break;

		case 0x35:
			m_extra_ram[0x36] += m_value0;
			m_valueresponse = 0x990000;
			break;

		case 0x37:
			m_extra_ram[0x33] += m_value0;
			m_valueresponse = 0x990000;
			break;

		case 0x34:
			m_valueresponse = m_extra_ram[0x36];
			break;

		case 0x38:
			m_valueresponse = m_extra_ram[0x33];
			break;

		case 0x80:
			m_valueresponse = oldsplus_80[m_value0];
			break;

		case 0xe7:
			m_extra_ram[0xe7] = m_value0;
			m_valueresponse = 0x990000;
			break;

		case 0xe5:
			switch (m_extra_ram[0xe7])
			{
				case 0xb000:
					m_slots[0xb] = m_value0;
					m_slots[0xc] = 0;
					break;

				case 0xc000:
					m_slots[0xc] = m_value0;
					break;

				case 0xd000:
					m_slots[0xd] = m_value0;
					break;

				case 0xf000:
					m_slots[0xf] = m_value0;
					break;
			}
			m_valueresponse = 0x990000;
			break;

		case 0xf8:
			m_valueresponse = m_slots[m_value0];
			break;

		case 0xfc:
			m_valueresponse = oldsplus_fc[m_value0];
			break;

		case 0xc5:
			m_slots[0xd] --;
			m_valueresponse = 0x990000;
			break;

		case 0xd6:
			m_slots[0xb] ++;
			m_valueresponse = 0x990000;
			break;

		case 0x3a:
			m_slots[0xf] = 0;
			m_valueresponse = 0x990000;
			break;

		case 0xf0:
			m_extra_ram[0xf0] = m_value0;
			m_valueresponse = 0x990000;
			break;

		case 0xed:
			m_valueresponse = m_value0 << 0x6;
			m_valueresponse += m_extra_ram[0xf0];
			m_valueresponse = m_valueresponse << 0x2;
			m_valueresponse += 0x900000;
			break;

		case 0xe0:
			m_extra_ram[0xe0] = m_value0;
			m_valueresponse = 0x990000;
			break;

		case 0xdc:
			m_valueresponse = m_value0 << 0x6;
			m_valueresponse += m_extra_ram[0xe0];
			m_valueresponse = m_valueresponse << 0x2;
			m_valueresponse += 0x904000;
			break;

		case 0xcb:
			m_valueresponse =  0xc000;
			break;

		case 0xa0:
			m_valueresponse = oldsplus_a0[m_value0];
			break;

		case 0xba:
			m_valueresponse = oldsplus_ba[m_value0];
			break;

		case 0x5e:
			m_valueresponse = oldsplus_5e[m_value0];
			break;

		case 0xb0:
			m_valueresponse = oldsplus_b0[m_value0];
			break;

		case 0xae:
			m_valueresponse = oldsplus_ae[m_value0];
			break;

		case 0x9d:
			m_valueresponse = oldsplus_9d[m_value0];
			break;

		case 0x90:
			m_valueresponse = oldsplus_90[m_value0];
			break;

		case 0x8c:
			m_valueresponse = oldsplus_8c[m_value0];
			break;

		default:
			m_valueresponse = 0x990000;
			printf("%06X: oldsplus_UNKNOWN W CMD %X  VAL %X\n", pc,m_value1,m_value0);
			break;
	}
}

WRITE16_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_sim_w )
{
	int pc = space.device().safe_pc();

	if (offset == 0)
	{
		m_value0 = data;
		return;
	}
	else if (offset == 1)
	{
		UINT16 realkey;
		if ((data >> 8) == 0xff)
			m_valuekey = 0xff00;
		realkey = m_valuekey >> 8;
		realkey |= m_valuekey;
		{
			m_valuekey += 0x0100;
			m_valuekey &= 0xff00;
			if (m_valuekey == 0xff00)
				m_valuekey =  0x0100;
		}
		data ^= realkey;
		m_value1 = data;
		m_value0 ^= realkey;

		m_ddp3lastcommand = m_value1 & 0xff;

		(this->*arm_sim_handler)(pc);
	}
	else if (offset==2)
	{
	}
}

READ16_MEMBER(pgm_arm_type1_state::pgm_arm7_type1_sim_protram_r )
{
	if (offset == 4)
		return m_simregion;

	return 0x0000;
}

READ16_MEMBER(pgm_arm_type1_state::pstars_arm7_type1_sim_protram_r )
{
	if (offset == 4)        //region
		return ioport("Region")->read();
	else if (offset >= 0x10)  //timer
	{
		logerror("PSTARS ACCESS COUNTER %6X\n", m_extra_ram[offset - 0x10]);
		return m_extra_ram[offset - 0x10]--;
	}
	return 0x0000;
}


DRIVER_INIT_MEMBER(pgm_arm_type1_state,ddp3)
{
	pgm_basic_init(false);
	pgm_py2k2_decrypt(machine()); // yes, it's the same as photo y2k2
	arm_sim_handler = &pgm_arm_type1_state::command_handler_ddp3;
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x500000, 0x500005, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_r),this), write16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_w),this));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,ket)
{
	pgm_basic_init(false);
	pgm_ket_decrypt(machine());
	arm_sim_handler = &pgm_arm_type1_state::command_handler_ddp3;
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x400000, 0x400005, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_r),this), write16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_w),this));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,espgal)
{
	pgm_basic_init(false);
	pgm_espgal_decrypt(machine());
	arm_sim_handler = &pgm_arm_type1_state::command_handler_ddp3;
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x400000, 0x400005, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_r),this), write16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_w),this));
}


int count_bits(UINT16 value)
{
	int count = 0;
	for (int i=0;i<16;i++)
	{
		int bit = (value >> i) & 1;

		if (bit) count++;
	}

	return count;
}

int get_position_of_bit(UINT16 value, int bit_wanted)
{
	int count = 0;
	for (int i=0;i<16;i++)
	{
		int bit = (value >> i) & 1;

		if (bit) count++;

		if (count==(bit_wanted+1))
			return i;
	}

	return -1;
}

int pgm_arm_type1_state::puzzli2_take_leveldata_value(UINT8 datvalue)
{
	if (stage==-1)
	{
		entries_left = 0;
		currentcolumn = 0;
		currentrow = 0;
		num_entries = 0;
		full_entry = 0;
		prev_tablloc = 0;
		numbercolumns = 0;
		depth = 0;
		m_row_bitmask = 0;

		puzzli2_printf("%02x <- table offset\n", datvalue);
		tableoffs = datvalue;
		tableoffs2 = 0;
		stage = 0;
	}
	else
	{
		UINT8 rawvalue = datvalue;
		UINT8 tableloc = (tableoffs+tableoffs2)&0xff;
		rawvalue ^= puzzli2_level_decode[tableloc];

		tableoffs2++;
		tableoffs2&=0xf;

		if (stage==0)
		{
			stage = 1;

			// this seems to be the first thing returned back when reading the level structure always seems to be 0x8 or 0x7, makes sense, levels would be too difficult otherwise ;-) (actually puzzli2 seems to have one specifying 5 unless it's a decrypt table error?!)
			depth = (rawvalue & 0xf0);
			numbercolumns = (rawvalue & 0x0f);
			numbercolumns++;

			puzzli2_printf("%02x <- Sizes (level depth %01x) (number of columns %01x)", rawvalue, depth>>4, numbercolumns);


			if ((depth != 0x80) && (depth != 0x70) && (depth != 0x50))
				fatalerror("depth isn't 0x5, 0x7 or 0x8");



			// it seems to use this to specify the number of columns, ie how many data structures follow, so that the ARM knows when to send back the 'finished' flag.
			// it also gets returned at the end with said flag
			if ((numbercolumns != 0x6) && (numbercolumns != 0x7) && (numbercolumns != 0x8))
				fatalerror("number of columns specified isn't 6,7, or 8");

			puzzli2_printf("\n");

		}
		else if (stage==1)
		{
			puzzli2_printf("%02x <- Number of Entries for this Column (and upper mask) (column is %d) (xor table location is %02x) ", rawvalue, currentcolumn, tableloc);
			stage = 2;
			entries_left = (rawvalue >> 4);
			m_row_bitmask = (rawvalue & 0x0f)<<8;

			full_entry = rawvalue;
			prev_tablloc = tableloc;

			num_entries = entries_left;

			if (num_entries == 0x00)
			{
				puzzli2_printf("0 entries for this column?"); // seems a valid condition based on the data
			}



			puzzli2_printf("\n");


		}
		else if (stage==2)
		{
			puzzli2_printf("%02x <- Mask value equal to number of entries (xor table location is %02x)", rawvalue, tableloc);
			stage = 3;

			m_row_bitmask |= rawvalue;

			int num_mask_bits = count_bits(m_row_bitmask);

			if (num_mask_bits != num_entries)
				puzzli2_printf(" error - number of mask bits != number of entries - ");

			//
			if (entries_left == 0)
			{
				// for 0 entries skip back to state 1 instead of 3, because there is nothing following
				stage = 1;
				currentcolumn++;
				currentrow = 0;
				m_row_bitmask = 0;

				coverage[tableloc] = 1;
				if (rawvalue!=0)
				{
					puzzli2_printf(" invalid mask after 00 length?");
					// attempt to correct the table
					//new_puzzli2_level_decode[tableloc] = new_puzzli2_level_decode[tableloc] ^ rawvalue;

				}
				coverage[prev_tablloc] = 1;
				if (full_entry!=0)
				{
					puzzli2_printf(" previous value wasn't 0x00");
				}

				if (currentcolumn==numbercolumns)
				{
					return 1;
				}

			}
			else
			{
				if (num_entries> 0xa)
				{
					puzzli2_printf(" more than 10 entries?");
				}
				else
				{
					// this isn't a strict rule
					// the mask is used so they can specify spaces between elements too without storing the 00 bytes
					coverage[tableloc] = 1;

					int desired_mask = 0;

					if (num_entries==0x00) desired_mask = 0x00;
					if (num_entries==0x01) desired_mask = 0x01;
					if (num_entries==0x02) desired_mask = 0x03;
					if (num_entries==0x03) desired_mask = 0x07;
					if (num_entries==0x04) desired_mask = 0x0f;
					if (num_entries==0x05) desired_mask = 0x1f;
					if (num_entries==0x06) desired_mask = 0x3f;
					if (num_entries==0x07) desired_mask = 0x7f;
					if (num_entries==0x08) desired_mask = 0xff;
					if (num_entries==0x09) desired_mask = 0xff;
					if (num_entries==0x0a) desired_mask = 0xff;

					if (rawvalue!=desired_mask)
					{
						puzzli2_printf(" possible wrong mask?");
					}


				}

			}

			puzzli2_printf("\n");

		}
		else if (stage==3)
		{
			UINT16 object_value;

			// return values
			// 0x0100 = normal fish
			// 0x0120 = fish in bubble
			// 0x0140 = fish in egg
			// 0x0160 = buggy fish in egg (displayed as normal fish)
			// 0x0180 = fish on hook
			// 0x01a0 = fish on hook (uncatchable)
			// 0x01c0 - fish on hook (uncatchable)
			// 0x01e0 - fish on hook (uncatchable)

			// fish values
			// 100 101 102 103 104 105 106 107 108 normal
			// 110 - renders as a flashing fish you can't catch? glitches game?
			// 111 - repeat of other fish type you can't catch...

			if (rawvalue<=0x10) // regular fish
			{
				int fishtype = rawvalue;
				puzzli2_printf("%02x <- fish type %d", rawvalue, fishtype);
				object_value = 0x0100 + fishtype;
				// 0x110 is a flashy fish? might be glitchy and need a special number..
			}
			else if (rawvalue<=0x21) // fish in bubbles
			{
				int fishtype = rawvalue - 0x11;
				puzzli2_printf("%02x <- fish in bubble %d", rawvalue, fishtype);
				object_value = 0x0120 + fishtype;
				// 0x130 is a flashy fish? might be glitchy and need a special number..
			}
			else if (rawvalue<=0x32) // fish in eggs
			{
				int fishtype = rawvalue - 0x22;
				puzzli2_printf("%02x <- fish in egg %d", rawvalue, fishtype);
				object_value = 0x0140 + fishtype;
				// 0x150 is a flashy fish? might be glitchy and need a special number..

			}
			else if (rawvalue<=0x43) // fish on hook cases, seem to be base 0x180
			{
				int fishtype = rawvalue - 0x33;
				puzzli2_printf("%02x <- fish on hook %d", rawvalue, fishtype);
				object_value = 0x0180 + fishtype;
				// 0x190 is a flashy fish? might be glitchy and need a special number..

			}
			////////////////////// special objects follow
			else if (rawvalue==0xd0) {object_value = 0x0200; puzzli2_printf("%02x <- generic bubbles", rawvalue);}

			else if (rawvalue==0xe0) {object_value = 0x8000; puzzli2_printf("%02x <- solid middle", rawvalue);}
			else if (rawvalue==0xe1) {object_value = 0x8020; puzzli2_printf("%02x <- solid top slant down", rawvalue);} // solid slant top down
			else if (rawvalue==0xe2) {object_value = 0x8040; puzzli2_printf("%02x <- solid top slant up", rawvalue);} // solid slant top up
			else if (rawvalue==0xe3) {object_value = 0x8060; puzzli2_printf("%02x <- solid bottom slant up", rawvalue);}
			else if (rawvalue==0xe4) {object_value = 0x8080; puzzli2_printf("%02x <- solid bottom slant down", rawvalue);} // sold slant bottom up


			else                     {object_value = 0xffff; puzzli2_printf("%02x <- unknown object", rawvalue);}

			puzzli2_printf("  (xor table location is %02x)\n",tableloc);

			if (object_value==0xffff)
			{
				object_value = 0x110;
				popmessage("unknown object type %02x\n", rawvalue);
			}

			int realrow = get_position_of_bit(m_row_bitmask, currentrow);

			if (realrow != -1)
				level_structure[currentcolumn][realrow] = object_value;

			currentrow++;

			entries_left--;
			if (entries_left == 0)
			{
				stage = 1;
				currentcolumn++;
				currentrow = 0;
				m_row_bitmask = 0;

				if (currentcolumn==numbercolumns)
				{
					return 1;
				}

			}
		}

	}

	return 0;
}



DRIVER_INIT_MEMBER(pgm_arm_type1_state,puzzli2)
{
	pgm_basic_init();


	pgm_puzzli2_decrypt(machine());
	arm_sim_handler = &pgm_arm_type1_state::command_handler_puzzli2;
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x500000, 0x500005, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_r),this), write16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0000, 0x4f003f, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_protram_r),this));
	m_irq4_disabled = 1; // // doesn't like this irq?? - seems to be RTC related

	hackcount = 0;
	hackcount2 = 0;
	hack_47_value = 0;
	hack_31_table_offset = 0;

//#define PUZZLI2_LEVEL_STRUCTURE_LOG
#ifdef PUZZLI2_LEVEL_STRUCTURE_LOG
	UINT8 *src2 = (UINT8 *) (machine().root_device().memregion("maincpu")->base());

	int offset;
	int limit;
	for (int i=0;i<256;i++)
		coverage[i] = 0;

	if (!strcmp(machine().system().name,"puzzli2"))
	{
		offset = 0x17ab66;
			limit = 476;
	}
	else
	{
			offset = 0x16c3ca;
			limit = 500;
	}




	for (int i=0;i<limit;i++)
	{
		UINT32 val1 = (src2[offset+1]<<24) | (src2[offset+0] << 16) | (src2[offset+3]<<8) | (src2[offset+2] << 0);
		offset += 4;
		UINT32 val2 = (src2[offset+1]<<24) | (src2[offset+0] << 16) | (src2[offset+3]<<8) | (src2[offset+2] << 0);


		printf("(%d) data range %08x %08x\n", i, val1, val2);

		int x = 0;


		stage = -1;

		for (x=val1; x<val2;x++)
		{
			int end = puzzli2_take_leveldata_value(src2[x^1]);

			if (end)
			{
				printf("--- ended at %08x (val 2 was %08x)\n",x ,val2);

				if ( (val2-x) > 2 )
				{
					fatalerror("ended even earlier than padding byte\n");
				}
				else if ( (val2-x) == 2)
				{
					printf("ended with padding byte\n");
				}
				else if ( (val2-x) == 1)
				{
					printf("ended normally\n");
				}
				else // can't happen
				{
					fatalerror("ended after marker?!\n");
				}

				x = val2;
			}

		}

		printf("total number of valid columns was %02x\n", currentcolumn);

		if ((currentcolumn!=0x6) && (currentcolumn!=0x7) && (currentcolumn!=0x8))  // 5 is suspicious
			fatalerror("invalid number of columns?\n");

		if (numbercolumns != currentcolumn)
			fatalerror("mismatch in number of columns vs specified amount\n");

		printf("\n");


	}


#endif

#if 0
	if (!strcmp(machine().system().name,"puzzli2"))
	{
	UINT8 *src3 = (UINT8 *) (machine().root_device().memregion("maincpu")->base());
	printf("how to play data pointer %02x %02x %02x %02x\n", src3[0x17b28e ^1], src3[0x17b28f ^1], src3[0x17b290 ^1], src3[0x17b291 ^1]);
	src3[0x17b28e ^1] = 0x00;
	src3[0x17b28f ^1] = 0x11;
	src3[0x17b290 ^1] = 0x42;
	src3[0x17b291 ^1] = 0x40;
	}


	pgm_puzzli2_decrypt(machine());

	{
		UINT8 *ROM = (UINT8*)memregion("maincpu")->base();

		FILE *fp;
		char filename[256];
		sprintf(filename,"trojan_%s", machine().system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(ROM+0x100000, 0x200000, 1, fp);
			fclose(fp);
		}
	}


	pgm_puzzli2_decrypt(machine());
#endif
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,py2k2)
{
	pgm_basic_init();
	pgm_py2k2_decrypt(machine());
	arm_sim_handler = &pgm_arm_type1_state::command_handler_py2k2;
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x500000, 0x500005, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_r),this), write16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0000, 0x4f003f, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_protram_r),this));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,pgm3in1)
{
	pgm_basic_init();
	pgm_decrypt_pgm3in1(machine());
	arm_sim_handler = &pgm_arm_type1_state::command_handler_py2k2;
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x500000, 0x500005,read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_r),this), write16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0000, 0x4f003f, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_protram_r),this));
	m_irq4_disabled = 1; // // doesn't like this irq??
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,pstar)
{
	pgm_basic_init();
	pgm_pstar_decrypt(machine());
	pgm_arm7_type1_latch_init();

	m_pstar_e7_value = 0;
	m_pstar_b1_value = 0;
	m_pstar_ce_value = 0;
	m_extra_ram[0] = 0;
	m_extra_ram[1] = 0;
	m_extra_ram[2] = 0;
	memset(m_slots, 0, 16 * sizeof(UINT32));

	arm_sim_handler = &pgm_arm_type1_state::command_handler_pstars;
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x500000, 0x500005, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_r),this), write16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0000, 0x4f003f, read16_delegate(FUNC(pgm_arm_type1_state::pstars_arm7_type1_sim_protram_r),this));

	save_item(NAME(m_pstar_e7_value));
	save_item(NAME(m_pstar_b1_value));
	save_item(NAME(m_pstar_ce_value));
	save_item(NAME(m_extra_ram));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,kov)
{
	pgm_basic_init();
	pgm_kov_decrypt(machine());
	pgm_arm7_type1_latch_init();
	m_curslots = 0;
	m_kov_c0_value = 0;
	m_kov_cb_value = 0;
	m_kov_fe_value = 0;
	arm_sim_handler = &pgm_arm_type1_state::command_handler_kov;
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x500000, 0x500005, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_r),this), write16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0000, 0x4f003f, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_protram_r),this));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovboot)
{
	pgm_basic_init();
//  pgm_kov_decrypt(machine());
	pgm_arm7_type1_latch_init();
	m_curslots = 0;
	m_kov_c0_value = 0;
	m_kov_cb_value = 0;
	m_kov_fe_value = 0;
	arm_sim_handler = &pgm_arm_type1_state::command_handler_kov;
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x500000, 0x500005, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_r),this), write16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0000, 0x4f003f, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_protram_r),this));

}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,oldsplus)
{
	pgm_basic_init();
	pgm_oldsplus_decrypt(machine());
	pgm_arm7_type1_latch_init();
	memset(m_extra_ram, 0, 0x100 * sizeof(UINT16));
	memset(m_slots, 0, 0x100 * sizeof(UINT32));
	arm_sim_handler = &pgm_arm_type1_state::command_handler_oldsplus;
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x500000, 0x500005, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_r),this), write16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4f0000, 0x4f003f, read16_delegate(FUNC(pgm_arm_type1_state::pgm_arm7_type1_sim_protram_r),this));
	save_item(NAME(m_extra_ram));
}

INPUT_PORTS_START( photoy2k )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")    /* Region - supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x00ff, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x00ff, "Untouched" ) // don't hack the region
INPUT_PORTS_END



INPUT_PORTS_START( kovsh )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")    /* Region - supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x00ff, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
	PORT_CONFSETTING(      0x00ff, "Untouched" ) // don't hack the region
INPUT_PORTS_END



INPUT_PORTS_START( sango )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")   /* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0005, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END

INPUT_PORTS_START( sango_ch )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")   /* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0000, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END


INPUT_PORTS_START( oldsplus )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")   /* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0001, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, DEF_STR( Japan ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0006, DEF_STR( Taiwan ) )
INPUT_PORTS_END

INPUT_PORTS_START( pstar )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")   /* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0005, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END

INPUT_PORTS_START( py2k2 )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")   /* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0003, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0006, "Singapore, Malaysia" )
INPUT_PORTS_END


INPUT_PORTS_START( puzzli2 )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")   /* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0005, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END
