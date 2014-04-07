// license:MAME
// copyright-holders:Robbbert
/************************************************************************************************************

Telcon Zorba

2013-08-25 Skeleton

This was one of the last CP/M-based systems, already out of date when it was released.
Because it doesn't use the standard Z80 peripherals, it uses a homebrew interrupt controller to make use
 of the Z80's IM2.

The keyboard is an intelligent serial device like the Kaypro's keyboard. They even have the same plug,
and might be swappable. Need a schematic.

ToDo:
- Add interrupt vector hardware and masking feature
- Connect devices to the above hardware
- Fix the unholy mess surrounding the dma and crtc
- Connect up the PIT
- Replace the ascii keyboard with the real one, if possible
- Probably lots of other things

Note: because the 8275 isn't working, a generic video handler is being used at this time.

*************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "machine/i8251.h"
#include "machine/6821pia.h"
#include "machine/z80dma.h"
#include "machine/pit8253.h"
#include "video/i8275x.h"
#include "sound/beep.h"
#include "machine/keyboard.h"
#include "machine/wd_fdc.h"

#define KEYBOARD_TAG "keyboard"

class zorba_state : public driver_device
{
public:
	zorba_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_p_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_beep(*this, "beeper"),
		m_dma(*this, "dma"),
		m_u0(*this, "uart0"),
		m_u1(*this, "uart1"),
		m_u2(*this, "uart2"),
		m_pia0(*this, "pia0"),
		m_pia1(*this, "pia1"),
		m_crtc(*this, "crtc"),
		m_fdc (*this, "fdc"),
		m_floppy0(*this, "fdc:0"),
		m_floppy1(*this, "fdc:1"),
		m_palette(*this, "palette")
	{
	}

public:
	const UINT8 *m_p_chargen;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(zorba);
	required_shared_ptr<UINT8> m_p_videoram;
	DECLARE_DRIVER_INIT(zorba);
	DECLARE_MACHINE_RESET(zorba);
	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(rom_r);
	DECLARE_WRITE8_MEMBER(rom_w);
	DECLARE_WRITE8_MEMBER(intmask_w);
	DECLARE_WRITE_LINE_MEMBER(busreq_w);
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_READ8_MEMBER(io_read_byte);
	DECLARE_WRITE8_MEMBER(io_write_byte);
	DECLARE_WRITE8_MEMBER(pia0_porta_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(keyboard_r);
	I8275_DRAW_CHARACTER_MEMBER( zorba_update_chr );

private:
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beep;
	required_device<z80dma_device> m_dma;
	required_device<i8251_device> m_u0;
	required_device<i8251_device> m_u1;
	required_device<i8251_device> m_u2;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<i8275x_device> m_crtc;
	required_device<fd1793_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
public:
	required_device<palette_device> m_palette;
};

static ADDRESS_MAP_START( zorba_mem, AS_PROGRAM, 8, zorba_state )
	AM_RANGE( 0x0000, 0x3fff ) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE( 0x4000, 0xf6ff ) AM_RAM
	AM_RANGE( 0xf700, 0xffff ) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( zorba_kbdmem, AS_PROGRAM, 8, zorba_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE( 0x000, 0x07f ) AM_RAM // internal RAM
	AM_RANGE( 0x080, 0x7ff ) AM_ROM // internal EPROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( zorba_io, AS_IO, 8, zorba_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("pit", pit8254_device, read, write)
	AM_RANGE(0x04, 0x04) AM_READWRITE(rom_r,rom_w)
	AM_RANGE(0x05, 0x05) AM_READWRITE(ram_r,ram_w)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE("crtc", i8275x_device, read, write)
	AM_RANGE(0x20, 0x20) AM_DEVREADWRITE("uart0", i8251_device, data_r, data_w)
	AM_RANGE(0x21, 0x21) AM_DEVREADWRITE("uart0", i8251_device, status_r, control_w)
	AM_RANGE(0x22, 0x22) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	AM_RANGE(0x23, 0x23) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	//AM_RANGE(0x24, 0x24) AM_DEVREADWRITE("uart2", i8251_device, data_r, data_w)
	//AM_RANGE(0x25, 0x25) AM_DEVREADWRITE("uart2", i8251_device, status_r, control_w)
	AM_RANGE(0x24, 0x25) AM_READ(keyboard_r) AM_WRITENOP
	AM_RANGE(0x26, 0x26) AM_WRITE(intmask_w)
	AM_RANGE(0x30, 0x30) AM_DEVREADWRITE("dma", z80dma_device, read, write)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE("fdc", fd1793_t, read, write)
	AM_RANGE(0x50, 0x53) AM_DEVREADWRITE("pia0", pia6821_device, read, write)
	AM_RANGE(0x60, 0x63) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
ADDRESS_MAP_END

READ8_MEMBER( zorba_state::ram_r )
{
	membank("bankr0")->set_entry(0);
	return 0;
}

WRITE8_MEMBER( zorba_state::ram_w )
{
	membank("bankr0")->set_entry(0);
}

READ8_MEMBER( zorba_state::rom_r )
{
	membank("bankr0")->set_entry(1);
	return 0;
}

WRITE8_MEMBER( zorba_state::rom_w )
{
	membank("bankr0")->set_entry(1);
}

WRITE8_MEMBER( zorba_state::intmask_w )
{
}

static INPUT_PORTS_START( zorba )
INPUT_PORTS_END

DRIVER_INIT_MEMBER( zorba_state, zorba )
{
	UINT8 *main = memregion("maincpu")->base();

	membank("bankr0")->configure_entry(0, &main[0x0000]);
	membank("bankr0")->configure_entry(1, &main[0x10000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
}

//-------------------------------------------------
//  Z80DMA_INTERFACE( dma_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( zorba_state::busreq_w )
{
// since our Z80 has no support for BUSACK, we assume it is granted immediately
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, state);
	m_maincpu->set_input_line(INPUT_LINE_HALT, state); // do we need this?
	m_dma->bai_w(state); // tell dma that bus has been granted
}

READ8_MEMBER(zorba_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(zorba_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

READ8_MEMBER(zorba_state::io_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(zorba_state::io_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_IO);

	if (offset == 0x10)
	{
		m_crtc->dack_w(space, 0, data);
	}
	else
	{
		return prog_space.write_byte(offset, data);
	}
}

// busack on cpu connects to bai pin
static Z80DMA_INTERFACE( dma_intf )
{
	//DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_HALT), //busreq - connects to busreq on cpu
	DEVCB_DRIVER_LINE_MEMBER(zorba_state, busreq_w), //busreq - connects to busreq on cpu
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0), //int/pulse - connects to IRQ0 on cpu
	DEVCB_NULL, //ba0 - not connected
	DEVCB_DRIVER_MEMBER(zorba_state, memory_read_byte),
	DEVCB_DRIVER_MEMBER(zorba_state, memory_write_byte),
	DEVCB_DRIVER_MEMBER(zorba_state, io_read_byte),
	DEVCB_DRIVER_MEMBER(zorba_state, io_write_byte),
};

static SLOT_INTERFACE_START( zorba_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

WRITE8_MEMBER( zorba_state::pia0_porta_w )
{
	m_beep->set_state(BIT(data, 7));
	m_fdc->dden_w(BIT(data, 6));

	floppy_image_device *floppy = NULL;
	if (!BIT(data, 0)) floppy = m_floppy0->get_device();
	if (!BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->ss_w(!BIT(data, 5)); // might need inverting
	}

	m_floppy0->get_device()->mon_w(BIT(data, 4));
	m_floppy1->get_device()->mon_w(BIT(data, 4));
}

PALETTE_INIT_MEMBER( zorba_state, zorba )
{
	palette.set_pen_color(0, 0, 0, 0 ); /* Black */
	palette.set_pen_color(1, 0, 255, 0 );   /* Full */
	palette.set_pen_color(2, 0, 128, 0 );   /* Dimmed */
}

