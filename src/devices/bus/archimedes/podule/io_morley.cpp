// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Morley Electronics User I2C Analogue mini-podule

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/Morley_UserICAnalogue.html

    Acorn AGA30 BBC I/O Podule

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AGA30_AnalogueUserP.html

**********************************************************************/

#include "emu.h"
#include "io_morley.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/upd7002.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/bbc/userport/userport.h"


namespace {

// ======================> arc_bbcio_aga30_device

class arc_bbcio_aga30_device :
	public device_t,
	public device_archimedes_podule_interface
{
	public:
	// construction/destruction
	arc_bbcio_aga30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override;
	virtual void memc_map(address_map &map) override;

private:
	required_memory_region m_podule_rom;
	required_device<input_merger_device> m_irqs;
	optional_device<bbc_analogue_slot_device> m_analog;

	u8 m_rom_page;

	int get_analogue_input(int channel_number);
	void upd7002_eoc(int state);
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


//-------------------------------------------------
//  ROM( aga30 )
//-------------------------------------------------

ROM_START( aga30 )
	ROM_REGION(0x4000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "1", "First release 08-Mar-90")
	ROMX_LOAD("morleyio_v2.01.bin", 0x0000, 0x4000, CRC(b149f631) SHA1(ed8d0caa478f030e11348b23e915897f27dcf153), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *arc_bbcio_aga30_device::device_rom_region() const
{
	return ROM_NAME( aga30 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_bbcio_aga30_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set([this](int state) { set_pirq(state); });

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

	upd7002_device &upd7002(UPD7002(config, "upd7002", DERIVED_CLOCK(1, 4)));
	upd7002.set_get_analogue_callback(FUNC(arc_bbcio_aga30_device::get_analogue_input));
	upd7002.set_eoc_callback(FUNC(arc_bbcio_aga30_device::upd7002_eoc));

	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);
	m_analog->lpstb_handler().set("via", FUNC(via6522_device::write_ca1));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_bbcio_aga30_device - constructor
//-------------------------------------------------

arc_bbcio_aga30_device::arc_bbcio_aga30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_BBCIO_AGA30, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_irqs(*this, "irqs")
	, m_analog(*this, "analogue")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_bbcio_aga30_device::device_start()
{
	save_item(NAME(m_rom_page));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_bbcio_aga30_device::device_reset()
{
	m_rom_page = 0x00;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

int arc_bbcio_aga30_device::get_analogue_input(int channel_number)
{
	return (0xff - m_analog->ch_r(channel_number)) << 8;
}

void arc_bbcio_aga30_device::upd7002_eoc(int state)
{
	m_irqs->in_w<1>(!state);
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_BBCIO_AGA30, device_archimedes_podule_interface, arc_bbcio_aga30_device, "arc_bbcio_aga30", "Acorn AGA30 BBC I/O Podule")
