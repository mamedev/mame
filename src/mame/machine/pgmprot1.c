/***********************************************************************
 PGM IGS027A (55857E* type) ARM protection simulations & emulation
   *guess, the part number might not be directly tied to behavior, see note below

 these are simulations of the 'kov' type ARM device
 used by

 Knights of Valor (kov) + bootlegs
 Knights of Valor Plus (kovplus)
 Puzzli 2 Super (puzzli2)
 Photo Y2k2 (py2k2)
 Puzzle Star (puzlstar)

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

 68k code is encrypted on these, decryption table is uploaded to
 ARM space.

 Game Region is supplied by internal ARM rom.


 ***********************************************************************/

#include "emu.h"
#include "includes/pgm.h"

/**************************** EMULATION *******************************/
/* used by photoy2k, kovsh */

static READ32_HANDLER( pgm_arm7_type1_protlatch_r )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();

	space.machine().scheduler().synchronize(); // force resync

	return (state->m_pgm_arm_type1_highlatch_68k_w << 16) | (state->m_pgm_arm_type1_lowlatch_68k_w);
}

static WRITE32_HANDLER( pgm_arm7_type1_protlatch_w )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();

	space.machine().scheduler().synchronize(); // force resync

	if (ACCESSING_BITS_16_31)
	{
		state->m_pgm_arm_type1_highlatch_arm_w = data >> 16;
		state->m_pgm_arm_type1_highlatch_68k_w = 0;
	}
	if (ACCESSING_BITS_0_15)
	{
		state->m_pgm_arm_type1_lowlatch_arm_w = data;
		state->m_pgm_arm_type1_lowlatch_68k_w = 0;
	}
}

static READ16_HANDLER( pgm_arm7_type1_68k_protlatch_r )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();

	space.machine().scheduler().synchronize(); // force resync

	switch (offset)
	{
		case 1: return state->m_pgm_arm_type1_highlatch_arm_w;
		case 0: return state->m_pgm_arm_type1_lowlatch_arm_w;
	}
	return -1;
}

static WRITE16_HANDLER( pgm_arm7_type1_68k_protlatch_w )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();

	space.machine().scheduler().synchronize(); // force resync

	switch (offset)
	{
		case 1:
			state->m_pgm_arm_type1_highlatch_68k_w = data;
			break;

		case 0:
			state->m_pgm_arm_type1_lowlatch_68k_w = data;
			break;
	}
}

static READ16_HANDLER( pgm_arm7_type1_ram_r )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();
	UINT16 *share16 = reinterpret_cast<UINT16 *>(state->m_arm7_shareram.target());

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Read: %04x = %04x (%08x) (%06x)\n", BYTE_XOR_LE(offset), share16[BYTE_XOR_LE(offset)], mem_mask, space.device().safe_pc());
	return share16[BYTE_XOR_LE(offset << 1)];
}

static WRITE16_HANDLER( pgm_arm7_type1_ram_w )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();
	UINT16 *share16 = reinterpret_cast<UINT16 *>(state->m_arm7_shareram.target());

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Write: %04x = %04x (%04x) (%06x)\n", BYTE_XOR_LE(offset), data, mem_mask, space.device().safe_pc());
	COMBINE_DATA(&share16[BYTE_XOR_LE(offset << 1)]);
}




static READ32_HANDLER( pgm_arm7_type1_unk_r )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();
	return state->m_pgm_arm_type1_counter++;
}

static READ32_HANDLER( pgm_arm7_type1_exrom_r )
{
	return 0x00000000;
}

static READ32_HANDLER( pgm_arm7_type1_shareram_r )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: ARM7 Shared RAM Read: %04x = %08x (%08x) (%06x)\n", offset << 2, state->m_arm7_shareram[offset], mem_mask, space.device().safe_pc());
	return state->m_arm7_shareram[offset];
}

static WRITE32_HANDLER( pgm_arm7_type1_shareram_w )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: ARM7 Shared RAM Write: %04x = %08x (%08x) (%06x)\n", offset << 2, data, mem_mask, space.device().safe_pc());
	COMBINE_DATA(&state->m_arm7_shareram[offset]);
}

/* 55857E? */
/* Knights of Valor, Photo Y2k */
/*  no execute only space? */
static ADDRESS_MAP_START( kov_map, AS_PROGRAM, 16, pgm_arm_type1_state )
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x4effff) AM_ROMBANK("bank1") /* Game ROM */
	AM_RANGE(0x4f0000, 0x4f003f) AM_READWRITE_LEGACY(pgm_arm7_type1_ram_r, pgm_arm7_type1_ram_w) /* ARM7 Shared RAM */
	AM_RANGE(0x500000, 0x500005) AM_READWRITE_LEGACY(pgm_arm7_type1_68k_protlatch_r, pgm_arm7_type1_68k_protlatch_w) /* ARM7 Latch */
