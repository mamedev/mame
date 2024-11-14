// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Computerware Hard Disc Podule

    TODO:
    - emulate HD63463 device.

**********************************************************************/

#include "emu.h"
#include "hdisc_cw.h"
//#include "machine/hd63463.h"
#include "imagedev/harddriv.h"


namespace {

// ======================> arc_hdisc_cw_device

class arc_hdisc_cw_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::DISK; }

	// construction/destruction
	arc_hdisc_cw_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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


void arc_hdisc_cw_device::ioc_map(address_map &map)
{
	map(0x0000, 0x0fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset]; })).umask32(0x000000ff);
	//map(0x1000, 0x1007).rw("hdc", FUNC(hd63463_device::read), FUNC(hd63463_device::write)).umask32(0x0000ffff);
}


//-------------------------------------------------
//  ROM( hdisc_cw )
//-------------------------------------------------

ROM_START( hdisc_cw )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_LOAD("hdisc_computerware.rom", 0x0000, 0x4000, CRC(adcef956) SHA1(da99cd0ee9d1eba237261f24b46e8399a2316086))
ROM_END

const tiny_rom_entry *arc_hdisc_cw_device::device_rom_region() const
{
	return ROM_NAME( hdisc_cw );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_hdisc_cw_device::device_add_mconfig(machine_config &config)
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
//  arc_hdisc_cw_device - constructor
//-------------------------------------------------

arc_hdisc_cw_device::arc_hdisc_cw_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_HDISC_CW, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_hdisc_cw_device::device_start()
{
	u8 *rom = m_podule_rom->base();

	u8 rom_copy[0x1000];
	memcpy(rom_copy, &rom[0], 0x1000);

	for (int i = 0x0000; i < 0x1000; i++)
	{
		rom[i] = rom_copy[bitswap<12>(i, 11, 10, 9, 8, 0, 1, 2, 3, 4, 5, 6, 7)];
	}
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_HDISC_CW, device_archimedes_podule_interface, arc_hdisc_cw_device, "arc_hdisc_cw", "Computerware Hard Disk Podule")
