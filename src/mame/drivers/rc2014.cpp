// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/**************************************************************************************

    RC2014 Modular Computer

**************************************************************************************/

#include "emu.h"
#include "bus/rc2014/rc2014.h"
#include "bus/rc2014/modules.h"

namespace {

class rc2014_state : public driver_device
{
public:
	rc2014_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rc2014_bus(*this, "bus")
	{ }

	//
	// RC2014 Classic
	//
	// Z80 CPU module
	// Clock/Reset
	// 32K RAM module
	// Switchable ROM module
	// Serial I/O
	//
	// Backplane-5 - 5 x 40pin slots
	//
	void rc2014(machine_config &config)
	{
		RC2014_BUS(config, m_rc2014_bus, 0);
		RC2014_SLOT(config, "bus:1", m_rc2014_bus, rc2014_bus_modules, "z80");
		RC2014_SLOT(config, "bus:2", m_rc2014_bus, rc2014_bus_modules, "clock");
		RC2014_SLOT(config, "bus:3", m_rc2014_bus, rc2014_bus_modules, "ram32k");
		RC2014_SLOT(config, "bus:4", m_rc2014_bus, rc2014_bus_modules, "sw_rom");
		RC2014_SLOT(config, "bus:5", m_rc2014_bus, rc2014_bus_modules, "serial");
	}

	//
	// Backplane-5 - 5 x 40pin slots
	//
	void rc2014bp5(machine_config &config)
	{
		RC2014_BUS(config, m_rc2014_bus, 0);
		RC2014_SLOT(config, "bus:1", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:2", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:3", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:4", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:5", m_rc2014_bus, rc2014_bus_modules, nullptr);
	}

	//
	// RC2014 Classic II
	//
	// Clock/Reset
	// Z80 CPU module
	// 32K RAM module
	// Switchable ROM module
	// Serial I/O
	//
	// Backplane-8 - 8 x 40pin slots
	//
	// all modules are in slightly different form factor
	//
	void rc2014cl2(machine_config &config)
	{
		RC2014_BUS(config, m_rc2014_bus, 0);
		RC2014_SLOT(config, "bus:1", m_rc2014_bus, rc2014_bus_modules, "z80");
		RC2014_SLOT(config, "bus:2", m_rc2014_bus, rc2014_bus_modules, "clock");
		RC2014_SLOT(config, "bus:3", m_rc2014_bus, rc2014_bus_modules, "ram32k");
		RC2014_SLOT(config, "bus:4", m_rc2014_bus, rc2014_bus_modules, "sw_rom");
		RC2014_SLOT(config, "bus:5", m_rc2014_bus, rc2014_bus_modules, "serial");
		RC2014_SLOT(config, "bus:6", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:7", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:8", m_rc2014_bus, rc2014_bus_modules, nullptr);
	}


