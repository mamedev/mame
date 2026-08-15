// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Commodore/DKB A4091

    Zorro III SCSI host adapter based on the NCR 53C710

    Hardware:
    - NCR 53C710 SCSI I/O processor
    - 50 MHz XTAL
    - Internal and external SCSI-2 connectors
    - 32 KiB boot ROM

    Notes:
    - Also sold by DKB in license

    TODO:
    - Map dip switch ID to actually used ID

***************************************************************************/

#include "emu.h"
#include "a4091.h"

#include "bus/nscsi/devices.h"
#include "machine/53c7xx.h"
#include "machine/nscsi_bus.h"

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace bus::amiga::zorro {


class a4091_device : public device_t, public device_zorro3_card_interface
{
public:
	a4091_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void busrst_w(int state) override;
	virtual void cfgin_w(int state) override;

private:
	static constexpr uint32_t BOARD_SIZE = 0x01000000;

	void mmio_map(address_map &map) ATTR_COLD;

	uint8_t dips_r();

	uint32_t autoconfig_r(offs_t offset, uint32_t mem_mask);
	void autoconfig_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void autoconfig_base_address(offs_t address);

	uint32_t rom_r(offs_t offset);
	uint32_t dma_r(offs_t offset, uint32_t mem_mask);
	void dma_w(offs_t offset, uint32_t data, uint32_t mem_mask);

	required_device<nscsi_bus_device> m_scsi;
	required_device<ncr53c710_device> m_ncr;
	required_region_ptr<uint8_t> m_rom;
	required_ioport m_dips;

	uint32_t m_base_address = 0;
	uint32_t m_autoconfig_address = 0;
};

a4091_device::a4091_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_A4091, tag, owner, clock),
	device_zorro3_card_interface(mconfig, *this),
	m_scsi(*this, "scsi"),
	m_ncr(*this, "ncr"),
	m_rom(*this, "rom"),
	m_dips(*this, "dips")
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void a4091_device::mmio_map(address_map &map)
{
	map(0x000000, 0x03ffff).mirror(0x7c0000).r(FUNC(a4091_device::rom_r));
	map(0x800000, 0x80003f).mirror(0x07ffc0).rw(m_ncr, FUNC(ncr53c710_device::read), FUNC(ncr53c710_device::write));
	map(0x8c0000, 0x8c0003).r(FUNC(a4091_device::dips_r)).umask32(0x000000ff);
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( a4091 )
	PORT_START("dips")
	PORT_DIPNAME(0x07, 0x07, "SCSI Address") PORT_DIPLOCATION("DIP:1,2,3")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x01, "1")
	PORT_DIPSETTING(0x02, "2")
	PORT_DIPSETTING(0x03, "3")
	PORT_DIPSETTING(0x04, "4")
	PORT_DIPSETTING(0x05, "5")
	PORT_DIPSETTING(0x06, "6")
	PORT_DIPSETTING(0x07, "7")
	PORT_DIPNAME(0x08, 0x08, "SCSI Fast Bus") PORT_DIPLOCATION("DIP:4")
	PORT_DIPSETTING(0x08, DEF_STR( Off ))
	PORT_DIPSETTING(0x00, DEF_STR( On ))
	PORT_DIPNAME(0x10, 0x10, "Short/Long Spinup") PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(0x10, DEF_STR( Off ))
	PORT_DIPSETTING(0x00, DEF_STR( On ))
	PORT_DIPNAME(0x20, 0x20, "Synchronous Mode") PORT_DIPLOCATION("DIP:6")
	PORT_DIPSETTING(0x20, DEF_STR( Off ))
	PORT_DIPSETTING(0x00, DEF_STR( On ))
	PORT_DIPNAME(0x40, 0x40, "External SCSI Termination") PORT_DIPLOCATION("DIP:7")
	PORT_DIPSETTING(0x40, DEF_STR( Off ))
	PORT_DIPSETTING(0x00, DEF_STR( On ))
	PORT_DIPNAME(0x80, 0x80, "Logical Unit (LUN) Enable") PORT_DIPLOCATION("DIP:8")
	PORT_DIPSETTING(0x80, DEF_STR( Off ))
	PORT_DIPSETTING(0x00, DEF_STR( On ))
INPUT_PORTS_END

ioport_constructor a4091_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a4091 );
}

uint8_t a4091_device::dips_r()
{
	return m_dips->read();
}


//**************************************************************************
//  ZORRO / AUTOCONFIG
//**************************************************************************

void a4091_device::busrst_w(int state)
{
	if (state)
		return;

	int2_w(0);
	m_ncr->reset();

	if (m_base_address)
		zorro3_space().unmap_readwrite(m_base_address, m_base_address + BOARD_SIZE - 1);

	m_base_address = 0;
	m_autoconfig_address = 0;
}

