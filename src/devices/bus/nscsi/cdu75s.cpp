// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*******************************************************************************

    Skeleton device for Sony/Apple CDU75S 2x cdrom reader

*******************************************************************************/

// Chips:
// - IC101: CXA1841Q       - RF signal analog amplifier
// - IC102: CXA1182Q-Z     - servo control
// - IC103: CXD2510Q       - CD DSP
// - IC105: BU4053BCF      - analog mux/demux
// - IC201: H8/3032        - main mcu, 64k internal rom
// - IC202: CXD1808AQ      - cdrom decoder
// - IC203: LH62800K-60    - DRAM 256k*8
// - IC204: FAS204 2405027 - Fast Architecture SCSI 204, ncr 53c94 compatible
// - IC205: CXD8532Q       - ? probable logic/processing, suspect it handles the jumpers, some dma, that kind of stuff
// - IC301: pcm-1715u      - DAC
// - IC401: BA6295AFP      - 2 channel motor driver

#include "emu.h"
#include "cdu75s.h"

DEFINE_DEVICE_TYPE(CDU75S, cdu75s_device, "cdu75s", "Sony/Apple CDU75S CD-R")

cdu75s_device::cdu75s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CDU75S, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "fas204")
	, m_mcu(*this, "mcu")
	, m_scsi(*this, "fas204")
	, m_rom(*this, "mcu")
{
}

void cdu75s_device::device_start()
{
	// We're getting interesting results with a basic vectors map,
	// e.g. vector<n> points to 0x10000+4*n, where there's a jmp to
	// the final destination

	// Actually used vectors:
	//  0 00 reset
	//  7 1c nmi
	//  9 24 ?
	// 11 2c ?
	// 12 30 irq0
	// 15 3c irq3
	// 18 48 reserved?
	// 21 54 reserved?
	// 25 64 imib0
	// 26 68 ovi0

	// kinda means it's not just 1:1

	for(u32 i = 0; i != 64; i++) {
		m_rom[i*2] = 1;
		m_rom[i*2+1] = i*4;
	}
}

void cdu75s_device::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("mcu", 0);
	map(0x40000, 0x4000f).m(m_scsi, FUNC(ncr53c94_device::map));

	map(0x60000, 0x60007); // CXD8532Q
	map(0x60005, 0x60005).r(FUNC(cdu75s_device::jumpers_r));

	map(0xc0000, 0xc003f); // CXD1808AQ
}

void cdu75s_device::io_map(address_map &map)
{
}

void cdu75s_device::device_add_mconfig(machine_config &config)
{
	H83032(config, m_mcu, 33.8688_MHz_XTAL/2);
	m_mcu->set_addrmap(AS_PROGRAM, &cdu75s_device::mem_map);
	m_mcu->set_addrmap(AS_IO, &cdu75s_device::io_map);

	NCR53C94(config, m_scsi, 25'000'000); // FAS204, compatible
}

ROM_START(cdu75s)
	ROM_REGION(0x20000, "mcu", 0)
	ROM_FILL(0x00000, 0x10000, 0x00) // Internal rom not yet dumped
	ROM_LOAD("cdu-75s_1.0j_95.03.14_apple.bin", 0x10000, 0x10000, CRC(f4ad4d48) SHA1(7d674116304bc6948fe4a52d9859b4eb5d40b914))
ROM_END

const tiny_rom_entry *cdu75s_device::device_rom_region() const
{
	return ROM_NAME(cdu75s);
}

u8 cdu75s_device::jumpers_r()
{
	u32 jumpers_id = strtol(owner()->basetag(), nullptr, 16);
	return (jumpers_id ^ 7) << 1;
}

