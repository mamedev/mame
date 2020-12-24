// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*******************************************************************************

Driver file to handle emulation of the Magnavox Odyssey 2 (stylized Odyssey²),
Philips Videopac G7000 and Philips Videopac+ G7400.

Magnavox was wholly owned by Philips at the time. The console had a limited
release late-1978 in Europe as the Philips G7000, but the launch was quickly
halted due to a hardware defect, wide release continued in 1979.

The 2 joysticks have no clear distinction between player 1 and 2, it differs
per game. And in MAME it's extra awkward due to the default input mapping
conflicting with the keyboard. An easy way to work around this if you're only
playing 1-player games, is to map both joysticks to the same inputs.

Videopac consoles:
- Philips Videopac G7000 (Europe, several models)
- Philips Videopac C52 (France)
- Philips Odyssey (Brazil)
- Magnavox Odyssey 2 (US)
- Radiola Jet 25 (France)
- Siera Videopac Computer G7000 (France)
- Schneider Videopac 7000 (Germany)
- Philips Videopac G7200 (Europe, Videopac with built-in screen)
- Philips Videojeu N60 (France)

Videopac+ consoles:
- Philips Videopac+ G7400/G7401 (Europe)
- Magnavox Odyssey 3 Command Center (US, prototype)
- Brandt Jopac JO7400 (France)
- Schneider Videopac 74+ (Germany)

Odyssey 2/Videopac hardware notes:
- Intel 8048 (1KB internal ROM, 64 bytes internal RAM)
- 128 bytes RAM(6810)
- Intel 8244 for video and sound (8245 on PAL consoles)
- 2 joysticks(either hardwired, or connectors), keyboard

Videopac+ G7400 hardware notes:
- same base hardware
- Intel 8243 I/O expander
- EF9340 + EF9341 graphics chips + 6KB VRAM(3*2128, only 4KB used)
- larger keyboard

XTAL notes (differs per model):
- Odyssey 2: 7.15909MHz
- G7000: 17.734476MHz
- C52/N60: 17.812MHz
- G7200: 5.911MHz + 3.547MHz
- G7400: 5.911MHz + 8.867MHz
- JO7400: 5.911MHz + 3.5625MHz

TODO:
- backgamm doesn't draw all the chars/sprites, it does multiple screen updates
  and writes to the ptr/color registers, but does not increment the Y regs
- screen resolution is not strictly defined, height(243) is correct, but
  horizontal overscan differs depending on monitor/tv? see syracuse for overscan
- 824x on the real console, overlapping characters on eachother will cause
  glitches (it is used to an advantage in some as-of-yet undumped homebrews)
- 8244(NTSC) is not supposed to show characters near the upper border, but
  hiding them will cause bugs in some Euro games
- 8245(PAL) video timing is not 100% accurate, though vtotal and htotal should
  be correct
- according to tests, 8244 does not have a sound interrupt, but the Philips
  service test cartridge for 8245 tests for it and fails if it did not get an irq
- likewise, 8244 does not have a horizontal interrupt, but does 8245 have it?
- tests done on 8244 suggests that Y(0xa4) is latched when reading X, but
  that is inconsistent with the Philips service test cartridge: It reads X, Y, X,
  then waits for 1 scanline, and reads Y again. It expects Y to change. Latching Y
  will also cause video glitches to look different on some games when compared
  to the real console, for example powerlrd.
- ppp(the tetris game) does not work properly on PAL, it does look like PAL/NTSC
  detection is working, see internal RAM $3D d7. So maybe it is due to inaccurate
  PAL video timing. The game does mid-scanline video updates.
- g7400 probably has different video timing too (not same as g7000)
- g7400 helicopt sometimes locks up at the sea level, timing related?
- 4in1 and musician are not supposed to work on g7400, but work fine on MAME,
  caused by bus conflict or because they write to P2?
- verify odyssey3 cpu/video clocks
- problems with natural keyboard: videopacp has two enter keys, odyssey3 has
  alternate inputs for -, =, +
- partial screen updates aren't shown when using MAME's debugger, this is caused
  by a forced full screen update and a reset_partial_updates in emu/video.cpp.
  For the same reason, collision detection also won't work properly when stepping
  through the debugger

BTANB:
- a lot of PAL games have problems on NTSC (the other way around, not so much)
- g7400 games don't look correct on odyssey3 and vice versa: ef934x graphics are
  placed lower on odyssey3
