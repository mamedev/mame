// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore Plus/4 User Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       P0
                _BRESET       3      C       RxD
                     P2       4      D       RTS
                     P3       5      E       DTR
                     P4       6      F       P7
                     P5       7      H       DCD
                    RxC       8      J       P6
                    ATN       9      K       P1
                  +9VAC      10      L       DSR
                  +9VAC      11      M       TxD
                    GND      12      N       GND

**********************************************************************/

#pragma once

#ifndef __PLUS4_USER_PORT__
#define __PLUS4_USER_PORT__

#include "bus/vic20/user.h"

SLOT_INTERFACE_EXTERN( plus4_user_port_cards );

#endif
