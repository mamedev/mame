// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*** m6809: Portable 6809 emulator ******************************************

    Copyright John Butler

    References:

        6809 Simulator V09, By L.C. Benschop, Eindhoven The Netherlands.

        m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
            the 6809 Simulator V09)

        6809 Microcomputer Programming & Interfacing with Experiments"
            by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

    System dependencies:    uint16_t must be 16 bit unsigned int
                            uint8_t must be 8 bit unsigned int
                            uint32_t must be more than 16 bits
                            arrays up to 65536 bytes must be supported
                            machine must be twos complement

    History:

January 2023 tlindner:
    Add 6809 undocumented opcodes as described here:
    https://github.com/hoglet67/6809Decoder/wiki/Undocumented-6809-Behaviours

July 2016 ErikGav:
    Unify with 6309 pairs and quads (A+B=D, E+F=W, D+W=Q)

March 2013 NPW:
    Rewrite of 6809/6309/Konami CPU; attempted to make cycle exact.

991026 HJB:
    Fixed missing calls to cpu_changepc() for the TFR and EXG ocpodes.
    Replaced m6809_slapstic checks by a macro (CHANGE_PC). ESB still
    needs the tweaks.

991024 HJB:
    Tried to improve speed: Using bit7 of cycles1/2 as flag for multi
    byte opcodes is gone, those opcodes now call fetch_effective_address().
    Got rid of the slow/fast flags for stack (S and U) memory accesses.
    Minor changes to use 32 bit values as arguments to memory functions
    and added defines for that purpose (e.g. X = 16bit XD = 32bit).

990312 HJB:
    Added bugfixes according to Aaron's findings.
    Reset only sets CC_II and CC_IF, DP to zero and PC from reset vector.
990311 HJB:
    Added _info functions. Now uses static m6808_Regs struct instead
    of single statics. Changed the 16 bit registers to use the generic
    PAIR union. Registers defined using macros. Split the core into
    four execution loops for M6802, M6803, M6808 and HD63701.
    TST, TSTA and TSTB opcodes reset carry flag.
    Modified the read/write stack handlers to push LSB first then MSB
    and pull MSB first then LSB.

990228 HJB:
    Changed the interrupt handling again. Now interrupts are taken
    either right at the moment the lines are asserted or whenever
    an interrupt is enabled and the corresponding line is still
    asserted. That way the pending_interrupts checks are not
    needed anymore. However, the CWAI and SYNC flags still need
    some flags, so I changed the name to 'int_state'.
    This core also has the code for the old interrupt system removed.

990225 HJB:
    Cleaned up the code here and there, added some comments.
    Slightly changed the SAR opcodes (similiar to other CPU cores).
    Added symbolic names for the flag bits.
    Changed the way CWAI/Interrupt() handle CPU state saving.
    A new flag M6809_STATE in pending_interrupts is used to determine
    if a state save is needed on interrupt entry or already done by CWAI.
    Added M6809_IRQ_LINE and M6809_FIRQ_LINE defines to m6809.h
    Moved the internal interrupt_pending flags from m6809.h to m6809.c
    Changed CWAI cycles2[0x3c] to be 2 (plus all or at least 19 if
    CWAI actually pushes the entire state).
    Implemented undocumented TFR/EXG for undefined source and mixed 8/16
    bit transfers (they should transfer/exchange the constant $ff).
    Removed unused jmp/jsr _slap functions from 6809ops.c,
    m6809_slapstick check moved into the opcode functions.

******************************************************************************

    M6809 cycle timings are relative to a four-phase clock cycle defined by
    the Q and E signals on pins 35 and 34. The Q clock must lead the E clock
    by approximately half a cycle. On the MC6809E, Q and E are inputs, and
    one 74LS74 wired as a two-stage Johnson counter is almost sufficient to
    generate both (though E requires voltage levels above TTL). On the
    MC6809, however, Q and E are output from an internal clock generator
    which can be driven by a crystal oscillator connected to pins 38 and 39.
    (The MC6809E reuses the same numbered pins for the unrelated TSC and LIC
    functions.) The frequencies of Q and E on the MC6809 are that of the XTAL
    divided by 4. MAME's emulation formerly assigned this internal clock
    divider to the MC6809E and not to the MC6809; the confusion resulting
    from this error is in the process of being straightened out.

    Maximum clock ratings:

                        Q & E           EXTAL
        MC6809(E)       1.0 MHz         4.0 MHz
        MC68A09(E)      1.5 MHz         6.0 MHz
        MC68B09(E)      2.0 MHz         8.0 MHz
        HD63B09(E)      2.0 MHz         8.0 MHz
        HD63C09(E)      3.0 MHz         12.0 MHz

*****************************************************************************/

#include "emu.h"
#include "m6809.h"
#include "m6809inl.h"
#include "6x09dasm.h"


//**************************************************************************
//  PARAMETERS
//**************************************************************************

#define LOG_INTERRUPTS (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

// turn off 'unreferenced label' errors
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-label"
#endif
#ifdef _MSC_VER
#pragma warning( disable : 4102 )
#endif

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

DEFINE_DEVICE_TYPE(MC6809, mc6809_device, "mc6809", "Motorola MC6809")
DEFINE_DEVICE_TYPE(MC6809E, mc6809e_device, "mc6809e", "Motorola MC6809E")
DEFINE_DEVICE_TYPE(M6809, m6809_device, "m6809", "MC6809 (legacy)")


//-------------------------------------------------
//  m6809_base_device - constructor
//-------------------------------------------------

m6809_base_device::m6809_base_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const device_type type, int divider) :
	cpu_device(mconfig, type, tag, owner, clock),
	m_lic_func(*this),
	m_syncack_write_func(*this),
	m_program_config("program", ENDIANNESS_BIG, 8, 16),
	m_sprogram_config("decrypted_opcodes", ENDIANNESS_BIG, 8, 16),
	m_clock_divider(divider)
{
	m_mintf = nullptr;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m6809_base_device::device_start()
{
	if (!m_mintf)
		m_mintf = std::make_unique<mi_default>();

	space(AS_PROGRAM).cache(m_mintf->cprogram);
	space(AS_PROGRAM).specific(m_mintf->program);
	space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).cache(m_mintf->csprogram);

	// register our state for the debugger
	state_add(STATE_GENPCBASE, "CURPC",     m_ppc.w).callimport().noshow();
	state_add(STATE_GENFLAGS,  "CURFLAGS",  m_cc).formatstr("%8s").noshow();
	state_add(M6809_PC,        "PC",        m_pc.w).callimport().mask(0xffff);
	state_add(M6809_S,         "S",         m_s.w).mask(0xffff);
	state_add(M6809_CC,        "CC",        m_cc).mask(0xff);
	state_add(M6809_DP,        "DP",        m_dp).mask(0xff);

	if (is_6809())
	{
		state_add(M6809_A,     "A",         m_q.r.a).mask(0xff);
		state_add(M6809_B,     "B",         m_q.r.b).mask(0xff);
		state_add(M6809_D,     "D",         m_q.r.d).mask(0xffff);
		state_add(M6809_X,     "X",         m_x.w).mask(0xffff);
		state_add(M6809_Y,     "Y",         m_y.w).mask(0xffff);
		state_add(M6809_U,     "U",         m_u.w).mask(0xffff);
	}

	// initialize variables
	m_cc = 0;
	m_pc.w = 0;
	m_ppc.w = 0;
	m_s.w = 0;
	m_u.w = 0;
	m_q.q = 0;
	m_x.w = 0;
	m_y.w = 0;
	m_dp = 0;
	m_temp.w = 0;
	m_opcode = 0;

	m_reg8 = nullptr;
	m_reg16 = nullptr;
	m_reg = 0;
	m_nmi_line = false;
	m_nmi_asserted = false;
	m_firq_line = false;
	m_irq_line = false;
	m_lds_encountered = false;

	m_state = 0;
	m_cond = false;
	m_free_run = false;

	// setup regtable
	save_item(NAME(m_pc.w));
	save_item(NAME(m_ppc.w));
	save_item(NAME(m_q.q));
	save_item(NAME(m_dp));
	save_item(NAME(m_u.w));
	save_item(NAME(m_s.w));
	save_item(NAME(m_x.w));
	save_item(NAME(m_y.w));
	save_item(NAME(m_cc));
	save_item(NAME(m_temp.w));
	save_item(NAME(m_opcode));

	save_item(NAME(m_nmi_asserted));
	save_item(NAME(m_nmi_line));
	save_item(NAME(m_firq_line));
	save_item(NAME(m_irq_line));
	save_item(NAME(m_lds_encountered));
	save_item(NAME(m_state));
	save_item(NAME(m_ea.w));
	save_item(NAME(m_addressing_mode));
	save_item(NAME(m_reg));
	save_item(NAME(m_cond));
	save_item(NAME(m_free_run));

	// set our instruction counter
	set_icountptr(m_icount);
	m_icount = 0;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m6809_base_device::device_reset()
{
	m_nmi_line = false;
	m_nmi_asserted = false;
	m_firq_line = false;
	m_irq_line = false;
	m_lds_encountered = false;
	m_free_run = false;

	m_dp = 0x00;        // reset direct page register

	m_cc |= CC_I;       // IRQ disabled
	m_cc |= CC_F;       // FIRQ disabled

	set_ea(VECTOR_RESET_FFFE);

	// reset sub-instruction state
	reset_state();
}



//-------------------------------------------------
//  device_pre_save - device-specific pre-save
//-------------------------------------------------

void m6809_base_device::device_pre_save()
{
	if (m_reg8 == &m_q.r.a)
		m_reg = M6809_A;
	else if (m_reg8 == &m_q.r.b)
		m_reg = M6809_B;
	else if (m_reg16 == &m_q.p.d)
		m_reg = M6809_D;
	else if (m_reg16 == &m_x)
		m_reg = M6809_X;
	else if (m_reg16 == &m_y)
		m_reg = M6809_Y;
	else if (m_reg16 == &m_u)
		m_reg = M6809_U;
	else if (m_reg16 == &m_s)
		m_reg = M6809_S;
	else
		m_reg = 0;
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void m6809_base_device::device_post_load()
{
	m_reg8 = nullptr;
	m_reg16 = nullptr;

	switch(m_reg)
	{
		case M6809_A:
			set_regop8(m_q.r.a);
			break;
		case M6809_B:
			set_regop8(m_q.r.b);
			break;
		case M6809_D:
			set_regop16(m_q.p.d);
			break;
		case M6809_X:
			set_regop16(m_x);
			break;
		case M6809_Y:
			set_regop16(m_y);
			break;
		case M6809_U:
			set_regop16(m_u);
			break;
		case M6809_S:
			set_regop16(m_s);
			break;
	}
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector m6809_base_device::memory_space_config() const
{
	if(has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_OPCODES, &m_sprogram_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config)
		};
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void m6809_base_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case M6809_PC:
		m_ppc.w = m_pc.w;
		break;

	case STATE_GENPCBASE:
		m_pc.w = m_ppc.w;
		break;
	}
}

//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void m6809_base_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				(m_cc & 0x80) ? 'E' : '.',
				(m_cc & 0x40) ? 'F' : '.',
				(m_cc & 0x20) ? 'H' : '.',
				(m_cc & 0x10) ? 'I' : '.',
				(m_cc & 0x08) ? 'N' : '.',
				(m_cc & 0x04) ? 'Z' : '.',
				(m_cc & 0x02) ? 'V' : '.',
				(m_cc & 0x01) ? 'C' : '.');
			break;
	}
}


