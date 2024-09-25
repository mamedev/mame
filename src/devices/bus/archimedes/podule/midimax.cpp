// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Wild Vision MidiMax

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/WildVision_MidiMax.html

**********************************************************************/

#include "emu.h"
#include "midimax.h"
#include "machine/ins8250.h"
#include "bus/midi/midi.h"


namespace {

// ======================> arc_midimax_device

class arc_midimax_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_midimax_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	arc_midimax_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

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


// ======================> arc_midimax2_device

class arc_midimax2_device : public arc_midimax_device
{
public:
	// construction/destruction
	arc_midimax2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


void arc_midimax_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0xf800)]; })).umask32(0x000000ff);
	map(0x2000, 0x2000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
	map(0x3000, 0x301f).rw("uart", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w)).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( midimax )
//-------------------------------------------------

ROM_START( midimax )
	ROM_REGION(0x10000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "106", "1.06 (26 Mar 1995)")
	ROMX_LOAD("midimax-1.06.rom", 0x0000, 0x10000, CRC(62d8a431) SHA1(8c291082d9e8a8533a4f3b59da857813aa357ecb), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "105", "1.05 (26 Apr 1994)")
	ROMX_LOAD("midimax-1.05.rom", 0x0000, 0x10000, CRC(2340a077) SHA1(8cba82ed214246b9fb55e24e87e4629231490465), ROM_BIOS(1))
ROM_END

ROM_START( midimax2 )
	ROM_REGION(0x10000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "201", "2.01 (10-Dec-97)")
	ROMX_LOAD("midimax-2.01.rom", 0x0000, 0x10000, CRC(b112bdb2) SHA1(d713bf8661542009defadb08cc6096af94efce3e), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *arc_midimax_device::device_rom_region() const
{
	return ROM_NAME( midimax );
}

const tiny_rom_entry *arc_midimax2_device::device_rom_region() const
{
	return ROM_NAME( midimax2 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_midimax_device::device_add_mconfig(machine_config &config)
{
	ns16550_device &uart(NS16550(config, "uart", DERIVED_CLOCK(1, 4))); // TODO: verify clock
	uart.out_tx_callback().set("mdout1", FUNC(midi_port_device::write_txd));
	uart.out_tx_callback().append("mdout2", FUNC(midi_port_device::write_txd));
	uart.out_tx_callback().append("mdthru", FUNC(midi_port_device::write_txd));
	uart.out_int_callback().set([this](int state) { set_pirq(state); });

	midiout_slot(MIDI_PORT(config, "mdthru"));
	midiout_slot(MIDI_PORT(config, "mdout1"));
	midiout_slot(MIDI_PORT(config, "mdout2"));
	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set("uart", FUNC(ns16550_device::rx_w));
	mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_midimax_device - constructor
//-------------------------------------------------

arc_midimax_device::arc_midimax_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}

arc_midimax_device::arc_midimax_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_midimax_device(mconfig, ARC_MIDIMAX, tag, owner, clock)
{
}

arc_midimax2_device::arc_midimax2_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_midimax_device(mconfig, ARC_MIDIMAX2, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_midimax_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_midimax_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_MIDIMAX, device_archimedes_podule_interface, arc_midimax_device, "arc_midimax", "Wild Vision MidiMax")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_MIDIMAX2, device_archimedes_podule_interface, arc_midimax2_device, "arc_midimax2", "Wild Vision MidiMax II")
