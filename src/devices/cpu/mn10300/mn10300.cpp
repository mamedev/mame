// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

// Panasonic MN10300 (AM33) execution core.

#include "emu.h"
#include "mn10300.h"
#include "mn103dasm.h"           // reuse the existing MN10300 disassembler
#include "mn10300_insn_length.h" // validated length table (shared with tests/)

#include <bit>

enum mn10300_flag
{
	FLAG_ZF = 0x0001, // zero
	FLAG_NF = 0x0002, // negative (result bit31)
	FLAG_CF = 0x0004, // carry / borrow
	FLAG_VF = 0x0008, // signed overflow
	FLAG_IM = 0x0700, // interrupt priority-level mask (EPSW_IM, bits 10:8)
	FLAG_IE = 0x0800  // interrupt enable (EPSW_IE) - confirmed from firmware
};

static constexpr int IM_SHIFT = 8;

DEFINE_DEVICE_TYPE(MN103002A, mn103002a_device, "mn103002a", "Panasonic MN103002A")

mn103002a_device::mn103002a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mn10300_device(mconfig, MN103002A, tag, owner, clock,
			address_map_constructor(FUNC(mn103002a_device::mn103002a_internal_map), this))
{
}

mn10300_device::mn10300_device(const machine_config &mconfig, device_type type, const char *tag,
		device_t *owner, uint32_t clock, address_map_constructor program)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0, program)
	, m_program(nullptr)
	, m_sio_tx_cb(*this)
	, m_sio_tx_done_cb(*this)
	, m_sio_rx_rdy_cb(*this)
	, m_sio_rx_enable_cb(*this)
	, m_intc_ack_cb(*this)
	, m_intc_accept_cb(*this)
	, m_intc_extmd_cb(*this)
	, m_iagr_latch(0), m_intc_280(0)
	, m_tm5_mode(0), m_tm5_base(0), m_tm5_timer(nullptr)
	, m_pc(0), m_sp(0), m_mdr(0), m_mdrq(0), m_mcrh(0), m_mcrl(0), m_mcvf(0), m_psw(0), m_lir(0), m_lar(0)
	, m_irq_state(CLEAR_LINE), m_irq_vector(0), m_irq_level(7)
	, m_icount(0)
{
	std::fill(std::begin(m_d), std::end(m_d), 0);
	std::fill(std::begin(m_a), std::end(m_a), 0);
	std::fill(std::begin(m_e), std::end(m_e), 0);
	std::fill(std::begin(m_sio_config), std::end(m_sio_config), 0);
	std::fill(std::begin(m_sio_control), std::end(m_sio_control), 0);
	for (auto &fifo : m_sio_rx_fifo)
		std::fill(std::begin(fifo), std::end(fifo), 0);
	std::fill(std::begin(m_sio_rx_head), std::end(m_sio_rx_head), 0);
	std::fill(std::begin(m_sio_rx_tail), std::end(m_sio_rx_tail), 0);
	std::fill(std::begin(m_gxicr), std::end(m_gxicr), 0);
}

void mn10300_device::mn103002a_internal_map(address_map &map)
{
	map(0x20000000, 0x2000001b).rw(FUNC(mn10300_device::ivar_r), FUNC(mn10300_device::ivar_w));
	// GxICR array, the 0x34000200 group register and the 0x34000280 EXTMD latch.
	map(0x34000100, 0x340002ff).rw(FUNC(mn10300_device::intc_r), FUNC(mn10300_device::intc_w));
	map(0x34000800, 0x3400082f).rw(FUNC(mn10300_device::sio_r), FUNC(mn10300_device::sio_w));
	// TM4/TM5: mode bytes @0x34001080/82, reloads @0x34001090/92, counters
	// @0x340010A0/A2 (only TM5 -- offset 1 of each word pair -- is live).
	map(0x34001080, 0x34001083).rw(FUNC(mn10300_device::tm45_mode_r), FUNC(mn10300_device::tm45_mode_w));
	map(0x34001090, 0x34001093).rw(FUNC(mn10300_device::tm45_base_r), FUNC(mn10300_device::tm45_base_w));
	map(0x340010a0, 0x340010a3).r(FUNC(mn10300_device::tm45_count_r));
}

device_memory_interface::space_config_vector mn10300_device::memory_space_config() const
{
	return space_config_vector { std::make_pair(AS_PROGRAM, &m_program_config) };
}

std::unique_ptr<util::disasm_interface> mn10300_device::create_disassembler()
{
	return std::make_unique<mn10300_disassembler>();
}

void mn10300_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_pc));
	save_item(NAME(m_d));
	save_item(NAME(m_a));
	save_item(NAME(m_e));
	save_item(NAME(m_sp));
	save_item(NAME(m_mdr));
	save_item(NAME(m_mdrq));
	save_item(NAME(m_mcrh));
	save_item(NAME(m_mcrl));
	save_item(NAME(m_mcvf));
	save_item(NAME(m_psw));
	save_item(NAME(m_lir));
	save_item(NAME(m_lar));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_irq_vector));
	save_item(NAME(m_irq_level));
	save_item(NAME(m_sio_config));
	save_item(NAME(m_sio_control));
	save_item(NAME(m_sio_rx_fifo));
	save_item(NAME(m_sio_rx_head));
	save_item(NAME(m_sio_rx_tail));
	save_item(NAME(m_gxicr));
	save_item(NAME(m_iagr_latch));
	save_item(NAME(m_intc_280));
	save_item(NAME(m_tm5_mode));
	save_item(NAME(m_tm5_base));
	save_item(NAME(m_ivar));
	// (m_tm5_timer is a device-allocated emu_timer and is save-stated
	// automatically.)

	m_tm5_timer = timer_alloc(FUNC(mn10300_device::tm5_tick), this);

	state_add(MN10300_PC,  "PC",  m_pc ).formatstr("%08X");
	state_add(MN10300_SP,  "SP",  m_sp ).formatstr("%08X");
	state_add(MN10300_MDR, "MDR", m_mdr).formatstr("%08X");
	state_add(MN10300_D0,  "D0",  m_d[0]).formatstr("%08X");
	state_add(MN10300_D1,  "D1",  m_d[1]).formatstr("%08X");
	state_add(MN10300_D2,  "D2",  m_d[2]).formatstr("%08X");
	state_add(MN10300_D3,  "D3",  m_d[3]).formatstr("%08X");
	state_add(MN10300_A0,  "A0",  m_a[0]).formatstr("%08X");
	state_add(MN10300_A1,  "A1",  m_a[1]).formatstr("%08X");
	state_add(MN10300_A2,  "A2",  m_a[2]).formatstr("%08X");
	state_add(MN10300_A3,  "A3",  m_a[3]).formatstr("%08X");
	state_add(MN10300_PSW, "PSW", m_psw).formatstr("%04X");

	state_add(STATE_GENPC,     "GENPC",    m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",    m_pc).noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS", m_psw).formatstr("%4s").noshow();

	set_icountptr(m_icount);
}

void mn10300_device::device_reset()
{
	// The boot address is strapped per board (set_reset_pc). SP is left at zero:
	// the reset code establishes its own before its first push or call.
	m_pc  = m_reset_pc;
	m_sp  = 0;
	m_psw = 0;
	m_mdr = 0;
	m_mdrq = m_mcrh = m_mcrl = m_mcvf = 0;
	std::fill(std::begin(m_d), std::end(m_d), 0);
	std::fill(std::begin(m_a), std::end(m_a), 0);
	std::fill(std::begin(m_e), std::end(m_e), 0);

	// On-chip SIO: config and control cleared, RX rings emptied by head = tail.
	for (unsigned ch = 0; ch < NUM_SIO; ch++)
	{
		m_sio_config[ch] = 0;
		m_sio_control[ch] = 0;
		m_sio_rx_head[ch] = m_sio_rx_tail[ch] = 0;
	}

	std::fill(std::begin(m_gxicr), std::end(m_gxicr), 0);
	// ...and the delivery state derived from them, which would otherwise survive
	// a reset holding the level and vector of an interrupt that no longer exists.
	std::fill(std::begin(m_ivar), std::end(m_ivar), 0);
	m_irq_state = CLEAR_LINE;
	m_irq_vector = 0;
	m_irq_level = 7;
	m_tm5_mode = 0;
	m_tm5_base = 0;
	m_tm5_timer->adjust(attotime::never);
}

