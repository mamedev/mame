// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/***************************************************************************

    TOSHIBA TLCS900 - TMP94C241 SERIAL

***************************************************************************/

#include "emu.h"
#include "tmp94c241_serial.h"

#define LOG_SERIAL (1U << 1)

#include "logmacro.h"


DEFINE_DEVICE_TYPE(TMP94C241_SERIAL, tmp94c241_serial_device, "tmp94c241_serial", "TMP94C241 Serial Channel")

tmp94c241_serial_device::tmp94c241_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TMP94C241_SERIAL, tag, owner, clock),
	m_setint_cb(*this),
	m_txd_cb(*this),
	m_sclk_in_cb(*this),
	m_sclk_out_cb(*this),
	m_tx_start_cb(*this),
	m_timer(nullptr),
	m_pffc_sclk(0),
	m_serial_control(0),
	m_serial_mode(0),
	m_baud_rate(0),
	m_hz(0),
	m_rx_clock_count(8),
	m_rx_shift_register(0),
	m_rx_buffer(0),
	m_rxd(0),
	m_rxd_prev(0),
	m_sioclk_state(0),
	m_tx_clock_count(0),
	m_tx_shift_register(0),
	m_txd(1),  // Idle state is HIGH for serial lines
	m_sclk_out(0),
	m_tx_skip_first_falling(false),
	m_tx_needs_trailing_edge(false),
	m_tx_buffer(0),
	m_tx_buffer_full(false)
{
}

void tmp94c241_serial_device::device_start()
{
	m_timer = timer_alloc(FUNC(tmp94c241_serial_device::timer_callback), this);

	save_item(NAME(m_pffc_sclk));
	save_item(NAME(m_serial_control));
	save_item(NAME(m_serial_mode));
	save_item(NAME(m_baud_rate));
	save_item(NAME(m_hz));
	save_item(NAME(m_rx_clock_count));
	save_item(NAME(m_rx_shift_register));
	save_item(NAME(m_rx_buffer));
	save_item(NAME(m_rxd));
	save_item(NAME(m_rxd_prev));
	save_item(NAME(m_sioclk_state));
	save_item(NAME(m_tx_clock_count));
	save_item(NAME(m_tx_shift_register));
	save_item(NAME(m_txd));
	save_item(NAME(m_sclk_out));
	save_item(NAME(m_tx_skip_first_falling));
	save_item(NAME(m_tx_needs_trailing_edge));
	save_item(NAME(m_tx_buffer));
	save_item(NAME(m_tx_buffer_full));

	m_sclk_out_cb(m_sclk_out);
	m_txd_cb(m_txd);
}

void tmp94c241_serial_device::device_reset()
{
	m_serial_control &= 0x80;
	m_serial_mode &= 0x80;
	m_baud_rate = 0x00;
	m_tx_skip_first_falling = false;
	m_tx_needs_trailing_edge = false;
	m_tx_buffer_full = false;
}

void tmp94c241_serial_device::sioclk(int state)
{
	if (m_sioclk_state == state)
		return;

	m_sioclk_state = state;

	// Always forward SCLK to the connected device.  We cannot gate on PFFC
	// or TX activity here because that causes clock state desync: the
	// internal m_sioclk_state keeps toggling during gated-off phases, so
	// the slave's state becomes stale and it misses edges when forwarding
	// resumes (same-state → no transition detected).  Instead, "orphan"
	// edges (driven by the timer for CPU RX completion after TX finishes)
	// are filtered at the cpanel level: the cpanel only counts RX bits
	// between tx_start signals, ignoring edges that arrive between bytes.

	if (state)
	{
		// Rising edge: Sample RXD BEFORE forwarding clock to slave.
		// The slave's rising-edge handler may complete an RX byte and call
		// send_byte(), which pre-outputs bit 0 via m_txd_cb — changing our
		// m_rxd before we can sample it. Capture the value first.
		uint8_t rxd_sample = m_rxd;

		m_sclk_out_cb(state);

		// Handle deferred INTTX from the last falling edge.  The receiver
		// (cpanel) just sampled bit 7 on this rising edge via sclk_out_cb.
		// Now it's safe to fire INTTX — the ISR may write SC1BUF which
		// pre-outputs bit 0 of the next byte, but bit 7 has already been
		// sampled.  On TMP94C241 hardware, the 8th rising edge occurs
		// 4µs after the last falling edge (at 250 kHz SCLK), well before
		// the ISR can write SC1BUF.  In MAME's event-driven scheduler,
		// CPU instructions run between timer ticks, so without this deferral
		// the ISR would write SC1BUF (pre-outputting bit 0) BEFORE the
		// trailing rising edge — corrupting bit 7 of every byte.
		if (m_tx_needs_trailing_edge)
		{
			m_tx_needs_trailing_edge = false;
			m_setint_cb(0x80);

			// Auto-load from TX buffer if data is pending (TX double buffering).
			// On TMP94C241, when the shift register finishes and the buffer
			// has data, the buffer auto-transfers to the shift register.
			if (m_tx_buffer_full)
			{
				m_tx_buffer_full = false;
				m_tx_shift_register = m_tx_buffer;
				m_tx_clock_count = 7;

				// Signal start of new byte transmission with current PFFC state
				// FIXME: data shouldn't be output at all if the pin is high-impedance
				m_tx_start_cb(m_pffc_sclk);

				// Pre-output bit 0 — receiver will sample it on the next rising edge
				m_txd_cb(m_tx_shift_register & 1);

				// Skip next falling edge — bit 0 is on TXD and must stay until
				// the receiver samples it on the following rising edge
				m_tx_skip_first_falling = true;
			}
		}

		if (m_rx_clock_count)
		{
			m_rx_clock_count--;

			m_rx_shift_register >>= 1;
			m_rx_shift_register |= (rxd_sample << 7);

			if (m_rx_clock_count == 0)
			{
				m_rx_clock_count = 8;
				m_rx_buffer = m_rx_shift_register;
				m_setint_cb(0x08);
			}
		}
	}
	else
	{
		// Falling edge: Forward clock to slave, then output our TXD.
		m_sclk_out_cb(state);

		if (m_tx_clock_count){
			if (m_tx_skip_first_falling) {
				// Skip this falling edge - bit 0 was pre-output in scNbuf_w
				// and we need to give the receiver a rising edge to sample it
				m_tx_skip_first_falling = false;
			} else {
				// Normal operation: shift out the next bit
				m_tx_shift_register >>= 1;
				m_txd_cb(m_tx_shift_register & 1);
				if (--m_tx_clock_count == 0) {
					// Byte shift-out complete.  Defer INTTX to the next
					// rising edge so the receiver can sample bit 7 before
					// the ISR writes the next byte (which pre-outputs bit 0).
					m_tx_needs_trailing_edge = true;
				}
			}
		}
	}
}

