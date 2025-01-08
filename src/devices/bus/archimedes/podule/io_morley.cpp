// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn AGA30 BBC I/O Podule

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AGA30_AnalogueUserP.html

    Morley Electronics Analogue and User Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/Morley_AnalogUIF.html

    Morley Electronics User/MIDI/Analogue Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/Morley_UserMIDIAnalogue.html

**********************************************************************/

#include "emu.h"
#include "io_morley.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "machine/upd7002.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/bbc/userport/userport.h"
#include "bus/midi/midi.h"


namespace {

// ======================> arc_io_morley_device

class arc_io_morley_device :
	public device_t,
	public device_archimedes_podule_interface
{
protected:
	arc_io_morley_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void add_userport(machine_config &config);
	void add_analogue(machine_config &config);
	void add_midi(machine_config &config);

	required_memory_region m_podule_rom;
	required_device<input_merger_device> m_irqs;
	optional_device<bbc_analogue_slot_device> m_analog;

	u8 m_rom_page;

	int get_analogue_input(int channel_number);
	void upd7002_eoc(int state);
};


// ======================> arc_bbcio_aga30_device

class arc_bbcio_aga30_device : public arc_io_morley_device
{
public:
	// construction/destruction
	arc_bbcio_aga30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
	virtual void memc_map(address_map &map) override ATTR_COLD;
};


// ======================> arc_ua_morley_device

class arc_ua_morley_device : public arc_io_morley_device
{
public:
	// construction/destruction
	arc_ua_morley_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
	virtual void memc_map(address_map &map) override ATTR_COLD;
};


// ======================> arc_uma_morley_device

class arc_uma_morley_device : public arc_io_morley_device
{
public:
	// construction/destruction
	arc_uma_morley_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
	virtual void memc_map(address_map &map) override ATTR_COLD;
};


void arc_bbcio_aga30_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x3800)]; })).umask32(0x000000ff);
	map(0x2000, 0x203f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask32(0x000000ff);
}

void arc_bbcio_aga30_device::memc_map(address_map &map)
{
	map(0x1000, 0x100f).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write)).umask32(0x000000ff);
}


void arc_ua_morley_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x7800)]; })).umask32(0x000000ff);
	map(0x2000, 0x203f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask32(0x000000ff);
}

void arc_ua_morley_device::memc_map(address_map &map)
{
	map(0x1000, 0x100f).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write)).umask32(0x000000ff);
}


void arc_uma_morley_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x7800)]; })).umask32(0x000000ff);
	map(0x2000, 0x203f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask32(0x000000ff);
	map(0x3000, 0x303f).rw("uart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
}

void arc_uma_morley_device::memc_map(address_map &map)
{
	map(0x1000, 0x100f).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write)).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( aga30 )
//-------------------------------------------------

ROM_START( aga30 )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "1", "First release 08-Mar-90")
	ROMX_LOAD("morleyio_v2.01.bin", 0x0000, 0x4000, CRC(b149f631) SHA1(ed8d0caa478f030e11348b23e915897f27dcf153), ROM_BIOS(0))
ROM_END

ROM_START( ua )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "1", "First release 08-Mar-90")
	ROMX_LOAD("morley_ua_16bit_v2.01.bin", 0x0000, 0x8000, CRC(736f8a07) SHA1(423208fb3132400d3582b29481b8f7b7e6cd0c70), ROM_BIOS(0))
ROM_END

