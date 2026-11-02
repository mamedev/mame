// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    ARM7 dynamic recompiler
    Emulation by R. Belmont
    Based in part on ppcdrc.cpp by Aaron Giles and R. Belmont and the SH DRC by R. Belmont
*/

#include "emu.h"
#include "arm7.h"

#include "arm7core.h"
#include "arm7fe.h"
#include "arm7help.h"

#include "cpu/drcumlsh.h"

#include <algorithm>
#include <bit>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


using namespace uml;


/***************************************************************************
    CONSTANTS
***************************************************************************/

namespace {

// compilation boundaries: how far back and forward the analysis extends
constexpr uint32_t COMPILE_BACKWARDS_BYTES = 128;
constexpr uint32_t COMPILE_FORWARDS_BYTES = 512;
constexpr uint32_t COMPILE_MAX_SEQUENCE = 64;

// exit codes
enum : uint32_t
{
	EXECUTE_OUT_OF_CYCLES = 0,
	EXECUTE_MISSING_CODE,
	EXECUTE_RESET_CACHE
};

// hash modes: register bank x T x 26-bit, plus DRC_MODE_HALF and DRC_MODE_EXACT
constexpr uint32_t DRC_MODES = 128;

// a slice with no more cycles than this enters the exact copies directly
constexpr int DRC_EXACT_ENTRY_CYCLES = 16;

// word check failures at one address that make it volatile code
constexpr uint32_t DRC_VOLATILE_MISSES = 4;

// register names for the logs
constexpr const char *REG_NAMES[] =
{
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "cpsr",
	"r8_fiq", "r9_fiq", "r10_fiq", "r11_fiq", "r12_fiq", "r13_fiq", "r14_fiq", "spsr_fiq",
	"r13_irq", "r14_irq", "spsr_irq", "r13_svc", "r14_svc", "spsr_svc",
	"r13_abt", "r14_abt", "spsr_abt", "r13_und", "r14_und", "spsr_und"
};

inline void alloc_handle(drcuml_state &drcuml, code_handle *&handleptr, const char *name)
{
	if (!handleptr)
	{
		handleptr = drcuml.handle_alloc(name);
	}
}

// a single opcode for the disassembler when logging
class opcode_buffer : public util::disasm_interface::data_buffer
{
public:
	opcode_buffer(uint32_t op) : m_op(op) { }

	virtual u8 r8(offs_t pc) const override { return m_op >> (8 * (pc & 3)); }
	virtual u16 r16(offs_t pc) const override { return m_op; }
	virtual u32 r32(offs_t pc) const override { return m_op; }
	virtual u64 r64(offs_t pc) const override { return m_op; }

private:
	const uint32_t m_op;
};

} // anonymous namespace


/***************************************************************************
    CORE CALLBACKS
***************************************************************************/

// allocates the cache and creates the back-end; the interpreter needs the cache too, for the register file
void arm7_cpu_device::drc_initialize()
{
	// the hash tables are word-granular: a Thumb block at 4n+2 has its own mode
	m_drcuml.initialize(0, DRC_MODES, 32, 2, COMPILE_FORWARDS_BYTES);
}


void arm7_cpu_device::drc_start()
{
	static_assert(std::size(REG_NAMES) == NUM_REGS);
	for (int regnum = 0; regnum < NUM_REGS; regnum++)
	{
		m_drcuml.symbol_add(&m_r[regnum], sizeof(m_r[regnum]), REG_NAMES[regnum]);
	}
	m_drcuml.symbol_add(&m_core->icount, sizeof(m_core->icount), "icount");
	m_drcuml.symbol_add(&m_core->mode, sizeof(m_core->mode), "mode");
	m_drcuml.symbol_add(&m_core->arg0, sizeof(m_core->arg0), "arg0");
	m_drcuml.symbol_add(&m_core->arg1, sizeof(m_core->arg1), "arg1");
	m_drcuml.symbol_add(&m_core->result, sizeof(m_core->result), "result");
	m_drcuml.symbol_add(&m_core->exact_miss, sizeof(m_core->exact_miss), "exact_miss");
	m_drcuml.symbol_add(&m_core->stop, sizeof(m_core->stop), "stop");

	// a change to the address map (views, bank installs) invalidates all compiled code
	m_drc_map_subscription = m_program->add_change_notifier([this] (read_or_write) { drc_remapped(); });
	m_drcuml.symbol_add(&m_control, sizeof(m_control), "control");
	m_drcuml.symbol_add(&m_pid_offset, sizeof(m_pid_offset), "pid_offset");
	m_drcuml.symbol_add(&m_pending_interrupt, sizeof(m_pending_interrupt), "pending");

	// under the debugger, one instruction per sequence with no look-ahead, so fetches match the interpreter
	if (debugger_enabled())
	{
		m_drcfe = std::make_unique<frontend>(*this, 0, 4, 1);
	}
	else
	{
		m_drcfe = std::make_unique<frontend>(*this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, COMPILE_MAX_SEQUENCE);
	}

	m_drc_generate = DRC_GENERATE_DEFAULT;
	m_drc_opt = DRC_OPT_DEFAULT;

	static_assert(DRC_TLB_SIZE >= 2 && DRC_TLB_SIZE <= 1024 && std::has_single_bit(DRC_TLB_SIZE));
	m_drc_tlb = m_drcuml.cache_alloc_near<uint64_t []>(4 * m_drc_tlb_size);
	if (!m_drc_tlb)
	{
		fatalerror("%s: no room for the data TLB in the recompiler cache's near area\n", tag());
	}
	drc_tlb_flush();
}


void arm7_cpu_device::device_stop()
{
	if (DRC_TLB_CHECK)
	{
		logerror("data TLB check: %u hits checked, %u of them walked (the vtlb had dropped the page), %u fills, %u misses\n",
				m_core->tlb_stats[0], m_core->tlb_stats[1], m_core->tlb_stats[2], m_core->tlb_stats[3]);
	}
	m_drcfe.reset();
}


void arm7_cpu_device::execute_run_drc()
{
	// the words prefetched before a change to the map run first, from the interpreter's buffer
	if (m_drc_prefetched)
	{
		m_drc_prefetched = false;
		execute_run_interpreter();
		return;
	}

	drc_r15_from_interp();
	if (DRC_TLB_FLUSH_SLICES)
	{
		drc_tlb_flush();
	}
	if (m_drc_cache_dirty || (m_core->stop & STOP_REMAP))
	{
		code_flush_cache();
		m_drc_cache_dirty = false;
		m_core->stop = 0;
	}

	m_tlb_log = m_actual_log;

	int result;
	do
	{
		result = m_drcuml.execute((m_core->icount <= DRC_EXACT_ENTRY_CYCLES) ? *m_entry_exact : *m_entry);

		if (result == EXECUTE_MISSING_CODE)
		{
			const uint8_t mode = m_core->mode | (m_core->exact_miss ? DRC_MODE_EXACT : 0);
			m_core->exact_miss = 0;
			if (m_core->verify_miss)
			{
				// a word that keeps changing (BBC BASIC's SYS stub) is fetched and interpreted as it runs
				m_core->verify_miss = 0;
				if (++m_drc_verify_misses[m_r[eR15]] == DRC_VOLATILE_MISSES)
				{
					m_drc_volatile.insert(m_r[eR15]);
					code_flush_cache();
				}
			}
			if (m_core->literal_miss)
			{
				// a folded literal changed: compile that load as a real load from now on
				m_core->literal_miss = 0;
				m_drc_nofold.insert(m_r[eR15]);
				code_flush_cache();
			}
			// nothing to compile (the last word of the address space): the interpreter takes the slice
			if (!code_compile_block(mode, m_r[eR15]))
			{
				drc_r15_to_interp();
				interpret_slice();
				return;
			}
			if (m_core->icount <= 0)
			{
				break;      // the cycles charged on the way out ran the slice out
			}
		}
		else if (result == EXECUTE_RESET_CACHE)
		{
			// this CPU remapped: if its prefetched words differ now, the interpreter runs the rest of the slice
			const bool prefetched = drc_prefetch_after_remap();
			code_flush_cache();
			m_drc_cache_dirty = false;
			m_core->stop = 0;
			if (prefetched)
			{
				if (m_core->icount <= 0)
				{
					m_drc_prefetched = true;    // the next slice starts with them
					break;
				}
				drc_r15_to_interp();
				execute_run_interpreter();
				return;
			}
			if (m_core->icount <= 0)
			{
				break;
			}
		}
	} while (result != EXECUTE_OUT_OF_CYCLES);

	drc_r15_to_interp();
	m_tlb_log = 0;
}


// true when the words prefetched before a remap differ from the map now and were queued to run
bool arm7_cpu_device::drc_prefetch_after_remap()
{
	const bool next = m_core->resync_next != 0;
	m_core->resync_next = 0;
	if (!next || m_insn_prefetch_depth != 3)
	{
		return false;
	}
	const auto found = m_drc_prefetch.find(m_r[eR15]);
	if (found == m_drc_prefetch.end())
	{
		return false;
	}
	const offs_t pc = m_r[eR15];
	const auto [word0, word1] = found->second;

	// a word behind a handler isn't read (the read could have side effects) and counts as different
	const auto same = [this] (offs_t address, uint32_t word)
	{
		offs_t physical = address;
		if ((m_control & COPRO_CTRL_MMU_EN) && !translate_vaddr_to_paddr(physical, 0))
		{
			return false;
		}
		const void *const host = insn_fetch_ptr(physical);
		return host && *reinterpret_cast<const uint32_t *>(host) == word;
	};
	if (same(pc, word0) && same(pc + 4, word1))
	{
		return false;
	}

	m_insn_prefetch_index = 0;
	m_insn_prefetch_count = 2;
	m_insn_prefetch_buffer[0] = word0;
	m_insn_prefetch_address[0] = pc;
	m_insn_prefetch_valid[0] = true;
	m_insn_prefetch_buffer[1] = word1;
	m_insn_prefetch_address[1] = pc + 4;
	m_insn_prefetch_valid[1] = true;
	return true;
}


void arm7_cpu_device::arm7drc_set_options(uint32_t options)
{
	m_drcoptions = options;
}


void arm7_cpu_device::arm7drc_add_fastram(offs_t start, offs_t end, bool readonly, void *base)
{
	if (m_fastram_select >= std::size(m_fastram))
	{
		throw emu_fatalerror("%s: more than %u fast RAM regions\n", tag(), std::size(m_fastram));
	}
	m_fastram[m_fastram_select].start = start;
	m_fastram[m_fastram_select].end = end;
	m_fastram[m_fastram_select].readonly = readonly;
	m_fastram[m_fastram_select].base = base;
	m_fastram_select++;
	m_drc_cache_dirty = true;
	m_drc_fastram_stale = true;
}


