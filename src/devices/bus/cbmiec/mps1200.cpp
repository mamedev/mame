// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Commodore MPS-1200 & MPS-1250 printers (skeleton)

    The MPS-1200's CPU board was originally designed for a standard
    Centronics parallel interface (Y8300). However, an alternate
    "Basic Interface Pack" board (Y8306) instead supported the IEC bus
    using some extra LSTTL glue logic to convert serial data input to
    the parallel format read by the CPU. The later MPS-1250 board
    (Y8307) had hardware to support both serial and parallel
    interfaces, but only used one at a time.

**********************************************************************/

#include "emu.h"
#include "mps1200.h"

#include "cpu/m6502/m50734.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definitions
DEFINE_DEVICE_TYPE(MPS1200, mps1200_device, "mps1200", "Commodore MPS-1200 Dot Matrix Printer")
DEFINE_DEVICE_TYPE(MPS1250, mps1250_device, "mps1250", "Commodore MPS-1250 Dot Matrix Printer")


//-------------------------------------------------
//  mps1200_device - constructor
//-------------------------------------------------

mps1200_device::mps1200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cbm_iec_interface(mconfig, *this)
	, m_mpscpu(*this, "mpscpu")
{
}

mps1200_device::mps1200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mps1200_device(mconfig, MPS1200, tag, owner, clock)
{
}


//-------------------------------------------------
//  mps1250_device - constructor
//-------------------------------------------------

mps1250_device::mps1250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mps1200_device(mconfig, MPS1250, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mps1200_device::device_start()
{
}


//-------------------------------------------------
//  cbm_iec_atn - ATN line handler
//-------------------------------------------------

WRITE_LINE_MEMBER(mps1200_device::cbm_iec_atn)
{
	// TODO
}


//-------------------------------------------------
//  cbm_iec_data - serial data line handler
//-------------------------------------------------

WRITE_LINE_MEMBER(mps1200_device::cbm_iec_data)
{
	// TODO
}


//-------------------------------------------------
//  cbm_iec_reset - reset line handler
//-------------------------------------------------

WRITE_LINE_MEMBER(mps1200_device::cbm_iec_reset)
{
	// TODO
}


//-------------------------------------------------
//  mem_map - address map for main memory space
//-------------------------------------------------

void mps1200_device::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x6000).ram().share("ram"); // M5M5165P-12
	map(0x8000, 0xffff).rom().region("firmware", 0x8000);
}


//-------------------------------------------------
//  data_map - address map for data memory space
//-------------------------------------------------

void mps1200_device::data_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x6000).ram().share("ram");
	map(0x8000, 0xffff).rom().region("firmware", 0);
}


//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------

void mps1200_device::device_add_mconfig(machine_config &config)
{
	M50734(config, m_mpscpu, 8_MHz_XTAL);
	m_mpscpu->set_addrmap(AS_PROGRAM, &mps1200_device::mem_map);
	m_mpscpu->set_addrmap(AS_DATA, &mps1200_device::data_map);
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START(mps1200)
	PORT_START("SW")
	PORT_DIPNAME(0x0080, 0x0080, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x0080, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0040, 0x0040, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x0040, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0020, 0x0020, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x0020, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0010, 0x0010, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x0010, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0008, 0x0008, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x0008, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0004, 0x0004, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x0004, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0002, 0x0002, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(0x0002, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x0001, 0x0001, DEF_STR(Unknown)) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x0001, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x8000, 0x8000, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x8000, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x4000, 0x4000, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x4000, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x2000, 0x2000, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x2000, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_DIPNAME(0x1000, 0x1000, DEF_STR(Unknown)) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x1000, DEF_STR(Off))
	PORT_DIPSETTING(0x0000, DEF_STR(On))
	PORT_BIT(0x0f00, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END


//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------

ioport_constructor mps1200_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mps1200);
}

ioport_constructor mps1250_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(mps1200);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START(mps1200)
	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("mps1200-k405-0202.bin", 0x00000, 0x10000, CRC(87aa884a) SHA1(0ceb753c17599bc69458cfbb1cb3e81c2b60d107)) // "VER 1.01" "JUL-24-86" "Y8306 COMMODORE B.I.P."
ROM_END

ROM_START(mps1250)
	ROM_REGION(0x10000, "firmware", 0)
	ROM_LOAD("mps1250_k111_0201.bin", 0x00000, 0x10000, CRC(f2de9b69) SHA1(bb7357e83497b333e3f95548d94970003b2dfa9d)) // "VER 1.34" "MAR-03-87" "Y8307 COMMODORE DUAL  B.I.P."
ROM_END

//-------------------------------------------------
//  device_rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *mps1200_device::device_rom_region() const
{
	return ROM_NAME(mps1200);
}

const tiny_rom_entry *mps1250_device::device_rom_region() const
{
	return ROM_NAME(mps1250);
}
