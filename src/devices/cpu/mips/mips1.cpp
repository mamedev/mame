// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Patrick Mackinlay

/*
 * MIPS-I emulation, including R2000[A], R3000[A] and IDT R30xx devices. The
 * IDT devices come in two variations: those with an "E" suffix include a TLB,
 * while those without have hard-wired address translation.
 *
 * TODO
 *   - R3041 features
 *   - cache emulation
 *
 */
#include "emu.h"
#include "mips1.h"
#include "mips1dsm.h"
#include "debugger.h"
#include "softfloat3/source/include/softfloat.h"

#define LOG_GENERAL (1U << 0)
#define LOG_TLB     (1U << 1)
#define LOG_IOP     (1U << 2)
#define LOG_RISCOS  (1U << 3)

//#define VERBOSE     (LOG_GENERAL|LOG_TLB)

#include "logmacro.h"

#define RSREG           ((op >> 21) & 31)
#define RTREG           ((op >> 16) & 31)
#define RDREG           ((op >> 11) & 31)
#define SHIFT           ((op >> 6) & 31)

#define FTREG           ((op >> 16) & 31)
#define FSREG           ((op >> 11) & 31)
#define FDREG           ((op >> 6) & 31)

#define SIMMVAL         s16(op)
#define UIMMVAL         u16(op)
#define LIMMVAL         (op & 0x03ffffff)

#define SR              m_cop0[COP0_Status]
#define CAUSE           m_cop0[COP0_Cause]

DEFINE_DEVICE_TYPE(R2000,       r2000_device,     "r2000",   "MIPS R2000")
DEFINE_DEVICE_TYPE(R2000A,      r2000a_device,    "r2000a",  "MIPS R2000A")
DEFINE_DEVICE_TYPE(R3000,       r3000_device,     "r3000",   "MIPS R3000")
DEFINE_DEVICE_TYPE(R3000A,      r3000a_device,    "r3000a",  "MIPS R3000A")
DEFINE_DEVICE_TYPE(R3041,       r3041_device,     "r3041",   "IDT R3041")
DEFINE_DEVICE_TYPE(R3051,       r3051_device,     "r3051",   "IDT R3051")
DEFINE_DEVICE_TYPE(R3052,       r3052_device,     "r3052",   "IDT R3052")
DEFINE_DEVICE_TYPE(R3052E,      r3052e_device,    "r3052e",  "IDT R3052E")
DEFINE_DEVICE_TYPE(R3071,       r3071_device,     "r3071",   "IDT R3071")
DEFINE_DEVICE_TYPE(R3081,       r3081_device,     "r3081",   "IDT R3081")
DEFINE_DEVICE_TYPE(SONYPS2_IOP, iop_device,       "sonyiop", "Sony Playstation 2 IOP")

ALLOW_SAVE_TYPE(mips1core_device_base::branch_state);

mips1core_device_base::mips1core_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, u32 cpurev, size_t icache_size, size_t dcache_size)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config_be("program", ENDIANNESS_BIG, 32, 32)
	, m_program_config_le("program", ENDIANNESS_LITTLE, 32, 32)
	, m_icache_config("icache", ENDIANNESS_BIG, 32, 32)
	, m_dcache_config("dcache", ENDIANNESS_BIG, 32, 32)
	, m_cpurev(cpurev)
	, m_endianness(ENDIANNESS_BIG)
	, m_icount(0)
	, m_icache_size(icache_size)
	, m_dcache_size(dcache_size)
	, m_in_brcond(*this)
{
}

mips1_device_base::mips1_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, u32 cpurev, size_t icache_size, size_t dcache_size)
	: mips1core_device_base(mconfig, type, tag, owner, clock, cpurev, icache_size, dcache_size)
	, m_fcr0(0)
{
}

r2000_device::r2000_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1_device_base(mconfig, R2000, tag, owner, clock, 0x0100, icache_size, dcache_size)
{
}

r2000a_device::r2000a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1_device_base(mconfig, R2000A, tag, owner, clock, 0x0210, icache_size, dcache_size)
{
}

r3000_device::r3000_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1_device_base(mconfig, R3000, tag, owner, clock, 0x0220, icache_size, dcache_size)
{
}

r3000a_device::r3000a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1_device_base(mconfig, R3000A, tag, owner, clock, 0x0230, icache_size, dcache_size)
{
}

r3041_device::r3041_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: mips1core_device_base(mconfig, R3041, tag, owner, clock, 0x0700, 2048, 512)
{
}

r3051_device::r3051_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: mips1core_device_base(mconfig, R3051, tag, owner, clock, 0x0200, 4096, 2048)
{
}

r3052_device::r3052_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: mips1core_device_base(mconfig, R3052, tag, owner, clock, 0x0200, 8192, 2048)
{
}

r3052e_device::r3052e_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: mips1_device_base(mconfig, R3052E, tag, owner, clock, 0x0200, 8192, 2048)
{
}

r3071_device::r3071_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1_device_base(mconfig, R3071, tag, owner, clock, 0x0200, icache_size, dcache_size)
{
}

r3081_device::r3081_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1_device_base(mconfig, R3081, tag, owner, clock, 0x0200, icache_size, dcache_size)
{
	set_fpu(0x0300);
}

iop_device::iop_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: mips1core_device_base(mconfig, SONYPS2_IOP, tag, owner, clock, 0x001f, 4096, 1024)
{
	m_endianness = ENDIANNESS_LITTLE;
}

/*
 * Two additional address spaces are defined to represent the instruction and
 * data caches. These are only used to simulate cache isolation functionality
 * at this point, but could simulate other behaviour as needed in future.
 */
void mips1core_device_base::device_add_mconfig(machine_config &config)
{
	set_addrmap(1, &mips1core_device_base::icache_map);
	set_addrmap(2, &mips1core_device_base::dcache_map);
}

void mips1core_device_base::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	// resolve conditional branch input handlers
	m_in_brcond.resolve_all_safe(0);

	// register our state for the debugger
	state_add(STATE_GENPC,      "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE,  "CURPC",     m_pc).noshow();

	state_add(MIPS1_PC,                   "PC",        m_pc);
	state_add(MIPS1_COP0 + COP0_Status,   "SR",        m_cop0[COP0_Status]);

	for (unsigned i = 0; i < std::size(m_r); i++)
		state_add(MIPS1_R0 + i, util::string_format("R%d", i).c_str(), m_r[i]);

	state_add(MIPS1_HI, "HI", m_hi);
	state_add(MIPS1_LO, "LO", m_lo);

	// cop0 exception registers
	state_add(MIPS1_COP0 + COP0_BadVAddr, "BadVAddr", m_cop0[COP0_BadVAddr]);
	state_add(MIPS1_COP0 + COP0_Cause,    "Cause",    m_cop0[COP0_Cause]);
	state_add(MIPS1_COP0 + COP0_EPC,      "EPC",      m_cop0[COP0_EPC]);

	// register our state for saving
	save_item(NAME(m_pc));
	save_item(NAME(m_hi));
	save_item(NAME(m_lo));
	save_item(NAME(m_r));
	save_item(NAME(m_cop0));
	save_item(NAME(m_branch_state));
	save_item(NAME(m_branch_target));

	// initialise cpu id register
	m_cop0[COP0_PRId] = m_cpurev;

	m_cop0[COP0_Cause] = 0;

	m_r[0] = 0;
}

void r3041_device::device_start()
{
	mips1core_device_base::device_start();

	// cop0 r3041 registers
	state_add(MIPS1_COP0 + COP0_BusCtrl,  "BusCtrl", m_cop0[COP0_BusCtrl]);
	state_add(MIPS1_COP0 + COP0_Config,   "Config", m_cop0[COP0_Config]);
	state_add(MIPS1_COP0 + COP0_Count,    "Count", m_cop0[COP0_Count]);
	state_add(MIPS1_COP0 + COP0_PortSize, "PortSize", m_cop0[COP0_PortSize]);
	state_add(MIPS1_COP0 + COP0_Compare,  "Compare", m_cop0[COP0_Compare]);

	m_cop0[COP0_BusCtrl] = 0x20130b00U;
	m_cop0[COP0_Config] = 0x40000000U;
	m_cop0[COP0_PortSize] = 0;
}

