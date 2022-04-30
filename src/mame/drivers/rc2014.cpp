// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic

#include "emu.h"
#include "bus/rc2014/rc2014.h"
#include "bus/rc2014/z80cpu.h"
#include "bus/rc2014/clock.h"
#include "bus/rc2014/ram.h"
#include "bus/rc2014/rom.h"
#include "bus/rc2014/serial.h"

namespace {

class rc2014_state : public driver_device
{
public:
	rc2014_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rc2014_bus(*this, "bus")
	{ }

	void rc2014(machine_config &config);
private:
	required_device<rc2014_bus_device> m_rc2014_bus;
};

static void rc2014_bus_devices(device_slot_interface &device)
{
	device.option_add("z80", RC2014_Z80CPU);
	device.option_add("clock", RC2014_SINGLE_CLOCK);
	device.option_add("ram32k", RC2014_RAM_32K);
	device.option_add("rom", RC2014_ROM);
	device.option_add("serial", RC2014_SERIAL_IO);
}

void rc2014_state::rc2014(machine_config &config)
{
	RC2014_BUS(config, m_rc2014_bus, 0);
	RC2014_SLOT(config, "bus:1", m_rc2014_bus, rc2014_bus_devices, "z80");
	RC2014_SLOT(config, "bus:2", m_rc2014_bus, rc2014_bus_devices, "clock");
	RC2014_SLOT(config, "bus:3", m_rc2014_bus, rc2014_bus_devices, "ram32k");
	RC2014_SLOT(config, "bus:4", m_rc2014_bus, rc2014_bus_devices, "rom");
	RC2014_SLOT(config, "bus:5", m_rc2014_bus, rc2014_bus_devices, "serial");
}

ROM_START(rc2014)
ROM_END

} // anonymous namespace

// This ties everything together
//    YEAR  NAME        PARENT    COMPAT    MACHINE    INPUT    CLASS           INIT           COMPANY           FULLNAME           FLAGS
COMP( 2016, rc2014,     0,        0,        rc2014,    0,       rc2014_state,   empty_init,    "RFC2795 Ltd",    "RC2014",          MACHINE_NO_SOUND_HW )
