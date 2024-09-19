// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*****************************************************************************
 *
 *   i4004.cpp
 *
 *   Intel MCS-40 CPU family
 *
 *****************************************************************************/
#include "emu.h"
#include "mcs40.h"
#include "mcs40dasm.h"


/*
MCS-40 uses an unusual scheme for memory.  RAMs contain four registers,
each of which has 16 memory characters and 4 status characters (all
characters are 4 bits wide).  We represent memory and status as separate
address spaces, storing one nybble per byte.

I/O is similarly unusual.  It's assumed that there's one 4-bit I/O port
per 256 bytes of ROM.  The upper four bits of RC select the ROM I/O port
for WRR and RDR instructions (along with the selected ROM bank for the
4040).  It's assumed that there's one output-only port per RAM.  The
upper two bits of RC along with the lower three bits of CR select the
RAM output port for WMP instructions.  This isn't too bad, but it's
complicated by the GPIO peripherals.  These chips respond to WRR/RDR,
but can be wired to the CM-RAM lines, so they can be selected by the
combination of the lower three bits of CR along with the upper four bits
of RC.  On top of this, the 4289 latches the entire RC value on its A
outputs at X1, allowing for a flat 8-bit I/O space using the WRR/RDR
instructions, as well as having CM lines for device selection.  This
means we need 12 bits to represent the entire range of possibilities
using the WRR/RDR instructions.

The WRR/RDR instructions operate on a flat 11- or 12-bit address space,
depending on whether the CPU has ROM banking support.  You can use
AM_MIRROR to mask out unused chip select lines, and then shift the
offset to mask out unused RC bits.

       CR     RC
4001: B--- RRRR----
4207: BCCC 11PP----
4209: BCCC 11PP----
4211: BCCC 11PP----
4289: B--- AAAAAAAA
4308: B--- RRPP----

The WMP instruction operates on a 5-bit address space - three low bits
of CR and two high bits of RC.

The "program memory" space is separate from the instruction, I/O and
opcode spaces.  It's accessed via a 4008/4009 pair, or a 4289.  With a
4004, or a 4040 with a 4008/4009 pair, this space is write-only; read
support requires a 4040 with a 4289.  Accesses are 4 bits wide.  The
address consists of the 8-bit value latched with the SRC instruction and
a first/last bit that toggles on each program memory operation.  There's
no way for the CPU to get the state of the first/last bit (even using
additional I/O to read it is difficult because it's only output during
program memory reads and writes), so the developer has to be very
careful to always do program memory operations in pairs or track the
current state.  The only way to set it to a fixed value is to reset the
4008 or 4289.  The original intention was to use this for program memory
write-back, using the RC value as the address and the first/last signal
as nybble lane select.

TODO: 4040 interrupt support (including BBS, EIN, DIN instructions)
*/


DEFINE_DEVICE_TYPE(I4004, i4004_cpu_device, "i4004", "Intel 4004")
DEFINE_DEVICE_TYPE(I4040, i4040_cpu_device, "i4040", "Intel 4040")


ALLOW_SAVE_TYPE(mcs40_cpu_device_base::cycle);
ALLOW_SAVE_TYPE(mcs40_cpu_device_base::pmem);
ALLOW_SAVE_TYPE(mcs40_cpu_device_base::phase);


static constexpr u8 f_cm_ram_table[8] = { 0x0eU, 0x0dU, 0x0bU, 0x07U, 0x09U, 0x05U, 0x03U, 0x01U };


mcs40_cpu_device_base::mcs40_cpu_device_base(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		u32 clock,
		bool extended_cm,
		unsigned rom_width,
		unsigned stack_ptr_mask,
		unsigned index_reg_cnt,
		unsigned cr_mask)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_space_config{
			{ "rom",     ENDIANNESS_LITTLE, 8, u8(rom_width),     0 },
			{ "ram",     ENDIANNESS_LITTLE, 8, u8(11),            0 },
			{ "romport", ENDIANNESS_LITTLE, 8, u8(rom_width - 1), 0 },
			{ "unused",  ENDIANNESS_LITTLE, 8, u8(0),             0 },
			{ "status",  ENDIANNESS_LITTLE, 8, u8(9),             0 },
			{ "ramport", ENDIANNESS_LITTLE, 8, u8(5),             0 },
			{ "program", ENDIANNESS_LITTLE, 8, u8(rom_width - 3), 0 }, }
	, m_spaces{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr }
	, m_bus_cycle_cb(*this)
	, m_sync_cb(*this)
	, m_cm_rom_cb(*this)
	, m_cm_ram_cb(*this)
	, m_cy_cb(*this), m_stp_ack_cb(*this)
	, m_4289_pm_cb(*this), m_4289_f_l_cb(*this)
	, m_extended_cm(extended_cm)
	, m_stack_ptr_mask(stack_ptr_mask), m_index_reg_cnt(index_reg_cnt), m_cr_mask(cr_mask)
	, m_pc_mask((1U << rom_width) - 1)
	, m_icount(0), m_phase(phase::A1), m_cycle(cycle::OP), m_io_pending(false), m_program_op(pmem::NONE)
	, m_stop_latch(false), m_stop_ff(false), m_decoded_halt(false), m_resume(false)
	, m_rom_bank(0U), m_rom_addr(0U), m_opr(0U), m_opa(0U), m_arg(0U), m_4289_first(false)
	, m_a(0U), m_c(0U)
	, m_addr_stack(), m_stack_ptr(0U)
	, m_index_regs(), m_index_reg_bank(0U)
	, m_cr(0U), m_pending_cr3(0U), m_latched_rc(0U), m_new_rc(0U), m_src(0U), m_rc_pending(false)
	, m_test(CLEAR_LINE), m_stp(CLEAR_LINE)
	, m_cm_rom(0U), m_cm_ram(0U), m_cy(0U), m_4289_a(0U), m_4289_c(0U), m_4289_pm(0U), m_4289_f_l(0U)
	, m_index_reg_halves(), m_pc(0U), m_pcbase(0U), m_genflags(0U)
{
	assert(!((1U + stack_ptr_mask) & stack_ptr_mask));
	assert((16U == index_reg_cnt) || (24U == index_reg_cnt));
}


