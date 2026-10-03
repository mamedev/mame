// license:BSD-3-Clause
// copyright-holders:Felipe Sanches

// Panasonic MN10300 (MN1030 series) execution core.

#include "emu.h"
#include "mn10300.h"

#include "mn103dasm.h"

#include <algorithm>
#include <bit>
#include <iterator>
#include <limits>


namespace {

enum : u16
{
	FLAG_ZF = 0x0001,
	FLAG_NF = 0x0002,
	FLAG_CF = 0x0004,
	FLAG_VF = 0x0008,
	FLAG_IM = 0x0700,   // interrupt mask level
	FLAG_IE = 0x0800
};

constexpr int IM_SHIFT = 8;

// Length of an instruction the main decoder does not handle (F7, F9, FB, FD and FF)
int insn_length(u8 op)
{
	switch (op)
	{
	case 0xf9: return 3;
	case 0xfb: return 4;
	case 0xfd: return 6;
	default:   return 1;
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE(MN103002A, mn103002a_device, "mn103002a", "Panasonic MN103002A")

mn103002a_device::mn103002a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mn10300_device(mconfig, MN103002A, tag, owner, clock, address_map_constructor(FUNC(mn103002a_device::mn103002a_internal_map), this))
{
}

mn10300_device::mn10300_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor program)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0, program)
	, m_reset_pc(0x4000'0000)
	, m_vector_base(0x4000'0000)
	, m_pc(0)
	, m_d{ 0, 0, 0, 0 }
	, m_a{ 0, 0, 0, 0 }
	, m_sp(0)
	, m_mdr(0)
	, m_mdrq(0)
	, m_mcrh(0)
	, m_mcrl(0)
	, m_psw(0)
	, m_lir(0)
	, m_lar(0)
	, m_icount(0)
	, m_gxicr{}
	, m_iagr(0)
	, m_extmd(0)
	, m_ivar{}
	, m_irq_pin{}
	, m_irq_pending(false)
	, m_irq_level(7)
	, m_tm_mode{ 0, 0 }
	, m_tm_base{ 0, 0 }
	, m_tm_timer{ nullptr, nullptr }
	, m_sio_tx_cb(*this)
	, m_sio_rx_enable_cb(*this)
	, m_sio_bit_rate{ 0, 0, 0 }
	, m_sio_async{ false, false, false }
	, m_sio_tx_timer{ nullptr, nullptr, nullptr }
	, m_sio_config{ 0, 0, 0 }
	, m_sio_control{ 0, 0, 0 }
	, m_sio_rxbuf{ 0, 0, 0 }
	, m_sio_rx_full{ false, false, false }
{
}

void mn10300_device::mn103002a_internal_map(address_map &map)
{
	map(0x20000000, 0x2000001b).rw(FUNC(mn10300_device::ivar_r), FUNC(mn10300_device::ivar_w));

	map(0x34000100, 0x340002ff).noprw();
	map(0x34000100, 0x34000101).r(FUNC(mn10300_device::iagr_r));
	map(0x34000108, 0x3400017f).rw(FUNC(mn10300_device::gxicr_r), FUNC(mn10300_device::gxicr_w));
	map(0x34000200, 0x34000201).r(FUNC(mn10300_device::group_level_r));
	map(0x34000280, 0x34000281).rw(FUNC(mn10300_device::extmd_r), FUNC(mn10300_device::extmd_w));

	map(0x34000800, 0x3400082f).noprw();
	map(0x34000800, 0x34000801).rw(FUNC(mn10300_device::sio_config_r<0>), FUNC(mn10300_device::sio_config_w<0>));
	map(0x34000804, 0x34000804).rw(FUNC(mn10300_device::sio_control_r<0>), FUNC(mn10300_device::sio_control_w<0>));
	map(0x34000808, 0x34000808).w(FUNC(mn10300_device::sio_txd_w<0>));
	map(0x34000809, 0x34000809).r(FUNC(mn10300_device::sio_rxd_r<0>));
	map(0x3400080c, 0x3400080d).r(FUNC(mn10300_device::sio_status_r<0>));
	map(0x34000810, 0x34000811).rw(FUNC(mn10300_device::sio_config_r<1>), FUNC(mn10300_device::sio_config_w<1>));
	map(0x34000814, 0x34000814).rw(FUNC(mn10300_device::sio_control_r<1>), FUNC(mn10300_device::sio_control_w<1>));
	map(0x34000818, 0x34000818).w(FUNC(mn10300_device::sio_txd_w<1>));
	map(0x34000819, 0x34000819).r(FUNC(mn10300_device::sio_rxd_r<1>));
	map(0x3400081c, 0x3400081d).r(FUNC(mn10300_device::sio_status_r<1>));
	map(0x34000820, 0x34000821).rw(FUNC(mn10300_device::sio_config_r<2>), FUNC(mn10300_device::sio_config_w<2>));
	map(0x34000824, 0x34000824).rw(FUNC(mn10300_device::sio_control_r<2>), FUNC(mn10300_device::sio_control_w<2>));
	map(0x34000828, 0x34000828).w(FUNC(mn10300_device::sio_txd_w<2>));
	map(0x34000829, 0x34000829).r(FUNC(mn10300_device::sio_rxd_r<2>));
	map(0x3400082c, 0x3400082d).r(FUNC(mn10300_device::sio_status_r<2>));

	map(0x34001080, 0x34001080).rw(FUNC(mn10300_device::tm_mode_r<0>), FUNC(mn10300_device::tm_mode_w<0>));
	map(0x34001082, 0x34001082).rw(FUNC(mn10300_device::tm_mode_r<1>), FUNC(mn10300_device::tm_mode_w<1>));
	map(0x34001090, 0x34001091).rw(FUNC(mn10300_device::tm_base_r<0>), FUNC(mn10300_device::tm_base_w<0>));
	map(0x34001092, 0x34001093).rw(FUNC(mn10300_device::tm_base_r<1>), FUNC(mn10300_device::tm_base_w<1>));
	map(0x340010a0, 0x340010a1).r(FUNC(mn10300_device::tm_count_r<0>));
	map(0x340010a2, 0x340010a3).r(FUNC(mn10300_device::tm_count_r<1>));
}

device_memory_interface::space_config_vector mn10300_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(AS_PROGRAM, &m_program_config) };
}

std::unique_ptr<util::disasm_interface> mn10300_device::create_disassembler()
{
	return std::make_unique<mn10300_disassembler>();
}

void mn10300_device::device_start()
{
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);

	for (auto &timer : m_tm_timer)
		timer = timer_alloc(FUNC(mn10300_device::tm_underflow), this);
	for (auto &timer : m_sio_tx_timer)
		timer = timer_alloc(FUNC(mn10300_device::sio_tx_shifted), this);

	save_item(NAME(m_pc));
	save_item(NAME(m_d));
	save_item(NAME(m_a));
	save_item(NAME(m_sp));
	save_item(NAME(m_mdr));
	save_item(NAME(m_mdrq));
	save_item(NAME(m_mcrh));
	save_item(NAME(m_mcrl));
	save_item(NAME(m_psw));
	save_item(NAME(m_lir));
	save_item(NAME(m_lar));
	save_item(NAME(m_gxicr));
	save_item(NAME(m_iagr));
	save_item(NAME(m_extmd));
	save_item(NAME(m_ivar));
	save_item(NAME(m_irq_pin));
	save_item(NAME(m_irq_pending));
	save_item(NAME(m_irq_level));
	save_item(NAME(m_tm_mode));
	save_item(NAME(m_tm_base));
	save_item(NAME(m_sio_config));
	save_item(NAME(m_sio_control));
	save_item(NAME(m_sio_rxbuf));
	save_item(NAME(m_sio_rx_full));

