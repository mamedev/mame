// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujitsu Micro F2MC-16 series 16-bit timer

***************************************************************************/

#include "emu.h"
#include "f2mc16_reload.h"

namespace {

struct TMCSR { enum : uint16_t
{
	CSL = 3 << 10,
	CSL_DIV2 = 0 << 10,
	CSL_DIV8 = 1 << 10,
	CSL_DIV32 = 2 << 10,
	CSL_EXTERNAL = 3 << 10,
	MOD2 = 1 << 9,
	MOD1 = 1 << 8,
	MOD0 = 1 << 7,
	MOD_INT = 7 << 7,
	MOD_EXT = 3 << 7,
	MOD_DISABLE = 0 << 7,
	MOD_RISING = 1 << 7,
	MOD_FALLING = 2 << 7,
	MOD_BOTH = 3 << 7,
	MOD_GATE = 5 << 7,
	MOD_GATE_LOW = 4 << 7,
	MOD_GATE_HIGH = 5 << 7,
	OUTE = 1 << 6,
	OUTL = 1 << 5,
	RELD = 1 << 4,
	INTE = 1 << 3,
	UF = 1 << 2,
	CNTE = 1 << 1,
	TRG = 1 << 0
}; };

} // anonymous namespace

DEFINE_DEVICE_TYPE(F2MC16_RELOAD_TIMER, f2mc16_reload_timer_device, "f2mc16_reload_timer", "F2MC16 16-bit reload timer")

f2mc16_reload_timer_device::f2mc16_reload_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, required_device<f2mc16_intc_device> &intc, uint8_t vector) :
	f2mc16_reload_timer_device(mconfig, tag, owner, clock)
{
	m_cpu = downcast<f2mc16_device *>(owner);
	m_intc.set_tag(intc);
	m_vector = vector;
}

f2mc16_reload_timer_device::f2mc16_reload_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, F2MC16_RELOAD_TIMER, tag, owner, clock),
	m_cpu(nullptr),
	m_intc(*this, finder_base::DUMMY_TAG),
	m_vector(0),
	m_internal_hz_cb(*this),
	m_tot_hz_cb(*this),
	m_peripheral_clock_changed(attotime::zero),
	m_tin_changed(attotime::zero),
	m_start_time(attotime::never),
	m_underflow_time(attotime::never),
	m_clocksel(0),
	m_peripheral_clock_hz(0),
	m_tin_hz(0),
	m_internal_hz(0),
	m_tot_hz(0),
	m_tin(1),
	m_i2osclr(0),
	m_tmcsr(TMCSR::UF),
	m_tmr(0),
	m_tmrlr(0)
{
}

void f2mc16_reload_timer_device::device_start()
{
	m_timer = timer_alloc(FUNC(f2mc16_reload_timer_device::timer_callback), this);

	save_item(NAME(m_peripheral_clock_changed));
	save_item(NAME(m_tin_changed));
	save_item(NAME(m_start_time));
	save_item(NAME(m_underflow_time));
	save_item(NAME(m_clocksel));
	save_item(NAME(m_peripheral_clock_hz));
	save_item(NAME(m_tin_hz));
	save_item(NAME(m_internal_hz));
	save_item(NAME(m_tot_hz));
	save_item(NAME(m_tin));
	save_item(NAME(m_i2osclr));
	save_item(NAME(m_tmcsr));
	save_item(NAME(m_tmr));
	save_item(NAME(m_tmrlr));
}

void f2mc16_reload_timer_device::device_clock_changed()
{
	if (machine().scheduler().currently_executing())
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(f2mc16_reload_timer_device::update_peripheral_clock), this), clock());
	else
		update_peripheral_clock(clock());
}

void f2mc16_reload_timer_device::device_reset()
{
	update_tmr();
	m_tmcsr = TMCSR::UF;
	update();
}

void f2mc16_reload_timer_device::tin_hz(uint32_t hz)
{
	if (started() && machine().scheduler().currently_executing())
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(f2mc16_reload_timer_device::update_tin_hz), this), hz);
	else
		update_tin_hz(hz);
}

