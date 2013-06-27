#ifndef __MM58274C_H__
#define __MM58274C_H__


/*
 Initializes the clock chip.
 day1 must be set to a value from 0 (sunday), 1 (monday) ...
 to 6 (saturday) and is needed to correctly retrieve the day-of-week
 from the host system clock.
 */

struct mm58274c_interface
{
	int m_mode24;     /* 24/12 mode */
	int m_day1;       /* first day of week */
};


/***************************************************************************
    MACROS
***************************************************************************/

class mm58274c_device : public device_t,
						public mm58274c_interface
{
public:
	mm58274c_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mm58274c_device() {}

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	TIMER_CALLBACK_MEMBER(rtc_increment_cb);
	TIMER_CALLBACK_MEMBER(rtc_interrupt_cb);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	attotime interrupt_period_table(int val);

	int m_status;     /* status register (*read* from address 0 = control register) */
	int m_control;    /* control register (*write* to address 0) */

	int m_clk_set;    /* clock setting register */
	int m_int_ctl;    /* interrupt control register */


	int m_wday;       /* day of the week (1-7 (1=day1 as set in init)) */
	int m_years1;     /* years (BCD: 0-99) */
	int m_years2;
	int m_months1;    /* months (BCD: 1-12) */
	int m_months2;
	int m_days1;      /* days (BCD: 1-31) */
	int m_days2;
	int m_hours1;     /* hours (BCD : 0-23) */
	int m_hours2;
	int m_minutes1;   /* minutes (BCD : 0-59) */
	int m_minutes2;
	int m_seconds1;   /* seconds (BCD : 0-59) */
	int m_seconds2;
	int m_tenths;     /* tenths of second (BCD : 0-9) */

	emu_timer *m_increment_rtc;
	emu_timer *m_interrupt_timer;
};

extern const device_type MM58274C;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MM58274C_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, MM58274C, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif
