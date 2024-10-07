// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    RISC Developments IDE Hard Disc System

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/RiscDev_IDE.html

**********************************************************************/

#include "emu.h"
#include "ide_rdev.h"
#include "bus/ata/ataintf.h"


namespace {

class arc_ide_rdev_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_ide_rdev_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;

private:
	required_memory_region m_podule_rom;

	u8 m_rom_page;
};


void arc_ide_rdev_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x7800)]; })).umask32(0x000000ff);
	map(0x2000, 0x2000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
	map(0x2400, 0x2403).nopw(); // TODO: interrupt enable?
	map(0x2800, 0x281f).rw("ata", FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)).umask32(0x0000ffff);
}


//-------------------------------------------------
//  ROM( ide_riscdev )
//-------------------------------------------------

ROM_START( ide_riscdev )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_LOAD("ide_1.24_riscdev.bin", 0x0000, 0x8000, CRC(53bc8e72) SHA1(84e5d31d631b69401c47ef1ce91f11f1bb317597))
ROM_END

const tiny_rom_entry *arc_ide_rdev_device::device_rom_region() const
{
	return ROM_NAME( ide_riscdev );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_ide_rdev_device::device_add_mconfig(machine_config &config)
{
	ata_interface_device &ata(ATA_INTERFACE(config, "ata").options(ata_devices, "hdd", nullptr, false));
	ata.irq_handler().set([this](int state) { set_pirq(state); });
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_ide_rdev_device - constructor
//-------------------------------------------------

arc_ide_rdev_device::arc_ide_rdev_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_IDE_RDEV, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_ide_rdev_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_ide_rdev_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_IDE_RDEV, device_archimedes_podule_interface, arc_ide_rdev_device, "arc_ide_rdev", "RISC Developments IDE Hard Disc System")