void mips1core_device_base::device_reset()
{
	// initialize the state
	m_pc = 0xbfc00000;
	m_branch_state = NONE;

	// non-tlb devices have tlb shut down
	m_cop0[COP0_Status] = SR_BEV | SR_TS;

	m_data_spacenum = 0;
	m_bus_error = false;
}

void r3041_device::device_reset()
{
	mips1core_device_base::device_reset();

	m_cop0[COP0_Count] = 0;
	m_cop0[COP0_Compare] = 0x00ffffffU;
}

void mips1core_device_base::execute_run()
{
	// core execution loop
	while (m_icount-- > 0)
	{
		// debugging
		debugger_instruction_hook(m_pc);

		if (VERBOSE & LOG_IOP)
		{
			if ((m_pc & 0x1fffffff) == 0x00012C48 || (m_pc & 0x1fffffff) == 0x0001420C || (m_pc & 0x1fffffff) == 0x0001430C)
			{
				u32 ptr = m_r[5];
				u32 length = m_r[6];
				if (length >= 4096)
					length = 4095;
				while (length)
				{
					load<u8>(ptr, [](char c) { printf("%c", c); });
					ptr++;
					length--;
				}
				fflush(stdout);
			}
		}

		// fetch instruction
		fetch(m_pc, [this](u32 const op)
		{
			// check for interrupts
			if ((CAUSE & SR & SR_IM) && (SR & SR_IEc))
			{
				generate_exception(EXCEPTION_INTERRUPT);
				return;
			}

			// decode and execute instruction
			switch (op >> 26)
			{
			case 0x00: // SPECIAL
				switch (op & 63)
				{
				case 0x00: // SLL
					m_r[RDREG] = m_r[RTREG] << SHIFT;
					break;
				case 0x02: // SRL
					m_r[RDREG] = m_r[RTREG] >> SHIFT;
					break;
				case 0x03: // SRA
					m_r[RDREG] = s32(m_r[RTREG]) >> SHIFT;
					break;
				case 0x04: // SLLV
					m_r[RDREG] = m_r[RTREG] << (m_r[RSREG] & 31);
					break;
				case 0x06: // SRLV
					m_r[RDREG] = m_r[RTREG] >> (m_r[RSREG] & 31);
					break;
				case 0x07: // SRAV
					m_r[RDREG] = s32(m_r[RTREG]) >> (m_r[RSREG] & 31);
					break;
				case 0x08: // JR
					m_branch_state = BRANCH;
					m_branch_target = m_r[RSREG];
					break;
				case 0x09: // JALR
					m_branch_state = BRANCH;
					m_branch_target = m_r[RSREG];
					m_r[RDREG] = m_pc + 8;
					break;
				case 0x0c: // SYSCALL
					generate_exception(EXCEPTION_SYSCALL);
					break;
				case 0x0d: // BREAK
					generate_exception(EXCEPTION_BREAK);
					break;
				case 0x10: // MFHI
					m_r[RDREG] = m_hi;
					break;
				case 0x11: // MTHI
					m_hi = m_r[RSREG];
					break;
				case 0x12: // MFLO
					m_r[RDREG] = m_lo;
					break;
				case 0x13: // MTLO
					m_lo = m_r[RSREG];
					break;
				case 0x18: // MULT
					{
						u64 product = mul_32x32(m_r[RSREG], m_r[RTREG]);

						m_lo = product;
						m_hi = product >> 32;
						m_icount -= 11;
					}
					break;
				case 0x19: // MULTU
					{
						u64 product = mulu_32x32(m_r[RSREG], m_r[RTREG]);

						m_lo = product;
						m_hi = product >> 32;
						m_icount -= 11;
					}
					break;
				case 0x1a: // DIV
					if (m_r[RTREG])
					{
						m_lo = s32(m_r[RSREG]) / s32(m_r[RTREG]);
						m_hi = s32(m_r[RSREG]) % s32(m_r[RTREG]);
					}
					m_icount -= 34;
					break;
				case 0x1b: // DIVU
					if (m_r[RTREG])
					{
						m_lo = m_r[RSREG] / m_r[RTREG];
						m_hi = m_r[RSREG] % m_r[RTREG];
					}
					m_icount -= 34;
					break;
				case 0x20: // ADD
					{
						u32 const sum = m_r[RSREG] + m_r[RTREG];

						// overflow: (sign(addend0) == sign(addend1)) && (sign(addend0) != sign(sum))
						if (!BIT(m_r[RSREG] ^ m_r[RTREG], 31) && BIT(m_r[RSREG] ^ sum, 31))
							generate_exception(EXCEPTION_OVERFLOW);
						else
							m_r[RDREG] = sum;
					}
					break;
				case 0x21: // ADDU
					m_r[RDREG] = m_r[RSREG] + m_r[RTREG];
					break;
				case 0x22: // SUB
					{
						u32 const difference = m_r[RSREG] - m_r[RTREG];

						// overflow: (sign(minuend) != sign(subtrahend)) && (sign(minuend) != sign(difference))
						if (BIT(m_r[RSREG] ^ m_r[RTREG], 31) && BIT(m_r[RSREG] ^ difference, 31))
							generate_exception(EXCEPTION_OVERFLOW);
						else
							m_r[RDREG] = difference;
					}
					break;
				case 0x23: // SUBU
					m_r[RDREG] = m_r[RSREG] - m_r[RTREG];
					break;
				case 0x24: // AND
					m_r[RDREG] = m_r[RSREG] & m_r[RTREG];
					break;
				case 0x25: // OR
					m_r[RDREG] = m_r[RSREG] | m_r[RTREG];
					break;
				case 0x26: // XOR
					m_r[RDREG] = m_r[RSREG] ^ m_r[RTREG];
					break;
				case 0x27: // NOR
					m_r[RDREG] = ~(m_r[RSREG] | m_r[RTREG]);
					break;
				case 0x2a: // SLT
					m_r[RDREG] = s32(m_r[RSREG]) < s32(m_r[RTREG]);
					break;
				case 0x2b: // SLTU
					m_r[RDREG] = u32(m_r[RSREG]) < u32(m_r[RTREG]);
					break;
				default:
					generate_exception(EXCEPTION_INVALIDOP);
					break;
				}
				break;
			case 0x01: // REGIMM
				/*
				 * Hardware testing has established that MIPS-1 processors do
				 * not decode bit 17 of REGIMM format instructions. This bit is
				 * used to add the "branch likely" instructions for MIPS-2 and
				 * later architectures.
				 *
				 * IRIX 5.3 inst(1M) uses this behaviour to distinguish MIPS-1
				 * from MIPS-2 processors; the latter nullify the delay slot
				 * instruction if the branch is not taken, whereas the former
				 * execute the delay slot instruction regardless.
				 */
				switch (RTREG & 0x1d)
				{
				case 0x00: // BLTZ
					if (s32(m_r[RSREG]) < 0)
					{
						m_branch_state = BRANCH;
						m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
					}
					break;
				case 0x01: // BGEZ
					if (s32(m_r[RSREG]) >= 0)
					{
						m_branch_state = BRANCH;
						m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
					}
					break;
				case 0x10: // BLTZAL
					if (s32(m_r[RSREG]) < 0)
					{
						m_branch_state = BRANCH;
						m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
						m_r[31] = m_pc + 8;
					}
					break;
				case 0x11: // BGEZAL
					if (s32(m_r[RSREG]) >= 0)
					{
						m_branch_state = BRANCH;
						m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
						m_r[31] = m_pc + 8;
					}
					break;
				default:
					generate_exception(EXCEPTION_INVALIDOP);
					break;
				}
				break;
			case 0x02: // J
				m_branch_state = BRANCH;
				m_branch_target = ((m_pc + 4) & 0xf0000000) | (LIMMVAL << 2);
				break;
			case 0x03: // JAL
				m_branch_state = BRANCH;
				m_branch_target = ((m_pc + 4) & 0xf0000000) | (LIMMVAL << 2);
				m_r[31] = m_pc + 8;
				break;
			case 0x04: // BEQ
				if (m_r[RSREG] == m_r[RTREG])
				{
					m_branch_state = BRANCH;
					m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
				}
				break;
			case 0x05: // BNE
				if (m_r[RSREG] != m_r[RTREG])
				{
					m_branch_state = BRANCH;
					m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
				}
				break;
			case 0x06: // BLEZ
				if (s32(m_r[RSREG]) <= 0)
				{
					m_branch_state = BRANCH;
					m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
				}
				break;
			case 0x07: // BGTZ
				if (s32(m_r[RSREG]) > 0)
				{
					m_branch_state = BRANCH;
					m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
				}
				break;
			case 0x08: // ADDI
				{
					u32 const sum = m_r[RSREG] + SIMMVAL;

					// overflow: (sign(addend0) == sign(addend1)) && (sign(addend0) != sign(sum))
					if (!BIT(m_r[RSREG] ^ s32(SIMMVAL), 31) && BIT(m_r[RSREG] ^ sum, 31))
						generate_exception(EXCEPTION_OVERFLOW);
					else
						m_r[RTREG] = sum;
				}
				break;
			case 0x09: // ADDIU
				m_r[RTREG] = m_r[RSREG] + SIMMVAL;
				break;
			case 0x0a: // SLTI
				m_r[RTREG] = s32(m_r[RSREG]) < s32(SIMMVAL);
				break;
			case 0x0b: // SLTIU
				m_r[RTREG] = u32(m_r[RSREG]) < u32(SIMMVAL);
				break;
			case 0x0c: // ANDI
				m_r[RTREG] = m_r[RSREG] & UIMMVAL;
				break;
			case 0x0d: // ORI
				m_r[RTREG] = m_r[RSREG] | UIMMVAL;
				break;
			case 0x0e: // XORI
				m_r[RTREG] = m_r[RSREG] ^ UIMMVAL;
				break;
			case 0x0f: // LUI
				m_r[RTREG] = UIMMVAL << 16;
				break;
			case 0x10: // COP0
				if (!(SR & SR_KUc) || (SR & SR_COP0))
					handle_cop0(op);
				else
					generate_exception(EXCEPTION_BADCOP0);
				break;
			case 0x11: // COP1
				handle_cop1(op);
				break;
			case 0x12: // COP2
				handle_cop2(op);
				break;
			case 0x13: // COP3
				handle_cop3(op);
				break;
			case 0x20: // LB
				load<u8>(SIMMVAL + m_r[RSREG], [this, op](s8 temp) { m_r[RTREG] = temp; });
				break;
			case 0x21: // LH
				load<u16>(SIMMVAL + m_r[RSREG], [this, op](s16 temp) { m_r[RTREG] = temp; });
				break;
			case 0x22: // LWL
				lwl(op);
				break;
			case 0x23: // LW
				load<u32>(SIMMVAL + m_r[RSREG], [this, op](u32 temp) { m_r[RTREG] = temp; });
				break;
			case 0x24: // LBU
				load<u8>(SIMMVAL + m_r[RSREG], [this, op](u8 temp) { m_r[RTREG] = temp; });
				break;
			case 0x25: // LHU
				load<u16>(SIMMVAL + m_r[RSREG], [this, op](u16 temp) { m_r[RTREG] = temp; });
				break;
			case 0x26: // LWR
				lwr(op);
				break;
			case 0x28: // SB
				store<u8>(SIMMVAL + m_r[RSREG], m_r[RTREG]);
				break;
			case 0x29: // SH
				store<u16>(SIMMVAL + m_r[RSREG], m_r[RTREG]);
				break;
			case 0x2a: // SWL
				swl(op);
				break;
			case 0x2b: // SW
				store<u32>(SIMMVAL + m_r[RSREG], m_r[RTREG]);
				break;
			case 0x2e: // SWR
				swr(op);
				break;
			case 0x31: // LWC1
				handle_cop1(op);
				break;
			case 0x32: // LWC2
				handle_cop2(op);
				break;
			case 0x33: // LWC3
				handle_cop3(op);
				break;
			case 0x39: // SWC1
				handle_cop1(op);
				break;
			case 0x3a: // SWC2
				handle_cop2(op);
				break;
			case 0x3b: // SWC3
				handle_cop3(op);
				break;
			default:
				generate_exception(EXCEPTION_INVALIDOP);
				break;
			}

			// clear register 0
			m_r[0] = 0;
		});

		// update pc and branch state
		switch (m_branch_state)
		{
		case NONE:
			m_pc += 4;
			break;

		case DELAY:
			m_branch_state = NONE;
			m_pc = m_branch_target;
			break;

		case BRANCH:
			m_branch_state = DELAY;
			m_pc += 4;
			break;

		case EXCEPTION:
			m_branch_state = NONE;
			break;
		}
	}
}