ROM_START( uma )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "2", "Second release 03-Jun-93")
	ROMX_LOAD("morley_uma_eprom_v1.00.bin", 0x0000, 0x8000, CRC(7474ec1a) SHA1(0db73540fc329b71a977a2be20c66ae2ae39df78), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *arc_bbcio_aga30_device::device_rom_region() const
{
	return ROM_NAME( aga30 );
}

const tiny_rom_entry *arc_ua_morley_device::device_rom_region() const
{
	return ROM_NAME( ua );
}

const tiny_rom_entry *arc_uma_morley_device::device_rom_region() const
{
	return ROM_NAME( uma );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_io_morley_device::add_userport(machine_config &config)
{
	via6522_device &via(MOS6522(config, "via", DERIVED_CLOCK(1, 4)));
	via.irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	via.readpa_handler().set(m_analog, FUNC(bbc_analogue_slot_device::pb_r)).lshift(2);
	via.writepa_handler().set([this](u8 data) { m_rom_page = data; });
	via.readpb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_r));
	via.writepb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_w));
	via.cb1_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb1));
	via.cb2_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb2));

	bbc_userport_slot_device &userport(BBC_USERPORT_SLOT(config, "userport", bbc_userport_devices, nullptr));
	userport.cb1_handler().set("via", FUNC(via6522_device::write_cb1));
	userport.cb2_handler().set("via", FUNC(via6522_device::write_cb2));
}

void arc_io_morley_device::add_analogue(machine_config &config)
{
	upd7002_device &upd7002(UPD7002(config, "upd7002", DERIVED_CLOCK(1, 4)));
	upd7002.set_get_analogue_callback(FUNC(arc_io_morley_device::get_analogue_input));
	upd7002.set_eoc_callback(FUNC(arc_io_morley_device::upd7002_eoc));

	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);
	m_analog->lpstb_handler().set("via", FUNC(via6522_device::write_ca1));
}

void arc_io_morley_device::add_midi(machine_config &config)
{
	scn2681_device &uart(SCN2681(config, "uart", DERIVED_CLOCK(1, 4)));
	uart.irq_cb().set(m_irqs, FUNC(input_merger_device::in_w<2>));
	uart.a_tx_cb().set("mdout", FUNC(midi_port_device::write_txd));
	uart.a_tx_cb().append("mdthru", FUNC(midi_port_device::write_txd));

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set("uart", FUNC(scn2681_device::rx_a_w));
	mdin.rxd_handler().append("mdthru", FUNC(midi_port_device::write_txd));

	midiout_slot(MIDI_PORT(config, "mdthru"));
	midiout_slot(MIDI_PORT(config, "mdout"));
}


void arc_bbcio_aga30_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set([this](int state) { set_pirq(state); });
	add_userport(config);
	add_analogue(config);
}

void arc_ua_morley_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set([this](int state) { set_pirq(state); });
	add_userport(config);
	add_analogue(config);
}

void arc_uma_morley_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set([this](int state) { set_pirq(state); });
	add_userport(config);
	add_analogue(config);
	add_midi(config);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_io_morley_device - constructor
//-------------------------------------------------

arc_io_morley_device::arc_io_morley_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_irqs(*this, "irqs")
	, m_analog(*this, "analogue")
	, m_rom_page(0)
{
}

arc_bbcio_aga30_device::arc_bbcio_aga30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_io_morley_device(mconfig, ARC_BBCIO_AGA30, tag, owner, clock)
{
}

arc_ua_morley_device::arc_ua_morley_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_io_morley_device(mconfig, ARC_UA_MORLEY, tag, owner, clock)
{
}

arc_uma_morley_device::arc_uma_morley_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_io_morley_device(mconfig, ARC_UMA_MORLEY, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_io_morley_device::device_start()
{
	save_item(NAME(m_rom_page));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_io_morley_device::device_reset()
{
	m_rom_page = 0x00;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

int arc_io_morley_device::get_analogue_input(int channel_number)
{
	return m_analog->ch_r(channel_number) << 8;
}

void arc_io_morley_device::upd7002_eoc(int state)
{
	m_irqs->in_w<1>(!state);
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_BBCIO_AGA30, device_archimedes_podule_interface, arc_bbcio_aga30_device, "arc_bbcio_aga30", "Acorn AGA30 BBC I/O Podule")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_UA_MORLEY, device_archimedes_podule_interface, arc_ua_morley_device, "arc_ua_morley", "Morley Electronics Analogue and User Interface")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_UMA_MORLEY, device_archimedes_podule_interface, arc_uma_morley_device, "arc_uma_morley", "Morley Electronics User/MIDI/Analogue Interface")
