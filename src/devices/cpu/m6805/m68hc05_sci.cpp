// license:BSD-3-Clause
// copyright-holders:Adam Di Carlo
/***************************************************************************

    Motorola M68HC05 serial communications interface (SCI)

    Section references are to the MC68HC05C4/C8 data sheet (ADI991R2).

    Both halves run from the RT clock, 16 times the bit rate (5.4, 5.6),
    counted from reset or the last rate change.  The transmitter starts a
    frame, preamble or break only on a bit boundary of that clock, and moves
    a waiting byte to the shifter (setting TDRE) at the boundary where the
    last block ends.  The receiver takes RT1 as the first RT sample to see
    RDI low after three high samples, verifies the start bit at RT3, RT5 and
    RT7, then takes each bit as the majority of the samples at RT8, RT9 and
    RT10 (figures 5-2, 5-3).

    Implemented: 8 and 9 bit frames, the BAUD rates (5.7.5), the double
    buffered transmitter with its preamble and break (5.7.3), TDRE and TC,
    the receiver with RDRF, IDLE, OR, NF and FE (5.7.4), the artificial start
    bit after a framing error that isn't a break (5.5), receiver wake-up on
    an idle line or an address bit (5.3), the flag clearing sequences and
    the interrupt enables.

    Inferred or approximate:
    - An idle line is RDI high for a whole frame (10 or 11 bit times) after
      a stop bit or after RDI rises.  IDLE is first possible after the first
      RDRF.  "RWU cannot be set if the line is idle" is taken to mean while
      an idle line has been detected.
    - Break needs TE.  A byte written to SCDR while TE is clear is held,
      and goes out after the preamble when TE is set.
    - TC stays as it was while a preamble or break goes out; the data sheet
      only says when it sets.
    - A rate change restarts the RT clock: a bit being sent ends a new bit
      time later, and a byte being received is dropped.
    - RDI reads high until it is driven.  TDO isn't released to high
      impedance when TE clears: it stays high.  The SCI doesn't take over
      PD0 and PD1 from the host's port D.
    - SCDR's two registers are left alone by reset (table 3-1 doesn't list
      them).

***************************************************************************/

#include "emu.h"
#include "m68hc05_sci.h"


#define LOG_SCI     (1U << 1)

//#define VERBOSE (LOG_SCI)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGSCI(...)     LOGMASKED(LOG_SCI, __VA_ARGS__)


namespace {

// 5.7.5: SCP1:0 divide the internal processor clock by 1, 3, 4 or 13
constexpr u8 SCP_DIVISOR[4] = { 1, 3, 4, 13 };

} // anonymous namespace


DEFINE_DEVICE_TYPE(M68HC05_SCI, m68hc05_sci_device, "m68hc05_sci", "Motorola M68HC05 SCI")


m68hc05_sci_device::m68hc05_sci_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
	device_t(mconfig, M68HC05_SCI, tag, owner, clock),
	m_tdo_cb(*this),
	m_irq_cb(*this),
	m_tx_timer(nullptr),
	m_rx_timer(nullptr),
	m_idle_timer(nullptr),
	m_baud(0x00),
	m_sccr1(0x00),
	m_sccr2(0x00),
	m_scsr(SCSR_TDRE | SCSR_TC), m_scsr_seen(0x00),
	m_rdr(0x00),
	m_tdr(0x00),
	m_irq_state(false),
	m_epoch(attotime::zero),
	m_rt(1),
	m_tdr_full(false),
	m_preamble_pending(false),
	m_break_pending(false),
	m_tx_state(TX_IDLE),
	m_tx_shift(0),
	m_tx_count(0),
	m_tdo(1),
	m_rdi(1),
	m_rdi_rise(attotime::zero),
	m_line_idle(false),
	m_idle_armed(false),
	m_rx_state(RX_IDLE),
	m_rx_rt1(0),
	m_rx_sample(0),
	m_rx_votes(0),
	m_rx_noise(false),
	m_rx_shift(0)
{
}


