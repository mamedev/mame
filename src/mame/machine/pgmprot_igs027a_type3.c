// license:BSD-3-Clause
// copyright-holders:David Haywood, Xing Xing
/***********************************************************************
 PGM IGS027A ARM protection emulation

 These all use the 55857G type chips

 these are emulation of the 'dmnfrnt' type ARM device
 used by

 Demon Front (dmnfrnt) *1
 The Gladiator (theglad) *2
 Spectral vs. Generation (svg) *2
 Happy 6-in-1 (happy6) *4
 The Killing Blade Plus (killbldp) *3


 *1 - We bypass the internal ROM entirely! Game doesn't jump back
 *2 - Partial dump of internal ROM is used (currently only dumped from a Japan PCB, patched for other types) The missing code from the EO area is replaced with our own fake code with the same function
 *3 - Complete dump of IGS027A ROM sourced from a bootleg, looks to be legitimate.
 *4 - not yet working

 ----

 These games use a larger amount of shared RAM which is banked between CPUs compared to previous protection CPUs

 In most cases the games run almost entirely on the ARM with the 68k just parsing the display lists / feeding inputs to the ARM

 ----

 All of these games have an external ARM rom.

 The internal ROMs contain an 'execute only' region at the start of the
 ROM which prevents reading out.

 IGS027A type 55857G has also been seen on various IGS gambling boards
 as the main CPU (eg. Haunted House, see igs_m027a)

 55857G is also used on the Cave single board PGM systems, but in those
 cases it behaves like the 55857E (pgmprot1.c)

 Most of these games run almost entirely on the ARM, simply passing display lists back to the 68k to process and reading inputs from the 68k into the shared RAM.

 ***********************************************************************/

#include "emu.h"
#include "includes/pgm.h"

WRITE32_MEMBER(pgm_arm_type3_state::svg_arm7_ram_sel_w )
{
//  printf("svg_arm7_ram_sel_w %08x\n", data);
	machine().scheduler().synchronize(); // force resync
	m_svg_ram_sel = data & 1;
}

READ32_MEMBER(pgm_arm_type3_state::svg_arm7_shareram_r )
{
	UINT32 retdata = m_svg_shareram[m_svg_ram_sel & 1][offset];
//  printf("(%08x) ARM7: shared read (bank %02x) offset - %08x retdata - %08x mask - %08x\n", space.device().safe_pc(), m_svg_ram_sel, offset*4, retdata, mem_mask );
	return retdata;
}

WRITE32_MEMBER(pgm_arm_type3_state::svg_arm7_shareram_w )
{
//  printf("(%08x) ARM7: shared write (bank %02x) offset - %08x retdata - %08x mask - %08x\n", space.device().safe_pc(), m_svg_ram_sel, offset*4, data, mem_mask );
	COMBINE_DATA(&m_svg_shareram[m_svg_ram_sel & 1][offset]);
}

READ16_MEMBER(pgm_arm_type3_state::svg_m68k_ram_r )
{
	int ram_sel = (m_svg_ram_sel & 1) ^ 1;
	UINT16 *share16 = (UINT16 *)(m_svg_shareram[ram_sel & 1]);

	return share16[BYTE_XOR_LE(offset)];
}

WRITE16_MEMBER(pgm_arm_type3_state::svg_m68k_ram_w )
{
	int ram_sel = (m_svg_ram_sel & 1) ^ 1;
	UINT16 *share16 = (UINT16 *)(m_svg_shareram[ram_sel & 1]);

	COMBINE_DATA(&share16[BYTE_XOR_LE(offset)]);
}

READ16_MEMBER(pgm_arm_type3_state::svg_68k_nmi_r )
{
	return 0;
}

WRITE16_MEMBER(pgm_arm_type3_state::svg_68k_nmi_w )
{
	generic_pulse_irq_line(*m_prot, ARM7_FIRQ_LINE, 1);
}

WRITE16_MEMBER(pgm_arm_type3_state::svg_latch_68k_w )
{
	if (PGMARM7LOGERROR)
		logerror("M68K: Latch write: %04x (%04x) (%06x)\n", data & 0x0000ffff, mem_mask, space.device().safe_pc());
	COMBINE_DATA(&m_svg_latchdata_68k_w);
}


READ16_MEMBER(pgm_arm_type3_state::svg_latch_68k_r )
{
	if (PGMARM7LOGERROR)
		logerror("M68K: Latch read: %04x (%04x) (%06x)\n", m_svg_latchdata_arm_w & 0x0000ffff, mem_mask, space.device().safe_pc());
	return m_svg_latchdata_arm_w;
}



