// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

NorthStar Horizon

2009-12-07 Skeleton driver.

http://www.hartetechnologies.com/manuals/Northstar/

The tiny bios (only about 200 bytes) initialises nothing, but only loads the
initial sector from the disk and transfers control to it. All the used memory
locations in the EA00-EB40 range are listed in the memory map. It does not
use the IO map, and has no text.

The 2MHz downgrade is suggested in the manual for the CPU board (ZPB-A). It
involves replacing the XTAL and reconnecting one jumper.

****************************************************************************/

/*

    TODO:

    - connect to S-100 bus
    - USARTs
    - parallel I/O
    - motherboard ports
    - RTC
    - RAM boards
    - floppy boards
    - floating point board
    - SOROC IQ 120 CRT terminal
    - NEC 5530-2 SPINWRITER printer
    - Anadex DP-8000 printer

*/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/s100/s100.h"
#include "bus/s100/am310.h"
//#include "bus/s100/dj2db.h"
//#include "bus/s100/djdma.h"
//#include "bus/s100/mm65k16s.h"
#include "bus/s100/nsmdsa.h"
#include "bus/s100/nsmdsad.h"
#include "bus/s100/seals8k.h"
//#include "bus/s100/wunderbus.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "softlist_dev.h"


namespace {

#define Z80_TAG         "z80"
#define I8251_L_TAG     "3a"
#define I8251_R_TAG     "4a"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"
#define S100_TAG        "s100"

class horizon_state : public driver_device
{
public:
	horizon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_usart_l(*this, I8251_L_TAG)
		, m_usart_r(*this, I8251_R_TAG)
		, m_s100(*this, S100_TAG)
		{ }

	void horizon(machine_config &config);
	void horizon2mhz(machine_config &config);

private:
	uint8_t ff_r();

	void horizon_io(address_map &map) ATTR_COLD;
	void horizon_mem(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_usart_l;
	required_device<i8251_device> m_usart_r;
	required_device<s100_bus_device> m_s100;
};



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( horizon_mem )
//-------------------------------------------------

void horizon_state::horizon_mem(address_map &map)
{
	map(0x0000, 0xe7ff).ram();
	map(0xe800, 0xe9ff).rom().region("roms", 0);
	map(0xea01, 0xea01);
	map(0xea11, 0xea11);
	map(0xea21, 0xea21);
	map(0xea31, 0xea31);
	map(0xeb10, 0xeb17).r(FUNC(horizon_state::ff_r));
	map(0xeb20, 0xeb20);
	map(0xeb35, 0xeb35);
	map(0xeb40, 0xeb40);
}


//-------------------------------------------------
//  ADDRESS_MAP( horizon_io )
//-------------------------------------------------

void horizon_state::horizon_io(address_map &map)
{
}



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS
//-------------------------------------------------

static INPUT_PORTS_START( horizon )
INPUT_PORTS_END

void horizon_state::machine_reset()
{
	m_maincpu->set_pc(0xe800);
}

uint8_t horizon_state::ff_r()
{
	return 0xff;
}

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  S100_INTERFACE( s100_intf )
//-------------------------------------------------

static void horizon_s100_cards(device_slot_interface &device)
{
	device.option_add("mdsa", S100_MDS_A);
	device.option_add("mdsad", S100_MDS_AD);
	//device.option_add("hram", S100_HRAM);
	//device.option_add("ram32a", S100_RAM32A);
	//device.option_add("ram16a", S100_RAM16A);
	//device.option_add("fpb", S100_FPB);
	device.option_add("8ksc", S100_8K_SC);
	device.option_add("8kscbb", S100_8K_SC_BB);
	device.option_add("am310", S100_AM310);
}



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  machine_config( horizon )
//-------------------------------------------------

void horizon_state::horizon(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &horizon_state::horizon_mem);
	m_maincpu->set_addrmap(AS_IO, &horizon_state::horizon_io);

