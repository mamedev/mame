// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Morley Electronics ST506 Controller Card

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/Morley_ST506Card.html

    TODO:
    - emulate HD63463 device.

**********************************************************************/

#include "emu.h"
#include "hdisc_morley.h"
//#include "machine/hd63463.h"
#include "imagedev/harddriv.h"


namespace {

// ======================> arc_hdisc_morley_device

class arc_hdisc_morley_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::DISK; }

	// construction/destruction
	arc_hdisc_morley_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;

private:
	required_memory_region m_podule_rom;
};


void arc_hdisc_morley_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset]; })).umask32(0x000000ff);
	//map(0x2000, 0x2007).rw("hdc", FUNC(hd63463_device::read), FUNC(hd63463_device::write)).umask32(0x0000ffff);
}


//-------------------------------------------------
//  ROM( hdisc_morley )
//-------------------------------------------------

ROM_START( hdisc_morley )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_LOAD("hdisc_v2.10.rom", 0x0000, 0x4000, CRC(df139312) SHA1(42947aef6b2bafb4b971e5bc61aa31ecb385afe0))
ROM_END

const tiny_rom_entry *arc_hdisc_morley_device::device_rom_region() const
{
	return ROM_NAME( hdisc_morley );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_hdisc_morley_device::device_add_mconfig(machine_config &config)
{
	//hd63463_device &hdc(HD63463(config, "hdc", 10_MHz_XTAL));
	//hdc.irq_callback().set([this](int state) { set_pirq(state); });
	//hdc.dreq_callback().set([this](int state) { set_pirq(state); });

	//HARDDISK(config, "hdc:0", "st506_hdd");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_hdisc_morley_device - constructor
//-------------------------------------------------

arc_hdisc_morley_device::arc_hdisc_morley_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_HDISC_MORLEY, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_hdisc_morley_device::device_start()
{
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_HDISC_MORLEY, device_archimedes_podule_interface, arc_hdisc_morley_device, "arc_hdisc_morley", "Morley Electronics Hard Disc Podule")
