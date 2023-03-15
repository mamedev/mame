// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***********************************************************************

    Apple II Parallel Interface Card (670-0021)

    DB25 connector, with Centronics assignments:

         Data In, Bit 0   1
          Signal Ground   2  19  GND
         Data In, Bit 2   3
          Signal Ground   4
        Data Out, Bit 0   5   2  D0
        Data Out, Bit 1   6   3  D1
             (blocked)*   7
        Data Out, Bit 2   8   4  D2
                     --   9
                     --  10
        Data Out, Bit 5  11   7  D5
        Data Out, Bit 6  12   8  D6
        Data Out, Bit 7  13   9  D7
         Data In, Bit 4  14
             Strobe Out  15   1  /STROBE
         Acknowledge In  16  10  /ACK
         Data In, Bit 1  17
         Data In, Bit 7  18  18  +5V PULLUP
         Data In, Bit 5  19  12  POUT
          Signal Ground  20
         Data In, Bit 6  21  13  SEL
        Data Out, Bit 3  22   5  D3
        Data Out, Bit 4  23   6  D4
          Signal Ground  24  16  0V
         Data In, Bit 3  25  32  /FAULT

        *wired to 500ns strobe but connector hole is blocked

    This card has significant flaws:
    * The strobe pulse begins on the same rising edge of the phase 1
      clock as the data is latched.  The parallel load enable input
      to the strobe time counter (6A) is delayed slightly, but the
      load happens on the rising phase 1 clock edge.  This could
      glitch on a real printer.  MAME always sets the data outputs
      before starting the strobe pulse.
    * Acknowledge is ignored while the strobe output is active.  If
      the printer acknowledges the data before the end of the strobe
      pulse, the card will miss it and wait forever.

***********************************************************************/
#ifndef MAME_BUS_A2BUS_A2PIC_H
#define MAME_BUS_A2BUS_A2PIC_H

#pragma once

#include "a2bus.h"

DECLARE_DEVICE_TYPE(A2BUS_PIC, device_a2bus_card_interface)

#endif // MAME_BUS_A2BUS_A2PIC_H