/***********************************************************************
    device_t implementation
***********************************************************************/

void mcs40_cpu_device_base::device_start()
{
	set_icountptr(m_icount);

	m_spaces[AS_ROM]            = &space(AS_ROM);
	m_spaces[AS_RAM_MEMORY]     = &space(AS_RAM_MEMORY);
	m_spaces[AS_ROM_PORTS]      = &space(AS_ROM_PORTS);
	m_spaces[AS_RAM_STATUS]     = &space(AS_RAM_STATUS);
	m_spaces[AS_RAM_PORTS]      = &space(AS_RAM_PORTS);
	m_spaces[AS_PROGRAM_MEMORY] = &space(AS_PROGRAM_MEMORY);
	m_spaces[AS_ROM]->cache(m_cache);

	m_bus_cycle_cb.resolve_safe();

	m_stop_latch = m_decoded_halt = m_resume = false;

	m_rom_addr = 0U;
	m_opr = m_opa = m_arg = 0U;

	m_a = m_c = 0U;

	m_addr_stack.reset(new u16[m_stack_ptr_mask + 1]);
	std::fill(&m_addr_stack[0], &m_addr_stack[m_stack_ptr_mask + 1], 0U);
	m_stack_ptr = 0U;

	m_index_regs.reset(new u8[m_index_reg_cnt >> 1]);
	std::fill(&m_index_regs[0], &m_index_regs[m_index_reg_cnt >> 1], 0U);

	m_latched_rc = m_new_rc = m_src = 0U;

	m_test = CLEAR_LINE;
	m_stp = CLEAR_LINE;
	m_cm_rom = 0x03U;
	m_cm_ram = 0x0fU;
	m_cy = 0x00U;
	m_4289_a = 0xffU;
	m_4289_c = 0x0fU;
	m_4289_pm = 0x01U;
	m_4289_f_l = 0x01U;

	m_index_reg_halves.reset(new u8[m_index_reg_cnt]);
	m_pc = m_pcbase = 0U;
	m_genflags = 0U;

	state_add(STATE_GENPC, "PC", m_pc).mask(m_pc_mask).callimport().callexport();
	state_add(STATE_GENPCBASE, "CURPC", m_pcbase).mask(m_pc_mask).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_genflags).mask(0x07U).noshow().callimport().callexport().formatstr("%4s");
	state_add(I4004_A, "A", m_a).mask(0x0fU);
	for (unsigned i = 0; (m_index_reg_cnt >> 1) > i; ++i)
	{
		state_add(
				I4004_R01 + i,
				string_format("R%XR%X%s", (i << 1) & 0x0fU, ((i << 1) + 1) & 0x0fU, BIT(i, 3) ? "*" : "").c_str(),
				m_index_regs[i]);
	}
	for (unsigned i = 0; m_index_reg_cnt > i; ++i)
	{
		state_add(
				I4004_R0 + i,
				string_format("R%X%s", i & 0x0fU, BIT(i, 4) ? "*" : "").c_str(),
				m_index_reg_halves[i]).mask(0x0fU).noshow().callimport().callexport();
	}
	state_add(I4004_SP, "SP", m_stack_ptr).mask(m_stack_ptr_mask);
	for (unsigned i = 0; m_stack_ptr_mask >= i; ++i)
		state_add(I4004_ADDR0 + i, string_format("ADDR%d", i).c_str(), m_addr_stack[i]).mask(0x0fff);
	state_add(I4004_CR, "CR", m_cr).mask(m_cr_mask);
	state_add(I4004_RC, "RC", m_latched_rc);
	state_add(I4004_RCN, "RC'", m_new_rc);
	// TODO: export SRC for 4040
	// TODO: export register bank for 4040

	save_item(NAME(m_phase));
	save_item(NAME(m_cycle));
	save_item(NAME(m_io_pending));
	save_item(NAME(m_program_op));
	save_item(NAME(m_stop_latch));
	save_item(NAME(m_stop_ff));
	save_item(NAME(m_decoded_halt));
	save_item(NAME(m_resume));
	save_item(NAME(m_rom_addr));
	save_item(NAME(m_opr));
	save_item(NAME(m_opa));
	save_item(NAME(m_arg));
	save_item(NAME(m_4289_first));
	save_item(NAME(m_a));
	save_item(NAME(m_c));
	save_pointer(NAME(m_addr_stack), m_stack_ptr_mask + 1);
	save_item(NAME(m_stack_ptr));
	save_pointer(NAME(m_index_regs), m_index_reg_cnt >> 1);
	save_item(NAME(m_index_reg_bank));
	save_item(NAME(m_cr));
	save_item(NAME(m_pending_cr3));
	save_item(NAME(m_latched_rc));
	save_item(NAME(m_new_rc));
	save_item(NAME(m_src));
	save_item(NAME(m_rc_pending));
	save_item(NAME(m_test));
	save_item(NAME(m_stp));
	save_item(NAME(m_cm_ram));
	save_item(NAME(m_cm_rom));
	save_item(NAME(m_cy));
	save_item(NAME(m_4289_a));
	save_item(NAME(m_4289_c));
	save_item(NAME(m_4289_pm));
	save_item(NAME(m_4289_f_l));
	save_item(NAME(m_pcbase));
}