void mips1core_device_base::execute_set_input(int irqline, int state)
{
	if (state != CLEAR_LINE)
	{
		CAUSE |= CAUSE_IPEX0 << irqline;

		// enable debugger interrupt breakpoints
		if ((SR & SR_IEc) && (SR & (SR_IMEX0 << irqline)))
			standard_irq_callback(irqline);
	}
	else
		CAUSE &= ~(CAUSE_IPEX0 << irqline);
}

device_memory_interface::space_config_vector mips1core_device_base::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, (m_endianness == ENDIANNESS_BIG) ? &m_program_config_be : &m_program_config_le),
		std::make_pair(1, &m_icache_config),
		std::make_pair(2, &m_dcache_config)
	};
}

bool mips1core_device_base::memory_translate(int spacenum, int intention, offs_t &address)
{
	// check for kernel memory address
	if (BIT(address, 31))
	{
		// check debug or kernel mode
		if ((intention & TRANSLATE_DEBUG_MASK) || !(SR & SR_KUc))
		{
			switch (address & 0xe0000000)
			{
			case 0x80000000: // kseg0: unmapped, cached, privileged
			case 0xa0000000: // kseg1: unmapped, uncached, privileged
				address &= ~0xe0000000;
				break;

			case 0xc0000000: // kseg2: mapped, cached, privileged
			case 0xe0000000:
				break;
			}
		}
		else if (SR & SR_KUc)
		{
			address_error(intention, address);

			return false;
		}
	}
	else
		// kuseg physical addresses have a 1GB offset
		address += 0x40000000;

	return true;
}

std::unique_ptr<util::disasm_interface> mips1core_device_base::create_disassembler()
{
	return std::make_unique<mips1_disassembler>();
}

void mips1core_device_base::icache_map(address_map &map)
{
	if (m_icache_size)
		map(0, m_icache_size - 1).ram().mirror(~(m_icache_size - 1));
}

void mips1core_device_base::dcache_map(address_map &map)
{
	if (m_dcache_size)
		map(0, m_dcache_size - 1).ram().mirror(~(m_dcache_size - 1));
}

