// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton device for Sanyo/Caravelle CDR-N820s.

*******************************************************************************/

#include "emu.h"
#include "bus/nscsi/cdrn820s.h"
#include "machine/wd33c9x.h"

DEFINE_DEVICE_TYPE(CDRN820S, cdrn820s_device, "cdrn820s", "Caravelle CDR-N820s")

cdrn820s_device::cdrn820s_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "scsic")
	, m_h8(*this, "h8")
{
}

cdrn820s_device::cdrn820s_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cdrn820s_device(mconfig, CDRN820S, tag, owner, clock)
{
}

void cdrn820s_device::device_start()
{
}

void cdrn820s_device::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("program", 0);
}

void cdrn820s_device::device_add_mconfig(machine_config &config)
{
	H83048(config, m_h8, 8'000'000); // type and clock unknown
	m_h8->set_addrmap(AS_PROGRAM, &cdrn820s_device::mem_map);
	m_h8->set_mode_a20();

	WD33C93A(config, "scsic", 10'000'000); // type and clock unknown
}

ROM_START(cdrn820s)
	ROM_REGION16_BE(0x40000, "program", 0)
	ROM_LOAD("cdr_120.bin", 0x00000, 0x20000, CRC(8cac6862) SHA1(e498dcd9006d257ced6cd0b50c76608e9a8023f7)) // Caravelle CDR-N820S 1.20
ROM_END

const tiny_rom_entry *cdrn820s_device::device_rom_region() const
{
	return ROM_NAME(cdrn820s);
}