void m68hc05_sci_device::map(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(m68hc05_sci_device::baud_r), FUNC(m68hc05_sci_device::baud_w));
	map(0x1, 0x1).rw(FUNC(m68hc05_sci_device::sccr1_r), FUNC(m68hc05_sci_device::sccr1_w));
	map(0x2, 0x2).rw(FUNC(m68hc05_sci_device::sccr2_r), FUNC(m68hc05_sci_device::sccr2_w));
	map(0x3, 0x3).r(FUNC(m68hc05_sci_device::scsr_r));
	map(0x4, 0x4).rw(FUNC(m68hc05_sci_device::scdr_r), FUNC(m68hc05_sci_device::scdr_w));
}


void m68hc05_sci_device::device_start()
{
	m_tx_timer = timer_alloc(FUNC(m68hc05_sci_device::tx_tick), this);
	m_rx_timer = timer_alloc(FUNC(m68hc05_sci_device::rx_sample), this);
	m_idle_timer = timer_alloc(FUNC(m68hc05_sci_device::idle_detected), this);

	save_item(NAME(m_baud));
	save_item(NAME(m_sccr1));
	save_item(NAME(m_sccr2));
	save_item(NAME(m_scsr));
	save_item(NAME(m_scsr_seen));
	save_item(NAME(m_rdr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_epoch));
	save_item(NAME(m_rt));
	save_item(NAME(m_tdr_full));
	save_item(NAME(m_preamble_pending));
	save_item(NAME(m_break_pending));
	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_tdo));
	save_item(NAME(m_rdi));
	save_item(NAME(m_rdi_rise));
	save_item(NAME(m_line_idle));
	save_item(NAME(m_idle_armed));
	save_item(NAME(m_rx_state));
	save_item(NAME(m_rx_rt1));
	save_item(NAME(m_rx_sample));
	save_item(NAME(m_rx_votes));
	save_item(NAME(m_rx_noise));
	save_item(NAME(m_rx_shift));
}

void m68hc05_sci_device::device_reset()
{
	// table 3-1: SCP1:0 are cleared; 5.7.5: SCR2:0 are not affected
	m_baud &= BAUD_SCR;
	restart_clock();

	// table 3-1: TE, RE, TIE, TCIE, RIE, ILIE, RWU, SBK and the receive flags are cleared,
	// TDRE and TC set; 5.7.2: R8, T8, M and WAKE are not affected
	m_sccr2 = 0x00;
	m_scsr = SCSR_TDRE | SCSR_TC;
	m_scsr_seen = 0x00;

	m_tdr_full = false;
	m_preamble_pending = false;
	m_break_pending = false;
	m_tx_state = TX_IDLE;
	m_tx_count = 0;
	m_tx_timer->adjust(attotime::never);
	m_tdo = 1;
	m_tdo_cb(1);

	m_line_idle = false;
	m_idle_armed = false;
	m_rx_state = RX_IDLE;
	m_rx_timer->adjust(attotime::never);
	m_idle_timer->adjust(attotime::never);
	if (m_rdi)
		arm_idle(machine().time());

	m_irq_state = false;
	m_irq_cb(0);
}

void m68hc05_sci_device::device_clock_changed()
{
	if (started())
		update_rate(true);
}


void m68hc05_sci_device::rdi_w(int state)
{
	state = state ? 1 : 0;
	if (state == m_rdi)
		return;
	m_rdi = state;

	attotime const now(machine().time());
	if (state)
	{
		m_rdi_rise = now;
		if (m_rx_state != RX_FRAME)
			arm_idle(now);
		return;
	}

	m_idle_timer->adjust(attotime::never);
	m_line_idle = false;
	if (!re() || !clock())
		return;

	u64 const rt1(next_tick(now, m_rt));
	if (m_rx_state == RX_IDLE)
	{
		// figure 5-2: three high samples before RT1
		if ((rt1 >= 3 * m_rt) && (tick_time(rt1 - 3 * m_rt) >= m_rdi_rise))
			rx_start(rt1);
	}
	else if (m_rx_state == RX_ARTIFICIAL)
	{
		// 5.5: the start qualifiers are forced, and the start bit is accepted no sooner
		// than it's anticipated
		m_rx_timer->adjust(attotime::never);
		rx_start(std::max(rt1, m_rx_rt1));
	}
}


