// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn AKA10 BBC I/O Podule
    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AKA10_IO.html

    Acorn AKA12 User Port/Midi Upgrade
    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AKA12_MIDI.html

    Acorn AKA15 Midi and BBC I/O Podule
    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AKA15_MIDI.html

    Acorn AKA16 Midi Podule
    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AKA16_MIDI.html

    TODO:
    - interrupts are optional, and are enabled with links.

**********************************************************************/

#include "emu.h"
#include "io.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "machine/upd7002.h"
#include "bus/bbc/1mhzbus/1mhzbus.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/bbc/userport/userport.h"
#include "bus/midi/midi.h"


namespace {

// ======================> arc_io_aka_device

class arc_io_aka_device :
	public device_t,
	public device_archimedes_podule_interface
{
protected:
	arc_io_aka_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void add_1mhzbus(machine_config &config);
	void add_analogue(machine_config &config);
	void add_midi_6850(machine_config &config);
	void add_midi_2691(machine_config &config);
	void add_userport(machine_config &config);

	required_memory_region m_podule_rom;
	required_device<input_merger_device> m_irqs;
	optional_device<bbc_analogue_slot_device> m_analog;

	u8 m_rom_page;

	u8 pa_r();
	int get_analogue_input(int channel_number);
	void upd7002_eoc(int state);

	int m_irq_en;
};


// ======================> arc_bbcio_aka10_device

class arc_bbcio_aka10_device : public arc_io_aka_device
{
public:
	// construction/destruction
	arc_bbcio_aka10_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
	virtual void memc_map(address_map &map) override ATTR_COLD;
};


// ======================> arc_upmidi_aka12_device

class arc_upmidi_aka12_device : public arc_io_aka_device
{
public:
	// construction/destruction
	arc_upmidi_aka12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
};


// ======================> arc_iomidi_aka15_device

class arc_iomidi_aka15_device : public arc_io_aka_device
{
public:
	// construction/destruction
	arc_iomidi_aka15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
	virtual void memc_map(address_map &map) override ATTR_COLD;
};


// ======================> arc_midi_aka16_device

class arc_midi_aka16_device : public arc_io_aka_device
{
public:
	// construction/destruction
	arc_midi_aka16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
};


void arc_bbcio_aka10_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x3800)]; })).umask32(0x000000ff);
	map(0x2000, 0x203f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask32(0x000000ff);
}

void arc_bbcio_aka10_device::memc_map(address_map &map)
{
	map(0x0000, 0x03ff).rw("1mhzbus", FUNC(bbc_1mhzbus_slot_device::fred_r), FUNC(bbc_1mhzbus_slot_device::fred_w)).umask32(0x000000ff);
	map(0x0800, 0x0bff).rw("1mhzbus", FUNC(bbc_1mhzbus_slot_device::jim_r), FUNC(bbc_1mhzbus_slot_device::jim_w)).umask32(0x000000ff);
	map(0x1000, 0x100f).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write)).umask32(0x000000ff);
}


void arc_upmidi_aka12_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x3800)]; })).umask32(0x000000ff);
	map(0x2000, 0x203f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask32(0x000000ff);
	map(0x3000, 0x303f).rw("uart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
}


void arc_iomidi_aka15_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x3800)]; })).umask32(0x000000ff);
	map(0x2000, 0x203f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask32(0x000000ff);
}

void arc_iomidi_aka15_device::memc_map(address_map &map)
{
	map(0x0000, 0x03ff).rw("1mhzbus", FUNC(bbc_1mhzbus_slot_device::fred_r), FUNC(bbc_1mhzbus_slot_device::fred_w)).umask32(0x000000ff);
	map(0x0800, 0x0bff).rw("1mhzbus", FUNC(bbc_1mhzbus_slot_device::jim_r), FUNC(bbc_1mhzbus_slot_device::jim_w)).umask32(0x000000ff);
	map(0x1000, 0x100f).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write)).umask32(0x000000ff);
	map(0x1800, 0x1807).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask32(0x000000ff);
}


void arc_midi_aka16_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x3800)]; })).umask32(0x000000ff);
	map(0x2000, 0x203f).rw("uart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
	map(0x3000, 0x3000).lw8(NAME([this](u8 data) { m_rom_page = data; }));
}


//-------------------------------------------------
//  ROM( aka10 )
//-------------------------------------------------

