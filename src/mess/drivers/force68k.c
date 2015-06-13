// license:BSD-3-Clause
// copyright-holders:Joamim Larsson Edström
/***************************************************************************

    Force SYS68K/CPU-1 VME SBC driver

    13/06/2015

    http://...

Based on the 68ksbc.c

    TODO:
    - Memory map
    - Dump ROM:s
    - Add 3 ACIA6850 
    - Add serial connector between ACIA:s and real terminal emulator
    - VME bus driver


****************************************************************************/

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/clock.h"

class force68k_state : public driver_device
{
public:
	force68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_acia1(*this, "acia1")
		m_acia2(*this, "acia2")
		m_acia3(*this, "acia3")
	{
	}

	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);

private:
	required_device<cpu_device> m_maincpu;
	required_device<acia6850_device> m_acia1;
	required_device<acia6850_device> m_acia2;
	required_device<acia6850_device> m_acia3;
};

static ADDRESS_MAP_START(force68k_mem, AS_PROGRAM, 16, force68k_state)
	AM_RANGE(0x000000, 0x000007) AM_ROM /* Vectors mapped from System EPROM */
	AM_RANGE(0x000008, 0x01ffff) AM_RAM /* DRAM */
	AM_RANGE(0x080008, 0x09ffff) AM_ROM /* System EPROM Area */
//	AM_RANGE(0x0a0000, 0x0bffff) AM_ROM /* User EPROM Area   */
//      AM_RANGE(0x0e0000, 0x0fffff) AM_READWRITE_PORT /* IO interfaces */
//	AM_RANGE(0x0e0000, 0x0e0001) AM_DEVREADWRITE8("acia", acia6850_device, status_r, control_w, 0x00ff)
//	AM_RANGE(0x0e0002, 0x0e0003) AM_DEVREADWRITE8("acia", acia6850_device, data_r, data_w, 0x00ff)
//	AM_RANGE(0x0e0004, 0x0e0005) AM_DEVREADWRITE8("acia", acia6850_device, data_r, data_w, 0x00ff)
//      AM_RANGE(0x100000, 0xfeffff) /* VMEbus Rev B addresses (24 bits) */
//      AM_RANGE(0xff0000, 0xffffff) /* VMEbus Rev B addresses (16 bits) */
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( force68k )
INPUT_PORTS_END


WRITE_LINE_MEMBER(force68k_state::write_acia_clock)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

static MACHINE_CONFIG_START( force68k, force68k_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 8000000) 
	MCFG_CPU_PROGRAM_MAP(force68k_mem)

/*
	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(force68k_state, write_acia_clock))
*/
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( forcecpu1 )
	ROM_REGION(0x1000000, "maincpu", 0)
//	ROM_LOAD( "forcecpu1.bin", 0x0000, 0x2f78, CRC(20a8d0d0) SHA1(544fd8bd8ed017115388c8b0f7a7a59a32253e43) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS         INIT    COMPANY                FULLNAME               FLAGS */
COMP( 1983, forcecpu1,  0,       0,    force68k,   force68k, driver_device,  0,  "Force Computers",  "SYS68K/CPU-1", GAME_NO_SOUND_HW)
