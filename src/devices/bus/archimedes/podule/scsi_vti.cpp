// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    VTI User Port and SCSI Podule

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/VTI_userportSCSI.html

    TODO:
    - map the EEPROM
    - fix User Port module

**********************************************************************/

#include "emu.h"
#include "scsi_vti.h"
#include "machine/6522via.h"
#include "machine/eepromser.h"
#include "machine/ncr53c90.h"
#include "bus/bbc/userport/userport.h"
#include "bus/nscsi/devices.h"


namespace {

class arc_scsi_vti_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_scsi_vti_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type imperfect_features() { return feature::DISK; }

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
	required_device<ncr53c94_device> m_fas216;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_memory_region m_podule_rom;

	u8 m_rom_page;
	u8 m_drq_status;
};


void arc_scsi_vti_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 11) & 0xf800)]; })).umask32(0x000000ff);
	map(0x3d00, 0x3d3f).rw("via", FUNC(via6522_device::read), FUNC(via6522_device::write)).umask32(0x000000ff);
}

void arc_scsi_vti_device::memc_map(address_map &map)
{
	map(0x2000, 0x21ff).lr8(NAME([this](offs_t offset) { return m_fas216->read(offset >> 3); })).umask32(0x000000ff);
	map(0x2000, 0x21ff).lw8(NAME([this](offs_t offset, u8 data) { m_fas216->write(offset >> 3, data); })).umask32(0x000000ff);
	map(0x2200, 0x2203).rw(m_fas216, FUNC(ncr53c94_device::dma16_r), FUNC(ncr53c94_device::dma16_w)).umask32(0x0000ffff);
	map(0x2600, 0x2600).lr8(NAME([this]() { return m_drq_status; }));
}


//-------------------------------------------------
//  ROM( scsi_vti )
//-------------------------------------------------

ROM_START( scsi_vti )
	ROM_REGION(0x10000, "podule_rom", 0)
	ROM_LOAD("vti_up_powerrom.bin", 0x0000, 0x10000, CRC(7ec5854c) SHA1(eb20b1369964cfaee156ba1fc3636997407cc7ba))
ROM_END

const tiny_rom_entry *arc_scsi_vti_device::device_rom_region() const
{
	return ROM_NAME( scsi_vti );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_scsi_vti_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("fas216", NCR53CF94).clock(24.576_MHz_XTAL) // FAS216
		.machine_config([this](device_t *device)
		{
			ncr53c94_device &fas216(downcast<ncr53c94_device &>(*device));
			fas216.set_busmd(ncr53c94_device::busmd_t::BUSMD_1);
			fas216.irq_handler_cb().set([this](int state) { set_pirq(state); });
			fas216.drq_handler_cb().set([this](int state) { m_drq_status = state; });
		});

	EEPROM_93C06_16BIT(config, m_eeprom); // 2600 b0

	via6522_device &via(MOS6522(config, "via", DERIVED_CLOCK(1, 4)));
	via.irq_handler().set([this](int state) { set_pirq(state); });
	via.writepa_handler().set([this](u8 data) { m_rom_page = data; });
	via.readpb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_r));
	via.writepb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_w));
	via.cb1_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb1));
	via.cb2_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb2));

	bbc_userport_slot_device &userport(BBC_USERPORT_SLOT(config, "userport", bbc_userport_devices, nullptr));
	userport.cb1_handler().set("via", FUNC(via6522_device::write_cb1));
	userport.cb2_handler().set("via", FUNC(via6522_device::write_cb2));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_scsi_vti_device - constructor
//-------------------------------------------------

arc_scsi_vti_device::arc_scsi_vti_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_SCSI_VTI, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_fas216(*this, "scsi:7:fas216")
	, m_eeprom(*this, "eeprom")
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
	, m_drq_status(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_scsi_vti_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_scsi_vti_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCSI_VTI, device_archimedes_podule_interface, arc_scsi_vti_device, "arc_scsi_vti", "VTI User Port and SCSI Podule")