void mn10300_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	if (entry.index() == STATE_GENFLAGS)
		str = string_format("%c%c%c%c",
			m_psw & FLAG_VF ? 'V' : '-',
			m_psw & FLAG_CF ? 'C' : '-',
			m_psw & FLAG_NF ? 'N' : '-',
			m_psw & FLAG_ZF ? 'Z' : '-');
}

void mn10300_device::execute_set_input(int inputnum, int state)
{
	// Line 0 is the single maskable interrupt raised by the on-chip interrupt
	// controller above. Latch it; the run loop re-checks it each instruction.
	if (inputnum == 0)
		m_irq_state = state;
}

void mn10300_device::take_irq(int level)
{
	standard_irq_callback(0, m_pc);
	intc_accept();
	push32(m_pc);
	push32(m_psw);
	m_psw = ((m_psw & ~FLAG_IM) | (level << IM_SHIFT)) & ~FLAG_IE;
	m_pc = m_irq_vector;
	m_icount -= 7;
}

void mn10300_device::check_irq()
{
	if (!(m_psw & FLAG_IE))
		return;
	const int im = (m_psw & FLAG_IM) >> IM_SHIFT;
	if (m_irq_state != CLEAR_LINE && m_irq_vector != 0 && m_irq_level < im)
		take_irq(m_irq_level);
}

uint16_t mn10300_device::ivar_r(offs_t offset, uint16_t mem_mask)
{
	return (offset & 1) ? 0 : m_ivar[offset >> 1];
}

void mn10300_device::ivar_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (!(offset & 1))
		COMBINE_DATA(&m_ivar[offset >> 1]);
}

uint32_t mn10300_device::level_vector(int level) const
{
	return (level < 7) ? m_vector_base + m_ivar[level] : 0;
}

int mn10300_device::intc_pending_group() const
{
	// Among enabled+requested groups the winner is the highest-priority one,
	// which is the LOWEST ICR LEVEL value (bits 14:12).
	int best = 0, best_level = 8;
	for (int g = 2; g < int(NUM_INTC_GROUPS); g++)
		if ((m_gxicr[g] & 0x0110) == 0x0110)   // ENABLE(0x100) & REQUEST(0x10)
		{
			const int level = (m_gxicr[g] >> 12) & 7;
			if (level < best_level) { best_level = level; best = g; }
		}
	return best;
}

// Freeze the arbitration result (group and vector, atomically) at the instant
// the CPU accepts the interrupt -- the INTC latches IAGR at acknowledge.
void mn10300_device::intc_accept()
{
	const int g = intc_pending_group();
	if (g)
	{
		m_iagr_latch = g;
		// Board hook: this group was latched into the IAGR (see intc_accept_cb).
		m_intc_accept_cb(uint8_t(g));
		const int level = (m_gxicr[g] >> 12) & 7;
		m_irq_vector = level_vector(level);
		m_irq_level = level;
	}
}

uint16_t mn10300_device::intc_r(offs_t offset, uint16_t mem_mask)
{
	const int reg = offset << 1;                  // byte offset within 0x34000100
	if (reg == 0x00)                              // IAGR: the group latched at interrupt accept
		return m_iagr_latch << 3;
	if (reg == 0x04)
		return 0;
	if (reg == 0x100)                             // 0x34000200: level-6 group register (latched at accept)
		return m_iagr_latch << 2;
	if (reg == 0x180)                             // 0x34000280: per-source 2-bit control fields (latched)
		return m_intc_280;                        // software only ever read-modify-writes it,
												  // so the state has to accumulate
	const int group = reg >> 2;                   // GxICR(group) at +group*4
	if (group < int(NUM_INTC_GROUPS))
		return m_gxicr[group];
	return 0;
}

void mn10300_device::intc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const int reg = offset << 1;
	if (reg == 0x180)                             // 0x34000280 = EXTMD (ext-int trigger modes)
	{
		COMBINE_DATA(&m_intc_280);
		m_intc_extmd_cb(m_intc_280);
		return;
	}
	if (reg < 0x08)
		return;                                   // IAGR is read-only
	const int group = reg >> 2;
	if (group >= int(NUM_INTC_GROUPS))
		return;
	// A write carrying DETECT-ack bits: report it outward, so a board that needs
	// level-like re-delivery for one of its sources can implement it.
	if (data & mem_mask & 0x000f)
		m_intc_ack_cb(uint8_t(group));
	if (group == 0x17 && (data & mem_mask & 0x000f))
	{
		m_gxicr[0x17] = uint16_t((data & mem_mask & 0xff00) | 0x0011);   // DETECT+REQUEST; ENABLE as written
		intc_recompute();
		return;
	}
	{
		const uint16_t cur = m_gxicr[group];
		const uint16_t nv  = (cur & ~mem_mask) | (data & mem_mask);
		uint16_t detect = (cur & 0x000f) & ~(data & mem_mask & 0x000f);
		m_gxicr[group] = (nv & 0xff00) | detect | (detect ? 0x0010 : 0x0000);
	}
	intc_recompute();
}

void mn10300_device::intc_assert(int group)
{
	// REQUEST + DETECT bit0; a handler scans DETECT bits 0-3 to pick the
	// sub-source within the group.
	m_gxicr[group & (NUM_INTC_GROUPS - 1)] |= 0x0011;
	intc_recompute();
}

void mn10300_device::intc_recompute()
{
	const int g = intc_pending_group();
	if (g)
	{
		const int level = (m_gxicr[g] >> 12) & 7;
		m_irq_vector = level_vector(level);
		m_irq_level = level;
	}
	// Drive our own maskable input line through the standard, synchronized path.
	set_input_line(0, g ? ASSERT_LINE : CLEAR_LINE);
}

uint16_t mn10300_device::tm45_mode_r(offs_t offset)
{
	return offset ? m_tm5_mode : 0;
}

void mn10300_device::tm45_mode_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset && ACCESSING_BITS_0_7)
		tm5_mode_w(data & 0xff);
}

uint16_t mn10300_device::tm45_base_r(offs_t offset)
{
	return offset ? m_tm5_base : 0;
}

void mn10300_device::tm45_base_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (offset)
	{
		uint16_t base = m_tm5_base;
		COMBINE_DATA(&base);      // a byte write must keep the other half
		tm5_base_w(base);
	}
}

uint16_t mn10300_device::tm45_count_r(offs_t offset)
{
	if (!offset || !(m_tm5_mode & 0x80))
		return 0;
	const attotime period = m_tm5_timer->period();
	if (period.is_never() || period.is_zero())
		return 0;
	return uint16_t(m_tm5_timer->remaining().as_ticks(clock() / 2) / TM5_PRESCALE);
}

void mn10300_device::tm5_mode_w(uint8_t data)
{
	const uint8_t rising = data & ~m_tm5_mode;
	m_tm5_mode = data;
	if (!(data & 0x80))
		m_tm5_timer->adjust(attotime::never);          // count disabled
	else if (rising & (0x80 | 0x40))
		tm5_rearm(true);                               // enabled, or load pulse while enabled
}

void mn10300_device::tm5_base_w(uint16_t data)
{
	m_tm5_base = data;
	// While running, the new reload takes effect at the next underflow: keep the
	// current countdown, change only the periodic reload.
	if ((m_tm5_mode & 0x80) && !m_tm5_timer->remaining().is_never())
		tm5_rearm(false);
}

void mn10300_device::tm5_rearm(bool restart_phase)
{
	const attotime period = attotime::from_ticks(uint64_t(m_tm5_base + 1) * TM5_PRESCALE, clock() / 2);
	if (restart_phase)
		m_tm5_timer->adjust(period, 0, period);
	else
		m_tm5_timer->adjust(m_tm5_timer->remaining(), 0, period);
}

TIMER_CALLBACK_MEMBER(mn10300_device::tm5_tick)
{
	intc_assert(0x07);      // GxICR 0x3400011C
}

uint16_t mn10300_device::sio_r(offs_t offset, uint16_t mem_mask)
{
	const int ch = offset / 8;
	const int reg = (offset << 1) & 0x0f;
	switch (reg)
	{
	case 0x0:                    // config
		return m_sio_config[ch];
	case 0x4:                    // control (byte @+4)
		return m_sio_control[ch];
	case 0x8:                    // +8 TX (write-only) / +9 RX (read, high byte)
		if (ACCESSING_BITS_8_15)
			return uint16_t(sio_rx_pop(ch)) << 8;
		return 0;
	case 0xc:                    // status: bit4 = RxRDY
		return sio_rx_ready(ch) ? 0x0010 : 0x0000;
	}
	return 0;
}

