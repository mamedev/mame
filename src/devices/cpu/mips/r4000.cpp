// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * This is a stripped-down MIPS-III CPU derived from the main mips3 code. Its
 * primary purpose is to act as a test-bed to aid in debugging MIPS-based
 * systems, after which the changes/improvements from here are expected to
 * be back-ported and incorporated into the original mips3 device.
 *
 * Because of this specific approach, no attempt is made to support many of the
 * current features of the mips3 device at this time. Key differences bewteen
 * this implementation and mips3 include:
 *
 *   - only supports MIPS R4000/R4400 and QED R4600
 *   - no dynamic recompilation
 *   - reworked address translation logic, including 64-bit modes
 *   - reworked softfloat3-based floating point
 *   - experimental primary instruction cache
 *   - configurable endianness
 *   - it's very very very slow
 *
 * TODO
 *   - enforce mode checks for cp1
 *   - cache instructions
 *   - check/improve instruction timing
 *
 */

#include "emu.h"
#include "debugger.h"
#include "r4000.h"
#include "mips3dsm.h"
#include "unicode.h"

#include "softfloat3/source/include/softfloat.h"

#define LOG_GENERAL   (1U << 0)
#define LOG_TLB       (1U << 1)
#define LOG_CACHE     (1U << 2)
#define LOG_EXCEPTION (1U << 3)
#define LOG_SYSCALL   (1U << 4)
#define LOG_STATS     (1U << 5)

#define VERBOSE       (LOG_GENERAL)

// operating system specific system call logging
#define SYSCALL_IRIX53 (1U << 0)
#define SYSCALL_WINNT4 (1U << 1)
#if VERBOSE & LOG_SYSCALL
#define SYSCALL_MASK   (SYSCALL_IRIX53)
#else
#define SYSCALL_MASK   (0)
#endif

// experimental primary instruction cache
#define ICACHE 0

#include "logmacro.h"

#define USE_ABI_REG_NAMES 1

// cpu instruction fields
#define RSREG ((op >> 21) & 31)
#define RTREG ((op >> 16) & 31)
#define RDREG ((op >> 11) & 31)
#define SHIFT ((op >> 6) & 31)

// cop1 instruction fields
#define FRREG ((op >> 21) & 31)
#define FTREG ((op >> 16) & 31)
#define FSREG ((op >> 11) & 31)
#define FDREG ((op >> 6) & 31)

#define R4000_ENDIAN_LE_BE(le, be) ((m_cp0[CP0_Config] & CONFIG_BE) ? (be) : (le))

// identify odd-numbered cop1 registers
#define ODD_REGS 0x00010840U

// address computation
#define ADDR(r, o) ((r) + (o))

#define SR         m_cp0[CP0_Status]
#define CAUSE      m_cp0[CP0_Cause]

DEFINE_DEVICE_TYPE(R4000, r4000_device, "r4000", "MIPS R4000")
DEFINE_DEVICE_TYPE(R4400, r4400_device, "r4400", "MIPS R4400")
DEFINE_DEVICE_TYPE(R4600, r4600_device, "r4600", "QED R4600")
DEFINE_DEVICE_TYPE(R5000, r5000_device, "r5000", "MIPS R5000")

u32 const r5000_device::s_fcc_masks[8] = { (1U << 23), (1U << 25), (1U << 26), (1U << 27), (1U << 28), (1U << 29), (1U << 30), (1U << 31) };
u32 const r5000_device::s_fcc_shifts[8] = { 23, 25, 26, 27, 28, 29, 30, 31 };

r4000_base_device::r4000_base_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, u32 prid, u32 fcr, cache_size icache_size, cache_size dcache_size, unsigned m32, unsigned m64, unsigned d32, unsigned d64)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config_le("program", ENDIANNESS_LITTLE, 64, 32)
	, m_program_config_be("program", ENDIANNESS_BIG, 64, 32)
	, m_hilo_cycles{ m32, m64, d32, d64 }
	, m_r{}
	, m_cp0{}
	, m_f{}
	, m_fcr0(fcr)
{
	m_cp0[CP0_PRId] = prid;

	// default configuration
	m_cp0[CP0_Config] = CONFIG_BE | (icache_size << 9) | (dcache_size << 6);
}

void r4000_base_device::device_start()
{
	// TODO: save state

	state_add(STATE_GENPC,     "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(MIPS3_PC,        "PC", m_pc).formatstr("%016X");

	// exception processing
	state_add(MIPS3_CP0 + CP0_Status,    "SR",       m_cp0[CP0_Status]).formatstr("%08X");
	state_add(MIPS3_CP0 + CP0_EPC,       "EPC",      m_cp0[CP0_EPC]).formatstr("%016X");
	state_add(MIPS3_CP0 + CP0_Cause,     "Cause",    m_cp0[CP0_Cause]).formatstr("%08X");
	state_add(MIPS3_CP0 + CP0_Context,   "Context",  m_cp0[CP0_Context]).formatstr("%016X");
	state_add(MIPS3_CP0 + CP0_BadVAddr,  "BadVAddr", m_cp0[CP0_BadVAddr]).formatstr("%016X");
	state_add(MIPS3_CP0 + CP0_Compare,   "Compare",  m_cp0[CP0_Compare]).formatstr("%08X");
	state_add(MIPS3_CP0 + CP0_WatchLo,   "WatchLo",  m_cp0[CP0_WatchLo]).formatstr("%08X");
	state_add(MIPS3_CP0 + CP0_WatchHi,   "WatchHi",  m_cp0[CP0_WatchHi]).formatstr("%08X");
	state_add(MIPS3_CP0 + CP0_XContext,  "XContext", m_cp0[CP0_XContext]).formatstr("%016X");

	// memory management
	state_add(MIPS3_CP0 + CP0_Index,     "Index",    m_cp0[CP0_Index]).formatstr("%08X");
	state_add(MIPS3_CP0 + CP0_EntryLo0,  "EntryLo0", m_cp0[CP0_EntryLo0]).formatstr("%016X");
	state_add(MIPS3_CP0 + CP0_EntryLo1,  "EntryLo1", m_cp0[CP0_EntryLo1]).formatstr("%016X");
	state_add(MIPS3_CP0 + CP0_PageMask,  "PageMask", m_cp0[CP0_PageMask]).formatstr("%08X");
	state_add(MIPS3_CP0 + CP0_Wired,     "Wired",    m_cp0[CP0_Wired]).formatstr("%08X");
	state_add(MIPS3_CP0 + CP0_EntryHi,   "EntryHi",  m_cp0[CP0_EntryHi]).formatstr("%016X");
	state_add(MIPS3_CP0 + CP0_LLAddr,    "LLAddr",   m_cp0[CP0_LLAddr]).formatstr("%08X");

#if USE_ABI_REG_NAMES
	state_add(MIPS3_R0 + 0,  "zero", m_r[0]).callimport().formatstr("%016X");   // Can't change R0
	state_add(MIPS3_R0 + 1,  "at", m_r[1]).formatstr("%016X");
	state_add(MIPS3_R0 + 2,  "v0", m_r[2]).formatstr("%016X");
	state_add(MIPS3_R0 + 3,  "v1", m_r[3]).formatstr("%016X");
	state_add(MIPS3_R0 + 4,  "a0", m_r[4]).formatstr("%016X");
	state_add(MIPS3_R0 + 5,  "a1", m_r[5]).formatstr("%016X");
	state_add(MIPS3_R0 + 6,  "a2", m_r[6]).formatstr("%016X");
	state_add(MIPS3_R0 + 7,  "a3", m_r[7]).formatstr("%016X");
	state_add(MIPS3_R0 + 8,  "t0", m_r[8]).formatstr("%016X");
	state_add(MIPS3_R0 + 9,  "t1", m_r[9]).formatstr("%016X");
	state_add(MIPS3_R0 + 10, "t2", m_r[10]).formatstr("%016X");
	state_add(MIPS3_R0 + 11, "t3", m_r[11]).formatstr("%016X");
	state_add(MIPS3_R0 + 12, "t4", m_r[12]).formatstr("%016X");
	state_add(MIPS3_R0 + 13, "t5", m_r[13]).formatstr("%016X");
	state_add(MIPS3_R0 + 14, "t6", m_r[14]).formatstr("%016X");
	state_add(MIPS3_R0 + 15, "t7", m_r[15]).formatstr("%016X");
	state_add(MIPS3_R0 + 16, "s0", m_r[16]).formatstr("%016X");
	state_add(MIPS3_R0 + 17, "s1", m_r[17]).formatstr("%016X");
	state_add(MIPS3_R0 + 18, "s2", m_r[18]).formatstr("%016X");
	state_add(MIPS3_R0 + 19, "s3", m_r[19]).formatstr("%016X");
	state_add(MIPS3_R0 + 20, "s4", m_r[20]).formatstr("%016X");
	state_add(MIPS3_R0 + 21, "s5", m_r[21]).formatstr("%016X");
	state_add(MIPS3_R0 + 22, "s6", m_r[22]).formatstr("%016X");
	state_add(MIPS3_R0 + 23, "s7", m_r[23]).formatstr("%016X");
	state_add(MIPS3_R0 + 24, "t8", m_r[24]).formatstr("%016X");
	state_add(MIPS3_R0 + 25, "t9", m_r[25]).formatstr("%016X");
	state_add(MIPS3_R0 + 26, "k0", m_r[26]).formatstr("%016X");
	state_add(MIPS3_R0 + 27, "k1", m_r[27]).formatstr("%016X");
	state_add(MIPS3_R0 + 28, "gp", m_r[28]).formatstr("%016X");
	state_add(MIPS3_R0 + 29, "sp", m_r[29]).formatstr("%016X");
	state_add(MIPS3_R0 + 30, "fp", m_r[30]).formatstr("%016X");
	state_add(MIPS3_R0 + 31, "ra", m_r[31]).formatstr("%016X");
#else
	state_add(MIPS3_R0,        "R0",       m_r[0]).callimport().formatstr("%016X");
	for (unsigned i = 1; i < 32; i++)
		state_add(MIPS3_R0 + i, util::string_format("R%d", i).c_str(), m_r[i]);
#endif

	state_add(MIPS3_HI,        "HI",       m_hi).formatstr("%016X");
	state_add(MIPS3_LO,        "LO",       m_lo).formatstr("%016X");

	// floating point registers
	state_add(MIPS3_FCR30,     "FCR30",    m_fcr30).formatstr("%08X");
	state_add(MIPS3_FCR31,     "FCR31",    m_fcr31).formatstr("%08X");
	for (unsigned i = 0; i < 32; i++)
		state_add(MIPS3_F0 + i, util::string_format("F%d", i).c_str(), m_f[i]);

	set_icountptr(m_icount);

	m_hard_reset = true;

	m_cp0_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(r4000_base_device::cp0_timer_callback), this));

	// compute icache line selection mask and allocate tag and data
	unsigned const config_ic = (m_cp0[CP0_Config] & CONFIG_IC) >> 9;

	m_icache_mask_hi = (0x1000U << config_ic) - 1;
	m_icache_tag = std::make_unique<u32[]>(0x100U << config_ic);
	m_icache_data = std::make_unique<u32 []>((0x1000U << config_ic) >> 2);

	R4000_ENDIAN_LE_BE(accessors(m_le), accessors(m_be));
}

void r4000_base_device::device_reset()
{
	if (!m_hard_reset)
	{
		m_cp0[CP0_Status] = SR_BEV | SR_ERL | SR_SR;
		m_cp0[CP0_ErrorEPC] = m_pc;
	}
	else
		m_cp0[CP0_Status] = SR_BEV | SR_ERL;

	m_hilo_delay = 0;
	m_branch_state = NONE;
	m_pc = s64(s32(0xbfc00000));
	m_r[0] = 0;

	m_cp0[CP0_Wired] = 0;
	m_cp0[CP0_Compare] = 0;
	m_cp0[CP0_Count] = 0;

	m_cp0_timer_zero = total_cycles();

	m_hard_reset = false;
	m_ll_active = false;
	m_bus_error = false;

	m_cp0[CP0_Cause] = 0;

	m_cp0[CP0_WatchLo] = 0;
	m_cp0[CP0_WatchHi] = 0;

	// initialize tlb mru index with identity mapping
	for (unsigned i = 0; i < std::size(m_tlb); i++)
	{
		m_tlb_mru[TRANSLATE_READ][i] = i;
		m_tlb_mru[TRANSLATE_WRITE][i] = i;
		m_tlb_mru[TRANSLATE_FETCH][i] = i;
	}

	// initialize statistics
	m_tlb_scans = 0;
	m_tlb_loops = 0;
	m_icache_hits = 0;
	m_icache_misses = 0;
}

void r4000_base_device::device_stop()
{
	if ((m_icache_hits + m_icache_misses) > 0)
		LOGMASKED(LOG_STATS, "icache hit ratio %.3f%% (%d hits %d misses)\n",
			double(m_icache_hits) / double(m_icache_hits + m_icache_misses) * 100.0, m_icache_hits, m_icache_misses);

	if (m_tlb_scans > 0)
		LOGMASKED(LOG_STATS, "tlb scans %d loops %d average %.3f loops per scan\n", m_tlb_scans, m_tlb_loops, double(m_tlb_loops) / double(m_tlb_scans));
}

device_memory_interface::space_config_vector r4000_base_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, R4000_ENDIAN_LE_BE(&m_program_config_le, &m_program_config_be))
	};
}

bool r4000_base_device::memory_translate(int spacenum, int intention, offs_t &address)
{
	// FIXME: address truncation
	u64 placeholder = s32(address);

	translate_result const t = translate(intention, placeholder);

	if (t == ERROR || t == MISS)
		return false;

	address = placeholder;
	return true;
}

std::unique_ptr<util::disasm_interface> r4000_base_device::create_disassembler()
{
	return std::make_unique<mips3_disassembler>();
}

