// license:BSD-3-Clause
// copyright-holders:buffi
/***************************************************************************

  SH7709 SCIF (Serial Communication Interface with FIFO)

  The SH7709/SH7709S has three serial blocks: a plain SCI in the P4 area, an
  IrDA capable SCIF (channel 1) and a second SCIF (channel 2), the latter two
  living in the on-chip peripheral area at 0x04000140 and 0x04000150.  This
  device models one SCIF channel; only the asynchronous mode is implemented,
  which is all the SCIF is capable of on this part.

  TODO:
    - The interrupt request lines are generated but the SH3 INTC has no SCIF
      sources yet (sh3_base_device::ipre_w only stores the register), so they
      currently go nowhere.  Wire them up when a driver needs them - the first
      consumer, cave/cv1k.cpp (mmmbanc), polls the status register instead.
    - Serial clock input/output (SCSCR CKE) is ignored, the internal baud rate
      generator is always used.
    - No modem control (SCFCR MCE, SCSPTR), no loopback, no break generation.
    - The FIFO error counts in the upper byte of SCSSR are four bits wide; what
      the hardware reports with sixteen erroneous bytes queued is unknown, here
      the count saturates at 15.

***************************************************************************/

#include "emu.h"
#include "sh7709_scif.h"

#include <bit>

#define LOG_REGISTERS (1U << 1)
#define LOG_TXRX      (1U << 2)
#define LOG_CLOCK     (1U << 3)
#define LOG_ERROR     (1U << 4)

// #define VERBOSE (LOG_ERROR | LOG_REGISTERS | LOG_CLOCK)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SH7709_SCIF, sh7709_scif_device, "sh7709scif", "SH7709 SCIF Controller")

sh7709_scif_device::sh7709_scif_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7709_SCIF, tag, owner, clock)
	, m_txd_cb(*this)
	, m_eri_cb(*this)
	, m_rxi_cb(*this)
	, m_bri_cb(*this)
	, m_txi_cb(*this)
{
}

void sh7709_scif_device::device_start()
{
	save_item(NAME(m_scsmr));
	save_item(NAME(m_scbrr));
	save_item(NAME(m_scscr));
	save_item(NAME(m_scssr));
	save_item(NAME(m_scfcr));
	save_item(NAME(m_rx_fifo));
	save_item(NAME(m_rx_err));
	save_item(NAME(m_tx_fifo));
	save_item(NAME(m_rx_head));
	save_item(NAME(m_rx_count));
	save_item(NAME(m_tx_head));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_clock_speed));
	save_item(NAME(m_rxd));
	save_item(NAME(m_rx_busy));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_rx_frame_bits));
	save_item(NAME(m_txd));
	save_item(NAME(m_tx_busy));
	save_item(NAME(m_tx_shift));
	save_item(NAME(m_tx_bit));
	save_item(NAME(m_tx_frame_bits));
	save_item(NAME(m_tx_stop_bit));

	m_rx_shift_timer = timer_alloc(FUNC(sh7709_scif_device::rx_shift), this);
	m_tx_shift_timer = timer_alloc(FUNC(sh7709_scif_device::tx_shift), this);
	m_rx_timeout = timer_alloc(FUNC(sh7709_scif_device::rx_timeout), this);
	m_rxd = 1;
	m_txd = 1;
}

void sh7709_scif_device::device_reset()
{
	m_scsmr = 0x00;
	m_scbrr = 0xff;
	m_scscr = 0x00;
	m_scssr = SCSSR_TDFE | SCSSR_TEND;
	m_scfcr = 0x00;

	std::fill(std::begin(m_rx_fifo), std::end(m_rx_fifo), 0);
	std::fill(std::begin(m_tx_fifo), std::end(m_tx_fifo), 0);
	clear_rx_fifo();
	clear_tx_fifo();

	m_clock_speed = attotime::never;
	m_rx_timeout->reset();

	abort_rx();
	abort_tx();

	// Drive the idle level even if m_txd already holds it, so the other end
	// starts out in sync.
	m_txd_cb(1);

	update_clock();
	update_interrupts();
}

void sh7709_scif_device::device_clock_changed()
{
	update_clock();
}