READ32_MEMBER(pgm_arm_type3_state::svg_latch_arm_r )
{
	if (PGMARM7LOGERROR)
		logerror("ARM7: Latch read: %08x (%08x) (%06x)\n", m_svg_latchdata_68k_w, mem_mask, space.device().safe_pc());
	return m_svg_latchdata_68k_w;
}

WRITE32_MEMBER(pgm_arm_type3_state::svg_latch_arm_w )
{
	if (PGMARM7LOGERROR)
		logerror("ARM7: Latch write: %08x (%08x) (%06x)\n", data, mem_mask, space.device().safe_pc());

	COMBINE_DATA(&m_svg_latchdata_arm_w);
}

/* 55857G? */
/* Demon Front, The Gladiator, Happy 6-in-1, Spectral Vs. Generation, Killing Blade EX */
/*  the ones with an EXECUTE ONLY region of ARM space? */
static ADDRESS_MAP_START( svg_68k_mem, AS_PROGRAM, 16, pgm_arm_type3_state )
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x1fffff) AM_ROMBANK("bank1")  /* Game ROM */

	AM_RANGE(0x500000, 0x50ffff) AM_READWRITE(svg_m68k_ram_r, svg_m68k_ram_w)    /* ARM7 Shared RAM */
	AM_RANGE(0x5c0000, 0x5c0001) AM_READWRITE(svg_68k_nmi_r, svg_68k_nmi_w)      /* ARM7 FIQ */
	AM_RANGE(0x5c0300, 0x5c0301) AM_READWRITE(svg_latch_68k_r, svg_latch_68k_w) /* ARM7 Latch */
ADDRESS_MAP_END


static ADDRESS_MAP_START( 55857G_arm7_map, AS_PROGRAM, 32, pgm_arm_type3_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08000000, 0x087fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM AM_SHARE("arm_ram2")
	AM_RANGE(0x18000000, 0x1803ffff) AM_RAM AM_SHARE("arm_ram")
	AM_RANGE(0x38000000, 0x3800ffff) AM_READWRITE(svg_arm7_shareram_r, svg_arm7_shareram_w)
	AM_RANGE(0x48000000, 0x48000003) AM_READWRITE(svg_latch_arm_r, svg_latch_arm_w) /* 68k Latch */
	AM_RANGE(0x40000018, 0x4000001b) AM_WRITE(svg_arm7_ram_sel_w) /* RAM SEL */
	AM_RANGE(0x50000000, 0x500003ff) AM_RAM
ADDRESS_MAP_END




MACHINE_RESET_MEMBER(pgm_arm_type3_state, pgm_arm_type3_reset)
{
	// internal roms aren't fully dumped
	UINT16 *temp16 = (UINT16 *)memregion("prot")->base();
	int base = -1;

	if (!strcmp(machine().system().name, "theglad")) base = 0x3316;
	if (!strcmp(machine().system().name, "theglad100")) base = 0x3316;
	if (!strcmp(machine().system().name, "theglad101")) base = 0x3316;
	if (!strcmp(machine().system().name, "happy6")) base = 0x3586;
	if (!strcmp(machine().system().name, "happy6101")) base = 0x3586;
	if (!strcmp(machine().system().name, "svgpcb")) base = 0x3a8e;
	if (!strcmp(machine().system().name, "svg")) base = 0x3c3e;
	if (!strcmp(machine().system().name, "svgtw")) base = 0x3a8e;

	if (base != -1)
	{
		int regionhack = ioport("RegionHack")->read();
		if (regionhack != 0xff)
		{
//          printf("%04x\n", temp16[(base) / 2]);
			temp16[(base) / 2] = regionhack; base += 2;
		}
	}
	MACHINE_RESET_CALL_MEMBER(pgm);
}

MACHINE_START_MEMBER(pgm_arm_type3_state,pgm_arm_type3)
{
	MACHINE_START_CALL_MEMBER(pgm);
	/* register type specific Save State stuff here */
}


/******* ARM 55857G *******/

MACHINE_CONFIG_START( pgm_arm_type3, pgm_arm_type3_state )
	MCFG_FRAGMENT_ADD(pgmbase)

	MCFG_MACHINE_START_OVERRIDE(pgm_arm_type3_state, pgm_arm_type3 )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(svg_68k_mem)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, XTAL_33MHz)    // 55857G - 33Mhz Xtal, at least on SVG
	MCFG_CPU_PROGRAM_MAP(55857G_arm7_map)

	MCFG_MACHINE_RESET_OVERRIDE(pgm_arm_type3_state, pgm_arm_type3_reset)
