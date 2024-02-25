// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * National Semiconductor NS32202 Interrupt Control Unit (ICU).
 *
 * Sources:
 *  - Microprocessor Databook, Series 32000, NSC800, 1989 Edition, National Semiconductor
 *
 * TODO:
 *  - more testing
 */

#include "emu.h"
#include "ns32202.h"

#define LOG_STATE     (1U << 1)
#define LOG_REGW      (1U << 2)
#define LOG_REGR      (1U << 3)
#define LOG_INTERRUPT (1U << 4)
#define LOG_COUNTER   (1U << 5)

//#define VERBOSE (LOG_GENERAL|LOG_STATE|LOG_REGW|LOG_REGR|LOG_INTERRUPT)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NS32202, ns32202_device, "ns32202", "National Semiconductor NS32202 Interrupt Control Unit")

enum mctl_mask : u8
{
	MCTL_T16N8 = 0x01, // data bus mode
	MCTL_NTAR  = 0x02, // not auto-rotate mode
	MCTL_FRZ   = 0x08, // freeze interrupt pending
	MCTL_CLKM  = 0x10, // clock mode (square wave/pulsed)
	MCTL_COUTM = 0x20, // cout mode (square wave/pulsed)
	MCTL_COUTD = 0x40, // cout/scin input/output
	MCTL_CFRZ  = 0x80, // freeze counter readings
};

enum cctl_mask : u8
{
	CCTL_CDCRL = 0x01, // decrement l-counter
	CCTL_CDCRH = 0x02, // decrement h-counter
	CCTL_CRUNL = 0x04, // l-counter running
	CCTL_CRUNH = 0x08, // h-counter running
	CCTL_COUT0 = 0x10, // zero detect l-counter
	CCTL_COUT1 = 0x20, // zero detect h-counter
	CCTL_CFNPS = 0x40, // clock not prescaled
	CCTL_CCON  = 0x80, // counters concatenated
};

enum cictl_mask : u8
{
	CICTL_WENL = 0x01, // l-counter write enable
	CICTL_CIEL = 0x02, // l-counter interrupt enable
	CICTL_CIRL = 0x04, // l-counter interrupt request
	CICTL_CERL = 0x08, // l-counter error flag
	CICTL_WENH = 0x10, // h-counter write enable
	CICTL_CIEH = 0x20, // h-counter interrupt enable
	CICTL_CIRH = 0x40, // h-counter interrupt request
	CICTL_CERH = 0x80, // h-counter error flag
};

ns32202_device::ns32202_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NS32202, tag, owner, clock)
	, m_out_int(*this)
	, m_out_cout(*this)
	, m_out_g(*this)
	, m_hvct(0)
	, m_pdat(0)
	, m_cctl(0)
	, m_csv{}
	, m_ccv{}
	, m_line_state(0xffff)
	, m_out_int_state(false)
	, m_out_cout_state(false)
	, m_pdat_in(0)
	, m_pdat_out(0)
{
}

void ns32202_device::device_start()
{
	save_item(NAME(m_hvct));
	save_item(NAME(m_eltg));
	save_item(NAME(m_tpl));
	save_item(NAME(m_ipnd));
	save_item(NAME(m_isrv));
	save_item(NAME(m_imsk));
	save_item(NAME(m_csrc));
	save_item(NAME(m_fprt));
	save_item(NAME(m_mctl));
	save_item(NAME(m_ocasn));
	save_item(NAME(m_ciptr));
	save_item(NAME(m_pdat));
	save_item(NAME(m_ips));
	save_item(NAME(m_pdir));
	save_item(NAME(m_cctl));
	save_item(NAME(m_cictl));
	save_item(NAME(m_csv));
	save_item(NAME(m_ccv));

	save_item(NAME(m_isrv_count));

	save_item(NAME(m_line_state));
	save_item(NAME(m_out_int_state));
	save_item(NAME(m_out_cout_state));

	m_interrupt = timer_alloc(FUNC(ns32202_device::interrupt), this);
	m_counter[0] = timer_alloc(FUNC(ns32202_device::counter<0>), this);
	m_counter[1] = timer_alloc(FUNC(ns32202_device::counter<1>), this);
}

void ns32202_device::device_reset()
{
	m_eltg = 0xffff;
	m_tpl = 0;
	m_ipnd = 0;
	m_isrv = 0;
	std::fill(std::begin(m_isrv_count), std::end(m_isrv_count), 0);
	m_imsk = 0xffff;
	m_csrc = 0;
	m_fprt = 0x0001;

	m_mctl = MCTL_COUTD;
	m_ocasn = 0;
	m_ciptr = 0xff;
	m_ips = 0xff;
	m_pdir = 0xff;
	m_cictl = 0;

	set_int(false);
	set_cout(false);

	update_port();
}

