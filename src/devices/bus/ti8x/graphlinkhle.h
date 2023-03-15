// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 High-level emulation of a somewhat idealised TI-Graph Link "grey"
 model.  Translates byte-oriented TI-8x link port protocol to/from
 9600 Baud RS232.  A real TI-Graph Link requires some delay between
 bytes.

 The buffer is there so that if you connect two emulated calculators
 together with these it has some chance of working.  The receiving
 calculator can't slow the sending calculator down like it would be able
 to in real life, so you inevitably get overruns without the buffer.
 */
#ifndef MAME_BUS_TI8X_GRAPHLINKHLE_H
#define MAME_BUS_TI8X_GRAPHLINKHLE_H

#pragma once

#include "ti8x.h"


DECLARE_DEVICE_TYPE(TI8X_GRAPH_LINK_HLE, device_ti8x_link_port_interface)

#endif // MAME_BUS_TI8X_GRAPHLINKHLE_H