	state_add(MN10300_PC, "PC", m_pc).formatstr("%08X");
	state_add(MN10300_SP, "SP", m_sp).formatstr("%08X");
	state_add(MN10300_PSW, "PSW", m_psw).formatstr("%04X");
	state_add(MN10300_MDR, "MDR", m_mdr).formatstr("%08X");
	for (int i = 0; i < 4; i++)
		state_add(MN10300_D0 + i, util::string_format("D%d", i), m_d[i]).formatstr("%08X");
	for (int i = 0; i < 4; i++)
		state_add(MN10300_A0 + i, util::string_format("A%d", i), m_a[i]).formatstr("%08X");
	state_add(MN10300_MDRQ, "MDRQ", m_mdrq).formatstr("%08X");
	state_add(MN10300_MCRH, "MCRH", m_mcrh).formatstr("%08X");
	state_add(MN10300_MCRL, "MCRL", m_mcrl).formatstr("%08X");
	state_add(MN10300_LIR, "LIR", m_lir).formatstr("%08X");
	state_add(MN10300_LAR, "LAR", m_lar).formatstr("%08X");

	state_add(STATE_GENPC, "GENPC", m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_psw).formatstr("%4s").noshow();

	set_icountptr(m_icount);
}

void mn10300_device::device_reset()
{
	// SP is undefined after reset; the reset code sets it before its first push or call.
	m_pc = m_reset_pc;
	m_sp = 0;
	m_psw = 0;
	m_mdr = 0;
	m_mdrq = 0;
	m_mcrh = 0;
	m_mcrl = 0;
	std::fill(std::begin(m_d), std::end(m_d), 0);
	std::fill(std::begin(m_a), std::end(m_a), 0);

	std::fill(std::begin(m_gxicr), std::end(m_gxicr), 0);
	std::fill(std::begin(m_ivar), std::end(m_ivar), 0);
	m_extmd = 0;
	m_iagr = 0;
	m_irq_pending = false;
	m_irq_level = 7;

	for (unsigned n = 0; n < 2; n++)
	{
		m_tm_mode[n] = 0;
		m_tm_base[n] = 0;
		m_tm_timer[n]->adjust(attotime::never);
	}

	for (unsigned ch = 0; ch < NUM_SIO; ch++)
	{
		if (BIT(m_sio_config[ch], 14))
			m_sio_rx_enable_cb[ch](0);
		m_sio_config[ch] = 0;
		m_sio_control[ch] = 0;
		m_sio_rx_full[ch] = false;
		m_sio_tx_timer[ch]->adjust(attotime::never);
	}
}

void mn10300_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	if (entry.index() == STATE_GENFLAGS)
	{
		str = string_format("%c%c%c%c",
				(m_psw & FLAG_VF) ? 'V' : '-',
				(m_psw & FLAG_CF) ? 'C' : '-',
				(m_psw & FLAG_NF) ? 'N' : '-',
				(m_psw & FLAG_ZF) ? 'Z' : '-');
	}
}


//**************************************************************************
//  Interrupt controller
//**************************************************************************

// Each group has an interrupt control register: DETECT in bits 3-0, REQUEST in
// bit 4, ENABLE in bit 8 and the priority level in bits 14-12. Writing a 1 to a
// DETECT bit acknowledges it.

void mn10300_device::execute_set_input(int inputnum, int state)
{
	if (inputnum < IRQ0 || inputnum > IRQ7)
		return;

	const bool prev = m_irq_pin[inputnum];
	m_irq_pin[inputnum] = state != CLEAR_LINE;
	const unsigned mode = BIT(m_extmd, inputnum * 2, 2);
	if (!BIT(mode, 1))
	{
		// edge triggered: 00 = rising, 01 = falling
		if (prev != m_irq_pin[inputnum] && m_irq_pin[inputnum] == !BIT(mode, 0))
			intc_assert(IRQ0_GROUP + inputnum);
	}
	else
	{
		irq_pin_update(inputnum);
		intc_recompute();
	}
}

// A level-triggered pin (EXTMD 10 = low, 11 = high) holds its group's DETECT bit
// while it is at the active level, so an acknowledge does not clear it and the
// request drops only when the pin does.
void mn10300_device::irq_pin_update(int pin)
{
	const unsigned mode = BIT(m_extmd, pin * 2, 2);
	if (!BIT(mode, 1))
		return;

	u16 &icr = m_gxicr[IRQ0_GROUP + pin];
	if (m_irq_pin[pin] == bool(BIT(mode, 0)))
		icr |= 0x0001;
	else
		icr &= ~0x0001;
	icr = (icr & ~0x0010) | ((icr & 0x000f) ? 0x0010 : 0x0000);
}

void mn10300_device::intc_assert(int group)
{
	m_gxicr[group & (NUM_INTC_GROUPS - 1)] |= 0x0011;
	intc_recompute();
}

// The winner among enabled, requesting groups is the one with the lowest level.
int mn10300_device::intc_pending_group() const
{
	int best = 0;
	int best_level = 8;
	for (int g = 2; g < int(NUM_INTC_GROUPS); g++)
	{
		if ((m_gxicr[g] & 0x0110) == 0x0110)
		{
			const int level = BIT(m_gxicr[g], 12, 3);
			if (level < best_level)
			{
				best_level = level;
				best = g;
			}
		}
	}
	return best;
}

void mn10300_device::intc_recompute()
{
	const int g = intc_pending_group();
	if (g)
		m_irq_level = BIT(m_gxicr[g], 12, 3);
	m_irq_pending = g != 0;
}

// The group is latched when the CPU accepts the interrupt.
void mn10300_device::intc_accept()
{
	const int g = intc_pending_group();
	if (g)
	{
		m_iagr = g;
		m_irq_level = BIT(m_gxicr[g], 12, 3);
	}
}

u32 mn10300_device::level_vector(int level) const
{
	return (level < 7) ? (m_vector_base + m_ivar[level]) : 0;
}

void mn10300_device::check_irq()
{
	if (!(m_psw & FLAG_IE))
		return;

	const int im = BIT(m_psw, IM_SHIFT, 3);
	if (m_irq_pending && m_irq_level < im)
		take_irq();
}

void mn10300_device::take_irq()
{
	// Take the level first: releasing a held pin recomputes m_irq_level
	intc_accept();
	const int level = m_irq_level;
	const u32 vector = level_vector(level);
	if (m_iagr >= IRQ0_GROUP && m_iagr <= IRQ0_GROUP + IRQ7)
		standard_irq_callback(m_iagr - IRQ0_GROUP, m_pc);
	push32(m_pc);
	push32(m_psw);
	m_psw = ((m_psw & ~FLAG_IM) | (level << IM_SHIFT)) & ~FLAG_IE;
	m_pc = vector;
	m_icount -= 7;
}

// IAGR (0x34000100) and 0x34000200 both read back the accepted group, scaled by
// 8 and by 4.
u16 mn10300_device::iagr_r()
{
	return m_iagr << 3;
}

u16 mn10300_device::group_level_r()
{
	return m_iagr << 2;
}

// Each GxICR occupies a 32-bit slot, and both halves reach it.
u16 mn10300_device::gxicr_r(offs_t offset)
{
	return m_gxicr[2 + (offset >> 1)];
}

void mn10300_device::gxicr_w(offs_t offset, u16 data, u16 mem_mask)
{
	const int group = 2 + (offset >> 1);
	const u16 cur = m_gxicr[group];
	const u16 upper = (cur & ~mem_mask) | (data & mem_mask);
	const u16 detect = (cur & 0x000f) & ~(data & mem_mask & 0x000f);
	m_gxicr[group] = (upper & 0xff00) | detect | (detect ? 0x0010 : 0x0000);
	if (group >= IRQ0_GROUP && group < IRQ0_GROUP + 8)
		irq_pin_update(group - IRQ0_GROUP);
	intc_recompute();
}

u16 mn10300_device::extmd_r()
{
	return m_extmd;
}

void mn10300_device::extmd_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 prev = m_extmd;
	COMBINE_DATA(&m_extmd);
	for (int pin = 0; pin < 8; pin++)
	{
		if (BIT(prev ^ m_extmd, pin * 2, 2))
			irq_pin_update(pin);
	}
	intc_recompute();
}

// IVAR0-IVAR6 hold the low 16 bits of each level's vector, one per 32-bit slot.
u16 mn10300_device::ivar_r(offs_t offset)
{
	return BIT(offset, 0) ? 0 : m_ivar[offset >> 1];
}

void mn10300_device::ivar_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (!BIT(offset, 0))
		COMBINE_DATA(&m_ivar[offset >> 1]);
}


//**************************************************************************
//  Timers
//**************************************************************************

