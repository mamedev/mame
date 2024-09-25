// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        VTech PreComputer 1000 / 2000

        04/12/2009 Skeleton driver.

        TODO:
        - dump the chargen
        - change PC2000 to use a SED1278F-0B instead of an HD44780 (different charset)

****************************************************************************

Other known international versions:
- Genio 2000 (Spanish version of Genius Leader 4000 Quadro)
- Genius 4000 (French version of Genius Leader 4000 Quadro)
- Genius 5000 (French version of Genius Leader 5000)
- Genius 6000 (French version of Genius Leader 6000 SL)
- Genius 6500 Duo (alternate French version of Genius Leader 6000 SL)
- Genius Notebook (French version of Genius Leader 3000 S)
- PreComputer Power Pad (English version of Genius Leader 4000 Quadro)
- PreComputer Power Pad Plus (English version of Genius Leader 5000)
- Talking Whiz-Kid Einstein (English version of Genius Leader Power Notebook)
- Talking Whiz-Kid Einstein Mouse (alternate English version of Genius Leader Power Notebook)

****************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/beep.h"
#include "video/hd44780.h"
#include "video/sed1520.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "gl3000s.lh"


namespace {

class pc2000_state : public driver_device
{
public:
	pc2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "hd44780")
		, m_screen(*this, "screen")
		, m_beep(*this, "beeper")
		, m_cart(*this, "cartslot")
		, m_bank0(*this, "bank0")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_rows{ { *this, "IN%X", 0 }, { *this, "IN%X", 8 } }
	{ }

	void pc2000(machine_config &config);
	void pc2000eur(machine_config &config);
	void gl2000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t key_matrix_r(offs_t offset);
	void key_matrix_w(uint8_t data);
	void rombank0_w(uint8_t data);
	void rombank1_w(uint8_t data);
	void rombank2_w(uint8_t data);
	uint8_t beep_r();
	void beep_w(uint8_t data);
	void pc2000_palette(palette_device &palette) const;
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void pc2000_io(address_map &map) ATTR_COLD;
	void pc2000_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	optional_device<hd44780_device> m_lcdc;
	required_device<screen_device> m_screen;
	required_device<beep_device> m_beep;
	required_device<generic_slot_device> m_cart;
	optional_memory_bank m_bank0;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank2;

	void pc2000gen(machine_config &config);

private:
	optional_ioport_array<8> m_rows[2];

	uint8_t m_mux_data;
	uint8_t m_beep_state;
};

class gl3000s_state : public pc2000_state
{
public:
	gl3000s_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc2000_state(mconfig, type, tag)
		, m_lcdc_r(*this, "sed1520_r")
		, m_lcdc_l(*this, "sed1520_l")
		, m_lev_out(*this, "LEV%u", 1U)
		, m_try_out(*this, "TRY%u", 1U)
		, m_tick_out(*this, "TICK%u", 0U)
		, m_time_out(*this, "TIME%u", 0U)
		, m_points_out(*this, "P%u%u", 1U, 0U)
	{ }

	void gl3000s(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gl3000s_io(address_map &map) ATTR_COLD;

private:
	int sed1520_screen_update(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *vram, int start_line, int adc, int start_x);
	SED1520_UPDATE_CB(screen_update_right);
	SED1520_UPDATE_CB(screen_update_left);

	required_device<sed1520_device> m_lcdc_r;
	required_device<sed1520_device> m_lcdc_l;
	output_finder<4> m_lev_out;
	output_finder<3> m_try_out;
	output_finder<8> m_tick_out;
	output_finder<3> m_time_out;
	output_finder<2, 3> m_points_out;
};

class gl4004_state : public pc2000_state
{
public:
	gl4004_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc2000_state(mconfig, type, tag)
	{ }