void mips1core_device_base::generate_exception(u32 exception, bool refill)
{
	if ((VERBOSE & LOG_RISCOS) && (exception == EXCEPTION_SYSCALL))
	{
		static char const *const sysv_syscalls[] =
		{
			"syscall",      "exit",         "fork",         "read",         "write",        "open",         "close",        "wait",         "creat",        "link",
			"unlink",       "execv",        "chdir",        "time",         "mknod",        "chmod",        "chown",        "brk",          "stat",         "lseek",
			"getpid",       "mount",        "umount",       "setuid",       "getuid",       "stime",        "ptrace",       "alarm",        "fstat",        "pause",
			"utime",        "stty",         "gtty",         "access",       "nice",         "statfs",       "sync",         "kill",         "fstatfs",      "setpgrp",
			nullptr,        "dup",          "pipe",         "times",        "profil",       "plock",        "setgid",       "getgid",       "signal",       "msgsys",
			"sysmips",      "acct",         "shmsys",       "semsys",       "ioctl",        "uadmin",       nullptr,        "utssys",       nullptr,        "execve",
			"umask",        "chroot",       "ofcntl",       "ulimit",       nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,
			"advfs",        "unadvfs",      "rmount",       "rumount",      "rfstart",      nullptr,        "rdebug",       "rfstop",       "rfsys",        "rmdir",
			"mkdir",        "getdents",     nullptr,        nullptr,        "sysfs",        "getmsg",       "putmsg",       "poll",         "sigreturn",    "accept",
			"bind",         "connect",      "gethostid",    "getpeername",  "getsockname",  "getsockopt",   "listen",       "recv",         "recvfrom",     "recvmsg",
			"select",       "send",         "sendmsg",      "sendto",       "sethostid",    "setsockopt",   "shutdown",     "socket",       "gethostname",  "sethostname",
			"getdomainname","setdomainname","truncate",     "ftruncate",    "rename",       "symlink",      "readlink",     "lstat",        "nfsmount",     "nfssvc",
			"getfh",        "async_daemon", "old_exportfs", "mmap",         "munmap",       "getitimer",    "setitimer",    nullptr,        nullptr,        nullptr,
			nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,
			nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,        nullptr,
			"cacheflush",   "cachectl",     "fchown",       "fchmod",       "wait3",        "mmap",         "munmap",       "madvise",      "getpagesize",  "setreuid",
			"setregid",     "setpgid",      "getgroups",    "setgroups",    "gettimeofday", "getrusage",    "getrlimit",    "setrlimit",    "exportfs",     "fcntl"
		};

		static char const *const bsd_syscalls[] =
		{
			"syscall",      "exit",         "fork",         "read",         "write",        "open",         "close",        nullptr,        "creat",        "link",
			"unlink",       "execv",        "chdir",        nullptr,        "mknod",        "chmod",        "chown",        "brk",          nullptr,        "lseek",
			"getpid",       "omount",       "oumount",      nullptr,        "getuid",       nullptr,        "ptrace",       nullptr,        nullptr,        nullptr,
			nullptr,        nullptr,        nullptr,        "access",       nullptr,        nullptr,        "sync",         "kill",         "stat",         nullptr,
			"lstat",        "dup",          "pipe",         nullptr,        "profil",       nullptr,        nullptr,        "getgid",       nullptr,        nullptr,
			nullptr,        "acct",         nullptr,        nullptr,        "ioctl",        "reboot",       nullptr,        "symlink",      "readlink",     "execve",
			"umask",        "chroot",       "fstat",        nullptr,        "getpagesize",  "mremap",       "vfork",        nullptr,        nullptr,        "sbrk",
			"sstk",         "mmap",         "vadvise",      "munmap",       "mprotec",      "madvise",      "vhangup",      nullptr,        "mincore",      "getgroups",
			"setgroups",    "getpgrp",      "setpgrp",      "setitimer",    "wait3",        "swapon",       "getitimer",    "gethostname",  "sethostname",  "getdtablesize",
			"dup2",         "getdopt",      "fcntl",        "select",       "setdopt",      "fsync",        "setpriority",  "socket",       "connect",      "accept",
			"getpriority",  "send",         "recv",         "sigreturn",    "bind",         "setsockopt",   "listen",       nullptr,        "sigvec",       "sigblock",
			"sigsetmask",   "sigpause",     "sigstack",     "recvmsg",      "sendmsg",      nullptr,        "gettimeofday", "getrusage",    "getsockopt",   nullptr,
			"readv",        "writev",       "settimeofday", "fchown",       "fchmod",       "recvfrom",     "setreuid",     "setregid",     "rename",       "truncate",
			"ftruncate",    "flock",        nullptr,        "sendto",       "shutdown",     "socketpair",   "mkdir",        "rmdir",        "utimes",       "sigcleanup",
			"adjtime",      "getpeername",  "gethostid",    "sethostid",    "getrlimit",    "setrlimit",    "killpg",       nullptr,        "setquota",     "quota",
			"getsockname",  "sysmips",      "cacheflush",   "cachectl",     "debug",        nullptr,        nullptr,        nullptr,        "nfssvc",       "getdirentries",
			"statfs",       "fstatfs",      "unmount",      "async_daemon", "getfh",        "getdomainname","setdomainname",nullptr,        "quotactl",     "old_exportfs",
			"mount",        "hdwconf",      "exportfs",     "nfsfh_open",   "libattach",    "libdetach"
		};

		static char const *const msg_syscalls[] = { "msgget", "msgctl", "msgrcv", "msgsnd" };
		static char const *const shm_syscalls[] = { "shmat", "shmctl", "shmdt", "shmget" };
		static char const *const sem_syscalls[] = { "semctl", "semget", "semop" };
		static char const *const mips_syscalls[] = { "mipskopt", "mipshwconf", "mipsgetrusage", "mipswait", "mipscacheflush", "mipscachectl" };

		unsigned const asid = (m_cop0[COP0_EntryHi] & EH_ASID) >> 6;
		switch (m_r[2])
		{
		case 1000: // indirect
			switch (m_r[4])
			{
			case 1049: // msgsys
				LOGMASKED(LOG_RISCOS, "asid %d syscall msgsys:%s() (%s)\n",
					asid, (m_r[5] < std::size(msg_syscalls)) ? msg_syscalls[m_r[5]] : "unknown", machine().describe_context());
				break;

			case 1052: // shmsys
				LOGMASKED(LOG_RISCOS, "asid %d syscall shmsys:%s() (%s)\n",
					asid, (m_r[5] < std::size(shm_syscalls)) ? shm_syscalls[m_r[5]] : "unknown", machine().describe_context());
				break;

			case 1053: // semsys
				LOGMASKED(LOG_RISCOS, "asid %d syscall semsys:%s() (%s)\n",
					asid, (m_r[5] < std::size(sem_syscalls)) ? sem_syscalls[m_r[5]] : "unknown", machine().describe_context());
				break;

			case 2151: // bsd_sysmips
				switch (m_r[5])
				{
				case 0x100: // mipskopt
					LOGMASKED(LOG_RISCOS, "asid %d syscall bsd_sysmips:mipskopt(\"%s\") (%s)\n",
						asid, debug_string(m_r[6]), machine().describe_context());
					break;

				default:
					if ((m_r[5] > 0x100) && (m_r[5] - 0x100) < std::size(mips_syscalls))
						LOGMASKED(LOG_RISCOS, "asid %d syscall bsd_sysmips:%s() (%s)\n",
							asid, mips_syscalls[m_r[5] - 0x100], machine().describe_context());
					else
						LOGMASKED(LOG_RISCOS, "asid %d syscall bsd_sysmips:unknown %d (%s)\n",
							asid, m_r[5], machine().describe_context());
					break;
				}
				break;

			default:
				if ((m_r[4] > 2000) && (m_r[4] - 2000 < std::size(bsd_syscalls)) && bsd_syscalls[m_r[4] - 2000])
					LOGMASKED(LOG_RISCOS, "asid %d syscall bsd_%s() (%s)\n",
						asid, bsd_syscalls[m_r[4] - 2000], machine().describe_context());
				else
					LOGMASKED(LOG_RISCOS, "asid %d syscall indirect:unknown %d (%s)\n",
						asid, m_r[4], machine().describe_context());
				break;
			}
			break;

		case 1003: // read
		case 1006: // close
		case 1054: // ioctl
		case 1169: // fcntl
			LOGMASKED(LOG_RISCOS, "asid %d syscall %s(%d) (%s)\n",
				asid, sysv_syscalls[m_r[2] - 1000], m_r[4], machine().describe_context());
			break;

		case 1004: // write
			if (m_r[4] == 1 || m_r[4] == 2)
				LOGMASKED(LOG_RISCOS, "asid %d syscall %s(%d, \"%s\") (%s)\n",
					asid, sysv_syscalls[m_r[2] - 1000], m_r[4], debug_string(m_r[5], m_r[6]), machine().describe_context());
			else
				LOGMASKED(LOG_RISCOS, "asid %d syscall %s(%d) (%s)\n",
					asid, sysv_syscalls[m_r[2] - 1000], m_r[4], machine().describe_context());
			break;

		case 1005: // open
		case 1008: // creat
		case 1009: // link
		case 1010: // unlink
		case 1012: // chdir
		case 1018: // stat
		case 1033: // access
			LOGMASKED(LOG_RISCOS, "asid %d syscall %s(\"%s\") (%s)\n",
				asid, sysv_syscalls[m_r[2] - 1000], debug_string(m_r[4]), machine().describe_context());
			break;

		case 1059: // execve
			LOGMASKED(LOG_RISCOS, "asid %d syscall execve(\"%s\", [ %s ], [ %s ]) (%s)\n",
				asid, debug_string(m_r[4]), debug_string_array(m_r[5]), debug_string_array(m_r[6]), machine().describe_context());
			break;

		case 1060: // umask
			LOGMASKED(LOG_RISCOS, "asid %d syscall umask(%#o) (%s)\n",
				asid, m_r[4] & 0777, machine().describe_context());
			break;

		default:
			if ((m_r[2] > 1000) && (m_r[2] - 1000 < std::size(sysv_syscalls)) && sysv_syscalls[m_r[2] - 1000])
				LOGMASKED(LOG_RISCOS, "asid %d syscall %s() (%s)\n", asid, sysv_syscalls[m_r[2] - 1000], machine().describe_context());
			else
				LOGMASKED(LOG_RISCOS, "asid %d syscall unknown %d (%s)\n", asid, m_r[2], machine().describe_context());
			break;
		}
	}

	// set the exception PC
	m_cop0[COP0_EPC] = m_pc;

	// load the cause register
	CAUSE = (CAUSE & CAUSE_IP) | exception;

	// if in a branch delay slot, restart the branch
	if (m_branch_state == DELAY)
	{
		m_cop0[COP0_EPC] -= 4;
		CAUSE |= CAUSE_BD;
	}
	m_branch_state = EXCEPTION;

	// shift the exception bits
	SR = (SR & ~SR_KUIE) | ((SR << 2) & SR_KUIEop);

	if (refill)
		m_pc = (SR & SR_BEV) ? 0xbfc00100 : 0x80000000;
	else
		m_pc = (SR & SR_BEV) ? 0xbfc00180 : 0x80000080;

	debugger_exception_hook(exception);

	if (SR & SR_KUp)
		debugger_privilege_hook();
}

