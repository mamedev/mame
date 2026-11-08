// license:BSD-3-Clause
// copyright-holders:Adam Di Carlo
/***************************************************************************

    Motorola M68HC05 serial communications interface (SCI)

    Section references are to the MC68HC05C4/C8 data sheet (ADI991R2).

    Implemented: 8N1 frames, the BAUD rates (5.7.5), the double buffered
    transmitter with its preamble when TE is set (5.7.3 B3), TDRE and TC
    (5.7.4 B7, B6), the receiver with RDRF, OR and FE (5.7.4), the flag
    clearing sequences and the interrupt enables in SCCR2.

    Not implemented (logged when selected): 9-bit characters (M, T8, R8),
    receiver wake-up (RWU, WAKE) and break (SBK).  IDLE never sets.  The
    receiver is a plain diserial receiver without the 16x oversampled start
    bit verification and majority vote (5.4), so NF never sets, and it
    doesn't recover from a framing error as 5.5 describes: it waits for the
    next falling edge.  TDO isn't released to high impedance when TE clears:
    it stays high.  The SCI doesn't take over PD0 and PD1: the host's port D
    is unaffected by RE and TE.

    Inferred or approximate:
    - A frame's start bit goes out a bit time after the byte moves to the
      shifter, so a byte written during a stop bit stretches that stop bit,
      and the line idles 11 bit times after a 10 bit preamble.
    - TC stays as it was while the preamble goes out; the data sheet only
      says when it sets.
    - A BAUD change in mid-frame ends the frame (diserial drops it): the
      line goes high and the frame ends a bit time later.  A byte being
      received is dropped too.  A preamble or stop bit in progress keeps its
      old length.
    - A byte written to SCDR while TE is clear is held, and goes out after
      the preamble when TE is set.
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
	device_serial_interface(mconfig, *this),
	m_tdo_cb(*this),
	m_irq_cb(*this),
	m_line_timer(nullptr),
	m_baud(0x00),
	m_sccr1(0x00),
	m_sccr2(0x00),
	m_scsr(SCSR_TDRE | SCSR_TC), m_scsr_seen(0x00),
	m_rdr(0x00),
	m_tdr(0x00),
	m_tdr_full(false),
	m_preamble_active(false),
	m_preamble_pending(false),
	m_stop_active(false),
	m_irq_state(false),
	m_divisor(0)
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
	m_line_timer = timer_alloc(FUNC(m68hc05_sci_device::line_done), this);

	save_item(NAME(m_baud));
	save_item(NAME(m_sccr1));
	save_item(NAME(m_sccr2));
	save_item(NAME(m_scsr));
	save_item(NAME(m_scsr_seen));
	save_item(NAME(m_rdr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_tdr_full));
	save_item(NAME(m_preamble_active));
	save_item(NAME(m_preamble_pending));
	save_item(NAME(m_stop_active));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_divisor));
}

void m68hc05_sci_device::device_reset()
{
	// table 3-1: SCP1:0 are cleared; 5.7.5: SCR2:0 are not affected.  The shifters stop.
	m_baud &= BAUD_SCR;
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	m_divisor = divisor();
	set_bit_time();

	// table 3-1: TE, RE, TIE, TCIE, RIE, ILIE, RWU, SBK and the receive flags are cleared,
	// TDRE and TC set; 5.7.2: R8, T8, M and WAKE are not affected
	m_sccr2 = 0x00;
	m_scsr = SCSR_TDRE | SCSR_TC;
	m_scsr_seen = 0x00;

	m_tdr_full = false;
	m_preamble_active = false;
	m_preamble_pending = false;
	m_stop_active = false;
	m_line_timer->adjust(attotime::never);
	m_tdo_cb(1);

	m_irq_state = false;
	m_irq_cb(0);
}

void m68hc05_sci_device::device_clock_changed()
{
	if (started())
		update_rate(true);
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
	u8 const rise(data & ~m_sccr1);
	if (!machine().side_effects_disabled())
	{
		if (rise & SCCR1_M)
			logerror("%s: 9-bit characters selected (SCCR1 M), not supported\n", machine().describe_context());
		if (rise & SCCR1_WAKE)
			logerror("%s: address mark wake-up selected (SCCR1 WAKE), not supported\n", machine().describe_context());
	}

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
	m_sccr2 = data;

	if (rise & SCCR2_SBK)
		logerror("%s: send break set (SCCR2 SBK), not supported\n", machine().describe_context());
	if (rise & SCCR2_RWU)
		logerror("%s: receiver wake-up set (SCCR2 RWU), not supported\n", machine().describe_context());

	// 5.7.3 B3: setting TE from a cleared state sends a preamble, after the byte in the
	// shifter if there is one
	if ((rise & SCCR2_TE) && !m_preamble_active)
	{
		LOGSCI("%s: transmitter enabled\n", machine().describe_context());
		if (tx_busy() || m_stop_active)
			m_preamble_pending = true;
		else
			start_preamble();
	}

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

	if (te() && m_tdr_full && !tx_busy() && !m_preamble_pending)
	{
		// during the last frame's stop bit, the new frame's start bit goes out a bit time
		// after this write, stretching the stop bit (inferred)
		if (m_stop_active)
		{
			m_stop_active = false;
			m_line_timer->adjust(attotime::never);
		}
		start_frame();
	}
}


void m68hc05_sci_device::rcv_complete()
{
	receive_register_extract();

	// 5.7.3 B2: the receiver flags are inhibited while RE is clear
	if (!re())
		return;

	if (!(m_scsr & SCSR_RDRF))
	{
		// FE doesn't stop the transfer (5.7.4 B1)
		m_rdr = get_received_char();
		LOGSCI("received %02X%s\n", m_rdr, is_receive_framing_error() ? " (framing error)" : "");
		set_flags(SCSR_RDRF | (is_receive_framing_error() ? SCSR_FE : 0));
	}
	else
	{
		// 5.7.4 B3: overrun, the new byte is lost
		LOGSCI("received %02X, overrun\n", get_received_char());
		set_flags(SCSR_OR);
	}
}

void m68hc05_sci_device::tra_callback()
{
	m_tdo_cb(transmit_register_get_data_bit());
}

void m68hc05_sci_device::tra_complete()
{
	// diserial calls this as the stop bit starts.  A byte waiting in TDR starts its frame
	// now, and its start bit goes out a bit time later, after the stop bit.  Otherwise the
	// line is timed to the end of the stop bit.
	if (te() && m_tdr_full && !m_preamble_pending)
		start_frame();
	else
		start_stop_bit();
}

TIMER_CALLBACK_MEMBER(m68hc05_sci_device::line_done)
{
	if (param == LINE_PREAMBLE)
		m_preamble_active = false;
	else
		m_stop_active = false;

	// 5.7.4 B6: TC sets at the end of a frame or preamble with nothing more to send, or
	// when TE is clear
	if (te() && m_preamble_pending)
		start_preamble();
	else if (te() && m_tdr_full)
		start_frame();
	else
		set_flags(SCSR_TC);
}


void m68hc05_sci_device::set_baud(u8 data)
{
	// a new rate ends a frame in progress, as a software write does
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


u32 m68hc05_sci_device::divisor() const
{
	// 5.7.5: a bit time is 16 * SCP * 2^SCR E clocks
	return (16 * SCP_DIVISOR[(m_baud >> 4) & 3]) << (m_baud & BAUD_SCR);
}

attotime m68hc05_sci_device::bit_time() const
{
	return attotime::from_ticks(m_divisor, clock());
}

void m68hc05_sci_device::update_rate(bool force)
{
	u32 const div(divisor());
	if (force || (div != m_divisor))
	{
		bool const busy(!is_transmit_register_empty());
		m_divisor = div;
		LOGSCI("bit time %u E clocks\n", m_divisor);
		set_bit_time();

		// diserial drops the frame in the shifter: release the line and end the frame a bit
		// time later (inferred, the data sheet doesn't cover a rate change in mid-frame).  The
		// line timer is idle: a preamble or stop bit only runs with the shifter empty.
		if (busy)
		{
			m_tdo_cb(1);
			start_stop_bit();
		}
	}
}

void m68hc05_sci_device::set_bit_time()
{
	set_rate(bit_time());

	// diserial shifts a bit on each rising edge of its half-rate clock, and the phase it's
	// left in decides whether a frame's start bit comes a half or a whole bit time after
	// the byte moves to the shifter; keep it a whole bit time
	tx_clock_w(1);
}

void m68hc05_sci_device::start_frame()
{
	// TDR moves to the shifter and TDRE sets again (5.7.4 B7)
	LOGSCI("sending %02X\n", m_tdr);
	m_tdr_full = false;
	transmit_register_setup(m_tdr);
	set_flags(SCSR_TDRE);
}

void m68hc05_sci_device::start_stop_bit()
{
	m_stop_active = true;
	m_line_timer->adjust(bit_time(), LINE_STOP);
}

void m68hc05_sci_device::start_preamble()
{
	// 5.7.3 B3: ten (eleven with M) bit times of idle line before the data; the line is
	// already high
	LOGSCI("preamble\n");
	m_preamble_pending = false;
	m_preamble_active = true;
	m_line_timer->adjust(bit_time() * ((m_sccr1 & SCCR1_M) ? 11 : 10), LINE_PREAMBLE);
}
