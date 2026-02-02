// license:BSD-3-Clause
// copyright-holders:


#include "emu.h"
#include "pc_vga_chips.h"

#include "screen.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(F65535_VGA,  f65535_vga_device,  "f65535_vga",  "Chips and Technologies F65535 VGA i/f")

f65535_vga_device::f65535_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, F65535_VGA, tag, owner, clock)
{
}


void f65535_vga_device::device_start()
{
	svga_device::device_start();
}

void f65535_vga_device::device_reset()
{
	svga_device::device_reset();
}
