// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56k.c
    Core implementation for the portable DSP56k emulator.
    Written by Andrew Gardner

****************************************************************************

    Note:
    This CPU emulator is very much a work-in-progress.

    DONE:
    1:  1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
       11,  ,  ,  ,  ,  ,  ,18,  ,   ,

    TODO:
    X 1-6 Explore CORE naming scheme.
    - 1-9 paragraph 1 : memory access timings
    - 1-9 Data ALU arithmetic operations generally use fractional two's complement arithmetic
          (Unsigned numbers are only supported by the multiply and multiply-accumulate instruction)
    - 1-9 For fractional arithmetic, the 31-bit product is added to the 40-bit contents of A or B.  No pipeline!
    - 1-10 Two types of rounding: convergent rounding and two's complement rounding.  See status register bit R.
    - 1-10 Logic unit is 16-bits wide and works on MSP portion of accum register
    - 1-10 The AGU can implement three types of arithmetic: linear, modulo, and reverse carry.
    - 1-12 "Two external interrupt pins!!!"
    - 1-12 Take care of all interrupt priority (IPR) stuff!
    - 1-19 Memory WAIT states
    - 1-20 The timer's interesting!
    - 1-21 Vectored exception requests on the Host Interface!
***************************************************************************/

#include "opcode.h"

#include "emu.h"
#include "debugger.h"
#include "dsp56k.h"

#include "dsp56def.h"

using namespace DSP56K;

/***************************************************************************
    COMPONENT FUNCTIONALITY
***************************************************************************/
/* 1-9 ALU */
// #include "dsp56alu.h"

/* 1-10 Address Generation Unit (AGU) */
// #include "dsp56agu.h"

/* 1-11 Program Control Unit (PCU) */
#include "dsp56pcu.h"

/* 5-1 Host Interface (HI) */
//#include "dsp56hi.h"

/* 4-8 Memory handlers for on-chip peripheral memory. */
#include "dsp56mem.h"


enum
{
	// PCU
	DSP56K_PC=1,
	DSP56K_SR,
	DSP56K_LC,
	DSP56K_LA,
	DSP56K_SP,
	DSP56K_OMR,

	// ALU
	DSP56K_X, DSP56K_Y,
	DSP56K_A, DSP56K_B,

	// AGU
	DSP56K_R0,DSP56K_R1,DSP56K_R2,DSP56K_R3,
	DSP56K_N0,DSP56K_N1,DSP56K_N2,DSP56K_N3,
	DSP56K_M0,DSP56K_M1,DSP56K_M2,DSP56K_M3,
	DSP56K_TEMP,
	DSP56K_STATUS,

	// CPU STACK
	DSP56K_ST0,
	DSP56K_ST1,
	DSP56K_ST2,
	DSP56K_ST3,
	DSP56K_ST4,
	DSP56K_ST5,
	DSP56K_ST6,
	DSP56K_ST7,
	DSP56K_ST8,
	DSP56K_ST9,
	DSP56K_ST10,
	DSP56K_ST11,
	DSP56K_ST12,
	DSP56K_ST13,
	DSP56K_ST14,
	DSP56K_ST15
};


const device_type DSP56156 = &device_creator<dsp56k_device>;


/****************************************************************************
 *  Internal Memory Maps
 ****************************************************************************/
static ADDRESS_MAP_START( dsp56156_program_map, AS_PROGRAM, 16, dsp56k_device )
	AM_RANGE(0x0000,0x07ff) AM_RAM AM_SHARE("dsk56k_program_ram")   /* 1-5 */
//  AM_RANGE(0x2f00,0x2fff) AM_ROM                              /* 1-5 PROM reserved memory.  Is this the right spot for it? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsp56156_x_data_map, AS_DATA, 16, dsp56k_device )
	AM_RANGE(0x0000,0x07ff) AM_RAM                              /* 1-5 */
	AM_RANGE(0xffc0,0xffff) AM_READWRITE(peripheral_register_r, peripheral_register_w)   /* 1-5 On-chip peripheral registers memory mapped in data space */
ADDRESS_MAP_END


