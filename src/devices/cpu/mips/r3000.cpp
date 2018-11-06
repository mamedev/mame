// license:BSD-3-Clause
// copyright-holders:Aaron Giles

/*
 * MIPS-I emulation, including R2000[A], R3000[A] and IDT R30xx devices. The
 * IDT devices come in two variations: those with an "E" suffix include a TLB,
 * while those without have hard-wired address translation.
 *
 * TODO
 *   - FPU support
 *   - fix and enable overflow exceptions
 *   - further cleanup on coprocessors
 *   - R3041 features
 *   - cache emulation
 *
 */
#include "emu.h"
#include "r3000.h"
#include "r3kdasm.h"
#include "debugger.h"

#define LOG_GENERAL (1U << 0)
#define LOG_TLB     (1U << 1)

//#define VERBOSE     (LOG_GENERAL|LOG_TLB)
#include "logmacro.h"

#define ENABLE_OVERFLOWS    (0)
#define ENABLE_IOP_KPUTS    (0)

#define RSREG           ((op >> 21) & 31)
#define RTREG           ((op >> 16) & 31)
#define RDREG           ((op >> 11) & 31)
#define SHIFT           ((op >> 6) & 31)

#define RSVAL           m_r[RSREG]
#define RTVAL           m_r[RTREG]
#define RDVAL           m_r[RDREG]

#define SIMMVAL         s16(op)
#define UIMMVAL         u16(op)
#define LIMMVAL         (op & 0x03ffffff)

#define ADDPC(x)        do { m_nextpc = m_pc + ((x) << 2); } while (0)
#define ADDPCL(x,l)     do { m_nextpc = m_pc + ((x) << 2); m_r[l] = m_pc + 4; } while (0)
#define ABSPC(x)        do { m_nextpc = (m_pc & 0xf0000000) | ((x) << 2); } while (0)
#define ABSPCL(x,l)     do { m_nextpc = (m_pc & 0xf0000000) | ((x) << 2); m_r[l] = m_pc + 4; } while (0)
#define SETPC(x)        do { m_nextpc = (x); } while (0)
#define SETPCL(x,l)     do { m_nextpc = (x); m_r[l] = m_pc + 4; } while (0)

#define SR              m_cpr[0][COP0_Status]
#define CAUSE           m_cpr[0][COP0_Cause]

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

mips1core_device_base::mips1core_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 cpurev, size_t icache_size, size_t dcache_size)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config_be("program", ENDIANNESS_BIG, 32, 32)
	, m_program_config_le("program", ENDIANNESS_LITTLE, 32, 32)
	, m_icache_config("icache", ENDIANNESS_BIG, 32, 32)
	, m_dcache_config("dcache", ENDIANNESS_BIG, 32, 32)
	, m_cpurev(cpurev)
	, m_hasfpu(false)
	, m_fpurev(0)
	, m_endianness(ENDIANNESS_BIG)
	, m_pc(0)
	, m_nextpc(0)
	, m_ppc(0)
	, m_icount(0)
	, m_icache_size(icache_size)
	, m_dcache_size(dcache_size)
	, m_in_brcond{ *this, *this, *this, *this }
{
}

mips1_device_base::mips1_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 cpurev, size_t icache_size, size_t dcache_size)
	: mips1core_device_base(mconfig, type, tag, owner, clock, cpurev, icache_size, dcache_size)
{
}

r2000_device::r2000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1_device_base(mconfig, R2000, tag, owner, clock, 0x0100, icache_size, dcache_size)
{
}

r2000a_device::r2000a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1_device_base(mconfig, R2000A, tag, owner, clock, 0x0216, icache_size, dcache_size)
{
}

r3000_device::r3000_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1_device_base(mconfig, R3000, tag, owner, clock, 0x0220, icache_size, dcache_size)
{
}

r3000a_device::r3000a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1_device_base(mconfig, R3000A, tag, owner, clock, 0x0230, icache_size, dcache_size)
{
}

r3041_device::r3041_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mips1core_device_base(mconfig, R3041, tag, owner, clock, 0x0700, 2048, 512)
{
}

r3051_device::r3051_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mips1core_device_base(mconfig, R3051, tag, owner, clock, 0x0200, 4096, 2048)
{
}

r3052_device::r3052_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mips1core_device_base(mconfig, R3052, tag, owner, clock, 0x0200, 8192, 2048)
{
}

r3052e_device::r3052e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mips1_device_base(mconfig, R3052E, tag, owner, clock, 0x0200, 8192, 2048)
{
}

r3071_device::r3071_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1core_device_base(mconfig, R3071, tag, owner, clock, 0x0200, icache_size, dcache_size)
{
}

