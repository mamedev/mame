// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi, Xing Xing
/***********************************************************************
 PGM IGS027A ARM protection emulation - type 2


 these are emulation of the 'kov2' type ARM device
 used by

 Knights of Valor 2 (kov2)
 Knights of Valor 2 Nine Dragons (kov2p)
 DoDonPachi 2 - Bee Storm (ddp2)
 Martial Masters (martmast)
 Dragon World 2001 (dw2001)
 Dragon World Pretty Chance (dwpc) (verified to be the same internal rom as dw2001)

 ----

  These games use a larger region of shared RAM than the earlier
  type and have a communication based on interrupts

 ----

 All of these games have an external ARM rom.

 The external ARM roms are encrypted, the internal ARM rom uploads
 the decryption tables.

 The external ARM rom is checksummed

 Game Region is supplied by internal ARM rom.

 ***********************************************************************/

#include "emu.h"
#include "includes/pgm.h"


READ32_MEMBER(pgm_arm_type2_state::arm7_latch_arm_r )
{
	m_prot->set_input_line(ARM7_FIRQ_LINE, CLEAR_LINE ); // guess

	if (PGMARM7LOGERROR)
		logerror("ARM7: Latch read: %08x (%08x) (%06x)\n", m_kov2_latchdata_68k_w, mem_mask, space.device().safe_pc());
	return m_kov2_latchdata_68k_w;
}

WRITE32_MEMBER(pgm_arm_type2_state::arm7_latch_arm_w )
{
	if (PGMARM7LOGERROR)
		logerror("ARM7: Latch write: %08x (%08x) (%06x)\n", data, mem_mask, space.device().safe_pc());

	COMBINE_DATA(&m_kov2_latchdata_arm_w);
}

READ32_MEMBER(pgm_arm_type2_state::arm7_shareram_r )
{
	if (PGMARM7LOGERROR)
		logerror("ARM7: ARM7 Shared RAM Read: %04x = %08x (%08x) (%06x)\n", offset << 2, m_arm7_shareram[offset], mem_mask, space.device().safe_pc());
	return m_arm7_shareram[offset];
}

WRITE32_MEMBER(pgm_arm_type2_state::arm7_shareram_w )
{
	if (PGMARM7LOGERROR)
		logerror("ARM7: ARM7 Shared RAM Write: %04x = %08x (%08x) (%06x)\n", offset << 2, data, mem_mask, space.device().safe_pc());
	COMBINE_DATA(&m_arm7_shareram[offset]);
}

READ16_MEMBER(pgm_arm_type2_state::arm7_latch_68k_r )
{
	if (PGMARM7LOGERROR)
		logerror("M68K: Latch read: %04x (%04x) (%06x)\n", m_kov2_latchdata_arm_w & 0x0000ffff, mem_mask, space.device().safe_pc());
	return m_kov2_latchdata_arm_w;
}

WRITE16_MEMBER(pgm_arm_type2_state::arm7_latch_68k_w )
{
	if (PGMARM7LOGERROR)
		logerror("M68K: Latch write: %04x (%04x) (%06x)\n", data & 0x0000ffff, mem_mask, space.device().safe_pc());
	COMBINE_DATA(&m_kov2_latchdata_68k_w);

	m_prot->set_input_line(ARM7_FIRQ_LINE, ASSERT_LINE ); // guess
}

READ16_MEMBER(pgm_arm_type2_state::arm7_ram_r )
{
	UINT16 *share16 = reinterpret_cast<UINT16 *>(m_arm7_shareram.target());

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Read: %04x = %04x (%08x) (%06x)\n", BYTE_XOR_LE(offset), share16[BYTE_XOR_LE(offset)], mem_mask, space.device().safe_pc());
	return share16[BYTE_XOR_LE(offset)];
}

WRITE16_MEMBER(pgm_arm_type2_state::arm7_ram_w )
{
	UINT16 *share16 = reinterpret_cast<UINT16 *>(m_arm7_shareram.target());

	if (PGMARM7LOGERROR)
		logerror("M68K: ARM7 Shared RAM Write: %04x = %04x (%04x) (%06x)\n", BYTE_XOR_LE(offset), data, mem_mask, space.device().safe_pc());
	COMBINE_DATA(&share16[BYTE_XOR_LE(offset)]);
}

