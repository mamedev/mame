// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

	Amiga 1200 Keyboard

    Skeleton device, needs MC68HC05Cx device support

    391508-01 = Rev 0 is MC68HC05C4AFN
    391508-02 = Rev 1 is MC68HC05C12FN

***************************************************************************/

#include "a1200.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type A1200_KBD = &device_creator<a1200_kbd_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static ADDRESS_MAP_START( mc68hc05c12_map, AS_PROGRAM, 8, a1200_kbd_device )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("mpu", 0)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( a1200kbd_rev1 )
	// should be MC68HC05C12
	MCFG_CPU_ADD("mpu", M6805, XTAL_3MHz)
	MCFG_CPU_PROGRAM_MAP(mc68hc05c12_map)
MACHINE_CONFIG_END

machine_config_constructor a1200_kbd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( a1200kbd_rev1 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( a1200kbd_rev1 )
	ROM_REGION(0x2000, "mpu", 0)
	// should be 0x3000, i assume it was hacked to allow it to be written to a 68hc705c8
	// it was labeled "rev b", so it's probably the rev 1 (a rev 0 is in our queue to be decapped)
	ROM_LOAD("391508-02.bin", 0x0000, 0x2000, BAD_DUMP CRC(2a77eec4) SHA1(301ec6a69404457d912c89e3fc54095eda9f0e93))
ROM_END

const tiny_rom_entry *a1200_kbd_device::device_rom_region() const
{
	return ROM_NAME( a1200kbd_rev1 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( a1200_us_keyboard )
INPUT_PORTS_END

ioport_constructor a1200_kbd_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( a1200_us_keyboard );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  a1200_kbd_device - constructor
//-------------------------------------------------

a1200_kbd_device::a1200_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, A1200_KBD, "Amiga 1200 Keyboard Rev 1", tag, owner, clock, "a1200kbd_r1", __FILE__),
	device_amiga_keyboard_interface(mconfig, *this),
	m_mpu(*this, "mpu")
{}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a1200_kbd_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void a1200_kbd_device::device_reset()
{
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

WRITE_LINE_MEMBER( a1200_kbd_device::kdat_w )
{
}