r3081_device::r3081_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, size_t icache_size, size_t dcache_size)
	: mips1core_device_base(mconfig, R3081, tag, owner, clock, 0x0200, icache_size, dcache_size)
{
	set_fpurev(0x0300);
}

iop_device::iop_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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

void mips1core_device_base::icache_map(address_map &map)
{
	map(0, m_icache_size - 1).ram().mirror(~(m_icache_size - 1));
}

void mips1core_device_base::dcache_map(address_map &map)
{
	map(0, m_dcache_size - 1).ram().mirror(~(m_dcache_size - 1));
}

void mips1core_device_base::device_start()
{
	// set our instruction counter
	set_icountptr(m_icount);

	// resolve conditional branch input handlers
	for (devcb_read_line &cb : m_in_brcond)
		cb.resolve_safe(0);

	// register our state for the debugger
	state_add(STATE_GENPC,      "GENPC",     m_pc).noshow();
	state_add(STATE_GENPCBASE,  "CURPC",     m_ppc).noshow();
	state_add(STATE_GENSP,      "GENSP",     m_r[31]).noshow();
	state_add(STATE_GENFLAGS,   "GENFLAGS",  m_cpr[0][COP0_Status]).noshow();

	state_add(MIPS1_PC,         "PC",        m_pc);
	state_add(MIPS1_COP0_SR,    "SR",        m_cpr[0][COP0_Status]);

	for (int i = 0; i < 32; i++)
		state_add(MIPS1_R0 + i, util::string_format("R%d", i).c_str(), m_r[i]);

	state_add(MIPS1_HI, "HI", m_hi);
	state_add(MIPS1_LO, "LO", m_lo);
	state_add(MIPS1_COP0_BADVADDR, "BadVAddr", m_cpr[0][COP0_BadVAddr]);
	state_add(MIPS1_COP0_CAUSE, "Cause", m_cpr[0][COP0_Cause]);
	state_add(MIPS1_COP0_EPC, "EPC", m_cpr[0][COP0_EPC]);

	// register our state for saving
	save_item(NAME(m_pc));
	save_item(NAME(m_nextpc));
	save_item(NAME(m_hi));
	save_item(NAME(m_lo));
	save_item(NAME(m_r));
	save_item(NAME(m_cpr));
	save_item(NAME(m_ccr));
	save_item(NAME(m_ppc));

	// initialise cpu and fpu id registers
	m_cpr[0][COP0_PRId] = m_cpurev;
	m_ccr[1][0] = m_fpurev;
}

void mips1_device_base::device_start()
{
	mips1core_device_base::device_start();

	// cop0 tlb registers
	state_add(MIPS1_COP0_INDEX, "Index", m_cpr[0][COP0_Index]);
	state_add(MIPS1_COP0_RANDOM, "Random", m_cpr[0][COP0_Random]);
	state_add(MIPS1_COP0_ENTRYLO, "EntryLo", m_cpr[0][COP0_EntryLo]);
	state_add(MIPS1_COP0_ENTRYHI, "EntryHi", m_cpr[0][COP0_EntryHi]);
	state_add(MIPS1_COP0_CONTEXT, "Context", m_cpr[0][COP0_Context]);

	save_item(NAME(m_reset_time));
	save_item(NAME(m_tlb));
}

void r3041_device::device_start()
{
	mips1core_device_base::device_start();

	// cop0 r3041 registers
	state_add(MIPS1_COP0_BUSCTRL, "BusCtrl", m_cpr[0][COP0_BusCtrl]);
	state_add(MIPS1_COP0_CONFIG, "Config", m_cpr[0][COP0_Config]);
	state_add(MIPS1_COP0_COUNT, "Count", m_cpr[0][COP0_Count]);
	state_add(MIPS1_COP0_PORTSIZE, "PortSize", m_cpr[0][COP0_PortSize]);
	state_add(MIPS1_COP0_COMPARE, "Compare", m_cpr[0][COP0_Compare]);
}

void mips1core_device_base::device_reset()
{
	// initialize the state
	m_pc = 0xbfc00000;
	m_nextpc = ~0;

	// non-tlb devices have tlb shut down
	m_cpr[0][COP0_Status] = SR_BEV | SR_TS;

	m_data_spacenum = 0;
}

void mips1_device_base::device_reset()
{
	mips1core_device_base::device_reset();

	// tlb is not shut down
	m_cpr[0][COP0_Status] &= ~SR_TS;

	m_reset_time = total_cycles();
}

device_memory_interface::space_config_vector mips1core_device_base::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, (m_endianness == ENDIANNESS_BIG) ? &m_program_config_be : &m_program_config_le),
		std::make_pair(1, &m_icache_config),
		std::make_pair(2, &m_dcache_config)
	};
}

