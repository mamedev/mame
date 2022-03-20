// license:BSD-3-Clause
// copyright-holders:Vas Crabb, R. Belmont
/***********************************************************************

    Videx Uniprint Parallel Interface Card

    Manual, including schematic and theory of operation:
    https://www.apple.asimov.net/documentation/hardware/io/Videx%20UniPrint%20Manual.pdf

        Connector P2 pinout (pins 1 and 11 are at the top of the card):
        STB    1  11  GND
        D0     2  12  GND
        D1     3  13  GND
        D2     4  14  GND
        D3     5  15  GND
        D4     6  16  GND
        D5     7  17  GND
        D6     8  18  GND
        D7     9  19  GND
        ACK   10  20  GND

***********************************************************************/
#ifndef MAME_BUS_A2BUS_UNIPRINT_H
#define MAME_BUS_A2BUS_UNIPRINT_H

#pragma once

#include "a2bus.h"

DECLARE_DEVICE_TYPE(A2BUS_UNIPRINT, device_a2bus_card_interface)

#endif // MAME_BUS_A2BUS_UNIPRINT_H