void mips1core_device_base::address_error(int intention, u32 const address)
{
	if (!machine().side_effects_disabled() && !(intention & TRANSLATE_DEBUG_MASK))
	{
		logerror("address_error 0x%08x (%s)\n", address, machine().describe_context());

		m_cop0[COP0_BadVAddr] = address;

		generate_exception((intention & TRANSLATE_WRITE) ? EXCEPTION_ADDRSTORE : EXCEPTION_ADDRLOAD);

		// address errors shouldn't typically occur, so a breakpoint is handy
		machine().debug_break();
	}
}

void mips1core_device_base::handle_cop0(u32 const op)
{
	switch (RSREG)
	{
	case 0x00: // MFC0
		m_r[RTREG] = get_cop0_reg(RDREG);
		break;
	case 0x04: // MTC0
		set_cop0_reg(RDREG, m_r[RTREG]);
		break;
	case 0x08: // BC0
		switch (RTREG)
		{
		case 0x00: // BC0F
			if (!m_in_brcond[0]())
			{
				m_branch_state = BRANCH;
				m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
			}
			break;
		case 0x01: // BC0T
			if (m_in_brcond[0]())
			{
				m_branch_state = BRANCH;
				m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
			}
			break;
		default:
			generate_exception(EXCEPTION_INVALIDOP);
			break;
		}
		break;
	case 0x10: // COP0
		switch (op & 31)
		{
			case 0x10: // RFE
				SR = (SR & ~SR_KUIE) | ((SR >> 2) & SR_KUIEpc);
				if (bool(SR & SR_KUc) ^ bool(SR & SR_KUp))
					debugger_privilege_hook();
				break;
			default:
				generate_exception(EXCEPTION_INVALIDOP);
				break;
		}
		break;
	default:
		generate_exception(EXCEPTION_INVALIDOP);
		break;
	}
}

u32 mips1core_device_base::get_cop0_reg(unsigned const reg)
{
	return m_cop0[reg];
}

void mips1core_device_base::set_cop0_reg(unsigned const reg, u32 const data)
{
	switch (reg)
	{
	case COP0_Status:
		{
			u32 const delta = SR ^ data;

			m_cop0[COP0_Status] = data;

			// handle cache isolation and swap
			m_data_spacenum = (data & SR_IsC) ? ((data & SR_SwC) ? 1 : 2) : 0;

			if ((delta & SR_KUc) && (m_branch_state != EXCEPTION))
				debugger_privilege_hook();
		}
		break;

	case COP0_Cause:
		CAUSE = (CAUSE & CAUSE_IPEX) | (data & ~CAUSE_IPEX);
		break;

	case COP0_PRId:
		// read-only register
		break;

	default:
		m_cop0[reg] = data;
		break;
	}
}

void mips1core_device_base::handle_cop1(u32 const op)
{
	if (!(SR & SR_COP1))
		generate_exception(EXCEPTION_BADCOP1);
}

void mips1core_device_base::handle_cop2(u32 const op)
{
	if (SR & SR_COP2)
	{
		switch (RSREG)
		{
		case 0x08: // BC2
			switch (RTREG)
			{
			case 0x00: // BC2F
				if (!m_in_brcond[2]())
				{
					m_branch_state = BRANCH;
					m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
				}
				break;
			case 0x01: // BC2T
				if (m_in_brcond[2]())
				{
					m_branch_state = BRANCH;
					m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
				}
				break;
			default:
				generate_exception(EXCEPTION_INVALIDOP);
				break;
			}
			break;
		default:
			generate_exception(EXCEPTION_INVALIDOP);
			break;
		}
	}
	else
		generate_exception(EXCEPTION_BADCOP2);
}

void mips1core_device_base::handle_cop3(u32 const op)
{
	if (SR & SR_COP3)
	{
		switch (RSREG)
		{
		case 0x08: // BC3
			switch (RTREG)
			{
			case 0x00: // BC3F
				if (!m_in_brcond[3]())
				{
					m_branch_state = BRANCH;
					m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
				}
				break;
			case 0x01: // BC3T
				if (m_in_brcond[3]())
				{
					m_branch_state = BRANCH;
					m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
				}
				break;
			default:
				generate_exception(EXCEPTION_INVALIDOP);
				break;
			}
			break;
		default:
			generate_exception(EXCEPTION_INVALIDOP);
			break;
		}
	}
	else
		generate_exception(EXCEPTION_BADCOP3);
}

void mips1core_device_base::lwl(u32 const op)
{
	offs_t const offset = SIMMVAL + m_r[RSREG];
	load<u32, false>(offset, [this, op, offset](u32 temp)
	{
		unsigned const shift = ((offset & 3) ^ (m_endianness == ENDIANNESS_LITTLE ? 3 : 0)) << 3;

		m_r[RTREG] = (m_r[RTREG] & ~u32(0xffffffffU << shift)) | (temp << shift);
	});
}

void mips1core_device_base::lwr(u32 const op)
{
	offs_t const offset = SIMMVAL + m_r[RSREG];
	load<u32, false>(offset, [this, op, offset](u32 temp)
	{
		unsigned const shift = ((offset & 0x3) ^ (m_endianness == ENDIANNESS_LITTLE ? 0 : 3)) << 3;

		m_r[RTREG] = (m_r[RTREG] & ~u32(0xffffffffU >> shift)) | (temp >> shift);
	});
}

void mips1core_device_base::swl(u32 const op)
{
	offs_t const offset = SIMMVAL + m_r[RSREG];
	unsigned const shift = ((offset & 3) ^ (m_endianness == ENDIANNESS_LITTLE ? 3 : 0)) << 3;

	store<u32, false>(offset, m_r[RTREG] >> shift, 0xffffffffU >> shift);
}

void mips1core_device_base::swr(u32 const op)
{
	offs_t const offset = SIMMVAL + m_r[RSREG];
	unsigned const shift = ((offset & 3) ^ (m_endianness == ENDIANNESS_LITTLE ? 0 : 3)) << 3;

	store<u32, false>(offset, m_r[RTREG] << shift, 0xffffffffU << shift);
}

template <typename T, bool Aligned, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(T)>>::value, void> mips1core_device_base::load(u32 address, U &&apply)
{
	// alignment error
	if (Aligned && (address & (sizeof(T) - 1)))
	{
		address_error(TRANSLATE_READ, address);
		return;
	}

	if (memory_translate(m_data_spacenum, TRANSLATE_READ, address))
	{
		// align address for ld[lr] instructions
		if (!Aligned)
			address &= ~(sizeof(T) - 1);

		T const data
			= (sizeof(T) == 1) ? space(m_data_spacenum).read_byte(address)
			: (sizeof(T) == 2) ? space(m_data_spacenum).read_word(address)
			: space(m_data_spacenum).read_dword(address);

		if (m_bus_error)
		{
			m_bus_error = false;
			generate_exception(EXCEPTION_BUSDATA);
		}
		else
			apply(data);
	}
}