std::unique_ptr<util::disasm_interface> mips1core_device_base::create_disassembler()
{
	return std::make_unique<mips1_disassembler>();
}

void mips1core_device_base::generate_exception(int exception)
{
	// set the exception PC
	m_cpr[0][COP0_EPC] = (exception == EXCEPTION_INTERRUPT) ? m_pc : m_ppc;

	// put the cause in the low 8 bits and clear the branch delay flag
	CAUSE = (CAUSE & ~0x800000ff) | (exception << 2);

	// if we were in a branch delay slot, adjust
	if (m_nextpc != ~0)
	{
		m_nextpc = ~0;
		m_cpr[0][COP0_EPC] -= 4;
		CAUSE |= 0x80000000;
	}

	// shift the exception bits
	SR = (SR & 0xffffffc0) | ((SR << 2) & 0x3c);

	// only tlb misses on kuseg have dedicated vectors
	if ((exception == EXCEPTION_TLBLOAD || exception == EXCEPTION_TLBSTORE) && !BIT(m_cpr[0][COP0_BadVAddr], 31))
		m_pc = (SR & SR_BEV) ? 0xbfc00100 : 0x80000000;
	else
		m_pc = (SR & SR_BEV) ? 0xbfc00180 : 0x80000080;

	debugger_exception_hook(exception);
}

void mips1core_device_base::check_irqs()
{
	if ((CAUSE & SR & 0xff00) && (SR & SR_IEc))
		generate_exception(EXCEPTION_INTERRUPT);
}

void mips1core_device_base::set_irq_line(int irqline, int state)
{
	if (state != CLEAR_LINE)
		CAUSE |= 0x400 << irqline;
	else
		CAUSE &= ~(0x400 << irqline);

	check_irqs();
}

u32 mips1core_device_base::get_cop0_reg(int const index)
{
	return m_cpr[0][index];
}

u32 mips1_device_base::get_cop0_reg(int const index)
{
	// assume 64-entry tlb with 8 wired entries
	if (index == COP0_Random)
		m_cpr[0][index] = (63 - ((total_cycles() - m_reset_time) % 56)) << 8;

	return m_cpr[0][index];
}

void mips1core_device_base::set_cop0_reg(int const index, u32 const data)
{
	if (index == COP0_Cause)
	{
		CAUSE = (CAUSE & 0xfc00) | (data & ~0xfc00);

		// update interrupts -- software ints can occur this way
		check_irqs();
	}
	else if (index == COP0_Status)
	{
		m_cpr[0][index] = data;

		// handle cache isolation and swap
		m_data_spacenum = (data & SR_IsC) ? ((data & SR_SwC) ? 1 : 2) : 0;

		// update interrupts
		check_irqs();
	}
	else if (index != COP0_PRId)
		m_cpr[0][index] = data;
}

