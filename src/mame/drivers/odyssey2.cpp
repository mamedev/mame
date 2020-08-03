// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

Driver file to handle emulation of the Odyssey2.

TODO:
- odyssey3 cpu/video should probably have a different XTAL
- backgamm does not work, it only shows the background graphics
- homecomp does not work, needs new slot device
- g7400 EF9341 R/W is connected to CPU A2, what happens if it is disobeyed?
- a lot more issues, probably, this TODO list was written by someone with
  not much knowledge on odyssey2

***************************************************************************/

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


class odyssey2_state : public driver_device
{
public:
	odyssey2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_i8244(*this, "i8244"),
		m_cart(*this, "cartslot"),
		m_keyboard(*this, "KEY.%u", 0),
		m_joysticks(*this, "JOY.%u", 0)
	{ }

	void odyssey2(machine_config &config);
	void videopac(machine_config &config);
	void videopacf(machine_config &config);

	uint32_t screen_update_odyssey2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	required_device<i8048_device> m_maincpu;
	required_device<i8244_device> m_i8244;
	required_device<o2_cart_slot_device> m_cart;

	uint8_t m_ram[0x80];
	uint8_t m_p1;
	uint8_t m_p2;
	uint8_t m_lum;

	DECLARE_READ_LINE_MEMBER(t1_read);
	void odyssey2_palette(palette_device &palette) const;

	void scanline_postprocess(uint16_t data);

	void odyssey2_io(address_map &map);
	void odyssey2_mem(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	/* constants */
	static const uint8_t P1_BANK_LO_BIT          = 0x01;
	static const uint8_t P1_BANK_HI_BIT          = 0x02;
	static const uint8_t P1_KEYBOARD_SCAN_ENABLE = 0x04; /* active low */
	static const uint8_t P1_VDC_ENABLE           = 0x08; /* active low */
	static const uint8_t P1_EXT_RAM_ENABLE       = 0x10; /* active low */
	static const uint8_t P1_VPP_ENABLE           = 0x20; /* active low */
	static const uint8_t P1_VDC_COPY_MODE_ENABLE = 0x40;
	static const uint8_t P2_KEYBOARD_SELECT_MASK = 0x07; /* select row to scan */

	required_ioport_array<6> m_keyboard;
	required_ioport_array<2> m_joysticks;

	uint8_t io_read(offs_t offset);
	void io_write(offs_t offset, uint8_t data);
	uint8_t bus_read();
	void bus_write(uint8_t data);
	uint8_t p1_read();
	void p1_write(uint8_t data);
	uint8_t p2_read();
	void p2_write(uint8_t data);
};

class g7400_state : public odyssey2_state
{
public:
	g7400_state(const machine_config &mconfig, device_type type, const char *tag)
		: odyssey2_state(mconfig, type, tag)
		, m_i8243(*this, "i8243")
		, m_ef9340_1(*this, "ef9340_1")
	{ }

	void g7400(machine_config &config);
	void odyssey3(machine_config &config);

private:
	required_device<i8243_device> m_i8243;
	required_device<ef9340_1_device> m_ef9340_1;

	void g7400_palette(palette_device &palette) const;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void p2_write(uint8_t data);
	uint8_t io_read(offs_t offset);
	void io_write(offs_t offset, uint8_t data);
	void i8243_p4_w(uint8_t data);
	void i8243_p5_w(uint8_t data);
	void i8243_p6_w(uint8_t data);
	void i8243_p7_w(uint8_t data);
	void scanline_postprocess(uint16_t data);

	void g7400_io(address_map &map);

	uint8_t m_ic674_decode[8];
	uint8_t m_ic678_decode[8];
};


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


void g7400_state::g7400_io(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(g7400_state::io_read), FUNC(g7400_state::io_write));
}


