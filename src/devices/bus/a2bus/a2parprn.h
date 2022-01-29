// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************

    Apple II Parallel Printer Interface Card

    20-pin header:

         GND   1   2  ACK
        SP1*   3   4  F
        SP2*   5   6  SP3*
        SP4*   7   8  STR
        SP5*   9  10  DP0
         DP1  11  12  DP2
         DP3  13  14  DP4
         DP5  15  16  DP6
         DP7  17  18  SP6*
        SP7*  19  20  GND

        *spare, SP1-SP5 brought out to pads for wire mods

    DIP jumper B1:

           no connection  NC   1  16  B   acknowledge latch clock
           no connection  NC   2  15  D   printer ready 1
        synchronised ACK   Y   3  14  C   printer ready 2
       synchronised /ACK  /Y   4  13  A   idle strobe
     acknowledge latch Q  /Z   5  12  /X  strobe on access
    acknowlwdge latch /Q   Z   6  11  S5  +5V via 12kΩ resistor
           signal ground GND   7  10  X   strobe after one cycle
            header pin 4   F   8   9  NC  no connection

    In real life, the card only samples the ACK input on rising
    edges of the phase 1 clock.  Acknowledge pulses shorter than one
    microsecond may be ignored.  This limitation is not emulated.

    The card completely ignores the R/W line.  The PROM driver uses
    an indexed write to access the C0nX region.  This produces a
    spurious read, causing data to be latched and triggering a
    delayed strobe pulse.  This is why the delay is important – the
    write needs to happen on the cycle immediately following the
    spurious read to prevent the open bus data from being printed.

    The card was designed to allow user modifications.  There are
    locations at 5A (16-pin) and 5B (20-pin) for additional DIP
    packages.  Common modifications included stretching strobe
    and/or acknowledge pulses to improve reliability.

***********************************************************************/
#ifndef MAME_BUS_A2BUS_A2PARPRN_H
#define MAME_BUS_A2BUS_A2PARPRN_H

#pragma once

#include "a2bus.h"

DECLARE_DEVICE_TYPE(A2BUS_PARPRN, device_a2bus_card_interface)

#endif // MAME_BUS_A2BUS_A2PARPRN_H
