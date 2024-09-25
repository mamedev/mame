// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    RISC Developments Ethernet Card

    TODO:
    - everything, this is skeleton with ROM.

**********************************************************************/

#include "emu.h"
#include "etherr.h"


namespace {

class arc_etherr_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_etherr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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


void arc_etherr_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x1f800)]; })).umask32(0x000000ff);
	map(0x0000, 0x0000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}

void arc_etherr_device::memc_map(address_map &map)
{
}


//-------------------------------------------------
//  ROM( etherr )
//-------------------------------------------------

ROM_START( etherr )
	ROM_REGION(0x20000, "podule_rom", 0)
	ROM_LOAD("etherr_1.06.rom", 0x0000, 0x20000, CRC(7b214fda) SHA1(a8045e3749fbf24e721e84e57acb68753fa46428))
ROM_END

const tiny_rom_entry *arc_etherr_device::device_rom_region() const
{
	return ROM_NAME( etherr );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_etherr_device::device_add_mconfig(machine_config &config)
{
	//NQ8005A(config, "edlc", 20_MHz_XTAL);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_etherr_device - constructor
//-------------------------------------------------

arc_etherr_device::arc_etherr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_ETHERR, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_etherr_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_etherr_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_ETHERR, device_archimedes_podule_interface, arc_etherr_device, "arc_etherr", "RISC Developments Ethernet Card")
