// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "h8_sci.h"

#include "h8.h"
#include "h8_intc.h"

#define LOG_REGS  (1 << 1U)  // Register writes
#define LOG_RREGS (1 << 2U)  // Register reads
#define LOG_RATE  (1 << 3U)  // Bitrate setting, beware that gk2000 changes it all the time ending up in a massive slowdown, don't leave it active
#define LOG_DATA  (1 << 4U)  // Bytes transmitted
#define LOG_CLOCK (1 << 5U)  // Clock and transmission start/stop
#define LOG_STATE (1 << 6U)  // State machine states
#define LOG_TICK  (1 << 7U)  // Clock ticks

#define VERBOSE (LOG_DATA)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(H8_SCI, h8_sci_device, "h8_sci", "H8 Serial Communications Interface")


// Clocking:
//   Async mode:
//     The circuit wants 16 events per bit.
//       * Internal clocking: the cpu clock is divided by one of (1, 4, 16, 64) from the cks field of smr
//         then by (brr+1) then by 2.
//       * External clocking: the external clock is supposed to be 16*bitrate.
//   Sync mode:
//     The circuit wants 2 events per bit, a positive and a negative edge.
//       * Internal clocking: the cpu clock is divided by one of (1, 4, 16, 64) from the cks field of smr
//         then by (brr+1) then by 2.  Events are then interpreted has been alternatively positive and
//         negative (e.g. another divide-by-two, sync-wise).
//       * External clocking: the external clock is supposed to be at bitrate, both edges are used.
//
// Synchronization:
//   Async mode:
//     Both modes use a 4-bits counter incremented on every event (16/bit).
//
//     * Transmit sets the counter to 0 at transmit start.  Output data line changes value
//       on counter == 0.  If the clock output is required, clk=1 outside of transmit,
//       clk=0 on counter==0, clk=1 on counter==8.
//
//     * Receive sets the counter to 0 when the data line initially goes down (start bit)
//       Output line is read on counter==8.  It is unknown whether the counter is reset
//       on every data line level change.
//
//   Sync mode:
//     * Transmit changes the data line on negative edges, the clock line, following positive and
//       negative edge definition, is output as long as transmit is active and is otherwise 1.
//
//     * Receive reads the data line on positive edges.
//
// Framing:
//   Async mode: 1 bit of start at 0, 7 or 8 bits of data, nothing or 1 bit of parity or 1 bit of multiprocessing, 1 or 2 bits of stop at 1.
//   Sync mode: 8 bits of data.
//
//   Multiprocessing bit is an extra bit which value can be set on transmit in bit zero of ssr.
//   On receive when zero the byte is dropped.


const char *const h8_sci_device::state_names[] = { "idle", "start", "bit", "parity", "stop", "last-tick" };

h8_sci_device::h8_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, H8_SCI, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_external_to_internal_ratio(0), m_internal_to_external_ratio(0), m_sync_timer(nullptr), m_id(0), m_eri_int(0), m_rxi_int(0), m_txi_int(0), m_tei_int(0),
	m_tx_state(0), m_rx_state(0), m_tx_bit(0), m_rx_bit(0), m_clock_state(0), m_tx_parity(0), m_rx_parity(0), m_tx_clock_counter(0), m_rx_clock_counter(0),
	m_clock_mode(INTERNAL_ASYNC), m_ext_clock_value(false), m_rx_value(true),
	m_rdr(0), m_tdr(0), m_smr(0), m_scr(0), m_ssr(0), m_brr(0), m_rsr(0), m_tsr(0), m_clock_event(0), m_divider(0)
{
	m_external_clock_period = attotime::never;
}

void h8_sci_device::do_set_external_clock_period(const attotime &period)
{
	m_external_clock_period = period;
}

void h8_sci_device::smr_w(u8 data)
{
	m_smr = data;

	LOGMASKED(LOG_REGS, "smr_w %02x %s %c%c%c%s /%d (%06x)\n", data,
			  data & SMR_CA ? "sync" : "async",
			  data & SMR_CHR ? '7' : '8',
			  data & SMR_PE ? data & SMR_OE ? 'o' : 'e' : 'n',
			  data & SMR_STOP ? '2' : '1',
			  data & SMR_MP ? " mp" : "",
			  1 << 2*(data & SMR_CKS),
			  m_cpu->pc());

	clock_update();
}

