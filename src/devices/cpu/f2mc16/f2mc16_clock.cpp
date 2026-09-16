// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series Clock Generator

***************************************************************************/

#include "emu.h"
#include "f2mc16_clock.h"

namespace {

struct CKSCR { enum : uint8_t
{
	RESERVED1 = 1 << 7,
	MCM = 1 << 6,
	WS = 3 << 4,
	RESERVED2 = 1 << 3,
	MCS = 1 << 2,
	CS = 3 << 0
}; };

struct LPMCR { enum : uint8_t
{
	RESERVED3 = (1 << 0),
	CG = 3 << 1,
	RESERVED4 = (1 << 3),
	RST = 1 << 4,
	SPL = 1 << 5,
	SLP = 1 << 6,
	STP = 1 << 7
}; };

struct TBTC { enum : uint8_t
{
	TEST = 1 << 7,
	TBIE = 1 << 4,
	TBOF = 1 << 3,
	TBR = 1 << 2,
	TBC = 3 << 0,
	TBC_2_12 = 0 << 0,
	TBC_2_14 = 1 << 0,
	TBC_2_16 = 2 << 0,
	TBC_2_19 = 3 << 0,
}; };

struct WDTC { enum : uint8_t
{
	WT = 3 << 0,
	WTE = 1 << 2,
	SRST = 1 << 3,
	ERST = 1 << 4,
	WRST = 1 << 5,
	STBR = 1 << 6,
	PONR = 1 << 7
}; };

} // anonymous namespace

DEFINE_DEVICE_TYPE(F2MC16_CLOCK_GENERATOR, f2mc16_clock_generator_device, "f2mc16_clock_generator", "F2MC16 Clock Generator")

f2mc16_clock_generator_device::f2mc16_clock_generator_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, required_device<f2mc16_intc_device> &intc, uint8_t tbtc_vector) :
	f2mc16_clock_generator_device(mconfig, tag, owner, clock)
{
	m_cpu = downcast<f2mc16_device *>(owner);
	m_intc.set_tag(intc);
	m_tbtc_vector = tbtc_vector;
}

f2mc16_clock_generator_device::f2mc16_clock_generator_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, F2MC16_CLOCK_GENERATOR, tag, owner, clock),
	m_cpu(nullptr),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_tbtc_vector(0),
	m_timebase_hz_cb(*this),
	m_timebase_start_time(attotime::zero),
	m_tbtc_overflow_time(attotime::never),
	m_watchdog_overflow_time(attotime::never),
	m_mainclock_changed(attotime::zero),
	m_timebase_counter(0),
	m_tbtc_hz(0),
	m_mainclock_hz(0),
	m_reset_reason(WDTC::PONR),
	m_watchdog_overflow_count(0),
	m_wt(0),
	m_ckscr(0),
	m_lpmcr(0),
	m_tbtc(0),
	m_wdtc(WDTC::WTE | WDTC::WT)
{
}

void f2mc16_clock_generator_device::device_start()
{
	m_timebase_timer = timer_alloc(FUNC(f2mc16_clock_generator_device::timebase_timer_callback), this);

	save_item(NAME(m_timebase_start_time));
	save_item(NAME(m_tbtc_overflow_time));
	save_item(NAME(m_mainclock_changed));
	save_item(NAME(m_timebase_counter));
	save_item(NAME(m_tbtc_hz));
	save_item(NAME(m_mainclock_hz));
	save_item(NAME(m_reset_reason));
	save_item(NAME(m_ckscr));
	save_item(NAME(m_lpmcr));
	save_item(NAME(m_tbtc));
	save_item(NAME(m_wdtc));
}

void f2mc16_clock_generator_device::device_clock_changed()
{
	if (machine().scheduler().currently_executing())
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(f2mc16_clock_generator_device::update_mainclock_hz), this), m_cpu->unscaled_clock());
	else
		update_mainclock_hz(m_cpu->unscaled_clock());
}

