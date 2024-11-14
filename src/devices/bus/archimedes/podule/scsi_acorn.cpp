// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn AKA31 SCSI Expansion Card

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AKA31_SCSI.html

    Acorn AKA32 CDFS & SCSI Expansion Card

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesA2G/Acorn_AKA32_SCSI.html

    TODO:
    - everything, this is skeleton with ROM.

**********************************************************************/

#include "emu.h"
#include "scsi_acorn.h"
#include "machine/upd71071.h"
#include "machine/wd33c9x.h"
#include "bus/nscsi/devices.h"


namespace {

// ======================> arc_scsi_aka31_device

class arc_scsi_aka31_device :
	public device_t,
	public device_archimedes_podule_interface
{
public:
	// construction/destruction
	arc_scsi_aka31_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	arc_scsi_aka31_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

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
	required_device<wd33c93a_device> m_wd33c93;
	required_device<upd71071_device> m_dmac;
	required_memory_region m_podule_rom;

	void update_interrupts();

	u8 m_memory_page;
	u8 m_interrupt_status;
	int m_sbic_int;
	int m_dmac_int;
	std::unique_ptr<u16[]> m_podule_ram;
};


// ======================> arc_scsi_aka32_device

class arc_scsi_aka32_device : public arc_scsi_aka31_device
{
public:
	// construction/destruction
	arc_scsi_aka32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


void arc_scsi_aka31_device::ioc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr8(NAME([this](offs_t offset) { return m_podule_rom->base()[offset | (((m_memory_page & 0x3f) << 11) & 0xf800)]; })).umask32(0x000000ff);
	map(0x2000, 0x2fff).lrw8([this]() { return m_interrupt_status; }, "isr_r", [this](u8 data) { m_dmac_int = 0; update_interrupts(); }, "clrint_w").umask32(0x000000ff);
	map(0x3000, 0x3000).mirror(0x0fff).lw8(NAME([this](u8 data) { m_memory_page = data; m_wd33c93->reset_w(BIT(data, 7)); update_interrupts(); }));
}

void arc_scsi_aka31_device::memc_map(address_map &map)
{
	map(0x0000, 0x1fff).lr16(NAME([this](offs_t offset) { return m_podule_ram[offset | (((m_memory_page & 0x3f) << 11) & 0xf800)]; })).umask32(0x0000ffff);
	map(0x2000, 0x2007).mirror(0x0ff8).rw(m_wd33c93, FUNC(wd33c93a_device::indir_r), FUNC(wd33c93a_device::indir_w)).umask32(0x000000ff);
	map(0x3000, 0x33ff).mirror(0x0c00).lr8(NAME([this](offs_t offset) { return m_dmac->read(bitswap<8>(offset, 0, 6, 5, 4, 3, 2, 1, 7)); })).umask32(0x000000ff);
	map(0x3000, 0x33ff).mirror(0x0c00).lw8(NAME([this](offs_t offset, u8 data) { m_dmac->write(bitswap<8>(offset, 0, 6, 5, 4, 3, 2, 1, 7), data); })).umask32(0x000000ff);
}


//-------------------------------------------------
//  ROM( aka31 )
//-------------------------------------------------

ROM_START( aka31 )
	ROM_REGION(0x10000, "podule_rom", 0)
	ROM_LOAD("aka31_0273,210-03_scsi_id_prom.rom", 0x0000, 0x10000, CRC(d8a51876) SHA1(fedbd8d8dafb225b931c76704ccb93d4ebdaea10))
ROM_END

ROM_START( aka32 )
	ROM_REGION(0x10000, "podule_rom", 0)
	ROM_SYSTEM_BIOS(0, "aka35", "AKA35")
	ROMX_LOAD("aka35.rom", 0x0000, 0x10000, CRC(2d00a646) SHA1(80e7a88d17bdf5d2111e4e2d1b406a1d8692d41b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "aka33", "AKA33")
	ROMX_LOAD("aka33.rom", 0x0000, 0x10000, CRC(f5b4b6b8) SHA1(ab47366e569add39aea0c6e90e92a4dfd16cb898), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "aka32", "AKA32")
	ROMX_LOAD("aka32_acorn_cdfs_version_2.20.rom", 0x0000, 0x10000, CRC(65d74620) SHA1(42bddbe2841de7085371c5fe11d7c9b152070886), ROM_BIOS(2))
ROM_END

const tiny_rom_entry *arc_scsi_aka31_device::device_rom_region() const
{
	return ROM_NAME( aka31 );
}

const tiny_rom_entry *arc_scsi_aka32_device::device_rom_region() const
{
	return ROM_NAME( aka32 );
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_scsi_aka31_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk", false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wd33c93a", WD33C93A).clock(DERIVED_CLOCK(1, 1))
		.machine_config([this](device_t *device)
		{
			wd33c93a_device &wd33c93(downcast<wd33c93a_device &>(*device));
			wd33c93.irq_cb().set([this](int state) { m_sbic_int = state; update_interrupts(); });
			wd33c93.drq_cb().set([this](int state) { m_dmac->dmarq(state, 0); });
		});

	UPD71071(config, m_dmac, 0);
	m_dmac->set_cpu_tag(":maincpu");
	m_dmac->set_clock(DERIVED_CLOCK(1, 1));
	m_dmac->out_eop_callback().set([this](int state) { m_dmac_int = state; update_interrupts(); });
	m_dmac->dma_read_callback<0>().set(m_wd33c93, FUNC(wd33c93a_device::dma_r));
	m_dmac->dma_write_callback<0>().set(m_wd33c93, FUNC(wd33c93a_device::dma_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  arc_scsi_aka31_device - constructor
//-------------------------------------------------

arc_scsi_aka31_device::arc_scsi_aka31_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_archimedes_podule_interface(mconfig, *this)
	, m_wd33c93(*this, "scsi:7:wd33c93a")
	, m_dmac(*this, "dma")
	, m_podule_rom(*this, "podule_rom")
	, m_memory_page(0)
	, m_interrupt_status(0)
	, m_sbic_int(0)
	, m_dmac_int(0)
{
}

arc_scsi_aka31_device::arc_scsi_aka31_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_scsi_aka31_device(mconfig, ARC_SCSI_AKA31, tag, owner, clock)
{
}

arc_scsi_aka32_device::arc_scsi_aka32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: arc_scsi_aka31_device(mconfig, ARC_SCSI_AKA32, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void arc_scsi_aka31_device::device_start()
{
	m_podule_ram = std::make_unique<u16[]>(0x8000);

	save_item(NAME(m_memory_page));
	save_item(NAME(m_interrupt_status));
	save_pointer(NAME(m_podule_ram), 0x8000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void arc_scsi_aka31_device::device_reset()
{
	m_memory_page = 0x00;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void arc_scsi_aka31_device::update_interrupts()
{
	// SBIC interrupt
	if (m_sbic_int)
		m_interrupt_status |= m_sbic_int << 3;
	else
		m_interrupt_status &= ~(m_sbic_int << 3);

	// DMAC terminal count interrupt
	if (m_dmac_int)
		m_interrupt_status |= m_dmac_int << 1;
	else
		m_interrupt_status &= ~(m_dmac_int << 1);

	// SEC requesting IRQ
	if (m_dmac_int || m_sbic_int)
		m_interrupt_status |= 0x01;
	else
		m_interrupt_status &= ~(0x01);

	if (BIT(m_memory_page, 6)) // interrupts enabled
		set_pirq(BIT(m_interrupt_status, 0));
	else
		set_pirq(0);
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCSI_AKA31, device_archimedes_podule_interface, arc_scsi_aka31_device, "arc_scsi_aka31", "Acorn AKA31 SCSI Expansion Card")
DEFINE_DEVICE_TYPE_PRIVATE(ARC_SCSI_AKA32, device_archimedes_podule_interface, arc_scsi_aka32_device, "arc_scsi_aka32", "Acorn AKA32 CDFS & SCSI Expansion Card")