void mn10300_device::sio_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const int ch = offset / 8;
	const int reg = (offset << 1) & 0x0f;
	switch (reg)
	{
	case 0x0:                    // config
		COMBINE_DATA(&m_sio_config[ch]);
		if (ch == 0 && (m_sio_config[ch] & 0x8000))
			m_sio_config[ch] &= 0x7fff;
		// RX enable (bit14): notify the endpoint, which may now send its queued
		// reply, one byte per RX interrupt.
		if (ch == 0 && (m_sio_config[ch] & 0x4000))
			m_sio_rx_enable_cb[ch](1);
		break;
	case 0x4:                    // control (byte @+4)
		if (ACCESSING_BITS_0_7)
			m_sio_control[ch] = data & 0xff;
		break;
	case 0x8:                    // TX data (byte @+8 = low byte)
		if (ACCESSING_BITS_0_7)
			sio_tx_byte(ch, data & 0xff);
		break;
	default:
		break;
	}
}

void mn10300_device::sio_tx_byte(int ch, uint8_t data)
{
	m_sio_tx_done_cb[ch](1);
	m_sio_tx_cb[ch](data);
}

void mn10300_device::sio_rx_push(int ch, uint8_t data)
{
	const uint8_t next = (m_sio_rx_head[ch] + 1) % std::size(m_sio_rx_fifo[ch]);
	if (next == m_sio_rx_tail[ch])
		return;                  // FIFO full -- drop (overrun)
	m_sio_rx_fifo[ch][m_sio_rx_head[ch]] = data;
	m_sio_rx_head[ch] = next;
	m_sio_rx_rdy_cb[ch](1);
}

uint8_t mn10300_device::sio_rx_pop(int ch)
{
	if (m_sio_rx_head[ch] == m_sio_rx_tail[ch])
		return 0;
	const uint8_t v = m_sio_rx_fifo[ch][m_sio_rx_tail[ch]];
	if (!machine().side_effects_disabled())
		m_sio_rx_tail[ch] = (m_sio_rx_tail[ch] + 1) % std::size(m_sio_rx_fifo[ch]);
	return v;
}

inline void mn10300_device::set_nz32(uint32_t r)
{
	m_psw &= ~(FLAG_ZF | FLAG_NF);
	if (r == 0)          m_psw |= FLAG_ZF;
	if (r & 0x80000000u) m_psw |= FLAG_NF;
}

// r = a + b + carry_in; sets Z,N,C,V; returns the 32-bit result.
inline uint32_t mn10300_device::do_add(uint32_t a, uint32_t b, uint32_t carry_in)
{
	uint64_t wide = (uint64_t)a + (uint64_t)b + carry_in;
	uint32_t r = (uint32_t)wide;
	m_psw &= ~(FLAG_ZF | FLAG_NF | FLAG_CF | FLAG_VF);
	if (r == 0)                              m_psw |= FLAG_ZF;
	if (r & 0x80000000u)                     m_psw |= FLAG_NF;
	if (wide > 0xFFFFFFFFull)                m_psw |= FLAG_CF;
	if ((~(a ^ b) & (a ^ r)) & 0x80000000u)  m_psw |= FLAG_VF;
	return r;
}

// r = a - b - borrow_in; sets Z,N,C(borrow),V; returns the 32-bit result.
inline uint32_t mn10300_device::do_sub(uint32_t a, uint32_t b, uint32_t borrow_in)
{
	uint64_t wide = (uint64_t)a - (uint64_t)b - borrow_in;
	uint32_t r = (uint32_t)wide;
	m_psw &= ~(FLAG_ZF | FLAG_NF | FLAG_CF | FLAG_VF);
	if (r == 0)                             m_psw |= FLAG_ZF;
	if (r & 0x80000000u)                    m_psw |= FLAG_NF;
	if ((wide >> 32) & 1)                   m_psw |= FLAG_CF; // borrow
	if (((a ^ b) & (a ^ r)) & 0x80000000u)  m_psw |= FLAG_VF;
	return r;
}

// Condition test for the C0..C9 b<cc> family (indices into f_conds order:
// blt,bgt,bge,ble,bcs,bhi,bcc,bls,beq,bne).
inline bool mn10300_device::test_cond(int cc)
{
	const bool Z = m_psw & FLAG_ZF, N = m_psw & FLAG_NF;
	const bool C = m_psw & FLAG_CF, V = m_psw & FLAG_VF;
	switch (cc)
	{
		case 0: return (N ^ V);            // blt
		case 1: return !((N ^ V) || Z);    // bgt
		case 2: return !(N ^ V);           // bge
		case 3: return ((N ^ V) || Z);     // ble
		case 4: return C;                  // bcs / blo
		case 5: return !(C || Z);          // bhi
		case 6: return !C;                 // bcc / bhs
		case 7: return (C || Z);           // bls
		case 8: return Z;                  // beq
		case 9: return !Z;                 // bne
	}
	return false;
}

inline void mn10300_device::push32(uint32_t val)
{
	m_sp -= 4;
	write_mem32(m_sp, val);
}
inline uint32_t mn10300_device::pop32()
{
	uint32_t v = read_mem32(m_sp);
	m_sp += 4;
	return v;
}