template <typename T, bool Aligned, typename U> std::enable_if_t<std::is_convertible<U, T>::value, void> mips1core_device_base::store(u32 address, U data, T mem_mask)
{
	// alignment error
	if (Aligned && (address & (sizeof(T) - 1)))
	{
		address_error(TRANSLATE_WRITE, address);
		return;
	}

	if (memory_translate(m_data_spacenum, TRANSLATE_WRITE, address))
	{
		// align address for sd[lr] instructions
		if (!Aligned)
			address &= ~(sizeof(T) - 1);

		switch (sizeof(T))
		{
		case 1: space(m_data_spacenum).write_byte(address, T(data)); break;
		case 2: space(m_data_spacenum).write_word(address, T(data), mem_mask); break;
		case 4: space(m_data_spacenum).write_dword(address, T(data), mem_mask); break;
		}
	}
}

bool mips1core_device_base::fetch(u32 address, std::function<void(u32)> &&apply)
{
	// alignment error
	if (address & 3)
	{
		address_error(TRANSLATE_FETCH, address);
		return false;
	}

	if (memory_translate(0, TRANSLATE_FETCH, address))
	{
		u32 const data = space(0).read_dword(address);

		if (m_bus_error)
		{
			m_bus_error = false;
			generate_exception(EXCEPTION_BUSINST);

			return false;
		}

		apply(data);
		return true;
	}
	else
		return false;
}

std::string mips1core_device_base::debug_string(u32 string_pointer, unsigned const limit)
{
	auto const suppressor(machine().disable_side_effects());

	bool done = false;
	bool mapped = false;
	std::string result("");

	while (!done)
	{
		done = true;
		load<u8>(string_pointer++, [limit, &done, &mapped, &result](u8 byte)
		{
			mapped = true;
			if (byte != 0)
			{
				result += byte;

				done = result.length() == limit;
			}
		});
	}

	if (!mapped)
		result.assign("[unmapped]");

	return result;
}

std::string mips1core_device_base::debug_string_array(u32 array_pointer)
{
	auto const suppressor(machine().disable_side_effects());

	bool done = false;
	std::string result("");

	while (!done)
	{
		done = true;
		load<u32>(array_pointer, [this, &done, &result](u32 string_pointer)
		{
			if (string_pointer != 0)
			{
				if (!result.empty())
					result += ", ";

				result += '\"' + debug_string(string_pointer) + '\"';

				done = false;
			}
		});

		array_pointer += 4;
	}

	return result;
}

void mips1_device_base::device_start()
{
	mips1core_device_base::device_start();

	// cop0 tlb registers
	state_add(MIPS1_COP0 + COP0_Index, "Index", m_cop0[COP0_Index]);
	state_add(MIPS1_COP0 + COP0_Random, "Random", m_cop0[COP0_Random]);
	state_add(MIPS1_COP0 + COP0_EntryLo, "EntryLo", m_cop0[COP0_EntryLo]);
	state_add(MIPS1_COP0 + COP0_EntryHi, "EntryHi", m_cop0[COP0_EntryHi]);
	state_add(MIPS1_COP0 + COP0_Context, "Context", m_cop0[COP0_Context]);

	// cop1 registers
	if (m_fcr0)
	{
		state_add(MIPS1_FCR31, "FCSR", m_fcr31);
		for (unsigned i = 0; i < std::size(m_f); i++)
			state_add(MIPS1_F0 + i, util::string_format("F%d", i * 2).c_str(), m_f[i]);
	}

	save_item(NAME(m_reset_time));
	save_item(NAME(m_tlb));

	save_item(NAME(m_fcr30));
	save_item(NAME(m_fcr31));
	save_item(NAME(m_f));
}

void mips1_device_base::device_reset()
{
	mips1core_device_base::device_reset();

	// tlb is not shut down
	m_cop0[COP0_Status] &= ~SR_TS;

	m_reset_time = total_cycles();

	// initialize tlb mru index with identity mapping
	for (unsigned i = 0; i < std::size(m_tlb); i++)
	{
		m_tlb_mru[TRANSLATE_READ][i] = i;
		m_tlb_mru[TRANSLATE_WRITE][i] = i;
		m_tlb_mru[TRANSLATE_FETCH][i] = i;
	}
}

void mips1_device_base::handle_cop0(u32 const op)
{
	switch (op)
	{
	case 0x42000001: // TLBR - read tlb
		{
			u8 const index = (m_cop0[COP0_Index] >> 8) & 0x3f;

			m_cop0[COP0_EntryHi] = m_tlb[index][0];
			m_cop0[COP0_EntryLo] = m_tlb[index][1];
		}
		break;

	case 0x42000002: // TLBWI - write tlb (indexed)
		{
			u8 const index = (m_cop0[COP0_Index] >> 8) & 0x3f;

			m_tlb[index][0] = m_cop0[COP0_EntryHi];
			m_tlb[index][1] = m_cop0[COP0_EntryLo];

			LOGMASKED(LOG_TLB, "asid %2d tlb write index %2d vpn 0x%08x pfn 0x%08x %c%c%c%c (%s)\n",
				(m_cop0[COP0_EntryHi] & EH_ASID) >> 6, index, m_cop0[COP0_EntryHi] & EH_VPN, m_cop0[COP0_EntryLo] & EL_PFN,
				m_cop0[COP0_EntryLo] & EL_N ? 'N' : '-',
				m_cop0[COP0_EntryLo] & EL_D ? 'D' : '-',
				m_cop0[COP0_EntryLo] & EL_V ? 'V' : '-',
				m_cop0[COP0_EntryLo] & EL_G ? 'G' : '-',
				machine().describe_context());
		}
		break;

	case 0x42000006: // TLBWR - write tlb (random)
		{
			u8 const random = get_cop0_reg(COP0_Random) >> 8;

			m_tlb[random][0] = m_cop0[COP0_EntryHi];
			m_tlb[random][1] = m_cop0[COP0_EntryLo];

			LOGMASKED(LOG_TLB, "asid %2d tlb write random %2d vpn 0x%08x pfn 0x%08x %c%c%c%c (%s)\n",
				(m_cop0[COP0_EntryHi] & EH_ASID) >> 6, random, m_cop0[COP0_EntryHi] & EH_VPN, m_cop0[COP0_EntryLo] & EL_PFN,
				m_cop0[COP0_EntryLo] & EL_N ? 'N' : '-',
				m_cop0[COP0_EntryLo] & EL_D ? 'D' : '-',
				m_cop0[COP0_EntryLo] & EL_V ? 'V' : '-',
				m_cop0[COP0_EntryLo] & EL_G ? 'G' : '-',
				machine().describe_context());
		}
		break;

	case 0x42000008: // TLBP - probe tlb
		m_cop0[COP0_Index] = 0x80000000;
		for (u8 index = 0; index < 64; index++)
		{
			// test vpn and optionally asid
			u32 const mask = (m_tlb[index][1] & EL_G) ? EH_VPN : EH_VPN | EH_ASID;
			if ((m_tlb[index][0] & mask) == (m_cop0[COP0_EntryHi] & mask))
			{
				LOGMASKED(LOG_TLB, "asid %2d tlb probe index %2d vpn 0x%08x (%s)\n",
					(m_cop0[COP0_EntryHi] & EH_ASID) >> 6, index, m_cop0[COP0_EntryHi] & mask, machine().describe_context());

				m_cop0[COP0_Index] = index << 8;
				break;
			}
		}
		if ((VERBOSE & LOG_TLB) && BIT(m_cop0[COP0_Index], 31))
			LOGMASKED(LOG_TLB, "asid %2d tlb probe miss vpn 0x%08x(%s)\n",
				(m_cop0[COP0_EntryHi] & EH_ASID) >> 6, m_cop0[COP0_EntryHi] & EH_VPN, machine().describe_context());
		break;

	default:
		mips1core_device_base::handle_cop0(op);
	}
}

u32 mips1_device_base::get_cop0_reg(unsigned const reg)
{
	// assume 64-entry tlb with 8 wired entries
	if (reg == COP0_Random)
		m_cop0[reg] = (63 - ((total_cycles() - m_reset_time) % 56)) << 8;

	return m_cop0[reg];
}

void mips1_device_base::set_cop0_reg(unsigned const reg, u32 const data)
{
	switch (reg)
	{
	case COP0_EntryHi:
		m_cop0[COP0_EntryHi] = data & EH_WM;
		break;

	case COP0_EntryLo:
		m_cop0[COP0_EntryLo] = data & EL_WM;
		break;

	case COP0_Context:
		m_cop0[COP0_Context] = (m_cop0[COP0_Context] & ~PTE_BASE) | (data & PTE_BASE);
		break;

	default:
		mips1core_device_base::set_cop0_reg(reg, data);
		break;
	}
}

