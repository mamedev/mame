// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/******************************************************************************

LLC1 driver by Miodrag Milanovic

2009-04-17 Preliminary driver.

2012-07-?? Updates by Robbbert

Handy addresses: (Press X then the 4 digits then Enter)
  0800 = BASIC cold start
  0803 = BASIC warm start
  13BE = display Monitor logo
This machine has an 8-digit LED display with hex keyboard,
and also a 64x16 monochrome screen with full keyboard.
The monitor uses the hex keyboard, while Basic uses the full keyboard.
Monitor output is on the digits, but the single-step command displays
a running register dump on the main screen.
There are no storage facilities, and no sound.
BASIC is integer only (-32768 to 32767), about 6k of space, and all
input is to be in uppercase. It does have minimal string capability,
but there is no $ sign, so how to create a string variable is unknown.
To exit back to the monitor, type BYE.
The user instructions of the monitor (in German) are here:
http://www.jens-mueller.org/jkcemu/llc1.html

ToDo:
- Get good dump of monitor rom, has one known bad byte, possibly more.
- In Basic, when it first scrolls, the start of the line shifts a character
  to the right. Unknown if a bug or a bad byte. (patched)


*******************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/keyboard.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"

#include "emupal.h"
#include "screen.h"

#include "llc1.lh"


namespace {

class llc1_state : public driver_device
{
public:
	llc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ctc(*this, "ctc")
		, m_p_chargen(*this, "chargen")
		, m_vram(*this, "videoram")
		, m_inputs(*this, "X%u", 4U)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void llc1(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(z3_button);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void kbd_put(u8 data);
	u8 port1a_r();
	u8 port2a_r();
	u8 port2b_r();
	void port1a_w(u8 data);
	void port1b_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_porta = 0U;
	u8 m_term_data = 0U;
	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_vram;
	required_ioport_array<3> m_inputs;
	output_finder<8> m_digits;
};


/* Address maps */
void llc1_state::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).rom(); // Monitor ROM
	map(0x0800, 0x13ff).rom(); // BASIC ROM
	map(0x1400, 0x1bff).ram(); // RAM
	map(0x1c00, 0x1fff).ram().share("videoram"); // Video RAM
}

void llc1_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xEC, 0xEF).rw("pio2", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xF4, 0xF7).rw("pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xF8, 0xFB).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

/* Input ports */
static INPUT_PORTS_START( llc1 )
	PORT_START("X4") // out F4,BF
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
	PORT_START("X5") // out F4,DF
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M (Mem)") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ST (Start)") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('^') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DF (Reset)") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27) PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
	PORT_START("X6") // out F4,EF
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ES (Step)") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DL (Go)") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HP (BP)") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHANGED_MEMBER(DEVICE_SELF, llc1_state, z3_button, 0)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) // does nothing
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(llc1_state::z3_button)
{
	m_ctc->trg3(newval);
}

void llc1_state::kbd_put(u8 data)
{
	static constexpr u8 s1[16]={0x40, 0x1e, 0x12, 0x1b, 0x19, 0x14, 0x15, 0x1d, 0x16, 0x17, 0x1c, 0x3c, 0x3f, 0x3d, 0x3e, 0x10}; // 0x20 to 0x2F
	static constexpr u8 s2[7] ={0x1a, 0x11, 0x7c, 0x13, 0x7b, 0x1f, 0x00}; // 0x3A to 0x40
	static constexpr u8 s3[6] ={0x5c, 0x00, 0x5b, 0x7e, 0x00, 0x5e}; // 0x5B to 0x60

	m_term_data = data;

	if ((data >= 0x20) && (data <= 0x2f))
		m_term_data = s1[data-0x20];
	else if ((data >= 0x3a) && (data <= 0x40))
		m_term_data = s2[data-0x3a];
	else if ((data >= 0x5b) && (data <= 0x60))
		m_term_data = s3[data-0x5b];
	else if (data >= 0x7b)
		m_term_data = 0;

	if (m_term_data)
		m_term_data |= 0x80;
}

// LLC1 BASIC keyboard
u8 llc1_state::port2b_r()
{
	if (BIT(m_term_data, 7))
	{
		m_term_data &= 0x7f;
		return 0xff;
	}
	else
		return m_term_data;
}

u8 llc1_state::port2a_r()
{
	return 0;
}

// LLC1 Monitor keyboard
u8 llc1_state::port1a_r()
{
	u8 data = 0;
	if (!BIT(m_porta, 4))
		data = m_inputs[0]->read();
	if (!BIT(m_porta, 5))
		data = m_inputs[1]->read();
	if (!BIT(m_porta, 6))
		data = m_inputs[2]->read();
	if (data & 0xf0)
		data = (data >> 4) | 0x80;

	return data;
}

