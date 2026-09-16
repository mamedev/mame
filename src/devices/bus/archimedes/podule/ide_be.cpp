// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Baildon Electronics IDE HD Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/BE_IDEHD.html

**********************************************************************/

#include "emu.h"
#include "ide_be.h"
#include "bus/ata/ataintf.h"


namespace {

// ======================> arc_ide_be_device

class arc_ide_be_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_ide_be_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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


void arc_ide_be_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x1800)]; })).umask32(0x000000ff);
	map(0x2000, 0x2000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
	map(0x3000, 0x301f).rw("ata", FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w)).umask32(0x0000ffff);
}


//-------------------------------------------------
//  ROM( ide_be )
//-------------------------------------------------

ROM_START( ide_be ) // if0
	ROM_REGION(0x2000, "podule_rom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "be202", "BE IDEFS 2.02 (14 Jan 1991)")
	ROMX_LOAD("ide1_be_2.02.rom", 0x0000, 0x2000, CRC(36d76e00) SHA1(25547b793d935ee44109ff9ac6ff4be010cdc33d), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "sfps114", "SF & PS IDEFS 1.14 (23 Apr 1991)")
	ROMX_LOAD("ide1_sfps_1.14.rom", 0x0000, 0x2000, CRC(26761c5d) SHA1(eb42f98ac7708b08faf1bba2840bef0ccdffbc20), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "sfps113", "SF & PS IDEFS 1.13 (22 Oct 1990)")
	ROMX_LOAD("ide1_sfps_1.13.rom", 0x0000, 0x2000, CRC(ef1dddff) SHA1(430432e6611b22da9c4fc1b03a986b08a120808b), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "zidefs", "ZIDEFS 1.10 (13 May 2018)") // FIXME: not working ??
	ROMX_LOAD("ide1_zidefs_1.10.rom", 0x0000, 0x1fe8, CRC(581fa41c) SHA1(f7d13fce76dbe7ac9a27fc7e4a859161bb98a9b5), ROM_BIOS(3))
ROM_END

const tiny_rom_entry *arc_ide_be_device::device_rom_region() const
{
	return ROM_NAME( ide_be );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_ide_be_device::device_add_mconfig(machine_config &config)
{
	ata_interface_device &ata(ATA_INTERFACE(config, "ata").options(ata_devices, "hdd", nullptr, false));
	ata.irq_handler().set([this](int state) { set_pirq(state); });
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_ide_be_device - constructor
//-------------------------------------------------

arc_ide_be_device::arc_ide_be_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_IDE_BE, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_ide_be_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_ide_be_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_IDE_BE, device_archimedes_podule_interface, arc_ide_be_device, "arc_ide_be", "Baildon Electronics IDE HD Interface")