ROM_START( aka10 )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "3", "Third release 22-Sep-89")
	ROMX_LOAD("0276,202-03_io.bin", 0x0000, 0x4000, CRC(f4df6587) SHA1(2c0402cd754ebeb1e1fe750dc8a88a2acc1ea06d), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "2", "Second release 18-Dec-87")
	ROMX_LOAD("0276,202-02_io.bin", 0x0000, 0x4000, CRC(d548b4dc) SHA1(22e1f1a0adf896f08b1e1d487aa24141296aa127), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "1", "Release 1.04 07-Aug-87")
	ROMX_LOAD("0276,202-01_io.bin", 0x0000, 0x4000, CRC(fc1d61d0) SHA1(4c3de606de594c8f6ed236ffa19374c76e307226), ROM_BIOS(2))
ROM_END

ROM_START( aka12 )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "2", "Second release 31-Aug-89")
	ROMX_LOAD("0280,320-02_a3000_upm.bin", 0x0000, 0x4000, CRC(3f0f5794) SHA1(4e0e2cf2586c8f2fc1431458f677a45164749ab2), ROM_BIOS(0))
ROM_END

ROM_START( aka15 )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "1", "Initial release 18-Dec-87")
	ROMX_LOAD("0276,211-01_iomidi.bin", 0x0000, 0x4000, CRC(e85a77c5) SHA1(32e990c435b439d4a169e85a009fc6a864121c3f), ROM_BIOS(0))
ROM_END

ROM_START( aka16 )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "2", "Second release 31-Aug-89")
	ROMX_LOAD("0276,281-02_midi-pod_v3.03.rom", 0x0000, 0x4000, CRC(1833afdd) SHA1(beb86b74bf6a9c9caf10f57e326cc539ffe9624b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "1", "Test version 11-Jan-88")
	ROMX_LOAD("0276,281-01_midi-pod.rom", 0x0000, 0x4000, CRC(c09ae053) SHA1(e0e63e0b44d3eba805aea9d2103ab35ea93e8c1c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "m", "Millipede Mod 8/1/98") // modified by Millipede to fix clash with their cards
	ROMX_LOAD("millipede_mod_8-1-98.rom", 0x0000, 0x4000, CRC(729b9d61) SHA1(f6ad28d3cf9a14af3c103f9fae74795f539f24e8), ROM_BIOS(2))
ROM_END

const tiny_rom_entry *arc_bbcio_aka10_device::device_rom_region() const
{
	return ROM_NAME( aka10 );
}

const tiny_rom_entry *arc_upmidi_aka12_device::device_rom_region() const
{
	return ROM_NAME( aka12 );
}

const tiny_rom_entry *arc_iomidi_aka15_device::device_rom_region() const
{
	return ROM_NAME( aka15 );
}

const tiny_rom_entry *arc_midi_aka16_device::device_rom_region() const
{
	return ROM_NAME( aka16 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_io_aka_device::add_userport(machine_config &config)
{
	via6522_device &via(MOS6522(config, "via", DERIVED_CLOCK(1, 4)));
	via.irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	via.readpa_handler().set(FUNC(arc_io_aka_device::pa_r));
	via.writepa_handler().set([this](u8 data) { m_rom_page = data; });
	via.readpb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_r));
	via.writepb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_w));
	via.ca2_handler().set([this](int state) { m_irq_en = !state; }); // AKA10/15 only
	via.cb1_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb1));
	via.cb2_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb2));

	bbc_userport_slot_device &userport(BBC_USERPORT_SLOT(config, "userport", bbc_userport_devices, nullptr));
	userport.cb1_handler().set("via", FUNC(via6522_device::write_cb1));
	userport.cb2_handler().set("via", FUNC(via6522_device::write_cb2));
}

void arc_io_aka_device::add_analogue(machine_config &config)
{
	upd7002_device &upd7002(UPD7002(config, "upd7002", DERIVED_CLOCK(1, 4)));
	upd7002.set_get_analogue_callback(FUNC(arc_io_aka_device::get_analogue_input));
	upd7002.set_eoc_callback(FUNC(arc_io_aka_device::upd7002_eoc));

	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);
	m_analog->lpstb_handler().set("via", FUNC(via6522_device::write_ca1));
}