u8 m68hc05_sci_device::baud_r()
{
	return m_baud;
}

void m68hc05_sci_device::baud_w(u8 data)
{
	if (machine().side_effects_disabled())
	{
		set_baud(data);
		return;
	}

	m_baud = data & BAUD_MASK;
	update_rate();
}

u8 m68hc05_sci_device::sccr1_r()
{
	return m_sccr1;
}

void m68hc05_sci_device::sccr1_w(u8 data)
{
	// R8 is read only
	m_sccr1 = (m_sccr1 & SCCR1_R8) | (data & SCCR1_MASK & ~SCCR1_R8);
}

u8 m68hc05_sci_device::sccr2_r()
{
	return m_sccr2;
}

void m68hc05_sci_device::sccr2_w(u8 data)
{
	if (machine().side_effects_disabled())
	{
		set_sccr2(data);
		return;
	}

	u8 const rise(data & ~m_sccr2);
	u8 const fall(m_sccr2 & ~data);
	m_sccr2 = data;

	// 5.7.3 B1: with idle line wake-up, RWU can't be set while the line is idle
	if ((rise & SCCR2_RWU) && !(m_sccr1 & SCCR1_WAKE) && m_line_idle)
		m_sccr2 &= ~SCCR2_RWU;

	// 5.7.3 B2: the receiver is disabled while RE is clear; once enabled, it looks for an
	// idle line from scratch
	if (fall & SCCR2_RE)
	{
		m_rx_timer->adjust(attotime::never);
		m_rx_state = RX_IDLE;
	}
	if ((rise & SCCR2_RE) && m_rdi)
	{
		m_line_idle = false;
		arm_idle(machine().time());
	}

	// 5.7.3 B3: setting TE from a cleared state sends a preamble, after the block in
	// progress if there is one
	if (rise & SCCR2_TE)
	{
		LOGSCI("%s: transmitter enabled\n", machine().describe_context());
		m_preamble_pending = true;
	}

	// 5.7.3 B0: setting SBK sends a break, even if it's cleared again straight away
	if (rise & SCCR2_SBK)
		m_break_pending = true;

	if (rise & (SCCR2_TE | SCCR2_SBK))
		tx_kick();

	update_irq();
}

u8 m68hc05_sci_device::scsr_r()
{
	if (!machine().side_effects_disabled())
		m_scsr_seen = m_scsr;
	return m_scsr;
}

u8 m68hc05_sci_device::scdr_r()
{
	// 5.7.4: RDRF, IDLE, OR, NF and FE clear by an SCSR read followed by an SCDR read
	if (!machine().side_effects_disabled() && (m_scsr_seen & SCSR_RX_FLAGS))
	{
		clear_flags(m_scsr_seen & SCSR_RX_FLAGS);
		m_scsr_seen &= ~SCSR_RX_FLAGS;
	}
	return m_rdr;
}

void m68hc05_sci_device::scdr_w(u8 data)
{
	if (machine().side_effects_disabled())
		return;

	// 5.7.4 B7: the write always lands in TDR, and replaces a byte not yet moved to the
	// shifter, but nothing is sent unless an SCSR read with TDRE set came first and the
	// write clears TDRE.  B6: TC clears by the same sequence.
	m_tdr = data;
	u8 const clear(m_scsr_seen & m_scsr & (SCSR_TDRE | SCSR_TC));
	if (clear & SCSR_TDRE)
		m_tdr_full = true;
	else if (m_scsr & SCSR_TDRE)
		LOGSCI("%s: write SCDR %02X without an SCSR read, not sent\n", machine().describe_context(), data);
	if (clear)
	{
		clear_flags(clear);
		m_scsr_seen &= ~clear;
	}

	if (m_tdr_full && te())
		tx_kick();
}


void m68hc05_sci_device::set_baud(u8 data)
{
	m_baud = data & BAUD_MASK;
	update_rate();
}

