// license:GPL-2.0+
// copyright-holders:Felipe Sanches

/***************************************************************************

    HD-SX3 hard disk unit for the SX-KN6000 and SX-KN6500, by Key Soft
    Service

    The HD-SX6 is the same board with a parallel port, a USB controller and
    four DACs for separate outputs fitted as well. The keyboard's CPU runs
    the unit's operating system from the flash.

    TODO:
    - find out whether the HD-SX3 shipped with an operating system other
      than the HD-SX6's

***************************************************************************/

#include "emu.h"
#include "hdsx3.h"

#include "bus/ata/ataintf.h"
#include "machine/intelfsh.h"

namespace {

class hdsx3_device : public device_t, public device_kn6000_expansion_interface
{
public:
	hdsx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device_kn6000_expansion_interface implementation
	virtual void isorom_map(address_map &map) override ATTR_COLD;
	virtual void hddcs_map(address_map &map) override ATTR_COLD;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<fujitsu_29lv160b_device> m_flash;
	required_device<ata_interface_device> m_ata;
	memory_share_creator<u32> m_ram;
};

hdsx3_device::hdsx3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HDSX3, tag, owner, clock)
	, device_kn6000_expansion_interface(mconfig, *this)
	, m_flash(*this, "flash")
	, m_ata(*this, "ata")
	, m_ram(*this, "ram", 0x100000, ENDIANNESS_LITTLE)
{
}

void hdsx3_device::device_start()
{
}

void hdsx3_device::isorom_map(address_map &map)
{
	// A20 selects the SRAM, so only the lower half of the flash is reachable
	map(0x000000, 0x0fffff).rw(m_flash, FUNC(fujitsu_29lv160b_device::read), FUNC(fujitsu_29lv160b_device::write));
	map(0x100000, 0x1fffff).ram().share(m_ram);
}

void hdsx3_device::hddcs_map(address_map &map)
{
	// A15-A14 select the ATA interface, a parallel port and a USB controller;
	// the HD-SX3 fits only the first
	map(0x0010, 0x001f).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
	map(0x0020, 0x002f).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w));
}

void hdsx3_device::device_add_mconfig(machine_config &config)
{
	// the keyboard's flash test accepts an MBM29LV160B or an AT49BV16X4
	FUJITSU_29LV160B(config, m_flash);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
}

ROM_START(hdsx3)
	ROM_REGION16_LE(0x200000, "flash", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("v11")

	// payload of Key Soft Service's HD-SX6 update disk, decompressed
	ROM_SYSTEM_BIOS(0, "v11", "HD-SX6 Version 1.1 (REV3) - July 21st, 2001")
	ROMX_LOAD("hd-sx6_v1_1_rev3.bin", 0x000000, 0x0c0000, CRC(83b8a6f1) SHA1(88699a7e9584e0c30c175babd1482e5aa586ad3d), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *hdsx3_device::device_rom_region() const
{
	return ROM_NAME(hdsx3);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HDSX3, device_kn6000_expansion_interface, hdsx3_device, "hdsx3", "HD-SX3 hard disk unit")
