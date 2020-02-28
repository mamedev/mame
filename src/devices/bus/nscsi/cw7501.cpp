// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton device for Panasonic CW-7501 CD-R drive and clones.

*******************************************************************************/

#include "emu.h"
#include "bus/nscsi/cw7501.h"
#include "machine/ncr5390.h"

DEFINE_DEVICE_TYPE(CW7501, cw7501_device, "cw7501", "Panasonic CW-7501 CD-R")
DEFINE_DEVICE_TYPE(CDR4210, cdr4210_device, "cdr4210", "Creative Technology Blaster CD-R 4210")

cw7501_device::cw7501_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, "scsic")
	, m_cdcpu(*this, "cdcpu")
{
}

cw7501_device::cw7501_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cw7501_device(mconfig, CW7501, tag, owner, clock)
{
}

cdr4210_device::cdr4210_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: cw7501_device(mconfig, CDR4210, tag, owner, clock)
{
}

void cw7501_device::device_start()
{
	m_mystery_address = 0;

	save_item(NAME(m_mystery_address));
}

u8 cw7501_device::mystery_data_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: Reading from mystery register #%02X\n", machine().describe_context(), m_mystery_address);

	return 0;
}

void cw7501_device::mystery_data_w(u8 data)
{
	logerror("%s: Writing %02X to mystery register #%02X\n", machine().describe_context(), data, m_mystery_address);
}

void cw7501_device::mystery_address_w(u8 data)
{
	m_mystery_address = data;
}

void cw7501_device::mem_map(address_map &map)
{
	map(0x000880, 0x007fff).ram();
	map(0x008000, 0x03ffff).rom().region("flash", 0x08000);
	map(0x050000, 0x050000).w(FUNC(cw7501_device::mystery_address_w));
	map(0x050001, 0x050001).rw(FUNC(cw7501_device::mystery_data_r), FUNC(cw7501_device::mystery_data_w));
	map(0x058000, 0x05800f).m("scsic", FUNC(ncr53cf94_device::map));
	map(0x060000, 0x060003).noprw(); // ?
}

void cw7501_device::device_add_mconfig(machine_config &config)
{
	M37710S4(config, m_cdcpu, 12'500'000); // type and clock are total guesses
	m_cdcpu->set_addrmap(AS_PROGRAM, &cw7501_device::mem_map);

	NCR53CF94(config, "scsic", 25'000'000); // type and clock guessed
}

ROM_START(cw7501)
	ROM_REGION16_LE(0x40000, "flash", 0)
	ROM_LOAD("mk200.bin", 0x00000, 0x40000, CRC(12efd802) SHA1(2986ee5eedbe0cb662a9a7e7fa4e6ca7ccd8c539)) // v2.00 (1996)
ROM_END

const tiny_rom_entry *cw7501_device::device_rom_region() const
{
	return ROM_NAME(cw7501);
}

ROM_START(cdr4210)
	ROM_REGION16_LE(0x40000, "flash", 0)
	ROM_LOAD("cr113.bin", 0x00000, 0x40000, CRC(fd2faff9) SHA1(6aafdedf12240ad347427287c0db289f90bd064d)) // v1.13 (1996)
ROM_END

const tiny_rom_entry *cdr4210_device::device_rom_region() const
{
	return ROM_NAME(cdr4210);
}

// another clone: Plasmon CDR 4240