MACHINE_CONFIG_END



void pgm_arm_type3_state::svg_basic_init()
{
	pgm_basic_init();
	m_svg_shareram[0] = auto_alloc_array(machine(), UINT32, 0x20000 / 4);
	m_svg_shareram[1] = auto_alloc_array(machine(), UINT32, 0x20000 / 4);
	m_svg_ram_sel = 0;

	save_pointer(NAME(m_svg_shareram[0]), 0x20000 / 4);
	save_pointer(NAME(m_svg_shareram[1]), 0x20000 / 4);
	save_item(NAME(m_svg_ram_sel));
}

void pgm_arm_type3_state::pgm_create_dummy_internal_arm_region(int size)
{
	UINT16 *temp16 = (UINT16 *)memregion("prot")->base();

	// fill with RX 14
	int i;
	for (i=0;i<size/2;i+=2)
	{
		temp16[i] = 0xff1e;
		temp16[i+1] = 0xe12f;

	}

	// jump straight to external area
	temp16[(0x0000)/2] = 0xd088;
	temp16[(0x0002)/2] = 0xe59f;
	temp16[(0x0004)/2] = 0x0680;
	temp16[(0x0006)/2] = 0xe3a0;
	temp16[(0x0008)/2] = 0xff10;
	temp16[(0x000a)/2] = 0xe12f;
	temp16[(0x0090)/2] = 0x0400;
	temp16[(0x0092)/2] = 0x1000;
}

void pgm_arm_type3_state::svg_latch_init()
{
	m_svg_latchdata_68k_w = 0;
	m_svg_latchdata_arm_w = 0;

	save_item(NAME(m_svg_latchdata_68k_w));
	save_item(NAME(m_svg_latchdata_arm_w));
}

READ32_MEMBER(pgm_arm_type3_state::theglad_speedup_r )
{
	int pc = space.device().safe_pc();
	if (pc == 0x7c4) space.device().execute().eat_cycles(500);
	//else printf("theglad_speedup_r %08x\n", pc);
	return m_arm_ram2[0x00c/4];
}


READ32_MEMBER(pgm_arm_type3_state::happy6_speedup_r )
{
	int pc = space.device().safe_pc();
	if (pc == 0x0a08) space.device().execute().eat_cycles(500);
	//else printf("theglad_speedup_r %08x\n", pc);
	return m_arm_ram2[0x00c/4];
}

// installed over rom
READ32_MEMBER(pgm_arm_type3_state::svg_speedup_r )
{
	int pc = space.device().safe_pc();
	if (pc == 0xb90) space.device().execute().eat_cycles(500);
	return m_armrom[0xb90/4];
}

READ32_MEMBER(pgm_arm_type3_state::svgpcb_speedup_r )
{
	int pc = space.device().safe_pc();
	if (pc == 0x9e0) space.device().execute().eat_cycles(500);
	return m_armrom[0x9e0/4];
}


