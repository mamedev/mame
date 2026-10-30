// license:BSD-3-Clause
// copyright-holders:AJR, David Haywood, R. Belmont
/***************************************************************************

    Tensilica Xtensa

    This core should be reasonably complete for XEA1 and XEA2, but some
    extensions are still missing.

***************************************************************************/

#include "emu.h"
#include "xtensa.h"
#include "xtensad.h"

#include "xtensa_helper.h"

#include <algorithm>
#include <bit>

#define LOG_UNHANDLED_OPS       (1U << 1)
#define LOG_UNHANDLED_CACHE_OPS (1U << 3)
#define LOG_UNHANDLED_SYNC_OPS  (1U << 4)
#define LOG_EXTREG_OPS          (1U << 8)
#define LOG_NAMED_REGS          (1U << 10)
#define LOG_TIMER_REGS          (1U << 11)

#define VERBOSE (LOG_UNHANDLED_OPS)
#include "logmacro.h"

// device type definitions
DEFINE_DEVICE_TYPE(XTENSA, xtensa_device, "xtensa", "Tensilica Xtensa core")

xtensa_device::xtensa_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cpu_device(mconfig, XTENSA, tag, owner, clock)
	, m_space_config("program", ENDIANNESS_LITTLE, 32, 32, 0)
	, m_extregs_config("extregs", ENDIANNESS_LITTLE, 32, 8, -2, address_map_constructor(FUNC(xtensa_device::ext_regs), this))
	, m_a_real{}
	, m_a{}
	, m_pc(0)
	, m_icount(0)
	, m_extreg_windowbase(0)
	, m_extreg_windowstart(0)
	, m_extreg_sar(0)
	, m_extreg_scompare1(0)
	, m_extreg_lbeg(0)
	, m_extreg_lend(0)
	, m_extreg_lcount(0)
	, m_extreg_ps(0)
	, m_extreg_cacheattr(0)
	, m_extreg_epc{}
	, m_extreg_depc(0)
	, m_extreg_eps{}
	, m_extreg_excsave{}
	, m_extreg_ibreaka0(0)
	, m_extreg_dbreaka0(0)
	, m_extreg_dbreakc0(0)
	, m_extreg_icountlevel(0)
	, m_extreg_interrupt(0)
	, m_extreg_intenable(0)
	, m_extreg_exccause(0)
	, m_extreg_ccount(0)
	, m_extreg_ccompare{}
	, m_nextpc(0)
	, m_lcount_at_fetch(0)
	, m_waiting(false)
	, m_aborted(false)
	, m_num_physical_regs(64)
	, m_startupvector(0)
	, m_kernel_exception_vector(0)
	, m_user_exception_vector(0)
	, m_double_exception_vector(0)
	, m_interrupt_vector{}
	, m_window_vector_base(0)
	, m_window_vectors_set(false)
	, m_excmlevel(1)
	, m_xea1(false)
	, m_level_mask{}
	, m_irq_soft_mask(0)
	, m_irq_level_sensitive_mask(0)
	, m_nlevel(1)
	, m_ncompare(0)
	, m_ps_write_mask(PS_WRITE_MASK)
	, m_loop_disable_mask(PS_EXCM)
	, m_windowbase_mask(15)
{
	std::fill(std::begin(m_irq_level), std::end(m_irq_level), 1);
	std::fill(std::begin(m_irq_type), std::end(m_irq_type), IRQ_LEVEL);
	std::fill(std::begin(m_timerint), std::end(m_timerint), -1);
}

std::unique_ptr<util::disasm_interface> xtensa_device::create_disassembler()
{
	return std::make_unique<xtensa_disassembler>();
}

device_memory_interface::space_config_vector xtensa_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_PROGRAM, &m_space_config),
		std::make_pair(AS_EXTREGS, &m_extregs_config)
	};
}

/* Special registers */

u32 xtensa_device::extreg_windowbase_r() { return m_extreg_windowbase; }
void xtensa_device::extreg_windowbase_w(u32 data) { LOGMASKED(LOG_NAMED_REGS, "WindowBase set to %08x\n", data); set_windowbase(data); }
u32 xtensa_device::extreg_windowstart_r() { return m_extreg_windowstart; }
void xtensa_device::extreg_windowstart_w(u32 data) { m_extreg_windowstart = data & util::make_bitmask<u32>(m_windowbase_mask + 1); LOGMASKED(LOG_NAMED_REGS, "WindowStart set to %08x\n", data); }

u32 xtensa_device::extreg_lbeg_r() { return m_extreg_lbeg; }
void xtensa_device::extreg_lbeg_w(u32 data) { m_extreg_lbeg = data; }
u32 xtensa_device::extreg_lend_r() { return m_extreg_lend; }
void xtensa_device::extreg_lend_w(u32 data) { m_extreg_lend = data; }
u32 xtensa_device::extreg_lcount_r() { return m_extreg_lcount; }
void xtensa_device::extreg_lcount_w(u32 data) { m_extreg_lcount = data; }

u32 xtensa_device::extreg_sar_r() { return m_extreg_sar; }
void xtensa_device::extreg_sar_w(u32 data) { m_extreg_sar = data & 0x3f; }
u32 xtensa_device::extreg_scompare1_r() { return m_extreg_scompare1; }
void xtensa_device::extreg_scompare1_w(u32 data) { m_extreg_scompare1 = data; }

u32 xtensa_device::extreg_ps_r() { return m_extreg_ps; }
void xtensa_device::extreg_ps_w(u32 data) { m_extreg_ps = data & m_ps_write_mask; LOGMASKED(LOG_NAMED_REGS, "PS set to %08x\n", m_extreg_ps); }

u32 xtensa_device::extreg_cacheattr_r() { return m_extreg_cacheattr; }
void xtensa_device::extreg_cacheattr_w(u32 data) { m_extreg_cacheattr = data; LOGMASKED(LOG_NAMED_REGS, "CACHEATTR set to %08x\n", data); }

/* Exceptions */

u32 xtensa_device::extreg_epc_r(offs_t offset) { return m_extreg_epc[offset + 1]; }
void xtensa_device::extreg_epc_w(offs_t offset, u32 data) { m_extreg_epc[offset + 1] = data; }
u32 xtensa_device::extreg_depc_r() { return m_extreg_depc; }
void xtensa_device::extreg_depc_w(u32 data) { m_extreg_depc = data; }
u32 xtensa_device::extreg_eps_r(offs_t offset) { return m_extreg_eps[offset + 2]; }
void xtensa_device::extreg_eps_w(offs_t offset, u32 data) { m_extreg_eps[offset + 2] = data & m_ps_write_mask; }
u32 xtensa_device::extreg_excsave_r(offs_t offset) { return m_extreg_excsave[offset + 1]; }
void xtensa_device::extreg_excsave_w(offs_t offset, u32 data) { m_extreg_excsave[offset + 1] = data; }
u32 xtensa_device::extreg_exccause_r() { return m_extreg_exccause; }
void xtensa_device::extreg_exccause_w(u32 data) { m_extreg_exccause = data & 0x3f; }

/* Interrupts */

u32 xtensa_device::extreg_interrupt_r() { return m_extreg_interrupt; }

void xtensa_device::extreg_intset_w(u32 data)
{
	// only software interrupts can be requested from software
	if (data & ~m_irq_soft_mask)
	{
		LOGMASKED(LOG_NAMED_REGS, "INTSET write %08x includes non-software interrupts (%08x), ignored\n", data, data & ~m_irq_soft_mask);
	}
	m_extreg_interrupt |= data & m_irq_soft_mask;
}

u32 xtensa_device::extreg_intclear_r() { return 0; } // no state of its own, reads as undefined

void xtensa_device::extreg_intclear_w(u32 data)
{
	// level-sensitive interrupts can only be cleared at the source
	m_extreg_interrupt &= ~(data & ~m_irq_level_sensitive_mask);
}

u32 xtensa_device::extreg_intenable_r() { return m_extreg_intenable; }
void xtensa_device::extreg_intenable_w(u32 data) { m_extreg_intenable = data; LOGMASKED(LOG_NAMED_REGS, "INTENABLE set to %08x\n", data); }

/* Debugging */

u32 xtensa_device::extreg_ibreaka0_r() { return m_extreg_ibreaka0; }
void xtensa_device::extreg_ibreaka0_w(u32 data) { m_extreg_ibreaka0 = data; }
u32 xtensa_device::extreg_dbreaka0_r() { return m_extreg_dbreaka0; }
void xtensa_device::extreg_dbreaka0_w(u32 data) { m_extreg_dbreaka0 = data; }
u32 xtensa_device::extreg_dbreakc0_r() { return m_extreg_dbreakc0; }
void xtensa_device::extreg_dbreakc0_w(u32 data) { m_extreg_dbreakc0 = data; }
u32 xtensa_device::extreg_icountlevel_r() { return m_extreg_icountlevel; }
void xtensa_device::extreg_icountlevel_w(u32 data) { m_extreg_icountlevel = data; }

/* Timer */

u32 xtensa_device::extreg_ccount_r() { return m_extreg_ccount; }
void xtensa_device::extreg_ccount_w(u32 data) { m_extreg_ccount = data; LOGMASKED(LOG_TIMER_REGS, "CCOUNT set to %08x\n", data); }
u32 xtensa_device::extreg_ccompare_r(offs_t offset) { return m_extreg_ccompare[offset]; }

void xtensa_device::extreg_ccompare_w(offs_t offset, u32 data)
{
	m_extreg_ccompare[offset] = data;
	LOGMASKED(LOG_TIMER_REGS, "CCOMPARE%d set to %08x (CCOUNT %08x)\n", offset, data, m_extreg_ccount);

	// writing CCOMPAREn clears a pending timer interrupt
	if (m_timerint[offset] >= 0)
	{
		m_extreg_interrupt &= ~(1U << m_timerint[offset]);
	}
}

