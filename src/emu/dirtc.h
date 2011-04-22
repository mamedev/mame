/***************************************************************************

    dirtc.h

    Device Real Time Clock interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIRTC_H__
#define __DIRTC_H__


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> device_config_rtc_interface

// class representing interface-specific configuration rtc
class device_config_rtc_interface : public device_config_interface
{
public:
	// construction/destruction
	device_config_rtc_interface(const machine_config &mconfig, device_config &device);
	virtual ~device_config_rtc_interface();
};



// ======================> device_rtc_interface

// class representing interface-specific live rtc
class device_rtc_interface : public device_interface
{
public:
	// construction/destruction
	device_rtc_interface(running_machine &machine, const device_config &config, device_t &device);
	virtual ~device_rtc_interface();

	// configuration access
	const device_config_rtc_interface &rtc_config() const { return m_rtc_config; }

protected:
	// derived class overrides
	virtual void rtc_set_time(int year, int month, int day, int day_of_week, int hour, int minute, int second) = 0;
	virtual bool rtc_is_year_2000_compliant() = 0;

	// configuration
	const device_config_rtc_interface &m_rtc_config;	// reference to our device_config_execute_interface
};


#endif	/* __DIRTC_H__ */
