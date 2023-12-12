// license:BSD-3-Clause
// copyright-holders:Antoine Mine, Olivier Galibert, Felipe Sanches
//
// HD-AE5000, Hard Disk & Audio Extension for Technics KN5000 emulation
//
// The HD-AE5000 was an extension board for the Technics KN5000 musical keyboard.
// It provided a hard-disk, additional audio outputs and a serial port to interface
// with a computer to transfer files to/from the hard-drive.

#include "emu.h"
#include "hdae5000.h"

DEFINE_DEVICE_TYPE(HDAE5000, hdae5000_device, "hdae5000", "HD-AE5000, Hard Disk & Audio Extension")

hdae5000_device::hdae5000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HDAE5000, tag, owner, clock)
	, kn5000_extension_interface(mconfig, *this)
	, m_hdd(*this, "hdd")
	, m_ppi(*this, "ppi")
	, m_rom(*this, "rom")
{
}

ROM_START(hdae5000)
	ROM_REGION16_LE(0x80000, "rom" , 0)
	ROM_DEFAULT_BIOS("v2.01i")

	ROM_SYSTEM_BIOS(0, "v1.10i", "Version 1.10i - July 6th, 1998")
	ROMX_LOAD("hd-ae5000_v1_10i.ic4", 0x000000, 0x80000, CRC(7461374b) SHA1(6019f3c28b6277730418974dde4dc6893fced00e), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v1.15i", "Version 1.15i - October 13th, 1998")
	ROMX_LOAD("hd-ae5000_v1_15i.ic4", 0x000000, 0x80000, CRC(e76d4b9f) SHA1(581fa58e2cd6fe381cfc312c73771d25ff2e662c), ROM_BIOS(1))

	// Version 2.01i is described as having "additions like lyrics display etc."
	ROM_SYSTEM_BIOS(2, "v2.01i", "Version 2.01i - January 15th, 1999") // installation file indicated "v2.0i" but signature inside the ROM is "v2.01i"
	ROMX_LOAD("hd-ae5000_v2_01i.ic4", 0x000000, 0x80000, CRC(961e6dcd) SHA1(0160c17baa7b026771872126d8146038a19ef53b), ROM_BIOS(2))
ROM_END

void hdae5000_device::rom_map(address_map &map)
{
	//map(0x130000, 0x13ffff).m(m_hddc, FUNC(?_device::?)); // Hard-drive Controller (model?) IC? on HD-AE5000 board 
	//map(0x160000, 0x16ffff) ... Optional parallel port interface (NEC uPD71055) IC9
	map(0x160000, 0x160000).lrw8([this](offs_t a) { return m_ppi->read(0); }, "ppi_r0", [this](offs_t a, u8 data) { m_ppi->write(0, data); }, "ppi_w0");
	map(0x160002, 0x160002).lrw8([this](offs_t a) { return m_ppi->read(1); }, "ppi_r1", [this](offs_t a, u8 data) { m_ppi->write(1, data); }, "ppi_w1");
	map(0x160004, 0x160004).lrw8([this](offs_t a) { return m_ppi->read(2); }, "ppi_r2", [this](offs_t a, u8 data) { m_ppi->write(2, data); }, "ppi_w2");
	map(0x160006, 0x160006).lrw8([this](offs_t a) { return m_ppi->read(3); }, "ppi_r3", [this](offs_t a, u8 data) { m_ppi->write(3, data); }, "ppi_w3");
	map(0x200000, 0x27ffff).ram(); //optional hsram: 2 * 256k bytes Static RAM @ IC5, IC6 (CS5)
	map(0x280000, 0x2fffff).rom().region(m_rom, 0); // 512k bytes FLASH ROM @ IC4 (CS5)
	map(0x800000, 0x8fffff).ram(); // hack <- I think this is the SRAM from the HD-AE5000 board
}

void hdae5000_device::io_map(address_map &map)
{
}

const tiny_rom_entry *hdae5000_device::device_rom_region() const
{
	return ROM_NAME(hdae5000);
}

void hdae5000_device::device_add_mconfig(machine_config &config)
{
	/* Optional Hard Disk - HD-AE5000 */
	IDE_HARDDISK(config, m_hdd, 0);

	/* Optional Parallel Port */
	I8255(config, m_ppi); // actual chip is a NEC uPD71055 @ IC9 on the HD-AE5000 board
	// m_ppi->in_pa_callback().set(FUNC(?_device::ppi_in_a));
	// m_ppi->out_pb_callback().set(FUNC(?_device::ppi_out_b));
	// m_ppi->in_pc_callback().set(FUNC(?_device::ppi_in_c));
	// m_ppi->out_pc_callback().set(FUNC(?_device::ppi_out_c));

//  we may later add this, for the auxiliary audio output provided by this extension board:
// 	SPEAKER(config, "mono").front_center();
}

void hdae5000_device::device_start()
{
//	save_item(NAME(m_...));
}

void hdae5000_device::device_reset()
{
}