u8 h8_sci_device::smr_r()
{
	LOGMASKED(LOG_RREGS, "smr_r %02x (%06x)\n", m_smr, m_cpu->pc());
	return m_smr;
}

void h8_sci_device::brr_w(u8 data)
{
	m_brr = data;
	LOGMASKED(LOG_REGS, "brr_w %02x (%06x)\n", m_brr, m_cpu->pc());
	clock_update();
}

u8 h8_sci_device::brr_r()
{
	LOGMASKED(LOG_RREGS, "brr_r %02x (%06x)\n", m_brr, m_cpu->pc());
	return m_brr;
}

bool h8_sci_device::is_sync_start() const
{
	return (m_smr & SMR_CA) && ((m_scr & (SCR_TE|SCR_RE)) == (SCR_TE|SCR_RE));
}

bool h8_sci_device::has_recv_error() const
{
	return m_ssr & (SSR_ORER|SSR_PER|SSR_FER);
}

void h8_sci_device::scr_w(u8 data)
{
	LOGMASKED(LOG_REGS, "scr_w %02x%s%s%s%s%s%s clk=%d (%06x)\n", data,
			  data & SCR_TIE  ? " txi" : "",
			  data & SCR_RIE  ? " rxi" : "",
			  data & SCR_TE   ? " tx" : "",
			  data & SCR_RE   ? " rx" : "",
			  data & SCR_MPIE ? " mpi" : "",
			  data & SCR_TEIE ? " tei" : "",
			  data & SCR_CKE,
			  m_cpu->pc());

	u8 delta = m_scr ^ data;
	m_scr = data;
	clock_update();

	if((delta & SCR_RE) && !(m_scr & SCR_RE)) {
		m_rx_state = ST_IDLE;
		clock_stop(CLK_RX);
	}

	if((delta & SCR_RE) && (m_scr & SCR_RE) && m_rx_state == ST_IDLE && !has_recv_error() && !is_sync_start())
		rx_start();
	if((delta & SCR_TIE) && (m_scr & SCR_TIE) && (m_ssr & SSR_TDRE))
		m_intc->internal_interrupt(m_txi_int);
	if((delta & SCR_TEIE) && (m_scr & SCR_TEIE) && (m_ssr & SSR_TEND))
		m_intc->internal_interrupt(m_tei_int);
	if((delta & SCR_RIE) && (m_scr & SCR_RIE) && (m_ssr & SSR_RDRF))
		m_intc->internal_interrupt(m_rxi_int);
	if((delta & SCR_RIE) && (m_scr & SCR_RIE) && has_recv_error())
		m_intc->internal_interrupt(m_eri_int);
}

u8 h8_sci_device::scr_r()
{
	LOGMASKED(LOG_RREGS, "scr_r %02x (%06x)\n", m_scr, m_cpu->pc());
	return m_scr;
}

void h8_sci_device::tdr_w(u8 data)
{
	LOGMASKED(LOG_REGS, "tdr_w %02x (%06x)\n", data, m_cpu->pc());
	m_tdr = data;
	if(m_cpu->access_is_dma()) {
		m_ssr &= ~SSR_TDRE;
		if(m_tx_state == ST_IDLE)
			tx_start();
	}
}

u8 h8_sci_device::tdr_r()
{
	LOGMASKED(LOG_RREGS, "tdr_r %02x (%06x)\n", m_tdr, m_cpu->pc());
	return m_tdr;
}

void h8_sci_device::ssr_w(u8 data)
{
	if(!(m_scr & SCR_TE)) {
		data |= SSR_TDRE;
		m_ssr |= SSR_TDRE;
	}
	if((m_ssr & SSR_TDRE) && !(data & SSR_TDRE))
		m_ssr &= ~SSR_TEND;
	m_ssr = ((m_ssr & ~SSR_MPBT) | (data & SSR_MPBT)) & (data | (SSR_TEND|SSR_MPB|SSR_MPBT));
	LOGMASKED(LOG_REGS, "ssr_w %02x -> %02x (%06x)\n", data, m_ssr, m_cpu->pc());

	if(m_tx_state == ST_IDLE && !(m_ssr & SSR_TDRE))
		tx_start();

	if((m_scr & SCR_RE) && m_rx_state == ST_IDLE && !has_recv_error() && !is_sync_start())
		rx_start();
}

