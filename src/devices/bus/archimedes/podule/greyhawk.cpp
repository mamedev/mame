// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Computer Concepts GreyHawk Video Digitiser

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/CC_GreyHawk.html

    TODO:
    - implement frame grabber from video source.

**********************************************************************/

#include "emu.h"
#include "greyhawk.h"
#include "imagedev/avivideo.h"


namespace {

class arc_greyhawk_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_greyhawk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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
	virtual void memc_map(address_map &map) override ATTR_COLD;

private:
	required_memory_region m_podule_rom;
	required_device<avivideo_image_device> m_avivideo;

	u8 m_rom_page;
};


void arc_greyhawk_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0xf800)]; })).umask32(0x000000ff);
	map(0x2000, 0x2000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}

void arc_greyhawk_device::memc_map(address_map &map)
{
}


//-------------------------------------------------
//  ROM( greyhawk )
//-------------------------------------------------

ROM_START( greyhawk )
	ROM_REGION(0x10000, "podule_rom", 0)
	ROM_LOAD("greyhawk.bin", 0x0000, 0x10000, CRC(1ecead90) SHA1(e8d5d271e2c10430ae858d2f4337fc73a0c63d82))
ROM_END

const tiny_rom_entry *arc_greyhawk_device::device_rom_region() const
{
	return ROM_NAME( greyhawk );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_greyhawk_device::device_add_mconfig(machine_config &config)
{
	IMAGE_AVIVIDEO(config, m_avivideo);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_greyhawk_device - constructor
//-------------------------------------------------

arc_greyhawk_device::arc_greyhawk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_GREYHAWK, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_avivideo(*this, "srcavi")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_greyhawk_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_greyhawk_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_GREYHAWK, device_archimedes_podule_interface, arc_greyhawk_device, "arc_greyhawk", "Computer Concepts GreyHawk Video Digitiser")