- Blackjack (Videopac 5) does not work on G7400, caused by a removed BIOS routine
- due to different XTAL ratio on Jopac JO7400, some games that do mid-screen video
  updates will have glitches on this machine, notably backgamm

Plenty games have minor bugs not worth mentioning here.

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/i8243.h"
#include "video/ef9340_1.h"
#include "video/i8244.h"

#include "bus/odyssey2/slot.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


namespace {

class odyssey2_state : public driver_device
{
public:
	odyssey2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_i8244(*this, "i8244"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_cart(*this, "cartslot"),
		m_keyboard(*this, "KEY.%u", 0),
		m_joysticks(*this, "JOY.%u", 0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(palette_changed) { adjust_palette(); }

	// Reset button is tied to 8048 RESET pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

	void odyssey2(machine_config &config);
	void videopac(machine_config &config);
	void videopacf(machine_config &config);

	void odyssey2_palette(palette_device &palette) const;

protected:
	required_device<i8048_device> m_maincpu;
	required_device<i8244_device> m_i8244;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<o2_cart_slot_device> m_cart;
	required_ioport_array<8> m_keyboard;
	required_ioport_array<2> m_joysticks;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void adjust_palette();

	virtual u8 io_read(offs_t offset);
	virtual void io_write(offs_t offset, u8 data);
	u8 bus_read();
	void p1_write(u8 data);
	u8 p2_read();
	void p2_write(u8 data);
	DECLARE_READ_LINE_MEMBER(t1_read);

	void odyssey2_io(address_map &map);
	void odyssey2_mem(address_map &map);

	u8 m_ram[0x80];
	u8 m_p1 = 0xff;
	u8 m_p2 = 0xff;

private:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

class vpp_state : public odyssey2_state
{
public:
	vpp_state(const machine_config &mconfig, device_type type, const char *tag) :
		odyssey2_state(mconfig, type, tag),
		m_i8243(*this, "i8243"),
		m_ef934x(*this, "ef934x")
	{ }

	void g7400(machine_config &config);
	void jo7400(machine_config &config);
	void odyssey3(machine_config &config);

protected:
	virtual void machine_start() override;

	virtual u8 io_read(offs_t offset) override;
	virtual void io_write(offs_t offset, u8 data) override;

private:
	required_device<i8243_device> m_i8243;
	required_device<ef9340_1_device> m_ef934x;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void p2_write(u8 data);
	u8 io_vpp(offs_t offset, u8 data);
	template<int P> void i8243_port_w(u8 data);

	inline offs_t ef934x_extram_address(offs_t offset);
	u8 ef934x_extram_r(offs_t offset);
	void ef934x_extram_w(offs_t offset, u8 data);

	u8 m_mix_i8244 = 0xff;
	u8 m_mix_ef934x = 0xff;
	u8 m_ef934x_extram[0x800];
};

void odyssey2_state::machine_start()
{
	memset(m_ram, 0, sizeof(m_ram));

	save_item(NAME(m_ram));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
}

void odyssey2_state::machine_reset()
{
	adjust_palette();
}

void vpp_state::machine_start()
{
	odyssey2_state::machine_start();
	memset(m_ef934x_extram, 0, sizeof(m_ef934x_extram));

	save_item(NAME(m_mix_i8244));
	save_item(NAME(m_mix_ef934x));
	save_item(NAME(m_ef934x_extram));
}



/******************************************************************************
    Video
******************************************************************************/

constexpr rgb_t odyssey2_colors[] =
{
	// Background,Grid Dim
	{ 0x00, 0x00, 0x00 },   /* Black */                                         // i r g b
	{ 0x79, 0x00, 0x00 },   /* Red            - Calibrated To Real VideoPac */  // i R g b
	{ 0x00, 0x6d, 0x07 },   /* Green          - Calibrated To Real VideoPac */  // i r G b
	{ 0x77, 0x67, 0x0b },   /* Khaki          - Calibrated To Real VideoPac */  // i R g B
	{ 0x1a, 0x37, 0xbe },   /* Blue           - Calibrated To Real VideoPac */  // i r g B
	{ 0x94, 0x30, 0x9f },   /* Violet         - Calibrated To Real VideoPac */  // i R g B
	{ 0x2a, 0xaa, 0xbe },   /* Blue-Green     - Calibrated To Real VideoPac */  // i r G B
	{ 0xce, 0xce, 0xce },   /* Lt Grey */                                       // i R G B

	// Background,Grid Bright
	{ 0x67, 0x67, 0x67 },   /* Grey           - Calibrated To Real VideoPac */  // I R g B
	{ 0xc7, 0x51, 0x51 },   /* Lt Red         - Calibrated To Real VideoPac */  // I R g b
	{ 0x56, 0xc4, 0x69 },   /* Lt Green       - Calibrated To Real VideoPac */  // I R g B
	{ 0xc6, 0xb8, 0x6a },   /* Lt Yellow      - Calibrated To Real VideoPac */  // I R G b
	{ 0x5c, 0x80, 0xf6 },   /* Lt Blue        - Calibrated To Real VideoPac */  // I R g B
	{ 0xdc, 0x84, 0xe8 },   /* Lt Violet      - Calibrated To Real VideoPac */  // I R g B
	{ 0x77, 0xe6, 0xeb },   /* Lt Blue-Green  - Calibrated To Real VideoPac */  // I R g b
	{ 0xff, 0xff, 0xff }    /* White */                                         // I R G B
};

void odyssey2_state::odyssey2_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, odyssey2_colors);
}

void odyssey2_state::adjust_palette()
{
	// JO7400 has an RGB port, on other consoles it's an optional homebrew modification
	if (ioport("CONF")->read() & 1)
		m_i8244->i8244_palette(*m_palette);
	else
		odyssey2_palette(*m_palette);
}

u32 odyssey2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_i8244->screen_update(screen, bitmap, cliprect);

