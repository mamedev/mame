// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

CM-1800
(note name is in cyrilic letters)

more info at http://ru.wikipedia.org/wiki/%D0%A1%D0%9C_%D0%AD%D0%92%D0%9C
         and http://www.computer-museum.ru/histussr/sm1800.htm

2011-04-26 Skeleton driver.

Commands to be in uppercase:
C Compare
D Dump
F Fill
G
I
L
M Move
S Edit
T
X Registers

For most commands, enter all 4 digits of each hex address, the system will
add the appropriate spacing as you type. No need to press Enter.

The L command looks like it might be for loading a file, for example
L ABC will read/write to port 70,71,73 and eventually time out if you wait
a while. No idea if it wants to read a disk or a tape. There doesn't seem
to be a save command.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/ay31015.h"
#include "bus/rs232/rs232.h"


class cm1800_state : public driver_device
{
public:
	cm1800_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_uart(*this, "uart")
	{ }

	DECLARE_READ8_MEMBER(uart_status_r);

	void cm1800(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<ay31015_device> m_uart;
};

READ8_MEMBER(cm1800_state::uart_status_r)
{
	return (m_uart->dav_r()) | (m_uart->tbmt_r() << 2);
}

ADDRESS_MAP_START(cm1800_state::mem_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff ) AM_ROM AM_REGION("roms", 0)
	AM_RANGE( 0x0800, 0xffff ) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(cm1800_state::io_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("uart", ay31015_device, receive, transmit)
	AM_RANGE(0x01, 0x01) AM_READ(uart_status_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( cm1800 )
INPUT_PORTS_END


void cm1800_state::machine_reset()
{
	m_uart->write_xr(0);
	m_uart->write_xr(1);
	m_uart->write_swe(0);
	m_uart->write_np(1);
	m_uart->write_tsb(0);
	m_uart->write_nb1(1);
	m_uart->write_nb2(1);
	m_uart->write_eps(1);
	m_uart->write_cs(1);
	m_uart->write_cs(0);
}

MACHINE_CONFIG_START(cm1800_state::cm1800)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, XTAL(2'000'000))
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	/* video hardware */
	MCFG_DEVICE_ADD("uart", AY51013, 0) // exact uart type is unknown
	MCFG_AY51013_TX_CLOCK(153600)
	MCFG_AY51013_RX_CLOCK(153600)
	MCFG_AY51013_READ_SI_CB(DEVREADLINE("rs232", rs232_port_device, rxd_r))
	MCFG_AY51013_WRITE_SO_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_AY51013_AUTO_RDAV(true)
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( cm1800 )
	ROM_REGION( 0x800, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "cm1800.rom", 0x0000, 0x0800, CRC(85d71d25) SHA1(42dc87d2eddc2906fa26d35db88a2e29d50fb481) )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT   STATE          INIT  COMPANY      FULLNAME   FLAGS */
COMP( 1981, cm1800,  0,      0,       cm1800,    cm1800, cm1800_state,  0,    "<unknown>", "CM-1800", MACHINE_NO_SOUND_HW)