ADDRESS_MAP_END

static ADDRESS_MAP_START( 55857E_arm7_map, AS_PROGRAM, 32, pgm_arm_type1_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08100000, 0x083fffff) AM_READ_LEGACY(pgm_arm7_type1_exrom_r) // unpopulated, returns 0 to keep checksum happy
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM // internal ram for asic
	AM_RANGE(0x40000000, 0x40000003) AM_READWRITE_LEGACY(pgm_arm7_type1_protlatch_r, pgm_arm7_type1_protlatch_w)
	AM_RANGE(0x40000008, 0x4000000b) AM_WRITENOP // ?
	AM_RANGE(0x4000000c, 0x4000000f) AM_READ_LEGACY(pgm_arm7_type1_unk_r)
	AM_RANGE(0x50800000, 0x5080003f) AM_READWRITE_LEGACY(pgm_arm7_type1_shareram_r, pgm_arm7_type1_shareram_w) AM_SHARE("arm7_shareram")
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



	m_prot = machine().device<cpu_device>("prot");

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
	MCFG_CPU_ADD("prot", ARM7, 20000000 )	// 55857E?
	MCFG_CPU_PROGRAM_MAP(55857E_arm7_map)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED( pgm_arm_type1, pgm_arm_type1_cave )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kov_map)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)	// 55857E?
	MCFG_CPU_PROGRAM_MAP(55857E_arm7_map)
MACHINE_CONFIG_END

void pgm_arm7_type1_latch_init( running_machine &machine )
{
	pgm_arm_type1_state *state = machine.driver_data<pgm_arm_type1_state>();

	state->m_pgm_arm_type1_highlatch_arm_w = 0;
	state->m_pgm_arm_type1_lowlatch_arm_w = 0;
	state->m_pgm_arm_type1_highlatch_68k_w = 0;
	state->m_pgm_arm_type1_lowlatch_68k_w = 0;
	state->m_pgm_arm_type1_counter = 1;

	state->save_item(NAME(state->m_pgm_arm_type1_highlatch_arm_w));
	state->save_item(NAME(state->m_pgm_arm_type1_lowlatch_arm_w));
	state->save_item(NAME(state->m_pgm_arm_type1_highlatch_68k_w));
	state->save_item(NAME(state->m_pgm_arm_type1_lowlatch_68k_w));
	state->save_item(NAME(state->m_pgm_arm_type1_counter));
}

static READ16_HANDLER( kovsh_fake_region_r )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();
	int regionhack = state->ioport("RegionHack")->read();
	if (regionhack != 0xff) return regionhack;

	offset = 0x4;
	UINT16 *share16 = reinterpret_cast<UINT16 *>(state->m_arm7_shareram.target());
	return share16[BYTE_XOR_LE(offset << 1)];
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,photoy2k)
{
	pgm_basic_init(machine());
	pgm_photoy2k_decrypt(machine());
	pgm_arm7_type1_latch_init(machine());
	/* we only have a china internal ROM dumped for now.. allow region to be changed for debugging (to ensure all alt titles / regions can be seen) */
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0008, 0x4f0009, FUNC(kovsh_fake_region_r));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovsh)
{
	pgm_basic_init(machine());
	pgm_kovsh_decrypt(machine());
	pgm_arm7_type1_latch_init(machine());
	/* we only have a china internal ROM dumped for now.. allow region to be changed for debugging (to ensure all alt titles / regions can be seen) */
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0008, 0x4f0009, FUNC(kovsh_fake_region_r));
}

/* Fake remapping of ASIC commands to the ones used by KOVSH due to the lack of the real ARM rom for this set */
WRITE16_HANDLER( kovshp_asic27a_write_word )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();

	switch (offset)
	{
		case 0:
			state->m_pgm_arm_type1_lowlatch_68k_w = data;
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

			state->m_pgm_arm_type1_highlatch_68k_w = asic_cmd ^ (asic_key | (asic_key << 8));
		}
		return;
	}
}


DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovshp)
{
	pgm_basic_init(machine());
	pgm_kovshp_decrypt(machine());
	pgm_arm7_type1_latch_init(machine());
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0008, 0x4f0009, FUNC(kovsh_fake_region_r));
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x500000, 0x500005, FUNC(kovshp_asic27a_write_word));
}



/* bootleg inits */

DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovshxas)
{
	pgm_basic_init(machine());
//  pgm_kovshp_decrypt(machine());
	pgm_arm7_type1_latch_init(machine());
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0008, 0x4f0009, FUNC(kovsh_fake_region_r));
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x500000, 0x500005, FUNC(kovshp_asic27a_write_word));
}

