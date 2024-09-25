// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn A500 SCSI Interface

    https://www.domesday86.com/?page_id=936#Prototype_SCSI_card

    As this podule contains no identification in ROM then the SCSI modules
    loaded from hard drive would expect to find the SCSI controller in
    podule slot 1.

    TODO:
    - map SCSI controller

**********************************************************************/

#include "emu.h"
#include "scsi_a500.h"
#include "machine/wd33c9x.h"
#include "bus/nscsi/devices.h"


namespace {

class arc_scsi_a500_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_scsi_a500_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_archimedes_podule_interface overrides
	virtual void ioc_map(address_map &map) override ATTR_COLD;

private:
	required_device<wd33c93_device> m_wd33c93;
	required_memory_region m_podule_rom;
};


void arc_scsi_a500_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset]; })).umask32(0x000000ff);
	//map(0x2000, 0x2007).rw(m_wd33c93, FUNC(wd33c93_device::indir_r), FUNC(wd33c93_device::indir_w)).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( scsi_a500 )
//-------------------------------------------------

ROM_START( scsi_a500 )
	ROM_REGION(0x2000, "podule_rom", ROMREGION_ERASEFF)
	ROM_LOAD("blank.rom", 0x0000, 0x2000, NO_DUMP) // ROM is labelled 'blank' and contains FF's
ROM_END

const tiny_rom_entry *arc_scsi_a500_device::device_rom_region() const
{
	return ROM_NAME( scsi_a500 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_scsi_a500_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr, false); // Philips VP415
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wd33c93", WD33C93).clock(DERIVED_CLOCK(1, 1))
		.machine_config([this](device_t *device)
		{
			wd33c93_device &wd33c93(downcast<wd33c93_device &>(*device));
			wd33c93.irq_cb().set([this](int state) { set_pirq(state); });
		});
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_scsi_a500_device - constructor
//-------------------------------------------------

arc_scsi_a500_device::arc_scsi_a500_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_SCSI_A500, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_wd33c93(*this, "scsi:7:wd33c93")
	, m_podule_rom(*this, "podule_rom")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_scsi_a500_device::device_start()
{
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCSI_A500, device_archimedes_podule_interface, arc_scsi_a500_device, "arc_scsi_a500", "Acorn A500 SCSI Interface")
