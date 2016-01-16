// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Skeleton driver for Televideo TS802

    2012-11-06 Skeleton
    2014-02-07 Started adding devices

    Status:
    - TS802:  After 5 seconds, Slowly prints dots
    - TS802H: After 5 seconds, type in any 5 characters, then you get a prompt.

    TODO:
    - Almost everything

    Technical manual at:
    http://bitsavers.org/pdf/televideo/TS800A_TS802_TS802H_Maintenance_Manual_1982.pdf

    includes in-depth discussion of the inner workings of the WD1000 HDD controller.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/terminal.h"
#include "machine/z80dma.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/wd_fdc.h"

#define TERMINAL_TAG "terminal"

class ts802_state : public driver_device
{
public:
	ts802_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_DRIVER_INIT(ts802);
	DECLARE_MACHINE_RESET(ts802);
	DECLARE_READ8_MEMBER(port00_r) { return 0x80; };
	DECLARE_READ8_MEMBER(port0c_r) { return 1; };
	DECLARE_READ8_MEMBER(port0e_r) { return 0; };
	DECLARE_READ8_MEMBER(port0f_r) { return (m_term_data) ? 5 : 4; };
	DECLARE_READ8_MEMBER(port0d_r);
	DECLARE_WRITE8_MEMBER(port04_w);
	DECLARE_WRITE8_MEMBER(port18_w);
	DECLARE_WRITE8_MEMBER(port80_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	DECLARE_WRITE8_MEMBER( kbd_put );
private:
	UINT8 m_term_data;
	address_space *m_mem;
	address_space *m_io;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START(ts802_mem, AS_PROGRAM, 8, ts802_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ts802_io, AS_IO, 8, ts802_state)
	//ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_READ(port00_r)  // DIP switches
	// 04 - written once after OS boot to bank in RAM from 0000-3FFF instead of ROM.  4000-FFFF is always RAM.
	AM_RANGE(0x04, 0x07) AM_WRITE(port04_w)
	// 08-0B: Z80 CTC
	AM_RANGE(0x08, 0x0b) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)
	// 0C-0F: Z80 SIO #1
	//AM_RANGE(0x0c, 0x0f) AM_DEVREADWRITE("z80dart1", z80dart_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x0c, 0x0c) AM_READ(port0c_r)
	AM_RANGE(0x0d, 0x0d) AM_READ(port0d_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x0e, 0x0e) AM_READ(port0e_r)
	AM_RANGE(0x0f, 0x0f) AM_READ(port0f_r)
	// 10: Z80 DMA
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("z80dma", z80dma_device, read, write)
	// 14-17: WD 1793
	AM_RANGE(0x14, 0x17) AM_DEVREADWRITE("fdc", fd1793_t, read, write)
	// 18: floppy misc.
	AM_RANGE(0x18, 0x1c) AM_WRITE(port18_w)
	// 20-23: Z80 SIO #2
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("z80dart2", z80dart_device, ba_cd_r, ba_cd_w)
	// 48-4F: WD1000 harddisk controller
	// 80: LEDs
	AM_RANGE(0x80, 0x80) AM_WRITE(port80_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( ts802 )
INPUT_PORTS_END

WRITE8_MEMBER( ts802_state::port04_w )
{
	membank("bankr0")->set_entry(1);
}

WRITE8_MEMBER( ts802_state::port18_w )
{
}

WRITE8_MEMBER( ts802_state::port80_w )
{
}

READ8_MEMBER( ts802_state::memory_read_byte )
{
	return m_mem->read_byte(offset);
}

WRITE8_MEMBER( ts802_state::memory_write_byte )
{
	m_mem->write_byte(offset, data);
}

READ8_MEMBER( ts802_state::io_read_byte )
{
	return m_io->read_byte(offset);
}

WRITE8_MEMBER( ts802_state::io_write_byte )
{
	m_io->write_byte(offset, data);
}

static SLOT_INTERFACE_START( ts802_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

MACHINE_RESET_MEMBER( ts802_state, ts802 )
{
	membank("bankr0")->set_entry(0); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
}

READ8_MEMBER( ts802_state::port0d_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( ts802_state::kbd_put )
{
	m_term_data = data;
}

#if 0
// not correct
static const z80_daisy_config daisy_chain_intf[] =
{
	{ "z80dart1" },
	{ "z80dart2" },
	{ "z80dma" },
	{ "z80ctc" },
	{ NULL }
};
#endif

DRIVER_INIT_MEMBER( ts802_state, ts802 )
{
	m_mem = &m_maincpu->space(AS_PROGRAM);
	m_io = &m_maincpu->space(AS_IO);

	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankr0")->configure_entry(0, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

static MACHINE_CONFIG_START( ts802, ts802_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(ts802_mem)
	MCFG_CPU_IO_MAP(ts802_io)
	//MCFG_CPU_CONFIG(daisy_chain_intf) // causes problems
	MCFG_MACHINE_RESET_OVERRIDE(ts802_state, ts802)

	/* Devices */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(ts802_state, kbd_put))

	MCFG_DEVICE_ADD("z80dma", Z80DMA, XTAL_16MHz / 4)
	MCFG_Z80DMA_OUT_BUSREQ_CB(INPUTLINE("maincpu", INPUT_LINE_HALT))
	MCFG_Z80DMA_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(ts802_state, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(ts802_state, memory_write_byte))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(ts802_state, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(ts802_state, io_write_byte))

	MCFG_Z80DART_ADD("z80dart1", XTAL_16MHz / 4, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_Z80DART_ADD("z80dart2", XTAL_16MHz / 4, 0, 0, 0, 0 )
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD("z80ctc", Z80CTC, XTAL_16MHz / 4)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	MCFG_FD1793_ADD("fdc", XTAL_4MHz / 2)                  // unknown clock
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ts802_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ts802 )
	ROM_REGION(0x11000, "maincpu", 0)
	ROM_LOAD( "ts802.rom", 0x10000, 0x1000, CRC(60bd086a) SHA1(82c5b60223e0d895683d3592a56684ef2dabfba6) )
ROM_END

ROM_START( ts802h )
	ROM_REGION(0x11000, "maincpu", 0)
	ROM_LOAD( "8000050 050 2732", 0x10000, 0x1000, CRC(7054f384) SHA1(cf0a01a32283272532ed4890c3a3c2082f1618bf) )

	ROM_REGION(0x2000, "roms", 0) // not Z80 code
	ROM_LOAD( "i800000 047d.a53", 0x0000, 0x1000, CRC(94bfcbc1) SHA1(87c5f8898b0041d012e142ee7f559cb8a90f4dc1) )
	ROM_LOAD( "a64",              0x1000, 0x1000, CRC(41b5feda) SHA1(c9435a97c032ffe457bdb84d5dde8ecf3677b56c) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "800000-003a.a68",  0x0000, 0x0800, CRC(24eeb74d) SHA1(77900937f1492b4c5a70ba3aac55da322d403fbd) )
	ROM_LOAD( "800000-002a.a67",  0x0800, 0x0800, CRC(4b6c6e29) SHA1(c236e4625bc16062154cbebc4dbc8d62183ef9ab) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE         INIT      COMPANY    FULLNAME       FLAGS */
COMP( 1982, ts802,   0,       0,     ts802,     ts802, ts802_state,  ts802,  "Televideo", "TS802",  MACHINE_IS_SKELETON )
COMP( 1982, ts802h,  ts802,   0,     ts802,     ts802, ts802_state,  ts802,  "Televideo", "TS802H", MACHINE_IS_SKELETON )
