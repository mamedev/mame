// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Morley Electronics 16bit Cached SCSI card

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/Morley_16bitCachedSCSI.html

    TODO:
    - ESP216 SCSI controller
    - everything, this is skeleton with ROM.

**********************************************************************/

#include "emu.h"
#include "scsi_morley.h"
#include "machine/7200fifo.h"
#include "machine/ncr53c90.h"
#include "bus/nscsi/devices.h"


namespace {

class arc_scsi_morley_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_scsi_morley_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

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
	required_device<ncr53c94_device> m_esp216;
	required_device<idt7201_device> m_fifo_in;
	required_device<idt7201_device> m_fifo_out;
	required_memory_region m_podule_rom;

	u8 m_rom_page;
};


void arc_scsi_morley_device::ioc_map(address_map &map)
{
	map(0x0000, 0x0fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | ((m_rom_page << 10) & 0xfc00)]; })).umask32(0x000000ff);
	//map(0x3000, 0x3000).lrw8(NAME([this]() { return m_rom_page; }), NAME([this](u8 data) { m_rom_page = data; }));
	//map(0x3000, 0x303f).rw(m_esp216, FUNC(ncr53c94_device::read), FUNC(ncr53c94_device::write)).umask32(0x000000ff);
}

void arc_scsi_morley_device::memc_map(address_map &map)
{
	map(0x0000, 0x0000).lrw8(NAME([this]() { return m_rom_page; }), NAME([this](u8 data) { m_rom_page = data; }));
}


//-------------------------------------------------
//  ROM( scsi_morley )
//-------------------------------------------------

ROM_START( scsi_morley )
	ROM_REGION(0x10000, "podule_rom", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "119", "V1.19")
	ROMX_LOAD("morley_scsi_rom_v1.19.rom", 0x0000, 0x10000, CRC(78e2e0da) SHA1(08d8f6233d5cae8a3a04f29c16c8cf85788dad46), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "117", "V1.17")
	ROMX_LOAD("morley_scsi_rom_v1.17.rom", 0x0000, 0x10000, CRC(1397a7b3) SHA1(5b0b7d2bb123db85922b493213c6d60ad4d89020), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "114", "V1.14")
	ROMX_LOAD("morley_scsi_rom_v1.14.rom", 0x0000, 0x10000, CRC(c6e15260) SHA1(e2b591484d47d4b4e0edb823f992257f03d32bd6), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "110", "V1.10")
	ROMX_LOAD("morley_scsi_rom_v1.10.rom", 0x0000, 0x10000, CRC(a0941679) SHA1(e92902e67d955e8742a7ac92dd86a8f212801dc5), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "108", "V1.08")
	ROMX_LOAD("morley_scsi_rom_v1.08.rom", 0x0000, 0x08000, CRC(e8357473) SHA1(9164ef9edc9d2ed6a7a7c9ba6258e02db9b1d2cb), ROM_BIOS(4))
ROM_END

const tiny_rom_entry *arc_scsi_morley_device::device_rom_region() const
{
	return ROM_NAME( scsi_morley );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_scsi_morley_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("esp216", NCR53CF94).clock(24_MHz_XTAL) // ESP216
		.machine_config([this](device_t *device)
		{
			ncr53c94_device &esp216(downcast<ncr53c94_device &>(*device));
			esp216.set_busmd(ncr53c94_device::busmd_t::BUSMD_1);
			esp216.irq_handler_cb().set([this](int state) { set_pirq(state); });
			//esp216.drq_handler_cb().set([this](int state) { m_drq_status = state; });
		});

	IDT7201(config, m_fifo_out);
	//m_fifo_out->hf_handler().set([this](int state) { set_irq(IRQ_FIFO_OUT, state); });
	IDT7201(config, m_fifo_in);
	//m_fifo_in->hf_handler().set([this](int state) { set_irq(IRQ_FIFO_IN, !state); });
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_scsi_morley_device - constructor
//-------------------------------------------------

arc_scsi_morley_device::arc_scsi_morley_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARC_SCSI_MORLEY, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_esp216(*this, "scsi:7:esp216")
	, m_fifo_in(*this, "fifo_in")
	, m_fifo_out(*this, "fifo_out")
	, m_podule_rom(*this, "podule_rom")
	, m_rom_page(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_scsi_morley_device::device_start()
{
	save_item(NAME(m_rom_page));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_scsi_morley_device::device_reset()
{
	m_rom_page = 0x00;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCSI_MORLEY, device_archimedes_podule_interface, arc_scsi_morley_device, "arc_scsi_morley", "Morley Electronics 16bit Cached SCSI card")