dsp56k_device::dsp56k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, DSP56156, "DSP56156", tag, owner, clock, "dsp56156", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 16, -1, ADDRESS_MAP_NAME(dsp56156_program_map))
	, m_data_config("data", ENDIANNESS_LITTLE, 16, 16, -1, ADDRESS_MAP_NAME(dsp56156_x_data_map))
	, m_program_ram(*this, "dsk56k_program_ram")
{
}

/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/
#define ROPCODE(pc)   cpustate->direct->read_word(pc)


/***************************************************************************
    IRQ HANDLING
***************************************************************************/
void dsp56k_device::execute_set_input(int irqline, int state)
{
	//logerror("DSP56k set irq line %d %d\n", irqline, state);

	switch(irqline)
	{
		case DSP56K_IRQ_MODA:
			// TODO: 1-12 Get this triggering right
			if (irqa_trigger(&m_dsp56k_core))
				logerror("DSP56k IRQA is set to fire on the \"Negative Edge\".\n");

			if (state != CLEAR_LINE)
				m_dsp56k_core.modA_state = TRUE;
			else
				m_dsp56k_core.modA_state = FALSE;

			if (m_dsp56k_core.reset_state != TRUE)
				dsp56k_add_pending_interrupt(&m_dsp56k_core, "IRQA");
			break;

		case DSP56K_IRQ_MODB:
			// TODO: 1-12 Get this triggering right
			if (irqb_trigger(&m_dsp56k_core))
				logerror("DSP56k IRQB is set to fire on the \"Negative Edge\".\n");

			if (state != CLEAR_LINE)
				m_dsp56k_core.modB_state = TRUE;
			else
				m_dsp56k_core.modB_state = FALSE;

			if (m_dsp56k_core.reset_state != TRUE)
				dsp56k_add_pending_interrupt(&m_dsp56k_core, "IRQB");
			break;

		case DSP56K_IRQ_MODC:
			if (state != CLEAR_LINE)
				m_dsp56k_core.modC_state = TRUE;
			else
				m_dsp56k_core.modC_state = FALSE;

			// TODO : Set bus mode or whatever
			break;

		case DSP56K_IRQ_RESET:
			if (state != CLEAR_LINE)
				m_dsp56k_core.reset_state = TRUE;
			else
			{
				/* If it changes state from asserted to cleared.  Call the reset function. */
				if (m_dsp56k_core.reset_state == TRUE)
					device_reset();

				m_dsp56k_core.reset_state = FALSE;
			}

			// dsp56k_add_pending_interrupt("Hardware RESET");
			break;

		default:
			logerror("DSP56k setting some weird irq line : %d", irqline);
			break;
	}

	/* If the reset line isn't asserted, service interrupts */
	// TODO: Is it right to immediately service interrupts?
	//if (cpustate->reset_state != TRUE)
	//  pcu_service_interrupts();
}


/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/
void dsp56k_device::agu_init()
{
	/* save states - dsp56k_agu members */
	save_item(NAME(m_dsp56k_core.AGU.r0));
	save_item(NAME(m_dsp56k_core.AGU.r1));
	save_item(NAME(m_dsp56k_core.AGU.r2));
	save_item(NAME(m_dsp56k_core.AGU.r3));
	save_item(NAME(m_dsp56k_core.AGU.n0));
	save_item(NAME(m_dsp56k_core.AGU.n1));
	save_item(NAME(m_dsp56k_core.AGU.n2));
	save_item(NAME(m_dsp56k_core.AGU.n3));
	save_item(NAME(m_dsp56k_core.AGU.m0));
	save_item(NAME(m_dsp56k_core.AGU.m1));
	save_item(NAME(m_dsp56k_core.AGU.m2));
	save_item(NAME(m_dsp56k_core.AGU.m3));
	save_item(NAME(m_dsp56k_core.AGU.temp));
}

void dsp56k_device::alu_init()
{
	/* save states - dsp56k_alu members */
	save_item(NAME(m_dsp56k_core.ALU.x));
	save_item(NAME(m_dsp56k_core.ALU.y));
	save_item(NAME(m_dsp56k_core.ALU.a));
	save_item(NAME(m_dsp56k_core.ALU.b));
}

