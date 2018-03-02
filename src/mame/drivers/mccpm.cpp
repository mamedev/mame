// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        mc-CP/M-Computer

        31/08/2010 Skeleton driver.
        18/11/2010 Connected to a terminal
        28/09/2011 Added more bioses

Some Monitor commands (varies between versions):

B - boot a floppy (^N to regain control)
E - prints a number
I - Select boot drive/set parameters
K,O - display version header
N - newline
Z - print 'EFFF'

URL for v3.4: http://www.hanshehl.de/mc-prog.htm (German language)

unknown I/O ports (my guess)
30 - fdc (1st drive)
40 - fdc (2nd drive)


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"


class mccpm_state : public driver_device
{
public:
	mccpm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "ram")
	{ }

	void mccpm(machine_config &config);
	void mccpm_io(address_map &map);
	void mccpm_mem(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_ram;
};


ADDRESS_MAP_START(mccpm_state::mccpm_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_SHARE("ram")
ADDRESS_MAP_END

ADDRESS_MAP_START(mccpm_state::mccpm_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf3) AM_DEVREADWRITE("sio", z80sio_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0xf4, 0xf7) AM_DEVREADWRITE("pio", z80pio_device, read_alt, write_alt)  // init bytes look like those for a Z80PIO
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mccpm )
INPUT_PORTS_END


void mccpm_state::machine_reset()
{
	uint8_t* bios = memregion("maincpu")->base();
	memcpy(m_p_ram, bios, 0x1000);
}

MACHINE_CONFIG_START(mccpm_state::mccpm)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL(4'000'000))
	MCFG_CPU_PROGRAM_MAP(mccpm_mem)
	MCFG_CPU_IO_MAP(mccpm_io)

	/* Devices */
	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("sio", z80sio_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio", z80sio_device, rxca_w))

	MCFG_DEVICE_ADD("sio", Z80SIO, XTAL(4'000'000))
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio", z80sio_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio", z80sio_device, ctsa_w))

	MCFG_DEVICE_ADD("pio", Z80PIO, XTAL(4'000'000))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mccpm )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v36", "V3.6")
	ROMX_LOAD( "mon36.j15",   0x0000, 0x1000, CRC(9c441537) SHA1(f95bad52d9392b8fc9d9b8779b7b861672a0022b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v34", "V3.4")
	ROMX_LOAD( "monhemc.bin", 0x0000, 0x1000, CRC(cae7b56e) SHA1(1f40be9491a595e6705099a452743cc0d49bfce8), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v34a", "V3.4 (alt)")
	ROMX_LOAD( "mc01mon.bin", 0x0000, 0x0d00, CRC(d1c89043) SHA1(f52a0ed3793dde0de74596be7339233b6a1770af), ROM_BIOS(3))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE        INIT  COMPANY                         FULLNAME            FLAGS
COMP( 1981, mccpm,  0,      0,       mccpm,     mccpm, mccpm_state, 0,    "GRAF Elektronik Systeme GmbH", "mc-CP/M-Computer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