void mips1_device_base::handle_cop1(u32 const op)
{
	if (!(SR & SR_COP1))
	{
		generate_exception(EXCEPTION_BADCOP1);
		return;
	}

	if (!m_fcr0)
		return;

	softfloat_exceptionFlags = 0;

	switch (op >> 26)
	{
	case 0x11: // COP1
		switch ((op >> 21) & 0x1f)
		{
		case 0x00: // MFC1
			if (FSREG & 1)
				// move the high half of the floating point register
				m_r[RTREG] = m_f[FSREG >> 1] >> 32;
			else
				// move the low half of the floating point register
				m_r[RTREG] = m_f[FSREG >> 1] >> 0;
			break;
		case 0x02: // CFC1
			switch (FSREG)
			{
			case 0:  m_r[RTREG] = m_fcr0; break;
			case 30: m_r[RTREG] = m_fcr30; break;
			case 31: m_r[RTREG] = m_fcr31; break;
				break;

			default:
				logerror("cfc1 undefined fpu control register %d (%s)\n", FSREG, machine().describe_context());
				break;
			}
			break;
		case 0x04: // MTC1
			if (FSREG & 1)
				// load the high half of the floating point register
				m_f[FSREG >> 1] = (u64(m_r[RTREG]) << 32) | u32(m_f[FSREG >> 1]);
			else
				// load the low half of the floating point register
				m_f[FSREG >> 1] = (m_f[FSREG >> 1] & ~0xffffffffULL) | m_r[RTREG];
			break;
		case 0x06: // CTC1
			switch (RDREG)
			{
			case 0: // register is read-only
				break;

			case 30:
				m_fcr30 = m_r[RTREG];
				break;

			case 31:
				m_fcr31 = m_r[RTREG];

				// update rounding mode
				switch (m_fcr31 & FCR31_RM)
				{
				case 0: softfloat_roundingMode = softfloat_round_near_even; break;
				case 1: softfloat_roundingMode = softfloat_round_minMag; break;
				case 2: softfloat_roundingMode = softfloat_round_max; break;
				case 3: softfloat_roundingMode = softfloat_round_min; break;
				}

				// exception check
				{
					bool const exception = (m_fcr31 & FCR31_CE) || (((m_fcr31 & FCR31_CM) >> 5) & (m_fcr31 & FCR31_EM));
					execute_set_input(m_fpu_irq, exception ? ASSERT_LINE : CLEAR_LINE);
				}
				break;

			default:
				logerror("ctc1 undefined fpu control register %d (%s)\n", RDREG, machine().describe_context());
				break;
			}
			break;
		case 0x08: // BC
			switch ((op >> 16) & 0x1f)
			{
			case 0x00: // BC1F
				if (!(m_fcr31 & FCR31_C))
				{
					m_branch_state = BRANCH;
					m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
				}
				break;
			case 0x01: // BC1T
				if (m_fcr31 & FCR31_C)
				{
					m_branch_state = BRANCH;
					m_branch_target = m_pc + 4 + (s32(SIMMVAL) << 2);
				}
				break;

			default:
				// unimplemented operation
				m_fcr31 |= FCR31_CE;
				execute_set_input(m_fpu_irq, ASSERT_LINE);
				break;
			}
			break;
		case 0x10: // S
			switch (op & 0x3f)
			{
			case 0x00: // ADD.S
				set_cop1_reg(FDREG >> 1, f32_add(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }).v);
				break;
			case 0x01: // SUB.S
				set_cop1_reg(FDREG >> 1, f32_sub(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }).v);
				break;
			case 0x02: // MUL.S
				set_cop1_reg(FDREG >> 1, f32_mul(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }).v);
				break;
			case 0x03: // DIV.S
				set_cop1_reg(FDREG >> 1, f32_div(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }).v);
				break;
			case 0x05: // ABS.S
				if (f32_lt(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ 0 }))
					set_cop1_reg(FDREG >> 1, f32_mul(float32_t{ u32(m_f[FSREG >> 1]) }, i32_to_f32(-1)).v);
				else
					set_cop1_reg(FDREG >> 1, u32(m_f[FSREG >> 1]));
				break;
			case 0x06: // MOV.S
				if (FDREG & 1)
					if (FSREG & 1)
						// move high half to high half
						m_f[FDREG >> 1] = (m_f[FSREG >> 1] & ~0xffffffffULL) | u32(m_f[FDREG >> 1]);
					else
						// move low half to high half
						m_f[FDREG >> 1] = (m_f[FSREG >> 1] << 32) | u32(m_f[FDREG >> 1]);
				else
					if (FSREG & 1)
						// move high half to low half
						m_f[FDREG >> 1] = (m_f[FDREG >> 1] & ~0xffffffffULL) | (m_f[FSREG >> 1] >> 32);
					else
						// move low half to low half
						m_f[FDREG >> 1] = (m_f[FDREG >> 1] & ~0xffffffffULL) | u32(m_f[FSREG >> 1]);
				break;
			case 0x07: // NEG.S
				set_cop1_reg(FDREG >> 1, f32_mul(float32_t{ u32(m_f[FSREG >> 1]) }, i32_to_f32(-1)).v);
				break;

			case 0x21: // CVT.D.S
				set_cop1_reg(FDREG >> 1, f32_to_f64(float32_t{ u32(m_f[FSREG >> 1]) }).v);
				break;
			case 0x24: // CVT.W.S
				set_cop1_reg(FDREG >> 1, f32_to_i32(float32_t{ u32(m_f[FSREG >> 1]) }, softfloat_roundingMode, true));
				break;

			case 0x30: // C.F.S (false)
				m_fcr31 &= ~FCR31_C;
				break;
			case 0x31: // C.UN.S (unordered)
				f32_eq(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) });
				if (softfloat_exceptionFlags & softfloat_flag_invalid)
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x32: // C.EQ.S (equal)
				if (f32_eq(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x33: // C.UEQ.S (unordered equal)
				if (f32_eq(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x34: // C.OLT.S (less than)
				if (f32_lt(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x35: // C.ULT.S (unordered less than)
				if (f32_lt(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x36: // C.OLE.S (less than or equal)
				if (f32_le(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x37: // C.ULE.S (unordered less than or equal)
				if (f32_le(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;

			case 0x38: // C.SF.S (signalling false)
				f32_eq(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) });

				m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x39: // C.NGLE.S (not greater, less than or equal)
				f32_eq(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) });

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_C | FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x3a: // C.SEQ.S (signalling equal)
				if (f32_eq(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x3b: // C.NGL.S (not greater or less than)
				if (f32_eq(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x3c: // C.LT.S (less than)
				if (f32_lt(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x3d: // C.NGE.S (not greater or equal)
				if (f32_lt(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x3e: // C.LE.S (less than or equal)
				if (f32_le(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x3f: // C.NGT.S (not greater than)
				if (f32_le(float32_t{ u32(m_f[FSREG >> 1]) }, float32_t{ u32(m_f[FTREG >> 1]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;

			default: // unimplemented operation
				m_fcr31 |= FCR31_CE;
				execute_set_input(m_fpu_irq, ASSERT_LINE);
				break;
			}
			break;
		case 0x11: // D
			switch (op & 0x3f)
			{
			case 0x00: // ADD.D
				set_cop1_reg(FDREG >> 1, f64_add(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }).v);
				break;
			case 0x01: // SUB.D
				set_cop1_reg(FDREG >> 1, f64_sub(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }).v);
				break;
			case 0x02: // MUL.D
				set_cop1_reg(FDREG >> 1, f64_mul(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }).v);
				break;
			case 0x03: // DIV.D
				set_cop1_reg(FDREG >> 1, f64_div(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }).v);
				break;

			case 0x05: // ABS.D
				if (f64_lt(float64_t{ m_f[FSREG >> 1] }, float64_t{ 0 }))
					set_cop1_reg(FDREG >> 1, f64_mul(float64_t{ m_f[FSREG >> 1] }, i32_to_f64(-1)).v);
				else
					set_cop1_reg(FDREG >> 1, m_f[FSREG >> 1]);
				break;
			case 0x06: // MOV.D
				m_f[FDREG >> 1] = m_f[FSREG >> 1];
				break;
			case 0x07: // NEG.D
				set_cop1_reg(FDREG >> 1, f64_mul(float64_t{ m_f[FSREG >> 1] }, i32_to_f64(-1)).v);
				break;

			case 0x20: // CVT.S.D
				set_cop1_reg(FDREG >> 1, f64_to_f32(float64_t{ m_f[FSREG >> 1] }).v);
				break;
			case 0x24: // CVT.W.D
				set_cop1_reg(FDREG >> 1, f64_to_i32(float64_t{ m_f[FSREG >> 1] }, softfloat_roundingMode, true));
				break;

			case 0x30: // C.F.D (false)
				m_fcr31 &= ~FCR31_C;
				break;
			case 0x31: // C.UN.D (unordered)
				f64_eq(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] });
				if (softfloat_exceptionFlags & softfloat_flag_invalid)
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x32: // C.EQ.D (equal)
				if (f64_eq(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x33: // C.UEQ.D (unordered equal)
				if (f64_eq(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x34: // C.OLT.D (less than)
				if (f64_lt(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x35: // C.ULT.D (unordered less than)
				if (f64_lt(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x36: // C.OLE.D (less than or equal)
				if (f64_le(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x37: // C.ULE.D (unordered less than or equal)
				if (f64_le(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;
				break;

			case 0x38: // C.SF.D (signalling false)
				f64_eq(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] });

				m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x39: // C.NGLE.D (not greater, less than or equal)
				f64_eq(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] });

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_C | FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				else
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x3a: // C.SEQ.D (signalling equal)
				if (f64_eq(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x3b: // C.NGL.D (not greater or less than)
				if (f64_eq(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x3c: // C.LT.D (less than)
				if (f64_lt(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x3d: // C.NGE.D (not greater or equal)
				if (f64_lt(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x3e: // C.LE.D (less than or equal)
				if (f64_le(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;
			case 0x3f: // C.NGT.D (not greater than)
				if (f64_le(float64_t{ m_f[FSREG >> 1] }, float64_t{ m_f[FTREG >> 1] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
					m_fcr31 |= FCR31_C;
				else
					m_fcr31 &= ~FCR31_C;

				if (softfloat_exceptionFlags & softfloat_flag_invalid)
				{
					m_fcr31 |= FCR31_CV;
					execute_set_input(m_fpu_irq, ASSERT_LINE);
				}
				break;

			default: // unimplemented operation
				m_fcr31 |= FCR31_CE;
				execute_set_input(m_fpu_irq, ASSERT_LINE);
				break;
			}
			break;
		case 0x14: // W
			switch (op & 0x3f)
			{
			case 0x20: // CVT.S.W
				set_cop1_reg(FDREG >> 1, i32_to_f32(s32(m_f[FSREG >> 1])).v);
				break;
			case 0x21: // CVT.D.W
				set_cop1_reg(FDREG >> 1, i32_to_f64(s32(m_f[FSREG >> 1])).v);
				break;

			default: // unimplemented operation
				m_fcr31 |= FCR31_CE;
				execute_set_input(m_fpu_irq, ASSERT_LINE);
				break;
			}
			break;

		default: // unimplemented operation
			m_fcr31 |= FCR31_CE;
			execute_set_input(m_fpu_irq, ASSERT_LINE);
			break;
		}
		break;
	case 0x31: // LWC1
		load<u32>(SIMMVAL + m_r[RSREG],
			[this, op](u32 data)
		{
			if (FTREG & 1)
				// load the high half of the floating point register
				m_f[FTREG >> 1] = (u64(data) << 32) | u32(m_f[FTREG >> 1]);
			else
				// load the low half of the floating point register
				m_f[FTREG >> 1] = (m_f[FTREG >> 1] & ~0xffffffffULL) | data;
		});
		break;
	case 0x39: // SWC1
		if (FTREG & 1)
			// store the high half of the floating point register
			store<u32>(SIMMVAL + m_r[RSREG], m_f[FTREG >> 1] >> 32);
		else
			// store the low half of the floating point register
			store<u32>(SIMMVAL + m_r[RSREG], m_f[FTREG >> 1]);
		break;
	}
}

template <typename T> void mips1_device_base::set_cop1_reg(unsigned const reg, T const data)
{
	// translate softfloat exception flags to cause register
	if (softfloat_exceptionFlags)
	{
		if (softfloat_exceptionFlags & softfloat_flag_inexact)
			m_fcr31 |= FCR31_CI;
		if (softfloat_exceptionFlags & softfloat_flag_underflow)
			m_fcr31 |= FCR31_CU;
		if (softfloat_exceptionFlags & softfloat_flag_overflow)
			m_fcr31 |= FCR31_CO;
		if (softfloat_exceptionFlags & softfloat_flag_infinite)
			m_fcr31 |= FCR31_CZ;
		if (softfloat_exceptionFlags & softfloat_flag_invalid)
			m_fcr31 |= FCR31_CV;

		// set flags
		m_fcr31 |= ((m_fcr31 & FCR31_CM) >> 10);

		// update exception state
		bool const exception = (m_fcr31 & FCR31_CE) || ((m_fcr31 & FCR31_CM) >> 5) & (m_fcr31 & FCR31_EM);
		execute_set_input(m_fpu_irq, exception ? ASSERT_LINE : CLEAR_LINE);

		if (exception)
			return;
	}

	if (sizeof(T) == 4)
		m_f[reg] = (m_f[reg] & ~0xffffffffULL) | data;
	else
		m_f[reg] = data;
}

bool mips1_device_base::memory_translate(int spacenum, int intention, offs_t &address)
{
	// check for kernel memory address
	if (BIT(address, 31))
	{
		// check debug or kernel mode
		if ((intention & TRANSLATE_DEBUG_MASK) || !(SR & SR_KUc))
		{
			switch (address & 0xe0000000)
			{
			case 0x80000000: // kseg0: unmapped, cached, privileged
			case 0xa0000000: // kseg1: unmapped, uncached, privileged
				address &= ~0xe0000000;
				return true;

			case 0xc0000000: // kseg2: mapped, cached, privileged
			case 0xe0000000:
				break;
			}
		}
		else if (SR & SR_KUc)
		{
			address_error(intention, address);

			return false;
		}
	}

	// key is a combination of VPN and ASID
	u32 const key = (address & EH_VPN) | (m_cop0[COP0_EntryHi] & EH_ASID);

	unsigned *mru = m_tlb_mru[intention & TRANSLATE_TYPE_MASK];

	bool refill = !BIT(address, 31);
	bool modify = false;

	for (unsigned i = 0; i < std::size(m_tlb); i++)
	{
		unsigned const index = mru[i];
		u32 const *const entry = m_tlb[index];

		// test vpn and optionally asid
		u32 const mask = (entry[1] & EL_G) ? EH_VPN : EH_VPN | EH_ASID;
		if ((entry[0] & mask) != (key & mask))
			continue;

		// test valid
		if (!(entry[1] & EL_V))
		{
			refill = false;
			break;
		}

		// test dirty
		if ((intention & TRANSLATE_WRITE) && !(entry[1] & EL_D))
		{
			refill = false;
			modify = true;
			break;
		}

		// translate the address
		address &= ~EH_VPN;
		address |= (entry[1] & EL_PFN);

		// promote the entry in the mru index
		if (i > 0)
			std::swap(mru[i - 1], mru[i]);

		return true;
	}

	if (!machine().side_effects_disabled() && !(intention & TRANSLATE_DEBUG_MASK))
	{
		if (VERBOSE & LOG_TLB)
		{
			if (modify)
				LOGMASKED(LOG_TLB, "asid %2d tlb modify address 0x%08x (%s)\n",
					(m_cop0[COP0_EntryHi] & EH_ASID) >> 6, address, machine().describe_context());
			else
				LOGMASKED(LOG_TLB, "asid %2d tlb miss %c address 0x%08x (%s)\n",
					(m_cop0[COP0_EntryHi] & EH_ASID) >> 6, (intention & TRANSLATE_WRITE) ? 'w' : 'r', address, machine().describe_context());
		}

		// load tlb exception registers
		m_cop0[COP0_BadVAddr] = address;
		m_cop0[COP0_EntryHi] = key;
		m_cop0[COP0_Context] = (m_cop0[COP0_Context] & PTE_BASE) | ((address >> 10) & BAD_VPN);

		generate_exception(modify ? EXCEPTION_TLBMOD : (intention & TRANSLATE_WRITE) ? EXCEPTION_TLBSTORE : EXCEPTION_TLBLOAD, refill);
	}

	return false;
}
