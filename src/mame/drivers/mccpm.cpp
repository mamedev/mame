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

private:
	void mccpm_io(address_map &map);
	void mccpm_mem(address_map &map);

	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_ram;
};


void mccpm_state::mccpm_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram().share("ram");
}

void mccpm_state::mccpm_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf3).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0xf4, 0xf7).rw("pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));  // init bytes look like those for a Z80PIO
}

/* Input ports */
static INPUT_PORTS_START( mccpm )
INPUT_PORTS_END


void mccpm_state::machine_reset()
{
	uint8_t* bios = memregion("maincpu")->base();
	memcpy(m_p_ram, bios, 0x1000);
}

void mccpm_state::mccpm(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &mccpm_state::mccpm_mem);
	m_maincpu->set_addrmap(AS_IO, &mccpm_state::mccpm_io);

	/* Devices */
	clock_device &uart_clock(CLOCK(config, "uart_clock", 153600));
	uart_clock.signal_handler().set("sio", FUNC(z80sio_device::txca_w));
	uart_clock.signal_handler().append("sio", FUNC(z80sio_device::rxca_w));

	z80sio_device& sio(Z80SIO(config, "sio", XTAL(4'000'000)));
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	sio.out_txda_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	sio.out_dtra_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	sio.out_rtsa_callback().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("sio", FUNC(z80sio_device::rxa_w));
	rs232.cts_handler().set("sio", FUNC(z80sio_device::ctsa_w));

	Z80PIO(config, "pio", XTAL(4'000'000));
}

/* ROM definition */
ROM_START( mccpm )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v36", "V3.6")
	ROMX_LOAD("mon36.j15",   0x0000, 0x1000, CRC(9c441537) SHA1(f95bad52d9392b8fc9d9b8779b7b861672a0022b), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v34", "V3.4")
	ROMX_LOAD("monhemc.bin", 0x0000, 0x1000, CRC(cae7b56e) SHA1(1f40be9491a595e6705099a452743cc0d49bfce8), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v34a", "V3.4 (alt)")
	ROMX_LOAD("mc01mon.bin", 0x0000, 0x0d00, CRC(d1c89043) SHA1(f52a0ed3793dde0de74596be7339233b6a1770af), ROM_BIOS(2))
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                         FULLNAME            FLAGS
COMP( 1981, mccpm, 0,      0,      mccpm,   mccpm, mccpm_state, empty_init, "GRAF Elektronik Systeme GmbH", "mc-CP/M-Computer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
