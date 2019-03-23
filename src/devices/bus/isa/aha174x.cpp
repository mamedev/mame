// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Adaptec AHA-1740/44 and AHA-1740A/42A Fast SCSI host adapters

    These are actually EISA cards, though they also have a compatibility
    mode that provides an interface like the older ISA AHA-154X series.

    On the AHA-1740 and AHA-1744, the HPC microcode is stored in an EEPROM
    (of unknown type), allowing it to be reprogrammed by the host. This is
    not possible on the AHA-1740A and AHA-1744A, which use a conventional
    27C256 EPROM for the microcode. In both cases the HPC copies the code
    to and then executes it out of RAM.

    Though the AHA-1740 and AHA-1740A have different board layouts, they
    share the following ICs:

        AIC-565 Bus Auxiliary Interface Chip
        AIC-575 EISA Configuration Chip
        AIC-4600 HPC (HPC46003V20)
        AIC-6251A SCSI Interface and Protocol Chip
        IDT7201 512x9 FIFO (2 on board)
        Intel 82355 Bus Master Interface Controller

    AHA-1742A is the same as AHA-1740A, only with the FDC populated.

    AHA-1744 uses the same layout as AHA-1740, but populates the area
    around the SCSI port with DS36F95J differential drivers.

***************************************************************************/

#include "emu.h"
#include "aha174x.h"

#include "machine/aic6250.h"
//#include "machine/i82355.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_hd.h"

DEFINE_DEVICE_TYPE(AHA1740, aha1740_device, "aha1740", "AHA-1740 Fast SCSI Host Adapter")
DEFINE_DEVICE_TYPE(AHA1742A, aha1742a_device, "aha1742a", "AHA-1742A Fast SCSI Host Adapter")


aha174x_device::aha174x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_hpc(*this, "hpc")
	, m_bios(*this, "bios")
{
}

aha1740_device::aha1740_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: aha174x_device(mconfig, AHA1740, tag, owner, clock)
{
}

aha1742a_device::aha1742a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: aha174x_device(mconfig, AHA1742A, tag, owner, clock)
	, m_fdc(*this, "fdc")
{
}

void aha174x_device::device_start()
{
}


void aha174x_device::hpc_map(address_map &map)
{
	map(0x5000, 0x500f).m("scsi:7:scsic", FUNC(aic6251a_device::map));
	map(0x8000, 0xffff).rom().region("mcode", 0);
}

static void aha174x_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("scsic", AIC6251A);
}

void aha174x_device::scsic_config(device_t *device)
{
	device->set_clock(40_MHz_XTAL / 2); // divider not verified
}

void aha1740_device::device_add_mconfig(machine_config &config)
{
	HPC46003(config, m_hpc, 40_MHz_XTAL / 2);
	m_hpc->set_addrmap(AS_PROGRAM, &aha1740_device::hpc_map);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", aha174x_scsi_devices, "scsic", true)
		.set_option_machine_config("scsic", [this] (device_t *device) { scsic_config(device); });
}

void aha1742a_device::device_add_mconfig(machine_config &config)
{
	HPC46003(config, m_hpc, 40_MHz_XTAL / 2);
	m_hpc->set_addrmap(AS_PROGRAM, &aha1742a_device::hpc_map);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", aha174x_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", aha174x_scsi_devices, "scsic", true)
		.set_option_machine_config("scsic", [this] (device_t *device) { scsic_config(device); });

	N82077AA(config, m_fdc, 24_MHz_XTAL);
}


ROM_START(aha1740)
	ROM_REGION(0x4000, "bios", 0)
	ROM_LOAD("b_dc00.bin", 0x0000, 0x4000, CRC(056d75ec) SHA1(8ca143adfc7d20ad5d49f14dedabc8276454bf9e))

	ROM_REGION16_LE(0x8000, "mcode", 0)
	ROM_SYSTEM_BIOS(0, "v140st", "BIOS v1.40 (Standard Mode)")
	ROMX_LOAD("standard.bin", 0x0000, 0x8000, CRC(8c15c6a2) SHA1(77e15b0244e3a814f53f957270e6474a8a839955), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v140en", "BIOS v1.40 (Enhanced Mode)")
	ROMX_LOAD("enhanced.bin", 0x0000, 0x8000, CRC(84b3df89) SHA1(a718c3ea5443a609b4b20bfe48be18193737ad25), ROM_BIOS(1))
	// Adaptec's help file claims that "the EEPROM on the board can hold firmware for both modes simultaneously."
	// The AHA-174XA firmware images obviously have this, but the files provided here do not agree.
ROM_END

ROM_START(aha1742a)
	ROM_REGION(0x4000, "bios", 0)
	ROM_DEFAULT_BIOS("v140")
	ROM_SYSTEM_BIOS(0, "v134", "BIOS v1.34")
	ROMX_LOAD("adaptec_inc_450214-00_a_bios_8800_1991.u47", 0x0000, 0x4000, CRC(6cf06151) SHA1(0da45634b12b33fc886920d065cc8ffb2cf376b8), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v140", "BIOS v1.40")
	ROMX_LOAD("adaptec_inc_450216-00_a_bios_dc00_1992.u47", 0x0000, 0x4000, CRC(056d75ec) SHA1(8ca143adfc7d20ad5d49f14dedabc8276454bf9e), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v140s", "BIOS v1.40 (Extended Timeout)")
	ROMX_LOAD("b_f100.bin", 0x0000, 0x4000, CRC(b695acc0) SHA1(683112fafdf83d5eb89237d9215f7d6eacc6eeaf), ROM_BIOS(2))

	ROM_REGION16_LE(0x8000, "mcode", 0)
	ROMX_LOAD("adaptec_inc_450117-00_c_mcode_23a8_1991.u10", 0x0000, 0x8000, NO_DUMP, ROM_BIOS(0))
	ROMX_LOAD("adaptec_inc_450113-00_d_mcode_b7d6_1992.u10", 0x0000, 0x8000, CRC(0a55a555) SHA1(ff400f56b33f0ad94e34564d7715a0773b335844), ROM_BIOS(1))
	ROMX_LOAD("m_c7b8.bin", 0x0000, 0x8000, CRC(21282e86) SHA1(18cb3960dc47f2c14beb88f9680c1f66c4652b04), ROM_BIOS(2))
ROM_END

const tiny_rom_entry *aha1740_device::device_rom_region() const
{
	return ROM_NAME(aha1740);
}

const tiny_rom_entry *aha1742a_device::device_rom_region() const
{
	return ROM_NAME(aha1742a);
}