void r4000_base_device::execute_run()
{
	while (m_icount > 0)
	{
		debugger_instruction_hook(m_pc);

		fetch(m_pc,
			[this](u32 const op)
			{
				// check interrupts
				if ((CAUSE & SR & CAUSE_IP) && (SR & SR_IE) && !(SR & (SR_EXL | SR_ERL)))
					cpu_exception(EXCEPTION_INT);
				else
					cpu_execute(op);

				// zero register zero
				m_r[0] = 0;
			});

		// update pc and branch state
		switch (m_branch_state & STATE)
		{
		case NONE:
			m_pc += 4;
			break;

		case BRANCH:
			m_pc = m_branch_state & TARGET;
			m_branch_state = NONE;
			break;

		case DELAY:
			m_pc += 4;
			m_branch_state = (m_branch_state & TARGET) | BRANCH;
			break;

		case NULLIFY:
			m_pc += 8;
			m_branch_state = NONE;
			break;
		}

		if (m_hilo_delay)
			m_hilo_delay--;

		m_icount--;
	}
}

void r4000_base_device::execute_set_input(int inputnum, int state)
{
	if (state)
		m_cp0[CP0_Cause] |= (CAUSE_IPEX0 << inputnum);
	else
		m_cp0[CP0_Cause] &= ~(CAUSE_IPEX0 << inputnum);
}

