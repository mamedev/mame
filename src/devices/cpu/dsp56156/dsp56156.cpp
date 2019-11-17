// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/***************************************************************************

    dsp56156.cpp
    Core implementation for the portable DSP56156 emulator.
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

#include "emu.h"
#include "dsp56156.h"
#include "dsp56dsm.h"

#include "opcode.h"

#include "debugger.h"

#include "dsp56def.h"

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


DEFINE_DEVICE_TYPE_NS(DSP56156, DSP_56156, dsp56156_device, "dsp56156", "Motorola DSP56156")


namespace DSP_56156 {

enum
{
	// PCU
	DSP56156_PC=1,
	DSP56156_SR,
	DSP56156_LC,
	DSP56156_LA,
	DSP56156_SP,
	DSP56156_OMR,

	// ALU
	DSP56156_X, DSP56156_Y,
	DSP56156_A, DSP56156_B,

	// AGU
	DSP56156_R0,DSP56156_R1,DSP56156_R2,DSP56156_R3,
	DSP56156_N0,DSP56156_N1,DSP56156_N2,DSP56156_N3,
	DSP56156_M0,DSP56156_M1,DSP56156_M2,DSP56156_M3,
	DSP56156_TEMP,
	DSP56156_STATUS,

	// CPU STACK
	DSP56156_ST0,
	DSP56156_ST1,
	DSP56156_ST2,
	DSP56156_ST3,
	DSP56156_ST4,
	DSP56156_ST5,
	DSP56156_ST6,
	DSP56156_ST7,
	DSP56156_ST8,
	DSP56156_ST9,
	DSP56156_ST10,
	DSP56156_ST11,
	DSP56156_ST12,
	DSP56156_ST13,
	DSP56156_ST14,
	DSP56156_ST15
};


/****************************************************************************
 *  Internal Memory Maps
 ****************************************************************************/
void dsp56156_device::dsp56156_program_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("dsk56156_program_ram");   /* 1-5 */
//  map(0x2f00, 0x2fff).rom();                              /* 1-5 PROM reserved memory.  Is this the right spot for it? */
}

void dsp56156_device::dsp56156_x_data_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();                              /* 1-5 */
	map(0xffc0, 0xffff).rw(FUNC(dsp56156_device::peripheral_register_r), FUNC(dsp56156_device::peripheral_register_w));   /* 1-5 On-chip peripheral registers memory mapped in data space */
}


dsp56156_device::dsp56156_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, DSP56156, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 16, 16, -1, address_map_constructor(FUNC(dsp56156_device::dsp56156_program_map), this))
	, m_data_config("data", ENDIANNESS_LITTLE, 16, 16, -1, address_map_constructor(FUNC(dsp56156_device::dsp56156_x_data_map), this))
	, m_program_ram(*this, "dsk56156_program_ram")
{
}

device_memory_interface::space_config_vector dsp56156_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}

/***************************************************************************
    MEMORY ACCESSORS
***************************************************************************/
#define ROPCODE(pc)   cpustate->cache->read_word(pc)


