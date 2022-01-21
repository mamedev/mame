// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Computer Concepts Scanner Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/CC_Scannerif.html

    Computer Concepts ScanLight Video 256

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/CC_ScanlightVideo256.html

    TODO:
    - implement scanner device
    - implement frame grabber from video source.

**********************************************************************/

#include "emu.h"
#include "scanlight.h"
#include "imagedev/avivideo.h"
#include "imagedev/picture.h"


namespace {

// ======================> arc_scanlight_device

class arc_scanlight_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_scanlight_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

protected:
	arc_scanlight_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override;

//private:
	required_memory_region m_podule_rom;
	required_device<picture_image_device> m_picture;

	u8 m_rom_page;
};


// ======================> arc_scanjunior_device

class arc_scanjunior_device : public arc_scanlight_device
{
public:
	// construction/destruction
	arc_scanjunior_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
};


// ======================> arc_scanvideo_device

class arc_scanvideo_device : public arc_scanlight_device
{
public:
	// construction/destruction
	arc_scanvideo_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override;

private:
	required_device<avivideo_image_device> m_avivideo;

	u8 m_rom_bank;
};


void arc_scanlight_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x7800)]; })).umask32(0x000000ff);
	map(0x2000, 0x2000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}

void arc_scanvideo_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0xf800)]; })).umask32(0x000000ff);
	map(0x2000, 0x2000).lw8(NAME([this](u8 data) { m_rom_page = data | (m_rom_bank << 4); }));
}


//-------------------------------------------------
//  ROM( scanlight )
//-------------------------------------------------

ROM_START( scanlight )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_LOAD("scan-light.rom", 0x0000, 0x8000, CRC(6ba1764b) SHA1(fa69fb69e01553b27409b65a37d3d30a376e7380))
ROM_END

ROM_START( scanjunior )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_LOAD("scan-light_jr.rom", 0x0000, 0x8000, CRC(bf1b8268) SHA1(3c63f8379d8fc222fc57838a9384ecec7c5d2484))
ROM_END

ROM_START( scanvideo )
	ROM_REGION(0x10000, "podule_rom", 0)
	ROM_LOAD("video256_1.20.rom", 0x0000, 0x10000, CRC(3e9a44b8) SHA1(9fe739b42c5bf5dab69e3c1ffb1c3c5a5d8d7a50))
ROM_END



const tiny_rom_entry *arc_scanlight_device::device_rom_region() const
{
	return ROM_NAME( scanlight );
}

const tiny_rom_entry *arc_scanjunior_device::device_rom_region() const
{
	return ROM_NAME( scanjunior );
}

const tiny_rom_entry *arc_scanvideo_device::device_rom_region() const
{
	return ROM_NAME( scanvideo );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_scanlight_device::device_add_mconfig(machine_config &config)
{
	IMAGE_PICTURE(config, m_picture);
}

void arc_scanvideo_device::device_add_mconfig(machine_config &config)
{
	arc_scanlight_device::device_add_mconfig(config);

	IMAGE_AVIVIDEO(config, m_avivideo);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_scanlight_device - constructor
//-------------------------------------------------

arc_scanlight_device::arc_scanlight_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_picture(*this, "srcimg")
	, m_rom_page(0)
{
}

arc_scanlight_device::arc_scanlight_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_scanlight_device(mconfig, ARC_SCANLIGHT, tag, owner, clock)
{
}

arc_scanjunior_device::arc_scanjunior_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_scanlight_device(mconfig, ARC_SCANJUNIOR, tag, owner, clock)
{
}

arc_scanvideo_device::arc_scanvideo_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_scanlight_device(mconfig, ARC_SCANVIDEO, tag, owner, clock)
	, m_avivideo(*this, "srcavi")
	, m_rom_bank(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_scanlight_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_scanlight_device::device_reset()
{
	m_rom_page = 0xff;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCANLIGHT, device_archimedes_podule_interface, arc_scanlight_device, "arc_scanlight", "Computer Concepts ScanLight")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCANJUNIOR, device_archimedes_podule_interface, arc_scanjunior_device, "arc_scanjunior", "Computer Concepts ScanLight Junior")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCANVIDEO, device_archimedes_podule_interface, arc_scanvideo_device, "arc_scanvideo", "Computer Concepts ScanLight Video 256")