I8275_DRAW_CHARACTER_MEMBER( zorba_state::zorba_update_chr )
{
	int i;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 gfx = m_p_chargen[(linecount & 15) + (charcode << 4)];

	if (vsp)
		gfx = 0;

	if (lten)
		gfx = 0xff;

	if (rvv)
		gfx ^= 0xff;

	for(i=0;i<8;i++)
		bitmap.pix32(y, x + 7 - i) = palette[BIT(gfx, i) ? (hlgt ? 2 : 1) : 0];
}

MACHINE_RESET_MEMBER( zorba_state, zorba )
{
	m_beep->set_frequency(800);
	m_p_chargen = memregion("chargen")->base();
	membank("bankr0")->set_entry(1); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
}

UINT32 zorba_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 11; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = 0; x < 80; x++)
			{
				gfx = 0;
				chr = m_p_videoram[x+ma];

				if (chr) gfx = m_p_chargen[(chr<<4) | ra ];

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=80;
	}
	return 0;
}

READ8_MEMBER( zorba_state::keyboard_r )
{
	if (offset)
		return (m_term_data) ? 0x87 : 0x85;

	UINT8 data = m_term_data;
	m_term_data = 0;
	return data;
}

WRITE8_MEMBER( zorba_state::kbd_put )
{
	m_term_data = data;
}