void pgm_arm_type3_state::pgm_create_dummy_internal_arm_region_theglad(int is_svg)
{
	UINT16 *temp16 = (UINT16 *)memregion("prot")->base();
	int i;
	for (i=0;i<0x188/2;i+=2)
	{
		temp16[i] = 0xFFFE;
		temp16[i+1] = 0xEAFF;

	}

	// the interrupt code appears to be at 0x08000010
	// so point the FIQ vector to jump there, the actual internal EO area code
	// would not look like this because this reads from the EO area to get the jump address which is verified
	// as impossible
	int base = 0x1c;
	temp16[(base) /2] = 0xf000; base += 2;
	temp16[(base) /2] = 0xe59f; base += 2;

	if (is_svg == 0)
	{
		temp16[(base) / 2] = 0x0010; base += 2;
		temp16[(base) / 2] = 0x0800; base += 2;
		temp16[(base) / 2] = 0x0010; base += 2;
		temp16[(base) / 2] = 0x0800; base += 2;
	}
	else
	{
		temp16[(base) / 2] = 0x0038; base += 2;
		temp16[(base) / 2] = 0x0800; base += 2;
		temp16[(base) / 2] = 0x0038; base += 2;
		temp16[(base) / 2] = 0x0800; base += 2;
	}

	// some startup code to set up the stacks etc. we're assuming
	// behavior is basically the same as killing blade plus here, this code
	// could be very wrong
	base = 0x30;

	temp16[(base) /2] = 0x00D2; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;
	temp16[(base) /2] = 0xE121; base += 2;

	temp16[(base) /2] = 0x4001; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0x4B06; base += 2;
	temp16[(base) /2] = 0xE284; base += 2;
	temp16[(base) /2] = 0x0CFA; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xD804; base += 2;
	temp16[(base) /2] = 0xE080; base += 2;
	temp16[(base) /2] = 0x00D1; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;
	temp16[(base) /2] = 0xE121; base += 2;
	temp16[(base) /2] = 0x0CF6; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xD804; base += 2;
	temp16[(base) /2] = 0xE080; base += 2;
	temp16[(base) /2] = 0x00D7; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;
	temp16[(base) /2] = 0xE121; base += 2;
	temp16[(base) /2] = 0x0CFF; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xD804; base += 2;
	temp16[(base) /2] = 0xE080; base += 2;
	temp16[(base) /2] = 0x00DB; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;
	temp16[(base) /2] = 0xE121; base += 2;
	temp16[(base) /2] = 0x4140; base += 2;
	temp16[(base) /2] = 0xE1C4; base += 2;
	temp16[(base) /2] = 0x0CFE; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xD804; base += 2;
	temp16[(base) /2] = 0xE080; base += 2;
	temp16[(base) /2] = 0x00D3; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;

	temp16[(base) /2] = 0xE121; base += 2;
	temp16[(base) /2] = 0x4A01; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0x0B01; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xD804; base += 2;
	temp16[(base) /2] = 0xE080; base += 2;
	temp16[(base) /2] = 0x5A0F; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0x0008; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0x8805; base += 2;
	temp16[(base) /2] = 0xE080; base += 2;
	temp16[(base) /2] = 0x0010; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0x0000; base += 2;
	temp16[(base) /2] = 0xE5C8; base += 2;
	temp16[(base) /2] = 0x7805; base += 2;
	temp16[(base) /2] = 0xE1A0; base += 2;
	temp16[(base) /2] = 0x6A01; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0x0012; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0x0A02; base += 2;
	temp16[(base) /2] = 0xE280; base += 2;
	temp16[(base) /2] = 0x6806; base += 2;
	temp16[(base) /2] = 0xE080; base += 2;
	temp16[(base) /2] = 0x6000; base += 2;
	temp16[(base) /2] = 0xE587; base += 2;

	// set the SR13 to something sensible
	temp16[(base) /2] = 0x00D3; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;
	temp16[(base) /2] = 0xE121; base += 2;
	temp16[(base) /2] = 0x4001; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0x4B06; base += 2;
	temp16[(base) /2] = 0xE284; base += 2;
	temp16[(base) /2] = 0x0CF2; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xD804; base += 2;
	temp16[(base) /2] = 0xE080; base += 2;

	temp16[(base) /2] = 0x0013; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;
	temp16[(base) /2] = 0xE121; base += 2;


	temp16[(base) / 2] = 0x0028; base += 2; // jump to 0x184
	temp16[(base) / 2] = 0xEA00; base += 2;


	base = 0;
	temp16[(base) /2] = 0x000a; base += 2;
	temp16[(base) /2] = 0xEA00; base += 2;

	// see table at ~080824A4 in The Gladiator (ARM space)
	// there are pointers to
	// 0000 00FC
	// 0000 00E8
	// 0000 0110
	// 0000 0150
	// in the table.. for e8 / fc we can deduce from the calling code and size of the functions expected that they should be the
	// same as those in the killing blade plus 'killbldp'  (there are also explicit jumps to these addresses in the code)
	//
	// 0x110 is called after the 'continue' screen, and on inserting coin, I guess it should change the game state, causing it to jump to the title screen when you insert a coin, and back to the attract after the game.. some kind of 'soft reset'
	// 0x150 I haven't seen called, I guess it is 0x38 in size because the execute-only area ends at 0x188


	base = 0xe8;
	temp16[(base) /2] = 0xE004; base += 2; // based on killbldp
	temp16[(base) /2] = 0xE52D; base += 2;
	temp16[(base) /2] = 0x00D3; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;
	temp16[(base) /2] = 0xE121; base += 2;
	temp16[(base) /2] = 0xE004; base += 2;
	temp16[(base) /2] = 0xE49D; base += 2;
	temp16[(base) /2] = 0xFF1E; base += 2;
	temp16[(base) /2] = 0xE12F; base += 2;

//  base = 0xfc; // already at 0xfc
	temp16[(base) /2] = 0xE004; base += 2; // based on killbldp
	temp16[(base) /2] = 0xE52D; base += 2;
	temp16[(base) /2] = 0x0013; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;
	temp16[(base) /2] = 0xE121; base += 2;
	temp16[(base) /2] = 0xE004; base += 2;
	temp16[(base) /2] = 0xE49D; base += 2;
	temp16[(base) /2] = 0xFF1E; base += 2;
	temp16[(base) /2] = 0xE12F; base += 2;

//  base = 0x110; // already at 0x110
//  temp16[(base) /2] = 0xff1e; base += 2;
//  temp16[(base) /2] = 0xe12f; base += 2;
//  temp16[(base) /2] = 0xf302; base += 2;
//  temp16[(base) /2] = 0xe3a0; base += 2;
	// set up stack again, soft-reset reset with a ram variable set to 0
	temp16[(base) /2] = 0x00D1; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;
	temp16[(base) /2] = 0xE121; base += 2;
	temp16[(base) /2] = 0xD0b8; base += 2;
	temp16[(base) /2] = 0xE59F; base += 2;
	temp16[(base) /2] = 0x00D3; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0xF000; base += 2;
	temp16[(base) /2] = 0xE121; base += 2;
	temp16[(base) /2] = 0xD0b0; base += 2;
	temp16[(base) /2] = 0xE59F; base += 2;
	temp16[(base) /2] = 0x10b8; base += 2;
	temp16[(base) /2] = 0xE59F; base += 2;
	temp16[(base) /2] = 0x0000; base += 2;
	temp16[(base) /2] = 0xE3A0; base += 2;
	temp16[(base) /2] = 0x0000; base += 2;
	temp16[(base) /2] = 0xE581; base += 2;
	if (is_svg == 0)
	{ // jump to start of external rom
		temp16[(base) / 2] = 0xF302; base += 2;
		temp16[(base) / 2] = 0xE3A0; base += 2;
	}
	else
	{
		temp16[(base) / 2] = 0xf0b0; base += 2;
		temp16[(base) / 2] = 0xe59f; base += 2;
	}



	base = 0x150;
	temp16[(base) /2] = 0xff1e; base += 2;
	temp16[(base) /2] = 0xe12f; base += 2;

	// the non-EO area starts in the middle of a function that seems similar to those at 000037E4 / 000037D4 in killbldp. by setting this up we allow the intro to run
	// it sets '0x10000038' to a value of 1
	base = 0x184;
	temp16[(base) /2] = 0x105c; base += 2;
	temp16[(base) /2] = 0xE59F; base += 2;
}