// TM4 and TM5 are 16-bit down-counters. Bit 7 of the mode byte enables counting
// and bit 6 is a load pulse; an underflow reloads from the base register and
// raises the timer's group, 6 for TM4 and 7 for TM5.

template <unsigned N>
u8 mn10300_device::tm_mode_r()
{
	return m_tm_mode[N];
}

template <unsigned N>
void mn10300_device::tm_mode_w(u8 data)
{
	const u8 rising = data & ~m_tm_mode[N];
	m_tm_mode[N] = data;
	if (!BIT(data, 7))
		m_tm_timer[N]->adjust(attotime::never);
	else if (rising & 0xc0)
		tm_rearm(N, true);
}

template <unsigned N>
u16 mn10300_device::tm_base_r()
{
	return m_tm_base[N];
}

template <unsigned N>
void mn10300_device::tm_base_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tm_base[N]);
	// a new reload value takes effect at the next underflow
	if (BIT(m_tm_mode[N], 7) && !m_tm_timer[N]->remaining().is_never())
		tm_rearm(N, false);
}

template <unsigned N>
u16 mn10300_device::tm_count_r()
{
	if (!BIT(m_tm_mode[N], 7) || m_tm_timer[N]->remaining().is_never())
		return 0;
	const u64 ticks = m_tm_timer[N]->remaining().as_ticks(clock() / 2);
	return u16(ticks ? ((ticks - 1) / TM_PRESCALE) : 0);
}

void mn10300_device::tm_rearm(unsigned n, bool restart_phase)
{
	const attotime period = attotime::from_ticks(u64(m_tm_base[n] + 1) * TM_PRESCALE, clock() / 2);
	if (restart_phase)
		m_tm_timer[n]->adjust(period, n, period);
	else
		m_tm_timer[n]->adjust(m_tm_timer[n]->remaining(), n, period);
}

TIMER_CALLBACK_MEMBER(mn10300_device::tm_underflow)
{
	intc_assert(TM4_GROUP + param);
}


//**************************************************************************
//  Serial channels
//**************************************************************************

template <unsigned Ch>
u16 mn10300_device::sio_config_r()
{
	return m_sio_config[Ch];
}

// Bit 15 is a strobe that reads back as 0; bit 14 enables the receiver.
template <unsigned Ch>
void mn10300_device::sio_config_w(offs_t offset, u16 data, u16 mem_mask)
{
	const u16 prev = m_sio_config[Ch];
	COMBINE_DATA(&m_sio_config[Ch]);
	m_sio_config[Ch] &= 0x7fff;
	if (BIT(prev ^ m_sio_config[Ch], 14))
		m_sio_rx_enable_cb[Ch](BIT(m_sio_config[Ch], 14));
}

template <unsigned Ch>
u8 mn10300_device::sio_control_r()
{
	return m_sio_control[Ch];
}

template <unsigned Ch>
void mn10300_device::sio_control_w(u8 data)
{
	m_sio_control[Ch] = data;
}

template <unsigned Ch>
void mn10300_device::sio_txd_w(u8 data)
{
	sio_tx_byte(Ch, data);
}

// Receive is single-buffered: a byte arriving before the last one was read
// replaces it.
template <unsigned Ch>
u8 mn10300_device::sio_rxd_r()
{
	if (!machine().side_effects_disabled())
		m_sio_rx_full[Ch] = false;
	return m_sio_rxbuf[Ch];
}

// Bit 4: a received byte is waiting
template <unsigned Ch>
u16 mn10300_device::sio_status_r()
{
	return m_sio_rx_full[Ch] ? 0x0010 : 0x0000;
}

// Transmit is single-buffered: a write goes straight to the shifter and restarts
// it, and the TX interrupt comes when the last byte written has been shifted out.
void mn10300_device::sio_tx_byte(int ch, u8 data)
{
	const u32 rate = m_sio_bit_rate[ch] ? m_sio_bit_rate[ch] : clock() / 2;
	m_sio_tx_cb[ch](data);
	m_sio_tx_timer[ch]->adjust(attotime::from_ticks(m_sio_async[ch] ? 10 : 8, rate), ch);
}

TIMER_CALLBACK_MEMBER(mn10300_device::sio_tx_shifted)
{
	intc_assert(SIO0_GROUP + param * 2 + 1);
}

template <unsigned Ch>
void mn10300_device::sio_rx_w(u8 data)
{
	if (!BIT(m_sio_config[Ch], 14))
		return;
	m_sio_rxbuf[Ch] = data;
	m_sio_rx_full[Ch] = true;
	intc_assert(SIO0_GROUP + Ch * 2);
}

template void mn10300_device::sio_rx_w<0>(u8 data);
template void mn10300_device::sio_rx_w<1>(u8 data);
template void mn10300_device::sio_rx_w<2>(u8 data);


//**************************************************************************
//  Execution helpers
//**************************************************************************

void mn10300_device::set_nz32(u32 r)
{
	m_psw &= ~(FLAG_ZF | FLAG_NF);
	if (!r)
		m_psw |= FLAG_ZF;
	if (BIT(r, 31))
		m_psw |= FLAG_NF;
}

// Logical operations and bit tests: Z and N from the result, C and V cleared
void mn10300_device::set_logic_flags(u32 r)
{
	m_psw &= ~(FLAG_ZF | FLAG_NF | FLAG_CF | FLAG_VF);
	if (!r)
		m_psw |= FLAG_ZF;
	if (BIT(r, 31))
		m_psw |= FLAG_NF;
}

u32 mn10300_device::do_add(u32 a, u32 b, u32 carry_in)
{
	const u64 wide = u64(a) + u64(b) + carry_in;
	const u32 r = u32(wide);
	m_psw &= ~(FLAG_ZF | FLAG_NF | FLAG_CF | FLAG_VF);
	if (!r)
		m_psw |= FLAG_ZF;
	if (BIT(r, 31))
		m_psw |= FLAG_NF;
	if (BIT(wide, 32))
		m_psw |= FLAG_CF;
	if (BIT(~(a ^ b) & (a ^ r), 31))
		m_psw |= FLAG_VF;
	return r;
}

// C is the borrow
u32 mn10300_device::do_sub(u32 a, u32 b, u32 borrow_in)
{
	const u64 wide = u64(a) - u64(b) - borrow_in;
	const u32 r = u32(wide);
	m_psw &= ~(FLAG_ZF | FLAG_NF | FLAG_CF | FLAG_VF);
	if (!r)
		m_psw |= FLAG_ZF;
	if (BIT(r, 31))
		m_psw |= FLAG_NF;
	if (BIT(wide, 32))
		m_psw |= FLAG_CF;
	if (BIT((a ^ b) & (a ^ r), 31))
		m_psw |= FLAG_VF;
	return r;
}

// Conditions in the order of the Bcc and Lcc opcodes
bool mn10300_device::test_cond(int cc)
{
	const bool z = m_psw & FLAG_ZF;
	const bool n = m_psw & FLAG_NF;
	const bool c = m_psw & FLAG_CF;
	const bool v = m_psw & FLAG_VF;
	switch (cc)
	{
	case 0: return n != v;              // lt
	case 1: return !((n != v) || z);    // gt
	case 2: return n == v;              // ge
	case 3: return (n != v) || z;       // le
	case 4: return c;                   // cs
	case 5: return !(c || z);           // hi
	case 6: return !c;                  // cc
	case 7: return c || z;              // ls
	case 8: return z;                   // eq
	case 9: return !z;                  // ne
	}
	return false;
}

void mn10300_device::push32(u32 val)
{
	m_sp -= 4;
	write_mem32(m_sp, val);
}

u32 mn10300_device::pop32()
{
	const u32 val = read_mem32(m_sp);
	m_sp += 4;
	return val;
}

