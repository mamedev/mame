// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    SJ Research Nexus Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/SJR_A300NexusI4.html

**********************************************************************/

#include "emu.h"
#include "nexus.h"
//#include "machine/imsc012.h"


namespace {

class arc_nexus_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_nexus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::COMMS; }

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
	required_memory_region m_podule_rom;

	u8 m_rom_page;
};


void arc_nexus_device::ioc_map(address_map &map)
{
	map(0x0000, 0x03fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 12) & 0x7000)]; })).umask32(0x000000ff);
}

void arc_nexus_device::memc_map(address_map &map)
{
	map(0x0040, 0x0040).lrw8(NAME([this]() { return m_rom_page; }), NAME([this](u8 data) { m_rom_page = data; }));
	//map(0x0060, 0x006f).rw("c012", FUNC(imsc012_device::read), FUNC(imsc012_device::write)).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( nexus )
//-------------------------------------------------

ROM_START( nexus )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_LOAD("a500_2.24.bin", 0x0000, 0x8000, CRC(5d857330) SHA1(4d1c8cf82fddde1e0772c475da1b3283986aa7d3))
ROM_END

const tiny_rom_entry *arc_nexus_device::device_rom_region() const
{
	return ROM_NAME( nexus );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_nexus_device::device_add_mconfig(machine_config &config)
{
	//IMSC012(config, "c012", 5_MHz_XTAL); // INMOS IMSC012-P20S link adaptor
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_nexus_device - constructor
//-------------------------------------------------

arc_nexus_device::arc_nexus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_NEXUS_A500, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_nexus_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_nexus_device::device_reset()
{
	m_rom_page = 0xff;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_NEXUS_A500, device_archimedes_podule_interface, arc_nexus_device, "arc_nexus_a500", "SJ Research Nexus Interface (A500)")
