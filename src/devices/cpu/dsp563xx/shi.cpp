// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Serial Host Interface for DSP56362/DSP56364

    Implements the SHI in SPI slave mode, which is the mechanism used by the
    Mode 5 bootstrap (see the on-chip boot ROM) to load a program over SPI.
    The host (e.g. an H8 SCI configured as SPI master) clocks 24-bit words
    MSB-first on MOSI; each completed word is pushed into the HRX FIFO and
    the HREQ pin (active-low) paces the transfer.

***************************************************************************/

#include "emu.h"
#include "shi.h"

#include "dsp563xx.h"

// device type definition
DEFINE_DEVICE_TYPE(DSP5636X_SHI, dsp5636x_shi_device, "dsp5636x_shi", "DSP5636x Serial Host Interface")

dsp5636x_shi_device::dsp5636x_shi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DSP5636X_SHI, tag, owner, clock)
	, m_cpu(*this, DEVICE_SELF_OWNER)
	, m_ss(true), m_sck(false), m_mosi(false), m_miso(false), m_hreq(true)
	, m_rx_shift(0), m_rx_bits(0), m_iosr(0), m_tx_bits(0)
	, m_hrx{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	, m_hrx_head(0), m_hrx_tail(0), m_hrx_count(0)
	, m_miso_cb(*this), m_hreq_cb(*this)
{
}

void dsp5636x_shi_device::device_start()
{
	save_item(NAME(m_hsar));
	save_item(NAME(m_hckr));
	save_item(NAME(m_hcsr));
	save_item(NAME(m_htx));
	save_item(NAME(m_ss));
	save_item(NAME(m_sck));
	save_item(NAME(m_mosi));
	save_item(NAME(m_miso));
	save_item(NAME(m_hreq));
	save_item(NAME(m_rx_shift));
	save_item(NAME(m_rx_bits));
	save_item(NAME(m_iosr));
	save_item(NAME(m_tx_bits));
	save_item(NAME(m_hrx));
	save_item(NAME(m_hrx_head));
	save_item(NAME(m_hrx_tail));
	save_item(NAME(m_hrx_count));
}

void dsp5636x_shi_device::device_reset()
{
	m_hsar = 0xb00000;    // default slave address (1011x0x)
	m_hckr = 0x000001;    // CPHA set, CPOL cleared (per UM: HCKR reset)
	m_hcsr = 0x008000;    // HTDE set (transmit data empty)
	m_htx = 0;

	m_ss = true;
	m_sck = false;
	m_mosi = false;
	m_miso = false;
	m_hreq = true;
	m_rx_shift = 0;
	m_rx_bits = 0;
	m_iosr = 0;
	m_tx_bits = 0;
	m_hrx_head = 0;
	m_hrx_tail = 0;
	m_hrx_count = 0;

	update_status();
	update_hreq();
}


void dsp5636x_shi_device::map(address_map &map)
{
	map(0*4, 0*4+3).rw(FUNC(dsp5636x_shi_device::hckr_r), FUNC(dsp5636x_shi_device::hckr_w));
	map(1*4, 1*4+3).rw(FUNC(dsp5636x_shi_device::hcsr_r), FUNC(dsp5636x_shi_device::hcsr_w));
	map(2*4, 2*4+3).rw(FUNC(dsp5636x_shi_device::hsar_r), FUNC(dsp5636x_shi_device::hsar_w));
	map(3*4, 3*4+3).w(FUNC(dsp5636x_shi_device::htx_w));
	map(4*4, 4*4+3).r(FUNC(dsp5636x_shi_device::hrx_r));
}

u32 dsp5636x_shi_device::hckr_r()
{
	return m_hckr;
}

void dsp5636x_shi_device::hckr_w(u32 data)
{
	m_hckr = data & 0x003fff;
	update_hreq();
}

u32 dsp5636x_shi_device::hcsr_r()
{
	return m_hcsr;
}

void dsp5636x_shi_device::hcsr_w(u32 data)
{
	// control bits are 0-13; 14 and above are status
	m_hcsr = (m_hcsr & ~0x003fff) | (data & 0x003fff);
	update_status();
	update_hreq();
}

u32 dsp5636x_shi_device::hsar_r()
{
	return m_hsar;
}

void dsp5636x_shi_device::hsar_w(u32 data)
{
	m_hsar = data & 0xf40000;
}

void dsp5636x_shi_device::htx_w(u32 data)
{
	m_htx = data & 0xffffff;
	if(m_tx_bits == 0)
		m_iosr = m_htx;
	update_status();
}

u32 dsp5636x_shi_device::hrx_r()
{
	u32 data = 0;
	if(m_hrx_count > 0) {
		data = m_hrx[m_hrx_head];
		m_hrx_head = (m_hrx_head + 1) % 10;
		m_hrx_count--;
		update_status();
		update_hreq();
	}
	return data;
}

bool dsp5636x_shi_device::spi_active() const
{
	return BIT(m_hcsr, HCSR_HEN) && !BIT(m_hcsr, HCSR_HI2C) && !BIT(m_hcsr, HCSR_HMST);
}

int dsp5636x_shi_device::spi_word_bits() const
{
	int hm = (m_hcsr >> HCSR_HM0) & 0x03;
	switch(hm) {
	case 1:
		return 16;
	case 2:
		return 24;
	default:
		return 8;
	}
}

int dsp5636x_shi_device::fifo_depth() const
{
	return BIT(m_hcsr, HCSR_HFIFO) ? 10 : 1;
}

void dsp5636x_shi_device::ss_w(int state)
{
	if(state != m_ss) {
		m_ss = state;
		update_hreq();
	}
}

void dsp5636x_shi_device::mosi_w(int state)
{
	m_mosi = state;
}

void dsp5636x_shi_device::sck_w(int state)
{
	if(state == m_sck)
		return;
	m_sck = state;

	if(!spi_active() || m_ss)
		return;

	// The capture edge samples MOSI; the other edge drives MISO.
	// Leading edge is rising for CPOL=0, falling for CPOL=1; CPHA selects
	// whether capture happens on the leading (0) or trailing (1) edge.
	int leading = BIT(m_hckr, HCKR_CPOL) ? 0 : 1;
	int capture = BIT(m_hckr, HCKR_CPHA) ? !leading : leading;
	if(state == capture)
		spi_rx_bit();
	else
		spi_tx_bit();
}

void dsp5636x_shi_device::spi_rx_bit()
{
	int wb = spi_word_bits();
	// The host transmits LSB-first within each byte, most significant byte
	// first (H8 SCI clocked-synchronous mode), so place each captured bit
	// at its position inside the corresponding byte of the word.
	int const byte = (wb / 8 - 1) - m_rx_bits / 8;
	int const bit = m_rx_bits % 8;
	if(m_mosi)
		m_rx_shift |= 1 << (byte * 8 + bit);
	m_rx_bits++;
	if(m_rx_bits >= wb)
		spi_rx_word_done();
	update_hreq();
}

void dsp5636x_shi_device::spi_tx_bit()
{
	int wb = spi_word_bits();
	// MISO carries LSB-first within each byte, most significant byte first.
	int const byte = (wb / 8 - 1) - m_tx_bits / 8;
	int const bit = m_tx_bits % 8;
	int const value = (m_iosr >> (byte * 8 + bit)) & 1;
	if(value != m_miso) {
		m_miso = value;
		m_miso_cb(value);
	}
	m_tx_bits++;
	if(m_tx_bits >= wb) {
		// word shifted out; reload from HTX
		m_tx_bits = 0;
		m_iosr = m_htx;
		update_status();
	}
}

void dsp5636x_shi_device::spi_rx_word_done()
{
	int depth = fifo_depth();
	if(m_hrx_count < depth) {
		// The host transmits each 24-bit word as three bytes, most significant
		// byte first, LSB-first within each byte; spi_rx_bit() reassembles the
		// word value directly.
		m_hrx[m_hrx_tail] = m_rx_shift & 0xffffff;
		m_hrx_tail = (m_hrx_tail + 1) % 10;
		m_hrx_count++;
	} else {
		m_hcsr |= (1U << HCSR_HROE); // receive overrun
	}
	m_rx_shift = 0;
	m_rx_bits = 0;
	update_status();
}

void dsp5636x_shi_device::update_status()
{
	int depth = fifo_depth();
	constexpr u32 HRNE = 1U << HCSR_HRNE;
	constexpr u32 HRFF = 1U << HCSR_HRFF;
	constexpr u32 HTDE = 1U << HCSR_HTDE;
	m_hcsr = (m_hcsr & ~(HRNE | HRFF | HTDE))
			| ((m_hrx_count > 0) ? HRNE : 0)
			| ((m_hrx_count == depth) ? HRFF : 0)
			| ((m_tx_bits == 0) ? HTDE : 0);
}

void dsp5636x_shi_device::update_hreq()
{
	// HCSR HRQE[1:0] (Table 7-5, SPI slave):
	//   01: HREQ asserted when the receive IOSR is ready for a new word
	//   10: HREQ asserted when the transmit IOSR is ready for a new word
	//   11: HREQ asserted when the IOSR is ready to transmit and receive
	int hrqe = (m_hcsr >> HCSR_HRQE0) & 0x03;
	bool rx_ready = (m_rx_bits == 0) && (m_hrx_count < fifo_depth());
	bool tx_ready = (m_tx_bits == 0);
	bool ready;
	switch(hrqe) {
	case 1:
		ready = rx_ready;
		break;
	case 2:
		ready = tx_ready;
		break;
	case 3:
		ready = rx_ready && tx_ready;
		break;
	default:
		ready = false;
		break;
	}
	if(!spi_active())
		ready = false;
	int pin = ready ? 0 : 1; // HREQ is active-low: 0 = asserted = ready
	if(pin != m_hreq) {
		m_hreq = pin;
		m_hreq_cb(pin);
	}
}