void ns32202_device::set_int(bool int_state)
{
	if (int_state != m_out_int_state)
	{
		LOGMASKED(LOG_STATE, "int %s\n", int_state ? "asserted" : "cleared");

		m_out_int_state = int_state;
		m_out_int(!m_out_int_state);
	}
}

void ns32202_device::set_cout(bool cout_state)
{
	if (cout_state != m_out_cout_state)
	{
		LOGMASKED(LOG_COUNTER, "cout %s\n", cout_state ? "asserted" : "cleared");

		m_out_cout_state = cout_state;
		m_out_cout(!m_out_cout_state);
	}
}

template <unsigned ST1> void ns32202_device::map(address_map &map)
{
	map(0x00, 0x00).r(&ns32202_device::hvct_r<ST1, true>, "ns32202_device::hvct_r");
	map(0x01, 0x01).rw(&ns32202_device::hvct_r<ST1, false>, "ns32202_device::svct_r", FUNC(ns32202_device::svct_w));
	map(0x02, 0x02).rw(FUNC(ns32202_device::eltgl_r), FUNC(ns32202_device::eltgl_w));
	map(0x03, 0x03).rw(FUNC(ns32202_device::eltgh_r), FUNC(ns32202_device::eltgh_w));
	map(0x04, 0x04).rw(FUNC(ns32202_device::tpll_r), FUNC(ns32202_device::tpll_w));
	map(0x05, 0x05).rw(FUNC(ns32202_device::tplh_r), FUNC(ns32202_device::tplh_w));
	map(0x06, 0x06).rw(FUNC(ns32202_device::ipndl_r), FUNC(ns32202_device::ipndl_w));
	map(0x07, 0x07).rw(FUNC(ns32202_device::ipndh_r), FUNC(ns32202_device::ipndh_w));
	map(0x08, 0x08).rw(FUNC(ns32202_device::isrvl_r), FUNC(ns32202_device::isrvl_w));
	map(0x09, 0x09).rw(FUNC(ns32202_device::isrvh_r), FUNC(ns32202_device::isrvh_w));
	map(0x0a, 0x0a).rw(FUNC(ns32202_device::imskl_r), FUNC(ns32202_device::imskl_w));
	map(0x0b, 0x0b).rw(FUNC(ns32202_device::imskh_r), FUNC(ns32202_device::imskh_w));
	map(0x0c, 0x0c).rw(FUNC(ns32202_device::csrcl_r), FUNC(ns32202_device::csrcl_w));
	map(0x0d, 0x0d).rw(FUNC(ns32202_device::csrch_r), FUNC(ns32202_device::csrch_w));
	map(0x0e, 0x0e).rw(FUNC(ns32202_device::fprtl_r), FUNC(ns32202_device::fprtl_w));
	map(0x0f, 0x0f).rw(FUNC(ns32202_device::fprth_r), FUNC(ns32202_device::fprth_w));
	map(0x10, 0x10).rw(FUNC(ns32202_device::mctl_r), FUNC(ns32202_device::mctl_w));
	map(0x11, 0x11).rw(FUNC(ns32202_device::ocasn_r), FUNC(ns32202_device::ocasn_w));
	map(0x12, 0x12).rw(FUNC(ns32202_device::ciptr_r), FUNC(ns32202_device::ciptr_w));
	map(0x13, 0x13).rw(FUNC(ns32202_device::pdat_r), FUNC(ns32202_device::pdat_w));
	map(0x14, 0x14).rw(FUNC(ns32202_device::ips_r), FUNC(ns32202_device::ips_w));
	map(0x15, 0x15).rw(FUNC(ns32202_device::pdir_r), FUNC(ns32202_device::pdir_w));
	map(0x16, 0x16).rw(FUNC(ns32202_device::cctl_r), FUNC(ns32202_device::cctl_w));
	map(0x17, 0x17).rw(FUNC(ns32202_device::cictl_r), FUNC(ns32202_device::cictl_w));
	map(0x18, 0x18).rw(FUNC(ns32202_device::csvl_r<0>), FUNC(ns32202_device::csvl_w<0>));
	map(0x19, 0x19).rw(FUNC(ns32202_device::csvh_r<0>), FUNC(ns32202_device::csvh_w<0>));
	map(0x1a, 0x1a).rw(FUNC(ns32202_device::csvl_r<1>), FUNC(ns32202_device::csvl_w<1>));
	map(0x1b, 0x1b).rw(FUNC(ns32202_device::csvh_r<1>), FUNC(ns32202_device::csvh_w<1>));
	map(0x1c, 0x1c).rw(FUNC(ns32202_device::ccvl_r<0>), FUNC(ns32202_device::ccvl_w<0>));
	map(0x1d, 0x1d).rw(FUNC(ns32202_device::ccvh_r<0>), FUNC(ns32202_device::ccvh_w<0>));
	map(0x1e, 0x1e).rw(FUNC(ns32202_device::ccvl_r<1>), FUNC(ns32202_device::ccvl_w<1>));
	map(0x1f, 0x1f).rw(FUNC(ns32202_device::ccvh_r<1>), FUNC(ns32202_device::ccvh_w<1>));
}

