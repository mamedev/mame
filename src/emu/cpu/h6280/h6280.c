/*****************************************************************************

    h6280.c - Portable HuC6280 emulator

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.


    NOTICE:

    This code is around 99% complete!  Several things are unimplemented,
    some due to lack of time, some due to lack of documentation, mainly
    due to lack of programs using these features.

    csh, csl opcodes are not supported.

    I am unsure if flag B is set upon execution of rti.

    Cycle counts should be quite accurate.


    Changelog, version 1.02:
        JMP + indirect X (0x7c) opcode fixed.
        SMB + RMB opcodes fixed in disassembler.
        change_pc function calls removed.
        TSB & TRB now set flags properly.
        BIT opcode altered.

    Changelog, version 1.03:
        Swapped IRQ mask for IRQ1 & IRQ2 (thanks Yasuhiro)

    Changelog, version 1.04, 28/9/99-22/10/99:
        Adjusted RTI (thanks Karl)
        TST opcodes fixed in disassembler (missing break statements in a case!).
        TST behaviour fixed.
        SMB/RMB/BBS/BBR fixed in disassembler.

    Changelog, version 1.05, 8/12/99-16/12/99:
        Added CAB's timer implementation (note: irq ack & timer reload are changed).
        Fixed STA IDX.
        Fixed B flag setting on BRK.
        Assumed CSH & CSL to take 2 cycles each.

        Todo:  Performance could be improved by precalculating timer fire position.

    Changelog, version 1.06, 4/5/00 - last opcode bug found?
        JMP indirect was doing a EAL++; instead of EAD++; - Obviously causing
        a corrupt read when L = 0xff!  This fixes Bloody Wolf and Trio The Punch!

    Changelog, version 1.07, 3/9/00:
        Changed timer to be single shot - fixes Crude Buster music in level 1.

    Changelog, version 1.08, 8/11/05: (Charles MacDonald)

        Changed timer implementation, no longer single shot and reading the
        timer registers returns the count only. Fixes the following:
        - Mesopotamia: Music tempo & in-game timer
        - Dragon Saber: DDA effects
        - Magical Chase: Music tempo and speed regulation
        - Cadash: Allows the first level to start
        - Turrican: Allows the game to start

        Changed PLX and PLY to set NZ flags. Fixes:
        - Afterburner: Graphics unpacking
        - Aoi Blink: Collision detection with background

        Fixed the decimal version of ADC/SBC to *not* update the V flag,
        only the binary ones do.

        Fixed B flag handling so it is always set outside of an interrupt;
        even after being set by PLP and RTI.

        Fixed P state after reset to set I and B, leaving T, D cleared and
        NVZC randomized (cleared in this case).

        Fixed interrupt processing order (Timer has highest priority followed
        by IRQ1 and finally IRQ2).

    Changelog, version 1.09, 1/07/06: (Rob Bohms)

        Added emulation of the T flag, fixes PCE Ankuku Densetsu title screen

    Changelog, version 1.10, 5/09/07: (Wilbert Pol)

        - Taking of interrupts is delayed to respect a pending instruction already
          in the instruction pipeline; fixes After Burner.
        - Added 1 cycle for decimal mode ADC and SBC instructions.
        - Changed cycle counts for CSH and CSL instructions to 3.
        - Added T flag support to the SBC instruction.
        - Fixed ADC T flag to set the Z flag based on the value read.
        - Added 3 cycle penalty to ADC, AND, EOR, ORA, and SBC instructions
          when the T flag is set.
        - Fixed cycle count and support for 65536 byte blocks for the TAI, TDD,
          TIA, TII, and TIN instructions.
        - Fixed RDWORD macro in the disassembler.
        - Fixed setting of N and V flags in the TST instructions.
        - Removed unneeded debug_mmr code.
        - Fixed TSB and TRB instructions.
        - Added 1 delay when accessing the VDC or VCE areas.
        - Implemented low and high speed cpu modes.

    Changelog, version 1.11, 18/09/07: (Wilbert Pol)

        - Improvements to the handling of taking of delayed interrupts.

******************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "h6280.h"

//static void set_irq_line(h6280_Regs* cpustate, int irqline, int state);

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

const device_type H6280 = &device_creator<h6280_device>;

//-------------------------------------------------
//  h6280_device - constructor
//-------------------------------------------------

h6280_device::h6280_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, H6280, "H6280", tag, owner, clock),
	m_program_config("program", ENDIANNESS_LITTLE, 8, 21),
	m_io_config("io", ENDIANNESS_LITTLE, 8, 2)
{
	// build the opcode table
	for (int op = 0; op < 256; op++)
		m_opcode[op] = s_opcodetable[op];
}


const h6280_device::ophandler h6280_device::s_opcodetable[256] =
{
	&h6280_device::op_000, &h6280_device::op_001, &h6280_device::op_002, &h6280_device::op_003, &h6280_device::op_004, &h6280_device::op_005, &h6280_device::op_006, &h6280_device::op_007,
	&h6280_device::op_008, &h6280_device::op_009, &h6280_device::op_00a, &h6280_device::op_00b, &h6280_device::op_00c, &h6280_device::op_00d, &h6280_device::op_00e, &h6280_device::op_00f,
	&h6280_device::op_010, &h6280_device::op_011, &h6280_device::op_012, &h6280_device::op_013, &h6280_device::op_014, &h6280_device::op_015, &h6280_device::op_016, &h6280_device::op_017,
	&h6280_device::op_018, &h6280_device::op_019, &h6280_device::op_01a, &h6280_device::op_01b, &h6280_device::op_01c, &h6280_device::op_01d, &h6280_device::op_01e, &h6280_device::op_01f,
	&h6280_device::op_020, &h6280_device::op_021, &h6280_device::op_022, &h6280_device::op_023, &h6280_device::op_024, &h6280_device::op_025, &h6280_device::op_026, &h6280_device::op_027,
	&h6280_device::op_028, &h6280_device::op_029, &h6280_device::op_02a, &h6280_device::op_02b, &h6280_device::op_02c, &h6280_device::op_02d, &h6280_device::op_02e, &h6280_device::op_02f,
	&h6280_device::op_030, &h6280_device::op_031, &h6280_device::op_032, &h6280_device::op_033, &h6280_device::op_034, &h6280_device::op_035, &h6280_device::op_036, &h6280_device::op_037,
	&h6280_device::op_038, &h6280_device::op_039, &h6280_device::op_03a, &h6280_device::op_03b, &h6280_device::op_03c, &h6280_device::op_03d, &h6280_device::op_03e, &h6280_device::op_03f,
	&h6280_device::op_040, &h6280_device::op_041, &h6280_device::op_042, &h6280_device::op_043, &h6280_device::op_044, &h6280_device::op_045, &h6280_device::op_046, &h6280_device::op_047,
	&h6280_device::op_048, &h6280_device::op_049, &h6280_device::op_04a, &h6280_device::op_04b, &h6280_device::op_04c, &h6280_device::op_04d, &h6280_device::op_04e, &h6280_device::op_04f,
	&h6280_device::op_050, &h6280_device::op_051, &h6280_device::op_052, &h6280_device::op_053, &h6280_device::op_054, &h6280_device::op_055, &h6280_device::op_056, &h6280_device::op_057,
	&h6280_device::op_058, &h6280_device::op_059, &h6280_device::op_05a, &h6280_device::op_05b, &h6280_device::op_05c, &h6280_device::op_05d, &h6280_device::op_05e, &h6280_device::op_05f,
	&h6280_device::op_060, &h6280_device::op_061, &h6280_device::op_062, &h6280_device::op_063, &h6280_device::op_064, &h6280_device::op_065, &h6280_device::op_066, &h6280_device::op_067,
	&h6280_device::op_068, &h6280_device::op_069, &h6280_device::op_06a, &h6280_device::op_06b, &h6280_device::op_06c, &h6280_device::op_06d, &h6280_device::op_06e, &h6280_device::op_06f,
	&h6280_device::op_070, &h6280_device::op_071, &h6280_device::op_072, &h6280_device::op_073, &h6280_device::op_074, &h6280_device::op_075, &h6280_device::op_076, &h6280_device::op_077,
	&h6280_device::op_078, &h6280_device::op_079, &h6280_device::op_07a, &h6280_device::op_07b, &h6280_device::op_07c, &h6280_device::op_07d, &h6280_device::op_07e, &h6280_device::op_07f,
	&h6280_device::op_080, &h6280_device::op_081, &h6280_device::op_082, &h6280_device::op_083, &h6280_device::op_084, &h6280_device::op_085, &h6280_device::op_086, &h6280_device::op_087,
	&h6280_device::op_088, &h6280_device::op_089, &h6280_device::op_08a, &h6280_device::op_08b, &h6280_device::op_08c, &h6280_device::op_08d, &h6280_device::op_08e, &h6280_device::op_08f,
	&h6280_device::op_090, &h6280_device::op_091, &h6280_device::op_092, &h6280_device::op_093, &h6280_device::op_094, &h6280_device::op_095, &h6280_device::op_096, &h6280_device::op_097,
	&h6280_device::op_098, &h6280_device::op_099, &h6280_device::op_09a, &h6280_device::op_09b, &h6280_device::op_09c, &h6280_device::op_09d, &h6280_device::op_09e, &h6280_device::op_09f,
	&h6280_device::op_0a0, &h6280_device::op_0a1, &h6280_device::op_0a2, &h6280_device::op_0a3, &h6280_device::op_0a4, &h6280_device::op_0a5, &h6280_device::op_0a6, &h6280_device::op_0a7,
	&h6280_device::op_0a8, &h6280_device::op_0a9, &h6280_device::op_0aa, &h6280_device::op_0ab, &h6280_device::op_0ac, &h6280_device::op_0ad, &h6280_device::op_0ae, &h6280_device::op_0af,
	&h6280_device::op_0b0, &h6280_device::op_0b1, &h6280_device::op_0b2, &h6280_device::op_0b3, &h6280_device::op_0b4, &h6280_device::op_0b5, &h6280_device::op_0b6, &h6280_device::op_0b7,
	&h6280_device::op_0b8, &h6280_device::op_0b9, &h6280_device::op_0ba, &h6280_device::op_0bb, &h6280_device::op_0bc, &h6280_device::op_0bd, &h6280_device::op_0be, &h6280_device::op_0bf,
	&h6280_device::op_0c0, &h6280_device::op_0c1, &h6280_device::op_0c2, &h6280_device::op_0c3, &h6280_device::op_0c4, &h6280_device::op_0c5, &h6280_device::op_0c6, &h6280_device::op_0c7,
	&h6280_device::op_0c8, &h6280_device::op_0c9, &h6280_device::op_0ca, &h6280_device::op_0cb, &h6280_device::op_0cc, &h6280_device::op_0cd, &h6280_device::op_0ce, &h6280_device::op_0cf,
	&h6280_device::op_0d0, &h6280_device::op_0d1, &h6280_device::op_0d2, &h6280_device::op_0d3, &h6280_device::op_0d4, &h6280_device::op_0d5, &h6280_device::op_0d6, &h6280_device::op_0d7,
	&h6280_device::op_0d8, &h6280_device::op_0d9, &h6280_device::op_0da, &h6280_device::op_0db, &h6280_device::op_0dc, &h6280_device::op_0dd, &h6280_device::op_0de, &h6280_device::op_0df,
	&h6280_device::op_0e0, &h6280_device::op_0e1, &h6280_device::op_0e2, &h6280_device::op_0e3, &h6280_device::op_0e4, &h6280_device::op_0e5, &h6280_device::op_0e6, &h6280_device::op_0e7,
	&h6280_device::op_0e8, &h6280_device::op_0e9, &h6280_device::op_0ea, &h6280_device::op_0eb, &h6280_device::op_0ec, &h6280_device::op_0ed, &h6280_device::op_0ee, &h6280_device::op_0ef,
	&h6280_device::op_0f0, &h6280_device::op_0f1, &h6280_device::op_0f2, &h6280_device::op_0f3, &h6280_device::op_0f4, &h6280_device::op_0f5, &h6280_device::op_0f6, &h6280_device::op_0f7,
	&h6280_device::op_0f8, &h6280_device::op_0f9, &h6280_device::op_0fa, &h6280_device::op_0fb, &h6280_device::op_0fc, &h6280_device::op_0fd, &h6280_device::op_0fe, &h6280_device::op_0ff
};

void h6280_device::device_start()
{
	// register our state for the debugger
	state_add(STATE_GENPC,    	"GENPC",		m_pc.w.l).noshow();
	state_add(STATE_GENFLAGS, 	"GENFLAGS",		m_p).callimport().callexport().formatstr("%8s").noshow();
	state_add(H6280_PC,       	"PC",    		m_pc.d).mask(0xffff);
	state_add(H6280_S,        	"S",        	m_sp.b.l).mask(0xff);
	state_add(H6280_P,        	"P",     		m_p).mask(0xff);
	state_add(H6280_A,        	"A",        	m_a).mask(0xff);
	state_add(H6280_X,        	"X",        	m_x).mask(0xff);
	state_add(H6280_Y,        	"Y",        	m_y).mask(0xff);
	state_add(H6280_IRQ_MASK, 	"IM",        	m_irq_mask).mask(0xff);
	state_add(H6280_TIMER_STATE,"TMR",        	m_timer_status).mask(0xff);
	state_add(H6280_NMI_STATE,  "NMI",        	m_nmi_state).mask(0xf);
	state_add(H6280_IRQ1_STATE, "IRQ1",        m_irq_state[0]).mask(0xf);
	state_add(H6280_IRQ2_STATE, "IRQ2",        m_irq_state[1]).mask(0xf);
	state_add(H6280_IRQT_STATE, "IRQT",        m_irq_state[2]).mask(0xf);
	state_add(H6280_M1,			"M1",        	m_mmr[0]).mask(0xff);
	state_add(H6280_M2,			"M2",        	m_mmr[1]).mask(0xff);
	state_add(H6280_M3,			"M3",        	m_mmr[2]).mask(0xff);
	state_add(H6280_M4,			"M4",        	m_mmr[3]).mask(0xff);
	state_add(H6280_M5,			"M5",        	m_mmr[4]).mask(0xff);
	state_add(H6280_M6,			"M6",        	m_mmr[5]).mask(0xff);
	state_add(H6280_M7,			"M7",        	m_mmr[6]).mask(0xff);
	state_add(H6280_M8,			"M8",        	m_mmr[7]).mask(0xff);

	save_item(NAME(m_ppc.w.l));
	save_item(NAME(m_pc.w.l));
	save_item(NAME(m_sp.w.l));
	save_item(NAME(m_zp.w.l));
	save_item(NAME(m_ea.w.l));
	save_item(NAME(m_a));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_p));
	save_item(NAME(m_mmr));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_timer_status));
	save_item(NAME(m_timer_ack));
	save_item(NAME(m_clocks_per_cycle));
	save_item(NAME(m_timer_value));
	save_item(NAME(m_timer_load));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_irq_state[0]));
	save_item(NAME(m_irq_state[1]));
	save_item(NAME(m_irq_state[2]));
	save_item(NAME(m_irq_pending));

	#if LAZY_FLAGS
	save_item(NAME(m_nz));
	#endif
	save_item(NAME(m_io_buffer));

	// set our instruction counter
	m_icountptr = &m_icount;
	m_icount = 0;
}

void h6280_device::device_reset()
{
	/* wipe out the h6280 structure */
	m_ppc.d = 0;
	m_pc.d = 0;
	m_zp.d = 0;
	m_ea.d = 0;
	m_a = 0;
	m_x = 0;
	m_y = 0;
	m_p = 0;
	memset(m_mmr, 0, sizeof(UINT8) * 8);
	m_irq_mask = 0;
	m_timer_ack = 0;
	m_timer_value = 0;
