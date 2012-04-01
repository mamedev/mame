/***********************************************************************
 PGM IGA027A (55857G* type) ARM protection emulation
  *guess, the part number might not be directly tied to behavior, see note below

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

static WRITE32_HANDLER( svg_arm7_ram_sel_w )
{
//  printf("svg_arm7_ram_sel_w %08x\n", data);
	space->machine().scheduler().synchronize(); // force resync

	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();
	state->m_svg_ram_sel = data & 1;
}

static READ32_HANDLER( svg_arm7_shareram_r )
{
	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();
	return state->m_svg_shareram[state->m_svg_ram_sel & 1][offset];
}

static WRITE32_HANDLER( svg_arm7_shareram_w )
{
	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();
	COMBINE_DATA(&state->m_svg_shareram[state->m_svg_ram_sel & 1][offset]);
}

static READ16_HANDLER( svg_m68k_ram_r )
{
	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();
	int ram_sel = (state->m_svg_ram_sel & 1) ^ 1;
	UINT16 *share16 = (UINT16 *)(state->m_svg_shareram[ram_sel & 1]);

	return share16[BYTE_XOR_LE(offset)];
}

static WRITE16_HANDLER( svg_m68k_ram_w )
{
	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();
	int ram_sel = (state->m_svg_ram_sel & 1) ^ 1;
	UINT16 *share16 = (UINT16 *)(state->m_svg_shareram[ram_sel & 1]);

	COMBINE_DATA(&share16[BYTE_XOR_LE(offset)]);
}

static READ16_HANDLER( svg_68k_nmi_r )
{
	return 0;
}

static WRITE16_HANDLER( svg_68k_nmi_w )
{
	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();
	generic_pulse_irq_line(state->m_prot, ARM7_FIRQ_LINE, 1);
}

static WRITE16_HANDLER( svg_latch_68k_w )
{
	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();
	if (PGMARM7LOGERROR)
		logerror("M68K: Latch write: %04x (%04x) (%06x)\n", data & 0x0000ffff, mem_mask, cpu_get_pc(&space->device()));
	COMBINE_DATA(&state->m_svg_latchdata_68k_w);
}


static READ16_HANDLER( svg_latch_68k_r )
{
	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();

	if (PGMARM7LOGERROR)
		logerror("M68K: Latch read: %04x (%04x) (%06x)\n", state->m_svg_latchdata_arm_w & 0x0000ffff, mem_mask, cpu_get_pc(&space->device()));
	return state->m_svg_latchdata_arm_w;
}



static READ32_HANDLER( svg_latch_arm_r )
{
	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: Latch read: %08x (%08x) (%06x)\n", state->m_svg_latchdata_68k_w, mem_mask, cpu_get_pc(&space->device()));
	return state->m_svg_latchdata_68k_w;
}

static WRITE32_HANDLER( svg_latch_arm_w )
{
	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();

	if (PGMARM7LOGERROR)
		logerror("ARM7: Latch write: %08x (%08x) (%06x)\n", data, mem_mask, cpu_get_pc(&space->device()));

	COMBINE_DATA(&state->m_svg_latchdata_arm_w);
}

/* 55857G? */
/* Demon Front, The Gladiator, Happy 6-in-1, Spectral Vs. Generation, Killing Blade EX */
/*  the ones with an EXECUTE ONLY region of ARM space? */
static ADDRESS_MAP_START( svg_68k_mem, AS_PROGRAM, 16, pgm_arm_type3_state )
	AM_IMPORT_FROM(pgm_mem)
	AM_RANGE(0x100000, 0x1fffff) AM_ROMBANK("bank1")  /* Game ROM */

	AM_RANGE(0x500000, 0x51ffff) AM_READWRITE_LEGACY(svg_m68k_ram_r, svg_m68k_ram_w)    /* ARM7 Shared RAM */
	AM_RANGE(0x5c0000, 0x5c0001) AM_READWRITE_LEGACY(svg_68k_nmi_r, svg_68k_nmi_w)      /* ARM7 FIQ */
	AM_RANGE(0x5c0300, 0x5c0301) AM_READWRITE_LEGACY(svg_latch_68k_r, svg_latch_68k_w) /* ARM7 Latch */
ADDRESS_MAP_END


static ADDRESS_MAP_START( 55857G_arm7_map, AS_PROGRAM, 32, pgm_arm_type3_state )
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
	AM_RANGE(0x08000000, 0x087fffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x10000000, 0x100003ff) AM_RAM
	AM_RANGE(0x18000000, 0x1803ffff) AM_RAM AM_BASE( m_arm_ram)
	AM_RANGE(0x38000000, 0x3801ffff) AM_READWRITE_LEGACY(svg_arm7_shareram_r, svg_arm7_shareram_w)
	AM_RANGE(0x48000000, 0x48000003) AM_READWRITE_LEGACY(svg_latch_arm_r, svg_latch_arm_w) /* 68k Latch */
	AM_RANGE(0x40000018, 0x4000001b) AM_WRITE_LEGACY(svg_arm7_ram_sel_w) /* RAM SEL */
	AM_RANGE(0x50000000, 0x500003ff) AM_RAM