u8 h8_sci_device::ssr_r()
{
	LOGMASKED(LOG_RREGS, "ssr_r %02x (%06x)\n", m_ssr, m_cpu->pc());
	return m_ssr;
}

u8 h8_sci_device::rdr_r()
{
	LOGMASKED(LOG_RREGS, "rdr_r %02x (%06x)\n", m_rdr, m_cpu->pc());

	if(!machine().side_effects_disabled() && m_cpu->access_is_dma())
		m_ssr &= ~SSR_RDRF;
	return m_rdr;
}

void h8_sci_device::scmr_w(u8 data)
{
	LOGMASKED(LOG_REGS, "scmr_w %02x (%06x)\n", data, m_cpu->pc());
}

u8 h8_sci_device::scmr_r()
{
	LOGMASKED(LOG_RREGS, "scmr_r (%06x)\n", m_cpu->pc());
	return 0x00;
}

void h8_sci_device::clock_update()
{
	m_divider = 2 << (2*(m_smr & SMR_CKS));
	m_divider *= m_brr+1;

	if(m_smr & SMR_CA) {
		if(m_scr & SCR_CKE1)
			m_clock_mode = EXTERNAL_SYNC;
		else
			m_clock_mode = INTERNAL_SYNC_OUT;
	} else {
		if(m_scr & SCR_CKE1)
			m_clock_mode = EXTERNAL_ASYNC;
		else if(m_scr & SCR_CKE0)
			m_clock_mode = INTERNAL_ASYNC_OUT;
		else
			m_clock_mode = INTERNAL_ASYNC;
	}

	if(m_clock_mode == EXTERNAL_ASYNC && !m_external_clock_period.is_never())
		m_clock_mode = EXTERNAL_RATE_ASYNC;
	if(m_clock_mode == EXTERNAL_SYNC && !m_external_clock_period.is_never())
		m_clock_mode = EXTERNAL_RATE_SYNC;

	if(VERBOSE & LOG_RATE) {
		std::string new_message;
		switch(m_clock_mode) {
		case INTERNAL_ASYNC:
			new_message = util::string_format("clock internal at %d Hz, async, bitrate %d bps\n", int(m_cpu->system_clock() / m_divider), int(m_cpu->system_clock() / (m_divider*16)));
			break;
		case INTERNAL_ASYNC_OUT:
			new_message = util::string_format("clock internal at %d Hz, async, bitrate %d bps, output\n", int(m_cpu->system_clock() / m_divider), int(m_cpu->system_clock() / (m_divider*16)));
			break;
		case EXTERNAL_ASYNC:
			new_message = "clock external, async\n";
			break;
		case EXTERNAL_RATE_ASYNC:
			new_message = util::string_format("clock external at %d Hz, async, bitrate %d bps\n", int(m_cpu->system_clock()*m_internal_to_external_ratio), int(m_cpu->system_clock()*m_internal_to_external_ratio/16));
			break;
		case INTERNAL_SYNC_OUT:
			new_message = util::string_format("clock internal at %d Hz, sync, output\n", int(m_cpu->system_clock() / (m_divider*2)));
			break;
		case EXTERNAL_SYNC:
			new_message = "clock external, sync\n";
			break;
		case EXTERNAL_RATE_SYNC:
			new_message = util::string_format("clock external at %d Hz, sync\n", int(m_cpu->system_clock()*m_internal_to_external_ratio));
			break;
		}
		if(new_message != m_last_clock_message) {
			(LOG_OUTPUT_FUNC)(new_message);
			m_last_clock_message = std::move(new_message);
		}
	}
}