// type: 0 and 1 = 32-bit (an address register if a_reg), 2 = byte, 3 = halfword
void mn10300_device::typed_load_store(int type, bool a_reg, int reg, u32 ea, bool store)
{
	switch (type)
	{
	case 0:
	case 1:
		if (a_reg)
		{
			if (store)
				write_mem32(ea, m_a[reg]);
			else
				m_a[reg] = read_mem32(ea);
		}
		else
		{
			if (store)
				write_mem32(ea, m_d[reg]);
			else
				m_d[reg] = read_mem32(ea);
		}
		break;
	case 2:
		if (store)
			write_mem8(ea, m_d[reg]);
		else
			m_d[reg] = read_mem8(ea);
		break;
	case 3:
		if (store)
			write_mem16(ea, m_d[reg]);
		else
			m_d[reg] = read_mem16(ea);
		break;
	}
}

// op: 0 = asl, 1 = lsr, 2 = asr
void mn10300_device::do_shift(int op, int dst, u32 count)
{
	// C is the last bit shifted out
	count &= 0x1f;
	bool carry = false;
	if (count)
	{
		if (op == 0)
		{
			carry = BIT(m_d[dst], 32 - count);
			m_d[dst] <<= count;
		}
		else if (op == 1)
		{
			carry = BIT(m_d[dst], count - 1);
			m_d[dst] >>= count;
		}
		else
		{
			carry = BIT(m_d[dst], count - 1);
			m_d[dst] = s32(m_d[dst]) >> count;
		}
	}
	m_psw = (m_psw & ~FLAG_CF) | (carry ? FLAG_CF : 0);
	set_nz32(m_d[dst]);
}

// movm and call/ret register lists. Bit 3 is the "other" group; bits 0-2 are
// AM33 register groups, which the MN1030 does not have.
void mn10300_device::store_regs(u8 mask)
{
	if (BIT(mask, 7))
		push32(m_d[2]);
	if (BIT(mask, 6))
		push32(m_d[3]);
	if (BIT(mask, 5))
		push32(m_a[2]);
	if (BIT(mask, 4))
		push32(m_a[3]);
	if (BIT(mask, 3))
	{
		push32(m_d[0]);
		push32(m_d[1]);
		push32(m_a[0]);
		push32(m_a[1]);
		push32(m_mdr);
		push32(m_lir);
		push32(m_lar);
		m_sp -= 4;
	}
}

void mn10300_device::load_regs(u8 mask)
{
	if (BIT(mask, 3))
	{
		m_sp += 4;
		m_lar = pop32();
		m_lir = pop32();
		m_mdr = pop32();
		m_a[1] = pop32();
		m_a[0] = pop32();
		m_d[1] = pop32();
		m_d[0] = pop32();
	}
	if (BIT(mask, 4))
		m_a[3] = pop32();
	if (BIT(mask, 5))
		m_a[2] = pop32();
	if (BIT(mask, 6))
		m_d[3] = pop32();
	if (BIT(mask, 7))
		m_d[2] = pop32();
}

void mn10300_device::store_regs_at(u32 base, u8 mask)
{
	u32 ea = base;
	auto const put = [this, &ea] (u32 val) { ea -= 4; write_mem32(ea, val); };
	if (BIT(mask, 7))
		put(m_d[2]);
	if (BIT(mask, 6))
		put(m_d[3]);
	if (BIT(mask, 5))
		put(m_a[2]);
	if (BIT(mask, 4))
		put(m_a[3]);
	if (BIT(mask, 3))
	{
		put(m_d[0]);
		put(m_d[1]);
		put(m_a[0]);
		put(m_a[1]);
		put(m_mdr);
		put(m_lir);
		put(m_lar);
	}
}

void mn10300_device::load_regs_at(u32 base, u8 mask)
{
	u32 ea = base;
	auto const get = [this, &ea] () { ea -= 4; return read_mem32(ea); };
	if (BIT(mask, 7))
		m_d[2] = get();
	if (BIT(mask, 6))
		m_d[3] = get();
	if (BIT(mask, 5))
		m_a[2] = get();
	if (BIT(mask, 4))
		m_a[3] = get();
	if (BIT(mask, 3))
	{
		m_d[0] = get();
		m_d[1] = get();
		m_a[0] = get();
		m_a[1] = get();
		m_mdr = get();
		m_lir = get();
		m_lar = get();
	}
}


//**************************************************************************
//  Execution
//**************************************************************************