void r4000_base_device::cpu_execute(u32 const op)
{
	switch (op >> 26)
	{
	case 0x00: // SPECIAL
		switch (op & 0x3f)
		{
		case 0x00: // SLL
			m_r[RDREG] = s64(s32(m_r[RTREG] << SHIFT));
			break;
		//case 0x01: // *
		case 0x02: // SRL
			m_r[RDREG] = s64(s32(u32(m_r[RTREG]) >> SHIFT));
			break;
		case 0x03: // SRA
			m_r[RDREG] = s64(s32(m_r[RTREG]) >> SHIFT);
			break;
		case 0x04: // SLLV
			m_r[RDREG] = s64(s32(m_r[RTREG] << (m_r[RSREG] & 31)));
			break;
		//case 0x05: // *
		case 0x06: // SRLV
			m_r[RDREG] = s64(s32(u32(m_r[RTREG]) >> (m_r[RSREG] & 31)));
			break;
		case 0x07: // SRAV
			m_r[RDREG] = s64(s32(m_r[RTREG]) >> (m_r[RSREG] & 31));
			break;
		case 0x08: // JR
			// FIXME: address error if low bits are set
			m_branch_state = ADDR(m_r[RSREG], 0) | DELAY;
			break;
		case 0x09: // JALR
			// FIXME: address error if low bits are set
			m_branch_state = ADDR(m_r[RSREG], 0) | DELAY;
			m_r[RDREG] = ADDR(m_pc, 8);
			break;
		//case 0x0a: // *
		//case 0x0b: // *
		case 0x0c: // SYSCALL
			if (VERBOSE & LOG_SYSCALL)
			{
				if (SYSCALL_MASK & SYSCALL_IRIX53)
				{
					switch (m_r[2])
					{
					case 0x3e9: // 1001 = exit
						LOGMASKED(LOG_SYSCALL, "exit(%d) (%s)\n", m_r[4], machine().describe_context());
						break;

					case 0x3ea: // 1002 = fork
						LOGMASKED(LOG_SYSCALL, "fork() (%s)\n", machine().describe_context());
						break;

					case 0x3eb: // 1003 = read
						LOGMASKED(LOG_SYSCALL, "read(%d, 0x%x, %d) (%s)\n", m_r[4], m_r[5], m_r[6], machine().describe_context());
						break;

					case 0x3ec: // 1004 = write
						LOGMASKED(LOG_SYSCALL, "write(%d, 0x%x, %d) (%s)\n", m_r[4], m_r[5], m_r[6], machine().describe_context());
						if (m_r[4] == 1 || m_r[4] == 2)
							printf("%s", debug_string(m_r[5], m_r[6]).c_str());
						break;

					case 0x3ed: // 1005 = open
						LOGMASKED(LOG_SYSCALL, "open(\"%s\", %#o) (%s)\n", debug_string(m_r[4]), m_r[5], machine().describe_context());
						break;

					case 0x3ee: // 1006 = close
						LOGMASKED(LOG_SYSCALL, "close(%d) (%s)\n", m_r[4], machine().describe_context());
						break;

					case 0x3ef: // 1007 = creat
						LOGMASKED(LOG_SYSCALL, "creat(\"%s\", %#o) (%s)\n", debug_string(m_r[4]), m_r[5], machine().describe_context());
						break;

					case 0x423: // 1059 = exece
						LOGMASKED(LOG_SYSCALL, "exece(\"%s\", [ %s ], [ %s ]) (%s)\n", debug_string(m_r[4]), debug_string_array(m_r[5]), debug_string_array(m_r[6]), machine().describe_context());
						break;

					default:
						LOGMASKED(LOG_SYSCALL, "syscall 0x%x (%s)\n", m_r[2], machine().describe_context());
						break;
					}
				}
				else if (SYSCALL_MASK & SYSCALL_WINNT4)
					LOGMASKED(LOG_SYSCALL, "syscall 0x%02x from 0x%08x (%s)\n", m_r[2], u32(m_r[31] - 8), machine().describe_context());
			}
			cpu_exception(EXCEPTION_SYS);
			break;
		case 0x0d: // BREAK
			cpu_exception(EXCEPTION_BP);
			break;
		//case 0x0e: // *
		case 0x0f: // SYNC
			break;
		case 0x10: // MFHI
			m_r[RDREG] = m_hi;
			if (m_hilo_delay)
			{
				m_icount -= m_hilo_delay;
				m_hilo_delay = 0;
			}
			break;
		case 0x11: // MTHI
			m_hi = m_r[RSREG];
			break;
		case 0x12: // MFLO
			m_r[RDREG] = m_lo;
			if (m_hilo_delay)
			{
				m_icount -= m_hilo_delay;
				m_hilo_delay = 0;
			}
			break;
		case 0x13: // MTLO
			m_lo = m_r[RSREG];
			break;
		case 0x14: // DSLLV
			m_r[RDREG] = m_r[RTREG] << (m_r[RSREG] & 63);
			break;
		//case 0x15: // *
		case 0x16: // DSRLV
			m_r[RDREG] = m_r[RTREG] >> (m_r[RSREG] & 63);
			break;
		case 0x17: // DSRAV
			m_r[RDREG] = s64(m_r[RTREG]) >> (m_r[RSREG] & 63);
			break;
		case 0x18: // MULT
			{
				u64 const product = mul_32x32(s32(m_r[RSREG]), s32(m_r[RTREG]));

				m_lo = s64(s32(product));
				m_hi = s64(s32(product >> 32));
				m_hilo_delay = m_hilo_cycles[0];
			}
			break;
		case 0x19: // MULTU
			{
				u64 const product = mulu_32x32(u32(m_r[RSREG]), u32(m_r[RTREG]));

				m_lo = s64(s32(product));
				m_hi = s64(s32(product >> 32));
				m_hilo_delay = m_hilo_cycles[0];
			}
			break;
		case 0x1a: // DIV
			if (m_r[RTREG])
			{
				m_lo = s64(s32(m_r[RSREG]) / s32(m_r[RTREG]));
				m_hi = s64(s32(m_r[RSREG]) % s32(m_r[RTREG]));
				m_hilo_delay = m_hilo_cycles[2];
			}
			break;
		case 0x1b: // DIVU
			if (m_r[RTREG])
			{
				m_lo = s64(s32(u32(m_r[RSREG]) / u32(m_r[RTREG])));
				m_hi = s64(s32(u32(m_r[RSREG]) % u32(m_r[RTREG])));
				m_hilo_delay = m_hilo_cycles[2];
			}
			break;
		case 0x1c: // DMULT
			m_lo = mul_64x64(m_r[RSREG], m_r[RTREG], *reinterpret_cast<s64 *>(&m_hi));
			m_hilo_delay = m_hilo_cycles[1];
			break;
		case 0x1d: // DMULTU
			m_lo = mulu_64x64(m_r[RSREG], m_r[RTREG], m_hi);
			m_hilo_delay = m_hilo_cycles[1];
			break;
		case 0x1e: // DDIV
			if (m_r[RTREG])
			{
				m_lo = s64(m_r[RSREG]) / s64(m_r[RTREG]);
				m_hi = s64(m_r[RSREG]) % s64(m_r[RTREG]);
				m_hilo_delay = m_hilo_cycles[3];
			}
			break;
		case 0x1f: // DDIVU
			if (m_r[RTREG])
			{
				m_lo = m_r[RSREG] / m_r[RTREG];
				m_hi = m_r[RSREG] % m_r[RTREG];
				m_hilo_delay = m_hilo_cycles[3];
			}
			break;
		case 0x20: // ADD
			{
				u32 const sum = u32(m_r[RSREG]) + u32(m_r[RTREG]);

				// overflow: (sign(addend0) == sign(addend1)) && (sign(addend0) != sign(sum))
				if (!BIT(u32(m_r[RSREG]) ^ u32(m_r[RTREG]), 31) && BIT(u32(m_r[RSREG]) ^ sum, 31))
					cpu_exception(EXCEPTION_OV);
				else
					m_r[RDREG] = s64(s32(sum));
			}
			break;
		case 0x21: // ADDU
			m_r[RDREG] = s64(s32(u32(m_r[RSREG]) + u32(m_r[RTREG])));
			break;
		case 0x22: // SUB
			{
				u32 const difference = u32(m_r[RSREG]) - u32(m_r[RTREG]);

				// overflow: (sign(minuend) != sign(subtrahend)) && (sign(minuend) != sign(difference))
				if (BIT(u32(m_r[RSREG]) ^ u32(m_r[RTREG]), 31) && BIT(u32(m_r[RSREG]) ^ difference, 31))
					cpu_exception(EXCEPTION_OV);
				else
					m_r[RDREG] = s64(s32(difference));
			}
			break;
		case 0x23: // SUBU
			m_r[RDREG] = s64(s32(u32(m_r[RSREG]) - u32(m_r[RTREG])));
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
		//case 0x28: // *
		//case 0x29: // *
		case 0x2a: // SLT
			m_r[RDREG] = s64(m_r[RSREG]) < s64(m_r[RTREG]);
			break;
		case 0x2b: // SLTU
			m_r[RDREG] = m_r[RSREG] < m_r[RTREG];
			break;
		case 0x2c: // DADD
			{
				u64 const sum = m_r[RSREG] + m_r[RTREG];

				// overflow: (sign(addend0) == sign(addend1)) && (sign(addend0) != sign(sum))
				if (!BIT(m_r[RSREG] ^ m_r[RTREG], 63) && BIT(m_r[RSREG] ^ sum, 63))
					cpu_exception(EXCEPTION_OV);
				else
					m_r[RDREG] = sum;
			}
			break;
		case 0x2d: // DADDU
			m_r[RDREG] = m_r[RSREG] + m_r[RTREG];
			break;
		case 0x2e: // DSUB
			{
				u64 const difference = m_r[RSREG] - m_r[RTREG];

				// overflow: (sign(minuend) != sign(subtrahend)) && (sign(minuend) != sign(difference))
				if (BIT(m_r[RSREG] ^ m_r[RTREG], 63) && BIT(m_r[RSREG] ^ difference, 63))
					cpu_exception(EXCEPTION_OV);
				else
					m_r[RDREG] = difference;
			}
			break;
		case 0x2f: // DSUBU
			m_r[RDREG] = m_r[RSREG] - m_r[RTREG];
			break;
		case 0x30: // TGE
			if (s64(m_r[RSREG]) >= s64(m_r[RTREG]))
				cpu_exception(EXCEPTION_TR);
			break;
		case 0x31: // TGEU
			if (m_r[RSREG] >= m_r[RTREG])
				cpu_exception(EXCEPTION_TR);
			break;
		case 0x32: // TLT
			if (s64(m_r[RSREG]) < s64(m_r[RTREG]))
				cpu_exception(EXCEPTION_TR);
			break;
		case 0x33: // TLTU
			if (m_r[RSREG] < m_r[RTREG])
				cpu_exception(EXCEPTION_TR);
			break;
		case 0x34: // TEQ
			if (m_r[RSREG] == m_r[RTREG])
				cpu_exception(EXCEPTION_TR);
			break;
		//case 0x35: // *
		case 0x36: // TNE
			if (m_r[RSREG] != m_r[RTREG])
				cpu_exception(EXCEPTION_TR);
			break;
		//case 0x37: // *
		case 0x38: // DSLL
			m_r[RDREG] = m_r[RTREG] << SHIFT;
			break;
		//case 0x39: // *
		case 0x3a: // DSRL
			m_r[RDREG] = m_r[RTREG] >> SHIFT;
			break;
		case 0x3b: // DSRA
			m_r[RDREG] = s64(m_r[RTREG]) >> SHIFT;
			break;
		case 0x3c: // DSLL32
			m_r[RDREG] = m_r[RTREG] << (SHIFT + 32);
			break;
		//case 0x3d: // *
		case 0x3e: // DSRL32
			m_r[RDREG] = m_r[RTREG] >> (SHIFT + 32);
			break;
		case 0x3f: // DSRA32
			m_r[RDREG] = s64(m_r[RTREG]) >> (SHIFT + 32);
			break;

		default:
			handle_reserved_instruction(op);
			break;
		}
		break;
	case 0x01: // REGIMM
		switch ((op >> 16) & 0x1f)
		{
		case 0x00: // BLTZ
			if (s64(m_r[RSREG]) < 0)
				m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
			break;
		case 0x01: // BGEZ
			if (s64(m_r[RSREG]) >= 0)
				m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
			break;
		case 0x02: // BLTZL
			if (s64(m_r[RSREG]) < 0)
				m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
			else
				m_branch_state = NULLIFY;
			break;
		case 0x03: // BGEZL
			if (s64(m_r[RSREG]) >= 0)
				m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
			else
				m_branch_state = NULLIFY;
			break;
		//case 0x04: // *
		//case 0x05: // *
		//case 0x06: // *
		//case 0x07: // *
		case 0x08: // TGEI
			if (s64(m_r[RSREG]) >= s16(op))
				cpu_exception(EXCEPTION_TR);
			break;
		case 0x09: // TGEIU
			if (m_r[RSREG] >= u64(s64(s16(op))))
				cpu_exception(EXCEPTION_TR);
			break;
		case 0x0a: // TLTI
			if (s64(m_r[RSREG]) < s16(op))
				cpu_exception(EXCEPTION_TR);
			break;
		case 0x0b: // TLTIU
			if (m_r[RSREG] >= u64(s64(s16(op))))
				cpu_exception(EXCEPTION_TR);
			break;
		case 0x0c: // TEQI
			if (m_r[RSREG] == u64(s64(s16(op))))
				cpu_exception(EXCEPTION_TR);
			break;
		//case 0x0d: // *
		case 0x0e: // TNEI
			if (m_r[RSREG] != u64(s64(s16(op))))
				cpu_exception(EXCEPTION_TR);
			break;
		//case 0x0f: // *
		case 0x10: // BLTZAL
			if (s64(m_r[RSREG]) < 0)
				m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
			m_r[31] = ADDR(m_pc, 8);
			break;
		case 0x11: // BGEZAL
			if (s64(m_r[RSREG]) >= 0)
				m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
			m_r[31] = ADDR(m_pc, 8);
			break;
		case 0x12: // BLTZALL
			if (s64(m_r[RSREG]) < 0)
				m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
			else
				m_branch_state = NULLIFY;
			m_r[31] = ADDR(m_pc, 8);
			break;
		case 0x13: // BGEZALL
			if (s64(m_r[RSREG]) >= 0)
				m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
			else
				m_branch_state = NULLIFY;
			m_r[31] = ADDR(m_pc, 8);
			break;
		//case 0x14: // *
		//case 0x15: // *
		//case 0x16: // *
		//case 0x17: // *
		//case 0x18: // *
		//case 0x19: // *
		//case 0x1a: // *
		//case 0x1b: // *
		//case 0x1c: // *
		//case 0x1d: // *
		//case 0x1e: // *
		//case 0x1f: // *

		default:
			// * Operation codes marked with an asterisk cause reserved
			// instruction exceptions in all current implementations and are
			// reserved for future versions of the architecture.
			handle_reserved_instruction(op);
			break;
		}
		break;
	case 0x02: // J
		m_branch_state = (ADDR(m_pc, 4) & ~0x0fffffffULL) | ((op & 0x03ffffffU) << 2) | DELAY;
		break;
	case 0x03: // JAL
		m_branch_state = (ADDR(m_pc, 4) & ~0x0fffffffULL) | ((op & 0x03ffffffU) << 2) | DELAY;
		m_r[31] = ADDR(m_pc, 8);
		break;
	case 0x04: // BEQ
		if (m_r[RSREG] == m_r[RTREG])
			m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
		break;
	case 0x05: // BNE
		if (m_r[RSREG] != m_r[RTREG])
			m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
		break;
	case 0x06: // BLEZ
		if (s64(m_r[RSREG]) <= 0)
			m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
		break;
	case 0x07: // BGTZ
		if (s64(m_r[RSREG]) > 0)
			m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
		break;
	case 0x08: // ADDI
		{
			u32 const sum = u32(m_r[RSREG]) + s16(op);

			// overflow: (sign(addend0) == sign(addend1)) && (sign(addend0) != sign(sum))
			if (!BIT(u32(m_r[RSREG]) ^ s32(s16(op)), 31) && BIT(u32(m_r[RSREG]) ^ sum, 31))
				cpu_exception(EXCEPTION_OV);
			else
				m_r[RTREG] = s64(s32(sum));
		}
		break;
	case 0x09: // ADDIU
		m_r[RTREG] = s64(s32(u32(m_r[RSREG]) + s16(op)));
		break;
	case 0x0a: // SLTI
		m_r[RTREG] = s64(m_r[RSREG]) < s64(s16(op));
		break;
	case 0x0b: // SLTIU
		m_r[RTREG] = m_r[RSREG] < u64(s64(s16(op)));
		break;
	case 0x0c: // ANDI
		m_r[RTREG] = m_r[RSREG] & u16(op);
		break;
	case 0x0d: // ORI
		m_r[RTREG] = m_r[RSREG] | u16(op);
		break;
	case 0x0e: // XORI
		m_r[RTREG] = m_r[RSREG] ^ u16(op);
		break;
	case 0x0f: // LUI
		m_r[RTREG] = s64(s16(op)) << 16;
		break;
	case 0x10: // COP0
		cp0_execute(op);
		break;
	case 0x11: // COP1
		cp1_execute(op);
		break;
	case 0x12: // COP2
		cp2_execute(op);
		break;
	case 0x13: // COP1X
		cp1x_execute(op);
		break;
	case 0x14: // BEQL
		if (m_r[RSREG] == m_r[RTREG])
			m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
		else
			m_branch_state = NULLIFY;
		break;
	case 0x15: // BNEL
		if (m_r[RSREG] != m_r[RTREG])
			m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
		else
			m_branch_state = NULLIFY;
		break;
	case 0x16: // BLEZL
		if (s64(m_r[RSREG]) <= 0)
			m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
		else
			m_branch_state = NULLIFY;
		break;
	case 0x17: // BGTZL
		if (s64(m_r[RSREG]) > 0)
			m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
		else
			m_branch_state = NULLIFY;
		break;
	case 0x18: // DADDI
		{
			u64 const sum = m_r[RSREG] + s64(s16(op));

			// overflow: (sign(addend0) == sign(addend1)) && (sign(addend0) != sign(sum))
			if (!BIT(m_r[RSREG] ^ s64(s16(op)), 63) && BIT(m_r[RSREG] ^ sum, 63))
				cpu_exception(EXCEPTION_OV);
			else
				m_r[RTREG] = sum;
		}
		break;
	case 0x19: // DADDIU
		m_r[RTREG] = m_r[RSREG] + s64(s16(op));
		break;
	case 0x1a: // LDL
		if (!(SR & SR_KSU) || (SR & (SR_EXL | SR_ERL)) || cp0_64())
			cpu_ldl(op);
		else
			cpu_exception(EXCEPTION_RI);
		break;
	case 0x1b: // LDR
		if (!(SR & SR_KSU) || (SR & (SR_EXL | SR_ERL)) || cp0_64())
			cpu_ldr(op);
		else
			cpu_exception(EXCEPTION_RI);
		break;
	//case 0x1c: // *
	//case 0x1d: // *
	//case 0x1e: // *
	//case 0x1f: // *
	case 0x20: // LB
		load<u8>(ADDR(m_r[RSREG], s16(op)),
			[this, op](s8 data)
			{
				m_r[RTREG] = data;
			});
		break;
	case 0x21: // LH
		load<u16>(ADDR(m_r[RSREG], s16(op)),
			[this, op](s16 data)
			{
				m_r[RTREG] = data;
			});
		break;
	case 0x22: // LWL
		cpu_lwl(op);
		break;
	case 0x23: // LW
		load<u32>(ADDR(m_r[RSREG], s16(op)),
			[this, op](s32 data)
			{
				m_r[RTREG] = data;
			});
		break;
	case 0x24: // LBU
		load<u8>(ADDR(m_r[RSREG], s16(op)),
			[this, op](u8 data)
			{
				m_r[RTREG] = data;
			});
		break;
	case 0x25: // LHU
		load<u16>(ADDR(m_r[RSREG], s16(op)),
			[this, op](u16 data)
			{
				m_r[RTREG] = data;
			});
		break;
	case 0x26: // LWR
		cpu_lwr(op);
		break;
	case 0x27: // LWU
		load<u32>(ADDR(m_r[RSREG], s16(op)),
			[this, op](u32 data)
			{
				m_r[RTREG] = data;
			});
		break;
	case 0x28: // SB
		store<u8>(ADDR(m_r[RSREG], s16(op)), u8(m_r[RTREG]));
		break;
	case 0x29: // SH
		store<u16>(ADDR(m_r[RSREG], s16(op)), u16(m_r[RTREG]));
		break;
	case 0x2a: // SWL
		cpu_swl(op);
		break;
	case 0x2b: // SW
		store<u32>(ADDR(m_r[RSREG], s16(op)), u32(m_r[RTREG]));
		break;
	case 0x2c: // SDL
		if (!(SR & SR_KSU) || (SR & (SR_EXL | SR_ERL)) || cp0_64())
			cpu_sdl(op);
		else
			cpu_exception(EXCEPTION_RI);
		break;
	case 0x2d: // SDR
		if (!(SR & SR_KSU) || (SR & (SR_EXL | SR_ERL)) || cp0_64())
			cpu_sdr(op);
		else
			cpu_exception(EXCEPTION_RI);
		break;
	case 0x2e: // SWR
		cpu_swr(op);
		break;
	case 0x2f: // CACHE
		if ((SR & SR_KSU) && !(SR & SR_CU0) && !(SR & (SR_EXL | SR_ERL)))
		{
			cpu_exception(EXCEPTION_CP0);
			break;
		}

		switch ((op >> 16) & 0x1f)
		{
		case 0x00: // index invalidate (I)
			if (ICACHE)
			{
				m_icache_tag[(ADDR(m_r[RSREG], s16(op)) & m_icache_mask_hi) >> m_icache_shift] &= ~ICACHE_V;
				break;
			}
			[[fallthrough]];
		case 0x04: // index load tag (I)
			if (ICACHE)
			{
				u32 const tag = m_icache_tag[(ADDR(m_r[RSREG], s16(op)) & m_icache_mask_hi) >> m_icache_shift];

				m_cp0[CP0_TagLo] = ((tag & ICACHE_PTAG) << 8) | ((tag & ICACHE_V) >> 18) | ((tag & ICACHE_P) >> 25);
				m_cp0[CP0_ECC] = 0; // data ecc or parity

				break;
			}
			[[fallthrough]];
		case 0x08: // index store tag (I)
			if (ICACHE)
			{
				// FIXME: compute parity
				m_icache_tag[(ADDR(m_r[RSREG], s16(op)) & m_icache_mask_hi) >> m_icache_shift] =
					(m_cp0[CP0_TagLo] & TAGLO_PTAGLO) >> 8 | (m_cp0[CP0_TagLo] & TAGLO_PSTATE) << 18;

				break;
			}
			[[fallthrough]];
		case 0x01: // index writeback invalidate (D)
		case 0x02: // index invalidate (SI)
		case 0x03: // index writeback invalidate (SD)

		case 0x05: // index load tag (D)
		case 0x06: // index load tag (SI)
		case 0x07: // index load tag (SI)

		case 0x09: // index store tag (D)
		case 0x0a: // index store tag (SI)
		case 0x0b: // index store tag (SD)

		case 0x0d: // create dirty exclusive (D)
		case 0x0f: // create dirty exclusive (SD)

		case 0x10: // hit invalidate (I)
		case 0x11: // hit invalidate (D)
		case 0x12: // hit invalidate (SI)
		case 0x13: // hit invalidate (SD)

		case 0x14: // fill (I)
		case 0x15: // hit writeback invalidate (D)
		case 0x17: // hit writeback invalidate (SD)

		case 0x18: // hit writeback (I)
		case 0x19: // hit writeback (D)
		case 0x1b: // hit writeback (SD)

		case 0x1e: // hit set virtual (SI)
		case 0x1f: // hit set virtual (SD)
			//LOGMASKED(LOG_CACHE, "cache 0x%08x unimplemented (%s)\n", op, machine().describe_context());
			break;
		}
		break;
	case 0x30: // LL
		load_linked<u32>(ADDR(m_r[RSREG], s16(op)),
			[this, op](u64 address, s32 data)
			{
				m_r[RTREG] = s64(data);
				m_cp0[CP0_LLAddr] = u32(address >> 4);
				m_ll_active = true;
			});
		break;
	case 0x31: // LWC1
		cp1_execute(op);
		break;
	case 0x32: // LWC2
		cp2_execute(op);
		break;
	case 0x34: // LLD
		load_linked<u64>(ADDR(m_r[RSREG], s16(op)),
			[this, op](u64 address, u64 data)
			{
				m_r[RTREG] = data;
				m_cp0[CP0_LLAddr] = u32(address >> 4);
				m_ll_active = true;
			});
		break;
	case 0x35: // LDC1
		cp1_execute(op);
		break;
	case 0x36: // LDC2
		cp2_execute(op);
		break;
	case 0x37: // LD
		load<u64>(ADDR(m_r[RSREG], s16(op)),
			[this, op](u64 data)
			{
				m_r[RTREG] = data;
			});
		break;
	case 0x38: // SC
		if (m_ll_active)
		{
			if (store<u32>(ADDR(m_r[RSREG], s16(op)), u32(m_r[RTREG])))
				m_r[RTREG] = 1;
			else
				m_r[RTREG] = 0;

			m_ll_active = false;
		}
		else
			m_r[RTREG] = 0;
		break;
	case 0x39: // SWC1
		cp1_execute(op);
		break;
	case 0x3a: // SWC2
		cp2_execute(op);
		break;
	//case 0x3b: // *
	case 0x3c: // SCD
		if (m_ll_active)
		{
			if (store<u64>(ADDR(m_r[RSREG], s16(op)), m_r[RTREG]))
				m_r[RTREG] = 1;
			else
				m_r[RTREG] = 0;

			m_ll_active = false;
		}
		else
			m_r[RTREG] = 0;
		break;
	case 0x3d: // SDC1
		cp1_execute(op);
		break;
	case 0x3e: // SDC2
		cp2_execute(op);
		break;
	case 0x3f: // SD
		store<u64>(ADDR(m_r[RSREG], s16(op)), m_r[RTREG]);
		break;

	default:
		// * Operation codes marked with an asterisk cause reserved instruction
		// exceptions in all current implementations and are reserved for future
		// versions of the architecture.
		handle_reserved_instruction(op);
		break;
	}
}

