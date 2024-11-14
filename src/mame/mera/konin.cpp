// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Mera-Elzab Konin

It's an industrial computer used in Poland

No information has been found. All code is guesswork.

2011-12-29 Skeleton driver.
2016-07-15 Added terminal and uart.

Press E to see some messages.

Terminal settings: 8 data bits, 2 stop bits, no parity @ 9600

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8212.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "bus/rs232/rs232.h"


namespace {

class konin_state : public driver_device
{
public:
	konin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_picu(*this, "picu")
		, m_ioppi(*this, "ioppi")
		, m_iopit(*this, "iopit")
	{ }

	void konin(machine_config &config);

private:
	void picu_r3_w(int state);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_device<i8214_device> m_picu;
	required_device<i8255_device> m_ioppi;
	required_device<pit8253_device> m_iopit;
};

void konin_state::picu_r3_w(int state)
{
	m_picu->r_w(4, !state);
}

void konin_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x4fff).rom();
	map(0x5000, 0x7fff).ram();
	map(0xf200, 0xf200).nopw(); // watchdog?
	map(0xf400, 0xfbff).ram();
	map(0xfc80, 0xfc83).rw("mainppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xfc84, 0xfc87).rw("mainpit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xff00, 0xffff).ram();
}

void konin_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x24, 0x24).w(m_picu, FUNC(i8214_device::b_w));
	map(0x80, 0x83).lrw8(
		NAME([this](offs_t offset) { return m_ioppi->read(offset^3); }),
		NAME([this](offs_t offset, u8 data) { m_ioppi->write(offset^3, data); }));
	map(0xf6, 0xf6).rw("uart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xf7, 0xf7).rw("uart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xf8, 0xfb).lrw8(
		NAME([this](offs_t offset) { return m_iopit->read(offset^3); }),
		NAME([this](offs_t offset, u8 data) { m_iopit->write(offset^3, data); }));
}

/* Input ports */
static INPUT_PORTS_START( konin )
INPUT_PORTS_END


void konin_state::machine_start()
{
}

void konin_state::konin(machine_config &config)
{
	/* basic machine hardware */
	i8080_cpu_device &maincpu(I8080(config, m_maincpu, XTAL(4'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &konin_state::mem_map);
	maincpu.set_addrmap(AS_IO, &konin_state::io_map);
	maincpu.out_inte_func().set(m_picu, FUNC(i8214_device::inte_w));
	maincpu.set_irq_acknowledge_callback("intlatch", FUNC(i8212_device::inta_cb));

	i8212_device &intlatch(I8212(config, "intlatch", 0));
	intlatch.md_rd_callback().set_constant(0);
	intlatch.di_rd_callback().set(m_picu, FUNC(i8214_device::vector_r));
	intlatch.int_wr_callback().set_inputline("maincpu", I8085_INTR_LINE);

	I8214(config, m_picu, XTAL(4'000'000));
	m_picu->int_wr_callback().set("intlatch", FUNC(i8212_device::stb_w));

	pit8253_device &mainpit(PIT8253(config, "mainpit", 0));
	// wild guess at UART clock and source
	mainpit.set_clk<0>(1536000);
	mainpit.out_handler<0>().set("uart", FUNC(i8251_device::write_txc));
	mainpit.out_handler<0>().append("uart", FUNC(i8251_device::write_rxc));

	I8255(config, "mainppi", 0);

	PIT8253(config, m_iopit, 0);

	I8255(config, m_ioppi, 0);

	i8251_device &uart(I8251(config, "uart", 0));
	uart.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	uart.rxrdy_handler().set(FUNC(konin_state::picu_r3_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("uart", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("uart", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("uart", FUNC(i8251_device::write_cts));
}

/* ROM definition */
ROM_START( konin )
	ROM_REGION( 0x5000, "maincpu", 0 )
	ROM_LOAD( "001.bin", 0x0000, 0x0800, CRC(0b13208a) SHA1(38ea17be591b729158d601c03bfd9954f32e0e67))
	ROM_LOAD( "008.bin", 0x0800, 0x0800, CRC(f003e407) SHA1(11f79ef3b90788cf627ee39705bbbd04dbf45f50))
	ROM_LOAD( "007.bin", 0x1000, 0x0800, CRC(3d390c03) SHA1(ac2fe31c065e8f630381d6cebd2eb58b403c1e02))
	ROM_LOAD( "006.bin", 0x1800, 0x0800, CRC(68c9732e) SHA1(f40a79719dca485a2db29be5c0c781f559c2551c))
	ROM_LOAD( "005.bin", 0x2000, 0x0800, CRC(14548ac4) SHA1(8987e528b3e479c4c5941366628f34f086d06838))
	ROM_LOAD( "004.bin", 0x2800, 0x0800, CRC(8a354cff) SHA1(24d9f1fb15458fc96f5265f79d54e030b68d9fc9))
	ROM_LOAD( "002.bin", 0x3000, 0x0800, CRC(791fb30d) SHA1(8dfbe0edb741e02cfdd138432999f89480b20471))
	ROM_LOAD( "003.bin", 0x3800, 0x0800, CRC(27dc9864) SHA1(0d3da7fd1db895883c106f5133f8c7228333ecc8))
	ROM_LOAD( "009.bin", 0x4000, 0x0800, CRC(80947d15) SHA1(0757fb191913d79f306874684f9fc082ce18a28e))
	ROM_LOAD( "010.bin", 0x4800, 0x0800, CRC(f0157e0c) SHA1(60ace1eaf0ba01a45987c2286e18f3d56441c994))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY       FULLNAME  FLAGS
COMP( 198?, konin, 0,      0,      konin,   konin, konin_state, empty_init, "Mera-Elzab", "Konin",  MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )
