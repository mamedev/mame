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
#include "pgm.h"
#include "pgmprot_igs027a_type2.h"

#define LOG_PROT    (1U << 1)
#define LOG_ALL     (LOG_PROT)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGPROT(...) LOGMASKED(LOG_PROT, __VA_ARGS__)

u32 pgm_arm_type2_state::arm7_latch_arm_r(offs_t offset, u32 mem_mask)
{
	if (!machine().side_effects_disabled())
		m_prot->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, CLEAR_LINE); // guess

	LOGPROT("%s ARM7: Latch read: %08x (%08x)\n", machine().describe_context(), m_kov2_latchdata_68k_w, mem_mask);
	return m_kov2_latchdata_68k_w;
}

void pgm_arm_type2_state::arm7_latch_arm_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGPROT("%s ARM7: Latch write: %08x (%08x)\n", machine().describe_context(), data, mem_mask);

	COMBINE_DATA(&m_kov2_latchdata_arm_w);
}

u32 pgm_arm_type2_state::arm7_shareram_r(offs_t offset, u32 mem_mask)
{
	LOGPROT("%s ARM7: ARM7 Shared RAM Read: %04x = %08x (%08x)\n", machine().describe_context(), offset << 2, m_arm7_shareram[offset], mem_mask);
	return m_arm7_shareram[offset];
}

void pgm_arm_type2_state::arm7_shareram_w(offs_t offset, u32 data, u32 mem_mask)
{
	LOGPROT("%s ARM7: ARM7 Shared RAM Write: %04x = %08x (%08x)\n", machine().describe_context(), offset << 2, data, mem_mask);
	COMBINE_DATA(&m_arm7_shareram[offset]);
}

u16 pgm_arm_type2_state::arm7_latch_68k_r(offs_t offset, u16 mem_mask)
{
	LOGPROT("%s M68K: Latch read: %04x (%04x)\n", machine().describe_context(), m_kov2_latchdata_arm_w & 0x0000ffff, mem_mask);
	return m_kov2_latchdata_arm_w;
}

void pgm_arm_type2_state::arm7_latch_68k_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGPROT("%s M68K: Latch write: %04x (%04x)\n", machine().describe_context(), data & 0x0000ffff, mem_mask);
	COMBINE_DATA(&m_kov2_latchdata_68k_w);

	m_prot->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, ASSERT_LINE); // guess
}

u16 pgm_arm_type2_state::arm7_ram_r(offs_t offset, u16 mem_mask)
{
	const u16 *share16 = reinterpret_cast<u16 *>(m_arm7_shareram.target());

	LOGPROT("%s M68K: ARM7 Shared RAM Read: %04x = %04x (%08x)\n", machine().describe_context(), BYTE_XOR_LE(offset), share16[BYTE_XOR_LE(offset)], mem_mask);
	return share16[BYTE_XOR_LE(offset)];
}

void pgm_arm_type2_state::arm7_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 *share16 = reinterpret_cast<u16 *>(m_arm7_shareram.target());

	LOGPROT("%s M68K: ARM7 Shared RAM Write: %04x = %04x (%04x)\n", machine().describe_context(), BYTE_XOR_LE(offset), data, mem_mask);
	COMBINE_DATA(&share16[BYTE_XOR_LE(offset)]);
}

/* 55857F? */
/* Knights of Valor 2, Martial Masters, DoDonpachi 2 */
/*  no execute only space? */
void pgm_arm_type2_state::kov2_mem(address_map &map)
{
	pgm_mem(map);
	map(0x100000, 0x5fffff).bankr("bank1"); /* Game ROM */
	map(0xd00000, 0xd0ffff).rw(FUNC(pgm_arm_type2_state::arm7_ram_r), FUNC(pgm_arm_type2_state::arm7_ram_w)); /* ARM7 Shared RAM */
	map(0xd10000, 0xd10001).rw(FUNC(pgm_arm_type2_state::arm7_latch_68k_r), FUNC(pgm_arm_type2_state::arm7_latch_68k_w)); /* ARM7 Latch */
}