void mn10300_device::execute_run()
{
	do
	{
		check_irq();

		debugger_instruction_hook(m_pc);

		const uint32_t start_pc = m_pc;   // PC-relative targets use the insn start
		const uint8_t op = read_arg8(m_pc);
		m_pc += 1;

		const int dst = op & 3;           // low field
		const int src = (op >> 2) & 3;    // bits[3:2]

		switch (op)
		{
		// ---- clr dD (D = bits[3:2]) ----
		case 0x00: case 0x04: case 0x08: case 0x0C:
			m_d[src] = 0;
			m_psw = (m_psw & ~(FLAG_NF | FLAG_CF | FLAG_VF)) | FLAG_ZF;
			break;

		// ---- extb/extbu/exth/exthu dD ----
		case 0x10: case 0x11: case 0x12: case 0x13: m_d[dst] = (int32_t)(int8_t)m_d[dst]; break;
		case 0x14: case 0x15: case 0x16: case 0x17: m_d[dst] &= 0x000000FF; break;
		case 0x18: case 0x19: case 0x1A: case 0x1B: m_d[dst] = (int32_t)(int16_t)m_d[dst]; break;
		case 0x1C: case 0x1D: case 0x1E: case 0x1F: m_d[dst] &= 0x0000FFFF; break;

		// ---- add imm8,aD / add imm8,dD (imm8 sign-extended) ----
		case 0x20: case 0x21: case 0x22: case 0x23:
			m_a[dst] = do_add(m_a[dst], (int32_t)(int8_t)read_arg8(m_pc), 0); m_pc += 1; break;
		case 0x28: case 0x29: case 0x2A: case 0x2B:
			m_d[dst] = do_add(m_d[dst], (int32_t)(int8_t)read_arg8(m_pc), 0); m_pc += 1; break;

		// ---- mov imm16,aD (zx) / mov imm16,dD (sx) ----
		case 0x24: case 0x25: case 0x26: case 0x27:
			m_a[dst] = read_arg16(m_pc); m_pc += 2; break;
		case 0x2C: case 0x2D: case 0x2E: case 0x2F:
			m_d[dst] = (int32_t)(int16_t)read_arg16(m_pc); m_pc += 2; break;

		// ---- mov sp,aD ----
		case 0x3C: case 0x3D: case 0x3E: case 0x3F: m_a[dst] = m_sp; break;

		// ---- inc dD (flags) / inc aD (no flags) (D = bits[3:2]) ----
		case 0x40: case 0x44: case 0x48: case 0x4C: m_d[src] = do_add(m_d[src], 1, 0); break;
		case 0x41: case 0x45: case 0x49: case 0x4D: m_a[src] += 1; break;

		// ---- mov dD/aD,(disp8,sp)  (disp8 unsigned) ----
		case 0x42: case 0x46: case 0x4A: case 0x4E:
			write_mem32(m_sp + read_arg8(m_pc), m_d[src]); m_pc += 1; break;
		case 0x43: case 0x47: case 0x4B: case 0x4F:
			write_mem32(m_sp + read_arg8(m_pc), m_a[src]); m_pc += 1; break;

		// ---- inc4 aD (no flags) / asl2 dD ----
		case 0x50: case 0x51: case 0x52: case 0x53: m_a[dst] += 4; break;
		case 0x54: case 0x55: case 0x56: case 0x57: m_d[dst] <<= 2; set_nz32(m_d[dst]); break;

		// ---- mov (disp8,sp),dD / (disp8,sp),aD  (disp8 unsigned) ----
		case 0x58: case 0x59: case 0x5A: case 0x5B:
			m_d[dst] = read_mem32(m_sp + read_arg8(m_pc)); m_pc += 1; break;
		case 0x5C: case 0x5D: case 0x5E: case 0x5F:
			m_a[dst] = read_mem32(m_sp + read_arg8(m_pc)); m_pc += 1; break;

		// ---- mov dS,(aM) (0x60-0x6F) / mov (aM),dD (0x70-0x7F), 32-bit ----
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
			write_mem32(m_a[dst], m_d[src]); break;
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
			m_d[src] = read_mem32(m_a[dst]); break;

		// ---- mov imm8,dD (src==dst) / mov dS,dD ----
		case 0x80: case 0x85: case 0x8A: case 0x8F:
			m_d[dst] = (int32_t)(int8_t)read_arg8(m_pc); m_pc += 1; break;
		case 0x81: case 0x82: case 0x83: case 0x84: case 0x86: case 0x87: case 0x88:
		case 0x89: case 0x8B: case 0x8C: case 0x8D: case 0x8E:
			m_d[dst] = m_d[src]; break;

		// ---- mov imm8,aD (zx, src==dst) / mov aS,aD ----
		case 0x90: case 0x95: case 0x9A: case 0x9F:
			m_a[dst] = (uint8_t)read_arg8(m_pc); m_pc += 1; break;
		case 0x91: case 0x92: case 0x93: case 0x94: case 0x96: case 0x97: case 0x98:
		case 0x99: case 0x9B: case 0x9C: case 0x9D: case 0x9E:
			m_a[dst] = m_a[src]; break;

		// ---- cmp imm8,dD (sx, src==dst) / cmp dS,dD ----
		case 0xA0: case 0xA5: case 0xAA: case 0xAF:
			do_sub(m_d[dst], (int32_t)(int8_t)read_arg8(m_pc), 0); m_pc += 1; break;
		case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA6: case 0xA7: case 0xA8:
		case 0xA9: case 0xAB: case 0xAC: case 0xAD: case 0xAE:
			do_sub(m_d[dst], m_d[src], 0); break;

		// ---- cmp imm8,aD (zx, src==dst) / cmp aS,aD ----
		case 0xB0: case 0xB5: case 0xBA: case 0xBF:
			do_sub(m_a[dst], (uint8_t)read_arg8(m_pc), 0); m_pc += 1; break;
		case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB6: case 0xB7: case 0xB8:
		case 0xB9: case 0xBB: case 0xBC: case 0xBD: case 0xBE:
			do_sub(m_a[dst], m_a[src], 0); break;

		// ---- conditional branches / bra (disp8, target = start_pc + sx8) ----
		case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4:
		case 0xC5: case 0xC6: case 0xC7: case 0xC8: case 0xC9:
		{
			int8_t disp = (int8_t)read_arg8(m_pc); m_pc += 1;
			if (test_cond(op & 0x0F)) m_pc = start_pc + disp;
			break;
		}
		case 0xCA: // bra
		{
			int8_t disp = (int8_t)read_arg8(m_pc); m_pc = start_pc + disp; break;
		}

		// ---- nop ----
		case 0xCB: break;

		// ---- jmp disp16 ----
		case 0xCC:
			m_pc = start_pc + (int32_t)(int16_t)read_arg16(m_pc); break;

		// ---- call disp16, regs, imm8 ----
		case 0xCD:
		{
			int16_t disp = (int16_t)read_arg16(m_pc);
			uint8_t regs = read_arg8(m_pc + 2);
			uint8_t adj  = read_arg8(m_pc + 3);
			uint32_t ret = m_pc + 4;      // return past this 5-byte instruction
			// imm8-total frame convention: return PC at [SP], regs below, SP -= imm8.
			write_mem32(m_sp, ret);
			store_regs_at(m_sp, regs);
			m_sp -= adj;
			m_mdr = ret;                  // AM33 caches the return address in MDR (for retf)
			m_pc = start_pc + disp;
			break;
		}

		// ---- movm (sp),regs (pop) / movm regs,(sp) (push) ----
		case 0xCE: load_regs(read_arg8(m_pc)); m_pc += 1; break;
		case 0xCF: store_regs(read_arg8(m_pc)); m_pc += 1; break;

		case 0xDB: // setlb
			m_lar = m_pc;
			m_lir = read_arg32(m_pc);
			break;
		case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4:
		case 0xD5: case 0xD6: case 0xD7: case 0xD8: case 0xD9: // Lcc
			if (test_cond(op & 0x0F)) m_pc = m_lar;
			break;
		case 0xDA: m_pc = m_lar; break;

		// ---- jmp disp32 (reset vector uses this) ----
		case 0xDC:
			m_pc = start_pc + read_arg32(m_pc); break;

		// ---- call disp32, regs, imm8 ----
		case 0xDD:
		{
			uint32_t disp = read_arg32(m_pc);
			uint8_t regs = read_arg8(m_pc + 4);
			uint8_t adj  = read_arg8(m_pc + 5);
			uint32_t ret = m_pc + 6;
			// imm8-total frame convention (see 0xCD).
			write_mem32(m_sp, ret);
			store_regs_at(m_sp, regs);
			m_sp -= adj;
			m_mdr = ret;           // AM33 caches the return address in MDR (for retf)
			m_pc = start_pc + disp;
			break;
		}

		case 0xDE:
		{
			uint8_t regs = read_arg8(m_pc);
			uint8_t adj  = read_arg8(m_pc + 1);
			m_sp += adj;
			m_pc = m_mdr;
			load_regs_at(m_sp, regs);
			break;
		}

		// ---- ret regs, imm8 : normal return (PC read from the frame top) ----
		case 0xDF:
		{
			uint8_t regs = read_arg8(m_pc);
			uint8_t adj  = read_arg8(m_pc + 1);
			m_sp += adj;
			load_regs_at(m_sp, regs);
			m_pc = read_mem32(m_sp);   // return PC sits at [SP] (the frame top)
			break;
		}

		// ---- add dS,dD ----
		case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
		case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
			m_d[dst] = do_add(m_d[dst], m_d[src], 0); break;

		// ---- prefixed groups ----
		case 0xF0: execute_f0(); break;   // reg-indirect moves + call/jmp/ret (aM)
		case 0xF1: execute_f1(); break;   // cross-type reg-reg arithmetic
		case 0xF2: execute_f2(); break;   // logical / mul / div / shift / special regs
		case 0xF3: execute_f3(); break;   // 32-bit indexed load/store
		case 0xF4: execute_f4(); break;   // byte/half indexed load/store
		case 0xF5: execute_f5(); break;   // AM33 DSP puts (putx/putchclx)
		case 0xF6: execute_f6(); break;   // AM33 DSP ops (mulq/getx/getchx/getclx/sat)
		case 0xF8: execute_f8(); break;   // imm8 / disp8 forms (incl. add imm8,sp)
		case 0xFA: execute_fa(); break;   // imm16 / disp16 forms
		case 0xFC: execute_fc(); break;   // imm32 / disp32 forms (mov imm32,reg etc.)
		case 0xFE: execute_fe(); break;   // bit ops on absolute address

		// TODO: 0xF9,0xFB,0xFD (udf imm) still unimplemented --
		// these fall through to the length-correct skip below until needed.
		default:
			{
				const uint8_t op2 = read_arg8(start_pc + 1);
				if ((op == 0xF9 || op == 0xFB || op == 0xFD) && (op2 >> 4) == 0
						&& !BIT(op2, 2) && !BIT(op2, 3))
				{
					const int dn = op2 & 3;
					int32_t s;
					if (op == 0xF9)      s = (int32_t)(int8_t) read_arg8 (start_pc + 2);
					else if (op == 0xFB) s = (int32_t)(int16_t)read_arg16(start_pc + 2);
					else                 s = (int32_t)        read_arg32(start_pc + 2);
					const int64_t t = (int64_t)(int32_t)m_d[dn] * (int64_t)s;
					m_d[dn] = uint32_t(t);
					m_mdrq  = uint32_t(uint64_t(t) >> 32);
					set_nz32(m_d[dn]);
					m_psw &= ~(FLAG_CF | FLAG_VF);
					m_pc = start_pc + mn10300_insn_length(op, op2);
				}
				else
				{
					m_pc = start_pc + mn10300_insn_length(op, op2);
					logerror("MN10300: unimplemented opcode %02X op2=%02X @ PC=%08X (skipped %d bytes)\n",
						op, op2, start_pc, (int)(m_pc - start_pc));
				}
			}
			break;
		}

		// TODO: real per-instruction cycle counts.
		m_icount -= 1;

	} while (m_icount > 0);
}