void mcs40_cpu_device_base::device_reset()
{
	m_phase = phase::A1;
	m_cycle = cycle::OP;
	m_stop_ff = false;

	m_rom_addr = 0U;
	m_4289_first = true;
	pc() = 0U;

	m_c = 0U;

	m_index_reg_bank = 0U;

	m_cr = 0U;
	m_pending_cr3 = 0U;
	m_rc_pending = false;

	update_cm_rom(0x03U);
	update_cm_ram(0x0fU);
	m_stp_ack_cb(1U);

	// TODO: it actually takes multiple cycles with reset asserted for everything to get cleared
	m_a = m_c = 0U;
	std::fill(&m_addr_stack[0], &m_addr_stack[m_stack_ptr_mask + 1], 0U);
	std::fill(&m_index_regs[0], &m_index_regs[m_index_reg_cnt >> 1], 0U);
}


/***********************************************************************
    device_execute_interface implementation
***********************************************************************/

void mcs40_cpu_device_base::execute_run()
{
	while (m_icount > 0)
	{
		switch (m_phase)
		{
		case phase::A1:
			do_a1();
			m_phase = phase::A2;
			break;
		case phase::A2:
			do_a2();
			m_phase = phase::A3;
			break;
		case phase::A3:
			do_a3();
			m_phase = phase::M1;
			break;
		case phase::M1:
			do_m1();
			m_phase = phase::M2;
			break;
		case phase::M2:
			do_m2();
			m_phase = phase::X1;
			break;
		case phase::X1:
			do_x1();
			m_phase = phase::X2;
			break;
		case phase::X2:
			do_x2();
			m_phase = phase::X3;
			break;
		case phase::X3:
			do_x3();
			m_phase = phase::A1;
			break;
		}
		--m_icount;
	}
}


/***********************************************************************
    device_memory_interface implementation
***********************************************************************/

device_memory_interface::space_config_vector mcs40_cpu_device_base::memory_space_config() const
{
	return space_config_vector {
			std::make_pair(AS_ROM,            &m_space_config[AS_ROM]),
			std::make_pair(AS_RAM_MEMORY,     &m_space_config[AS_RAM_MEMORY]),
			std::make_pair(AS_ROM_PORTS,      &m_space_config[AS_ROM_PORTS]),
			std::make_pair(AS_RAM_STATUS,     &m_space_config[AS_RAM_STATUS]),
			std::make_pair(AS_RAM_PORTS,      &m_space_config[AS_RAM_PORTS]),
			std::make_pair(AS_PROGRAM_MEMORY, &m_space_config[AS_PROGRAM_MEMORY]) };
}


/***********************************************************************
    device_state_interface implementation
***********************************************************************/

void mcs40_cpu_device_base::state_import(device_state_entry const &entry)
{
	if ((I4004_R0 <= entry.index()) && (I4040_R23 >= entry.index()))
	{
		u8 const reg(entry.index() - I4004_R0), pair(reg >> 1), shift(BIT(~reg, 0) << 2), mask(0x0fU << shift);
		m_index_regs[pair] = (m_index_regs[pair] & ~mask) | ((m_index_reg_halves[reg] << shift) & mask);
	}
	else switch (entry.index())
	{
	case STATE_GENPC:
		pc() = m_pc_mask & m_pc & 0x0fffU;
		if (BIT(m_pc, 12))
			m_cr |= 0x08;
		else
			m_cr &= 0x07;
		m_rom_bank = m_pc_mask & m_pc & 0xf000U;
		if ((cycle::OP == m_cycle) && (phase::M1 > m_phase))
		{
			m_rom_addr = pc();
			m_pcbase = m_pc;
		}
		break;
	case STATE_GENFLAGS:
		m_stop_ff = BIT(m_genflags, 3);
		m_c = BIT(m_genflags, 1);
		m_test = BIT(m_genflags, 0) ? ASSERT_LINE : CLEAR_LINE;
		break;
	}
}

