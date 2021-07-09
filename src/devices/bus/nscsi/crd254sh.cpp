// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*******************************************************************************

    Skeleton device for Sony/Apple CRD-254SH 4x cdrom reader

*******************************************************************************/

// Chips:
//   - Hitachi H8/3040     : CPU, external rom mode
//   - Sanyo LC89512W      : CD-ROM error correction + SCSI
//   - Sanyo LC7868K       : DSP for CD players
//   - Sanyo LC98200       : ?
//   - Sanyo LA9220        : Analog signal processing and servo control
//   - Sanyo LC78816W      : ?
//   - Sanyo LA6531        : 2-Channel BTL-USE driver
//   - 2x NPN NN514256J-45 : 256Kx4 DRAM
//
//  The LC89512W combines a LC89517 (cdrom) upgraded for 4x and a LC8945 (scsi)
//  The LC89517 is a LC89515 upgraded for x2 capability
//  The LC89515 is an enhanced LC8951, and is documented


#include "emu.h"
#include "crd254sh.h"

DEFINE_DEVICE_TYPE(CRD254SH, crd254sh_device, "crd254sh", "Sanyo CRD-254SH CD-ROM")

crd254sh_device::crd254sh_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CRD254SH, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "dummy_scsi")
	, m_mcu(*this, "mcu")
	, m_scsi(*this, "dummy_scsi")
	, m_rom(*this, "mcu")
{
}

void crd254sh_device::device_start()
{
}

void crd254sh_device::mem_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("mcu", 0);
}

void crd254sh_device::io_map(address_map &map)
{
}

void crd254sh_device::device_add_mconfig(machine_config &config)
{
	H83040(config, m_mcu, 20_MHz_XTAL);
	m_mcu->set_addrmap(AS_PROGRAM, &crd254sh_device::mem_map);
	m_mcu->set_addrmap(AS_IO, &crd254sh_device::io_map);

	NCR53C94(config, m_scsi, 25_MHz_XTAL); // Placeholder until we implement the real chip
}

ROM_START(crd254sh)
	ROM_REGION(0x10000, "mcu", 0)
	ROM_LOAD("crd-254s_1.02.bin", 0x00000, 0x10000, CRC(539df5f0) SHA1(edb1f7d96f351ef70b8ade03449fb63e0e0a652b))
ROM_END

const tiny_rom_entry *crd254sh_device::device_rom_region() const
{
	return ROM_NAME(crd254sh);
}
