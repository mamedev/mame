// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Beebug Spectra Colour Scanner

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Beebug_ColourScanner.html

    TODO:
    - unknown scanner.

**********************************************************************/

#include "emu.h"
#include "spectra.h"
#include "imagedev/picture.h"


namespace {

class arc_spectra_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_spectra_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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


void arc_spectra_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x7800)]; })).umask32(0x000000ff);
	map(0x2000, 0x2000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}


//-------------------------------------------------
//  ROM( spectra )
//-------------------------------------------------

ROM_START( spectra )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_LOAD("spectra_1.05.bin", 0x0000, 0x8000, CRC(51b858ae) SHA1(bac53e4cd1047606e6314992e2581acf09e1b658))
ROM_END

const tiny_rom_entry *arc_spectra_device::device_rom_region() const
{
	return ROM_NAME( spectra );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_spectra_device::device_add_mconfig(machine_config &config)
{
	IMAGE_PICTURE(config, m_picture);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_spectra_device - constructor
//-------------------------------------------------

arc_spectra_device::arc_spectra_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_SPECTRA, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_picture(*this, "srcimg")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_spectra_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_spectra_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_SPECTRA, device_archimedes_podule_interface, arc_spectra_device, "arc_spectra", "Beebug Spectra Colour Scanner")