	void gl4000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	HD44780_PIXEL_UPDATE(gl4000_pixel_update);
};


// TODO: put a breakpoint at 1625 and test the inputs, writes at dce4 are the scancode values
uint8_t pc2000_state::key_matrix_r(offs_t offset)
{
	uint8_t data = 0xff;
	for (int line = 0; line < 8; line++)
		if (BIT(m_mux_data, line))
			data &= m_rows[offset & 1][line]->read();

	return data;
}

void pc2000_state::key_matrix_w(uint8_t data)
{
	m_mux_data = data;
}

void pc2000_state::rombank0_w(uint8_t data)
{
	m_bank0->set_entry(data & 0x1f);
}

void pc2000_state::rombank1_w(uint8_t data)
{
	m_bank1->set_entry(data & 0x1f);
}

void pc2000_state::rombank2_w(uint8_t data)
{
	if (data & 0x80)
		m_bank2->set_entry(data & 0x8f);   //cartridge
	else
		m_bank2->set_entry(data & 0x1f);
}

uint8_t pc2000_state::beep_r()
{
	return m_beep_state;
}

void pc2000_state::beep_w(uint8_t data)
{
	m_beep->set_state(BIT(data, 3));
	m_beep_state = data;
}

void pc2000_state::pc2000_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).bankr("bank0");
	map(0x4000, 0x7fff).bankr("bank1");
	map(0x8000, 0xbfff).bankr("bank2");    //0x8000 - 0xbfff tests a cartridge, header is 0x55 0xaa 0x59 0x45, if it succeeds a jump at 0x8004 occurs
	map(0xc000, 0xdfff).ram();
}

void pc2000_state::pc2000_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(pc2000_state::rombank0_w));
	map(0x01, 0x01).w(FUNC(pc2000_state::rombank1_w));
	map(0x03, 0x03).w(FUNC(pc2000_state::rombank2_w));
	map(0x0a, 0x0b).rw(m_lcdc, FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x10, 0x11).rw(FUNC(pc2000_state::key_matrix_r), FUNC(pc2000_state::key_matrix_w));
	map(0x12, 0x12).rw(FUNC(pc2000_state::beep_r), FUNC(pc2000_state::beep_w));
}


void gl3000s_state::machine_start()
{
	pc2000_state::machine_start();

	m_lev_out.resolve();
	m_try_out.resolve();
	m_tick_out.resolve();
	m_time_out.resolve();
	m_points_out.resolve();
}

uint32_t gl3000s_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0);
	m_lcdc_l->screen_update(screen, bitmap, cliprect);
	m_lcdc_r->screen_update(screen, bitmap, cliprect);
	return 0;
}

int gl3000s_state::sed1520_screen_update(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *vram, int start_line, int adc, int start_x)
{
	for (int y=0; y<2; y++)
	{
		int row_pos = 0;
		for (int x=0; x<61; x++)
		{
			int addr = (y + (start_line >> 3)) * 80 + row_pos;
			for (int yi=0; yi<8; yi++)
			{
				int px = start_x - (adc ? (80 - x) : x);
				int py = 8 + y*8 + yi;

				if (cliprect.contains(px, py))
					bitmap.pix(py, px) = (vram[addr % 0x140] >> yi) & 1;
			}

			row_pos++;
		}
	}
	return 0;
}

SED1520_UPDATE_CB(gl3000s_state::screen_update_right)
{
	if (lcd_on)
		return sed1520_screen_update(bitmap, cliprect, dram, start_line, adc, 119);

	bitmap.fill(0, cliprect);
	return 0;
}