void h8_sci_device::device_start()
{
	m_sync_timer = timer_alloc(FUNC(h8_sci_device::sync_tick), this);

	if(m_external_clock_period.is_never()) {
		m_internal_to_external_ratio = 0;
		m_external_to_internal_ratio = 0;
	} else {
		m_external_to_internal_ratio = (m_external_clock_period*m_cpu->system_clock()).as_double();
		m_internal_to_external_ratio = 1/m_external_to_internal_ratio;
	}

	save_item(NAME(m_tx_state));
	save_item(NAME(m_rx_state));
	save_item(NAME(m_tx_bit));
	save_item(NAME(m_rx_bit));
	save_item(NAME(m_clock_state));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_rx_parity));
	save_item(NAME(m_tx_clock_counter));
	save_item(NAME(m_rx_clock_counter));
	save_item(NAME(m_clock_mode));
	save_item(NAME(m_ext_clock_value));
	save_item(NAME(m_rx_value));

	save_item(NAME(m_rdr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_smr));
	save_item(NAME(m_scr));
	save_item(NAME(m_ssr));
	save_item(NAME(m_brr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_clock_event));
	save_item(NAME(m_clock_step));
	save_item(NAME(m_divider));
}

void h8_sci_device::device_reset()
{
	m_rdr = 0x00;
	m_tdr = 0xff;
	m_smr = 0x00;
	m_scr = 0x00;
	m_ssr = 0x84;
	m_brr = 0xff;
	m_rsr = 0x00;
	m_tsr = 0xff;
	m_rx_bit = 0;
	m_tx_bit = 0;
	m_tx_state = ST_IDLE;
	m_rx_state = ST_IDLE;
	m_clock_state = 0;
	m_clock_mode = INTERNAL_ASYNC;
	m_clock_event = 0;
	clock_update();
	m_ext_clock_value = true;
	m_tx_clock_counter = 0;
	m_rx_clock_counter = 0;
	m_cpu->do_sci_clk(m_id, 1);
	m_cpu->do_sci_tx(m_id, 1);
}

TIMER_CALLBACK_MEMBER(h8_sci_device::sync_tick)
{
	// Used only to force system-wide syncs
}

void h8_sci_device::do_rx_w(int state)
{
	if(m_cpu->standby()) {
		m_rx_value = state;
		return;
	}

	if(state != m_rx_value && (m_clock_state & CLK_RX))
		if(m_rx_clock_counter == 1 || m_rx_clock_counter == 15)
			m_rx_clock_counter = 0;

	m_rx_value = state;
	if(!m_rx_value && !(m_clock_state & CLK_RX) && m_rx_state != ST_IDLE)
		clock_start(CLK_RX);
}

void h8_sci_device::do_clk_w(int state)
{
	if(m_ext_clock_value == state)
		return;

	m_ext_clock_value = state;
	if(!m_clock_state || m_cpu->standby())
		return;

	if(m_clock_mode == EXTERNAL_ASYNC) {
		if(m_clock_state & CLK_TX)
			tx_async_tick();
		if(m_clock_state & CLK_RX)
			rx_async_tick();
	} else if(m_clock_mode == EXTERNAL_SYNC) {
		if(m_clock_state & CLK_TX)
			tx_sync_tick();
		if(m_clock_state & CLK_RX)
			rx_sync_tick();
	}
}

u64 h8_sci_device::internal_update(u64 current_time)
{
	if(!m_clock_event || current_time < m_clock_event)
		return m_clock_event;

	if(m_clock_mode == INTERNAL_ASYNC || m_clock_mode == INTERNAL_ASYNC_OUT || m_clock_mode == EXTERNAL_RATE_ASYNC) {
		if(m_clock_state & CLK_TX)
			tx_async_tick();
		if(m_clock_state & CLK_RX)
			rx_async_tick();
	} else if(m_clock_mode == INTERNAL_SYNC_OUT || m_clock_mode == EXTERNAL_RATE_SYNC) {
		if(m_clock_state & CLK_TX)
			tx_sync_tick();
		if(m_clock_state & CLK_RX)
			rx_sync_tick();
	}

	if(m_clock_state) {
		if(m_clock_step)
			m_clock_event += m_clock_step;
		else if(m_clock_mode == EXTERNAL_RATE_ASYNC || m_clock_mode == EXTERNAL_RATE_SYNC)
			m_clock_event = u64(u64(m_clock_event * m_internal_to_external_ratio + 1) * m_external_to_internal_ratio + 1);
		else
			m_clock_event = 0;

		if(m_clock_event) {
			if(s64 ticks = m_clock_event - m_cpu->now_as_cycles(); ticks >= 0LL)
				m_sync_timer->adjust(attotime::from_ticks(ticks, m_cpu->system_clock()));
			m_cpu->internal_update();
		}

	} else if(!m_clock_state) {
		m_clock_event = 0;
		if(m_clock_mode == INTERNAL_ASYNC_OUT || m_clock_mode == INTERNAL_SYNC_OUT)
			m_cpu->do_sci_clk(m_id, 1);
	}

	return m_clock_event;
}

