// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

MC-80.20 driver by Miodrag Milanovic

2009-05-12 Skeleton driver.
2009-05-15 Initial implementation
2011-09-01 Modernised, added a keyboard

ToDo:
- Find out if it has sound hardware, add if so
- What port B on PIO is for
- Find out the correct frequencies and connections for the CTC

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "emupal.h"
#include "screen.h"


namespace {

class mc8020_state : public driver_device
{
public:
	mc8020_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_keyboard(*this, "X%u", 0U)
	{ }

	void mc8020(machine_config &config);

private:
	u8 port_b_r();
	void port_a_w(u8 data);
	void port_b_w(u8 data);
	uint32_t screen_update_mc8020(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	u8 m_row = 0U;
	virtual void machine_start() override ATTR_COLD;
	required_shared_ptr<u8> m_p_videoram;
	required_device<z80_device> m_maincpu;
	required_ioport_array<7> m_keyboard;
};

void mc8020_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0fff).ram().share("videoram");// 1KB RAM ZRE
	map(0x2000, 0x5fff).rom();
	map(0x6000, 0xffff).ram();
}

void mc8020_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf3).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xf4, 0xf7).rw("pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}


/* Input ports */
static INPUT_PORTS_START( mc8020 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(34)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('^')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('/')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('?')
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
INPUT_PORTS_END


u8 mc8020_state::port_b_r()
{
	if (m_row == 0x40)
		return m_keyboard[0]->read();
	else
	if (m_row == 0x20)
		return m_keyboard[1]->read();
	else
	if (m_row == 0x10)
		return m_keyboard[2]->read();
	else
	if (m_row == 0x08)
		return m_keyboard[3]->read();
	else
	if (m_row == 0x04)
		return m_keyboard[4]->read();
	else
	if (m_row == 0x02)
		return m_keyboard[5]->read();
	else
	if (m_row == 0x01)
		return m_keyboard[6]->read();
	else
		return 0;
}

void mc8020_state::port_a_w(u8 data)
{
	m_row = data;
}

void mc8020_state::port_b_w(u8 data)
{
}

void mc8020_state::machine_start()
{
	save_item(NAME(m_row));
}


// This is not a content of U402 510
// but order is fine
static const uint8_t prom[] = {
	0x0e,0x11,0x13,0x15,0x17,0x10,0x0e,0x00, // @
	0x04,0x0a,0x11,0x11,0x1f,0x11,0x11,0x00, // A
	0x1e,0x11,0x11,0x1e,0x11,0x11,0x1e,0x00, // B
	0x0e,0x11,0x10,0x10,0x10,0x11,0x0e,0x00, // C
	0x1e,0x09,0x09,0x09,0x09,0x09,0x1e,0x00, // D
	0x1f,0x10,0x10,0x1e,0x10,0x10,0x1f,0x00, // E
	0x1f,0x10,0x10,0x1e,0x10,0x10,0x10,0x00, // F
	0x0e,0x11,0x10,0x10,0x13,0x11,0x0f,0x00, // G

	0x11,0x11,0x11,0x1f,0x11,0x11,0x11,0x00, // H
	0x0e,0x04,0x04,0x04,0x04,0x04,0x0e,0x00, // I
	0x01,0x01,0x01,0x01,0x11,0x11,0x0e,0x00, // J
	0x11,0x12,0x14,0x18,0x14,0x12,0x11,0x00, // K
	0x10,0x10,0x10,0x10,0x10,0x10,0x1f,0x00, // L
	0x11,0x1b,0x15,0x15,0x11,0x11,0x11,0x00, // M
	0x11,0x11,0x19,0x15,0x13,0x11,0x11,0x00, // N
	0x0e,0x11,0x11,0x11,0x11,0x11,0x0e,0x00, // O

	0x1e,0x11,0x11,0x1e,0x10,0x10,0x10,0x00, // P
	0x0e,0x11,0x11,0x11,0x15,0x12,0x0d,0x00, // Q
	0x1e,0x11,0x11,0x1e,0x14,0x12,0x11,0x00, // R
	0x0e,0x11,0x10,0x0e,0x01,0x11,0x0e,0x00, // S
	0x1f,0x04,0x04,0x04,0x04,0x04,0x04,0x00, // T
	0x11,0x11,0x11,0x11,0x11,0x11,0x0e,0x00, // U
	0x11,0x11,0x11,0x0a,0x0a,0x04,0x04,0x00, // V
	0x11,0x11,0x11,0x15,0x15,0x15,0x0a,0x00, // W

	0x11,0x11,0x0a,0x04,0x0a,0x11,0x11,0x00, // X
	0x11,0x11,0x0a,0x04,0x04,0x04,0x04,0x00, // Y
	0x1f,0x01,0x02,0x04,0x08,0x10,0x1f,0x00, // Z
	0x1c,0x10,0x10,0x10,0x10,0x10,0x1c,0x00, // [
	0x00,0x10,0x08,0x04,0x02,0x01,0x00,0x00, // backslash
	0x07,0x01,0x01,0x01,0x01,0x01,0x07,0x00, // ]
	0x0e,0x11,0x00,0x00,0x00,0x00,0x00,0x00, // ^
	0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x00, // _

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //
	0x04,0x04,0x04,0x04,0x04,0x00,0x04,0x00, // !
	0x0a,0x0a,0x0a,0x00,0x00,0x00,0x00,0x00, // "
	0x0a,0x0a,0x1f,0x0a,0x1f,0x0a,0x0a,0x00, // #
	0x00,0x11,0x0e,0x0a,0x0e,0x11,0x00,0x00, // []
	0x18,0x19,0x02,0x04,0x08,0x13,0x03,0x00, // %
	0x04,0x0a,0x0a,0x0c,0x15,0x12,0x0d,0x00, // &
	0x04,0x04,0x08,0x00,0x00,0x00,0x00,0x00, // '

	0x02,0x04,0x08,0x08,0x08,0x04,0x02,0x00, // (
	0x08,0x04,0x02,0x02,0x02,0x04,0x08,0x00, // )
	0x00,0x04,0x15,0x0e,0x15,0x04,0x00,0x00, // *
	0x00,0x04,0x04,0x1f,0x04,0x04,0x00,0x00, // +
	0x00,0x00,0x00,0x00,0x08,0x08,0x10,0x00, // ,
	0x00,0x00,0x00,0x1f,0x00,0x00,0x00,0x00, // -
	0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00, // .
	0x00,0x01,0x02,0x04,0x08,0x10,0x00,0x00, // /

	0x0e,0x11,0x13,0x15,0x19,0x11,0x0e,0x00, // 0
	0x04,0x0c,0x04,0x04,0x04,0x04,0x0e,0x00, // 1
	0x0e,0x11,0x01,0x06,0x08,0x10,0x1f,0x00, // 2
	0x1f,0x01,0x02,0x06,0x01,0x11,0x0e,0x00, // 3
	0x02,0x06,0x0a,0x12,0x1f,0x02,0x02,0x00, // 4
	0x1f,0x10,0x1e,0x01,0x01,0x11,0x0e,0x00, // 5
	0x07,0x08,0x10,0x1e,0x11,0x11,0x0e,0x00, // 6
	0x1f,0x01,0x02,0x04,0x08,0x08,0x08,0x00, // 7

	0x0e,0x11,0x11,0x0e,0x11,0x11,0x0e,0x00, // 8
	0x0e,0x11,0x11,0x0f,0x01,0x02,0x1c,0x00, // 9
	0x00,0x00,0x00,0x00,0x08,0x00,0x08,0x00, // :
	0x00,0x00,0x04,0x00,0x04,0x04,0x08,0x00, // ;
	0x02,0x04,0x08,0x10,0x08,0x04,0x02,0x00, // <
	0x00,0x00,0x1f,0x00,0x1f,0x00,0x00,0x00, // =
	0x08,0x04,0x02,0x01,0x02,0x04,0x08,0x00, // >
	0x0e,0x11,0x01,0x02,0x04,0x00,0x04,0x00  // ?
};


uint32_t mc8020_state::screen_update_mc8020(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy=0,ma=0;

	for (u8 y = 0; y < 8; y++ )
	{
		for (u8 ra = 0; ra < 16; ra++)
		{
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 32; x++)
			{
				u8 gfx;
				if (ra > 3 && ra < 12)
				{
					u8 chr = m_p_videoram[x];
					gfx = prom[(chr<<3) | (ra-4)];
				}
				else
					gfx = 0;

				/* Display a scanline of a character */
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

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};

void mc8020_state::mc8020(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(2'457'600));
	m_maincpu->set_addrmap(AS_PROGRAM, &mc8020_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mc8020_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(32*6, 16*8);
	screen.set_visarea(0, 32*6-1, 0, 16*8-1);
	screen.set_screen_update(FUNC(mc8020_state::screen_update_mc8020));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* devices */
	z80pio_device& pio(Z80PIO(config, "pio", XTAL(2'457'600)));
	pio.out_pa_callback().set(FUNC(mc8020_state::port_a_w));
	pio.in_pb_callback().set(FUNC(mc8020_state::port_b_r));
	pio.out_pb_callback().set(FUNC(mc8020_state::port_b_w));

	z80ctc_device &ctc(Z80CTC(config, "ctc", XTAL(2'457'600)));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.set_clk<2>(XTAL(2'457'600) / 64); // guess
	ctc.zc_callback<2>().set("ctc", FUNC(z80ctc_device::trg1));
	ctc.zc_callback<2>().append("ctc", FUNC(z80ctc_device::trg0));
}


/* ROM definition */
ROM_START( mc8020 )
	ROM_REGION( 0x6000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "ver1", "Version 1")
	ROMX_LOAD( "s01.rom",     0x0000, 0x0400, CRC(0f1c1a62) SHA1(270c0a9e8e165658f3b09d40a3e8bb3dc1b88184), ROM_BIOS(0))
	ROMX_LOAD( "s02.rom",     0x0400, 0x0400, CRC(93b5811c) SHA1(8559d24072c9b5908a2627ff986d818308f51d59), ROM_BIOS(0))
	ROMX_LOAD( "s03.rom",     0x0800, 0x0400, CRC(3d32c334) SHA1(56d3012595540d03054ad3c6795ed5d929581a04), ROM_BIOS(0))
	ROMX_LOAD( "mo01_v2.rom", 0x2000, 0x0400, CRC(7e47201c) SHA1(db49afdc5c1fe4065a979c56cbdbd3c58f5d942f), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "ver2", "Version 2")
	ROMX_LOAD( "s01.rom",    0x0000, 0x0400, CRC(0f1c1a62) SHA1(270c0a9e8e165658f3b09d40a3e8bb3dc1b88184), ROM_BIOS(1))
	ROMX_LOAD( "s02_v2.rom", 0x0400, 0x0400, CRC(dd26c90a) SHA1(1108c11362fa63d21110a3b17868c1854a318c09), ROM_BIOS(1))
	ROMX_LOAD( "s03_v2.rom", 0x0800, 0x0400, CRC(5b64ee7b) SHA1(3b4cbfcb8e2dedcfd4a3680c81fe6ceb2211b275), ROM_BIOS(1))
	ROMX_LOAD( "mo01.rom",   0x2000, 0x0400, CRC(c65a470f) SHA1(71325fed1a342149b5efc2234ecfc8adfff0a42d), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "ver3", "Version 3")
	ROMX_LOAD( "s01.rom",    0x0000, 0x0400, CRC(0f1c1a62) SHA1(270c0a9e8e165658f3b09d40a3e8bb3dc1b88184), ROM_BIOS(2))
	ROMX_LOAD( "s02_v3.rom", 0x0400, 0x0400, CRC(40c7a694) SHA1(bcdf382e8dad9bb6e06d23ec018e9df55c8d8d0c), ROM_BIOS(2))
	ROMX_LOAD( "s03.rom",    0x0800, 0x0400, CRC(3d32c334) SHA1(56d3012595540d03054ad3c6795ed5d929581a04), ROM_BIOS(2))
	ROMX_LOAD( "mo01_v2.rom",0x2000, 0x0400, CRC(7e47201c) SHA1(db49afdc5c1fe4065a979c56cbdbd3c58f5d942f), ROM_BIOS(2))

	// m02 doesn't exist on board
	ROM_LOAD( "mo03.rom", 0x2800, 0x0400, CRC(29685056) SHA1(39e77658fb7af5a28112341f0893e007d73c1b7a))
	ROM_LOAD( "mo04.rom", 0x2c00, 0x0400, CRC(fd315b73) SHA1(cfb943ec8c884a9b92562d05f92faf06fe42ad68))
	ROM_LOAD( "mo05.rom", 0x3000, 0x0400, CRC(453d6370) SHA1(d96f0849a2da958d7e92a31667178ad140719477))
	ROM_LOAD( "mo06.rom", 0x3400, 0x0400, CRC(6357aba5) SHA1(a4867766f6e14e9fe66f22a6839f17c01058c8af))
	ROM_LOAD( "mo07.rom", 0x3800, 0x0400, CRC(a1eb6021) SHA1(b05b63f02de89f065f337bbe54c5b48244e3a4ba))
	ROM_LOAD( "mo08.rom", 0x3c00, 0x0400, CRC(8221cc32) SHA1(65e0ee4241d39d138205c88374b3bcd127e21511))
	ROM_LOAD( "mo09.rom", 0x4000, 0x0400, CRC(7ad5944d) SHA1(ef2781b114277a09ce0cf2e7decfdb7c48a693e3))
	ROM_LOAD( "mo10.rom", 0x4400, 0x0400, CRC(11de8c76) SHA1(b384d22506ff7e3e28bd2dcc33b7a69617eeb52a))
	ROM_LOAD( "mo11.rom", 0x4800, 0x0400, CRC(370cc672) SHA1(133f6e8bfd4e1ca2e9e0a8e2342084419f895e3c))
	ROM_LOAD( "mo12.rom", 0x4c00, 0x0400, CRC(a3838f2b) SHA1(e3602943700bf5068117946bf86f052f5c587169))
	ROM_LOAD( "mo13.rom", 0x5000, 0x0400, CRC(88b61d12) SHA1(00dd4452b4d4191e589ab58ba924ed21b10f323b))
	ROM_LOAD( "mo14.rom", 0x5400, 0x0400, CRC(2168da19) SHA1(c1ce1263167067d8be0a90604d9c29b7379a0545))
	ROM_LOAD( "mo15.rom", 0x5800, 0x0400, CRC(e32f54c4) SHA1(c3d9ca2204e7adbc625cf96031acb8c1df0447c7))
	ROM_LOAD( "mo16.rom", 0x5c00, 0x0400, CRC(403be935) SHA1(4e74355a78ab090ce180437156fed8e4a1d1c787))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                FULLNAME       FLAGS
COMP( 198?, mc8020, 0,      0,      mc8020,  mc8020, mc8020_state, empty_init, "VEB Elektronik Gera", "MC-80.21/22", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
