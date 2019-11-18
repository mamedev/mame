// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldstub.c

    Laserdisc player stubs.

*************************************************************************/

#include "emu.h"
#include "ldstub.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(PIONEER_PR7820,  pioneer_pr7820_device,  "pr7820",  "Pioneer PR-7820")
DEFINE_DEVICE_TYPE(PHILIPS_22VP932, philips_22vp932_device, "22vp932", "Philips 22VP932")


pioneer_pr7820_device::pioneer_pr7820_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: laserdisc_device(mconfig, PIONEER_PR7820, tag, owner, clock)
{
}


philips_22vp932_device::philips_22vp932_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: laserdisc_device(mconfig, PHILIPS_22VP932, tag, owner, clock)
{
}