ADDRESS_MAP_END


MACHINE_START( pgm_arm_type3 )
{
	MACHINE_START_CALL(pgm);
	pgm_arm_type3_state *state = machine.driver_data<pgm_arm_type3_state>();

	state->m_prot = machine.device<cpu_device>("prot");

	/* register type specific Save State stuff here */
}


/******* ARM 55857G *******/

MACHINE_CONFIG_START( pgm_arm_type3, pgm_arm_type3_state )
	MCFG_FRAGMENT_ADD(pgmbase)

	MCFG_MACHINE_START( pgm_arm_type3 )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(svg_68k_mem)

	/* protection CPU */
	MCFG_CPU_ADD("prot", ARM7, 33333333)	// 55857G
	MCFG_CPU_PROGRAM_MAP(55857G_arm7_map)

MACHINE_CONFIG_END



static void svg_basic_init(running_machine &machine)
{
	pgm_arm_type3_state *state = machine.driver_data<pgm_arm_type3_state>();

	pgm_basic_init(machine);
	state->m_svg_shareram[0] = auto_alloc_array(machine, UINT32, 0x10000 / 4);
	state->m_svg_shareram[1] = auto_alloc_array(machine, UINT32, 0x10000 / 4);
	state->m_svg_ram_sel = 0;

	state->save_pointer(NAME(state->m_svg_shareram[0]), 0x10000 / 4);
	state->save_pointer(NAME(state->m_svg_shareram[1]), 0x10000 / 4);
	state->save_item(NAME(state->m_svg_ram_sel));
}

static void pgm_create_dummy_internal_arm_region(running_machine &machine)
{
	UINT16 *temp16 = (UINT16 *)machine.region("prot")->base();

	// fill with RX 14
	int i;
	for (i=0;i<0x4000/2;i+=2)
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



static void svg_latch_init( running_machine &machine )
{
	pgm_arm_type3_state *state = machine.driver_data<pgm_arm_type3_state>();

	state->m_svg_latchdata_68k_w = 0;
	state->m_svg_latchdata_arm_w = 0;

	state->save_item(NAME(state->m_svg_latchdata_68k_w));
	state->save_item(NAME(state->m_svg_latchdata_arm_w));
}


DRIVER_INIT( theglad )
{
	svg_basic_init(machine);
	pgm_theglad_decrypt(machine);
	svg_latch_init(machine);
	pgm_create_dummy_internal_arm_region(machine);
}

DRIVER_INIT( svg )
{
	svg_basic_init(machine);
	pgm_svg_decrypt(machine);
	svg_latch_init(machine);
	pgm_create_dummy_internal_arm_region(machine);
}

DRIVER_INIT( svgpcb )
{
	svg_basic_init(machine);
	pgm_svgpcb_decrypt(machine);
	svg_latch_init(machine);
	pgm_create_dummy_internal_arm_region(machine);
}

DRIVER_INIT( killbldp )
{
	svg_basic_init(machine);
	pgm_killbldp_decrypt(machine);
	svg_latch_init(machine);
}

static READ32_HANDLER( dmnfrnt_speedup_r )
{
	pgm_arm_type3_state *state = space->machine().driver_data<pgm_arm_type3_state>();
	int pc = cpu_get_pc(&space->device());
	if (pc == 0x8000fea) device_eat_cycles(&space->device(), 500);
//  else printf("dmn_speedup_r %08x\n", pc);
	return state->m_arm_ram[0x000444/4];
}

static READ16_HANDLER( dmnfrnt_main_speedup_r )
{
	UINT16 data = pgm_mainram[0xa03c/2];
	int pc = cpu_get_pc(&space->device());
	if (pc == 0x10193a) device_spin_until_interrupt(&space->device());
	else if (pc == 0x1019a4) device_spin_until_interrupt(&space->device());
	return data;
}

DRIVER_INIT( dmnfrnt )
{
	pgm_arm_type3_state *state = machine.driver_data<pgm_arm_type3_state>();

	svg_basic_init(machine);
	pgm_dfront_decrypt(machine);
	svg_latch_init(machine);

	/* put some fake code for the ARM here ... */
	pgm_create_dummy_internal_arm_region(machine);

	machine.device("prot")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x18000444, 0x18000447, FUNC(dmnfrnt_speedup_r));
	machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_read_handler(0x80a03c, 0x80a03d, FUNC(dmnfrnt_main_speedup_r));

	state->m_svg_ram_sel = 1;

	// the internal rom probably also supplies the region here
	// we have to copy it to both shared ram regions because it reads from a different one before the attract story?
	// could be a timing error? or shared ram behavior isn't how we think it is?
	UINT16 *share16;
	share16 = (UINT16 *)(state->m_svg_shareram[1]);
	share16[0x158/2] = 0x0005;
	share16 = (UINT16 *)(state->m_svg_shareram[0]);
	share16[0x158/2] = 0x0005;
}

DRIVER_INIT( happy6 )
{
	svg_basic_init(machine);
	pgm_happy6_decrypt(machine);
	svg_latch_init(machine);
	pgm_create_dummy_internal_arm_region(machine);
}