void mn10300_device::execute_run()
{
	do
	{
		check_irq();

		debugger_instruction_hook(m_pc);

		const u32 start_pc = m_pc;
		const u8 op = read_arg8(m_pc);
		m_pc += 1;

		const int dst = op & 3;
		const int src = BIT(op, 2, 2);

		switch (op)
		{
		// clr Dn (n = bits 3-2)
		case 0x00: case 0x04: case 0x08: case 0x0c:
			m_d[src] = 0;
			m_psw = (m_psw & ~(FLAG_NF | FLAG_CF | FLAG_VF)) | FLAG_ZF;
			break;

		// mov/movbu/movhu Dm,(abs16)
		case 0x01: case 0x05: case 0x09: case 0x0d:
		case 0x02: case 0x06: case 0x0a: case 0x0e:
		case 0x03: case 0x07: case 0x0b: case 0x0f:
			typed_load_store(op & 3, false, src, read_arg16(m_pc), true);
			m_pc += 2;
			break;

		// extb/extbu/exth/exthu Dn
		case 0x10: case 0x11: case 0x12: case 0x13:
			m_d[dst] = util::sext(m_d[dst], 8);
			break;
		case 0x14: case 0x15: case 0x16: case 0x17:
			m_d[dst] &= 0x0000'00ff;
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b:
			m_d[dst] = util::sext(m_d[dst], 16);
			break;
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			m_d[dst] &= 0x0000'ffff;
			break;

		// add imm8,An
		case 0x20: case 0x21: case 0x22: case 0x23:
			m_a[dst] = do_add(m_a[dst], util::sext(read_arg8(m_pc), 8), 0);
			m_pc += 1;
			break;

		// mov imm16,An (zero-extended)
		case 0x24: case 0x25: case 0x26: case 0x27:
			m_a[dst] = read_arg16(m_pc);
			m_pc += 2;
			break;

		// add imm8,Dn
		case 0x28: case 0x29: case 0x2a: case 0x2b:
			m_d[dst] = do_add(m_d[dst], util::sext(read_arg8(m_pc), 8), 0);
			m_pc += 1;
			break;

		// mov imm16,Dn (sign-extended)
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			m_d[dst] = util::sext(read_arg16(m_pc), 16);
			m_pc += 2;
			break;

		// mov/movbu/movhu (abs16),Dn
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
		case 0x38: case 0x39: case 0x3a: case 0x3b:
			typed_load_store(src ? (src + 1) : 0, false, dst, read_arg16(m_pc), false);
			m_pc += 2;
			break;

		// mov sp,An
		case 0x3c: case 0x3d: case 0x3e: case 0x3f:
			m_a[dst] = m_sp;
			break;

		// inc Dn / inc An (n = bits 3-2; inc An leaves the flags alone)
		case 0x40: case 0x44: case 0x48: case 0x4c:
			m_d[src] = do_add(m_d[src], 1, 0);
			break;
		case 0x41: case 0x45: case 0x49: case 0x4d:
			m_a[src] += 1;
			break;

		// mov Dm,(d8,sp) / mov Am,(d8,sp) (d8 zero-extended)
		case 0x42: case 0x46: case 0x4a: case 0x4e:
			write_mem32(m_sp + read_arg8(m_pc), m_d[src]);
			m_pc += 1;
			break;
		case 0x43: case 0x47: case 0x4b: case 0x4f:
			write_mem32(m_sp + read_arg8(m_pc), m_a[src]);
			m_pc += 1;
			break;

		// inc4 An / asl2 Dn
		case 0x50: case 0x51: case 0x52: case 0x53:
			m_a[dst] += 4;
			break;
		case 0x54: case 0x55: case 0x56: case 0x57:
			m_d[dst] <<= 2;
			set_nz32(m_d[dst]);
			break;

		// mov (d8,sp),Dn / mov (d8,sp),An (d8 zero-extended)
		case 0x58: case 0x59: case 0x5a: case 0x5b:
			m_d[dst] = read_mem32(m_sp + read_arg8(m_pc));
			m_pc += 1;
			break;
		case 0x5c: case 0x5d: case 0x5e: case 0x5f:
			m_a[dst] = read_mem32(m_sp + read_arg8(m_pc));
			m_pc += 1;
			break;

		// mov Dm,(An) / mov (Am),Dn
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
			write_mem32(m_a[dst], m_d[src]);
			break;
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
		case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
			m_d[src] = read_mem32(m_a[dst]);
			break;

		// mov imm8,Dn (sign-extended) / mov Dm,Dn
		case 0x80: case 0x85: case 0x8a: case 0x8f:
			m_d[dst] = util::sext(read_arg8(m_pc), 8);
			m_pc += 1;
			break;
		case 0x81: case 0x82: case 0x83: case 0x84: case 0x86: case 0x87:
		case 0x88: case 0x89: case 0x8b: case 0x8c: case 0x8d: case 0x8e:
			m_d[dst] = m_d[src];
			break;

		// mov imm8,An (zero-extended) / mov Am,An
		case 0x90: case 0x95: case 0x9a: case 0x9f:
			m_a[dst] = read_arg8(m_pc);
			m_pc += 1;
			break;
		case 0x91: case 0x92: case 0x93: case 0x94: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9b: case 0x9c: case 0x9d: case 0x9e:
			m_a[dst] = m_a[src];
			break;

		// cmp imm8,Dn (sign-extended) / cmp Dm,Dn
		case 0xa0: case 0xa5: case 0xaa: case 0xaf:
			do_sub(m_d[dst], util::sext(read_arg8(m_pc), 8), 0);
			m_pc += 1;
			break;
		case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa6: case 0xa7:
		case 0xa8: case 0xa9: case 0xab: case 0xac: case 0xad: case 0xae:
			do_sub(m_d[dst], m_d[src], 0);
			break;

		// cmp imm8,An (zero-extended) / cmp Am,An
		case 0xb0: case 0xb5: case 0xba: case 0xbf:
			do_sub(m_a[dst], read_arg8(m_pc), 0);
			m_pc += 1;
			break;
		case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb6: case 0xb7:
		case 0xb8: case 0xb9: case 0xbb: case 0xbc: case 0xbd: case 0xbe:
			do_sub(m_a[dst], m_a[src], 0);
			break;

		// Bcc d8
		case 0xc0: case 0xc1: case 0xc2: case 0xc3: case 0xc4:
		case 0xc5: case 0xc6: case 0xc7: case 0xc8: case 0xc9:
		{
			const u32 target = start_pc + util::sext(read_arg8(m_pc), 8);
			m_pc += 1;
			if (test_cond(op & 0x0f))
				m_pc = target;
			break;
		}

		// bra d8
		case 0xca:
			m_pc = start_pc + util::sext(read_arg8(m_pc), 8);
			break;

		// nop
		case 0xcb:
			break;

		// jmp d16
		case 0xcc:
			m_pc = start_pc + util::sext(read_arg16(m_pc), 16);
			break;

		// call d16,regs,imm8: the return address goes at (sp) and in MDR, the
		// registers below it, and SP drops by imm8
		case 0xcd:
		{
			const u32 target = start_pc + util::sext(read_arg16(m_pc), 16);
			const u8 regs = read_arg8(m_pc + 2);
			const u8 adj = read_arg8(m_pc + 3);
			const u32 ret = m_pc + 4;
			write_mem32(m_sp, ret);
			store_regs_at(m_sp, regs);
			m_sp -= adj;
			m_mdr = ret;
			m_pc = target;
			break;
		}

		// movm (sp),regs / movm regs,(sp)
		case 0xce:
			load_regs(read_arg8(m_pc));
			m_pc += 1;
			break;
		case 0xcf:
			store_regs(read_arg8(m_pc));
			m_pc += 1;
			break;

		// Lcc and lra branch to the loop start, LAR - 4
		case 0xd0: case 0xd1: case 0xd2: case 0xd3: case 0xd4:
		case 0xd5: case 0xd6: case 0xd7: case 0xd8: case 0xd9:
			if (test_cond(op & 0x0f))
				m_pc = m_lar - 4;
			break;
		case 0xda:
			m_pc = m_lar - 4;
			break;

		// setlb: LIR takes the loop's first four bytes, LAR points past them
		case 0xdb:
			m_lir = read_arg32(m_pc);
			m_lar = start_pc + 5;
			break;

		// jmp d32
		case 0xdc:
			m_pc = start_pc + read_arg32(m_pc);
			break;

		// call d32,regs,imm8
		case 0xdd:
		{
			const u32 target = start_pc + read_arg32(m_pc);
			const u8 regs = read_arg8(m_pc + 4);
			const u8 adj = read_arg8(m_pc + 5);
			const u32 ret = m_pc + 6;
			write_mem32(m_sp, ret);
			store_regs_at(m_sp, regs);
			m_sp -= adj;
			m_mdr = ret;
			m_pc = target;
			break;
		}

		// retf regs,imm8: return through MDR
		case 0xde:
		{
			const u8 regs = read_arg8(m_pc);
			const u8 adj = read_arg8(m_pc + 1);
			m_sp += adj;
			m_pc = m_mdr;
			load_regs_at(m_sp, regs);
			break;
		}

		// ret regs,imm8: return through (sp)
		case 0xdf:
		{
			const u8 regs = read_arg8(m_pc);
			const u8 adj = read_arg8(m_pc + 1);
			m_sp += adj;
			load_regs_at(m_sp, regs);
			m_pc = read_mem32(m_sp);
			break;
		}

		// add Dm,Dn
		case 0xe0: case 0xe1: case 0xe2: case 0xe3: case 0xe4: case 0xe5: case 0xe6: case 0xe7:
		case 0xe8: case 0xe9: case 0xea: case 0xeb: case 0xec: case 0xed: case 0xee: case 0xef:
			m_d[dst] = do_add(m_d[dst], m_d[src], 0);
			break;

		case 0xf0: execute_f0(); break;
		case 0xf1: execute_f1(); break;
		case 0xf2: execute_f2(); break;
		case 0xf3: execute_f3(); break;
		case 0xf4: execute_f4(); break;
		case 0xf5: execute_f5(); break;
		case 0xf6: execute_f6(); break;
		case 0xf8: execute_f8(); break;
		case 0xfa: execute_fa(); break;
		case 0xfc: execute_fc(); break;
		case 0xfe: execute_fe(); break;

		default:
		{
			const u8 op2 = read_arg8(start_pc + 1);
			m_pc = start_pc + insn_length(op);
			if ((op == 0xf9 || op == 0xfb || op == 0xfd) && op2 <= 0x03)
			{
				// mulq imm,Dn: signed multiply by an immediate, high word to MDRQ
				const s32 imm = (op == 0xf9) ? util::sext(read_arg8(start_pc + 2), 8)
						: (op == 0xfb) ? util::sext(read_arg16(start_pc + 2), 16)
						: s32(read_arg32(start_pc + 2));
				const s64 t = s64(s32(m_d[op2 & 3])) * imm;
				m_d[op2 & 3] = u32(t);
				m_mdrq = u32(u64(t) >> 32);
				set_logic_flags(m_d[op2 & 3]);
			}
			else if ((op == 0xf9 || op == 0xfb || op == 0xfd) && (op2 & 0xfc) == 0x14)
			{
				// mulqu imm,Dn: unsigned multiply by a sign-extended immediate, high word to MDRQ
				const u32 imm = (op == 0xf9) ? u32(util::sext(read_arg8(start_pc + 2), 8))
						: (op == 0xfb) ? u32(util::sext(read_arg16(start_pc + 2), 16))
						: read_arg32(start_pc + 2);
				const u64 t = u64(m_d[op2 & 3]) * imm;
				m_d[op2 & 3] = u32(t);
				m_mdrq = u32(t >> 32);
				set_logic_flags(m_d[op2 & 3]);
			}
			else
			{
				logerror("unimplemented opcode %02X op2=%02X @ %08X (skipped %d bytes)\n", op, op2, start_pc, m_pc - start_pc);
			}
			break;
		}
		}

		// TODO: per-instruction cycle counts
		m_icount -= 1;
	} while (m_icount > 0);
}

