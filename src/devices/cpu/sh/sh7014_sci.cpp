// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 SCI Controller

  TODO list (not comprehensive):
    - RX is untested
    - Multiprocessor bit is not handled at all

***************************************************************************/

#include "emu.h"
#include "sh7014_sci.h"

#define LOG_REGISTERS (1U << 1)
#define LOG_TXRX (1U << 2)
#define LOG_CLOCK (1U << 3)

// #define VERBOSE (LOG_GENERAL | LOG_REGISTERS | LOG_TXRX | LOG_CLOCK)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SH7014_SCI, sh7014_sci_device, "sh7014sci", "SH7014 SCI Controller")

sh7014_sci_device::sh7014_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7014_SCI, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_intc(*this, finder_base::DUMMY_TAG)
	, m_sci_tx_cb(*this)
	, m_hack_set_full_data_transmit_on_sync(false)
{
	m_external_clock_period = attotime::never;
}

void sh7014_sci_device::device_start()
{
	save_item(NAME(m_smr));
	save_item(NAME(m_brr));
	save_item(NAME(m_scr));
	save_item(NAME(m_tdr));
	save_item(NAME(m_ssr));
	save_item(NAME(m_rdr));
	save_item(NAME(m_is_dma_source_tx));
	save_item(NAME(m_is_dma_source_rx));
	save_item(NAME(m_clock_speed));
	save_item(NAME(m_external_clock_period));
}

void sh7014_sci_device::device_reset()
{
	m_smr = 0;
	m_brr = 0xff;
	m_scr = 0;
	m_tdr = 0xff;
	m_ssr = SSR_TDRE | SSR_TEND;
	m_rdr = 0;
	m_is_dma_source_tx = m_is_dma_source_rx = false;
	m_clock_speed = attotime::never;

	update_data_format();
	update_clock();
}

void sh7014_sci_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(sh7014_sci_device::smr_r), FUNC(sh7014_sci_device::smr_w));
	map(0x01, 0x01).rw(FUNC(sh7014_sci_device::brr_r), FUNC(sh7014_sci_device::brr_w));
	map(0x02, 0x02).rw(FUNC(sh7014_sci_device::scr_r), FUNC(sh7014_sci_device::scr_w));
	map(0x03, 0x03).rw(FUNC(sh7014_sci_device::tdr_r), FUNC(sh7014_sci_device::tdr_w));
	map(0x04, 0x04).rw(FUNC(sh7014_sci_device::ssr_r), FUNC(sh7014_sci_device::ssr_w));
	map(0x05, 0x05).r(FUNC(sh7014_sci_device::rdr_r));
}

///

void sh7014_sci_device::set_send_full_data_transmit_on_sync_hack(bool enabled)
{
	// Synchronous clock mode forces a fixed 8-bit transmissions with no start, stop, parity, or multiprocessor bits.
	// This flag makes it so that all 8 bits in the transmit register will be transferred at the start of a transmission
	// instead of transmitting 1 bit at a time when in synchronous clock mode, allowing the transmit clock to be set to
	// 1/8th of its normal speed.
	m_hack_set_full_data_transmit_on_sync = enabled;
}

void sh7014_sci_device::set_external_clock_period(const attotime &period)
{
	m_external_clock_period = period;

	// Update clock again if it's being used
	if (m_scr & SCR_CKE1)
		update_clock();
}

uint8_t sh7014_sci_device::smr_r()
{
	return m_smr;
}

void sh7014_sci_device::smr_w(uint8_t data)
{
	LOGMASKED(LOG_REGISTERS, "smr_w %02x %s %c%c%c%s /%d\n", data,
			  data & SMR_CA ? "sync" : "async",
			  data & SMR_CHR ? '7' : '8',
			  data & SMR_PE ? data & SMR_OE ? 'o' : 'e' : 'n',
			  data & SMR_STOP ? '2' : '1',
			  data & SMR_MP ? " mp" : "",
			  1 << (2 * (data & SMR_CKS)));

	bool do_clock_update = (data & (SMR_CKS | SMR_CA)) != (m_smr & (SMR_CKS | SMR_CA));
	bool do_format_update = false;

	if (!(data & SMR_CA))
		do_format_update = (data & ~SMR_CKS) != (m_smr & ~SMR_CKS);
	else
		do_format_update = (data & SMR_CA) != (m_smr & SMR_CA);

	m_smr = data;

	if (do_format_update)
		update_data_format();

	if (do_clock_update)
		update_clock();
}

uint8_t sh7014_sci_device::scr_r()
{
	return m_scr;
}