void arm7_cpu_device::arm7drc_add_hotspot(offs_t pc, uint32_t opcode, uint32_t cycles)
{
	if (m_drc_hotspot_select >= std::size(m_drc_hotspot))
	{
		throw emu_fatalerror("%s: more than %u hotspots\n", tag(), std::size(m_drc_hotspot));
	}
	m_drc_hotspot[m_drc_hotspot_select].pc = pc;
	m_drc_hotspot[m_drc_hotspot_select].opcode = opcode;
	m_drc_hotspot[m_drc_hotspot_select].cycles = cycles;
	m_drc_hotspot_select++;
}



/***************************************************************************
    C FUNCTION CALLBACKS
***************************************************************************/

// the address map or a TCM changed: flush the cache at the next slice, or right after the access
void arm7_cpu_device::drc_remapped()
{
	m_drc_cache_dirty = true;
	m_drc_fastram_stale = true;
	m_core->stop |= STOP_REMAP;
	drc_tlb_flush();    // at once: the rest of the instruction that changed the map must miss
}


// a fast RAM region with a tap or another handler installed over it is accessed through the bus
void arm7_cpu_device::drc_check_fastram()
{
	// dumping the map is slow, and MMU systems flush the cache often
	if (!m_drc_fastram_stale || !m_fastram_select)
	{
		return;
	}
	m_drc_fastram_stale = false;

	std::vector<memory_entry> reads, writes;
	m_program->dump_maps(reads, writes);
	const auto direct = [this] (const std::vector<memory_entry> &map, const fast_ram_info &ram, bool write)
	{
		for (const memory_entry &entry : map)
		{
			if (entry.end < ram.start || entry.start > ram.end)
			{
				continue;
			}
			if (entry.entry->is_passthrough() || entry.entry->is_units())
			{
				return false;
			}

			// the pointer is to the 32-bit unit holding the address
			const offs_t address = std::max(entry.start, ram.start);
			const void *const ptr = write ? m_program->get_write_ptr(address) : m_program->get_read_ptr(address);
			if (ptr != reinterpret_cast<const uint8_t *>(ram.base) + ((address - ram.start) & ~offs_t(3)))
			{
				return false;
			}
		}
		return true;
	};

	for (uint32_t ramnum = 0; ramnum < m_fastram_select; ramnum++)
	{
		fast_ram_info &ram = m_fastram[ramnum];
		ram.direct = (ram.base != nullptr) && direct(reads, ram, false) && (ram.readonly || direct(writes, ram, true));
	}
}

// data TLB: four tables (read/write x privileged/user) of (addend << 32) | page; host = base + u32(va + addend)
void arm7_cpu_device::drc_tlb_choose_base()
{
	m_drc_tlb_base = nullptr;
	if (!m_drc_tlb || !(m_drc_opt & DRC_OPT_TLB) || !(m_drc_opt & DRC_OPT_FASTRAM) || debugger_enabled() || !drc_memory_fast_path())
	{
		return;
	}

	// the lowest region, page-aligned so that offsets keep the host address's alignment
	uintptr_t lowest = ~uintptr_t(0);
	for (uint32_t ramnum = 0; ramnum < m_fastram_select; ramnum++)
	{
		if (m_fastram[ramnum].direct)
		{
			lowest = std::min(lowest, uintptr_t(m_fastram[ramnum].base));
		}
	}
	for (const tcm_info &tcm : m_drc_tcm)
	{
		if (tcm.data != nullptr)
		{
			lowest = std::min(lowest, uintptr_t(tcm.data));
		}
	}
	if (lowest != ~uintptr_t(0))
	{
		m_drc_tlb_base = reinterpret_cast<uint8_t *>(lowest & ~uintptr_t(0xfff));
	}
}

uint32_t arm7_cpu_device::drc_lane_xor(int size) const
{
	return ((m_endian == ENDIANNESS_BIG) == (std::endian::native == std::endian::little)) ? ((4 - size) & 3) : 0;
}

bool arm7_cpu_device::drc_tlb_fits(const void *host, uint32_t bytes) const
{
	const uintptr_t start = uintptr_t(host);
	const uintptr_t base = uintptr_t(m_drc_tlb_base);
	return m_drc_tlb_base != nullptr && !(start & 3) && start >= base && (start - base + bytes) <= 0x7fffffff;
}

void arm7_cpu_device::drc_tlb_flush()
{
	if (m_drc_tlb)
	{
		// an empty slot holds a page whose index bits are not the slot's own
		const uint32_t mask = m_drc_tlb_size - 1;
		for (uint32_t table = 0; table < 4; table++)
		{
			for (uint32_t slot = 0; slot < m_drc_tlb_size; slot++)
			{
				m_drc_tlb[table * m_drc_tlb_size + slot] = uint64_t((slot ^ 1) & mask) << 12;
			}
		}
	}
}

void arm7_cpu_device::drc_tlb_flush_page(offs_t va)
{
	if (m_drc_tlb)
	{
		const uint32_t mask = m_drc_tlb_size - 1;
		const uint32_t slot = (va >> 12) & mask;
		for (uint32_t table = 0; table < 4; table++)
		{
			uint64_t &entry = m_drc_tlb[table * m_drc_tlb_size + slot];
			if (!((uint32_t(entry) ^ va) & 0xfffff000))
			{
				entry = uint64_t((slot ^ 1) & mask) << 12;
			}
		}
	}
}

// a page of the relocated (FCSE) address space: the virtual pages that can map to it
void arm7_cpu_device::drc_tlb_flush_mva(offs_t mva)
{
	drc_tlb_flush_page(mva);
	if (m_pid_offset != 0 && mva >= m_pid_offset && (mva - m_pid_offset) < 0x02000000)
	{
		drc_tlb_flush_page(mva - m_pid_offset);
	}
}


// the MMU, DATA32 and late-abort bits are compiled in, so a change to them flushes the cache
void arm7_cpu_device::drc_control_written(uint32_t data)
{
	if (((data ^ m_control) & (COPRO_CTRL_MMU_EN | COPRO_CTRL_DATA32 | COPRO_CTRL_LATE_ABORT)) != 0)
	{
		drc_remapped();
	}
}


// in the 26-bit modes the interpreter keeps the PSR in R15, generated code the address alone
void arm7_cpu_device::drc_r15_to_interp()
{
	if (m_core->mode & 1)
	{
		const uint32_t cpsr = m_r[eCPSR];
		m_r[eR15] = (m_r[eR15] & 0x03fffffc) | (cpsr & 0xf0000003) | ((cpsr & (I_MASK | F_MASK)) << 20);
	}
}

void arm7_cpu_device::drc_r15_from_interp()
{
	if (m_core->mode & 1)
	{
		m_r[eR15] &= 0x03fffffc;
	}
}


// arg0 = word, R15 = its address; ARM or Thumb by the CPSR now, which the debugger may have changed
void arm7_cpu_device::func_execute()
{
	drc_r15_to_interp();
	if (T_IS_SET(m_r[eCPSR]))
	{
		const uint32_t pc = m_r[eR15];
		execute_thumb_insn(pc, uint16_t(m_core->arg0 >> ((pc & 2) ? m_prefetch_word1_shift : m_prefetch_word0_shift)));
	}
	else
	{
		execute_arm_insn(m_core->arg0);
	}
	arm7_check_irq_state();
	drc_r15_from_interp();
}

static void cfunc_execute(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_execute();
}

// volatile code (m_drc_volatile): the word is fetched now, from arg1, as the interpreter's prefetch would
void arm7_cpu_device::func_execute_volatile()
{
	m_core->arg0 = insn_fetch_word(m_core->arg1);
	func_execute();
}

static void cfunc_execute_volatile(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_execute_volatile();
}


void arm7_cpu_device::interpret_slice()
{
	// the prefetch buffer is stale, so refill it from R15
	m_insn_prefetch_count = 0;
	m_insn_prefetch_index = 0;
	execute_run_interpreter();
}


void arm7_cpu_device::func_check_irq()
{
	drc_r15_to_interp();
	arm7_check_irq_state();
	drc_r15_from_interp();
}

static void cfunc_check_irq(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_check_irq();
}


// what the interpreter does when the prefetch fails; result = 0
void arm7_cpu_device::drc_prefetch_abort(offs_t pc)
{
	prefetch_abort(pc);
	m_pendingAbtP = true;
	update_irq_state();
	drc_r15_to_interp();
	arm7_check_irq_state();
	drc_r15_from_interp();
	m_core->result = 0;
}

// result = 1 if the page translates now, otherwise the prefetch abort is taken
void arm7_cpu_device::func_fetch_fault()
{
	const offs_t pc = m_core->arg0;
	offs_t physical = pc;
	if (!(m_control & COPRO_CTRL_MMU_EN) || translate_vaddr_to_paddr(physical, ARM7_TLB_ABORT_P | ARM7_TLB_READ))
	{
		m_core->result = 1;
		return;
	}

	drc_prefetch_abort(pc);
}

static void cfunc_fetch_fault(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_fetch_fault();
}


// retranslate a block's page, adding the fetch permission, or take the prefetch abort (result = 0)
void arm7_cpu_device::func_fetch_refill()
{
	const offs_t pc = m_core->arg0;
	offs_t physical = pc;
	if (translate_vaddr_to_paddr(physical, ARM7_TLB_ABORT_P | ARM7_TLB_READ))
	{
		// pages the walk doesn't fill (ARM1176, tiny pages, differing subpages) get the fetch permission here
		const offs_t index = ((pc < 0x02000000) ? (pc + m_pid_offset) : pc) >> 12;
		const vtlb_entry fetchbit = (GET_MODE == eARM7_MODE_USER) ? USER_FETCH_ALLOWED : FETCH_ALLOWED;
		const vtlb_entry entry = vtlb_table()[index];
		if ((entry ^ physical) & 0xfffff000)
		{
			if (entry & FLAG_VALID)
			{
				drc_tlb_flush_mva(index << 12);     // the data TLB may hold the old mapping
			}
			vtlb_dynload(index, physical, fetchbit);
		}
		else if (!(entry & fetchbit))
		{
			vtlb_dynload(index, physical, (entry & (FLAGS_MASK & ~FLAG_VALID)) | fetchbit);
		}
		m_core->result = 1;
		return;
	}

	drc_prefetch_abort(pc);
}

static void cfunc_fetch_refill(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_fetch_refill();
}


