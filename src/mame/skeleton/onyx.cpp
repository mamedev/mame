// license:BSD-3-Clause
// copyright-holders:Robbbert
/***********************************************************************************************

Onyx C8002

2013-08-18 Skeleton Driver

The C8002 is one of the earliest minicomputers to use Unix as an operating system.

The system consists of a main CPU (Z8002), and a slave CPU for Mass Storage control (Z80)

The Z80 board contains a 19.6608 and 16 MHz crystals; 2x Z80CTC; 3x Z80SIO/0; Z80DMA; 3x Z80PIO;
2 eproms marked 459-3 and 460-3, plus 2 proms.

The Z8002 board contains a 16 MHz crystal; 3x Z80CTC; 5x Z80SIO/0; 3x Z80PIO; 2 eproms marked
466-E and 467E, plus the remaining 7 small proms.

The system can handle 8 RS232 terminals, 7 hard drives, a tape cartridge drive, parallel i/o,
and be connected to a RS422 network.

Status:
- Main screen prints an error with CTC (because there's no clock into it atm)
- Subcpu screen (after a while) prints various statuses then waits for the fdc to respond

To Do:
- Hook up daisy chains (see p8k.cpp for how to hook up a 16-bit chain)
  (keyboard input depends on interrupts)
- Remaining devices
- Whatever hooks up to the devices
- Eventually we'll need software
- Manuals / schematics would be nice

*************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z8000/z8000.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"
//#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
//#include "machine/z80dma.h"

class onyx_state : public driver_device
{
public:
	onyx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sio(*this, "sio%u", 1U)
		, m_ctc(*this, "ctc%u", 1U)
		, m_pio(*this, "pio%u", 1U)
	{ }

	void c8002(machine_config &config);
	void c5000(machine_config &config);
	void c5000_io(address_map &map) ATTR_COLD;

private:
	void c5000_mem(address_map &map) ATTR_COLD;
	void c8002_io(address_map &map) ATTR_COLD;
	void c8002_mem(address_map &map) ATTR_COLD;
	void subio(address_map &map) ATTR_COLD;
	void submem(address_map &map) ATTR_COLD;
	void z8002_m1_w(uint8_t data);

	required_device<cpu_device> m_maincpu; // z8002 or z80 depending on driver
	optional_device_array<z80sio_device, 5> m_sio;
	optional_device_array<z80ctc_device, 3> m_ctc;
	optional_device_array<z80pio_device, 2> m_pio;
};


/* Input ports */
static INPUT_PORTS_START( c8002 )
INPUT_PORTS_END


void onyx_state::c8002_mem(address_map &map)
{
	map(0x00000, 0x00fff).rom();
	map(0x01000, 0x07fff).ram();
	map(0x08000, 0x0ffff).ram(); // Z8002 has 64k memory
}

void onyx_state::z8002_m1_w(uint8_t data)
{
	// ED 4D (Z80 RETI opcode) is written here
	for (auto &sio : m_sio)
		sio->z80daisy_decode(data);
	for (auto &ctc : m_ctc)
		ctc->z80daisy_decode(data);
	for (auto &pio : m_pio)
		pio->z80daisy_decode(data);
}

void onyx_state::c8002_io(address_map &map)
{
	map(0xff00, 0xff07).rw(m_sio[0], FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w)).umask16(0x00ff);
	map(0xff08, 0xff0f).rw(m_sio[1], FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w)).umask16(0x00ff);
	map(0xff10, 0xff17).rw(m_sio[2], FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w)).umask16(0x00ff);
	map(0xff18, 0xff1f).rw(m_sio[3], FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w)).umask16(0x00ff);
	map(0xff20, 0xff27).rw(m_sio[4], FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w)).umask16(0x00ff);
	map(0xff30, 0xff37).rw(m_ctc[0], FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)).umask16(0x00ff);
	map(0xff38, 0xff3f).rw(m_ctc[1], FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)).umask16(0x00ff);
	map(0xff40, 0xff47).rw(m_ctc[2], FUNC(z80ctc_device::read), FUNC(z80ctc_device::write)).umask16(0x00ff);
	map(0xff50, 0xff57).rw(m_pio[0], FUNC(z80pio_device::read), FUNC(z80pio_device::write)).umask16(0x00ff);
	map(0xff58, 0xff5f).rw(m_pio[1], FUNC(z80pio_device::read), FUNC(z80pio_device::write)).umask16(0x00ff);
	map(0xffb9, 0xffb9).w(FUNC(onyx_state::z8002_m1_w));
}

void onyx_state::submem(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0xffff).ram();
}