void mips1core_device_base::handle_cop0(u32 const op)
{
	switch (RSREG)
	{
		case 0x00:  /* MFCz */      if (RTREG) RTVAL = get_cop0_reg(RDREG);          break;
		case 0x02:  /* CFCz */      if (RTREG) RTVAL = get_cop_creg<0>(RDREG);       break;
		case 0x04:  /* MTCz */      set_cop0_reg(RDREG, RTVAL);                      break;
		case 0x06:  /* CTCz */      set_cop_creg<0>(RDREG, RTVAL);                   break;
		case 0x08:  /* BC */
			switch (RTREG)
			{
				case 0x00:  /* BCzF */  if (!m_in_brcond[0]()) ADDPC(SIMMVAL);       break;
				case 0x01:  /* BCzT */  if (m_in_brcond[0]()) ADDPC(SIMMVAL);        break;
				case 0x02:  /* BCzFL */ generate_exception(EXCEPTION_INVALIDOP);     break;
				case 0x03:  /* BCzTL */ generate_exception(EXCEPTION_INVALIDOP);     break;
				default:    generate_exception(EXCEPTION_INVALIDOP);                 break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  /* COP */
			switch (op & 0x01ffffff)
			{
				case 0x01:  /* TLBR */                                               break;
				case 0x02:  /* TLBWI */                                              break;
				case 0x06:  /* TLBWR */                                              break;
				case 0x08:  /* TLBP */                                               break;
				case 0x10:  /* RFE */   SR = (SR & 0xfffffff0) | ((SR >> 2) & 0x0f); break;
				case 0x18:  /* ERET */  generate_exception(EXCEPTION_INVALIDOP);     break;
				default:    generate_exception(EXCEPTION_INVALIDOP);                 break;
			}
			break;
		default:    generate_exception(EXCEPTION_INVALIDOP);                         break;
	}
}

void mips1_device_base::handle_cop0(u32 const op)
{
	switch (op)
	{
	case 0x42000001: // TLBR - read tlb
		{
			u8 const index = (m_cpr[0][COP0_Index] >> 8) & 0x3f;

			m_cpr[0][COP0_EntryHi] = m_tlb[index][0];
			m_cpr[0][COP0_EntryLo] = m_tlb[index][1];
		}
		break;

	case 0x42000002: // TLBWI - write tlb (indexed)
		{
			u8 const index = (m_cpr[0][COP0_Index] >> 8) & 0x3f;

			m_tlb[index][0] = m_cpr[0][COP0_EntryHi];
			m_tlb[index][1] = m_cpr[0][COP0_EntryLo];

			LOGMASKED(LOG_TLB, "tlb index %d program 0x%08x physical 0x%08x\n",
				index, m_cpr[0][COP0_EntryHi] & ~0xfff, m_cpr[0][COP0_EntryLo] & ~0xfff);
		}
		break;

	case 0x42000006: // TLBWR - write tlb (random)
		{
			u8 const random = get_cop0_reg(COP0_Random) >> 8;

			m_tlb[random][0] = m_cpr[0][COP0_EntryHi];
			m_tlb[random][1] = m_cpr[0][COP0_EntryLo];

			LOGMASKED(LOG_TLB, "tlb random %d program 0x%08x physical 0x%08x\n",
				random, m_cpr[0][COP0_EntryHi] & ~0xfff, m_cpr[0][COP0_EntryLo] & ~0xfff);
		}
		break;

	case 0x42000008: // TLBP - probe tlb
		m_cpr[0][COP0_Index] = 0x80000000;
		for (u8 index = 0; index < 64; index++)
		{
			// test vpn and optionally asid
			u32 const mask = (m_tlb[index][1] & EL_G) ? EH_VPN : EH_VPN | EH_ASID;
			if ((m_tlb[index][0] & mask) == (m_cpr[0][COP0_EntryHi] & mask))
			{
				m_cpr[0][COP0_Index] = index << 8;
				break;
			}
		}
		break;

	default:
		mips1core_device_base::handle_cop0(op);
	}
}

void mips1core_device_base::set_cop1_creg(int idx, u32 val)
{
	// fpu revision register is read-only
	if (idx)
		m_ccr[1][idx] = val;
}

void mips1core_device_base::handle_cop1(u32 const op)
{
	if (!m_hasfpu)
		return;

	switch (RSREG)
	{
		case 0x00:  /* MFCz */      if (RTREG) RTVAL = get_cop_reg<1>(RDREG);        break;
		case 0x02:  /* CFCz */      if (RTREG) RTVAL = get_cop_creg<1>(RDREG);       break;
		case 0x04:  /* MTCz */      set_cop_reg<1>(RDREG, RTVAL);                    break;
		case 0x06:  /* CTCz */      set_cop1_creg(RDREG, RTVAL);                     break;
		case 0x08:  /* BC */
			switch (RTREG)
			{
				case 0x00:  /* BCzF */  if (!m_in_brcond[1]()) ADDPC(SIMMVAL);       break;
				case 0x01:  /* BCzT */  if (m_in_brcond[1]()) ADDPC(SIMMVAL);        break;
				case 0x02:  /* BCzFL */ generate_exception(EXCEPTION_INVALIDOP);     break;
				case 0x03:  /* BCzTL */ generate_exception(EXCEPTION_INVALIDOP);     break;
				default:    generate_exception(EXCEPTION_INVALIDOP);                 break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  /* COP */       generate_exception(EXCEPTION_INVALIDOP);         break;
		default:    generate_exception(EXCEPTION_INVALIDOP);                         break;
	}
}

template <unsigned Coprocessor> void mips1core_device_base::handle_cop(u32 const op)
{
	switch (RSREG)
	{
		case 0x00:  /* MFCz */      if (RTREG) RTVAL = get_cop_reg<Coprocessor>(RDREG);      break;
		case 0x02:  /* CFCz */      if (RTREG) RTVAL = get_cop_creg<Coprocessor>(RDREG);     break;
		case 0x04:  /* MTCz */      set_cop_reg<Coprocessor>(RDREG, RTVAL);                  break;
		case 0x06:  /* CTCz */      set_cop_creg<Coprocessor>(RDREG, RTVAL);                 break;
		case 0x08:  /* BC */
			switch (RTREG)
			{
				case 0x00:  /* BCzF */  if (!m_in_brcond[Coprocessor]()) ADDPC(SIMMVAL);     break;
				case 0x01:  /* BCzT */  if (m_in_brcond[Coprocessor]()) ADDPC(SIMMVAL);      break;
				case 0x02:  /* BCzFL */ generate_exception(EXCEPTION_INVALIDOP);     break;
				case 0x03:  /* BCzTL */ generate_exception(EXCEPTION_INVALIDOP);     break;
				default:    generate_exception(EXCEPTION_INVALIDOP);                 break;
			}
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
		case 0x1c:
		case 0x1d:
		case 0x1e:
		case 0x1f:  /* COP */       generate_exception(EXCEPTION_INVALIDOP);         break;
		default:    generate_exception(EXCEPTION_INVALIDOP);                         break;
	}
}

void mips1core_device_base::execute_run()
{
	// check for IRQs
	check_irqs();

	// core execution loop
	do
	{
		// debugging
		m_ppc = m_pc;
		debugger_instruction_hook(m_pc);

#if ENABLE_IOP_KPUTS
		if ((m_pc & 0x1fffffff) == 0x00012C48 || (m_pc & 0x1fffffff) == 0x0001420C || (m_pc & 0x1fffffff) == 0x0001430C)
		{
			u32 ptr = m_r[5];
			u32 length = m_r[6];
			if (length >= 4096)
				length = 4095;
			while (length)
			{
				load<u8>(ptr, [this](char c) { printf("%c", c); });
				ptr++;
				length--;
			}
			fflush(stdout);
		}
#endif

		// fetch and execute instruction
		fetch(m_pc, [this](u32 const op)
		{
			{
				// adjust for next PC
				if (m_nextpc != ~0)
				{
					m_pc = m_nextpc;
					m_nextpc = ~0;
				}
				else
					m_pc += 4;

				// parse the instruction
				switch (op >> 26)
				{
				case 0x00:  /* SPECIAL */
					switch (op & 63)
					{
					case 0x00:  /* SLL */       if (RDREG) RDVAL = RTVAL << SHIFT;                 break;
					case 0x02:  /* SRL */       if (RDREG) RDVAL = RTVAL >> SHIFT;                 break;
					case 0x03:  /* SRA */       if (RDREG) RDVAL = s32(RTVAL) >> SHIFT;            break;
					case 0x04:  /* SLLV */      if (RDREG) RDVAL = RTVAL << (RSVAL & 31);          break;
					case 0x06:  /* SRLV */      if (RDREG) RDVAL = RTVAL >> (RSVAL & 31);          break;
					case 0x07:  /* SRAV */      if (RDREG) RDVAL = s32(RTVAL) >> (RSVAL & 31);     break;
					case 0x08:  /* JR */        SETPC(RSVAL);                                      break;
					case 0x09:  /* JALR */      SETPCL(RSVAL, RDREG);                              break;
					case 0x0c:  /* SYSCALL */   generate_exception(EXCEPTION_SYSCALL);             break;
					case 0x0d:  /* BREAK */     generate_exception(EXCEPTION_BREAK);               break;
					case 0x0f:  /* SYNC */      generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x10:  /* MFHI */      if (RDREG) RDVAL = m_hi;                           break;
					case 0x11:  /* MTHI */      m_hi = RSVAL;                                      break;
					case 0x12:  /* MFLO */      if (RDREG) RDVAL = m_lo;                           break;
					case 0x13:  /* MTLO */      m_lo = RSVAL;                                      break;
					case 0x18:  /* MULT */
					{
						u64 product = mul_32x32(RSVAL, RTVAL);

						m_lo = product;
						m_hi = product >> 32;
						m_icount -= 11;
					}
					break;
					case 0x19:  /* MULTU */
					{
						u64 product = mulu_32x32(RSVAL, RTVAL);

						m_lo = product;
						m_hi = product >> 32;
						m_icount -= 11;
					}
					break;
					case 0x1a:  /* DIV */
						if (RTVAL)
						{
							m_lo = s32(RSVAL) / s32(RTVAL);
							m_hi = s32(RSVAL) % s32(RTVAL);
						}
						m_icount -= 34;
						break;
					case 0x1b:  /* DIVU */
						if (RTVAL)
						{
							m_lo = RSVAL / RTVAL;
							m_hi = RSVAL % RTVAL;
						}
						m_icount -= 34;
						break;
					case 0x20:  /* ADD */
						if (ENABLE_OVERFLOWS && RSVAL > ~RTVAL) generate_exception(EXCEPTION_OVERFLOW);
						else RDVAL = RSVAL + RTVAL;
						break;
					case 0x21:  /* ADDU */      if (RDREG) RDVAL = RSVAL + RTVAL;                  break;
					case 0x22:  /* SUB */
						if (ENABLE_OVERFLOWS && RSVAL < RTVAL) generate_exception(EXCEPTION_OVERFLOW);
						else RDVAL = RSVAL - RTVAL;
						break;
					case 0x23:  /* SUBU */      if (RDREG) RDVAL = RSVAL - RTVAL;                  break;
					case 0x24:  /* AND */       if (RDREG) RDVAL = RSVAL & RTVAL;                  break;
					case 0x25:  /* OR */        if (RDREG) RDVAL = RSVAL | RTVAL;                  break;
					case 0x26:  /* XOR */       if (RDREG) RDVAL = RSVAL ^ RTVAL;                  break;
					case 0x27:  /* NOR */       if (RDREG) RDVAL = ~(RSVAL | RTVAL);               break;
					case 0x2a:  /* SLT */       if (RDREG) RDVAL = s32(RSVAL) < s32(RTVAL);        break;
					case 0x2b:  /* SLTU */      if (RDREG) RDVAL = u32(RSVAL) < u32(RTVAL);        break;
					case 0x30:  /* TEQ */       generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x31:  /* TGEU */      generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x32:  /* TLT */       generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x33:  /* TLTU */      generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x34:  /* TGE */       generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x36:  /* TNE */       generate_exception(EXCEPTION_INVALIDOP);           break;
					default:    /* ??? */       generate_exception(EXCEPTION_INVALIDOP);           break;
					}
					break;

				case 0x01:  /* REGIMM */
					switch (RTREG)
					{
					case 0x00:  /* BLTZ */      if (s32(RSVAL) < 0) ADDPC(SIMMVAL);                break;
					case 0x01:  /* BGEZ */      if (s32(RSVAL) >= 0) ADDPC(SIMMVAL);               break;
					case 0x02:  /* BLTZL */     generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x03:  /* BGEZL */     generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x08:  /* TGEI */      generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x09:  /* TGEIU */     generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x0a:  /* TLTI */      generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x0b:  /* TLTIU */     generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x0c:  /* TEQI */      generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x0e:  /* TNEI */      generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x10:  /* BLTZAL */    if (s32(RSVAL) < 0) ADDPCL(SIMMVAL, 31);           break;
					case 0x11:  /* BGEZAL */    if (s32(RSVAL) >= 0) ADDPCL(SIMMVAL, 31);          break;
					case 0x12:  /* BLTZALL */   generate_exception(EXCEPTION_INVALIDOP);           break;
					case 0x13:  /* BGEZALL */   generate_exception(EXCEPTION_INVALIDOP);           break;
					default:    /* ??? */       generate_exception(EXCEPTION_INVALIDOP);           break;
					}
					break;

				case 0x02:  /* J */         ABSPC(LIMMVAL);                                        break;
				case 0x03:  /* JAL */       ABSPCL(LIMMVAL, 31);                                   break;
				case 0x04:  /* BEQ */       if (RSVAL == RTVAL) ADDPC(SIMMVAL);                    break;
				case 0x05:  /* BNE */       if (RSVAL != RTVAL) ADDPC(SIMMVAL);                    break;
				case 0x06:  /* BLEZ */      if (s32(RSVAL) <= 0) ADDPC(SIMMVAL);                   break;
				case 0x07:  /* BGTZ */      if (s32(RSVAL) > 0) ADDPC(SIMMVAL);                    break;
				case 0x08:  /* ADDI */
					if (ENABLE_OVERFLOWS && RSVAL > ~SIMMVAL) generate_exception(EXCEPTION_OVERFLOW);
					else if (RTREG) RTVAL = RSVAL + SIMMVAL;
					break;
				case 0x09:  /* ADDIU */     if (RTREG) RTVAL = RSVAL + SIMMVAL;                    break;
				case 0x0a:  /* SLTI */      if (RTREG) RTVAL = s32(RSVAL) < s32(SIMMVAL);          break;
				case 0x0b:  /* SLTIU */     if (RTREG) RTVAL = u32(RSVAL) < u32(SIMMVAL);          break;
				case 0x0c:  /* ANDI */      if (RTREG) RTVAL = RSVAL & UIMMVAL;                    break;
				case 0x0d:  /* ORI */       if (RTREG) RTVAL = RSVAL | UIMMVAL;                    break;
				case 0x0e:  /* XORI */      if (RTREG) RTVAL = RSVAL ^ UIMMVAL;                    break;
				case 0x0f:  /* LUI */       if (RTREG) RTVAL = UIMMVAL << 16;                      break;
				case 0x10:  /* COP0 */
					if (!(SR & SR_KUc) || (SR & SR_COP0))
						handle_cop0(op);
					else
						generate_exception(EXCEPTION_BADCOP);
					break;
				case 0x11: // COP1
					if (SR & SR_COP1)
						handle_cop1(op);
					else
						generate_exception(EXCEPTION_BADCOP);
					break;
				case 0x12: // COP2
					if (SR & SR_COP2)
						handle_cop<2>(op);
					else
						generate_exception(EXCEPTION_BADCOP);
					break;
				case 0x13: // COP3
					if (SR & SR_COP3)
						handle_cop<3>(op);
					else
						generate_exception(EXCEPTION_BADCOP);
					break;
				case 0x14:  /* BEQL */      generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x15:  /* BNEL */      generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x16:  /* BLEZL */     generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x17:  /* BGTZL */     generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x20:  /* LB */        load<u8>(SIMMVAL + RSVAL, [this, op](s8 temp) { if (RTREG) RTVAL = temp; });         break;
				case 0x21:  /* LH */        load<u16>(SIMMVAL + RSVAL, [this, op](s16 temp) { if (RTREG) RTVAL = temp; });       break;
				case 0x22:  /* LWL */       lwl(op);                                                                             break;
				case 0x23:  /* LW */        load<u32>(SIMMVAL + RSVAL, [this, op](u32 temp) { if (RTREG) RTVAL = temp; });       break;
				case 0x24:  /* LBU */       load<u8>(SIMMVAL + RSVAL, [this, op](u8 temp) { if (RTREG) RTVAL = temp; });         break;
				case 0x25:  /* LHU */       load<u16>(SIMMVAL + RSVAL, [this, op](u16 temp) { if (RTREG) RTVAL = temp; });       break;
				case 0x26:  /* LWR */       lwr(op);                                               break;
				case 0x28:  /* SB */        store<u8>(SIMMVAL + RSVAL, RTVAL);                     break;
				case 0x29:  /* SH */        store<u16>(SIMMVAL + RSVAL, RTVAL);                    break;
				case 0x2a:  /* SWL */       swl(op);                                               break;
				case 0x2b:  /* SW */        store<u32>(SIMMVAL + RSVAL, RTVAL);                    break;
				case 0x2e:  /* SWR */       swr(op);                                               break;
				case 0x2f:  /* CACHE */     generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x30:  /* LL */        generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x31:  /* LWC1 */      load<u32>(SIMMVAL + RSVAL, [this, op](u32 temp) { set_cop_reg<1>(RTREG, temp); });   break;
				case 0x32:  /* LWC2 */      load<u32>(SIMMVAL + RSVAL, [this, op](u32 temp) { set_cop_reg<2>(RTREG, temp); });   break;
				case 0x33:  /* LWC3 */      load<u32>(SIMMVAL + RSVAL, [this, op](u32 temp) { set_cop_reg<3>(RTREG, temp); });   break;
				case 0x34:  /* LDC0 */      generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x35:  /* LDC1 */      generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x36:  /* LDC2 */      generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x37:  /* LDC3 */      generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x38:  /* SC */        generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x39:  /* LWC1 */      store<u32>(SIMMVAL + RSVAL, get_cop_reg<1>(RTREG));    break;
				case 0x3a:  /* LWC2 */      store<u32>(SIMMVAL + RSVAL, get_cop_reg<2>(RTREG));    break;
				case 0x3b:  /* LWC3 */      store<u32>(SIMMVAL + RSVAL, get_cop_reg<3>(RTREG));    break;
				case 0x3c:  /* SDC0 */      generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x3d:  /* SDC1 */      generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x3e:  /* SDC2 */      generate_exception(EXCEPTION_INVALIDOP);               break;
				case 0x3f:  /* SDC3 */      generate_exception(EXCEPTION_INVALIDOP);               break;
				default:    /* ??? */       generate_exception(EXCEPTION_INVALIDOP);               break;
				}
			}
		});
		m_icount--;

	} while (m_icount > 0 || m_nextpc != ~0);
}

void mips1core_device_base::lwl(u32 const op)
{
	offs_t const offset = SIMMVAL + RSVAL;
	load<u32>(offset & ~3, [this, op, offset](u32 temp)
	{
		if (RTREG)
		{
			unsigned const shift = ((offset & 3) ^ ENDIAN_VALUE_LE_BE(m_endianness, 3, 0)) << 3;

			RTVAL = (RTVAL & ~u32(0xffffffffU << shift)) | (temp << shift);
		}
	});
}

void mips1core_device_base::lwr(u32 const op)
{
	offs_t const offset = SIMMVAL + RSVAL;
	load<u32>(offset & ~3, [this, op, offset](u32 temp)
	{
		if (RTREG)
		{
			unsigned const shift = ((offset & 0x3) ^ ENDIAN_VALUE_LE_BE(m_endianness, 0, 3)) << 3;

			RTVAL = (RTVAL & ~u32(0xffffffffU >> shift)) | (temp >> shift);
		}
	});
}

void mips1core_device_base::swl(u32 const op)
{
	offs_t const offset = SIMMVAL + RSVAL;
	unsigned const shift = ((offset & 3) ^ ENDIAN_VALUE_LE_BE(m_endianness, 3, 0)) << 3;

	// only load if necessary
	if (shift)
	{
		load<u32>(offset & ~3, [this, op, offset, shift](u32 temp)
		{
			store<u32>(offset & ~3, (temp & ~u32(0xffffffffU >> shift)) | (RTVAL >> shift));
		});
	}
	else
		store<u32>(offset & ~3, RTVAL);
}


void mips1core_device_base::swr(u32 const op)
{
	offs_t const offset = SIMMVAL + RSVAL;
	unsigned const shift = ((offset & 3) ^ ENDIAN_VALUE_LE_BE(m_endianness, 0, 3)) << 3;

	// only load if necessary
	if (shift)
	{
		load<u32>(offset & ~3, [this, op, offset, shift](u32 temp)
		{
			store<u32>(offset & ~3, (temp & ~u32(0xffffffffU << shift)) | (RTVAL << shift));
		});
	}
	else
		store<u32>(offset & ~3, RTVAL);
}

template <typename T, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(T)>>::value, void> mips1core_device_base::load(u32 program_address, U &&apply)
{
	offs_t translated_address = program_address;

	if (memory_translate(m_data_spacenum, TRANSLATE_READ, translated_address))
	{
		switch (sizeof(T))
		{
		case 1: apply(T(space(m_data_spacenum).read_byte(translated_address))); break;
		case 2: apply(T(space(m_data_spacenum).read_word(translated_address))); break;
		case 4: apply(T(space(m_data_spacenum).read_dword(translated_address))); break;
		}
	}
}

template <typename T, typename U> std::enable_if_t<std::is_convertible<U, T>::value, void> mips1core_device_base::store(u32 program_address, U data)
{
	offs_t translated_address = program_address;

	if (memory_translate(m_data_spacenum, TRANSLATE_WRITE, translated_address))
	{
		switch (sizeof(T))
		{
		case 1: space(m_data_spacenum).write_byte(translated_address, T(data)); break;
		case 2: space(m_data_spacenum).write_word(translated_address, T(data)); break;
		case 4: space(m_data_spacenum).write_dword(translated_address, T(data)); break;
		}
	}
}

bool mips1core_device_base::fetch(u32 program_address, std::function<void(u32)> &&apply)
{
	offs_t translated_address = program_address;

	if (memory_translate(0, TRANSLATE_FETCH, translated_address))
	{
		apply(space(0).read_dword(translated_address));

		return true;
	}
	else
		return false;
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
			// exception
			m_cpr[0][COP0_BadVAddr] = address;

			generate_exception((intention & TRANSLATE_WRITE) ? EXCEPTION_ADDRSTORE : EXCEPTION_ADDRLOAD);
			return false;
		}
	}
	else
		// kuseg physical addresses have a 1GB offset
		address += 0x40000000;

	return true;
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
			// exception
			m_cpr[0][COP0_BadVAddr] = address;

			generate_exception((intention & TRANSLATE_WRITE) ? EXCEPTION_ADDRSTORE : EXCEPTION_ADDRLOAD);
			return false;
		}
	}

	// key is a combination of VPN and ASID
	u32 const key = (address & EH_VPN) | (m_cpr[0][COP0_EntryHi] & EH_ASID);
	bool dirty = false;

	for (u32 const *entry : m_tlb)
	{
		// test vpn and optionally asid
		u32 const mask = (entry[1] & EL_G) ? EH_VPN : EH_VPN | EH_ASID;
		if ((entry[0] & mask) != (key & mask))
			continue;

		// test valid
		if (!(entry[1] & EL_V))
			break;

		// test dirty
		if ((intention & TRANSLATE_WRITE) && !(entry[1] & EL_D))
		{
			dirty = true;
			break;
		}

		// translate the address
		address &= ~EH_VPN;
		address |= (entry[1] & EL_PFN);
		return true;
	}

	if (!(intention & TRANSLATE_DEBUG_MASK))
	{
		if ((VERBOSE & LOG_TLB) && !dirty)
			LOGMASKED(LOG_TLB, "tlb miss address 0x%08x\n", address);

		// exception
		m_cpr[0][COP0_BadVAddr] = address;
		m_cpr[0][COP0_EntryHi] = key;
		m_cpr[0][COP0_Context] &= ~0x001fffff;
		m_cpr[0][COP0_Context] |= (address >> 11) & ~0x3;

		generate_exception(dirty ? EXCEPTION_TLBMOD : (intention & TRANSLATE_WRITE) ? EXCEPTION_TLBSTORE : EXCEPTION_TLBLOAD);
	}

	return false;
}
