/***************************************************************************

    dirtc.c

    Device Real Time Clock interfaces.

***************************************************************************/

#include "emu.h"



//**************************************************************************
//  DEVICE RTC INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_rtc_interface - constructor
//-------------------------------------------------

device_rtc_interface::device_rtc_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device)
{
}


//-------------------------------------------------
//  ~device_rtc_interface - destructor
//-------------------------------------------------

device_rtc_interface::~device_rtc_interface()
{
}