// arg0 = virtual address, arg1 = offset from the TLB base, result = table (bit 1 write, bit 0 user)
void arm7_cpu_device::func_tlb_check()
{
	const offs_t va = m_core->arg0;
	const bool iswrite = (m_core->result & 2) != 0;
	const bool user = (m_core->result & 1) != 0;
	const uint8_t *const got = m_drc_tlb_base + int32_t(m_core->arg1);
	m_core->tlb_stats[0]++;

	if ((m_archFlags & ARCHFLAG_MODE26) && !(m_control & COPRO_CTRL_DATA32) && (va & 0xfc000000))
	{
		fatalerror("%s: data TLB hit at %08X in the 26-bit address exception range\n", tag(), va);
	}
	offs_t pa = va;
	if (m_control & COPRO_CTRL_MMU_EN)
	{
		const offs_t mva = (va < 0x02000000) ? (va + m_pid_offset) : va;
		const vtlb_entry entry = vtlb_table()[mva >> 12];
		if (entry & ((iswrite ? WRITE_ALLOWED : READ_ALLOWED) << (user ? TR_USER : 0)))
		{
			pa = (entry & ~offs_t(0xfff)) | (mva & 0xfff);
		}
		else
		{
			m_core->tlb_stats[1]++;
			pa = mva;
			if (!page_table_translate(pa, 0))
			{
				fatalerror("%s: data TLB hit at %08X on a page the tables do not map\n", tag(), va);
			}
		}
	}

	const uint8_t *expected = nullptr;
	for (const tcm_info &tcm : m_drc_tcm)
	{
		if (tcm.data != nullptr && *tcm.base <= *tcm.end && va >= *tcm.base && va <= *tcm.end)
		{
			expected = reinterpret_cast<const uint8_t *>(tcm.data) + (va & tcm.mask);
			break;
		}
	}
	for (uint32_t ramnum = 0; expected == nullptr && ramnum < m_fastram_select; ramnum++)
	{
		const fast_ram_info &ram = m_fastram[ramnum];
		if (ram.direct && (!iswrite || !ram.readonly) && pa >= ram.start && pa <= ram.end)
		{
			expected = reinterpret_cast<const uint8_t *>(ram.base) + (pa - ram.start);
		}
	}
	if (got != expected)
	{
		fatalerror("%s: data TLB %s %s hit at %08X (physical %08X) gives %p, the translation %p\n", tag(), user ? "user" : "privileged",
				iswrite ? "write" : "read", va, pa, got, expected);
	}
}

static void cfunc_tlb_check(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_tlb_check();
}


// the slow path: the interpreter's accessors, with aborts reported through the stop word
void arm7_cpu_device::func_read8()
{
	m_core->result = arm7_cpu_read8(m_core->arg0);
	m_core->stop |= m_pendingAbtD ? STOP_ABORT : 0;
}

void arm7_cpu_device::func_read16()
{
	m_core->result = arm7_cpu_read16(m_core->arg0);
	m_core->stop |= m_pendingAbtD ? STOP_ABORT : 0;
}

void arm7_cpu_device::func_read32()
{
	m_core->result = arm7_cpu_read32(m_core->arg0);
	m_core->stop |= m_pendingAbtD ? STOP_ABORT : 0;
}

void arm7_cpu_device::func_write8()
{
	arm7_cpu_write8(m_core->arg0, m_core->arg1);
	m_core->stop |= m_pendingAbtD ? STOP_ABORT : 0;
}

void arm7_cpu_device::func_write16()
{
	arm7_cpu_write16(m_core->arg0, m_core->arg1);
	m_core->stop |= m_pendingAbtD ? STOP_ABORT : 0;
}

void arm7_cpu_device::func_write32()
{
	arm7_cpu_write32(m_core->arg0, m_core->arg1);
	m_core->stop |= m_pendingAbtD ? STOP_ABORT : 0;
}

static void cfunc_read8(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_read8();
}

static void cfunc_read16(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_read16();
}

static void cfunc_read32(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_read32();
}

static void cfunc_write8(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_write8();
}

static void cfunc_write16(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_write16();
}

static void cfunc_write32(void *param)
{
	reinterpret_cast<arm7_cpu_device *>(param)->func_write32();
}



/***************************************************************************
    CACHE MANAGEMENT
***************************************************************************/

void arm7_cpu_device::code_flush_cache()
{
	m_drcuml.reset();
	m_drc_prefetch.clear();
	m_drc_verify_misses.clear();
	drc_check_fastram();
	drc_tlb_choose_base();
	drc_tlb_flush();

	try
	{
		{
			drcuml_block &block(m_drcuml.begin_block(2048));
			int label = 1;

			static_generate_entry_point(block);
			static_generate_nocode_handler(block, label);
			static_generate_out_of_cycles(block);
			static_generate_resync(block, label);
			static_generate_slice_exact(block);
			static_generate_shift_reg(block, label, 0, "shift_lsl", m_shift_reg[0]);
			static_generate_shift_reg(block, label, 1, "shift_lsr", m_shift_reg[1]);
			static_generate_shift_reg(block, label, 2, "shift_asr", m_shift_reg[2]);
			static_generate_shift_reg(block, label, 3, "shift_ror", m_shift_reg[3]);

			block.end();
		}

		for (int user = 0; user < 2; user++)
		{
			drcuml_block &block(m_drcuml.begin_block(4096));
			int label = 1;

			static_generate_memory_accessor(block, label, user, 1, false, user ? "read8_user" : "read8", m_read8[user]);
			static_generate_memory_accessor(block, label, user, 1, true, user ? "write8_user" : "write8", m_write8[user]);
			static_generate_memory_accessor(block, label, user, 2, false, user ? "read16_user" : "read16", m_read16[user]);
			static_generate_memory_accessor(block, label, user, 2, true, user ? "write16_user" : "write16", m_write16[user]);
			static_generate_memory_accessor(block, label, user, 4, false, user ? "read32_user" : "read32", m_read32[user]);
			static_generate_memory_accessor(block, label, user, 4, true, user ? "write32_user" : "write32", m_write32[user]);

			block.end();
		}
	}
	catch (drcuml_block::abort_compilation &)
	{
		fatalerror("%s: unrecoverable error generating static code\n", tag());
	}
}


// an upper bound, for sizing the block
uint32_t arm7_cpu_device::estimate_uml_ops(const opcode_desc &desc)
{
	uint32_t ops = 64;
	if (desc.reads_memory() || desc.writes_memory())
	{
		ops += 64;                                                      // the data TLB probes inline (two for LDRD, STRD)
	}
	if (desc.length == 4 && (desc.opptr & 0x0e000000) == 0x08000000)
	{
		ops += 16 * std::popcount(desc.opptr & 0xffff);                 // LDM, STM: the loop and the data TLB's fast path
	}
	else if (desc.length == 2 && ((desc.opptr & 0xf000) == 0xc000 || (desc.opptr & 0xf600) == 0xb400))
	{
		ops += 16 * (std::popcount(desc.opptr & 0xff) + 1);             // LDMIA, STMIA, PUSH, POP
	}
	return ops;
}


bool arm7_cpu_device::code_compile_block(uint8_t mode, offs_t pc)
{
	compiler_state compiler;
	compiler.exact = (mode & DRC_MODE_EXACT) != 0;
	bool override = false;

	auto profile = g_profiler.start(PROFILER_DRC_COMPILE);

	const opcode_desc *desclist = m_drcfe->describe_code(pc);
	if (desclist == nullptr)
	{
		return false;
	}
	if (m_drcuml.logging() || m_drcuml.logging_native())
	{
		log_opcode_desc(desclist);
	}
	uint32_t maxinst = 128;
	for (const opcode_desc *curdesc = desclist; curdesc != nullptr; curdesc = curdesc->next())
	{
		maxinst += estimate_uml_ops(*curdesc);
	}

	bool succeeded = false;
	while (!succeeded)
	{
		try
		{
			drcuml_block &block(m_drcuml.begin_block(maxinst));

			const opcode_desc *seqlast;
			for (const opcode_desc *seqhead = desclist; seqhead != nullptr; seqhead = seqlast->next())
			{
				compiler.seqhead = seqhead;
				if (m_drcuml.logging())
				{
					block.append_comment("-------------------------");
				}

				for (seqlast = seqhead; seqlast != nullptr; seqlast = seqlast->next())
				{
					if (seqlast->end_sequence())
					{
						break;
					}
				}
				assert(seqlast != nullptr);

				const uint8_t key = drc_hash_key(mode, seqhead->pc);
				if (override || !m_drcuml.hash_exists(key, seqhead->pc))
				{
					UML_HASH(block, key, seqhead->pc);
				}
				else if (seqhead == desclist)
				{
					// a recompile after going out of sync: replace from here on
					override = true;
					UML_HASH(block, key, seqhead->pc);
				}
				else
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);
					UML_HASHJMP(block, key, seqhead->pc, *m_nocode_miss);
					continue;
				}
				UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                 // mapvar  CYCLES,0

				// a branch from within the block lands on the word check
				generate_validate_fetch(block, compiler, seqhead);
				if (seqhead->is_branch_target())
				{
					UML_LABEL(block, seqhead->pc | 0x80000000);
				}
				generate_checksum_block(block, compiler, seqhead, seqlast);

				generate_slice_check(block, compiler, seqhead, seqlast);

				for (const opcode_desc *curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
				{
					// the head's page was validated above
					if (curdesc != seqhead && curdesc->validate_tlb())
					{
						generate_validate_fetch(block, compiler, curdesc);
					}
					// the words after a store are checked again, outside any condition run
					if (curdesc == compiler.verify_next)
					{
						generate_verify_chunk(block, compiler, curdesc, seqlast);
					}
					generate_condition_run(block, compiler, curdesc, seqlast);
					generate_sequence_instruction(block, compiler, curdesc);
					generate_condition_run_end(block, compiler, curdesc);
				}

				if (!drc_generates(*seqlast) || seqlast->compiler_page_fault())
				{
					// the interpreter ran it: R15 and the mode are as it left them, and a store may have remapped
					if (!seqlast->compiler_page_fault())
					{
						UML_TEST(block, mem(&m_core->stop), ~uint32_t(0));         // test    stop,~0
						UML_EXHc(block, uml::COND_NZ, *m_resync, mem(&m_r[eR15])); // exh     resync,<r15>,nz
					}
					generate_update_cycles(block, compiler, mem(&m_r[eR15]));
					generate_hashjmp_dynamic(block);
				}
				else if (!seqlast->is_unconditional_branch())
				{
					// fall through to the next instruction, through the hash table unless it follows in this block
					const uint32_t nextpc = seqlast->return_to_start() ? pc : (seqlast->pc + seqlast->length);
					generate_update_cycles(block, compiler, nextpc);
					if (seqlast->can_change_modes())
					{
						UML_OR(block, I0, mem(&m_core->mode), drc_hash_key(0, nextpc));    // or      i0,<mode>,half
						UML_HASHJMP(block, I0, nextpc, *m_nocode_miss);                     // hashjmp i0,nextpc,nocode_miss
					}
					else if (compiler.exact || seqlast->next() == nullptr || seqlast->next()->pc != nextpc)
					{
						UML_HASHJMP(block, drc_hash_key(mode & ~DRC_MODE_EXACT, nextpc), nextpc, *m_nocode_miss);   // an exact copy returns to the full-speed code
					}
				}
				else
				{
					// a generated unconditional branch has charged the cycles and left; nothing carries over
					compiler.cycles = 0;
				}

				// an exact copy is one sequence: what follows goes through the hash table and is checked afresh
				if (compiler.exact)
				{
					break;
				}
			}

			block.end();
			succeeded = true;
		}
		catch (drcuml_block::abort_compilation &)
		{
			code_flush_cache();
		}
	}
	return true;
}