void r4000_base_device::handle_reserved_instruction(u32 const op)
{
	// Unhandled operation codes cause reserved instruction
	// exceptions in all current implementations and are
	// reserved for future versions of the architecture.
	cpu_exception(EXCEPTION_RI);
}

void r5000_device::handle_reserved_instruction(u32 const op)
{
	switch (op >> 26)
	{
	case 0x00: // SPECIAL
		switch (op & 0x3f)
		{
		case 0x01: // MOVT / MOVF
			if (((m_fcr31 >> s_fcc_shifts[(op >> 18) & 7]) & 1) == ((op >> 16) & 1))
			{
				if (RDREG)
				{
					m_r[RDREG] = m_r[RSREG];
				}
			}
			return;
		case 0x0a: // MOVZ
			if (m_r[RTREG] == 0) { if (RDREG) m_r[RDREG] = m_r[RSREG]; }
			return;
		case 0x0b: // MOVN
			if (m_r[RTREG] != 0) { if (RDREG) m_r[RDREG] = m_r[RSREG]; }
			return;
		}
		break;
	case 0x33: // PREF (effective no-op)
		return;
	}

	r4000_base_device::handle_reserved_instruction(op);
}

void r4000_base_device::cpu_exception(u32 exception, u16 const vector)
{
	if (exception != EXCEPTION_INT)
		LOGMASKED(LOG_EXCEPTION, "exception 0x%08x\n", exception);

	if (!(SR & SR_EXL))
	{
		m_cp0[CP0_EPC] = m_pc;

		CAUSE = (CAUSE & CAUSE_IP) | exception;

		// if executing a delay slot instruction, restart from the branch
		if ((m_branch_state & STATE) == BRANCH)
		{
			m_cp0[CP0_EPC] -= 4;
			CAUSE |= CAUSE_BD;
		}

		SR |= SR_EXL;
	}
	else
		CAUSE = (CAUSE & (CAUSE_BD | CAUSE_IP)) | exception;

	m_branch_state = (((SR & SR_BEV) ? s64(s32(0xbfc00200)) : s64(s32(0x80000000))) + vector) | BRANCH;

	if (exception == EXCEPTION_INT)
	{
		if (machine().debug_flags & DEBUG_FLAG_ENABLED)
		{
			// notify the debugger of the first pending hardware interrupt
			u32 const iphw = CAUSE & SR & CAUSE_IPHW;

			if (iphw)
				debug()->interrupt_hook(22 - count_leading_zeros_32((iphw - 1) & ~iphw));
		}
	}
	else
		debugger_exception_hook(exception);
}

void r4000_base_device::cpu_lwl(u32 const op)
{
	u64 const offset = ADDR(m_r[RSREG], s16(op));
	unsigned const shift = ((offset & 3) ^ R4000_ENDIAN_LE_BE(3, 0)) << 3;

	load<u32, false>(offset,
		[this, op, shift](u32 const data)
		{
			m_r[RTREG] = s32((m_r[RTREG] & ~u32(~u32(0) << shift)) | (data << shift));
		});
}

void r4000_base_device::cpu_lwr(u32 const op)
{
	u64 const offset = ADDR(m_r[RSREG], s16(op));
	unsigned const shift = ((offset & 0x3) ^ R4000_ENDIAN_LE_BE(0, 3)) << 3;

	load<u32, false>(offset,
		[this, op, shift](u32 const data)
		{
			m_r[RTREG] = s32((m_r[RTREG] & ~u32(~u32(0) >> shift)) | (data >> shift));
		});
}

void r4000_base_device::cpu_swl(u32 const op)
{
	u64 const offset = ADDR(m_r[RSREG], s16(op));
	unsigned const shift = ((offset & 3) ^ R4000_ENDIAN_LE_BE(3, 0)) << 3;

	store<u32, false>(offset, u32(m_r[RTREG]) >> shift, ~u32(0) >> shift);
}

void r4000_base_device::cpu_swr(u32 const op)
{
	u64 const offset = ADDR(m_r[RSREG], s16(op));
	unsigned const shift = ((offset & 3) ^ R4000_ENDIAN_LE_BE(0, 3)) << 3;

	store<u32, false>(offset, u32(m_r[RTREG]) << shift, ~u32(0) << shift);
}

void r4000_base_device::cpu_ldl(u32 const op)
{
	u64 const offset = ADDR(m_r[RSREG], s16(op));
	unsigned const shift = ((offset & 7) ^ R4000_ENDIAN_LE_BE(7, 0)) << 3;

	load<u64, false>(offset,
		[this, op, shift](u64 const data)
		{
			m_r[RTREG] = (m_r[RTREG] & ~u64(~u64(0) << shift)) | (data << shift);
		});
}

void r4000_base_device::cpu_ldr(u32 const op)
{
	u64 const offset = ADDR(m_r[RSREG], s16(op));
	unsigned const shift = ((offset & 7) ^ R4000_ENDIAN_LE_BE(0, 7)) << 3;

	load<u64, false>(offset,
		[this, op, shift](u64 const data)
		{
			m_r[RTREG] = (m_r[RTREG] & ~u64(~u64(0) >> shift)) | (data >> shift);
		});
}

void r4000_base_device::cpu_sdl(u32 const op)
{
	u64 const offset = ADDR(m_r[RSREG], s16(op));
	unsigned const shift = ((offset & 7) ^ R4000_ENDIAN_LE_BE(7, 0)) << 3;

	store<u64, false>(offset, m_r[RTREG] >> shift, ~u64(0) >> shift);
}

void r4000_base_device::cpu_sdr(u32 const op)
{
	u64 const offset = ADDR(m_r[RSREG], s16(op));
	unsigned const shift = ((offset & 7) ^ R4000_ENDIAN_LE_BE(0, 7)) << 3;

	store<u64, false>(offset, m_r[RTREG] << shift, ~u64(0) << shift);
}

void r4000_base_device::cp0_execute(u32 const op)
{
	if ((SR & SR_KSU) && !(SR & SR_CU0) && !(SR & (SR_EXL | SR_ERL)))
	{
		cpu_exception(EXCEPTION_CP0);
		return;
	}

	switch ((op >> 21) & 0x1f)
	{
	case 0x00: // MFC0
		m_r[RTREG] = s32(cp0_get(RDREG));
		break;
	case 0x01: // DMFC0
		// ε Operation codes marked with epsilon are valid when the
		// processor is operating either in the Kernel mode or in the
		// 64-bit non-Kernel (User or Supervisor) mode. These instructions
		// cause a reserved instruction exception if 64-bit operation is
		// not enabled in User or Supervisor mode.
		if (!(SR & SR_KSU) || (SR & (SR_EXL | SR_ERL)) || cp0_64())
			m_r[RTREG] = cp0_get(RDREG);
		else
			cpu_exception(EXCEPTION_RI);
		break;
	case 0x02: // CFC0
		break;

	case 0x04: // MTC0
		cp0_set(RDREG, s64(s32(m_r[RTREG])));
		break;
	case 0x05: // DMTC0
		// ε Operation codes marked with epsilon are valid when the
		// processor is operating either in the Kernel mode or in the
		// 64-bit non-Kernel (User or Supervisor) mode. These instructions
		// cause a reserved instruction exception if 64-bit operation is
		// not enabled in User or Supervisor mode.
		if (!(SR & SR_KSU) || (SR & (SR_EXL | SR_ERL)) || cp0_64())
			cp0_set(RDREG, m_r[RTREG]);
		else
			cpu_exception(EXCEPTION_RI);
		break;
	case 0x06: // CTC0
		break;

	case 0x08: // BC0
		switch ((op >> 16) & 0x1f)
		{
			case 0x00: // BC0F
			case 0x01: // BC0T
			case 0x02: // BC0FL
			case 0x03: // BC0TL
				// fall through

			default:
				// γ Operation codes marked with a gamma cause a reserved
				// instruction exception. They are reserved for future versions
				// of the architecture.
				cpu_exception(EXCEPTION_RI);
				break;
		}
		break;

	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		// CP0 function
		switch (op & 0x3f)
		{
		case 0x01: // TLBR
			cp0_tlbr();
			break;
		case 0x02: // TLBWI
			cp0_tlbwi(m_cp0[CP0_Index] & 0x3f);
			break;

		case 0x06: // TLBWR
			cp0_tlbwr();
			break;

		case 0x08: // TLBP
			cp0_tlbp();
			break;

		case 0x10: // RFE
			// ξ Operation codes marked with a xi cause a reserved
			// instruction exception on R4000 processors.
			cpu_exception(EXCEPTION_RI);
			break;

		case 0x18: // ERET
			if (SR & SR_ERL)
			{
				m_branch_state = m_cp0[CP0_ErrorEPC] | BRANCH;
				SR &= ~SR_ERL;
			}
			else
			{
				m_branch_state = m_cp0[CP0_EPC] | BRANCH;
				SR &= ~SR_EXL;
			}

			m_ll_active = false;
			break;

		default:
			// Φ Operation codes marked with a phi are invalid but do not
			// cause reserved instruction exceptions in R4000 implementations.
			break;
		}
		break;

	default:
		// γ Operation codes marked with a gamma cause a reserved
		// instruction exception. They are reserved for future versions
		// of the architecture.
		cpu_exception(EXCEPTION_RI);
		break;
	}
}

u64 r4000_base_device::cp0_get(unsigned const reg)
{
	switch (reg)
	{
	case CP0_Count:
		return u32((total_cycles() - m_cp0_timer_zero) / 2);

	case CP0_Random:
		{
			u8 const wired = m_cp0[CP0_Wired] & 0x3f;

			if (wired < std::size(m_tlb))
				return ((total_cycles() - m_cp0_timer_zero) % (std::size(m_tlb) - wired) + wired) & 0x3f;
			else
				return std::size(m_tlb) - 1;
		}
		break;

	default:
		return m_cp0[reg];
	}
}

void r4000_base_device::cp0_set(unsigned const reg, u64 const data)
{
	switch (reg)
	{
	case CP0_Index:
		m_cp0[CP0_Index] = data & 0x3f;
		break;
	case CP0_EntryLo0:
		m_cp0[CP0_EntryLo0] = data & (EL_PFN | EL_C | EL_D | EL_V | EL_G);
		break;
	case CP0_EntryLo1:
		m_cp0[CP0_EntryLo1] = data & (EL_PFN | EL_C | EL_D | EL_V | EL_G);
		break;
	case CP0_Context:
		m_cp0[CP0_Context] = data & ~0xf;
		break;
	case CP0_PageMask:
		m_cp0[CP0_PageMask] = data & PAGEMASK;
		break;
	case CP0_Wired:
		m_cp0[CP0_Wired] = data & 0x3f;
		break;
	case CP0_Count:
		m_cp0[CP0_Count] = u32(data);
		m_cp0_timer_zero = total_cycles() - m_cp0[CP0_Count] * 2;

		cp0_update_timer();
		break;
	case CP0_EntryHi:
		m_cp0[CP0_EntryHi] = data & (EH_R | EH_VPN2_64 | EH_ASID);
		break;
	case CP0_Compare:
		m_cp0[CP0_Compare] = u32(data);
		CAUSE &= ~CAUSE_IPEX5;

		cp0_update_timer(true);
		break;
	case CP0_Status:
		m_cp0[CP0_Status] = u32(data) & ~u32(0x01a80000);

		if (data & SR_RE)
			fatalerror("unimplemented reverse endian mode enabled (%s)\n", machine().describe_context().c_str());
		break;
	case CP0_Cause:
		m_cp0[CP0_Cause] = (m_cp0[CP0_Cause] & ~CAUSE_IPSW) | (data & CAUSE_IPSW);
		break;
	case CP0_EPC:
		m_cp0[CP0_EPC] = data;
		break;
	case CP0_Config:
		m_cp0[CP0_Config] = (m_cp0[CP0_Config] & ~CONFIG_WM) | (data & CONFIG_WM);

		if (m_cp0[CP0_Config] & CONFIG_IB)
		{
			m_icache_line_size = 32;
			m_icache_shift = 5;
			m_icache_mask_lo = ~u32(0x1f);
		}
		else
		{
			m_icache_line_size = 16;
			m_icache_shift = 4;
			m_icache_mask_lo = ~u32(0xf);
		}

		LOGMASKED(LOG_CACHE, "icache/dcache line sizes %d/%d bytes\n",
			m_icache_line_size, m_cp0[CP0_Config] & CONFIG_DB ? 32 : 16);
		break;
	case CP0_LLAddr:
		m_cp0[CP0_LLAddr] = data;
		break;
	case CP0_WatchLo:
		m_cp0[CP0_WatchLo] = data & ~0x4;
		break;
	case CP0_WatchHi:
		m_cp0[CP0_WatchHi] = data & 0xf;
		break;
	case CP0_XContext:
		m_cp0[CP0_XContext] = data & ~0xf;
		break;
	case CP0_ECC:
		m_cp0[CP0_ECC] = data & 0xff;
		break;
	case CP0_TagLo:
		m_cp0[CP0_TagLo] = data;
		break;
	case CP0_TagHi:
		m_cp0[CP0_TagHi] = data;
		break;
	case CP0_ErrorEPC:
		m_cp0[CP0_ErrorEPC] = data;
		break;
	default:
		logerror("write to read-only or undefined cp0 register %d\n data 0x%x", reg, data);
		break;
	}
}

void r4000_base_device::cp0_tlbr()
{
	u8 const index = m_cp0[CP0_Index] & 0x3f;

	if (index < std::size(m_tlb))
	{
		tlb_entry const &entry = m_tlb[index];

		m_cp0[CP0_PageMask] = entry.mask;
		m_cp0[CP0_EntryHi] = entry.vpn;
		u64 const global = (entry.vpn & EH_G) ? EL_G : 0x0;
		m_cp0[CP0_EntryLo0] = entry.pfn[0] | global;
		m_cp0[CP0_EntryLo1] = entry.pfn[1] | global;
	}
}

