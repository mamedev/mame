// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    HCCS User/Analogue Podule

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/HCCS_UserAnalogue.html

**********************************************************************/

#include "emu.h"
#include "io_hccs.h"
#include "machine/6522via.h"
#include "machine/upd7002.h"
#include "bus/bbc/analogue/analogue.h"
#include "bus/bbc/userport/userport.h"


namespace {

// ======================> arc_upa_hccs_device

class arc_upa_hccs_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_upa_hccs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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
	required_device<bbc_analogue_slot_device> m_analog;

	int get_analogue_input(int channel_number);
};


void arc_upa_hccs_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | 0x4000]; })).umask32(0x000000ff);
	map(0x2000, 0x203f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask32(0x000000ff);
	map(0x3000, 0x300f).rw("upd7002", FUNC(upd7002_device::read), FUNC(upd7002_device::write)).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( upa_hccs )
//-------------------------------------------------

ROM_START( upa_hccs )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "7", "V7.00 (04 Jul 1991)")
	ROMX_LOAD("upa_v7.00.bin", 0x0000, 0x8000, CRC(e4d5514e) SHA1(1a5d5c2571624df6fbeff91045c5b40effeda1d9), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *arc_upa_hccs_device::device_rom_region() const
{
	return ROM_NAME( upa_hccs );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_upa_hccs_device::device_add_mconfig(machine_config &config)
{
	via6522_device &via(MOS6522(config, "via", DERIVED_CLOCK(1, 4)));
	via.irq_handler().set([this](int state) { set_pirq(state); });
	via.readpa_handler().set(m_analog, FUNC(bbc_analogue_slot_device::pb_r));
	via.readpb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_r));
	via.writepb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_w));
	via.cb1_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb1));
	via.cb2_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb2));

	bbc_userport_slot_device &userport(BBC_USERPORT_SLOT(config, "userport", bbc_userport_devices, nullptr));
	userport.cb1_handler().set("via", FUNC(via6522_device::write_cb1));
	userport.cb2_handler().set("via", FUNC(via6522_device::write_cb2));

	upd7002_device &upd7002(UPD7002(config, "upd7002", DERIVED_CLOCK(1, 4)));
	upd7002.set_get_analogue_callback(FUNC(arc_upa_hccs_device::get_analogue_input));
	upd7002.set_eoc_callback("via", FUNC(via6522_device::write_ca2));

	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);
	m_analog->lpstb_handler().set("via", FUNC(via6522_device::write_ca1));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_upa_hccs_device - constructor
//-------------------------------------------------

arc_upa_hccs_device::arc_upa_hccs_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_UPA_HCCS, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_analog(*this, "analogue")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_upa_hccs_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

int arc_upa_hccs_device::get_analogue_input(int channel_number)
{
	return m_analog->ch_r(channel_number) << 8;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_UPA_HCCS, device_archimedes_podule_interface, arc_upa_hccs_device, "arc_upa_hccs", "HCCS User/Analogue Podule")