/***************************************************************************
    STATIC CODEGEN
***************************************************************************/

void arm7_cpu_device::generate_hashjmp_dynamic(drcuml_block &block)
{
	UML_MOV(block, I0, mem(&m_core->mode));                                     // mov     i0,<mode>
	UML_ROLINS(block, I0, mem(&m_r[eR15]), 4, DRC_MODE_HALF);                   // rolins  i0,<r15>,4,HALF
	UML_HASHJMP(block, I0, mem(&m_r[eR15]), *m_nocode_miss);                    // hashjmp i0,<r15>,nocode_miss
}


// R15 as the interpreter reads it in the 26-bit modes, plus addend (an add, so a carry out of the PC matches)
void arm7_cpu_device::generate_r15_psr26(drcuml_block &block, uml::parameter dst, uml::parameter addend, uml::parameter temp)
{
	UML_ROLAND(block, temp, mem(&m_r[eCPSR]), 20, (I_MASK | F_MASK) << 20);     // roland  temp,cpsr,20,IF
	UML_AND(block, dst, mem(&m_r[eCPSR]), 0xf0000003);                          // and     dst,cpsr,NZCV|mode
	UML_OR(block, dst, dst, temp);                                              // or      dst,dst,temp
	UML_ADD(block, dst, dst, addend);                                           // add     dst,dst,addend
}


void arm7_cpu_device::static_generate_entry_point(drcuml_block &block)
{
	alloc_handle(m_drcuml, m_nocode_miss, "nocode_miss");
	alloc_handle(m_drcuml, m_nocode_exact, "nocode_exact");
	alloc_handle(m_drcuml, m_entry, "entry");
	UML_HANDLE(block, *m_entry);                                                // handle  entry
	generate_hashjmp_dynamic(block);                                            // hashjmp <mode>,<r15>,nocode_miss

	alloc_handle(m_drcuml, m_entry_exact, "entry_exact");
	UML_HANDLE(block, *m_entry_exact);                                          // handle  entry_exact
	UML_OR(block, I0, mem(&m_core->mode), DRC_MODE_EXACT);                      // or      i0,<mode>,EXACT
	UML_ROLINS(block, I0, mem(&m_r[eR15]), 4, DRC_MODE_HALF);                   // rolins  i0,<r15>,4,HALF
	UML_HASHJMP(block, I0, mem(&m_r[eR15]), *m_nocode_exact);                   // hashjmp i0,<r15>,nocode_exact
}


// RECOVER only works from a site inside a block, so only the handlers raised in a sequence recover cycles
void arm7_cpu_device::static_generate_nocode_handler(drcuml_block &block, int &label)
{
	// raised within a sequence: the cycles so far are owed, and a failed word check counts towards volatile code
	const code_label owed = label++;
	alloc_handle(m_drcuml, m_nocode_verify, "nocode_verify");
	UML_HANDLE(block, *m_nocode_verify);                                        // handle  nocode_verify
	UML_MOV(block, mem(&m_core->verify_miss), 1);                               // mov     verify_miss,1
	UML_JMP(block, owed);                                                       // jmp     owed

	alloc_handle(m_drcuml, m_nocode, "nocode");
	UML_HANDLE(block, *m_nocode);                                               // handle  nocode
	UML_LABEL(block, owed);                                                     // owed:
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_r[eR15]), I0);                                        // mov     r15,i0
	UML_RECOVER(block, I1, MAPVAR_CYCLES);                                      // recover i1,CYCLES
	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), I1);             // sub     icount,icount,i1
	UML_EXIT(block, EXECUTE_MISSING_CODE);                                      // exit    EXECUTE_MISSING_CODE

	// a hash miss: nothing is owed
	alloc_handle(m_drcuml, m_nocode_miss, "nocode_miss");
	UML_HANDLE(block, *m_nocode_miss);                                          // handle  nocode_miss
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_r[eR15]), I0);                                        // mov     r15,i0
	UML_EXIT(block, EXECUTE_MISSING_CODE);                                      // exit    EXECUTE_MISSING_CODE

	// a hash miss in the exact copies: the block compiled next is an exact one
	alloc_handle(m_drcuml, m_nocode_exact, "nocode_exact");
	UML_HANDLE(block, *m_nocode_exact);                                         // handle  nocode_exact
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_r[eR15]), I0);                                        // mov     r15,i0
	UML_MOV(block, mem(&m_core->exact_miss), 1);                                // mov     exact_miss,1
	UML_EXIT(block, EXECUTE_MISSING_CODE);                                      // exit    EXECUTE_MISSING_CODE

	// a folded PC-relative literal changed: the load is compiled as a load from then on
	alloc_handle(m_drcuml, m_nocode_literal, "nocode_literal");
	UML_HANDLE(block, *m_nocode_literal);                                       // handle  nocode_literal
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_r[eR15]), I0);                                        // mov     r15,i0
	UML_RECOVER(block, I1, MAPVAR_CYCLES);                                      // recover i1,CYCLES
	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), I1);             // sub     icount,icount,i1
	UML_MOV(block, mem(&m_core->literal_miss), 1);                              // mov     literal_miss,1
	UML_EXIT(block, EXECUTE_MISSING_CODE);                                      // exit    EXECUTE_MISSING_CODE
}


void arm7_cpu_device::static_generate_out_of_cycles(drcuml_block &block)
{
	alloc_handle(m_drcuml, m_out_of_cycles, "out_of_cycles");
	UML_HANDLE(block, *m_out_of_cycles);                                        // handle  out_of_cycles
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_r[eR15]), I0);                                        // mov     r15,i0
	UML_EXIT(block, EXECUTE_OUT_OF_CYCLES);                                     // exit    EXECUTE_OUT_OF_CYCLES
}


// leaves a sequence after a memory access: charges the cycles, takes a pending exception, handles a remap
void arm7_cpu_device::static_generate_resync(drcuml_block &block, int &label)
{
	// resync_next continues in sequence, so a remap runs the words the interpreter had prefetched
	const code_label common = label++;
	alloc_handle(m_drcuml, m_resync_next, "resync_next");
	UML_HANDLE(block, *m_resync_next);                                          // handle  resync_next
	UML_MOV(block, mem(&m_core->resync_next), 1);                               // mov     resync_next,1
	UML_JMP(block, common);                                                     // jmp     common

	alloc_handle(m_drcuml, m_resync, "resync");
	UML_HANDLE(block, *m_resync);                                               // handle  resync
	UML_MOV(block, mem(&m_core->resync_next), 0);                               // mov     resync_next,0
	UML_LABEL(block, common);                                                   // common:
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_MOV(block, mem(&m_r[eR15]), I0);                                        // mov     r15,i0
	UML_RECOVER(block, I1, MAPVAR_CYCLES);                                      // recover i1,CYCLES
	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), I1);             // sub     icount,icount,i1

	// an abort or interrupt raised by the access is taken before the next instruction, as the interpreter does
	UML_LOAD(block, I2, &m_pending_interrupt, 0, SIZE_BYTE, SCALE_x1);          // load    i2,[pending],byte
	UML_TEST(block, I2, I2);                                                    // test    i2,i2
	UML_CALLCc(block, uml::COND_NZ, cfunc_check_irq, this);                     // callc   check_irq,nz

	// a remap flushes the cache, and execution resumes at R15
	const code_label kept = label++;
	UML_MOV(block, I2, mem(&m_core->stop));                                     // mov     i2,stop
	UML_MOV(block, mem(&m_core->stop), 0);                                      // mov     stop,0
	UML_TEST(block, I2, STOP_REMAP);                                            // test    i2,REMAP
	UML_JMPc(block, uml::COND_Z, kept);                                         // jmp     kept,z
	UML_EXIT(block, EXECUTE_RESET_CACHE);                                       // exit    EXECUTE_RESET_CACHE
	UML_LABEL(block, kept);                                                     // kept:
	UML_CMP(block, mem(&m_core->icount), 0);                                    // cmp     icount,0
	UML_EXHc(block, uml::COND_LE, *m_out_of_cycles, mem(&m_r[eR15]));           // exh     out_of_cycles,<r15>,le
	generate_hashjmp_dynamic(block);                                            // hashjmp <mode>,<r15>,nocode_miss
}


// the slice can't take the whole sequence: run its exact copy instead
void arm7_cpu_device::static_generate_slice_exact(drcuml_block &block)
{
	alloc_handle(m_drcuml, m_slice_exact, "slice_exact");
	UML_HANDLE(block, *m_slice_exact);                                          // handle  slice_exact
	UML_GETEXP(block, I0);                                                      // getexp  i0
	UML_OR(block, I1, mem(&m_core->mode), DRC_MODE_EXACT);                      // or      i1,<mode>,EXACT
	UML_ROLINS(block, I1, I0, 4, DRC_MODE_HALF);                                // rolins  i1,i0,4,HALF
	UML_HASHJMP(block, I1, I0, *m_nocode_exact);                                // hashjmp i1,i0,nocode_exact
}