void h8_sci_device::notify_standby(int state)
{
	if(!state && m_clock_event)
		m_clock_event += m_cpu->total_cycles() - m_cpu->standby_time();
}

void h8_sci_device::clock_start(int mode)
{
	// Happens when back-to-back
	if(m_clock_state & mode)
		return;

	if(mode == CLK_TX)
		m_tx_clock_counter = 15;
	else
		m_rx_clock_counter = 15;

	m_clock_state |= mode;
	if(m_clock_state != mode)
		return;

	m_clock_step = 0;

	switch(m_clock_mode) {
	case INTERNAL_ASYNC:
	case INTERNAL_ASYNC_OUT:
	case INTERNAL_SYNC_OUT: {
		LOGMASKED(LOG_CLOCK, "Starting internal clock\n");
		m_clock_step = m_divider;
		u64 now = mode == CLK_TX ? m_cpu->total_cycles() : m_cpu->now_as_cycles();
		m_clock_event = (now / m_clock_step + 1) * m_clock_step;
		m_sync_timer->adjust(attotime::from_ticks(m_clock_event - now, m_cpu->system_clock()));
		m_cpu->internal_update();
		break;
	}

	case EXTERNAL_RATE_ASYNC:
	case EXTERNAL_RATE_SYNC: {
		LOGMASKED(LOG_CLOCK, "Simulating external clock\n", m_clock_mode == EXTERNAL_RATE_ASYNC ? "async" : "sync");
		u64 now = mode == CLK_TX ? m_cpu->total_cycles() : m_cpu->now_as_cycles();
		m_clock_event = u64(u64(now * m_internal_to_external_ratio + 1) * m_external_to_internal_ratio + 1);
		m_sync_timer->adjust(attotime::from_ticks(m_clock_event - now, m_cpu->system_clock()));
		m_cpu->internal_update();
		break;
	}

	case EXTERNAL_ASYNC:
	case EXTERNAL_SYNC:
		LOGMASKED(LOG_CLOCK, "Waiting for external clock\n");
		break;
	}
}

void h8_sci_device::clock_stop(int mode)
{
	m_clock_state &= ~mode;
	if(!m_clock_state) {
		m_clock_event = 0;
		m_clock_step = 0;
		LOGMASKED(LOG_CLOCK, "Stopping clocks\n");
	}
	m_cpu->internal_update();
}

void h8_sci_device::tx_start()
{
	m_ssr |= SSR_TDRE;
	m_tsr = m_tdr;
	m_tx_parity = m_smr & SMR_OE ? 0 : 1;
	LOGMASKED(LOG_DATA, "start transmit %02x '%c'\n", m_tsr, m_tsr >= 32 && m_tsr < 127 ? m_tsr : '.');
	if(m_scr & SCR_TIE)
		m_intc->internal_interrupt(m_txi_int);
	if(m_smr & SMR_CA) {
		m_tx_state = ST_BIT;
		m_tx_bit = 8;
	} else {
		m_tx_state = ST_START;
		m_tx_bit = 1;
	}
	clock_start(CLK_TX);
	if(m_rx_state == ST_IDLE && !has_recv_error() && is_sync_start())
		rx_start();
}

void h8_sci_device::tx_async_tick()
{
	m_tx_clock_counter = (m_tx_clock_counter + 1) & 15;
	LOGMASKED(LOG_TICK, "tx_async_tick %x\n", m_tx_clock_counter);
	if(m_tx_clock_counter == 0) {
		tx_async_step();

		if(m_clock_mode == INTERNAL_ASYNC_OUT)
			m_cpu->do_sci_clk(m_id, 0);

	} else if(m_tx_clock_counter == 8 && m_clock_mode == INTERNAL_ASYNC_OUT)
		m_cpu->do_sci_clk(m_id, 1);
}