/***************************************************************************
    IRQ HANDLING
***************************************************************************/
void dsp56156_device::execute_set_input(int irqline, int state)
{
	//logerror("DSP56156 set irq line %d %d\n", irqline, state);

	switch(irqline)
	{
		case DSP56156_IRQ_MODA:
			// TODO: 1-12 Get this triggering right
			if (irqa_trigger(&m_core))
				logerror("DSP56156 IRQA is set to fire on the \"Negative Edge\".\n");

			if (state != CLEAR_LINE)
				m_core.modA_state = true;
			else
				m_core.modA_state = false;

			if (m_core.reset_state != true)
				dsp56156_add_pending_interrupt(&m_core, "IRQA");
			break;

		case DSP56156_IRQ_MODB:
			// TODO: 1-12 Get this triggering right
			if (irqb_trigger(&m_core))
				logerror("DSP56156 IRQB is set to fire on the \"Negative Edge\".\n");

			if (state != CLEAR_LINE)
				m_core.modB_state = true;
			else
				m_core.modB_state = false;

			if (m_core.reset_state != true)
				dsp56156_add_pending_interrupt(&m_core, "IRQB");
			break;

		case DSP56156_IRQ_MODC:
			if (state != CLEAR_LINE)
				m_core.modC_state = true;
			else
				m_core.modC_state = false;

			// TODO : Set bus mode or whatever
			break;

		case DSP56156_IRQ_RESET:
			if (state != CLEAR_LINE)
				m_core.reset_state = true;
			else
			{
				/* If it changes state from asserted to cleared.  Call the reset function. */
				if (m_core.reset_state == true)
					device_reset();

				m_core.reset_state = false;
			}

			// dsp56156_add_pending_interrupt("Hardware RESET");
			break;

		default:
			logerror("DSP56156 setting some weird irq line : %d", irqline);
			break;
	}

	/* If the reset line isn't asserted, service interrupts */
	// TODO: Is it right to immediately service interrupts?
	//if (cpustate->reset_state != true)
	//  pcu_service_interrupts();
}


/***************************************************************************
    INITIALIZATION AND SHUTDOWN
***************************************************************************/
void dsp56156_device::agu_init()
{
	/* save states - dsp56156_agu members */
	save_item(NAME(m_core.AGU.r0));
	save_item(NAME(m_core.AGU.r1));
	save_item(NAME(m_core.AGU.r2));
	save_item(NAME(m_core.AGU.r3));
	save_item(NAME(m_core.AGU.n0));
	save_item(NAME(m_core.AGU.n1));
	save_item(NAME(m_core.AGU.n2));
	save_item(NAME(m_core.AGU.n3));
	save_item(NAME(m_core.AGU.m0));
	save_item(NAME(m_core.AGU.m1));
	save_item(NAME(m_core.AGU.m2));
	save_item(NAME(m_core.AGU.m3));
	save_item(NAME(m_core.AGU.temp));
}

void dsp56156_device::alu_init()
{
	/* save states - dsp56156_alu members */
	save_item(NAME(m_core.ALU.x));
	save_item(NAME(m_core.ALU.y));
	save_item(NAME(m_core.ALU.a));
	save_item(NAME(m_core.ALU.b));
}