// decodeShift() for register amounts: I1 = value, I2 = amount (0-255) in; I1 = result, I3 = carry (C_MASK/0) out
void arm7_cpu_device::static_generate_shift_reg(drcuml_block &block, int &label, uint32_t type, const char *name, uml::code_handle *&handleptr)
{
	alloc_handle(m_drcuml, handleptr, name);
	UML_HANDLE(block, *handleptr);                                                  // handle  name

	const uml::parameter cpsr = uml::mem(&m_r[eCPSR]);
	const uml::code_label done = label++;
	const uml::code_label normal = label++;

	// an amount of 0 leaves the value and the carry alone
	UML_AND(block, uml::I3, cpsr, C_MASK);                                          // and     i3,cpsr,C
	UML_TEST(block, uml::I2, uml::I2);                                              // test    i2,i2
	UML_JMPc(block, uml::COND_Z, done);                                             // jmp     done,z

	switch (type)
	{
		case 0: // LSL: 32 shifts everything out with bit 0 as the carry, more shifts the carry out too
			{
				const uml::code_label big = label++;
				UML_CMP(block, uml::I2, 32);                                        // cmp     i2,32
				UML_JMPc(block, uml::COND_AE, big);                                 // jmp     big,ae
				UML_SUB(block, uml::I3, 32, uml::I2);                               // sub     i3,32,i2
				UML_SHR(block, uml::I3, uml::I1, uml::I3);                          // shr     i3,i1,i3
				UML_ROLAND(block, uml::I3, uml::I3, C_BIT, C_MASK);                 // roland  i3,i3,29,C
				UML_SHL(block, uml::I1, uml::I1, uml::I2);                          // shl     i1,i1,i2
				UML_JMP(block, done);                                               // jmp     done
				UML_LABEL(block, big);                                              // big:
				UML_MOV(block, uml::I3, 0);                                         // mov     i3,0
				UML_CMP(block, uml::I2, 32);                                        // cmp     i2,32
				UML_JMPc(block, uml::COND_NE, normal);                              // jmp     normal,ne
				UML_ROLAND(block, uml::I3, uml::I1, C_BIT, C_MASK);                 // roland  i3,i1,29,C
				UML_LABEL(block, normal);                                           // normal:
				UML_MOV(block, uml::I1, 0);                                         // mov     i1,0
			}
			break;

		case 1: // LSR: 32 shifts everything out with bit 31 as the carry, more shifts the carry out too
			{
				const uml::code_label big = label++;
				UML_CMP(block, uml::I2, 32);                                        // cmp     i2,32
				UML_JMPc(block, uml::COND_AE, big);                                 // jmp     big,ae
				UML_SUB(block, uml::I3, uml::I2, 1);                                // sub     i3,i2,1
				UML_SHR(block, uml::I3, uml::I1, uml::I3);                          // shr     i3,i1,i3
				UML_ROLAND(block, uml::I3, uml::I3, C_BIT, C_MASK);                 // roland  i3,i3,29,C
				UML_SHR(block, uml::I1, uml::I1, uml::I2);                          // shr     i1,i1,i2
				UML_JMP(block, done);                                               // jmp     done
				UML_LABEL(block, big);                                              // big:
				UML_MOV(block, uml::I3, 0);                                         // mov     i3,0
				UML_CMP(block, uml::I2, 32);                                        // cmp     i2,32
				UML_JMPc(block, uml::COND_NE, normal);                              // jmp     normal,ne
				UML_ROLAND(block, uml::I3, uml::I1, 30, C_MASK);                    // roland  i3,i1,30,C
				UML_LABEL(block, normal);                                           // normal:
				UML_MOV(block, uml::I1, 0);                                         // mov     i1,0
			}
			break;

		case 2: // ASR: 32 or more fills with the sign, which is also the carry
			{
				const uml::code_label big = label++;
				UML_CMP(block, uml::I2, 32);                                        // cmp     i2,32
				UML_JMPc(block, uml::COND_AE, big);                                 // jmp     big,ae
				UML_SUB(block, uml::I3, uml::I2, 1);                                // sub     i3,i2,1
				UML_SHR(block, uml::I3, uml::I1, uml::I3);                          // shr     i3,i1,i3
				UML_ROLAND(block, uml::I3, uml::I3, C_BIT, C_MASK);                 // roland  i3,i3,29,C
				UML_SAR(block, uml::I1, uml::I1, uml::I2);                          // sar     i1,i1,i2
				UML_JMP(block, done);                                               // jmp     done
				UML_LABEL(block, big);                                              // big:
				UML_ROLAND(block, uml::I3, uml::I1, 30, C_MASK);                    // roland  i3,i1,30,C
				UML_SAR(block, uml::I1, uml::I1, 31);                               // sar     i1,i1,31
			}
			break;

		case 3: // ROR: only the bottom five bits count; a multiple of 32 leaves the value with bit 31 as the carry
		default:
			UML_AND(block, uml::I2, uml::I2, 31);                                   // and     i2,i2,31
			UML_JMPc(block, uml::COND_Z, normal);                                   // jmp     normal,z
			UML_SUB(block, uml::I3, uml::I2, 1);                                    // sub     i3,i2,1
			UML_SHR(block, uml::I3, uml::I1, uml::I3);                              // shr     i3,i1,i3
			UML_ROLAND(block, uml::I3, uml::I3, C_BIT, C_MASK);                     // roland  i3,i3,29,C
			UML_ROR(block, uml::I1, uml::I1, uml::I2);                              // ror     i1,i1,i2
			UML_JMP(block, done);                                                   // jmp     done
			UML_LABEL(block, normal);                                               // normal:
			UML_ROLAND(block, uml::I3, uml::I1, 30, C_MASK);                        // roland  i3,i1,30,C
			break;
	}

	UML_LABEL(block, done);                                                         // done:
	UML_RET(block);                                                                 // ret
}


// I0 = address, I1 = write data; a hit leaves the result in I0, a miss jumps with I0/I1 intact; I2, I3 trashed
void arm7_cpu_device::generate_tlb_probe(drcuml_block &block, int size, bool iswrite, bool user, uml::code_label miss)
{
	const operand_size osize = (size == 1) ? SIZE_BYTE : (size == 2) ? SIZE_WORD : SIZE_DWORD;
	const bool rotates = !iswrite && (size == 4 || (size == 2 && drc_memory_rotates_halfwords()));
	const uint32_t lanexor = drc_lane_xor(size);

	generate_tlb_lookup(block, I0, iswrite, user, miss);
	if (size == 4)
	{
		if (!iswrite)
		{
			UML_SHL(block, I3, I0, 3);                                          // shl     i3,i0,3
		}
		UML_AND(block, I2, I2, ~uint32_t(3));                                   // and     i2,i2,~3
	}
	else if (size == 2)
	{
		if (rotates)
		{
			UML_AND(block, I3, I0, 1);                                          // and     i3,i0,1
			UML_SHL(block, I3, I3, 3);                                          // shl     i3,i3,3
		}
		UML_AND(block, I2, I2, ~uint32_t(1));                                   // and     i2,i2,~1
	}
	if (lanexor != 0)
	{
		UML_XOR(block, I2, I2, lanexor);                                        // xor     i2,i2,lanexor
	}
	if (iswrite)
	{
		UML_STORE(block, m_drc_tlb_base, I2, I1, osize, SCALE_x1);              // store   base,i2,i1
	}
	else
	{
		UML_LOAD(block, I0, m_drc_tlb_base, I2, osize, SCALE_x1);               // load    i0,base,i2
		if (rotates)
		{
			UML_ROR(block, I0, I0, I3);                                         // ror     i0,i0,i3
		}
	}
}


// a hit leaves addr's offset from the TLB base in I2, a miss jumps to the label; I3 trashed
void arm7_cpu_device::generate_tlb_lookup(drcuml_block &block, uml::parameter addr, bool iswrite, bool user, uml::code_label miss)
{
	UML_ROLAND(block, I3, addr, 20, m_drc_tlb_size - 1);                        // roland  i3,addr,20,mask
	UML_DLOAD(block, I2, drc_tlb_table(iswrite, user), I3, SIZE_QWORD, SCALE_x8);   // dload   i2,[tlb],i3,qword
	UML_XOR(block, I3, I2, addr);                                               // xor     i3,i2,addr
	UML_TEST(block, I3, 0xfffff000);                                            // test    i3,0xfffff000
	UML_JMPc(block, uml::COND_NZ, miss);                                        // jmp     miss,nz
	UML_DSHR(block, I2, I2, 32);                                                // dshr    i2,i2,32
	UML_ADD(block, I2, I2, addr);                                               // add     i2,i2,addr  (the offset from the base)
	if (DRC_TLB_CHECK)
	{
		generate_tlb_check_call(block, addr, I2, iswrite, user);
	}
}


void arm7_cpu_device::generate_tlb_check_call(drcuml_block &block, uml::parameter addr, uml::parameter offset, bool iswrite, bool user)
{
	UML_MOV(block, mem(&m_core->arg0), addr);                                   // mov     arg0,addr
	UML_MOV(block, mem(&m_core->arg1), offset);                                 // mov     arg1,offset
	UML_MOV(block, mem(&m_core->result), (iswrite ? 2 : 0) | (user ? 1 : 0));   // mov     result,table
	UML_CALLC(block, cfunc_tlb_check, this);                                    // callc   tlb_check
}


// fills the entry for the virtual address in I2, whose host address is base + hostdelta + offset; I2, temp trashed
void arm7_cpu_device::generate_tlb_fill(drcuml_block &block, uint64_t *table, uml::parameter offset, uint32_t hostdelta, uml::parameter temp)
{
	UML_AND(block, temp, offset, ~uint32_t(0xfff));                             // and     temp,offset,~0xfff
	UML_ADD(block, temp, temp, hostdelta);                                      // add     temp,temp,hostdelta (the host page from the base)
	UML_DAND(block, I2, I2, 0xfffff000);                                        // dand    i2,i2,0xfffff000    (the virtual page)
	UML_SUB(block, temp, temp, I2);                                             // sub     temp,temp,i2        (the addend)
	UML_DSHL(block, temp, temp, 32);                                            // dshl    temp,temp,32
	UML_DOR(block, temp, temp, I2);                                             // dor     temp,temp,i2
	UML_ROLAND(block, I2, I2, 20, m_drc_tlb_size - 1);                          // roland  i2,i2,20,mask
	UML_DSTORE(block, table, I2, temp, SIZE_QWORD, SCALE_x8);                   // dstore  [tlb],i2,temp,qword
	if (DRC_TLB_CHECK)
	{
		UML_DADD(block, mem(&m_core->tlb_stats[2]), mem(&m_core->tlb_stats[2]), 1); // dadd    fills,fills,1
	}
}


