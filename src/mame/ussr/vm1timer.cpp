// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    K1801VM1 on-chip timer

    https://github.com/1801BM1/cpu11/blob/master/vm1/doc/1801vm1.pdf
    https://zx-pk.ru/threads/15090.html

    Not implemented:
    - SP mode (external trigger)
    - IRQ (only present on die revision G)

**********************************************************************/

#include "emu.h"
#include "vm1timer.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(K1801VM1_TIMER, k1801vm1_timer_device, "1801vm1_timer", "1801VM1 timer")



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************


//-------------------------------------------------
//  k1801vm1_timer_device - constructor
//-------------------------------------------------

k1801vm1_timer_device::k1801vm1_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, K1801VM1_TIMER, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void k1801vm1_timer_device::device_start()
{
	// register for state saving
	save_item(NAME(m_csr));
	save_item(NAME(m_limit));
	save_item(NAME(m_counter));

	m_timer = timer_alloc(FUNC(k1801vm1_timer_device::timer_tick), this);
	m_reload = timer_alloc(FUNC(k1801vm1_timer_device::timer_reload), this);
	m_timer->adjust(attotime::never, 0, attotime::never);
	m_reload->adjust(attotime::never, 0, attotime::never);

	m_limit = rand();
	m_counter = rand();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void k1801vm1_timer_device::device_reset()
{
	init_w();
}

void k1801vm1_timer_device::init_w()
{
//	m_limit is not reset by INIT or DCLO
	m_csr = 0;
}


//-------------------------------------------------
//  read - register read
//-------------------------------------------------

uint16_t k1801vm1_timer_device::read(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_limit;
		break;

	case 1:
		data = m_counter;
		break;

	case 2:
		data = m_csr | 0177400;
		break;
	}

	return data;
}

//-------------------------------------------------
//  write - register write
//-------------------------------------------------

void k1801vm1_timer_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("%s W %06o <- %06o & %06o\n", machine().describe_context(), 0177706 + (offset << 1), data, mem_mask);

	switch (offset)
	{
	case 0:
		m_limit = data;
		break;

	case 2:
		if (BIT(data, 4))
		{
			int divisor = 128;
			if (BIT(data, 5)) divisor *= 16;
			if (BIT(data, 6)) divisor *= 4;
			m_timer->adjust(attotime::from_hz(clock() / divisor), 0, attotime::from_hz(clock() / divisor));
			LOG("%11.6f timer set: %u. limit (%u. cur) %d div (%d hz), oneshot %d, err %d\n",
				machine().time().as_double(), m_limit, m_counter, divisor, clock()/divisor, BIT(data, 3), BIT(m_csr, 15));
		}
		else
		{
			m_timer->adjust(attotime::never, 0, attotime::never);
			LOG("%11.6f timer stopped\n", machine().time().as_double());
		}

		m_csr = ((m_csr & ~TMRCSR_WR) | (data & TMRCSR_WR));
		m_counter = m_limit;

		break;
	}
}

TIMER_CALLBACK_MEMBER(k1801vm1_timer_device::timer_tick)
{
	// external trigger mode
	if (m_csr & TMRCSR_SP) return;

	if (--m_counter == 0)
	{
		LOG("%11.6f timer done err %d ovf %d irq %d\n", machine().time().as_double(), BIT(m_csr, 15), BIT(m_csr, 7), BIT(m_csr, 2));
		if (m_csr & TMRCSR_MON)
		{
			if (m_csr & CSR_ERR)
				m_csr |= TMRCSR_FL;
			m_csr |= CSR_ERR;
		}

		if (!(m_csr & TMRCSR_CAP))
		{
			if (m_csr & TMRCSR_OS)
			{
				m_csr &= ~TMRCSR_RUN;
				m_timer->adjust(attotime::never, 0, attotime::never);
				LOG("timer stopped (OS)\n");
			}
			m_reload->adjust(attotime::from_hz(clock() / 4));
		}
	}
}

TIMER_CALLBACK_MEMBER(k1801vm1_timer_device::timer_reload)
{
	LOG("%11.6f timer reloaded\n", machine().time().as_double());
	m_counter = m_limit;
}
