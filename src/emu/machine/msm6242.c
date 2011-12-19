/***************************************************************************

    MSM6242 Real Time Clock

	Note:
	- this RTC has a y2k bug

	TODO:
	- HOLD mechanism
	- IRQ ACK

***************************************************************************/

#include "emu.h"
#include "machine/msm6242.h"
#include "machine/devhelpr.h"


enum
{
	MSM6242_REG_S1		= 0,
	MSM6242_REG_S10,
	MSM6242_REG_MI1,
	MSM6242_REG_MI10,
	MSM6242_REG_H1,
	MSM6242_REG_H10,
	MSM6242_REG_D1,
	MSM6242_REG_D10,
	MSM6242_REG_MO1,
	MSM6242_REG_MO10,
	MSM6242_REG_Y1,
	MSM6242_REG_Y10,
	MSM6242_REG_W,
	MSM6242_REG_CD,
	MSM6242_REG_CE,
	MSM6242_REG_CF
};

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type msm6242 = &device_creator<msm6242_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  xxx_device - constructor
//-------------------------------------------------

msm6242_device::msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, msm6242, "msm6242", tag, owner, clock)
{

}

void msm6242_device::rtc_timer_callback()
{
	static const UINT8 dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };
	int dpm_count;

	m_tick++;

	if(m_irq_flag == 1 && m_irq_type == 0 && ((m_tick % 0x200) == 0)) // 1/64 of second
	{
		if ( !m_out_int_func.isnull() )
			m_out_int_func( ASSERT_LINE );
	}

	if(m_tick & 0x8000) // 32,768 KHz == 0x8000 ticks
	{
		m_tick = 0;
		m_rtc.sec++;

		if(m_irq_flag == 1 && m_irq_type == 1) // 1 second clock
			if ( !m_out_int_func.isnull() )
				m_out_int_func(ASSERT_LINE);

		if(m_rtc.sec >= 60)
		{
			m_rtc.min++; m_rtc.sec = 0;
			if(m_irq_flag == 1 && m_irq_type == 2) // 1 minute clock
				if ( !m_out_int_func.isnull() )
					m_out_int_func(ASSERT_LINE);
		}
		if(m_rtc.min >= 60)
		{
			m_rtc.hour++; m_rtc.min = 0;
			if(m_irq_flag == 1 && m_irq_type == 3) // 1 hour clock
				if ( !m_out_int_func.isnull() )
					m_out_int_func(ASSERT_LINE);
		}
		if(m_rtc.hour >= 24)			{ m_rtc.day++; m_rtc.wday++; m_rtc.hour = 0; }
		if(m_rtc.wday >= 6)				{ m_rtc.wday = 1; }

		/* TODO: crude leap year support */
		dpm_count = (m_rtc.month)-1;

		if(((m_rtc.year % 4) == 0) && m_rtc.month == 2)
		{
			if((m_rtc.day) >= dpm[dpm_count]+1+1)
				{ m_rtc.month++; m_rtc.day = 0x01; }
		}
		else if(m_rtc.day >= dpm[dpm_count]+1)		{ m_rtc.month++; m_rtc.day = 0x01; }
		if(m_rtc.month >= 0x13)						{ m_rtc.year++; m_rtc.month = 1; }
		if(m_rtc.year >= 100)						{ m_rtc.year = 0; } //1900-1999 possible timeframe
	}
}

TIMER_CALLBACK( msm6242_device::rtc_inc_callback )
{
	reinterpret_cast<msm6242_device *>(ptr)->rtc_timer_callback();
}

//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

