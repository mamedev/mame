// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Heathkit H19

        12/05/2009 Skeleton driver.

        The keyboard consists of a 9x10 matrix connected to a MM5740N
        mask-programmed keyboard controller. The output of this passes
        through a rom. The MM5740N may be custom-made, in which case it
        will need to be procured and tested. There is a "default" array;
        perhaps this is used. The presence of the rom would seem to
        indicate that the default is being used. We won't know, of
        course, until the device is emulated.

        Keyboard input can also come from the serial port (a 8250).
        Either device will signal an interrupt to the CPU when a key
        is pressed/sent.

        For the moment, the "ascii keyboard" device is used for the keyboard.

        TODO:
        - Create device or HLE of MM5740N keyboard controller
        - Make sure correct mask programming is used for it
        - Use keyboard rom
        - Finish connecting up the 8250
        - Verify beep lengths
        - Verify ram size
        - When the internal keyboard works, get rid of the "ascii_keyboard".

        Bios 1 (super19) has the videoram at D800. This is not emulated.
        However, a keyclick can be heard, to assure you it does in fact work.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "sound/beep.h"
#include "machine/ins8250.h"
#include "machine/keyboard.h"

#define KEYBOARD_TAG "keyboard"

#define H19_CLOCK (XTAL_12_288MHz / 6)
#define H19_BEEP_FRQ (H19_CLOCK / 1024)


class h19_state : public driver_device
{
public:
	enum
	{
		TIMER_BEEP_OFF
	};

	h19_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_crtc(*this, "crtc"),
		m_ace(*this, "ins8250"),
		m_beep(*this, "beeper"),
		m_p_videoram(*this, "videoram"),
		m_palette(*this, "palette")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<ins8250_device> m_ace;
	required_device<beep_device> m_beep;
	DECLARE_READ8_MEMBER(h19_80_r);
	DECLARE_READ8_MEMBER(h19_a0_r);
	DECLARE_WRITE8_MEMBER(h19_c0_w);
	DECLARE_WRITE8_MEMBER(h19_kbd_put);
	MC6845_UPDATE_ROW(crtc_update_row);
	required_shared_ptr<UINT8> m_p_videoram;
	required_device<palette_device> m_palette;
	UINT8 *m_p_chargen;
	UINT8 m_term_data;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_WRITE_LINE_MEMBER(h19_ace_irq);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


void h19_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BEEP_OFF:
		m_beep->set_state(0);
		break;
	default:
		assert_always(FALSE, "Unknown id in h19_state::device_timer");
	}
}