void arc_io_aka_device::add_1mhzbus(machine_config &config)
{
	bbc_1mhzbus_slot_device &bus(BBC_1MHZBUS_SLOT(config, "1mhzbus", DERIVED_CLOCK(1, 8), bbcm_1mhzbus_devices, nullptr));
	bus.irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	bus.nmi_handler().set([this](int state) { set_pfiq(state); });
}

void arc_io_aka_device::add_midi_6850(machine_config &config)
{
	acia6850_device &acia(ACIA6850(config, "acia", 0));
	acia.irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	acia.txd_handler().set("mdout", FUNC(midi_port_device::write_txd));

	clock_device &acia_clock(CLOCK(config, "acia_clock", DERIVED_CLOCK(1, 4)));
	acia_clock.signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append("acia", FUNC(acia6850_device::write_rxc));

	midiout_slot(MIDI_PORT(config, "mdout"));

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
}

void arc_io_aka_device::add_midi_2691(machine_config &config)
{
	scn2681_device &uart(SCN2681(config, "uart", DERIVED_CLOCK(1, 4))); // SCC2691
	uart.irq_cb().set(m_irqs, FUNC(input_merger_device::in_w<3>));
	uart.a_tx_cb().set("mdout", FUNC(midi_port_device::write_txd));
	uart.a_tx_cb().append("mdthru", FUNC(midi_port_device::write_txd));

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set("uart", FUNC(scn2681_device::rx_a_w));
	mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	midiout_slot(MIDI_PORT(config, "mdthru"));
	midiout_slot(MIDI_PORT(config, "mdout"));
}


void arc_bbcio_aka10_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set([this](int state) { set_pirq(m_irq_en && state); });
	add_userport(config);
	add_1mhzbus(config);
	add_analogue(config);
}

void arc_upmidi_aka12_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set([this](int state) { set_pirq(state); });
	add_userport(config);
	add_midi_2691(config);
}

void arc_iomidi_aka15_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set([this](int state) { set_pirq(m_irq_en && state); });
	add_userport(config);
	add_1mhzbus(config);
	add_analogue(config);
	add_midi_6850(config);
}

void arc_midi_aka16_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set([this](int state) { set_pirq(state); });
	add_midi_2691(config);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_io_aka_device - constructor
//-------------------------------------------------

arc_io_aka_device::arc_io_aka_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_irqs(*this, "irqs")
	, m_analog(*this, "analogue")
	, m_rom_page(0)
	, m_irq_en(0)
{
}

arc_bbcio_aka10_device::arc_bbcio_aka10_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_io_aka_device(mconfig, ARC_BBCIO_AKA10, tag, owner, clock)
{
}

arc_upmidi_aka12_device::arc_upmidi_aka12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_io_aka_device(mconfig, ARC_UPMIDI_AKA12, tag, owner, clock)
{
}

arc_iomidi_aka15_device::arc_iomidi_aka15_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_io_aka_device(mconfig, ARC_IOMIDI_AKA15, tag, owner, clock)
{
}

arc_midi_aka16_device::arc_midi_aka16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_io_aka_device(mconfig, ARC_MIDI_AKA16, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_io_aka_device::device_start()
{
	save_item(NAME(m_rom_page));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_io_aka_device::device_reset()
{
	m_rom_page = 0x00;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

u8 arc_io_aka_device::pa_r()
{
	u8 data = 0xff;

	if (m_analog)
	{
		data = (m_analog->pb_r() << 2) | 0x3f;
	}

	return data;
}

int arc_io_aka_device::get_analogue_input(int channel_number)
{
	return m_analog->ch_r(channel_number) << 8;
}

void arc_io_aka_device::upd7002_eoc(int state)
{
	m_irqs->in_w<1>(!state);
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_BBCIO_AKA10, device_archimedes_podule_interface, arc_bbcio_aka10_device, "arc_bbcio_aka10", "Acorn AKA10 BBC I/O Podule")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_UPMIDI_AKA12, device_archimedes_podule_interface, arc_upmidi_aka12_device, "arc_upmidi_aka12", "Acorn AKA12 User Port/MIDI Podule")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_IOMIDI_AKA15, device_archimedes_podule_interface, arc_iomidi_aka15_device, "arc_iomidi_aka15", "Acorn AKA15 MIDI and BBC I/O Podule")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_MIDI_AKA16, device_archimedes_podule_interface, arc_midi_aka16_device, "arc_midi_aka16", "Acorn AKA16 MIDI Podule")