void h8_sci_device::tx_async_step()
{
	LOGMASKED(LOG_STATE, "tx_async_step state=%s bit=%d\n", state_names[m_tx_state], m_tx_bit);
	switch(m_tx_state) {
	case ST_START:
		m_cpu->do_sci_tx(m_id, false);
		assert(m_tx_bit == 1);
		m_tx_state = ST_BIT;
		m_tx_bit = m_smr & SMR_CHR ? 7 : 8;
		break;

	case ST_BIT:
		m_tx_parity ^= (m_tsr & 1);
		m_cpu->do_sci_tx(m_id, m_tsr & 1);
		m_tsr >>= 1;
		m_tx_bit--;
		if(!m_tx_bit) {
			if(m_smr & SMR_CA) {
				if(!(m_ssr & SSR_TDRE))
					tx_start();
				else {
					m_tx_state = ST_LAST_TICK;
					m_tx_bit = 0;
				}
			} else if(m_smr & SMR_PE) {
				m_tx_state = ST_PARITY;
				m_tx_bit = 1;
			} else {
				m_tx_state = ST_STOP;
				m_tx_bit = m_smr & SMR_STOP ? 2 : 1;
			}
		}
		break;

	case ST_PARITY:
		m_cpu->do_sci_tx(m_id, m_tx_parity);
		assert(m_tx_bit == 1);
		m_tx_state = ST_STOP;
		m_tx_bit = m_smr & SMR_STOP ? 2 : 1;
		break;

	case ST_STOP:
		m_cpu->do_sci_tx(m_id, true);
		m_tx_bit--;
		if(!m_tx_bit) {
			if(!(m_ssr & SSR_TDRE))
				tx_start();
			else {
				m_tx_state = ST_LAST_TICK;
				m_tx_bit = 0;
			}
		}
		break;

	case ST_LAST_TICK:
		m_tx_state = ST_IDLE;
		m_tx_bit = 0;
		clock_stop(CLK_TX);
		m_cpu->do_sci_tx(m_id, 1);
		m_ssr |= SSR_TEND;
		if(m_scr & SCR_TEIE)
			m_intc->internal_interrupt(m_tei_int);

		// if there's more to send, start the transmitter
		if((m_scr & SCR_TE) && !(m_ssr & SSR_TDRE))
			tx_start();
		break;

	default:
		abort();
	}
	LOGMASKED(LOG_STATE, "            -> state=%s bit=%d\n", state_names[m_tx_state], m_tx_bit);
}

void h8_sci_device::tx_sync_tick()
{
	m_tx_clock_counter = (m_tx_clock_counter + 1) & 1;
	LOGMASKED(LOG_TICK, "tx_sync_tick %x\n", m_tx_clock_counter);
	if(m_tx_clock_counter == 0) {
		tx_sync_step();

		if(m_clock_mode == INTERNAL_SYNC_OUT && m_tx_state != ST_IDLE)
			m_cpu->do_sci_clk(m_id, 0);

	} else if(m_tx_clock_counter == 1 && m_clock_mode == INTERNAL_SYNC_OUT)
		m_cpu->do_sci_clk(m_id, 1);
}

void h8_sci_device::tx_sync_step()
{
	LOGMASKED(LOG_STATE, "tx_sync_step bit=%d\n", m_tx_bit);
	if(!m_tx_bit) {
		m_tx_state = ST_IDLE;
		clock_stop(CLK_TX);
		m_cpu->do_sci_tx(m_id, 1);
		m_ssr |= SSR_TEND;
		if(m_scr & SCR_TEIE)
			m_intc->internal_interrupt(m_tei_int);

		// if there's more to send, start the transmitter
		if((m_scr & SCR_TE) && !(m_ssr & SSR_TDRE))
			tx_start();
	} else {
		m_cpu->do_sci_tx(m_id, m_tsr & 1);
		m_tsr >>= 1;
		m_tx_bit--;
	}
}

void h8_sci_device::rx_start()
{
	m_rx_parity = m_smr & SMR_OE ? 0 : 1;
	m_rsr = 0x00;
	LOGMASKED(LOG_STATE, "start receive\n");
	if(m_smr & SMR_CA) {
		m_rx_state = ST_BIT;
		m_rx_bit = 8;
		clock_start(CLK_RX);
	} else {
		m_rx_state = ST_START;
		m_rx_bit = 1;
		if(!m_rx_value)
			clock_start(CLK_RX);
	}
}

