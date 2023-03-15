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
		, m_bus(*this, "bus")
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
		RC2014_BUS(config, m_bus, 0);
		RC2014_SLOT(config, "bus:1", m_bus, rc2014_bus_modules, "z80");
		RC2014_SLOT(config, "bus:2", m_bus, rc2014_bus_modules, "clock");
		RC2014_SLOT(config, "bus:3", m_bus, rc2014_bus_modules, "ram32k");
		RC2014_SLOT(config, "bus:4", m_bus, rc2014_bus_modules, "sw_rom");
		RC2014_SLOT(config, "bus:5", m_bus, rc2014_bus_modules, "serial");
	}

	//
	// Backplane-5 - 5 x 40pin slots
	//
	void rc2014bp5(machine_config &config)
	{
		RC2014_BUS(config, m_bus, 0);
		RC2014_SLOT(config, "bus:1", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:2", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:3", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:4", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:5", m_bus, rc2014_bus_modules, nullptr);
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
		RC2014_BUS(config, m_bus, 0);
		RC2014_SLOT(config, "bus:1", m_bus, rc2014_bus_modules, "z80");
		RC2014_SLOT(config, "bus:2", m_bus, rc2014_bus_modules, "clock");
		RC2014_SLOT(config, "bus:3", m_bus, rc2014_bus_modules, "ram32k");
		RC2014_SLOT(config, "bus:4", m_bus, rc2014_bus_modules, "sw_rom");
		RC2014_SLOT(config, "bus:5", m_bus, rc2014_bus_modules, "serial");
		RC2014_SLOT(config, "bus:6", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:7", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:8", m_bus, rc2014_bus_modules, nullptr);
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
		RC2014_BUS(config, m_bus, 0);
		RC2014_SLOT(config, "bus:1", m_bus, rc2014_bus_modules, "z80_21_40p");
		RC2014_SLOT(config, "bus:2", m_bus, rc2014_bus_modules, "dual_clk_40p");
		RC2014_SLOT(config, "bus:3", m_bus, rc2014_bus_modules, "rom_ram");
		RC2014_SLOT(config, "bus:4", m_bus, rc2014_bus_modules, "sio_40p");
		RC2014_SLOT(config, "bus:5", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:6", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:7", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:8", m_bus, rc2014_bus_modules, nullptr);
	}

	//
	// Backplane 8 - 8 x 40pin slots
	//
	void rc2014bp8(machine_config &config)
	{
		RC2014_BUS(config, m_bus, 0);
		RC2014_SLOT(config, "bus:1", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:2", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:3", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:4", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:5", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:6", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:7", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:8", m_bus, rc2014_bus_modules, nullptr);
	}

	//
	// SC133 – MODULAR BACKPLANE (RC2014)
	//
	void sc133(machine_config &config)
	{
		RC2014_BUS(config, m_bus, 0);
		RC2014_SLOT(config, "bus:1", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:2", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:3", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:4", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:5", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:6", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:7", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:8", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:9", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:10", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:11", m_bus, rc2014_bus_modules, nullptr);
		RC2014_SLOT(config, "bus:e",  m_bus, rc2014_bus_edge_modules, nullptr);
	}

private:
	required_device<rc2014_bus_device> m_bus;
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

ROM_START(sc133)
ROM_END

class rc2014pro_state : public driver_device
{
public:
	rc2014pro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bus(*this, "bus")
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
		RC2014_EXT_BUS(config, m_bus, 0);
		RC2014_EXT_SLOT(config, "bus:1", m_bus, rc2014_ext_bus_modules, "z80_21");
		RC2014_EXT_SLOT(config, "bus:2", m_bus, rc2014_ext_bus_modules, "dual_clk");
		RC2014_EXT_SLOT(config, "bus:3", m_bus, rc2014_ext_bus_modules, "ram64k");
		RC2014_EXT_SLOT(config, "bus:4", m_bus, rc2014_ext_bus_modules, "page_rom");
		RC2014_EXT_SLOT(config, "bus:5", m_bus, rc2014_ext_bus_modules, "sio");
		RC2014_EXT_SLOT(config, "bus:6", m_bus, rc2014_ext_bus_modules, "cf");
		RC2014_EXT_SLOT(config, "bus:7", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:8", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:9", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:10", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:11", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:12", m_bus, rc2014_ext_bus_modules, nullptr);
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
		RC2014_EXT_BUS(config, m_bus, 0);
		RC2014_EXT_SLOT(config, "bus:1", m_bus, rc2014_ext_bus_modules, "z80_21");
		RC2014_EXT_SLOT(config, "bus:2", m_bus, rc2014_ext_bus_modules, "dual_clk");
		RC2014_EXT_SLOT(config, "bus:3", m_bus, rc2014_ext_bus_modules, "rom_ram");
		RC2014_EXT_SLOT(config, "bus:4", m_bus, rc2014_ext_bus_modules, "sio");
		RC2014_EXT_SLOT(config, "bus:5", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:6", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:7", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:8", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:9", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:10", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:11", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:12", m_bus, rc2014_ext_bus_modules, nullptr);
	}

	//
	// Backplane Pro - 12 x extended slots
	//
	void rc2014bppro(machine_config &config)
	{
		RC2014_EXT_BUS(config, m_bus, 0);
		RC2014_EXT_SLOT(config, "bus:1", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:2", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:3", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:4", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:5", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:6", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:7", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:8", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:9", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:10", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:11", m_bus, rc2014_ext_bus_modules, nullptr);
		RC2014_EXT_SLOT(config, "bus:12", m_bus, rc2014_ext_bus_modules, nullptr);
	}
private:
	required_device<rc2014_ext_bus_device> m_bus;
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
		, m_bus(*this, "bus")
	{ }

	//
	// RC2014 Mini
	//
	// Added by Chris Swan
	// https://rc2014.co.uk/full-kits/rc2014-mini/
	//
	void rc2014mini(machine_config &config)
	{
		RC2014_BUS(config, m_bus, 0);
		RC2014_SLOT(config, "board", m_bus, rc2014_mini_bus_modules, "mini", true);
		RC2014_SLOT(config, "bus:1", m_bus, rc2014_mini_bus_modules, nullptr);
	}

	//
	// RC2014 Mini with CP/M Upgrade
	//
	void rc2014minicpm(machine_config &config)
	{
		RC2014_BUS(config, m_bus, 0);
		RC2014_SLOT(config, "board", m_bus, rc2014_mini_bus_modules, "mini", true).set_option_device_input_defaults("mini", DEVICE_INPUT_DEFAULTS_NAME(mini_cpm));
		RC2014_SLOT(config, "bus:1", m_bus, rc2014_mini_bus_modules, "mini_cpm", true);
	}

	//
	// RC2014 Micro
	//
	// This is card that can be used directly without any backplane board
	// Power is coming from usb-to-serial adapter
	//
	void rc2014micro(machine_config &config)
	{
		RC2014_BUS(config, m_bus, 0);
		RC2014_SLOT(config, "board", m_bus, rc2014_bus_modules, "micro", true);
	}
private:
	required_device<rc2014_bus_device> m_bus;
};

ROM_START(rc2014mini)
ROM_END

ROM_START(rc2014minicpm)
ROM_END

ROM_START(rc2014micro)
ROM_END

class scc_state : public driver_device
{
public:
	scc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bus(*this, "bus")
	{ }

	//
	// SC105 – MODULAR BACKPLANE (RC2014)
	//
	// Input: Power supply
	// Cards: 6 x RC2014/80 pin sockets
	// Output: Edge mounted BP80 socket
	//
	void sc105(machine_config &config)
	{
		RC2014_RC80_BUS(config, m_bus, 0);
		RC2014_RC80_SLOT(config, "bus:1", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:2", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:3", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:4", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:5", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:6", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:e", m_bus, rc2014_rc80_bus_edge_modules, nullptr);
	}

	//
	// SC112 – MODULAR BACKPLANE (RC2014)
	//
	// Input: Power supply
	// Cards: 6 x RC2014/80 pin sockets
	// Output: Through-hole BP80 socket
	//
	void sc112(machine_config &config)
	{
		RC2014_RC80_BUS(config, m_bus, 0);
		RC2014_RC80_SLOT(config, "bus:1", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:2", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:3", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:4", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:5", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:6", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:e", m_bus, rc2014_rc80_bus_edge_modules, nullptr);
	}

	//
	// SC116 – 3-SLOT BACKPLANE (RC2014)
	//
	// Input: Power supply
	// Cards: 3 x RC2014/80 pin sockets
	//
	void sc116(machine_config &config)
	{
		RC2014_RC80_BUS(config, m_bus, 0);
		RC2014_RC80_SLOT(config, "bus:1", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:2", m_bus, rc2014_rc80_bus_modules, nullptr);
		RC2014_RC80_SLOT(config, "bus:3", m_bus, rc2014_rc80_bus_modules, nullptr);
	}

	//
	// SC203 – Modular Z180 Computer
	//
	// SC111 - Z180 CPU module
	// SC119 - Z180 memory module
	// SC112 - 6-slot backplane with power input, or
	// SC116 - 3-slot backplane with power input
	//
	void sc203(machine_config &config)
	{
		RC2014_RC80_BUS(config, m_bus, 0);
		RC2014_RC80_SLOT(config, "bus:1", m_bus, rc2014_rc80_bus_modules, "sc111");
		RC2014_RC80_SLOT(config, "bus:2", m_bus, rc2014_rc80_bus_modules, "sc119");
		RC2014_RC80_SLOT(config, "bus:3", m_bus, rc2014_rc80_bus_modules, nullptr);
	}

private:
	required_device<rc2014_rc80_bus_device> m_bus;
};

ROM_START(sc105)
ROM_END

ROM_START(sc112)
ROM_END

ROM_START(sc116)
ROM_END

ROM_START(sc203)
ROM_END

} // anonymous namespace

// This ties everything together
//    YEAR  NAME          PARENT    COMPAT    MACHINE         INPUT    CLASS             INIT           COMPANY              FULLNAME                         FLAGS
COMP( 2016, rc2014,       0,        0,        rc2014,         0,       rc2014_state,     empty_init,    "RFC2795 Ltd",       "RC2014 Classic",                MACHINE_SUPPORTS_SAVE )
COMP( 2017, rc2014pro,    rc2014,   0,        rc2014pro,      0,       rc2014pro_state,  empty_init,    "RFC2795 Ltd",       "RC2014 Pro",                    MACHINE_SUPPORTS_SAVE )
COMP( 2020, rc2014cl2,    rc2014,   0,        rc2014cl2,      0,       rc2014_state,     empty_init,    "RFC2795 Ltd",       "RC2014 Classic II",             MACHINE_SUPPORTS_SAVE )
COMP( 2018, rc2014zed,    rc2014,   0,        rc2014zed,      0,       rc2014_state,     empty_init,    "RFC2795 Ltd",       "RC2014 Zed",                    MACHINE_SUPPORTS_SAVE )
COMP( 2018, rc2014zedp,   rc2014,   0,        rc2014zedp,     0,       rc2014pro_state,  empty_init,    "RFC2795 Ltd",       "RC2014 Zed Pro",                MACHINE_SUPPORTS_SAVE )
COMP( 2016, rc2014mini,   rc2014,   0,        rc2014mini,     0,       rc2014mini_state, empty_init,    "RFC2795 Ltd",       "RC2014 Mini",                   MACHINE_SUPPORTS_SAVE )
COMP( 2016, rc2014minicpm,rc2014,   0,        rc2014minicpm,  0,       rc2014mini_state, empty_init,    "RFC2795 Ltd",       "RC2014 Mini with CP/M Upgrade", MACHINE_SUPPORTS_SAVE )
COMP( 2019, rc2014micro,  rc2014,   0,        rc2014micro,    0,       rc2014mini_state, empty_init,    "RFC2795 Ltd",       "RC2014 Micro",                  MACHINE_SUPPORTS_SAVE )
COMP( 2018, sc203,        rc2014,   0,        sc203,          0,       scc_state,        empty_init,    "Stephen C Cousins", "SC203 - Modular Z180 Computer", MACHINE_SUPPORTS_SAVE )
// Backplanes
COMP( 2016, rc2014bp5,    rc2014,   0,        rc2014bp5,      0,       rc2014_state,     empty_init,    "RFC2795 Ltd",       "RC2014 Backplane-5",                 MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 2016, rc2014bp8,    rc2014,   0,        rc2014bp8,      0,       rc2014_state,     empty_init,    "RFC2795 Ltd",       "RC2014 Backplane-8",                 MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 2017, rc2014bppro,  rc2014,   0,        rc2014bppro,    0,       rc2014pro_state,  empty_init,    "RFC2795 Ltd",       "RC2014 Backplane Pro",               MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 2018, sc105,        rc2014,   0,        sc105,          0,       scc_state,        empty_init,    "Stephen C Cousins", "SC105 - Modular Backplane (RC2014)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 2018, sc112,        rc2014,   0,        sc112,          0,       scc_state,        empty_init,    "Stephen C Cousins", "SC112 - Modular Backplane (RC2014)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 2018, sc116,        rc2014,   0,        sc116,          0,       scc_state,        empty_init,    "Stephen C Cousins", "SC116 - Modular Backplane (RC2014)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
COMP( 2018, sc133,        rc2014,   0,        sc133,          0,       rc2014_state,     empty_init,    "Stephen C Cousins", "SC133 - Modular Backplane (RC2014)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
