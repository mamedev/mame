// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Armadillo Systems A448 Sound Sampler

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Armadillo_A448.html

    TODO:
    - everything, requires driver software to determine how hardware is accessed.

**********************************************************************/

#include "emu.h"
#include "a448.h"
#include "bus/midi/midi.h"


namespace {

// ======================> arc_a448_device

class arc_a448_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

	// construction/destruction
	arc_a448_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	arc_a448_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

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


// ======================> arc_a448b_device

class arc_a448b_device : public arc_a448_device
{
public:
	// construction/destruction
	arc_a448b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


void arc_a448_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset]; })).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( a448 )
//-------------------------------------------------

ROM_START( a448 )
	ROM_REGION(0x1000, "podule_rom", 0)
	ROM_LOAD("a448.rom", 0x0000, 0x1000, CRC(e1988e76) SHA1(8140e9cf82dc67324963766d33efd5c8ad6d53f8))
ROM_END

ROM_START( a448b )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_LOAD("a448b_midi.rom", 0x0000, 0x4000, CRC(864f7967) SHA1(125b5a505498d91ca049f95fd05e693b2396bda6))
ROM_END

const tiny_rom_entry *arc_a448_device::device_rom_region() const
{
	return ROM_NAME( a448 );
}

const tiny_rom_entry *arc_a448b_device::device_rom_region() const
{
	return ROM_NAME( a448b );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_a448_device::device_add_mconfig(machine_config &config)
{
	//ZN448(config, "zn448", 0);
}

void arc_a448b_device::device_add_mconfig(machine_config &config)
{
	arc_a448_device::device_add_mconfig(config);

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set("mdthru", FUNC(midi_port_device::write_txd));
	midiout_slot(MIDI_PORT(config, "mdthru"));
	midiout_slot(MIDI_PORT(config, "mdout"));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_a448_device - constructor
//-------------------------------------------------

arc_a448_device::arc_a448_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
{
}

arc_a448_device::arc_a448_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_a448_device(mconfig, ARC_A448, tag, owner, clock)
{
}

arc_a448b_device::arc_a448b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_a448_device(mconfig, ARC_A448B, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_a448_device::device_start()
{
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_A448, device_archimedes_podule_interface, arc_a448_device, "arc_a448", "Armadillo Systems A448 Sound Sampler")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_A448B, device_archimedes_podule_interface, arc_a448b_device, "arc_a448b", "Armadillo Systems A448b Stereo MIDI Sound Sampler")
