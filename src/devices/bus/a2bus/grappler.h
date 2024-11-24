// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************

    Orange Micro Grappler/Grappler+ Printer Interface

    With references to schematics from The Grappler™ Interface
    Operators Manual (Copyright 1982 Orance Micro, Inc.) and
    Grappler™+ Printer Interface Series Operators Manual (© Orange
    Micro, Inc. 1982).  This uses the IC locations on the "long"
    version of the Grappler+ card - the newer "short" version of the
    card has slightly different circuitry and completely different IC
    locations.

    26-pin two-row header to printer:

        STB    1   2  GND
        D0     3   4  GND
        D1     5   6  GND
        D2     7   8  GND
        D3     9  10  GND
        D4    11  12  GND
        D5    23  14  GND
        D6    15  16  GND
        D7    17  18  GND
        ACK   19  20  GND
        BUSY  21  22  GND
        P.E.  23  24  GND
        SLCT  25  26  GND

    Orange Micro Buffered Grappler+ Printer Interface

    26-pin two-row header to printer:

        STB    1   2  GND
        D0     3   4  GND
        D1     5   6  GND
        D2     7   8  GND
        D3     9  10  GND
        D4    11  12  GND
        D5    23  14  GND
        D6    15  16  GND
        D7    17  18  GND
        ACK   19  20  GND
        BUSY  21  22  GND
        P.E.  23  24  GND
        SLCT  25  26  N/C

***********************************************************************/
#ifndef MAME_BUS_A2BUS_GRAPPLER_H
#define MAME_BUS_A2BUS_GRAPPLER_H

#pragma once

#include "a2bus.h"


DECLARE_DEVICE_TYPE(A2BUS_GRAPPLER, device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_GRAPPLERPLUS, device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_BUFGRAPPLERPLUS, device_a2bus_card_interface)
DECLARE_DEVICE_TYPE(A2BUS_BUFGRAPPLERPLUSA, device_a2bus_card_interface)

#endif // MAME_BUS_A2BUS_GRAPPLER_H
