// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    MSM6242 / Epson RTC 62421 / 62423 / 72421 / 72423 Real Time Clock

    TODO:
    - Stop timer callbacks on every single tick
    - HOLD mechanism
    - IRQs are grossly mapped
    - STOP / RESET mechanism
    - why skns.c games try to read uninitialized registers?

***************************************************************************/

#include "emu.h"
#include "machine/msm6242.h"

#define LOG_GENERAL     (1U << 0)
#define LOG_UNMAPPED    (1U << 1)
#define LOG_IRQ         (1U << 2)
#define LOG_IRQ_ENABLE  (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_UNMAPPED | LOG_IRQ | LOG_IRQ_ENABLE)
#include "logmacro.h"

#define LOGUNMAPPED(...)    LOGMASKED(LOG_UNMAPPED, __VA_ARGS__)
#define LOGIRQ(...)         LOGMASKED(LOG_IRQ, __VA_ARGS__)
#define LOGIRQENABLE(...)   LOGMASKED(LOG_IRQ_ENABLE, __VA_ARGS__)


//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum
{
	MSM6242_REG_S1      = 0,
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

// device type definitions
DEFINE_DEVICE_TYPE(MSM6242,  msm6242_device,  "msm6242",  "OKI MSM6242 RTC")
DEFINE_DEVICE_TYPE(RTC62421, rtc62421_device, "rtc62421", "Epson RTC-62421 RTC")
DEFINE_DEVICE_TYPE(RTC62423, rtc62423_device, "rtc62423", "Epson RTC-62423 RTC")
DEFINE_DEVICE_TYPE(RTC72421, rtc72421_device, "rtc72421", "Epson RTC-72421 RTC")
DEFINE_DEVICE_TYPE(RTC72423, rtc72423_device, "rtc72423", "Epson RTC-72423 RTC")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  msm6242_device - constructor
//-------------------------------------------------

msm6242_device::msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msm6242_device(mconfig, MSM6242, tag, owner, clock)
{
}

msm6242_device::msm6242_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_rtc_interface(mconfig, *this)
	, m_out_int_handler(*this)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm6242_device::device_start()
{
	m_out_int_handler.resolve();

	// let's call the timer callback every tick
	m_timer = timer_alloc(FUNC(msm6242_device::rtc_timer_callback), this);
	m_timer->adjust(attotime::zero);
	m_timer_irq_clear = timer_alloc(FUNC(msm6242_device::rtc_irq_pulse_timer_callback), this);
	m_timer_irq_clear->adjust(attotime::zero);

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
	save_item(NAME(m_last_update_time));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void msm6242_device::device_reset()
{
	set_irq(false);
}



//-------------------------------------------------
//  device_pre_save - called prior to saving the
//  state, so that registered variables can be
//  properly normalized
//-------------------------------------------------

void msm6242_device::device_pre_save()
{
	// update the RTC registers so that we can get the right values
	update_rtc_registers();
}



//-------------------------------------------------
//  device_post_load - called after the loading a
//  saved state, so that registered variables can
//  be expaneded as necessary
//-------------------------------------------------

void msm6242_device::device_post_load()
{
	// this is probably redundant, because the timer state is saved; but it isn't
	// a terribly bad idea
	update_timer();
}



//-------------------------------------------------
//  set_irq - set the IRQ flag and output
//-------------------------------------------------

void msm6242_device::set_irq(bool active)
{
	if (active)
		m_reg[0] |= 0x04;
	else
		m_reg[0] &= 0x0b;

	if (!m_out_int_handler.isnull())
		m_out_int_handler(active ? ASSERT_LINE : CLEAR_LINE);
	
	if (active)
	{
		if (!BIT(m_reg[1], 1)) // irq is pulsed
		{
			m_timer_irq_clear->adjust(attotime::from_nsec(7812500));
		}
	}
}


//-------------------------------------------------
//  irq
//-------------------------------------------------

void msm6242_device::irq(u8 irq_type)
{
	// are we actually raising this particular IRQ?
	if (m_irq_flag == 1 && m_irq_type == irq_type)
	{
		// log if appropriate
		LOGIRQ("%s: MSM6242 logging IRQ #%u\n", machine().describe_context(), irq_type);

		// ...and assert the output line
		set_irq(true);
	}
}



//-------------------------------------------------
//  bump
//-------------------------------------------------

u64 msm6242_device::bump(int rtc_register, u64 delta, u64 register_min, u64 register_range)
{
	u64 carry = 0;

	if (delta > 0)
	{
		// get the register value
		u64 register_value = (rtc_register == RTC_TICKS)
			? m_tick
			: get_clock_register(rtc_register);

		// increment the value
		u64 new_register_value = ((register_value - register_min + delta) % register_range) + register_min;

		// calculate the cary
		carry = ((register_value - register_min) + delta) / register_range;

		// store the new register value
		if (rtc_register == RTC_TICKS)
			m_tick = u16(new_register_value);
		else
			set_clock_register(rtc_register, int(new_register_value));
	}

	return carry;
}



//-------------------------------------------------
//  current_time
//-------------------------------------------------

u64 msm6242_device::current_time()
{
	return machine().time().as_ticks(clock());
}



//-------------------------------------------------
//  update_rtc_registers
//-------------------------------------------------

void msm6242_device::update_rtc_registers()
{
	// get the absolute current time, in ticks
	u64 curtime = current_time();

	// how long as it been since we last updated?
	u64 delta = curtime - m_last_update_time;

	// set current time
	m_last_update_time = curtime;

	// no delta?  just return
	if (delta == 0)
		return;

	// ticks
	if ((m_tick / 0x200) != int((delta + m_tick) / 0x200))
		irq(IRQ_64THSECOND);
	delta = bump(RTC_TICKS, delta, 0, 0x8000);
	if (delta == 0)
		return;

	// seconds
	irq(IRQ_SECOND);
	delta = bump(RTC_SECOND, delta, 0, 60);
	if (delta == 0)
		return;

	// minutes
	irq(IRQ_MINUTE);
	delta = bump(RTC_MINUTE, delta, 0, 60);
	if (delta == 0)
		return;

	// hours
	irq(IRQ_HOUR);
	delta = bump(RTC_HOUR, delta, 0, 24);
	if (delta == 0)
		return;

	// days
	while(delta--)
		advance_days();
}



//-------------------------------------------------
//  update_timer
//-------------------------------------------------

void msm6242_device::update_timer()
{
	u64 callback_ticks = 0;
	attotime callback_time = attotime::never;

	// we only need to call back if the IRQ flag is on, and we have a handler
	if (!m_out_int_handler.isnull() && m_irq_flag == 1)
	{
		switch(m_irq_type)
		{
			case IRQ_HOUR:
				callback_ticks += (59 - get_clock_register(RTC_MINUTE)) * (0x8000 * 60);
				[[fallthrough]];

			case IRQ_MINUTE:
				callback_ticks += (59 - get_clock_register(RTC_SECOND)) * 0x8000;
				[[fallthrough]];

			case IRQ_SECOND:
				callback_ticks += 0x8000 - m_tick;
				break;

			case IRQ_64THSECOND:
				callback_ticks += 0x200 - (m_tick % 0x200);
				break;
		}
	}

	// if set, convert ticks to an attotime
	if (callback_ticks > 0)
	{
		// get the current time
		u64 curtime = current_time();

		// we need the absolute callback time, in ticks
		u64 absolute_callback_ticks = curtime + callback_ticks;

		// convert that to an attotime
		attotime absolute_callback_time = attotime::from_ticks(absolute_callback_ticks, clock());

		// and finally get the delta as an attotime
		callback_time = absolute_callback_time - machine().time();
	}

	m_timer->adjust(callback_time);
}



//-------------------------------------------------
//  rtc_clock_updated
//-------------------------------------------------

void msm6242_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
	m_last_update_time = current_time();
}



//-------------------------------------------------
//  rtc_timer_callback
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(msm6242_device::rtc_timer_callback)
{
	update_rtc_registers();
	update_timer();
}



//-------------------------------------------------
//  rtc_irq_pulse_timer_callback
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(msm6242_device::rtc_irq_pulse_timer_callback)
{
	set_irq(false);
}



//-------------------------------------------------
//  get_clock_nibble
//-------------------------------------------------

u8 msm6242_device::get_clock_nibble(int rtc_register, bool high)
{
	int value = get_clock_register(rtc_register);
	value /= high ? 10 : 1;
	return u8((value % 10) & 0x0F);
}



//-------------------------------------------------
//  get_clock_nibble
//-------------------------------------------------

const char *msm6242_device::irq_type_string(u8 irq_type)
{
	switch(irq_type)
	{
		case IRQ_64THSECOND:    return "1/64th second";
		case IRQ_SECOND:        return "second";
		case IRQ_MINUTE:        return "minute";
		case IRQ_HOUR:          return "hour";
		default:                return "???";
	}
}



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  read
//-------------------------------------------------

u8 msm6242_device::read(offs_t offset)
{
	int hour, pm;
	u8 result;

	// update the registers; they may have changed
	update_rtc_registers();

	switch(offset)
	{
		case MSM6242_REG_S1:
			result = get_clock_nibble(RTC_SECOND, false);
			break;

		case MSM6242_REG_S10:
			result = get_clock_nibble(RTC_SECOND, true);
			break;

		case MSM6242_REG_MI1:
			result = get_clock_nibble(RTC_MINUTE, false);
			break;

		case MSM6242_REG_MI10:
			result = get_clock_nibble(RTC_MINUTE, true);
			break;

		case MSM6242_REG_H1:
		case MSM6242_REG_H10:
			pm = 0;
			hour = get_clock_register(RTC_HOUR);

			// check for 12/24 hour mode
			if ((m_reg[2] & 0x04) == 0) // 12 hour mode?
			{
				if (hour >= 12)
					pm = 1;

				hour %= 12;

				if ( hour == 0 )
					hour = 12;
			}

			if ( offset == MSM6242_REG_H1 )
				result = hour % 10;
			else
				result = (hour / 10) | (pm <<2);
			break;

		case MSM6242_REG_D1:
			result = get_clock_nibble(RTC_DAY, false);
			break;

		case MSM6242_REG_D10:
			result = get_clock_nibble(RTC_DAY, true);
			break;

		case MSM6242_REG_MO1:
			result = get_clock_nibble(RTC_MONTH, false);
			break;

		case MSM6242_REG_MO10:
			result = get_clock_nibble(RTC_MONTH, true);
			break;

		case MSM6242_REG_Y1:
			result = get_clock_nibble(RTC_YEAR, false);
			break;

		case MSM6242_REG_Y10:
			result = get_clock_nibble(RTC_YEAR, true);
			break;

		case MSM6242_REG_W:
			result = u8(get_clock_register(RTC_DAY_OF_WEEK) - 1);
			break;

		case MSM6242_REG_CD:
		case MSM6242_REG_CE:
		case MSM6242_REG_CF:
			result = m_reg[offset - MSM6242_REG_CD];
			break;

		default:
			result = 0x00;
			LOGUNMAPPED("%s: MSM6242 unmapped offset %02x read\n", machine().describe_context(), offset);
			break;
	}

	return result;
}



//-------------------------------------------------
//  write
//-------------------------------------------------

void msm6242_device::write(offs_t offset, u8 data)
{
	switch(offset)
	{
		case MSM6242_REG_CD:
			//  x--- 30s ADJ
			//  -x-- IRQ FLAG (software can only clear this)
			//  --x- BUSY (read-only)
			//  ---x HOLD
			if (!BIT(data, 2) && BIT(m_reg[0], 2))
			{
				LOGIRQENABLE("%s: MSM6242 acknowledging irq\n", machine().describe_context());
				set_irq(false);
				m_timer_irq_clear->adjust(attotime::zero);
			}
			m_reg[0] = (data & 0x09) | (m_reg[0] & 0x06);
			break;

		case MSM6242_REG_CE:
			//  xx-- t0,t1 (timing irq)
			//  --x- STD
			//  ---x MASK
			m_reg[1] = data & 0x0f;
			if((data & 1) == 0) // MASK = 0
			{
				m_irq_flag = 1;
				m_irq_type = (data & 0xc) >> 2;

				LOGIRQENABLE("%s: MSM6242 enabling irq '%s'\n", machine().describe_context(), irq_type_string(m_irq_type));
			}
			else
			{
				m_irq_flag = 0;
				set_irq(false);

				LOGIRQENABLE("%s: MSM6242 disabling irq\n", machine().describe_context());
			}
			break;

		case MSM6242_REG_CF:
			//  x--- TEST
			//  -x-- 24/12
			//  --x- STOP
			//  ---x RESET

			// the 12/24 mode bit can only be changed when RESET does a 1 -> 0 transition
			if (((data & 0x01) == 0x00) && (m_reg[2] & 0x01))
				m_reg[2] = (m_reg[2] & ~0x04) | (data & 0x04);
			else
				m_reg[2] = (data & 0x0b) | (m_reg[2] & 4);
			break;

		default:
			LOGUNMAPPED("%s: MSM6242 unmapped offset %02x written with %02x\n", machine().describe_context(), offset, data);
			break;
	}

	// update the timer variable in response to potential changes
	update_timer();
}


//-------------------------------------------------
//  rtc62421_device - constructor
//-------------------------------------------------

rtc62421_device::rtc62421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msm6242_device(mconfig, RTC62421, tag, owner, clock)
{
}


//-------------------------------------------------
//  rtc62423_device - constructor
//-------------------------------------------------

rtc62423_device::rtc62423_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msm6242_device(mconfig, RTC62423, tag, owner, clock)
{
}


//-------------------------------------------------
//  rtc72421_device - constructor
//-------------------------------------------------

rtc72421_device::rtc72421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msm6242_device(mconfig, RTC72421, tag, owner, clock)
{
}


//-------------------------------------------------
//  rtc72423_device - constructor
//-------------------------------------------------

rtc72423_device::rtc72423_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msm6242_device(mconfig, RTC72423, tag, owner, clock)
{
}