void sh7709_scif_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(sh7709_scif_device::scsmr_r), FUNC(sh7709_scif_device::scsmr_w));
	map(0x02, 0x02).rw(FUNC(sh7709_scif_device::scbrr_r), FUNC(sh7709_scif_device::scbrr_w));
	map(0x04, 0x04).rw(FUNC(sh7709_scif_device::scscr_r), FUNC(sh7709_scif_device::scscr_w));
	map(0x06, 0x06).w(FUNC(sh7709_scif_device::scftdr_w));
	map(0x08, 0x09).rw(FUNC(sh7709_scif_device::scssr_r), FUNC(sh7709_scif_device::scssr_w));
	map(0x0a, 0x0a).r(FUNC(sh7709_scif_device::scfrdr_r));
	map(0x0c, 0x0c).rw(FUNC(sh7709_scif_device::scfcr_r), FUNC(sh7709_scif_device::scfcr_w));
	map(0x0e, 0x0f).r(FUNC(sh7709_scif_device::scfdr_r));
}


//**************************************************************************
//  REGISTERS
//**************************************************************************

uint8_t sh7709_scif_device::scsmr_r()
{
	return m_scsmr;
}

void sh7709_scif_device::scsmr_w(uint8_t data)
{
	LOGMASKED(LOG_REGISTERS, "scsmr_w %02x: %c%c%c /%d\n", data,
			(data & SCSMR_CHR) ? '7' : '8',
			(data & SCSMR_PE) ? ((data & SCSMR_OE) ? 'o' : 'e') : 'n',
			(data & SCSMR_STOP) ? '2' : '1',
			1 << (2 * (data & SCSMR_CKS)));

	// The frame format is picked up at the start of each frame.
	m_scsmr = data;
	update_clock();
}

uint8_t sh7709_scif_device::scbrr_r()
{
	return m_scbrr;
}

void sh7709_scif_device::scbrr_w(uint8_t data)
{
	LOGMASKED(LOG_REGISTERS, "scbrr_w %02x\n", data);

	m_scbrr = data;
	update_clock();
}

uint8_t sh7709_scif_device::scscr_r()
{
	return m_scscr;
}

void sh7709_scif_device::scscr_w(uint8_t data)
{
	LOGMASKED(LOG_REGISTERS, "scscr_w %02x:%s%s%s%s cke=%d\n", data,
			(data & SCSCR_TIE) ? " tie" : "",
			(data & SCSCR_RIE) ? " rie" : "",
			(data & SCSCR_TE) ? " te" : "",
			(data & SCSCR_RE) ? " re" : "",
			data & SCSCR_CKE);

	const uint8_t old = m_scscr;
	m_scscr = data;

	// Clearing TE or RE only stops the transmitter or receiver.  SCSSR and the
	// FIFOs keep their contents, they are only emptied through SCFCR TFRST and
	// RFRST (SH7709S hardware manual 16.3.2).
	if (!(m_scscr & SCSCR_TE))
	{
		abort_tx();

		if (old & SCSCR_TE)
			m_scssr |= SCSSR_TEND;
	}

	if (!(m_scscr & SCSCR_RE))
		abort_rx();

	update_status();
	update_tx_state();
}

void sh7709_scif_device::scftdr_w(uint8_t data)
{
	if (m_scfcr & SCFCR_TFRST)
	{
		LOGMASKED(LOG_ERROR, "scftdr_w %02x: transmit FIFO held in reset\n", data);
		return;
	}

	if (m_tx_count >= FIFO_LENGTH)
	{
		LOGMASKED(LOG_ERROR, "scftdr_w %02x: transmit FIFO overrun\n", data);
		return;
	}

	LOGMASKED(LOG_TXRX, "scftdr_w %02x\n", data);

	m_tx_fifo[(m_tx_head + m_tx_count) % FIFO_LENGTH] = data;
	m_tx_count++;

	// TEND drops as soon as there is something left to send
	m_scssr &= ~SCSSR_TEND;

	update_status();
	update_tx_state();
}

uint16_t sh7709_scif_device::scssr_r()
{
	return m_scssr;
}