	//
	// RC2014 Zed
	//
	// Z80 2.1 CPU Module
	// Dual Clock Module
	// 512k ROM 512k RAM Module
	// Dual Serial SIO/2 Module
	//
	// Backplane 8 - 8 x 40pin slots
	//
	// Some modules are extended bus modules in standard slots
	//
	void rc2014zed(machine_config &config)
	{
		RC2014_BUS(config, m_rc2014_bus, 0);
		RC2014_SLOT(config, "bus:1", m_rc2014_bus, rc2014_bus_modules, "z80_21_40p");
		RC2014_SLOT(config, "bus:2", m_rc2014_bus, rc2014_bus_modules, "dual_clk_40p");
		RC2014_SLOT(config, "bus:3", m_rc2014_bus, rc2014_bus_modules, "rom_ram");
		RC2014_SLOT(config, "bus:4", m_rc2014_bus, rc2014_bus_modules, "sio_40p");
		RC2014_SLOT(config, "bus:5", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:6", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:7", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:8", m_rc2014_bus, rc2014_bus_modules, nullptr);
	}

	//
	// Backplane 8 - 8 x 40pin slots
	//
	void rc2014bp8(machine_config &config)
	{
		RC2014_BUS(config, m_rc2014_bus, 0);
		RC2014_SLOT(config, "bus:1", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:2", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:3", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:4", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:5", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:6", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:7", m_rc2014_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:8", m_rc2014_bus, rc2014_bus_modules, nullptr);
	}

private:
	required_device<rc2014_bus_device> m_rc2014_bus;
};

ROM_START(rc2014)
ROM_END

ROM_START(rc2014bp5)
ROM_END

ROM_START(rc2014cl2)
ROM_END

ROM_START(rc2014zed)
ROM_END

ROM_START(rc2014bp8)
ROM_END

class rc2014pro_state : public driver_device
{
public:
	rc2014pro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rc2014_bus(*this, "bus")
	{ }

	//
	// RC2014 PRo
	//
	// Z80 2.1 CPU Module
	// Dual Clock Module
	// 64k RAM
	// Pageable ROM
	// Dual Serial SIO/2 Module
	// Compact Flash storage
	//
	// Backplane Pro - 12 x extended slots
	//
	void rc2014pro(machine_config &config)
	{
		RC2014_EXT_BUS(config, m_rc2014_bus, 0);
		RC2014_EXT_SLOT(config, "bus:1", m_rc2014_bus, rc2014_ext_bus_modules, "z80_21");
		RC2014_EXT_SLOT(config, "bus:2", m_rc2014_bus, rc2014_ext_bus_modules, "dual_clk");
		RC2014_EXT_SLOT(config, "bus:3", m_rc2014_bus, rc2014_ext_bus_modules, "ram64k");
		RC2014_EXT_SLOT(config, "bus:4", m_rc2014_bus, rc2014_ext_bus_modules, "page_rom");
		RC2014_EXT_SLOT(config, "bus:5", m_rc2014_bus, rc2014_ext_bus_modules, "sio");
		RC2014_EXT_SLOT(config, "bus:6", m_rc2014_bus, rc2014_ext_bus_modules, "cf");
		RC2014_EXT_SLOT(config, "bus:7", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:8", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:9", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:10", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:11", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:12", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
	}

	//
	// RC2014 Zed
	//
	// Z80 2.1 CPU Module
	// Dual Clock Module
	// 512k ROM 512k RAM Module
	// Dual Serial SIO/2 Module
	//
	// Backplane Pro - 12 x extended slots
	//
	void rc2014zedp(machine_config &config)
	{
		RC2014_EXT_BUS(config, m_rc2014_bus, 0);
		RC2014_EXT_SLOT(config, "bus:1", m_rc2014_bus, rc2014_ext_bus_modules, "z80_21");
		RC2014_EXT_SLOT(config, "bus:2", m_rc2014_bus, rc2014_ext_bus_modules, "dual_clk");
		RC2014_EXT_SLOT(config, "bus:3", m_rc2014_bus, rc2014_ext_bus_modules, "rom_ram");
		RC2014_EXT_SLOT(config, "bus:4", m_rc2014_bus, rc2014_ext_bus_modules, "sio");
		RC2014_EXT_SLOT(config, "bus:5", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:6", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:7", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:8", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:9", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:10", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:11", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:12", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
	}

	//
	// Backplane Pro - 12 x extended slots
	//
	void rc2014bppro(machine_config &config)
	{
		RC2014_EXT_BUS(config, m_rc2014_bus, 0);
		RC2014_EXT_SLOT(config, "bus:1", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:2", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:3", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:4", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:5", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:6", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:7", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:8", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:9", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:10", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:11", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:12", m_rc2014_bus, rc2014_ext_bus_modules, nullptr);
	}
private:
	required_device<rc2014_ext_bus_device> m_rc2014_bus;
};

ROM_START(rc2014pro)
ROM_END

ROM_START(rc2014zedp)
ROM_END

ROM_START(rc2014bppro)
ROM_END

static DEVICE_INPUT_DEFAULTS_START(mini_cpm)
	DEVICE_INPUT_DEFAULTS("ROM", 0x1, 0x0)
DEVICE_INPUT_DEFAULTS_END

class rc2014mini_state : public driver_device
{
public:
	rc2014mini_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_rc2014_bus(*this, "bus")
	{ }

	//
	// RC2014 Mini
	//
	// Added by Chris Swan
	// https://rc2014.co.uk/full-kits/rc2014-mini/
	//
	void rc2014mini(machine_config &config)
	{
		RC2014_BUS(config, m_rc2014_bus, 0);
		RC2014_SLOT(config, "board", m_rc2014_bus, rc2014_mini_bus_modules, "mini", true);
		RC2014_SLOT(config, "bus:1", m_rc2014_bus, rc2014_mini_bus_modules, nullptr);
	}

	//
	// RC2014 Mini with CP/M Upgrade
	//
	void rc2014minicpm(machine_config &config)
	{
		RC2014_BUS(config, m_rc2014_bus, 0);
		RC2014_SLOT(config, "board", m_rc2014_bus, rc2014_mini_bus_modules, "mini", true).set_option_device_input_defaults("mini", DEVICE_INPUT_DEFAULTS_NAME(mini_cpm));
		RC2014_SLOT(config, "bus:1", m_rc2014_bus, rc2014_mini_bus_modules, "mini_cpm", true);
	}

private:
	required_device<rc2014_bus_device> m_rc2014_bus;
};

ROM_START(rc2014mini)
ROM_END

ROM_START(rc2014minicpm)
ROM_END

} // anonymous namespace

// This ties everything together
//    YEAR  NAME          PARENT    COMPAT    MACHINE         INPUT    CLASS             INIT           COMPANY           FULLNAME                         FLAGS
COMP( 2016, rc2014,       0,        0,        rc2014,         0,       rc2014_state,     empty_init,    "RFC2795 Ltd",    "RC2014 Classic",                MACHINE_SUPPORTS_SAVE )
COMP( 2017, rc2014pro,    rc2014,   0,        rc2014pro,      0,       rc2014pro_state,  empty_init,    "RFC2795 Ltd",    "RC2014 Pro",                    MACHINE_SUPPORTS_SAVE )
COMP( 2020, rc2014cl2,    rc2014,   0,        rc2014cl2,      0,       rc2014_state,     empty_init,    "RFC2795 Ltd",    "RC2014 Classic II",             MACHINE_SUPPORTS_SAVE )
COMP( 2018, rc2014zed,    rc2014,   0,        rc2014zed,      0,       rc2014_state,     empty_init,    "RFC2795 Ltd",    "RC2014 Zed",                    MACHINE_SUPPORTS_SAVE )
COMP( 2018, rc2014zedp,   rc2014,   0,        rc2014zedp,     0,       rc2014pro_state,  empty_init,    "RFC2795 Ltd",    "RC2014 Zed Pro",                MACHINE_SUPPORTS_SAVE )
COMP( 2016, rc2014mini,   rc2014,   0,        rc2014mini,     0,       rc2014mini_state, empty_init,    "RFC2795 Ltd",    "RC2014 Mini",                   MACHINE_SUPPORTS_SAVE )
COMP( 2016, rc2014minicpm,rc2014,   0,        rc2014minicpm,  0,       rc2014mini_state, empty_init,    "RFC2795 Ltd",    "RC2014 Mini with CP/M Upgrade", MACHINE_SUPPORTS_SAVE )

// Backplanes
COMP( 2016, rc2014bp5,    rc2014,   0,        rc2014bp5,      0,       rc2014_state,     empty_init,    "RFC2795 Ltd",    "RC2014 Backplane-5",            MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 2016, rc2014bp8,    rc2014,   0,        rc2014bp8,      0,       rc2014_state,     empty_init,    "RFC2795 Ltd",    "RC2014 Backplane-8",            MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 2017, rc2014bppro,  rc2014,   0,        rc2014bppro,    0,       rc2014pro_state,  empty_init,    "RFC2795 Ltd",    "RC2014 Backplane Pro",          MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