void mcs40_cpu_device_base::state_export(device_state_entry const &entry)
{
	if ((I4004_R0 <= entry.index()) && (I4040_R23 >= entry.index()))
	{
		u8 const reg(entry.index() - I4004_R0), pair(reg >> 1), shift(BIT(~reg, 0) << 2);
		m_index_reg_halves[reg] = (m_index_regs[pair] >> shift) & 0x0fU;
	}
	else switch (entry.index())
	{
	case STATE_GENPC:
		m_pc = rom_bank() | pc();
		break;
	case STATE_GENFLAGS:
		m_genflags =
				(m_stop_ff ? 0x08 : 0x00) |
				(m_a ? 0x00 : 0x04) |
				(m_c ? 0x02 : 0x00) |
				((CLEAR_LINE != m_test) ? 0x01 : 0x00);
		break;
	}
}

void mcs40_cpu_device_base::state_string_export(device_state_entry const &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = util::string_format(
				"%c%c%c%c",
				m_stop_ff ? 'S' : '.',
				m_a ? '.' : 'Z',
				m_c ? 'C' : '.',
				(CLEAR_LINE != m_test) ? 'T' : '.');
		break;
	}
}


/***********************************************************************
    register access
***********************************************************************/

inline u8 mcs40_cpu_device_base::get_a() const
{
	return m_a;
}

inline u8 mcs40_cpu_device_base::get_c() const
{
	return m_c;
}

inline void mcs40_cpu_device_base::set_a(u8 val)
{
	m_a = val & 0x0fU;
}

inline void mcs40_cpu_device_base::set_c(u8 val)
{
	m_c = val & 0x01U;
}

inline void mcs40_cpu_device_base::set_a_c(u8 val)
{
	m_a = val & 0x0fU;
	m_c = BIT(val, 4);
}

inline void mcs40_cpu_device_base::set_pc(u16 addr, u16 mask)
{
	set_rom_addr(pc() = (addr & mask) | (pc() & ~mask), 0x0fffU);
}

inline void mcs40_cpu_device_base::push_pc()
{
	m_stack_ptr = (m_stack_ptr + 1) & m_stack_ptr_mask;
}

inline void mcs40_cpu_device_base::pop_pc()
{
	m_stack_ptr = (m_stack_ptr - 1) & m_stack_ptr_mask;
	set_rom_addr(pc(), 0x0fffU);
}

inline u8 &mcs40_cpu_device_base::index_reg_pair(unsigned n)
{
	return m_index_regs[(BIT(n, 2) ? 0 : m_index_reg_bank) | (n & 0x7U)];
}

inline u8 mcs40_cpu_device_base::get_index_reg(unsigned n)
{
	return (index_reg_pair(n >> 1) >> (BIT(n, 0) ? 0 : 4)) & 0x0fU;
}

inline void mcs40_cpu_device_base::set_index_reg(unsigned n, u8 val)
{
	u8 &reg_pair(index_reg_pair(n >> 1));
	bool const lsn(BIT(n, 0));
	reg_pair = (reg_pair & (lsn ? 0xf0U : 0x0fU)) | ((val & 0x0fU) << (lsn ? 0 : 4));
}

inline void mcs40_cpu_device_base::set_index_reg_bank(u8 val)
{
	m_index_reg_bank = BIT(val, 0) << 3;
}


/***********************************************************************
    I/O control
***********************************************************************/

inline void mcs40_cpu_device_base::halt_decoded()
{
	m_decoded_halt = true;
}

inline void mcs40_cpu_device_base::set_rom_addr(u16 addr, u16 mask)
{
	m_rom_addr = (addr & mask) | (m_rom_addr & ~mask);
}

inline u8 mcs40_cpu_device_base::get_cr()
{
	return m_cr;
}

inline void mcs40_cpu_device_base::set_cr(u8 val, u8 mask)
{
	m_cr = (val & mask) | (m_cr & ~mask);
}

inline void mcs40_cpu_device_base::set_pending_rom_bank(u8 val)
{
	m_pending_cr3 = (m_pending_cr3 & 0x0eU) | (val & 0x01U);
}

inline void mcs40_cpu_device_base::set_rc(u8 val)
{
	m_rc_pending = true;
	m_new_rc = val;
	m_src = val; // TODO: lock out during interrupt processing
}

inline u8 mcs40_cpu_device_base::read_memory()
{
	return m_spaces[AS_RAM_MEMORY]->read_byte((u16(m_cr & 0x7U) << 8) | m_latched_rc) & 0x0fU;
}

inline void mcs40_cpu_device_base::write_memory(u8 val)
{
	m_spaces[AS_RAM_MEMORY]->write_byte((u16(m_cr & 0x7U) << 8) | m_latched_rc, val & 0x0fU);
}

inline u8 mcs40_cpu_device_base::read_status()
{
	u16 const addr((((u16(m_cr) << 6) | (m_latched_rc >> 2)) & 0x01fcU) | (m_opa & 0x0003U));
	return m_spaces[AS_RAM_STATUS]->read_byte(addr) & 0x0fU;
}

