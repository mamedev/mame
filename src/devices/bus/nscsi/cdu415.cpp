// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*******************************************************************************

    Skeleton device for Sony CDU415 cdrom reader

*******************************************************************************/

// Chips:
// - IC201: H8/3032        - main mcu, 64k internal rom
// - IC202: CXD1804AR      - cdrom decoder and scsi interface
// - IC203: V53C16126HK40  - DRAM 128k*16
// - IC205: BU6243K        - ?
// - IC207: AM28F512       - Firmware

#include "emu.h"
#include "cdu415.h"

DEFINE_DEVICE_TYPE(CDU415, cdu415_device, "cdu415", "Sony CDU415 CD-R")

cdu415_device::cdu415_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CDU415, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "cxd1804ar")
	, m_mcu(*this, "mcu")
	, m_scsi(*this, "cxd1804ar")
	, m_rom(*this, "mcu")
{
}

void cdu415_device::device_start()
{
	// The reset vector is the only obvious one there.
	m_rom[0] = m_rom[0x8000];
	m_rom[1] = m_rom[0x8001];
}

void cdu415_device::mem_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("mcu", 0);
	map(0x20000, 0x2ffff).rom().region("mcu", 0x10000);
}

void cdu415_device::io_map(address_map &map)
{
}

void cdu415_device::device_add_mconfig(machine_config &config)
{
	H83032(config, m_mcu, 10000000);
	m_mcu->set_addrmap(AS_PROGRAM, &cdu415_device::mem_map);
	m_mcu->set_addrmap(AS_IO, &cdu415_device::io_map);

	NCR53C94(config, m_scsi, 25'000'000); // Temporary placeholder
}

ROM_START(cdu415)
	ROM_REGION(0x20000, "mcu", 0)
	ROM_FILL(0x00000, 0x10000, 0x00) // Internal rom not yet dumped
	ROM_LOAD("cdu415.bin", 0x10000, 0x10000, CRC(9873b898) SHA1(72d0b192efc32f7c8985dd96df3e346c57a236e7))
ROM_END

const tiny_rom_entry *cdu415_device::device_rom_region() const
{
	return ROM_NAME(cdu415);
}

u8 cdu415_device::jumpers_r()
{
	u32 jumpers_id = strtol(owner()->basetag(), nullptr, 16);
	return (jumpers_id ^ 7) << 1;
}