template void ns32202_device::map<0>(address_map &map);
template void ns32202_device::map<1>(address_map &map);

/*
 * Set (and clear, for level-triggered interrupts) interrupt pending state
 * based on edge/level/polarity configuration and previous/current line state,
 * regardless of mask.
 */
template <unsigned Number> void ns32202_device::ir_w(int state)
{
	LOGMASKED(LOG_INTERRUPT, "ir_w%d %d\n", Number, state);

	// ignore external interrupts assigned to counters
	if (((m_cictl & CICTL_CIEL) && (m_ciptr & 15) == Number) ||
		((m_cictl & CICTL_CIEH) && (m_ciptr >> 4) == Number))
		return;

	u16 const mask = 1 << Number;

	if (m_eltg & mask)
	{
		// level triggered
		if (state == BIT(m_tpl, Number))
		{
			if (!(m_mctl & MCTL_FRZ))
				m_ipnd |= mask;
		}
		else
			m_ipnd &= ~mask;
	}
	else
	{
		// TODO: freeze bit MCTL_FRZ causes delayed edge-triggered recognition?

		// edge triggered
		if (bool(state) == BIT(m_tpl, Number) && bool(state) ^ BIT(m_line_state, Number))
			m_ipnd |= mask;
	}

	// record input line state
	if (state)
		m_line_state |= mask;
	else
		m_line_state &= ~mask;

	// datasheet states maximum 800ns
	m_interrupt->adjust(attotime::from_nsec(600));
}

// instantiate all valid interrupt request templates
template void ns32202_device::ir_w<0>(int state);
template void ns32202_device::ir_w<1>(int state);
template void ns32202_device::ir_w<2>(int state);
template void ns32202_device::ir_w<3>(int state);
template void ns32202_device::ir_w<4>(int state);
template void ns32202_device::ir_w<5>(int state);
template void ns32202_device::ir_w<6>(int state);
template void ns32202_device::ir_w<7>(int state);
template void ns32202_device::ir_w<8>(int state);
template void ns32202_device::ir_w<9>(int state);
template void ns32202_device::ir_w<10>(int state);
template void ns32202_device::ir_w<11>(int state);
template void ns32202_device::ir_w<12>(int state);
template void ns32202_device::ir_w<13>(int state);
template void ns32202_device::ir_w<14>(int state);
template void ns32202_device::ir_w<15>(int state);

/*
 * Assert interrupt output if there are any unmasked pending interrupts; and
 *   - in auto-rotate mode and no interrupts are in-service; or
 *   - in fixed priority mode; and
 *     - no interrupts are in-service; or
 *     - unmasked pending interrupt has priority > in-service interrupt; or
 *     - unmasked pending cascade interrupt has priorty >= in-service interrupt
 */
void ns32202_device::interrupt(s32 param)
{
	bool int_state = false;

	// check for unmasked pending interrupts
	if (m_ipnd & ~m_imsk)
	{
		// fixed priority mode
		if (m_mctl & MCTL_NTAR)
		{
			// check any interrupts in-service
			if (m_isrv)
			{
				// check interrupts in descending priority order
				u16 mask = m_fprt;
				for (unsigned i = 0; i < 16; i++)
				{
					// check interrupt in-service
					if (m_isrv & mask)
					{
						// check equal priority unmasked pending cascade interrupt
						if ((m_csrc & mask) && (m_ipnd & mask) && !(m_imsk & mask))
						{
							LOGMASKED(LOG_STATE, "unmasked pending cascade in-service interrupt %d\n", 31 - count_leading_zeros_32(mask));
							int_state = true;
						}

						break;
					}

					// check unmasked pending interrupt
					if ((m_ipnd & mask) && !(m_imsk & mask))
					{
						LOGMASKED(LOG_STATE, "unmasked pending interrupt %d\n", 31 - count_leading_zeros_32(mask));
						int_state = true;
						break;
					}

					// rotate priority mask
					mask = (mask << 1) | (mask >> 15);
				}
			}
			else
				int_state = true;
		}
		else if (!m_isrv)
			int_state = true;
	}

	set_int(int_state);
}