// I0 = virtual address, I1 = write data on entry; I0 = read data on exit; I0-I3 trashed
void arm7_cpu_device::static_generate_memory_accessor(drcuml_block &block, int &label, bool user, int size, bool iswrite, const char *name, uml::code_handle *&handleptr)
{
	alloc_handle(m_drcuml, handleptr, name);
	UML_HANDLE(block, *handleptr);                                              // handle  name

	const code_label slow = label++;

	if (drc_memory_fast_path())
	{
		const bool static_control = drc_static_control();
		const bool mmu = (m_control & COPRO_CTRL_MMU_EN) != 0;
		const code_label physical = label++;
		const vtlb_entry allowed = (iswrite ? WRITE_ALLOWED : READ_ALLOWED) << (user ? TR_USER : 0);
		const operand_size osize = (size == 1) ? SIZE_BYTE : (size == 2) ? SIZE_WORD : SIZE_DWORD;
		const bool rotates = !iswrite && (size == 4 || (size == 2 && drc_memory_rotates_halfwords()));
		const uml::parameter scratch = iswrite ? I3 : I1;   // for an offset within a region: a write has no rotate, a read no data
		const uint32_t lanexor = drc_lane_xor(size);
		uint64_t *const tlb = (m_drc_tlb_base != nullptr) ? drc_tlb_table(iswrite, user) : nullptr;

		// a block that probes the data TLB inline comes in at the _miss handle
		if (tlb != nullptr)
		{
			const code_label miss = label++;
			generate_tlb_probe(block, size, iswrite, user, miss);
			UML_RET(block);                                                     // ret
			UML_LABEL(block, miss);                                             // miss:
			const std::string missname = std::string(name) + "_miss";
			alloc_handle(m_drcuml, m_access_miss[iswrite ? 1 : 0][(size == 1) ? 0 : (size == 2) ? 1 : 2][user ? 1 : 0], missname.c_str());
			UML_HANDLE(block, *m_access_miss[iswrite ? 1 : 0][(size == 1) ? 0 : (size == 2) ? 1 : 2][user ? 1 : 0]);   // handle  name_miss
			if (DRC_TLB_CHECK)
			{
				UML_DADD(block, mem(&m_core->tlb_stats[3]), mem(&m_core->tlb_stats[3]), 1); // dadd    misses,misses,1
			}
		}

		UML_MOV(block, I2, I0);                                                 // mov     i2,i0       (the virtual address, for the slow path)

		if (m_archFlags & ARCHFLAG_MODE26)
		{
			// the 26-bit data space: with DATA32 clear, an address with bits 31:26 set traps or is masked
			if (!static_control)
			{
				const code_label data32 = label++;
				UML_LOAD(block, I3, &m_control, 0, SIZE_DWORD, SCALE_x1);       // load    i3,[control],dword
				UML_TEST(block, I3, COPRO_CTRL_DATA32);                         // test    i3,DATA32
				UML_JMPc(block, uml::COND_NZ, data32);                          // jmp     data32,nz
				UML_TEST(block, I0, 0xfc000000);                                // test    i0,0xfc000000
				UML_JMPc(block, uml::COND_NZ, slow);                            // jmp     slow,nz
				UML_LABEL(block, data32);                                       // data32:
			}
			else if (!(m_control & COPRO_CTRL_DATA32))
			{
				UML_TEST(block, I0, 0xfc000000);                                // test    i0,0xfc000000
				UML_JMPc(block, uml::COND_NZ, slow);                            // jmp     slow,nz
			}
		}

		// with a static control register, the MMU bit is compiled in (drc_control_written())
		if (!static_control || mmu)
		{
			const code_label nopid = label++;
			if (!static_control)
			{
				UML_LOAD(block, I3, &m_control, 0, SIZE_DWORD, SCALE_x1);       // load    i3,[control],dword
				UML_TEST(block, I3, COPRO_CTRL_MMU_EN);                         // test    i3,MMU_EN
				UML_JMPc(block, uml::COND_Z, physical);                         // jmp     physical,z
			}

			// the process ID relocates the bottom 32 MB
			UML_CMP(block, I0, 0x02000000);                                     // cmp     i0,0x02000000
			UML_JMPc(block, uml::COND_AE, nopid);                               // jmp     nopid,ae
			UML_LOAD(block, I3, &m_pid_offset, 0, SIZE_DWORD, SCALE_x1);        // load    i3,[pid_offset],dword
			UML_ADD(block, I0, I0, I3);                                         // add     i0,i0,i3
			UML_LABEL(block, nopid);                                            // nopid:

			// the translation table holds the pages the walk has seen, with both privilege levels' permissions
			UML_SHR(block, I3, I0, 12);                                         // shr     i3,i0,12
			UML_LOAD(block, I3, vtlb_table(), I3, SIZE_DWORD, SCALE_x4);        // load    i3,[vtlb_table],i3,dword
			UML_TEST(block, I3, allowed);                                       // test    i3,allowed
			UML_JMPc(block, uml::COND_Z, slow);                                 // jmp     slow,z
			UML_ROLINS(block, I0, I3, 0, 0xfffff000);                           // rolins  i0,i3,0,0xfffff000
		}
		UML_LABEL(block, physical);                                             // physical:

		// an unaligned read rotates the aligned data, eight bits per byte of misalignment
		if (size == 4)
		{
			if (!iswrite)
			{
				UML_SHL(block, I3, I0, 3);                                      // shl     i3,i0,3
			}
			UML_AND(block, I0, I0, ~uint32_t(3));                               // and     i0,i0,~3
		}
		else if (size == 2)
		{
			if (rotates)
			{
				UML_AND(block, I3, I0, 1);                                      // and     i3,i0,1
				UML_SHL(block, I3, I3, 3);                                      // shl     i3,i3,3
			}
			UML_AND(block, I0, I0, ~uint32_t(1));                               // and     i0,i0,~1
		}

		// TCMs are accessed in place; their bounds are compiled in (a change flushes the cache)
		for (const tcm_info &tcm : m_drc_tcm)
		{
			if (tcm.data != nullptr && *tcm.base <= *tcm.end)
			{
				const code_label skip = label++;
				if (*tcm.base != 0)
				{
					UML_SUB(block, scratch, I2, *tcm.base);                     // sub     scratch,i2,base
					UML_CMP(block, scratch, *tcm.end - *tcm.base);              // cmp     scratch,end-base
				}
				else
				{
					UML_CMP(block, I2, *tcm.end);                               // cmp     i2,end
				}
				UML_JMPc(block, uml::COND_A, skip);                             // jmp     skip,a
				UML_AND(block, I0, I0, tcm.mask);                               // and     i0,i0,mask

				// pages wholly inside the window go to the data TLB (the TCM mirrors at its size), unless its hits swap lanes
				const uint32_t fill_first = (*tcm.base + 0xfff) & ~uint32_t(0xfff);
				const uint32_t fill_end = (*tcm.end + 1) & ~uint32_t(0xfff);
				const bool fill = (tlb != nullptr) && (lanexor == 0) && fill_end > fill_first && tcm.mask >= 0xfff && drc_tlb_fits(tcm.data, tcm.mask + 1);
				const auto tcm_fill = [this, &block, &label, &tcm, tlb, fill_first, fill_end] ()
				{
					const code_label nofill = label++;
					UML_SUB(block, I1, I2, fill_first);                         // sub     i1,i2,first
					UML_CMP(block, I1, fill_end - fill_first - 1);              // cmp     i1,end-first-1
					UML_JMPc(block, uml::COND_A, nofill);                       // jmp     nofill,a
					generate_tlb_fill(block, tlb, I0, uint32_t(reinterpret_cast<uint8_t *>(tcm.data) - m_drc_tlb_base), I1);
					UML_LABEL(block, nofill);                                   // nofill:
				};
				if (iswrite)
				{
					UML_STORE(block, tcm.data, I0, I1, osize, SCALE_x1);        // store   tcm,i0,i1
					if (fill)
					{
						tcm_fill();
					}
				}
				else
				{
					if (fill)
					{
						tcm_fill();
					}
					UML_LOAD(block, I0, tcm.data, I0, osize, SCALE_x1);         // load    i0,tcm,i0
					if (rotates)
					{
						UML_ROR(block, I0, I0, I3);                             // ror     i0,i0,i3
					}
				}
				UML_RET(block);                                                 // ret
				UML_LABEL(block, skip);                                         // skip:
			}
		}

		// fast RAM is accessed in place, bypassing watchpoints and memory taps
		if (!debugger_enabled() && (m_drc_opt & DRC_OPT_FASTRAM))
		{
			for (uint32_t ramnum = 0; ramnum < m_fastram_select; ramnum++)
			{
				const fast_ram_info &ram = m_fastram[ramnum];
				if (ram.direct && (!iswrite || !ram.readonly))
				{
					// LOAD and STORE indexes are signed 32-bit, so index from the region's base, not from address 0
					const code_label skip = label++;
					uml::parameter index = I0;
					if (ram.start != 0)
					{
						UML_SUB(block, scratch, I0, ram.start);                 // sub     scratch,i0,start
						UML_CMP(block, scratch, ram.end - ram.start);           // cmp     scratch,end-start
						UML_JMPc(block, uml::COND_A, skip);                     // jmp     skip,a
						index = scratch;
					}
					else if (ram.end != 0xffffffff)
					{
						UML_CMP(block, I0, ram.end);                            // cmp     i0,end
						UML_JMPc(block, uml::COND_A, skip);                     // jmp     skip,a
					}

					// a region of whole pages within reach of the base goes to the data TLB
					const bool fill = (tlb != nullptr) && !(ram.start & 0xfff) && !((ram.end + 1) & 0xfff) && drc_tlb_fits(ram.base, ram.end - ram.start + 1);
					const uint32_t hostdelta = fill ? uint32_t(reinterpret_cast<uint8_t *>(ram.base) - m_drc_tlb_base) : 0;
					if (fill && !iswrite)
					{
						generate_tlb_fill(block, tlb, index, hostdelta, (index == I0) ? I1 : I0);
					}

					// bytes and halfwords swap lanes when guest and host endianness differ
					if (lanexor != 0)
					{
						UML_XOR(block, index, index, lanexor);                  // xor     index,index,lanexor
					}
					if (iswrite)
					{
						UML_STORE(block, ram.base, index, I1, osize, SCALE_x1); // store   base,index,i1
						if (fill)
						{
							generate_tlb_fill(block, tlb, index, hostdelta, I1);
						}
					}
					else
					{
						UML_LOAD(block, I0, ram.base, index, osize, SCALE_x1);  // load    i0,base,index
						if (rotates)
						{
							UML_ROR(block, I0, I0, I3);                         // ror     i0,i0,i3
						}
					}
					UML_RET(block);                                             // ret
					UML_LABEL(block, skip);                                     // skip:
				}
			}
		}

		// the bus: bytes and halfwords go out with the other lanes clear, as the interpreter sends them
		switch (size)
		{
			case 1:
				if (iswrite)
				{
					UML_AND(block, I1, I1, 0xff);                               // and     i1,i1,0xff
					UML_WRITE(block, I0, I1, SIZE_BYTE, SPACE_PROGRAM);         // write   i0,i1,program_byte
				}
				else
				{
					UML_READ(block, I0, I0, SIZE_BYTE, SPACE_PROGRAM);          // read    i0,i0,program_byte
				}
				break;

			case 2:
				if (iswrite)
				{
					UML_AND(block, I1, I1, 0xffff);                             // and     i1,i1,0xffff
					UML_WRITE(block, I0, I1, SIZE_WORD, SPACE_PROGRAM);         // write   i0,i1,program_word
				}
				else
				{
					UML_READ(block, I0, I0, SIZE_WORD, SPACE_PROGRAM);          // read    i0,i0,program_word
					if (rotates)
					{
						UML_ROR(block, I0, I0, I3);                             // ror     i0,i0,i3
					}
				}
				break;

			case 4:
			default:
				if (iswrite)
				{
					UML_WRITE(block, I0, I1, SIZE_DWORD, SPACE_PROGRAM);        // write   i0,i1,program_dword
				}
				else
				{
					UML_READ(block, I0, I0, SIZE_DWORD, SPACE_PROGRAM);         // read    i0,i0,program_dword
					UML_ROR(block, I0, I0, I3);                                 // ror     i0,i0,i3
				}
				break;
		}
		UML_RET(block);                                                         // ret

		UML_LABEL(block, slow);                                                 // slow:
		UML_MOV(block, I0, I2);                                                 // mov     i0,i2
	}

	// the interpreter's accessor: aborts, the 26-bit data space, the subclass overrides
	UML_MOV(block, mem(&m_core->arg0), I0);                                     // mov     arg0,i0
	if (iswrite)
	{
		UML_MOV(block, mem(&m_core->arg1), I1);                                 // mov     arg1,i1
	}
	switch (size)
	{
		case 1:
			UML_CALLC(block, iswrite ? cfunc_write8 : cfunc_read8, this);       // callc   read8/write8
			break;
		case 2:
			UML_CALLC(block, iswrite ? cfunc_write16 : cfunc_read16, this);     // callc   read16/write16
			break;
		case 4:
		default:
			UML_CALLC(block, iswrite ? cfunc_write32 : cfunc_read32, this);     // callc   read32/write32
			break;
	}
	if (!iswrite)
	{
		UML_MOV(block, I0, mem(&m_core->result));                               // mov     i0,result
	}
	UML_RET(block);                                                             // ret
}



