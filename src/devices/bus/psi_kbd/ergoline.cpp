// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Kontron Ergoline Keyboard

***************************************************************************/

#include "emu.h"
#include "ergoline.h"
#include "cpu/mcs51/mcs51.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ERGOLINE_KEYBOARD, ergoline_keyboard_device, "ergoline_kbd", "Ergoline Keyboard")


//-------------------------------------------------
//  address_map - device-specific address maps
//-------------------------------------------------

void ergoline_keyboard_device::kbd_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region("firmware", 0);
}

void ergoline_keyboard_device::kbd_io(address_map &map)
{
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( keyboard )
INPUT_PORTS_END

ioport_constructor ergoline_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( keyboard );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( kbd_pcb )
	ROM_REGION(0x1000, "firmware", 0)
	ROM_LOAD("mcg_21_1035.ic10", 0x0000, 0x1000, CRC(cde2417e) SHA1(8a2e1a894fda3e92fd760b8523121ba171281206))  // SUM16: 06f0 (ok)
ROM_END

const tiny_rom_entry *ergoline_keyboard_device::device_rom_region() const
{
	return ROM_NAME(kbd_pcb);
}

void ergoline_keyboard_device::device_add_mconfig(machine_config &config)
{
	i8031_device &maincpu(I8031(config, "maincpu", XTAL(5'529'600)));
	maincpu.set_addrmap(AS_PROGRAM, &ergoline_keyboard_device::kbd_mem);
	maincpu.set_addrmap(AS_IO, &ergoline_keyboard_device::kbd_io);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ergoline_keyboard_device - constructor
//-------------------------------------------------

ergoline_keyboard_device::ergoline_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ERGOLINE_KEYBOARD, tag, owner, clock),
	device_psi_keyboard_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ergoline_keyboard_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ergoline_keyboard_device::device_reset()
{
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

//-------------------------------------------------
//  tx_w - receive bit from host
//-------------------------------------------------

void ergoline_keyboard_device::tx_w(int state)
{
	logerror("tx_w: %d\n", state);
}
