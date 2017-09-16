// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/********************************************************************************************************************

2013-07-31 Skeleton Driver [Curt Coder]
2013-07-31 Connected to terminal [Robbbert]
2016-07-11 After 10 seconds the monitor program will start [Robbbert]

Commands: (no spaces allowed)
B - Boot the disk
D - Dump memory to screen
F - Fill Memory
G - Go To
H - Help
P - Alter port values
S - Alter memory


The photos show 3 boards:
- A scsi board (all 74-series TTL)
- CPU board (64k dynamic RAM, Z80A CPU, 2x Z80CTC, 2x Z80SIO/0, MB8877A, Z80DMA, 4x MC1488,
  4x MC1489, XTALS 1.8432MHz and 24MHz)
- ADES board (Adaptec Inc AIC-100, AIC-250, AIC-300, Intel D8086AH, unknown crystal)

Both roms contain Z80 code.


********************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/clock.h"
#include "bus/rs232/rs232.h"


class dsb46_state : public driver_device
{
public:
	dsb46_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sio(*this, "sio")
	{ }

	DECLARE_WRITE_LINE_MEMBER(clock_tick);
	DECLARE_WRITE8_MEMBER(port1a_w);
	DECLARE_DRIVER_INIT(dsb46);
	DECLARE_MACHINE_RESET(dsb46);

private:
	required_device<cpu_device> m_maincpu;
	required_device<z80sio_device> m_sio;
};

static ADDRESS_MAP_START( dsb46_mem, AS_PROGRAM, 8, dsb46_state )
	AM_RANGE(0x0000, 0x07ff) AM_READ_BANK("read") AM_WRITE_BANK("write")
	AM_RANGE(0x0800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dsb46_io, AS_IO, 8, dsb46_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("sio", z80sio_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ctc1", z80ctc_device, read, write)
	AM_RANGE(0x1a, 0x1a) AM_WRITE(port1a_w)
	//AM_RANGE(0x10, 0x10) disk related
	//AM_RANGE(0x14, 0x14) ??
	//AM_RANGE(0x18, 0x18) ??
	//AM_RANGE(0x1c, 0x1c) disk data
	//AM_RANGE(0x1d, 0x1d) disk status (FF = no fdc)
ADDRESS_MAP_END

static INPUT_PORTS_START( dsb46 )
INPUT_PORTS_END

DRIVER_INIT_MEMBER(dsb46_state, dsb46)
{
	uint8_t *RAM = memregion("maincpu")->base();
	membank("read")->configure_entry(0, &RAM[0x10000]);
	membank("read")->configure_entry(1, &RAM[0x00000]);
	membank("write")->configure_entry(0, &RAM[0x00000]);
}

MACHINE_RESET_MEMBER( dsb46_state,dsb46 )
{
	membank("read")->set_entry(0);
	membank("write")->set_entry(0);
	m_maincpu->reset();
}

WRITE8_MEMBER( dsb46_state::port1a_w )
{
	membank("read")->set_entry(data & 1);
}

// source of baud frequency is unknown, so we invent a clock
WRITE_LINE_MEMBER( dsb46_state::clock_tick )
{
	m_sio->txca_w(state);
	m_sio->rxca_w(state);
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc1" },
	{ "sio" },
	{ nullptr }
};


static MACHINE_CONFIG_START( dsb46 )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(dsb46_mem)
	MCFG_CPU_IO_MAP(dsb46_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)

	MCFG_MACHINE_RESET_OVERRIDE(dsb46_state, dsb46)

	/* video hardware */
	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(dsb46_state, clock_tick))

	/* Devices */
	MCFG_Z80SIO_ADD("sio", XTAL_4MHz, 0, 0, 0, 0)
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80SIO_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80SIO_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80SIO_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("sio", z80sio_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("sio", z80sio_device, ctsa_w))

	MCFG_DEVICE_ADD("ctc1", Z80CTC, XTAL_4MHz)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
MACHINE_CONFIG_END

ROM_START( dsb46 )
	ROM_REGION( 0x10800, "maincpu", 0 )
	ROM_LOAD( "1538a.bin", 0x10000, 0x800, CRC(65b3e26e) SHA1(afe1f03f266b7d13fdb1f1bc6762df5e0aa5c764) )

	ROM_REGION( 0x4000, "ades", 0 )
	ROM_LOAD( "ades.bin", 0x0000, 0x4000, CRC(d374abf0) SHA1(331f51a2bb81375aeffbe63c1ebc1d7cd779b9c3) )
ROM_END

COMP( 198?, dsb46, 0, 0, dsb46, dsb46, dsb46_state, dsb46, "Davidge", "DSB-4/6",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