void r4000_base_device::cp0_tlbwi(u8 const index)
{
	if (index < std::size(m_tlb))
	{
		tlb_entry &entry = m_tlb[index];

		entry.mask = m_cp0[CP0_PageMask];
		entry.vpn = m_cp0[CP0_EntryHi] & EH_WM;
		if ((m_cp0[CP0_EntryLo0] & EL_G) && (m_cp0[CP0_EntryLo1] & EL_G))
			entry.vpn |= EH_G;
		entry.pfn[0] = m_cp0[CP0_EntryLo0] & EL_WM;
		entry.pfn[1] = m_cp0[CP0_EntryLo1] & EL_WM;

		entry.low_bit = 32 - count_leading_zeros_32((entry.mask >> 1) | 0xfff);

		LOGMASKED(LOG_TLB, "tlb write index %02d mask 0x%016x vpn2 0x%016x %c asid 0x%02x pfn0 0x%016x %c%c pfn1 0x%016x %c%c (%s)\n",
			index, entry.mask,
			entry.vpn, entry.vpn & EH_G ? 'G' : '-', entry.vpn & EH_ASID,
			entry.pfn[0] & EL_PFN, entry.pfn[0] & EL_D ? 'D' : '-', entry.pfn[0] & EL_V ? 'V' : '-',
			entry.pfn[1] & EL_PFN, entry.pfn[1] & EL_D ? 'D' : '-', entry.pfn[1] & EL_V ? 'V' : '-',
			machine().describe_context());
	}
}

void r4000_base_device::cp0_tlbwr()
{
	u8 const wired = m_cp0[CP0_Wired] & 0x3f;
	u8 const unwired = std::size(m_tlb) - wired;

	u8 const index = (unwired > 0) ? ((total_cycles() - m_cp0_timer_zero) % unwired + wired) & 0x3f : (std::size(m_tlb) - 1);

	cp0_tlbwi(index);
}

void r4000_base_device::cp0_tlbp()
{
	m_cp0[CP0_Index] = 0x80000000;
	for (u8 index = 0; index < std::size(m_tlb); index++)
	{
		tlb_entry const &entry = m_tlb[index];

		u64 const mask = (cp0_64() ? EH_R | (EH_VPN2_64 & ~entry.mask) : (EH_VPN2_32 & ~entry.mask))
			| ((entry.vpn & EH_G) ? 0 : EH_ASID);

		if ((entry.vpn & mask) == (m_cp0[CP0_EntryHi] & mask))
		{
			m_cp0[CP0_Index] = index;
			break;
		}
	}

	if (m_cp0[CP0_Index] == 0x80000000)
		LOGMASKED(LOG_TLB, "tlbp miss 0x%08x (%s)\n", m_cp0[CP0_EntryHi], machine().describe_context());
	else
		LOGMASKED(LOG_TLB, "tlbp hit 0x%08x index %02d (%s)\n", m_cp0[CP0_EntryHi], m_cp0[CP0_Index], machine().describe_context());
}

void r4000_base_device::cp0_update_timer(bool start)
{
	if (start || m_cp0_timer->enabled())
	{
		u32 const count = (total_cycles() - m_cp0_timer_zero) / 2;
		u32 const delta = m_cp0[CP0_Compare] - count;

		m_cp0_timer->adjust(cycles_to_attotime(u64(delta) * 2));
	}
}

TIMER_CALLBACK_MEMBER(r4000_base_device::cp0_timer_callback)
{
	m_cp0[CP0_Cause] |= CAUSE_IPEX5;
}

bool r4000_base_device::cp0_64() const
{
	switch (SR & (SR_KSU | SR_ERL | SR_EXL))
	{
	case SR_KSU_U: return bool(SR & SR_UX);
	case SR_KSU_S: return bool(SR & SR_SX);

	default:
		return bool(SR & SR_KX);
	}
}

void r4000_base_device::cp1_unimplemented()
{
	m_fcr31 |= FCR31_CE;

	cpu_exception(EXCEPTION_FPE);
}

template <> bool r4000_base_device::cp1_op<float32_t>(float32_t op)
{
	// detect denormalized or quiet NaN operand
	if ((!(op.v & 0x7f800000UL) && (op.v & 0x001fffffUL)) || (op.v & 0x7fc00000UL) == 0x7fc00000UL)
	{
		cp1_unimplemented();

		return false;
	}
	else
		return true;
}

template <> bool r4000_base_device::cp1_op<float64_t>(float64_t op)
{
	// detect denormalized or quiet NaN operand
	if ((!(op.v & 0x7ff00000'00000000ULL) && (op.v & 0x000fffff'ffffffffULL)) || (op.v & 0x7ff80000'00000000ULL) == 0x7ff80000'00000000ULL)
	{
		cp1_unimplemented();

		return false;
	}
	else
		return true;
}

void r5000_device::cp1_execute(u32 const op)
{
	if (!(SR & SR_CU1))
	{
		cpu_exception(EXCEPTION_CP1);
		return;
	}

	softfloat_exceptionFlags = 0;
	switch (op >> 26)
	{
	case 0x11: // COP1
	switch ((op >> 21) & 0x1f)
	{
		case 0x10: // S
		{
			switch (op & 0x3f)
			{
			case 0x11: // MOVF.S / MOVT.S
				if (((m_fcr31 >> s_fcc_shifts[(op >> 18) & 7]) & 1) == ((op >> 16) & 1))
					cp1_mov_s(op);
				return;
			case 0x12: // MOVZ.S
				if (m_r[RTREG] == 0)
					cp1_mov_s(op);
				return;
			case 0x13: // MOVN.S
				if (m_r[RTREG] != 0)
					cp1_mov_s(op);
				return;
			case 0x15: // RECIP.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
					{
						cp1_set(FDREG, f32_div(i32_to_f32(1), fs).v);
					}
				}
				return;
			case 0x16: // RSQRT.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
					{
						cp1_set(FDREG, f32_div(i32_to_f32(1), f32_sqrt(fs)).v);
					}
				}
				return;
			case 0x30: // C.F.S (false)
				if ((SR & SR_FR) || !(op & ODD_REGS))
					m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				return;
			case 0x31: // C.UN.S (unordered)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) });
					if (softfloat_exceptionFlags & softfloat_flag_invalid)
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x32: // C.EQ.S (equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x33: // C.UEQ.S (unordered equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x34: // C.OLT.S (less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_lt(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x35: // C.ULT.S (unordered less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_lt(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x36: // C.OLE.S (less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_le(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x37: // C.ULE.S (unordered less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_le(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;

			case 0x38: // C.SF.S (signalling false)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) });

					m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x39: // C.NGLE.S (not greater, less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) });

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7] | FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x3a: // C.SEQ.S (signalling equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x3b: // C.NGL.S (not greater or less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x3c: // C.LT.S (less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_lt(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x3d: // C.NGE.S (not greater or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_lt(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x3e: // C.LE.S (less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_le(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x3f: // C.NGT.S (not greater than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_le(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			}
		}
		[[fallthrough]];
		case 0x11: // D
			switch (op & 0x3f)
			{
			case 0x11: // MOVF.D / MOVT.D
				if (((m_fcr31 >> s_fcc_shifts[(op >> 18) & 7]) & 1) == ((op >> 16) & 1))
					cp1_mov_d(op);
				return;
			case 0x12: // MOVZ.D
				if (m_r[RTREG] == 0)
					cp1_mov_d(op);
				return;
			case 0x13: // MOVN.D
				if (m_r[RTREG] != 0)
					cp1_mov_d(op);
				return;
			case 0x15: // RECIP.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
					{
						cp1_set(FDREG, f64_div(i32_to_f64(1), fs).v);
					}
				}
				return;
			case 0x16: // RSQRT.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
					{
						cp1_set(FDREG, f64_div(i32_to_f64(1), f64_sqrt(fs)).v);
					}
				}
				return;
			case 0x30: // C.F.D (false)
				if ((SR & SR_FR) || !(op & ODD_REGS))
					m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				return;
			case 0x31: // C.UN.D (unordered)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] });
					if (softfloat_exceptionFlags & softfloat_flag_invalid)
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x32: // C.EQ.D (equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x33: // C.UEQ.D (unordered equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x34: // C.OLT.D (less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_lt(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x35: // C.ULT.D (unordered less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_lt(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x36: // C.OLE.D (less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_le(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x37: // C.ULE.D (unordered less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_le(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;

			case 0x38: // C.SF.D (signalling false)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] });

					m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x39: // C.NGLE.D (not greater, less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] });

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7] | FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];
				}
				return;
			case 0x3a: // C.SEQ.D (signalling equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x3b: // C.NGL.D (not greater or less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x3c: // C.LT.D (less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_lt(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x3d: // C.NGE.D (not greater or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_lt(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x3e: // C.LE.D (less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_le(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			case 0x3f: // C.NGT.D (not greater than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_le(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= s_fcc_masks[(op >> 18) & 7];
					else
						m_fcr31 &= ~s_fcc_masks[(op >> 18) & 7];

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				return;
			}
			return;
		}
	}

	r4000_base_device::cp1_execute(op);
}