SED1520_UPDATE_CB(gl3000s_state::screen_update_left)
{
	uint8_t sec[5];
	uint8_t points[2][5];
	memset(sec, 0, sizeof(sec));
	memset(points, 0, sizeof(points));

	for (int y=0; y<2; y++)
		for (int x=59; x<85; x++)
		{
			uint8_t data = 0;
			if (lcd_on)
				data = dram[((y + (start_line >> 3)) * 80 + x) & 0x1ff];

			int32_t dpos = (x - 74) / 2;
			if (dpos < 0)
			{
				dpos = 0;
			}

			for (int yi=0; yi<8; yi++)
			{
				int state = (data >> yi) & 1;

				if (y == 0 && (x == 74 || x == 76 || x == 78) && yi == 7)         sec[dpos] |= (state << 0);
				else if (y == 0 && (x == 74 || x == 76 || x == 78) && yi == 0)    sec[dpos] |= (state << 2);
				else if (y == 0 && (x == 75 || x == 77 || x == 79) && yi == 7)    sec[dpos] |= (state << 5);
				else if (y == 0 && (x == 75 || x == 77 || x == 79) && yi == 0)    sec[dpos] |= (state << 4);
				else if (y == 0 && (x == 75 || x == 77 || x == 79) && yi == 1)    sec[dpos] |= (state << 3);
				else if (y == 1 && (x == 74 || x == 76 || x == 78) && yi == 7)    sec[dpos] |= (state << 1);
				else if (y == 1 && (x == 75 || x == 77 || x == 79) && yi == 7)    sec[dpos] |= (state << 6);

				else if ((x == 74 || x == 76 || x == 78) && yi == 3)    points[y][dpos] |= (state << 0);
				else if ((x == 74 || x == 76 || x == 78) && yi == 4)    points[y][dpos] |= (state << 1);
				else if ((x == 74 || x == 76 || x == 78) && yi == 5)    points[y][dpos] |= (state << 2);
				else if ((x == 75 || x == 77 || x == 79) && yi == 3)    points[y][dpos] |= (state << 5);
				else if ((x == 75 || x == 77 || x == 79) && yi == 4)    points[y][dpos] |= (state << 6);
				else if ((x == 75 || x == 77 || x == 79) && yi == 5)    points[y][dpos] |= (state << 4);
				else if ((x == 75 || x == 77 || x == 79) && yi == 6)    points[y][dpos] |= (state << 3);

				else if (y == 1 && x >= 65 && x <= 68 && yi == 7)       m_lev_out[x - 65] = state;
				else if (x >= 59  && x <= 60 && yi == 7)                m_try_out[x - 59 + (y ? 0 : 1)] = state;
				else if (y == 1 && x >= 61 && x <= 64 && yi == 7)       m_tick_out[x - 59] = state;
				else if (y == 0 && x >= 61 && x <= 64 && yi == 7)       m_tick_out[62 - x + (x >= 63 ? 8 : 0)] = state;

				else if (x < 74 && yi < 7)
				{
					int cx = x - 59;
					bitmap.pix(yi, (y ? 0 : 89) + (16 - (cx + cx / 5))) = state;
				}
			}
		}

	for (int i = 0; i < 3; i++)
	{
		m_time_out[i] = sec[i];
		m_points_out[0][i] = points[1][i];
		m_points_out[1][i] = points[0][i];
	}

	if (lcd_on)
		return sed1520_screen_update(bitmap, cliprect, dram, start_line, adc, 58);

	bitmap.fill(0, cliprect);
	return 0;
}


void gl3000s_state::gl3000s_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x01, 0x01).w(FUNC(gl3000s_state::rombank1_w));
	map(0x03, 0x03).w(FUNC(gl3000s_state::rombank2_w));
	map(0x08, 0x09).rw(m_lcdc_r, FUNC(sed1520_device::read), FUNC(sed1520_device::write));
	map(0x0a, 0x0b).rw(m_lcdc_l, FUNC(sed1520_device::read), FUNC(sed1520_device::write));
	map(0x10, 0x11).rw(FUNC(gl3000s_state::key_matrix_r), FUNC(gl3000s_state::key_matrix_w));
}