	// devices
	I8251(config, m_usart_l, 0);
	m_usart_l->txd_handler().set(RS232_A_TAG, FUNC(rs232_port_device::write_txd));
	m_usart_l->dtr_handler().set(RS232_A_TAG, FUNC(rs232_port_device::write_dtr));
	m_usart_l->rts_handler().set(RS232_A_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, RS232_A_TAG, default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set(m_usart_l, FUNC(i8251_device::write_rxd));
	rs232a.dsr_handler().set(m_usart_l, FUNC(i8251_device::write_dsr));
	rs232a.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	I8251(config, m_usart_r, 0);
	m_usart_r->txd_handler().set(RS232_B_TAG, FUNC(rs232_port_device::write_txd));
	m_usart_r->dtr_handler().set(RS232_B_TAG, FUNC(rs232_port_device::write_dtr));
	m_usart_r->rts_handler().set(RS232_B_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232b(RS232_PORT(config, RS232_B_TAG, default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_usart_r, FUNC(i8251_device::write_rxd));
	rs232b.dsr_handler().set(m_usart_r, FUNC(i8251_device::write_dsr));

	// S-100
	S100_BUS(config, m_s100, XTAL(8'000'000) / 4);
	//m_s100->rdy().set_inputline(m_maincpu, Z80_INPUT_LINE_WAIT);
	//S100_SLOT(config, S100_TAG":1", horizon_s100_cards, nullptr, nullptr); // CPU
	S100_SLOT(config, "s100:2", horizon_s100_cards, nullptr); // RAM
	S100_SLOT(config, "s100:3", horizon_s100_cards, "mdsad"); // MDS
	S100_SLOT(config, "s100:4", horizon_s100_cards, nullptr); // FPB
	S100_SLOT(config, "s100:5", horizon_s100_cards, nullptr);
	S100_SLOT(config, "s100:6", horizon_s100_cards, nullptr);
	S100_SLOT(config, "s100:7", horizon_s100_cards, nullptr);
	S100_SLOT(config, "s100:8", horizon_s100_cards, nullptr);
	S100_SLOT(config, "s100:9", horizon_s100_cards, nullptr);
	S100_SLOT(config, "s100:10", horizon_s100_cards, nullptr);
	S100_SLOT(config, "s100:11", horizon_s100_cards, nullptr);
	S100_SLOT(config, "s100:12", horizon_s100_cards, nullptr);

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("horizon");
}

void horizon_state::horizon2mhz(machine_config &config)
{
	horizon(config);
	m_maincpu->set_clock(XTAL(4'000'000) / 2);
	m_s100->set_clock(XTAL(4'000'000) / 2);
}



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( nshrz )
//-------------------------------------------------

ROM_START( nshrz )
	ROM_REGION( 0x400, "roms", 0 )
	ROM_LOAD( "option.prom", 0x000, 0x400, NO_DUMP )
ROM_END

#define rom_nshrz2mhz rom_nshrz


//-------------------------------------------------
//  ROM( vector1 )
//-------------------------------------------------

ROM_START( vector1 ) // This one have different I/O
	ROM_REGION( 0x400, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "horizon.bin", 0x0000, 0x0100, CRC(7aafa134) SHA1(bf1552c4818f30473798af4f54e65e1957e0db48))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME       PARENT  COMPAT  MACHINE      INPUT    CLASS          INIT        COMPANY                 FULLNAME                                FLAGS
COMP( 1976, nshrz,     0,      0,      horizon,     horizon, horizon_state, empty_init, "North Star Computers", "Horizon (North Star Computers, 4MHz)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1976, nshrz2mhz, nshrz,  0,      horizon2mhz, horizon, horizon_state, empty_init, "North Star Computers", "Horizon (North Star Computers, 2MHz)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )

// This really should be in its own driver
COMP( 1979, vector1,   0,      0,      horizon,     horizon, horizon_state, empty_init, "Vector Graphic",       "Vector 1+ (DD drive)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
