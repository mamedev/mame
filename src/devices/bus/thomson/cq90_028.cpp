// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// CQ 90-028 - QDD drive controller built from a motorola 6852 (serial chip)
//
// Handles n? QDD drives (QD 90-128)

// Nonfunctional, essentially because we have no container for the QDD
// (it's not a floppy, the closest equivalent would be a digital tape)

#include "emu.h"
#include "cq90_028.h"

DEFINE_DEVICE_TYPE(CQ90_028, cq90_028_device, "cq90_028", "Thomson CQ90-028 QDD controller")

cq90_028_device::cq90_028_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CQ90_028, tag, owner, clock),
	thomson_extension_interface(mconfig, *this),
	m_serial(*this, "serial"),
	m_rom(*this, "rom")
{
}

ROM_START(cq90_028)
	ROM_REGION( 0x7c0, "rom", 0 )
	ROM_LOAD ( "cq90-028.rom", 0x000, 0x7c0, CRC(ca4dba3d) SHA1(949c1f777c892da62c242215d79757d61e71e62b) )
ROM_END

void cq90_028_device::rom_map(address_map &map)
{
	map(0x000, 0x7bf).rom().region(m_rom, 0);
}

void cq90_028_device::io_map(address_map &map)
{
	map(0x0, 0x1).rw(m_serial, FUNC(mc6852_device::read), FUNC(mc6852_device::write));
	map(0x8, 0x8).rw(FUNC(cq90_028_device::status_r), FUNC(cq90_028_device::drive_w));
	map(0xc, 0xc).w(FUNC(cq90_028_device::motor_w));
}

const tiny_rom_entry *cq90_028_device::device_rom_region() const
{
	return ROM_NAME(cq90_028);
}

void cq90_028_device::device_add_mconfig(machine_config &config)
{
	MC6852(config, m_serial, 16_MHz_XTAL / 16); // Comes from the main board
	// Base tx/rx clock is 101564Hz
	// There's probably a pll in the gate array
}

void cq90_028_device::device_start()
{
}

void cq90_028_device::device_reset()
{
}

void cq90_028_device::drive_w(u8 data)
{
	logerror("drive_w %02x\n", data);
}

void cq90_028_device::motor_w(u8 data)
{
	logerror("motor_w %02x\n", data);
}

u8 cq90_028_device::status_r()
{
	// 40 = disk absent
	// 80 = index pulse
	return 0x40;
}
