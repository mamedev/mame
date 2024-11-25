// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Clares Armadeus Sampler Board

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Clares_SoundSampler.html

**********************************************************************/

#include "emu.h"
#include "armadeus.h"
#include "sound/dac.h"
#include "speaker.h"


namespace {

class arc_armadeus_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

	// construction/destruction
	arc_armadeus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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


void arc_armadeus_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset]; })).umask32(0x000000ff);
	map(0x2000, 0x2000).w("dac", FUNC(zn428e_device::data_w));
}


//-------------------------------------------------
//  ROM( armadeus )
//-------------------------------------------------

ROM_START( armadeus )
	ROM_REGION(0x0800, "podule_rom", 0)
	ROM_LOAD("sound_sampler_issue_0.20.rom", 0x0000, 0x0800, CRC(dc41cfd9) SHA1(6129d91bcd21bdab75f1293f9daa7612feaffc4b))
ROM_END

const tiny_rom_entry *arc_armadeus_device::device_rom_region() const
{
	return ROM_NAME( armadeus );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_armadeus_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	ZN428E(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5);

	//ZN448(config, "zn448", 0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_armadeus_device - constructor
//-------------------------------------------------

arc_armadeus_device::arc_armadeus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_ARMADEUS, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_armadeus_device::device_start()
{
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_ARMADEUS, device_archimedes_podule_interface, arc_armadeus_device, "arc_armadeus", "Clares Armadeus Sampler Board")
