// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn AEH54 10Base2 Ethernet Podule

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AEH54_Ethernet.html

    TODO:
    - everything, this is skeleton with ROM.

**********************************************************************/

#include "emu.h"
#include "ether3.h"


namespace {

class arc_ether3_aeh54_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_ether3_aeh54_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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


void arc_ether3_aeh54_device::ioc_map(address_map &map)
{
	map(0x0000, 0x3fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 12) & 0x1f000)]; })).umask32(0x000000ff);
	map(0x0000, 0x0000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}


//-------------------------------------------------
//  ROM( ether3_aeh54 )
//-------------------------------------------------

ROM_START( ether3_aeh54 )
	ROM_REGION(0x20000, "podule_rom", 0)
	ROM_LOAD("aun_client_rom.rom", 0x0000, 0x20000, CRC(8137b9eb) SHA1(103318a527ea5f9a3fe373bfb8f8d0ad3697617f))
ROM_END

const tiny_rom_entry *arc_ether3_aeh54_device::device_rom_region() const
{
	return ROM_NAME( ether3_aeh54 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_ether3_aeh54_device::device_add_mconfig(machine_config &config)
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_ether3_aeh54_device - constructor
//-------------------------------------------------

arc_ether3_aeh54_device::arc_ether3_aeh54_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_ETHER3_AEH54, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_ether3_aeh54_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_ether3_aeh54_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_ETHER3_AEH54, device_archimedes_podule_interface, arc_ether3_aeh54_device, "arc_ether3_aeh54", "Acorn AEH54 10Base2 Ethernet Podule")