READ8_MEMBER( h19_state::h19_80_r )
{
// keyboard data
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( h19_state::h19_a0_r )
{
// keyboard status
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0x7f; // says that a key is ready and no modifier keys are pressed
}

WRITE8_MEMBER( h19_state::h19_c0_w )
{
/* Beeper control - a 96L02 contains 2 oneshots, one for bell and one for keyclick.
- lengths need verifying
    offset 00-1F = keyclick
    offset 20-3F = terminal bell */

	UINT8 length = (offset & 0x20) ? 200 : 4;
	m_beep->set_state(1);
	timer_set(attotime::from_msec(length), TIMER_BEEP_OFF);
}

static ADDRESS_MAP_START(h19_mem, AS_PROGRAM, 8, h19_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( h19_io, AS_IO, 8, h19_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x1F) AM_READ_PORT("S401")
	AM_RANGE(0x20, 0x3F) AM_READ_PORT("S402")
	AM_RANGE(0x40, 0x47) AM_MIRROR(0x18) AM_DEVREADWRITE("ins8250", ins8250_device, ins8250_r, ins8250_w )
	AM_RANGE(0x60, 0x60) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x61, 0x61) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x80, 0x9F) AM_READ(h19_80_r)
	AM_RANGE(0xA0, 0xBF) AM_READ(h19_a0_r)
	AM_RANGE(0xC0, 0xFF) AM_WRITE(h19_c0_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( h19 )
// Port codes are omitted to prevent collision with terminal keys

	PORT_START("MODIFIERS")
	// bit 0 connects to B8 of MM5740 - purpose unknown
	// bit 7 is low if a key is pressed
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CapsLock")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Break")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OffLine")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LeftShift")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Repeat")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Reset")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RightShift")

	PORT_START("X1")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X2")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0-pad")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Dot-pad")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter-pad")

	PORT_START("X2")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\'")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("{")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1-pad")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2-pad")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3-pad")

	PORT_START("X3")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LF")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4-pad")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5-pad")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6-pad")

	PORT_START("X4")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("AccentAcute")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X1")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7-pad")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8-pad")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9-pad")

	PORT_START("X5")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Erase")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Blue")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Red")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Gray")

	PORT_START("X6")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")

	PORT_START("X7")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Scroll")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")

	PORT_START("X8")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tab")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")

	PORT_START("X9")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc")
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")
	PORT_BIT(0x200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")

	PORT_START("S401")
	PORT_DIPNAME( 0x0f, 0x03, "Baud Rate")
	PORT_DIPSETTING(    0x01, "110")
	PORT_DIPSETTING(    0x02, "150")
	PORT_DIPSETTING(    0x03, "300")
	PORT_DIPSETTING(    0x04, "600")
	PORT_DIPSETTING(    0x05, "1200")
	PORT_DIPSETTING(    0x06, "1800")
	PORT_DIPSETTING(    0x07, "2000")
	PORT_DIPSETTING(    0x08, "2400")
	PORT_DIPSETTING(    0x09, "3600")
	PORT_DIPSETTING(    0x0a, "4800")
	PORT_DIPSETTING(    0x0b, "7200")
	PORT_DIPSETTING(    0x0c, "9600")
	PORT_DIPSETTING(    0x0d, "19200")
	PORT_DIPNAME( 0x30, 0x00, "Parity")
	PORT_DIPSETTING(    0x00, DEF_STR(None))
	PORT_DIPSETTING(    0x10, "Odd")
	PORT_DIPSETTING(    0x30, "Even")
	PORT_DIPNAME( 0x40, 0x00, "Parity Type")
	PORT_DIPSETTING(    0x00, DEF_STR(Normal))
	PORT_DIPSETTING(    0x40, "Stick")
	PORT_DIPNAME( 0x80, 0x00, "Duplex")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x80, "Full")

	PORT_START("S402") // stored at 40C8
	PORT_DIPNAME( 0x01, 0x00, "Cursor")
	PORT_DIPSETTING(    0x00, "Underline")
	PORT_DIPSETTING(    0x01, "Block")
	PORT_DIPNAME( 0x02, 0x00, "Keyclick")
	PORT_DIPSETTING(    0x02, DEF_STR(No))
	PORT_DIPSETTING(    0x00, DEF_STR(Yes))
	PORT_DIPNAME( 0x04, 0x04, "Wrap at EOL")
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPSETTING(    0x04, DEF_STR(Yes))
	PORT_DIPNAME( 0x08, 0x08, "Auto LF on CR")
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPSETTING(    0x08, DEF_STR(Yes))
	PORT_DIPNAME( 0x10, 0x00, "Auto CR on LF")
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPSETTING(    0x10, DEF_STR(Yes))
	PORT_DIPNAME( 0x20, 0x00, "Mode")
	PORT_DIPSETTING(    0x00, "VT52")
	PORT_DIPSETTING(    0x20, "ANSI")
	PORT_DIPNAME( 0x40, 0x00, "Keypad Shifted")
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPSETTING(    0x40, DEF_STR(Yes))
	PORT_DIPNAME( 0x80, 0x00, "Refresh")
	PORT_DIPSETTING(    0x00, "50Hz")
	PORT_DIPSETTING(    0x80, "60Hz")
INPUT_PORTS_END


void h19_state::machine_reset()
{
	m_beep->set_frequency(H19_BEEP_FRQ);
}

void h19_state::video_start()
{
	m_p_chargen = memregion("chargen")->base();
}

