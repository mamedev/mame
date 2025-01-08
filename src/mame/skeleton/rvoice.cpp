// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Kevin Horton
/******************************************************************************
*
*  Bare bones Realvoice PC driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  Binary supplied by Kevin 'kevtris' Horton
*

******************************************************************************/

/* Core includes */
#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6801.h"
#include "machine/clock.h"
#include "machine/mos6551.h"

/* Components */


namespace {

class rvoice_state : public driver_device
{
public:
	rvoice_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void rvoicepc(machine_config &config);

	void init_rvoicepc();

private:
	required_device<hd6301y_cpu_device> m_maincpu;
	void hd63701_main_mem(address_map &map) ATTR_COLD;
};


/* Devices */

void rvoice_state::init_rvoicepc()
{
}


/******************************************************************************
 Address Maps
******************************************************************************/

void rvoice_state::hd63701_main_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x001e, 0x001e).nopr(); // FIXME: TRCSR2 needs implementation
	map(0x0034, 0x0037).rw("acia65c51", FUNC(mos6551_device::read), FUNC(mos6551_device::write)); // ACIA 65C51
	map(0x2000, 0x7fff).ram(); // EXTERNAL SRAM
	map(0x8000, 0xffff).rom(); // 27512 EPROM
}


/******************************************************************************
 Input Ports
******************************************************************************/
static INPUT_PORTS_START( rvoicepc )
	PORT_START("BAUD")
	PORT_DIPNAME(0x06, 0x06, "Baud Rate")
	PORT_DIPSETTING(0x00, "1200")
	PORT_DIPSETTING(0x02, "2400")
	PORT_DIPSETTING(0x04, "4800")
	PORT_DIPSETTING(0x06, "9600")
	PORT_DIPNAME(0x20, 0x00, "Data Bits")
	PORT_DIPSETTING(0x20, "7")
	PORT_DIPSETTING(0x00, "8")
	PORT_DIPNAME(0x80, 0x00, "Stop Bits")
	PORT_DIPSETTING(0x00, "1")
	PORT_DIPSETTING(0x80, "2")
	PORT_BIT(0x59, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("PARITY")
	PORT_DIPNAME(0x60, 0x00, "Parity")
	PORT_DIPSETTING(0x00, "None")
	PORT_DIPSETTING(0x20, "Odd")
	PORT_DIPSETTING(0x60, "Even")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x90, IP_ACTIVE_HIGH, IPT_UNUSED) // output pins
INPUT_PORTS_END

/******************************************************************************
 Machine Drivers
******************************************************************************/

void rvoice_state::rvoicepc(machine_config &config)
{
	/* basic machine hardware */
	HD6303Y(config, m_maincpu, XTAL(7'372'800));
	m_maincpu->set_addrmap(AS_PROGRAM, &rvoice_state::hd63701_main_mem);
	m_maincpu->in_p5_cb().set_ioport("PARITY");
	m_maincpu->in_p6_cb().set_ioport("BAUD");

	//hd63701_cpu_device &playercpu(HD63701(config "playercpu", XTAL(7'372'800))); // not dumped yet
	//playercpu.set_addrmap(AS_PROGRAM, &rvoice_state::hd63701_slave_mem);
	//playercpu.set_addrmap(AS_PROGRAM, &rvoice_state::hd63701_slave_io);
	config.set_maximum_quantum(attotime::from_hz(60));

	mos6551_device &acia(MOS6551(config, "acia65c51", 0));
	acia.set_xtal(1.8432_MHz_XTAL);
	acia.irq_handler().set_inputline(m_maincpu, HD6301_IRQ1_LINE);
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	// HACK: some signal needs to be connected to Tin
	CLOCK(config, "testclock", 60).signal_handler().set_inputline(m_maincpu, M6801_TIN_LINE);

	/* video hardware */

	/* sound hardware */

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("acia65c51", FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set("acia65c51", FUNC(mos6551_device::write_dcd));
	rs232.dsr_handler().set("acia65c51", FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set("acia65c51", FUNC(mos6551_device::write_cts));
}



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(rvoicepc)

	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rv_pc.bin", 0x08000, 0x08000, CRC(4001cd5f) SHA1(d973c6e19e493eedd4f7216bc530ddb0b6c4921e))
	ROM_CONTINUE(0x8000, 0x8000) // first half of 27c512 rom is blank due to stupid address decoder circuit

ROM_END

} // anonymous namespace


/******************************************************************************
 Drivers
******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE   INPUT     CLASS         INIT           COMPANY                           FULLNAME        FLAGS
COMP( 1988?, rvoicepc, 0,      0,      rvoicepc, rvoicepc, rvoice_state, init_rvoicepc, "Adaptive Communication Systems", "Realvoice PC", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