void xtensa_device::ext_regs(address_map &map)
{
	// Loop Option (0-2)
	map(0x00, 0x00).rw(FUNC(xtensa_device::extreg_lbeg_r), FUNC(xtensa_device::extreg_lbeg_w));
	map(0x01, 0x01).rw(FUNC(xtensa_device::extreg_lend_r), FUNC(xtensa_device::extreg_lend_w));
	map(0x02, 0x02).rw(FUNC(xtensa_device::extreg_lcount_r), FUNC(xtensa_device::extreg_lcount_w));

	// Core Architecture (3)
	map(0x03, 0x03).rw(FUNC(xtensa_device::extreg_sar_r), FUNC(xtensa_device::extreg_sar_w));

	// Boolean Option (4)
	//map(0x04, 0x04) // "br",

	// Extended L32R Option (5)
	//map(0x05, 0x05) // "litbase",

	// Conditional Store Option (12)
	map(0x0c, 0x0c).rw(FUNC(xtensa_device::extreg_scompare1_r), FUNC(xtensa_device::extreg_scompare1_w));

	// MAC16 Option (16-17, 32-35)
	//map(0x10, 0x11) // "acclo", "acchi"
	//map(0x20, 0x23) // "m0".."m3"

	// Windowed Register Option (72-73)
	map(0x48, 0x48).rw(FUNC(xtensa_device::extreg_windowbase_r), FUNC(xtensa_device::extreg_windowbase_w));
	map(0x49, 0x49).rw(FUNC(xtensa_device::extreg_windowstart_r), FUNC(xtensa_device::extreg_windowstart_w));

	// MMU Option (83, 90-92), Trace Port Option (89)
	//map(0x53, 0x53) // "ptevaddr",
	//map(0x59, 0x59) // "mmid",
	//map(0x5a, 0x5c) // "rasid", "itlbcfg", "dtlbcfg"

	// Debug Option (96)
	//map(0x60, 0x60) // "ibreakenable",

	// XEA1 Only (98)
	map(0x62, 0x62).rw(FUNC(xtensa_device::extreg_cacheattr_r), FUNC(xtensa_device::extreg_cacheattr_w));

	// Conditional Store Option (99)
	//map(0x63, 0x63) // "atomctl",

	// Debug Option (104)
	//map(0x68, 0x68) // "ddr",

	// Memory ECC/Parity Option (106-111)
	//map(0x6a, 0x6f) // "mepc", "meps", "mesave", "mesr", "mecr", "mevaddr"

	// Debug Option (128-129, 144-145, 160-161)
	map(0x80, 0x80).rw(FUNC(xtensa_device::extreg_ibreaka0_r), FUNC(xtensa_device::extreg_ibreaka0_w));
	//map(0x81, 0x81) // "ibreaka1",
	map(0x90, 0x90).rw(FUNC(xtensa_device::extreg_dbreaka0_r), FUNC(xtensa_device::extreg_dbreaka0_w));
	//map(0x91, 0x91) // "dbreaka1",
	map(0xa0, 0xa0).rw(FUNC(xtensa_device::extreg_dbreakc0_r), FUNC(xtensa_device::extreg_dbreakc0_w));
	//map(0xa1, 0xa1) // "dbreakc1",

	// Exception Option (177), High-Priority Interrupt Option (178-183)
	map(0xb1, 0xb7).rw(FUNC(xtensa_device::extreg_epc_r), FUNC(xtensa_device::extreg_epc_w)); // "epc1".."epc7"

	// Exception Option (192)
	map(0xc0, 0xc0).rw(FUNC(xtensa_device::extreg_depc_r), FUNC(xtensa_device::extreg_depc_w));

	// High-Priority Interrupt Option (194-199)
	map(0xc2, 0xc7).rw(FUNC(xtensa_device::extreg_eps_r), FUNC(xtensa_device::extreg_eps_w)); // "eps2".."eps7"

	// Exception Option (209), High-Priority Interrupt Option (210-215)
	map(0xd1, 0xd7).rw(FUNC(xtensa_device::extreg_excsave_r), FUNC(xtensa_device::extreg_excsave_w)); // "excsave1".."excsave7"

	// Coprocessor Option (224)
	//map(0xe0, 0xe0) // "cpenable",

	// Interrupt Option (226-228)
	map(0xe2, 0xe2).rw(FUNC(xtensa_device::extreg_interrupt_r), FUNC(xtensa_device::extreg_intset_w)); // "interrupt" (read) / "intset" (write)
	map(0xe3, 0xe3).rw(FUNC(xtensa_device::extreg_intclear_r), FUNC(xtensa_device::extreg_intclear_w)); // "intclear"
	map(0xe4, 0xe4).rw(FUNC(xtensa_device::extreg_intenable_r), FUNC(xtensa_device::extreg_intenable_w)); // "intenable"

	// various options (230)
	map(0xe6, 0xe6).rw(FUNC(xtensa_device::extreg_ps_r), FUNC(xtensa_device::extreg_ps_w)); // "ps"

	// Relocatable Vector Option (231)
	//map(0xe7, 0xe7) // "vecbase",

	// Exception Option (232)
	map(0xe8, 0xe8).rw(FUNC(xtensa_device::extreg_exccause_r), FUNC(xtensa_device::extreg_exccause_w)); // "exccause"

	// Debug Option (233)
	//map(0xe9, 0xe9) // "debugcause",

	// Timer Interrupt Option (234)
	map(0xea, 0xea).rw(FUNC(xtensa_device::extreg_ccount_r), FUNC(xtensa_device::extreg_ccount_w)); // "ccount"

	// Processor ID Option (235)
	//map(0xeb, 0xeb) // "prid",

	// Debug Option (236-237)
	//map(0xec, 0xec) // "icount",
	map(0xed, 0xed).rw(FUNC(xtensa_device::extreg_icountlevel_r), FUNC(xtensa_device::extreg_icountlevel_w)); // "icountlevel"

	// Exception Option (238)
	//map(0xee, 0xee) // "excvaddr",

	// Timer Interrupt Option (240-242)
	map(0xf0, 0xf2).rw(FUNC(xtensa_device::extreg_ccompare_r), FUNC(xtensa_device::extreg_ccompare_w)); // "ccompare0".."ccompare2"

	// Miscellaneous Special Registers Option (244-247)
	//map(0xf4, 0xf7) // "misc0".."misc3"
}


void xtensa_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_space);

	if (m_num_physical_regs != 32 && m_num_physical_regs != 64)
	{
		fatalerror("%s: the number of address registers must be 32 or 64\n", tag());
	}
	m_windowbase_mask = (m_num_physical_regs / 4) - 1;

	std::fill(std::begin(m_level_mask), std::end(m_level_mask), 0);
	m_irq_soft_mask = 0;
	m_irq_level_sensitive_mask = 0;
	m_nlevel = 1;
	for (int i = 0; i < 32; i++)
	{
		const int level = std::clamp<int>(m_irq_level[i], 1, 7);
		m_level_mask[level] |= 1U << i;
		m_nlevel = std::max(m_nlevel, level);
		if (m_irq_type[i] == IRQ_SOFTWARE)
		{
			m_irq_soft_mask |= 1U << i;
		}
		if (m_irq_type[i] == IRQ_LEVEL)
		{
			m_irq_level_sensitive_mask |= 1U << i;
		}
	}
	m_ncompare = 0;
	for (int i = 0; i < 3; i++)
	{
		if (m_timerint[i] >= 0)
		{
			m_ncompare = i + 1;
		}
	}
	if (m_double_exception_vector == 0)
	{
		m_double_exception_vector = m_kernel_exception_vector;
	}
	m_ps_write_mask = m_xea1 ? PS_WRITE_MASK_XEA1 : PS_WRITE_MASK;
	m_loop_disable_mask = m_xea1 ? 0 : PS_EXCM; // XEA1 never disables loops (A.2.1)

	set_icountptr(m_icount);

	state_add(XTENSA_PC, "PC", m_pc);
	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(XTENSA_PS, "PS", m_extreg_ps).mask(m_ps_write_mask);
	state_add(XTENSA_WINDOW, "WinBase", m_extreg_windowbase);
	state_add(XTENSA_WINDOWSTART, "WinStart", m_extreg_windowstart);
	state_add(XTENSA_SAR, "SAR", m_extreg_sar).mask(0x3f);
	state_add(XTENSA_INTENABLE, "IntEnable", m_extreg_intenable);
	state_add(XTENSA_INTERRUPT, "Interrupt", m_extreg_interrupt);
	state_add(XTENSA_CCOUNT, "CCount", m_extreg_ccount);
	state_add(XTENSA_CCOMPARE0, "CCompare0", m_extreg_ccompare[0]);
	state_add(XTENSA_EXCCAUSE, "ExcCause", m_extreg_exccause).mask(0x3f);
	for (int i = 1; i <= 7; i++)
	{
		state_add(XTENSA_EPC1 + i - 1, string_format("EPC%d", i), m_extreg_epc[i]);
	}
	state_add(XTENSA_LOOPBEGIN, "LoopBegin", m_extreg_lbeg);
	state_add(XTENSA_LOOPEND, "LoopEnd", m_extreg_lend);
	state_add(XTENSA_LOOPCOUNT, "LoopCount", m_extreg_lcount);

	for (int i = 0; i < 16; i++)
		state_add(XTENSA_A0 + i, string_format("a%d", i), m_a[i]);

	save_item(NAME(m_a_real));
	save_item(NAME(m_a));
	save_item(NAME(m_pc));
	save_item(NAME(m_icount));
	save_item(NAME(m_extreg_windowbase));
	save_item(NAME(m_extreg_windowstart));
	save_item(NAME(m_extreg_sar));
	save_item(NAME(m_extreg_scompare1));
	save_item(NAME(m_extreg_lbeg));
	save_item(NAME(m_extreg_lend));
	save_item(NAME(m_extreg_lcount));
	save_item(NAME(m_extreg_ps));
	save_item(NAME(m_extreg_cacheattr));
	save_item(NAME(m_extreg_epc));
	save_item(NAME(m_extreg_depc));
	save_item(NAME(m_extreg_eps));
	save_item(NAME(m_extreg_excsave));
	save_item(NAME(m_extreg_ibreaka0));
	save_item(NAME(m_extreg_dbreaka0));
	save_item(NAME(m_extreg_dbreakc0));
	save_item(NAME(m_extreg_icountlevel));
	save_item(NAME(m_extreg_interrupt));
	save_item(NAME(m_extreg_intenable));
	save_item(NAME(m_extreg_exccause));
	save_item(NAME(m_extreg_ccount));
	save_item(NAME(m_extreg_ccompare));
	save_item(NAME(m_nextpc));
	save_item(NAME(m_waiting));
	save_item(NAME(m_aborted));
}

void xtensa_device::device_reset()
{
	// everything else is undefined after reset
	m_pc = m_startupvector;
	m_nextpc = m_startupvector;
	m_extreg_ps = m_xea1 ? PS_INTLEVEL_MASK : (PS_INTLEVEL_MASK | PS_EXCM); // 4.4.1.3
	m_extreg_cacheattr = 0x22222222; // 4.6.3.3
	m_extreg_lcount = 0; // 4.3.2.1
	m_waiting = false;
	m_aborted = false;
}