u8 ns32202_device::interrupt_acknowledge(bool side_effects)
{
	side_effects &= !machine().side_effects_disabled();
	u8 vector = m_hvct | 0x0f;

	if ((m_ipnd & ~m_imsk) && m_fprt)
	{
		// find highest priority unmasked pending interrupt
		u16 mask = m_fprt;
		for (unsigned i = 0; i < 16; i++)
		{
			if ((m_ipnd & mask) && !(m_imsk & mask))
				break;

			// rotate priority mask
			mask = (mask << 1) | (mask >> 15);
		}

		unsigned const number = 31 - count_leading_zeros_32(mask);
		if (side_effects)
		{
			LOGMASKED(LOG_STATE, "acknowledge highest priority unmasked interrupt %d\n", number);

			if (m_mctl & MCTL_NTAR)
			{
				if (m_csrc & mask)
					m_isrv_count[number]++;
			}
			else
				m_fprt = mask;

			// mark interrupt in-service
			m_isrv |= mask;

			// clear interrupt pending (only if edge-triggered or internal)
			if (!(m_eltg & mask) || ((m_line_state ^ m_tpl) & mask))
				m_ipnd &= ~mask;

			// clear l-counter interrupt pending
			if ((m_cictl & CICTL_CIEL) && (m_cictl & CICTL_CIRL) && BIT(mask, m_ciptr & 15))
				m_cictl &= ~CICTL_CIRL;

			// clear h-counter interrupt pending
			if ((m_cictl & CICTL_CIEH) && (m_cictl & CICTL_CIRH) && BIT(mask, m_ciptr >> 4))
				m_cictl &= ~CICTL_CIRH;
		}

		// compute acknowledge vector
		if (m_csrc & mask)
			vector = 0xf0 | number;
		else
			vector = m_hvct | number;
	}
	else if (side_effects)
	{
		if (m_fprt)
			LOGMASKED(LOG_STATE, "acknowledge without unmasked interrupt pending\n");
		else
			LOGMASKED(LOG_STATE, "acknowledge with FPRT clear\n");

		// clear pending edge for interrupt 15
		if (!BIT(m_eltg, 15))
			m_ipnd &= ~(1 << 15);

		// clear first priority
		if (!(m_mctl & MCTL_NTAR))
			m_fprt = 0;
	}

	if (side_effects)
	{
		// clear interrupt output
		if (!(m_ipnd & ~m_imsk & ~m_isrv) || !m_fprt)
			set_int(false);
	}

	return vector;
}

u8 ns32202_device::interrupt_return(bool side_effects)
{
	side_effects &= !machine().side_effects_disabled();
	u8 vector = m_hvct | 0x0f;

	// find highest priority in-service interrupt
	if (m_isrv && m_fprt)
	{
		u16 mask = m_fprt;
		for (unsigned i = 0; i < 16; i++)
		{
			if (m_isrv & mask)
				break;

			// rotate priority mask
			mask = (mask << 1) | (mask >> 15);
		}
		unsigned const number = 31 - count_leading_zeros_32(mask);

		if (side_effects)
		{
			LOGMASKED(LOG_STATE, "return highest priority in-service interrupt %d\n", number);

			if (m_mctl & MCTL_NTAR)
			{
				if (m_csrc & mask)
				{
					m_isrv_count[number]--;

					if (!m_isrv_count[number])
						m_isrv &= ~mask;
				}
				else
					// clear interrupt in-service
					m_isrv &= ~mask;
			}
			else
			{
				// clear interrupt in-service
				m_isrv &= ~mask;

				// rotate priority mask
				m_fprt = (m_fprt << 1) | (m_fprt >> 15);
			}
		}

		// compute return vector
		if (m_csrc & mask)
			vector = 0xf0 | number;
		else
			vector = m_hvct | number;
	}
	else if (side_effects)
	{
		if (m_fprt)
			LOGMASKED(LOG_STATE, "return without in-service interrupt\n");
		else
			LOGMASKED(LOG_STATE, "return with FPRT clear\n");

		if (!(m_mctl & MCTL_NTAR))
			// rotate priority mask
			m_fprt = (m_fprt << 1) | (m_fprt >> 15);
	}

	return vector;
}