/* Input ports */
static INPUT_PORTS_START( pc2000 )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" ) //0x83
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps Lock")  PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1")          PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")          PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")          PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")          PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\\")         PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")      PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2")  PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3")  PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")  PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")  PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")  PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")  PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")  PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")  PORT_CODE(KEYCODE_W) PORT_CHAR('W')

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4")  PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5")  PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")  PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")  PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")  PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")  PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")  PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")  PORT_CODE(KEYCODE_R) PORT_CHAR('R')

	PORT_START("IN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6")  PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7")  PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")  PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")  PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")  PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")  PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")  PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")  PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')

	PORT_START("IN5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8")  PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9")  PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")  PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")  PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")  PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",")  PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".")  PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")  PORT_CODE(KEYCODE_I) PORT_CHAR('I')

	PORT_START("IN6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0")  PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("HOME")   PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("=")  PORT_CODE(KEYCODE_EQUALS)   PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";")  PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\'") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/")  PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")  PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("IN7")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT")   PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT")  PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER")  PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL")    PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC")    PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Scramble")           PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Guess Rork")         PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Missing Letter")     PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Letter Hunt")        PORT_CODE(KEYCODE_F4)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Letter Zapper")      PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Letter Switch")      PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sentence Jumble")    PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Grammar Challenge")  PORT_CODE(KEYCODE_F8)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Plurals")            PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Past Tense")         PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Synonyms")           PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Antonyms")           PORT_CODE(KEYCODE_F12)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Basic Math")         PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Advanced Math")      PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Math Riddles")       PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Number Challenge")   PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ecology")            PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("History")            PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Geography")          PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Super Power")        PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IND")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Typing Game")        PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alpha Jumble")       PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Word Flash")         PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sign Search")        PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PC2000 Basic")       PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Spell Checker")      PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Calculator")         PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load Cartridge")     PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INF")
	PORT_DIPNAME( 0x01, 0x01, "INF" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(DEF_STR( Level_Select )) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Num Players")        PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( gl3000s )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Einstellen Antwort")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Hilfe")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)  PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Druck Symbol")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR(U'Ä')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)

	PORT_START("IN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')

	PORT_START("IN5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)  PORT_CHAR(U'ß')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR(U'Ö')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')

	PORT_START("IN6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(U'Ü')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')

	PORT_START("IN7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')

	PORT_START("IN8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Off")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BASIC")              PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Logische Folgen")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bruch / Dezimalrechnen")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Addition")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN9")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Textverarbeitung")   PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Buchstabenschlüssel")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bruch / Prozentrechnen") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Subtraktion")        PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0xe1, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INA")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Taschenrechner") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Gleitpfeile")    PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Algebra")        PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Multiplikation") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0xe1, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ZusatzKassette") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stufe")          PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Spieler")        PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Zeichensuche")   PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Division")       PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INC")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Präteritum")   PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Synonyme")       PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tipp Dich Fit")  PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Super Schlau-Quiz")  PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0xe1, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IND")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Silbentrennung")     PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Knifflige Grammatik")  PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Textaufgaben")       PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Geographie-Quiz")    PORT_CODE(KEYCODE_F9)
	PORT_BIT(0xe1, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Worträtsel")       PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Verdrehte Sätze")  PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Blitz-Sätze")      PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Geschichte-Quiz")    PORT_CODE(KEYCODE_F5)
	PORT_BIT(0xe1, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("INF")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Verrückte Rätsel")  PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Wortarten")           PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Wortschlange")        PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Naturwissenschaften-Quiz")   PORT_CODE(KEYCODE_F1)
	PORT_BIT(0xe1, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

void pc2000_state::machine_start()
{
	uint8_t *bios = memregion("bios")->base();
	memory_region *cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	uint8_t *cart = cart_region ? cart_region->base() : bios;

	m_bank0->configure_entries(0, 0x10, bios, 0x4000);
	m_bank1->configure_entries(0, 0x10, bios, 0x4000);
	m_bank2->configure_entries(0, 0x10, bios, 0x4000);
	m_bank2->configure_entries(0x80, 0x10, cart, 0x4000);
}

void gl4004_state::machine_start()
{
	uint8_t *bios = memregion("bios")->base();
	memory_region *cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
	uint8_t *cart = (cart_region != nullptr) ? cart_region->base() : memregion("bios")->base();

	m_bank0->configure_entries(0, 0x20, bios, 0x4000);
	m_bank1->configure_entries(0, 0x20, bios, 0x4000);
	m_bank2->configure_entries(0, 0x20, bios, 0x4000);
	m_bank2->configure_entries(0x80, 0x10, cart, 0x4000);
}

void pc2000_state::machine_reset()
{
	//set the initial bank
	m_bank0->set_entry(0);
	m_bank1->set_entry(0);
	m_bank2->set_entry(0);
}

void pc2000_state::pc2000_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

static const gfx_layout hd44780_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	256,                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( gfx_pc2000 )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, hd44780_charlayout, 0, 1 )
GFXDECODE_END