void f2mc16_reload_timer_device::tin(int state)
{
	bool rising_edge = (state && !m_tin);
	bool falling_edge = (!state && m_tin);

	if (m_tin_hz == 0 && (m_tmcsr & TMCSR::CNTE) && (rising_edge || falling_edge))
	{
		if ((m_tmcsr & TMCSR::CSL) == TMCSR::CSL_EXTERNAL)
		{
			if ((((m_tmcsr & TMCSR::MOD_EXT) == TMCSR::MOD_RISING) && rising_edge) ||
				(((m_tmcsr & TMCSR::MOD_EXT) == TMCSR::MOD_RISING) && falling_edge) ||
				((m_tmcsr & TMCSR::MOD_EXT) == TMCSR::MOD_BOTH))
			{
				m_tmr--;

				if (m_tmr == 0xffff)
				{
					if (m_tmcsr & TMCSR::RELD)
						m_tmr = m_tmrlr;
					else
						m_start_time = attotime::never;

					m_underflow_time = machine().time();
					update();
				}
			}
		}
		else
		{
			if ((((m_tmcsr & TMCSR::MOD_INT) == TMCSR::MOD_RISING) && rising_edge) ||
				(((m_tmcsr & TMCSR::MOD_INT) == TMCSR::MOD_RISING) && falling_edge) ||
				((m_tmcsr & TMCSR::MOD_INT) == TMCSR::MOD_BOTH))
			{
				trigger();
				update();
			}
			else if ((m_tmcsr & TMCSR::MOD_GATE) == TMCSR::MOD_GATE_LOW ||
				(m_tmcsr & TMCSR::MOD_GATE) == TMCSR::MOD_GATE_HIGH)
			{
				update_tmr();
				update();
			}
		}
	}

	m_tin = state;
}

void f2mc16_reload_timer_device::i2osclr(int state)
{
	if (state && !m_i2osclr)
	{
		update_tmr();

		m_underflow_time = attotime::never;

		update();
	}

	m_i2osclr = state;
}

uint16_t f2mc16_reload_timer_device::tmcsr_r(offs_t offset, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7 && !m_cpu->rmw() && m_underflow_time > machine().time())
		return m_tmcsr & ~TMCSR::UF;

	return m_tmcsr;
}

void f2mc16_reload_timer_device::tmcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t tmcsr = m_tmcsr;
	COMBINE_DATA(&tmcsr);
	tmcsr &= TMCSR::CSL | TMCSR::MOD_INT | TMCSR::OUTE | TMCSR::OUTL | TMCSR::RELD | TMCSR::INTE | TMCSR::UF | TMCSR::CNTE | TMCSR::TRG;

	if (m_tmcsr != tmcsr)
	{
		update_tmr();

		m_tmcsr = tmcsr;

		if (ACCESSING_BITS_0_7)
		{
			if (m_tmcsr & TMCSR::TRG)
			{
				if (m_tmcsr & TMCSR::CNTE)
					trigger();

				m_tmcsr &= ~TMCSR::TRG;
			}

			if (!(m_tmcsr & TMCSR::UF))
			{
				if (m_underflow_time <= machine().time())
					m_underflow_time = attotime::never;

				m_tmcsr |= TMCSR::UF;
			}
		}

		update();
	}
}

uint16_t f2mc16_reload_timer_device::tmr_r()
{
	return calculate_tmr();
}

void f2mc16_reload_timer_device::tmrlr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t tmrlr = m_tmrlr;
	COMBINE_DATA(&tmrlr);

	if (m_tmrlr != tmrlr)
	{
		if (m_tmcsr & TMCSR::RELD)
			update_tmr();

		m_tmrlr = tmrlr;

		if (m_tmcsr & TMCSR::RELD)
			update();
	}
}

