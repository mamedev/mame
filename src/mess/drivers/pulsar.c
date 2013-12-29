// license:MAME
// copyright-holders:Robbbert
/***************************************************************************

Pulsar Little Big Board

2013-12-29 Skeleton driver.

Chips: Z80A @4MHz, Z80DART, FD1797-02, 8255A-5, AY-5-8116, MSM5832.
Crystals: 4 MHz, 5.0688 MHz

This is a complete CP/M single-board computer. You needed to supply your own
power supply and serial terminal.

ToDo:
- Hook up 8255
- Add 5832 rtc
- Find out why data from board is corrupt but from keyboard is fine
- Need software


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80dart.h"
#include "machine/i8255.h"
#include "machine/com8116.h"
#include "machine/serial.h"
#include "machine/wd_fdc.h"


class pulsar_state : public driver_device
{
public:
	pulsar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dart(*this, "z80dart")
		, m_brg(*this, "brg")
		, m_fdc (*this, "fdc")
		, m_floppy0(*this, "fdc:0")
	{ }

	DECLARE_DRIVER_INIT(pulsar);
	DECLARE_MACHINE_RESET(pulsar);
	TIMER_CALLBACK_MEMBER(pulsar_reset);
	DECLARE_WRITE8_MEMBER(baud_w);
	DECLARE_WRITE8_MEMBER(port00_w);
	DECLARE_WRITE_LINE_MEMBER(fr_w);
	DECLARE_WRITE_LINE_MEMBER(ft_w);
private:
	required_device<cpu_device> m_maincpu;
	required_device<z80dart_device> m_dart;
	required_device<com8116_device> m_brg;
	required_device<fd1797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
};

/*
d0..d3 Drive select 0-3 (we only emulate 1 drive)
d4     Side select 0=side0
d5     /DDEN
d6     Banking 0=rom
d7     FDC master clock 0=8MHz 1=16MHz (for 20cm disks, not emulated)
*/
WRITE8_MEMBER( pulsar_state::port00_w )
{
	membank("bankr1")->set_entry(BIT(data, 6));
	m_fdc->dden_w(BIT(data, 5));
	floppy_image_device *floppy = NULL;
	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	m_fdc->set_floppy(floppy);
	if (floppy)
		floppy->ss_w(BIT(data, 4));
}

static ADDRESS_MAP_START(pulsar_mem, AS_PROGRAM, 8, pulsar_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x0800, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_READ_BANK("bankr1") AM_WRITE_BANK("bankw1")
ADDRESS_MAP_END

static ADDRESS_MAP_START(pulsar_io, AS_IO, 8, pulsar_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xc0, 0xc3) AM_MIRROR(0x0c) AM_DEVREADWRITE("z80dart", z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0xd0, 0xd3) AM_MIRROR(0x0c) AM_DEVREADWRITE("fdc", fd1797_t, read, write)
	AM_RANGE(0xe0, 0xe3) AM_MIRROR(0x0c) AM_DEVREADWRITE("ppi", i8255_device, read, write)
	AM_RANGE(0xf0, 0xff) AM_WRITE(baud_w)
ADDRESS_MAP_END

WRITE_LINE_MEMBER( pulsar_state::fr_w )
{
	m_dart->rxcb_w(state);
	m_dart->txcb_w(state);
}

WRITE_LINE_MEMBER( pulsar_state::ft_w )
{
	m_dart->rxca_w(state);
	m_dart->txca_w(state);
}

WRITE8_MEMBER( pulsar_state::baud_w )
{
	m_brg->str_w(data & 0x0f);
	m_brg->stt_w(data >> 4);
}

/* after the first 4 bytes have been read from ROM, switch the ram back in */
TIMER_CALLBACK_MEMBER( pulsar_state::pulsar_reset)
{
	membank("bankr0")->set_entry(0);
}

static const z80_daisy_config daisy_chain_intf[] =
{
	{ "z80dart" },
	{ NULL }
};

static I8255_INTERFACE( ppi_intf )
{
	DEVCB_NULL,//DEVCB_DRIVER_MEMBER(pulsar_state, ppi_pa_r),   // Port A read
	DEVCB_NULL,//DEVCB_DRIVER_MEMBER(pulsar_state, ppi_pa_w),   // Port A write
	DEVCB_NULL,//DEVCB_DRIVER_MEMBER(pulsar_state, ppi_pb_r),   // Port B read
	DEVCB_NULL,//DEVCB_DRIVER_MEMBER(pulsar_state, ppi_pb_w),   // Port B write
	DEVCB_NULL,//DEVCB_DRIVER_MEMBER(pulsar_state, ppi_pc_r),   // Port C read
	DEVCB_NULL,//DEVCB_DRIVER_MEMBER(pulsar_state, ppi_pc_w),   // Port C write
};

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "TERM_FRAME", 0x0f, 0x06 ) // 9600
	DEVICE_INPUT_DEFAULTS( "TERM_FRAME", 0x30, 0x10 ) // 7E1