void r4000_base_device::cp1_execute(u32 const op)
{
	if (!(SR & SR_CU1))
	{
		cpu_exception(EXCEPTION_CP1);
		return;
	}

	softfloat_exceptionFlags = 0;
	switch (op >> 26)
	{
	case 0x11: // COP1
		switch ((op >> 21) & 0x1f)
		{
		case 0x00: // MFC1
			if (SR & SR_FR)
				m_r[RTREG] = s64(s32(m_f[RDREG]));
			else
				if (RDREG & 1)
					// move the high half of the even floating point register
					m_r[RTREG] = s64(s32(m_f[RDREG & ~1] >> 32));
				else
					// move the low half of the even floating point register
					m_r[RTREG] = s64(s32(m_f[RDREG & ~1] >> 0));
			break;
		case 0x01: // DMFC1
			// TODO: MIPS3 only
			if ((SR & SR_FR) || !(RDREG & 1))
				m_r[RTREG] = m_f[RDREG];
			break;
		case 0x02: // CFC1
			switch (RDREG)
			{
			case 0:  m_r[RTREG] = m_fcr0; break;
			case 30: m_r[RTREG] = m_fcr30; break;
			case 31: m_r[RTREG] = m_fcr31; break;

			default:
				logerror("cfc1 undefined fpu control register %d (%s)\n", RDREG, machine().describe_context());
				break;
			}
			break;
		case 0x04: // MTC1
			if (SR & SR_FR)
				m_f[RDREG] = (m_f[RDREG] & ~0xffffffffULL) | u32(m_r[RTREG]);
			else
				if (RDREG & 1)
					// load the high half of the even floating point register
					m_f[RDREG & ~1] = (m_r[RTREG] << 32) | u32(m_f[RDREG & ~1]);
				else
					// load the low half of the even floating point register
					m_f[RDREG & ~1] = (m_f[RDREG & ~1] & ~0xffffffffULL) | u32(m_r[RTREG]);
			break;
		case 0x05: // DMTC1
			// TODO: MIPS3 only
			if ((SR & SR_FR) || !(RDREG & 1))
				m_f[RDREG] = m_r[RTREG];
			break;
		case 0x06: // CTC1
			switch (RDREG)
			{
			case 0: // register is read-only
				break;

			case 30: // unknown
				m_fcr30 = u32(m_r[RTREG]);
				break;

			case 31:
				m_fcr31 = u32(m_r[RTREG]);

				// update rounding mode
				switch (m_fcr31 & FCR31_RM)
				{
				case 0: softfloat_roundingMode = softfloat_round_near_even; break;
				case 1: softfloat_roundingMode = softfloat_round_minMag; break;
				case 2: softfloat_roundingMode = softfloat_round_max; break;
				case 3: softfloat_roundingMode = softfloat_round_min; break;
				}

				// exception check
				if ((m_fcr31 & FCR31_CE) || ((m_fcr31 & FCR31_CM) >> 5) & (m_fcr31 & FCR31_EM))
					cpu_exception(EXCEPTION_FPE);

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
					m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
				break;
			case 0x01: // BC1T
				if (m_fcr31 & FCR31_C)
					m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
				break;
			case 0x02: // BC1FL
				if (!(m_fcr31 & FCR31_C))
					m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
				else
					m_branch_state = NULLIFY;
				break;
			case 0x03: // BC1TL
				if (m_fcr31 & FCR31_C)
					m_branch_state = ADDR(m_pc + 4, s32(s16(op)) << 2) | DELAY;
				else
					m_branch_state = NULLIFY;
				break;

			default:
				// reserved instructions
				cpu_exception(EXCEPTION_RI);
				break;
			}
			break;

		case 0x10: // S
			switch (op & 0x3f)
			{
			case 0x00: // ADD.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };
					float32_t const ft = float32_t{ u32(m_f[FTREG]) };

					if (cp1_op(fs) && cp1_op(ft))
						cp1_set(FDREG, f32_add(fs, ft).v);
				}
				break;
			case 0x01: // SUB.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };
					float32_t const ft = float32_t{ u32(m_f[FTREG]) };

					if (cp1_op(fs) && cp1_op(ft))
						cp1_set(FDREG, f32_sub(fs, ft).v);
				}
				break;
			case 0x02: // MUL.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };
					float32_t const ft = float32_t{ u32(m_f[FTREG]) };

					if (cp1_op(fs) && cp1_op(ft))
						cp1_set(FDREG, f32_mul(fs, ft).v);
				}
				break;
			case 0x03: // DIV.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };
					float32_t const ft = float32_t{ u32(m_f[FTREG]) };

					if (cp1_op(fs) && cp1_op(ft))
						cp1_set(FDREG, f32_div(fs, ft).v);
				}
				break;
			case 0x04: // SQRT.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_sqrt(fs).v);
				}
				break;
			case 0x05: // ABS.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
					{
						if (f32_lt(fs, float32_t{ 0 }))
							cp1_set(FDREG, f32_mul(fs, i32_to_f32(-1)).v);
						else
							cp1_set(FDREG, fs.v);
					}
				}
				break;
			case 0x06: // MOV.S
				cp1_mov_s(op);
				break;
			case 0x07: // NEG.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_mul(fs, i32_to_f32(-1)).v);
				}
				break;
			case 0x08: // ROUND.L.S
				// TODO: MIPS3 only
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_i64(fs, softfloat_round_near_even, true));
				}
				break;
			case 0x09: // TRUNC.L.S
				// TODO: MIPS3 only
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_i64(fs, softfloat_round_minMag, true));
				}
				break;
			case 0x0a: // CEIL.L.S
				// TODO: MIPS3 only
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_i64(fs, softfloat_round_max, true));
				}
				break;
			case 0x0b: // FLOOR.L.S
				// TODO: MIPS3 only
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_i64(fs, softfloat_round_min, true));
				}
				break;
			case 0x0c: // ROUND.W.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_i32(fs, softfloat_round_near_even, true));
				}
				break;
			case 0x0d: // TRUNC.W.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_i32(fs, softfloat_round_minMag, true));
				}
				break;
			case 0x0e: // CEIL.W.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_i32(fs, softfloat_round_max, true));
				}
				break;
			case 0x0f: // FLOOR.W.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_i32(fs, softfloat_round_min, true));
				}
				break;

			case 0x21: // CVT.D.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_f64(fs).v);
				}
				break;
			case 0x24: // CVT.W.S
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_i32(fs, softfloat_roundingMode, true));
				}
				break;
			case 0x25: // CVT.L.S
				// TODO: MIPS3 only
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float32_t const fs = float32_t{ u32(m_f[FSREG]) };

					if (cp1_op(fs))
						cp1_set(FDREG, f32_to_i64(fs, softfloat_roundingMode, true));
				}
				break;

			case 0x30: // C.F.S (false)
				if ((SR & SR_FR) || !(op & ODD_REGS))
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x31: // C.UN.S (unordered)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) });
					if (softfloat_exceptionFlags & softfloat_flag_invalid)
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x32: // C.EQ.S (equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x33: // C.UEQ.S (unordered equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x34: // C.OLT.S (less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_lt(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x35: // C.ULT.S (unordered less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_lt(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x36: // C.OLE.S (less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_le(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x37: // C.ULE.S (unordered less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_le(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;

			case 0x38: // C.SF.S (signalling false)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) });

					m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x39: // C.NGLE.S (not greater, less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) });

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_C | FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x3a: // C.SEQ.S (signalling equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x3b: // C.NGL.S (not greater or less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_eq(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x3c: // C.LT.S (less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_lt(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x3d: // C.NGE.S (not greater or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_lt(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x3e: // C.LE.S (less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_le(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x3f: // C.NGT.S (not greater than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f32_le(float32_t{ u32(m_f[FSREG]) }, float32_t{ u32(m_f[FTREG]) }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;

			default: // unimplemented operations
				cp1_unimplemented();
				break;
			}
			break;
		case 0x11: // D
			switch (op & 0x3f)
			{
			case 0x00: // ADD.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };
					float64_t const ft = float64_t{ m_f[FTREG] };

					if (cp1_op(fs) && cp1_op(ft))
						cp1_set(FDREG, f64_add(fs, ft).v);
				}
				break;
			case 0x01: // SUB.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };
					float64_t const ft = float64_t{ m_f[FTREG] };

					if (cp1_op(fs) && cp1_op(ft))
						cp1_set(FDREG, f64_sub(fs, ft).v);
				}
				break;
			case 0x02: // MUL.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };
					float64_t const ft = float64_t{ m_f[FTREG] };

					if (cp1_op(fs) && cp1_op(ft))
						cp1_set(FDREG, f64_mul(fs, ft).v);
				}
				break;
			case 0x03: // DIV.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };
					float64_t const ft = float64_t{ m_f[FTREG] };

					if (cp1_op(fs) && cp1_op(ft))
						cp1_set(FDREG, f64_div(fs, ft).v);
				}
				break;
			case 0x04: // SQRT.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_sqrt(fs).v);
				}
				break;
			case 0x05: // ABS.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
					{
						if (f64_lt(fs, float64_t{ 0 }))
							cp1_set(FDREG, f64_mul(fs, i32_to_f64(-1)).v);
						else
							cp1_set(FDREG, fs.v);
					}
				}
				break;
			case 0x06: // MOV.D
				cp1_mov_d(op);
				break;
			case 0x07: // NEG.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_mul(fs, i32_to_f64(-1)).v);
				}
				break;
			case 0x08: // ROUND.L.D
				// TODO: MIPS3 only
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_i64(fs, softfloat_round_near_even, true));
				}
				break;
			case 0x09: // TRUNC.L.D
				// TODO: MIPS3 only
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_i64(fs, softfloat_round_minMag, true));
				}
				break;
			case 0x0a: // CEIL.L.D
				// TODO: MIPS3 only
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_i64(fs, softfloat_round_max, true));
				}
				break;
			case 0x0b: // FLOOR.L.D
				// TODO: MIPS3 only
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_i64(fs, softfloat_round_min, true));
				}
				break;
			case 0x0c: // ROUND.W.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_i32(fs, softfloat_round_near_even, true));
				}
				break;
			case 0x0d: // TRUNC.W.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_i32(fs, softfloat_round_minMag, true));
				}
				break;
			case 0x0e: // CEIL.W.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_i32(fs, softfloat_round_max, true));
				}
				break;
			case 0x0f: // FLOOR.W.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_i32(fs, softfloat_round_min, true));
				}
				break;

			case 0x20: // CVT.S.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_f32(fs).v);
				}
				break;
			case 0x24: // CVT.W.D
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_i32(fs, softfloat_roundingMode, true));
				}
				break;
			case 0x25: // CVT.L.D
				// TODO: MIPS3 only
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					float64_t const fs = float64_t{ m_f[FSREG] };

					if (cp1_op(fs))
						cp1_set(FDREG, f64_to_i64(fs, softfloat_roundingMode, true));
				}
				break;

			case 0x30: // C.F.D (false)
				if ((SR & SR_FR) || !(op & ODD_REGS))
					m_fcr31 &= ~FCR31_C;
				break;
			case 0x31: // C.UN.D (unordered)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] });
					if (softfloat_exceptionFlags & softfloat_flag_invalid)
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x32: // C.EQ.D (equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x33: // C.UEQ.D (unordered equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x34: // C.OLT.D (less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_lt(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x35: // C.ULT.D (unordered less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_lt(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x36: // C.OLE.D (less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_le(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x37: // C.ULE.D (unordered less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_le(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;

			case 0x38: // C.SF.D (signalling false)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] });

					m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x39: // C.NGLE.D (not greater, less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					// detect unordered
					f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] });

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_C | FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
					else
						m_fcr31 &= ~FCR31_C;
				}
				break;
			case 0x3a: // C.SEQ.D (signalling equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x3b: // C.NGL.D (not greater or less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_eq(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x3c: // C.LT.D (less than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_lt(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x3d: // C.NGE.D (not greater or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_lt(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x3e: // C.LE.D (less than or equal)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_le(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;
			case 0x3f: // C.NGT.D (not greater than)
				if ((SR & SR_FR) || !(op & ODD_REGS))
				{
					if (f64_le(float64_t{ m_f[FSREG] }, float64_t{ m_f[FTREG] }) || (softfloat_exceptionFlags & softfloat_flag_invalid))
						m_fcr31 |= FCR31_C;
					else
						m_fcr31 &= ~FCR31_C;

					if (softfloat_exceptionFlags & softfloat_flag_invalid)
					{
						m_fcr31 |= FCR31_CV;
						cpu_exception(EXCEPTION_FPE);
					}
				}
				break;

			default: // unimplemented operations
				cp1_unimplemented();
				break;
			}
			break;
		case 0x14: // W
			switch (op & 0x3f)
			{
			case 0x20: // CVT.S.W
				if ((SR & SR_FR) || !(op & ODD_REGS))
					cp1_set(FDREG, i32_to_f32(s32(m_f[FSREG])).v);
				break;
			case 0x21: // CVT.D.W
				if ((SR & SR_FR) || !(op & ODD_REGS))
					cp1_set(FDREG, i32_to_f64(s32(m_f[FSREG])).v);
				break;

			default: // unimplemented operations
				cp1_unimplemented();
				break;
			}
			break;
		case 0x15: // L
			// TODO: MIPS3 only
			switch (op & 0x3f)
			{
			case 0x20: // CVT.S.L
				if ((SR & SR_FR) || !(op & ODD_REGS))
					cp1_set(FDREG, i64_to_f32(s64(m_f[FSREG])).v);
				break;
			case 0x21: // CVT.D.L
				if ((SR & SR_FR) || !(op & ODD_REGS))
					cp1_set(FDREG, i64_to_f64(s64(m_f[FSREG])).v);
				break;

			default: // unimplemented operations
				cp1_unimplemented();
				break;
			}
			break;

		default: // unimplemented operations
			cp1_unimplemented();
			break;
		}
		break;

	case 0x31: // LWC1
		load<u32>(ADDR(m_r[RSREG], s16(op)),
			[this, op](u32 data)
			{
				if (SR & SR_FR)
					m_f[RTREG] = (m_f[RTREG] & ~0xffffffffULL) | data;
				else
					if (RTREG & 1)
						// load the high half of the even floating point register
						m_f[RTREG & ~1] = (u64(data) << 32) | u32(m_f[RTREG & ~1]);
					else
						// load the low half of the even floating point register
						m_f[RTREG & ~1] = (m_f[RTREG & ~1] & ~0xffffffffULL) | data;
			});
		break;

	case 0x35: // LDC1
		load<u64>(ADDR(m_r[RSREG], s16(op)),
			[this, op](u64 data)
			{
				if ((SR & SR_FR) || !(RTREG & 1))
					m_f[RTREG] = data;
			});
		break;

	case 0x39: // SWC1
		if (SR & SR_FR)
			store<u32>(ADDR(m_r[RSREG], s16(op)), u32(m_f[RTREG]));
		else
			if (RTREG & 1)
				// store the high half of the even floating point register
				store<u32>(ADDR(m_r[RSREG], s16(op)), u32(m_f[RTREG & ~1] >> 32));
			else
				// store the low half of the even floating point register
				store<u32>(ADDR(m_r[RSREG], s16(op)), u32(m_f[RTREG & ~1]));
		break;

	case 0x3d: // SDC1
		store<u64>(ADDR(m_r[RSREG], s16(op)), m_f[RTREG]);
		break;
	}
}

void r4000_base_device::cp1_mov_s(u32 const op)
{
	if (SR & SR_FR)
		m_f[FDREG] = (m_f[FDREG] & ~0xffffffffULL) | u32(m_f[FSREG]);
	else
		if (FDREG & 1)
			if (FSREG & 1)
				// move high half to high half
				m_f[FDREG & ~1] = (m_f[FSREG & ~1] & ~0xffffffffULL) | u32(m_f[FDREG & ~1]);
			else
				// move low half to high half
				m_f[FDREG & ~1] = (m_f[FSREG & ~1] << 32) | u32(m_f[FDREG & ~1]);
		else
			if (FSREG & 1)
				// move high half to low half
				m_f[FDREG & ~1] = (m_f[FDREG & ~1] & ~0xffffffffULL) | (m_f[FSREG & ~1] >> 32);
			else
				// move low half to low half
				m_f[FDREG & ~1] = (m_f[FDREG & ~1] & ~0xffffffffULL) | u32(m_f[FSREG & ~1]);
}

void r4000_base_device::cp1_mov_d(u32 const op)
{
	if ((SR & SR_FR) || !(op & ODD_REGS))
		m_f[FDREG] = m_f[FSREG];
}

void r4000_base_device::cp1x_execute(u32 const op)
{
	if (!(SR & SR_CU1))
	{
		cpu_exception(EXCEPTION_CP1);
		return;
	}

	logerror("cp1x not supported < R5000 (%s)\n", machine().describe_context());
}

