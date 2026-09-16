// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series Programmable Pulse Generator

***************************************************************************/

#include "emu.h"
#include "f2mc16_ppg.h"

namespace {

struct PPGC { enum : uint8_t
{
	PEN = 1 << 7,
	PCS = 1 << 6, // channel 1
	POE = 1 << 5,
	PIE = 1 << 4,
	PUF = 1 << 3,
	PCM = 3 << 1, // channel 0
	PCM_DIV1 = 0 << 1,
	PCM_DIV4 = 1 << 1,
	PCM_DIV16 = 2 << 1,
	PCM_TIMEBASE = 3 << 1,
	MD = 3 << 1, // channel 1
	MD_2_CHANNEL = 0 << 1,
	MD_SINGLE_CHANNEL_8BIT_PRESCALER = 1 << 1,
	MD_SINGLE_CHANNEL_16BIT = 3 << 1,
	RESERVED = 1 << 0
}; };

} // anonymous namespace

DEFINE_DEVICE_TYPE(F2MC16_PPG, f2mc16_ppg_device, "f2mc16_ppg", "F2MC16 Programmable Pulse Generator")

f2mc16_ppg_device::f2mc16_ppg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, required_device<f2mc16_intc_device> &intc, uint8_t ppg0_vector, uint8_t ppg1_vector) :
	f2mc16_ppg_device(mconfig, tag, owner, clock)
{
	m_cpu = downcast<f2mc16_device *>(owner);
	m_intc.set_tag(intc);
	m_vector[0] = ppg0_vector;
	m_vector[1] = ppg1_vector;
}

f2mc16_ppg_device::f2mc16_ppg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, F2MC16_PPG, tag, owner, clock),
	m_cpu(nullptr),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_vector{ 0,0 },
	m_output_cb(*this),
	m_peripheral_clock_hz(0),
	m_peripheral_clock_changed(attotime::zero),
	m_timebase_hz(0),
	m_timebase_changed(attotime::zero),
	m_clocksel(),
	m_hz(),
	m_duty(),
	m_pcnt(),
	m_pcntrl(),
	m_lh(),
	m_ppgc(),
	m_prll(),
	m_prlh()
{
}

void f2mc16_ppg_device::device_start()
{
	m_timer[0] = timer_alloc(FUNC(f2mc16_ppg_device::timer_callback<0>), this);
	m_timer[1] = timer_alloc(FUNC(f2mc16_ppg_device::timer_callback<1>), this);
	m_start_time[0] = attotime::never;
	m_start_time[1] = attotime::never;

	save_item(NAME(m_peripheral_clock_hz));
	save_item(NAME(m_timebase_hz));
	save_item(NAME(m_start_time));
	save_item(NAME(m_underflow_time));
	save_item(NAME(m_clocksel));
	save_item(NAME(m_hz));
	save_item(NAME(m_duty));
	save_item(NAME(m_pcnt));
	save_item(NAME(m_pcntrl));
	save_item(NAME(m_lh));
	save_item(NAME(m_ppgc));
	save_item(NAME(m_prll));
	save_item(NAME(m_prlh));
}

void f2mc16_ppg_device::device_clock_changed()
{
	if (machine().scheduler().currently_executing())
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(f2mc16_ppg_device::update_peripheral_clock), this), clock());
	else
		update_peripheral_clock(clock());
}

void f2mc16_ppg_device::device_reset()
{
	update_pcnt();

	for (int i = 0; i < 2; i++)
		m_ppgc[i] = PPGC::PUF | PPGC::RESERVED;

	update();
}

void f2mc16_ppg_device::timebase_hz(uint32_t hz)
{
	if (started() && machine().scheduler().currently_executing())
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(f2mc16_ppg_device::update_timebase_hz), this), hz);
	else
		update_timebase_hz(hz);
}

uint8_t f2mc16_ppg_device::ppgc_r(offs_t offset)
{
	if (!m_cpu->rmw() && m_underflow_time[offset] >= machine().time())
		return m_ppgc[offset] & ~PPGC::PUF;

	return m_ppgc[offset];
}