void xtensa_device::handle_reserved(u32 inst)
{
	logerror("%08x: %-8s0x%02X ; reserved\n", m_pc, "db", inst & 0xff);
	abort_instruction(EXCCAUSE_ILLEGAL);
}

u32 xtensa_device::get_reg(const u8 reg)
{
	return m_a[reg];
}

void xtensa_device::set_reg(const u8 reg, u32 value)
{
	m_a[reg] = value;
}

void xtensa_device::switch_windowbase(s32 change)
{
	for (int i=0;i<16;i++)
	{
		s32 realreg = (i + m_extreg_windowbase * 4) & (m_num_physical_regs-1);
		m_a_real[realreg] = m_a[i];
	}

	m_extreg_windowbase = (m_extreg_windowbase + change) & m_windowbase_mask;

	for (int i=0;i<16;i++)
	{
		s32 realreg = (i + m_extreg_windowbase * 4) & (m_num_physical_regs-1);
		m_a[i] = m_a_real[realreg];
	}
}

void xtensa_device::set_windowbase(u32 wb)
{
	switch_windowbase(s32(wb & m_windowbase_mask) - s32(m_extreg_windowbase));
}

void xtensa_device::set_ws_bit(u32 window, bool value)
{
	const u32 bit = 1U << (window & m_windowbase_mask);
	if (value)
		m_extreg_windowstart |= bit;
	else
		m_extreg_windowstart &= ~bit;
}

bool xtensa_device::cwoe() const
{
	// XEA1 has no EXCM and takes no window exceptions while INTLEVEL is non-zero
	if (m_xea1)
	{
		return (m_extreg_ps & (PS_WOE | PS_INTLEVEL_MASK)) == PS_WOE;
	}
	return (m_extreg_ps & (PS_WOE | PS_EXCM)) == PS_WOE;
}

void xtensa_device::raise_window_exception(u32 vector, u32 owb)
{
	// the instruction is retried after RFWO/RFWU
	m_extreg_lcount = m_lcount_at_fetch;
	m_extreg_ps = (m_extreg_ps & ~PS_OWB_MASK) | ((owb & 0xf) << 8);
	if (m_xea1)
	{
		m_extreg_ps = (m_extreg_ps & ~(PS_WOE | PS_INTLEVEL_MASK)) | 1; // A.2.2
	}
	else
	{
		m_extreg_ps |= PS_EXCM;
	}
	m_extreg_epc[1] = m_pc;
	m_pc = vector;
	m_aborted = true;
	m_waiting = false;
}

u8 xtensa_device::live_frame_above(u8 need) const
{
	// quads to the first live frame above the window, or 0
	const u32 wb = m_extreg_windowbase;
	if (ws_bit(wb + 1))
	{
		return 1;
	}
	if (need >= 2 && ws_bit(wb + 2))
	{
		return 2;
	}
	if (need >= 3 && ws_bit(wb + 3))
	{
		return 3;
	}
	return 0;
}

u32 xtensa_device::overflow_vector(u32 window) const
{
	// the frame size is given by where the next live frame starts
	return m_window_vector_base + (ws_bit(window + 1) ? 0x000 : ws_bit(window + 2) ? 0x080 : 0x100);
}

bool xtensa_device::window_check(u8 need)
{
	// need is the highest register quad an operand reaches, or the CALL/ENTRY increment
	if (!m_window_vectors_set || !cwoe())
	{
		return false;
	}

	const u8 n = live_frame_above(need);
	if (n == 0)
	{
		return false;
	}

	const u32 wb = m_extreg_windowbase;
	const u32 m = (wb + n) & m_windowbase_mask;
	raise_window_exception(overflow_vector(m), wb);
	set_windowbase(m);
	return true;
}

u8 xtensa_device::window_class(u32 inst, u8 op0) const
{
	const u8 r = BIT(inst, 14, 2);
	const u8 s = BIT(inst, 10, 2);
	const u8 t = BIT(inst, 6, 2);
	switch (op0)
	{
	case 0b0000: // QRST
		switch (BIT(inst, 16, 4))
		{
		case 0b0000: // RST0
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // ST0
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // SNM0: RET, RETW, JX, CALLXn
					if (BIT(inst, 4, 4) == 0b1010) // JX
					{
						return s;
					}
					if (BIT(inst, 4, 4) >= 0b1100) // CALLXn also needs n free quads
					{
						return std::max(s, u8(BIT(inst, 4, 2)));
					}
					return 0;

				case 0b0001: // MOVSP
					return std::max(s, t);

				case 0b0110: // RSIL
					return t;

				default:
					return 0;
				}

			case 0b0001: case 0b0010: case 0b0011: // AND, OR, XOR
			case 0b1000: case 0b1001: case 0b1010: case 0b1011: // ADD, ADDX2, ADDX4, ADDX8
			case 0b1100: case 0b1101: case 0b1110: case 0b1111: // SUB, SUBX2, SUBX4, SUBX8
				return std::max({r, s, t});

			case 0b0100: // ST1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: case 0b0001: case 0b0010: case 0b0011: // SSR, SSL, SSA8L, SSA8B
					return s;

				case 0b0110: case 0b0111: case 0b1110: case 0b1111: // RER, WER, NSA, NSAU
					return std::max(s, t);

				default:
					return 0;
				}

			case 0b0101: // TLB
				return std::max(s, t);

			case 0b0110: // RT0: NEG, ABS
				return std::max(r, t);

			default:
				return 0;
			}
		case 0b0001: // RST1
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: // SLLI
			case 0b1010: // SLL
				return std::max(r, s);

			case 0b0010: case 0b0011: // SRAI
			case 0b0100: // SRLI
			case 0b1001: // SRL
			case 0b1011: // SRA
				return std::max(r, t);

			case 0b0110: // XSR
				return t;

			case 0b0111: // ACCER
			case 0b1111: // IMP
				return std::max(s, t);

			default: // SRC, MUL16U, MUL16S
				return std::max({r, s, t});
			}
		case 0b0010: // RST2: boolean ops use no ARs, the rest are r, s, t
			return (BIT(inst, 20, 4) >= 0b1000) ? std::max({r, s, t}) : 0;

		case 0b0011: // RST3
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: // RSR, WSR
			case 0b1111: // WUR
				return t;

			case 0b0010: case 0b0011: // SEXT, CLAMPS
			case 0b1100: case 0b1101: // MOVF, MOVT
				return std::max(r, s);

			case 0b1110: // RUR
				return r;

			default:
				return std::max({r, s, t});
			}
		case 0b0100: case 0b0101: // EXTUI
			return std::max(r, t);

		case 0b1000: // LSCX
		case 0b1001: // LSC4: L32E, S32E
			return std::max(s, t);

		case 0b1010: case 0b1011: // FP0, FP1 (approximate)
			return std::max({r, s, t});

		default:
			return 0;
		}
	case 0b0001: // L32R
		return t;

	case 0b0010: // LSAI
		switch (BIT(inst, 12, 4))
		{
		case 0b0111: // CACHE
			return s;

		case 0b1010: // MOVI
			return t;

		default: // loads, stores, ADDI, ADDMI
			return std::max(s, t);
		}
	case 0b0011: // LSCI
		return s;

	case 0b0100: // MAC16
		return std::max(s, t);

	case 0b0101: // CALLN: the callee's window must be free
		return BIT(inst, 4, 2);

	case 0b0110: // SI
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // J
			return 0;

		case 0b11: // BI1
			if (BIT(inst, 6, 2) == 0b00) // ENTRY
			{
				return std::max(s, get_callinc());
			}

			if (BIT(inst, 6, 2) == 0b01 && BIT(inst, 12, 4) < 0b1000) // BF, BT
			{
				return 0;
			}
			return s;

		default: // BZ, BI0
			return s;
		}
	case 0b0111: // B
		return (BIT(inst, 13, 2) == 0b11) ? s : std::max(s, t); // BBCI/BBSI only use s

	case 0b1000: case 0b1001: // L32I.N, S32I.N
		return std::max(s, t);

	case 0b1010: // ADD.N
		return std::max({r, s, t});

	case 0b1011: // ADDI.N
		return std::max(r, s);

	case 0b1100: // MOVI.N, BEQZ.N, BNEZ.N
		return s;

	case 0b1101: // ST3
		return (BIT(inst, 12, 4) == 0b0000) ? std::max(s, t) : 0; // MOV.N

	default:
		return 0;
	}
}

u32 xtensa_device::get_mem32(u32 addr)
{
	//if (addr & 3)
	//  logerror("get_mem32 unaligned\n");

	return m_space.read_dword(addr & ~3);
}

void xtensa_device::set_mem32(u32 addr, u32 data)
{
	//if (addr & 3)
	//  logerror("set_mem32 unaligned\n");

	m_space.write_dword(addr &~ 3, data);
}

u8 xtensa_device::get_mem8(u32 addr)
{
	return m_space.read_byte(addr);
}

void xtensa_device::set_mem8(u32 addr, u8 data)
{
	m_space.write_byte(addr, data);
}

u16 xtensa_device::get_mem16(u32 addr)
{
	if (addr & 1)
		logerror("get_mem16 unaligned\n");

	return m_space.read_word(addr & ~1);
}

void xtensa_device::set_mem16(u32 addr, u16 data)
{
	if (addr & 1)
		logerror("set_mem16 unaligned\n");

	m_space.write_word(addr & ~1, data);
}

void xtensa_device::handle_retw()
{
	const u32 a0 = get_reg(0);
	const u8 n = (a0 >> 30) & 3;
	const u32 owb = m_extreg_windowbase;
	const u32 target = (m_pc & 0xc0000000) | (a0 & 0x3fffffff);

	if (n == 0)
	{
		LOGMASKED(LOG_UNHANDLED_OPS, "%08x: retw with no window increment in a0 (%08x)\n", m_pc, a0);
	}

	switch_windowbase(-n);
	if (!m_window_vectors_set || n == 0 || !cwoe() || ws_bit(m_extreg_windowbase))
	{
		// the caller's registers are still in the register file
		set_ws_bit(owb, false);
		m_nextpc = target;
	}
	else
	{
		// window underflow: the handler reloads the caller's frame and RFWU retries this RETW
		raise_window_exception(m_window_vector_base + 0x040 + (n - 1) * 0x080, owb);
	}
}