void r5000_device::cp1x_execute(u32 const op)
{
	if (!(SR & SR_CU1))
	{
		cpu_exception(EXCEPTION_CP1);
		return;
	}

	switch (op & 0x3f)
	{
	case 0x00: // LWXC1
		load<u32>(s64(s32(u32(m_r[RSREG]) + u32(m_r[RTREG]))),
			[this, op](u32 data)
			{
				if (SR & SR_FR)
					m_f[RTREG] = (m_f[RTREG] & ~0xffffffffULL) | data;
				else
					if (RTREG & 1)
						// load the high half of the even floating point register
						m_f[RTREG & ~1] = (u64(data) << 32) | u32(m_f[RTREG & ~1]);
					else
						// load the low half of the even floating point register
						m_f[RTREG & ~1] = (m_f[RTREG & ~1] & ~0xffffffffULL) | data;
			});
		break;
	case 0x01: // LDXC1
		load<u64>(s64(s32(u32(m_r[RSREG]) + u32(m_r[RTREG]))),
			[this, op](u64 data)
			{
				if ((SR & SR_FR) || !(RTREG & 1))
					m_f[RTREG] = data;
			});
		break;
	case 0x08: // SWXC1
		if (SR & SR_FR)
			store<u32>(s64(s32(u32(m_r[RSREG]) + u32(m_r[RTREG]))), u32(m_f[RTREG]));
		else
			if (RTREG & 1)
				// store the high half of the even floating point register
				store<u32>(s64(s32(u32(m_r[RSREG]) + u32(m_r[RTREG]))), u32(m_f[RTREG & ~1] >> 32));
			else
				// store the low half of the even floating point register
				store<u32>(s64(s32(u32(m_r[RSREG]) + u32(m_r[RTREG]))), u32(m_f[RTREG & ~1]));
		break;
	case 0x09: // SDXC1
		if ((SR & SR_FR) || !(RTREG & 1))
			store<u64>(s64(s32(u32(m_r[RSREG]) + u32(m_r[RTREG]))), m_f[RTREG]);
		break;
	case 0x0f: // PREFX
		// Do nothing for now (implementations are permitted to do this)
		break;
	case 0x20: // MADD.S
		if ((SR & SR_FR) || !(op & ODD_REGS))
		{
			float32_t const fr = float32_t{ u32(m_f[FRREG]) };
			float32_t const fs = float32_t{ u32(m_f[FSREG]) };
			float32_t const ft = float32_t{ u32(m_f[FTREG]) };

			if (cp1_op(fr) && cp1_op(fs) && cp1_op(ft))
				cp1_set(FDREG, f32_add(f32_mul(fs, ft), fr).v);
		}
		break;
	case 0x21: // MADD.D
		if ((SR & SR_FR) || !(op & ODD_REGS))
		{
			float64_t const fr = float64_t{ m_f[FRREG] };
			float64_t const fs = float64_t{ m_f[FSREG] };
			float64_t const ft = float64_t{ m_f[FTREG] };

			if (cp1_op(fr) && cp1_op(fs) && cp1_op(ft))
				cp1_set(FDREG, f64_add(f64_mul(fs, ft), fr).v);
		}
		break;
	case 0x28: // MSUB.S
		if ((SR & SR_FR) || !(op & ODD_REGS))
		{
			float32_t const fr = float32_t{ u32(m_f[FRREG]) };
			float32_t const fs = float32_t{ u32(m_f[FSREG]) };
			float32_t const ft = float32_t{ u32(m_f[FTREG]) };

			if (cp1_op(fr) && cp1_op(fs) && cp1_op(ft))
				cp1_set(FDREG, f32_sub(f32_mul(fs, ft), fr).v);
		}
		break;
	case 0x29: // MSUB.D
		if ((SR & SR_FR) || !(op & ODD_REGS))
		{
			float64_t const fr = float64_t{ m_f[FRREG] };
			float64_t const fs = float64_t{ m_f[FSREG] };
			float64_t const ft = float64_t{ m_f[FTREG] };

			if (cp1_op(fr) && cp1_op(fs) && cp1_op(ft))
				cp1_set(FDREG, f64_sub(f64_mul(fs, ft), fr).v);
		}
		break;
	case 0x30: // NMADD.S
		if ((SR & SR_FR) || !(op & ODD_REGS))
		{
			float32_t const fr = float32_t{ u32(m_f[FRREG]) };
			float32_t const fs = float32_t{ u32(m_f[FSREG]) };
			float32_t const ft = float32_t{ u32(m_f[FTREG]) };

			if (cp1_op(fr) && cp1_op(fs) && cp1_op(ft))
				cp1_set(FDREG, f32_mul(f32_add(f32_mul(fs, ft), fr), i32_to_f32(-1)).v);
		}
		break;
	case 0x31: // NMADD.D
		if ((SR & SR_FR) || !(op & ODD_REGS))
		{
			float64_t const fr = float64_t{ m_f[FRREG] };
			float64_t const fs = float64_t{ m_f[FSREG] };
			float64_t const ft = float64_t{ m_f[FTREG] };

			if (cp1_op(fr) && cp1_op(fs) && cp1_op(ft))
				cp1_set(FDREG, f64_mul(f64_add(f64_mul(fs, ft), fr), i32_to_f64(-1)).v);
		}
		break;
	case 0x38: // NMSUB.S
		if ((SR & SR_FR) || !(op & ODD_REGS))
		{
			float32_t const fr = float32_t{ u32(m_f[FRREG]) };
			float32_t const fs = float32_t{ u32(m_f[FSREG]) };
			float32_t const ft = float32_t{ u32(m_f[FTREG]) };

			if (cp1_op(fr) && cp1_op(fs) && cp1_op(ft))
				cp1_set(FDREG, f32_mul(f32_sub(f32_mul(fs, ft), fr), i32_to_f32(-1)).v);
		}
		break;
	case 0x39: // NMSUB.D
		if ((SR & SR_FR) || !(op & ODD_REGS))
		{
			float64_t const fr = float64_t{ m_f[FRREG] };
			float64_t const fs = float64_t{ m_f[FSREG] };
			float64_t const ft = float64_t{ m_f[FTREG] };

			if (cp1_op(fr) && cp1_op(fs) && cp1_op(ft))
				cp1_set(FDREG, f64_mul(f64_sub(f64_mul(fs, ft), fr), i32_to_f64(-1)).v);
		}
		break;
	case 0x24:      /* MADD.W */
		logerror("cp1x unsupported op (%s): MADD.W\n", machine().describe_context());
		break;
	case 0x25:      /* MADD.L */
		logerror("cp1x unsupported op (%s): MADD.L\n", machine().describe_context());
		break;
	case 0x2c:      /* MSUB.W */
		logerror("cp1x unsupported op (%s): MSUB.W\n", machine().describe_context());
		break;
	case 0x2d:      /* MSUB.L */
		logerror("cp1x unsupported op (%s): MSUB.L\n", machine().describe_context());
		break;
	case 0x34:      /* NMADD.W */
		logerror("cp1x unsupported op (%s): NMADD.W\n", machine().describe_context());
		break;
	case 0x35:      /* NMADD.L */
		logerror("cp1x unsupported op (%s): NMADD.L\n", machine().describe_context());
		break;
	case 0x3c:      /* NMSUB.W */
		logerror("cp1x unsupported op (%s): NMSUB.W\n", machine().describe_context());
		break;
	case 0x3d:      /* NMSUB.L */
		logerror("cp1x unsupported op (%s): NMSUB.L\n", machine().describe_context());
		break;
	default:
		logerror("cp1x unsupported op (%s): [unknown]\n", machine().describe_context());
		break;
	}
}

template <typename T> void r4000_base_device::cp1_set(unsigned const reg, T const data)
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

		// check if exception is enabled
		if (((m_fcr31 & FCR31_CM) >> 5) & (m_fcr31 & FCR31_EM))
		{
			cpu_exception(EXCEPTION_FPE);
			return;
		}

		// set flags
		m_fcr31 |= ((m_fcr31 & FCR31_CM) >> 10);
	}

	if (sizeof(T) == 4)
		m_f[reg] = (m_f[reg] & ~0xffffffffULL) | data;
	else
		m_f[reg] = data;
}

void r4000_base_device::cp2_execute(u32 const op)
{
	if (!(SR & SR_CU2))
	{
		cpu_exception(EXCEPTION_CP2);
		return;
	}

	switch (op >> 26)
	{
	case 0x12: // COP2
		switch ((op >> 21) & 0x1f)
		{
		case 0x00: // MFC2
			logerror("mfc2 unimplemented (%s)\n", machine().describe_context());
			break;
		case 0x01: // DMFC2
			// ε Operation codes marked with epsilon are valid when the
			// processor is operating either in the Kernel mode or in the
			// 64-bit non-Kernel (User or Supervisor) mode. These instructions
			// cause a reserved instruction exception if 64-bit operation is
			// not enabled in User or Supervisor mode.
			if (!(SR & SR_KSU) || (SR & (SR_EXL | SR_ERL)) || cp0_64())
				logerror("dmfc2 unimplemented (%s)\n", machine().describe_context());
			else
				cpu_exception(EXCEPTION_RI);
			break;
		case 0x02: // CFC2
			logerror("cfc2 unimplemented (%s)\n", machine().describe_context());
			break;

		case 0x04: // MTC2
			logerror("mtc2 unimplemented (%s)\n", machine().describe_context());
			break;
		case 0x05: // DMTC2
			// ε Operation codes marked with epsilon are valid when the
			// processor is operating either in the Kernel mode or in the
			// 64-bit non-Kernel (User or Supervisor) mode. These instructions
			// cause a reserved instruction exception if 64-bit operation is
			// not enabled in User or Supervisor mode.
			if (!(SR & SR_KSU) || (SR & (SR_EXL | SR_ERL)) || cp0_64())
				logerror("dmtc2 unimplemented (%s)\n", machine().describe_context());
			else
				cpu_exception(EXCEPTION_RI);
			break;
		case 0x06: // CTC2
			logerror("ctc2 unimplemented (%s)\n", machine().describe_context());
			break;

		case 0x08: // BC2
			switch ((op >> 16) & 0x1f)
			{
			case 0x00: // BC2F
			case 0x01: // BC2F
			case 0x02: // BC2FL
			case 0x03: // BC2TL
				logerror("bc2 unimplemented (%s)\n", machine().describe_context());
				break;

			default:
				// γ Operation codes marked with a gamma cause a reserved
				// instruction exception. They are reserved for future versions
				// of the architecture.
				cpu_exception(EXCEPTION_RI);
				break;
			}
			break;

		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			// CP2 function
			logerror("function unimplemented (%s)\n", machine().describe_context());
			break;

		default:
			// γ Operation codes marked with a gamma cause a reserved
			// instruction exception. They are reserved for future versions
			// of the architecture.
			cpu_exception(EXCEPTION_RI);
			break;
		}
		break;

	case 0x32: // LWC2
		logerror("lwc2 unimplemented (%s)\n", machine().describe_context());
		break;

	case 0x36: // LDC2
		logerror("ldc2 unimplemented (%s)\n", machine().describe_context());
		break;

	case 0x3a: // SWC2
		logerror("swc2 unimplemented (%s)\n", machine().describe_context());
		break;

	case 0x3e: // SDC2
		logerror("sdc2 unimplemented (%s)\n", machine().describe_context());
		break;
	}
}