void f2mc16_ppg_device::ppgc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t ppgc = (m_ppgc[1] << 8) | m_ppgc[0];
	uint16_t prev = ppgc;
	COMBINE_DATA(&ppgc);

	if (ppgc != prev)
	{
		update_pcnt();

		m_ppgc[0] = ppgc >> 0;
		m_ppgc[1] = ppgc >> 8;

		if (ACCESSING_BITS_0_7 && !(m_ppgc[0] & PPGC::PUF) && m_underflow_time[0] <= machine().time())
		{
			m_ppgc[0] |= PPGC::PUF;
			m_underflow_time[0] = attotime::never;
		}

		if (ACCESSING_BITS_8_15 && !(m_ppgc[1] & PPGC::PUF) && m_underflow_time[1] <= machine().time())
		{
			m_ppgc[1] |= PPGC::PUF;
			m_underflow_time[1] = attotime::never;
		}

		update();
	}
}

template<unsigned N>
uint16_t f2mc16_ppg_device::prl_r()
{
	return (m_prlh[N] << 8) | m_prll[N];
}

template<unsigned N>
void f2mc16_ppg_device::prl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t prl = (m_prlh[N] << 8) | m_prll[N];
	uint16_t prev = prl;
	COMBINE_DATA(&prl);
	if (prl != prev)
	{
		update_pcnt();

		m_prll[N] = prl >> 0;
		m_prlh[N] = prl >> 8;

		update();
	}
}

void f2mc16_ppg_device::update()
{
	attotime now = machine().time();

	for (int i = 0; i < 2; i++)
	{
		bool start = false;

		if (!(m_ppgc[i] & PPGC::PEN))
			m_start_time[i] = attotime::never;
		else if (m_start_time[i].is_never())
		{
			m_start_time[i] = now;
			start = true;
		}

		uint8_t pcm = (i == 0 || (m_ppgc[1] & PPGC::MD) != PPGC::MD_2_CHANNEL) ?
			(m_ppgc[0] & PPGC::PCM) :
			(m_ppgc[1] & PPGC::PCS) ? PPGC::PCM_TIMEBASE : PPGC::PCM_DIV1;

		m_clocksel[i] = m_start_time[i].is_never() ? 0 :
			(pcm == PPGC::PCM_DIV1) ? m_peripheral_clock_hz :
			(pcm == PPGC::PCM_DIV4) ? m_peripheral_clock_hz / 4 :
			(pcm == PPGC::PCM_DIV16) ? m_peripheral_clock_hz / 16 :
			(pcm == PPGC::PCM_TIMEBASE) ? m_timebase_hz :
			0;

		if ((m_ppgc[1] & PPGC::MD) == PPGC::MD_2_CHANNEL)
		{
			m_pcntrl[i][0] = m_prll[i] + 1;
			m_pcntrl[i][1] = m_prlh[i] + 1;
		}
		else if ((m_ppgc[1] & PPGC::MD) == PPGC::MD_SINGLE_CHANNEL_8BIT_PRESCALER)
		{
			if (i == 0)
			{
				m_pcntrl[i][0] = m_prll[i] + 1;
				m_pcntrl[i][1] = m_prlh[i] + 1;
			}
			else
			{
				uint16_t prescale = (m_prll[0] + 1) + (m_prlh[0] + 1);
				m_pcntrl[i][0] = (m_prll[i] + 1) * prescale;
				m_pcntrl[i][1] = (m_prlh[i] + 1) * prescale;
			}
		}
		else if ((m_ppgc[1] & PPGC::MD) == PPGC::MD_SINGLE_CHANNEL_16BIT)
		{
			m_pcntrl[i][0] = ((m_prll[1] << 8) | m_prll[0]) + 1;
			m_pcntrl[i][1] = ((m_prlh[1] << 8) | m_prlh[0]) + 1;
		}
		else
		{
			m_pcntrl[i][0] = 0;
			m_pcntrl[i][1] = 0;
		}

		if (start)
			m_pcnt[i][m_lh[i]] = m_pcntrl[i][m_lh[i]];

		m_pcnt[i][!m_lh[i]] = m_pcntrl[i][!m_lh[i]];

		attotime event_time = attotime::never;

		attotime puf = m_start_time[i].is_never() || !m_clocksel[i] ? attotime::never :
			m_start_time[i] + attotime::from_ticks(m_pcnt[i][m_lh[i]], m_clocksel[i]);

		if (m_underflow_time[i] > now)
		{
			m_underflow_time[i] = puf;

			if ((m_ppgc[i] & PPGC::PIE) && event_time > puf)
				event_time = puf;
		}

		uint32_t total = (m_ppgc[i] & PPGC::POE) ? m_pcntrl[i][0] + m_pcntrl[i][1] : 0;
		uint32_t hz = total ? m_clocksel[i] / total : 0;
		uint32_t duty = hz ? (0x100000000ULL * m_pcntrl[i][1]) / total : 0;

		if (m_hz[i] != hz || m_duty[i] != duty)
		{
			if ((m_lh[i] || m_pcnt[i][0] != m_pcntrl[i][0] || m_pcnt[i][1] != m_pcntrl[i][1]) && hz)
			{
				if (event_time > puf)
					event_time = puf;
			}
			else
			{
				if (!m_output_cb[i].isunset())
					m_output_cb[i](0, hz, duty);
				else if ((m_ppgc[1] & PPGC::MD) != PPGC::MD_SINGLE_CHANNEL_8BIT_PRESCALER || (m_output_cb[0].isunset() && m_output_cb[1].isunset()))
					logerror("%s PPG%d unmapped write %d, %d, %08x\n", machine().describe_context(), i, 0, hz, duty);

				m_hz[i] = hz;
				m_duty[i] = duty;
			}
		}

		m_timer[i]->adjust(event_time.is_never() ? event_time : event_time - now);
		m_intc->set_irq(m_vector[i], ((m_ppgc[i] & PPGC::PIE) && m_underflow_time[i] <= now) ? 1 : 0);
	}
}