inline void mn10300_device::typed_load_store(int type, bool a_reg, int reg, uint32_t ea, bool store)
{
	switch (type)
	{
		case 0: case 1:  // 32-bit mov (a-reg iff a_reg)
			if (a_reg) { if (store) write_mem32(ea, m_a[reg]); else m_a[reg] = read_mem32(ea); }
			else       { if (store) write_mem32(ea, m_d[reg]); else m_d[reg] = read_mem32(ea); }
			break;
		case 2:  if (store) write_mem8 (ea, m_d[reg]); else m_d[reg] = read_mem8 (ea); break; // movbu
		case 3:  if (store) write_mem16(ea, m_d[reg]); else m_d[reg] = read_mem16(ea); break; // movhu
	}
}

inline void mn10300_device::do_shift(int op, int dst, uint32_t count)
{
	count &= 0x1F;
	uint32_t carry = 0;
	if (count)
	{
		if (op == 0)      { carry = (m_d[dst] >> (32 - count)) & 1; m_d[dst] <<= count; }          // asl
		else if (op == 1) { carry = (m_d[dst] >> (count - 1)) & 1;  m_d[dst] >>= count; }           // lsr
		else              { carry = (m_d[dst] >> (count - 1)) & 1;  m_d[dst] = (int32_t)m_d[dst] >> count; } // asr
	}
	m_psw = (m_psw & ~FLAG_CF) | (carry ? FLAG_CF : 0);
	set_nz32(m_d[dst]);
}

// 0xFC: imm32 / disp32 / abs32 forms. Always 6 bytes (op, op2, then a 32-bit
// operand). PC is set to start+6 at the end unless a control-flow op returns.
void mn10300_device::execute_fc()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t  op2 = read_arg8(m_pc);          // start+1
	const uint32_t imm = read_arg32(m_pc + 1);     // start+2 : imm32 / disp32 / abs32
	const int dst = op2 & 3;

	if (op2 < 0x80)                                 // mov/movbu/movhu (disp32,aM)
	{
		const int  type = (op2 >> 5) & 3;
		const bool a_reg = (type == 1);
		typed_load_store(type, a_reg, (op2 >> 2) & 3, m_a[op2 & 3] + imm, op2 & 0x10);
	}
	else if (op2 < 0xA0)                            // store reg -> (disp32,sp) / (abs32)
	{
		const int  type = op2 & 3;
		const bool a_reg = (type == 0);
		typed_load_store(type, a_reg, (op2 >> 2) & 3, (op2 & 0x10) ? m_sp + imm : imm, true);
	}
	else if (op2 < 0xC0)                            // load (disp32,sp)/(abs32) -> reg
	{
		const int  type = (op2 >> 2) & 3;
		const bool a_reg = (type == 0);
		typed_load_store(type, a_reg, op2 & 3, (op2 & 0x10) ? m_sp + imm : imm, false);
	}
	else switch (op2 & 0xFC)
	{
		case 0xC0: m_d[dst] = do_add(m_d[dst], imm, 0); break;   // add imm32,dD
		case 0xD0: m_a[dst] = do_add(m_a[dst], imm, 0); break;   // add imm32,aD
		case 0xC4: m_d[dst] = do_sub(m_d[dst], imm, 0); break;   // sub imm32,dD
		case 0xD4: m_a[dst] = do_sub(m_a[dst], imm, 0); break;   // sub imm32,aD
		case 0xC8: do_sub(m_d[dst], imm, 0); break;              // cmp imm32,dD
		case 0xD8: do_sub(m_a[dst], imm, 0); break;              // cmp imm32,aD
		case 0xCC: m_d[dst] = imm; break;                        // mov imm32,dD
		case 0xDC: m_a[dst] = imm; break;                        // mov imm32,aD
		case 0xE0: m_d[dst] &= imm; set_logic_flags(m_d[dst]); break; // and
		case 0xE4: m_d[dst] |= imm; set_logic_flags(m_d[dst]); break; // or
		case 0xE8: m_d[dst] ^= imm; set_logic_flags(m_d[dst]); break; // xor
		case 0xEC: m_psw = (m_psw & ~FLAG_ZF) | ((m_d[dst] & imm) ? 0 : FLAG_ZF); break; // btst imm32,dD
		case 0xFC:
			if (op2 == 0xFE)      { m_sp += imm; }               // add imm32,sp (SP arith, no flags)
			else if (op2 == 0xFF) { write_mem32(m_sp, start_pc + 6); m_mdr = start_pc + 6; m_pc = start_pc + imm; return; } // calls (disp32): PC->[SP], SP unchanged
			else logerror("MN10300: unimplemented FC %02X @ %08X\n", op2, start_pc);
			break;
		default:
			logerror("MN10300: unimplemented FC %02X @ %08X\n", op2, start_pc);
			break;
	}
	m_pc = start_pc + 6;
}

void mn10300_device::execute_f5()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t op2 = read_arg8(m_pc);
	const int dst = op2 & 3, src = (op2 >> 2) & 3;
	switch (op2 >> 4)
	{
	case 0x0: m_mdrq = m_d[dst]; break;                       // putx     Dn   -> MDRQ
	case 0x1: m_mcrh = m_d[src]; m_mcrl = m_d[dst]; break;    // putchclx Dm->MCRH, Dn->MCRL
	default:  logerror("MN10300: unimplemented F5 %02X @ %08X\n", op2, start_pc); break;
	}
	m_pc = start_pc + 2;
}

void mn10300_device::execute_f6()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t op2 = read_arg8(m_pc);
	const int dst = op2 & 3, src = (op2 >> 2) & 3;
	switch (op2 >> 4)
	{
	case 0x0: { int64_t t = (int64_t)(int32_t)m_d[dst] * (int32_t)m_d[src];   // mulq
				m_d[dst] = uint32_t(t); m_mdrq = uint32_t(uint64_t(t) >> 32);
				set_nz32(m_d[dst]); m_psw &= ~(FLAG_CF | FLAG_VF); } break;
	case 0x1: { uint64_t t = (uint64_t)m_d[dst] * (uint64_t)m_d[src];         // mulqu
				m_d[dst] = uint32_t(t); m_mdrq = uint32_t(t >> 32);
				set_nz32(m_d[dst]); m_psw &= ~(FLAG_CF | FLAG_VF); } break;
	case 0x4: { int32_t v = (int32_t)m_d[src];                               // sat16
				v = std::clamp(v, -0x8000, 0x7fff); m_d[dst] = uint32_t(v); } break;
	case 0x5: { int32_t v = (int32_t)m_d[src];                               // sat24
				v = std::clamp(v, -0x800000, 0x7fffff); m_d[dst] = uint32_t(v); } break;
	case 0xC: m_d[dst] = m_mcrh; break;                                      // getchx MCRH -> Dn
	case 0xD: m_d[dst] = m_mcrl; break;                                      // getclx MCRL -> Dn
	case 0xF: m_d[dst] = m_mdrq; break;                                     // getx MDRQ -> Dn (no flag change: the ISR context-save re-reads PSW after this, so it must not touch flags)
	case 0x7: {  // udf07 Dm,Dn -- BSCH (bit search): Dn = position (0..15) of the
		// most-significant set bit in Dm's low 16 bits (0 if none). Used by the
		// Huffman leading-run decode in software JPEG decoders.
		const uint32_t v = m_d[src] & 0xffff;
		m_d[dst] = v ? uint32_t(31 - std::countl_zero(v)) : 0u;
	} break;
	default:  logerror("MN10300: unimplemented F6 %02X @ %08X\n", op2, start_pc); break;
	}
	m_pc = start_pc + 2;
}

