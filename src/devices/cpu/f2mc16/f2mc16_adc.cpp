// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series ADC

***************************************************************************/

#include "emu.h"
#include "f2mc16_adc.h"

namespace {

struct ADCS { enum : uint16_t
{
	ANE = 7 << 0,
	ANS = 7 << 3,
	MD = 3 << 6,
	MD_SINGLE_REACTIVATION = 0 << 6,
	MD_SINGLE = 1 << 6,
	MD_CONTINUOUS = 2 << 6,
	MD_STOP = 3 << 6,
	RESERVED = 1 << 8,
	STRT = 1 << 9,
	STS = 3 << 10,
	STS_ATGX = 1 << 10,
	STS_TIMER = 2 << 10,
	PAUS = 1 << 12,
	INTE = 1 << 13,
	INT = 1 << 14,
	BUSY = 1 << 15
}; };

struct ADCR { enum : uint16_t
{
	D07 = 0xff << 0,
	S10 = 1 << 15,
	D89 = 3 << 8
}; };

} // anonymous namespace

DEFINE_DEVICE_TYPE(F2MC16_ADC, f2mc16_adc_device, "f2mc16_adc", "F2MC16 ADC")

f2mc16_adc_device::f2mc16_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, required_device<f2mc16_intc_device> &intc, uint8_t vector) :
	f2mc16_adc_device(mconfig, tag, owner, clock)
{
	m_intc.set_tag(intc);
	m_vector = vector;
}

f2mc16_adc_device::f2mc16_adc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, F2MC16_ADC, tag, owner, clock),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_channel_cb(*this, 0x000),
	m_peripheral_clock_changed(attotime::zero),
	m_internal_timer_changed(attotime::zero),
	m_internal_timer_expired(attotime::never),
	m_conversion_start_time(attotime::never),
	m_conversion_finished(attotime::never),
	m_peripheral_clock_hz(0),
	m_internal_timer_hz(0),
	m_conversion_ticks(0),
	m_channel(-1),
	m_atgx(1),
	m_i2osclr(0),
	m_adcs(0),
	m_adcr(0)
{
}

void f2mc16_adc_device::device_start()
{
	m_timer = timer_alloc(FUNC(f2mc16_adc_device::timer_callback), this);

	save_item(NAME(m_peripheral_clock_changed));
	save_item(NAME(m_internal_timer_changed));
	save_item(NAME(m_internal_timer_expired));
	save_item(NAME(m_conversion_start_time));
	save_item(NAME(m_conversion_finished));
	save_item(NAME(m_peripheral_clock_hz));
	save_item(NAME(m_internal_timer_hz));
	save_item(NAME(m_conversion_ticks));
	save_item(NAME(m_channel));
	save_item(NAME(m_atgx));
	save_item(NAME(m_i2osclr));
	save_item(NAME(m_adcs));
	save_item(NAME(m_adcr));
}

void f2mc16_adc_device::device_reset()
{
	m_adcs = 0;
	m_adcr &= ~ADCR::S10;
	update();
}

void f2mc16_adc_device::device_clock_changed()
{
	if (machine().scheduler().currently_executing())
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(f2mc16_adc_device::update_peripheral_clock), this), clock());
	else
		update_peripheral_clock(clock());
}

void f2mc16_adc_device::i2osclr(int state)
{
	if (state && !m_i2osclr)
	{
		m_adcs &= ~ADCS::INT;

		update();
	}

	m_i2osclr = state;
}

void f2mc16_adc_device::internal_timer_hz(uint32_t hz)
{
	if (machine().scheduler().currently_executing())
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(f2mc16_adc_device::update_internal_timer), this), hz);
	else
		update_internal_timer(hz);
}

void f2mc16_adc_device::atgx(int state)
{
	if (!m_atgx && state && (m_adcs & ADCS::STS_ATGX))
	{
		trigger();
		update();
	}

	m_atgx = state;
}

uint16_t f2mc16_adc_device::adcs_r()
{
	return m_adcs;
}

void f2mc16_adc_device::adcs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t prev = m_adcs;
	COMBINE_DATA(&m_adcs);

	if (ACCESSING_BITS_8_15)
	{
		m_adcs = m_adcs & (prev | ~(ADCS::BUSY | ADCS::INT | ADCS::PAUS));
		if (m_adcs != prev)
		{
			if (m_adcs & ADCS::STRT)
			{
				m_adcs &= ~ADCS::STRT;
				trigger();
			}

			update();
		}
	}
}

uint16_t f2mc16_adc_device::adcr_r()
{
	if (!machine().side_effects_disabled())
	{
		m_adcs &= ~ADCS::PAUS;

		if ((m_adcs & ADCS::BUSY) && (m_adcs & ADCS::MD) != ADCS::MD_STOP)
			start_conversion();

		update();
	}

	return m_adcr;
}