/*
 * Check for level-triggered interrupts which become pending due to change of
 * edge/level or polarity registers.
 */
void ns32202_device::interrupt_update()
{
	if (m_mctl & MCTL_FRZ)
		return;

	// compute new pending state
	u16 const ipnd = m_ipnd | (m_eltg & ~(m_line_state ^ m_tpl));

	// update and assert if state changed
	if (ipnd ^ m_ipnd)
	{
		m_ipnd = ipnd;
		m_interrupt->adjust(attotime::zero);
	}
}

// N=0 -> l-counter
template <unsigned N> void ns32202_device::counter(s32 param)
{
	u32 const scaled_clock = clock() / ((m_cctl & CCTL_CFNPS) ? 1 : 4);

	// for now, assume this is the periodic timer triggered when we hit zero
	// reload on cycle after zero
	if (param)
	{
		u32 const ticks = (m_cctl & CCTL_CCON)
			? ((u32(m_csv[1]) << 16) | m_csv[0]) - ((u32(m_ccv[1]) << 16) | m_ccv[0])
			: m_csv[N] - m_ccv[N];

		// reload current value
		if (m_cctl & CCTL_CCON)
		{
			m_ccv[0] = m_csv[0];
			m_ccv[1] = m_csv[1];
		}
		else
			m_ccv[N] = m_csv[N];

		// reschedule counter
		m_counter[N]->adjust(attotime::from_ticks(ticks, scaled_clock), 0);
	}
	else
	{
		// clear current value
		if (m_cctl & CCTL_CCON)
		{
			m_ccv[0] = 0;
			m_ccv[1] = 0;
		}
		else
			m_ccv[N] = 0;

		// schedule reload cycle
		m_counter[N]->adjust(attotime::from_ticks(1, scaled_clock), 1);

		// update cout
		if (!(m_mctl & MCTL_COUTD) && (m_cctl & (CCTL_COUT0 << N)))
		{
			if (m_mctl & MCTL_COUTM)
			{
				set_cout(true);
				set_cout(false);
			}
			else
				set_cout(!m_out_cout_state);
		}

		// update port
		if ((N == 1) && !(m_mctl & MCTL_T16N8) && (m_ocasn & 15))
		{
			// TODO: trigger interrupts if IPS != 0

			if (m_mctl & MCTL_CLKM)
			{
				for (unsigned i = 0; i < 4; i++)
				{
					if (BIT(m_ocasn, i) && !BIT(m_pdir, i))
					{
						m_out_g[i](1);
						m_out_g[i](0);
					}
				}
			}
			else
			{
				for (unsigned i = 0; i < 4; i++)
				{
					if (BIT(m_ocasn, i) && !BIT(m_pdir, i))
					{
						m_pdat_out ^= 1U << i;
						m_out_g[i](BIT(m_pdat_out, i));
					}
				}
			}
		}

		// interrupts
		unsigned const shift = N ? 4 : 0;
		if (m_cictl & (CICTL_CIEL << shift))
		{
			// check counter interrupt error
			if (m_cictl & (CICTL_CIRL << shift))
				m_cictl |= (CICTL_CERL << shift);

			// set counter interrupt request
			m_cictl |= (CICTL_CIRL << shift);

			// raise interrupt
			m_ipnd |= 1 << ((m_ciptr >> shift) & 15);
			m_interrupt->adjust(attotime::zero);

			LOGMASKED(LOG_COUNTER, "counter %d interrupt %d\n", N, (m_ciptr >> shift) & 15);
		}
	}
}

template <unsigned ST1, bool SideEffects> u8 ns32202_device::hvct_r()
{
	u8 data = ST1 ? interrupt_return(SideEffects) : interrupt_acknowledge(SideEffects);
	LOGMASKED(LOG_REGR, "%cvct_r 0x%02x%s (%s)\n",
		SideEffects ? 'h' : 's', data, SideEffects ? (ST1 ? " interrupt return" : " interrupt acknowledge") : "", machine().describe_context());

	return data;
}

void ns32202_device::svct_w(u8 data)
{
	LOGMASKED(LOG_REGW, "svct_w 0x%02x (%s)\n", data, machine().describe_context());
	m_hvct = data & 0xf0;
}

