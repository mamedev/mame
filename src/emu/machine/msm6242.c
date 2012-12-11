/***************************************************************************

    MSM6242 / Epson RTC 62421 / 62423 Real Time Clock

    TODO:
    - Stop timer callbacks on every single tick
    - HOLD mechanism
    - IRQs are grossly mapped
    - STOP / RESET mechanism
    - why skns.c games try to read uninitialized registers?

***************************************************************************/

#include "emu.h"
#include "machine/msm6242.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

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

#define TIMER_RTC_CALLBACK		1



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type msm6242 = &device_creator<msm6242_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  msm6242_device - constructor
//-------------------------------------------------

msm6242_device::msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, msm6242, "msm6242", tag, owner, clock),
	  device_rtc_interface(mconfig, *this)
{

}



//-------------------------------------------------
//  rtc_timer_callback
//-------------------------------------------------

void msm6242_device::rtc_timer_callback()
{
	static const UINT8 dpm[12] = { 0x31, 0x28, 0x31, 0x30, 0x31, 0x30, 0x31, 0x31, 0x30, 0x31, 0x30, 0x31 };
	int dpm_count;

	m_tick++;

	if(m_irq_flag == 1 && m_irq_type == 0 && ((m_tick % 0x200) == 0)) // 1/64 of second
	{
		if ( !m_res_out_int_func.isnull() )
			m_res_out_int_func( ASSERT_LINE );
	}

	if(m_tick & 0x8000) // 32,768 KHz == 0x8000 ticks
	{
		int sec = get_clock_register(RTC_SECOND);
		int minute = get_clock_register(RTC_MINUTE);
		int hour = get_clock_register(RTC_HOUR);
		int day = get_clock_register(RTC_DAY);
		int month = get_clock_register(RTC_MONTH);
		int weekday = get_clock_register(RTC_DAY_OF_WEEK);
		int year = get_clock_register(RTC_YEAR);

		m_tick = 0;
		sec++;

		if(m_irq_flag == 1 && m_irq_type == 1) // 1 second clock
			if ( !m_res_out_int_func.isnull() )
				m_res_out_int_func(ASSERT_LINE);

		if(sec >= 60)
		{
			minute++; sec = 0;
			if(m_irq_flag == 1 && m_irq_type == 2) // 1 minute clock
				if ( !m_res_out_int_func.isnull() )
					m_res_out_int_func(ASSERT_LINE);
		}
		if(minute >= 60)
		{
			hour++; minute = 0;
			if(m_irq_flag == 1 && m_irq_type == 3) // 1 hour clock
				if ( !m_res_out_int_func.isnull() )
					m_res_out_int_func(ASSERT_LINE);
		}
		if(hour >= 24)			{ day++; weekday++; hour = 0; }
		if(weekday >= 6)				{ weekday = 1; }

		/* TODO: crude leap year support */
		dpm_count = (month)-1;

		if(((year % 4) == 0) && month == 2)
		{
			if((day) >= dpm[dpm_count]+1+1)
				{ month++; day = 0x01; }
		}
		else if(day >= dpm[dpm_count]+1)		{ month++; day = 0x01; }
		if(month >= 0x13)						{ year++; month = 1; }
		if(year >= 100)						{ year = 0; } //1900-1999 possible timeframe

		set_clock_register(RTC_SECOND, sec);
		set_clock_register(RTC_MINUTE, minute);
		set_clock_register(RTC_HOUR, hour);
		set_clock_register(RTC_DAY, day);
		set_clock_register(RTC_MONTH, month);
		set_clock_register(RTC_DAY_OF_WEEK, weekday);
		set_clock_register(RTC_YEAR, year);
	}
}