void m68hc05_sci_device::set_sccr2(u8 data)
{
	m_sccr2 = data;
	update_irq();
}

void m68hc05_sci_device::set_scsr(u8 data)
{
	// a flag that changes here isn't covered by an earlier SCSR read
	u8 const old(m_scsr);
	m_scsr = data & SCSR_MASK;
	m_scsr_seen &= old & m_scsr;
	update_irq();
}


u32 m68hc05_sci_device::rt_ticks() const
{
	// 5.7.5: a bit time is 16 * SCP * 2^SCR E clocks, so an RT tick is SCP * 2^SCR
	return u32(SCP_DIVISOR[(m_baud >> 4) & 3]) << (m_baud & BAUD_SCR);
}

attotime m68hc05_sci_device::tick_time(u64 ticks) const
{
	return m_epoch + attotime::from_ticks(ticks, clock());
}

u64 m68hc05_sci_device::next_tick(attotime const &t, u32 period) const
{
	// the first multiple of period E clocks from m_epoch at or after t
	if (t <= m_epoch)
		return 0;
	u64 n((t - m_epoch).as_ticks(clock()));
	n = (n + period - 1) / period * period;
	while (tick_time(n) < t)
		n += period;
	return n;
}

void m68hc05_sci_device::restart_clock()
{
	m_epoch = machine().time();
	m_rt = rt_ticks();
}

void m68hc05_sci_device::update_rate(bool force)
{
	if (!force && (rt_ticks() == m_rt))
		return;
	restart_clock();
	LOGSCI("bit time %u E clocks\n", 16 * m_rt);
	if (!clock())
		return;

	// a block in progress carries on at the new rate from here; a byte being received is
	// dropped (inferred, the data sheet doesn't cover a rate change in mid-frame)
	if (m_tx_timer->enabled())
		m_tx_timer->adjust(attotime::from_ticks(16 * m_rt, clock()));
	if (m_rx_state != RX_IDLE)
	{
		m_rx_timer->adjust(attotime::never);
		m_rx_state = RX_IDLE;
	}
	if (m_rdi && m_idle_timer->enabled())
		arm_idle(machine().time());
}


void m68hc05_sci_device::set_flags(u8 mask)
{
	// a flag set by the hardware after an SCSR read isn't covered by that read
	m_scsr |= mask;
	m_scsr_seen &= ~mask;
	update_irq();
}

void m68hc05_sci_device::clear_flags(u8 mask)
{
	m_scsr &= ~mask;
	update_irq();
}

void m68hc05_sci_device::update_irq()
{
	// 5.7.3: TIE & TDRE, TCIE & TC, RIE & (RDRF | OR), ILIE & IDLE; NF and FE request
	// nothing of their own
	bool const state(
			(m_scsr & m_sccr2 & (SCSR_TDRE | SCSR_TC | SCSR_RDRF | SCSR_IDLE)) ||
			((m_scsr & SCSR_OR) && (m_sccr2 & SCCR2_RIE)));
	if (state != m_irq_state)
	{
		m_irq_state = state;
		m_irq_cb(state ? 1 : 0);
	}
}


void m68hc05_sci_device::set_tdo(int state)
{
	if (state != m_tdo)
	{
		m_tdo = state;
		m_tdo_cb(state);
	}
}

void m68hc05_sci_device::tx_kick()
{
	// something to send while the shifter is idle: start at the next bit boundary
	if ((m_tx_state == TX_IDLE) && !m_tx_timer->enabled() && clock())
	{
		attotime const now(machine().time());
		m_tx_timer->adjust(tick_time(next_tick(now, 16 * m_rt)) - now);
	}
}

TIMER_CALLBACK_MEMBER(m68hc05_sci_device::tx_tick)
{
	if (!m_tx_count)
		tx_next();

	if (m_tx_count)
	{
		set_tdo(m_tx_shift & 1);
		m_tx_shift >>= 1;
		m_tx_count--;
		m_tx_timer->adjust(attotime::from_ticks(16 * m_rt, clock()));
	}
}