void onyx_state::subio(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("pio1s", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x04, 0x04).nopr();   // disk status?
	map(0x0c, 0x0f).rw("sio1s", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
}

/***************************************************************************

    Machine Drivers

****************************************************************************/

void onyx_state::c8002(machine_config &config)
{
	/* basic machine hardware */
	z8002_device& maincpu(Z8002(config, m_maincpu, XTAL(4'000'000)));
	//maincpu.set_daisy_config(main_daisy_chain);
	maincpu.set_addrmap(AS_PROGRAM, &onyx_state::c8002_mem);
	//maincpu.set_addrmap(AS_DATA, &onyx_state::c8002_data);
	maincpu.set_addrmap(AS_IO, &onyx_state::c8002_io);

	z80_device& subcpu(Z80(config, "subcpu", XTAL(4'000'000)));
	//subcpu.set_daisy_config(sub_daisy_chain);
	subcpu.set_addrmap(AS_PROGRAM, &onyx_state::submem);
	subcpu.set_addrmap(AS_IO, &onyx_state::subio);

	clock_device &sio1_clock(CLOCK(config, "sio1_clock", 307200));
	sio1_clock.signal_handler().set(m_sio[0], FUNC(z80sio_device::rxca_w));
	sio1_clock.signal_handler().append(m_sio[0], FUNC(z80sio_device::txca_w));

	/* peripheral hardware */
	Z80PIO(config, m_pio[0], XTAL(16'000'000)/4);
	//m_pio[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	Z80PIO(config, m_pio[1], XTAL(16'000'000)/4);
	//m_pio[1]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	Z80CTC(config, m_ctc[0], XTAL(16'000'000) /4);
	Z80CTC(config, m_ctc[1], XTAL(16'000'000) /4);
	Z80CTC(config, m_ctc[2], XTAL(16'000'000) /4);
	Z80SIO(config, m_sio[0], XTAL(16'000'000) /4);
	m_sio[0]->out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_sio[0]->out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_sio[0]->out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	Z80SIO(config, m_sio[1], XTAL(16'000'000) /4);
	Z80SIO(config, m_sio[2], XTAL(16'000'000) /4);
	Z80SIO(config, m_sio[3], XTAL(16'000'000) /4);
	Z80SIO(config, m_sio[4], XTAL(16'000'000) /4);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_sio[0], FUNC(z80sio_device::rxa_w));
	rs232.dcd_handler().set(m_sio[0], FUNC(z80sio_device::dcda_w));
	rs232.cts_handler().set(m_sio[0], FUNC(z80sio_device::ctsa_w));

	Z80PIO(config, "pio1s", XTAL(16'000'000)/4);
	//z80pio_device& pio1s(Z80PIO(config, "pio1s", XTAL(16'000'000)/4));
	//pio1s->out_int_callback().set_inputline("subcpu", INPUT_LINE_IRQ0);

	clock_device &sio1s_clock(CLOCK(config, "sio1s_clock", 614400));
	sio1s_clock.signal_handler().set("sio1s", FUNC(z80sio_device::rxtxcb_w));
	//sio1s_clock.signal_handler().append("sio1s", FUNC(z80sio_device::txca_w));

	z80sio_device& sio1s(Z80SIO(config, "sio1s", XTAL(16'000'000) /4));
	sio1s.out_txdb_callback().set("rs232s", FUNC(rs232_port_device::write_txd));
	sio1s.out_dtrb_callback().set("rs232s", FUNC(rs232_port_device::write_dtr));
	sio1s.out_rtsb_callback().set("rs232s", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232s(RS232_PORT(config, "rs232s", default_rs232_devices, "terminal"));
	rs232s.rxd_handler().set("sio1s", FUNC(z80sio_device::rxb_w));
	rs232s.dcd_handler().set("sio1s", FUNC(z80sio_device::dcdb_w));
	rs232s.cts_handler().set("sio1s", FUNC(z80sio_device::ctsb_w));
}

/* ROM definition */
ROM_START( c8002 )
	ROM_REGION16_BE( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("466-e", 0x0001, 0x0800, CRC(13534bcb) SHA1(976c76c69af40b0c0a5038e428a10b39a619a036))
	ROM_LOAD16_BYTE("467-e", 0x0000, 0x0800, CRC(0d5b557f) SHA1(0802bc6c2774f4e7de38a9c92e8558d710eed287))

	ROM_REGION( 0x10000, "subcpu", 0 )
	ROM_LOAD("459-3",   0x0000, 0x0800, CRC(c8906653) SHA1(7dea9fffa974479ef5926df567261f2aaa7a3283))
	ROM_LOAD("460-3",   0x0800, 0x0800, CRC(ce6c0214) SHA1(f69ee4c6c0d1e72574a9cf828dbb3e08f06d029a))

	ROM_REGION( 0x900, "proms", 0 )
	// for main cpu
	ROM_LOAD("468-a",  0x000, 0x100, CRC(89781491) SHA1(f874d0cf42a733eb2b92b15647aeac7178d7b9b1))
	ROM_LOAD("469-a",  0x100, 0x100, CRC(45e439de) SHA1(4f1af44332ae709d92e919c9e48433f29df5e632))
	ROM_LOAD("470a-3", 0x200, 0x100, CRC(c50622a9) SHA1(deda0df93fc4e4b5f4be313e4bfe0c5fc669a024))
	ROM_LOAD("471-a",  0x300, 0x100, CRC(c09ca06b) SHA1(cb99172f5342427c68a109ee108a0c49b44e7010))
	ROM_LOAD("472-a",  0x400, 0x100, CRC(e1316fed) SHA1(41ed2d822c74da4e1ce06eb229629576e7f5035f))
	ROM_LOAD("473-a",  0x500, 0x100, CRC(5e8efd7f) SHA1(647064e0c3b0d795a333febc57228472b1b32345))
	ROM_LOAD("474-a",  0x600, 0x100, CRC(0052edfd) SHA1(b5d18c9a6adce7a6d627ece40a60aab8c55a6597))
	// for sub cpu
	ROM_LOAD("453-a",  0x700, 0x100, CRC(7bc3871e) SHA1(6f75eb04911fa1ff66714276b8a88be62438a1b0))
	ROM_LOAD("454-a",  0x800, 0x100, CRC(aa2233cd) SHA1(4ec3a8c06cccda02f080e89831ecd8a9c96d3650))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY         FULLNAME  FLAGS
COMP( 1982, c8002, 0,      0,      c8002,   c8002, onyx_state, empty_init, "Onyx Systems", "C8002",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )



/********************************************************************************************************************************

Onyx Systems C5000.
(says C8000 on the backplate)

Chips: 256k dynamic RAM, Z80A, Z80DMA, 5x Z80PIO, 2x Z80SIO/0, 2x Z80CTC, 5? undumped proms, 3 red leds, 1x 4-sw DIP
Crystals: 16.000000, 19.660800
Labels of proms: 339, 153, XMN4, 2_1, 1_2

*********************************************************************************************************************************/

void onyx_state::c5000_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0xffff).ram();
}