// 0xF8: imm8 / disp8 forms. Always 3 bytes (op, op2, then one operand byte).
void mn10300_device::execute_f8()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t  op2   = read_arg8(m_pc);        // start+1
	const uint8_t  b     = read_arg8(m_pc + 1);    // start+2 : disp8 / imm8 / shift count
	const int8_t   sdisp = (int8_t)b;

	if (op2 < 0x80)                                 // mov/movbu/movhu (disp8,aM)
	{
		const int  type = (op2 >> 5) & 3;
		const bool a_reg = (type == 1);
		typed_load_store(type, a_reg, (op2 >> 2) & 3, m_a[op2 & 3] + sdisp, op2 & 0x10);
	}
	else if ((op2 & 0xF2) == 0x92)                  // movbu/movhu dD,(disp8,sp) store (disp8 unsigned)
	{
		typed_load_store(op2 & 3, false, (op2 >> 2) & 3, m_sp + b, true);
	}
	else switch (op2 & 0xFC)
	{
		case 0xB8: case 0xBC:                        // movbu/movhu (disp8,sp),dD load (disp8 unsigned)
			typed_load_store((op2 >> 2) & 3, false, op2 & 3, m_sp + b, false);
			break;
		case 0xC0: case 0xC4: case 0xC8:             // asl/lsr/asr imm8,dD
			do_shift((op2 >> 2) & 3, op2 & 3, b);
			break;
		case 0xE0: case 0xE4: case 0xEC:             // and/or/btst imm8(zx),dD
		{
			const int d = op2 & 3;
			const int lop = (op2 >> 2) & 3;          // 0=and 1=or 3=btst
			if (lop == 0)      { m_d[d] &= b; set_logic_flags(m_d[d]); }
			else if (lop == 1) { m_d[d] |= b; set_logic_flags(m_d[d]); }
			else               { m_psw = (m_psw & ~FLAG_ZF) | ((m_d[d] & b) ? 0 : FLAG_ZF); } // btst
			break;
		}
		case 0xE8:                                   // ext-branch bvc/bvs/bnc/bns
		{
			bool take = false;
			switch (op2 & 3)
			{
				case 0: take = !(m_psw & FLAG_VF); break; // bvc
				case 1: take =  (m_psw & FLAG_VF); break; // bvs
				case 2: take = !(m_psw & FLAG_NF); break; // bnc
				case 3: take =  (m_psw & FLAG_NF); break; // bns
			}
			if (take) { m_pc = start_pc + sdisp; return; }
			break;
		}
		case 0xF0: m_sp = read_mem32(m_a[op2 & 3] + sdisp); break;  // mov (disp8,aM),sp
		case 0xF4: write_mem32(m_a[op2 & 3] + sdisp, m_sp); break;  // mov sp,(disp8,aM)
		case 0xFC:
			if (op2 == 0xFE) m_sp += (int32_t)sdisp;  // add imm8,sp (SP arith, no flags)
			else logerror("MN10300: unimplemented F8 %02X @ %08X\n", op2, start_pc);
			break;
		default:
			logerror("MN10300: unimplemented F8 %02X @ %08X\n", op2, start_pc);
			break;
	}
	m_pc = start_pc + 3;
}

inline void mn10300_device::set_logic_flags(uint32_t r)
{
	// and/or/xor/not: set Z,N; clear V (C left undefined -> cleared here).
	m_psw &= ~(FLAG_ZF | FLAG_NF | FLAG_CF | FLAG_VF);
	if (r == 0)          m_psw |= FLAG_ZF;
	if (r & 0x80000000u) m_psw |= FLAG_NF;
}

// 0xF0: reg-indirect moves and indirect call/jmp/ret. Always 2 bytes.
void mn10300_device::execute_f0()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t op2 = read_arg8(m_pc);
	const int r  = (op2 >> 2) & 3;   // data/addr register field
	const int am = op2 & 3;          // base address register (aM)

	switch (op2 >> 4)
	{
		case 0x0: m_a[r] = read_mem32(m_a[am]); break;                 // mov (aM),aD
		case 0x4: m_d[r] = read_mem8 (m_a[am]); break;                 // movbu (aM),dD
		case 0x6: m_d[r] = read_mem16(m_a[am]); break;                 // movhu (aM),dD
		case 0x1: write_mem32(m_a[am], m_a[r]); break;                 // mov aS,(aM)
		case 0x5: write_mem8 (m_a[am], m_d[r]); break;                 // movbu dS,(aM)
		case 0x7: write_mem16(m_a[am], m_d[r]); break;                 // movhu dS,(aM)
		case 0x8: { uint8_t v = read_mem8(m_a[am]); m_psw = (m_psw & ~FLAG_ZF) | ((v & m_d[r]) ? 0 : FLAG_ZF); write_mem8(m_a[am], v | m_d[r]); break; } // bset
		case 0x9: { uint8_t v = read_mem8(m_a[am]); m_psw = (m_psw & ~FLAG_ZF) | ((v & m_d[r]) ? 0 : FLAG_ZF); write_mem8(m_a[am], v & ~m_d[r]); break; } // bclr
		case 0xF:
			// Control flow, selected by the whole op2 value.
			if (op2 <= 0xF3)      { write_mem32(m_sp, start_pc + 2); m_mdr = start_pc + 2; m_pc = m_a[am]; return; } // calls (aM): PC->[SP], SP unchanged
			else if (op2 <= 0xF7) { m_pc = m_a[am]; return; }                       // jmp (aM)
			else if (op2 == 0xFC) { m_pc = read_mem32(m_sp); return; }               // rets: PC from [SP], SP unchanged
			else if (op2 == 0xFD) { m_psw = pop32(); m_pc = pop32(); return; }      // rti
			// TODO: FE trap
			else logerror("MN10300: unimplemented F0 %02X @ %08X\n", op2, start_pc);
			break;
		default:
			logerror("MN10300: unimplemented F0 %02X @ %08X\n", op2, start_pc);
			break;
	}
	m_pc = start_pc + 2;
}

// 0xF1: cross-type reg-reg arithmetic (between data and address registers).
// Always 2 bytes. dst=bits[1:0], src=bits[3:2].
void mn10300_device::execute_f1()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t op2 = read_arg8(m_pc);
	const int dst = op2 & 3, src = (op2 >> 2) & 3;
	const uint32_t C = (m_psw & FLAG_CF) ? 1 : 0;

	switch (op2 >> 4)
	{
		case 0x0: m_d[dst] = do_sub(m_d[dst], m_d[src], 0); break; // sub dS,dD
		case 0x1: m_d[dst] = do_sub(m_d[dst], m_a[src], 0); break; // sub aS,dD
		case 0x2: m_a[dst] = do_sub(m_a[dst], m_d[src], 0); break; // sub dS,aD
		case 0x3: m_a[dst] = do_sub(m_a[dst], m_a[src], 0); break; // sub aS,aD
		case 0x4: m_d[dst] = do_add(m_d[dst], m_d[src], C); break; // addc dS,dD
		case 0x5: m_d[dst] = do_add(m_d[dst], m_a[src], 0); break; // add aS,dD
		case 0x6: m_a[dst] = do_add(m_a[dst], m_d[src], 0); break; // add dS,aD
		case 0x7: m_a[dst] = do_add(m_a[dst], m_a[src], 0); break; // add aS,aD
		case 0x8: m_d[dst] = do_sub(m_d[dst], m_d[src], C); break; // subc dS,dD
		case 0x9: do_sub(m_d[dst], m_a[src], 0); break;            // cmp aS,dD
		case 0xA: do_sub(m_a[dst], m_d[src], 0); break;            // cmp dS,aD
		case 0xD: m_d[dst] = m_a[src]; break;                      // mov aS,dD
		case 0xE: m_a[dst] = m_d[src]; break;                      // mov dS,aD
		default:  logerror("MN10300: illegal F1 %02X @ %08X\n", op2, start_pc); break;
	}
	m_pc = start_pc + 2;
}

// 0xF2: logical / mul / div / shift / special-register moves. Always 2 bytes.
void mn10300_device::execute_f2()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t op2 = read_arg8(m_pc);
	const int dst = op2 & 3, src = (op2 >> 2) & 3;

	switch (op2 >> 4)
	{
		case 0x0: m_d[dst] &= m_d[src]; set_logic_flags(m_d[dst]); break; // and
		case 0x1: m_d[dst] |= m_d[src]; set_logic_flags(m_d[dst]); break; // or
		case 0x2: m_d[dst] ^= m_d[src]; set_logic_flags(m_d[dst]); break; // xor
		case 0x3:
			if (op2 < 0x34) { m_d[dst] = ~m_d[dst]; set_logic_flags(m_d[dst]); } // not dD
			else logerror("MN10300: illegal F2 %02X @ %08X\n", op2, start_pc);
			break;
		case 0x4: { int64_t p = (int64_t)(int32_t)m_d[src] * (int32_t)m_d[dst]; m_d[dst] = (uint32_t)p; m_mdr = (uint32_t)(p >> 32); set_nz32(m_d[dst]); break; } // mul
		case 0x5: { uint64_t p = (uint64_t)m_d[src] * m_d[dst]; m_d[dst] = (uint32_t)p; m_mdr = (uint32_t)(p >> 32); set_nz32(m_d[dst]); break; } // mulu
		case 0x6: // div (signed): (MDR:dD) / dS
		{
			const int64_t num = ((int64_t)m_mdr << 32) | m_d[dst];
			const int32_t dv = (int32_t)m_d[src];
			// INT64_MIN / -1 traps on the host, and so does the matching %, so the
			// no-result cases are decided BEFORE either operator runs.
			const bool no_result = (dv == 0) || (num == INT64_MIN && dv == -1);
			const int64_t q = no_result ? 0 : num / dv;
			if (!no_result && q >= INT32_MIN && q <= INT32_MAX)
			{
				m_d[dst] = (uint32_t)q;
				m_mdr = (uint32_t)(num % dv);
				m_psw &= ~FLAG_VF;
				set_nz32(m_d[dst]);
			}
			else
				m_psw |= FLAG_VF;
			break;
		}
		case 0x7: // divu (unsigned)
		{
			const uint64_t num = ((uint64_t)m_mdr << 32) | m_d[dst];
			const uint32_t dv = m_d[src];
			if (dv && (num / dv) <= 0xFFFFFFFFULL)
			{
				m_d[dst] = (uint32_t)(num / dv);
				m_mdr = (uint32_t)(num % dv);
				m_psw &= ~FLAG_VF;
				set_nz32(m_d[dst]);
			}
			else
				m_psw |= FLAG_VF;
			break;
		}
		case 0x8: // rol / ror dD (rotate through carry)
		{
			uint32_t c = (m_psw & FLAG_CF) ? 1 : 0;
			if (op2 < 0x84) { uint32_t nc = m_d[dst] >> 31; m_d[dst] = (m_d[dst] << 1) | c; m_psw = (m_psw & ~FLAG_CF) | (nc ? FLAG_CF : 0); } // rol
			else            { uint32_t nc = m_d[dst] & 1;   m_d[dst] = (m_d[dst] >> 1) | (c << 31); m_psw = (m_psw & ~FLAG_CF) | (nc ? FLAG_CF : 0); } // ror
			set_nz32(m_d[dst]);
			break;
		}
		case 0x9: case 0xA: case 0xB: // asl / lsr / asr dS,dD (shift dD by dS)
		{
			uint32_t n = m_d[src] & 0x1F;
			uint32_t carry = 0;
			if (n)
			{
				if ((op2 >> 4) == 0x9)      { carry = (m_d[dst] >> (32 - n)) & 1; m_d[dst] <<= n; }        // asl
				else if ((op2 >> 4) == 0xA) { carry = (m_d[dst] >> (n - 1)) & 1;  m_d[dst] >>= n; }         // lsr
				else                        { carry = (m_d[dst] >> (n - 1)) & 1;  m_d[dst] = (int32_t)m_d[dst] >> n; } // asr
			}
			m_psw = (m_psw & ~FLAG_CF) | (carry ? FLAG_CF : 0);
			set_nz32(m_d[dst]);
			break;
		}
		case 0xD:
			if (op2 < 0xD4) m_mdr = (m_d[dst] & 0x80000000u) ? 0xFFFFFFFFu : 0; // ext dD
			else logerror("MN10300: illegal F2 %02X @ %08X\n", op2, start_pc);
			break;
		case 0xE:
			if (op2 < 0xE4)      m_d[dst] = m_mdr;       // mov mdr,dD
			else if (op2 < 0xE8) m_d[dst] = m_psw;       // mov psw,dD
			else logerror("MN10300: illegal F2 %02X @ %08X\n", op2, start_pc);
			break;
		case 0xF:
			if (op2 & 2) { if (op2 & 1) m_psw = m_d[src]; else m_mdr = m_d[src]; } // mov dS,psw / mov dS,mdr
			else if ((op2 & 1) == 0) m_sp = m_a[src];                              // mov aS,sp
			else logerror("MN10300: illegal F2 %02X @ %08X\n", op2, start_pc);
			break;
		default: logerror("MN10300: unimplemented F2 %02X @ %08X\n", op2, start_pc); break;
	}
	m_pc = start_pc + 2;
}

// 0xF3: 32-bit indexed load/store, EA = aM + dI. Always 2 bytes.
void mn10300_device::execute_f3()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t op2 = read_arg8(m_pc);
	const bool    store  = op2 & 0x40;
	const bool    a_reg  = op2 & 0x80;
	const int     reg    = (op2 >> 4) & 3;
	const int     dI     = (op2 >> 2) & 3;
	const int     am     = op2 & 3;
	const uint32_t ea    = m_a[am] + m_d[dI];

	if (store) write_mem32(ea, a_reg ? m_a[reg] : m_d[reg]);
	else      (a_reg ? m_a[reg] : m_d[reg]) = read_mem32(ea);
	m_pc = start_pc + 2;
}

// 0xF4: byte/half indexed load/store, EA = aM + dI. Always 2 bytes.
void mn10300_device::execute_f4()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t op2 = read_arg8(m_pc);
	const bool    half   = op2 & 0x80;   // movhu vs movbu
	const bool    store  = op2 & 0x40;
	const int     reg    = (op2 >> 4) & 3;
	const int     dI     = (op2 >> 2) & 3;
	const int     am     = op2 & 3;
	const uint32_t ea    = m_a[am] + m_d[dI];

	if (store) { if (half) write_mem16(ea, m_d[reg]); else write_mem8(ea, m_d[reg]); }
	else       { m_d[reg] = half ? read_mem16(ea) : read_mem8(ea); }
	m_pc = start_pc + 2;
}

// 0xFA: imm16 / disp16 forms. Always 4 bytes (op, op2, imm16-lo, imm16-hi).
void mn10300_device::execute_fa()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t op2 = read_arg8(m_pc);
	const uint16_t imm16 = read_arg16(m_pc + 1);   // operand at start+2
	const int dst = op2 & 3;

	if (op2 < 0x80)
	{
		// mov/movbu/movhu (disp16,aM): type=bits[6:5], bit4=dir(store), reg=bits[3:2], aM=bits[1:0]
		const int type = (op2 >> 5) & 3;   // 0=mov.d 1=mov.a 2=movbu.d 3=movhu.d
		const bool store = op2 & 0x10;
		const int r = (op2 >> 2) & 3, am = op2 & 3;
		const uint32_t ea = m_a[am] + (int32_t)(int16_t)imm16;
		switch (type)
		{
			case 0: if (store) write_mem32(ea, m_d[r]); else m_d[r] = read_mem32(ea); break;
			case 1: if (store) write_mem32(ea, m_a[r]); else m_a[r] = read_mem32(ea); break;
			case 2: if (store) write_mem8 (ea, m_d[r]); else m_d[r] = read_mem8 (ea); break;
			case 3: if (store) write_mem16(ea, m_d[r]); else m_d[r] = read_mem16(ea); break;
		}
	}
	else
	{
		switch (op2 & 0xFC)
		{
			case 0xC0: m_d[dst] = do_add(m_d[dst], (int32_t)(int16_t)imm16, 0); break; // add imm16,dD (sx)
			case 0xD0: m_a[dst] = do_add(m_a[dst], (int32_t)(int16_t)imm16, 0); break; // add imm16,aD (sx)
			case 0xC8: do_sub(m_d[dst], (int32_t)(int16_t)imm16, 0); break;            // cmp imm16,dD (sx)
			case 0xD8: do_sub(m_a[dst], (uint32_t)imm16, 0); break;                    // cmp imm16,aD (zx)
			case 0xE0: m_d[dst] &= (uint32_t)imm16; set_logic_flags(m_d[dst]); break;  // and imm16,dD (zx)
			case 0xE4: m_d[dst] |= (uint32_t)imm16; set_logic_flags(m_d[dst]); break;  // or
			case 0xE8: m_d[dst] ^= (uint32_t)imm16; set_logic_flags(m_d[dst]); break;  // xor
			case 0xEC: m_psw = (m_psw & ~FLAG_ZF) | ((m_d[dst] & (uint32_t)imm16) ? 0 : FLAG_ZF); break; // btst imm16,dD (zx)
			case 0xB0: m_a[dst] = read_mem32(m_sp + imm16); break;                     // mov (disp16,sp),aD
			case 0xB4: m_d[dst] = read_mem32(m_sp + imm16); break;                     // mov (disp16,sp),dD
			case 0xB8: m_d[dst] = read_mem8 (m_sp + imm16); break;                     // movbu (disp16,sp),dD
			case 0xBC: m_d[dst] = read_mem16(m_sp + imm16); break;                     // movhu (disp16,sp),dD
			// store reg -> (disp16,sp): op2 = 1001_rrtt, reg=bits[3:2], tt=bits[1:0]
			// (0 mov aM / 1 mov dM / 2 movbu dM / 3 movhu dM). binutils 0xfa90..0xfa93.
			case 0x90: case 0x94: case 0x98: case 0x9C:
			{
				const int reg = (op2 >> 2) & 3;
				const uint32_t ea = m_sp + imm16;
				switch (op2 & 3)
				{
					case 0: write_mem32(ea, m_a[reg]); break; // mov   aM,(disp16,sp)
					case 1: write_mem32(ea, m_d[reg]); break; // mov   dM,(disp16,sp)
					case 2: write_mem8 (ea, m_d[reg]); break; // movbu dM,(disp16,sp)
					case 3: write_mem16(ea, m_d[reg]); break; // movhu dM,(disp16,sp)
				}
				break;
			}
			case 0xA0: m_a[dst] = read_mem32(imm16); break;                            // mov (abs16),aD
			case 0xFC:
				if (op2 == 0xFE)      { m_sp += (int32_t)(int16_t)imm16; }             // add imm16,sp
				else if (op2 == 0xFF) { write_mem32(m_sp, start_pc + 4); m_mdr = start_pc + 4; m_pc = start_pc + (int32_t)(int16_t)imm16; return; } // calls (disp16): PC->[SP], SP unchanged
				else if (op2 == 0xFC) { m_psw &= (uint32_t)imm16; }                    // and imm16,psw
				else if (op2 == 0xFD) { m_psw |= (uint32_t)imm16; }                    // or imm16,psw
				break;
			default:
				logerror("MN10300: unimplemented FA %02X @ %08X\n", op2, start_pc);
				break;
		}
	}
	m_pc = start_pc + 4;
}