inline void mcs40_cpu_device_base::write_status(u8 val)
{
	u16 const addr((((u16(m_cr) << 6) | (m_latched_rc >> 2)) & 0x01fcU) | (m_opa & 0x0003U));
	m_spaces[AS_RAM_STATUS]->write_byte(addr, val & 0x0fU);
}

inline u8 mcs40_cpu_device_base::read_rom_port()
{
	return m_spaces[AS_ROM_PORTS]->read_byte((u16(m_cr) << 8) | m_latched_rc) & 0x0fU;
}

inline void mcs40_cpu_device_base::write_rom_port(u8 val)
{
	m_spaces[AS_ROM_PORTS]->write_byte((u16(m_cr) << 8) | m_latched_rc, val & 0x0fU);
}

inline void mcs40_cpu_device_base::write_memory_port(u8 val)
{
	m_spaces[AS_RAM_PORTS]->write_byte(((m_cr << 2) & 0x1cU) | (m_latched_rc >> 6), val & 0x0fU);
}


/***********************************************************************
    input lines
***********************************************************************/

inline bool mcs40_cpu_device_base::get_test()
{
	bool const result(CLEAR_LINE != m_test);
	if (ASSERT_LINE != m_test)
		m_test = CLEAR_LINE;
	return result;
}

inline void mcs40_cpu_device_base::set_test(int state)
{
	m_test = ((ASSERT_LINE == state) || (HOLD_LINE == state)) ? state : CLEAR_LINE;
}

inline void mcs40_cpu_device_base::set_stp(int state)
{
	m_stp = ((ASSERT_LINE == state) || (HOLD_LINE == state)) ? state : CLEAR_LINE;
}


/***********************************************************************
    instruction phases
***********************************************************************/

inline void mcs40_cpu_device_base::do_a1()
{
	m_pending_cr3 = (m_pending_cr3 << 1) | BIT(m_cr, 3);
	m_cr = (m_cr & 0x07U) | (m_pending_cr3 & 0x08U);
	if (cycle::OP == m_cycle)
	{
		m_pcbase = rom_bank() | m_rom_addr;
		if (machine().debug_flags & DEBUG_FLAG_ENABLED)
			debugger_instruction_hook(pc());
		if (m_stop_latch)
		{
			m_stp = (ASSERT_LINE == m_stp) ? ASSERT_LINE : CLEAR_LINE;
			if (!m_stop_ff)
			{
				m_stop_ff = true;
				m_stp_ack_cb(0U);
			}
		}
	}
	m_4289_a = (m_4289_a & 0xf0U) | (m_rom_addr & 0x0fU);
	m_sync_cb(1);
	update_4289_pm(1U);
	update_4289_f_l(1U);
	m_bus_cycle_cb(phase::A1, 1U, m_rom_addr & 0x000fU);
}

inline void mcs40_cpu_device_base::do_a2()
{
	m_4289_a = (m_4289_a & 0x0fU) | (m_rom_addr & 0xf0U);
	m_bus_cycle_cb(phase::A2, 1U, (m_rom_addr >> 4) & 0x000fU);
}

inline void mcs40_cpu_device_base::do_a3()
{
	m_4289_c = (m_rom_addr >> 8) & 0x0fU;
	update_cm_rom(BIT(m_cr, 3) ? 0x01U : 0x02U);
	update_cm_ram(f_cm_ram_table[m_cr & 0x07U]);
	m_bus_cycle_cb(phase::A3, 1U, (m_rom_addr >> 8) & 0x000fU);
}

inline void mcs40_cpu_device_base::do_m1()
{
	if (!m_extended_cm || (cycle::OP != m_cycle))
	{
		update_cm_rom(0x03U);
		update_cm_rom(0x0fU);
	}
	// TODO: just read the high nybble here - MAME doesn't support this
	u8 const read = m_cache.read_byte(rom_bank() | m_rom_addr);
	if (cycle::OP == m_cycle)
	{
		m_opr = (m_stop_ff) ? 0x0U : (read >> 4);
		m_io_pending = is_io_op(m_opr);
	}
	else
	{
		m_arg = read;
	}
	m_decoded_halt = false;
	m_bus_cycle_cb(phase::M1, 1U, (read >> 4) & 0x0fU);
}

inline void mcs40_cpu_device_base::do_m2()
{
	// TODO: just read the low nybble here - MAME doesn't support this
	u8 const read = m_cache.read_byte(rom_bank() | m_rom_addr);
	if (cycle::OP == m_cycle)
		m_opa = (m_stop_ff) ? 0x0U : (read & 0x0fU);
	else
		m_arg = read;
	if (m_io_pending)
	{
		update_cm_rom(BIT(m_cr, 3) ? 0x01U : 0x02U);
		update_cm_ram(f_cm_ram_table[m_cr & 0x07U]);
	}
	m_resume = m_stop_latch && (CLEAR_LINE == m_stp);
	m_stop_latch = CLEAR_LINE != m_stp;
	if (!m_stop_ff && (cycle::IN != m_cycle))
		pc() = (pc() + 1) & 0x0fff;
	m_rom_addr = pc();
	m_bus_cycle_cb(phase::M2, 1U, read & 0x0fU);
}

