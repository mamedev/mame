// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Homebrew Z80 Computer by Kun-Szabo Marton

            http://digitarworld.uw.hu/z80.htm

        31/10/2010 Initial driver by Miodrag Milanovic

        All commands must be entered in uppercase, and since the capslock
        doesn't work, you need to hold the shift key down.

        There is next to no error checking, for example the T command
        is to set the time. Entering T by itself will set the time to
        99:99:99, while G will cause a jump to 9999 and so forth. Also
        the parameter must be right next to the command, spaces will cause
        invalid input. Example M1234 will display a byte of memory at 1234,
        while M 1234 will display memory at 9123.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"


class homez80_state : public driver_device
{
public:
	homez80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void homez80(machine_config &config);

private:

	DECLARE_READ8_MEMBER( homez80_keyboard_r );
	INTERRUPT_GEN_MEMBER(homez80_interrupt);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void homez80_io(address_map &map);
	void homez80_mem(address_map &map);

	bool m_irq;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};


READ8_MEMBER( homez80_state::homez80_keyboard_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"LINE%d",offset);
	return ioport(kbdrow)->read();
}

void homez80_state::homez80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();  // Monitor
	map(0x2000, 0x23ff).ram().share("videoram"); // Video RAM
	map(0x7020, 0x702f).r(FUNC(homez80_state::homez80_keyboard_r));
	map(0x8000, 0xffff).ram();  // 32 K RAM
}

void homez80_state::homez80_io(address_map &map)
{
	map.unmap_value_high();
}

/* Input ports */
INPUT_PORTS_START( homez80 )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)
	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)
	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)
	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)
	PORT_START("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)
	PORT_START("LINE8")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SCRLOCK)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PRTSCR)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE9")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("LINE10")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)
	PORT_START("LINE11")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)
	PORT_START("LINE12")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)
	PORT_START("LINE13")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)
	PORT_START("LINE14")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)
	PORT_START("LINE15")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void homez80_state::machine_reset()
{
	m_irq = 0;
}

uint32_t homez80_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;

	for (y = 0; y < 32; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++, 44);

			for (x = ma; x < ma+32; x++)
			{
				chr = m_p_videoram[x];

				/* get pattern of pixels for that character scanline */
				gfx = m_p_chargen[ (chr<<3) | ra];

				/* Display a scanline of a character (8 pixels) */
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
		ma+=32;
	}
	return 0;
}


const gfx_layout homez80_charlayout =
{
	8, 8,               /* 8x8 characters */
	256,                /* 256 characters */
	1,                  /* 1 bits per pixel */
	{0},                /* no bitplanes; 1 bit per pixel */
	{0, 1, 2, 3, 4, 5, 6, 7},
	{0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8},
	8*8                 /* size of one char */
};

static GFXDECODE_START( gfx_homez80 )
	GFXDECODE_ENTRY( "chargen", 0x0000, homez80_charlayout, 0, 1 )
GFXDECODE_END


INTERRUPT_GEN_MEMBER(homez80_state::homez80_interrupt)
{
	device.execute().set_input_line(0, (m_irq) ? HOLD_LINE : CLEAR_LINE);
	m_irq ^= 1;
}

void homez80_state::homez80(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &homez80_state::homez80_mem);
	m_maincpu->set_addrmap(AS_IO, &homez80_state::homez80_io);
	m_maincpu->set_periodic_int(FUNC(homez80_state::homez80_interrupt), attotime::from_hz(50));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(homez80_state::screen_update));
	screen.set_size(344, 32*8);
	screen.set_visarea(0, 344-1, 0, 32*8-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
	GFXDECODE(config, "gfxdecode", "palette", gfx_homez80);
}

/* ROM definition */
ROM_START( homez80 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sysrom.bin",  0x0000, 0x1000, CRC(37ca7545) SHA1(3f597d7e45b1ab211d5bd4a99abb21915723c357) )

	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD( "chargen.bin", 0x0000, 0x0800, CRC(93243be3) SHA1(718efc06c131843c15383e50af23f3a5cf44dd9b) )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY             FULLNAME                 FLAGS */
COMP( 2008, homez80, 0,      0,      homez80, homez80, homez80_state, empty_init, "Kun-Szabo Marton", "Homebrew Z80 Computer", MACHINE_NO_SOUND_HW)
