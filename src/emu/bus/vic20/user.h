// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore VIC-20 User Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       CB1
                 /RESET       3      C       PB0
                   JOY0       4      D       PB1
                   JOY1       5      E       PB2
                   JOY2       6      F       PB3
              LIGHT PEN       7      H       PB4
        CASSETTE SWITCH       8      J       PB5
                    ATN       9      K       PB6
                  +9VAC      10      L       PB7
                  +9VAC      11      M       CB2
                    GND      12      N       GND

**********************************************************************/

#pragma once

#ifndef __VIC20_USER_PORT__
#define __VIC20_USER_PORT__

#include "bus/pet/user.h"

SLOT_INTERFACE_EXTERN( vic20_user_port_cards );

#endif