//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void msm6242_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case TIMER_RTC_CALLBACK:
			rtc_timer_callback();
			break;
	}
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm6242_device::device_start()
{
	const msm6242_interface *intf = reinterpret_cast<const msm6242_interface *>(static_config());
	if (intf != NULL)
		m_res_out_int_func.resolve(intf->m_out_int_func, *this);

	// let's call the timer callback every tick
	m_timer = timer_alloc(TIMER_RTC_CALLBACK);
	m_timer->adjust(attotime::zero, 0, attotime::from_hz(clock()));

	// get real time from system
	set_current_time(machine());

	// set up registers
	m_tick = 0;
	m_irq_flag = 0;
	m_irq_type = 0;

	// TODO: skns writes 0x4 to D then expects E == 6 and F == 4, perhaps those are actually saved in the RTC CMOS?
	m_reg[0] = 0;
	m_reg[1] = 0x6;
	m_reg[2] = 0x4;

	// save states
	save_item(NAME(m_reg));
	save_item(NAME(m_irq_flag));
	save_item(NAME(m_irq_type));
	save_item(NAME(m_tick));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void msm6242_device::device_reset()
{
	if ( !m_res_out_int_func.isnull() )
		m_res_out_int_func( CLEAR_LINE );
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  read
//-------------------------------------------------

READ8_MEMBER( msm6242_device::read )
{
	int sec = get_clock_register(RTC_SECOND);
	int minute = get_clock_register(RTC_MINUTE);
	int hour = get_clock_register(RTC_HOUR);
	int day = get_clock_register(RTC_DAY);
	int month = get_clock_register(RTC_MONTH);
	int weekday = get_clock_register(RTC_DAY_OF_WEEK);
	int year = get_clock_register(RTC_YEAR);

	switch(offset)
	{
		case MSM6242_REG_S1: return (sec % 10) & 0xf;
		case MSM6242_REG_S10: return (sec / 10) & 0xf;
		case MSM6242_REG_MI1: return (minute % 10) & 0xf;
		case MSM6242_REG_MI10: return (minute / 10) & 0xf;
		case MSM6242_REG_H1:
		case MSM6242_REG_H10:
		{
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

		case MSM6242_REG_D1: return (day % 10) & 0xf;
		case MSM6242_REG_D10: return (day / 10) & 0xf;
		case MSM6242_REG_MO1: return (month % 10) & 0xf;
		case MSM6242_REG_MO10: return (month / 10) & 0xf;
		case MSM6242_REG_Y1: return (year % 10) & 0xf;
		case MSM6242_REG_Y10: return ((year / 10) % 10) & 0xf;
		case MSM6242_REG_W: return weekday;
		case MSM6242_REG_CD: return m_reg[0];
		case MSM6242_REG_CE: return m_reg[1];
		case MSM6242_REG_CF: return m_reg[2];
	}

	logerror("%s: MSM6242 unmapped offset %02x read\n", machine().describe_context(), offset);
	return 0;
}



//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER( msm6242_device::write )
{
	switch(offset)
	{
		case MSM6242_REG_CD:
		{
            //	x--- 30s ADJ
            //	-x-- IRQ FLAG
            //	--x- BUSY
            //	---x HOLD

			m_reg[0] = data & 0x0f;

			return;
		}

		case MSM6242_REG_CE:
		{
            //	xx-- t0,t1 (timing irq)
            //	--x- STD
            //	---x MASK

			m_reg[1] = data & 0x0f;
			if((data & 3) == 0) // MASK & STD = 0
			{
				m_irq_flag = 1;
				m_irq_type = (data & 0xc) >> 2;
			}
			else
			{
				m_irq_flag = 0;
				if ( !m_res_out_int_func.isnull() )
					m_res_out_int_func( CLEAR_LINE );
			}

			return;
		}

		case MSM6242_REG_CF:
		{
            //	x--- TEST
            //	-x-- 24/12
            //	--x- STOP
            //	---x RESET

			// the 12/24 mode bit can only be changed when RESET does a 1 -> 0 transition
			if (((data & 0x01) == 0x00) && (m_reg[2] & 0x01))
				m_reg[2] = (m_reg[2] & ~0x04) | (data & 0x04);
			else
				m_reg[2] = (data & 0x0b) | (m_reg[2] & 4);

			return;
		}
	}

	logerror("%s: MSM6242 unmapped offset %02x written with %02x\n", machine().describe_context(), offset, data);
}