void f2mc16_clock_generator_device::device_reset()
{
	if (!m_reset_reason)
		m_reset_reason = WDTC::ERST;

	m_ckscr = ~CKSCR::MCS;
	m_lpmcr = LPMCR::RESERVED4 | LPMCR::RESERVED3 | LPMCR::RST;
	m_wdtc |= m_reset_reason | WDTC::WTE | WDTC::WT;
	m_tbtc = TBTC::TEST | TBTC::TBR;
	m_reset_reason = 0;
	m_watchdog_overflow_time = attotime::never;

	timebase_reset();
	timebase_update();
}

uint8_t f2mc16_clock_generator_device::ckscr_r()
{
	return m_ckscr;
}

void f2mc16_clock_generator_device::ckscr_w(uint8_t data)
{
	data = (m_ckscr & ~(CKSCR::WS | CKSCR::MCS | CKSCR::CS)) | (data & (CKSCR::WS | CKSCR::MCS | CKSCR::CS));

	if (m_ckscr != data)
	{
		if (!(m_ckscr & CKSCR::MCS) && (data & CKSCR::MCS))
		{
			timebase_reset();
			timebase_update();
		}

		m_ckscr = data;

		m_cpu->set_clock_scale((m_ckscr & CKSCR::MCS) ? 1 : (m_ckscr & CKSCR::CS) + 1);
	}
}

uint8_t f2mc16_clock_generator_device::lpmcr_r()
{
	return m_lpmcr & (LPMCR::SPL | LPMCR::CG);
}

void f2mc16_clock_generator_device::lpmcr_w(uint8_t data)
{
	data |= LPMCR::RESERVED4 | LPMCR::RESERVED3;

	if (m_lpmcr != data)
	{
		m_lpmcr = data;

		if (!(m_lpmcr & LPMCR::RST))
		{
			m_lpmcr |= LPMCR::RST;
			m_reset_reason |= WDTC::SRST;
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(f2mc16_clock_generator_device::soft_reset), this));
		}
	}
}

TIMER_CALLBACK_MEMBER(f2mc16_clock_generator_device::soft_reset)
{
	m_cpu->reset();
}

uint8_t f2mc16_clock_generator_device::wdtc_r()
{
	uint8_t data = m_wdtc;

	if (!machine().side_effects_disabled())
		m_wdtc &= ~(WDTC::PONR | WDTC::STBR | WDTC::WRST | WDTC::SRST);

	return data | WDTC::WTE | WDTC::WT;
}

void f2mc16_clock_generator_device::wdtc_w(uint8_t data)
{
	uint8_t prev = m_wdtc;
	m_wdtc = data & (m_wdtc | ~WDTC::WTE);

	if (!(data & WDTC::WTE))
	{
		m_watchdog_overflow_count = 0;

		if (prev & WDTC::WTE)
		{
			m_wt = m_wdtc & WDTC::WT;

			timebase_update_counter();
			timebase_update();
		}
	}
}

uint8_t f2mc16_clock_generator_device::tbtc_r()
{
	if (!m_cpu->rmw() && m_tbtc_overflow_time > machine().time())
		return m_tbtc & ~TBTC::TBOF;

	return m_tbtc;
}

void f2mc16_clock_generator_device::tbtc_w(uint8_t data)
{
	if (m_tbtc != data)
	{
		timebase_update_counter();

		m_tbtc = data;

		if (!(data & TBTC::TBR))
		{
			timebase_reset();
			m_tbtc |= TBTC::TBR;
		}

		if (!(data & TBTC::TBOF))
		{
			if (m_tbtc_overflow_time <= machine().time())
				m_tbtc_overflow_time = attotime::never;

			m_tbtc |= TBTC::TBOF;
		}

		timebase_update();
	}
}