inline void mcs40_cpu_device_base::do_x1()
{
	// FIXME: is 4004 output on the second cycle of two-cycle instruction OPA or low nybble of the argument?
	u8 const output(m_extended_cm ? m_a : (cycle::OP == m_cycle) ? m_opa : m_arg);
	update_cy(m_c);
	update_cm_rom(0x03U);
	update_cm_ram(0x0fU);
	if (cycle::OP == m_cycle)
	{
		m_program_op = pmem::NONE;
		m_cycle = do_cycle1(m_opr, m_opa, m_program_op);
	}
	else
	{
		do_cycle2(m_opr, m_opa, m_arg);
		m_cycle = cycle::OP;
	}
	m_4289_a = m_latched_rc;
	if (pmem::NONE == m_program_op)
	{
		m_4289_c = (m_latched_rc >> 4) & 0x0fU;
	}
	else
	{
		assert(cycle::OP == m_cycle);
		m_4289_c = 0x0fU;
		update_4289_pm(0x00U);
		update_4289_f_l(m_4289_first ? 0x01 : 0x00);
		m_4289_first = !m_4289_first;
		if (pmem::READ == m_program_op)
			m_arg = m_spaces[AS_PROGRAM_MEMORY]->read_byte(program_addr()) & 0x0fU;
		else
			assert(pmem::WRITE == m_program_op);
	}
	m_bus_cycle_cb(phase::X1, 1U, output);
}

void mcs40_cpu_device_base::do_x2()
{
	u8 output((m_new_rc >> 4) & 0x0fU); // FIXME: what appears on the bus if it isn't SRC, I/O or program memory access?
	if (m_io_pending)
	{
		assert(phase::X2 == m_phase);
		assert(m_latched_rc == m_new_rc);
		assert(!m_rc_pending);
		output = do_io(m_opr, m_opa);
		m_io_pending = false;
	}
	if (m_rc_pending)
	{
		update_cm_rom(BIT(m_cr, 3) ? 0x01U : 0x02U);
		update_cm_ram(f_cm_ram_table[m_cr & 0x07U]);
		m_latched_rc = (m_latched_rc & 0x0fU) | (m_new_rc & 0xf0U);
		output = (m_new_rc >> 4) & 0x0fU;
	}
	else
	{
		assert(m_latched_rc == m_new_rc);
	}
	if (pmem::READ == m_program_op)
		set_a(output = m_arg & 0x0fU);
	else if (pmem::WRITE == m_program_op)
		output = get_a();
	m_bus_cycle_cb(phase::X2, 1U, output);
}

void mcs40_cpu_device_base::do_x3()
{
	m_sync_cb(0);
	update_cm_rom(0x03U);
	update_cm_ram(0x0fU);
	if (m_rc_pending)
	{
		m_latched_rc = (m_latched_rc & 0xf0U) | (m_new_rc & 0x0fU);
		m_rc_pending = false;
	}
	else
	{
		assert(m_latched_rc == m_new_rc);
	}
	if (pmem::WRITE == m_program_op)
		m_spaces[AS_PROGRAM_MEMORY]->write_byte(program_addr(), get_a());
	if (!m_stop_ff && m_decoded_halt)
	{
		m_stop_ff = true;
		m_stp_ack_cb(0U);
	}
	else if (m_stop_ff && m_resume)
	{
		m_stop_ff = false;
		m_stp_ack_cb(1U);
	}
	m_resume = false;
	m_bus_cycle_cb(phase::X3, 0U, m_new_rc & 0x0fU); // FIXME: what appears on the bus if it isn't SRC?
}


/***********************************************************************
    internal helpers
***********************************************************************/

inline void mcs40_cpu_device_base::update_cm_rom(u8 val)
{
	u8 const diff(val ^ m_cm_rom);
	m_cm_rom = val;
	if (BIT(diff, 0))
		m_cm_rom_cb[0](BIT(val, 0));
	if (BIT(diff, 1))
		m_cm_rom_cb[1](BIT(val, 1));
}

inline void mcs40_cpu_device_base::update_cm_ram(u8 val)
{
	u8 const diff(val ^ m_cm_ram);
	m_cm_ram = val;
	if (BIT(diff, 0))
		m_cm_ram_cb[0](BIT(val, 0));
	if (BIT(diff, 1))
		m_cm_ram_cb[1](BIT(val, 1));
	if (BIT(diff, 2))
		m_cm_ram_cb[2](BIT(val, 2));
	if (BIT(diff, 3))
		m_cm_ram_cb[3](BIT(val, 3));
}

inline void mcs40_cpu_device_base::update_cy(u8 val)
{
	u8 const diff(val ^ m_cy);
	m_cy = val;
	if (BIT(diff, 0))
		m_cy_cb(BIT(val, 0));
}

inline void mcs40_cpu_device_base::update_4289_pm(u8 val)
{
	u8 const diff(val ^ m_4289_pm);
	m_4289_pm = val;
	if (BIT(diff, 0))
		m_4289_pm_cb(BIT(val, 0));
}