void ns32202_device::eltgl_w(u8 data)
{
	LOGMASKED(LOG_REGW, "eltgl_w 0x%02x (%s)\n", data, machine().describe_context());
	m_eltg = (m_eltg & 0xff00) | data;

	interrupt_update();
}

void ns32202_device::eltgh_w(u8 data)
{
	LOGMASKED(LOG_REGW, "eltgh_w 0x%02x (%s)\n", data, machine().describe_context());
	m_eltg = (u16(data) << 8) | u8(m_eltg);

	interrupt_update();
}

void ns32202_device::tpll_w(u8 data)
{
	LOGMASKED(LOG_REGW, "tpll_w 0x%02x (%s)\n", data, machine().describe_context());
	m_tpl = (m_tpl & 0xff00) | data;

	interrupt_update();
}

void ns32202_device::tplh_w(u8 data)
{
	LOGMASKED(LOG_REGW, "tplh_w 0x%02x (%s)\n", data, machine().describe_context());
	m_tpl = (u16(data) << 8) | u8(m_tpl);

	interrupt_update();
}

void ns32202_device::csrcl_w(u8 data)
{
	m_csrc = (m_csrc & 0xff00) | data;

	// clear in-service counters
	for (unsigned i = 0; i < 8; i++)
		if (!BIT(m_csrc, i))
			m_isrv_count[i] = 0;
}

void ns32202_device::csrch_w(u8 data)
{
	m_csrc = (u16(data) << 8) | u8(m_csrc);

	// clear in-service counters
	for (unsigned i = 8; i < 16; i++)
		if (!BIT(m_csrc, i))
			m_isrv_count[i] = 0;
}

void ns32202_device::ipndl_w(u8 data)
{
	if (BIT(data, 6))
	{
		// clear all pending interrupts
		LOGMASKED(LOG_REGW, "ipndl_w 0x%02x clear all pending interrupts (%s)\n", data, machine().describe_context());

		m_ipnd &= 0xff00;
	}
	else if ((data & 0xf8) == 0x80)
	{
		// set pending interrupt
		LOGMASKED(LOG_REGW, "ipndl_w 0x%02x set pending interrupt %d (%s)\n", data, data & 15, machine().describe_context());

		m_ipnd |= 1 << (data & 15);
	}
	else if ((data & 0xf8) == 0x00)
	{
		// clear pending interrupt
		LOGMASKED(LOG_REGW, "ipndl_w 0x%02x clear pending interrupt %d (%s)\n", data, data & 15, machine().describe_context());

		m_ipnd &= ~(1 << (data & 15));
	}
	else
		LOGMASKED(LOG_REGW, "ipndl_w 0x%02x unknown (%s)\n", data, machine().describe_context());

	m_interrupt->adjust(attotime::zero);
}

void ns32202_device::ipndh_w(u8 data)
{
	if (BIT(data, 6))
	{
		// clear all pending interrupts
		LOGMASKED(LOG_REGW, "ipndh_w 0x%02x clear all pending interrupts (%s)\n", data, machine().describe_context());

		m_ipnd &= 0x00ff;
	}
	else if ((data & 0xf8) == 0x88)
	{
		// set pending interrupt
		LOGMASKED(LOG_REGW, "ipndh_w 0x%02x set pending interrupt %d (%s)\n", data, data & 15, machine().describe_context());

		m_ipnd |= 1 << (data & 15);
	}
	else if ((data & 0xf8) == 0x08)
	{
		// clear pending interrupt
		LOGMASKED(LOG_REGW, "ipndh_w 0x%02x clear pending interrupt %d (%s)\n", data, data & 15, machine().describe_context());

		m_ipnd &= ~(1 << (data & 15));
	}
	else
		LOGMASKED(LOG_REGW, "ipndh_w 0x%02x unknown (%s)\n", data, machine().describe_context());

	m_interrupt->adjust(attotime::zero);
}

void ns32202_device::isrvl_w(u8 data)
{
	LOGMASKED(LOG_REGW, "isrvl_w 0x%02x (%s)\n", data, machine().describe_context());
	m_isrv = (m_isrv & 0xff00) | data;
}

void ns32202_device::isrvh_w(u8 data)
{
	LOGMASKED(LOG_REGW, "isrvh_w 0x%02x (%s)\n", data, machine().describe_context());
	m_isrv = (u16(data) << 8) | u8(m_isrv);
}

