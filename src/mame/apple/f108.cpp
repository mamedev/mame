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

#include <algorithm>


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
	map(0x50f1a000, 0x50f1a01f).umask32(0xffff0000).rw(FUNC(f108_device::ata_cs0_r), FUNC(f108_device::ata_cs0_w));
	map(0x50f1a000, 0x50f1a003).rw(FUNC(f108_device::ata_data_r), FUNC(f108_device::ata_data_w));
	map(0x50f1a020, 0x50f1a03f).umask32(0xffff0000).rw(FUNC(f108_device::ata_cs1_r), FUNC(f108_device::ata_cs1_w));
	map(0x50f1a040, 0x50f1a04b).rw(FUNC(f108_device::ata_timing_r), FUNC(f108_device::ata_timing_w));
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
	// ... bus devices to be populated by drivers ...

	NCR53C96(config, m_ncr1, DERIVED_CLOCK(1, 2));
	m_scsibus->set_external_device(7, m_ncr1);
	m_ncr1->set_busmd(ncr53c96_device::BUSMD_1);


	SCC85C30(config, m_scc, 31.3344_MHz_XTAL/4);
	m_scc->configure_channels(3'686'400, 3'686'400, 3'686'400, 3'686'400);
	m_scc->out_int_callback().set(FUNC(f108_device::scc_irq_w));
	m_scc->out_txda_callback().set("modem", FUNC(rs232_port_device::write_txd));
	m_scc->out_txdb_callback().set("printer", FUNC(rs232_port_device::write_txd));
	// HSKo is driven from /DTR, not /RTS (the line driver is enabled by /RTS), and is connected to RS-232 DTR
	m_scc->out_dtra_callback().set("modem", FUNC(rs232_port_device::write_dtr));
	m_scc->out_dtrb_callback().set("printer", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232a(RS232_PORT(config, "modem", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	rs232a.dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	rs232a.cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w)).invert();

	rs232_port_device &rs232b(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	rs232b.dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	rs232b.cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w)).invert();
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
	m_ncr1(*this, "ncr53c96"),
	m_scc(*this, "scc"),
	m_rom(*this, finder_base::DUMMY_TAG),
	m_ata_irq(*this),
	m_overlay(false),
	m_scc_sync(true),
	m_ata_timing_clock(0),
	m_ata_read_wait(attotime::zero),
	m_ata_write_wait(attotime::zero)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void f108_device::device_start()
{
	m_rom_ptr = m_rom ? &m_rom[0] : nullptr;
	m_rom_size = m_rom ? m_rom.length() << 2 : 0;

	m_ata_active = 0;
	m_ata_recovery = 0;
	save_item(NAME(m_ata_active));
	save_item(NAME(m_ata_recovery));
	save_item(NAME(m_overlay));
	save_item(NAME(m_ata_ready));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void f108_device::device_reset()
{
	m_overlay = true;
	m_ata_ready = attotime::zero;
	update_overlay();
}

void f108_device::device_post_load()
{
	update_overlay();
}

void f108_device::update_overlay()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	if (!m_overlay)
	{
		space.install_ram(0x00000000, m_ram_size - 1, m_ram_ptr);
		return;
	}

	space.unmap_readwrite(0x00000000, std::max<u32>(m_ram_size, 0x400000) - 1);
	if (!m_rom_ptr)
		return;

	// put ROM mirror at 0
	const u32 memory_size = std::min<u32>(0x400000, m_rom_size);
	const u32 memory_end = memory_size - 1;
	offs_t memory_mirror = memory_end & ~(memory_size - 1);

	space.install_rom(0x00000000, memory_end & ~memory_mirror, memory_mirror, m_rom_ptr);
}

void f108_device::overlay_off()
{
	if (m_overlay && !machine().side_effects_disabled())
	{
		m_overlay = false;
		update_overlay();
	}
}

