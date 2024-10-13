// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        Z1013 driver by Miodrag Milanovic

        22/04/2009 Preliminary driver.

The 8x4 keyboard contains letters A-W, enter, cursor right/left, space, and
4 shift keys.
S1 - X-Z, numbers
S2 - a-w
S3 - x-z, punctuation
S4 - control keys

Control Keys:
G - Graphics
A - Alpha
T - clear screen
U - enter
P - cursor left
Q - cursor right

Monitor commands (debug)
R - registers
B - breakpoint
E - execute
G - resume after breakpoint
N - single-step

Monitor commands (general)
H - switch to hex keyboard (H to Q become 0 to 9)
A - switch back to normal alpha keyboard
C - Compare memory blocks
D - Dump memory
F - Find bytes in memory
T - Copy memory block
M - Modify bytes (; to exit)
I - reboot
J - Jump to address
K - Fill memory (K by itself fills all of memory)
L - Load Cassette
S - Save Cassette
W - window (example: W EF00 EFFF)

Due to no input checking, misuse of commands can crash the system.



****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "sound/spkrdev.h"


namespace {

class z1013_state : public driver_device
{
public:
	z1013_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainpio(*this, "z80pio")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
		, m_cass(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_clock_config(*this, "TAKT")
	{ }

	void z1013k76(machine_config &config);
	void z1013a2(machine_config &config);
	void z1013(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(clock_config_changed);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void z1013_keyboard_w(uint8_t data);
	uint8_t port_a_r();
	uint8_t port_b_r();
	uint8_t a2_port_b_r();
	void port_b_w(uint8_t data);
	uint8_t k7659_port_b_r();
	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);
	uint32_t screen_update_z1013(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t m_keyboard_line = 0U;
	bool m_keyboard_part = false;

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_mainpio;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
	required_device<cassette_image_device> m_cass;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	optional_ioport_array<9> m_io_keyboard;
	required_ioport m_clock_config;
};


/* Address maps */
void z1013_state::mem_map(address_map &map)
{
	map(0x0000, 0xebff).ram().share("mainram");
	map(0xec00, 0xefff).ram().share("videoram");
	map(0xf000, 0xffff).rom().region("maincpu",0);
}

void z1013_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_mainpio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x08, 0x08).w(FUNC(z1013_state::z1013_keyboard_w));
}

/* Input ports */
static INPUT_PORTS_START( z1013_8x4 )
	PORT_START("X0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_TAB)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S1") PORT_CODE(KEYCODE_LSHIFT)
	PORT_START("X1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S2") PORT_CODE(KEYCODE_LCONTROL)
	PORT_START("X2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S3") PORT_CODE(KEYCODE_RSHIFT)
	PORT_START("X3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S4") PORT_CODE(KEYCODE_RCONTROL)
	PORT_START("X4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_START("X5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_START("X6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_START("X7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ent") PORT_CODE(KEYCODE_ENTER)
	PORT_START("X8")
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_START("TAKT")
		PORT_CONFNAME(3, 3, "System Clock") PORT_CHANGED_MEMBER(DEVICE_SELF, z1013_state, clock_config_changed, 0)
		PORT_CONFSETTING(3, "1 MHz")
		PORT_CONFSETTING(2, "2 MHz")
		PORT_CONFSETTING(1, "4 MHz")
INPUT_PORTS_END

static INPUT_PORTS_START( z1013_8x8 )
	PORT_START("X0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_START("X1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_START("X2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_START("X3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_START("X4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_COLON)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH)
	PORT_START("X5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_QUOTE)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("_") PORT_CODE(KEYCODE_MINUS)
	PORT_START("X6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Graph") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ent") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_START("X7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_START("X8")
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_START("TAKT")
		PORT_CONFNAME(3, 3, "System Clock") PORT_CHANGED_MEMBER(DEVICE_SELF, z1013_state, clock_config_changed, 0)
		PORT_CONFSETTING(3, "1 MHz")
		PORT_CONFSETTING(2, "2 MHz")
		PORT_CONFSETTING(1, "4 MHz")
INPUT_PORTS_END

static INPUT_PORTS_START( z1013 )
	PORT_START("X8")
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_START("TAKT")
		PORT_CONFNAME(3, 3, "System Clock") PORT_CHANGED_MEMBER(DEVICE_SELF, z1013_state, clock_config_changed, 0)
		PORT_CONFSETTING(3, "1 MHz")
		PORT_CONFSETTING(2, "2 MHz")
		PORT_CONFSETTING(1, "4 MHz")
INPUT_PORTS_END


uint32_t z1013_state::screen_update_z1013(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma=0;

	for (uint8_t y = 0; y < 32; y++)
	{
		for (uint8_t ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix(sy++);

			for (uint16_t x = ma; x < ma+32; x++)
			{
				uint8_t chr = m_p_videoram[x];

				/* get pattern of pixels for that character scanline */
				uint8_t gfx = m_p_chargen[(chr<<3) | ra];

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
		ma+=32;
	}
	return 0;
}

void z1013_state::machine_reset()
{
	m_keyboard_part = false;
	m_keyboard_line = 0U;

	m_maincpu->set_state_int(Z80_PC, 0xf000);

	const XTAL clock = 8_MHz_XTAL / (1 << m_clock_config->read());
	m_maincpu->set_unscaled_clock(clock);
	m_mainpio->set_unscaled_clock(clock);
}

INPUT_CHANGED_MEMBER(z1013_state::clock_config_changed)
{
	const XTAL clock = 8_MHz_XTAL / (1 << newval);
	m_maincpu->set_unscaled_clock(clock);
	m_mainpio->set_unscaled_clock(clock);
}

void z1013_state::machine_start()
{
	save_item(NAME(m_keyboard_line));
	save_item(NAME(m_keyboard_part));
}

void z1013_state::z1013_keyboard_w(uint8_t data)
{
	m_keyboard_line = data & 7;
}

uint8_t z1013_state::port_a_r()
{
	return m_io_keyboard[8]->read();
}

uint8_t z1013_state::port_b_r()
{
	uint8_t data = m_io_keyboard[m_keyboard_line]->read() & 15;

	if (m_cass->input() > 0.03)
		data |= 0x40;

	return data;
}

uint8_t z1013_state::a2_port_b_r()
{
	u8 data;
	if (m_keyboard_part)
		data = BIT(m_io_keyboard[m_keyboard_line]->read(),4,4);
	else
		data = BIT(m_io_keyboard[m_keyboard_line]->read(),0,4);

	if (m_cass->input() > 0.03)
		data |= 0x40;

	return data;
}

void z1013_state::port_b_w(uint8_t data)
{
	m_keyboard_part = BIT(data, 4); // for z1013a2 only
	m_cass->output(BIT(data, 7) ? -1.0 : +1.0);
	m_speaker->level_w(BIT(data, 7) ? -1.0 : +1.0);
}

uint8_t z1013_state::k7659_port_b_r()
{
	return 0xff;
}

SNAPSHOT_LOAD_MEMBER(z1013_state::snapshot_cb)
{
/* header layout
0000,0001 - load address
0002,0003 - end address
0004,0005 - exec address
0006-000B - unknown
000C      - Filetype (appears B=Basic, C=Machine Language, I=?, could be more)
000D-000F - bytes D3, D3, D3
0010-001F - Filename
0020 up   - Program to load
*/

	if (image.length() < 0x20)
		return std::make_pair(image_error::INVALIDIMAGE, "File too short to contain Z1013 image header");

	std::vector<uint8_t> data(image.length());
	image.fread(&data[0], image.length());
	if ((data[13] != data[14]) || (data[14] != data[15]))
		return std::make_pair(image_error::INVALIDIMAGE, "Not a Z1013 image");

	uint16_t const startaddr = data[0] + data[1]*256;
	uint16_t const endaddr   = data[2] + data[3]*256;
	uint16_t const runaddr   = data[4] + data[5]*256;
	if (endaddr < startaddr)
	{
		return std::make_pair(
				image_error::INVALIDIMAGE,
				util::string_format("End address 0x%04X is less than start address 0x%04X", endaddr, startaddr));
	}
	else if ((endaddr - startaddr + 1) > (data.size() - 0x20))
	{
		return std::make_pair(
				image_error::INVALIDIMAGE,
				util::string_format("File too short to contain %u-byte program", endaddr - startaddr + 1));
	}

	memcpy(m_maincpu->space(AS_PROGRAM).get_read_ptr(startaddr), &data[0x20], endaddr - startaddr + 1);

	if (runaddr)
		m_maincpu->set_state_int(Z80_PC, runaddr);
	else
	{
		osd_printf_error("%s: Loaded but cannot run due to zero entry point\n", image.basename());
		image.message(" Loaded but cannot run due to zero entry point");
	}

	return std::make_pair(std::error_condition(), std::string());
}

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	512,                    /* 2 x 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_z1013 )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

/* Machine driver */
void z1013_state::z1013(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &z1013_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &z1013_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(8_MHz_XTAL, 512, 0, 256, 302, 0, 256);
	screen.set_screen_update(FUNC(z1013_state::screen_update_z1013));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_z1013);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	Z80PIO(config, m_mainpio, 8_MHz_XTAL / 8);
	m_mainpio->in_pa_callback().set(FUNC(z1013_state::port_a_r));
	m_mainpio->in_pb_callback().set(FUNC(z1013_state::port_b_r));
	m_mainpio->out_pb_callback().set(FUNC(z1013_state::port_b_w));

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cass->set_interface("z1013_cass");

	snapshot_image_device &snapshot(SNAPSHOT(config, "snapshot", "z80"));
	snapshot.set_delay(attotime::from_seconds(2));
	snapshot.set_load_callback(FUNC(z1013_state::snapshot_cb));
	snapshot.set_interface("z1013_snap");

	SOFTWARE_LIST(config, "cass_list").set_original("z1013_cass");
	SOFTWARE_LIST(config, "snap_list").set_original("z1013_snap");
}

void z1013_state::z1013a2(machine_config &config)
{
	z1013(config);

	m_mainpio->in_pb_callback().set(FUNC(z1013_state::a2_port_b_r));
}

void z1013_state::z1013k76(machine_config &config)
{
	z1013(config);

	m_mainpio->in_pb_callback().set(FUNC(z1013_state::k7659_port_b_r));
	m_mainpio->out_pb_callback().set_nop();
}

/* ROM definition */
ROM_START( z1013 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "202", "Original" )
	ROMX_LOAD( "mon_202.bin", 0x0000, 0x0800, CRC(5884edab) SHA1(c3a45ea5cc4da2b7c270068ba1e2d75916960709), ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "jm", "Jens Muller version" )
	ROMX_LOAD( "mon_jm_1992.bin", 0x0000, 0x0800, CRC(186d2888) SHA1(b52ccb557c41c96bace7db4c4f5031a0cd736168), ROM_BIOS(1))

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD ("z1013font.bin",   0x0000, 0x0800, CRC(7023088f) SHA1(8b197a51c070efeba173d10be197bd41e764358c))
	ROM_LOAD ("altfont.bin",     0x0800, 0x0800, CRC(2dc96f9c) SHA1(d0b9b0751cc1e91be731547f6442c649b6dd6979))
ROM_END

ROM_START( z1013a2 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mon_a2.bin", 0x0000, 0x0800, CRC(98b19b10) SHA1(97e158f589198cb96aae1567ee0aa6e47824027e))

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD ("z1013font.bin",   0x0000, 0x0800, CRC(7023088f) SHA1(8b197a51c070efeba173d10be197bd41e764358c))
	ROM_LOAD ("altfont.bin",     0x0800, 0x0800, CRC(2dc96f9c) SHA1(d0b9b0751cc1e91be731547f6442c649b6dd6979))
ROM_END

ROM_START( z1013k76 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mon_rb_k7659.bin", 0x0000, 0x1000, CRC(b3d88c45) SHA1(0bcd20338cf0706b384f40901b7f8498c6f6c320))

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD ("z1013font.bin",   0x0000, 0x0800, CRC(7023088f) SHA1(8b197a51c070efeba173d10be197bd41e764358c))
	ROM_LOAD ("altfont.bin",     0x0800, 0x0800, CRC(2dc96f9c) SHA1(d0b9b0751cc1e91be731547f6442c649b6dd6979))

	ROM_REGION(0x1000, "k7659",0)
	ROM_LOAD ("k7659n.bin", 0x0000, 0x0800, CRC(7454bf0a) SHA1(b97e7df93778fa371b96b6f4fb1a5b1c8b89d7ba) )
ROM_END

ROM_START( z1013s60 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "v1", "Version 1" )
	ROMX_LOAD( "mon_rb_s6009.bin", 0x0000, 0x1000, CRC(b37faeed) SHA1(ce2e69af5378d39284e8b3be23da50416a0b0fbe), ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "v2", "Version 2" )
	ROMX_LOAD( "4k-moni-k7652.bin", 0x0000, 0x1000, CRC(a1625fce) SHA1(f0847399502b38a73ad26b38ee2d85ba04ab85ec), ROM_BIOS(1))

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD ("z1013font.bin",   0x0000, 0x0800, CRC(7023088f) SHA1(8b197a51c070efeba173d10be197bd41e764358c))
	ROM_LOAD ("altfont.bin",     0x0800, 0x0800, CRC(2dc96f9c) SHA1(d0b9b0751cc1e91be731547f6442c649b6dd6979))
ROM_END

ROM_START( z1013k69 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "4k-moni-k7669.bin", 0x0000, 0x1000, CRC(09cd2a7a) SHA1(0b8500320d464469868a6b48db31105f34710c41))

	ROM_REGION(0x1000, "chargen",0)
	ROM_LOAD ("z1013font.bin",   0x0000, 0x0800, CRC(7023088f) SHA1(8b197a51c070efeba173d10be197bd41e764358c))
	ROM_LOAD ("altfont.bin",     0x0800, 0x0800, CRC(2dc96f9c) SHA1(d0b9b0751cc1e91be731547f6442c649b6dd6979))
ROM_END
/* Driver */

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT      CLASS        INIT        COMPANY                           FULLNAME               FLAGS
COMP( 1985, z1013,    0,      0,      z1013,    z1013_8x4, z1013_state, empty_init, "VEB Robotron Electronics Riesa", "Z1013 (matrix 8x4)",  MACHINE_SUPPORTS_SAVE )
COMP( 1985, z1013a2,  z1013,  0,      z1013a2,  z1013_8x8, z1013_state, empty_init, "VEB Robotron Electronics Riesa", "Z1013 (matrix 8x8)",  MACHINE_SUPPORTS_SAVE )
COMP( 1985, z1013k76, z1013,  0,      z1013k76, z1013,     z1013_state, empty_init, "VEB Robotron Electronics Riesa", "Z1013 (K7659)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1985, z1013s60, z1013,  0,      z1013k76, z1013_8x8, z1013_state, empty_init, "VEB Robotron Electronics Riesa", "Z1013 (K7652/S6009)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1985, z1013k69, z1013,  0,      z1013k76, z1013,     z1013_state, empty_init, "VEB Robotron Electronics Riesa", "Z1013 (K7669)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