void ns32202_device::imskl_w(u8 data)
{
	if (data ^ u8(m_imsk))
		LOGMASKED(LOG_REGW, "imskl_w 0x%02x (%s)\n", data, machine().describe_context());
	m_imsk = (m_imsk & 0xff00) | data;

	m_interrupt->adjust(attotime::zero);
}

void ns32202_device::imskh_w(u8 data)
{
	if (data ^ (m_imsk >> 8))
		LOGMASKED(LOG_REGW, "imskh_w 0x%02x (%s)\n", data, machine().describe_context());
	m_imsk = (u16(data) << 8) | u8(m_imsk);

	m_interrupt->adjust(attotime::zero);
}

void ns32202_device::fprtl_w(u8 data)
{
	m_fprt = 1 << (data & 15);
}

void ns32202_device::cctl_w(u8 data)
{
	// disable l-counter in concatenated mode
	if ((data & CCTL_CCON) && m_counter[0]->enabled())
		m_counter[0]->enable(false);

	// compute scaled clock
	u32 const scaled_clock = clock() / ((data & CCTL_CFNPS) ? 1 : 4);

	// start/stop h-counter
	if (!(m_cctl & CCTL_CRUNH) && (data & CCTL_CRUNH))
	{
		LOGMASKED(LOG_COUNTER, "cctl_w start h-counter clock %d\n", scaled_clock);
		m_counter[1]->adjust(attotime::from_ticks(1, scaled_clock), 1);
	}
	else if ((m_cctl & CCTL_CRUNH) && !(data & CCTL_CRUNH))
	{
		LOGMASKED(LOG_COUNTER, "cctl_w stop h-counter\n");
		update_ccv();
		m_counter[1]->enable(false);
	}

	if (!(data & CCTL_CRUNH) && (data & CCTL_CDCRH))
		{} // TODO: decrement h-counter

	// start/stop l-counter
	if (!(data & CCTL_CCON))
	{
		if (!(m_cctl & CCTL_CRUNL) && (data & CCTL_CRUNL))
		{
			LOGMASKED(LOG_COUNTER, "cctl_w start l-counter clock %d\n", scaled_clock);
			m_counter[0]->adjust(attotime::from_ticks(1, scaled_clock), 1);
		}
		else if ((m_cctl & CCTL_CRUNL) && !(data & CCTL_CRUNL))
		{
			LOGMASKED(LOG_COUNTER, "cctl_w stop l-counter\n");
			update_ccv();
			m_counter[0]->enable(false);
		}

		if (!(data & CCTL_CRUNL) && (data & CCTL_CDCRL))
			{} // TODO: decrement l-counter
	}

	m_cctl = data & ~(CCTL_CDCRH | CCTL_CDCRL);
}

void ns32202_device::cictl_w(u8 data)
{
	u8 const mask =
		((data & CICTL_WENL) ? (CICTL_CERL | CICTL_CIRL | CICTL_CIEL | CICTL_WENL) : 0) |
		((data & CICTL_WENH) ? (CICTL_CERH | CICTL_CIRH | CICTL_CIEH | CICTL_WENH) : 0);

	m_cictl = (m_cictl & ~mask) | (data & mask);
}

template <unsigned N> void ns32202_device::csvl_w(u8 data)
{
	LOGMASKED(LOG_REGW, "csvl%d_w 0x%02x (%s)\n", N, data, machine().describe_context());
	m_csv[N] = (m_csv[N] & 0xff00) | data;
}

template <unsigned N> void ns32202_device::csvh_w(u8 data)
{
	LOGMASKED(LOG_REGW, "csvh%d_w 0x%02x (%s)\n", N, data, machine().describe_context());
	m_csv[N] = (u16(data) << 8) | u8(m_csv[N]);
}

template <unsigned N> void ns32202_device::ccvl_w(u8 data)
{
	LOGMASKED(LOG_REGW, "ccvl%d_w 0x%02x (%s)\n", N, data, machine().describe_context());
	if ((N == 0 && !(m_cctl & CCTL_CRUNL)) || ((N == 1) && !(m_cctl & CCTL_CRUNH)))
		m_ccv[N] = (m_ccv[N] & 0xff00) | data;
}

template <unsigned N> void ns32202_device::ccvh_w(u8 data)
{
	LOGMASKED(LOG_REGW, "ccvh%d_w 0x%02x (%s)\n", N, data, machine().describe_context());
	if ((N == 0 && !(m_cctl & CCTL_CRUNL)) || ((N == 1) && !(m_cctl & CCTL_CRUNH)))
		m_ccv[N] = (u16(data) << 8) | u8(m_ccv[N]);
}

