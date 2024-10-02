// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn AKD52 Hard Disc Podule

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AKD52_HDPodule.html

    TODO:
    - emulate HD63463 device.

**********************************************************************/

#include "emu.h"
#include "hdisc.h"
//#include "machine/hd63463.h"
#include "imagedev/harddriv.h"


namespace {

// ======================> arc_hdisc_akd52_device

class arc_hdisc_akd52_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::DISK; }

	// construction/destruction
	arc_hdisc_akd52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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


void arc_hdisc_akd52_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset]; })).umask32(0x000000ff);
	//map(0x2000, 0x2007).rw("hdc", FUNC(hd63463_device::read), FUNC(hd63463_device::write)).umask32(0x0000ffff);
}


//-------------------------------------------------
//  ROM( hdisc_akd52 )
//-------------------------------------------------

ROM_START( hdisc_akd52 )
	ROM_REGION(0x0800, "podule_rom", 0)
	ROM_LOAD("0276,242-01_hdisc_pod.rom", 0x0000, 0x0800, CRC(5e43bf28) SHA1(c8ebec931d6cef53b7b2965f526e8721c6cb3d28))
ROM_END

const tiny_rom_entry *arc_hdisc_akd52_device::device_rom_region() const
{
	return ROM_NAME( hdisc_akd52 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_hdisc_akd52_device::device_add_mconfig(machine_config &config)
{
	//hd63463_device &hdc(HD63463(config, "hdc", 10_MHz_XTAL));
	//hdc.irq_callback().set([this](int state) { set_pirq(state); });
	//hdc.dreq_callback().set([this](int state) { set_pirq(state); });

	//HARDDISK(config, "hdc:0", "st506_hdd");
	//HARDDISK(config, "hdc:1", "st506_hdd");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_hdisc_akd52_device - constructor
//-------------------------------------------------

arc_hdisc_akd52_device::arc_hdisc_akd52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_HDISC_AKD52, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_hdisc_akd52_device::device_start()
{
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_HDISC_AKD52, device_archimedes_podule_interface, arc_hdisc_akd52_device, "arc_hdisc_akd52", "Acorn AKD52 Hard Disc Podule")
