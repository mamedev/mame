// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************************************

Datamax 8000

2016-07-24 Skeleton driver

This computer was designed and made by Datamax Pty Ltd, in Manly, Sydney, Australia.

It could have up to 4x 8 inch floppy drives, in the IBM format.
There were a bunch of optional extras such as 256kB RAM, numeric coprocessor, RTC, etc.


ToDo:
- Parallel port
- Centronics port
- AUX serial port
- FDC/FDD/HD setup - no schematics available of this section, so guessing...

What there is of the schematic shows no sign of a daisy chain or associated interrupts.


****************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/wd_fdc.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80ctc.h"
#include "machine/mm58274c.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"



class dmax8000_state : public driver_device
{
public:
	dmax8000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dart1(*this, "dart1")
		, m_dart2(*this, "dart2")
		, m_ctc (*this, "ctc")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{
	}

	DECLARE_DRIVER_INIT(dmax8000);
	DECLARE_MACHINE_RESET(dmax8000);
	DECLARE_WRITE8_MEMBER(port0c_w);
	DECLARE_WRITE8_MEMBER(port0d_w);
	DECLARE_WRITE8_MEMBER(port14_w);
	DECLARE_WRITE8_MEMBER(port40_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	WRITE_LINE_MEMBER(fdc_drq_w);
	WRITE_LINE_MEMBER(clock_w);

private:
	required_device<cpu_device> m_maincpu;
	required_device<z80dart_device> m_dart1;
	required_device<z80dart_device> m_dart2;
	required_device<z80ctc_device> m_ctc;
	required_device<fd1793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
};


WRITE_LINE_MEMBER( dmax8000_state::fdc_drq_w )
{
	if (state) printf("DRQ ");
}

WRITE8_MEMBER( dmax8000_state::port0c_w )
{
	printf("Port0c=%X\n", data);
	m_fdc->dden_w(BIT(data, 6));
	floppy_image_device *floppy = nullptr;
	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy)
	{
		floppy->mon_w(0);
		floppy->ss_w(0); // no idea
	}
}

WRITE8_MEMBER( dmax8000_state::port0d_w )
{
	printf("Port0d=%X\n", data);
}

WRITE8_MEMBER( dmax8000_state::port14_w )
{
	printf("Port14=%X\n", data);
}

WRITE8_MEMBER( dmax8000_state::port40_w )
{
	membank("bankr0")->set_entry(BIT(data, 0));
}

static ADDRESS_MAP_START(dmax8000_mem, AS_PROGRAM, 8, dmax8000_state)
	AM_RANGE(0x0000, 0x0fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(dmax8000_io, AS_IO, 8, dmax8000_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("fdc", fd1793_t, read, write)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("dart1", z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
	AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("pio1", z80pio_device, read, write) // fdd controls
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("pio2", z80pio_device, read, write) // centronics & parallel ports
	AM_RANGE(0x14, 0x17) AM_WRITE(port14_w) // control lines for the centronics & parallel ports
	//AM_RANGE(0x18, 0x19) AM_MIRROR(2) AM_DEVREADWRITE("am9511", am9512_device, read, write) // optional numeric coprocessor
	//AM_RANGE(0x1c, 0x1d) AM_MIRROR(2)  // optional hard disk controller (1C=status, 1D=data)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("dart2", z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x40, 0x40) AM_WRITE(port40_w) // memory bank control
	//AM_RANGE(0x60, 0x67) // optional IEEE488 GPIB
	AM_RANGE(0x70, 0x7f) AM_DEVREADWRITE("rtc", mm58274c_device, read, write) // optional RTC
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( dmax8000 )
INPUT_PORTS_END

MACHINE_RESET_MEMBER( dmax8000_state, dmax8000 )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	m_maincpu->reset();
	m_maincpu->set_input_line_vector(0, 0xee); // fdc vector
}

DRIVER_INIT_MEMBER( dmax8000_state, dmax8000 )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

// Baud rate generator. All inputs are 2MHz.
DECLARE_WRITE_LINE_MEMBER( dmax8000_state::clock_w )
{
	m_ctc->trg0(state);
	m_ctc->trg1(state);
	m_ctc->trg2(state);
}

WRITE_LINE_MEMBER( dmax8000_state::ctc_z0_w )
{
	m_dart1->rxca_w(state);
	m_dart1->txca_w(state);
	m_dart2->rxca_w(state);
	m_dart2->txca_w(state);
}

static SLOT_INTERFACE_START( floppies )
	SLOT_INTERFACE( "drive0", FLOPPY_8_DSDD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( dmax8000, dmax8000_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz ) // no idea what crystal is used, but 4MHz clock is confirmed
	MCFG_CPU_PROGRAM_MAP(dmax8000_mem)
	MCFG_CPU_IO_MAP(dmax8000_io)
	MCFG_MACHINE_RESET_OVERRIDE(dmax8000_state, dmax8000)

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, XTAL_4MHz / 2) // 2MHz
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(dmax8000_state, clock_w))

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_4MHz)
	MCFG_Z80CTC_ZC0_CB(WRITELINE(dmax8000_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE("dart2", z80dart_device, rxtxcb_w))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE("dart1", z80dart_device, rxtxcb_w))

	MCFG_Z80DART_ADD("dart1",  XTAL_4MHz, 0, 0, 0, 0 ) // A = terminal; B = aux
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("dart1", z80dart_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("dart1", z80dart_device, dcda_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("dart1", z80dart_device, ria_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("dart1", z80dart_device, ctsa_w))

	MCFG_Z80DART_ADD("dart2",  XTAL_4MHz, 0, 0, 0, 0 ) // RS232 ports

	MCFG_DEVICE_ADD("pio1", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(dmax8000_state, port0c_w))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(dmax8000_state, port0d_w))

	MCFG_DEVICE_ADD("pio2", Z80PIO, XTAL_4MHz)

	MCFG_FD1793_ADD("fdc", XTAL_2MHz) // no idea
	MCFG_WD_FDC_INTRQ_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(dmax8000_state, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", floppies, "drive0", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_DEVICE_ADD("rtc", MM58274C, 0) // MM58174
	// this is all guess
	MCFG_MM58274C_MODE24(0) // 12 hour
	MCFG_MM58274C_DAY1(1)   // monday
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( dmax8000 )
	ROM_REGION( 0x11000, "maincpu", 0 )
	ROM_LOAD( "rev1_0.rom", 0x10000, 0x001000, CRC(acbec83f) SHA1(fce0a4307a791250dbdc6bb6a190f7fec3619d82) )
	// ROM_LOAD( "rev1_1.rom", 0x10000, 0x001000, CRC(2eb98a61) SHA1(cdd9a58f63ee7e3d3dd1c4ae3fd4376b308fd10f) )  // this is a hacked rom to speed up the serial port
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT   COMPAT  MACHINE     INPUT     CLASS            INIT      COMPANY      FULLNAME       FLAGS */
COMP( 1981, dmax8000,  0,       0,      dmax8000,   dmax8000, dmax8000_state, dmax8000, "Datamax", "Datamax 8000", MACHINE_NOT_WORKING )