void m68hc05_sci_device::tx_next()
{
	// at a bit boundary with the last block finished: pick the next one
	bool const after_break(m_tx_state == TX_BREAK);
	if (te() && m_preamble_pending)
	{
		// 5.7.3 B3: 10 or 11 ones
		LOGSCI("preamble\n");
		m_preamble_pending = false;
		m_tx_state = TX_PREAMBLE;
		m_tx_shift = 0xffff;
		m_tx_count = frame_bits();
	}
	else if (te() && (m_break_pending || (m_sccr2 & SCCR2_SBK)))
	{
		// 5.7.3 B0: 10 or 11 zeros, repeated while SBK stays set
		LOGSCI("break\n");
		m_break_pending = false;
		m_tx_state = TX_BREAK;
		m_tx_shift = 0x0000;
		m_tx_count = frame_bits();
	}
	else if (after_break)
	{
		// 5.7.3 B0: at least one high bit after a break
		m_tx_state = TX_MARK;
		m_tx_shift = 0x0001;
		m_tx_count = 1;
	}
	else if (te() && m_tdr_full)
	{
		// 5.7.1: TDR moves to the shifter, and TDRE sets (5.7.4 B7); start bit, eight
		// data bits LSB first, T8 with M set, stop bit
		LOGSCI("sending %02X\n", m_tdr);
		m_tdr_full = false;
		m_tx_state = TX_FRAME;
		if (m_sccr1 & SCCR1_M)
			m_tx_shift = (u16(m_tdr) << 1) | ((m_sccr1 & SCCR1_T8) ? 0x200 : 0x000) | 0x400;
		else
			m_tx_shift = (u16(m_tdr) << 1) | 0x200;
		m_tx_count = frame_bits();
		set_flags(SCSR_TDRE);
	}
	else
	{
		// 5.7.4 B6: TC sets at the end of a frame, preamble or break with nothing more to
		// send, or when TE is clear
		if (m_tx_state != TX_IDLE)
			set_flags(SCSR_TC);
		m_tx_state = TX_IDLE;
		m_break_pending = false;
		set_tdo(1);
	}
}


void m68hc05_sci_device::rx_start(u64 rt1)
{
	m_rx_state = RX_FRAME;
	m_rx_rt1 = rt1;
	m_rx_sample = 0;
	m_rx_votes = 0;
	m_rx_noise = false;
	m_rx_shift = 0;
	m_idle_timer->adjust(attotime::never);
	rx_schedule();
}

void m68hc05_sci_device::rx_schedule()
{
	// samples 0-3 are RT1 and the verification samples RT3, RT5 and RT7 (figure 5-2), then
	// each bit is sampled at RT8, RT9 and RT10 of its bit time (figure 5-3)
	unsigned offset;
	if (m_rx_sample < 4)
		offset = m_rx_sample * 2;
	else
		offset = ((m_rx_sample - 4) / 3) * 16 + 7 + ((m_rx_sample - 4) % 3);

	attotime const now(machine().time());
	attotime const when(tick_time(m_rx_rt1 + u64(offset) * m_rt));
	m_rx_timer->adjust((when > now) ? (when - now) : attotime::zero);
}

TIMER_CALLBACK_MEMBER(m68hc05_sci_device::rx_sample)
{
	if (m_rx_state == RX_ARTIFICIAL)
	{
		// 5.5: the anticipated start; a low line here is taken as the start bit
		if (!m_rdi)
			rx_start(m_rx_rt1);
		return;
	}

	unsigned const n(m_rx_sample);
	if (!n)
	{
		// RT1 must see the line low
		if (m_rdi)
		{
			m_rx_state = RX_IDLE;
			arm_idle(m_rdi_rise);
			return;
		}
	}
	else if (n < 4)
	{
		m_rx_votes += m_rdi;
		if (n == 3)
		{
			// 5.4: at least two of the three verification samples low, or it was noise on
			// an idle line; one high sample sets NF
			if (m_rx_votes >= 2)
			{
				m_rx_state = RX_IDLE;
				if (m_rdi)
					arm_idle(m_rdi_rise);
				return;
			}
			m_rx_noise = m_rx_votes != 0;
			m_rx_votes = 0;
		}
	}
	else
	{
		m_rx_votes += m_rdi;
		if (((n - 4) % 3) == 2)
		{
			// 5.4: the majority of three samples, NF when they don't agree
			unsigned const bit((n - 4) / 3);
			bool const high(m_rx_votes >= 2);
			if (m_rx_votes && (m_rx_votes < 3))
				m_rx_noise = true;
			m_rx_votes = 0;

			if ((bit + 1) == frame_bits())
			{
				rx_frame_done(high);
				return;
			}
			else if (bit && high)
			{
				m_rx_shift |= 1U << (bit - 1);
			}
		}
	}

	m_rx_sample++;
	rx_schedule();
}