static void pgm_decode_kovlsqh2_tiles( running_machine &machine )
{
	int i, j;
	UINT16 *src = (UINT16 *)(machine.root_device().memregion("tiles")->base() + 0x180000);
	UINT16 *dst = auto_alloc_array(machine, UINT16, 0x800000);

	for (i = 0; i < 0x800000 / 2; i++)
	{
		j = BITSWAP24(i, 23, 22, 9, 8, 21, 18, 0, 1, 2, 3, 16, 15, 14, 13, 12, 11, 10, 19, 20, 17, 7, 6, 5, 4);

		dst[j] = BITSWAP16(src[i], 1, 14, 8, 7, 0, 15, 6, 9, 13, 2, 5, 10, 12, 3, 4, 11);
	}

	memcpy( src, dst, 0x800000 );

	auto_free( machine, dst );
}

static void pgm_decode_kovlsqh2_sprites( running_machine &machine, UINT8 *src )
{
	int i, j;
	UINT8 *dst = auto_alloc_array(machine, UINT8, 0x800000);

	for (i = 0; i < 0x800000; i++)
	{
		j = BITSWAP24(i, 23, 10, 9, 22, 19, 18, 20, 21, 17, 16, 15, 14, 13, 12, 11, 8, 7, 6, 5, 4, 3, 2, 1, 0);

		dst[j] = src[i];
	}

	memcpy( src, dst, 0x800000 );

	auto_free( machine, dst );
}

static void pgm_decode_kovlsqh2_samples( running_machine &machine )
{
	int i;
	UINT8 *src = (UINT8 *)(machine.root_device().memregion("ics")->base() + 0x400000);

	for (i = 0; i < 0x400000; i+=2) {
		src[i + 0x000001] = src[i + 0x400001];
	}

	memcpy( src + 0x400000, src, 0x400000 );
}

static void pgm_decode_kovqhsgs_program( running_machine &machine )
{
	int i;
	UINT16 *src = (UINT16 *)(machine.root_device().memregion("maincpu")->base() + 0x100000);
	UINT16 *dst = auto_alloc_array(machine, UINT16, 0x400000);

	for (i = 0; i < 0x400000 / 2; i++)
	{
		int j = BITSWAP24(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 6, 7, 5, 4, 3, 2, 1, 0);

		dst[j] = BITSWAP16(src[i], 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 4, 5, 3, 2, 1, 0);
	}

	memcpy( src, dst, 0x400000 );

	auto_free( machine, dst );
}

static void pgm_decode_kovqhsgs2_program( running_machine &machine )
{
	int i;
	UINT16 *src = (UINT16 *)(machine.root_device().memregion("maincpu")->base() + 0x100000);
	UINT16 *dst = auto_alloc_array(machine, UINT16, 0x400000);

	for (i = 0; i < 0x400000 / 2; i++)
	{
		int j = BITSWAP24(i, 23, 22, 21, 20, 19, 16, 15, 14, 13, 12, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 18, 17, 7);

		dst[j] = src[i];
	}

	memcpy( src, dst, 0x400000 );

	auto_free( machine, dst );
}


DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovlsqh2)
{
	pgm_decode_kovqhsgs2_program(machine());
	pgm_decode_kovlsqh2_tiles(machine());

	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x0800000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x1000000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x1800000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x2000000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x2800000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprmask")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprmask")->base() + 0x0800000);

	pgm_decode_kovlsqh2_samples(machine());
	pgm_basic_init(machine());
	pgm_arm7_type1_latch_init(machine());
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0008, 0x4f0009, FUNC(kovsh_fake_region_r));
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x500000, 0x500005, FUNC(kovshp_asic27a_write_word));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovqhsgs)
{
	pgm_decode_kovqhsgs_program(machine());
	pgm_decode_kovlsqh2_tiles(machine());

	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x0800000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x1000000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x1800000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x2000000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprcol")->base() + 0x2800000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprmask")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(machine(), machine().root_device().memregion("sprmask")->base() + 0x0800000);

	pgm_decode_kovlsqh2_samples(machine());
	pgm_basic_init(machine());
	pgm_arm7_type1_latch_init(machine());
	/* we only have a china internal ROM dumped for now.. allow region to be changed for debugging (to ensure all alt titles / regions can be seen) */
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0008, 0x4f0009, FUNC(kovsh_fake_region_r));
}

/*
 in Ketsui (ket) @ 000A719C (move.w)

 if you change D0 to 0x12
 the game will runs to "Asic27 Test" mode

 bp A71A0,1,{d0=0x12;g}
*/

static READ16_HANDLER( pgm_arm7_type1_sim_r )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();

	if (offset == 0)
	{
		UINT16 d = state->m_valueresponse & 0xffff;
		UINT16 realkey = state->m_valuekey >> 8;
		realkey |= state->m_valuekey;
		d ^= realkey;

		return d;

	}
	else if (offset == 1)
	{
		UINT16 d = state->m_valueresponse >> 16;
		UINT16 realkey = state->m_valuekey >> 8;
		realkey |= state->m_valuekey;
		d ^= realkey;
		return d;

	}
	return 0xffff;
}

