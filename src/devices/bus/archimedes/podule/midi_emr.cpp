// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    EMR MIDI Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/EMR_MIDI4.html

**********************************************************************/

#include "emu.h"
#include "midi_emr.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "bus/midi/midi.h"


namespace {

// ======================> arc_midi2_emr_device

class arc_midi2_emr_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_midi2_emr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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

	u8 m_rom_page;
};


// ======================> arc_midi4_emr_device

class arc_midi4_emr_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_midi4_emr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
};


void arc_midi2_emr_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | (((m_rom_page << 11) & 0x7800) ^ 0x4000)]; })).umask32(0x000000ff);
	map(0x1000, 0x1000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
	map(0x2000, 0x203f).rw("uart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
}


void arc_midi4_emr_device::ioc_map(address_map &map)
{
	map(0x2000, 0x203f).rw("uart1", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
	map(0x3000, 0x303f).rw("uart2", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( midi2 )
//-------------------------------------------------

ROM_START( midi2 )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "314", "V3.14/9/8/89")
	ROMX_LOAD("emr_midi2_v3.14.bin", 0x0000, 0x8000, CRC(7de739fe) SHA1(d0440e4ef0fa7bbab2ea3d6f9f3f1d4513aec8c0), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *arc_midi2_emr_device::device_rom_region() const
{
	return ROM_NAME( midi2 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_midi2_emr_device::device_add_mconfig(machine_config &config)
{
	scn2681_device &uart(SCN2681(config, "uart", DERIVED_CLOCK(1, 4))); // TODO: confirm device
	uart.irq_cb().set([this](int state) { set_pirq(state); });
	uart.a_tx_cb().set("mdout_a", FUNC(midi_port_device::write_txd));
	uart.b_tx_cb().set("mdout_b", FUNC(midi_port_device::write_txd));

	midi_port_device &mdin1(MIDI_PORT(config, "mdin_a", midiin_slot, "midiin"));
	mdin1.rxd_handler().set("uart", FUNC(scn2681_device::rx_a_w));
	midi_port_device &mdin2(MIDI_PORT(config, "mdin_b", midiin_slot, "midiin"));
	mdin2.rxd_handler().set("uart", FUNC(scn2681_device::rx_b_w));

	MIDI_PORT(config, "mdout_a", midiout_slot, "midiout");
	MIDI_PORT(config, "mdout_b", midiout_slot, "midiout");
}

void arc_midi4_emr_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set([this](int state) { set_pirq(state); });

	scn2681_device &uart1(SCN2681(config, "uart1", 3.6864_MHz_XTAL));
	uart1.irq_cb().set("irqs", FUNC(input_merger_device::in_w<0>));
	uart1.a_tx_cb().set("mdout_a", FUNC(midi_port_device::write_txd));
	uart1.b_tx_cb().set("mdout_b", FUNC(midi_port_device::write_txd));

	midi_port_device &mdin1(MIDI_PORT(config, "mdin_a", midiin_slot, "midiin"));
	mdin1.rxd_handler().set("uart1", FUNC(scn2681_device::rx_a_w));
	midi_port_device &mdin2(MIDI_PORT(config, "mdin_b", midiin_slot, "midiin"));
	mdin2.rxd_handler().set("uart1", FUNC(scn2681_device::rx_b_w));

	MIDI_PORT(config, "mdout_a", midiout_slot, "midiout");
	MIDI_PORT(config, "mdout_b", midiout_slot, "midiout");

	scn2681_device &uart2(SCN2681(config, "uart2", 3.6864_MHz_XTAL));
	uart2.irq_cb().set("irqs", FUNC(input_merger_device::in_w<1>));
	uart2.a_tx_cb().set("mdout_c", FUNC(midi_port_device::write_txd));
	uart2.b_tx_cb().set("mdout_d", FUNC(midi_port_device::write_txd));

	midi_port_device &mdin3(MIDI_PORT(config, "mdin_c", midiin_slot, "midiin"));
	mdin3.rxd_handler().set("uart2", FUNC(scn2681_device::rx_a_w));
	midi_port_device &mdin4(MIDI_PORT(config, "mdin_d", midiin_slot, "midiin"));
	mdin4.rxd_handler().set("uart2", FUNC(scn2681_device::rx_b_w));

	MIDI_PORT(config, "mdout_c", midiout_slot, "midiout");
	MIDI_PORT(config, "mdout_d", midiout_slot, "midiout");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_midi_emr_device - constructor
//-------------------------------------------------

arc_midi2_emr_device::arc_midi2_emr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_MIDI2_EMR, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}

arc_midi4_emr_device::arc_midi4_emr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_MIDI4_EMR, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_midi2_emr_device::device_start()
{
	save_item(NAME(m_rom_page));
}

void arc_midi4_emr_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_midi2_emr_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_MIDI2_EMR, device_archimedes_podule_interface, arc_midi2_emr_device, "arc_midi2", "EMR MIDI 2 Interface")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_MIDI4_EMR, device_archimedes_podule_interface, arc_midi4_emr_device, "arc_midi4", "EMR MIDI 4 Interface")
