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

#define LOG_REGISTERS (1U << 1)
#define LOG_TXRX      (1U << 2)
#define LOG_CLOCK     (1U << 3)
#define LOG_ERROR     (1U << 4)

// #define VERBOSE (LOG_ERROR | LOG_REGISTERS | LOG_CLOCK)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SH7709_SCIF, sh7709_scif_device, "sh7709scif", "SH7709 SCIF Controller")

sh7709_scif_device::sh7709_scif_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7709_SCIF, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
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

	m_rx_timeout = timer_alloc(FUNC(sh7709_scif_device::rx_timeout), this);
	m_rxd = 1;
}

void sh7709_scif_device::device_reset()
{
	m_scsmr = 0x00;
	m_scbrr = 0xff;
	m_scscr = 0x00;
	m_scssr = SCSSR_TDFE | SCSSR_TEND;
	m_scfcr = 0x00;

	std::fill(std::begin(m_rx_fifo), std::end(m_rx_fifo), 0);
	std::fill(std::begin(m_rx_err), std::end(m_rx_err), 0);
	std::fill(std::begin(m_tx_fifo), std::end(m_tx_fifo), 0);
	clear_rx_fifo();
	clear_tx_fifo();

	m_clock_speed = attotime::never;
	m_rx_timeout->adjust(attotime::never);

	receive_register_reset();
	transmit_register_reset();
	m_txd_cb(1);

	update_data_format();
	update_clock();
	update_interrupts();
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

	const uint8_t old = m_scsmr;
	m_scsmr = data;

	if ((data & ~SCSMR_CKS) != (old & ~SCSMR_CKS))
		update_data_format();
	if ((data & SCSMR_CKS) != (old & SCSMR_CKS))
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
		transmit_register_reset();
		m_txd_cb(1);

		if (old & SCSCR_TE)
			m_scssr |= SCSSR_TEND;
	}

	if (!(m_scscr & SCSCR_RE))
		receive_register_reset();

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
		receive_register_reset();
	}

	if (data & SCFCR_TFRST)
	{
		clear_tx_fifo();
		transmit_register_reset();
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

	if (!m_tx_count && is_transmit_register_empty())
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
	if (!(m_scscr & SCSCR_TE) || (m_scfcr & SCFCR_TFRST) || !m_tx_count || !is_transmit_register_empty())
		return;

	const uint8_t data = m_tx_fifo[m_tx_head];
	m_tx_head = (m_tx_head + 1) % FIFO_LENGTH;
	m_tx_count--;

	LOGMASKED(LOG_TXRX, "transmitting %02x\n", data);

	transmit_register_setup(data);

	m_scssr &= ~SCSSR_TEND;
	update_status();
}

void sh7709_scif_device::update_data_format()
{
	set_data_frame(
			1,
			(m_scsmr & SCSMR_CHR) ? 7 : 8,
			(m_scsmr & SCSMR_PE) ? ((m_scsmr & SCSMR_OE) ? PARITY_ODD : PARITY_EVEN) : PARITY_NONE,
			(m_scsmr & SCSMR_STOP) ? STOP_BITS_2 : STOP_BITS_1);
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

	m_clock_speed = clock_speed;
	set_rate(clock_speed);
}


//**************************************************************************
//  SERIAL LINE
//**************************************************************************

void sh7709_scif_device::rxd_w(int state)
{
	m_rxd = state;
	device_serial_interface::rx_w(state);
}

void sh7709_scif_device::rcv_complete()
{
	receive_register_extract();

	if (!(m_scscr & SCSCR_RE))
		return;

	const uint8_t data = get_received_char();
	uint8_t err = 0;
	if (is_receive_framing_error())
		err |= SCSSR_FER;
	if (is_receive_parity_error())
		err |= SCSSR_PER;

	// The DR timeout runs from the end of the last received frame.
	m_rx_timeout->adjust(m_clock_speed.is_never() ? attotime::never : m_clock_speed * 15);

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
	if (m_rx_count && (m_rx_count < rx_trigger()) && !is_receive_register_synchronized())
	{
		LOGMASKED(LOG_TXRX, "receive timeout, %d bytes below trigger\n", m_rx_count);
		m_scssr |= SCSSR_DR;
		update_interrupts();
	}
}

void sh7709_scif_device::tra_callback()
{
	m_txd_cb(transmit_register_get_data_bit());
}

void sh7709_scif_device::tra_complete()
{
	update_tx_state();

	if (!m_tx_count && is_transmit_register_empty())
	{
		m_scssr |= SCSSR_TEND;
		update_status();
	}
}
