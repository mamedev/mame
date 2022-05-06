// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic

#include "emu.h"
#include "bus/rc2014/rc2014.h"
#include "bus/rc2014/z80cpu.h"
#include "bus/rc2014/clock.h"
#include "bus/rc2014/ram.h"
#include "bus/rc2014/rom.h"
#include "bus/rc2014/serial.h"
#include "bus/rc2014/cf.h"

namespace {

static void rc2014_bus_devices(device_slot_interface &device)
{
	device.option_add("z80", RC2014_Z80CPU);
	device.option_add("clock", RC2014_SINGLE_CLOCK);
	device.option_add("ram32k", RC2014_RAM_32K);
	device.option_add("sw_rom", RC2014_SWITCH_ROM);
	device.option_add("serial", RC2014_SERIAL_IO);
	device.option_add("cf", RC2014_COMPACT_FLASH);

	device.option_add("z80_21", RC2014_Z80CPU_21);
	device.option_add("dual_clk", RC2014_DUAL_CLOCK);
	device.option_add("sio", RC2014_DUAL_SERIAL);
	device.option_add("page_rom", RC2014_PAGABLE_ROM);
}

class rc2014_state : public driver_device
{
public:
	rc2014_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rc2014_bus(*this, "bus")
	{ }

	void rc2014(machine_config &config)
	{
		RC2014_BUS(config, m_rc2014_bus, 0);
		RC2014_SLOT(config, "bus:1", m_rc2014_bus, rc2014_bus_devices, "z80");
		RC2014_SLOT(config, "bus:2", m_rc2014_bus, rc2014_bus_devices, "clock");
		RC2014_SLOT(config, "bus:3", m_rc2014_bus, rc2014_bus_devices, "ram32k");
		RC2014_SLOT(config, "bus:4", m_rc2014_bus, rc2014_bus_devices, "sw_rom");
		RC2014_SLOT(config, "bus:5", m_rc2014_bus, rc2014_bus_devices, "serial");
	}

	void rc2014cl2(machine_config &config)
	{
		RC2014_BUS(config, m_rc2014_bus, 0);
		RC2014_SLOT(config, "bus:1", m_rc2014_bus, rc2014_bus_devices, "z80");
		RC2014_SLOT(config, "bus:2", m_rc2014_bus, rc2014_bus_devices, "clock");
		RC2014_SLOT(config, "bus:3", m_rc2014_bus, rc2014_bus_devices, "ram32k");
		RC2014_SLOT(config, "bus:4", m_rc2014_bus, rc2014_bus_devices, "sw_rom");
		RC2014_SLOT(config, "bus:5", m_rc2014_bus, rc2014_bus_devices, "serial");
		RC2014_SLOT(config, "bus:6", m_rc2014_bus, rc2014_bus_devices, nullptr);
		RC2014_SLOT(config, "bus:7", m_rc2014_bus, rc2014_bus_devices, nullptr);
		RC2014_SLOT(config, "bus:8", m_rc2014_bus, rc2014_bus_devices, nullptr);
	}

private:
	required_device<rc2014_bus_device> m_rc2014_bus;
};

ROM_START(rc2014)
ROM_END
ROM_START(rc2014cl2)
ROM_END

class rc2014pro_state : public driver_device
{
public:
	rc2014pro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rc2014_bus(*this, "bus")
	{ }

	void rc2014pro(machine_config &config)
	{
		RC2014_EXT_BUS(config, m_rc2014_bus, 0);
		RC2014_EXT_SLOT(config, "bus:1", m_rc2014_bus, rc2014_bus_devices, "z80_21");
		RC2014_EXT_SLOT(config, "bus:2", m_rc2014_bus, rc2014_bus_devices, "dual_clk");
		RC2014_EXT_SLOT(config, "bus:3", m_rc2014_bus, rc2014_bus_devices, "ram32k");
		RC2014_EXT_SLOT(config, "bus:4", m_rc2014_bus, rc2014_bus_devices, "page_rom");
		RC2014_EXT_SLOT(config, "bus:5", m_rc2014_bus, rc2014_bus_devices, "sio");
		RC2014_EXT_SLOT(config, "bus:6", m_rc2014_bus, rc2014_bus_devices, "cf");
		RC2014_EXT_SLOT(config, "bus:7", m_rc2014_bus, rc2014_bus_devices, nullptr);
		RC2014_EXT_SLOT(config, "bus:8", m_rc2014_bus, rc2014_bus_devices, nullptr);
		RC2014_EXT_SLOT(config, "bus:9", m_rc2014_bus, rc2014_bus_devices, nullptr);
		RC2014_EXT_SLOT(config, "bus:10", m_rc2014_bus, rc2014_bus_devices, nullptr);
		RC2014_EXT_SLOT(config, "bus:11", m_rc2014_bus, rc2014_bus_devices, nullptr);
		RC2014_EXT_SLOT(config, "bus:12", m_rc2014_bus, rc2014_bus_devices, nullptr);
	}
private:
	required_device<rc2014_ext_bus_device> m_rc2014_bus;
};

ROM_START(rc2014pro)
ROM_END

} // anonymous namespace

// This ties everything together
//    YEAR  NAME        PARENT    COMPAT    MACHINE    INPUT    CLASS            INIT           COMPANY           FULLNAME             FLAGS
COMP( 2016, rc2014,     0,        0,        rc2014,    0,       rc2014_state,    empty_init,    "RFC2795 Ltd",    "RC2014 Classic",    MACHINE_IS_SKELETON )
COMP( 2016, rc2014pro,  rc2014,   0,        rc2014pro, 0,       rc2014pro_state, empty_init,    "RFC2795 Ltd",    "RC2014 Pro",        MACHINE_IS_SKELETON )
COMP( 2016, rc2014cl2,  rc2014,   0,        rc2014cl2, 0,       rc2014_state,    empty_init,    "RFC2795 Ltd",    "RC2014 Classic 2",  MACHINE_IS_SKELETON )