void sh7014_sci_device::scr_w(uint8_t data)
{
	const auto old = m_scr;

	LOGMASKED(LOG_REGISTERS, "scr_w %02x%s%s%s%s%s%s clk=%d\n", data,
			  (data & SCR_TIE) ? " txi" : "",
			  (data & SCR_RIE) ? " rxi" : "",
			  (data & SCR_TE) ? " tx" : "",
			  (data & SCR_RE) ? " rx" : "",
			  (data & SCR_MPIE) ? " mpi" : "",
			  (data & SCR_TEIE) ? " tei" : "",
			  data & SCR_CKE);

	if ((m_scr & SCR_TE) && !(data & SCR_TE))
		m_ssr |= SSR_TEND | SSR_TDRE;

	if ((m_scr & SCR_TIE) && !(data & SCR_TIE))
		m_intc->set_interrupt(m_txi_int, CLEAR_LINE);

	if ((m_scr & SCR_TEIE) && !(data & SCR_TEIE))
		m_intc->set_interrupt(m_tei_int, CLEAR_LINE);

	if ((m_scr & SCR_RIE) && !(data & SCR_RIE)) {
		m_intc->set_interrupt(m_rxi_int, CLEAR_LINE);
		m_intc->set_interrupt(m_eri_int, CLEAR_LINE);
	}

	m_scr = data;

	if ((data & SCR_CKE) != (old & SCR_CKE))
		update_clock();
}

uint8_t sh7014_sci_device::ssr_r()
{
	LOGMASKED(LOG_REGISTERS, "ssr_r %02x\n", m_ssr);
	return m_ssr;
}

void sh7014_sci_device::ssr_w(uint8_t data)
{
	const auto old = m_ssr;
	bool do_tx_update = false;

	m_ssr = (data & (m_ssr & (SSR_TDRE | SSR_RDRF | SSR_ORER | SSR_FER | SSR_PER)))
		| (m_ssr & (SSR_TEND | SSR_MPB))
		| (data & SSR_MPBT);

	if (!(m_scr & SCR_TE)) {
		m_ssr |= SSR_TEND | SSR_TDRE;
	} else if ((old & SSR_TDRE) || !(m_scr & SSR_TDRE)) {
		do_tx_update = true;
		m_ssr &= ~(SSR_TEND | SSR_TDRE);
	}

	if ((old & (SSR_ORER | SSR_FER | SSR_PER)) && !(data & (SSR_ORER | SSR_FER | SSR_PER)))
		m_intc->set_interrupt(m_eri_int, CLEAR_LINE);

	LOGMASKED(LOG_REGISTERS, "ssr_w %02x | %02x -> %02x\n", data, old, m_ssr);

	if (do_tx_update)
		update_tx_state();
}

uint8_t sh7014_sci_device::brr_r()
{
	return m_brr;
}

void sh7014_sci_device::brr_w(uint8_t data)
{
	LOGMASKED(LOG_REGISTERS, "brr_w %02x\n", data);
	m_brr = data;
	update_clock();
}

uint8_t sh7014_sci_device::tdr_r()
{
	LOGMASKED(LOG_TXRX, "tdr_r %02x\n", m_tdr);
	return m_tdr;
}

void sh7014_sci_device::tdr_w(uint8_t data)
{
	LOGMASKED(LOG_TXRX, "tdr_w%s %02x %d %d\n", m_is_dma_source_tx ? " (dma)" : "", data, is_transmit_register_empty(), (m_scr & SCR_TE) != 0);

	m_tdr = data;

	if (!(m_scr & SCR_TE))
		return;

	if (m_is_dma_source_tx) {
		// Normally this would happen with a write to SCR but DMAs can only write to TDR,
		// so these flags are handled here as a special case for DMA writes only
		m_ssr &= ~(SSR_TEND | SSR_TDRE);
		m_is_dma_source_tx = false;

		update_tx_state();
	}
}

void sh7014_sci_device::tra_callback()
{
	if (!is_transmit_register_empty())
		m_sci_tx_cb(transmit_register_get_data_bit());

	if ((m_smr & SMR_CA) && m_hack_set_full_data_transmit_on_sync) {
		while (!is_transmit_register_empty())
			m_sci_tx_cb(transmit_register_get_data_bit());
	}
}

void sh7014_sci_device::tra_complete()
{
	if (!(m_ssr & SSR_TDRE)) {
		update_tx_state();
		return;
	}

	LOGMASKED(LOG_TXRX, "transmit ended\n");

	m_ssr |= SSR_TEND;
	if (m_scr & SCR_TEIE)
		m_intc->set_interrupt(m_tei_int, ASSERT_LINE);
}