void m68hc05_sci_device::rx_frame_done(bool stop)
{
	unsigned const bits(frame_bits());
	bool const fe(!stop);
	bool const brk(fe && !m_rx_shift);
	u64 const next_rt1(m_rx_rt1 + u64(16 * bits) * m_rt);
	m_rx_state = RX_IDLE;

	bool const address(m_rx_shift & ((m_sccr1 & SCCR1_M) ? 0x100 : 0x080));
	if (rwu())
	{
		// 5.3: a sleeping receiver sets no flags; with address mark wake-up, a frame with
		// its MSB set wakes it and is received
		if ((m_sccr1 & SCCR1_WAKE) && address)
			m_sccr2 &= ~SCCR2_RWU;
	}

	if (!rwu())
	{
		if (m_scsr & SCSR_RDRF)
		{
			// 5.7.4 B3: overrun, the new byte is lost, and FE isn't set
			LOGSCI("received %03X, overrun\n", m_rx_shift);
			set_flags(SCSR_OR);
		}
		else
		{
			LOGSCI("received %03X%s%s\n", m_rx_shift, fe ? (brk ? " (break)" : " (framing error)") : "", m_rx_noise ? " (noise)" : "");
			m_rdr = u8(m_rx_shift);
			if (m_sccr1 & SCCR1_M)
				m_sccr1 = (m_sccr1 & ~SCCR1_R8) | (BIT(m_rx_shift, 8) ? SCCR1_R8 : 0);
			m_idle_armed = true;
			set_flags(SCSR_RDRF | (m_rx_noise ? SCSR_NF : 0) | (fe ? SCSR_FE : 0));
		}
	}

	if (fe && !brk)
	{
		// 5.5: carry on as if there had been a stop bit, and look for the next start bit
		// where it's anticipated
		m_rx_state = RX_ARTIFICIAL;
		m_rx_rt1 = next_rt1;
		attotime const now(machine().time());
		attotime const when(tick_time(next_rt1));
		m_rx_timer->adjust((when > now) ? (when - now) : attotime::zero);
	}

	// a break needs the line to go high before the next start bit, which the start
	// qualifiers already require
	if (m_rdi)
		arm_idle(std::max(m_rdi_rise, tick_time(next_rt1)));
}

void m68hc05_sci_device::arm_idle(attotime const &from)
{
	if (!clock())
		return;

	attotime const now(machine().time());
	attotime const when(from + attotime::from_ticks(u64(16 * frame_bits()) * m_rt, clock()));
	m_idle_timer->adjust((when > now) ? (when - now) : attotime::zero);
}

TIMER_CALLBACK_MEMBER(m68hc05_sci_device::idle_detected)
{
	if (!m_rdi || (m_rx_state == RX_FRAME))
		return;

	m_line_idle = true;
	if (!re())
		return;

	if (rwu())
	{
		// 5.7.3 B1: idle line wake-up clears RWU, and doesn't set IDLE (5.7.4 B4)
		if (!(m_sccr1 & SCCR1_WAKE))
			m_sccr2 &= ~SCCR2_RWU;
	}
	else if (m_idle_armed)
	{
		// 5.7.4 B4: once after each RDRF
		LOGSCI("idle line\n");
		m_idle_armed = false;
		set_flags(SCSR_IDLE);
	}
}
