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
const device_type SONY_LDP1450 = &device_creator<sony_ldp1450_device>;
const device_type PIONEER_PR7820 = &device_creator<pioneer_pr7820_device>;
const device_type PHILLIPS_22VP932 = &device_creator<phillips_22vp932_device>;
