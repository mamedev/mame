// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Lingenuity SCSI Podule

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/Lingenuity_SCSI.html

    TODO:
    - everything, this is skeleton with ROM.

**********************************************************************/

#include "emu.h"
#include "scsi_ling.h"
#include "machine/ncr5380.h"
#include "bus/nscsi/devices.h"


namespace {

class arc_scsi_ling_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_scsi_ling_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
	virtual void memc_map(address_map &map) override ATTR_COLD;

private:
	required_device<ncr5380_device> m_ncr5380;
	required_memory_region m_podule_rom;

	u8 m_rom_page;
};


void arc_scsi_ling_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0xf800)]; })).umask32(0x000000ff);
	map(0x3800, 0x3800).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}

void arc_scsi_ling_device::memc_map(address_map &map)
{
	//map(0x2000, 0x20ff).m(m_ncr5380, FUNC(ncr5380_device::map)).umask32(0x000000ff);
	map(0x2000, 0x20ff).lr8(NAME([this](offs_t offset) { logerror("r: offset %02x\n", offset >> 3); return m_ncr5380->read(offset >> 3); })).umask32(0x000000ff);
	map(0x2000, 0x20ff).lw8(NAME([this](offs_t offset, u8 data) { logerror("w: offset %02x data %02x\n", offset >> 3, data); m_ncr5380->write(offset >> 3, data); })).umask32(0x000000ff);
	map(0x2100, 0x2100).r(m_ncr5380, FUNC(ncr5380_device::dma_r));
	map(0x2100, 0x2100).w(m_ncr5380, FUNC(ncr5380_device::dma_w));
}


//-------------------------------------------------
//  ROM( scsi_ling )
//-------------------------------------------------

ROM_START( scsi_ling )
	ROM_REGION(0x10000, "podule_rom", 0)
	ROM_LOAD("scsi_2.54.rom", 0x0000, 0x10000, CRC(32926220) SHA1(c05ea28f33afa98bd9d18bfe7dafee0a5fe7f2f4))
ROM_END

const tiny_rom_entry *arc_scsi_ling_device::device_rom_region() const
{
	return ROM_NAME( scsi_ling );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_scsi_ling_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR5380)
		.machine_config([this](device_t *device)
		{
			downcast<ncr5380_device &>(*device).irq_handler().set([this](int state) { set_pirq(state); });
		});
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_scsi_ling_device - constructor
//-------------------------------------------------

arc_scsi_ling_device::arc_scsi_ling_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_SCSI_LING, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_ncr5380(*this, "scsi:7:ncr5380")
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_scsi_ling_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_scsi_ling_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCSI_LING, device_archimedes_podule_interface, arc_scsi_ling_device, "arc_scsi_ling", "Lingenuity SCSI Podule")
