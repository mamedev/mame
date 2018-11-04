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
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "softlist.h"

#define Z80_TAG         "z80"
#define I8251_L_TAG     "3a"
#define I8251_R_TAG     "4a"
#define RS232_A_TAG     "rs232a"
#define RS232_B_TAG     "rs232b"

class horizon_state : public driver_device
{
public:
	horizon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_usart_l(*this, I8251_L_TAG)
		, m_usart_r(*this, I8251_R_TAG)
		{ }

	DECLARE_READ8_MEMBER(ff_r);

	void horizon(machine_config &config);
	void horizon2mhz(machine_config &config);
	void horizon_io(address_map &map);
	void horizon_mem(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<i8251_device> m_usart_l;
	required_device<i8251_device> m_usart_r;
};



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( horizon_mem )
//-------------------------------------------------

ADDRESS_MAP_START(horizon_state::horizon_mem)
	AM_RANGE(0x0000, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xe9ff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xea01, 0xea01)
	AM_RANGE(0xea11, 0xea11)
	AM_RANGE(0xea21, 0xea21)
	AM_RANGE(0xea31, 0xea31)
	AM_RANGE(0xeb10, 0xeb17) AM_READ(ff_r)
	AM_RANGE(0xeb20, 0xeb20)
	AM_RANGE(0xeb35, 0xeb35)
	AM_RANGE(0xeb40, 0xeb40)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( horizon_io )
//-------------------------------------------------

ADDRESS_MAP_START(horizon_state::horizon_io)
ADDRESS_MAP_END



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

READ8_MEMBER( horizon_state::ff_r )
{
	return 0xff;
}

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  S100_INTERFACE( s100_intf )
//-------------------------------------------------

// slot devices
//#include "bus/s100/dj2db.h"
//#include "bus/s100/djdma.h"
//#include "bus/s100/mm65k16s.h"
#include "bus/s100/nsmdsa.h"
#include "bus/s100/nsmdsad.h"
#include "bus/s100/seals8k.h"
//#include "bus/s100/wunderbus.h"

static SLOT_INTERFACE_START( horizon_s100_cards )
	SLOT_INTERFACE("mdsa", S100_MDS_A)
	SLOT_INTERFACE("mdsad", S100_MDS_AD)
	//SLOT_INTERFACE("hram", S100_HRAM)
	//SLOT_INTERFACE("ram32a", S100_RAM32A)
	//SLOT_INTERFACE("ram16a", S100_RAM16A)
	//SLOT_INTERFACE("fpb", S100_FPB)
	SLOT_INTERFACE("8ksc", S100_8K_SC)
	SLOT_INTERFACE("8kscbb", S100_8K_SC_BB)
SLOT_INTERFACE_END



//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( horizon )
//-------------------------------------------------

MACHINE_CONFIG_START(horizon_state::horizon)
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL(8'000'000) / 2)
	MCFG_CPU_PROGRAM_MAP(horizon_mem)
	MCFG_CPU_IO_MAP(horizon_io)

	// devices
	MCFG_DEVICE_ADD(I8251_L_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_A_TAG, rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD(RS232_A_TAG, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251_L_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251_L_TAG, i8251_device, write_dsr))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	MCFG_DEVICE_ADD(I8251_R_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_B_TAG, rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD(RS232_B_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251_R_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251_R_TAG, i8251_device, write_dsr))

	// S-100
	MCFG_DEVICE_ADD("s100", S100_BUS, XTAL(8'000'000) / 4)
	MCFG_S100_RDY_CALLBACK(INPUTLINE(Z80_TAG, Z80_INPUT_LINE_BOGUSWAIT))
	//MCFG_S100_SLOT_ADD("s100:1", horizon_s100_cards, nullptr, nullptr) // CPU
	MCFG_S100_SLOT_ADD("s100:2", horizon_s100_cards, nullptr) // RAM
	MCFG_S100_SLOT_ADD("s100:3", horizon_s100_cards, "mdsad") // MDS
	MCFG_S100_SLOT_ADD("s100:4", horizon_s100_cards, nullptr) // FPB
	MCFG_S100_SLOT_ADD("s100:5", horizon_s100_cards, nullptr)
	MCFG_S100_SLOT_ADD("s100:6", horizon_s100_cards, nullptr)
	MCFG_S100_SLOT_ADD("s100:7", horizon_s100_cards, nullptr)
	MCFG_S100_SLOT_ADD("s100:8", horizon_s100_cards, nullptr)
	MCFG_S100_SLOT_ADD("s100:9", horizon_s100_cards, nullptr)
	MCFG_S100_SLOT_ADD("s100:10", horizon_s100_cards, nullptr)
	MCFG_S100_SLOT_ADD("s100:11", horizon_s100_cards, nullptr)
	MCFG_S100_SLOT_ADD("s100:12", horizon_s100_cards, nullptr)

	// software list
	MCFG_SOFTWARE_LIST_ADD("flop_list", "horizon")
MACHINE_CONFIG_END

MACHINE_CONFIG_START(horizon_state::horizon2mhz)
	horizon(config);
	MCFG_CPU_MODIFY("z80")
	MCFG_CPU_CLOCK(XTAL(4'000'000) / 2)

	MCFG_DEVICE_MODIFY("s100")
	MCFG_DEVICE_CLOCK(XTAL(4'000'000) / 2)
MACHINE_CONFIG_END



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



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    STATE          INIT  COMPANY                 FULLNAME                          FLAGS
COMP( 1976, nshrz,     0,     0, horizon,     horizon, horizon_state, 0, "North Star Computers", "Horizon (North Star Computers, 4MHz)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1976, nshrz2mhz, nshrz, 0, horizon2mhz, horizon, horizon_state, 0, "North Star Computers", "Horizon (North Star Computers, 2MHz)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )

// This really should be in its own driver
COMP( 1979, vector1,  0,      0, horizon,     horizon, horizon_state, 0, "Vector Graphic",       "Vector 1+ (DD drive)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