//-------------------------------------------------
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> m6809_base_device::create_disassembler()
{
	return std::make_unique<m6809_disassembler>();
}


//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_clocks_to_cycles - convert the raw
//  clock into cycles per second
//-------------------------------------------------

uint64_t m6809_base_device::execute_clocks_to_cycles(uint64_t clocks) const noexcept
{
	return (clocks + m_clock_divider - 1) / m_clock_divider;
}


//-------------------------------------------------
//  execute_cycles_to_clocks - convert a cycle
//  count back to raw clocks
//-------------------------------------------------

uint64_t m6809_base_device::execute_cycles_to_clocks(uint64_t cycles) const noexcept
{
	return cycles * m_clock_divider;
}


//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t m6809_base_device::execute_min_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t m6809_base_device::execute_max_cycles() const noexcept
{
	return 19;
}


//-------------------------------------------------
//  execute_set_input - act on a changed input/
//  interrupt line
//-------------------------------------------------

void m6809_base_device::execute_set_input(int inputnum, int state)
{
	LOGMASKED(LOG_INTERRUPTS, "%s: inputnum=%s state=%d totalcycles=%d\n", machine().describe_context(), inputnum_string(inputnum), state, attotime_to_clocks(machine().time()));

	switch(inputnum)
	{
		case INPUT_LINE_NMI:
			// NMI is edge triggered
			m_nmi_asserted = m_nmi_asserted || ((state != CLEAR_LINE) && !m_nmi_line && m_lds_encountered);
			m_nmi_line = (state != CLEAR_LINE);
			break;

		case M6809_FIRQ_LINE:
			// FIRQ is line triggered
			m_firq_line = (state != CLEAR_LINE);
			break;

		case M6809_IRQ_LINE:
			// IRQ is line triggered
			m_irq_line = (state != CLEAR_LINE);
			break;
	}
}


//-------------------------------------------------
//  inputnum_string
//-------------------------------------------------

const char *m6809_base_device::inputnum_string(int inputnum)
{
	switch(inputnum)
	{
		case INPUT_LINE_NMI:    return "NMI";
		case M6809_FIRQ_LINE:   return "FIRQ";
		case M6809_IRQ_LINE:    return "IRQ";
		default:                return "???";
	}
}