DRIVER_INIT_MEMBER(pgm_arm_type3_state,theglad)
{
	svg_basic_init();
	pgm_theglad_decrypt(machine());
	svg_latch_init();
//  pgm_create_dummy_internal_arm_region(0x188);

	pgm_create_dummy_internal_arm_region_theglad(0);


	machine().device("prot")->memory().space(AS_PROGRAM).install_read_handler(0x1000000c, 0x1000000f, read32_delegate(FUNC(pgm_arm_type3_state::theglad_speedup_r),this));
}


void pgm_arm_type3_state::pgm_patch_external_arm_rom_jumptable_theglada(int base)
{
	// we don't have the correct internal ROM for this version, so insead we use the one we have and patch the jump table in the external ROM
	UINT32 subroutine_addresses[] =
	{
		0x00FC, 0x00E8, 0x0110, 0x0150, 0x0194, 0x06C8, 0x071C, 0x0728,
		0x0734, 0x0740, 0x0784, 0x0794, 0x07FC, 0x0840, 0x086C, 0x0988,
		0x0A54, 0x0AA8, 0x0AD4, 0x0EB8, 0x0EF8, 0x0F2C, 0x0F3C, 0x0F78,
		0x0FA8, 0x0FD8, 0x1028, 0x1038, 0x1048, 0x1058, 0x1068, 0x1070,
		0x1090, 0x10B0, 0x10D4, 0x1100, 0x113C, 0x1198, 0x1234, 0x1258,
		0x127C, 0x12A8, 0x12E4, 0x1368, 0x142C, 0x0B10, 0x0B54, 0x0B74,
		0x0C08, 0x0C90, 0x0D18, 0x0D90, 0x1570, 0x1600, 0x1640, 0x1694,
		0x1730, 0x176C, 0x17AC, 0x17D8, 0x18C4, 0x18E0, 0x1904, 0x1930,
		0x19D8, 0x1A38, 0x1950, 0x1970, 0x1990, 0x19B8, 0x19C8, 0x1A9C,
		0x1AC4, 0x1AE8, 0x1B20, 0x1B48, 0x1B70, 0x1B8C, 0x1BB4, 0x1BD8,
		0x1BFC, 0x1C10, 0x1C24, 0x1CA0, 0x1D5C, 0x1D7C, 0x1D8C, 0x1DAC,
		0x1DCC, 0x1DE0, 0x1DF4, 0x1E1C, 0x1E2C, 0x1E60, 0x1E94, 0x1EA4,
		0x1ECC, 0x1ED8, 0x1EE4, 0x1F14, 0x1F44, 0x1FB4, 0x1FC4, 0x2040,
		0x20BC, 0x2140, 0x21C4, 0x2240, 0x22BC, 0x2340, 0x23C4, 0x23D0,
		0x2400, 0x2430, 0x244C, 0x245C, 0x246C, 0x2FCC, 0x3000, 0x3028,
		0x3050, 0x30A4, 0x30F8, 0x3120, 0x249C, 0x24C0, 0x27BC, 0x2B40,
		0x2BF4, 0x2CD8, 0x2E2C
	};
	UINT16 *extprot = (UINT16 *)memregion("user1")->base();
	/*
	0x00C8,0x00B4,0x00DC,0x011C,0x0160,0x02DC,0x0330,0x033C,
	0x0348,0x0354,0x0398,0x03A8,0x0410,0x0454,0x0480,0x059C,
	0x0668,0x06BC,0x06E8,0x0ACC,0x0B0C,0x0B40,0x0B50,0x0B8C,
	0x0BBC,0x0BEC,0x0C3C,0x0C4C,0x0C5C,0x0C6C,0x0C7C,0x0C84,
	0x0CA4,0x0CC4,0x0CE8,0x0D14,0x0D50,0x0DAC,0x0E48,0x0E6C,
	0x0E90,0x0EBC,0x0EF8,0x0F7C,0x1040,0x0724,0x0768,0x0788,
	0x081C,0x08A4,0x092C,0x09A4,0x1184,0x1214,0x1254,0x12A8,
	0x1344,0x1380,0x13C0,0x13EC,0x14D8,0x14F4,0x1518,0x1544,
	0x15EC,0x164C,0x1564,0x1584,0x15A4,0x15CC,0x15DC,0x16B0,
	0x16D8,0x16FC,0x1734,0x175C,0x1784,0x17A0,0x17C8,0x17EC,
	0x1810,0x1824,0x1838,0x18B4,0x1970,0x1990,0x19A0,0x19C0,
	0x19E0,0x19F4,0x1A08,0x1A30,0x1A40,0x1A74,0x1AA8,0x1AB8,
	0x1AE0,0x1AEC,0x1AF8,0x1B28,0x1B58,0x1BC8,0x1BD8,0x1C54,
	0x1CD0,0x1D54,0x1DD8,0x1E54,0x1ED0,0x1F54,0x1FD8,0x1FE4,
	0x2014,0x2044,0x2060,0x2070,0x2080,0x2BE0,0x2C14,0x2C3C,
	0x2C64,0x2CB8,0x2D0C,0x2D34,0x20B0,0x20D4,0x23D0,0x2754,
	0x2808,0x28EC,0x2A40
	*/


	for (int i = 0; i < 131; i++)
	{
//      UINT32 addr = extprot[(base/2)] | (extprot[(base/2) + 1] << 16);
		extprot[(base / 2)] = subroutine_addresses[i];

		base += 4;
//      printf("%04x (%08x)\n", subroutine_addresses[i], addr );
	}
}