u32 f108_device::rom_switch_r(offs_t offset)
{
	// disable the overlay
	overlay_off();

	return m_rom_ptr ? m_rom_ptr[offset & ((m_rom_size - 1) >> 2)] : 0;
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

u8 f108_device::ata_timing_r(offs_t offset)
{
	switch (offset & ~2)
	{
		case 1:
			return 0x20;

		case 5:
			return m_ata_active;

		case 9:
			return m_ata_recovery;
	}

	return 0xff;
}

void f108_device::ata_timing_w(offs_t offset, u8 data)
{
	switch (offset & ~2)
	{
		case 5:
			m_ata_active = data & 0x3f;
			break;

		case 9:
			m_ata_recovery = data & 0x0f;
			break;
	}
}

void f108_device::ata_wait(const attotime &bus_wait)
{
	if (!m_ata_timing_clock || machine().side_effects_disabled())
		return;

	// The bus wait is added once per host access, before the first ATA cycle of the access
	// Active and recovery counts are in timing clock ticks, and active counts of 0 and 1 are 64 and 65 ticks
	// Bus and CPU work between cycles counts toward recovery
	// Recovery is at least 3 ticks, and a recovery count of 0 is 16 ticks
	m_maincpu->adjust_icount(-int(m_maincpu->attotime_to_cycles(bus_wait)));
	attotime now = m_maincpu->local_time();
	if (now < m_ata_ready)
	{
		m_maincpu->adjust_icount(-int(m_maincpu->attotime_to_cycles(m_ata_ready - now)));
		now = m_ata_ready;
	}
	const u8 active_ticks = (m_ata_active < 2) ? (m_ata_active + 64) : m_ata_active;
	const attotime active = attotime::from_ticks(active_ticks, m_ata_timing_clock);
	m_maincpu->adjust_icount(-int(m_maincpu->attotime_to_cycles(active)));
	const u8 recovery = m_ata_recovery ? std::max<u8>(m_ata_recovery, 3) : 16;
	m_ata_ready = now + active + attotime::from_ticks(recovery, m_ata_timing_clock);
}

u16 f108_device::ata_cs0_r(offs_t offset)
{
	ata_wait(m_ata_read_wait);
	return m_ata->cs0_swap_r(offset);
}

void f108_device::ata_cs0_w(offs_t offset, u16 data)
{
	ata_wait(m_ata_write_wait);
	m_ata->cs0_swap_w(offset, data);
}

u16 f108_device::ata_cs1_r(offs_t offset)
{
	ata_wait(m_ata_read_wait);
	return m_ata->cs1_swap_r(offset);
}

void f108_device::ata_cs1_w(offs_t offset, u16 data)
{
	ata_wait(m_ata_write_wait);
	m_ata->cs1_swap_w(offset, data);
}

u32 f108_device::ata_data_r(offs_t offset, u32 mem_mask)
{
	u32 retval = 0;

	if (mem_mask == 0xffffffff)
	{
		retval = u32(ata_cs0_r(0)) << 16;
		ata_wait(attotime::zero);
		retval |= m_ata->cs0_swap_r(0);
	}
	else if ((mem_mask & 0xffff0000) != 0)
	{
		retval = u32(ata_cs0_r(0)) << 16;
	}

	return retval;
}

void f108_device::ata_data_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (mem_mask == 0xffffffff)
	{
		ata_cs0_w(0, data >> 16);
		ata_wait(attotime::zero);
		m_ata->cs0_swap_w(0, data & 0xffff);
	}
	else if ((mem_mask & 0xffff0000) != 0)
	{
		ata_cs0_w(0, data >> 16);
	}
}

u16 f108_device::scc_r(offs_t offset)
{
	if (m_scc_sync && !machine().side_effects_disabled())
		m_primetimeii->via_sync();
	u16 result = m_scc->dc_ab_r(offset);
	return (result << 8) | result;
}

void f108_device::scc_w(offs_t offset, u16 data)
{
	if (m_scc_sync)
		m_primetimeii->via_sync();
	m_scc->dc_ab_w(offset, data >> 8);
}

void f108_device::scc_irq_w(int state)
{
	m_primetimeii->scc_irq_w(state);
}