void h8_sci_device::rx_done()
{
	if(!(m_ssr & SSR_FER)) {
		if((m_smr & SMR_PE) && m_rx_parity) {
			m_ssr |= SSR_PER;
			LOGMASKED(LOG_DATA, "Receive parity error\n");
		} else if(m_ssr & SSR_RDRF) {
			m_ssr |= SSR_ORER;
			LOGMASKED(LOG_DATA, "Receive overrun\n");
		} else {
			m_ssr |= SSR_RDRF;
			LOGMASKED(LOG_DATA, "Received %02x '%c'\n", m_rsr, m_rsr >= 32 && m_rsr < 127 ? m_rsr : '.');
			m_rdr = m_rsr;
		}
	}
	if(m_scr & SCR_RIE) {
		if(has_recv_error())
			m_intc->internal_interrupt(m_eri_int);
		else
			m_intc->internal_interrupt(m_rxi_int);
	}
	if((m_scr & SCR_RE) && !has_recv_error() && !is_sync_start())
		rx_start();
	else {
		clock_stop(CLK_RX);
		m_rx_state = ST_IDLE;
	}
}

void h8_sci_device::rx_async_tick()
{
	m_rx_clock_counter = (m_rx_clock_counter + 1) & 15;
	LOGMASKED(LOG_TICK, "rx_async_tick %x\n", m_rx_clock_counter);
	if(m_rx_clock_counter == 8)
		rx_async_step();
}

void h8_sci_device::rx_async_step()
{
	LOGMASKED(LOG_STATE, "rx_async_step state=%s bit=%d\n", state_names[m_rx_state], m_rx_bit);
	switch(m_rx_state) {
	case ST_START:
		if(m_rx_value) {
			clock_stop(CLK_RX);
			break;
		}
		m_rx_state = ST_BIT;
		m_rx_bit = m_smr & SMR_CHR ? 7 : 8;
		break;

	case ST_BIT:
		m_rx_parity ^= m_rx_value;
		m_rsr >>= 1;
		if(m_rx_value) {
			m_rx_parity = !m_rx_parity;
			m_rsr |= (m_smr & (SMR_CA|SMR_CHR)) == SMR_CHR ? 0x40 : 0x80;
		}
		m_rx_bit--;
		if(!m_rx_bit) {
			if(m_smr & SMR_CA)
				rx_done();
			else if(m_smr & SMR_PE) {
				m_rx_state = ST_PARITY;
				m_rx_bit = 1;
			} else {
				m_rx_state = ST_STOP;
				m_rx_bit = 1; // Always 1 on rx
			}
		}
		break;

	case ST_PARITY:
		m_rx_parity ^= m_rx_value;
		assert(m_rx_bit == 1);
		m_rx_state = ST_STOP;
		m_rx_bit = 1;
		break;

	case ST_STOP:
		assert(m_rx_bit == 1);
		if(!m_rx_value)
			m_ssr |= SSR_FER;
		else if((m_smr & SMR_PE) && m_rx_parity)
			m_ssr |= SSR_PER;
		rx_done();
		break;

	default:
		abort();
	}
	LOGMASKED(LOG_STATE, "            -> state=%s, bit=%d\n", state_names[m_rx_state], m_rx_bit);
}

void h8_sci_device::rx_sync_tick()
{
	m_rx_clock_counter = (m_rx_clock_counter + 1) & 1;
	LOGMASKED(LOG_TICK, "rx_sync_tick %x\n", m_rx_clock_counter);

	if(m_rx_clock_counter == 0 && m_clock_mode == INTERNAL_SYNC_OUT)
		m_cpu->do_sci_clk(m_id, 0);

	else if(m_rx_clock_counter == 1) {
		if(m_clock_mode == INTERNAL_SYNC_OUT)
			m_cpu->do_sci_clk(m_id, 1);

		rx_sync_step();
	}
}

void h8_sci_device::rx_sync_step()
{
	LOGMASKED(LOG_STATE, "rx_sync_step bit=%d\n", m_rx_value);
	m_rsr >>= 1;
	if(m_rx_value)
		m_rsr |= 0x80;
	m_rx_bit--;

	if(!m_rx_bit)
		rx_done();
}
