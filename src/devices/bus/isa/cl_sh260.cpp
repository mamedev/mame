// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    ISA 16-bit disk controllers based on the Cirrus Logic CL-SH260-15PC-D
    * Everex EV-346
    * Joincom Electronic JC-1310

***************************************************************************/

#include "emu.h"
#include "cl_sh260.h"

#include "cpu/mcs51/mcs51.h"

DEFINE_DEVICE_TYPE(EV346, isa16_ev346_device, "ev346", "Everex EV-346 disk controller")
DEFINE_DEVICE_TYPE(JC1310, isa16_jc1310_device, "jc1310", "Joincom JC-1310 disk controller")

isa16_cl_sh260_device::isa16_cl_sh260_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
{
}

isa16_ev346_device::isa16_ev346_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: isa16_cl_sh260_device(mconfig, EV346, tag, owner, clock)
{
}

isa16_jc1310_device::isa16_jc1310_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: isa16_cl_sh260_device(mconfig, JC1310, tag, owner, clock)
{
}

void isa16_cl_sh260_device::device_start()
{
}

void isa16_cl_sh260_device::i8031_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("mcu", 0);
}

void isa16_ev346_device::ext_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).noprw(); // CL-SH260 interface
}

void isa16_jc1310_device::ext_map(address_map &map)
{
	map(0xc000, 0xc0ff).noprw(); // CL-SH260 interface
}

void isa16_ev346_device::device_add_mconfig(machine_config &config)
{
	i8031_device &mcu(I8031(config, "mcu", 12_MHz_XTAL)); // SCN8031HCCN40
	mcu.set_addrmap(AS_PROGRAM, &isa16_ev346_device::i8031_map);
	mcu.set_addrmap(AS_IO, &isa16_ev346_device::ext_map);

	DP8473(config, m_fdc, 24_MHz_XTAL);
}

void isa16_jc1310_device::device_add_mconfig(machine_config &config)
{
	i8031_device &mcu(I8031(config, "mcu", 10_MHz_XTAL)); // P8031AH
	mcu.set_addrmap(AS_PROGRAM, &isa16_jc1310_device::i8031_map);
	mcu.set_addrmap(AS_IO, &isa16_jc1310_device::ext_map);

	WD37C65C(config, m_fdc, 16_MHz_XTAL, 9.6_MHz_XTAL); // WD37C65B-PL
}

ROM_START(ev346)
	ROM_REGION(0x2000, "mcu", 0)
	ROM_LOAD("everex_ev346_vers_3.2.bin", 0x0000, 0x2000, CRC(49b10ca7) SHA1(ef31b62f5ac38db7cacfc4d30e203a4cc1414913))
ROM_END

ROM_START(jc1310)
	ROM_REGION(0x2000, "mcu", 0) // "(C) COPYRIGHT JOINCOM ELECTRONIC CORP. 9/16/1988 JC1310"
	ROM_LOAD("jc-1310 - apr-c 89.u3", 0x0000, 0x2000, CRC(e044f5e1) SHA1(5a32d2001bb1a489657f9488136b5d621f803703)) // TMS 2764-20JL
ROM_END

const tiny_rom_entry *isa16_ev346_device::device_rom_region() const
{
	return ROM_NAME(ev346);
}

const tiny_rom_entry *isa16_jc1310_device::device_rom_region() const
{
	return ROM_NAME(jc1310);
}
