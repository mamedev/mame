// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Apple "F108" memory controller
    Emulation by R. Belmont

    F108 contains:
    - A memory controller
    - The usual Mac ROM/RAM switch so at boot the processor has vectors at 0
    - An ATA bus interface
    - An SCC interface
    - A SCSI controller which is claimed to be "just like a 53C96".  A real 53C96 seems to work fine.
*/

#include "emu.h"
#include "f108.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/devices.h"
#include "bus/rs232/rs232.h"

#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(F108, f108_device, "macf108", "Apple F108 memory controller")

//-------------------------------------------------
//  ADDRESS_MAP
//-------------------------------------------------

void f108_device::map(address_map &map)
{
	map(0x40000000, 0x400fffff).r(FUNC(f108_device::rom_switch_r)).mirror(0x0ff00000).nopw();
	map(0x5000c000, 0x5000dfff).rw(FUNC(f108_device::scc_r), FUNC(f108_device::scc_w)).mirror(0x00fc0000);
	map(0x5001a000, 0x5001a01f).rw(m_ata, FUNC(ata_interface_device::cs0_swap_r), FUNC(ata_interface_device::cs0_swap_w)).umask32(0xffff0000).mirror(0x00fc0000);
	map(0x5001a000, 0x5001a003).rw(FUNC(f108_device::ata_data_r), FUNC(f108_device::ata_data_w)).mirror(0x00fc0000);
	map(0x5001a020, 0x5001a03f).rw(m_ata, FUNC(ata_interface_device::cs1_swap_r), FUNC(ata_interface_device::cs1_swap_w)).umask32(0xffff0000).mirror(0x00fc0000);
	// a040 is probably a configuration register based on later Apple ATA implementations, but the values written don't make sense
	map(0xf9000000, 0xf90fffff).ram();  // VRAM
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void f108_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
	m_ata->irq_handler().set(FUNC(f108_device::ata_irq_w));

	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3").option_set("cdrom", NSCSI_CDROM_APPLE).machine_config(
		[](device_t *device)
		{
			device->subdevice<cdda_device>("cdda")->add_route(0, "^^^primetimeii:speaker", 1.0, 0);
			device->subdevice<cdda_device>("cdda")->add_route(1, "^^^primetimeii:speaker", 1.0, 1);
		});
	NSCSI_CONNECTOR(config, "scsi:4", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mac_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53c96", NCR53C96).clock(40_MHz_XTAL).machine_config(
		[this] (device_t *device)
		{
			ncr53c96_device &adapter = downcast<ncr53c96_device &>(*device);

			adapter.set_busmd(ncr53c96_device::BUSMD_1);
			adapter.irq_handler_cb().set(m_primetimeii, FUNC(primetime_device::scsi_irq_w));
			adapter.drq_handler_cb().set(m_primetimeii, FUNC(primetime_device::scsi_drq_w));
		});

	SOFTWARE_LIST(config, "hdd_list").set_original("mac_hdd");
	SOFTWARE_LIST(config, "cd_list").set_original("mac_cdrom").set_filter("MC68040");

	SCC85C30(config, m_scc, 31.3344_MHz_XTAL/4);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(FUNC(f108_device::scc_irq_w));
	m_scc->out_txda_callback().set("printer", FUNC(rs232_port_device::write_txd));
	m_scc->out_txdb_callback().set("modem", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232a(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
}

//-------------------------------------------------
//  f108_device - constructor
//-------------------------------------------------

f108_device::f108_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, F108, tag, owner, clock),
	m_maincpu(*this, finder_base::DUMMY_TAG),
	m_primetimeii(*this, finder_base::DUMMY_TAG),
	m_ata(*this, "ata"),
	m_scsibus(*this, "scsi"),
	m_ncr1(*this, "scsi:7:ncr53c96"),
	m_scc(*this, "scc"),
	m_rom(*this, finder_base::DUMMY_TAG),
	m_ata_irq(*this),
	m_overlay(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void f108_device::device_start()
{
	m_rom_ptr = &m_rom[0];
	m_rom_size = m_rom.length() << 2;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void f108_device::device_reset()
{
	m_overlay = true;

	// put ROM mirror at 0
	address_space &space = m_maincpu->space(AS_PROGRAM);
	const u32 memory_size = std::min((u32)0x3fffff, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.unmap_write(0x00000000, memory_end);
	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);
}

u32 f108_device::rom_switch_r(offs_t offset)
{
	// disable the overlay
	if (m_overlay && !machine().side_effects_disabled())
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		const u32 memory_end = m_ram_size - 1;
		void *memory_data = m_ram_ptr;
		offs_t memory_mirror = memory_end & ~memory_end;

		space.install_ram(0x00000000, memory_end & ~memory_mirror, memory_mirror, memory_data);
		m_overlay = false;
	}

	return m_rom_ptr[offset & ((m_rom_size - 1) >> 2)];
}

void f108_device::set_ram_info(u32 *ram, u32 size)
{
	m_ram_ptr = ram;
	m_ram_size = size;
}

void f108_device::ata_irq_w(int state)
{
	m_ata_irq(state);
}

u32 f108_device::ata_data_r(offs_t offset, u32 mem_mask)
{
	u32 retval = 0;

	if (mem_mask == 0xffffffff)
	{
		retval = m_ata->cs0_swap_r(0, 0xffff) << 16;
		retval |= m_ata->cs0_swap_r(0, 0xffff);
	}
	else if ((mem_mask & 0xffff0000) != 0)
	{
		retval = m_ata->cs0_swap_r(0, mem_mask >> 16) << 16;
	}

	return retval;
}

void f108_device::ata_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask == 0xffffffff)
	{
		m_ata->cs0_swap_w(0, data >> 16, 0xffff);
		m_ata->cs0_swap_w(0, data & 0xffff, 0xffff);
	}
	else if ((mem_mask & 0xffff0000) != 0)
	{
		m_ata->cs0_swap_w(0, data >> 16, mem_mask >> 16);
	}
}

u16 f108_device::scc_r(offs_t offset)
{
	m_primetimeii->via_sync();
	u16 result = m_scc->dc_ab_r(offset);
	return (result << 8) | result;
}

void f108_device::scc_w(offs_t offset, u16 data)
{
	m_primetimeii->via_sync();
	m_scc->dc_ab_w(offset, data >> 8);
}

void f108_device::scc_irq_w(int state)
{
	m_primetimeii->scc_irq_w(state);
}
