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
#include "machine/clock.h"
#include "bus/rs232/rs232.h"


class zsbc3_state : public driver_device
{
public:
	zsbc3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void zsbc3(machine_config &config);
	void zsbc3_io(address_map &map);
	void zsbc3_mem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};


ADDRESS_MAP_START(zsbc3_state::zsbc3_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff ) AM_ROM
	AM_RANGE( 0x0800, 0xffff ) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(zsbc3_state::zsbc3_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x0b) //AM_DEVREADWRITE("pio", z80pio_device, read, write) // the control bytes appear to be for a PIO
	AM_RANGE(0x28, 0x2b) AM_DEVREADWRITE("sio", z80sio_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
	AM_RANGE(0x38, 0x38) // unknown device, init byte = C3
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( zsbc3 )
INPUT_PORTS_END


MACHINE_CONFIG_START(zsbc3_state::zsbc3)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL(16'000'000) / 4)
	MCFG_CPU_PROGRAM_MAP(zsbc3_mem)
	MCFG_CPU_IO_MAP(zsbc3_io)

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL(16'000'000) / 4)
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("sio", z80sio_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("sio", z80sio_device, rxca_w))

	MCFG_DEVICE_ADD("clk2mhz", CLOCK, XTAL(16'000'000) / 8)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("ctc", z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc", z80ctc_device, trg1))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc", z80ctc_device, trg2))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc", z80ctc_device, trg3))

	MCFG_DEVICE_ADD("sio", Z80SIO, XTAL(16'000'000) / 4)
	//MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))  // no evidence of a daisy chain because IM2 is not set
	MCFG_Z80SIO_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio", z80sio_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio", z80sio_device, ctsa_w))

	MCFG_DEVICE_ADD("pio", Z80PIO, XTAL(16'000'000) / 4)
	//MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( zsbc3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "54-3002_zsbc_monitor_1.09.bin", 0x0000, 0x0800, CRC(628588e9) SHA1(8f0d489147ec8382ca007236e0a95a83b6ebcd86))

	ROM_REGION( 0x10000, "hdc", ROMREGION_ERASEFF )
	ROM_LOAD( "54-8622_hdc13.bin", 0x0000, 0x0400, CRC(02c7cd6d) SHA1(494281ba081a0f7fbadfc30a7d2ea18c59e55101))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   STATE         INIT  COMPANY                  FULLNAME  FLAGS */
COMP( 1980, zsbc3,  0,      0,       zsbc3,     zsbc3,  zsbc3_state,  0,    "Digital Microsystems",  "ZSBC-3", MACHINE_NO_SOUND_HW)