	u8 lum = ~m_p1 >> 4 & 0x08;

	// apply external LUM setting
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			bitmap.pix(y, x) |= lum;

	return 0;
}

u32 vpp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 lum = ~m_p1 >> 4 & 0x08;
	bitmap_ind16 *ef934x_bitmap = m_ef934x->get_bitmap();

	// apply external LUM setting
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		rectangle clip = cliprect;
		clip.min_y = clip.max_y = y;

		m_i8244->screen_update(screen, bitmap, clip);

		for (int x = clip.min_x; x <= clip.max_x; x++)
		{
			u16 d = bitmap.pix(y, x) & 7;
			u16 e = ef934x_bitmap->pix(y, x);

			// i8244 decoder enable is masked with cartridge pin B
			bool en = (e & 8) || !m_cart->b_read();
			e &= 7;

			// ef934x decoder output is tied to CX
			bool i2 = !BIT(m_mix_ef934x, e);
			m_i8244->write_cx(x, i2);

			if (en && BIT(m_mix_i8244, d))
			{
				// Use i8245 input
				bitmap.pix(y, x) |= lum;
			}
			else
			{
				// Use EF934x input
				bitmap.pix(y, x) = e | (i2 ? 8 : 0);
			}
		}
	}

	return 0;
}



/******************************************************************************
    I/O
******************************************************************************/

// 8048 ports

u8 odyssey2_state::io_read(offs_t offset)
{
	u8 data = m_cart->io_read(offset);
	if (!(m_p1 & 0x10) && ~offset & 0x80)
		data &= m_ram[offset];

	if ((m_p1 & 0x48) == 0)
		data &= m_i8244->read(offset);

	return data;
}

void odyssey2_state::io_write(offs_t offset, u8 data)
{
	if (!(m_p1 & 0x40))
	{
		m_cart->io_write(offset, data);
		if (!(m_p1 & 0x10) && ~offset & 0x80)
			m_ram[offset] = data;
	}

	if (!(m_p1 & 0x08))
		m_i8244->write(offset, data);
}

void odyssey2_state::p1_write(u8 data)
{
	// LUM changed
	if ((m_p1 ^ data) & 0x80)
		m_screen->update_now();

	m_p1 = data;
	m_cart->write_p1(m_p1 & 0x13);
}

u8 odyssey2_state::p2_read()
{
	u8 data = 0xff;

	if (!(m_p1 & 0x04))
	{
		// 74148 priority encoder, GS to P24, outputs to P25-P27
		u8 inp = count_leading_zeros(m_keyboard[m_p2 & 0x07]->read()) - 24;
		if (inp < 8)
			data &= inp << 5 | 0xf;
	}

	return data;
}

void odyssey2_state::p2_write(u8 data)
{
	m_p2 = data;
	m_cart->write_p2(m_p2 & 0x0f);
}

u8 odyssey2_state::bus_read()
{
	u8 data = 0xff;

	if (!(m_p1 & 0x04))
	{
		u8 sel = m_p2 & 0x07;
		if (sel < 2)
			data &= ~m_joysticks[sel]->read();
	}

	return data;
}

READ_LINE_MEMBER(odyssey2_state::t1_read)
{
	return m_i8244->vblank() | m_i8244->hblank();
}


