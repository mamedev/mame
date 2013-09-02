/***************************************************************************

Ampro Little Z80 Board

2013-09-02 Skeleton driver.

Chips: Z80A @4MHz, Z80CTC, Z80DART, WD1770/2, NCR5380. Crystal: 16 MHz

This is a complete CP/M single-board computer that could be mounted
on top of a standard 13cm floppy drive. You needed to supply your own
power supply and serial terminal.

The later versions included a SCSI chip (NCR5380) enabling the use
of a hard drive of up to 88MB.

Status: The system will start up into the monitor, as long as no disk
        has been mounted.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/terminal.h"
//#include "machine/serial.h"
#include "machine/wd_fdc.h"


class ampro_state : public driver_device
{
public:
	ampro_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, TERMINAL_TAG)
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{ }

	DECLARE_DRIVER_INIT(ampro);
	DECLARE_MACHINE_RESET(ampro);
	DECLARE_READ8_MEMBER(port80_r);
	DECLARE_READ8_MEMBER(port84_r);
	DECLARE_WRITE8_MEMBER(port00_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<wd1772_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
};

READ8_MEMBER( ampro_state::port80_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( ampro_state::port84_r )
{
	return (m_term_data) ? 5 : 4;
}

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
	floppy_image_device *floppy = NULL;
	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy)
	{
		floppy->ss_w(BIT(data, 4)); // might need inverting
		floppy->mon_w(0);
	}
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
	//AM_RANGE(0x40, 0x7f) AM_READWRITE(ctc device)
	//AM_RANGE(0x80, 0x8f) AM_READWRITE(dart device)
	AM_RANGE(0x80, 0x80) AM_READ(port80_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x84, 0x84) AM_READ(port84_r)
	AM_RANGE(0xc0, 0xc3) AM_DEVWRITE("fdc", wd1772_t, write)
	AM_RANGE(0xc4, 0xc7) AM_DEVREAD("fdc", wd1772_t, read)
ADDRESS_MAP_END

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "z80ctc" },
	{ "z80dart" },
	{ NULL }
};

static Z80DART_INTERFACE( dart_intf )
{
	0, 0, 0, 0,

	// console#3
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	// printer
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static Z80CTC_INTERFACE( ctc_intf )
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), // interrupt callback
	DEVCB_NULL,//DEVCB_DEVICE_LINE_MEMBER("z80sio", z80dart_device, rxtxcb_w),         /* ZC/TO0 callback - SIO Ch B */
	DEVCB_NULL,//DEVCB_DRIVER_LINE_MEMBER(altos5_state, ctc_z1_w),         /* ZC/TO1 callback - Z80DART Ch A, SIO Ch A */
	DEVCB_NULL//DEVCB_DEVICE_LINE_MEMBER("z80dart", z80dart_device, rxtxcb_w),         /* ZC/TO2 callback - Z80DART Ch B */
};

#if 0
static const rs232_port_interface rs232_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,//DEVCB_DEVICE_LINE_MEMBER("z80sio", z80dart_device, dcdb_w),
	DEVCB_NULL,
	DEVCB_NULL,//DEVCB_DEVICE_LINE_MEMBER("z80sio", z80dart_device, rib_w),
	DEVCB_NULL //DEVCB_DEVICE_LINE_MEMBER("z80sio", z80dart_device, ctsb_w)
};
#endif
static SLOT_INTERFACE_START( ampro_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

/* Input ports */
static INPUT_PORTS_START( ampro )
INPUT_PORTS_END

WRITE8_MEMBER( ampro_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(ampro_state, kbd_put)
};

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

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)

	/* Devices */
	MCFG_Z80CTC_ADD( "z80ctc",   XTAL_16MHz / 4, ctc_intf )
	MCFG_Z80DART_ADD("z80dart",  XTAL_16MHz / 4, dart_intf )
	//MCFG_RS232_PORT_ADD("rs232", rs232_intf, default_rs232_devices, "serial_terminal")
	//MCFG_TIMER_DRIVER_ADD_PERIODIC("ctc_tick", altos5_state, ctc_tick, attotime::from_hz(XTAL_16MHz / 4))
	MCFG_WD1772x_ADD("fdc", XTAL_16MHz / 16)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ampro_floppies, "525dd", floppy_image_device::default_floppy_formats)
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
COMP( 1980, ampro,  0,      0,       ampro,     ampro,   ampro_state,   ampro,  "Ampro", "Little Z80 Board", GAME_NOT_WORKING | GAME_NO_SOUND_HW)
