// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    UltraStor Ultra 14F AT Bus Fast SCSI-2 Bus Master Controller

***************************************************************************/

#include "emu.h"
#include "ultra14f.h"

#include "cpu/m68000/m68000.h"
#include "machine/ncr5390.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_hd.h"

DEFINE_DEVICE_TYPE(ULTRA14F, ultra14f_device, "ultra14f", "Ultra-14F SCSI Host Adapter")

ultra14f_device::ultra14f_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ULTRA14F, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_uscpu(*this, "uscpu")
	, m_fdc(*this, "fdc")
	, m_bios(*this, "bios")
{
}

void ultra14f_device::device_start()
{
}

void ultra14f_device::uscpu_map(address_map &map)
{
	map(0x000000, 0x007fff).rom().region("firmware", 0);
	map(0x00e000, 0x00e00f).m("scsi:7:scsic", FUNC(ncr53cf94_device::map));
	map(0x3f8000, 0x3fffff).ram();
}

static void u14f_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add_internal("scsic", NCR53CF94); // Emulex FAS216 or similar custom-marked as USC060-6-40B
}

void ultra14f_device::scsic_config(device_t *device)
{
	device->set_clock(40_MHz_XTAL);
	downcast<ncr53cf94_device &>(*device).irq_handler_cb().set_inputline(m_uscpu, M68K_IRQ_1);
}

void ultra14f_device::device_add_mconfig(machine_config &config)
{
	M68008FN(config, m_uscpu, 40_MHz_XTAL / 4); // custom-marked as USC080-5-10A
	m_uscpu->set_addrmap(AS_PROGRAM, &ultra14f_device::uscpu_map);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", u14f_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", u14f_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", u14f_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", u14f_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", u14f_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", u14f_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", u14f_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7", u14f_scsi_devices, "scsic", true)
		.set_option_machine_config("scsic", [this] (device_t *device) { scsic_config(device); });

	DP8473(config, m_fdc, 24_MHz_XTAL); // custom-marked as USC020-1-24
}

ROM_START(ultra14f)
	ROM_REGION(0x8000, "firmware", 0)
	ROM_LOAD("28004.006", 0x0000, 0x8000, CRC(489a872e) SHA1(692faaf945856b3888be5d8778f5643e51711ee2))

	ROM_REGION(0x4000, "bios", 0) // "Date : 01/07/93 Version 2.01"
	ROM_LOAD("38004.005", 0x0000, 0x4000, CRC(0d9d831d) SHA1(9c1539f8473f8330be323e728e7aabccc77542d0))
ROM_END

const tiny_rom_entry *ultra14f_device::device_rom_region() const
{
	return ROM_NAME(ultra14f);
}