void dsp56k_device::device_start()
{
	memset(&m_dsp56k_core, 0, sizeof(m_dsp56k_core));

	m_dsp56k_core.device = this;
	m_dsp56k_core.program_ram = m_program_ram;

	/* Call specific module inits */
	pcu_init(&m_dsp56k_core, this);
	agu_init();
	alu_init();

	/* HACK - You're not in bootstrap mode upon bootup */
	m_dsp56k_core.bootstrap_mode = BOOTSTRAP_OFF;

	/* Clear the irq states */
	m_dsp56k_core.modA_state = FALSE;
	m_dsp56k_core.modB_state = FALSE;
	m_dsp56k_core.modC_state = FALSE;
	m_dsp56k_core.reset_state = FALSE;

	/* save states - dsp56k_core members */
	save_item(NAME(m_dsp56k_core.modA_state));
	save_item(NAME(m_dsp56k_core.modB_state));
	save_item(NAME(m_dsp56k_core.modC_state));
	save_item(NAME(m_dsp56k_core.reset_state));
	save_item(NAME(m_dsp56k_core.bootstrap_mode));
	save_item(NAME(m_dsp56k_core.repFlag));
	save_item(NAME(m_dsp56k_core.repAddr));
	save_item(NAME(m_dsp56k_core.ppc));
	save_item(NAME(m_dsp56k_core.op));
	save_item(NAME(m_dsp56k_core.interrupt_cycles));

	/* save states - dsp56k_host_interface members */
	save_item(NAME(m_dsp56k_core.HI.icr));
	save_item(NAME(m_dsp56k_core.HI.cvr));
	save_item(NAME(m_dsp56k_core.HI.isr));
	save_item(NAME(m_dsp56k_core.HI.ivr));
	save_item(NAME(m_dsp56k_core.HI.trxh));
	save_item(NAME(m_dsp56k_core.HI.trxl));
	save_item(NAME(m_dsp56k_core.HI.bootstrap_offset));

	save_item(NAME(m_dsp56k_core.peripheral_ram));

	m_dsp56k_core.program = &space(AS_PROGRAM);
	m_dsp56k_core.direct = &m_dsp56k_core.program->direct();
	m_dsp56k_core.data = &space(AS_DATA);

	state_add(DSP56K_PC,     "PC", m_dsp56k_core.PCU.pc).formatstr("%04X");
	state_add(DSP56K_SR,     "SR", m_dsp56k_core.PCU.sr).formatstr("%04X");
	state_add(DSP56K_LC,     "LC", m_dsp56k_core.PCU.lc).formatstr("%04X");
	state_add(DSP56K_LA,     "LA", m_dsp56k_core.PCU.la).formatstr("%04X");
	state_add(DSP56K_SP,     "SP", m_dsp56k_core.PCU.sp).formatstr("%02X");
	state_add(DSP56K_OMR,    "OMR", m_dsp56k_core.PCU.omr).formatstr("%02X");

	state_add(DSP56K_X,      "X", m_dsp56k_core.ALU.x.d).mask(0xffffffff).formatstr("%9s");
	state_add(DSP56K_Y,      "Y", m_dsp56k_core.ALU.y.d).mask(0xffffffff).formatstr("%9s");

	state_add(DSP56K_A,      "A", m_dsp56k_core.ALU.a.q).mask((UINT64)U64(0xffffffffffffffff)).formatstr("%12s"); /* could benefit from a better mask? */
	state_add(DSP56K_B,      "B", m_dsp56k_core.ALU.b.q).mask((UINT64)U64(0xffffffffffffffff)).formatstr("%12s"); /* could benefit from a better mask? */

	state_add(DSP56K_R0,     "R0", m_dsp56k_core.AGU.r0).formatstr("%04X");
	state_add(DSP56K_R1,     "R1", m_dsp56k_core.AGU.r1).formatstr("%04X");
	state_add(DSP56K_R2,     "R2", m_dsp56k_core.AGU.r2).formatstr("%04X");
	state_add(DSP56K_R3,     "R3", m_dsp56k_core.AGU.r3).formatstr("%04X");

	state_add(DSP56K_N0,     "N0", m_dsp56k_core.AGU.n0).formatstr("%04X");
	state_add(DSP56K_N1,     "N1", m_dsp56k_core.AGU.n1).formatstr("%04X");
	state_add(DSP56K_N2,     "N2", m_dsp56k_core.AGU.n2).formatstr("%04X");
	state_add(DSP56K_N3,     "N3", m_dsp56k_core.AGU.n3).formatstr("%04X");

	state_add(DSP56K_M0,     "M0", m_dsp56k_core.AGU.m0).formatstr("%04X");
	state_add(DSP56K_M1,     "M1", m_dsp56k_core.AGU.m1).formatstr("%04X");
	state_add(DSP56K_M2,     "M2", m_dsp56k_core.AGU.m2).formatstr("%04X");
	state_add(DSP56K_M3,     "M3", m_dsp56k_core.AGU.m3).formatstr("%04X");

	state_add(DSP56K_TEMP,   "TMP", m_dsp56k_core.AGU.temp).formatstr("%04X").noshow();
	//state_add(DSP56K_STATUS, "STS", STATUS).formatstr("%02X");

	state_add(DSP56K_ST0,    "ST0", m_dsp56k_core.PCU.ss[0].d).formatstr("%08X");
	state_add(DSP56K_ST1,    "ST1", m_dsp56k_core.PCU.ss[1].d).formatstr("%08X");
	state_add(DSP56K_ST2,    "ST2", m_dsp56k_core.PCU.ss[2].d).formatstr("%08X");
	state_add(DSP56K_ST3,    "ST3", m_dsp56k_core.PCU.ss[3].d).formatstr("%08X");
	state_add(DSP56K_ST4,    "ST4", m_dsp56k_core.PCU.ss[4].d).formatstr("%08X");
	state_add(DSP56K_ST5,    "ST5", m_dsp56k_core.PCU.ss[5].d).formatstr("%08X");
	state_add(DSP56K_ST6,    "ST6", m_dsp56k_core.PCU.ss[6].d).formatstr("%08X");
	state_add(DSP56K_ST7,    "ST7", m_dsp56k_core.PCU.ss[7].d).formatstr("%08X");
	state_add(DSP56K_ST8,    "ST8", m_dsp56k_core.PCU.ss[8].d).formatstr("%08X");
	state_add(DSP56K_ST9,    "ST9", m_dsp56k_core.PCU.ss[9].d).formatstr("%08X");
	state_add(DSP56K_ST10,   "ST10", m_dsp56k_core.PCU.ss[10].d).formatstr("%08X");
	state_add(DSP56K_ST11,   "ST11", m_dsp56k_core.PCU.ss[11].d).formatstr("%08X");
	state_add(DSP56K_ST12,   "ST12", m_dsp56k_core.PCU.ss[12].d).formatstr("%08X");
	state_add(DSP56K_ST13,   "ST13", m_dsp56k_core.PCU.ss[13].d).formatstr("%08X");
	state_add(DSP56K_ST14,   "ST14", m_dsp56k_core.PCU.ss[14].d).formatstr("%08X");
	state_add(DSP56K_ST15,   "ST15", m_dsp56k_core.PCU.ss[15].d).formatstr("%08X");

	state_add(STATE_GENPC, "GENPC", m_dsp56k_core.PCU.pc).noshow();
	state_add(STATE_GENSP, "GENSP", m_dsp56k_core.PCU.sp).noshow();
	state_add(STATE_GENPCBASE, "GENPCBASE", m_dsp56k_core.ppc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_dsp56k_core.PCU.sr).formatstr("%14s").noshow();

	m_icountptr = &m_dsp56k_core.icount;
}