void xtensa_device::handle_bz(u32 inst)
{
	const u8 reg = BIT(inst, 8, 4);
	u32 addr = m_pc + 4 + util::sext(inst >> 12, 12);

	switch (BIT(inst, 6, 2))
	{
	case 0b00:// beqz
	{
		if (get_reg(reg) == 0)
		{
			m_nextpc = addr;
		}
		break;
	}
	case 0b01:// bnez
	{
		if (get_reg(reg) != 0)
		{
			m_nextpc = addr;
		}
		break;
	}
	case 0b10:// bltz
	{
		if (get_reg(reg) & 0x80000000)
		{
			m_nextpc = addr;
		}
		break;
	}
	case 0b11:// bgez
	{
		if (!(get_reg(reg) & 0x80000000))
		{
			m_nextpc = addr;
		}
		break;
	}
	break;
	}
}

void xtensa_device::getop_and_execute()
{
	u32 inst = m_cache.read_byte(m_pc);
	inst |= m_cache.read_byte(m_pc+1)<<8;

	const u8 op0 = BIT(inst, 0, 4);
	if (op0 < 0b1000)
	{
		inst |= u32(m_cache.read_byte(m_pc+2)) << 16;
		m_nextpc = m_pc + 3;
	}
	else
	{
		m_nextpc = m_pc + 2;
	}

	m_lcount_at_fetch = m_extreg_lcount;
	if (m_extreg_lcount != 0 && m_nextpc == m_extreg_lend && !(m_extreg_ps & m_loop_disable_mask))
	{
		m_extreg_lcount--;
		m_nextpc = m_extreg_lbeg;
	}

	// operands in a live frame above the window take a window overflow
	m_aborted = false;
	if (m_window_vectors_set && live_frame_above(3))
	{
		const u8 wneed = window_class(inst, op0);
		if (wneed != 0 && window_check(wneed))
		{
			return;
		}
	}

	switch (op0)
	{
	case 0b0000: // QRST
		switch (BIT(inst, 16, 4))
		{
		case 0b0000: // RST0
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // ST0
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // SNM0
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // ILL
						abort_instruction(EXCCAUSE_ILLEGAL);
						break;

					case 0b1000: // RET
					{
						m_nextpc = get_reg(0);
						break;
					}

					case 0b1001: // RETW (with Windowed Register Option)
					{
						handle_retw();
						break;
					}


					case 0b1010: // JX
					{
						const u8 reg = BIT(inst, 8, 4);
						m_nextpc = get_reg(reg);
						break;
					}

					case 0b1100: // CALLX0
					{
						const u8 reg = BIT(inst, 8, 4);
						const u32 next = get_reg(reg);
						set_reg(0, m_pc + 3);
						m_nextpc = next;
						break;
					}

					case 0b1101: // CALLX4 (with Windowed Register Option)
					case 0b1110: // CALLX8 (with Windowed Register Option)
					case 0b1111: // CALLX12 (with Windowed Register Option)
					{
						const u8 reg = BIT(inst, 8, 4);
						const u8 xval = BIT(inst, 4, 2);
						set_callinc(xval);
						const u32 next = get_reg(reg);
						set_reg(xval*4, ((m_pc + 3) & 0x3fffffff) | (xval << 30));
						m_nextpc = next;
						break;
					}

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0001: // MOVSP (with Windowed Register Option)
				{
					// the caller's frame must still be in the register file
					const u8 reg_t = BIT(inst, 4, 4);
					const u8 reg_s = BIT(inst, 8, 4);
					const u32 wb = m_extreg_windowbase;
					if (m_window_vectors_set && (!m_xea1 || (m_extreg_ps & PS_INTLEVEL_MASK) == 0) && !ws_bit(wb - 1) && !ws_bit(wb - 2) && !ws_bit(wb - 3))
					{
						abort_instruction(EXCCAUSE_ALLOCA);
						break;
					}
					set_reg(reg_t, get_reg(reg_s));
					break;
				}

				case 0b0010: // SYNC
					switch (BIT(inst, 4, 8))
					{
					case 0b00000000: // ISYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "isync\n");
						break;

					case 0b00000001: // RSYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "rsync\n");
						break;

					case 0b00000010: // ESYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "esync\n");
						break;

					case 0b00000011: // DSYNC
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "dsync\n");
						break;

					case 0b00001000: // EXCW (with Exception Option)
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "excw\n");
						break;

					case 0b00001100: // MEMW
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "memw\n");
						break;

					case 0b00001101: // EXTW (added in RA-2004.1)
						LOGMASKED(LOG_UNHANDLED_SYNC_OPS, "extw\n");
						break;

					case 0b00001111: // NOP (added in RA-2004.1; was assembly macro previously)
						// nothing
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0011: // RFEI
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // RFET
						switch (BIT(inst, 8, 4))
						{
						case 0b0000: // RFE (with Exception Option)
						{
							if (m_xea1)
							{
								m_extreg_ps &= ~PS_INTLEVEL_MASK; // A.2.7
							}
							else
							{
								m_extreg_ps &= ~PS_EXCM;
							}
							m_nextpc = m_extreg_epc[1];
							break;
						}

						case 0b0001: // RFUE (with Exception Option; XEA1 only)
						{
							if (m_xea1)
							{
								m_extreg_ps = (m_extreg_ps & ~PS_INTLEVEL_MASK) | PS_UM | PS_WOE;
								m_nextpc = m_extreg_epc[1];
							}
							else
							{
								handle_reserved(inst);
							}
							break;
						}

						case 0b0010: // RFDE (with Exception Option)
							m_nextpc = m_extreg_depc; // PS.EXCM stays set
							break;

						case 0b0100: // RFWO (with Windowed Register option)
						case 0b0101: // RFWU (with Windowed Register option)
						{
							// RFWO frees the spilled frame, RFWU marks the reloaded one live; then retry
							set_ws_bit(m_extreg_windowbase, BIT(inst, 8));
							if (m_xea1)
							{
								m_extreg_ps = (m_extreg_ps & ~PS_INTLEVEL_MASK) | PS_WOE; // A.2.9
							}
							else
							{
								m_extreg_ps &= ~PS_EXCM;
							}
							set_windowbase((m_extreg_ps & PS_OWB_MASK) >> 8);
							m_nextpc = m_extreg_epc[1];
							break;
						}

						default:
							handle_reserved(inst);
							break;
						}
						break;

					case 0b0001: // RFI (with High-Priority Interrupt Option)
					{
						const u8 level = BIT(inst, 8, 4);
						if (level >= 2 && level <= 7)
						{
							m_nextpc = m_extreg_epc[level];
							m_extreg_ps = m_extreg_eps[level];
						}
						else
						{
							LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d ; undefined level\n", "rfi", level);
						}
						break;
					}

					case 0b0010: // RFME (with Memory ECC/Parity Option)
						LOGMASKED(LOG_UNHANDLED_OPS, "rfme\n");
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				case 0b0100: // BREAK (with Debug Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d, %d\n", "break", BIT(inst, 8, 4), BIT(inst, 4, 4));
					break;

				case 0b0101: // SYSCALL (with Exception Option)
					abort_instruction(EXCCAUSE_SYSCALL); // the handler skips the SYSCALL
					break;

				case 0b0110: // RSIL (with Interrupt Option)
				{
					const u8 reg_t = BIT(inst, 4, 4);
					const u8 level = BIT(inst, 8, 4);
					set_reg(reg_t, m_extreg_ps);
					m_extreg_ps = (m_extreg_ps & ~PS_INTLEVEL_MASK) | level;
					break;
				}

				case 0b0111: // WAITI (with Interrupt Option)
				{
					const u8 level = BIT(inst, 8, 4);
					m_extreg_ps = (m_extreg_ps & ~PS_INTLEVEL_MASK) | level;
					m_waiting = true; // until an interrupt is taken
					break;
				}

				case 0b1000: // ANY4 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "any4", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1001: // ALL4 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "all4", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1010: // ANY8 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "any8", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1011: // ALL8 (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d\n", "all8", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0001: // AND
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				set_reg(dstreg, get_reg(reg_s) & get_reg(reg_t));
				break;
			}
			case 0b0010: // OR / MOV
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				set_reg(dstreg, get_reg(reg_s) | get_reg(reg_t));
			}
			break;

			case 0b0011: // XOR
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				set_reg(dstreg, get_reg(reg_s) ^ get_reg(reg_t));
				break;
			}

			case 0b0100: // ST1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // SSR - Set Shift Amount for Right Shift
				{
					const u8 srcreg = BIT(inst, 8, 4);
					m_extreg_sar = get_reg(srcreg) & 0x1f;
					break;
				}
				case 0b0001: // SSL - Set Shift Amount for Left Shift
				{
					const u8 srcreg = BIT(inst, 8, 4);
					m_extreg_sar = 32 - (get_reg(srcreg) & 0x1f);
					break;
				}
				case 0b0010: // SSA8L - Set Shift Amount for LE Byte Shift
				{
					const u8 srcreg = BIT(inst, 8, 4);
					m_extreg_sar = (get_reg(srcreg) & 0x3)<<3;
					break;
				}

				case 0b0011: // SSA8B - Set Shift Amount for BE Byte Shift
				{
					const u8 srcreg = BIT(inst, 8, 4);
					m_extreg_sar = 32 - ((get_reg(srcreg) & 0x3)<<3);
					break;
				}

				case 0b0100: // SSAI
				{
					const u8 imm = BIT(inst, 8, 4) + (inst & 0x000010);
					m_extreg_sar = imm;
					break;
				}

				case 0b0110: // RER - Read External Register
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", xtensa_helper::s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0111: // WER - Write External Register
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", xtensa_helper::s_st1_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1110: // NSA (with Miscellaneous Operations Option) - Normalization Shift Amount
				{
					// redundant sign bits; 31 for 0 and -1
					const u8 dstreg = BIT(inst, 4, 4);
					const u8 srcreg = BIT(inst, 8, 4);
					const u32 srcval = get_reg(srcreg);
					const u32 sign_extended = srcval ^ u32(s32(srcval) >> 31);
					set_reg(dstreg, (sign_extended == 0) ? 31 : std::countl_zero(sign_extended) - 1);
					break;
				}

				case 0b1111: // NSAU (with Miscellaneous Operations Option) - Normalization Shift Amount Unsigned
				{
					const u8 dstreg = BIT(inst, 4, 4);
					const u8 srcreg = BIT(inst, 8, 4);
					u32 result;
					const u32 srcval = get_reg(srcreg);

					if (srcval == 0)
					{
						result = 32;
					}
					else
					{
						result = std::countl_zero(srcval);
					}
					set_reg(dstreg, result);
					break;
				}

				case 0b1000: // ROTW (with Windowed Register Option)
				{
					const s8 imm = util::sext(inst >> 4, 4);
					switch_windowbase(imm);
					break;
				}

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0101: // TLB (with Region Translation Option or MMU Option)
				switch (BIT(inst, 12, 4))
				{
				case 0b0011: case 0b0101: case 0b0110: case 0b0111: // RITLB0, PITLB, WITLB, RITLB1
				case 0b1011: case 0b1101: case 0b1110: case 0b1111: // RDTLB0, PDTLB, WDTLB, RDTLB1
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", xtensa_helper::s_tlb_ops[BIT(inst, 12, 4)], BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0100: case 0b1100: // IITLB, IDTLB
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d\n", xtensa_helper::s_tlb_ops[BIT(inst, 12, 4)], BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b0110: // RT0
				switch (BIT(inst, 8, 4))
				{
				case 0b0000: // NEG
				{
					const u8 dstreg = BIT(inst, 12, 4);
					const u8 srcreg = BIT(inst, 4, 4);
					const u32 src = get_reg(srcreg);
					u32 result;
					result = (src ^ 0xffffffff) + 1;
					set_reg(dstreg, result);
					break;
				}

				case 0b0001: // ABS
				{
					const u8 dstreg = BIT(inst, 12, 4);
					const u8 srcreg = BIT(inst, 4, 4);
					const u32 src = get_reg(srcreg);
					u32 result;
					if (src & 0x80000000)
						result = 0x80000000 - (src & 0x7fffffff);
					else
						result = src;
					set_reg(dstreg, result);
					break;
				}
				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b1000:// ADD
			case 0b1001:// ADDX2
			case 0b1010:// ADDX4
			case 0b1011:// ADDX8
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				const u8 shift = BIT(inst, 20, 2);
				set_reg(dstreg, (get_reg(reg_s)<<shift)+get_reg(reg_t));
				break;
			}

			case 0b1100: // SUB
			case 0b1101: // SUBX2
			case 0b1110: // SUBX4
			case 0b1111: // SUBX8
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				const u8 shift = BIT(inst, 20, 2);
				set_reg(dstreg, (get_reg(reg_s)<<shift)-get_reg(reg_t));
				break;
			}

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0001: // RST1
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: // SLLI (shift count is 1..31) - Shift Left Logical Immediate
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 8, 4);
				const u16 shift = 32 - (BIT(inst, 4, 4) + (BIT(inst, 20) ? 16 : 0));
				if (shift == 32)
				{
					// undefined behavior
				}
				else
				{
					set_reg(dstreg, get_reg(srcreg) << shift);
				}
				break;
			}

			case 0b0010: // SRAI (shift count is 0..31) - Shift Right Arithmetic Immediate
			case 0b0011:
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 4, 4);
				const u8 amount = BIT(inst, 8, 4) + (BIT(inst, 20) ? 16 : 0);
				const u32 source = get_reg(srcreg);
				u32 result = source >> amount;
				if (source & 0x80000000)
					result |= 0xffffffff << (31 - (amount & 0x1f));

				set_reg(dstreg, result);
				break;
			}

			case 0b0100: // SRLI (shift count is 0..15) - Shift Right Logical Immediate
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 4, 4);
				const u8 amount = BIT(inst, 8, 4);
				set_reg(dstreg, get_reg(srcreg) >> amount);
				break;
			}

			case 0b0110: // XSR (added in T1040)
			{
				const u8 spcreg = BIT(inst, 8, 8);
				const u8 reg = BIT(inst, 4, 4);
				const u32 spcregval = space(AS_EXTREGS).read_dword(spcreg);
				const u32 regval = get_reg(reg);
				space(AS_EXTREGS).write_dword(spcreg, regval);
				set_reg(reg, spcregval);
				break;
			}
			case 0b0111: // ACCER (added in RC-2009.0)
				switch (BIT(inst, 20, 4))
				{
				case 0b0000: // RER
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "rer", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // WER
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "wer", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b1000: // SRC -  Shift Right Combined
			{
				const u8 reg_r = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				// AR[s] is the high word; SAR may be 0..32
				const u64 fullvalue = (u64(get_reg(reg_s)) << 32) | get_reg(reg_t);
				const u32 result = u32(fullvalue >> (m_extreg_sar & 0x3f));
				set_reg(reg_r, result);

				break;
			}

			case 0b1001: // SRL
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 4, 4);
				set_reg(dstreg, u32(u64(get_reg(srcreg)) >> (m_extreg_sar & 0x3f)));
				break;
			}
			case 0b1010: // SLL - Shift Left Logical
			{
				// SAR holds 32 minus the shift amount (see SSL)
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 8, 4);
				set_reg(dstreg, u32((u64(get_reg(srcreg)) << 32) >> (m_extreg_sar & 0x3f)));
				break;
			}

			case 0b1011: // SRA
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 4, 4);
				set_reg(dstreg, u32(s64(s32(get_reg(srcreg))) >> (m_extreg_sar & 0x3f)));
				break;
			}

			case 0b1100: // MUL16U (with 16-bit Integer Multiply Option)
			{
				const u8 reg_r = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				set_reg(reg_r, (get_reg(reg_s)&0xffff) * (get_reg(reg_t)&0xffff));
				break;
			}

			case 0b1101: // MUL16S (with 16-bit Integer Multiply Option)
			{
				const u8 reg_r = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				set_reg(reg_r, s16((get_reg(reg_s)&0xffff)) * s16((get_reg(reg_t)&0xffff)));
				break;
			}

			case 0b1111: // IMP (Implementation-Specific)
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: // LICT (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "lict", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0001: // SICT (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sict", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0010: // LICW (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "licw", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b0011: // SICW (with Instruction Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sicw", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1000: // LDCT (with Data Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "ldct", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1001: // SDCT (with Data Cache Test Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d\n", "sdct", BIT(inst, 4, 4), BIT(inst, 8, 4));
					break;

				case 0b1110: // RFDX (with On-Chip Debug)
					switch (BIT(inst, 4, 4))
					{
					case 0b0000: // RFDO
						LOGMASKED(LOG_UNHANDLED_OPS, "rfdo\n");
						break;

					case 0b0001: // RFDD
						LOGMASKED(LOG_UNHANDLED_OPS, "rfdd\n");
						break;

					default:
						handle_reserved(inst);
						break;
					}
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0010: // RST2
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011: case 0b0100: // ANDB, ANDBC, ORB, ORBC, XORB (with Boolean Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, b%d, b%d\n", xtensa_helper::s_rst2_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1010: case 0b1011: // MULL, MULUH, MULSH (with 32-bit Integer Multiply Option)
			case 0b1100: case 0b1101: case 0b1110: case 0b1111: // QUOU, QUOS, REMU, REMS (with 32-bit Integer Divide Option)
			{
				const u8 reg_r = BIT(inst, 12, 4);
				const u32 src_s = get_reg(BIT(inst, 8, 4));
				const u32 src_t = get_reg(BIT(inst, 4, 4));
				const u8 op2 = BIT(inst, 20, 4);
				if (op2 >= 0b1100 && src_t == 0)
				{
					abort_instruction(EXCCAUSE_DIVIDE_BY_ZERO);
					break;
				}
				// -2^31 / -1 wraps without an exception
				const bool overflow = (src_s == 0x80000000 && src_t == 0xffffffff);
				switch (op2)
				{
				case 0b1000: set_reg(reg_r, src_s * src_t); break;
				case 0b1010: set_reg(reg_r, u32((u64(src_s) * u64(src_t)) >> 32)); break;
				case 0b1011: set_reg(reg_r, u32(u64(s64(s32(src_s)) * s64(s32(src_t))) >> 32)); break;
				case 0b1100: set_reg(reg_r, src_s / src_t); break;
				case 0b1101: set_reg(reg_r, overflow ? 0x80000000 : u32(s32(src_s) / s32(src_t))); break;
				case 0b1110: set_reg(reg_r, src_s % src_t); break;
				case 0b1111: set_reg(reg_r, overflow ? 0 : u32(s32(src_s) % s32(src_t))); break;
				}
				break;
			}

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0011: // RST3
			switch (BIT(inst, 20, 4))
			{
			case 0b0000:// RSR
			{
				const u8 spcreg = BIT(inst, 8, 8);
				const u8 reg = BIT(inst, 4, 4);
				LOGMASKED(LOG_EXTREG_OPS, "%s.%-3d a%d\n", "rsr", xtensa_helper::special_reg(spcreg, BIT(inst, 20)), reg);
				set_reg(reg, space(AS_EXTREGS).read_dword(spcreg));
				break;
			}

			case 0b0001:// WSR
			{
				const u8 spcreg = BIT(inst, 8, 8);
				const u8 reg = BIT(inst, 4, 4);
				LOGMASKED(LOG_EXTREG_OPS, "%s.%-3d a%d\n", "wsr", xtensa_helper::special_reg(spcreg, BIT(inst, 20)), reg);
				space(AS_EXTREGS).write_dword(spcreg, get_reg(reg));
				break;
			}

			case 0b0010: // SEXT (with Miscellaneous Operations Option)
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 8, 4);
				const u8 bit = BIT(inst, 4, 4) + 7;
				set_reg(dstreg, util::sext(get_reg(srcreg), bit+1));
				break;
			}

			case 0b0011: // CLAMPS (with Miscellaneous Operations Option)
			{
				// saturate to a signed (imm+1)-bit value, imm = 7..22
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 srcreg = BIT(inst, 8, 4);
				const u8 bits = BIT(inst, 4, 4) + 7;
				const s32 src = s32(get_reg(srcreg));
				const s32 upper = (1 << bits) - 1;
				const s32 lower = -(1 << bits);
				set_reg(dstreg, u32(std::clamp(src, lower, upper)));
				break;
			}

			case 0b0100: // MIN (with Miscellaneous Operations Option)
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const s32 src_s = s32(get_reg(BIT(inst, 8, 4)));
				const s32 src_t = s32(get_reg(BIT(inst, 4, 4)));
				set_reg(dstreg, u32(std::min(src_s, src_t)));
				break;
			}

			case 0b0101: // MAX (with Miscellaneous Operations Option)
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const s32 src_s = s32(get_reg(BIT(inst, 8, 4)));
				const s32 src_t = s32(get_reg(BIT(inst, 4, 4)));
				set_reg(dstreg, u32(std::max(src_s, src_t)));
				break;
			}

			case 0b0110: // MINU (with Miscellaneous Operations Option) - Minimum Value Unsigned
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);

				if (get_reg(reg_s) < get_reg(reg_t))
					set_reg(dstreg, get_reg(reg_s));
				else
					set_reg(dstreg, get_reg(reg_t));

				break;
			}

			case 0b0111: // MAXU (with Miscellaneous Operations Option)
			{
				const u8 dstreg = BIT(inst, 12, 4);
				set_reg(dstreg, std::max(get_reg(BIT(inst, 8, 4)), get_reg(BIT(inst, 4, 4))));
				break;
			}

			case 0b1000: // MOVEQZ
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				if (get_reg(reg_t) == 0)
				{
					set_reg(dstreg, get_reg(reg_s));
				}
				break;
			}
			case 0b1001: // MOVNEZ
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				if (get_reg(reg_t) != 0)
				{
					set_reg(dstreg, get_reg(reg_s));
				}
				break;
			}

			case 0b1010: // MOVLTZ
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				if ((get_reg(reg_t) & 0x80000000))
				{
					set_reg(dstreg, get_reg(reg_s));
				}
				break;
			}

			case 0b1011: // MOVGEZ
			{
				const u8 dstreg = BIT(inst, 12, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				const u8 reg_t = BIT(inst, 4, 4);
				if (!(get_reg(reg_t) & 0x80000000))
				{
					set_reg(dstreg, get_reg(reg_s));
				}
				break;
			}

			case 0b1100: case 0b1101: // MOVF, MOVT (with Boolean Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, b%d\n", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1110: case 0b1111: // RUR, WUR (TODO: TIE user_register names)
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.u%-2d a%d\n", xtensa_helper::s_rst3_ops[BIT(inst, 20, 4)], BIT(inst, 4, 8), BIT(inst, 12, 4));
				break;
			}
			break;

		case 0b0100: case 0b0101: // EXTUI
		{
			const u8 dstreg = BIT(inst, 12, 4);
			const u8 srcreg = BIT(inst, 4, 4);
			const u8 shift = BIT(inst, 8, 4) + (BIT(inst, 16) ? 16 : 0);
			const u8 numbits = BIT(inst, 20, 4) + 1;
			set_reg(dstreg, (get_reg(srcreg) >> shift) & ((1 << numbits)-1));
			break;
		}

		case 0b0110: case 0b0111: // CUST0, CUST1
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8s0x%02X ; cust%d?\n", "db", inst & 0xff, BIT(inst, 16));
			m_nextpc = m_pc + 1;
			break;

		case 0b1000: // LSCX (with Floating-Point Coprocessor Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // LSX
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "lsx", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0001: // LSXU
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "lsxu", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0100: // SSX
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "ssx", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b0101: // SSXU
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, a%d\n", "ssxu", BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1001: // LSC4 (with Windowed Register Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: // L32E (with Windowed Register Option), offset -64..-4
			{
				const u8 reg_t = BIT(inst, 4, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				set_reg(reg_t, get_mem32(get_reg(reg_s) + BIT(inst, 12, 4) * 4 - 64));
				break;
			}

			case 0b0100: // S32E (with Windowed Register Option), offset -64..-4
			{
				const u8 reg_t = BIT(inst, 4, 4);
				const u8 reg_s = BIT(inst, 8, 4);
				set_mem32(get_reg(reg_s) + BIT(inst, 12, 4) * 4 - 64, get_reg(reg_t));
				break;
			}

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1010: // FP0 (with Floating-Point Coprocessor Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0100: case 0b0101: // ADD.S, SUB.S, MUL.S, MADD.S, MSUB.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d, f%d\n", xtensa_helper::s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1001: case 0b1010: case 0b1011: case 0b1110: // ROUND.S, TRUNC.S, FLOOR.S, CEIL.S, UTRUNC.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-7s a%d, f%d, %d\n", xtensa_helper::s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // FLOAT.S, UFLOAT.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-7s f%d, a%d, %d\n", xtensa_helper::s_fp0_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1111: // FP1OP
				switch (BIT(inst, 4, 4))
				{
				case 0b0000: // MOV.S
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d\n", "mov.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0001: // ABS.S
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d\n", "abs.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0100: // RFR
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, f%d\n", "rfr", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0101: // WFR
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d\n", "wfr", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				case 0b0110: // NEG.S
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d\n", "neg.s", BIT(inst, 12, 4), BIT(inst, 8, 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1011: // FP1 (with Floating-Point Option)
			switch (BIT(inst, 20, 4))
			{
			case 0b0001: case 0b0010: case 0b0011: case 0b0100: case 0b0101: case 0b0110: case 0b0111: // UN.S, OEQ.S, UEQ.S, OLT.S, ULT.S, OLE.S, ULE.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, f%d, f%d\n", xtensa_helper::s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1000: case 0b1001: case 0b1010: case 0b1011: // MOVEQZ.S, MOVNEZ.S, MOVLTZ.S, MOVGEZ.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d, a%d\n", xtensa_helper::s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			case 0b1100: case 0b1101: // MOVF.S, MOVT.S
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, f%d, b%d\n", xtensa_helper::s_fp1_ops[BIT(inst, 20, 4)], BIT(inst, 12, 4), BIT(inst, 8, 4), BIT(inst, 4, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0001: // L32R (virtual address is always aligned)
	{
		const u8 reg = BIT(inst, 4, 4);
		const u32 addr = (m_pc + 3 - 0x40000 + (inst >> 8) * 4) & 0xfffffffc;
		set_reg(reg, get_mem32(addr));
		break;
	}

	case 0b0010: // LSAI
		switch (BIT(inst, 12, 4))
		{
		case 0b0000: // L8UI
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16);
			set_reg(dstreg, get_mem8(get_reg(basereg) + imm));
			break;
		}

		case 0b0100: // S8I
		{
			const u8 reg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u8 offset = inst >> 16;
			const u32 addr = get_reg(basereg) + offset;
			set_mem8(addr, get_reg(reg)&0xff);
			break;
		}

		case 0b0001: // L16UI
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16) * 2;
			set_reg(dstreg, get_mem16(get_reg(basereg) + imm));
			break;
		}

		case 0b0101: // S16I
		{
			const u8 reg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u16 offset = (inst >> 16) * 2;
			const u32 addr = get_reg(basereg) + offset;
			set_mem16(addr, get_reg(reg)&0xffff);
			break;
		}

		case 0b1001: // L16SI
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16) * 2;
			u32 value = get_mem16(get_reg(basereg) + imm);
			if (value & 0x00008000)
				value |= 0xffff0000;
			set_reg(dstreg, value);
			break;
		}

		case 0b0010: // L32I
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16) * 4;
			set_reg(dstreg, get_mem32(get_reg(basereg) + imm));
			break;
		}

		case 0b0110: // S32I
		{
			const u8 srcreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16) * 4;
			set_mem32(get_reg(basereg) + imm, get_reg(srcreg));
			break;
		}

		case 0b1011: // L32AI (with Multiprocessor Synchronization Option)
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16) * 4;
			set_reg(dstreg, get_mem32(get_reg(basereg) + imm));
			break;
		}

		case 0b1111: // S32RI (with Multiprocessor Synchronization Option)
		{
			const u8 srcreg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 imm = (inst >> 16) * 4;
			set_mem32(get_reg(basereg) + imm, get_reg(srcreg));
			break;
		}

		case 0b1110: // S32C1I (with Conditional Store Option)
		{
			// store if memory matches SCOMPARE1; return the old value
			const u8 reg = BIT(inst, 4, 4);
			const u8 basereg = BIT(inst, 8, 4);
			const u32 addr = get_reg(basereg) + (inst >> 16) * 4;
			const u32 old = get_mem32(addr);
			if (old == m_extreg_scompare1)
			{
				set_mem32(addr, get_reg(reg));
			}
			set_reg(reg, old);
			break;
		}

		case 0b0111: // CACHE
			switch (BIT(inst, 4, 4))
			{
			case 0b0000: case 0b0001: case 0b0010: case 0b0011: // DPFR, DPFW, DPFRO, DPFWO (with Data Cache Option)
			case 0b0100: case 0b0101: case 0b0110: case 0b0111: // DHWB, DHWBI, DHI, DII (with Data Cache Option)
			case 0b1100: case 0b1110: case 0b1111: // IPF, IHI, III (with Instruction Cache Option)
				LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", xtensa_helper::s_cache_ops[BIT(inst, 4, 4)], BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 16) * 4));
				break;

			case 0b1000: // DCE (with Data Cache Option)
				switch (BIT(inst, 16, 4))
				{
				case 0b0000: // DPFL (with Data Cache Index Lock Option)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "dpfl", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0010: // DHU (with Data Cache Index Lock Option)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "dhu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0011: // DIU (with Data Cache Index Lock Option)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "diu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0100: // DIWB (added in T1050)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "diwb", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0101: // DIWBI (added in T1050)
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "diwbi", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;
				}
				break;

			case 0b1101: // ICE (with Instruction Cache Index Lock Option)
				switch (BIT(inst, 16, 4))
				{
				case 0b0000: // IPFL
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "ipfl", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0010: // IHU
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "ihu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				case 0b0011: // IIU
					LOGMASKED(LOG_UNHANDLED_CACHE_OPS, "%-8sa%d, %s\n", "iiu", BIT(inst, 8, 4), xtensa_helper::format_imm((inst >> 20) * 4));
					break;

				default:
					handle_reserved(inst);
					break;
				}
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		case 0b1010: // MOVI
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const s32 imm = util::sext((inst & 0x000f00) + (inst >> 16), 12);
			set_reg(dstreg, imm);
			break;
		}

		case 0b1100: // ADDI
		{
			const u8 reg_s = BIT(inst, 8, 4);
			const u8 reg_t = BIT(inst, 4, 4);
			const s32 imm = s8(u8(inst >> 16));
			set_reg(reg_t, get_reg(reg_s)+imm);
			break;
		}

		case 0b1101: // ADDMI
		{
			const u8 reg_s = BIT(inst, 8, 4);
			const u8 reg_t = BIT(inst, 4, 4);
			const s32 imm = s8(u8(inst >> 16)) * 256;
			set_reg(reg_t, get_reg(reg_s)+imm);
			break;
		}

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0011: // LSCI (with Floating-Point Coprocessor Option)
		if (BIT(inst, 12, 2) == 0)
		{
			// LSI, SSI, LSIU, SSIU
			LOGMASKED(LOG_UNHANDLED_OPS, "%-8sf%d, a%d, %s\n", xtensa_helper::s_lsci_ops[BIT(inst, 14, 2)], BIT(inst, 4, 4), BIT(inst, 8, 4), xtensa_helper::format_imm(BIT(inst, 16, 8) * 4));
			break;
		}
		else
		{
			handle_reserved(inst);
			break;
		}

	case 0b0100: // MAC16 (with MAC16 Option)
		switch (BIT(inst, 20, 4))
		{
		case 0b0000: case 0b0001: // MACID, MACCD
			if (BIT(inst, 18, 2) == 0b10)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.dd.%s.%s m%d, a%d, m%d, m%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)],
											xtensa_helper::s_mac16_half[BIT(inst, 16, 2)],
											BIT(inst, 20) ? "lddec" : "ldinc",
											BIT(inst, 12, 2), BIT(inst, 8, 4),
											BIT(inst, 14), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0100: case 0b0101: // MACIA, MACCA
			if (BIT(inst, 18, 2) == 0b10)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.da.%s.%s m%d, a%d, m%d, a%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)],
											xtensa_helper::s_mac16_half[BIT(inst, 16, 2)],
											BIT(inst, 20) ? "lddec" : "ldinc",
											BIT(inst, 12, 2), BIT(inst, 8, 4),
											BIT(inst, 14), BIT(inst, 4, 4));
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0010: // MACDD
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.dd.%s m%d, m%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 14), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0011: // MACAD
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.ad.%s a%d, m%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 8, 4), BIT(inst, 6) + 2);
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0110: // MACDA
			if (BIT(inst, 18, 2) != 0b00)
			{
				LOGMASKED(LOG_UNHANDLED_OPS, "%s.da.%s m%d, a%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 14), BIT(inst, 4, 4));
			}
			else
			{
				handle_reserved(inst);
				break;
			}
			break;

		case 0b0111: // MACAA
			LOGMASKED(LOG_UNHANDLED_OPS, "%s.aa.%s a%d, a%d\n", xtensa_helper::s_mac16_ops[BIT(inst, 18, 2)], xtensa_helper::s_mac16_half[BIT(inst, 16, 2)], BIT(inst, 8, 4), BIT(inst, 4, 4));
			break;

		case 0b1000: case 0b1001: // MACI, MACC
			switch (BIT(inst, 16, 4))
			{
			case 0b0000: // LDINC, LDDEC
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8sm%d, a%d\n", BIT(inst, 20) ? "lddec" : "ldinc", BIT(inst, 12, 2), BIT(inst, 8, 4));
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	case 0b0101: // CALLN (target address is always aligned)
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // CALL0
		{
			const u32 addr = (m_pc & 0xfffffffc) + 4 + util::sext(inst >> 6, 18) * 4;
			const u32 next = addr;
			set_reg(0, m_pc + 3);
			m_nextpc = next;
			break;
		}

		case 0b01: // CALL4 (with Windowed Register Option)
		case 0b10: // CALL8 (with Windowed Register Option)
		case 0b11: // CALL12 (with Windowed Register Option)
		{
			const u32 addr = (m_pc & 0xfffffffc) + 4 + util::sext(inst >> 6, 18) * 4;
			const u8 xval = BIT(inst, 4, 2);
			set_callinc(xval);
			const u32 next = addr;
			set_reg(xval*4, ((m_pc + 3) & 0x3fffffff) | (xval << 30));
			m_nextpc = next;
			break;
		}
		}
		break;

	case 0b0110: // SI
		switch (BIT(inst, 4, 2))
		{
		case 0b00: // J
		{
			const u32 newpc = m_pc + 4 + util::sext(inst >> 6, 18);
			m_nextpc = newpc;
			break;
		}

		case 0b01: // BZ
			handle_bz(inst);
			break;

		case 0b10: // BI0
		{
			const u8 reg = BIT(inst, 8, 4);
			const u32 imm = xtensa_helper::s_b4const[BIT(inst, 12, 4)];
			const u32 addr = m_pc + 4 + s8(u8(inst >> 16));
			const u8 optype = BIT(inst, 6, 2);
			switch (optype)
			{
			case 0b00: // beqi
			{
				if (imm == get_reg(reg))
				{
					m_nextpc = addr;
				}
				break;
			}
			case 0b01: // bnei
			{
				if (imm != get_reg(reg))
				{
					m_nextpc = addr;
				}
				break;
			}
			case 0b10: // blti
				if ((s32)get_reg(reg) < (s32)imm)
				{
					m_nextpc = addr;
				}
				break;
			case 0b11: // bgei
				if ((s32)get_reg(reg) >= (s32)imm)
				{
					m_nextpc = addr;
				}
				break;
			}
			break;
		}

		case 0b11: // BI1
			switch (BIT(inst, 6, 2))
			{
			case 0b00: // ENTRY (window check done at fetch)
			{
				const u8 reg = BIT(inst, 8, 4);
				const u32 stacksize = (inst >> 12) << 3;
				const u32 stack = get_reg(reg) - stacksize; // read from the caller's window
				switch_windowbase(get_callinc());
				set_reg(reg, stack);
				set_ws_bit(m_extreg_windowbase, true);
				break;
			}

			case 0b01: // B1
				switch (BIT(inst, 12, 4))
				{
				case 0b0000: case 0b0001: // BF, BT (with Boolean Option)
					LOGMASKED(LOG_UNHANDLED_OPS, "%-8sb%d, 0x%08X\n", BIT(inst, 12) ? "bt" : "bf", BIT(inst, 8, 4), m_pc + 4 + s8(u8(inst >> 16)));
					break;

				case 0b1000: // LOOP (with Loop Option)
				{
					const u8 reg = BIT(inst, 8, 4);
					const u32 addr = m_pc + 4 + u8(inst >> 16); // offset is zero-extended
					m_extreg_lcount = get_reg(reg)-1;
					m_extreg_lbeg = m_pc + 3;
					m_extreg_lend = addr;
					break;
				}

				case 0b1001: // LOOPNEZ (with Loop Option)
				{
					const u8 reg = BIT(inst, 8, 4);
					const u32 addr = m_pc + 4 + u8(inst >> 16); // offset is zero-extended
					m_extreg_lcount = get_reg(reg)-1;
					m_extreg_lbeg = m_pc + 3;
					m_extreg_lend = addr;
					if (!get_reg(reg))
					{
						m_nextpc = m_extreg_lend;
					}
					break;
				}

				case 0b1010: // LOOPGTZ (with Loop Option)
				{
					const u8 reg = BIT(inst, 8, 4);
					const u32 addr = m_pc + 4 + u8(inst >> 16); // offset is zero-extended
					m_extreg_lcount = get_reg(reg)-1;
					m_extreg_lbeg = m_pc + 3;
					m_extreg_lend = addr;
					if (s32(get_reg(reg)) <= 0)
					{
						m_nextpc = m_extreg_lend;
					}
					break;
				}

				default:
					handle_reserved(inst);
					break;
				}
				break;

			case 0b10: // BLTUI - Branch if Less Than Unsigned Immediate
			{
				const u8 reg = BIT(inst, 8, 4);
				const u32 imm = xtensa_helper::s_b4constu[BIT(inst, 12, 4)];
				const u32 addr = m_pc + 4 + s8(u8(inst >> 16));
				if (get_reg(reg) < imm)
				{
					m_nextpc = addr;
				}
				break;
			}

			case 0b11: // BGEUI - Branch if Greater Than or Eq Unsigned Immediate
			{
				const u8 reg = BIT(inst, 8, 4);
				const u32 imm = xtensa_helper::s_b4constu[BIT(inst, 12, 4)];
				const u32 addr = m_pc + 4 + s8(u8(inst >> 16));
				if (get_reg(reg) >= imm)
				{
					m_nextpc = addr;
				}
				break;
			}
			}
			break;
		}
		break;

	case 0b0111: // B
		if (BIT(inst, 13, 2) == 0b11)
		{
			// BBCI, BBSI
			const u8 reg = BIT(inst, 8, 4);
			const u8 imm =  BIT(inst, 4, 4) + (BIT(inst, 12) ? 16 : 0);
			const u32 addr = m_pc + 4 + s8(u8(inst >> 16));
			if (BIT(inst, 15)) // BBSI
			{
				if ((BIT(get_reg(reg), imm)))
				{
					m_nextpc = addr;
				}
			}
			else // BBCI
			{
				if (!(BIT(get_reg(reg), imm)))
				{
					m_nextpc = addr;
				}
			}
			break;
		}
		else
		{
			// BNONE, BEQ, BLT, BLTU, BALL, BBC, BBCI, BANY, BNE, BGE, BGEU, BNALL, BBS
			const u8 as = BIT(inst, 8, 4);
			const u8 at = BIT(inst, 4, 4);
			const u32 addr = m_pc + 4 + s8(u8(inst >> 16));

			switch (BIT(inst, 12, 4))
			{
			case 0b0000:// bnone
				if (!(get_reg(as) & get_reg(at)))
				{
					m_nextpc = addr;
				}
				break;
			case 0b0001:// beq
				if ((get_reg(as) == get_reg(at)))
				{
					m_nextpc = addr;
				}
				break;
			case 0b0010:// blt
				if ((s32)get_reg(as) < (s32)get_reg(at))
				{
					m_nextpc = addr;
				}
				break;
			case 0b0011:// bltu
				if (get_reg(as) < get_reg(at))
				{
					m_nextpc = addr;
				}
				break;
			case 0b0100:// ball
				if (!(~get_reg(as) & get_reg(at)))
				{
					m_nextpc = addr;
				}
				break;
			case 0b0101:// bbc
				if (!(BIT(get_reg(as), get_reg(at)&0x1f)))
				{
					m_nextpc = addr;
				}

				break;
			//case 0b0110:// bbci
			//  LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbci", as, at, addr);
			//  break;
			//case 0b0111:// bbci
			//  LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbci", as, at, addr);
			//  break;
			case 0b1000:// bany
				if (get_reg(as) & get_reg(at))
				{
					m_nextpc = addr;
				}
				break;
			case 0b1001:// bne
				if ((get_reg(as) != get_reg(at)))
				{
					m_nextpc = addr;
				}
				break;
			case 0b1010:// bge
				if ((s32)get_reg(as) >= (s32)get_reg(at))
				{
					m_nextpc = addr;
				}
				break;
			case 0b1011:// bgeu
				if (get_reg(as) >= get_reg(at))
				{
					m_nextpc = addr;
				}
				break;
			case 0b1100:// bnall
				if (~get_reg(as) & get_reg(at))
				{
					m_nextpc = addr;
				}
				break;
			case 0b1101:// bbs
				if (BIT(get_reg(as), get_reg(at) & 0x1f))
				{
					m_nextpc = addr;
				}
				break;
			//case 0b1110:// bbsi
			//  LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbsi", as, at, addr);
			//  break;
			//case 0b1111:// bbsih
			//  LOGMASKED(LOG_UNHANDLED_OPS, "%-8sa%d, a%d, 0x%08X\n", "bbsih", as, at, addr);
			//  break;
			default:
				break;
			}
		}
		break;

	case 0b1000: // L32I.N (with Code Density Option)
	{
		const u8 dstreg = BIT(inst, 4, 4);
		const u8 basereg = BIT(inst, 8, 4);
		const u32 imm = BIT(inst, 12, 4) * 4;
		set_reg(dstreg, get_mem32(get_reg(basereg) + imm));
		break;
	}

	case 0b1001: // S32I.N (with Code Density Option)
	{
		const u8 srcreg = BIT(inst, 4, 4);
		const u8 basereg = BIT(inst, 8, 4);
		const u32 imm = BIT(inst, 12, 4) * 4;
		set_mem32(get_reg(basereg) + imm, get_reg(srcreg));
		break;
	}

	case 0b1010: // ADD.N (with Code Density Option)
	{
		const u8 dstreg = BIT(inst, 12, 4);
		const u8 reg_s = BIT(inst, 8, 4);
		const u8 reg_t = BIT(inst, 4, 4);
		set_reg(dstreg, get_reg(reg_s)+get_reg(reg_t));
		break;
	}

	case 0b1011: // ADDI.N (with Code Density Option)
	{
		const u8 dstreg = BIT(inst, 12, 4);
		const u8 srcreg = BIT(inst, 8, 4);
		const s32 imm = BIT(inst, 4, 4) == 0 ? -1 : int(BIT(inst, 4, 4));
		set_reg(dstreg, get_reg(srcreg)+imm);
		break;
	}

	case 0b1100: // ST2 (with Code Density Option)
		if (!BIT(inst, 7))
		{
			// 7-bit immediate field uses asymmetric sign extension (range is -32..95)
			const u8 dstreg = BIT(inst, 8, 4);
			const s32 imm = int((inst & 0x0070) + BIT(inst, 12, 4) - (BIT(inst, 5, 2) == 0b11 ? 128 : 0));
			set_reg(dstreg,imm);
			break;
		}
		else
		{
			if (BIT(inst, 6))
			{
				// 6-bit immediate field is zero-extended (these forms can branch forward only)
				const u8 reg = BIT(inst, 8, 4);
				const u32 addr = m_pc + 4 + (inst & 0x0030) + BIT(inst, 12, 4);
				if (get_reg(reg) != 0)
				{
					m_nextpc = addr;
				}
				break;
			}
			else
			{
				// 6-bit immediate field is zero-extended (these forms can branch forward only)
				const u8 reg = BIT(inst, 8, 4);
				const u32 addr = m_pc + 4 + (inst & 0x0030) + BIT(inst, 12, 4);
				if (get_reg(reg) == 0)
				{
					m_nextpc = addr;
				}
				break;
			}
			break;
		}

	case 0b1101: // ST3 (with Code Density Option)
		switch (BIT(inst, 12, 4))
		{
		case 0b0000: // MOV.N
		{
			const u8 dstreg = BIT(inst, 4, 4);
			const u8 srcreg = BIT(inst, 8, 4);
			set_reg(dstreg, get_reg(srcreg));
			break;
		}

		case 0b1111: // S3
			switch (BIT(inst, 4, 4))
			{
			case 0b0000: // RET.N
			{
				m_nextpc = get_reg(0);
				break;
			}

			case 0b0001: // RETW.N (with Windowed Register Option)
			{
				handle_retw();
				break;
			}

			case 0b0010: // BREAK.N (with Debug Option)
				LOGMASKED(LOG_UNHANDLED_OPS, "%-8s%d\n", "break.n", BIT(inst, 8, 4));
				break;

			case 0b0011: // NOP.N
				// nothing
				break;

			case 0b0110: // ILL.N
				abort_instruction(EXCCAUSE_ILLEGAL);
				break;

			default:
				handle_reserved(inst);
				break;
			}
			break;

		default:
			handle_reserved(inst);
			break;
		}
		break;

	default:
		handle_reserved(inst);
		break;
	}

	if (!m_aborted)
		m_pc = m_nextpc;
}