//-------------------------------------------------
//  read_tfr_register
//-------------------------------------------------

uint16_t m6809_base_device::read_tfr_exg_816_register(uint8_t reg)
{
	uint16_t result;

	switch(reg & 0x0F)
	{
		case  0: result = m_q.r.d;   break;  // D
		case  1: result = m_x.w;     break;  // X
		case  2: result = m_y.w;     break;  // Y
		case  3: result = m_u.w;     break;  // U
		case  4: result = m_s.w;     break;  // S
		case  5: result = m_pc.w;    break;  // PC
		case  8: result = ((uint16_t)0xff00) | m_q.r.a;   break;  // A
		case  9: result = ((uint16_t)0xff00) | m_q.r.b;   break;  // B
		case 10: result = ((uint16_t)m_cc) << 8 | m_cc;   break;  // CC
		case 11: result = ((uint16_t)m_dp) << 8 | m_dp;   break;  // DP
		default: result = 0xffff; break;
	}

	return result;
}


uint16_t m6809_base_device::read_exg_168_register(uint8_t reg)
{
	uint16_t result;

	switch(reg & 0x0F)
	{
		case  0: result = m_q.r.d;   break;  // D
		case  1: result = m_x.w;     break;  // X
		case  2: result = m_y.w;     break;  // Y
		case  3: result = m_u.w;     break;  // U
		case  4: result = m_s.w;     break;  // S
		case  5: result = m_pc.w;    break;  // PC
		case  8: result = ((uint16_t)0xff00) | m_q.r.a;   break;  // A
		case  9: result = ((uint16_t)0xff00) | m_q.r.b;   break;  // B
		case 10: result = ((uint16_t)0xff00) | m_cc;   break;  // CC
		case 11: result = ((uint16_t)0xff00) | m_dp;   break;  // DP
		default: result = 0xffff; break;
	}

	return result;
}


//-------------------------------------------------
//  write_exgtfr_register
//-------------------------------------------------

void m6809_base_device::write_exgtfr_register(uint8_t reg, uint16_t value)
{
	switch(reg & 0x0F)
	{
		case  0: m_q.r.d = value;    break;  // D
		case  1: m_x.w   = value;    break;  // X
		case  2: m_y.w   = value;    break;  // Y
		case  3: m_u.w   = value;    break;  // U
		case  4: m_s.w   = value;    break;  // S
		case  5: m_pc.w  = value;    break;  // PC
		case  8: m_q.r.a = (uint8_t)value; break;  // A
		case  9: m_q.r.b = (uint8_t)value; break;  // B
		case 10: m_cc    = (uint8_t)value; break;  // CC
		case 11: m_dp    = (uint8_t)value; break;  // DP
	}
}


//-------------------------------------------------
//  log_illegal - used to log hits to illegal
//  instructions (except for HD6309 which traps)
//-------------------------------------------------

void m6809_base_device::log_illegal()
{
	logerror("%s: illegal opcode at %04x\n", machine().describe_context(), (unsigned) m_pc.w);
}


//-------------------------------------------------
//  execute_one - try to execute a single instruction
//-------------------------------------------------

void m6809_base_device::execute_one()
{
	switch(pop_state())
	{
#include "cpu/m6809/m6809.hxx"
	}
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void m6809_base_device::execute_run()
{
	do
	{
		execute_one();
	} while(m_icount > 0);
}


uint8_t m6809_base_device::mi_default::read(uint16_t adr)
{
	return program.read_byte(adr);
}

uint8_t m6809_base_device::mi_default::read_opcode(uint16_t adr)
{
	return csprogram.read_byte(adr);
}

uint8_t m6809_base_device::mi_default::read_opcode_arg(uint16_t adr)
{
	return cprogram.read_byte(adr);
}


void m6809_base_device::mi_default::write(uint16_t adr, uint8_t val)
{
	program.write_byte(adr, val);
}



//-------------------------------------------------
//  mc6809_device
//-------------------------------------------------

mc6809_device::mc6809_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6809_base_device(mconfig, tag, owner, clock, MC6809, 4)
{
}



//-------------------------------------------------
//  mc6809e_device
//-------------------------------------------------

mc6809e_device::mc6809e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6809_base_device(mconfig, tag, owner, clock, MC6809E, 1)
{
}



//-------------------------------------------------
//  m6809_device
//-------------------------------------------------

m6809_device::m6809_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: m6809_base_device(mconfig, tag, owner, clock, M6809, 1)
{
}