void tmp94c241_serial_device::rxd(int state)
{
	if (m_rxd != state)
	{
		m_rxd = state;
	}
}

uint8_t tmp94c241_serial_device::scNbuf_r()
{
	return m_rx_buffer;
}

void tmp94c241_serial_device::scNbuf_w(uint8_t data)
{
	// TX double buffering: TMP94C241 has a TX buffer register and
	// a TX shift register.  CPU writes always go to the buffer.  If the
	// shift register is idle, the buffer auto-transfers immediately.
	// If the shift register is busy, the buffer holds the data until
	// the current byte finishes, then auto-loads (see sioclk() trailing
	// rising edge handler).
	bool was_idle = (m_tx_clock_count == 0 && !m_tx_needs_trailing_edge && !m_tx_skip_first_falling);
	if (!was_idle)
	{
		// TX is busy — store in buffer (overwrites any previous buffered byte)
		m_tx_buffer = data;
		m_tx_buffer_full = true;
		return;
	}

	// TX is idle — load shift register directly
	m_tx_shift_register = data;
	m_tx_clock_count = 7;  // 7 more bits to send (bits 1-7) after pre-outputting bit 0

	// Signal start of new transmission.
	// Pass the PFFC state so the slave knows whether this byte is "real"
	// (PFFC enabled, SCLK pin driven → data reaches panel on real hardware)
	// or "phantom" (PFFC disabled, pin is high-impedance → data never
	// reaches the panel on real hardware, but MAME still forwards SCLK).
	// FIXME: data shouldn't be output at all if the pin is high-impedance
	m_tx_start_cb(m_pffc_sclk);

	// Pre-output first bit immediately so slave can sample it on the first rising edge
	m_txd_cb(m_tx_shift_register & 1);

	// Only skip the first falling edge if clock is currently HIGH.
	// If clock is HIGH: next edge = falling (skip it to avoid outputting bit 1 before receiver samples bit 0)
	// If clock is LOW: next edge = rising (receiver samples bit 0), then falling outputs bit 1 (no skip needed)
	m_tx_skip_first_falling = (m_sioclk_state == 1);
}

uint8_t tmp94c241_serial_device::scNcr_r()
{
	return m_serial_control;
}

void tmp94c241_serial_device::scNcr_w(uint8_t data)
{
	// Do NOT reset m_rx_clock_count here.  On TMP94C241 hardware,
	// writing SCxCR configures the serial control register (IOC, SCLKS,
	// parity, error flags) but does NOT abort an in-progress RX byte
	// reception.  The RX shift register has its own bit counter that
	// completes independently of SCxCR writes.
	//
	// The firmware writes SC1CR in the INTRX1 ISR (CPanel_SM_RXByte1,
	// CPanel_SM_RXByteN) to maintain IOC=1 / SCLKS=0 between received
	// bytes.  If this ISR fires between rising edges of the NEXT byte
	// (which happens nondeterministically when the CPU processes the
	// interrupt between self-clock timer callbacks), resetting
	// rx_clock_count=8 causes data loss: the current byte completes
	// 1 bit short, and the residual bits produce a corrupted "phantom
	// byte" at the next INTA session boundary, causing segment header
	// misinterpretation and button identity swapping.
	m_serial_control = data;
}