// G7400-specific

u8 vpp_state::io_read(offs_t offset)
{
	u8 data = odyssey2_state::io_read(offset);
	return io_vpp(offset, data);
}

void vpp_state::io_write(offs_t offset, u8 data)
{
	odyssey2_state::io_write(offset, data);
	io_vpp(offset, data);
}

u8 vpp_state::io_vpp(offs_t offset, u8 data)
{
	if (!(m_p1 & 0x20))
	{
		// A2 to R/W pin
		if (offset & 4)
			data &= m_ef934x->ef9341_read(offset & 0x02, offset & 0x01);
		else
			m_ef934x->ef9341_write(offset & 0x02, offset & 0x01, data);
	}

	return data;
}

void vpp_state::p2_write(u8 data)
{
	odyssey2_state::p2_write(data);
	m_i8243->p2_w(m_p2 & 0x0f);
}

template<int P>
void vpp_state::i8243_port_w(u8 data)
{
	// P4,P5: color mix I8244 side (IC674)
	// P6,P7: color mix EF9340 side (IC678)
	u8 mask = 0xf;
	if constexpr (~P & 1)
	{
		data <<= 4;
		mask <<= 4;
	}

	m_screen->update_now();

	if constexpr ((P & 2) != 0)
		m_mix_i8244 = (m_mix_i8244 & ~mask) | (data & mask);
	else
		m_mix_ef934x = (m_mix_ef934x & ~mask) | (data & mask);
}


// EF9341 extended RAM

offs_t vpp_state::ef934x_extram_address(offs_t offset)
{
	u8 latch = (offset >> 12 & 0x80) | (offset >> 4 & 0x7f);
	u16 address = (latch & 0x1f) | (offset << 9 & 0x200) | (latch << 3 & 0x400);

	if (offset & 8)
		return address | (latch & 0x60);
	else
		return address | (offset << 4 & 0x60) | (latch << 2 & 0x180);
}

u8 vpp_state::ef934x_extram_r(offs_t offset)
{
	return m_ef934x_extram[ef934x_extram_address(offset)];
}

void vpp_state::ef934x_extram_w(offs_t offset, u8 data)
{
	m_ef934x_extram[ef934x_extram_address(offset)] = data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void odyssey2_state::odyssey2_mem(address_map &map)
{
	map(0x0000, 0x03ff).rom();
	map(0x0400, 0x0bff).r(m_cart, FUNC(o2_cart_slot_device::read_rom04));
	map(0x0c00, 0x0fff).r(m_cart, FUNC(o2_cart_slot_device::read_rom0c));
}

void odyssey2_state::odyssey2_io(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(odyssey2_state::io_read), FUNC(odyssey2_state::io_write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( o2 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"×") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"÷") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('/')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y / Yes") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N / No") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("KEY.6")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY.7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("JOY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)     PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)  PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)   PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)   PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)         PORT_PLAYER(2)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("JOY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)     PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)  PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)   PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)   PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)         PORT_PLAYER(1)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, odyssey2_state, reset_button, 0)

	PORT_START("CONF")
	PORT_CONFNAME( 0x01, 0x00, "Color Output" ) PORT_CHANGED_MEMBER(DEVICE_SELF, odyssey2_state, palette_changed, 0)
	PORT_CONFSETTING(    0x00, "Composite" )
	PORT_CONFSETTING(    0x01, "RGB" )
INPUT_PORTS_END

