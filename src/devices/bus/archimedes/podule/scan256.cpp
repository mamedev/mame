// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics 256 Grey-Scale Scanner

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/WE_Scanner.html

    TODO:
    - implement scanner.

**********************************************************************/

#include "emu.h"
#include "scan256.h"
#include "imagedev/picture.h"


namespace {

class arc_scan256_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_scan256_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

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
	required_device<picture_image_device> m_picture;

	u8 m_rom_page;
};


void arc_scan256_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0xf800)]; })).umask32(0x000000ff);
	map(0x2000, 0x2000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}


//-------------------------------------------------
//  ROM( scan256 )
//-------------------------------------------------

ROM_START( scan256 )
	ROM_REGION(0x20000, "podule_rom", 0)
	ROM_LOAD("scan256_1.00.bin", 0x0000, 0x20000, CRC(3e5d5877) SHA1(9ea4bba1a752090b19a8ab9bf06414053fb8f7ff))
ROM_END

const tiny_rom_entry *arc_scan256_device::device_rom_region() const
{
	return ROM_NAME( scan256 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_scan256_device::device_add_mconfig(machine_config &config)
{
	IMAGE_PICTURE(config, m_picture);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_scan256_device - constructor
//-------------------------------------------------

arc_scan256_device::arc_scan256_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_SCAN256, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_picture(*this, "srcimg")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_scan256_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_scan256_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCAN256, device_archimedes_podule_interface, arc_scan256_device, "arc_scan256", "Watford Electronics 256 Grey-Scale Scanner")
