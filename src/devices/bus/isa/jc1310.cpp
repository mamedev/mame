// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Joincom Electronic JC-1310 MFM floppy disk controller

***************************************************************************/

#include "emu.h"
#include "jc1310.h"

#include "cpu/mcs51/mcs51.h"

DEFINE_DEVICE_TYPE(JC1310, isa16_jc1310_device, "jc1310", "Joincom JC-1310 FDC")

isa16_jc1310_device::isa16_jc1310_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, JC1310, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
{
}

void isa16_jc1310_device::device_start()
{
}

void isa16_jc1310_device::prog_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("mcu", 0);
}

void isa16_jc1310_device::ext_map(address_map &map)
{
	map(0xc000, 0xc7ff).ram(); // CXK5816PN-10L
}

void isa16_jc1310_device::device_add_mconfig(machine_config &config)
{
	i8031_device &mcu(I8031(config, "mcu", 10_MHz_XTAL)); // P8031AH
	mcu.set_addrmap(AS_PROGRAM, &isa16_jc1310_device::prog_map);
	mcu.set_addrmap(AS_IO, &isa16_jc1310_device::ext_map);

	WD37C65C(config, m_fdc, 16_MHz_XTAL); // WD37C65B-PL
	//m_fdc->set_clock2(9.6_MHz_XTAL);
}

ROM_START(jc1310)
	ROM_REGION(0x2000, "mcu", 0) // "(C) COPYRIGHT JOINCOM ELECTRONIC CORP. 9/16/1988 JC1310"
	ROM_LOAD("jc-1310 - apr-c 89.u3", 0x0000, 0x2000, CRC(e044f5e1) SHA1(5a32d2001bb1a489657f9488136b5d621f803703)) // TMS 2764-20JL
ROM_END

const tiny_rom_entry *isa16_jc1310_device::device_rom_region() const
{
	return ROM_NAME(jc1310);
}