DEVICE_INPUT_DEFAULTS_END

static Z80DART_INTERFACE( dart_intf )
{
	0, 0, 0, 0,

	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, tx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, dtr_w),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, rts_w),
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL, // out data
	DEVCB_NULL, // DTR
	DEVCB_NULL, // RTS
	DEVCB_NULL, // WRDY
	DEVCB_NULL, // SYNC

	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static SLOT_INTERFACE_START( pulsar_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

/* Input ports */
static INPUT_PORTS_START( pulsar )
INPUT_PORTS_END

MACHINE_RESET_MEMBER( pulsar_state, pulsar )
{
	machine().scheduler().timer_set(attotime::from_usec(10), timer_expired_delegate(FUNC(pulsar_state::pulsar_reset),this));
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	membank("bankr1")->set_entry(0); // point at rom
	membank("bankw1")->set_entry(0); // always write to ram
}

DRIVER_INIT_MEMBER( pulsar_state, pulsar )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);

	membank("bankr1")->configure_entry(1, &main[0xf800]);
	membank("bankr1")->configure_entry(0, &main[0x10000]);
	membank("bankw1")->configure_entry(0, &main[0xf800]);
}

static MACHINE_CONFIG_START( pulsar, pulsar_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(pulsar_mem)
	MCFG_CPU_IO_MAP(pulsar_io)
	MCFG_CPU_CONFIG(daisy_chain_intf)
	MCFG_MACHINE_RESET_OVERRIDE(pulsar_state, pulsar)

	/* Devices */
	MCFG_I8255_ADD( "ppi", ppi_intf )
	MCFG_COM8116_ADD("brg", XTAL_5_0688MHz, NULL, WRITELINE(pulsar_state, fr_w), WRITELINE(pulsar_state, ft_w))
	MCFG_Z80DART_ADD("z80dart",  XTAL_4MHz, dart_intf )
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "serial_terminal")
	MCFG_SERIAL_OUT_RX_HANDLER(DEVWRITELINE("z80dart", z80dart_device, rxa_w))
	//MCFG_RS232_OUT_DCD_HANDLER(DEVWRITELINE("z80dart", z80dart_device, dcda_w))
	//MCFG_RS232_OUT_CTS_HANDLER(DEVWRITELINE("z80dart", z80dart_device, ctsa_w))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("serial_terminal", terminal)
	MCFG_FD1797x_ADD("fdc", XTAL_4MHz / 2)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pulsar_floppies, "525dd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pulsar )
	ROM_REGION( 0x10800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "mp7a.bin", 0x10000, 0x800, CRC(726b8a19) SHA1(43b2af84d5622c1f67584c501b730acf002a6113) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    CLASS          INIT     COMPANY       FULLNAME       FLAGS */
COMP( 1980, pulsar, 0,      0,       pulsar,    pulsar,  pulsar_state,  pulsar,  "Pulsar", "Little Big Board", GAME_NOT_WORKING | GAME_NO_SOUND_HW)
