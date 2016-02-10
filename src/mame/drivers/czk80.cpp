// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

CZK-80

2010-08-30 Skeleton driver
2010-11-27 Connected to a terminal
2014-01-08 Added devices

Only known info: http://forum.z80.de/showtopic.php?threadid=280

On main board there are Z80A CPU, Z80A PIO, Z80A DART and Z80A CTC
   there is 8K ROM and XTAL 16MHz
   Two undumped proms, AM27S20DC and D3631-1.
FDC board contains Z80A DMA and NEC 765A (XTAL on it is 8MHZ)
Mega board contains 74LS612 and memory chips (32x 41256)

Status:
  It prints 2 lines of text, then:
  - If fdc is enabled in address map, it hangs waiting for a fdc response.
  - Otherwise, it displays an error, and you can press a key to try again.

ToDo:
- Everything... no diagram or manuals, so EVERYTHING below is guesswork.
- Need software

I/O ports: These ranges are what is guessed
  40 : rom switching
  4c-4F : PIO
  50-53 : DART
  54-57 : CTC
  80-81 : Parallel port (no programming bytes are sent, so it isn't a device)
  C0-C1 : FDC
  It is not known what address is used by:
  - the DMA
  - the Motor-on signal(s)
  as there are no unknown writes.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/upd765.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80dart.h"
#include "machine/z80ctc.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class czk80_state : public driver_device
{
public:
	czk80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_fdc(*this, "fdc")
	{
	}

	DECLARE_DRIVER_INIT(czk80);
	DECLARE_MACHINE_RESET(czk80);
	TIMER_CALLBACK_MEMBER(czk80_reset);
	DECLARE_READ8_MEMBER(port80_r);
	DECLARE_READ8_MEMBER(port81_r);
	DECLARE_READ8_MEMBER(portc0_r);
	DECLARE_WRITE8_MEMBER(port40_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<upd765a_device> m_fdc;
};


WRITE8_MEMBER( czk80_state::port40_w )
{
	membank("bankr1")->set_entry(BIT(data, 1));
}

READ8_MEMBER( czk80_state::port80_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( czk80_state::portc0_r )
{
	return 0x80;
}

READ8_MEMBER( czk80_state::port81_r )
{
	return (m_term_data) ? 3 : 1;
}

static ADDRESS_MAP_START(czk80_mem, AS_PROGRAM, 8, czk80_state)
	AM_RANGE(0x0000, 0x1fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x2000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_READ_BANK("bankr1") AM_WRITE_BANK("bankw1")
ADDRESS_MAP_END

static ADDRESS_MAP_START(czk80_io, AS_IO, 8, czk80_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x40, 0x40) AM_WRITE(port40_w)
	AM_RANGE(0x4c, 0x4f) AM_DEVREADWRITE("z80pio", z80pio_device, read, write)
	AM_RANGE(0x50, 0x53) AM_DEVREADWRITE("z80dart", z80dart_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x54, 0x57) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)
	AM_RANGE(0x80, 0x80) AM_READ(port80_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x81, 0x81) AM_READ(port81_r)
	/* Select one of the below */
	//AM_RANGE(0xc0, 0xc0) AM_READ(portc0_r)
	AM_RANGE(0xc0, 0xc1) AM_DEVICE("fdc", upd765a_device, map)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( czk80 )
INPUT_PORTS_END

/* Z80 Daisy Chain */

static const z80_daisy_config daisy_chain[] =
{
	{ "z80pio" },
	{ "z80dart" },
	{ "z80ctc" },
	{ nullptr }
};

/* Z80-CTC Interface */

WRITE_LINE_MEMBER( czk80_state::ctc_z0_w )
{
// guess this generates clock for z80dart
}

WRITE_LINE_MEMBER( czk80_state::ctc_z1_w )
{
}

WRITE_LINE_MEMBER( czk80_state::ctc_z2_w )
{
}

/* after the first 4 bytes have been read from ROM, switch the ram back in */
TIMER_CALLBACK_MEMBER( czk80_state::czk80_reset)
{
	membank("bankr0")->set_entry(1);
}

MACHINE_RESET_MEMBER( czk80_state, czk80 )
{
	machine().scheduler().timer_set(attotime::from_usec(3), timer_expired_delegate(FUNC(czk80_state::czk80_reset),this));
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	membank("bankr1")->set_entry(0); // point at rom
	membank("bankw1")->set_entry(0); // always write to ram
}

DRIVER_INIT_MEMBER( czk80_state, czk80 )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);

	membank("bankr1")->configure_entry(1, &main[0xe000]);
	membank("bankr1")->configure_entry(0, &main[0x10000]);
	membank("bankw1")->configure_entry(0, &main[0xe000]);
}

static SLOT_INTERFACE_START( czk80_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

WRITE8_MEMBER( czk80_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( czk80, czk80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(czk80_mem)
	MCFG_CPU_IO_MAP(czk80_io)
	MCFG_CPU_CONFIG(daisy_chain)
	MCFG_MACHINE_RESET_OVERRIDE(czk80_state, czk80)

	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(czk80_state, kbd_put))
	MCFG_UPD765A_ADD("fdc", false, true)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", czk80_floppies, "525dd", floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD("z80ctc", Z80CTC, XTAL_16MHz / 4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(czk80_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(czk80_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(czk80_state, ctc_z2_w))

	MCFG_Z80DART_ADD("z80dart",  XTAL_16MHz / 4, 0, 0, 0, 0 )
	//MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	//MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("z80pio", Z80PIO, XTAL_16MHz/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( czk80 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "czk80.rom", 0x10000, 0x2000, CRC(7081b7c6) SHA1(13f75b14ea73b252bdfa2384e6eead6e720e49e3))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT   COMPAT MACHINE  INPUT  CLASS         INIT    COMPANY      FULLNAME       FLAGS */
COMP( 198?, czk80,  0,       0,     czk80,   czk80, czk80_state, czk80, "<unknown>",  "CZK-80", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
