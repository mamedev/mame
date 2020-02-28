// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Novell Disk Coprocessor Board (DCB)

    This special SCSI host adapter, later acquired by ADIC, was designed
    for use with Novell NetWare.

***************************************************************************/

#include "emu.h"
#include "dcb.h"

DEFINE_DEVICE_TYPE(NOVELL_DCB, novell_dcb_device, "novell_dcb", "Novell Disk Coprocessor Board (#738-133-001)")

novell_dcb_device::novell_dcb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NOVELL_DCB, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_localcpu(*this, "localcpu")
	, m_eeprom(*this, "eeprom")
{
}

void novell_dcb_device::device_start()
{
}

void novell_dcb_device::eeprom_w(u8 data)
{
	m_eeprom->cs_write(BIT(data, 5));
	m_eeprom->clk_write(BIT(data, 6));
	m_eeprom->di_write(BIT(data, 7));
}

u8 novell_dcb_device::misc_r()
{
	return m_eeprom->do_read() << 7;
}

void novell_dcb_device::mem_map(address_map &map)
{
	map(0x00000, 0x03fff).ram();
	map(0xfe000, 0xfffff).rom().region("localcpu", 0);
}

void novell_dcb_device::io_map(address_map &map)
{
	map(0x0100, 0x0100).w(FUNC(novell_dcb_device::eeprom_w));
	map(0x0180, 0x0180).r(FUNC(novell_dcb_device::misc_r));
}

void novell_dcb_device::device_add_mconfig(machine_config &config)
{
	I80188(config, m_localcpu, 16_MHz_XTAL);
	m_localcpu->set_addrmap(AS_PROGRAM, &novell_dcb_device::mem_map);
	m_localcpu->set_addrmap(AS_IO, &novell_dcb_device::io_map);

	EEPROM_93C06_16BIT(config, m_eeprom); // NMC9306
}

ROM_START(novell_dcb)
	ROM_REGION(0x2000, "localcpu", 0)
	ROM_LOAD("817-186-001_rev_e_5800_11-04-86.bin", 0x0000, 0x2000, CRC(2e2037f4) SHA1(a13c0aab46084a0805256f1d2b8b8beaccc9e253))
ROM_END

const tiny_rom_entry *novell_dcb_device::device_rom_region() const
{
	return ROM_NAME(novell_dcb);
}
