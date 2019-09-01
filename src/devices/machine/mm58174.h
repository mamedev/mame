// license:BSD-3-Clause
// copyright-holders:Raphael Nabet,Sergey Svishchev

#ifndef MAME_MACHINE_MM58174_H
#define MAME_MACHINE_MM58174_H

/***************************************************************************
    MACROS
***************************************************************************/

class mm58174_device : public device_t
{
public:
	mm58174_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	TIMER_CALLBACK_MEMBER(rtc_increment_cb);
	TIMER_CALLBACK_MEMBER(rtc_interrupt_cb);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state

	attotime interrupt_period_table(int val);

	int m_status;     /* status register (internal) */
	int m_control;    /* control register (write to address 14) */

	int m_int_ctl;    /* interrupt control register */

	int m_years;
	int m_wday;       /* day of the week (1-7 (1=day1 as set in init)) */
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

DECLARE_DEVICE_TYPE(MM58174, mm58174_device)

#endif // MAME_MACHINE_MM58174_H
