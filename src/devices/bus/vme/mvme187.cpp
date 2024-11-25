// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Motorola MVME187
 *
 * Sources:
 *  - MVME187 RISC Single Board Computer Installation Guide (MVME187IG/D4), Motorola 1997
 *  - Single Board Computers Programmer's Reference Guide (VMESBCA1/PG1), Motorola 1996
 *
 * TODO:
 *  - skeleton only
 *
 * Skeleton is based on the variant of the MVME187 used in the Motorola M8120 system.
 *
 * M8120 variant
 *  - no SRAM
 *  - two additional serial ports (total 6)
 *  - no parallel port
 *  - only two ROM sockets
 */

/*
 * M8120 parts
 * -----------
 * MC88100RC25
 * MC88200RC25 x 2
 * DS1643-150               8Kx8 NVRAM with RTC (MK48T08 compatible)
 * 53C710                   SCSI controller
 * TL-SCSI285N              SCSI voltage regulator
 * DS1232S                  MicroMonitor chip
 * KU82596CA-33             Ethernet controller
 * D82C501AD                Ethernet serial interface
 * CL-CD2401-10QC-I x 2     Four-channel multi-protocol communications controller
 * MC145406DW x 9           RS-232-D driver/receiver
 *
 * 51-W6281B01 1444F0011    Motorola custom ASIC (PCCChip2 or VMEChip2?)
 * 51-W6280B02 16H7Y0005    Motorola custom ASIC (PCCChip2 or VMEChip2?)
 * M5M27C202JK x 2          131072 word x 16 bit CMOS EEPROM
 *
 * DS1                      run LED (green)
 * DS2                      stat LED (yellow)
 * DS3                      scsi LED (yellow)
 * DS4                      fail LED (red)
 * abort switch
 * reset switch
 *
 * 50.000MHz
 * 20.0000MHz
 *
 * RAM mezzanine board has MEMC040 or MCECC memory controller ASIC
 */

#include "emu.h"
#include "mvme187.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(VME_MVME187, vme_mvme187_card_device, "mvme187", "Motorola MVME187")

vme_mvme187_card_device::vme_mvme187_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, VME_MVME187, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_mmu(*this, "mmu%u", 0U)
	, m_rtc(*this, "rtc")
	//, m_uart(*this, "uart")
	, m_serial(*this, "serial%u", 0U)
	//, m_scsi(*this, "scsi")
	, m_lan(*this, "lan")
	, m_boot(*this, "boot")
{
}

ROM_START(mvme187)
	ROM_REGION32_BE(0x8'0000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "m8120", "M8120 Debugger/Diagnostics Release Version 1.2 - 04/14/93")
	ROMX_LOAD("6631b05c__03_08_94__8120.u1", 0x0002, 0x4'0000, CRC(d31cc388) SHA1(c2e613ded8d66102d3afa41ca763e997d9829d28), ROM_REVERSE | ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0)) // M5M27C202JK
	ROMX_LOAD("6631b06c__03_08_94__8120.u4", 0x0000, 0x4'0000, CRC(c62923e9) SHA1(62515c6e0dbeca6d18f373ad33e185096f10d2fc), ROM_REVERSE | ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(0)) // M5M27C202JK
ROM_END

static INPUT_PORTS_START(mvme187)
INPUT_PORTS_END

const tiny_rom_entry *vme_mvme187_card_device::device_rom_region() const
{
	return ROM_NAME(mvme187);
}

ioport_constructor vme_mvme187_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mvme187);
}

void vme_mvme187_card_device::device_start()
{
}

void vme_mvme187_card_device::device_reset()
{
	m_boot.select(0);
}

void vme_mvme187_card_device::device_add_mconfig(machine_config &config)
{
	MC88100(config, m_cpu, 50_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &vme_mvme187_card_device::cpu_mem);
	m_cpu->set_cmmu_code([this](u32 const address) -> mc88200_device & { return *m_mmu[0]; });
	m_cpu->set_cmmu_data([this](u32 const address) -> mc88200_device & { return *m_mmu[1]; });

	MC88200(config, m_mmu[0], 50_MHz_XTAL / 2, 0x77).set_mbus(m_cpu, AS_PROGRAM);
	MC88200(config, m_mmu[1], 50_MHz_XTAL / 2, 0x7f).set_mbus(m_cpu, AS_PROGRAM);

	DS1643(config, m_rtc);

	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[2], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[3], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[4], default_rs232_devices, nullptr);
	RS232_PORT(config, m_serial[5], default_rs232_devices, nullptr);

	//NCR53C7XX(config, m_scsi, 0);
	I82596_BE32(config, m_lan, 20'000'000);
}

void vme_mvme187_card_device::cpu_mem(address_map &map)
{
	map(0x0000'0000, 0x00ff'ffff).view(m_boot);
	m_boot[0](0x0000'0000, 0x0007'ffff).rom().region("eprom", 0);
	m_boot[1](0x0000'0000, 0x00ff'ffff).ram();

	map(0xff80'0000, 0xff87'ffff).rom().region("eprom", 0);
}