void f2mc16_reload_timer_device::update()
{
	attotime now = machine().time();

	if (!(m_tmcsr & TMCSR::CNTE))
		m_start_time = attotime::never;

	m_clocksel = m_start_time.is_never() ? 0 :
		((m_tmcsr & TMCSR::CSL) == TMCSR::CSL_EXTERNAL) ?
			(((m_tmcsr & TMCSR::MOD_EXT) == TMCSR::MOD_DISABLE) ? 0 :
			((m_tmcsr & TMCSR::MOD_EXT) == TMCSR::MOD_BOTH) ? m_tin_hz * 2 :
			m_tin_hz) :
		(((m_tmcsr & TMCSR::MOD_GATE) == TMCSR::MOD_GATE_LOW && m_tin) ? 0 :
			((m_tmcsr & TMCSR::MOD_GATE) == TMCSR::MOD_GATE_HIGH && !m_tin) ? 0 :
			((m_tmcsr & TMCSR::CSL) == TMCSR::CSL_DIV2) ? clock() / 2 :
			((m_tmcsr & TMCSR::CSL) == TMCSR::CSL_DIV8) ? clock() / 8 :
			((m_tmcsr & TMCSR::CSL) == TMCSR::CSL_DIV32) ? clock() / 32 :
			0);

	attotime event_time = attotime::never;

	attotime uf = (m_start_time.is_never() || !m_clocksel) ? attotime::never :
		m_start_time + attotime::from_ticks(m_tmr + 1, m_clocksel);

	if (m_underflow_time > now)
	{
		m_underflow_time = uf;

		if ((m_tmcsr & TMCSR::INTE) && event_time > uf)
			event_time = uf;
	}

	uint32_t internal_hz = (m_tmcsr & TMCSR::RELD) ? m_clocksel / (m_tmrlr + 1) / 2: 0;
	uint32_t tot_hz = (m_tmcsr & TMCSR::OUTE) ? internal_hz : 0;

	if ((m_internal_hz != internal_hz && !m_internal_hz_cb.isunset()) ||
		(m_tot_hz != tot_hz && !m_tot_hz_cb.isunset()))
	{
		if (m_tmr != m_tmrlr && (m_tmcsr & TMCSR::RELD) && internal_hz)
		{
			if (event_time > uf)
				event_time = uf;
		}
		else
		{
			if (m_internal_hz != internal_hz && !m_internal_hz_cb.isunset())
			{
				m_internal_hz_cb(internal_hz);
				m_internal_hz = internal_hz;
			}

			if (m_tot_hz != tot_hz && !m_tot_hz_cb.isunset())
			{
				m_tot_hz_cb(tot_hz);
				m_tot_hz = tot_hz;
			}
		}
	}

	m_timer->adjust(event_time.is_never() ? event_time : event_time - now);
	m_intc->set_irq(m_vector, ((m_tmcsr & TMCSR::INTE) && m_underflow_time <= now) ? 1 : 0);
}

void f2mc16_reload_timer_device::trigger()
{
	m_tmr = m_tmrlr;
	m_start_time = machine().time();
}

void f2mc16_reload_timer_device::update_tmr()
{
	m_tmr = calculate_tmr();
	if (m_start_time != attotime::never)
		m_start_time = machine().time();
}

uint16_t f2mc16_reload_timer_device::calculate_tmr()
{
	if (m_start_time.is_never() || m_clocksel == 0)
		return m_tmr;

	uint64_t ticks = (machine().time() - m_start_time).as_ticks(m_clocksel);
	if (m_tmr >= ticks)
		return m_tmr - ticks;

	if (m_tmcsr & TMCSR::RELD)
		return m_tmrlr - ((ticks - m_tmr - 1) % (m_tmrlr + 1));

	return 0xffff;
}

TIMER_CALLBACK_MEMBER(f2mc16_reload_timer_device::update_tin_hz)
{
	if ((m_tmcsr & TMCSR::CSL) == TMCSR::CSL_EXTERNAL)
		update_tmr();

	m_tin_hz = param;

	if (started())
	{
		m_tin_changed = machine().time();

		if ((m_tmcsr & TMCSR::CSL) == TMCSR::CSL_EXTERNAL)
			update();
	}
}

TIMER_CALLBACK_MEMBER(f2mc16_reload_timer_device::update_peripheral_clock)
{
	if ((m_tmcsr & TMCSR::CSL) != TMCSR::CSL_EXTERNAL)
		update_tmr();

	m_peripheral_clock_hz = param;
	m_peripheral_clock_changed = machine().time();

	if ((m_tmcsr & TMCSR::CSL) != TMCSR::CSL_EXTERNAL)
		update();
}

TIMER_CALLBACK_MEMBER(f2mc16_reload_timer_device::timer_callback)
{
	if (m_tmcsr & TMCSR::RELD)
	{
		m_tmr = m_tmrlr;
		m_start_time = machine().time();
	}
	else
	{
		m_tmr = 0xffff;
		m_start_time = attotime::never;
	}

	update();
}