DRIVER_INIT_MEMBER(pgm_arm_type3_state, theglada)
{
	DRIVER_INIT_CALL(theglad);

	pgm_patch_external_arm_rom_jumptable_theglada(0x82078);

}

INPUT_PORTS_START( theglad )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")    /* Region - actually supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x00ff, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	//PORT_CONFSETTING(      0x0002, DEF_STR( Japan ) ) // it doesn't appear that carts of the Japanese version were released, the PCB has an extra sample ROM used in Japanese mode for the music
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, "Spanish Territories" )
	PORT_CONFSETTING(      0x0006, DEF_STR( World ) )
	PORT_CONFSETTING(      0x00ff, "Don't Change" ) // don't hack the region
INPUT_PORTS_END

INPUT_PORTS_START( svg )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")    /* Region - actually supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x00ff, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, DEF_STR( Japan ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, "Spanish Territories" )
	PORT_CONFSETTING(      0x0006, DEF_STR( World ) )
	PORT_CONFSETTING(      0x00ff, "Don't Change" ) // don't hack the region
INPUT_PORTS_END

INPUT_PORTS_START( svgtw )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")    /* Region - actually supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x0001, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, DEF_STR( Japan ) )
	PORT_CONFSETTING(      0x0003, DEF_STR( Korea ) )
	PORT_CONFSETTING(      0x0004, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0005, "Spanish Territories" )
	PORT_CONFSETTING(      0x0006, DEF_STR( World ) )
	PORT_CONFSETTING(      0x00ff, "Don't Change" ) // don't hack the region