/* 55857F? */
/* Knights of Valor 2, Martial Masters, DoDonpachi 2 */
/*  no execute only space? */
static ADDRESS_MAP_START( kov2_mem, AS_PROGRAM, 16, pgm_arm_type2_state )
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x5fffff) AM_ROMBANK("bank1") /* Game ROM */
	AM_RANGE(0xd00000, 0xd0ffff) AM_READWRITE(arm7_ram_r, arm7_ram_w) /* ARM7 Shared RAM */
	AM_RANGE(0xd10000, 0xd10001) AM_READWRITE(arm7_latch_68k_r, arm7_latch_68k_w) /* ARM7 Latch */
ADDRESS_MAP_END


static ADDRESS_MAP_START( 55857F_arm7_map, AS_PROGRAM, 32, pgm_arm_type2_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08000000, 0x083fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM
	AM_RANGE(0x18000000, 0x1800ffff) AM_RAM AM_SHARE("arm_ram")
	AM_RANGE(0x38000000, 0x38000003) AM_READWRITE(arm7_latch_arm_r, arm7_latch_arm_w) /* 68k Latch */
	AM_RANGE(0x48000000, 0x4800ffff) AM_READWRITE(arm7_shareram_r, arm7_shareram_w) AM_SHARE("arm7_shareram")
	AM_RANGE(0x50000000, 0x500003ff) AM_RAM
ADDRESS_MAP_END

MACHINE_START_MEMBER(pgm_arm_type2_state,pgm_arm_type2)
{
	MACHINE_START_CALL_MEMBER(pgm);
	/* register type specific Save State stuff here */
}

/******* ARM 55857F *******/

MACHINE_CONFIG_START( pgm_arm_type2, pgm_arm_type2_state )
	MCFG_FRAGMENT_ADD(pgmbase)

	MCFG_MACHINE_START_OVERRIDE(pgm_arm_type2_state, pgm_arm_type2 )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kov2_mem)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 20000000)    // 55857F
	MCFG_CPU_PROGRAM_MAP(55857F_arm7_map)
MACHINE_CONFIG_END




void pgm_arm_type2_state::kov2_latch_init()
{
	m_kov2_latchdata_68k_w = 0;
	m_kov2_latchdata_arm_w = 0;

	save_item(NAME(m_kov2_latchdata_68k_w));
	save_item(NAME(m_kov2_latchdata_arm_w));
}

WRITE32_MEMBER(pgm_arm_type2_state::kov2_arm_region_w )
{
	int pc = space.device().safe_pc();
	int regionhack = ioport("RegionHack")->read();
	if (pc==0x190 && regionhack != 0xff) data = (data & 0xffff0000) | (regionhack << 0);
	COMBINE_DATA(&m_arm7_shareram[0x138/4]);
}

WRITE32_MEMBER(pgm_arm_type2_state::kov2p_arm_region_w )
{
	int pc = space.device().safe_pc();
	int regionhack = ioport("RegionHack")->read();
//  printf("%08x\n", pc);
	if (pc==0x1b0 && regionhack != 0xff) data = (data & 0xffff0000) | (regionhack << 0);
	COMBINE_DATA(&m_arm7_shareram[0x138/4]);
}


DRIVER_INIT_MEMBER(pgm_arm_type2_state,kov2)
{
	pgm_basic_init();
	pgm_kov2_decrypt(machine());
	kov2_latch_init();

	// we only have a HK internal ROM dumped for now, allow us to override that for debugging purposes.
	machine().device("prot")->memory().space(AS_PROGRAM).install_write_handler(0x48000138, 0x4800013b, write32_delegate(FUNC(pgm_arm_type2_state::kov2_arm_region_w),this));
}


