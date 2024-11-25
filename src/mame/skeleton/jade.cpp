// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Single board Z80 computer on a S100 card (not the JGZ80).
    The Jade SPIO board adds four CTCs, two SIOs and one PIO.

    2013-09-12 Skeleton driver.

    No info found as yet.

    Type HE to get a list of commands.

    If you hit a key as soon as it is started, the system will present a
    prompt and work. Otherwise it runs into the weeds because the rom banking
    isn't yet emulated.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"


namespace {

class jade_state : public driver_device
{
public:
	jade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void jade(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void jade_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom().region("roms", 0);
	map(0x0800, 0xffff).ram();
}

void jade_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x28, 0x2b).rw("ctc2", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x30, 0x33).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x40, 0x43).rw("ctc1", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

/* Input ports */
static INPUT_PORTS_START( jade )
INPUT_PORTS_END


void jade_state::jade(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &jade_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &jade_state::io_map);

	Z80CTC(config, "ctc1", 4_MHz_XTAL);

	z80ctc_device &ctc2(Z80CTC(config, "ctc2", 4_MHz_XTAL));
	ctc2.set_clk<0>(4_MHz_XTAL / 2);
	ctc2.zc_callback<0>().set("sio", FUNC(z80sio_device::rxca_w));
	ctc2.zc_callback<0>().append("sio", FUNC(z80sio_device::txca_w));

	/* Devices */
	z80sio_device& sio(Z80SIO(config, "sio", 4_MHz_XTAL));
	//sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0); // no evidence of a daisy chain because IM2 is not set
	sio.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));
}

/* ROM definition */
ROM_START( jade )
	ROM_REGION( 0x800, "roms", 0 )
	ROM_LOAD( "jade.rom", 0x0000, 0x0800, CRC(90c4a1ef) SHA1(8a93a11051cc27f3edca24f0f4297ebe0099964e) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                   FULLNAME                                                            FLAGS
COMP( 1983, jade, 0,      0,      jade,    jade,  jade_state, empty_init, "Jade Computer Products", "unknown S-100 computer with Serial/Parallel/Interrupt Controller", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