u32 xtensa_device::cintlevel() const
{
	// on XEA2 PS.EXCM masks everything up to EXCMLEVEL
	if (m_xea1)
	{
		return m_extreg_ps & PS_INTLEVEL_MASK;
	}
	return std::max<u32>((m_extreg_ps & PS_EXCM) ? m_excmlevel : 0, m_extreg_ps & PS_INTLEVEL_MASK);
}

void xtensa_device::acknowledge_irqs(u32 mask)
{
	for (u32 pending = mask; pending; pending &= pending - 1)
	{
		const int irq = std::countr_zero(pending);
		if (m_irq_type[irq] != IRQ_SOFTWARE && m_irq_type[irq] != IRQ_TIMER)
		{
			standard_irq_callback(irq, m_pc);
		}
	}
}

void xtensa_device::take_interrupt(int level, u32 mask)
{
	// high-priority interrupt (4.4.5.4)
	acknowledge_irqs(mask);

	m_extreg_epc[level] = m_pc;
	m_extreg_eps[level] = m_extreg_ps;
	m_extreg_ps = (m_extreg_ps & ~PS_INTLEVEL_MASK) | level | (m_xea1 ? 0 : PS_EXCM);
	m_pc = m_interrupt_vector[level];
	m_waiting = false;
}

void xtensa_device::abort_instruction(u8 cause)
{
	// the instruction has no side effects, including the loop-back
	m_extreg_lcount = m_lcount_at_fetch;
	m_aborted = true;
	take_exception(cause);
}

