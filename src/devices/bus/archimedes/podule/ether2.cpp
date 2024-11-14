// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn AEH50 Ethernet II

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AEH50_Ethernet.html

    TODO:
    - everything, this is skeleton with ROM.

**********************************************************************/

#include "emu.h"
#include "ether2.h"
#include "machine/dp8390.h"


namespace {

class arc_ether2_aeh50_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_ether2_aeh50_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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
	required_device <dp8390_device> m_dp8390;

	u8 m_rom_page;
};


void arc_ether2_aeh50_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x3800)]; })).umask32(0x000000ff);
	map(0x3000, 0x3000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}

void arc_ether2_aeh50_device::memc_map(address_map &map)
{
}


//-------------------------------------------------
//  ROM( ether2_aeh50 )
//-------------------------------------------------

ROM_START( ether2_aeh50 )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_LOAD("0273,270.01_enet_id.rom", 0x0000, 0x4000, CRC(87d4fbc6) SHA1(417000f1fa8f61f8092f8f74a0359ffb3ce72768))
ROM_END

const tiny_rom_entry *arc_ether2_aeh50_device::device_rom_region() const
{
	return ROM_NAME( ether2_aeh50 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_ether2_aeh50_device::device_add_mconfig(machine_config &config)
{
	DP8390D(config, m_dp8390, 20_MHz_XTAL);
	m_dp8390->irq_callback().set([this](int state) { set_pirq(state); });
	//m_dp8390->mem_read_callback().set(FUNC(arc_ether2_aeh50_device::mem_read));
	//m_dp8390->mem_write_callback().set(FUNC(arc_ether2_aeh50_device::mem_write));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_ether2_aeh50_device - constructor
//-------------------------------------------------

arc_ether2_aeh50_device::arc_ether2_aeh50_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_ETHER2_AEH50, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_dp8390(*this, "dp8390")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_ether2_aeh50_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_ether2_aeh50_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_ETHER2_AEH50, device_archimedes_podule_interface, arc_ether2_aeh50_device, "arc_ether2_aeh50", "Acorn AEH50 Ethernet II")