#if LAZY_FLAGS
	m_nz = 0;
#endif
	m_io_buffer = 0;

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	/* set I and B flags */
	P = _fI | _fB;

    /* stack starts at 0x01ff */
	m_sp.d = 0x1ff;

    /* read the reset vector into PC */
	PCL = program_read8(H6280_RESET_VEC);
	PCH = program_read8(H6280_RESET_VEC + 1);

	/* CPU starts in low speed mode */
    m_clocks_per_cycle = 4;

	/* timer off by default */
	m_timer_status = 0;
	m_timer_load = 128 * 1024;

    /* clear pending interrupts */
	for (int i = 0; i < 3; i++)
	{
		m_irq_state[i] = CLEAR_LINE;
	}
	m_nmi_state = CLEAR_LINE;

	m_irq_pending = 0;
}

void h6280_device::device_stop()
{
	/* nothing */
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *h6280_device::memory_space_config(address_spacenum spacenum) const
{
	if (spacenum == AS_PROGRAM)
	{
		return &m_program_config;
	}
	else if (spacenum == AS_IO)
	{
		return &m_io_config;
	}
	return NULL;
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void h6280_device::state_string_export(const device_state_entry &entry, astring &string)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			string.printf("%c%c%c%c%c%c%c%c",
				(m_p & 0x80) ? 'N':'.',
				(m_p & 0x40) ? 'V':'.',
				(m_p & 0x20) ? 'R':'.',
				(m_p & 0x10) ? 'B':'.',
				(m_p & 0x08) ? 'D':'.',
				(m_p & 0x04) ? 'I':'.',
				(m_p & 0x02) ? 'Z':'.',
				(m_p & 0x01) ? 'C':'.');
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 h6280_device::disasm_min_opcode_bytes() const
{
	return 1;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 h6280_device::disasm_max_opcode_bytes() const
{
	return 7;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t h6280_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( h6280 );
	return cpu_disassemble_h6280(NULL, buffer, pc, oprom, opram, options);
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 h6280_device::execute_min_cycles() const
{
	return 2;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 h6280_device::execute_max_cycles() const
{
	return 17 + 6*65536;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 h6280_device::execute_input_lines() const
{
	return 4;
}


//-------------------------------------------------
//  execute_set_input - act on a changed input/
//  interrupt line
//-------------------------------------------------

void h6280_device::execute_set_input(int inputnum, int state)
{
	switch(inputnum)
	{
		case H6280_IRQ1_STATE:
		case 0:
			set_irq_line(0, state);
			break;
		case H6280_IRQ2_STATE:
		case 1:
			set_irq_line(1, state);
			break;
		case H6280_IRQT_STATE:
		case 2:
			set_irq_line(2, state);
			break;
		case H6280_NMI_STATE:
		case INPUT_LINE_NMI:
			set_irq_line(INPUT_LINE_NMI, state);
			break;
	}
}

/***************************************************************
 *  program_read8		read memory
 ***************************************************************/
UINT8 h6280_device::program_read8(offs_t addr)
{
	CHECK_VDC_VCE_PENALTY(addr);
	return m_program->read_byte(TRANSLATED(addr));
}

/***************************************************************
 *  program_write8		write memory
 ***************************************************************/
void h6280_device::program_write8(offs_t addr, UINT8 data)
{
	CHECK_VDC_VCE_PENALTY(addr);
	m_program->write_byte(TRANSLATED(addr), data);
}

/***************************************************************
 *  program_read8z		read memory - zero page
 ***************************************************************/
UINT8 h6280_device::program_read8z(offs_t addr)
{
	return m_program->read_byte((m_mmr[1] << 13) | (addr & 0x1fff));
}

/***************************************************************
 *  program_write8z		write memory - zero page
 ***************************************************************/
void h6280_device::program_write8z(offs_t addr, UINT8 data)
{
	m_program->write_byte((m_mmr[1] << 13) | (addr & 0x1fff), data);
}

/***************************************************************
 *  program_read16		read word from memory
 ***************************************************************/
UINT16 h6280_device::program_read16(offs_t addr)
{
	return m_program->read_byte(TRANSLATED(addr)) |
		  (m_program->read_byte(TRANSLATED(addr + 1)) << 8);
}

/***************************************************************
 *  program_read16z		read a word from a zero page address
 ***************************************************************/
UINT16 h6280_device::program_read16z(offs_t addr)
{
	if ((addr & 0xff) == 0xff)
	{
		return m_program->read_byte((m_mmr[1] << 13) | (addr & 0x1fff)) |
			  (m_program->read_byte((m_mmr[1] << 13) | ((addr - 0xff) & 0x1fff)) << 8);
	}
	else
	{
		return m_program->read_byte((m_mmr[1] << 13) | (addr & 0x1fff)) |
			  (m_program->read_byte((m_mmr[1] << 13) | ((addr + 1) & 0x1fff)) << 8);
	}
}

/***************************************************************
 * push a register onto the stack
 ***************************************************************/
void h6280_device::push(UINT8 value)
{
	m_program->write_byte((m_mmr[1] << 13) | m_sp.d, value);
	S--;
}

/***************************************************************
 * pull a register from the stack
 ***************************************************************/
void h6280_device::pull(UINT8 &value)
{
	S++;
	value = m_program->read_byte((m_mmr[1] << 13) | m_sp.d);
}

/***************************************************************
 *  read_opcode		read an opcode
 ***************************************************************/
UINT8 h6280_device::read_opcode()
{
	return m_direct->read_decrypted_byte(TRANSLATED(PCW));
}

/***************************************************************
 *  read_opcode_arg	read an opcode argument
 ***************************************************************/
UINT8 h6280_device::read_opcode_arg()
{
	return m_direct->read_raw_byte(TRANSLATED(PCW));
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void h6280_device::execute_run()
{
	int in;

	if (m_irq_pending == 2)
	{
		m_irq_pending--;
	}

	/* Execute instructions */
	do
    {
		m_ppc = m_pc;

		debugger_instruction_hook(this, PCW);

		/* Execute 1 instruction */
		in = read_opcode();
		PCW++;
		(this->*m_opcode[in])();

		if (m_irq_pending)
		{
			if (m_irq_pending == 1)
			{
				if (!(P & _fI))
				{
					m_irq_pending--;
					CHECK_AND_TAKE_IRQ_LINES;
				}
			}
			else
			{
				m_irq_pending--;
			}
		}

		/* Check internal timer */
		if (m_timer_status)
		{
			if (m_timer_value<=0)
			{
				if (!m_irq_pending)
				{
					m_irq_pending = 1;
				}
				while (m_timer_value <= 0)
				{
					m_timer_value += m_timer_load;
				}
				set_irq_line(2, ASSERT_LINE);
			}
		}
	} while (m_icount > 0);
}


//**************************************************************************
//  IRQ HANDLING
//**************************************************************************

void h6280_device::set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (state != ASSERT_LINE)
			return;
		m_nmi_state = state;
		CHECK_IRQ_LINES;
	}
	else if (irqline < 3)
	{
		/* If the state has not changed, just return */
		if (m_irq_state[irqline] == state)
			return;

	    m_irq_state[irqline] = state;

		CHECK_IRQ_LINES;
	}
}


//**************************************************************************
//  REGISTER HANDLING
//**************************************************************************

READ8_MEMBER( h6280_device::irq_status_r )
{
	int status;

	switch (offset & 3)
	{
	default:
		return m_io_buffer;
	case 3:
		{
			status = 0;
			if (m_irq_state[1] != CLEAR_LINE)
				status |= 1; /* IRQ 2 */
			if (m_irq_state[0] != CLEAR_LINE)
				status |= 2; /* IRQ 1 */
			if (m_irq_state[2] != CLEAR_LINE)
				status |= 4; /* TIMER */
			return status | (m_io_buffer & (~H6280_IRQ_MASK));
		}
	case 2:
		return m_irq_mask | (m_io_buffer & (~H6280_IRQ_MASK));
	}
}

WRITE8_MEMBER( h6280_device::irq_status_w )
{
	m_io_buffer = data;
	switch (offset & 3)
	{
		default:
			m_io_buffer = data;
			break;

		case 2: /* Write irq mask */
			m_irq_mask = data & 0x7;
			CHECK_IRQ_LINES;
			break;

		case 3: /* Timer irq ack */
			set_irq_line(2, CLEAR_LINE);
			break;
	}
}

READ8_MEMBER( h6280_device::timer_r )
{
	/* only returns countdown */
	return ((m_timer_value >> 10) & 0x7F) | (m_io_buffer & 0x80);
}

WRITE8_MEMBER( h6280_device::timer_w )
{
	m_io_buffer = data;
	switch (offset & 1)
	{
		case 0: /* Counter preload */
			m_timer_load = m_timer_value = ((data & 127) + 1) * 1024;
			return;

		case 1: /* Counter enable */
			if (data & 1)
			{	/* stop -> start causes reload */
				if(m_timer_status == 0)
					m_timer_value = m_timer_load;
			}
			m_timer_status = data & 1;
			return;
	}
}

bool h6280_device::memory_translate(address_spacenum spacenum, int intention, offs_t &address)
{
	if (spacenum == AS_PROGRAM)
		address = TRANSLATED(address);

	return TRUE;
}

UINT8 h6280_device::io_get_buffer()
{
	return m_io_buffer;
}

void h6280_device::io_set_buffer(UINT8 data)
{
	m_io_buffer = data;
}
