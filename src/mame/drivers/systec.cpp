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

private:
	void systec_io(address_map &map);
	void systec_mem(address_map &map);

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


void systec_state::systec(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(16'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &systec_state::systec_mem);
	m_maincpu->set_addrmap(AS_IO, &systec_state::systec_io);

	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set("sio", FUNC(z80sio_device::txca_w));
	uart_clock.signal_handler().append("sio", FUNC(z80sio_device::rxca_w));

	/* Devices */
	z80sio_device& sio(Z80SIO(config, "sio", XTAL(4'000'000)));
	//sio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0); // no evidence of a daisy chain because IM2 is not set
	sio.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
}


/* ROM definition */
ROM_START( systec )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_REGION( 0x10000, "roms", 0 )
	ROM_LOAD( "systec.bin",   0x0000, 0x2000, CRC(967108ab) SHA1(a414db032ca7db0f9fdbe22aa68a099a93efb593))
ROM_END

/* Driver */

//   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY   FULLNAME      FLAGS
COMP(19??, systec, 0,      0,      systec,  systec, systec_state, empty_init, "Systec", "Systec Z80", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
