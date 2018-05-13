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

void systec_state::systec_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram().region("maincpu", 0);
}

void systec_state::systec_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x68, 0x6b); // fdc?
	map(0x6c, 0x6c); // motor control?
	map(0xc4, 0xc7).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
}

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
	MCFG_DEVICE_ADD("maincpu", Z80, XTAL(16'000'000) / 4)
	MCFG_DEVICE_PROGRAM_MAP(systec_mem)
	MCFG_DEVICE_IO_MAP(systec_io)

	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("sio", z80sio_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio", z80sio_device, rxca_w))

	/* Devices */
	MCFG_DEVICE_ADD("sio", Z80SIO, XTAL(4'000'000))
	//MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))  // no evidence of a daisy chain because IM2 is not set
	MCFG_Z80SIO_OUT_TXDA_CB(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(WRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(WRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio", z80sio_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio", z80sio_device, ctsa_w))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( systec )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_REGION( 0x10000, "roms", 0 )
	ROM_LOAD( "systec.bin",   0x0000, 0x2000, CRC(967108ab) SHA1(a414db032ca7db0f9fdbe22aa68a099a93efb593))
ROM_END

/* Driver */

//   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY   FULLNAME      FLAGS
COMP(19??, systec, 0,      0,      systec,  systec, systec_state, empty_init, "Systec", "Systec Z80", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