// 0xFE: bit set/clear/test on an absolute address. 5 bytes (abs16) or 7 (abs32).
void mn10300_device::execute_fe()
{
	const uint32_t start_pc = m_pc - 1;
	const uint8_t op2 = read_arg8(m_pc);
	uint32_t addr; uint8_t imm8; int len;

	if (op2 <= 0x02)      { addr = read_arg32(m_pc + 1); imm8 = read_arg8(m_pc + 5); len = 7; }
	else if (op2 >= 0x80 && op2 <= 0x82) { addr = read_arg16(m_pc + 1); imm8 = read_arg8(m_pc + 3); len = 5; }
	else { logerror("MN10300: illegal FE %02X @ %08X\n", op2, start_pc); m_pc = start_pc + 2; return; }

	const uint8_t v = read_mem8(addr);
	m_psw = (m_psw & ~FLAG_ZF) | ((v & imm8) ? 0 : FLAG_ZF);
	const int kind = op2 & 0x0F;
	if (kind == 0x00)      write_mem8(addr, v | imm8);    // bset
	else if (kind == 0x01) write_mem8(addr, v & ~imm8);   // bclr
	// kind == 0x02: btst (no write)
	m_pc = start_pc + len;
}

void mn10300_device::store_regs(uint8_t mask)  // push (regs -> stack, SP down)
{
	if (mask & 0x04) { push32(m_e[2]); push32(m_e[3]); }
	if (mask & 0x02) { push32(m_e[4]); push32(m_e[5]); push32(m_e[6]); push32(m_e[7]); }
	if (mask & 0x01) { push32(m_e[0]); push32(m_e[1]); m_sp -= 16; } // + MDRQ/MCRH/MCRL/MCVF gap
	if (mask & 0x80) push32(m_d[2]);
	if (mask & 0x40) push32(m_d[3]);
	if (mask & 0x20) push32(m_a[2]);
	if (mask & 0x10) push32(m_a[3]);
	if (mask & 0x08) // {D0,D1,A0,A1,MDR,LIR,LAR} + a 4-byte dummy slot
	{
		push32(m_d[0]); push32(m_d[1]); push32(m_a[0]); push32(m_a[1]);
		push32(m_mdr);  push32(m_lir);  push32(m_lar);  m_sp -= 4;
	}
}
void mn10300_device::load_regs(uint8_t mask)   // pop (stack -> regs, SP up); exact reverse
{
	if (mask & 0x08) // {D0,D1,A0,A1,MDR,LIR,LAR} + dummy (reverse of store)
	{
		m_sp += 4;   // skip the dummy slot
		m_lar = pop32();
		m_lir = pop32();
		m_mdr = pop32();
		m_a[1] = pop32();
		m_a[0] = pop32();
		m_d[1] = pop32();
		m_d[0] = pop32();
	}
	if (mask & 0x10) m_a[3] = pop32();
	if (mask & 0x20) m_a[2] = pop32();
	if (mask & 0x40) m_d[3] = pop32();
	if (mask & 0x80) m_d[2] = pop32();
	if (mask & 0x01) { m_sp += 16; m_e[1] = pop32(); m_e[0] = pop32(); } // MDRQ/... gap + E0/E1
	if (mask & 0x02) { m_e[7] = pop32(); m_e[6] = pop32(); m_e[5] = pop32(); m_e[4] = pop32(); }
	if (mask & 0x04) { m_e[3] = pop32(); m_e[2] = pop32(); }
}

void mn10300_device::store_regs_at(uint32_t base, uint8_t mask)
{
	int32_t off = -4;
	if (mask & 0x04) { write_mem32(base + off, m_e[2]); off -= 4; write_mem32(base + off, m_e[3]); off -= 4; }
	if (mask & 0x02) { write_mem32(base + off, m_e[4]); off -= 4; write_mem32(base + off, m_e[5]); off -= 4;
					   write_mem32(base + off, m_e[6]); off -= 4; write_mem32(base + off, m_e[7]); off -= 4; }
	if (mask & 0x01) { off -= 16; write_mem32(base + off, m_e[0]); off -= 4; write_mem32(base + off, m_e[1]); off -= 4; }
	if (mask & 0x80) { write_mem32(base + off, m_d[2]); off -= 4; }
	if (mask & 0x40) { write_mem32(base + off, m_d[3]); off -= 4; }
	if (mask & 0x20) { write_mem32(base + off, m_a[2]); off -= 4; }
	if (mask & 0x10) { write_mem32(base + off, m_a[3]); off -= 4; }
	if (mask & 0x08) { write_mem32(base + off, m_d[0]); off -= 4; write_mem32(base + off, m_d[1]); off -= 4;
					   write_mem32(base + off, m_a[0]); off -= 4; write_mem32(base + off, m_a[1]); off -= 4;
					   write_mem32(base + off, m_mdr); off -= 4; write_mem32(base + off, m_lir); off -= 4;
					   write_mem32(base + off, m_lar); off -= 4; }
}
void mn10300_device::load_regs_at(uint32_t base, uint8_t mask)
{
	int32_t off = -4;
	if (mask & 0x04) { m_e[2] = read_mem32(base + off); off -= 4; m_e[3] = read_mem32(base + off); off -= 4; }
	if (mask & 0x02) { m_e[4] = read_mem32(base + off); off -= 4; m_e[5] = read_mem32(base + off); off -= 4;
					   m_e[6] = read_mem32(base + off); off -= 4; m_e[7] = read_mem32(base + off); off -= 4; }
	if (mask & 0x01) { off -= 16; m_e[0] = read_mem32(base + off); off -= 4; m_e[1] = read_mem32(base + off); off -= 4; }
	if (mask & 0x80) { m_d[2] = read_mem32(base + off); off -= 4; }
	if (mask & 0x40) { m_d[3] = read_mem32(base + off); off -= 4; }
	if (mask & 0x20) { m_a[2] = read_mem32(base + off); off -= 4; }
	if (mask & 0x10) { m_a[3] = read_mem32(base + off); off -= 4; }
	if (mask & 0x08) { m_d[0] = read_mem32(base + off); off -= 4; m_d[1] = read_mem32(base + off); off -= 4;
					   m_a[0] = read_mem32(base + off); off -= 4; m_a[1] = read_mem32(base + off); off -= 4;
					   m_mdr = read_mem32(base + off); off -= 4; m_lir = read_mem32(base + off); off -= 4;
					   m_lar = read_mem32(base + off); off -= 4; }
}