void a4091_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state)
		return;

	// the autoconfig nibbles are stored in the boot ROM
	zorro3_space().install_readwrite_handler(0xff000000, 0xff00ffff,
		read32s_delegate(*this, FUNC(a4091_device::autoconfig_r)),
		write32s_delegate(*this, FUNC(a4091_device::autoconfig_w)), 0xffffffff);
}

uint32_t a4091_device::autoconfig_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t const data = (uint32_t(m_rom[offset] | 0x0f) << 24) | 0x00ffffff;

	if (!machine().side_effects_disabled())
		LOG("autoconfig_r: %08x @ %04x [mask = %08x]\n", data, uint32_t(offset << 2), mem_mask);

	return data;
}

void a4091_device::autoconfig_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t const reg = offset << 2;

	LOG("autoconfig_w: %08x @ %04x [mask = %08x]\n", data, reg, mem_mask);

	switch (reg)
	{
	case 0x44:
		if (ACCESSING_BITS_24_31)
			m_autoconfig_address = (m_autoconfig_address & 0x00ffffff) | (data & 0xff000000);
		if (ACCESSING_BITS_16_23)
			m_autoconfig_address = (m_autoconfig_address & 0xff00ffff) | (data & 0x00ff0000);

		// a write to the high byte completes the configuration
		if (ACCESSING_BITS_24_31)
			autoconfig_base_address(m_autoconfig_address);
		break;

	case 0x48:
		if (ACCESSING_BITS_24_31)
			m_autoconfig_address = (m_autoconfig_address & 0xff00ffff) | ((data >> 8) & 0x00ff0000);
		break;

	case 0x4c:
		if (ACCESSING_BITS_24_31)
			autoconfig_base_address(0);
		break;
	}
}

void a4091_device::autoconfig_base_address(offs_t address)
{
	LOG("Autoconfig address received: 0x%08x\n", uint32_t(address));

	// stop responding to default autoconfig
	zorro3_space().unmap_readwrite(0xff000000, 0xff00ffff);

	m_base_address = address;

	// install the board if we have a valid address
	if (m_base_address)
		zorro3_space().install_device(m_base_address, m_base_address + BOARD_SIZE - 1, *this, &a4091_device::mmio_map);

	// we're done
	cfgout_w(0);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint32_t a4091_device::rom_r(offs_t offset)
{
	// each rom byte is presented as two nibbles on alternate byte lanes
	uint8_t const high = m_rom[offset] | 0x0f;
	uint8_t const low = (m_rom[offset] << 4) | 0x0f;

	return (uint32_t(high) << 24) | 0x00ff0000 | (uint32_t(low) << 8) | 0x000000ff;
}

uint32_t a4091_device::dma_r(offs_t offset, uint32_t mem_mask)
{
	return zorro3_dma_r(offset, mem_mask);
}

void a4091_device::dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	zorro3_dma_w(offset, data, mem_mask);
}

void a4091_device::device_start()
{
	save_item(NAME(m_base_address));
	save_item(NAME(m_autoconfig_address));
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void a4091_device::device_add_mconfig(machine_config &config)
{
	NCR53C710(config, m_ncr, 50_MHz_XTAL);
	m_ncr->irq_handler().set([this](int state) { int2_w(state); });
	m_ncr->big_lit_handler().set_constant(1);
	m_ncr->host_read().set(FUNC(a4091_device::dma_r));
	m_ncr->host_write().set(FUNC(a4091_device::dma_w));

	NSCSI_BUS(config, m_scsi);
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	m_scsi->set_external_device(7, m_ncr);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( a4091 )
	ROM_REGION(0x10000, "rom", 0)
	ROM_DEFAULT_BIOS("v4013")

	ROM_SYSTEM_BIOS(0, "v404", "v40.4")
	ROMX_LOAD("a4091_v404.u206", 0x0000, 0x8000, CRC(9ba7e7dc) SHA1(18985b7ec95239da21a242a4fc42f13496434534), ROM_BIOS(0))
	ROM_RELOAD(                  0x8000, 0x8000)
	ROM_SYSTEM_BIOS(1, "v409", "v40.9")
	ROMX_LOAD("a4091_v409.u206", 0x0000, 0x8000, CRC(7e12a120) SHA1(a411d5726801dc443d41428f382ae3d56f44ef27), ROM_BIOS(1))
	ROM_RELOAD(                  0x8000, 0x8000)
	ROM_SYSTEM_BIOS(2, "v4013", "v40.13")
	ROMX_LOAD("391592-02_a4091_v4013.u206", 0x0000, 0x8000, CRC(54cb9e85) SHA1(3ce66919f6fd67974923a12d91b730f1ffb4a7ba), ROM_BIOS(2))
	ROM_RELOAD(                             0x8000, 0x8000)
ROM_END

const tiny_rom_entry *a4091_device::device_rom_region() const
{
	return ROM_NAME( a4091 );
}


} // namespace bus::amiga::zorro


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(AMIGA_A4091, device_zorro3_card_interface, bus::amiga::zorro::a4091_device, "amiga_a4091", "Commodore/DKB A4091 SCSI Host Adapter")
