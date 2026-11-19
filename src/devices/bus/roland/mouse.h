// license:BSD-3-Clause
// copyright-holders:Carl Lom
/**********************************************************************

    Roland MU-1 mouse emulation

Uses the MSX mouse protocol (see bus/msx/ctrl/mouse.cpp) — the MU-1
has the same pinout and protocol, and MSX mice are known to work when
plugged into a Roland EXT port.

**********************************************************************/

#ifndef MAME_BUS_ROLAND_MOUSE_H
#define MAME_BUS_ROLAND_MOUSE_H

#pragma once

#include "extport.h"


DECLARE_DEVICE_TYPE(ROLAND_EXT_MOUSE, device_roland_ext_port_interface)

#endif // MAME_BUS_ROLAND_MOUSE_H
