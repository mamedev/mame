/***************************************************************************

    dirtc.c

    Device Real Time Clock interfaces.

***************************************************************************/

#include "emu.h"



//**************************************************************************
//  DEVICE CONFIG RTC INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_config_rtc_interface - constructor
//-------------------------------------------------

device_config_rtc_interface::device_config_rtc_interface(const machine_config &mconfig, device_config &devconfig)
	: device_config_interface(mconfig, devconfig)
{
}


//-------------------------------------------------
//  ~device_config_rtc_interface - destructor
//-------------------------------------------------

device_config_rtc_interface::~device_config_rtc_interface()
{
}



//**************************************************************************
//  DEVICE RTC INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_rtc_interface - constructor
//-------------------------------------------------

device_rtc_interface::device_rtc_interface(running_machine &machine, const device_config &config, device_t &device)
	: device_interface(machine, config, device),
	  m_rtc_config(dynamic_cast<const device_config_rtc_interface &>(config))
{
}


//-------------------------------------------------
//  ~device_rtc_interface - destructor
//-------------------------------------------------

device_rtc_interface::~device_rtc_interface()
{
}