/* working */
void command_handler_ddp3(pgm_arm_type1_state *state, int pc)
{
	switch (state->m_ddp3lastcommand)
	{
		default:
			printf("%06x command %02x | %04x\n", pc, state->m_ddp3lastcommand, state->m_value0);
			state->m_valueresponse = 0x880000;
			break;

		case 0x40:
			state->m_valueresponse = 0x880000;
		    state->m_slots[(state->m_value0>>10)&0x1F]=
				(state->m_slots[(state->m_value0>>5)&0x1F]+
				 state->m_slots[(state->m_value0>>0)&0x1F])&0xffffff;
			break;

		case 0x67: // set high bits
	//      printf("%06x command %02x | %04x\n", space.device().safe_pc(), state->m_ddp3lastcommand, state->m_value0);
			state->m_valueresponse = 0x880000;
			state->m_curslots = (state->m_value0 & 0xff00)>>8;
			state->m_slots[state->m_curslots] = (state->m_value0 & 0x00ff) << 16;
			break;

		case 0xe5: // set low bits for operation?
		//  printf("%06x command %02x | %04x\n", space.device().safe_pc(), state->m_ddp3lastcommand, state->m_value0);
			state->m_valueresponse = 0x880000;
			state->m_slots[state->m_curslots] |= (state->m_value0 & 0xffff);
			break;


		case 0x8e: // read back result of operations
	//      printf("%06x command %02x | %04x\n", space.device().safe_pc(), state->m_ddp3lastcommand, state->m_value0);
			state->m_valueresponse = state->m_slots[state->m_value0&0xff];
			break;


		case 0x99: // reset?
			state->m_valuekey = 0x100;
			state->m_valueresponse = 0x00880000;
			break;

	}
}

/* preliminary */

void command_handler_puzzli2(pgm_arm_type1_state *state, int pc)
{
	printf("%08x: %02x %04x\n",pc, state->m_ddp3lastcommand, state->m_value0);

	switch (state->m_ddp3lastcommand)
	{
		case 0x13: // ASIC status?
			state->m_valueresponse = 0x74<<16; // 2d or 74! (based on?)
		break;

		case 0x31:
		{
			// how is this selected? command 54?

			// just a wild guess
			if (state->m_puzzli_54_trigger) {
				// pc == 1387de
				state->m_valueresponse = 0x63<<16; // ?
			} else {
				// pc == 14cf58
				state->m_valueresponse = 0xd2<<16;
			}

			state->m_puzzli_54_trigger = 0;
		}
		break;

		case 0x38: // Reset
			state->m_valueresponse = 0x78<<16;
			state->m_valuekey = 0x100;
			state->m_puzzli_54_trigger = 0;
		break;

		case 0x41: // ASIC status?
			state->m_valueresponse = 0x74<<16;
		break;

		case 0x47: // ASIC status?
			state->m_valueresponse = 0x74<<16;
		break;

		case 0x52: // ASIC status?
		{
			// how is this selected?

			//if (state->m_value0 == 6) {
				state->m_valueresponse = (0x74<<16)|1; // |1?
			//} else {
			//  state->m_valueresponse = 0x74<<16;
			//}
		}
		break;

		case 0x54: // ??
			state->m_puzzli_54_trigger = 1;
			state->m_valueresponse = 0x36<<16;
		break;

		case 0x61: // ??
			state->m_valueresponse = 0x36<<16;
		break;

		case 0x63: // used as a read address by the 68k code (related to previous uploaded values like cave?) should point at a table of ~0x80 in size? seems to use values as further pointers?
			state->m_valueresponse = 0x00600000;
		break;

		case 0x67: // used as a read address by the 68k code (related to previous uploaded values like cave?) directly reads ~0xDBE from the address..
			state->m_valueresponse = 0x00400000;
		break;

		default:
			state->m_valueresponse = 0x74<<16;
		break;
	}
}