void sh7709_scif_device::scssr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// A status bit that has been read as 1 is cleared by writing 0 to it.  FER,
	// PER and the error counts are read only, they follow the receive FIFO.
	const uint16_t old = m_scssr;
	m_scssr &= data | ~(SCSSR_RW & mem_mask);

	if ((old & (SCSSR_ER | SCSSR_BRK)) && !(m_scssr & (SCSSR_ER | SCSSR_BRK)))
		LOGMASKED(LOG_ERROR, "scssr_w %04x: error flags cleared\n", data);

	LOGMASKED(LOG_REGISTERS, "scssr_w %04x & %04x: %04x -> %04x\n", data, mem_mask, old, m_scssr);

	// The hardware re-asserts the FIFO driven flags as long as their condition holds.
	update_status();
	update_tx_state();
}

uint8_t sh7709_scif_device::scfrdr_r()
{
	if (!m_rx_count)
	{
		LOGMASKED(LOG_TXRX, "scfrdr_r: receive FIFO empty\n");
		return 0;
	}

	const uint8_t data = m_rx_fifo[m_rx_head];

	if (!machine().side_effects_disabled())
	{
		m_rx_err[m_rx_head] = 0;
		m_rx_head = (m_rx_head + 1) % FIFO_LENGTH;
		m_rx_count--;

		LOGMASKED(LOG_TXRX, "scfrdr_r %02x (%d left)\n", data, m_rx_count);

		update_status();
	}

	return data;
}

uint8_t sh7709_scif_device::scfcr_r()
{
	return m_scfcr;
}

void sh7709_scif_device::scfcr_w(uint8_t data)
{
	LOGMASKED(LOG_REGISTERS, "scfcr_w %02x\n", data);

	m_scfcr = data;

	// TFRST and RFRST hold their FIFO empty for as long as they are set.
	if (data & SCFCR_RFRST)
	{
		clear_rx_fifo();
		abort_rx();
	}

	if (data & SCFCR_TFRST)
	{
		clear_tx_fifo();
		abort_tx();
	}

	update_status();
	update_tx_state();
}

uint16_t sh7709_scif_device::scfdr_r()
{
	return (uint16_t(m_tx_count) << 8) | m_rx_count;
}


//**************************************************************************
//  INTERNALS
//**************************************************************************

unsigned sh7709_scif_device::rx_trigger() const
{
	static constexpr unsigned LEVELS[4] = { 1, 4, 8, 14 };
	return LEVELS[(m_scfcr & SCFCR_RTRG) >> 6];
}

unsigned sh7709_scif_device::tx_trigger() const
{
	static constexpr unsigned LEVELS[4] = { 8, 4, 2, 1 };
	return LEVELS[(m_scfcr & SCFCR_TTRG) >> 4];
}

void sh7709_scif_device::clear_rx_fifo()
{
	std::fill(std::begin(m_rx_err), std::end(m_rx_err), 0);
	m_rx_head = m_rx_count = 0;
}

void sh7709_scif_device::clear_tx_fifo()
{
	m_tx_head = m_tx_count = 0;
}

void sh7709_scif_device::update_status()
{
	// FER and PER describe the byte at the head of the receive FIFO, the one
	// SCFRDR returns next.  The upper byte counts the erroneous bytes queued.
	unsigned fer_count = 0, per_count = 0;
	for (unsigned i = 0; i < m_rx_count; i++)
	{
		const uint8_t err = m_rx_err[(m_rx_head + i) % FIFO_LENGTH];
		if (err & SCSSR_FER)
			fer_count++;
		if (err & SCSSR_PER)
			per_count++;
	}

	m_scssr &= ~(SCSSR_PERN | SCSSR_FERN | SCSSR_FER | SCSSR_PER);
	m_scssr |= std::min(per_count, 15U) << 12;
	m_scssr |= std::min(fer_count, 15U) << 8;
	if (m_rx_count)
		m_scssr |= m_rx_err[m_rx_head];

	// RDF reflects the receive FIFO having reached its trigger level.
	if (m_rx_count >= rx_trigger())
		m_scssr |= SCSSR_RDF;
	else
		m_scssr &= ~SCSSR_RDF;

	if (m_tx_count <= tx_trigger())
		m_scssr |= SCSSR_TDFE;
	else
		m_scssr &= ~SCSSR_TDFE;

	if (!m_tx_count && !m_tx_busy)
		m_scssr |= SCSSR_TEND;

	update_interrupts();
}

