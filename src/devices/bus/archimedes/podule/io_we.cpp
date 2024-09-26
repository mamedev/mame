// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics A3000 BBC User I/O

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/WE_A3000_IICUserAnalog.html

**********************************************************************/

#include "emu.h"
#include "io_we.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/upd7002.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/bbc/userport/userport.h"


namespace {

// ======================> arc_bbcio_we_device

class arc_bbcio_we_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_bbcio_we_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;
	virtual void memc_map(address_map &map) override ATTR_COLD;

private:
	required_memory_region m_podule_rom;
	required_device<input_merger_device> m_irqs;
	required_device<bbc_analogue_slot_device> m_analog;
	required_device<upd7002_device> m_upd7002;

	u8 m_rom_page;

	int get_analogue_input(int channel_number);
	void upd7002_eoc(int state);

	int m_adc_irq;
	int m_via_irq;
};


void arc_bbcio_we_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x3800)]; })).umask32(0x000000ff);
	map(0x2000, 0x203f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask32(0x000000ff);
	map(0x3000, 0x3000).lr8(NAME([this]() { return (m_via_irq << 7); }));
	map(0x3800, 0x3800).lr8(NAME([this]() { return (m_adc_irq << 7); }));
}

void arc_bbcio_we_device::memc_map(address_map &map)
{
	map(0x1000, 0x10ff).lr8(NAME([this](offs_t offset) { return m_upd7002->read(offset >> 4); })).umask32(0x000000ff);
	map(0x1000, 0x10ff).lw8(NAME([this](offs_t offset, u8 data) { m_upd7002->write(offset >> 4, data); })).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( bbcio_we )
//-------------------------------------------------

ROM_START( bbcio_we )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "100", "1.00 (16 Jan 1992)")
	ROMX_LOAD("a3000_io-1.01.bin", 0x0000, 0x4000, CRC(a432bdcd) SHA1(05c79e38707e406eed23cec6b3d2730e8fe550e8), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *arc_bbcio_we_device::device_rom_region() const
{
	return ROM_NAME( bbcio_we );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_bbcio_we_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set([this](int state) { set_pirq(state); });

	via6522_device &via(MOS6522(config, "via", DERIVED_CLOCK(1, 4)));
	via.irq_handler().set([this](int state) { m_via_irq = state; m_irqs->in_w<0>(state); });
	via.readpa_handler().set(m_analog, FUNC(bbc_analogue_slot_device::pb_r)).lshift(2);
	via.writepa_handler().set([this](u8 data) { m_rom_page = data; });
	via.readpb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_r));
	via.writepb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_w));
	via.cb1_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb1));
	via.cb2_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb2));

	bbc_userport_slot_device &userport(BBC_USERPORT_SLOT(config, "userport", bbc_userport_devices, nullptr));
	userport.cb1_handler().set("via", FUNC(via6522_device::write_cb1));
	userport.cb2_handler().set("via", FUNC(via6522_device::write_cb2));

	upd7002_device &upd7002(UPD7002(config, "upd7002", DERIVED_CLOCK(1, 4)));
	upd7002.set_get_analogue_callback(FUNC(arc_bbcio_we_device::get_analogue_input));
	upd7002.set_eoc_callback(FUNC(arc_bbcio_we_device::upd7002_eoc));

	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);
	m_analog->lpstb_handler().set("via", FUNC(via6522_device::write_ca1));

	// TODO: internal A3000 version adds IIC port
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_bbcio_we_device - constructor
//-------------------------------------------------

arc_bbcio_we_device::arc_bbcio_we_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_BBCIO_WE, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_irqs(*this, "irqs")
	, m_analog(*this, "analogue")
	, m_upd7002(*this, "upd7002")
	, m_rom_page(0)
	, m_adc_irq(0)
	, m_via_irq(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_bbcio_we_device::device_start()
{
	save_item(NAME(m_rom_page));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_bbcio_we_device::device_reset()
{
	m_rom_page = 0x00;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

int arc_bbcio_we_device::get_analogue_input(int channel_number)
{
	return m_analog->ch_r(channel_number) << 8;
}

void arc_bbcio_we_device::upd7002_eoc(int state)
{
	m_adc_irq = !state;

	m_irqs->in_w<1>(!state);
}


} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_BBCIO_WE, device_archimedes_podule_interface, arc_bbcio_we_device, "arc_bbcio_we", "Watford Electronics BBC User I/O Card")
