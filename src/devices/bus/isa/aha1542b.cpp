// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Adaptec AHA-1540/42A and AHA-1540/42B SCSI controllers

    The alternate BIOSes using port 334h instead of 330h are provided due
    to certain MIDI cards requiring the 330h port.

***************************************************************************/

#include "emu.h"
#include "aha1542b.h"

#include "cpu/i8085/i8085.h"
#include "machine/aic6250.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_hd.h"


DEFINE_DEVICE_TYPE(AHA1542A, aha1542a_device, "aha1542a", "AHA-1542A SCSI Controller")
DEFINE_DEVICE_TYPE(AHA1542B, aha1542b_device, "aha1542b", "AHA-1542B SCSI Controller")


aha154x_device::aha154x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_bios(*this, "bios")
{
}

aha1542a_device::aha1542a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: aha154x_device(mconfig, AHA1542A, tag, owner, clock)
{
}

aha1542b_device::aha1542b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: aha154x_device(mconfig, AHA1542B, tag, owner, clock)
{
}

void aha154x_device::device_start()
{
}

void aha154x_device::i8085_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("mcode", 0);
	map(0x8000, 0x800f).m("scsi:7:scsic", FUNC(aic6250_device::map));
	map(0xe000, 0xe7ff).ram();
}

static void aha154x_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("scsic", AIC6250);
}

void aha154x_device::scsic_config(device_t *device)
{
	device->set_clock(20'000'000);
	downcast<aic6250_device &>(*device).int_cb().set_inputline("^^localcpu", I8085_RST65_LINE);
}

void aha154x_device::device_add_mconfig(machine_config &config)
{
	i8085a_cpu_device &localcpu(I8085A(config, "localcpu", 10'000'000));
	localcpu.set_addrmap(AS_PROGRAM, &aha154x_device::i8085_map);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", aha154x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", aha154x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", aha154x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", aha154x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", aha154x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", aha154x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", aha154x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", aha154x_scsi_devices, "scsic", true)
		.set_option_machine_config("scsic", [this] (device_t *device) { scsic_config(device); });

	DP8473(config, m_fdc, 24_MHz_XTAL);
}


ROM_START(aha1542a)
	ROM_REGION(0x4000, "bios", 0)
	ROM_LOAD("b_9700.bin", 0x0000, 0x4000, CRC(35f546e9) SHA1(f559b08f52044f53836021a83f56f628e32216bd))

	ROM_REGION(0x4000, "mcode", 0)
	ROM_LOAD("m_e7bc.bin", 0x0000, 0x4000, CRC(985b7a31) SHA1(bba0d84fa1b67ea71905953c25201fa2020cf465))
ROM_END

ROM_START(aha1542b)
	ROM_REGION(0x4000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v310", "AT/SCSI BIOS Version 3.10")
	ROMX_LOAD("adaptec_inc_420412-00_b_bios_bc00_1990.bin", 0x0000, 0x4000, CRC(bd3f74e7) SHA1(c38d82fd50e5439812fa093e0d4f5fd136c63844), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v310a", "AT/SCSI BIOS Version 3.10A (port 334h)")
	ROMX_LOAD("154xp334.bin", 0x0000, 0x4000, CRC(4911f232) SHA1(2e24ce380c6f7694c45484019857cb919e2a9965), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v311", "AT/SCSI BIOS Version 3.11")
	ROMX_LOAD("bios_c900.u13", 0x0000, 0x4000, CRC(4660d0c1) SHA1(a581291de96836b6f6cc0b6244b8fa1ee333346a), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v320g", "AT/SCSI BIOS Version 3.20 (> 1 GB support)")
	ROMX_LOAD("b_bd00.bin", 0x0000, 0x4000, CRC(2387197b) SHA1(703e1fe1ba924c02d617ac37ec7a20e12bef1cc7), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "v320gt", "AT/SCSI BIOS Version 3.20 (extended timeout)")
	ROMX_LOAD("b_b300.bin", 0x0000, 0x4000, CRC(4c5b07d8) SHA1(692e824f916f55519c9905839f5f6609f5e8c0a5), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "v320a", "AT/SCSI BIOS Version 3.20Alt (port 334h)")
	ROMX_LOAD("b_ac00.bin", 0x0000, 0x4000, CRC(becd6d08) SHA1(b5e7cbdeb241c1ff57602291e87c58ac0ee72d54), ROM_BIOS(5))

	ROM_REGION(0x4000, "mcode", 0)
	ROMX_LOAD("adaptec_inc_434108-00_a_mcode_fc8a_1990.bin", 0x0000, 0x4000, CRC(6801f89e) SHA1(33d36bc93734105b950414e7c433a283032838e9), ROM_BIOS(0))
	ROMX_LOAD("adaptec_inc_434108-00_a_mcode_fc8a_1990.bin", 0x0000, 0x4000, CRC(6801f89e) SHA1(33d36bc93734105b950414e7c433a283032838e9), ROM_BIOS(1)) // assumed compatible with v310a BIOS
	ROMX_LOAD("firmware_62d3.u12", 0x0000, 0x4000, CRC(6056ca33) SHA1(8dd4aaffcb107dbcc85ac87d878fa6093b904a20), ROM_BIOS(2))
	ROMX_LOAD("m_3054.bin", 0x0000, 0x4000, CRC(461b1885) SHA1(50dc49b0fd88b116b83e3c71f58c758b618d1ddf), ROM_BIOS(3))
	ROMX_LOAD("m_5d98.bin", 0x0000, 0x4000, CRC(f7d51536) SHA1(5ad1bb4bde3e8c30380b05d32ac273c781ab12a8), ROM_BIOS(4)) // also provided with v320g BIOS
	ROMX_LOAD("m_3054.bin", 0x0000, 0x4000, CRC(461b1885) SHA1(50dc49b0fd88b116b83e3c71f58c758b618d1ddf), ROM_BIOS(5))
ROM_END

const tiny_rom_entry *aha1542a_device::device_rom_region() const
{
	return ROM_NAME(aha1542a);
}

const tiny_rom_entry *aha1542b_device::device_rom_region() const
{
	return ROM_NAME(aha1542b);
}