/***************************************************************************
    CODE GENERATION
***************************************************************************/

// an interpreted instruction costs the interpreter loop's 3, its handler adds its own adjustments
uint32_t arm7_cpu_device::instruction_cycles(const opcode_desc *desc) const
{
	uint32_t cycles = drc_generates(*desc) ? desc->cycles : 3;
	for (uint32_t hotnum = 0; hotnum < m_drc_hotspot_select; hotnum++)
	{
		if (m_drc_hotspot[hotnum].pc == desc->pc && m_drc_hotspot[hotnum].opcode == desc->opptr)
		{
			cycles += m_drc_hotspot[hotnum].cycles;
			break;
		}
	}
	return cycles;
}


// the multiplies add up to 4 cycles for the magnitude of Rs
uint32_t arm7_cpu_device::instruction_cycles_max(const opcode_desc *desc) const
{
	uint32_t cycles = instruction_cycles(desc);
	if (desc->length == 4 && m_archRev >= 3 && drc_generates(*desc) && (desc->opptr & 0x0f0000f0) == 0x00000090)
	{
		cycles += 4;
	}
	return cycles;
}


void arm7_cpu_device::generate_slice_check(drcuml_block &block, compiler_state &compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	// an exact copy charges as it goes, and a one-instruction sequence charges it and leaves anyway
	if (compiler.exact || seqhead == seqlast || seqhead->compiler_page_fault())
	{
		return;
	}

	// an upper bound: a failed condition costs less, and the multiplies are counted at their largest
	uint32_t bound = 0;
	for (const opcode_desc *curdesc = seqhead; curdesc != seqlast->next(); curdesc = curdesc->next())
	{
		bound += instruction_cycles_max(curdesc);
	}
	UML_CMP(block, mem(&m_core->icount), bound);                                // cmp     icount,bound
	UML_EXHc(block, uml::COND_LE, *m_slice_exact, seqhead->pc);                 // exh     slice_exact,pc,le
}


// charges the cycles so far without leaving, so a memory access sees the interpreter's time
void arm7_cpu_device::generate_flush_cycles(drcuml_block &block, compiler_state &compiler)
{
	if (compiler.cycles > 0)
	{
		UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), compiler.cycles);        // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                    // mapvar  cycles,0
		if (compiler.run_end != nullptr)
		{
			compiler.run_flushed += compiler.cycles;    // a skipped run jumps over this
		}
	}
	compiler.cycles = 0;
}


void arm7_cpu_device::generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param)
{
	if (compiler.cycles > 0)
	{
		UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), MAPVAR_CYCLES);          // sub     icount,icount,cycles
		UML_MAPVAR(block, MAPVAR_CYCLES, 0);                                    // mapvar  cycles,0
		UML_EXHc(block, uml::COND_LE, *m_out_of_cycles, param);                      // exh     out_of_cycles,param,le
	}
	compiler.cycles = 0;
}


void arm7_cpu_device::generate_validate_fetch(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	if (!desc->validate_tlb() || desc->compiler_page_fault())
	{
		return;
	}

	// with the control register compiled in (drc_control_written()), a page fetched with the MMU off stays put
	const bool mmu_on = (m_control & COPRO_CTRL_MMU_EN) != 0;
	const bool static_control = drc_static_control();
	if (static_control && !mmu_on)
	{
		return;
	}

	const code_label done = compiler.labelnum++;
	const code_label abort = compiler.labelnum++;
	const vtlb_entry fetchbit = (GET_MODE == eARM7_MODE_USER) ? USER_FETCH_ALLOWED : FETCH_ALLOWED;
	const uint32_t expected = (desc->physpc & 0xfffff000) | fetchbit;

	if (!static_control)
	{
		UML_LOAD(block, I0, &m_control, 0, SIZE_DWORD, SCALE_x1);               // load    i0,[control],dword
		UML_TEST(block, I0, COPRO_CTRL_MMU_EN);                                 // test    i0,MMU_EN
		if (mmu_on)
		{
			// compiled with the MMU on: with it off now the page holds other code
			UML_EXHc(block, uml::COND_Z, *m_nocode, desc->pc);                  // exh     nocode,pc,z
		}
		else
		{
			// compiled with the MMU off: with it on now, the table has to say the page maps to itself
			UML_JMPc(block, uml::COND_Z, done);                                      // jmp     done,z
		}
	}

	// the table index is the process-relocated address
	const auto load_entry = [this, &block, desc, fetchbit] ()
	{
		if (desc->pc < 0x02000000)
		{
			UML_LOAD(block, I1, &m_pid_offset, 0, SIZE_DWORD, SCALE_x1);        // load    i1,[pid_offset],dword
			UML_ADD(block, I1, I1, desc->pc);                                   // add     i1,i1,pc
			UML_SHR(block, I1, I1, 12);                                         // shr     i1,i1,12
			UML_LOAD(block, I0, vtlb_table(), I1, SIZE_DWORD, SCALE_x4);        // load    i0,[vtlb_table],i1,dword
		}
		else
		{
			UML_LOAD(block, I0, &vtlb_table()[desc->pc >> 12], 0, SIZE_DWORD, SCALE_x4);   // load    i0,[entry],0,dword
		}
		UML_AND(block, I0, I0, 0xfffff000 | fetchbit);                          // and     i0,i0,page|fetch
	};

	load_entry();
	UML_CMP(block, I0, expected);                                               // cmp     i0,expected
	UML_JMPc(block, uml::COND_E, done);                                         // jmp     done,e

	// a stale or missing entry: translate again, then compare again (the abort it may take returns to R15)
	UML_MOV(block, mem(&m_r[eR15]), desc->pc);                                  // mov     r15,pc
	UML_MOV(block, mem(&m_core->arg0), desc->pc);                               // mov     arg0,pc
	UML_CALLC(block, cfunc_fetch_refill, this);                                 // callc   fetch_refill
	UML_TEST(block, mem(&m_core->result), ~uint32_t(0));                        // test    result,~0
	UML_JMPc(block, uml::COND_Z, abort);                                        // jmp     abort,z
	load_entry();
	UML_CMP(block, I0, expected);                                               // cmp     i0,expected
	UML_EXHc(block, uml::COND_NE, *m_nocode, desc->pc);                         // exh     nocode,pc,ne
	UML_JMP(block, done);                                                       // jmp     done

	// the fetch aborted: the cycles so far are owed, plus the interpreter's 3 for the instruction
	UML_LABEL(block, abort);                                                    // abort:
	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), compiler.cycles + 3);        // sub     icount,icount,cycles+3
	UML_EXHc(block, uml::COND_LE, *m_out_of_cycles, mem(&m_r[eR15]));           // exh     out_of_cycles,[r15],le
	generate_hashjmp_dynamic(block);                                            // hashjmp <mode>,<r15>,nocode_miss

	UML_LABEL(block, done);                                                     // done:
}


void arm7_cpu_device::generate_checksum_block(drcuml_block &block, compiler_state &compiler, const opcode_desc *seqhead, const opcode_desc *seqlast)
{
	compiler.verified = ~offs_t(0);
	compiler.verify_next = nullptr;
	if (seqhead->compiler_page_fault())
	{
		return;
	}
	if (m_drcuml.logging())
	{
		block.append_comment("[Validation for %08X]", seqhead->pc);             // comment
	}
	if ((m_drcoptions & DRC_STRICT_VERIFY) && (m_drc_opt & DRC_OPT_SUMS))
	{
		generate_verify_chunk(block, compiler, seqhead, seqlast);
	}
	else
	{
		generate_verify_word(block, compiler, seqhead);
	}
}


// checks the words up to and including the first store or loop branch; the rest are checked after it
void arm7_cpu_device::generate_verify_chunk(drcuml_block &block, compiler_state &compiler, const opcode_desc *first, const opcode_desc *seqlast)
{
	// a loader writing just past itself would otherwise fail the check on every turn of its loop
	const auto loop_end = [] (const opcode_desc *desc)
	{
		return desc->is_conditional_branch() && desc->targetpc != BRANCH_TARGET_DYNAMIC && desc->targetpc <= desc->pc;
	};

	// each word once (two Thumb instructions share one); a page that couldn't be fetched ends the sequence
	std::vector<std::pair<offs_t, uint32_t>> words;
	const opcode_desc *last = first;
	for (const opcode_desc *desc = first; desc != nullptr && !desc->compiler_page_fault(); desc = desc->next())
	{
		const offs_t waddr = desc->physpc & ~offs_t(3);
		if (m_drc_volatile.count(desc->pc))
		{
			// fetched as it runs: nothing to check (it ends its sequence)
		}
		else if (words.empty() || words.back().first != waddr)
		{
			words.emplace_back(waddr, desc->opword);
		}
		last = desc;
		if (desc == seqlast || desc->writes_memory() || loop_end(desc))
		{
			break;
		}
	}
	compiler.verify_next = (last != seqlast && (last->writes_memory() || loop_end(last))) ? last->next() : nullptr;
	if (words.empty())
	{
		return;
	}

	// read in place or through the bus; a memory_bank switch isn't notified, but no ARM driver runs code from one
	std::vector<const uint8_t *> hosts;
	hosts.reserve(words.size());
	for (const auto &[waddr, word] : words)
	{
		const uint8_t *const host = reinterpret_cast<const uint8_t *>(insn_fetch_ptr(waddr));
		hosts.push_back(host);
		if (host == nullptr)
		{
			UML_READ(block, I0, waddr, SIZE_DWORD, SPACE_PROGRAM);              // read    i0,waddr,dword
			UML_CMP(block, I0, word);                                           // cmp     i0,word
			UML_EXHc(block, uml::COND_NE, *m_nocode_verify, first->pc);         // exh     nocode_verify,pc,ne
		}
	}

	// the rest summed, a 64-bit load for two words that lie together on an 8-byte boundary
	uint64_t sum = 0;
	uint32_t pieces = 0;
	bool wide = false;
	for (size_t i = 0; i < words.size(); )
	{
		if (hosts[i] == nullptr)
		{
			i++;
			continue;
		}
		const bool pair = (i + 1 < words.size()) && (hosts[i + 1] == hosts[i] + 4) && ((uintptr_t(hosts[i]) & 7) == 0);
		uint64_t expect = words[i].second;
		if (pair)
		{
			const uint64_t second = words[i + 1].second;
			expect = (std::endian::native == std::endian::little) ? (expect | (second << 32)) : ((expect << 32) | second);
			wide = true;
		}
		UML_DLOAD(block, (pieces == 0) ? I0 : I1, hosts[i], 0, pair ? SIZE_QWORD : SIZE_DWORD, SCALE_x1);    // dload   i0/i1,[words],0
		if (pieces != 0)
		{
			UML_DADD(block, I0, I0, I1);                                        // dadd    i0,i0,i1
		}
		sum += expect;
		pieces++;
		i += pair ? 2 : 1;
	}
	if (pieces == 0)
	{
		return;
	}
	if (pieces == 1 && !wide)
	{
		UML_CMP(block, I0, uint32_t(sum));                                      // cmp     i0,word
	}
	else
	{
		UML_DCMP(block, I0, sum);                                               // dcmp    i0,sum
	}
	UML_EXHc(block, uml::COND_NE, *m_nocode_verify, first->pc);                 // exh     nocode_verify,pc,ne
}


