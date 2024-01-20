// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Zoran ZR36057 / ZR36067 PCI-based chipsets

PCI glue logic for multimedia MJPEG, MPEG1 & DVD.
Paired with every single TV standard for video capture in the fairly decent number of subvendor
iterations.
- https://www.kernel.org/doc/html/v4.14/media/v4l-drivers/zoran.html
- misc/sliver.cpp uses ZR36050
- misc/magictg.cpp uses ZR36016

**************************************************************************************************/

#include "emu.h"
#include "zr36057.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(ZR36057_PCI, zr36057_device,   "zr36057",   "Zoran ZR36057-based PCI Enhanced Multimedia Controller card")
//DEFINE_DEVICE_TYPE(ZR36067_PCI, zr36067_device,   "zr36067",   "Zoran ZR36067-based PCI AV Controller card")


zr36057_device::zr36057_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
	// ZR36057PQC Video cutting chipset
	// device ID reportedly same for ZR36057 and ZR36067, revision 0x02 for latter.
	// Four known subvendors:
	// - 0x10317efe: Pinnacle/Miro DC10+
	// - 0x1031fc00: Pinnacle/Miro DC50, Motion JPEG Capture/CODEC Board
	// - 0x12f88a02: Electronic Design GmbH Tekram Video Kit
	// - 0x13ca4231: Iomega JPEG/TV Card
	// NOTE: subvendor is omitted in '36057 design (missing?), driven at PCIRST time to 32 pins in
	// '36067 thru pull-up or pull-down resistors (subvendor responsibility?)
	set_ids(0x11de6057, 0x01, 0x040000, 0x10317efe);
}

zr36057_device::zr36057_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: zr36057_device(mconfig, ZR36057_PCI, tag, owner, clock)
{
}

void zr36057_device::device_add_mconfig(machine_config &config)
{

}

void zr36057_device::device_start()
{
	pci_card_device::device_start();

	add_map(4096, M_MEM, FUNC(zr36057_device::map));

	// INTA#
	intr_pin = 1;
}

void zr36057_device::device_reset()
{
	pci_card_device::device_reset();

	// fast DEVSEL#
	command = 0x0000;
	status = 0x0000;
	intr_line = 0x0a;
	// TODO: PCI regs 0x3e/0x3f max_lat = 0x10 (4 usec), min_gnt = 0x02 (0.5 usec)

	remap_cb();
}

void zr36057_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}

void zr36057_device::map(address_map &map)
{
}