void dsp56k_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	dsp56k_core *cpustate = &m_dsp56k_core;

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%s%s %s%s%s%s%s%s%s%s %s%s",
				/* Status Register */
				LF_bit(cpustate) ? "L" : ".",
				FV_bit(cpustate) ? "F" : ".",

				S_bit(cpustate) ? "S" : ".",
				L_bit(cpustate) ? "L" : ".",
				E_bit(cpustate) ? "E" : ".",
				U_bit(cpustate) ? "U" : ".",
				N_bit(cpustate) ? "N" : ".",
				Z_bit(cpustate) ? "Z" : ".",
				V_bit(cpustate) ? "V" : ".",
				C_bit(cpustate) ? "C" : ".",

				/* Stack Pointer */
				UF_bit(cpustate) ? "U" : ".",
				SE_bit(cpustate) ? "S" : ".");
			break;

		case DSP56K_X:
			strprintf(str, "%04x %04x", X1, X0);
			break;

		case DSP56K_Y:
			strprintf(str, "%04x %04x", Y1, Y0);
			break;

		case DSP56K_A:
			strprintf(str, "%02x %04x %04x", A2, A1, A0);
			break;

		case DSP56K_B:
			strprintf(str, "%02x %04x %04x", B2, B1, B0);
			break;
	}
}