static INPUT_PORTS_START( vpp )
	PORT_INCLUDE( o2 )

	PORT_MODIFY("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0  #") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1  !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2  \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"3  £") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR(0xa3)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4  $") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5  %") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6  &") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7  '") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')

	PORT_MODIFY("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8  (") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9  )") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')

	PORT_MODIFY("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"+  \u2191") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+') PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_MODIFY("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')

	PORT_MODIFY("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')

	PORT_MODIFY("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"-  \u2193") PORT_CODE(KEYCODE_MINUS) PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-') PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"×  \u2196") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(0xd7) PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"÷  \u2190") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR(0xf7) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"=  \u2192") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y / Yes") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N / No") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear  ;") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8) PORT_CHAR(';')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter  _") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(10) PORT_CHAR('_')

	PORT_MODIFY("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ret") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13) // clash with KEY.5 0x80
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(":  *") PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("|  @") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('|') PORT_CHAR('@')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("]  [") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('[')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(u8"¨  ^") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0xa8) PORT_CHAR('^')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",  /") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("<  >") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('<') PORT_CHAR('>')

	PORT_MODIFY("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_END) PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cntl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
INPUT_PORTS_END

static INPUT_PORTS_START( o3 )
	PORT_INCLUDE( vpp )

	PORT_MODIFY("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0  )") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1  !") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2  @") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3  #") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6  ^") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7  &") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')

	PORT_MODIFY("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8  *") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9  (") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/  ?") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')

	PORT_MODIFY("KEY.4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".  >") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_MODIFY("KEY.5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_MODIFY("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("]  }") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",  <") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";  :") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-  _") PORT_CODE(KEYCODE_PGDN) // clash with KEY.5 0x01
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("'  \"") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[  {") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("=  +") PORT_CODE(KEYCODE_PGUP) // clash with KEY.2 0x01 and KEY.5 0x08

	PORT_MODIFY("KEY.7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void odyssey2_state::odyssey2(machine_config &config)
{
	/* basic machine hardware */
	I8048(config, m_maincpu, (7.15909_MHz_XTAL * 3) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &odyssey2_state::odyssey2_mem);
	m_maincpu->set_addrmap(AS_IO, &odyssey2_state::odyssey2_io);
	m_maincpu->p1_out_cb().set(FUNC(odyssey2_state::p1_write));
	m_maincpu->p2_in_cb().set(FUNC(odyssey2_state::p2_read));
	m_maincpu->p2_out_cb().set(FUNC(odyssey2_state::p2_write));
	m_maincpu->bus_in_cb().set(FUNC(odyssey2_state::bus_read));
	m_maincpu->t0_in_cb().set("cartslot", FUNC(o2_cart_slot_device::t0_read));
	m_maincpu->t1_in_cb().set(FUNC(odyssey2_state::t1_read));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(odyssey2_state::screen_update));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(odyssey2_state::odyssey2_palette), 16);

	I8244(config, m_i8244, 7.15909_MHz_XTAL / 2);
	m_i8244->set_screen("screen");
	m_i8244->set_screen_size(360, 243);
	m_i8244->irq_cb().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_i8244->add_route(ALL_OUTPUTS, "mono", 0.40);

	SPEAKER(config, "mono").front_center();

	/* cartridge */
	O2_CART_SLOT(config, m_cart, o2_cart, nullptr);
	SOFTWARE_LIST(config, "cart_list").set_original("videopac").set_filter("O2");
}

void odyssey2_state::videopac(machine_config &config)
{
	odyssey2(config);

	// PAL video chip
	I8245(config.replace(), m_i8244, 17.734476_MHz_XTAL / 5);
	m_i8244->set_screen("screen");
	m_i8244->set_screen_size(360, 243);
	m_i8244->irq_cb().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_i8244->add_route(ALL_OUTPUTS, "mono", 0.40);

	m_maincpu->set_clock(17.734476_MHz_XTAL / 3);

	subdevice<software_list_device>("cart_list")->set_filter("VP");
}

void odyssey2_state::videopacf(machine_config &config)
{
	videopac(config);

	// different master XTAL
	m_maincpu->set_clock(17.812_MHz_XTAL / 3);
	m_i8244->set_clock(17.812_MHz_XTAL / 5);
}


void vpp_state::g7400(machine_config &config)
{
	/* basic machine hardware */
	I8048(config, m_maincpu, 5.911_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vpp_state::odyssey2_mem);
	m_maincpu->set_addrmap(AS_IO, &vpp_state::odyssey2_io);
	m_maincpu->p1_out_cb().set(FUNC(vpp_state::p1_write));
	m_maincpu->p2_in_cb().set(FUNC(vpp_state::p2_read));
	m_maincpu->p2_out_cb().set(FUNC(vpp_state::p2_write));
	m_maincpu->bus_in_cb().set(FUNC(vpp_state::bus_read));
	m_maincpu->t0_in_cb().set("cartslot", FUNC(o2_cart_slot_device::t0_read));
	m_maincpu->t1_in_cb().set(FUNC(vpp_state::t1_read));
	m_maincpu->prog_out_cb().set(m_i8243, FUNC(i8243_device::prog_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(vpp_state::screen_update));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(odyssey2_state::odyssey2_palette), 16);

	I8243(config, m_i8243);
	m_i8243->p4_out_cb().set(FUNC(vpp_state::i8243_port_w<0>));
	m_i8243->p5_out_cb().set(FUNC(vpp_state::i8243_port_w<1>));
	m_i8243->p6_out_cb().set(FUNC(vpp_state::i8243_port_w<2>));
	m_i8243->p7_out_cb().set(FUNC(vpp_state::i8243_port_w<3>));

	EF9340_1(config, m_ef934x, (8.867_MHz_XTAL * 2) / 5, "screen");
	m_ef934x->set_offsets(15, 5);
	m_ef934x->read_exram().set(FUNC(vpp_state::ef934x_extram_r));
	m_ef934x->write_exram().set(FUNC(vpp_state::ef934x_extram_w));

	I8245(config, m_i8244, (8.867_MHz_XTAL * 2) / 5);
	m_i8244->set_screen("screen");
	m_i8244->set_screen_size(360, 243);
	m_i8244->irq_cb().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_i8244->add_route(ALL_OUTPUTS, "mono", 0.40);

	SPEAKER(config, "mono").front_center();

	/* cartridge */
	O2_CART_SLOT(config, m_cart, o2_cart, nullptr);
	SOFTWARE_LIST(config, "cart_list").set_original("videopac").set_filter("VPP");
}

void vpp_state::jo7400(machine_config &config)
{
	g7400(config);

	// different video clock
	m_i8244->set_clock(3.5625_MHz_XTAL);
	m_ef934x->set_clock(3.5625_MHz_XTAL);
}

void vpp_state::odyssey3(machine_config &config)
{
	g7400(config);

	// NTSC video chip
	I8244(config.replace(), m_i8244, 7.15909_MHz_XTAL / 2);
	m_i8244->set_screen("screen");
	m_i8244->set_screen_size(360, 243);
	m_i8244->irq_cb().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_i8244->add_route(ALL_OUTPUTS, "mono", 0.40);

	m_ef934x->set_clock(7.15909_MHz_XTAL / 2);
	m_ef934x->set_offsets(15, 15);

	m_maincpu->set_clock((7.15909_MHz_XTAL * 3) / 4);

	subdevice<software_list_device>("cart_list")->set_filter("O3");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START (videopac)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD("o2bios.rom", 0x0000, 0x0400, CRC(8016a315) SHA1(b2e1955d957a475de2411770452eff4ea19f4cee))
ROM_END

ROM_START (odyssey2)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD("o2bios.rom", 0x0000, 0x0400, CRC(8016a315) SHA1(b2e1955d957a475de2411770452eff4ea19f4cee))
ROM_END

ROM_START (videopacf)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD("c52.rom", 0x0000, 0x0400, CRC(a318e8d6) SHA1(a6120aed50831c9c0d95dbdf707820f601d9452e))
ROM_END


ROM_START (videopacp)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD("g7400.bin", 0x0000, 0x0400, CRC(e20a9f41) SHA1(5130243429b40b01a14e1304d0394b8459a6fbae))
ROM_END

ROM_START (jopac)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD("jopac.bin", 0x0000, 0x0400, CRC(11647ca5) SHA1(54b8d2c1317628de51a85fc1c424423a986775e4))
ROM_END

ROM_START (odyssey3)
	ROM_REGION(0x0400, "maincpu", 0)
	ROM_LOAD("odyssey3.bin", 0x0000, 0x0400, CRC(e2b23324) SHA1(0a38c5f2cea929d2fe0a23e5e1a60de9155815dc))
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT    CMP MACHINE    INPUT  STATE           INIT        COMPANY, FULLNAME, FLAGS
COMP( 1978, videopac,  0,         0, videopac,  o2,    odyssey2_state, empty_init, "Philips", "Videopac G7000 (Europe)", MACHINE_SUPPORTS_SAVE )
COMP( 1979, videopacf, videopac,  0, videopacf, o2,    odyssey2_state, empty_init, "Philips", "Videopac C52 (France)", MACHINE_SUPPORTS_SAVE )
COMP( 1979, odyssey2,  videopac,  0, odyssey2,  o2,    odyssey2_state, empty_init, "Magnavox", "Odyssey 2 (US)", MACHINE_SUPPORTS_SAVE )

COMP( 1983, videopacp, 0,         0, g7400,     vpp,   vpp_state,      empty_init, "Philips", "Videopac+ G7400 (Europe)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, jopac,     videopacp, 0, jo7400,    vpp,   vpp_state,      empty_init, "Philips (Brandt license)", "Jopac JO7400 (France)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, odyssey3,  videopacp, 0, odyssey3,  o3,    vpp_state,      empty_init, "Philips", "Odyssey 3 Command Center (US, prototype)", MACHINE_SUPPORTS_SAVE )
