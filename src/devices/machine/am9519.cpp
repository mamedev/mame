// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Carl

//TODO: rotating priority, cascade

#include "emu.h"
#include "machine/am9519.h"

#define LOG_GENERAL (1U << 0)

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

void am9519_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if(!BIT(m_mode, 7)) // chip disabled
		return;

	/* check the various IRQs */
	for(int n = 0, irq = m_prio; n < 8; n++, irq = (irq + 1) & 7)
	{
		u8 mask = 1 << irq;

		/* is this IRQ in service */
		if(m_isr & mask)
		{
			LOG("am9519_timerproc(): UIC IRQ #%d still in service\n", irq);
			break;
		}

		/* is this IRQ pending and enabled? */
		if((m_irr & mask) && !(m_imr & mask))
		{
			LOG("am9519_timerproc(): UIC triggering IRQ #%d\n", irq);
			if(!BIT(m_mode, 2))
				m_out_int_func(1);
			return;
		}
	}
	m_out_int_func(0);
}


void am9519_device::set_irq_line(int irq, int state)
{
	u8 mask = (1 << irq);
	bool active = !((state == ASSERT_LINE ? 1 : 0) ^ BIT(m_mode, 4));

	if(active)
	{
		/* setting IRQ line */
		LOG("am9519_set_irq_line(): UIC set IRQ line #%d\n", irq);

		if(!(m_irq_lines & mask)) // edge trig only
			m_irr |= mask;
		m_irq_lines |= mask;
	}
	else
	{
		/* clearing IRQ line */
		LOG("am9519_device::set_irq_line(): UIC cleared IRQ line #%d\n", irq);

		m_irq_lines &= ~mask;
		m_irr &= ~mask;
	}
	set_timer();
}


u32 am9519_device::acknowledge()
{
	for(int n = 0, irq = m_prio; n < 8; n++, irq = (irq + 1) & 7)
	{
		u8 mask = 1 << irq;

		/* is this IRQ pending and enabled? */
		if((m_irr & mask) && !(m_imr & mask))
		{
			LOG("am9519_acknowledge(): UIC acknowledge IRQ #%d\n", irq);

			if(!(mask & m_aclear))
				m_isr |= mask;

			m_irr &= ~mask;
			set_timer();

			u32 ret = 0;
			int irqr = BIT(m_mode, 1) ? 0 : irq;
			for(int i = 0; i < m_count[irqr]; i++)
				ret = (ret << 8) | m_resp[irqr][i];

			return ret;
		}
	}
	logerror("Spurious IRQ\n");
	return 0;
}


IRQ_CALLBACK_MEMBER(am9519_device::iack_cb)
{
	return acknowledge();
}


u8 am9519_device::stat_r()
{
	u8 stat = 0;
	for(int n = 0, irq = m_prio; n < 8; n++, irq = (irq + 1) & 7)
	{
		u8 mask = 1 << irq;
		if((m_irr & mask) && !(m_imr & mask))
		{
			stat = 0x80 | irq;
			break;
		}
	}
	stat |= BIT(m_mode, 7) << 3;
	stat |= BIT(m_mode, 2) << 4;
	stat |= BIT(m_mode, 0) << 5;
	return stat;
}

u8 am9519_device::data_r()
{
	switch((m_mode & 0x60) >> 5)
	{
		case 0:
			return m_isr;
		case 1:
			return m_imr;
		case 2:
			return m_irr;
		case 3:
			return m_aclear;
	}
	return 0;
}

void am9519_device::cmd_w(u8 data)
{
	m_cmd = data;
	switch(data >> 3)
	{
		case 0:
			reset();
			break;
		case 2:
			m_irr = m_imr = 0;
			break;
		case 3:
			m_irr &= ~(1 << (data & 7));
			m_imr &= ~(1 << (data & 7));
			break;
		case 4:
			m_imr = 0;
			break;
		case 5:
			m_imr &= ~(1 << (data & 7));
			break;
		case 6:
			m_imr = 0xff;
			break;
		case 7:
			m_imr |= 1 << (data & 7);
			break;
		case 8:
			m_irr = 0;
			break;
		case 9:
			m_irr &= ~(1 << (data & 7));
			break;
		case 10:
			m_irr = 0xff;
			break;
		case 11:
			m_irr |= 1 << (data & 7);
			break;
		case 12:
		case 13:
		{
			for(int n = 0, irq = m_prio; n < 8; n++, irq = (irq + 1) & 7)
			{
				if(m_isr & (1 << irq))
					m_isr &= ~(1 << irq);
			}
			break;
		}
		case 14:
			m_isr = 0;
			break;
		case 15:
			m_isr &= ~(1 << (data & 7));
			break;
		case 16:
		case 17:
		case 18:
		case 19:
			m_mode = (m_mode & 0xe0) | (data & 0x1f);
			break;
		case 20:
		case 21:
			m_mode = (m_mode & 0x9f) | (data & 0x60);
			switch(data & 3)
			{
				case 1:
					m_mode |= 0x80;
					break;
				case 2:
					m_mode &= 0x7f;
					break;
			}
			break;
		case 28:
		case 29:
		case 30:
		case 31:
			m_count[m_cmd & 7] = ((data >> 3) & 3) + 1;
			m_curcnt = 0;
			break;
	}
	set_timer();
}

void am9519_device::data_w(u8 data)
{
	if((m_cmd & 0xf0) >= 0xb0)
	{
		switch(m_cmd >> 4)
		{
			case 11:
				m_imr = data;
				break;
			case 12:
				m_aclear = data;
				break;
			case 14:
			case 15:
				if(m_curcnt < m_count[m_cmd & 7])
				{
					m_resp[m_cmd & 7][m_curcnt] = data;
					m_curcnt++;
					return;
				}
				break;
		}
		m_cmd = 0;
	}
	set_timer();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void am9519_device::device_start()
{
	// resolve callbacks
	m_out_int_func.resolve_safe();

	// Register save state items
	save_item(NAME(m_isr));
	save_item(NAME(m_irr));
	save_item(NAME(m_prio));
	save_item(NAME(m_imr));
	save_item(NAME(m_irq_lines));
	save_item(NAME(m_mode));
	save_item(NAME(m_count));
	save_item(NAME(m_resp));
	save_item(NAME(m_aclear));
	save_item(NAME(m_cmd));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void am9519_device::device_reset()
{
	m_isr = 0;
	m_irr = 0;
	m_irq_lines = 0;
	m_prio = 0;
	m_imr = 0xff;
	m_mode = 0;
	m_aclear = 0;
	m_cmd = 0;
	m_curcnt = 0;
	for(int i = 0; i < 8; i++)
	{
		m_count[i] = 0;
		m_resp[i][0] = 0;
		m_resp[i][1] = 0;
		m_resp[i][2] = 0;
		m_resp[i][3] = 0;
	}
}

DEFINE_DEVICE_TYPE(AM9519, am9519_device, "am9519", "AMD AM9519 Universal Interrupt Controller")

am9519_device::am9519_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AM9519, tag, owner, clock)
	, m_out_int_func(*this)
	, m_irr(0)
	, m_irq_lines(0)
{
}