// 0xf0: register-indirect moves, bit operations on (An), and indirect calls and
// returns. Always 2 bytes.
void mn10300_device::execute_f0()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	const int r = BIT(op2, 2, 2);
	const int am = op2 & 3;

	switch (op2 >> 4)
	{
	case 0x0: // mov (Am),An
		m_a[r] = read_mem32(m_a[am]);
		break;
	case 0x1: // mov Am,(An)
		write_mem32(m_a[am], m_a[r]);
		break;
	case 0x4: // movbu (Am),Dn
		m_d[r] = read_mem8(m_a[am]);
		break;
	case 0x5: // movbu Dm,(An)
		write_mem8(m_a[am], m_d[r]);
		break;
	case 0x6: // movhu (Am),Dn
		m_d[r] = read_mem16(m_a[am]);
		break;
	case 0x7: // movhu Dm,(An)
		write_mem16(m_a[am], m_d[r]);
		break;
	case 0x8: // bset Dm,(An)
	{
		const u8 v = read_mem8(m_a[am]);
		set_logic_flags(v & m_d[r]);
		write_mem8(m_a[am], v | m_d[r]);
		break;
	}
	case 0x9: // bclr Dm,(An)
	{
		const u8 v = read_mem8(m_a[am]);
		set_logic_flags(v & m_d[r]);
		write_mem8(m_a[am], v & ~m_d[r]);
		break;
	}
	case 0xf:
		if (op2 <= 0xf3)
		{
			// calls (An): the return address goes at (sp) and in MDR, SP unchanged
			write_mem32(m_sp, start_pc + 2);
			m_mdr = start_pc + 2;
			m_pc = m_a[am];
			return;
		}
		else if (op2 <= 0xf7)
		{
			// jmp (An)
			m_pc = m_a[am];
			return;
		}
		else if (op2 == 0xfc)
		{
			// rets
			m_pc = read_mem32(m_sp);
			return;
		}
		else if (op2 == 0xfd)
		{
			// rti
			m_psw = pop32();
			m_pc = pop32();
			return;
		}
		logerror("unimplemented F0 %02X @ %08X\n", op2, start_pc);
		break;
	default:
		logerror("unimplemented F0 %02X @ %08X\n", op2, start_pc);
		break;
	}
	m_pc = start_pc + 2;
}

// 0xf1: arithmetic between data and address registers. Always 2 bytes.
void mn10300_device::execute_f1()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	const int dst = op2 & 3;
	const int src = BIT(op2, 2, 2);
	const u32 c = (m_psw & FLAG_CF) ? 1 : 0;

	switch (op2 >> 4)
	{
	case 0x0: m_d[dst] = do_sub(m_d[dst], m_d[src], 0); break;  // sub Dm,Dn
	case 0x1: m_d[dst] = do_sub(m_d[dst], m_a[src], 0); break;  // sub Am,Dn
	case 0x2: m_a[dst] = do_sub(m_a[dst], m_d[src], 0); break;  // sub Dm,An
	case 0x3: m_a[dst] = do_sub(m_a[dst], m_a[src], 0); break;  // sub Am,An
	case 0x4: // addc Dm,Dn: Z stays set only if it was set, for multi-word results
	{
		const u16 z = m_psw & FLAG_ZF;
		m_d[dst] = do_add(m_d[dst], m_d[src], c);
		m_psw &= ~FLAG_ZF | z;
		break;
	}
	case 0x5: m_d[dst] = do_add(m_d[dst], m_a[src], 0); break;  // add Am,Dn
	case 0x6: m_a[dst] = do_add(m_a[dst], m_d[src], 0); break;  // add Dm,An
	case 0x7: m_a[dst] = do_add(m_a[dst], m_a[src], 0); break;  // add Am,An
	case 0x8: // subc Dm,Dn: as addc
	{
		const u16 z = m_psw & FLAG_ZF;
		m_d[dst] = do_sub(m_d[dst], m_d[src], c);
		m_psw &= ~FLAG_ZF | z;
		break;
	}
	case 0x9: do_sub(m_d[dst], m_a[src], 0); break;             // cmp Am,Dn
	case 0xa: do_sub(m_a[dst], m_d[src], 0); break;             // cmp Dm,An
	case 0xd: m_d[dst] = m_a[src]; break;                       // mov Am,Dn
	case 0xe: m_a[dst] = m_d[src]; break;                       // mov Dm,An
	default:
		logerror("illegal F1 %02X @ %08X\n", op2, start_pc);
		break;
	}
	m_pc = start_pc + 2;
}

// 0xf2: logical operations, multiply and divide, shifts and special-register
// moves. Always 2 bytes.
void mn10300_device::execute_f2()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	const int dst = op2 & 3;
	const int src = BIT(op2, 2, 2);

	switch (op2 >> 4)
	{
	case 0x0: // and Dm,Dn
		m_d[dst] &= m_d[src];
		set_logic_flags(m_d[dst]);
		break;
	case 0x1: // or Dm,Dn
		m_d[dst] |= m_d[src];
		set_logic_flags(m_d[dst]);
		break;
	case 0x2: // xor Dm,Dn
		m_d[dst] ^= m_d[src];
		set_logic_flags(m_d[dst]);
		break;
	case 0x3: // not Dn
		if (op2 < 0x34)
		{
			m_d[dst] = ~m_d[dst];
			set_logic_flags(m_d[dst]);
		}
		else
		{
			logerror("illegal F2 %02X @ %08X\n", op2, start_pc);
		}
		break;
	case 0x4: // mul Dm,Dn
	{
		const s64 p = s64(s32(m_d[src])) * s32(m_d[dst]);
		m_d[dst] = u32(p);
		m_mdr = u32(u64(p) >> 32);
		set_logic_flags(m_d[dst]);
		break;
	}
	case 0x5: // mulu Dm,Dn
	{
		const u64 p = u64(m_d[src]) * m_d[dst];
		m_d[dst] = u32(p);
		m_mdr = u32(p >> 32);
		set_logic_flags(m_d[dst]);
		break;
	}
	case 0x6: // div Dm,Dn: (MDR:Dn) / Dm
	{
		const s64 num = s64((u64(m_mdr) << 32) | m_d[dst]);
		const s32 dv = s32(m_d[src]);
		// dividing INT64_MIN by -1 traps on the host, so the no-result cases are
		// decided before either operator runs
		const bool no_result = !dv || (num == std::numeric_limits<s64>::min() && dv == -1);
		const s64 q = no_result ? 0 : num / dv;
		if (!no_result && q >= std::numeric_limits<s32>::min() && q <= std::numeric_limits<s32>::max())
		{
			m_d[dst] = u32(q);
			m_mdr = u32(num % dv);
			set_logic_flags(m_d[dst]);
		}
		else
		{
			m_psw |= FLAG_VF;
		}
		break;
	}
	case 0x7: // divu Dm,Dn
	{
		const u64 num = (u64(m_mdr) << 32) | m_d[dst];
		const u32 dv = m_d[src];
		if (dv && (num / dv) <= 0xffff'ffffU)
		{
			m_d[dst] = u32(num / dv);
			m_mdr = u32(num % dv);
			set_logic_flags(m_d[dst]);
		}
		else
		{
			m_psw |= FLAG_VF;
		}
		break;
	}
	case 0x8: // rol Dn / ror Dn, through carry
	{
		if (op2 >= 0x88)
		{
			logerror("illegal F2 %02X @ %08X\n", op2, start_pc);
			break;
		}
		const u32 c = (m_psw & FLAG_CF) ? 1 : 0;
		const bool left = op2 < 0x84;
		const bool out = left ? BIT(m_d[dst], 31) : BIT(m_d[dst], 0);
		m_d[dst] = left ? ((m_d[dst] << 1) | c) : ((m_d[dst] >> 1) | (c << 31));
		set_logic_flags(m_d[dst]);
		if (out)
			m_psw |= FLAG_CF;
		break;
	}
	case 0x9: // asl Dm,Dn
	case 0xa: // lsr Dm,Dn
	case 0xb: // asr Dm,Dn
		do_shift((op2 >> 4) - 0x9, dst, m_d[src]);
		break;
	case 0xd: // ext Dn
		if (op2 < 0xd4)
			m_mdr = BIT(m_d[dst], 31) ? 0xffff'ffff : 0;
		else
			logerror("illegal F2 %02X @ %08X\n", op2, start_pc);
		break;
	case 0xe:
		if (op2 < 0xe4)      // mov mdr,Dn
			m_d[dst] = m_mdr;
		else if (op2 < 0xe8) // mov psw,Dn
			m_d[dst] = m_psw;
		else
			logerror("illegal F2 %02X @ %08X\n", op2, start_pc);
		break;
	case 0xf:
		if (BIT(op2, 1))
		{
			if (BIT(op2, 0))  // mov Dm,psw
				m_psw = m_d[src];
			else              // mov Dm,mdr
				m_mdr = m_d[src];
		}
		else if (!BIT(op2, 0)) // mov Am,sp
		{
			m_sp = m_a[src];
		}
		else
		{
			logerror("illegal F2 %02X @ %08X\n", op2, start_pc);
		}
		break;
	default:
		logerror("unimplemented F2 %02X @ %08X\n", op2, start_pc);
		break;
	}
	m_pc = start_pc + 2;
}