void dsp56156_device::device_start()
{
	memset(&m_core, 0, sizeof(m_core));

	m_core.device = this;
	m_core.program_ram = m_program_ram;

	/* Call specific module inits */
	pcu_init(&m_core, this);
	agu_init();
	alu_init();

	/* HACK - You're not in bootstrap mode upon bootup */
	m_core.bootstrap_mode = BOOTSTRAP_OFF;

	/* Clear the irq states */
	m_core.modA_state = false;
	m_core.modB_state = false;
	m_core.modC_state = false;
	m_core.reset_state = false;

	/* save states - dsp56156_core members */
	save_item(NAME(m_core.modA_state));
	save_item(NAME(m_core.modB_state));
	save_item(NAME(m_core.modC_state));
	save_item(NAME(m_core.reset_state));
	save_item(NAME(m_core.bootstrap_mode));
	save_item(NAME(m_core.repFlag));
	save_item(NAME(m_core.repAddr));
	save_item(NAME(m_core.ppc));
	save_item(NAME(m_core.op));
	save_item(NAME(m_core.interrupt_cycles));

	/* save states - dsp56156_host_interface members */
	save_item(NAME(m_core.HI.icr));
	save_item(NAME(m_core.HI.cvr));
	save_item(NAME(m_core.HI.isr));
	save_item(NAME(m_core.HI.ivr));
	save_item(NAME(m_core.HI.trxh));
	save_item(NAME(m_core.HI.trxl));
	save_item(NAME(m_core.HI.bootstrap_offset));

	save_item(NAME(m_core.peripheral_ram));

	m_core.program = &space(AS_PROGRAM);
	m_core.cache = m_core.program->cache<1, -1, ENDIANNESS_LITTLE>();
	m_core.data = &space(AS_DATA);

	state_add(DSP56156_PC,     "PC", m_core.PCU.pc).formatstr("%04X");
	state_add(DSP56156_SR,     "SR", m_core.PCU.sr).formatstr("%04X");
	state_add(DSP56156_LC,     "LC", m_core.PCU.lc).formatstr("%04X");
	state_add(DSP56156_LA,     "LA", m_core.PCU.la).formatstr("%04X");
	state_add(DSP56156_SP,     "SP", m_core.PCU.sp).formatstr("%02X");
	state_add(DSP56156_OMR,    "OMR", m_core.PCU.omr).formatstr("%02X");

	state_add(DSP56156_X,      "X", m_core.ALU.x.d).mask(0xffffffff).formatstr("%9s");
	state_add(DSP56156_Y,      "Y", m_core.ALU.y.d).mask(0xffffffff).formatstr("%9s");

	state_add(DSP56156_A,      "A", m_core.ALU.a.q).mask(u64(0xffffffffffffffffU)).formatstr("%12s"); /* could benefit from a better mask? */
	state_add(DSP56156_B,      "B", m_core.ALU.b.q).mask(u64(0xffffffffffffffffU)).formatstr("%12s"); /* could benefit from a better mask? */

	state_add(DSP56156_R0,     "R0", m_core.AGU.r0).formatstr("%04X");
	state_add(DSP56156_R1,     "R1", m_core.AGU.r1).formatstr("%04X");
	state_add(DSP56156_R2,     "R2", m_core.AGU.r2).formatstr("%04X");
	state_add(DSP56156_R3,     "R3", m_core.AGU.r3).formatstr("%04X");

	state_add(DSP56156_N0,     "N0", m_core.AGU.n0).formatstr("%04X");
	state_add(DSP56156_N1,     "N1", m_core.AGU.n1).formatstr("%04X");
	state_add(DSP56156_N2,     "N2", m_core.AGU.n2).formatstr("%04X");
	state_add(DSP56156_N3,     "N3", m_core.AGU.n3).formatstr("%04X");

	state_add(DSP56156_M0,     "M0", m_core.AGU.m0).formatstr("%04X");
	state_add(DSP56156_M1,     "M1", m_core.AGU.m1).formatstr("%04X");
	state_add(DSP56156_M2,     "M2", m_core.AGU.m2).formatstr("%04X");
	state_add(DSP56156_M3,     "M3", m_core.AGU.m3).formatstr("%04X");

	state_add(DSP56156_TEMP,   "TMP", m_core.AGU.temp).formatstr("%04X").noshow();
	//state_add(DSP56156_STATUS, "STS", STATUS).formatstr("%02X");

	state_add(DSP56156_ST0,    "ST0", m_core.PCU.ss[0].d).formatstr("%08X");
	state_add(DSP56156_ST1,    "ST1", m_core.PCU.ss[1].d).formatstr("%08X");
	state_add(DSP56156_ST2,    "ST2", m_core.PCU.ss[2].d).formatstr("%08X");
	state_add(DSP56156_ST3,    "ST3", m_core.PCU.ss[3].d).formatstr("%08X");
	state_add(DSP56156_ST4,    "ST4", m_core.PCU.ss[4].d).formatstr("%08X");
	state_add(DSP56156_ST5,    "ST5", m_core.PCU.ss[5].d).formatstr("%08X");
	state_add(DSP56156_ST6,    "ST6", m_core.PCU.ss[6].d).formatstr("%08X");
	state_add(DSP56156_ST7,    "ST7", m_core.PCU.ss[7].d).formatstr("%08X");
	state_add(DSP56156_ST8,    "ST8", m_core.PCU.ss[8].d).formatstr("%08X");
	state_add(DSP56156_ST9,    "ST9", m_core.PCU.ss[9].d).formatstr("%08X");
	state_add(DSP56156_ST10,   "ST10", m_core.PCU.ss[10].d).formatstr("%08X");
	state_add(DSP56156_ST11,   "ST11", m_core.PCU.ss[11].d).formatstr("%08X");
	state_add(DSP56156_ST12,   "ST12", m_core.PCU.ss[12].d).formatstr("%08X");
	state_add(DSP56156_ST13,   "ST13", m_core.PCU.ss[13].d).formatstr("%08X");
	state_add(DSP56156_ST14,   "ST14", m_core.PCU.ss[14].d).formatstr("%08X");
	state_add(DSP56156_ST15,   "ST15", m_core.PCU.ss[15].d).formatstr("%08X");

	state_add(STATE_GENPC, "GENPC", m_core.PCU.pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_core.ppc).noshow();
	state_add(STATE_GENSP, "GENSP", m_core.PCU.sp).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_core.PCU.sr).formatstr("%14s").noshow();

	set_icountptr(m_core.icount);
}


