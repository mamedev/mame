// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Jade JGZ80 (with Serial Parallel Interrupt Controller)

    Single board Z80 computer on a S100 card.
    The SPIO board adds four CTCs, two SIOs and one PIO.

    2013-09-12 Skeleton driver.

    No info found as yet.

    It takes about 8 seconds to start up.
    Type HE to get a list of commands.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "bus/rs232/rs232.h"
//#include "bus/s100/s100.h"


class jade_state : public driver_device
{
public:
	jade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void jade(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};


ADDRESS_MAP_START(jade_state::mem_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x0800, 0xffff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(jade_state::io_map)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x28, 0x2b) AM_DEVREADWRITE("ctc2", z80ctc_device, read, write)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE("sio", z80sio_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE("ctc1", z80ctc_device, read, write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( jade )
INPUT_PORTS_END


MACHINE_CONFIG_START(jade_state::jade)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL(4'000'000))
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_DEVICE_ADD("ctc1", Z80CTC, XTAL(4'000'000))

	MCFG_DEVICE_ADD("ctc2", Z80CTC, XTAL(4'000'000))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("sio", z80sio_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio", z80sio_device, txca_w))

	MCFG_DEVICE_ADD("trg0", CLOCK, XTAL(4'000'000) / 2)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("ctc2", z80ctc_device, trg0))

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
ROM_START( jgz80 )
	ROM_REGION( 0x800, "roms", 0 )
	ROM_LOAD( "jgz80.rom",   0x0000, 0x0800, CRC(90c4a1ef) SHA1(8a93a11051cc27f3edca24f0f4297ebe0099964e) )
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT   MACHINE  INPUT  CLASS       INIT      COMPANY               FULLNAME   FLAGS
COMP( 1983, jgz80,   0,      0,       jade,    jade,  jade_state, 0,    "Jade Computer Products",  "JGZ80",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