// 0xf3: 32-bit indexed load/store, EA = An + Di. Always 2 bytes.
void mn10300_device::execute_f3()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	const u32 ea = m_a[op2 & 3] + m_d[BIT(op2, 2, 2)];
	u32 &reg = BIT(op2, 7) ? m_a[BIT(op2, 4, 2)] : m_d[BIT(op2, 4, 2)];

	if (BIT(op2, 6))
		write_mem32(ea, reg);
	else
		reg = read_mem32(ea);
	m_pc = start_pc + 2;
}

// 0xf4: byte and halfword indexed load/store, EA = An + Di. Always 2 bytes.
void mn10300_device::execute_f4()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	const u32 ea = m_a[op2 & 3] + m_d[BIT(op2, 2, 2)];
	const int reg = BIT(op2, 4, 2);

	typed_load_store(BIT(op2, 7) ? 3 : 2, false, reg, ea, BIT(op2, 6));
	m_pc = start_pc + 2;
}

// 0xf5: moves into the extended multiply registers. Always 2 bytes.
void mn10300_device::execute_f5()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	const int dst = op2 & 3;
	const int src = BIT(op2, 2, 2);

	switch (op2 >> 4)
	{
	case 0x0: // putx Dn
		m_mdrq = m_d[dst];
		break;
	// udf21 Dm,Dn: user-defined. The firmware stores two registers with it and reads
	// them back with udf12/udf13, so it runs as the AM33's putchclx.
	case 0x1:
		m_mcrh = m_d[src];
		m_mcrl = m_d[dst];
		break;
	default:
		logerror("unimplemented F5 %02X @ %08X\n", op2, start_pc);
		break;
	}
	m_pc = start_pc + 2;
}

// 0xf6: multiply, saturate and extended-register moves. Always 2 bytes.
void mn10300_device::execute_f6()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	const int dst = op2 & 3;
	const int src = BIT(op2, 2, 2);

	switch (op2 >> 4)
	{
	case 0x0: // mulq Dm,Dn
	{
		const s64 t = s64(s32(m_d[dst])) * s32(m_d[src]);
		m_d[dst] = u32(t);
		m_mdrq = u32(u64(t) >> 32);
		set_logic_flags(m_d[dst]);
		break;
	}
	case 0x1: // mulqu Dm,Dn
	{
		const u64 t = u64(m_d[dst]) * m_d[src];
		m_d[dst] = u32(t);
		m_mdrq = u32(t >> 32);
		set_logic_flags(m_d[dst]);
		break;
	}
	case 0x4: // sat16 Dm,Dn: Z and N from the 16-bit result, C and V unchanged
		m_d[dst] = u32(std::clamp<s32>(s32(m_d[src]), -0x8000, 0x7fff));
		m_psw = (m_psw & ~(FLAG_ZF | FLAG_NF)) | (m_d[dst] ? 0 : FLAG_ZF) | (BIT(m_d[dst], 15) ? FLAG_NF : 0);
		break;
	case 0x5: // sat24 Dm,Dn: Z and N from the 24-bit result, C and V unchanged
		m_d[dst] = u32(std::clamp<s32>(s32(m_d[src]), -0x800000, 0x7fffff));
		m_psw = (m_psw & ~(FLAG_ZF | FLAG_NF)) | (m_d[dst] ? 0 : FLAG_ZF) | (BIT(m_d[dst], 23) ? FLAG_NF : 0);
		break;
	// bsch Dm,Dn (udf07): the highest set bit of Dm below bit Dn (searching from bit
	// 31 when Dn is 0) to Dn, or 0 if none; C if one was found. As the AM33's bsch;
	// the firmware's JPEG decoder uses it.
	case 0x7:
	{
		const unsigned start = m_d[dst] & 0x1f;
		const u32 v = start ? (m_d[src] & util::make_bitmask<u32>(start)) : m_d[src];
		m_d[dst] = v ? u32(31 - std::countl_zero(v)) : 0;
		m_psw = (m_psw & ~FLAG_CF) | (v ? FLAG_CF : 0);
		break;
	}
	case 0xc: // udf12 Dn (AM33 getchx)
		m_d[dst] = m_mcrh;
		break;
	case 0xd: // udf13 Dn (AM33 getclx)
		m_d[dst] = m_mcrl;
		break;
	case 0xf: // getx Dn
		m_d[dst] = m_mdrq;
		set_logic_flags(m_d[dst]);
		break;
	default:
		logerror("unimplemented F6 %02X @ %08X\n", op2, start_pc);
		break;
	}
	m_pc = start_pc + 2;
}

// 0xf8: 8-bit immediate and displacement forms. Always 3 bytes.
void mn10300_device::execute_f8()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	const u8 b = read_arg8(m_pc + 1);
	const s32 sdisp = util::sext(b, 8);
	const int dst = op2 & 3;

	if (op2 < 0x80)
	{
		// mov/movbu/movhu between a register and (d8,An)
		const int type = BIT(op2, 5, 2);
		typed_load_store(type, type == 1, BIT(op2, 2, 2), m_a[dst] + sdisp, BIT(op2, 4));
	}
	else if ((op2 & 0xf2) == 0x92)
	{
		// movbu/movhu Dm,(d8,sp) (d8 zero-extended)
		typed_load_store(op2 & 3, false, BIT(op2, 2, 2), m_sp + b, true);
	}
	else
	{
		switch (op2 & 0xfc)
		{
		case 0xb8: // movbu (d8,sp),Dn
		case 0xbc: // movhu (d8,sp),Dn
			typed_load_store(BIT(op2, 2, 2), false, dst, m_sp + b, false);
			break;
		case 0xc0: // asl imm8,Dn
		case 0xc4: // lsr imm8,Dn
		case 0xc8: // asr imm8,Dn
			do_shift(BIT(op2, 2, 2), dst, b);
			break;
		case 0xe0: // and imm8,Dn
			m_d[dst] &= b;
			set_logic_flags(m_d[dst]);
			break;
		case 0xe4: // or imm8,Dn
			m_d[dst] |= b;
			set_logic_flags(m_d[dst]);
			break;
		case 0xec: // btst imm8,Dn
			set_logic_flags(m_d[dst] & b);
			break;
		case 0xe8: // bvc, bvs, bnc, bns
		{
			bool take = false;
			switch (dst)
			{
			case 0: take = !(m_psw & FLAG_VF); break;
			case 1: take = m_psw & FLAG_VF; break;
			case 2: take = !(m_psw & FLAG_NF); break;
			case 3: take = m_psw & FLAG_NF; break;
			}
			if (take)
			{
				m_pc = start_pc + sdisp;
				return;
			}
			break;
		}
		case 0xf0: // mov (d8,An),sp
			m_sp = read_mem32(m_a[dst] + sdisp);
			break;
		case 0xf4: // mov sp,(d8,An)
			write_mem32(m_a[dst] + sdisp, m_sp);
			break;
		case 0xfc:
			if (op2 == 0xfe) // add imm8,sp
				m_sp += sdisp;
			else
				logerror("unimplemented F8 %02X @ %08X\n", op2, start_pc);
			break;
		default:
			logerror("unimplemented F8 %02X @ %08X\n", op2, start_pc);
			break;
		}
	}
	m_pc = start_pc + 3;
}

