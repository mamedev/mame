// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-FXGA C-Bus

Notes:
- Pressing ESC on host system will quit the loader (and reset FXGA at same time);
- $9b4 bit 2 is checked twice after "floppy error", probably as an handshake mechanism;

TODO:
- placeholder;
- Brainstorm a way to communicate with a pcfxga driver;
- Understand how to format floppies in a way that fxga.exe is happy with (from guest system
  in backup menu?);

**************************************************************************************************/

#include "emu.h"
#include "pcfxga.h"

#include "softlist_dev.h"

DEFINE_DEVICE_TYPE(PCFXGA_CBUS, pcfxga_cbus_device, "pcfxga_cbus", "NEC PC-FXGA C-Bus i/f")

pcfxga_cbus_device::pcfxga_cbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PCFXGA_CBUS, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
{
}

void pcfxga_cbus_device::device_add_mconfig(machine_config &config)
{
	SOFTWARE_LIST(config, "cdfx_list").set_original("pcfx");
}

void pcfxga_cbus_device::device_start()
{
	save_item(NAME(m_comms));
}

void pcfxga_cbus_device::device_reset()
{
	std::fill(std::begin(m_comms), std::end(m_comms), 0U);
}

void pcfxga_cbus_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0xffff, *this, &pcfxga_cbus_device::io_map);
	}
}


void pcfxga_cbus_device::io_map(address_map &map)
{
	map(0x09b0, 0x09bf).lrw8(
		NAME([this](offs_t offset) { return m_comms[offset]; }),
		NAME([this](offs_t offset, u8 data) { m_comms[offset] = data; })
	);
}

