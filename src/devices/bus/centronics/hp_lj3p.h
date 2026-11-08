// license:BSD-3-Clause
// copyright-holders:rfka01
/**********************************************************************

    HP LaserJet IIIP laser printer emulation

    The printer has a Centronics parallel and an RS-232C serial interface
    built in; it can be connected to either.

**********************************************************************/

#ifndef MAME_BUS_CENTRONICS_HP_LJ3P_H
#define MAME_BUS_CENTRONICS_HP_LJ3P_H

#pragma once

#include "ctronics.h"
#include "bus/rs232/rs232.h"

// device type declaration: one device for a parallel or a serial port
DECLARE_DEVICE_TYPE(HP_LJ3P, device_t)

#endif // MAME_BUS_CENTRONICS_HP_LJ3P_H
