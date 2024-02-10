// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "h8_sci.h"

#include "h8.h"
#include "h8_intc.h"

// Verbosity level
// 0 = no messages
// 1 = transmitted/recieved bytes, reception errors and clock setup
// 2 = everything but status register reads
// 3 = everything
static constexpr int V = 1;


DEFINE_DEVICE_TYPE(H8_SCI, h8_sci_device, "h8_sci", "H8 Serial Communications Interface")

const char *const h8_sci_device::state_names[] = { "idle", "start", "bit", "parity", "stop", "last-tick" };

h8_sci_device::h8_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, H8_SCI, tag, owner, clock),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_external_to_internal_ratio(0), m_internal_to_external_ratio(0), m_sync_timer(nullptr), m_id(0), m_eri_int(0), m_rxi_int(0), m_txi_int(0), m_tei_int(0),
	m_tx_state(0), m_rx_state(0), m_tx_bit(0), m_rx_bit(0), m_clock_state(0), m_tx_parity(0), m_rx_parity(0), m_ext_clock_counter(0), m_clock_mode(clock_mode_t::INTERNAL_ASYNC), m_clock_value(false), m_ext_clock_value(false), m_rx_value(false),
	m_rdr(0), m_tdr(0), m_smr(0), m_scr(0), m_ssr(0), m_brr(0), m_rsr(0), m_tsr(0), m_clock_base(0), m_divider(0)
{
	m_external_clock_period = attotime::never;
}

void h8_sci_device::do_set_external_clock_period(const attotime &period)
{
	m_external_clock_period = period;
}

void h8_sci_device::smr_w(uint8_t data)
{
	m_smr = data;
	if(V>=2) logerror("smr_w %02x %s %c%c%c%s /%d (%06x)\n", data,
						data & SMR_CA ? "sync" : "async",
						data & SMR_CHR ? '7' : '8',
						data & SMR_PE ? data & SMR_OE ? 'o' : 'e' : 'n',
						data & SMR_STOP ? '2' : '1',
						data & SMR_MP ? " mp" : "",
						1 << 2*(data & SMR_CKS),
						m_cpu->pc());
	clock_update();
}

uint8_t h8_sci_device::smr_r()
{
	if(V>=2) logerror("smr_r %02x (%06x)\n", m_smr, m_cpu->pc());
	return m_smr;
}

void h8_sci_device::brr_w(uint8_t data)
{
	m_brr = data;
	if(V>=2) logerror("brr_w %02x (%06x)\n", data, m_cpu->pc());
	clock_update();
}

