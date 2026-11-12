// license:BSD-3-Clause
// copyright-holders:superctr
/***************************************************************************

  SH7016/SH7017 mid-speed A/D converter

  Four multiplexed inputs, one 10 bit result register each, left aligned in
  the upper ten bits.  Single mode converts the channel ADCSR names and stops;
  scan mode sweeps channel 0 up to that channel and restarts until ADST is
  cleared.  A conversion takes 266 states, or 134 with CKS set.

  TODO list (not comprehensive):
  - external trigger (ADCR TRGE) and the MTU's conversion start request
  - the SH7014's own high speed converter is a different module

***************************************************************************/

#include "emu.h"
#include "sh7014_adc.h"

#define LOG_READ (1U << 1)
#define LOG_WRITE (1U << 2)

// #define VERBOSE (LOG_GENERAL | LOG_READ | LOG_WRITE)

#include "logmacro.h"


DEFINE_DEVICE_TYPE(SH7014_ADC, sh7014_adc_device, "sh7014adc", "SH7014 A/D Converter")


sh7014_adc_device::sh7014_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7014_ADC, tag, owner, clock)
	, m_intc(*this, finder_base::DUMMY_TAG)
	, m_analog_cb(*this, 0)
{
}

void sh7014_adc_device::device_start()
{
	m_timer = timer_alloc(FUNC(sh7014_adc_device::conversion_done), this);

	save_item(NAME(m_addr));
	save_item(NAME(m_adcsr));
	save_item(NAME(m_adcr));
	save_item(NAME(m_int_state));
}

void sh7014_adc_device::device_reset()
{
	std::fill(std::begin(m_addr), std::end(m_addr), 0);
	m_adcsr = 0;
	m_adcr = 0x7f;
	m_int_state = false;
	m_timer->adjust(attotime::never);
}

void sh7014_adc_device::map(address_map &map)
{
	map(0x00, 0x07).r(FUNC(sh7014_adc_device::addr_r));
	map(0x08, 0x08).rw(FUNC(sh7014_adc_device::adcsr_r), FUNC(sh7014_adc_device::adcsr_w));
	map(0x09, 0x09).rw(FUNC(sh7014_adc_device::adcr_r), FUNC(sh7014_adc_device::adcr_w));
}

uint16_t sh7014_adc_device::addr_r(offs_t offset)
{
	LOGMASKED(LOG_READ, "addr_r %d %04x\n", offset & 3, m_addr[offset & 3]);
	return m_addr[offset & 3];
}

uint8_t sh7014_adc_device::adcsr_r()
{
	LOGMASKED(LOG_READ, "adcsr_r %02x\n", m_adcsr);
	return m_adcsr;
}

void sh7014_adc_device::adcsr_w(uint8_t data)
{
	LOGMASKED(LOG_WRITE, "adcsr_w %02x\n", data);

	const bool was_running = m_adcsr & ADCSR_ADST;

	// ADF only clears, and only by a zero written over a one
	m_adcsr = (m_adcsr & data & ADCSR_ADF) | (data & ~ADCSR_ADF);
	update_interrupt();

	if ((m_adcsr & ADCSR_ADST) && !was_running)
		start_conversion();
	else if (!(m_adcsr & ADCSR_ADST))
		m_timer->adjust(attotime::never);
}

uint8_t sh7014_adc_device::adcr_r()
{
	return m_adcr;
}

void sh7014_adc_device::adcr_w(uint8_t data)
{
	LOGMASKED(LOG_WRITE, "adcr_w %02x\n", data);
	m_adcr = data | 0x7f;
}

void sh7014_adc_device::start_conversion()
{
	const int channels = (m_adcsr & ADCSR_SCAN) ? (m_adcsr & ADCSR_CH) + 1 : 1;
	m_timer->adjust(attotime::from_ticks(((m_adcsr & ADCSR_CKS) ? 134 : 266) * channels, clock()));
}

void sh7014_adc_device::update_interrupt()
{
	const bool state = (m_adcsr & (ADCSR_ADF | ADCSR_ADIE)) == (ADCSR_ADF | ADCSR_ADIE);
	if (state != m_int_state)
	{
		m_int_state = state;
		m_intc->set_interrupt(sh7014_intc_device::INT_VECTOR_AD, state ? ASSERT_LINE : CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(sh7014_adc_device::conversion_done)
{
	const int last = m_adcsr & ADCSR_CH;
	if (m_adcsr & ADCSR_SCAN)
	{
		for (int channel = 0; channel <= last; channel++)
			m_addr[channel] = (m_analog_cb[channel]() & 0x3ff) << 6;
		start_conversion();
	}
	else
	{
		m_addr[last] = (m_analog_cb[last]() & 0x3ff) << 6;
		m_adcsr &= ~ADCSR_ADST;
	}

	m_adcsr |= ADCSR_ADF;
	update_interrupt();
}