void dsp56156_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	const dsp56156_core *cpustate = &m_core;

	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%s%s %s%s%s%s%s%s%s%s %s%s",
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

		case DSP56156_X:
			str = string_format("%04x %04x", X1, X0);
			break;

		case DSP56156_Y:
			str = string_format("%04x %04x", Y1, Y0);
			break;

		case DSP56156_A:
			str = string_format("%02x %04x %04x", A2, A1, A0);
			break;

		case DSP56156_B:
			str = string_format("%02x %04x %04x", B2, B1, B0);
			break;
	}
}

/***************************************************************************
    RESET BEHAVIOR
***************************************************************************/
static void agu_reset(dsp56156_core* cpustate)
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

static void alu_reset(dsp56156_core* cpustate)
{
	X = 0x00000000;
	Y = 0x00000000;
	A = 0x0000000000;
	B = 0x0000000000;
}

void dsp56156_device::device_reset()
{
	logerror("DSP56156 reset\n");

	m_core.interrupt_cycles = 0;

	m_core.repFlag = 0;
	m_core.repAddr = 0x0000;

	pcu_reset(&m_core);
	mem_reset(&m_core);
	agu_reset(&m_core);
	alu_reset(&m_core);

	m_core.ppc = m_core.PCU.pc;

	/* HACK - Put a jump to 0x0000 at 0x0000 - this keeps the CPU locked to the instruction at address 0x0000 */
	m_core.program->write_word(0x0000, 0x0124);
}



/***************************************************************************
    CORE INCLUDE
***************************************************************************/
#include "dsp56ops.hxx"


/***************************************************************************
    CORE EXECUTION LOOP
***************************************************************************/
// Execute a single opcode and return how many cycles it took.
static size_t execute_one_new(dsp56156_core* cpustate)
{
	// For MAME
	cpustate->ppc = PC;
	if (cpustate->device->machine().debug_flags & DEBUG_FLAG_CALL_HOOK) // FIXME: if this was a member, the helper would work
		cpustate->device->debug()->instruction_hook(PC);

	cpustate->op = ROPCODE(PC);
	uint16_t w0 = ROPCODE(PC);
	uint16_t w1 = ROPCODE(PC + 1);

	Opcode op(w0, w1);
	op.evaluate(cpustate);
	PC += op.evalSize();    // Special size function needed to handle jmps, etc.

	// TODO: Currently all operations take up 4 cycles (inst->cycles()).
	return 4;
}

void dsp56156_device::execute_run()
{
	/* If reset line is asserted, do nothing */
	if (m_core.reset_state)
	{
		m_core.icount = 0;
		return;
	}

	/* HACK - if you're in bootstrap mode, simply pretend you ate up all your cycles waiting for data. */
	if (m_core.bootstrap_mode != BOOTSTRAP_OFF)
	{
		m_core.icount = 0;
		return;
	}

	//m_core.icount -= m_core.interrupt_cycles;
	//m_core.interrupt_cycles = 0;

	while(m_core.icount > 0)
	{
		execute_one(&m_core);
		if (0) m_core.icount -= execute_one_new(&m_core);
		pcu_service_interrupts(&m_core);   // TODO: Is it incorrect to service after each instruction?
	}
}


std::unique_ptr<util::disasm_interface> dsp56156_device::create_disassembler()
{
	return std::make_unique<dsp56156_disassembler>();
}

} // namespace DSP_56156