r4000_base_device::translate_result r4000_base_device::translate(int intention, u64 &address)
{
	/*
	 * Decode the program address into one of the following ranges depending on
	 * the active status register bits.
	 *
	 * 32-bit modes
	 * user:   0x0000'0000-0x7fff'ffff (useg, mapped)
	 *
	 * super:  0x0000'0000-0x7fff'ffff (suseg, mapped)
	 *         0xc000'0000-0xdfff'ffff (ssseg, mapped)
	 *
	 * kernel: 0x0000'0000-0x7fff'ffff (kuseg, mapped)
	 *         0x8000'0000-0x9fff'ffff (kseg0, unmapped, cached)
	 *         0xa000'0000-0xbfff'ffff (kseg1, unmapped, uncached)
	 *         0xc000'0000-0xdfff'ffff (ksseg, mapped)
	 *         0xe000'0000-0xffff'ffff (kseg3, mapped)
	 *
	 * 64-bit modes
	 * user:   0x0000'0000'0000'0000-0x0000'00ff'ffff'ffff (xuseg, mapped)
	 *
	 * super:  0x0000'0000'0000'0000-0x0000'00ff'ffff'ffff (xsuseg, mapped)
	 *         0x4000'0000'0000'0000-0x4000'00ff'ffff'ffff (xsseg, mapped)
	 *         0xffff'ffff'c000'0000-0xffff'ffff'dfff'ffff (csseg, mapped)
	 *
	 * kernel: 0x0000'0000'0000'0000-0x0000'00ff'ffff'ffff (xkuseg, mapped)
	 *         0x4000'0000'0000'0000-0x4000'00ff'ffff'ffff (xksseg, mapped)
	 *         0x8000'0000'0000'0000-0xbfff'ffff'ffff'ffff (xkphys, unmapped)
	 *         0xc000'0000'0000'0000-0xc000'00ff'7fff'ffff (xkseg, mapped)
	 *         0xffff'ffff'8000'0000-0xffff'ffff'9fff'ffff (ckseg0, unmapped, cached)
	 *         0xffff'ffff'a000'0000-0xffff'ffff'bfff'ffff (ckseg1, unmapped, uncached)
	 *         0xffff'ffff'c000'0000-0xffff'ffff'dfff'ffff (cksseg, mapped)
	 *         0xffff'ffff'e000'0000-0xffff'ffff'ffff'ffff (ckseg3, mapped)
	 */

	bool extended = false;

	switch (SR & (SR_KSU | SR_ERL | SR_EXL))
	{
	case SR_KSU_U:
		// user mode
		if (SR & SR_UX)
		{
			// 64-bit user mode
			if (address & 0xffff'ff00'0000'0000)
				return ERROR; // exception
			else
				extended = true; // xuseg
		}
		else
		{
			// 32-bit user mode
			if (address & 0x8000'0000)
				return ERROR; // exception
			else
				extended = false; // useg
		}
		break;

	case SR_KSU_S:
		// supervisor mode
		if (SR & SR_SX)
		{
			// 64-bit supervisor mode
			if (address & 0xffff'ff00'0000'0000)
				if ((address & 0xffff'ff00'0000'0000) == 0x4000'0000'0000'0000)
					extended = true; // xsseg
				else
					if ((address & 0xffff'ffff'e000'0000) == 0xffff'ffff'c000'0000)
						extended = true; // csseg
					else
						return ERROR; // exception
			else
				extended = true; // xsuseg
		}
		else
		{
			// 32-bit supervisor mode
			if (address & 0x8000'0000)
				if ((address & 0xe000'0000) == 0xc000'0000)
					extended = false; // sseg
				else
					return ERROR; // exception
			else
				extended = false; // suseg
		}
		break;

	case SR_KSU_U | SR_KSU_S:
		fatalerror("invalid ksu bits 0x%08x (%s)\n", u32(SR), machine().describe_context().c_str());
		break;

	default:
		// kernel mode
		if (SR & SR_KX)
		{
			// 64-bit kernel mode
			if (address & 0xffff'ff00'0000'0000)
				if ((address & 0xffff'ff00'0000'0000) == 0x4000'0000'0000'0000)
					extended = true; // xksseg
				else
					if ((address & 0xc000'0000'0000'0000) == 0x8000'0000'0000'0000)
					{
						address &= 0x0000'000f'ffff'ffff; // xkphys

						// FIXME: caching depends on top three bits
						return CACHED;
					}
					else
						if ((address & 0xffff'ff00'0000'0000) == 0xc000'0000'0000'0000)
							if ((address & 0x0000'00ff'8000'0000) == 0x0000'00ff'8000'0000)
								return ERROR; // exception
							else
								extended = true; // xkseg
						else
							// FIXME: ckseg0 caching depends on config regiter
							switch (address & 0xffff'ffff'e000'0000)
							{
							case 0xffff'ffff'8000'0000: address &= 0x7fff'ffff; return CACHED;   // ckseg0
							case 0xffff'ffff'a000'0000: address &= 0x1fff'ffff; return UNCACHED; // ckseg1
							case 0xffff'ffff'c000'0000: extended = true; break; // cksseg
							case 0xffff'ffff'e000'0000: extended = true; break; // ckseg3
							default: return ERROR; // exception
							}
			else
				if (SR & SR_ERL)
					// FIXME: documentation says 2^31, but assume it should be 2^40
					return UNCACHED; // xkuseg (unmapped, uncached)
				else
					extended = true; // xkuseg
		}
		else
		{
			// 32-bit kernel mode
			if (address & 0x8000'0000)
				switch (address & 0xe000'0000)
				{
				case 0x8000'0000: address &= 0x7fff'ffff; return CACHED;   // kseg0
				case 0xa000'0000: address &= 0x1fff'ffff; return UNCACHED; // kseg1
				case 0xc000'0000: extended = false; break; // ksseg
				case 0xe000'0000: extended = false; break; // kseg3
				}
			else
				if (SR & SR_ERL)
					return UNCACHED; // kuseg (unmapped, uncached)
				else
					extended = false; // kuseg
		}
		break;
	}

	// address needs translation, using a combination of VPN2 and ASID
	u64 const key = (address & (extended ? (EH_R | EH_VPN2_64) : EH_VPN2_32)) | (m_cp0[CP0_EntryHi] & EH_ASID);

	unsigned *mru = m_tlb_mru[intention & TRANSLATE_TYPE_MASK];
	if (VERBOSE & LOG_STATS)
		m_tlb_scans++;

	bool invalid = false;
	bool modify = false;
	for (unsigned i = 0; i < std::size(m_tlb); i++)
	{
		unsigned const index = mru[i];
		tlb_entry const &entry = m_tlb[index];

		// test vpn and asid
		u64 const mask = (extended ? EH_R | (EH_VPN2_64 & ~entry.mask) : (EH_VPN2_32 & ~entry.mask))
			| ((entry.vpn & EH_G) ? 0 : EH_ASID);

		if ((entry.vpn & mask) != (key & mask))
			continue;

		if (VERBOSE & LOG_STATS)
			m_tlb_loops += i + 1;

		u64 const pfn = entry.pfn[BIT(address, entry.low_bit)];

		// test valid
		if (!(pfn & EL_V))
		{
			invalid = true;
			break;
		}

		// test dirty
		if ((intention & TRANSLATE_WRITE) && !(pfn & EL_D))
		{
			modify = true;
			break;
		}

		// translate the address
		address &= (entry.mask >> 1) | 0xfff;
		address |= ((pfn & EL_PFN) << 6) & ~(entry.mask >> 1);

		// promote the entry in the mru index
		if (i > 0)
			std::swap(mru[i - 1], mru[i]);

		return ((pfn & EL_C) == C_2) ? UNCACHED : CACHED;
	}

	// tlb miss, invalid entry, or a store to a non-dirty entry
	if (!machine().side_effects_disabled() && !(intention & TRANSLATE_DEBUG_MASK))
	{
		if (VERBOSE & LOG_TLB)
		{
			static char const mode[] = { 'r', 'w', 'x' };

			if (modify)
				LOGMASKED(LOG_TLB, "tlb modify asid 0x%02x address 0x%016x (%s)\n",
					m_cp0[CP0_EntryHi] & EH_ASID, address, machine().describe_context());
			else
				LOGMASKED(LOG_TLB, "tlb miss %c asid 0x%02x address 0x%016x (%s)\n",
					mode[intention & TRANSLATE_TYPE_MASK], m_cp0[CP0_EntryHi] & EH_ASID, address, machine().describe_context());
		}

		// load tlb exception registers
		m_cp0[CP0_BadVAddr] = address;
		m_cp0[CP0_EntryHi] = key;
		m_cp0[CP0_Context] = (m_cp0[CP0_Context] & CONTEXT_PTEBASE) | ((address >> 9) & CONTEXT_BADVPN2);
		m_cp0[CP0_XContext] = (m_cp0[CP0_XContext] & XCONTEXT_PTEBASE) | ((address >> 31) & XCONTEXT_R) | ((address >> 9) & XCONTEXT_BADVPN2);

		if (invalid || modify || (SR & SR_EXL))
			cpu_exception(modify ? EXCEPTION_MOD : (intention & TRANSLATE_WRITE) ? EXCEPTION_TLBS : EXCEPTION_TLBL);
		else
			cpu_exception((intention & TRANSLATE_WRITE) ? EXCEPTION_TLBS : EXCEPTION_TLBL, extended ? 0x080 : 0x000);
	}

	return MISS;
}

void r4000_base_device::address_error(int intention, u64 const address)
{
	if (!machine().side_effects_disabled() && !(intention & TRANSLATE_DEBUG_MASK))
	{
		logerror("address_error 0x%016x (%s)\n", address, machine().describe_context());

		// TODO: check this
		if (!(SR & SR_EXL))
			m_cp0[CP0_BadVAddr] = address;

		cpu_exception((intention & TRANSLATE_WRITE) ? EXCEPTION_ADES : EXCEPTION_ADEL);

		// address errors shouldn't typically occur, so a breakpoint is handy
		machine().debug_break();
	}
}

template <typename T> void r4000_base_device::accessors(T &m)
{
	space(AS_PROGRAM).cache(m);

	read_byte = [&m](offs_t offset) { return m.read_byte(offset); };
	read_word = [&m](offs_t offset) { return m.read_word(offset); };
	read_dword = [&m](offs_t offset) { return m.read_dword(offset); };
	read_qword = [&m](offs_t offset) { return m.read_qword(offset); };

	write_byte = [&m](offs_t offset, u8 data) { m.write_byte(offset, data); };
	write_word = [&m](offs_t offset, u16 data, u16 mem_mask) { m.write_word(offset, data, mem_mask); };
	write_dword = [&m](offs_t offset, u32 data, u32 mem_mask) { m.write_dword(offset, data, mem_mask); };
	write_qword = [&m](offs_t offset, u64 data, u64 mem_mask) { m.write_qword(offset, data, mem_mask); };
}

template <typename T, bool Aligned, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(T)>>::value, bool> r4000_base_device::load(u64 address, U &&apply)
{
	// alignment error
	if (Aligned && (address & (sizeof(T) - 1)))
	{
		address_error(TRANSLATE_READ, address);
		return false;
	}

	translate_result const t = translate(TRANSLATE_READ, address);

	// address error
	if (t == ERROR)
	{
		address_error(TRANSLATE_READ, address);

		return false;
	}

	// tlb miss
	if (t == MISS)
		return false;

	// watchpoint
	if ((m_cp0[CP0_WatchLo] & WATCHLO_R) && !(SR & SR_EXL))
	{
		u64 const watch_address = ((m_cp0[CP0_WatchHi] & WATCHHI_PADDR1) << 32) | (m_cp0[CP0_WatchLo] & WATCHLO_PADDR0);

		if ((address & ~7) == watch_address)
		{
			cpu_exception(EXCEPTION_WATCH);
			return false;
		}
	}

	// align address for l[dw][lr] instructions
	if (!Aligned)
		address &= ~(sizeof(T) - 1);

	// TODO: cache lookup

	T value = 0;
	switch (sizeof(T))
	{
	case 1: value = T(read_byte(address)); break;
	case 2: value = T(read_word(address)); break;
	case 4: value = T(read_dword(address)); break;
	case 8: value = T(read_qword(address)); break;
	}

	if (m_bus_error)
	{
		m_bus_error = false;
		cpu_exception(EXCEPTION_DBE);
	}
	else
	{
		apply(value);
	}

	return true;
}

template <typename T, typename U> std::enable_if_t<std::is_convertible<U, std::function<void(u64, T)>>::value, bool> r4000_base_device::load_linked(u64 address, U &&apply)
{
	// alignment error
	if (address & (sizeof(T) - 1))
	{
		address_error(TRANSLATE_READ, address);
		return false;
	}

	translate_result const t = translate(TRANSLATE_READ, address);

	// address error
	if (t == ERROR)
	{
		address_error(TRANSLATE_READ, address);
		return false;
	}

	// tlb miss
	if (t == MISS)
		return false;

	// watchpoint
	if ((m_cp0[CP0_WatchLo] & WATCHLO_R) && !(SR & SR_EXL))
	{
		u64 const watch_address = ((m_cp0[CP0_WatchHi] & WATCHHI_PADDR1) << 32) | (m_cp0[CP0_WatchLo] & WATCHLO_PADDR0);

		if ((address & ~7) == watch_address)
		{
			cpu_exception(EXCEPTION_WATCH);
			return false;
		}
	}

	// TODO: cache lookup

	switch (sizeof(T))
	{
	case 4: apply(address, T(read_dword(address))); break;
	case 8: apply(address, T(read_qword(address))); break;
	}

	return true;
}

template <typename T, bool Aligned, typename U> std::enable_if_t<std::is_convertible<U, T>::value, bool> r4000_base_device::store(u64 address, U data, T mem_mask)
{
	// alignment error
	if (Aligned && (address & (sizeof(T) - 1)))
	{
		address_error(TRANSLATE_WRITE, address);
		return false;
	}

	translate_result const t = translate(TRANSLATE_WRITE, address);

	// address error
	if (t == ERROR)
	{
		address_error(TRANSLATE_WRITE, address);
		return false;
	}

	// tlb miss
	if (t == MISS)
		return false;

	// watchpoint
	if ((m_cp0[CP0_WatchLo] & WATCHLO_W) && !(SR & SR_EXL))
	{
		u64 const watch_address = ((m_cp0[CP0_WatchHi] & WATCHHI_PADDR1) << 32) | (m_cp0[CP0_WatchLo] & WATCHLO_PADDR0);

		if ((address & ~7) == watch_address)
		{
			cpu_exception(EXCEPTION_WATCH);
			return false;
		}
	}

	// align address for s[dw][lr] instructions
	if (!Aligned)
		address &= ~(sizeof(T) - 1);

	// TODO: cache lookup

	switch (sizeof(T))
	{
	case 1: write_byte(address, T(data)); break;
	case 2: write_word(address, T(data), mem_mask); break;
	case 4: write_dword(address, T(data), mem_mask); break;
	case 8: write_qword(address, T(data), mem_mask); break;
	}

	return true;
}

bool r4000_base_device::fetch(u64 address, std::function<void(u32)> &&apply)
{
	u64 const program_address = address;

	// alignment error
	if (address & 3)
	{
		address_error(TRANSLATE_FETCH, address);

		return false;
	}

	translate_result const t = translate(TRANSLATE_FETCH, address);

	// address error
	if (t == ERROR)
	{
		address_error(TRANSLATE_FETCH, address);

		return false;
	}

	// tlb miss
	if (t == MISS)
		return false;

	if (ICACHE)
	{
		if (t == UNCACHED)
		{
			const u32 insn = read_dword(address);

			if (m_bus_error)
			{
				m_bus_error = false;
				cpu_exception(EXCEPTION_IBE);
			}
			else
			{
				apply(insn);
			}

			return true;
		}

		// look up the tag
		u32 const cache_address = (program_address & m_icache_mask_hi);
		u32 &tag = m_icache_tag[cache_address >> m_icache_shift];

		// check for cache miss
		if (!(tag & ICACHE_V) || (tag & ICACHE_PTAG) != (address >> 12))
		{
			// cache miss
			m_icache_misses++;

			// reload the cache line
			tag = ICACHE_V | (address >> 12);
			for (unsigned i = 0; i < m_icache_line_size; i += 8)
			{
				u64 const data = read_qword((address & m_icache_mask_lo) | i);

				m_icache_data[(((cache_address & m_icache_mask_lo) | i) >> 2) + 0] = u32(data);
				m_icache_data[(((cache_address & m_icache_mask_lo) | i) >> 2) + 1] = data >> 32;
			}
		}
		else
			m_icache_hits++;

		// apply the result
		apply(m_icache_data[cache_address >> 2]);
	}
	else
	{
		const u32 insn = read_dword(address);

		if (m_bus_error)
		{
			m_bus_error = false;
			cpu_exception(EXCEPTION_IBE);
		}
		else
		{
			apply(insn);
		}
	}

	return true;
}

std::string r4000_base_device::debug_string(u64 string_pointer, unsigned limit)
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

std::string r4000_base_device::debug_string_array(u64 array_pointer)
{
	auto const suppressor(machine().disable_side_effects());

	bool done = false;
	std::string result("");

	while (!done)
	{
		done = true;
		load<u32>(array_pointer, [this, &done, &result](s32 string_pointer)
		{
			if (string_pointer != 0)
			{
				if (!result.empty())
					result += ", ";

				result += '\"' + debug_string(s64(string_pointer)) + '\"';

				done = false;
			}
		});

		array_pointer += 4;
	}

	return result;
}

std::string r4000_base_device::debug_unicode_string(u64 unicode_string_pointer)
{
	auto const suppressor(machine().disable_side_effects());

	std::wstring result(L"");

	if (!load<u16>(unicode_string_pointer,
		[this, unicode_string_pointer, &result](u16 const length)
		{
			if (length)
				if (!load<u32>(unicode_string_pointer + 4,
					[this, length, &result](s32 buffer)
					{
						for (int i = 0; i < length; i += 2)
							load<u16>(buffer + i, [&result](wchar_t const character) { result += character; });
					}))
					result.assign(L"[unmapped]");
		}))
		result.assign(L"[unmapped]");

	return utf8_from_wstring(result);
}