INPUT_PORTS_END

DRIVER_INIT_MEMBER(pgm_arm_type3_state,svg)
{
	svg_basic_init();
	pgm_svg_decrypt(machine());
	svg_latch_init();
	pgm_create_dummy_internal_arm_region_theglad(1);
	m_armrom = (UINT32 *)memregion("prot")->base();
	machine().device("prot")->memory().space(AS_PROGRAM).install_read_handler(0xB90, 0xB93, read32_delegate(FUNC(pgm_arm_type3_state::svg_speedup_r),this));


}

DRIVER_INIT_MEMBER(pgm_arm_type3_state,svgpcb)
{
	svg_basic_init();
	pgm_svgpcb_decrypt(machine());
	svg_latch_init();
	pgm_create_dummy_internal_arm_region_theglad(0);
	m_armrom = (UINT32 *)memregion("prot")->base();
	machine().device("prot")->memory().space(AS_PROGRAM).install_read_handler(0x9e0, 0x9e3, read32_delegate(FUNC(pgm_arm_type3_state::svgpcb_speedup_r),this));

}


READ32_MEMBER(pgm_arm_type3_state::killbldp_speedup_r )
{
	int pc = space.device().safe_pc();
	if (pc == 0x7d8) space.device().execute().eat_cycles(500);
	//else printf("killbldp_speedup_r %08x\n", pc);
	return m_arm_ram2[0x00c/4];
}

DRIVER_INIT_MEMBER(pgm_arm_type3_state,killbldp)
{
	svg_basic_init();
	pgm_killbldp_decrypt(machine());
	svg_latch_init();

	machine().device("prot")->memory().space(AS_PROGRAM).install_read_handler(0x1000000c, 0x1000000f, read32_delegate(FUNC(pgm_arm_type3_state::killbldp_speedup_r),this));

//  UINT16 *temp16 = (UINT16 *)memregion("prot")->base();
//  int base = 0xfc; // startup table uploads
//  temp16[(base) /2] = 0x0000; base += 2;
//  temp16[(base) /2] = 0xE1A0; base += 2;

//  base = 0xd4; // startup table uploads
//  temp16[(base) /2] = 0x0000; base += 2;
//  temp16[(base) /2] = 0xE1A0; base += 2;
//
//  base = 0x120; // reset game state, uncomment this to break boot sequence how theglad was broken...
//  temp16[(base) /2] = 0x0000; base += 2;
//  temp16[(base) /2] = 0xE1A0; base += 2;

}

READ32_MEMBER(pgm_arm_type3_state::dmnfrnt_speedup_r )
{
	int pc = space.device().safe_pc();
	if (pc == 0x8000fea) space.device().execute().eat_cycles(500);
//  else printf("dmn_speedup_r %08x\n", pc);
	return m_arm_ram[0x000444/4];
}

READ16_MEMBER(pgm_arm_type3_state::dmnfrnt_main_speedup_r )
{
	UINT16 data = m_mainram[0xa03c/2];
	int pc = space.device().safe_pc();
	if (pc == 0x10193a) space.device().execute().spin_until_interrupt();
	else if (pc == 0x1019a4) space.device().execute().spin_until_interrupt();
	return data;
}