void arm7_cpu_device::generate_verify_word(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	// the other half of a word already verified, or volatile code, fetched as it runs
	const offs_t waddr = desc->physpc & ~offs_t(3);
	if (waddr == compiler.verified || m_drc_volatile.count(desc->pc))
	{
		return;
	}
	compiler.verified = waddr;

	// read in place or through the bus; a memory_bank switch isn't notified, but no ARM driver runs code from one
	const void *const host = insn_fetch_ptr(waddr);
	if (host)
	{
		UML_LOAD(block, I0, host, 0, SIZE_DWORD, SCALE_x4);                     // load    i0,[word],0,dword
	}
	else
	{
		UML_READ(block, I0, waddr, SIZE_DWORD, SPACE_PROGRAM);                  // read    i0,waddr,dword
	}
	UML_CMP(block, I0, desc->opword);                                           // cmp     i0,word
	UML_EXHc(block, uml::COND_NE, *m_nocode_verify, desc->pc);                  // exh     nocode_verify,pc,ne
}


void arm7_cpu_device::generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	if (m_drcuml.logging() && !desc->virtual_noop())
	{
		log_add_disasm_comment(block, desc->pc, desc->opptr);
	}

	if (desc->compiler_page_fault())
	{
		// nothing is owed if the block is replaced from here; the abort itself costs the 3 cycles of a fetch
		generate_update_cycles(block, compiler, desc->pc);
		generate_fetch_fault(block, desc);
		compiler.cycles += 3;
		UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles);                      // mapvar  CYCLES,compiler.cycles
		return;
	}

	// without DRC_OPT_SUMS each word is checked as it runs (the head's with the sequence)
	if ((m_drcoptions & DRC_STRICT_VERIFY) && !(m_drc_opt & DRC_OPT_SUMS))
	{
		generate_verify_word(block, compiler, desc);
	}

	// memory accesses and the interpreter see the time the cycles so far make
	const bool generated = drc_generates(*desc);
	if (!generated || desc->reads_memory() || desc->writes_memory())
	{
		generate_flush_cycles(block, compiler);
	}
	compiler.cycles += instruction_cycles(desc);
	UML_MAPVAR(block, MAPVAR_CYCLES, compiler.cycles);                          // mapvar  CYCLES,compiler.cycles

	// the interpreter, the debugger (with the 26-bit PSR) and memory handlers reading pc() take the address from R15
	if (debugger_enabled() && (m_core->mode & 1))
	{
		generate_r15_psr26(block, mem(&m_r[eR15]), desc->pc, I0);               // r15 = psr + pc
	}
	else if (!generated || debugger_enabled() || desc->reads_memory() || desc->writes_memory())
	{
		UML_MOV(block, mem(&m_r[eR15]), desc->pc);                              // mov     r15,pc
	}

	// the word the interpreter's prefetch fetches at PC+8 (set_fetch_shadow())
	if (desc->pc >= m_fetch_shadow_start && desc->pc <= m_fetch_shadow_end)
	{
		UML_MOV(block, mem(&m_core->fetch_shadow), insn_fetch_word((desc->physpc & ~offs_t(3)) + 8));   // mov     fetch_shadow,word
	}

	if (debugger_enabled())
	{
		UML_DEBUG(block, desc->pc);                                             // debug   pc
		if (generated)
		{
			generate_debug_guard(block, compiler, desc);
		}
	}

	if (!desc->virtual_noop())
	{
		if (!generated)
		{
			generate_fallback(block, desc);
		}
		else if (!generate_opcode(block, compiler, desc))
		{
			fatalerror("%s: no generator for %08X at %08X\n", tag(), desc->opptr, desc->pc);
		}
	}

	// an exact copy charges after every instruction as the interpreter does (the last with the sequence)
	if (compiler.exact && !desc->end_sequence())
	{
		generate_update_cycles(block, compiler, desc->pc + desc->length);
	}
}


// if the debugger hook moved R15 or changed the mode, the word runs through the interpreter instead
void arm7_cpu_device::generate_debug_guard(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc)
{
	const code_label same = compiler.labelnum++;
	const code_label changed = compiler.labelnum++;

	if (m_core->mode & 1)
	{
		UML_AND(block, I0, mem(&m_r[eR15]), 0x03fffffc);                        // and     i0,r15,pc_bits
		UML_CMP(block, I0, desc->pc);                                           // cmp     i0,pc
	}
	else
	{
		UML_CMP(block, mem(&m_r[eR15]), desc->pc);                              // cmp     r15,pc
	}
	UML_JMPc(block, uml::COND_NE, changed);                                     // jmp     changed,ne
	UML_CMP(block, mem(&m_core->mode), m_core->mode);                           // cmp     mode,<mode>
	UML_JMPc(block, uml::COND_E, same);                                         // jmp     same,e

	// the interpreter charges 3 cycles plus its own adjustments where the sequence has charged the static count
	UML_LABEL(block, changed);                                                  // changed:
	const int32_t adjust = int32_t(instruction_cycles(desc)) - 3;
	if (adjust != 0)
	{
		UML_ADD(block, mem(&m_core->icount), mem(&m_core->icount), adjust);     // add     icount,icount,adjust
	}
	generate_fallback(block, desc);
	UML_SUB(block, mem(&m_core->icount), mem(&m_core->icount), MAPVAR_CYCLES);  // sub     icount,icount,cycles
	UML_EXHc(block, uml::COND_LE, *m_out_of_cycles, mem(&m_r[eR15]));           // exh     out_of_cycles,<r15>,le
	generate_hashjmp_dynamic(block);                                            // hashjmp <mode>,<r15>,nocode_miss

	UML_LABEL(block, same);                                                     // same:
	if (m_core->mode & 1)
	{
		UML_AND(block, mem(&m_r[eR15]), mem(&m_r[eR15]), 0x03fffffc);           // and     r15,r15,pc_bits
	}
}


// the page didn't translate at compile time: take the prefetch abort, or recompile if it translates now
void arm7_cpu_device::generate_fetch_fault(drcuml_block &block, const opcode_desc *desc)
{
	UML_MOV(block, mem(&m_r[eR15]), desc->pc);                                  // mov     r15,pc
	UML_MOV(block, mem(&m_core->arg0), desc->pc);                               // mov     arg0,pc
	UML_CALLC(block, cfunc_fetch_fault, this);                                  // callc   fetch_fault
	UML_TEST(block, mem(&m_core->result), ~uint32_t(0));                        // test    result,~0
	UML_EXHc(block, uml::COND_NZ, *m_nocode, desc->pc);                         // exh     nocode,pc,nz
}


void arm7_cpu_device::generate_fallback(drcuml_block &block, const opcode_desc *desc)
{
	// R15 holds the address (generate_sequence_instruction()); the handler advances it
	if (m_drc_volatile.count(desc->pc))
	{
		UML_MOV(block, mem(&m_core->arg1), desc->physpc & ~offs_t(3));          // mov     arg1,address
		UML_CALLC(block, cfunc_execute_volatile, this);                         // callc   execute_volatile
		return;
	}
	UML_MOV(block, mem(&m_core->arg0), desc->opword);                           // mov     arg0,word
	UML_CALLC(block, cfunc_execute, this);                                      // callc   execute
}



/***************************************************************************
    LOGGING
***************************************************************************/

void arm7_cpu_device::log_add_disasm_comment(drcuml_block &block, uint32_t pc, uint32_t op)
{
	if (m_drcuml.logging())
	{
		arm7_disassembler dasm(this);
		opcode_buffer buffer(op);
		std::ostringstream stream;
		dasm.disassemble(stream, pc, buffer, buffer);
		block.append_comment("%08X: %s", pc, stream.str());                     // comment
	}
}


void arm7_cpu_device::log_opcode_desc(const opcode_desc *desclist)
{
	m_drcuml.log_printf("\nDescriptor list @ %08X\n", desclist->pc);

	for ( ; desclist != nullptr; desclist = desclist->next())
	{
		std::ostringstream buffer;
		if (desclist->virtual_noop())
		{
			buffer << "<virtual nop>";
		}
		else
		{
			arm7_disassembler dasm(this);
			opcode_buffer opcode(desclist->opptr);
			dasm.disassemble(buffer, desclist->pc, opcode, opcode);
		}

		std::string flags;
		if (desclist->is_unconditional_branch())
		{
			flags += 'U';
		}
		if (desclist->is_conditional_branch())
		{
			flags += 'C';
		}
		if (desclist->is_branch_target())
		{
			flags += 'T';
		}
		if (desclist->can_cause_exception())
		{
			flags += 'e';
		}
		if (desclist->will_cause_exception())
		{
			flags += 'E';
		}
		if (desclist->validate_tlb())
		{
			flags += 'V';
		}
		if (desclist->compiler_page_fault())
		{
			flags += 'P';
		}
		if (desclist->can_change_modes())
		{
			flags += 'M';
		}
		if (desclist->reads_memory())
		{
			flags += 'r';
		}
		if (desclist->writes_memory())
		{
			flags += 'w';
		}
		if (desclist->end_sequence())
		{
			flags += '.';
		}

		m_drcuml.log_printf("%08X [%08X] t:%08X c:%u f:%-8s %-40s use:%s mod:%s\n", desclist->pc, desclist->physpc, desclist->targetpc,
				desclist->cycles, flags, buffer.str(), log_register_list(desclist->regin), log_register_list(desclist->regout));

		if (desclist->end_sequence())
		{
			m_drcuml.log_printf("-----\n");
		}
	}
}


std::string arm7_cpu_device::log_register_list(const std::bitset<24> &reglist)
{
	std::ostringstream result;
	for (uint32_t regnum = 0; regnum < 16; regnum++)
	{
		if (reglist[regnum])
		{
			util::stream_format(result, "r%u ", regnum);
		}
	}
	if (reglist[opcode_desc::REG_FLAG_NZCV])
	{
		result << "nzcv ";
	}
	if (reglist[opcode_desc::REG_FLAG_CPSR])
	{
		result << "cpsr ";
	}
	std::string text = std::move(result).str();
	return text.empty() ? "-" : text;
}
