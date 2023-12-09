// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Motorola MVME327A VMEbus To SCSI Bus Adapter
 *
 * Sources:
 *  - MVME327A VMEbus To SCSI Bus Adapter And MVME717 Transition Module User's Manual (MVME327A/D1)
 *
 * TODO:
 *  - skeleton only
 */

#include "emu.h"
#include "mvme327a.h"

#include "bus/nscsi/hd.h"
#include "bus/nscsi/cd.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VME_MVME327A, vme_mvme327a_device, "mvme327a", "Motorola MVME327A")

vme_mvme327a_device::vme_mvme327a_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VME_MVME327A, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_pit(*this, "pit")
	, m_bim(*this, "bim")
	, m_fdc(*this, "fdc")
	, m_scsi(*this, "scsi:7:wd33c93a")
	, m_boot(*this, "boot")
{
}

ROM_START(mvme327a)
	ROM_REGION16_BE(0x20000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "v27", "327A 2.7 4/17/95")
	ROMX_LOAD("5527c29a.u34", 0x0001, 0x10000, CRC(2ef4cc9a) SHA1(0e80e208756995cb34c006d2f1e84a5978ad17d6), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("5527c30a.u42", 0x0000, 0x10000, CRC(7be983d0) SHA1(ec5fc0709d8e91119d39231ace66f4fcbdb457e9), ROM_SKIP(1) | ROM_BIOS(0))
ROM_END

static INPUT_PORTS_START(mvme327a)
INPUT_PORTS_END

const tiny_rom_entry *vme_mvme327a_device::device_rom_region() const
{
	return ROM_NAME(mvme327a);
}

ioport_constructor vme_mvme327a_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mvme327a);
}

void vme_mvme327a_device::device_start()
{
}

void vme_mvme327a_device::device_reset()
{
	m_boot.select(0);
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("wd33c93a", WD33C93A);
}

void vme_mvme327a_device::device_add_mconfig(machine_config &config)
{
	M68010(config, m_cpu, 20_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &vme_mvme327a_device::cpu_mem);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &vme_mvme327a_device::cpu_int);

	MC68153(config, m_bim, 0);
	PIT68230(config, m_pit, 0);

	WD37C65C(config, m_fdc, 0);

	NSCSI_BUS(config, "scsi", 0);
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wd33c93a", WD33C93A).machine_config(
			[] (device_t *device)
			{
				wd33c9x_base_device &wd33c93(downcast<wd33c9x_base_device &>(*device));

				wd33c93.set_clock(10000000);
				//wd33c93.irq_cb().set(*this, ...);
				//wd33c93.drq_cb().set(*this, ...);
			});
}

void vme_mvme327a_device::cpu_mem(address_map &map)
{
	map(0x00'0000, 0x01'ffff).view(m_boot);
	m_boot[0](0x00'0000, 0x01'ffff).rom().region("eprom", 0);
	m_boot[1](0x00'0000, 0x01'ffff).ram();

	//map(0x04'0000, 0x04'0001).rw(m_scsi, FUNC(wd33c93a_device))
	//map(0x07'0000, 0x07'0003).m(m_fdc, &wd37c65c_device::map).umask16(0x00ff);
	map(0x0b'0000, 0x0b'003f).rw(m_pit, FUNC(pit68230_device::read), FUNC(pit68230_device::write)).mirror(0xffc0).umask16(0x00ff);
	map(0x0c'0000, 0x0c'000f).rw(m_bim, FUNC(bim68153_device::read), FUNC(bim68153_device::write)).mirror(0xfff0).umask16(0x00ff);

	map(0x10'0000, 0x11'ffff).rom().region("eprom", 0);
	map(0x10'0000, 0x10'0003).lw16([this](u32 data) { m_boot.select(1); }, "ram_enable");

	// 0x80'0000-0xff'ffff VMEbus access
}

void vme_mvme327a_device::cpu_int(address_map &map)
{
	map(0xfffff3, 0xfffff3).lr8(NAME([]() { return m68000_base_device::autovector(1); }));
	map(0xfffff5, 0xfffff5).lr8(NAME([]() { return m68000_base_device::autovector(2); }));
	map(0xfffff7, 0xfffff7).lr8(NAME([]() { return m68000_base_device::autovector(3); }));
	map(0xfffff9, 0xfffff9).lr8(NAME([]() { return m68000_base_device::autovector(4); }));
	map(0xfffffb, 0xfffffb).lr8(NAME([]() { return m68000_base_device::autovector(5); }));
	map(0xfffffd, 0xfffffd).lr8(NAME([]() { return m68000_base_device::autovector(6); }));
	map(0xffffff, 0xffffff).lr8(NAME([]() { return m68000_base_device::autovector(7); }));
}