u32 pgm_arm_type2_state::external_rom_r(offs_t offset)
{
	return m_external_rom[offset] ^ m_xor_table[offset & 0x00ff];
}

void pgm_arm_type2_state::xor_table_w(offs_t offset, u8 data)
{
	m_xor_table[offset] = (u32(data) << 24) | (u32(data) << 8);
}

void pgm_arm_type2_state::_55857F_arm7_map(address_map &map)
{
	map(0x00000000, 0x00003fff).rom();
	map(0x08000000, 0x083fffff).rom().r(FUNC(pgm_arm_type2_state::external_rom_r));
	map(0x10000000, 0x100003ff).ram();
	map(0x18000000, 0x1800ffff).ram().share("arm_ram");
	map(0x38000000, 0x38000003).rw(FUNC(pgm_arm_type2_state::arm7_latch_arm_r), FUNC(pgm_arm_type2_state::arm7_latch_arm_w)); /* 68k Latch */
	map(0x48000000, 0x4800ffff).rw(FUNC(pgm_arm_type2_state::arm7_shareram_r), FUNC(pgm_arm_type2_state::arm7_shareram_w)).share("arm7_shareram");
	map(0x50000000, 0x500003ff).ram().umask32(0x000000ff).w(FUNC(pgm_arm_type2_state::xor_table_w));
}

/******* ARM 55857F *******/

void pgm_arm_type2_state::pgm_arm_type2(machine_config &config) // ARM7 Shared motherboard XTAL
{
	pgmbase(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &pgm_arm_type2_state::kov2_mem);

	/* protection CPU */
	ARM7(config, m_prot, 20000000);    // 55857F
	m_prot->set_addrmap(AS_PROGRAM, &pgm_arm_type2_state::_55857F_arm7_map);
}

void pgm_arm_type2_state::pgm_arm_type2_22m(machine_config &config) // ARM7 uses 22MHz XTAL (martmast dw2001 dwpc)
{
	pgm_arm_type2(config);

	m_prot->set_clock(22000000);
}


void pgm_arm_type2_state::kov2_latch_init()
{
	m_kov2_latchdata_68k_w = 0;
	m_kov2_latchdata_arm_w = 0;
	std::fill(std::begin(m_xor_table), std::end(m_xor_table), 0);

	save_item(NAME(m_kov2_latchdata_68k_w));
	save_item(NAME(m_kov2_latchdata_arm_w));
	save_item(NAME(m_xor_table));
}

void pgm_arm_type2_state::kov2_arm_region_w(offs_t offset, u32 data, u32 mem_mask)
{
	const int pc = m_prot->pc();
	const int regionhack = m_regionhack->read();
	if (pc == 0x190 && regionhack != 0xff) data = (data & 0xffff0000) | (regionhack << 0);
	COMBINE_DATA(&m_arm7_shareram[0x138/4]);
}

void pgm_arm_type2_state::kov2p_arm_region_w(offs_t offset, u32 data, u32 mem_mask)
{
	const int pc = m_prot->pc();
	const int regionhack = m_regionhack->read();
//  printf("%08x\n", pc);
	if (pc == 0x1b0 && regionhack != 0xff) data = (data & 0xffff0000) | (regionhack << 0);
	COMBINE_DATA(&m_arm7_shareram[0x138/4]);
}


void pgm_arm_type2_state::init_kov2()
{
	pgm_basic_init();
	pgm_kov2_decrypt(machine());
	kov2_latch_init();

	// we only have a HK internal ROM dumped for now, allow us to override that for debugging purposes.
	m_prot->space(AS_PROGRAM).install_write_handler(0x48000138, 0x4800013b, write32s_delegate(*this, FUNC(pgm_arm_type2_state::kov2_arm_region_w)));
}


