// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/******************************************************************************

        LLC1 driver by Miodrag Milanovic

        17/04/2009 Preliminary driver.

        July 2012, updates by Robbbert

        Very little info available on these computers.

        LLC1:
        Handy addresses (set the pc register in the debugger, because the
        monitor's Go command has problems):
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
        - LLC1: Get good dump of monitor rom, has a number of bad bytes
        - LLC1: In Basic, pressing enter several times causes the start
          of the line to be shifted 1 or 2 characters to the right.
        - LLC1: Go command crashes
        - Lots of other things

*******************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/keyboard.h"
#include "emupal.h"
#include "screen.h"

#include "llc1.lh"


class llc1_state : public driver_device
{
public:
	llc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_vram(*this, "videoram")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void llc1(machine_config &config);

private:
	void machine_start() override;
	void machine_reset() override;
	void kbd_put(u8 data);
	u8 llc1_port1_a_r();
	u8 llc1_port2_a_r();
	u8 llc1_port2_b_r();
	void llc1_port1_a_w(u8 data);
	void llc1_port1_b_w(u8 data);
	u32 screen_update_llc1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	u8 m_term_status;
	u8 m_llc1_key;
	u8 m_porta;
	u8 m_term_data;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_vram;
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
	map(0xEC, 0xEF).rw("z80pio2", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xF4, 0xF7).rw("z80pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xF8, 0xFB).rw("z80ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

/* Input ports */
static INPUT_PORTS_START( llc1 )
	PORT_START("X4") // out F4,BF
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_START("X5") // out F4,DF
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M (Mem)") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ST (Start)") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('^')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) // resets
	PORT_START("X6") // out F4,EF
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ES (Step)") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DL (Go)") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HP (BP)") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) // does nothing
INPUT_PORTS_END

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
		m_term_status = 0xff;
}

// LLC1 BASIC keyboard
u8 llc1_state::llc1_port2_b_r()
{
	u8 retVal = 0;

	if (m_term_status)
	{
		retVal = m_term_status;
		m_term_status = 0;
	}
	else
		retVal = m_term_data;

	return retVal;
}

u8 llc1_state::llc1_port2_a_r()
{
	return 0;
}

// LLC1 Monitor keyboard
u8 llc1_state::llc1_port1_a_r()
{
	u8 data = 0;
	if (!BIT(m_porta, 4))
		data = ioport("X4")->read();
	if (!BIT(m_porta, 5))
		data = ioport("X5")->read();
	if (!BIT(m_porta, 6))
		data = ioport("X6")->read();
	if (data & 0xf0)
		data = (data >> 4) | 0x80;

	data |= (m_porta & 0x70);

	// do not repeat key
	if (data & 15)
	{
		if (data == m_llc1_key)
			data &= 0x70;
		else
			m_llc1_key = data;
	}
	else
	if ((data & 0x70) == (m_llc1_key & 0x70))
		m_llc1_key = 0;

	return data;
}

void llc1_state::llc1_port1_a_w(u8 data)
{
	m_porta = data;
}

void llc1_state::llc1_port1_b_w(u8 data)
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
	m_term_status = 0;
	m_llc1_key = 0;
}

void llc1_state::machine_start()
{
	m_digits.resolve();

	save_item(NAME(m_term_status));
	save_item(NAME(m_llc1_key));
	save_item(NAME(m_porta));
	save_item(NAME(m_term_data));
}

u32 llc1_state::screen_update_llc1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 y,ra,chr,gfx,inv;
	u16 sy=0,ma=0,x;

	for (y = 0; y < 16; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			u16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				inv = (m_vram[x] & 0x80) ? 0xff : 0;
				chr = m_vram[x] & 0x7f;

				/* get pattern of pixels for that character scanline */
				gfx = m_p_chargen[ chr | (ra << 7) ] ^ inv;

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

static const z80_daisy_config llc1_daisy_chain[] =
{
	{ "z80ctc" },
	{ nullptr }
};

/* F4 Character Displayer */
static const gfx_layout llc1_charlayout =
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
	GFXDECODE_ENTRY( "chargen", 0x0000, llc1_charlayout, 0, 1 )
GFXDECODE_END

/* Machine driver */
void llc1_state::llc1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(3'000'000));
	m_maincpu->set_daisy_config(llc1_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &llc1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &llc1_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 16*8);
	screen.set_visarea(0, 64*8-1, 0, 16*8-1);
	screen.set_screen_update(FUNC(llc1_state::screen_update_llc1));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_llc1);
	PALETTE(config, "palette", palette_device::MONOCHROME);
	config.set_default_layout(layout_llc1);

	z80pio_device& pio1(Z80PIO(config, "z80pio1", XTAL(3'000'000)));
	pio1.in_pa_callback().set(FUNC(llc1_state::llc1_port1_a_r));
	pio1.out_pa_callback().set(FUNC(llc1_state::llc1_port1_a_w));
	pio1.out_pb_callback().set(FUNC(llc1_state::llc1_port1_b_w));

	z80pio_device& pio2(Z80PIO(config, "z80pio2", XTAL(3'000'000)));
	pio2.in_pa_callback().set(FUNC(llc1_state::llc1_port2_a_r));
	pio2.in_pb_callback().set(FUNC(llc1_state::llc1_port2_b_r));

	z80ctc_device& ctc(Z80CTC(config, "z80ctc", XTAL(3'000'000)));
	// timer 0 irq does digit display, and timer 3 irq does scan of the
	// monitor keyboard.
	// No idea how the CTC is connected, so guessed.
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.zc_callback<0>().set("z80ctc", FUNC(z80ctc_device::trg1));
	ctc.zc_callback<1>().set("z80ctc", FUNC(z80ctc_device::trg3));

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
	ROM_FILL(0x23b, 1, 0x00) // don't reboot when typing into the monitor
	ROM_FILL(0x2dc, 1, 0x0f) // fix display of AF in the reg command

	ROM_REGION( 0x0400, "chargen", 0 )
	ROM_LOAD ("llc1_zg.bin",  0x0000, 0x0400, CRC(fa2cd659) SHA1(1fa5f9992f35929f656c4ce55ed6980c5da1772b) )
ROM_END


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT       COMPANY  FULLNAME  FLAGS */
COMP( 1984, llc1, 0,      0,      llc1,    llc1,  llc1_state, empty_init, "SCCH",  "LLC-1",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
