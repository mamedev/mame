// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

[Cyrix/National Semiconductor/AMD] [MediaGX/Geode] [Cx/CS]5530 VIDEO implementation (XpressGRAPHICS?)

TODO:
- extensions for host display section (GX_BASE+8300h);

**************************************************************************************************/

#include "emu.h"
#include "mediagx_cs5530_video.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(MEDIAGX_CS5530_VIDEO, mediagx_cs5530_video_device, "mediagx_cs5530_video", "MediaGX CS5530 Video Controller")

mediagx_cs5530_video_device::mediagx_cs5530_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, MEDIAGX_CS5530_VIDEO, tag, owner, clock)
{
	set_ids(0x10780104, 0x00, 0x030000, 0x00);
}

void mediagx_cs5530_video_device::config_map(address_map &map)
{
	pci_device::config_map(map);
//	map(0x14, 0xff).unmaprw(); // <reserved>
}

void mediagx_cs5530_video_device::io_map(address_map &map)
{

}

void mediagx_cs5530_video_device::device_start()
{
	pci_device::device_start();

	add_map(4*1024, M_MEM, FUNC(mediagx_cs5530_video_device::io_map));

	// no INT pin
}

void mediagx_cs5530_video_device::device_reset()
{
	pci_device::device_reset();

	command = 0x0000;
	command_mask = 0x0003;
	status = 0x0280;
}