DRIVER_INIT_MEMBER(pgm_arm_type3_state,dmnfrnt)
{
	svg_basic_init();
	pgm_dfront_decrypt(machine());
	svg_latch_init();

	/* put some fake code for the ARM here ... */
	pgm_create_dummy_internal_arm_region(0x4000);

	machine().device("prot")->memory().space(AS_PROGRAM).install_read_handler(0x18000444, 0x18000447, read32_delegate(FUNC(pgm_arm_type3_state::dmnfrnt_speedup_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x80a03c, 0x80a03d, read16_delegate(FUNC(pgm_arm_type3_state::dmnfrnt_main_speedup_r),this));

	m_svg_ram_sel = 1;

	// the internal rom probably also supplies the region here
	// we have to copy it to both shared ram regions because it reads from a different one before the attract story?
	// could be a timing error? or shared ram behavior isn't how we think it is?
	UINT16 *share16;
	share16 = (UINT16 *)(m_svg_shareram[1]);
	share16[0x158/2] = 0x0005;
	share16 = (UINT16 *)(m_svg_shareram[0]);
	share16[0x158/2] = 0x0005;
}

//
// int j = BITSWAP24(i, 23, 20, 17, 16, 19, 18, 15, 14, 13, 12, 11, 10, 9, 22, 21, 6, 7, 6, 5, 4, 3, 2, 1, 0);
// buffer[i] = src[j]

// todo, collapse these to an address swap
void pgm_arm_type3_state::pgm_descramble_happy6(UINT8* src)
{
	dynamic_buffer buffer(0x800000);
	int writeaddress = 0;

	for (int j = 0; j < 0x800; j += 0x200)
	{
		for (int i = j; i < 0x800000; i += 0x800)
		{
			memcpy(&buffer[writeaddress], src + i, 0x200);
			writeaddress += 0x200;
		}
	}
	memcpy(src, &buffer[0], 0x800000);
}



void pgm_arm_type3_state::pgm_descramble_happy6_2(UINT8* src)
{
	dynamic_buffer buffer(0x800000);
	int writeaddress = 0;
	for (int k = 0; k < 0x800000; k += 0x100000)
	{
		for (int j = 0; j < 0x40000; j += 0x10000)
		{
			for (int i = j; i < 0x100000; i += 0x40000)
			{
				memcpy(&buffer[writeaddress], src + i + k, 0x10000);
				writeaddress += 0x10000;
			}
		}
	}
	memcpy(src, &buffer[0], 0x800000);
}

INPUT_PORTS_START( happy6 )
	PORT_INCLUDE ( pgm )

	PORT_START("RegionHack")    /* Region - actually supplied by protection device */
	PORT_CONFNAME( 0x00ff, 0x0000, DEF_STR( Region ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( China ) )
	PORT_CONFSETTING(      0x0001, DEF_STR( Taiwan ) )
	PORT_CONFSETTING(      0x0002, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(      0x0003, "Singapore" )
	PORT_CONFSETTING(      0x0004, "Oversea" ) // unlikely this actually exists, there's no english anything!
	PORT_CONFSETTING(      0x00ff, "Don't Change" ) // don't hack the region
INPUT_PORTS_END

DRIVER_INIT_MEMBER(pgm_arm_type3_state,happy6)
{
	UINT8 *src;

	src = (UINT8 *)(machine().root_device().memregion("tiles")->base()) + 0x180000;
	pgm_descramble_happy6(src);
	pgm_descramble_happy6_2(src);

	src = (UINT8 *)(machine().root_device().memregion("sprcol")->base()) + 0x000000;
	pgm_descramble_happy6(src);
	pgm_descramble_happy6_2(src);

	src = (UINT8 *)(machine().root_device().memregion("sprcol")->base()) + 0x0800000;
	pgm_descramble_happy6(src);
	pgm_descramble_happy6_2(src);

	src = (UINT8 *)(machine().root_device().memregion("sprmask")->base());
	pgm_descramble_happy6(src);
	pgm_descramble_happy6_2(src);

	src = (UINT8 *)(machine().root_device().memregion("ics")->base()) + 0x400000;
	pgm_descramble_happy6(src);
	pgm_descramble_happy6_2(src);

	svg_basic_init();
	pgm_happy6_decrypt(machine());
	svg_latch_init();
	pgm_create_dummy_internal_arm_region_theglad(0);

	machine().device("prot")->memory().space(AS_PROGRAM).install_read_handler(0x1000000c, 0x1000000f, read32_delegate(FUNC(pgm_arm_type3_state::happy6_speedup_r),this));
}
