// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ANT Ethernet 10base2 mini-podule

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/ANT_Ethernet10base2Mini.html

    TODO:
    - everything, this is skeleton with ROM.

**********************************************************************/

#include "emu.h"
#include "ethera.h"


namespace {

class arc_ethera_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_ethera_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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

private:
	required_memory_region m_podule_rom;

	u8 m_rom_page;
};


void arc_ethera_device::ioc_map(address_map &map)
{
	map(0x0000, 0x0fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 10) & 0x3f800)]; })).umask32(0x000000ff);
	map(0x0000, 0x0000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}


//-------------------------------------------------
//  ROM( ethera )
//-------------------------------------------------

ROM_START( ethera )
	ROM_REGION(0x40000, "podule_rom", 0)
	ROM_LOAD("ethernet5.rom", 0x0000, 0x40000, CRC(7bb0f699) SHA1(c6c9c9630b6777f4d49824b051fcd16fcb21b8f3))
ROM_END

const tiny_rom_entry *arc_ethera_device::device_rom_region() const
{
	return ROM_NAME( ethera );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_ethera_device::device_add_mconfig(machine_config &config)
{
	//NQ8005(config, "edlc", 20_MHz_XTAL);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_ethera_device - constructor
//-------------------------------------------------

arc_ethera_device::arc_ethera_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_ETHERA, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_ethera_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_ethera_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_ETHERA, device_archimedes_podule_interface, arc_ethera_device, "arc_ethera", "ANT Ethernet 10base2 mini-podule")
