/***********************************************************************
 PGM IGS027A ARM protection emulation

 These all use the 55857G type chips

 these are emulation of the 'dmnfrnt' type ARM device
 used by

 Demon Front (dmnfrnt) *1
 The Gladiator (theglad)
 Spectral vs. Generation (svg)
 Happy 6-in-1 (happy6)
 The Killing Blade Plus (killbldp) *2

 None of these work at all, with the following exception.

 *1 - We bypass the internal ROM entirely! Game doesn't jump back
 *2 - Partial dump of the internal ROM is used, but 'Execute Only' region is missing

 ----

 These games use a large amount of shared RAM which is banked between CPUs

 ----

 All of these games have an external ARM rom.

 The internal ROMs contain an 'execute only' region at the start of the
 ROM which prevents reading out.

 IGS027A type 55857G has also been seen on various IGS gambling boards
 as the main CPU (eg. Haunted House, see igs_m027a)

 55857G is also used on the Cave single board PGM systems, but in those
 cases it behaves like the 55857E (pgmprot1.c)

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
//	printf("(%08x) ARM7: shared read (bank %02x) offset - %08x retdata - %08x mask - %08x\n", space.device().safe_pc(), m_svg_ram_sel, offset*4, retdata, mem_mask );
	return retdata;
}

WRITE32_MEMBER(pgm_arm_type3_state::svg_arm7_shareram_w )
{
//	printf("(%08x) ARM7: shared write (bank %02x) offset - %08x retdata - %08x mask - %08x\n", space.device().safe_pc(), m_svg_ram_sel, offset*4, data, mem_mask );
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

//	AM_RANGE(0xc0000000, 0xffffffff) AM_RAM

ADDRESS_MAP_END


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
	MCFG_CPU_ADD("prot", ARM7, 33333333)    // 55857G
	MCFG_CPU_PROGRAM_MAP(55857G_arm7_map)

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


DRIVER_INIT_MEMBER(pgm_arm_type3_state,theglad)
{
	svg_basic_init();
	pgm_theglad_decrypt(machine());
	svg_latch_init();
//	pgm_create_dummy_internal_arm_region(0x188);

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
	temp16[(base) /2] = 0x0010; base += 2;
	temp16[(base) /2] = 0x0800; base += 2;
	temp16[(base) /2] = 0x0010; base += 2;
	temp16[(base) /2] = 0x0800; base += 2;
	 

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



	temp16[(base) /2] = 0x0028; base += 2; // jump to 0x184
	temp16[(base) /2] = 0xEA00; base += 2;


	
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
	// 0x110 is called after the 'continue' screen, I suspect it is unique code to the gladiator, probably 0x40 bytes long due to next entry being at 0x150
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

//	base = 0xfc; // already at 0xfc
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

//	base = 0x110; // already at 0x110
	temp16[(base) /2] = 0xff1e; base += 2;
	temp16[(base) /2] = 0xe12f; base += 2;

	base = 0x150;
	temp16[(base) /2] = 0xff1e; base += 2;
	temp16[(base) /2] = 0xe12f; base += 2;

	// the non-EO area starts in the middle of a function that seems similar to those  at 000037E4 / 000037D4 in killbldp.. by setting this up we allow the intro to run, but can no longer can get in game..
	// it sets '0x10000038' to a value ot 1
	// maybe this flag needs to be flipped on interrupts or similar??
	base = 0x184;
	temp16[(base) /2] = 0x105c; base += 2;
	temp16[(base) /2] = 0xE59F; base += 2;

	// this is the location of the region in the internal rom, for some reaosn Japan doesn't play attract music (original game feature? bad code flow?)
	base = 0x3316;
	temp16[(base) / 2] = 0x0006; base += 2;


#if 0
	m_svg_ram_sel = 1;

	for (int i = 0; i < 0x8000; i++)
	{
		UINT16 *share16;
		share16 = (UINT16 *)(m_svg_shareram[1]);
		share16[i / 2] = 0x0003;
		share16 = (UINT16 *)(m_svg_shareram[0]);
		share16[i / 2] = 0x0003;
	}
#endif

	machine().device("prot")->memory().space(AS_PROGRAM).install_read_handler(0x1000000c, 0x1000000f, read32_delegate(FUNC(pgm_arm_type3_state::theglad_speedup_r),this));
}

DRIVER_INIT_MEMBER(pgm_arm_type3_state,svg)
{
	svg_basic_init();
	pgm_svg_decrypt(machine());
	svg_latch_init();
	pgm_create_dummy_internal_arm_region(0x4000);
}

DRIVER_INIT_MEMBER(pgm_arm_type3_state,svgpcb)
{
	svg_basic_init();
	pgm_svgpcb_decrypt(machine());
	svg_latch_init();
	pgm_create_dummy_internal_arm_region(0x4000);
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

	UINT16 *temp16 = (UINT16 *)memregion("prot")->base();
	int base = 0xfc;
	temp16[(base) /2] = 0x0000; base += 2;	
	temp16[(base) /2] = 0xE1A0; base += 2;	
	
	base = 0xd4;
	temp16[(base) /2] = 0x0000; base += 2;	
	temp16[(base) /2] = 0xE1A0; base += 2;	

//	base = 0x120;
//	temp16[(base) /2] = 0x0000; base += 2;	
//	temp16[(base) /2] = 0xE1A0; base += 2;	

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

DRIVER_INIT_MEMBER(pgm_arm_type3_state,happy6)
{
	svg_basic_init();
	pgm_happy6_decrypt(machine());
	svg_latch_init();
	pgm_create_dummy_internal_arm_region(0x4000);
}
