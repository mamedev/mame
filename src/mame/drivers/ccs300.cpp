// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        CCS Model 300

        2009-12-11 Skeleton driver.

It requires a floppy disk to boot from.

There's no info available on this system, however the bankswitching
appears to be the same as their other systems.

Early on, it does a read from port F2. If bit 3 is low, the system becomes
a Model 400.

Since IM2 is used, it is assumed there are Z80 peripherals on board.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"


class ccs300_state : public driver_device
{
public:
	ccs300_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void init_ccs300();
	DECLARE_MACHINE_RESET(ccs300);
	DECLARE_WRITE8_MEMBER(port40_w);

	void ccs300(machine_config &config);
	void ccs300_io(address_map &map);
	void ccs300_mem(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};

void ccs300_state::ccs300_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).bankr("bankr0").bankw("bankw0");
	map(0x0800, 0xffff).ram();
}

void ccs300_state::ccs300_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x10, 0x13).rw("sio", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x14, 0x17).rw("pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));  // init bytes seem to be for a PIO
	map(0x18, 0x1b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));  // init bytes seem to be for a CTC
	map(0x30, 0x33); // fdc?
	map(0x34, 0x34); // motor control?
	map(0x40, 0x40).w(this, FUNC(ccs300_state::port40_w));
	map(0xf0, 0xf0); // unknown, long sequence of init bytes
	map(0xf2, 0xf2); // dip or jumper?
}

/* Input ports */
static INPUT_PORTS_START( ccs300 )
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "sio" },
	{ "pio" },
	{ nullptr }
};

//*************************************
//
//  Machine
//
//*************************************
WRITE8_MEMBER( ccs300_state::port40_w )
{
	membank("bankr0")->set_entry( (data) ? 1 : 0);
}

MACHINE_RESET_MEMBER( ccs300_state, ccs300 )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
}

void ccs300_state::init_ccs300()
{
	uint8_t *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

// bit 7 needs to be stripped off, we do this by choosing 7 bits and even parity
static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

MACHINE_CONFIG_START(ccs300_state::ccs300)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu",Z80, XTAL(4'000'000))
	MCFG_DEVICE_PROGRAM_MAP(ccs300_mem)
	MCFG_DEVICE_IO_MAP(ccs300_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)

	MCFG_MACHINE_RESET_OVERRIDE(ccs300_state, ccs300)

	/* video hardware */
	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE("sio", z80sio_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("sio", z80sio_device, rxca_w))

	/* Devices */
	MCFG_DEVICE_ADD("sio", Z80SIO, XTAL(4'000'000))
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO_OUT_TXDA_CB(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(WRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(WRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("sio", z80sio_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(WRITELINE("sio", z80sio_device, ctsa_w))
	MCFG_SLOT_OPTION_DEVICE_INPUT_DEFAULTS("terminal", terminal) // must be exactly here

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL(4'000'000))
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("pio", Z80PIO, XTAL(4'000'000))
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ccs300 )
	ROM_REGION( 0x10800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ccs300.rom", 0x10000, 0x0800, CRC(6cf22e31) SHA1(9aa3327cd8c23d0eab82cb6519891aff13ebe1d0))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY                        FULLNAME         FLAGS */
COMP( 19??, ccs300, ccs2810, 0,      ccs300,  ccs300, ccs300_state, init_ccs300, "California Computer Systems", "CCS Model 300", MACHINE_IS_SKELETON )