void pgm_arm_type2_state::init_kov2p()
{
	// this hacks the identification of the kov2 rom to return the string required for kov2p
	// this isn't guaranteed to work properly (and definitely wouldn't on real hardware due to the internal
	// ROM uploading the encryption table)  The internal ROM should be dumped properly.
	pgm_basic_init();
	pgm_kov2p_decrypt(machine());
	kov2_latch_init();

	// we only have a China internal ROM dumped for now, allow us to override that for debugging purposes.
	m_prot->space(AS_PROGRAM).install_write_handler(0x48000138, 0x4800013b, write32s_delegate(*this, FUNC(pgm_arm_type2_state::kov2p_arm_region_w)));
}

void pgm_arm_type2_state::martmast_arm_region_w(offs_t offset, u32 data, u32 mem_mask)
{
	const int pc = m_prot->pc();
	const int regionhack = m_regionhack->read();
	if (pc == 0x170 && regionhack != 0xff) data = (data & 0xffff0000) | (regionhack << 0);
	COMBINE_DATA(&m_arm7_shareram[0x138/4]);
}


void pgm_arm_type2_state::init_martmast()
{
	pgm_basic_init();
	pgm_mm_decrypt(machine());
	kov2_latch_init();

	// we only have a USA / CHINA internal ROMs dumped for now, allow us to override that for debugging purposes.
	m_prot->space(AS_PROGRAM).install_write_handler(0x48000138, 0x4800013b, write32s_delegate(*this, FUNC(pgm_arm_type2_state::martmast_arm_region_w)));
}


u32 pgm_arm_type2_state::ddp2_speedup_r(address_space &space)
{
	const int pc = m_prot->pc();
	const u32 data = m_arm_ram[0x300c/4];

	if (pc == 0x080109b4)
	{
		/* if we've hit the loop where this is read and both values are 0 then the only way out is an interrupt */
		int r4 = (m_prot->state_int(arm7_cpu_device::ARM7_R4));
		r4 += 0xe;

		if (r4 == 0x18002f9e)
		{
			const u32 data2 =  m_arm_ram[0x2F9C/4] & 0xffff0000;
			if ((data == 0x00000000) && (data2 == 0x00000000)) space.device().execute().spin_until_interrupt();
		}
	}

	return data;
}

u16 pgm_arm_type2_state::ddp2_main_speedup_r()
{
	const u16 data = m_mainram[0x0ee54/2];
	const int pc = m_maincpu->pc();

	if (pc == 0x149dce) m_maincpu->spin_until_interrupt();
	if (pc == 0x149cfe) m_maincpu->spin_until_interrupt();

	return data;

}

void pgm_arm_type2_state::init_ddp2()
{
	pgm_basic_init();
	pgm_ddp2_decrypt(machine());
	kov2_latch_init();

	m_prot->space(AS_PROGRAM).install_read_handler(0x1800300c, 0x1800300f, read32mo_delegate(*this, FUNC(pgm_arm_type2_state::ddp2_speedup_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80ee54, 0x80ee55, read16smo_delegate(*this, FUNC(pgm_arm_type2_state::ddp2_main_speedup_r)));
}


void pgm_arm_type2_state::init_dw2001()
{
	pgm_basic_init();
	kov2_latch_init();
	pgm_mm_decrypt(machine()); // encryption is the same as martial masters
}

void pgm_arm_type2_state::init_dwpc()
{
	pgm_basic_init();
	pgm_dwpc_decrypt(machine());
	kov2_latch_init();

	// we only have a JAPAN internal ROM dumped for now, so hack it to allow booting the Chinese version
	u8 *armrom = memregion("prot")->base();
	armrom[0x3c8] = 0x01;
}

void pgm_arm_type2_state::init_dwpc101j()
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
	PORT_CONFSETTING(      0x0006, "USA (Andamiro USA license)" )
	PORT_CONFSETTING(      0x00ff, "Untouched" ) // don't hack the region
INPUT_PORTS_END


INPUT_PORTS_START( dw2001 )
	PORT_INCLUDE ( pgm )

	PORT_MODIFY("Region")   /* Region - supplied by protection device */
	PORT_BIT(      0xfff0, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_CONFNAME( 0x000f, 0x0005, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, "Japan (Alta license)" )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, DEF_STR( World ) )
INPUT_PORTS_END
