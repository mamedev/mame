// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

	hp9895.cpp

	HP9895 floppy disk drive

*********************************************************************/

#include "hp9895.h"

// device type definition
const device_type HP9895 = &device_creator<hp9895_device>;

hp9895_device::hp9895_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP9895, "HP9895", tag, owner, clock, "HP9895", __FILE__),
	  device_ieee488_interface(mconfig, *this),
	  m_cpu(*this , "cpu"),
	  m_phi(*this , "phi")
{
}

#if 0
ioport_constructor hp9895_device::device_input_ports() const
{
}
#endif
void hp9895_device::device_start()
{
}

void hp9895_device::device_reset()
{
}

void hp9895_device::ieee488_eoi(int state)
{
}

void hp9895_device::ieee488_dav(int state)
{
}

void hp9895_device::ieee488_nrfd(int state)
{
}

void hp9895_device::ieee488_ndac(int state)
{
}

void hp9895_device::ieee488_ifc(int state)
{
}

void hp9895_device::ieee488_srq(int state)
{
}

void hp9895_device::ieee488_atn(int state)
{
}

void hp9895_device::ieee488_ren(int state)
{
}

ROM_START(hp9895)
	ROM_REGION(0x2000 , "cpu" , 0)
	ROM_LOAD("1818-1391a.bin" , 0 , 0x2000 , CRC(b50dbfb5) SHA1(96edf9af78be75fbad2a0245b8af43958ba32752))
ROM_END

static ADDRESS_MAP_START(z80_program_map , AS_PROGRAM , 8 , hp9895_device)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000 , 0x1fff) AM_ROM AM_REGION("cpu" , 0)
	AM_RANGE(0x6000 , 0x63ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(z80_io_map , AS_IO , 8 , hp9895_device)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// TODO: TEMP!
	AM_RANGE(0x10 , 0x17) AM_DEVREADWRITE("phi" , phi_device , reg8_r , reg8_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT(hp9895)
	MCFG_CPU_ADD("cpu" , Z80 , 4000000)
	MCFG_CPU_PROGRAM_MAP(z80_program_map)
	MCFG_CPU_IO_MAP(z80_io_map)

	MCFG_DEVICE_ADD("phi" , PHI , 0)
MACHINE_CONFIG_END

const tiny_rom_entry *hp9895_device::device_rom_region() const
{
	return ROM_NAME(hp9895);
}

machine_config_constructor hp9895_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(hp9895);
}
