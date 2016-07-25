// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************************

Datamax 8000

2016-07-24 Skeleton driver

This computer was designed and made by Datamax Pty Ltd, in Manly, Sydney, Australia.

It could have up to 4x 8 inch floppy drives, in the IBM format.
There were a bunch of optional extras such as 256kB RAM, numeric coprocessor, RTC, etc.


ToDo:
- Everything... there are partial schematics, but there's still a lot of guesswork.




****************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/wd_fdc.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80ctc.h"
#include "machine/terminal.h"



class dmax8000_state : public driver_device
{
public:
	dmax8000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_fdc(*this, "fdc")
	{
	}

	DECLARE_DRIVER_INIT(dmax8000);
	DECLARE_MACHINE_RESET(dmax8000);
	DECLARE_READ8_MEMBER(port80_r);
	DECLARE_READ8_MEMBER(port81_r);
	DECLARE_WRITE8_MEMBER(port40_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<fd1793_t> m_fdc;
};


WRITE8_MEMBER( dmax8000_state::port40_w )
{
	membank("bankr0")->set_entry(BIT(data, 0));
}

READ8_MEMBER( dmax8000_state::port80_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( dmax8000_state::port81_r )
{
	return (m_term_data) ? 13 : 12;
}

static ADDRESS_MAP_START(dmax8000_mem, AS_PROGRAM, 8, dmax8000_state)
	AM_RANGE(0x0000, 0x0fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(dmax8000_io, AS_IO, 8, dmax8000_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("fdc", fd1793_t, read, write)
	//AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("dart1", z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("pio1", z80pio_device, read, write)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("pio2", z80pio_device, read, write)
	//AM_RANGE(0x14, 0x17) AM_WRITE(port14_r)
	//AM_RANGE(0x18, 0x19) AM_MIRROR(2) AM_DEVREADWRITE("am9512", am9512_device, read, write) // optional numeric coprocessor
	//AM_RANGE(0x1c, 0x1d) AM_MIRROR(2)  // optional hard disk controller (1C=status, 1D=data)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("dart2", z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(port40_w) // memory bank control
	//AM_RANGE(0x60, 0x67) // optional IEEE488 GPIB
	//AM_RANGE(0x70, 0x7f) AM_DEVREADWRITE("rtc", mm58274c_device, read, write) // optional RTC
	AM_RANGE(0x04, 0x04) AM_READ(port80_r) AM_DEVWRITE("terminal", generic_terminal_device, write)
	AM_RANGE(0x05, 0x05) AM_READ(port81_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( dmax8000 )
INPUT_PORTS_END

/* Z80 Daisy Chain */

// order unknown
static const z80_daisy_config daisy_chain[] =
{
	{ "pio1" },
	{ "pio2" },
	{ "dart1" },
	{ "dart2" },
	{ "ctc" },
	{ nullptr }
};

/* Z80-CTC Interface */

WRITE_LINE_MEMBER( dmax8000_state::ctc_z0_w )
{
// guess this generates clock for z80dart
}

WRITE_LINE_MEMBER( dmax8000_state::ctc_z1_w )
{
}

WRITE_LINE_MEMBER( dmax8000_state::ctc_z2_w )
{
}

/* after the first 4 bytes have been read from ROM, switch the ram back in */
MACHINE_RESET_MEMBER( dmax8000_state, dmax8000 )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
}

DRIVER_INIT_MEMBER( dmax8000_state, dmax8000 )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

static SLOT_INTERFACE_START( floppies )
	SLOT_INTERFACE( "drive0", FLOPPY_8_DSDD )
SLOT_INTERFACE_END

WRITE8_MEMBER( dmax8000_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( dmax8000, dmax8000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz ) // no idea what crystal is used, but 4MHz clock is confirmed
	MCFG_CPU_PROGRAM_MAP(dmax8000_mem)
	MCFG_CPU_IO_MAP(dmax8000_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)
	MCFG_MACHINE_RESET_OVERRIDE(dmax8000_state, dmax8000)

	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(dmax8000_state, kbd_put))

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_4MHz)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(dmax8000_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(dmax8000_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(dmax8000_state, ctc_z2_w))

	MCFG_Z80DART_ADD("dart1",  XTAL_4MHz, 0, 0, 0, 0 )
	//MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	//MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	//MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	//MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart", z80dart_device, write_rxd))
	//MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart", z80dart_device, write_dsr))
	//MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart", z80dart_device, write_cts))
	//MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", dmax8000 )

	MCFG_Z80DART_ADD("dart2",  XTAL_4MHz, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("pio1", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_DEVICE_ADD("pio2", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_FD1793_ADD("fdc", XTAL_2MHz)
	//MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(dmax8000_state, fdc_intrq_w))
	//MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(dmax8000_state, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", floppies, "drive0", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( dmax8000 )
	ROM_REGION( 0x11000, "maincpu", 0 )
	ROM_LOAD( "rev1_0.rom", 0x10000, 0x001000, CRC(acbec83f) SHA1(fce0a4307a791250dbdc6bb6a190f7fec3619d82) )
	// ROM_LOAD( "rev1_1.rom", 0x10000, 0x001000, CRC(2eb98a61) SHA1(cdd9a58f63ee7e3d3dd1c4ae3fd4376b308fd10f) )  // this is a hacked rom to speed up the serial port
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT   COMPAT  MACHINE     INPUT     CLASS            INIT      COMPANY      FULLNAME       FLAGS */
COMP( 1981, dmax8000,  0,       0,      dmax8000,   dmax8000, dmax8000_state, dmax8000, "Datamax", "Datamax 8000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