void f2mc16_clock_generator_device::timebase_reset()
{
	m_timebase_counter = 0;
	m_timebase_start_time = attotime::never;
	m_tbtc_overflow_time = attotime::never;
	m_tbtc_hz = 0;
}

void f2mc16_clock_generator_device::timebase_update()
{
	static const int cycles[4] = { 1 << 12, 1 << 14, 1 << 16, 1 << 19 };
	attotime now = machine().time();

	if (m_timebase_start_time == attotime::never)
		m_timebase_start_time = now;

	uint32_t event_ticks = 0;
	attotime event_time = attotime::never;

	if (m_tbtc_overflow_time > now)
	{
		uint32_t tbtc_ticks = cycles[m_tbtc & TBTC::TBC] - (m_timebase_counter & (cycles[m_tbtc & TBTC::TBC] - 1));
		m_tbtc_overflow_time = m_timebase_start_time.is_never() || !m_mainclock_hz ? attotime::never :
			m_timebase_start_time + attotime::from_ticks(tbtc_ticks, m_mainclock_hz);

		if ((m_tbtc & TBTC::TBIE) && event_time > m_tbtc_overflow_time)
		{
			event_time = m_tbtc_overflow_time;
			event_ticks = tbtc_ticks;
		}
	}

	if (!(m_wdtc & WDTC::WTE))
	{
		if (m_watchdog_overflow_time < now)
		{
			m_watchdog_overflow_count++;
			if (m_watchdog_overflow_count == 4)
			{
				m_reset_reason |= WDTC::WRST;
				m_cpu->reset();
			}
		}

		uint32_t watchdog_ticks = cycles[m_wt] - (m_timebase_counter & (cycles[m_wt] - 1));
		attotime wdof = m_timebase_start_time.is_never() || !m_mainclock_hz ? attotime::never :
			m_timebase_start_time + attotime::from_ticks(watchdog_ticks, m_mainclock_hz);
		m_watchdog_overflow_time = wdof;

		if (event_time > wdof)
		{
			event_time = wdof;
			event_ticks = watchdog_ticks;
		}
	}

	if (m_tbtc_hz != m_mainclock_hz && !m_timebase_hz_cb.isunset())
	{
		if ((m_timebase_counter % 512) != 0 && m_mainclock_hz)
		{
			uint32_t sync_ticks = 512 - (m_timebase_counter & (512 - 1));
			attotime sync_time = m_timebase_start_time.is_never() || !m_mainclock_hz ? attotime::never :
				m_timebase_start_time + attotime::from_ticks(sync_ticks, m_mainclock_hz);

			if (event_time > sync_time)
			{
				event_time = sync_time;
				event_ticks = sync_ticks;
			}
		}
		else
		{
			m_timebase_hz_cb(m_mainclock_hz / 512);
			m_tbtc_hz = m_mainclock_hz;
		}
	}

	m_timebase_timer->adjust(event_time.is_never() ? event_time : event_time - now, event_ticks);
	m_intc->set_irq(m_tbtc_vector, ((m_tbtc & TBTC::TBIE) && m_tbtc_overflow_time <= now) ? 1 : 0);
}

void f2mc16_clock_generator_device::timebase_update_counter()
{
	attotime now = machine().time();
	uint64_t ticks = ((now - m_timebase_start_time) + attotime::zero).as_ticks(m_mainclock_hz);
	m_timebase_counter += ticks;
	m_timebase_counter &= 0x3ffff;
	m_timebase_start_time = now;
}

TIMER_CALLBACK_MEMBER(f2mc16_clock_generator_device::update_mainclock_hz)
{
	timebase_update_counter();

	m_mainclock_hz = param;
	m_mainclock_changed = machine().time();

	timebase_update();
}

TIMER_CALLBACK_MEMBER(f2mc16_clock_generator_device::timebase_timer_callback)
{
	m_timebase_counter += param;
	m_timebase_counter &= 0x3ffff;
	m_timebase_start_time = machine().time();

	timebase_update();
}