DEVICE_IMAGE_LOAD_MEMBER( pc2000_state::cart_load )
{
	uint32_t const size = m_cart->common_get_size("rom");

	// we always allocate a 0x40000 region, even if most carts span only 0x20000,
	// because the bankswitch code accesses up to 16 x 16K banks...
	m_cart->rom_alloc(0x40000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void pc2000_state::pc2000gen(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000)); /* probably not accurate */
	m_maincpu->set_addrmap(AS_PROGRAM, &pc2000_state::pc2000_mem);
	m_maincpu->set_addrmap(AS_IO, &pc2000_state::pc2000_io);
	m_maincpu->set_periodic_int(FUNC(pc2000_state::irq0_line_hold), attotime::from_hz(50));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update("hd44780", FUNC(hd44780_device::screen_update));
	m_screen->set_size(120, 18); //2x20 chars
	m_screen->set_visarea(0, 120-1, 0, 18-1);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(pc2000_state::pc2000_palette), 2);
	GFXDECODE(config, "gfxdecode", "palette", gfx_pc2000);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 3250).add_route(ALL_OUTPUTS, "mono", 1.00);

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "genius_cart").set_device_load(FUNC(pc2000_state::cart_load));

	SOFTWARE_LIST(config, "pc1000_cart").set_compatible("pc1000");
}

void pc2000_state::pc2000(machine_config &config)
{
	pc2000gen(config);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 20);
}

void pc2000_state::pc2000eur(machine_config &config)
{
	pc2000gen(config);

	SED1278(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_default_bios_tag("0b");
	m_lcdc->set_lcd_size(2, 20);
}

void pc2000_state::gl2000(machine_config &config)
{
	pc2000(config);

	SOFTWARE_LIST(config, "cart_list").set_original("gl2000");
	SOFTWARE_LIST(config, "misterx_cart").set_compatible("misterx");
}

HD44780_PIXEL_UPDATE(gl4004_state::gl4000_pixel_update)
{
	if (pos < 40)
	{
		static const uint8_t gl4000_display_layout[] =
		{
			0x00, 0x01, 0x02, 0x03, 0x28, 0x29, 0x2a, 0x2b, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x2c, 0x2d, 0x2e, 0x2f,
			0x30, 0x31, 0x32, 0x33, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
			0x14, 0x15, 0x16, 0x17, 0x3c, 0x3d, 0x3e, 0x3f, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x40, 0x41, 0x42, 0x43,
			0x44, 0x45, 0x46, 0x47, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f
		};

		uint8_t char_pos = gl4000_display_layout[line*40 + pos];
		bitmap.pix((char_pos / 20) * 9 + y, (char_pos % 20) * 6 + x) = state;
	}
}

void gl3000s_state::gl3000s(machine_config &config)
{
	pc2000(config);

	m_maincpu->set_addrmap(AS_IO, &gl3000s_state::gl3000s_io);

	config.device_remove("hd44780");
	SED1520(config, "sed1520_l").set_screen_update_cb(FUNC(gl3000s_state::screen_update_left));  // left panel is 59 pixels (0-58)
	SED1520(config, "sed1520_r").set_screen_update_cb(FUNC(gl3000s_state::screen_update_right)); // right panel is 61 pixels (59-119)

	m_screen->set_size(120, 24);
	m_screen->set_visarea(0, 120-1, 0, 24-1);
	m_screen->set_screen_update(FUNC(gl3000s_state::screen_update));

	config.set_default_layout(layout_gl3000s);
	config.device_remove("gfxdecode");

	SOFTWARE_LIST(config, "gl2000_cart").set_compatible("gl2000");
	SOFTWARE_LIST(config, "misterx_cart").set_compatible("misterx");
}

void gl4004_state::gl4000(machine_config &config)
{
	pc2000(config);

	m_screen->set_size(120, 36); // 4x20 chars
	m_screen->set_visarea(0, 120-1, 0, 36-1);

	m_lcdc->set_lcd_size(4, 20);
	m_lcdc->set_pixel_update_cb(FUNC(gl4004_state::gl4000_pixel_update));

	SOFTWARE_LIST(config, "gl2000_cart").set_compatible("gl2000");
	SOFTWARE_LIST(config, "misterx_cart").set_compatible("misterx");
}

/* ROM definition */
ROM_START( pc2000 )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "lh532hee_9344_d.u4", 0x000000, 0x040000, CRC(0b03bf33) SHA1(cb344b94b14975c685041d3e669f386e8a21909f))
ROM_END

