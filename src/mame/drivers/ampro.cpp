// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Ampro Little Z80 Board

2013-09-02 Skeleton driver.

Chips: Z80A @4MHz, Z80CTC, Z80DART, WD1770/2, NCR5380. Crystal: 16 MHz

This is a complete CP/M single-board computer that could be mounted
on top of a standard 13cm floppy drive. You needed to supply your own
power supply and serial terminal.

The later versions included a SCSI chip (NCR5380) enabling the use
of a hard drive of up to 88MB.

ToDo:
- (maybe) add scsi interface
- Add printer

****************************************************************************/

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/wd_fdc.h"
#include "softlist.h"

class ampro_state : public driver_device
{
public:
	ampro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dart(*this, "z80dart")
		, m_ctc (*this, "z80ctc")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{ }

	DECLARE_DRIVER_INIT(ampro);
	DECLARE_MACHINE_RESET(ampro);
	TIMER_DEVICE_CALLBACK_MEMBER(ctc_tick);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE8_MEMBER(port00_w);
	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
private:
	required_device<cpu_device> m_maincpu;
	required_device<z80dart_device> m_dart;
	required_device<z80ctc_device> m_ctc;
	required_device<wd1772_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
};

/*
d0..d3 Drive select 0-3 (we only emulate 1 drive)
d4     Side select 0=side0
d5     /DDEN
d6     Banking 0=rom
d7     FDC master clock 0=8MHz 1=16MHz (for 20cm disks, not emulated)
*/
WRITE8_MEMBER( ampro_state::port00_w )
{
	membank("bankr0")->set_entry(BIT(data, 6));
	m_fdc->dden_w(BIT(data, 5));
	floppy_image_device *floppy = nullptr;
	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy)
		floppy->ss_w(BIT(data, 4));
}

READ8_MEMBER( ampro_state::io_r )
{
	if (offset < 0x40)
		return m_ctc->read(space, offset>>4);
	else
		return m_dart->ba_cd_r(space, offset>>2);
}

WRITE8_MEMBER( ampro_state::io_w )
{
	if (offset < 0x40)
		m_ctc->write(space, offset>>4, data);
	else
		m_dart->ba_cd_w(space, offset>>2, data);
}

static ADDRESS_MAP_START(ampro_mem, AS_PROGRAM, 8, ampro_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ampro_io, AS_IO, 8, ampro_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(port00_w) // system
	//AM_RANGE(0x01, 0x01) AM_WRITE(port01_w) // printer data
	//AM_RANGE(0x02, 0x03) AM_WRITE(port02_w) // printer strobe
	//AM_RANGE(0x20, 0x27) AM_READWRITE() // scsi chip
	//AM_RANGE(0x28, 0x28) AM_WRITE(port28_w) // scsi control
	//AM_RANGE(0x29, 0x29) AM_READ(port29_r) // ID port
	AM_RANGE(0x40, 0x8f) AM_READWRITE(io_r,io_w)
	AM_RANGE(0xc0, 0xc3) AM_DEVWRITE("fdc", wd1772_t, write)
	AM_RANGE(0xc4, 0xc7) AM_DEVREAD("fdc", wd1772_t, read)
ADDRESS_MAP_END

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "z80ctc" },
	{ "z80dart" },
	{ nullptr }
};

// Baud rate generator. All inputs are 2MHz.
TIMER_DEVICE_CALLBACK_MEMBER( ampro_state::ctc_tick )
{
	m_ctc->trg0(1);
	m_ctc->trg0(0);
	m_ctc->trg1(1);
	m_ctc->trg1(0);
}

WRITE_LINE_MEMBER( ampro_state::ctc_z0_w )
{
	m_dart->rxca_w(state);
	m_dart->txca_w(state);
}

static SLOT_INTERFACE_START( ampro_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

/* Input ports */
static INPUT_PORTS_START( ampro )
INPUT_PORTS_END

MACHINE_RESET_MEMBER( ampro_state, ampro )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
}

DRIVER_INIT_MEMBER( ampro_state, ampro )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

static MACHINE_CONFIG_START( ampro, ampro_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(ampro_mem)
	MCFG_CPU_IO_MAP(ampro_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)
	MCFG_MACHINE_RESET_OVERRIDE(ampro_state, ampro)

	/* Devices */
	MCFG_DEVICE_ADD("z80ctc", Z80CTC, XTAL_16MHz / 4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(ampro_state, ctc_z0_w))    // Z80DART Ch A, SIO Ch A
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE("z80dart", z80dart_device, rxtxcb_w))   // SIO Ch B

	MCFG_Z80DART_ADD("z80dart", XTAL_16MHz / 4, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("z80dart", z80dart_device, rxa_w))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc_tick", ampro_state, ctc_tick, attotime::from_hz(XTAL_16MHz / 8))
	MCFG_WD1772_ADD("fdc", XTAL_16MHz / 2)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ampro_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_SOFTWARE_LIST_ADD("flop_list", "ampro")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ampro )
	ROM_REGION( 0x11000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "mntr", "Monitor")
	ROMX_LOAD( "mntr", 0x10000, 0x1000, CRC(d59d0909) SHA1(936410f414b1e71445253840eea0045545e4ff0b), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "boot", "Boot")
	ROMX_LOAD( "boot", 0x10000, 0x1000, CRC(b3524046) SHA1(5466f7d28c1a04cfbf328095cb35ad1525e91f44), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "scsi", "SCSI Boot")
	ROMX_LOAD( "scsi", 0x10000, 0x1000, CRC(8eb20e5d) SHA1(0ab1ff65cf6d3c1a713a8ac5c1ee4c662ac3da0c), ROM_BIOS(3))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS          INIT     COMPANY       FULLNAME       FLAGS */
COMP( 1980, ampro,  0,      0,       ampro,     ampro,   ampro_state,   ampro,  "Ampro", "Little Z80 Board", MACHINE_NO_SOUND_HW)