void sh7709_scif_device::update_interrupts()
{
	// RIE enables all three receive side interrupts.
	const bool rie = m_scscr & SCSCR_RIE;
	m_eri_cb((rie && (m_scssr & SCSSR_ER)) ? 1 : 0);
	m_bri_cb((rie && (m_scssr & SCSSR_BRK)) ? 1 : 0);
	m_rxi_cb((rie && (m_scssr & (SCSSR_RDF | SCSSR_DR))) ? 1 : 0);
	m_txi_cb(((m_scscr & SCSCR_TIE) && (m_scssr & SCSSR_TDFE)) ? 1 : 0);
}

void sh7709_scif_device::update_tx_state()
{
	// Start the next frame once the transmitter is idle.
	if (!(m_scscr & SCSCR_TE) || (m_scfcr & SCFCR_TFRST) || !m_tx_count || m_tx_busy || m_clock_speed.is_never())
		return;

	const uint8_t data = m_tx_fifo[m_tx_head];
	m_tx_head = (m_tx_head + 1) % FIFO_LENGTH;
	m_tx_count--;

	start_tx_frame(data);

	m_scssr &= ~SCSSR_TEND;
	update_status();
}

void sh7709_scif_device::update_clock()
{
	// Baud rate = PERIPHERAL_CLOCK / (32 * 4^CKS * (SCBRR + 1))
	const unsigned divider = 32 * (1 << (2 * (m_scsmr & SCSMR_CKS))) * (m_scbrr + 1);
	const attotime clock_speed = attotime::from_ticks(divider, clock());

	if (clock_speed == m_clock_speed)
		return;

	LOGMASKED(LOG_CLOCK, "baud rate %f (cks=%d brr=%02x pclk=%d)\n",
			clock_speed.as_hz(), m_scsmr & SCSMR_CKS, m_scbrr, clock());

	// Frames already being shifted pick up the new bit time on their next bit.
	m_clock_speed = clock_speed;
	update_tx_state();
}

unsigned sh7709_scif_device::data_bits() const
{
	return (m_scsmr & SCSMR_CHR) ? 7 : 8;
}

unsigned sh7709_scif_device::parity_bit(uint8_t data) const
{
	// Even parity makes the number of 1 bits including the parity bit even.
	return (std::popcount(data) & 1) ^ ((m_scsmr & SCSMR_OE) ? 1 : 0);
}


//**************************************************************************
//  SERIAL LINE
//**************************************************************************

void sh7709_scif_device::rxd_w(int state)
{
	// The receiver synchronises on the falling edge of the start bit and
	// latches each bit in the middle, on the eighth pulse of the 16x base clock
	// (SH7709S hardware manual 16.3.2).
	if (m_rxd && !state && !m_rx_busy && (m_scscr & SCSCR_RE) && !m_clock_speed.is_never())
	{
		m_rx_busy = true;
		m_rx_shift = 0;
		m_rx_bits = 0;

		// Only the first stop bit is checked, a second one is just idle line.
		m_rx_frame_bits = 1 + data_bits() + ((m_scsmr & SCSMR_PE) ? 1 : 0) + 1;

		m_rx_shift_timer->adjust(m_clock_speed / 2);
	}

	m_rxd = state;
}

TIMER_CALLBACK_MEMBER(sh7709_scif_device::rx_shift)
{
	m_rx_shift |= uint16_t(m_rxd ? 1 : 0) << m_rx_bits;
	if (++m_rx_bits < m_rx_frame_bits)
	{
		m_rx_shift_timer->adjust(m_clock_speed);
		return;
	}

	m_rx_busy = false;

	const unsigned bits = data_bits();
	const uint8_t data = BIT(m_rx_shift, 1, bits);
	uint8_t err = 0;
	if (!BIT(m_rx_shift, m_rx_frame_bits - 1))
		err |= SCSSR_FER;
	if ((m_scsmr & SCSMR_PE) && (BIT(m_rx_shift, 1 + bits) != parity_bit(data)))
		err |= SCSSR_PER;

	receive_frame(data, err);
}