ROM_START( pc2000s )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "lh531h74_9527_d.u4", 0x000000, 0x040000, CRC(b729d10a) SHA1(b0260649c9337f3cdf0fe619e6c8fa50f7b4803a))
ROM_END

ROM_START( gl2000 )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "lh532hez_9416_d.bin", 0x000000, 0x040000, CRC(532f219e) SHA1(4044f0cf098087af4cc9d1b2a80c3c9ec06f154e))
ROM_END

ROM_START( gl2000c )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "27-5491-00", 0x000000, 0x020000, CRC(cbb9fe90) SHA1(a2a7a8afb027fe764a5998e3b35e87f291c24df1))
ROM_END

ROM_START( gl2000p )
	ROM_REGION( 0x40000, "bios", 0 )
	ROM_LOAD( "27-5615-00_9534_d.bin", 0x000000, 0x040000, CRC(481c1000) SHA1(da6f60e5bb25145ec5239310296bedaabeeaee28))
ROM_END

ROM_START( gl3000s )
	ROM_REGION(0x40000, "bios", 0)
	ROM_LOAD( "27-5713-00", 0x000000, 0x040000, CRC(18b113e0) SHA1(27a12893c38068efa35a99fa97a260dbfbd497e3) )
ROM_END

ROM_START( gl4000 )
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD( "27-5480-00",   0x000000, 0x040000, CRC(8de047d3) SHA1(bb1d869954773bb7b8b51caa54531015d6b751ec) )
ROM_END

ROM_START( gl4004 )
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD( "27-5762-00.u2", 0x000000, 0x080000, CRC(fb242f0f) SHA1(aae1beeb94873e29920726ad35475641d9f1e94e) )
ROM_END

ROM_START( gl5000 )
	ROM_REGION(0x80000, "bios", 0)
	ROM_LOAD( "27-5912-00.u1", 0x000000, 0x080000, CRC(9fe4c04a) SHA1(823d1d46e49e21f921260296874bc3ee5f718a5f) )
ROM_END

ROM_START( gl5005x )
	ROM_REGION(0x200000, "bios", 0)
	ROM_LOAD( "27-6426-00.u1", 0x000000, 0x200000, CRC(adde3581) SHA1(80f2bde7c5c339534614f24a9ca6ea362ee2f816) )
ROM_END

ROM_START( glpn )
	ROM_REGION( 0x200000, "bios", 0 )
	ROM_LOAD( "27-5755-01.u1", 0x00000, 0x80000, CRC(dc28346b) SHA1(148fe664bef5b2f68c6702c74462802b76900ca0) )
ROM_END

ROM_START( gmtt )
	ROM_REGION(0x100000, "bios", 0)
	ROM_LOAD( "27-6154-00.u4", 0x000000, 0x100000, CRC(e908262d) SHA1(a7964c9f9d304b6b2cce61822e8c6151b50388be) )
ROM_END

ROM_START( gbs5505x )
	ROM_REGION(0x200000, "bios", 0)
	ROM_LOAD( "27-7006-00.u5", 0x000000, 0x200000, CRC(28af3ca7) SHA1(5df7063c7327263c23d5ac2aac3aa66f7e0821c5) )
ROM_END