void xtensa_device::take_exception(u8 cause)
{
	// m_pc is the faulting instruction
	if (m_xea1)
	{
		// A.2.2: exception mode is PS.INTLEVEL = 1
		m_extreg_epc[1] = m_pc;
		m_extreg_ps = (m_extreg_ps & ~PS_INTLEVEL_MASK) | 1;
		m_extreg_exccause = cause;
		if (m_extreg_ps & PS_UM)
		{
			m_pc = m_user_exception_vector;
			m_extreg_ps &= ~(PS_UM | PS_WOE);
		}
		else
		{
			// kernel mode spills a live frame first; the exception recurs when RFWO retries (A.2.2)
			const u8 n = m_window_vectors_set ? live_frame_above(3) : 0;
			if (n != 0)
			{
				const u32 wb = m_extreg_windowbase;
				const u32 m = (wb + n) & m_windowbase_mask;
				m_extreg_ps = (m_extreg_ps & ~(PS_OWB_MASK | PS_WOE)) | ((wb & 0xf) << 8);
				m_pc = overflow_vector(m);
				set_windowbase(m);
			}
			else
			{
				m_pc = m_kernel_exception_vector;
			}
		}
	}
	else
	{
		// XEA2 (section 4.4.1.10)
		if (m_extreg_ps & PS_EXCM)
		{
			m_extreg_depc = m_pc;
			m_pc = m_double_exception_vector;
		}
		else
		{
			m_extreg_epc[1] = m_pc;
			m_pc = (m_extreg_ps & PS_UM) ? m_user_exception_vector : m_kernel_exception_vector;
		}
		m_extreg_exccause = cause;
		m_extreg_ps |= PS_EXCM;
	}
	m_waiting = false;
}