void f2mc16_adc_device::adcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
		m_adcr = (m_adcr & ~ADCR::S10) | (data & ADCR::S10);
}

void f2mc16_adc_device::trigger()
{
	if (!(m_adcs & ADCS::BUSY) || (m_adcs & ADCS::MD) == ADCS::MD_SINGLE_REACTIVATION)
	{
		m_channel = (m_adcs & ADCS::ANS) >> 3;
		m_adcs |= ADCS::BUSY;
		start_conversion();
	}
	else if ((m_adcs & ADCS::MD) == ADCS::MD_STOP)
		start_conversion();
}

TIMER_CALLBACK_MEMBER(f2mc16_adc_device::update_peripheral_clock)
{
	if (m_adcs & ADCS::BUSY)
		update_conversion_ticks();

	m_peripheral_clock_hz = param;
	m_peripheral_clock_changed = machine().time();

	if (m_adcs & ADCS::BUSY)
		update();
}

TIMER_CALLBACK_MEMBER(f2mc16_adc_device::update_internal_timer)
{
	m_internal_timer_hz = param;
	m_internal_timer_changed = machine().time();

	if (m_adcs & ADCS::STS_TIMER)
		update();
}

void f2mc16_adc_device::start_conversion()
{
	m_conversion_ticks = 98;
}

void f2mc16_adc_device::update_conversion_ticks()
{
	if (!m_conversion_start_time.is_never())
	{
		attotime now = machine().time();
		if (m_peripheral_clock_hz)
			m_conversion_ticks -= ((now - m_conversion_start_time) + attotime::zero).as_ticks(m_peripheral_clock_hz);
		m_conversion_start_time = now;
	}
}

void f2mc16_adc_device::update()
{
	attotime now = machine().time();

	attotime event_time = attotime::never;

	if (m_conversion_finished <= now)
	{
		uint16_t data = m_channel_cb[m_channel]();
		if (m_channel_cb[m_channel].isunset())
			logerror("unmapped read AN%d\n", m_channel);

		if (m_adcr & ADCR::S10)
			m_adcr = (ADCR::S10) | data >> 2;
		else
			m_adcr = data;

		m_adcs |= ADCS::PAUS | ADCS::INT;

		if (m_channel == (m_adcs & ADCS::ANE))
		{
			if ((m_adcs & ADCS::MD) == ADCS::MD_CONTINUOUS || (m_adcs & ADCS::MD) == ADCS::MD_STOP)
				m_channel = (m_adcs & ADCS::ANS) >> 3;
			else
				m_adcs &= ~ADCS::BUSY;
		}
		else
			m_channel = (m_channel + 1) & 7;

		m_conversion_ticks = 0;
		m_conversion_start_time = attotime::never;
	}

	if (!(m_adcs & ADCS::BUSY) && m_channel >= 0)
	{
		m_channel = -1;
		m_conversion_ticks = 0;
		m_conversion_start_time = attotime::never;
	}

	if (m_conversion_start_time.is_never() && m_conversion_ticks && m_peripheral_clock_hz)
	{
		m_conversion_start_time = m_peripheral_clock_changed + attotime::from_ticks((now - m_peripheral_clock_changed).as_ticks(m_peripheral_clock_hz) + 1, m_peripheral_clock_hz);
		if (m_conversion_start_time <= now)
			m_conversion_start_time += attotime::from_ticks(1, m_peripheral_clock_hz);
	}

	m_conversion_finished = m_conversion_start_time.is_never() || !m_peripheral_clock_hz ? attotime::never :
		m_conversion_start_time + attotime::from_ticks(m_conversion_ticks, m_peripheral_clock_hz);

	if (event_time > m_conversion_finished)
		event_time = m_conversion_finished;

	if (m_internal_timer_expired <= now)
		trigger();

	m_internal_timer_expired = !(m_adcs & ADCS::STS_TIMER) || !m_internal_timer_hz ? attotime::never :
		m_internal_timer_changed + attotime::from_ticks((now - m_internal_timer_changed).as_ticks(m_internal_timer_hz) + 1, m_internal_timer_hz);
	if (m_internal_timer_expired <= now)
		m_internal_timer_expired += attotime::from_ticks(1, m_internal_timer_hz);

	if (event_time > m_internal_timer_expired)
		event_time = m_internal_timer_expired;

	m_timer->adjust(event_time.is_never() ? event_time : event_time - now);
	m_intc->set_irq(m_vector, (m_adcs & ADCS::INTE) && (m_adcs & ADCS::INT));
}

TIMER_CALLBACK_MEMBER(f2mc16_adc_device::timer_callback)
{
	update();
}