bool msm6242_device::device_validity_check(emu_options &options, const game_driver &driver) const
{
	bool error = false;
	return error;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm6242_device::device_start()
{
	m_out_int_func.resolve( m_out_int_cb, *this );

	/* let's call the timer callback every second */
	machine().scheduler().timer_pulse(attotime::from_hz(clock()), FUNC(rtc_inc_callback), 0, (void *)this);

	system_time systime;
	machine().base_datetime(systime);

	m_rtc.day = (systime.local_time.mday);
	m_rtc.month = (systime.local_time.month+1);
	m_rtc.wday = (systime.local_time.weekday);
	m_rtc.year = (systime.local_time.year % 100);
	m_rtc.hour = (systime.local_time.hour);
	m_rtc.min = (systime.local_time.minute);
	m_rtc.sec = (systime.local_time.second);
	m_tick = 0;
	m_irq_flag = 0;
	m_irq_type = 0;

	m_reg[0] = 0;
	m_reg[1] = 0;
	m_reg[2] = 0;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void msm6242_device::device_reset()
{
	if ( !m_out_int_func.isnull() )
		m_out_int_func( CLEAR_LINE );
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void msm6242_device::device_config_complete()
{
	const msm6242_interface *intf = reinterpret_cast<const msm6242_interface *>(static_config());

	if ( intf != NULL )
	{
		*static_cast<msm6242_interface *>(this) = *intf;
	}
	else
	{
		memset(&m_out_int_cb, 0, sizeof(m_out_int_cb));
	}
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( msm6242_device::read )
{
	rtc_regs_t cur_time;

	//cur_time = (m_reg[0] & 1) ? m_hold : m_rtc;

	cur_time = m_rtc;

	switch(offset)
	{
		case MSM6242_REG_S1: return (cur_time.sec % 10) & 0xf;
		case MSM6242_REG_S10: return (cur_time.sec / 10) & 0xf;
		case MSM6242_REG_MI1: return (cur_time.min % 10) & 0xf;
		case MSM6242_REG_MI10: return (cur_time.min / 10) & 0xf;
		case MSM6242_REG_H1:
		case MSM6242_REG_H10:
		{
			int	hour = cur_time.hour;
			int pm = 0;

			/* check for 12/24 hour mode */
			if ((m_reg[2] & 0x04) == 0) /* 12 hour mode? */
			{
				if (hour >= 12)
					pm = 1;

				hour %= 12;

				if ( hour == 0 )
					hour = 12;
			}

			if ( offset == MSM6242_REG_H1 )
				return hour % 10;

			return (hour / 10) | (pm <<2);
		}

		case MSM6242_REG_D1: return (cur_time.day % 10) & 0xf;
		case MSM6242_REG_D10: return (cur_time.day / 10) & 0xf;
		case MSM6242_REG_MO1: return (cur_time .month % 10) & 0xf;
		case MSM6242_REG_MO10: return (cur_time.month / 10) & 0xf;
		case MSM6242_REG_Y1: return (cur_time.year % 10) & 0xf;
		case MSM6242_REG_Y10: return ((cur_time.year / 10) % 10) & 0xf;
		case MSM6242_REG_W: return cur_time.wday;
		case MSM6242_REG_CD: return m_reg[0];
		case MSM6242_REG_CE: return m_reg[1];
		case MSM6242_REG_CF: return m_reg[2];
	}

	logerror("%s: MSM6242 unmapped offset %02x read\n", machine().describe_context(), offset);
	return 0;
}

WRITE8_MEMBER( msm6242_device::write )
{
	switch(offset)
	{
		case MSM6242_REG_CD:
		{
			m_reg[0] = data & 0x0f;

			#if 0
			if (data & 1)	/* was Hold set? */
			{
				m_hold.day = m_rtc.day;
				m_hold.month = m_rtc.month;
				m_hold.hour = m_rtc.hour;
				m_hold.day = m_rtc.day;
				m_hold.month = m_rtc.month;
				m_hold.year = m_rtc.year;
				m_hold.wday = m_rtc.wday;
			}
			#endif

			return;
		}

		case MSM6242_REG_CE:
		m_reg[1] = data & 0x0f;
		if((data & 3) == 0) // MASK & STD = 0
		{
			m_irq_flag = 1;
			m_irq_type = (data & 0xc) >> 2;
			//m_std_timer->adjust(attotime::from_msec(timer_param[(data & 0xc) >> 2]), 0, attotime::from_msec(timer_param[(data & 0xc) >> 2]));
		}
		else
		{
			m_irq_flag = 0;
			if ( !m_out_int_func.isnull() )
				m_out_int_func( CLEAR_LINE );
		}

		return;

		case MSM6242_REG_CF:
		{
			/* the 12/24 mode bit can only be changed while REST is 1 */
			if ((data ^ m_reg[2]) & 0x04)
			{
				m_reg[2] = (m_reg[2] & 0x04) | (data & ~0x04);

				if (m_reg[2] & 1)
					m_reg[2] = (m_reg[2] & ~0x04) | (data & 0x04);
			}
			else
			{
				m_reg[2] = data & 0x0f;
			}
			return;
		}
	}

	logerror("%s: MSM6242 unmapped offset %02x written with %02x\n", machine().describe_context(), offset, data);
}






