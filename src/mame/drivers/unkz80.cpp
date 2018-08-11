// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Unknown Z80 (with Serial Parallel Interrupt Controller)

Single board Z80 computer on a S100 card.
The SPIO board adds four CTCs, two SIOs and one PIO.

2013-09-12 Skeleton driver.

No info found as yet.

It should display P-Mon 4b 08/29/83 SPIC, then pause a bit,
then show a # prompt. Type HE to get a list of commands.

Currently does nothing due to a SIO regression.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"


class unkz80_state : public driver_device
{
public:
	unkz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void unkz80(machine_config &config);

private:
	void io_map(address_map &map);
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};


void unkz80_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom().region("roms", 0);
	map(0x0800, 0xffff).ram();
}

void unkz80_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x28, 0x2b).rw("ctc2", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x30, 0x33).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x40, 0x43).rw("ctc1", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

/* Input ports */
static INPUT_PORTS_START( unkz80 )
INPUT_PORTS_END


MACHINE_CONFIG_START(unkz80_state::unkz80)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu",Z80, 4_MHz_XTAL)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	Z80CTC(config, "ctc1", 4_MHz_XTAL);

	z80ctc_device &ctc2(Z80CTC(config, "ctc2", 4_MHz_XTAL));
	ctc2.zc_callback<0>().set("sio", FUNC(z80sio_device::rxca_w));
	ctc2.zc_callback<0>().append("sio", FUNC(z80sio_device::txca_w));

	CLOCK(config, "trg0", 4_MHz_XTAL / 2).signal_handler().set("ctc2", FUNC(z80ctc_device::trg0));

	/* Devices */
	MCFG_DEVICE_ADD("sio", Z80SIO, 4_MHz_XTAL)
	MCFG_Z80SIO_OUT_TXDA_CB(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(WRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(WRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio", z80sio_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio", z80sio_device, ctsa_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( unkz80 )
	ROM_REGION( 0x800, "roms", 0 )
	ROM_LOAD( "unkz80.rom",   0x0000, 0x0800, CRC(90c4a1ef) SHA1(8a93a11051cc27f3edca24f0f4297ebe0099964e) )
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                   FULLNAME  FLAGS
COMP( 1983, unkz80, 0,      0,      unkz80,    unkz80,  unkz80_state, empty_init, "<unknown>", "Unknown Z80 computer",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