void xtensa_device::check_interrupts()
{
	const u32 pending = m_extreg_interrupt & m_extreg_intenable;
	if (!pending)
	{
		return;
	}

	// highest level first
	const u32 current = cintlevel();
	for (int level = m_nlevel; level >= 2; level--)
	{
		if (current < u32(level) && (pending & m_level_mask[level]))
		{
			take_interrupt(level, pending & m_level_mask[level]);
			return;
		}
	}

	if (current < 1 && (pending & m_level_mask[1]))
	{
		acknowledge_irqs(pending & m_level_mask[1]);
		take_exception(EXCCAUSE_LEVEL1_INTERRUPT);
	}
}

void xtensa_device::update_timers()
{
	// latched until CCOMPAREn is written
	for (int i = 0; i < m_ncompare; i++)
	{
		if (m_timerint[i] >= 0 && m_extreg_ccount == m_extreg_ccompare[i])
			m_extreg_interrupt |= 1U << m_timerint[i];
	}
}


void xtensa_device::execute_run()
{
	while (m_icount > 0)
	{
		if (m_waiting)
		{
			// WAITI: only CCOUNT advances
			debugger_wait_hook();
			m_extreg_ccount++;
			update_timers();
			check_interrupts();
			m_icount--;
			continue;
		}

		check_interrupts();

		debugger_instruction_hook(m_pc);

		m_extreg_ccount++;
		update_timers();

		getop_and_execute();
		m_icount--;
	}
}

bool xtensa_device::execute_input_edge_triggered(int inputnum) const noexcept
{
	return inputnum >= 0 && inputnum < 32 && (m_irq_type[inputnum] == IRQ_EDGE || m_irq_type[inputnum] == IRQ_WRITE_ERROR);
}

void xtensa_device::execute_set_input(int inputnum, int state)
{
	// input lines are numbered by INTERRUPT register bit
	if (inputnum < 0 || inputnum >= 32)
	{
		return;
	}

	const u32 bit = 1U << inputnum;
	switch (m_irq_type[inputnum])
	{
	case IRQ_LEVEL:
		if (state == CLEAR_LINE)
		{
			m_extreg_interrupt &= ~bit;
		}
		else
		{
			m_extreg_interrupt |= bit;
		}
		break;

	case IRQ_EDGE:
	case IRQ_WRITE_ERROR:
		// pending until WSR.INTCLEAR
		if (state != CLEAR_LINE)
		{
			m_extreg_interrupt |= bit;
		}
		break;

	default:
		logerror("interrupt %d has no input pin (software/timer type)\n", inputnum);
		break;
	}
}