MC6845_UPDATE_ROW( h19_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 chr,gfx;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		UINT8 inv=0;
		if (x == cursor_x) inv=0xff;
		mem = (ma + x) & 0x7ff;
		chr = m_p_videoram[mem];

		if (chr & 0x80)
		{
			inv ^= 0xff;
			chr &= 0x7f;
		}

		/* get pattern of pixels for that character scanline */
		gfx = m_p_chargen[(chr<<4) | ra] ^ inv;

		/* Display a scanline of a character (8 pixels) */
		*p++ = palette[BIT(gfx, 7)];
		*p++ = palette[BIT(gfx, 6)];
		*p++ = palette[BIT(gfx, 5)];
		*p++ = palette[BIT(gfx, 4)];
		*p++ = palette[BIT(gfx, 3)];
		*p++ = palette[BIT(gfx, 2)];
		*p++ = palette[BIT(gfx, 1)];
		*p++ = palette[BIT(gfx, 0)];
	}
}

WRITE_LINE_MEMBER(h19_state::h19_ace_irq)
{
	m_maincpu->set_input_line(0, (state ? HOLD_LINE : CLEAR_LINE));
}

/* F4 Character Displayer */
static const gfx_layout h19_charlayout =
{
	8, 10,                  /* 8 x 10 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( h19 )
	GFXDECODE_ENTRY( "chargen", 0x0000, h19_charlayout, 0, 1 )
GFXDECODE_END

WRITE8_MEMBER( h19_state::h19_kbd_put )
{
	m_term_data = data;
	m_maincpu->set_input_line(0, HOLD_LINE);
}

static MACHINE_CONFIG_START( h19, h19_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, H19_CLOCK) // From schematics
	MCFG_CPU_PROGRAM_MAP(h19_mem)
	MCFG_CPU_IO_MAP(h19_io)
	//MCFG_DEVICE_PERIODIC_INT_DRIVER(h19_state, irq0_line_hold,  50) // for testing, causes a keyboard scan

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 200 - 1)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", h19)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_MC6845_ADD("crtc", MC6845, "screen", XTAL_12_288MHz / 8) // clk taken from schematics
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8) /*?*/
	MCFG_MC6845_UPDATE_ROW_CB(h19_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(INPUTLINE("maincpu", INPUT_LINE_NMI)) // frame pulse

	MCFG_DEVICE_ADD("ins8250", INS8250, XTAL_12_288MHz / 4) // 3.072mhz clock which gets divided down for the various baud rates
	MCFG_INS8250_OUT_INT_CB(WRITELINE(h19_state, h19_ace_irq)) // interrupt

	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(h19_state, h19_kbd_put))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( h19 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// Original
	ROM_SYSTEM_BIOS(0, "orig", "Original")
	ROMX_LOAD( "2732_444-46_h19code.bin", 0x0000, 0x1000, CRC(f4447da0) SHA1(fb4093d5b763be21a9580a0defebed664b1f7a7b), ROM_BIOS(1))
	// Super H19 ROM (
	ROM_SYSTEM_BIOS(1, "super", "Super 19")
	ROMX_LOAD( "2732_super19_h447.bin", 0x0000, 0x1000, CRC(6c51aaa6) SHA1(5e368b39fe2f1af44a905dc474663198ab630117), ROM_BIOS(2))
	// Watzman ROM
	ROM_SYSTEM_BIOS(2, "watzman", "Watzman")
	ROMX_LOAD( "watzman.bin", 0x0000, 0x1000, CRC(8168b6dc) SHA1(bfaebb9d766edbe545d24bc2b6630be4f3aa0ce9), ROM_BIOS(3))

	ROM_REGION( 0x0800, "chargen", 0 )
	// Original font dump
	ROM_LOAD( "2716_444-29_h19font.bin", 0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	ROM_REGION( 0x1000, "keyboard", 0 )
	// Original dump
	ROM_LOAD( "2716_444-37_h19keyb.bin", 0x0000, 0x0800, CRC(5c3e6972) SHA1(df49ce64ae48652346a91648c58178a34fb37d3c))
	// Watzman keyboard
	ROM_LOAD( "keybd.bin", 0x0800, 0x0800, CRC(58dc8217) SHA1(1b23705290bdf9fc6342065c6a528c04bff67b13))
ROM_END

/* Driver (year is either 1978 or 1979) */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1978, h19,     0,       0,    h19,    h19, driver_device,  0,     "Heath Inc", "Heathkit H-19", MACHINE_NOT_WORKING )