/***************************************************************************
    RESET BEHAVIOR
***************************************************************************/
static void agu_reset(dsp56k_core* cpustate)
{
	/* FM.4-3 */
	R0 = 0x0000;
	R1 = 0x0000;
	R2 = 0x0000;
	R3 = 0x0000;

	N0 = 0x0000;
	N1 = 0x0000;
	N2 = 0x0000;
	N3 = 0x0000;

	M0 = 0xffff;
	M1 = 0xffff;
	M2 = 0xffff;
	M3 = 0xffff;

	TEMP = 0x0000;
}

static void alu_reset(dsp56k_core* cpustate)
{
	X = 0x00000000;
	Y = 0x00000000;
	A = 0x0000000000;
	B = 0x0000000000;
}

void dsp56k_device::device_reset()
{
	logerror("Dsp56k reset\n");

	m_dsp56k_core.interrupt_cycles = 0;
	m_dsp56k_core.ppc = 0x0000;

	m_dsp56k_core.repFlag = 0;
	m_dsp56k_core.repAddr = 0x0000;

	pcu_reset(&m_dsp56k_core);
	mem_reset(&m_dsp56k_core);
	agu_reset(&m_dsp56k_core);
	alu_reset(&m_dsp56k_core);

	/* HACK - Put a jump to 0x0000 at 0x0000 - this keeps the CPU locked to the instruction at address 0x0000 */
	m_dsp56k_core.program->write_word(0x0000, 0x0124);
}



/***************************************************************************
    CORE INCLUDE
***************************************************************************/
#include "dsp56ops.inc"


/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/
// Execute a single opcode and return how many cycles it took.
static size_t execute_one_new(dsp56k_core* cpustate)
{
	// For MAME
	cpustate->op = ROPCODE(ADDRESS(PC));
	debugger_instruction_hook(cpustate->device, PC);

	UINT16 w0 = ROPCODE(ADDRESS(PC));
	UINT16 w1 = ROPCODE(ADDRESS(PC) + ADDRESS(1));

	Opcode op(w0, w1);
	op.evaluate(cpustate);
	PC += op.evalSize();    // Special size function needed to handle jmps, etc.

	// TODO: Currently all operations take up 4 cycles (inst->cycles()).
	return 4;
}

void dsp56k_device::execute_run()
{
	/* If reset line is asserted, do nothing */
	if (m_dsp56k_core.reset_state)
	{
		m_dsp56k_core.icount = 0;
		return;
	}

	/* HACK - if you're in bootstrap mode, simply pretend you ate up all your cycles waiting for data. */
	if (m_dsp56k_core.bootstrap_mode != BOOTSTRAP_OFF)
	{
		m_dsp56k_core.icount = 0;
		return;
	}

	//m_dsp56k_core.icount -= m_dsp56k_core.interrupt_cycles;
	//m_dsp56k_core.interrupt_cycles = 0;

	while(m_dsp56k_core.icount > 0)
	{
		execute_one(&m_dsp56k_core);
		if (0) m_dsp56k_core.icount -= execute_one_new(&m_dsp56k_core);
		pcu_service_interrupts(&m_dsp56k_core);   // TODO: Is it incorrect to service after each instruction?
	}
}


offs_t dsp56k_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( dsp56k );
	return CPU_DISASSEMBLE_NAME(dsp56k)(this, buffer, pc, oprom, opram, options);
}