inline void mcs40_cpu_device_base::update_4289_f_l(u8 val)
{
	u8 const diff(val ^ m_4289_f_l);
	m_4289_f_l = val;
	if (BIT(diff, 0))
		m_4289_f_l_cb(BIT(val, 0));
}



i4004_cpu_device::i4004_cpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: mcs40_cpu_device_base(mconfig, I4004, tag, owner, clock, false, 12U, 0x3U, 16U, 0x7U)
{
}


/***********************************************************************
    device_execute_interface implementation
***********************************************************************/

void i4004_cpu_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case I4004_TEST_LINE:
		set_test(state);
		break;
	default:
		mcs40_cpu_device_base::execute_set_input(inputnum, state);
	}
}


/***********************************************************************
    device_disasm_interface implementation
***********************************************************************/

std::unique_ptr<util::disasm_interface> i4004_cpu_device::create_disassembler()
{
	return std::make_unique<i4004_disassembler>();
}


/***********************************************************************
    mcs40_cpu_device_base implementation
***********************************************************************/

bool i4004_cpu_device::is_io_op(u8 opr)
{
	return 0x0e == opr;
}

i4004_cpu_device::cycle i4004_cpu_device::do_cycle1(u8 opr, u8 opa, pmem &program_op)
{
	static constexpr u8 kbp_table[] = { 0x0, 0x1, 0x2, 0xf, 0x3, 0xf, 0xf, 0xf, 0x4, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf };

	switch (opr)
	{
	case 0x0:
		switch (opa)
		{
		case 0x0: // NOP
			return cycle::OP;
		default:
			break;
		}
		break;

	case 0x1: // JCN
	case 0x4: // JUN
	case 0x5: // JMS
	case 0x7: // ISZ
		return cycle::IM;

	case 0x2:
		if (BIT(opa, 0))
		{
			// SRC
			set_rc(index_reg_pair(opa >> 1));
			return cycle::OP;
		}
		else
		{
			// FIM
			return cycle::IM;
		}

	case 0x3:
		if (BIT(opa, 0))
		{
			// JIN
			set_pc(index_reg_pair(opa >> 1), 0x00ffU);
			return cycle::OP;
		}
		else
		{
			// FIN
			set_rom_addr(index_reg_pair(0), 0x00ffU);
			return cycle::IN;
		}

	case 0x6: // INC
		set_index_reg(opa, get_index_reg(opa) + 1U);
		return cycle::OP;

	case 0x8: // ADD
		set_a_c(get_a() + get_index_reg(opa) + get_c());
		return cycle::OP;

	case 0x9: // SUB
		set_a_c(get_a() + (get_index_reg(opa) ^ 0x0fU) + (get_c() ^ 0x01U));
		return cycle::OP;

	case 0xa: // LD
		set_a(get_index_reg(opa));
		return cycle::OP;

	case 0xb: // XCH
		{
			u8 const val = get_a();
			set_a(get_index_reg(opa));
			set_index_reg(opa, val);
		}
		return cycle::OP;

	case 0xc: // BBL
		pop_pc();
		set_a(opa);
		return cycle::OP;

	case 0xd: // LDM
		set_a(opa);
		return cycle::OP;

	case 0xe: // WRM/WMP/WRR/WPM/WR0/WR1/WR2/WR3/SBM/RDM/RDR/ADM/RD0/RD1/RD2/RD3
		if (0x3 == opa)
			program_op = pmem::WRITE;
		return cycle::OP;

	case 0xf:
		switch (opa)
		{
		case 0x0: // CLB
			set_a(0U);
			set_c(0U);
			return cycle::OP;
		case 0x1: // CLC
			set_c(0U);
			return cycle::OP;
		case 0x2: // IAC
			set_a_c(get_a() + 1U);
			return cycle::OP;
		case 0x3: // CMC
			set_c(get_c() ^ 0x01U);
			return cycle::OP;
		case 0x4: // CMA
			set_a(get_a() ^ 0x0fU);
			return cycle::OP;
		case 0x5: // RAL
			set_a_c((get_a() << 1) | get_c());
			return cycle::OP;
		case 0x6: // RAR
			{
				u8 const c(BIT(get_a(), 0));
				set_a((get_a() >> 1) | (get_c()  << 3));
				set_c(c);
			}
			return cycle::OP;
		case 0x7: // TCC
			set_a(get_c());
			set_c(0U);
			return cycle::OP;
		case 0x8: // DAC
			set_a_c(get_a() + 0x0fU);
			return cycle::OP;
		case 0x9: // TCS
			set_a(9U + get_c());
			set_c(0U);
			return cycle::OP;
		case 0xa: // STC
			set_c(1U);
			return cycle::OP;
		case 0xb: // DAA
			if (get_c() || (9U < get_a()))
			{
				u8 const val(get_a() + 6U);
				set_a(val);
				if (BIT(val, 4))
					set_c(1U);
			}
			return cycle::OP;
		case 0xc: // KBP
			set_a(kbp_table[get_a()]);
			return cycle::OP;
		case 0xd: // DCL
			set_cr(get_a(), 0x07U);
			return cycle::OP;
		default:
			break;
		}
		break;

	default: // something is badly wrong if we get here
		throw false;
	}

	logerror("MCS-40: unhandled instruction OPR=%X OPA=%X\n", opr, opa);
	return cycle::OP;
}

