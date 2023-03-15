// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM75 MCU

*/

#include "emu.h"
#include "mm75.h"


DEFINE_DEVICE_TYPE(MM75, mm75_device, "mm75", "Rockwell MM75 A7500") // stripped-down MM76 (no serial i/o, less pins)


// constructor
mm75_device::mm75_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm76_device(mconfig, MM75, tag, owner, clock, 10, address_map_constructor(FUNC(mm75_device::program_0_6k), this), 6, address_map_constructor(FUNC(mm75_device::data_48x4), this))
{ }


// initialize
void mm75_device::device_start()
{
	mm76_device::device_start();
	set_d_pins(9);
}