void f2mc16_ppg_device::update_pcnt()
{
	for (int i = 0; i < 2; i++)
	{
		if (m_start_time[i] != attotime::never)
		{
			attotime now = machine().time();
			int64_t ticks = (now - m_start_time[i]).as_ticks(m_clocksel[i]);

			for (int p = 0; p < 4 && ticks; p++)
			{
				if (m_pcnt[i][m_lh[i]] > ticks)
				{
					m_pcnt[i][m_lh[i]] -= ticks;
					ticks = 0;
				}
				else
				{
					ticks -= m_pcnt[i][m_lh[i]];
					m_pcnt[i][m_lh[i]] = m_pcntrl[i][m_lh[i]];
					m_lh[i] = !m_lh[i];
				}

				if (p == 1 && (m_pcntrl[i][0] + m_pcntrl[i][1]))
					ticks %= (m_pcntrl[i][0] + m_pcntrl[i][1]);
			}

			m_start_time[i] = now;
		}
	}
}

TIMER_CALLBACK_MEMBER(f2mc16_ppg_device::update_timebase_hz)
{
	update_pcnt();

	m_timebase_hz = param;

	if (started())
	{
		m_timebase_changed = machine().time();

		update();
	}
}

TIMER_CALLBACK_MEMBER(f2mc16_ppg_device::update_peripheral_clock)
{
	update_pcnt();

	m_peripheral_clock_hz = param;
	m_peripheral_clock_changed = machine().time();

	update();
}

template <unsigned N>
TIMER_CALLBACK_MEMBER(f2mc16_ppg_device::timer_callback)
{
	m_lh[N] = !m_lh[N];
	m_pcnt[N][m_lh[N]] = m_pcntrl[N][m_lh[N]];
	m_start_time[N] = machine().time();

	update();
}

template uint16_t f2mc16_ppg_device::prl_r<0>();
template uint16_t f2mc16_ppg_device::prl_r<1>();
template void f2mc16_ppg_device::prl_w<0>(offs_t offset, uint16_t data, uint16_t mem_mask);
template void f2mc16_ppg_device::prl_w<1>(offs_t offset, uint16_t data, uint16_t mem_mask);