uint8_t h8_sci_device::brr_r()
{
	if(V>=2) logerror("brr_r %02x (%06x)\n", m_brr, m_cpu->pc());
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

void h8_sci_device::scr_w(uint8_t data)
{
	if(V>=2) logerror("scr_w %02x%s%s%s%s%s%s clk=%d (%06x)\n", data,
						data & SCR_TIE  ? " txi" : "",
						data & SCR_RIE  ? " rxi" : "",
						data & SCR_TE   ? " tx" : "",
						data & SCR_RE   ? " rx" : "",
						data & SCR_MPIE ? " mpi" : "",
						data & SCR_TEIE ? " tei" : "",
						data & SCR_CKE,
						m_cpu->pc());

	uint8_t delta = m_scr ^ data;
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

uint8_t h8_sci_device::scr_r()
{
	if(V>=2) logerror("scr_r %02x (%06x)\n", m_scr, m_cpu->pc());
	return m_scr;
}

void h8_sci_device::tdr_w(uint8_t data)
{
	if(V>=2) logerror("tdr_w %02x (%06x)\n", data, m_cpu->pc());
	m_tdr = data;
	if(m_cpu->access_is_dma()) {
		m_ssr &= ~SSR_TDRE;
		if(m_tx_state == ST_IDLE)
			tx_start();
	}
}

uint8_t h8_sci_device::tdr_r()
{
	if(V>=2) logerror("tdr_r %02x (%06x)\n", m_tdr, m_cpu->pc());
	return m_tdr;
}

void h8_sci_device::ssr_w(uint8_t data)
{
	if(!(m_scr & SCR_TE)) {
		data |= SSR_TDRE;
		m_ssr |= SSR_TDRE;
	}
	if((m_ssr & SSR_TDRE) && !(data & SSR_TDRE))
		m_ssr &= ~SSR_TEND;
	m_ssr = ((m_ssr & ~SSR_MPBT) | (data & SSR_MPBT)) & (data | (SSR_TEND|SSR_MPB|SSR_MPBT));
	if(V>=2) logerror("ssr_w %02x -> %02x (%06x)\n", data, m_ssr, m_cpu->pc());

	if(m_tx_state == ST_IDLE && !(m_ssr & SSR_TDRE))
		tx_start();

	if((m_scr & SCR_RE) && m_rx_state == ST_IDLE && !has_recv_error() && !is_sync_start())
		rx_start();
}

uint8_t h8_sci_device::ssr_r()
{
	if(V>=3) logerror("ssr_r %02x (%06x)\n", m_ssr, m_cpu->pc());
	return m_ssr;
}

uint8_t h8_sci_device::rdr_r()
{
	if(V>=2) logerror("rdr_r %02x (%06x)\n", m_rdr, m_cpu->pc());
	if(m_cpu->access_is_dma())
		m_ssr &= ~SSR_RDRF;
	return m_rdr;
}

void h8_sci_device::scmr_w(uint8_t data)
{
	if(V>=2) logerror("scmr_w %02x (%06x)\n", data, m_cpu->pc());
}

uint8_t h8_sci_device::scmr_r()
{
	if(V>=2) logerror("scmr_r (%06x)\n", m_cpu->pc());
	return 0x00;
}

void h8_sci_device::clock_update()
{
	// Sync: Divider must be the time of a half-period (both edges are used, datarate*2)
	// Async: Divider must be the time of one period (only raising edge used, datarate*16)

	m_divider = 2 << (2*(m_smr & SMR_CKS));
	m_divider *= m_brr+1;

	if(m_smr & SMR_CA) {
		if(m_scr & SCR_CKE1)
			m_clock_mode = clock_mode_t::EXTERNAL_SYNC;
		else
			m_clock_mode = clock_mode_t::INTERNAL_SYNC_OUT;
	} else {
		if(m_scr & SCR_CKE1)
			m_clock_mode = clock_mode_t::EXTERNAL_ASYNC;
		else if(m_scr & SCR_CKE0)
			m_clock_mode = clock_mode_t::INTERNAL_ASYNC_OUT;
		else
			m_clock_mode = clock_mode_t::INTERNAL_ASYNC;
	}

	if(m_clock_mode == clock_mode_t::EXTERNAL_ASYNC && !m_external_clock_period.is_never())
		m_clock_mode = clock_mode_t::EXTERNAL_RATE_ASYNC;
	if(m_clock_mode == clock_mode_t::EXTERNAL_SYNC && !m_external_clock_period.is_never())
		m_clock_mode = clock_mode_t::EXTERNAL_RATE_SYNC;

	if(V>=1) {
		std::string new_message;
		switch(m_clock_mode) {
		case clock_mode_t::INTERNAL_ASYNC:
			new_message = util::string_format("clock internal at %d Hz, async, bitrate %d bps\n", int(m_cpu->clock() / m_divider), int(m_cpu->clock() / (m_divider*16)));
			break;
		case clock_mode_t::INTERNAL_ASYNC_OUT:
			new_message = util::string_format("clock internal at %d Hz, async, bitrate %d bps, output\n", int(m_cpu->clock() / m_divider), int(m_cpu->clock() / (m_divider*16)));
			break;
		case clock_mode_t::EXTERNAL_ASYNC:
			new_message = "clock external, async\n";
			break;
		case clock_mode_t::EXTERNAL_RATE_ASYNC:
			new_message = util::string_format("clock external at %d Hz, async, bitrate %d bps\n", int(m_cpu->clock()*m_internal_to_external_ratio), int(m_cpu->clock()*m_internal_to_external_ratio/16));
			break;
		case clock_mode_t::INTERNAL_SYNC_OUT:
			new_message = util::string_format("clock internal at %d Hz, sync, output\n", int(m_cpu->clock() / (m_divider*2)));
			break;
		case clock_mode_t::EXTERNAL_SYNC:
			new_message = "clock external, sync\n";
			break;
		case clock_mode_t::EXTERNAL_RATE_SYNC:
			new_message = util::string_format("clock external at %d Hz, sync\n", int(m_cpu->clock()*m_internal_to_external_ratio));
			break;
		}
		if(new_message != m_last_clock_message) {
			logerror(new_message);
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
		m_external_to_internal_ratio = (m_external_clock_period*m_cpu->clock()).as_double();
		m_internal_to_external_ratio = 1/m_external_to_internal_ratio;
	}


	save_item(NAME(m_rdr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_smr));
	save_item(NAME(m_scr));
	save_item(NAME(m_ssr));
	save_item(NAME(m_brr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_tsr));
	save_item(NAME(m_rx_bit));
	save_item(NAME(m_tx_bit));
	save_item(NAME(m_rx_state));
	save_item(NAME(m_tx_state));
	save_item(NAME(m_tx_parity));
	save_item(NAME(m_clock_state));
	save_item(NAME(m_clock_value));
	save_item(NAME(m_clock_base));
	save_item(NAME(m_divider));
	save_item(NAME(m_ext_clock_value));
	save_item(NAME(m_ext_clock_counter));
	save_item(NAME(m_cur_sync_time));
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
	m_clock_mode = clock_mode_t::INTERNAL_ASYNC;
	m_clock_base = 0;
	clock_update();
	m_clock_value = true;
	m_ext_clock_value = true;
	m_ext_clock_counter = 0;
	m_rx_value = true;
	m_cpu->do_sci_clk(m_id, m_clock_value);
	m_cpu->do_sci_tx(m_id, 1);
	m_cur_sync_time = attotime::never;
}

void h8_sci_device::device_post_load()
{
	// Set clock_mode correctly as it's not saved
	clock_update();
}

TIMER_CALLBACK_MEMBER(h8_sci_device::sync_tick)
{
	// Used only to force system-wide syncs
}

void h8_sci_device::do_rx_w(int state)
{
	m_rx_value = state;
	if(V>=2) logerror("rx=%d\n", state);
	if(!m_rx_value && !(m_clock_state & CLK_RX) && m_rx_state != ST_IDLE && !m_cpu->standby())
		clock_start(CLK_RX);
}

void h8_sci_device::do_clk_w(int state)
{
	if(m_ext_clock_value != state) {
		m_ext_clock_value = state;
		if(m_clock_state && !m_cpu->standby()) {
			switch(m_clock_mode) {
			case clock_mode_t::EXTERNAL_ASYNC:
				if(m_ext_clock_value) {
					m_ext_clock_counter = (m_ext_clock_counter+1) & 15;

					if((m_clock_state & CLK_TX) && m_ext_clock_counter == 0)
						tx_dropped_edge();
					if((m_clock_state & CLK_RX) && m_ext_clock_counter == 8)
						rx_raised_edge();
				}
				break;

			case clock_mode_t::EXTERNAL_SYNC:
				if((!m_ext_clock_value) && (m_clock_state & CLK_TX))
					tx_dropped_edge();

				else if(m_ext_clock_value && (m_clock_state & CLK_RX))
					rx_raised_edge();
				break;
			default:
				// Do nothing
				break;
			}
		}
	}
}

uint64_t h8_sci_device::internal_update(uint64_t current_time)
{
	uint64_t event = 0;
	switch(m_clock_mode) {
	case clock_mode_t::INTERNAL_SYNC_OUT:
		if(m_clock_state || !m_clock_value) {
			uint64_t fp = m_divider*2;
			if(current_time >= m_clock_base) {
				uint64_t delta = current_time - m_clock_base;
				if(delta >= fp) {
					delta -= fp;
					m_clock_base += fp;
				}
				assert(delta < fp);

				bool new_clock = delta >= m_divider;
				if(new_clock != m_clock_value) {
					machine().scheduler().synchronize();
					if((!new_clock) && (m_clock_state & CLK_TX))
						tx_dropped_edge();

					else if(new_clock && (m_clock_state & CLK_RX))
						rx_raised_edge();

					m_clock_value = new_clock;
					if(m_clock_state || m_clock_value)
						m_cpu->do_sci_clk(m_id, m_clock_value);
				}
			}
			event = m_clock_base + (m_clock_value ? fp : m_divider);
		}
		break;

	case clock_mode_t::INTERNAL_ASYNC:
	case clock_mode_t::INTERNAL_ASYNC_OUT:
		if(m_clock_state || !m_clock_value) {
			uint64_t fp = m_divider*16;
			if(current_time >= m_clock_base) {
				uint64_t delta = current_time - m_clock_base;
				if(delta >= fp) {
					delta -= fp;
					m_clock_base += fp;
				}
				assert(delta < fp);
				bool new_clock = delta >= m_divider*8;
				if(new_clock != m_clock_value) {
					machine().scheduler().synchronize();
					if((!new_clock) && (m_clock_state & CLK_TX))
						tx_dropped_edge();

					else if(new_clock && (m_clock_state & CLK_RX))
						rx_raised_edge();

					m_clock_value = new_clock;
					if(m_clock_mode == clock_mode_t::INTERNAL_ASYNC_OUT && (m_clock_state || !m_clock_value))
						m_cpu->do_sci_clk(m_id, m_clock_value);
				}
			}

			event = m_clock_base + (m_clock_value ? fp : m_divider*8);
		}
		break;

	case clock_mode_t::EXTERNAL_RATE_SYNC:
		if(m_clock_state || !m_clock_value) {
			uint64_t ctime = uint64_t(current_time*m_internal_to_external_ratio*2);
			if(ctime >= m_clock_base) {
				uint64_t delta = ctime - m_clock_base;
				m_clock_base += delta & ~1;
				delta &= 1;
				bool new_clock = delta >= 1;
				if(new_clock != m_clock_value) {
					machine().scheduler().synchronize();
					if((!new_clock) && (m_clock_state & CLK_TX))
						tx_dropped_edge();

					else if(new_clock && (m_clock_state & CLK_RX))
						rx_raised_edge();

					m_clock_value = new_clock;
				}
			}

			event = uint64_t((m_clock_base + (m_clock_value ? 2 : 1))*m_external_to_internal_ratio)+1;
		}
		break;

	case clock_mode_t::EXTERNAL_RATE_ASYNC:
		if(m_clock_state || !m_clock_value) {
			uint64_t ctime = uint64_t(current_time*m_internal_to_external_ratio);
			if(ctime >= m_clock_base) {
				uint64_t delta = ctime - m_clock_base;
				m_clock_base += delta & ~15;
				delta &= 15;
				bool new_clock = delta >= 8;
				if(new_clock != m_clock_value) {
					machine().scheduler().synchronize();
					if((!new_clock) && (m_clock_state & CLK_TX))
						tx_dropped_edge();

					else if(new_clock && (m_clock_state & CLK_RX))
						rx_raised_edge();

					m_clock_value = new_clock;
				}
			}

			event = uint64_t((m_clock_base + (m_clock_value ? 16 : 8))*m_external_to_internal_ratio)+1;
		}
		break;

	case clock_mode_t::EXTERNAL_ASYNC:
	case clock_mode_t::EXTERNAL_SYNC:
		break;
	}
	if(event) {
		attotime ctime = machine().time();
		attotime sync_time = attotime::from_ticks(event-10, m_cpu->clock());
		if(m_cur_sync_time != sync_time && sync_time > ctime) {
			m_sync_timer->adjust(sync_time - ctime);
			m_cur_sync_time = sync_time;
		}
	}

	return event;
}

void h8_sci_device::clock_start(int mode)
{
	// Happens when back-to-back
	if(m_clock_state & mode)
		return;

	if(!m_clock_state) {
		machine().scheduler().synchronize();
		m_clock_state = mode;
		switch(m_clock_mode) {
		case clock_mode_t::INTERNAL_ASYNC:
		case clock_mode_t::INTERNAL_ASYNC_OUT:
		case clock_mode_t::INTERNAL_SYNC_OUT:
			if(V>=2) logerror("Starting internal clock\n");
			m_clock_base = m_cpu->total_cycles();
			m_cpu->internal_update();
			break;

		case clock_mode_t::EXTERNAL_RATE_ASYNC:
			if(V>=2) logerror("Simulating external clock async\n");
			m_clock_base = uint64_t(m_cpu->total_cycles()*m_internal_to_external_ratio);
			m_cpu->internal_update();
			break;

		case clock_mode_t::EXTERNAL_RATE_SYNC:
			if(V>=2) logerror("Simulating external clock sync\n");
			m_clock_base = uint64_t(m_cpu->total_cycles()*2*m_internal_to_external_ratio);
			m_cpu->internal_update();
			break;

		case clock_mode_t::EXTERNAL_ASYNC:
			if(V>=2) logerror("Waiting for external clock async\n");
			m_ext_clock_counter = 15;
			break;

		case clock_mode_t::EXTERNAL_SYNC:
			if(V>=2) logerror("Waiting for external clock sync\n");
			break;
		}
	} else
		m_clock_state |= mode;
}

void h8_sci_device::clock_stop(int mode)
{
	m_clock_state &= ~mode;
	m_cpu->internal_update();
}

void h8_sci_device::tx_start()
{
	m_ssr |= SSR_TDRE;
	m_tsr = m_tdr;
	m_tx_parity = m_smr & SMR_OE ? 0 : 1;
	if(V>=1) logerror("start transmit %02x '%c'\n", m_tsr, m_tsr >= 32 && m_tsr < 127 ? m_tsr : '.');
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

void h8_sci_device::tx_dropped_edge()
{
	if(V>=2) logerror("tx_dropped_edge state=%s bit=%d\n", state_names[m_tx_state], m_tx_bit);
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
	if(V>=2) logerror("            -> state=%s bit=%d\n", state_names[m_tx_state], m_tx_bit);
}

void h8_sci_device::rx_start()
{
	m_rx_parity = m_smr & SMR_OE ? 0 : 1;
	m_rsr = 0x00;
	if(V>=2) logerror("start receive\n");
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
			if(V>=1) logerror("Receive parity error\n");
		} else if(m_ssr & SSR_RDRF) {
			m_ssr |= SSR_ORER;
			if(V>=1) logerror("Receive overrun\n");
		} else {
			m_ssr |= SSR_RDRF;
			if(V>=1) logerror("Received %02x '%c'\n", m_rsr, m_rsr >= 32 && m_rsr < 127 ? m_rsr : '.');
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

void h8_sci_device::rx_raised_edge()
{
	if(V>=2) logerror("rx_raised_edge state=%s bit=%d\n", state_names[m_rx_state], m_rx_bit);
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
	if(V>=2) logerror("            -> state=%s, bit=%d\n", state_names[m_rx_state], m_rx_bit);
}