void i4004_cpu_device::do_cycle2(u8 opr, u8 opa, u8 arg)
{
	switch (opr)
	{
	case 0x1: // JCN
		{
			// FIXME: on which cycle is TEST sampled?
			// order of expression is important because of how HOLD_LINE is consumed for TEST input
			bool const jump((BIT(opa, 0) && !get_test()) || (BIT(opa, 1) && get_c()) || (BIT(opa, 2) && !get_a()));
			if (bool(BIT(opa, 3)) != jump)
				set_pc(arg, 0x00ff);
		}
		break;

	case 0x2: // FIM
		assert(!BIT(opa, 0));
		index_reg_pair(opa >> 1) = arg;
		break;

	case 0x3: // FIN
		assert(!BIT(opa, 0));
		index_reg_pair(opa >> 1) = arg;
		break;

	case 0x4: // JUN
		set_pc((u16(opa) << 8) | arg, 0x0fffU);
		break;

	case 0x5: // JMS
		push_pc();
		set_pc((u16(opa) << 8) | arg, 0x0fffU);
		break;

	case 0x7: // ISZ
		{
			u8 const val((get_index_reg(opa) + 1U) & 0x0fU);
			set_index_reg(opa, val);
			if (val)
				set_pc(arg, 0x00ffU);
		}
		break;

	default: // something is badly wrong if we get here
		throw false;
	}
}

u8 i4004_cpu_device::do_io(u8 opr, u8 opa)
{
	assert(0xe == opr);
	u8 result;
	switch (opa)
	{
	case 0x0: // WRM
		result = get_a();
		write_memory(result);
		return result;
	case 0x1: // WMP
		result = get_a();
		write_memory_port(result);
		return result;
	case 0x2: // WRR
		result = get_a();
		write_rom_port(result);
		return result;
	case 0x3: // WPM
		// FIXME: with early 4002 chips this overwrites memory
		return get_a();
	case 0x4: // WR0
	case 0x5: // WR1
	case 0x6: // WR2
	case 0x7: // WR3
		result = get_a();
		write_status(result);
		return result;
	case 0x8: // SBM
		result = read_memory();
		set_a_c(get_a() + (result ^ 0x0fU) + (get_c() ^ 0x01U));
		return result;
	case 0x9: // RDM
		result = read_memory();
		set_a(result);
		return result;
	case 0xa: // RDR
		result = read_rom_port();
		set_a(result);
		return result;
	case 0xb: // ADM
		result = read_memory();
		set_a_c(get_a() + result + get_c());
		return result;
	case 0xc: // RD0
	case 0xd: // RD1
	case 0xe: // RD2
	case 0xf: // RD3
		result = read_status();
		set_a(result);
		return result;
	default: // something is badly wrong if we get here
		throw false;
	}
}



i4040_cpu_device::i4040_cpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: i4004_cpu_device(mconfig, I4040, tag, owner, clock, true, 13U, 0x7U, 24U, 0xfU)
{
}


/***********************************************************************
    device_execute_interface implementation
***********************************************************************/

void i4040_cpu_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case I4040_STP_LINE:
		set_stp(state);
		break;
	default:
		i4004_cpu_device::execute_set_input(inputnum, state);
	}
}


/***********************************************************************
    device_disasm_interface implementation
***********************************************************************/

std::unique_ptr<util::disasm_interface> i4040_cpu_device::create_disassembler()
{
	return std::make_unique<i4040_disassembler>();
}


/***********************************************************************
    mcs40_cpu_device_base implementation
***********************************************************************/

i4040_cpu_device::cycle i4040_cpu_device::do_cycle1(u8 opr, u8 opa, pmem &program_op)
{
	switch (opr)
	{
	case 0x0:
		switch (opa)
		{
		case 0x1: // HLT
			halt_decoded();
			return cycle::OP;
		case 0x3: // LCR
			set_a(get_cr());
			return cycle::OP;
		case 0x4: // OR4
		case 0x5: // OR5
			set_a(get_a() | get_index_reg(4U | BIT(opa, 0)));
			return cycle::OP;
		case 0x6: // AN6
		case 0x7: // AN7
			set_a(get_a() & get_index_reg(6U | BIT(opa, 0)));
			return cycle::OP;
		case 0x8: // DB0
		case 0x9: // DB1
			set_pending_rom_bank(BIT(opa, 0));
			return cycle::OP;
		case 0xa: // SB0
		case 0xb: // SB1
			set_index_reg_bank(BIT(opa, 0));
			return cycle::OP;
		case 0xe: // RPM
			program_op = pmem::READ;
			return cycle::OP;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return i4004_cpu_device::do_cycle1(opr, opa, program_op);
}


#if 0
void i4040_cpu_device::execute_one(unsigned opcode)
{
	switch (opcode)
	{
	case 0x02: // BBS
	case 0x0c: // EIN
	case 0x0d: // DIN
	default:
		i4004_cpu_device::execute_one(opcode);
	}
}
#endif
