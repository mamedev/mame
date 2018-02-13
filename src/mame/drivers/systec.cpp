// license:BSD-3-Clause
// copyright-holders: Miodrag Milanovic, Robbbert
/***************************************************************************

        Systec Z80

        More data :
            http://www.hd64180-cpm.de/html/systecz80.html

        30/08/2010 Skeleton driver

        Systec Platine 1

        SYSTEC 155.1L
        EPROM 2764 CP/M LOADER 155 / 9600 Baud
        Z8400APS CPU
        Z8420APS PIO
        Z8430APS CTC
        Z8470APS DART

        Systec Platine 2

        SYSTEC 100.3B
        MB8877A FDC Controller
        FDC9229BT SMC 8608
        Z8410APS DMA
        Z8420APS PIO

        MB8877A Compatible FD1793

        2011-12-22 connected to a terminal [Robbbert]


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80sio.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"


class systec_state : public driver_device
{
public:
	systec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void systec(machine_config &config);
	void systec_io(address_map &map);
	void systec_mem(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};

ADDRESS_MAP_START(systec_state::systec_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(systec_state::systec_io)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x68, 0x6b) // fdc?
	AM_RANGE(0x6c, 0x6c) // motor control?
	AM_RANGE(0xc4, 0xc7) AM_DEVREADWRITE("sio", z80sio_device, cd_ba_r, cd_ba_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( systec )
INPUT_PORTS_END

void systec_state::machine_reset()
{
	uint8_t *m_p_maincpu = memregion("maincpu")->base();
	uint8_t *m_p_roms = memregion("roms")->base();
	memcpy(m_p_maincpu, m_p_roms, 0x2000);
}


MACHINE_CONFIG_START(systec_state::systec)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL(16'000'000) / 4)
	MCFG_CPU_PROGRAM_MAP(systec_mem)
	MCFG_CPU_IO_MAP(systec_io)

	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("sio", z80sio_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio", z80sio_device, rxca_w))

	/* Devices */
	MCFG_DEVICE_ADD("sio", Z80SIO, XTAL(4'000'000))
	//MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))  // no evidence of a daisy chain because IM2 is not set
	MCFG_Z80SIO_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio", z80sio_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio", z80sio_device, ctsa_w))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( systec )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_REGION( 0x10000, "roms", 0 )
	ROM_LOAD( "systec.bin",   0x0000, 0x2000, CRC(967108ab) SHA1(a414db032ca7db0f9fdbe22aa68a099a93efb593))
ROM_END

/* Driver */

//   YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   STATE         INIT  COMPANY   FULLNAME      FLAGS
COMP(19??, systec, 0,      0,       systec,  systec, systec_state, 0,    "Systec", "Systec Z80", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