uint8_t tmp94c241_serial_device::scNmod_r()
{
	return m_serial_mode;
}

void tmp94c241_serial_device::scNmod_w(uint8_t data)
{
	switch((data >> 2) & 3)
	{
		case 0: LOGMASKED(LOG_SERIAL,"I/O interface mode\n"); break;
		case 1: LOGMASKED(LOG_SERIAL,"7-bit uart mode (Not implemented yet)\n"); break;
		case 2: LOGMASKED(LOG_SERIAL,"8-bit uart mode (Not implemented yet)\n"); break;
		case 3: LOGMASKED(LOG_SERIAL,"9-bit uart mode (Not implemented yet)\n"); break;
	}
	switch(data & 3)
	{
		case 0: LOGMASKED(LOG_SERIAL,"clk source: TO2 trigger (Not implemented yet)\n"); break;
		case 1: LOGMASKED(LOG_SERIAL,"clk source: Baud rate generator (Not implemented yet)\n"); break;
		case 2: LOGMASKED(LOG_SERIAL,"clk source: Internal clock at ϕ1 (Not implemented yet)\n"); break;
		case 3: LOGMASKED(LOG_SERIAL,"clk source: external clock (SCLK%d) (Not implemented yet)\n", m_channel); break;
	}
	m_serial_mode = data;

	// Do NOT fire INTTX here.  On TMP94C241 hardware, writing SC1MOD
	// configures the serial mode — it does not trigger a transmit-complete
	// interrupt.  The firmware writes SC1MOD at the start of every TX
	// sequence (SM_StartTX).  A spurious INTTX would cause the TX state
	// machine to advance prematurely (thinking byte 1 finished before it
	// started), corrupting specific command bytes and causing wrong LED
	// states.
}

uint8_t tmp94c241_serial_device::brNcr_r()
{
	return m_baud_rate;
}

void tmp94c241_serial_device::brNcr_w(uint8_t data)
{
	m_baud_rate = data;
	const uint16_t divisor = data & 0x0f;
	constexpr uint8_t input_clocks[] = {0, 2, 8, 32};
	const uint8_t shift_amount = (((data >> 4) & 3) + 1) * 2;
	LOGMASKED(LOG_SERIAL,"baud rate: Divisor=%d  Internal Clock T%d\n", divisor, input_clocks[(data >> 4) & 3]);
	if (divisor)
	{
		uint32_t fc = clock();
		m_hz = fc / (divisor << shift_amount);
		m_timer->adjust(attotime::from_hz(m_hz), 0, attotime::from_hz(m_hz));
		LOGMASKED(LOG_SERIAL,"timer set to %d Hz.\n", m_hz);
	}
	else
	{
		m_timer->reset(attotime::never);
		m_hz = 0;
		LOGMASKED(LOG_SERIAL,"timer disabled.\n");
	}
}

TIMER_CALLBACK_MEMBER(tmp94c241_serial_device::timer_callback)
{
	// In TO2 trigger mode (mode 0), IOC=1 means the clock comes from an
	// external device (cpanel's self-clock via SCLK pin after INTA).
	// Don't drive from the baud rate timer — that would inject extra
	// edges and corrupt data during INTA-driven reception.
	//
	// In baud rate generator mode (mode 1), the baud rate timer is the
	// clock source regardless of IOC, so only gate on mode 0.
	if ((m_serial_mode & 3) == 0 && BIT(m_serial_control, 0))
		return;

	// Keep clocking while TX is in progress, RX hasn't completed its byte,
	// or we need a trailing rising edge for the receiver to sample bit 7.
	//
	// The trailing edge is critical: without it, the baud rate timer stops
	// after TX's last falling edge (tx_clock_count=0).  Between timer ticks,
	// the CPU processes the INTTX1 interrupt and writes the next byte to
	// SC1BUF, pre-outputting bit 0 on TXD.  The next rising edge (from the
	// new byte's timer) would then sample bit 0 of the NEW byte as bit 7
	// of the OLD byte — corrupting every byte's MSB.  The trailing edge
	// ensures bit 7 is sampled correctly before INTTX1 fires.
	//
	// Note: we do NOT gate on PFFC here.  On TMP94C241 hardware,
	// PFFC controls whether the SCLK pin is driven externally, but the
	// internal serial clock (baud rate generator) always runs.  The shift
	// register must complete even during "phantom" bytes (PFFC off) so
	// that INTTX1 fires and the firmware's TX state machine advances.
	// Phantom bytes are filtered at the cpanel level via tx_start_cb:
	// tx_start(0) sets accept_next_byte=false, so the cpanel assembles
	// but rejects phantom bytes.  See sioclk() comments for why we also
	// don't gate sclk_out_cb on PFFC (clock desync issues).
	bool need_clock = (m_tx_clock_count > 0) || m_tx_skip_first_falling || (m_rx_clock_count != 8) || m_tx_needs_trailing_edge;
	if (m_hz && need_clock)
	{
		sioclk(m_sioclk_state ^ 1);
	}
}