void llc1_state::port1a_w(u8 data)
{
	m_porta = data;
}

void llc1_state::port1b_w(u8 data)
{
	static u8 count = 0, digit = 0;

	if (data == 0)
	{
		digit = 0;
		count = 0;
	}
	else
		count++;

	if (count == 1)
	{
		if (digit < 8)
			m_digits[digit] = data & 0x7f;
	}
	else
	if (count == 3)
	{
		count = 0;
		digit++;
	}
}

void llc1_state::machine_reset()
{
	m_term_data = 0;
}

void llc1_state::machine_start()
{
	m_digits.resolve();
	save_item(NAME(m_porta));
	save_item(NAME(m_term_data));
}

u32 llc1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0,ma=0;

	for (u8 y = 0; y < 16; y++)
	{
		for (u8 ra = 0; ra < 8; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 64; x++)
			{
				u8 const inv = (m_vram[x] & 0x80) ? 0xff : 0;
				u8 const chr = m_vram[x] & 0x7f;

				/* get pattern of pixels for that character scanline */
				u8 const gfx = m_p_chargen[ chr | (ra << 7) ] ^ inv;

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
		ma+=64;
	}
	return 0;
}

static const z80_daisy_config daisy_chain[] =
{
	{ "pio2" },
	{ "ctc" },
	{ nullptr }
};

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8 },
	8                   /* every char takes 8 x 1 bytes */
};

static GFXDECODE_START( gfx_llc1 )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

/* Machine driver */
void llc1_state::llc1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(2'000'000));
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &llc1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &llc1_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 16*8);
	screen.set_visarea(0, 64*8-1, 0, 16*8-1);
	screen.set_screen_update(FUNC(llc1_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_llc1);
	PALETTE(config, "palette", palette_device::MONOCHROME);
	config.set_default_layout(layout_llc1);

	z80pio_device& pio1(Z80PIO(config, "pio1", XTAL(2'000'000)));
	pio1.in_pa_callback().set(FUNC(llc1_state::port1a_r));
	pio1.out_pa_callback().set(FUNC(llc1_state::port1a_w));
	pio1.out_pb_callback().set(FUNC(llc1_state::port1b_w));

	z80pio_device& pio2(Z80PIO(config, "pio2", XTAL(2'000'000)));
	pio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio2.in_pa_callback().set(FUNC(llc1_state::port2a_r));
	pio2.in_pb_callback().set(FUNC(llc1_state::port2b_r));

	Z80CTC(config, m_ctc, XTAL(2'000'000));
	// timer 0 irq does digit display
	// timer 3 is kicked off by a key press, and scans the monitor keyboard.
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<1>().set(m_ctc, FUNC(z80ctc_device::trg2));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(llc1_state::kbd_put));
}

/* ROM definition */

ROM_START( llc1 )
	ROM_REGION( 0x1400, "maincpu", 0 )
	//ROM_LOAD( "llc1-monitor.rom", 0x0000, 0x0800, BAD_DUMP CRC(0e81378d) SHA1(0fbb6eca016d0f439ea1c9aa0cb0affb5f49ea69) )
	ROM_LOAD( "llc1mon.bin",  0x0000, 0x0800, BAD_DUMP CRC(e291dd63) SHA1(31a71bef84f7c164a270d0895cb645e078e9c6f2) )
	ROM_LOAD( "llc1_tb1.bin", 0x0800, 0x0400, CRC(0d9d4039) SHA1(b515e385af57f4faf3a9f7b4a1edd59a1c1ea260) )
	ROM_LOAD( "llc1_tb2.bin", 0x0c00, 0x0400, CRC(28bfea2a) SHA1(a68a8b87bfc931627ddd8d124b153e511477fbaf) )
	ROM_LOAD( "llc1_tb3.bin", 0x1000, 0x0400, CRC(fe5e3132) SHA1(cc3b191e41f5772a4b86b8eb0ebe6fce67872df6) )
	ROM_FILL(0x02dc, 1, 0x0f) // fix display of AF in the reg command (confirmed from monitor listing)
	ROM_FILL(0x1361, 1, 0x40) // fix scrolling in Basic

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD ("llc1_zg.bin",  0x0000, 0x0400, CRC(fa2cd659) SHA1(1fa5f9992f35929f656c4ce55ed6980c5da1772b) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT       COMPANY  FULLNAME  FLAGS */
COMP( 1984, llc1, 0,      0,      llc1,    llc1,  llc1_state, empty_init, "SCCH",  "LLC-1",  MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
