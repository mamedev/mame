// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Intelligent Interfaces Dual RS423 Serial Interface

    TODO:
    - everything, this is skeleton with ROM.

**********************************************************************/

#include "emu.h"
#include "rs423.h"
#include "machine/6522via.h"
#include "machine/mc68681.h"
#include "bus/rs232/rs232.h"


namespace {

// ======================> arc_rs423_device

class arc_rs423_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_rs423_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

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


void arc_rs423_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0x7800)]; })).umask32(0x000000ff);
	//map(0x2000, 0x203f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask32(0x000000ff);
	//map(0x3000, 0x303f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( rs423 )
//-------------------------------------------------

ROM_START( rs423 )
	ROM_REGION(0x8000, "podule_rom", 0)
	ROM_LOAD("iidual_id.rom", 0x0000, 0x8000, CRC(ae113882) SHA1(eb97839cd66dce33598aed90ac2f0624de1aa60d))
ROM_END

const tiny_rom_entry *arc_rs423_device::device_rom_region() const
{
	return ROM_NAME( rs423 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_rs423_device::device_add_mconfig(machine_config &config)
{
	via6522_device &via(R65C22(config, "via", DERIVED_CLOCK(1, 4)));
	via.writepa_handler().set([this](u8 data) { m_rom_page = data; });
	via.irq_handler().set([this](int state) { set_pirq(state); });

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL)); // SC26C92
	duart.a_tx_cb().set("rs232a", FUNC(rs232_port_device::write_txd));
	duart.b_tx_cb().set("rs232b", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set("duart", FUNC(scn2681_device::rx_a_w));
	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("duart", FUNC(scn2681_device::rx_b_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_rs423_device - constructor
//-------------------------------------------------

arc_rs423_device::arc_rs423_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_RS423, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_rs423_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_rs423_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_RS423, device_archimedes_podule_interface, arc_rs423_device, "arc_rs423", "Archimedes Dual RS423 Serial Interface")