DRIVER_INIT_MEMBER(pgm_arm_type2_state,kov2p)
{
	// this hacks the identification of the kov2 rom to return the string required for kov2p
	// this isn't guaranteed to work properly (and definitely wouldn't on real hardware due to the internal
	// ROM uploading the encryption table)  The internal ROM should be dumped properly.
	pgm_basic_init();
	pgm_kov2p_decrypt(machine());
	kov2_latch_init();

	// we only have a China internal ROM dumped for now, allow us to override that for debugging purposes.
	machine().device("prot")->memory().space(AS_PROGRAM).install_write_handler(0x48000138, 0x4800013b, write32_delegate(FUNC(pgm_arm_type2_state::kov2p_arm_region_w),this));
}

WRITE32_MEMBER(pgm_arm_type2_state::martmast_arm_region_w )
{
	int pc = space.device().safe_pc();
	int regionhack = ioport("RegionHack")->read();
	if (pc==0x170 && regionhack != 0xff) data = (data & 0xffff0000) | (regionhack << 0);
	COMBINE_DATA(&m_arm7_shareram[0x138/4]);
}


DRIVER_INIT_MEMBER(pgm_arm_type2_state,martmast)
{
	pgm_basic_init();
	pgm_mm_decrypt(machine());
	kov2_latch_init();

	// we only have a USA / CHINA internal ROMs dumped for now, allow us to override that for debugging purposes.
	machine().device("prot")->memory().space(AS_PROGRAM).install_write_handler(0x48000138, 0x4800013b, write32_delegate(FUNC(pgm_arm_type2_state::martmast_arm_region_w),this));
}




READ32_MEMBER(pgm_arm_type2_state::ddp2_speedup_r )
{
	int pc = space.device().safe_pc();
	UINT32 data = m_arm_ram[0x300c/4];

	if (pc==0x080109b4)
	{
		/* if we've hit the loop where this is read and both values are 0 then the only way out is an interrupt */
		int r4 = (space.device().state().state_int(ARM7_R4));
		r4 += 0xe;

		if (r4==0x18002f9e)
		{
			UINT32 data2 =  m_arm_ram[0x2F9C/4]&0xffff0000;
			if ((data==0x00000000) && (data2==0x00000000)) space.device().execute().spin_until_interrupt();
		}
	}

	return data;
}

READ16_MEMBER(pgm_arm_type2_state::ddp2_main_speedup_r )
{
	UINT16 data = m_mainram[0x0ee54/2];
	int pc = space.device().safe_pc();

	if (pc == 0x149dce) space.device().execute().spin_until_interrupt();
	if (pc == 0x149cfe) space.device().execute().spin_until_interrupt();

	return data;

}

DRIVER_INIT_MEMBER(pgm_arm_type2_state,ddp2)
{
	pgm_basic_init();
	pgm_ddp2_decrypt(machine());
	kov2_latch_init();

	machine().device("prot")->memory().space(AS_PROGRAM).install_read_handler(0x1800300c, 0x1800300f, read32_delegate(FUNC(pgm_arm_type2_state::ddp2_speedup_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80ee54, 0x80ee55, read16_delegate(FUNC(pgm_arm_type2_state::ddp2_main_speedup_r),this));
}


DRIVER_INIT_MEMBER(pgm_arm_type2_state,dw2001)
{
	pgm_basic_init();
	kov2_latch_init();
	pgm_mm_decrypt(machine()); // encryption is the same as martial masters
}

DRIVER_INIT_MEMBER(pgm_arm_type2_state,dwpc)
{
	pgm_basic_init();
	kov2_latch_init();
	pgm_mm_decrypt(machine()); // encryption is the same as martial masters
}



INPUT_PORTS_START( kov2 )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")    /* Region - actually supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x00ff, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" ) // 'Busyou Souha' title for kov2p Japan
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
	PORT_CONFSETTING(      0x00ff, "Untouched" ) // don't hack the region
INPUT_PORTS_END

INPUT_PORTS_START( martmast )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")    /* Region - actually supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x00ff, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, DEF_STR( Japan ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
	PORT_CONFSETTING(      0x0006, DEF_STR( USA ) )
	PORT_CONFSETTING(      0x00ff, "Untouched" ) // don't hack the region
INPUT_PORTS_END


INPUT_PORTS_START( dw2001 )
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