static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(zorba_state, kbd_put)
};

static MACHINE_CONFIG_START( zorba, zorba_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, XTAL_24MHz / 6)
	MCFG_CPU_PROGRAM_MAP(zorba_mem)
	MCFG_CPU_IO_MAP(zorba_io)
	MCFG_MACHINE_RESET_OVERRIDE(zorba_state, zorba)

	/* keyboard */
	MCFG_CPU_ADD("kbdcpu", M68705, XTAL_3_579545MHz) // MC68705P3S (0x80 bytes ram, 0x780 bytes of rom)
	MCFG_CPU_PROGRAM_MAP(zorba_kbdmem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	//MCFG_SCREEN_UPDATE_DEVICE("crtc", i8275x_device, screen_update) // not working
	MCFG_SCREEN_UPDATE_DRIVER(zorba_state, screen_update)
	MCFG_SCREEN_SIZE(640, 276)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 275)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(zorba_state, zorba)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* Devices */
	MCFG_Z80DMA_ADD("dma", XTAL_24MHz/6, dma_intf)

	MCFG_DEVICE_ADD("uart0", I8251, 0)
	// COM port

	MCFG_DEVICE_ADD("uart1", I8251, 0)
	// printer port

	MCFG_DEVICE_ADD("uart2", I8251, 0)
	// keyboard

// port A - disk select etc, beeper
// port B - parallel interface
	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(zorba_state, pia0_porta_w))

// IEEE488 interface
	MCFG_DEVICE_ADD("pia1", PIA6821, 0)

	MCFG_DEVICE_ADD("pit", PIT8254, 0)
	MCFG_PIT8253_CLK0(XTAL_24MHz / 3) /* Timer 0: ? */
	MCFG_PIT8253_CLK1(XTAL_24MHz / 3) /* Timer 1: ? */
	MCFG_PIT8253_CLK2(XTAL_24MHz / 3) /* Timer 2: ? */

	MCFG_DEVICE_ADD("crtc", I8275x, XTAL_14_31818MHz/7)
	MCFG_I8275_CHARACTER_WIDTH(8)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(zorba_state, zorba_update_chr)
	MCFG_I8275_DRQ_CALLBACK(DEVWRITELINE("dma", z80dma_device, rdy_w))
	MCFG_I8275_IRQ_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_FD1793x_ADD("fdc", XTAL_24MHz / 24)
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", zorba_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", zorba_floppies, "525dd", floppy_image_device::default_floppy_formats)

	/* Keyboard */
	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)
MACHINE_CONFIG_END

ROM_START( zorba )
	ROM_REGION( 0x14000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "780000.u47", 0x10000, 0x1000, CRC(6d58f2c5) SHA1(7763f08c801cd36e5a761c6dc9f30a50b3bc482d) )

	ROM_REGION( 0x800, "kbdcpu", 0 )
	ROM_LOAD( "8999-1 3-28-83", 0x080, 0x780, CRC(79fe6c0d) SHA1(4b6fca9379d5199d1347ad1187cbfdebfc4c73e7) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "773000.u5", 0x0000, 0x1000, CRC(d0a2f8fc) SHA1(29aee7ee657778c46e9800abd4955e6d4b33ef68) )

	ROM_REGION( 0x60, "proms", 0 )
	ROM_LOAD( "74ls288.u37", 0x0000, 0x0020, CRC(0a67edd6) SHA1(c1ece8978a3a061e0130d43907fa63a71e75e75d) )
	ROM_LOAD( "74ls288.u38", 0x0020, 0x0020, CRC(5ec93ea7) SHA1(3a84c098474b05d5cbe1939a3e15f66d06470581) )
	ROM_LOAD( "74ls288.u77", 0x0040, 0x0020, CRC(946e03b0) SHA1(24240bdd7bdf507a5b51628fb36ad1266fc53a28) )
ROM_END

COMP( 1982, zorba, 0, 0, zorba, zorba, zorba_state, zorba, "Telcon Industries", "Zorba", GAME_NOT_WORKING )
