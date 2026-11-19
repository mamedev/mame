// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Skeleton device for NEC CDR-52 CD-Rom drive

"NEC MAIN PWD PWA-331 -A / 72933312 NEC-14T"
- IC108 surface scratched CPU, code looks 78k of some kind, near unreadable Xi03 xtal (?)
- IC102 Sony CXD2500AQ DSP, near unreadable X101 xtal
- IC103 Sony CXA1372Q (C is surface scratched in available picture)
- IC111 NEC D6378GF, surface scratched on another picture
- IC110 / IC112 Hyundai HY6264ALJ-10
- IC113 NHE Japan SLA7220F
- IC101 (unreadable, CXA1185?)
- 4-bit DIP bank on top-right, near three audio connectors labeled VR102, VR103, VR104, a 8 pin
  CN102 connector and the SCSI connector;
- A two wire CNI04, near IC109 ROM socket at bottom-left

SCSI1, requires NECCDA.SYS in PC-98 land. Built-in option for PC-98GS/PC-9821/Ce/Ap2/An/XaC10W

**************************************************************************************************/

#include "emu.h"
#include "cdr52.h"

DEFINE_DEVICE_TYPE(NSCSI_CDR52, nscsi_cdr52_device, "cdr52", "NEC CDR-52 CD-R")

nscsi_cdr52_device::nscsi_cdr52_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NSCSI_CDR52, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "scsic")
	, m_mcu(*this, "mcu")
	, m_scsic(*this, "scsic")
	, m_rom(*this, "rom")
{
}

void nscsi_cdr52_device::device_start()
{
}

void nscsi_cdr52_device::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("rom", 0);
	map(0x9000, 0x900f).m("scsic", FUNC(cxd1185_device::map));
	map(0xc000, 0xffff).ram();
}

void nscsi_cdr52_device::device_add_mconfig(machine_config &config)
{
	// TODO: unknown clock, unknown exact flavour (surface scratched)
	UPD78C10(config, m_mcu, 24_MHz_XTAL / 2);
	m_mcu->set_addrmap(AS_PROGRAM, &nscsi_cdr52_device::mem_map);

	// TODO: guess, confirm me
	CXD1185(config, "scsic", 24_MHz_XTAL / 2);
}

ROM_START(cdr52)
	ROM_REGION(0x8000, "rom", 0)
	ROM_LOAD("nec_cdr_52.ic109", 0x0000, 0x8000, CRC(fe7a4358) SHA1(25764425c203074c8bbb9b38ad95201f85ffed04))
ROM_END

const tiny_rom_entry *nscsi_cdr52_device::device_rom_region() const
{
	return ROM_NAME(cdr52);
}

