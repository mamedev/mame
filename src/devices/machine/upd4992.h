// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/***************************************************************************

    uPD4992 RTC

***************************************************************************/

#pragma once

#ifndef __UPD4992DEV_H__
#define __UPD4992DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UPD4992_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, UPD4992, XTAL_32_768kHz)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> upd4992_device

class upd4992_device : public device_t,
						public device_rtc_interface
{
public:
	// construction/destruction
	upd4992_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;

private:
	enum
	{
		TIMER_CLOCK
		//TIMER_TP,
		//TIMER_DATA_OUT,
		//TIMER_TEST_MODE
	};

	emu_timer *m_timer_clock;
	UINT8 m_rtc_regs[8];
};


// device type definition
extern const device_type UPD4992;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