static INPUT_PORTS_START( odyssey2 )
	PORT_START("KEY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("KEY.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("?? :") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("?? $") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("KEY.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("KEY.3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')

	PORT_START("KEY.4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')

	PORT_START("KEY.5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('/')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(DEF_STR( Yes )) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(DEF_STR( No )) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')

	PORT_START("JOY.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)     PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)   PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)   PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1)         PORT_PLAYER(1)
	PORT_BIT( 0xe0, 0xe0,    IPT_UNUSED )

	PORT_START("JOY.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)     PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)   PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)   PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1)         PORT_PLAYER(2)
	PORT_BIT( 0xe0, 0xe0,    IPT_UNUSED )
INPUT_PORTS_END


/* character sprite colors
   dark grey, red, green, yellow, blue, violet, light grey, white
   dark back / grid colors
   black, dark blue, dark green, light green, red, violet, yellow, light grey
   light back / grid colors
   black, blue, green, light green, red, violet, yellow, light grey */

constexpr rgb_t odyssey2_colors[] =
{
	// Background,Grid Dim
	{ 0x00, 0x00, 0x00 },   /* Black */                                         // i r g b
	{ 0x1a, 0x37, 0xbe },   /* Blue           - Calibrated To Real VideoPac */  // i r g B
	{ 0x00, 0x6d, 0x07 },   /* Green          - Calibrated To Real VideoPac */  // i r G b
	{ 0x2a, 0xaa, 0xbe },   /* Blue-Green     - Calibrated To Real VideoPac */  // i r G B
	{ 0x79, 0x00, 0x00 },   /* Red            - Calibrated To Real VideoPac */  // i R g b
	{ 0x94, 0x30, 0x9f },   /* Violet         - Calibrated To Real VideoPac */  // i R g B
	{ 0x77, 0x67, 0x0b },   /* Khaki          - Calibrated To Real VideoPac */  // i R g B
	{ 0xce, 0xce, 0xce },   /* Lt Grey */                                       // i R G B

	// Background,Grid Bright
	{ 0x67, 0x67, 0x67 },   /* Grey           - Calibrated To Real VideoPac */  // I R g B
	{ 0x5c, 0x80, 0xf6 },   /* Lt Blue        - Calibrated To Real VideoPac */  // I R g B
	{ 0x56, 0xc4, 0x69 },   /* Lt Green       - Calibrated To Real VideoPac */  // I R g B
	{ 0x77, 0xe6, 0xeb },   /* Lt Blue-Green  - Calibrated To Real VideoPac */  // I R g b
	{ 0xc7, 0x51, 0x51 },   /* Lt Red         - Calibrated To Real VideoPac */  // I R g b
	{ 0xdc, 0x84, 0xe8 },   /* Lt Violet      - Calibrated To Real VideoPac */  // I R g B
	{ 0xc6, 0xb8, 0x6a },   /* Lt Yellow      - Calibrated To Real VideoPac */  // I R G b
	{ 0xff, 0xff, 0xff }    /* White */                                         // I R G B
};


void odyssey2_state::odyssey2_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, odyssey2_colors);
}


void g7400_state::g7400_palette(palette_device &palette) const
{
	constexpr rgb_t g7400_colors[]{
		{ 0x00, 0x00, 0x00 }, // Black
		{ 0x1a, 0x37, 0xbe }, // Blue
		{ 0x00, 0x6d, 0x07 }, // Green
		{ 0x2a, 0xaa, 0xbe }, // Blue-Green
		{ 0x79, 0x00, 0x00 }, // Red
		{ 0x94, 0x30, 0x9f }, // Violet
		{ 0x77, 0x67, 0x0b }, // Khaki
		{ 0xce, 0xce, 0xce }, // Lt Grey

		{ 0x67, 0x67, 0x67 }, // Grey
		{ 0x5c, 0x80, 0xf6 }, // Lt Blue
		{ 0x56, 0xc4, 0x69 }, // Lt Green
		{ 0x77, 0xe6, 0xeb }, // Lt Blue-Green
		{ 0xc7, 0x51, 0x51 }, // Lt Red
		{ 0xdc, 0x84, 0xe8 }, // Lt Violet
		{ 0xc6, 0xb8, 0x6a }, // Lt Yellow
		{ 0xff, 0xff, 0xff }  // White
	};

	palette.set_pen_colors(0, g7400_colors);
}


void odyssey2_state::machine_start()
{
	memset(m_ram, 0, 0x80);

	save_item(NAME(m_ram));
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_lum));
}


void odyssey2_state::machine_reset()
{
}


void g7400_state::machine_start()
{
	odyssey2_state::machine_start();

	save_pointer(NAME(m_ic674_decode),8);
	save_pointer(NAME(m_ic678_decode),8);
}


void g7400_state::machine_reset()
{
	odyssey2_state::machine_reset();

	for ( int i = 0; i < 8; i++ )
	{
		m_ic674_decode[i] = 0;
		m_ic678_decode[i] = 0;
	}
}

/****** External RAM ******************************/

uint8_t odyssey2_state::io_read(offs_t offset)
{
	u8 data = m_cart->io_read(offset);
	if (!(m_p1 & P1_EXT_RAM_ENABLE) && ~offset & 0x80)
		data &= m_ram[offset];

	if ((m_p1 & (P1_VDC_COPY_MODE_ENABLE | P1_VDC_ENABLE)) == 0)
		data &= m_i8244->read(offset);

	return data;
}


void odyssey2_state::io_write(offs_t offset, uint8_t data)
{
	if (!(m_p1 & P1_VDC_COPY_MODE_ENABLE))
	{
		m_cart->io_write(offset, data);
		if (!(m_p1 & P1_EXT_RAM_ENABLE) && ~offset & 0x80)
			m_ram[offset] = data;
	}

	if (!(m_p1 & P1_VDC_ENABLE))
		m_i8244->write(offset, data);
}


uint8_t g7400_state::io_read(offs_t offset)
{
	u8 data = odyssey2_state::io_read(offset);

	if (!(m_p1 & P1_VPP_ENABLE) && offset & 4)
		data &= m_ef9340_1->ef9341_read( offset & 0x02, offset & 0x01 );

	return data;
}


void g7400_state::io_write(offs_t offset, uint8_t data)
{
	odyssey2_state::io_write(offset, data);

	if (!(m_p1 & P1_VPP_ENABLE) && ~offset & 4)
		m_ef9340_1->ef9341_write( offset & 0x02, offset & 0x01, data );
}


void odyssey2_state::scanline_postprocess(uint16_t data)
{
	int vpos = data;
	bitmap_ind16 *bitmap = m_i8244->get_bitmap();

	if ( vpos < i8244_device::START_Y || vpos >= i8244_device::START_Y + i8244_device::SCREEN_HEIGHT )
	{
		return;
	}

	// apply external LUM setting
	for ( int x = i8244_device::START_ACTIVE_SCAN; x < i8244_device::END_ACTIVE_SCAN; x++ )
	{
		bitmap->pix16( vpos, x ) |= ( m_lum ^ 0x08 );
	}
}


void g7400_state::scanline_postprocess(uint16_t data)
{
	int vpos = data;
	int y = vpos - i8244_device::START_Y - 5;
	bitmap_ind16 *bitmap = m_i8244->get_bitmap();
	bitmap_ind16 *ef934x_bitmap = m_ef9340_1->get_bitmap();

	if ( vpos < i8244_device::START_Y || vpos >= i8244_device::START_Y + i8244_device::SCREEN_HEIGHT )
	{
		return;
	}

	// apply external LUM setting
	int x_real_start = i8244_device::START_ACTIVE_SCAN + i8244_device::BORDER_SIZE + 5;
	int x_real_end = i8244_device::END_ACTIVE_SCAN - i8244_device::BORDER_SIZE + 5;
	for ( int x = i8244_device::START_ACTIVE_SCAN; x < i8244_device::END_ACTIVE_SCAN; x++ )
	{
		uint16_t d = bitmap->pix16( vpos, x );

		if ( ( ! m_ic678_decode[ d & 0x07 ] ) && x >= x_real_start && x < x_real_end && y >= 0 && y < 240 )
		{
			// Use EF934x input
			d = ef934x_bitmap->pix16( y, x - x_real_start ) & 0x07;

			if ( ! m_ic674_decode[ d & 0x07 ] )
			{
				d |= 0x08;
			}
		}
		else
		{
			// Use i8245 input
			d |= ( m_lum ^ 0x08 );
		}
		bitmap->pix16( vpos, x ) = d;
	}
}


uint32_t odyssey2_state::screen_update_odyssey2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_i8244->screen_update(screen, bitmap, cliprect);
}


READ_LINE_MEMBER(odyssey2_state::t1_read)
{
	if ( m_i8244->vblank() || m_i8244->hblank() )
	{
		return 1;
	}
	return 0;
}


uint8_t odyssey2_state::p1_read()
{
	uint8_t data = m_p1;

	return data;
}


void odyssey2_state::p1_write(uint8_t data)
{
	m_p1 = data;
	m_lum = ( data & 0x80 ) >> 4;
	m_cart->write_p1(m_p1 & 0x13);
}


uint8_t odyssey2_state::p2_read()
{
	uint8_t h = 0xFF;
	int i, j;

	if (!(m_p1 & P1_KEYBOARD_SCAN_ENABLE))
	{
		if ((m_p2 & P2_KEYBOARD_SELECT_MASK) <= 5)  /* read keyboard */
		{
			h &= m_keyboard[m_p2 & P2_KEYBOARD_SELECT_MASK]->read();
		}

		for (i= 0x80, j = 0; i > 0; i >>= 1, j++)
		{
			if (!(h & i))
			{
				m_p2 &= ~0x10;                   /* set key was pressed indicator */
				m_p2 = (m_p2 & ~0xE0) | (j << 5);  /* column that was pressed */

				break;
			}
		}

		if (h == 0xFF)  /* active low inputs, so no keypresses */
		{
			m_p2 = m_p2 | 0xF0;
		}
	}
	else
	{
		m_p2 = m_p2 | 0xF0;
	}

	return m_p2;
}


void odyssey2_state::p2_write(uint8_t data)
{
	m_p2 = data;
	m_cart->write_p2(m_p2 & 0x0f);
}


void g7400_state::p2_write(uint8_t data)
{
	odyssey2_state::p2_write(data);
	m_i8243->p2_w(m_p2 & 0x0f);
}


uint8_t odyssey2_state::bus_read()
{
	uint8_t data = 0xff;

	if ((m_p2 & P2_KEYBOARD_SELECT_MASK) == 1)
	{
		data &= m_joysticks[0]->read();
	}

	if ((m_p2 & P2_KEYBOARD_SELECT_MASK) == 0)
	{
		data &= m_joysticks[1]->read();
	}

	return data;
}


void odyssey2_state::bus_write(uint8_t data)
{
	logerror("%.6f bus written %.2x\n", machine().time().as_double(), data);
}


/*
    i8243 in the g7400
*/

void g7400_state::i8243_p4_w(uint8_t data)
{
	// "port 4"
	logerror("setting ef-port4 to %02x\n", data);
	m_ic674_decode[4] = BIT(data,0);
	m_ic674_decode[5] = BIT(data,1);
	m_ic674_decode[6] = BIT(data,2);
	m_ic674_decode[7] = BIT(data,3);
}


void g7400_state::i8243_p5_w(uint8_t data)
{
	// "port 5"
	logerror("setting ef-port5 to %02x\n", data);
	m_ic674_decode[0] = BIT(data,0);
	m_ic674_decode[1] = BIT(data,1);
	m_ic674_decode[2] = BIT(data,2);
	m_ic674_decode[3] = BIT(data,3);
}


void g7400_state::i8243_p6_w(uint8_t data)
{
	// "port 6"
	logerror("setting vdc-port6 to %02x\n", data);
	m_ic678_decode[4] = BIT(data,0);
	m_ic678_decode[5] = BIT(data,1);
	m_ic678_decode[6] = BIT(data,2);
	m_ic678_decode[7] = BIT(data,3);
}


void g7400_state::i8243_p7_w(uint8_t data)
{
	// "port 7"
	logerror("setting vdc-port7 to %02x\n", data);
	m_ic678_decode[0] = BIT(data,0);
	m_ic678_decode[1] = BIT(data,1);
	m_ic678_decode[2] = BIT(data,2);
	m_ic678_decode[3] = BIT(data,3);
}



void odyssey2_state::odyssey2(machine_config &config)
{
	/* basic machine hardware */
	I8048(config, m_maincpu, (XTAL(7'159'090) * 3) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &odyssey2_state::odyssey2_mem);
	m_maincpu->set_addrmap(AS_IO, &odyssey2_state::odyssey2_io);
	m_maincpu->p1_in_cb().set(FUNC(odyssey2_state::p1_read));
	m_maincpu->p1_out_cb().set(FUNC(odyssey2_state::p1_write));
	m_maincpu->p2_in_cb().set(FUNC(odyssey2_state::p2_read));
	m_maincpu->p2_out_cb().set(FUNC(odyssey2_state::p2_write));
	m_maincpu->bus_in_cb().set(FUNC(odyssey2_state::bus_read));
	m_maincpu->bus_out_cb().set(FUNC(odyssey2_state::bus_write));
	m_maincpu->t0_in_cb().set("cartslot", FUNC(o2_cart_slot_device::t0_read));
	m_maincpu->t1_in_cb().set(FUNC(odyssey2_state::t1_read));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(odyssey2_state::screen_update_odyssey2));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(odyssey2_state::odyssey2_palette), 16);

	SPEAKER(config, "mono").front_center();
	I8244(config, m_i8244, XTAL(7'159'090) / 2);
	m_i8244->set_screen("screen");
	m_i8244->irq_cb().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_i8244->postprocess_cb().set(FUNC(odyssey2_state::scanline_postprocess));
	m_i8244->add_route(ALL_OUTPUTS, "mono", 0.40);

	/* cartridge */
	O2_CART_SLOT(config, m_cart, o2_cart, nullptr);
	SOFTWARE_LIST(config, "cart_list").set_original("odyssey2");
}

void odyssey2_state::videopac(machine_config &config)
{
	odyssey2(config);

	// different master XTAL
	m_maincpu->set_clock(XTAL(17'734'470) / 3);

	// PAL video chip
	I8245(config.replace(), m_i8244, XTAL(17'734'470) / 5);
	m_i8244->set_screen("screen");
	m_i8244->irq_cb().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_i8244->postprocess_cb().set(FUNC(odyssey2_state::scanline_postprocess));
	m_i8244->add_route(ALL_OUTPUTS, "mono", 0.40);
}

void odyssey2_state::videopacf(machine_config &config)
{
	videopac(config);

	// different master XTAL
	m_maincpu->set_clock(XTAL(17'812'000) / 3);
	m_i8244->set_clock(XTAL(17'812'000) / 5);
}


void g7400_state::g7400(machine_config &config)
{
	/* basic machine hardware */
	I8048(config, m_maincpu, XTAL(5'911'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &g7400_state::odyssey2_mem);
	m_maincpu->set_addrmap(AS_IO, &g7400_state::g7400_io);
	m_maincpu->p1_in_cb().set(FUNC(g7400_state::p1_read));
	m_maincpu->p1_out_cb().set(FUNC(g7400_state::p1_write));
	m_maincpu->p2_in_cb().set(FUNC(g7400_state::p2_read));
	m_maincpu->p2_out_cb().set(FUNC(g7400_state::p2_write));
	m_maincpu->bus_in_cb().set(FUNC(g7400_state::bus_read));
	m_maincpu->bus_out_cb().set(FUNC(g7400_state::bus_write));
	m_maincpu->t0_in_cb().set("cartslot", FUNC(o2_cart_slot_device::t0_read));
	m_maincpu->t1_in_cb().set(FUNC(g7400_state::t1_read));
	m_maincpu->prog_out_cb().set(m_i8243, FUNC(i8243_device::prog_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(odyssey2_state::screen_update_odyssey2));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(g7400_state::g7400_palette), 16);

	I8243(config, m_i8243);
	m_i8243->p4_out_cb().set(FUNC(g7400_state::i8243_p4_w));
	m_i8243->p5_out_cb().set(FUNC(g7400_state::i8243_p5_w));
	m_i8243->p6_out_cb().set(FUNC(g7400_state::i8243_p6_w));
	m_i8243->p7_out_cb().set(FUNC(g7400_state::i8243_p7_w));

	EF9340_1(config, m_ef9340_1, XTAL(8'867'000)/5 * 2, "screen");

	SPEAKER(config, "mono").front_center();
	I8245(config, m_i8244, XTAL(8'867'000)/5 * 2);
	m_i8244->set_screen("screen");
	m_i8244->irq_cb().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_i8244->postprocess_cb().set(FUNC(g7400_state::scanline_postprocess));
	m_i8244->add_route(ALL_OUTPUTS, "mono", 0.40);

	/* cartridge */
	O2_CART_SLOT(config, m_cart, o2_cart, nullptr);
	SOFTWARE_LIST(config, "cart_list").set_original("g7400");
	SOFTWARE_LIST(config, "ody2_list").set_compatible("odyssey2");
}

void g7400_state::odyssey3(machine_config &config)
{
	g7400(config);

	// NTSC video chip
	I8244(config.replace(), m_i8244, XTAL(8'867'000)/5 * 2);
	m_i8244->set_screen("screen");
	m_i8244->irq_cb().set_inputline(m_maincpu, MCS48_INPUT_IRQ);
	m_i8244->postprocess_cb().set(FUNC(g7400_state::scanline_postprocess));
	m_i8244->add_route(ALL_OUTPUTS, "mono", 0.40);
}


ROM_START (odyssey2)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD ("o2bios.rom", 0x0000, 0x0400, CRC(8016a315) SHA1(b2e1955d957a475de2411770452eff4ea19f4cee))
ROM_END

ROM_START (videopac)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD ("o2bios.rom", 0x0000, 0x0400, CRC(8016a315) SHA1(b2e1955d957a475de2411770452eff4ea19f4cee))
ROM_END

ROM_START (videopacf)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD ("c52.rom", 0x0000, 0x0400, CRC(a318e8d6) SHA1(a6120aed50831c9c0d95dbdf707820f601d9452e))
ROM_END


ROM_START (g7400)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD ("g7400.bin", 0x0000, 0x0400, CRC(e20a9f41) SHA1(5130243429b40b01a14e1304d0394b8459a6fbae))
ROM_END

ROM_START (jopac)
	ROM_REGION(0x0400,"maincpu",0)
	ROM_LOAD ("jopac.bin", 0x0000, 0x0400, CRC(11647ca5) SHA1(54b8d2c1317628de51a85fc1c424423a986775e4))
ROM_END

ROM_START (odyssey3)
	ROM_REGION(0x0400, "maincpu", 0)
	ROM_LOAD ("odyssey3.bin", 0x0000, 0x0400, CRC(e2b23324) SHA1(0a38c5f2cea929d2fe0a23e5e1a60de9155815dc))
ROM_END


/*    YEAR  NAME       PARENT    COMPAT  MACHINE    INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS */
COMP( 1979, odyssey2,  0,        0,      odyssey2,  odyssey2, odyssey2_state, empty_init, "Magnavox", "Odyssey 2 (US)", 0 )
COMP( 1979, videopac,  odyssey2, 0,      videopac,  odyssey2, odyssey2_state, empty_init, "Philips", "Videopac G7000 (Europe)", 0 )
COMP( 1979, videopacf, odyssey2, 0,      videopacf, odyssey2, odyssey2_state, empty_init, "Philips", "Videopac C52 (France)", 0 )

COMP( 1983, g7400,     0,        0,      g7400,     odyssey2, g7400_state,    empty_init, "Philips", "Videopac Plus G7400 (Europe)", MACHINE_IMPERFECT_GRAPHICS )
COMP( 1983, jopac,     g7400,    0,      g7400,     odyssey2, g7400_state,    empty_init, "Philips (Brandt license)", "Jopac JO7400 (France)", MACHINE_IMPERFECT_GRAPHICS )
COMP( 1983, odyssey3,  g7400,    0,      odyssey3,  odyssey2, g7400_state,    empty_init, "Magnavox", "Odyssey 3 Command Center (US, prototype)", MACHINE_IMPERFECT_GRAPHICS )
