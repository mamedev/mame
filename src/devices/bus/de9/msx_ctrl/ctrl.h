// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    MSX General Purpose port

Pin assignment:
pin 1 - Up (I)
pin 2 - Down (I)
pin 3 - Left (I)
pin 4 - Right (I)
pin 5 - +5V
pin 6 - Button 1 (I/O)
pin 7 - Button 2 (I/O)
pin 8 - Strobe (O)
pin 9 - GND

**********************************************************************/

#ifndef MAME_BUS_DE9_MSX_CTRL_CTRL_H
#define MAME_BUS_DE9_MSX_CTRL_CTRL_H

#pragma once

void msx_general_purpose_port_devices(device_slot_interface &device);

#endif // MAME_BUS_DE9_MSX_CTRL_CTRL_H