// 0xfa: 16-bit immediate and displacement forms. Always 4 bytes.
void mn10300_device::execute_fa()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	const u16 imm16 = read_arg16(m_pc + 1);
	const int dst = op2 & 3;

	if (op2 < 0x80)
	{
		// mov/movbu/movhu between a register and (d16,An)
		const int type = BIT(op2, 5, 2);
		typed_load_store(type, type == 1, BIT(op2, 2, 2), m_a[dst] + util::sext(imm16, 16), BIT(op2, 4));
	}
	else
	{
		switch (op2 & 0xfc)
		{
		case 0x80: case 0x84: case 0x88: case 0x8c: // mov Am,(abs16)
			if (op2 & 3)
				logerror("illegal FA %02X @ %08X\n", op2, start_pc);
			else
				write_mem32(imm16, m_a[BIT(op2, 2, 2)]);
			break;
		case 0x90: case 0x94: case 0x98: case 0x9c:
		{
			// mov Am / mov Dm / movbu Dm / movhu Dm,(d16,sp) (d16 zero-extended)
			const int type = op2 & 3;
			typed_load_store(type, type == 0, BIT(op2, 2, 2), m_sp + imm16, true);
			break;
		}
		case 0xa0: // mov (abs16),An
			m_a[dst] = read_mem32(imm16);
			break;
		case 0xb0: case 0xb4: case 0xb8: case 0xbc:
		{
			// mov (d16,sp),An / mov / movbu / movhu (d16,sp),Dn (d16 zero-extended)
			const int type = BIT(op2, 2, 2);
			typed_load_store(type, type == 0, dst, m_sp + imm16, false);
			break;
		}
		case 0xc0: // add imm16,Dn (sign-extended)
			m_d[dst] = do_add(m_d[dst], util::sext(imm16, 16), 0);
			break;
		case 0xc8: // cmp imm16,Dn (sign-extended)
			do_sub(m_d[dst], util::sext(imm16, 16), 0);
			break;
		case 0xd0: // add imm16,An (sign-extended)
			m_a[dst] = do_add(m_a[dst], util::sext(imm16, 16), 0);
			break;
		case 0xd8: // cmp imm16,An (zero-extended)
			do_sub(m_a[dst], imm16, 0);
			break;
		case 0xe0: // and imm16,Dn
			m_d[dst] &= imm16;
			set_logic_flags(m_d[dst]);
			break;
		case 0xe4: // or imm16,Dn
			m_d[dst] |= imm16;
			set_logic_flags(m_d[dst]);
			break;
		case 0xe8: // xor imm16,Dn
			m_d[dst] ^= imm16;
			set_logic_flags(m_d[dst]);
			break;
		case 0xec: // btst imm16,Dn
			set_logic_flags(m_d[dst] & imm16);
			break;
		case 0xf0: // bset imm8,(d8,An)
		case 0xf4: // bclr imm8,(d8,An)
		case 0xf8: // btst imm8,(d8,An)
		{
			const u32 ea = m_a[dst] + util::sext(imm16 & 0xff, 8);
			const u8 mask = imm16 >> 8;
			const u8 v = read_mem8(ea);
			set_logic_flags(v & mask);
			if ((op2 & 0xfc) == 0xf0)
				write_mem8(ea, v | mask);
			else if ((op2 & 0xfc) == 0xf4)
				write_mem8(ea, v & ~mask);
			break;
		}
		case 0xfc:
			if (op2 == 0xfc)      // and imm16,psw
			{
				m_psw &= imm16;
			}
			else if (op2 == 0xfd) // or imm16,psw
			{
				m_psw |= imm16;
			}
			else if (op2 == 0xfe) // add imm16,sp
			{
				m_sp += util::sext(imm16, 16);
			}
			else                  // calls d16: the return address goes at (sp) and in MDR
			{
				write_mem32(m_sp, start_pc + 4);
				m_mdr = start_pc + 4;
				m_pc = start_pc + util::sext(imm16, 16);
				return;
			}
			break;
		default:
			logerror("unimplemented FA %02X @ %08X\n", op2, start_pc);
			break;
		}
	}
	m_pc = start_pc + 4;
}

// 0xfc: 32-bit immediate, displacement and absolute forms. Always 6 bytes.
void mn10300_device::execute_fc()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	const u32 imm = read_arg32(m_pc + 1);
	const int dst = op2 & 3;

	if (op2 < 0x80)
	{
		// mov/movbu/movhu between a register and (d32,An)
		const int type = BIT(op2, 5, 2);
		typed_load_store(type, type == 1, BIT(op2, 2, 2), m_a[dst] + imm, BIT(op2, 4));
	}
	else if (op2 < 0xa0)
	{
		// store to (d32,sp) or (abs32)
		const int type = op2 & 3;
		typed_load_store(type, type == 0, BIT(op2, 2, 2), BIT(op2, 4) ? (m_sp + imm) : imm, true);
	}
	else if (op2 < 0xc0)
	{
		// load from (d32,sp) or (abs32)
		const int type = BIT(op2, 2, 2);
		typed_load_store(type, type == 0, dst, BIT(op2, 4) ? (m_sp + imm) : imm, false);
	}
	else
	{
		switch (op2 & 0xfc)
		{
		case 0xc0: m_d[dst] = do_add(m_d[dst], imm, 0); break;  // add imm32,Dn
		case 0xc4: m_d[dst] = do_sub(m_d[dst], imm, 0); break;  // sub imm32,Dn
		case 0xc8: do_sub(m_d[dst], imm, 0); break;             // cmp imm32,Dn
		case 0xcc: m_d[dst] = imm; break;                       // mov imm32,Dn
		case 0xd0: m_a[dst] = do_add(m_a[dst], imm, 0); break;  // add imm32,An
		case 0xd4: m_a[dst] = do_sub(m_a[dst], imm, 0); break;  // sub imm32,An
		case 0xd8: do_sub(m_a[dst], imm, 0); break;             // cmp imm32,An
		case 0xdc: m_a[dst] = imm; break;                       // mov imm32,An
		case 0xe0: // and imm32,Dn
			m_d[dst] &= imm;
			set_logic_flags(m_d[dst]);
			break;
		case 0xe4: // or imm32,Dn
			m_d[dst] |= imm;
			set_logic_flags(m_d[dst]);
			break;
		case 0xe8: // xor imm32,Dn
			m_d[dst] ^= imm;
			set_logic_flags(m_d[dst]);
			break;
		case 0xec: // btst imm32,Dn
			set_logic_flags(m_d[dst] & imm);
			break;
		case 0xfc:
			if (op2 == 0xfe)      // add imm32,sp
			{
				m_sp += imm;
			}
			else if (op2 == 0xff) // calls d32: the return address goes at (sp) and in MDR
			{
				write_mem32(m_sp, start_pc + 6);
				m_mdr = start_pc + 6;
				m_pc = start_pc + imm;
				return;
			}
			else
			{
				logerror("unimplemented FC %02X @ %08X\n", op2, start_pc);
			}
			break;
		default:
			logerror("unimplemented FC %02X @ %08X\n", op2, start_pc);
			break;
		}
	}
	m_pc = start_pc + 6;
}

// 0xfe: bset/bclr/btst imm8 on an absolute byte, 7 bytes for abs32 and 5 for abs16.
void mn10300_device::execute_fe()
{
	const u32 start_pc = m_pc - 1;
	const u8 op2 = read_arg8(m_pc);
	if (op2 > 0x02 && (op2 < 0x80 || op2 > 0x82))
	{
		logerror("illegal FE %02X @ %08X\n", op2, start_pc);
		m_pc = start_pc + 2;
		return;
	}

	const bool abs16 = BIT(op2, 7);
	const u32 addr = abs16 ? read_arg16(m_pc + 1) : read_arg32(m_pc + 1);
	const u8 mask = read_arg8(m_pc + (abs16 ? 3 : 5));
	const int len = abs16 ? 5 : 7;

	const u8 v = read_mem8(addr);
	set_logic_flags(v & mask);
	if ((op2 & 0x0f) == 0x00)
		write_mem8(addr, v | mask);
	else if ((op2 & 0x0f) == 0x01)
		write_mem8(addr, v & ~mask);
	m_pc = start_pc + len;
}
