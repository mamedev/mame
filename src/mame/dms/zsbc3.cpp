// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Digital Microsystems ZSBC-3

        11/01/2010 Skeleton driver.
        28/11/2010 Connected to a terminal
        11/01/2011 Converted to Modern.

****************************************************************************

        Monitor commands: [] indicates optional

        Bx = Boot from device x (0 to 7)
        Dx [y] = Dump memory (hex and ascii) in range x [to y]
        Fx y z = Fill memory x to y with z
        Gx = Execute program at address x
        Ix = Display IN of port x
        Ox y = Output y to port x
        Sx = Enter memory editing mode, press enter for next address
        Mx y = unknown (affects memory)
        Tx = unknown (does strange things)
        enter = dump memory from 9000 to 907F (why?)

****************************************************************************

        TODO:
        Everything really...

        Devices of all kinds;
        Use the other rom for something..

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/z80pio.h"
#include "bus/rs232/rs232.h"


namespace {

class zsbc3_state : public driver_device
{
public:
	zsbc3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void zsbc3(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void zsbc3_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0xffff).ram();
}

void zsbc3_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x08, 0x0b); //.rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write)); // the control bytes appear to be for a PIO
	map(0x28, 0x2b).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x30, 0x33).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x38, 0x38); // unknown device, init byte = C3
}

/* Input ports */
static INPUT_PORTS_START( zsbc3 )
INPUT_PORTS_END


void zsbc3_state::zsbc3(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 16_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &zsbc3_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &zsbc3_state::io_map);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 16_MHz_XTAL / 4));
	ctc.set_clk<0>(16_MHz_XTAL / 8);
	ctc.set_clk<1>(16_MHz_XTAL / 8);
	ctc.set_clk<2>(16_MHz_XTAL / 8);
	ctc.set_clk<3>(16_MHz_XTAL / 8);
	ctc.zc_callback<0>().set("sio", FUNC(z80sio_device::txca_w));
	ctc.zc_callback<0>().append("sio", FUNC(z80sio_device::rxca_w));

	z80sio_device &sio(Z80SIO(config, "sio", 16_MHz_XTAL / 4));
	//sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);  // no evidence of a daisy chain because IM2 is not set
	sio.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));

	/*z80pio_device &pio(*/Z80PIO(config, "pio", 16_MHz_XTAL / 4)/*)*/;
	//pio.out_int_callback.set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}

/* ROM definition */
ROM_START( zsbc3 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "54-3002_zsbc_monitor_1.09.bin", 0x0000, 0x0800, CRC(628588e9) SHA1(8f0d489147ec8382ca007236e0a95a83b6ebcd86))

	ROM_REGION( 0x0400, "hdc", 0 )
	ROM_LOAD( "54-8622_hdc13.bin", 0x0000, 0x0400, CRC(02c7cd6d) SHA1(494281ba081a0f7fbadfc30a7d2ea18c59e55101))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  STATE        INIT        COMPANY                 FULLNAME  FLAGS */
COMP( 1980, zsbc3, 0,      0,      zsbc3,   zsbc3, zsbc3_state, empty_init, "Digital Microsystems", "ZSBC-3", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