void ns32202_device::mctl_w(u8 data)
{
	LOGMASKED(LOG_REGW, "mctl_w 0x%02x (%s)\n", data, machine().describe_context());
	if (!(m_mctl & MCTL_CFRZ) && (data & MCTL_CFRZ))
		update_ccv();

	m_mctl = data;
}

void ns32202_device::ocasn_w(u8 data)
{
	if (VERBOSE & LOG_REGW)
	{
		std::string outputs;
		if (data & 0x0f)
		{
			for (unsigned i = 0; i < 4; i++)
				if (BIT(data, i))
					outputs.append(util::string_format(" g%d/ir%d", i, i * 2));
		}

		LOGMASKED(LOG_REGW, "ocasn_w 0x%02x%s (%s)\n", data, outputs, machine().describe_context());
	}

	m_ocasn = data;
}

void ns32202_device::ciptr_w(u8 data)
{
	LOGMASKED(LOG_REGW, "ciptr_w 0x%02x h=%d l=%d (%s)\n", data, BIT(data, 4, 4), BIT(data, 0, 4), machine().describe_context());
	m_ciptr = data;
}

void ns32202_device::update_ccv()
{
	u32 const scaled_clock = clock() / ((m_cctl & CCTL_CFNPS) ? 1 : 4);

	if (m_cctl & CCTL_CCON)
	{
		if (m_cctl & CCTL_CRUNH)
		{
			u32 const delta = ((u32(m_csv[1]) << 16) | m_csv[0]) - m_counter[1]->elapsed().as_ticks(scaled_clock);

			m_ccv[1] = delta >> 16;
			m_ccv[0] = u16(delta);
		}
	}
	else
	{
		if (m_cctl & CCTL_CRUNH)
			m_ccv[1] = m_csv[1] - m_counter[1]->elapsed().as_ticks(scaled_clock);

		if (m_cctl & CCTL_CRUNL)
			m_ccv[0] = m_csv[0] - m_counter[0]->elapsed().as_ticks(scaled_clock);
	}
}

template <unsigned Number> void ns32202_device::g_w(int state)
{
	if (state)
		m_pdat_in |= 1U << Number;
	else
		m_pdat_in &= ~(1U << Number);
}

// instantiate all valid general purpose input templates
template void ns32202_device::g_w<0>(int state);
template void ns32202_device::g_w<1>(int state);
template void ns32202_device::g_w<2>(int state);
template void ns32202_device::g_w<3>(int state);
template void ns32202_device::g_w<4>(int state);
template void ns32202_device::g_w<5>(int state);
template void ns32202_device::g_w<6>(int state);
template void ns32202_device::g_w<7>(int state);

u8 ns32202_device::pdat_r()
{
	u8 const data = ((m_pdat_in & m_pdir) | (m_pdat & ~m_pdir)) & ~m_ips & ~m_ocasn;
	LOGMASKED(LOG_REGR, "pdat_r 0x%02x (%s)\n", data, machine().describe_context());

	return data;
}

void ns32202_device::pdat_w(u8 data)
{
	LOGMASKED(LOG_REGW, "pdat_w 0x%02x (%s)\n", data, machine().describe_context());
	m_pdat = data;

	update_port();
}

void ns32202_device::ips_w(u8 data)
{
	LOGMASKED(LOG_REGW, "ips_w 0x%02x (%s)\n", data, machine().describe_context());
	m_ips = data;

	update_port();
}

void ns32202_device::pdir_w(u8 data)
{
	LOGMASKED(LOG_REGW, "pdir_w 0x%02x (%s)\n", data, machine().describe_context());
	m_pdir = data;

	update_port();
}

void ns32202_device::update_port()
{
	// find port output bits which need to be updated
	u8 const output = ~m_ips & ~m_pdir & ~m_ocasn & (m_pdat ^ m_pdat_out);

	// release interrupt/input lines (assume float high)
	u8 const release = (m_ips | m_pdir) & ~m_ocasn & ~m_pdat_out;

	for (unsigned i = 0; i < 8; i++)
	{
		if (BIT(output, i))
		{
			// record the output state
			if (BIT(m_pdat, i))
				m_pdat_out |= 1U << i;
			else
				m_pdat_out &= ~(1U << i);

			// update the line
			m_out_g[i](BIT(m_pdat, i));
		}
		else if (BIT(release, i))
		{
			m_pdat_out |= 1U << i;

			m_out_g[i](1);
		}
	}
}