void sh7014_sci_device::update_tx_state()
{
	if (!(m_scr & SCR_TE) || (m_ssr & SSR_TDRE) || !is_transmit_register_empty())
		return;

	LOGMASKED(LOG_TXRX, "transmitting %02x\n", m_tdr);

	transmit_register_setup(m_tdr);

	m_ssr = (m_ssr & ~SSR_TEND) | SSR_TDRE;

	if (m_scr & SCR_TIE)
		m_intc->set_interrupt(m_txi_int, ASSERT_LINE);
}

uint8_t sh7014_sci_device::rdr_r()
{
	auto r = m_rdr;

	LOGMASKED(LOG_TXRX, "rdr_r%s %02x\n", m_is_dma_source_rx ? " (dma)" : "", m_rdr);

	// DMA reads cause RDRF to be cleared
	if (m_is_dma_source_rx)
		m_ssr &= ~SSR_RDRF;

	return r;
}

void sh7014_sci_device::rcv_complete()
{
	receive_register_extract();

	if (!(m_scr & SCR_RE))
		return;

	if (is_receive_framing_error())
		m_ssr |= SSR_FER;
	if (is_receive_parity_error())
		m_ssr |= SSR_PER;

	if (!(m_ssr & SSR_RDRF)) {
		m_rdr = get_received_char();
		m_ssr |= SSR_RDRF;

		if (m_scr & SCR_RIE)
			m_intc->set_interrupt(m_rxi_int, ASSERT_LINE);

		if ((m_ssr & SSR_FER) | (m_ssr & SSR_PER) || (m_ssr & SSR_ORER))
			m_intc->set_interrupt(m_eri_int, ASSERT_LINE);
	} else {
		m_ssr |= SSR_ORER;
	}
}

void sh7014_sci_device::update_data_format()
{
	if (m_smr & SMR_CA) {
		// Synchronous clock is a fixed 8-bit data length transmission
		set_data_frame(0, 8, PARITY_NONE, STOP_BITS_0);
		return;
	}

	// Async
	set_data_frame(
		1,
		(m_smr & SMR_CHR) ? 8 : 7,
		(m_smr & SMR_MP) ? PARITY_NONE : ((m_smr & SMR_PE) ? PARITY_ODD : PARITY_EVEN), // Multiprocessor mode does not use parity
		(m_smr & SMR_STOP) ? STOP_BITS_1 : STOP_BITS_2
	);
}

void sh7014_sci_device::update_clock()
{
	auto clock_mode = INTERNAL_SYNC_OUT;
	if (m_smr & SMR_CA) {
		if (m_scr & SCR_CKE1)
			clock_mode = EXTERNAL_SYNC;
		else
			clock_mode = INTERNAL_SYNC_OUT;
	} else {
		if (m_scr & SCR_CKE1)
			clock_mode = EXTERNAL_ASYNC;
		else if (m_scr & SCR_CKE0)
			clock_mode = INTERNAL_ASYNC_OUT;
		else
			clock_mode = INTERNAL_ASYNC;
	}

	if (clock_mode == EXTERNAL_ASYNC && !m_external_clock_period.is_never())
		clock_mode = EXTERNAL_RATE_ASYNC;
	else if (clock_mode == EXTERNAL_SYNC && !m_external_clock_period.is_never())
		clock_mode = EXTERNAL_RATE_SYNC;

	auto clock_speed = attotime::zero;
	switch (clock_mode) {
		case INTERNAL_ASYNC:
		case INTERNAL_ASYNC_OUT:
		case INTERNAL_SYNC_OUT:
		{
			int divider = (1 << (2 * (m_smr & SMR_CKS))) * (m_brr + 1);
			clock_speed = attotime::from_ticks(divider, clock());
		}
		break;

		case EXTERNAL_ASYNC:
		case EXTERNAL_RATE_ASYNC:
		case EXTERNAL_SYNC:
		case EXTERNAL_RATE_SYNC:
			clock_speed = m_external_clock_period;
			break;
	}

	if (clock_speed != m_clock_speed && !clock_speed.is_never()) {
		LOGMASKED(LOG_CLOCK, "Changing SCI ch %d rate set to %lf (%d %04x %04x)\n", m_channel_id, clock_speed.as_hz(), clock_mode, m_brr, m_smr);

		if (m_hack_set_full_data_transmit_on_sync && (m_smr & SMR_CA))
			set_rate(clock_speed * 8);
		else
			set_rate(clock_speed);

		m_clock_speed = clock_speed;
	}
}