void sh7709_scif_device::receive_frame(uint8_t data, uint8_t err)
{
	// The DR timeout runs from the end of the last received frame.
	m_rx_timeout->adjust(m_clock_speed * 15);

	// While BRK is set no further data is transferred to the receive FIFO
	// (SH7709S hardware manual 16.2.7, 16.3.2).
	if (m_scssr & SCSSR_BRK)
	{
		LOGMASKED(LOG_ERROR, "break pending, dropping %02x\n", data);
		return;
	}

	if (m_scfcr & SCFCR_RFRST)
	{
		LOGMASKED(LOG_ERROR, "receive FIFO held in reset, dropping %02x\n", data);
		return;
	}

	// The SCIF has no overrun error, data arriving while the FIFO is full is
	// silently lost (SH7709S hardware manual 16.2.2).
	if (m_rx_count >= FIFO_LENGTH)
	{
		LOGMASKED(LOG_ERROR, "receive FIFO full, dropping %02x\n", data);
		return;
	}

	if (err)
		m_scssr |= SCSSR_ER;

	// A framing error on an all zero frame with the line still at space is a
	// break.  The break frame itself is stored, later ones are not.
	if ((err & SCSSR_FER) && !data && !m_rxd)
	{
		LOGMASKED(LOG_ERROR, "break detected\n");
		m_scssr |= SCSSR_BRK;
	}

	const unsigned slot = (m_rx_head + m_rx_count) % FIFO_LENGTH;
	m_rx_fifo[slot] = data;
	m_rx_err[slot] = err;
	m_rx_count++;

	LOGMASKED(LOG_TXRX, "received %02x%s%s (%d queued)\n", data,
			(err & SCSSR_FER) ? " FER" : "",
			(err & SCSSR_PER) ? " PER" : "",
			m_rx_count);

	update_status();
}

TIMER_CALLBACK_MEMBER(sh7709_scif_device::rx_timeout)
{
	// DR: data below the receive trigger level and no new frame for 15 etu
	// after the last stop bit.
	if (m_rx_count && (m_rx_count < rx_trigger()) && !m_rx_busy)
	{
		LOGMASKED(LOG_TXRX, "receive timeout, %d bytes below trigger\n", m_rx_count);
		m_scssr |= SCSSR_DR;
		update_interrupts();
	}
}

void sh7709_scif_device::start_tx_frame(uint8_t data)
{
	LOGMASKED(LOG_TXRX, "transmitting %02x\n", data);

	// Start bit, data LSB first, optional parity, then ones for the stop bits.
	const unsigned bits = data_bits();
	const unsigned pe = (m_scsmr & SCSMR_PE) ? 1 : 0;
	data &= (1 << bits) - 1;
	uint16_t frame = data;
	if (pe)
		frame |= parity_bit(data) << bits;
	m_tx_shift = (frame | (0xffff << (bits + pe))) << 1;
	m_tx_stop_bit = 1 + bits + pe;
	m_tx_frame_bits = m_tx_stop_bit + ((m_scsmr & SCSMR_STOP) ? 2 : 1);

	m_tx_busy = true;
	m_tx_bit = 0;
	set_txd(BIT(m_tx_shift, 0));
	m_tx_shift_timer->adjust(m_clock_speed);
}

TIMER_CALLBACK_MEMBER(sh7709_scif_device::tx_shift)
{
	if (++m_tx_bit < m_tx_frame_bits)
	{
		set_txd(BIT(m_tx_shift, m_tx_bit));

		// TEND is set when the stop bit goes out with SCFTDR empty (16.3.2).
		if ((m_tx_bit == m_tx_stop_bit) && !m_tx_count)
			m_scssr |= SCSSR_TEND;

		m_tx_shift_timer->adjust(m_clock_speed);
		return;
	}

	// Frame done, start the next one or leave the line marking.
	m_tx_busy = false;
	update_tx_state();
	update_status();
}

void sh7709_scif_device::abort_rx()
{
	m_rx_busy = false;
	m_rx_shift_timer->reset();
}

void sh7709_scif_device::abort_tx()
{
	m_tx_busy = false;
	m_tx_shift_timer->reset();
	set_txd(1);
}

void sh7709_scif_device::set_txd(int state)
{
	if (state != m_txd)
	{
		m_txd = state;
		m_txd_cb(state);
	}
}