void onyx_state::c5000_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x10, 0x13).rw(m_sio[0], FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
}

void onyx_state::c5000(machine_config &config)
{
	/* basic machine hardware */
	z80_device& maincpu(Z80(config, m_maincpu, XTAL(16'000'000) / 4));
	//maincpu.set_daisy_config(sub_daisy_chain);
	maincpu.set_addrmap(AS_PROGRAM, &onyx_state::c5000_mem);
	maincpu.set_addrmap(AS_IO, &onyx_state::c5000_io);

	clock_device &sio1_clock(CLOCK(config, "sio1_clock", 614400));
	sio1_clock.signal_handler().set(m_sio[0], FUNC(z80sio_device::rxtxcb_w));
	//sio1_clock.signal_handler().append(m_sio[0], FUNC(z80sio_device::txca_w));

	/* peripheral hardware */
	//Z80PIO(config, m_pio[0], XTAL(16'000'000)/4);
	//m_pio[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	//Z80PIO(config, m_pio[1], XTAL(16'000'000)/4);
	//m_pio[1]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	//Z80CTC(config, m_ctc[0], XTAL(16'000'000) /4);
	//Z80CTC(config, m_ctc[1], XTAL(16'000'000) /4);
	//Z80CTC(config, m_ctc[2], XTAL(16'000'000) /4);

	Z80SIO(config, m_sio[0], XTAL(16'000'000) /4);
	m_sio[0]->out_txdb_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_sio[0]->out_dtrb_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_sio[0]->out_rtsb_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(m_sio[0], FUNC(z80sio_device::rxb_w));
	rs232.dcd_handler().set(m_sio[0], FUNC(z80sio_device::dcdb_w));
	rs232.cts_handler().set(m_sio[0], FUNC(z80sio_device::ctsb_w));

	//Z80SIO(config, m_sio[1], XTAL(16'000'000) /4);
}


ROM_START( c5000 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "860-3.prom1", 0x0000, 0x1000, CRC(31b52df3) SHA1(e221c7829b4805805cde1bde763bd5a936e7db1a) )
	ROM_LOAD( "861-3.prom2", 0x1000, 0x1000, CRC(d1eba182) SHA1(850035497975b821fc1e51fbb73642cba3ff9784) )
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT   MACHINE    INPUT  CLASS       INIT        COMPANY          FULLNAME  FLAGS
COMP( 1981, c5000, 0,      0,       c5000,     c8002, onyx_state, empty_init, "Onyx Systems",  "C5000",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