ROM_START( lexipcm )
	ROM_REGION( 0x200000, "bios", 0 )
	ROM_LOAD( "epoxy.u3", 0x00000, 0x100000, CRC(0a410790) SHA1(be04d5f74208a2f3b200daed75e04e966f64b545) )
ROM_END

ROM_START( primusex ) // Z84C0006PEC + 2x CP82C55A + V05040INS (?) with 80x48 pixel LCD
	ROM_REGION( 0x80000, "bios", 0 )
	ROM_LOAD( "mtrom.u11", 0x000000, 0x080000, CRC(b35f8e80) SHA1(aa79175f0e590201b62ba1c19492833064e69f71))
ROM_END

} // anonymous expert


/* Driver */

//    YEAR  NAME      PARENT  COMPAT MACHINE    INPUT    CLASS         INIT        COMPANY                    FULLNAME                                  FLAGS
COMP( 198?, primusex, 0,      0,     pc2000,    pc2000,  pc2000_state, empty_init, "Yeno",                    "Primus Expert mit Stimme",               MACHINE_IS_SKELETON )
COMP( 1993, pc2000,   0,      0,     pc2000,    pc2000,  pc2000_state, empty_init, "Video Technology",        "PreComputer 2000",                       MACHINE_NOT_WORKING )
COMP( 1993, pc2000s,  pc2000, 0,     pc2000eur, pc2000,  pc2000_state, empty_init, "Video Technology",        "PreComputer 2000 (Spain)",               MACHINE_NOT_WORKING )
COMP( 1993, gl2000,   0,      0,     gl2000,    pc2000,  pc2000_state, empty_init, "Video Technology",        "Genius Leader 2000",                     MACHINE_NOT_WORKING )
COMP( 1994, gl2000c,  gl2000, 0,     gl2000,    pc2000,  pc2000_state, empty_init, "Video Technology",        "Genius Leader 2000 Compact",             MACHINE_NOT_WORKING )
COMP( 1995, gl2000p,  gl2000, 0,     gl2000,    pc2000,  pc2000_state, empty_init, "Video Technology",        "Genius Leader 2000 Plus",                MACHINE_NOT_WORKING )
COMP( 1996, gl3000s,  0,      0,     gl3000s,   gl3000s, gl3000s_state,empty_init, "Video Technology",        "Genius Leader 3000S (Germany)",          MACHINE_NOT_WORKING )
COMP( 1994, gl4000,   0,      0,     gl4000,    pc2000,  gl4004_state, empty_init, "Video Technology",        "Genius Leader 4000 Quadro (Germany)",    MACHINE_NOT_WORKING )
COMP( 1996, gl4004,   0,      0,     gl4000,    pc2000,  gl4004_state, empty_init, "Video Technology",        "Genius Leader 4004 Quadro L (Germany)",  MACHINE_NOT_WORKING )
COMP( 1997, gl5000,   0,      0,     pc2000,    pc2000,  pc2000_state, empty_init, "Video Technology",        "Genius Leader 5000 (Germany)",           MACHINE_IS_SKELETON )
COMP( 1997, gl5005x,  0,      0,     pc2000,    pc2000,  pc2000_state, empty_init, "Video Technology",        "Genius Leader 5005X (Germany)",          MACHINE_IS_SKELETON )
COMP( 1997, glpn,     0,      0,     gl4000,    pc2000,  gl4004_state, empty_init, "Video Technology",        "Genius Leader Power Notebook (Germany)", MACHINE_IS_SKELETON )
COMP( 1998, gmtt ,    0,      0,     gl4000,    pc2000,  gl4004_state, empty_init, "Video Technology",        "Genius Master Table Top (Germany)",      MACHINE_IS_SKELETON )
COMP( 2001, gbs5505x, 0,      0,     pc2000,    pc2000,  pc2000_state, empty_init, "Video Technology",        "Genius BrainStation 5505X (Germany)",    MACHINE_IS_SKELETON )
COMP( 1999, lexipcm,  0,      0,     pc2000,    pc2000,  pc2000_state, empty_init, "Lexibook",                "LexiPC Mega 2000 (Germany)",             MACHINE_IS_SKELETON )
