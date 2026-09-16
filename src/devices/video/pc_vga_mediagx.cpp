// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

MediaGX VGA virtual interface

TODO:
- Currently cheat around software VGA, MediaGX notoriously triggers SMI for every access to
  VGA legacy ranges, which is horrible both for emulation purposes and for performance.
- A good benchmark about fixing pc_vga infrastructure:
  it connects thru a VGA connector anyway & it has VGA features (DDC, HW cursor, BitBLT)

**************************************************************************************************/

#include "emu.h"
#include "pc_vga_mediagx.h"

#include "screen.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MEDIAGX_VGA,  mediagx_vga_device,  "mediagx_vga",  "MediaGX Virtual VGA i/f")

mediagx_vga_device::mediagx_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: svga_device(mconfig, MEDIAGX_VGA, tag, owner, clock)
{
	m_crtc_space_config = address_space_config("crtc_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(mediagx_vga_device::crtc_map), this));
}

void mediagx_vga_device::device_start()
{
	svga_device::device_start();
}

void mediagx_vga_device::device_reset()
{
	svga_device::device_reset();
}

void mediagx_vga_device::crtc_map(address_map &map)
{
	svga_device::crtc_map(map);
	// astropc.cpp read this at boot, DDC?
	map(0x3e, 0x3e).lr8(NAME([] () { return 0xff; }));
}

uint8_t mediagx_vga_device::mem_r(offs_t offset)
{
	return svga_device::mem_r(offset);
}

void mediagx_vga_device::mem_w(offs_t offset, uint8_t data)
{
	svga_device::mem_w(offset, data);
}