/* preliminary */
void command_handler_py2k2(pgm_arm_type1_state *state, int pc)
{
	switch (state->m_ddp3lastcommand)
	{
		default:
			printf("%06x command %02x | %04x\n", pc, state->m_ddp3lastcommand, state->m_value0);
			state->m_valueresponse = 0x880000;
			break;

		case 0xc0:
			printf("%06x command %02x | %04x\n", pc, state->m_ddp3lastcommand, state->m_value0);
			state->m_valueresponse = 0x880000;
			break;

		case 0x99: // reset?
			state->m_valuekey = 0x100;
			state->m_valueresponse = 0x00880000;
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

void command_handler_pstars(pgm_arm_type1_state *state, int pc)
{
	switch (state->m_ddp3lastcommand)
	{
		case 0x99:
			state->m_valuekey = 0x100;
			state->m_valueresponse = 0x880000;
			break;

		case 0xe0:
			state->m_valueresponse = 0xa00000 + (state->m_value0 << 6);
			break;

		case 0xdc:
			state->m_valueresponse = 0xa00800 + (state->m_value0 << 6);
			break;

		case 0xd0:
			state->m_valueresponse = 0xa01000 + (state->m_value0 << 5);
			break;

		case 0xb1:
			state->m_pstar_b1_value = state->m_value0;
			state->m_valueresponse = 0x890000;
			break;

		case 0xbf:
			state->m_valueresponse = state->m_pstar_b1_value * state->m_value0;
			break;

		case 0xc1: //TODO:TIMER  0,1,2,FIX TO 0 should be OK?
			state->m_valueresponse = 0;
			break;

		case 0xce: //TODO:TIMER  0,1,2
			state->m_pstar_ce_value = state->m_value0;
			state->m_valueresponse=0x890000;
			break;

		case 0xcf: //TODO:TIMER  0,1,2
			state->m_extra_ram[state->m_pstar_ce_value] = state->m_value0;
			state->m_valueresponse = 0x890000;
			break;

		case 0xe7:
			state->m_pstar_e7_value = (state->m_value0 >> 12) & 0xf;
			state->m_slots[state->m_pstar_e7_value] &= 0xffff;
			state->m_slots[state->m_pstar_e7_value] |= (state->m_value0 & 0xff) << 16;
			state->m_valueresponse = 0x890000;
			break;

		case 0xe5:
			state->m_slots[state->m_pstar_e7_value] &= 0xff0000;
			state->m_slots[state->m_pstar_e7_value] |= state->m_value0;
			state->m_valueresponse = 0x890000;
			break;

		case 0xf8: //@73C
			state->m_valueresponse = state->m_slots[state->m_value0 & 0xf] & 0xffffff;
			break;

		case 0xba:
			state->m_valueresponse = pstar_ba[state->m_value0];
			break;

		case 0xb0:
			state->m_valueresponse = pstar_b0[state->m_value0];
			break;

		case 0xae:
			state->m_valueresponse = pstar_ae[state->m_value0];
			break;

		case 0xa0:
			state->m_valueresponse = pstar_a0[state->m_value0];
			break;

		case 0x9d:
			state->m_valueresponse = pstar_9d[state->m_value0];
			break;

		case 0x90:
			state->m_valueresponse = pstar_90[state->m_value0];
			break;

		case 0x8c:
			state->m_valueresponse = pstar_8c[state->m_value0];
			break;

		case 0x80:
			state->m_valueresponse = pstar_80[state->m_value0];
			break;

		default:
			state->m_valueresponse = 0x890000;
			logerror("PSTARS PC(%06x) UNKNOWN %4X %4X\n", pc, state->m_value1, state->m_value0);
	}
}

/* Old KOV and bootlegs sim ... really these should be read out... */

static const UINT8 kov_BATABLE[0x40] = {
	0x00,0x29,0x2c,0x35,0x3a,0x41,0x4a,0x4e,0x57,0x5e,0x77,0x79,0x7a,0x7b,0x7c,0x7d,
	0x7e,0x7f,0x80,0x81,0x82,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x90,
	0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9e,0xa3,0xd4,0xa9,0xaf,0xb5,0xbb,0xc1
};

static const UINT8 kov_B0TABLE[16] = { 2, 0, 1, 4, 3 }; // Maps char portraits to tables


void command_handler_kov(pgm_arm_type1_state *state, int pc)
{
	switch (state->m_ddp3lastcommand)
	{
		case 0x67: // unknown or status check?
		case 0x8e:
		case 0xa3:
		case 0x33: // kovsgqyz (a3)
		case 0x3a: // kovplus
		case 0xc5: // kovplus
			state->m_valueresponse = 0x880000;
		break;

		case 0x99: // Reset
			state->m_valueresponse = 0x880000;
			state->m_valuekey = 0x100;
		break;

		case 0x9d: // Sprite palette offset
			state->m_valueresponse = 0xa00000 + ((state->m_value0 & 0x1f) * 0x40);
		break;

		case 0xb0: // Read from data table
			state->m_valueresponse = kov_B0TABLE[state->m_value0 & 0x0f];
		break;

		case 0xb4: // Copy slot 'a' to slot 'b'
		case 0xb7: // kovsgqyz (b4)
		{
			state->m_valueresponse = 0x880000;

			if (state->m_value0 == 0x0102) state->m_value0 = 0x0100; // why?

			state->m_slots[(state->m_value0 >> 8) & 0x0f] = state->m_slots[(state->m_value0 >> 0) & 0x0f];
		}
		break;

		case 0xba: // Read from data table
			state->m_valueresponse = kov_BATABLE[state->m_value0 & 0x3f];
		break;

		case 0xc0: // Text layer 'x' select
			state->m_valueresponse = 0x880000;
			state->m_kov_c0_value = state->m_value0;
		break;

		case 0xc3: // Text layer offset
			state->m_valueresponse = 0x904000 + ((state->m_kov_c0_value + (state->m_value0 * 0x40)) * 4);
		break;

		case 0xcb: // Background layer 'x' select
			state->m_valueresponse = 0x880000;
			state->m_kov_cb_value = state->m_value0;
		break;

		case 0xcc: // Background layer offset
			if (state->m_value0 & 0x400) state->m_value0 = -(0x400 - (state->m_value0 & 0x3ff));
			state->m_valueresponse = 0x900000 + ((state->m_kov_cb_value + (state->m_value0 * 0x40)) * 4);
		break;

		case 0xd0: // Text palette offset
		case 0xcd: // kovsgqyz (d0)
			state->m_valueresponse = 0xa01000 + (state->m_value0 * 0x20);
		break;

		case 0xd6: // Copy slot to slot 0
			state->m_valueresponse = 0x880000;
			state->m_slots[0] = state->m_slots[state->m_value0 & 0x0f];
		break;

		case 0xdc: // Background palette offset
		case 0x11: // kovsgqyz (dc)
			state->m_valueresponse = 0xa00800 + (state->m_value0 * 0x40);
		break;

		case 0xe0: // Sprite palette offset
		case 0x9e: // kovsgqyz (e0)
			state->m_valueresponse = 0xa00000 + ((state->m_value0 & 0x1f) * 0x40);
		break;

		case 0xe5: // Write slot (low)
		{
			state->m_valueresponse = 0x880000;

			INT32 sel = (state->m_curslots >> 12) & 0x0f;
			state->m_slots[sel] = (state->m_slots[sel] & 0x00ff0000) | ((state->m_value0 & 0xffff) <<  0);
		}
		break;

		case 0xe7: // Write slot (and slot select) (high)
		{
			state->m_valueresponse = 0x880000;
			state->m_curslots = state->m_value0;

			INT32 sel = (state->m_curslots >> 12) & 0x0f;
			state->m_slots[sel] = (state->m_slots[sel] & 0x0000ffff) | ((state->m_value0 & 0x00ff) << 16);
		}
		break;

		case 0xf0: // Some sort of status read?
			state->m_valueresponse = 0x00c000;
		break;

		case 0xf8: // Read slot
		case 0xab: // kovsgqyz (f8)
			state->m_valueresponse = state->m_slots[state->m_value0 & 0x0f] & 0x00ffffff;
		break;

		case 0xfc: // Adjust damage level to char experience level
			state->m_valueresponse = (state->m_value0 * state->m_kov_fe_value) >> 6;
		break;

		case 0xfe: // Damage level adjust
			state->m_valueresponse = 0x880000;
			state->m_kov_fe_value = state->m_value0;
		break;

		default:
			state->m_valueresponse = 0x880000;
//          bprintf (PRINT_NORMAL, _T("Unknown ASIC27 command: %2.2x data: %4.4x\n"), (data ^ state->m_valuekey) & 0xff, state->m_value0);
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


void command_handler_oldsplus(pgm_arm_type1_state *state, int pc)
{
	switch (state->m_ddp3lastcommand)
	{
		case 0x88:
			state->m_valuekey = 0x100;
			state->m_valueresponse = 0x990000;
			break;

		case 0xd0:
			state->m_valueresponse = 0xa01000 + (state->m_value0 << 5);
			break;

		case 0xc0:
			state->m_valueresponse = 0xa00000 + (state->m_value0 << 6);
			break;

		case 0xc3:
			state->m_valueresponse = 0xa00800 + (state->m_value0 << 6);
			break;

		case 0x36:
			state->m_extra_ram[0x36] = state->m_value0;
			state->m_valueresponse = 0x990000;
			break;

		case 0x33:
			state->m_extra_ram[0x33] = state->m_value0;
			state->m_valueresponse = 0x990000;
			break;

		case 0x35:
			state->m_extra_ram[0x36] += state->m_value0;
			state->m_valueresponse = 0x990000;
			break;

		case 0x37:
			state->m_extra_ram[0x33] += state->m_value0;
			state->m_valueresponse = 0x990000;
			break;

		case 0x34:
			state->m_valueresponse = state->m_extra_ram[0x36];
			break;

		case 0x38:
			state->m_valueresponse = state->m_extra_ram[0x33];
			break;

		case 0x80:
			state->m_valueresponse = oldsplus_80[state->m_value0];
			break;

		case 0xe7:
			state->m_extra_ram[0xe7] = state->m_value0;
			state->m_valueresponse = 0x990000;
			break;

		case 0xe5:
			switch (state->m_extra_ram[0xe7])
			{
				case 0xb000:
					state->m_slots[0xb] = state->m_value0;
					state->m_slots[0xc] = 0;
					break;

				case 0xc000:
					state->m_slots[0xc] = state->m_value0;
					break;

				case 0xd000:
					state->m_slots[0xd] = state->m_value0;
					break;

				case 0xf000:
					state->m_slots[0xf] = state->m_value0;
					break;
			}
			state->m_valueresponse = 0x990000;
			break;

		case 0xf8:
			state->m_valueresponse = state->m_slots[state->m_value0];
			break;

		case 0xfc:
			state->m_valueresponse = oldsplus_fc[state->m_value0];
			break;

		case 0xc5:
			state->m_slots[0xd] --;
			state->m_valueresponse = 0x990000;
			break;

		case 0xd6:
			state->m_slots[0xb] ++;
			state->m_valueresponse = 0x990000;
			break;

		case 0x3a:
			state->m_slots[0xf] = 0;
			state->m_valueresponse = 0x990000;
			break;

		case 0xf0:
			state->m_extra_ram[0xf0] = state->m_value0;
			state->m_valueresponse = 0x990000;
			break;

		case 0xed:
			state->m_valueresponse = state->m_value0 << 0x6;
			state->m_valueresponse += state->m_extra_ram[0xf0];
			state->m_valueresponse = state->m_valueresponse << 0x2;
			state->m_valueresponse += 0x900000;
			break;

		case 0xe0:
			state->m_extra_ram[0xe0] = state->m_value0;
			state->m_valueresponse = 0x990000;
			break;

		case 0xdc:
			state->m_valueresponse = state->m_value0 << 0x6;
			state->m_valueresponse += state->m_extra_ram[0xe0];
			state->m_valueresponse = state->m_valueresponse << 0x2;
			state->m_valueresponse += 0x904000;
			break;

		case 0xcb:
			state->m_valueresponse =  0xc000;
			break;

		case 0xa0:
			state->m_valueresponse = oldsplus_a0[state->m_value0];
			break;

		case 0xba:
			state->m_valueresponse = oldsplus_ba[state->m_value0];
			break;

		case 0x5e:
			state->m_valueresponse = oldsplus_5e[state->m_value0];
			break;

		case 0xb0:
			state->m_valueresponse = oldsplus_b0[state->m_value0];
			break;

		case 0xae:
			state->m_valueresponse = oldsplus_ae[state->m_value0];
			break;

		case 0x9d:
			state->m_valueresponse = oldsplus_9d[state->m_value0];
			break;

		case 0x90:
			state->m_valueresponse = oldsplus_90[state->m_value0];
			break;

		case 0x8c:
			state->m_valueresponse = oldsplus_8c[state->m_value0];
			break;

		default:
			state->m_valueresponse = 0x990000;
			printf("%06X: oldsplus_UNKNOWN W CMD %X  VAL %X\n", pc,state->m_value1,state->m_value0);
			break;
	}
}

static WRITE16_HANDLER( pgm_arm7_type1_sim_w )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();
	int pc = space.device().safe_pc();

	if (offset == 0)
	{
		state->m_value0 = data;
		return;
	}
	else if (offset == 1)
	{
		UINT16 realkey;
		if ((data >> 8) == 0xff)
			state->m_valuekey = 0xff00;
		realkey = state->m_valuekey >> 8;
		realkey |= state->m_valuekey;
		{
			state->m_valuekey += 0x0100;
			state->m_valuekey &= 0xff00;
			if (state->m_valuekey == 0xff00)
				state->m_valuekey =  0x0100;
		}
		data ^= realkey;
		state->m_value1 = data;
		state->m_value0 ^= realkey;

		state->m_ddp3lastcommand = state->m_value1 & 0xff;

		state->arm_sim_handler(state, pc);
	}
	else if (offset==2)
	{

	}
}

static READ16_HANDLER( pgm_arm7_type1_sim_protram_r )
{
	if (offset == 4)
		return space.machine().root_device().ioport("Region")->read();

	return 0x0000;
}

static READ16_HANDLER( pstars_arm7_type1_sim_protram_r )
{
	pgm_arm_type1_state *state = space.machine().driver_data<pgm_arm_type1_state>();

	if (offset == 4)		//region
		return state->ioport("Region")->read();
	else if (offset >= 0x10)  //timer
	{
		logerror("PSTARS ACCESS COUNTER %6X\n", state->m_extra_ram[offset - 0x10]);
		return state->m_extra_ram[offset - 0x10]--;
	}
	return 0x0000;
}


DRIVER_INIT_MEMBER(pgm_arm_type1_state,ddp3)
{
	pgm_basic_init(machine(), false);
	pgm_py2k2_decrypt(machine()); // yes, it's the same as photo y2k2
	arm_sim_handler = command_handler_ddp3;
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x500000, 0x500005, FUNC(pgm_arm7_type1_sim_r), FUNC(pgm_arm7_type1_sim_w));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,ket)
{
	pgm_basic_init(machine(), false);
	pgm_ket_decrypt(machine());
	arm_sim_handler = command_handler_ddp3;
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x400000, 0x400005, FUNC(pgm_arm7_type1_sim_r), FUNC(pgm_arm7_type1_sim_w));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,espgal)
{
	pgm_basic_init(machine(), false);
	pgm_espgal_decrypt(machine());
	arm_sim_handler = command_handler_ddp3;
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x400000, 0x400005, FUNC(pgm_arm7_type1_sim_r), FUNC(pgm_arm7_type1_sim_w));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,puzzli2)
{
	pgm_basic_init(machine());
	pgm_puzzli2_decrypt(machine());
	arm_sim_handler = command_handler_puzzli2;
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x500000, 0x500005, FUNC(pgm_arm7_type1_sim_r), FUNC(pgm_arm7_type1_sim_w));
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0000, 0x4f003f, FUNC(pgm_arm7_type1_sim_protram_r));
	m_irq4_disabled = 1; // // doesn't like this irq??
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,py2k2)
{
	pgm_basic_init(machine());
	pgm_py2k2_decrypt(machine());
	arm_sim_handler = command_handler_py2k2;
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x500000, 0x500005, FUNC(pgm_arm7_type1_sim_r), FUNC(pgm_arm7_type1_sim_w));
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0000, 0x4f003f, FUNC(pgm_arm7_type1_sim_protram_r));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,pstar)
{
	pgm_basic_init(machine());
	pgm_pstar_decrypt(machine());
	pgm_arm7_type1_latch_init(machine());

	m_pstar_e7_value = 0;
	m_pstar_b1_value = 0;
	m_pstar_ce_value = 0;
	m_extra_ram[0] = 0;
	m_extra_ram[1] = 0;
	m_extra_ram[2] = 0;
	memset(m_slots, 0, 16 * sizeof(UINT32));

	arm_sim_handler = command_handler_pstars;
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x500000, 0x500005, FUNC(pgm_arm7_type1_sim_r), FUNC(pgm_arm7_type1_sim_w));
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0000, 0x4f003f, FUNC(pstars_arm7_type1_sim_protram_r));

	save_item(NAME(m_pstar_e7_value));
	save_item(NAME(m_pstar_b1_value));
	save_item(NAME(m_pstar_ce_value));
	save_item(NAME(m_extra_ram));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,kov)
{
	pgm_basic_init(machine());
	pgm_kov_decrypt(machine());
	pgm_arm7_type1_latch_init(machine());
	m_curslots = 0;
	m_kov_c0_value = 0;
	m_kov_cb_value = 0;
	m_kov_fe_value = 0;
	arm_sim_handler = command_handler_kov;
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x500000, 0x500005, FUNC(pgm_arm7_type1_sim_r), FUNC(pgm_arm7_type1_sim_w));
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0000, 0x4f003f, FUNC(pgm_arm7_type1_sim_protram_r));
}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,kovboot)
{
	pgm_basic_init(machine());
//  pgm_kov_decrypt(machine());
	pgm_arm7_type1_latch_init(machine());
	m_curslots = 0;
	m_kov_c0_value = 0;
	m_kov_cb_value = 0;
	m_kov_fe_value = 0;
	arm_sim_handler = command_handler_kov;
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x500000, 0x500005, FUNC(pgm_arm7_type1_sim_r), FUNC(pgm_arm7_type1_sim_w));
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0000, 0x4f003f, FUNC(pgm_arm7_type1_sim_protram_r));

}

DRIVER_INIT_MEMBER(pgm_arm_type1_state,oldsplus)
{
	pgm_basic_init(machine());
	pgm_oldsplus_decrypt(machine());
	pgm_arm7_type1_latch_init(machine());
	memset(m_extra_ram, 0, 0x100 * sizeof(UINT16));
	memset(m_slots, 0, 0x100 * sizeof(UINT32));
	arm_sim_handler = command_handler_oldsplus;
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x500000, 0x500005, FUNC(pgm_arm7_type1_sim_r), FUNC(pgm_arm7_type1_sim_w));
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x4f0000, 0x4f003f, FUNC(pgm_arm7_type1_sim_protram_r));
	state_save_register_global_array(machine(), m_extra_ram);
	state_save_register_global_array(machine(), m_slots);
}

INPUT_PORTS_START( photoy2k )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")	/* Region - supplied by protection device */
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

	PORT_START("RegionHack")	/* Region - supplied by protection device */
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

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
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

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
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

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
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

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
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

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
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

	PORT_MODIFY("Region")	/* Region - supplied by protection device */
	PORT_CONFNAME( 0x000f, 0x0005, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END
