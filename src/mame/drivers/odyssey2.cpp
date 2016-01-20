// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  /drivers/odyssey2.c

  Driver file to handle emulation of the Odyssey2.

  Minor update to "the voice" rom names, and add comment about
  the older revision of "the voice" - LN, 10/03/08

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "video/i8244.h"
#include "machine/i8243.h"
#include "video/ef9340_1.h"

#include "bus/odyssey2/slot.h"
#include "softlist.h"

class odyssey2_state : public driver_device
{
public:
	odyssey2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_i8244(*this, "i8244"),
		m_cart(*this, "cartslot"),
		m_keyboard(*this, "KEY"),
		m_joysticks(*this, "JOY") { }

	required_device<cpu_device> m_maincpu;
	required_device<i8244_device> m_i8244;
	required_device<o2_cart_slot_device> m_cart;

	UINT8 m_ram[256];
	UINT8 m_p1;
	UINT8 m_p2;
	UINT8 m_lum;
	DECLARE_READ8_MEMBER(io_read);
	DECLARE_WRITE8_MEMBER(io_write);
	DECLARE_READ8_MEMBER(bus_read);
	DECLARE_WRITE8_MEMBER(bus_write);
	DECLARE_READ8_MEMBER(p1_read);
	DECLARE_WRITE8_MEMBER(p1_write);
	DECLARE_READ8_MEMBER(p2_read);
	DECLARE_WRITE8_MEMBER(p2_write);
	DECLARE_READ8_MEMBER(t1_read);
	DECLARE_DRIVER_INIT(odyssey2);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(odyssey2);
	UINT32 screen_update_odyssey2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(irq_callback);

	DECLARE_WRITE16_MEMBER(scanline_postprocess);

protected:
	/* constants */
	static const UINT8 P1_BANK_LO_BIT          = 0x01;
	static const UINT8 P1_BANK_HI_BIT          = 0x02;
	static const UINT8 P1_KEYBOARD_SCAN_ENABLE = 0x04; /* active low */
	static const UINT8 P1_VDC_ENABLE           = 0x08; /* active low */
	static const UINT8 P1_EXT_RAM_ENABLE       = 0x10; /* active low */
	static const UINT8 P1_VPP_ENABLE           = 0x20; /* active low */
	static const UINT8 P1_VDC_COPY_MODE_ENABLE = 0x40;
	static const UINT8 P2_KEYBOARD_SELECT_MASK = 0x07; /* select row to scan */

	required_ioport_array<6> m_keyboard;
	required_ioport_array<2> m_joysticks;
};

class g7400_state : public odyssey2_state
{
public:
	g7400_state(const machine_config &mconfig, device_type type, const char *tag)
		: odyssey2_state(mconfig, type, tag)
		, m_i8243(*this, "i8243")
		, m_ef9340_1(*this, "ef9340_1")
	{ }

	required_device<i8243_device> m_i8243;
	required_device<ef9340_1_device> m_ef9340_1;

	DECLARE_PALETTE_INIT(g7400);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_WRITE8_MEMBER(p2_write);
	DECLARE_READ8_MEMBER(io_read);
	DECLARE_WRITE8_MEMBER(io_write);
	DECLARE_WRITE8_MEMBER(i8243_port_w);
	DECLARE_WRITE16_MEMBER(scanline_postprocess);

protected:
	UINT8 m_ic674_decode[8];
	UINT8 m_ic678_decode[8];
};


static ADDRESS_MAP_START( odyssey2_mem , AS_PROGRAM, 8, odyssey2_state )
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0400, 0x0bff) AM_DEVREAD("cartslot", o2_cart_slot_device, read_rom04)
	AM_RANGE(0x0c00, 0x0fff) AM_DEVREAD("cartslot", o2_cart_slot_device, read_rom0c)
ADDRESS_MAP_END


static ADDRESS_MAP_START( odyssey2_io , AS_IO, 8, odyssey2_state )
	AM_RANGE(0x00,           0xff)           AM_READWRITE(io_read, io_write)
	AM_RANGE(MCS48_PORT_P1,  MCS48_PORT_P1)  AM_READWRITE(p1_read, p1_write)
	AM_RANGE(MCS48_PORT_P2,  MCS48_PORT_P2)  AM_READWRITE(p2_read, p2_write)
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READWRITE(bus_read, bus_write)
	AM_RANGE(MCS48_PORT_T0,  MCS48_PORT_T0)  AM_DEVREAD("cartslot", o2_cart_slot_device, t0_read)
	AM_RANGE(MCS48_PORT_T1,  MCS48_PORT_T1)  AM_READ(t1_read)
ADDRESS_MAP_END


static ADDRESS_MAP_START( g7400_io , AS_IO, 8, g7400_state )
	AM_RANGE(0x00,            0xff)            AM_READWRITE(io_read, io_write)
	AM_RANGE(MCS48_PORT_P1,   MCS48_PORT_P1)   AM_READWRITE(p1_read, p1_write)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2)   AM_READWRITE(p2_read, p2_write)
	AM_RANGE(MCS48_PORT_BUS,  MCS48_PORT_BUS)  AM_READWRITE(bus_read, bus_write)
	AM_RANGE(MCS48_PORT_T0,   MCS48_PORT_T0)   AM_DEVREAD("cartslot", o2_cart_slot_device, t0_read)
	AM_RANGE(MCS48_PORT_T1,   MCS48_PORT_T1)   AM_READ(t1_read)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_DEVWRITE("i8243", i8243_device, i8243_prog_w);
ADDRESS_MAP_END


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

const UINT8 odyssey2_colors[] =
{
	/* Background,Grid Dim */
	0x00,0x00,0x00,   /* Black */                                         // i r g b
	0x1A,0x37,0xBE,   /* Blue           - Calibrated To Real VideoPac */  // i r g B
	0x00,0x6D,0x07,   /* Green          - Calibrated To Real VideoPac */  // i r G b
	0x2A,0xAA,0xBE,   /* Blue-Green     - Calibrated To Real VideoPac */  // i r G B
	0x79,0x00,0x00,   /* Red            - Calibrated To Real VideoPac */  // i R g b
	0x94,0x30,0x9F,   /* Violet         - Calibrated To Real VideoPac */  // i R g B
	0x77,0x67,0x0B,   /* Khaki          - Calibrated To Real VideoPac */  // i R g B
	0xCE,0xCE,0xCE,   /* Lt Grey */                                       // i R G B

	/* Background,Grid Bright */
	0x67,0x67,0x67,   /* Grey           - Calibrated To Real VideoPac */  // I R g B
	0x5C,0x80,0xF6,   /* Lt Blue        - Calibrated To Real VideoPac */  // I R g B
	0x56,0xC4,0x69,   /* Lt Green       - Calibrated To Real VideoPac */  // I R g B
	0x77,0xE6,0xEB,   /* Lt Blue-Green  - Calibrated To Real VideoPac */  // I R g b
	0xC7,0x51,0x51,   /* Lt Red         - Calibrated To Real VideoPac */  // I R g b
	0xDC,0x84,0xE8,   /* Lt Violet      - Calibrated To Real VideoPac */  // I R g B
	0xC6,0xB8,0x6A,   /* Lt Yellow      - Calibrated To Real VideoPac */  // I R G b
	0xFF,0xFF,0xFF,   /* White */                                         // I R G B
};


PALETTE_INIT_MEMBER(odyssey2_state, odyssey2)
{
	for ( int i = 0; i < 16; i++ )
	{
		palette.set_pen_color( i, odyssey2_colors[i*3], odyssey2_colors[i*3+1], odyssey2_colors[i*3+2] );
	}
}


PALETTE_INIT_MEMBER(g7400_state, g7400)
{
	const UINT8 g7400_colors[] =
	{
	0x00,0x00,0x00, // Black
	0x1A,0x37,0xBE, // Blue
	0x00,0x6D,0x07, // Green
	0x2A,0xAA,0xBE, // Blue-Green
	0x79,0x00,0x00, // Red
	0x94,0x30,0x9F, // Violet
	0x77,0x67,0x0B, // Khaki
	0xCE,0xCE,0xCE, // Lt Grey

	0x67,0x67,0x67, // Grey
	0x5C,0x80,0xF6, // Lt Blue
	0x56,0xC4,0x69, // Lt Green
	0x77,0xE6,0xEB, // Lt Blue-Green
	0xC7,0x51,0x51, // Lt Red
	0xDC,0x84,0xE8, // Lt Violet
	0xC6,0xB8,0x6A, // Lt Yellow
	0xff,0xff,0xff  // White

	};

	for ( int i = 0; i < 16; i++ )
	{
		palette.set_pen_color( i, g7400_colors[i*3], g7400_colors[i*3+1], g7400_colors[i*3+2] );
	}
}


WRITE_LINE_MEMBER(odyssey2_state::irq_callback)
{
	m_maincpu->set_input_line(0, state);
}


DRIVER_INIT_MEMBER(odyssey2_state,odyssey2)
{
	UINT8 *gfx = memregion("gfx1")->base();

	for (int i = 0; i < 256; i++)
	{
		gfx[i] = i;     /* TODO: Why i and not 0? */
		m_ram[i] = 0;
	}
}


void odyssey2_state::machine_start()
{
	save_pointer(NAME(m_ram),256);
	save_item(NAME(m_p1));
	save_item(NAME(m_p2));
	save_item(NAME(m_lum));
}


void odyssey2_state::machine_reset()
{
	m_lum = 0;

	/* jump to "last" bank, will work for all sizes due to being mirrored */
	m_p1 = 0xff;
	m_p2 = 0xff;
	m_cart->write_bank(m_p1);
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

READ8_MEMBER(odyssey2_state::io_read)
{
	if ((m_p1 & (P1_VDC_COPY_MODE_ENABLE | P1_VDC_ENABLE)) == 0)
	{
		return m_i8244->read(space, offset);
	}
	if (!(m_p1 & P1_EXT_RAM_ENABLE))
	{
		return m_ram[offset];
	}

	return 0;
}


WRITE8_MEMBER(odyssey2_state::io_write)
{
	if ((m_p1 & (P1_EXT_RAM_ENABLE | P1_VDC_COPY_MODE_ENABLE)) == 0x00)
	{
		m_ram[offset] = data;
		if (offset & 0x80)
		{
			logerror("voice write %02X, data = %02X (p1 = %02X)\n", offset, data, m_p1);
			m_cart->io_write(space, offset, data);
		}
	}
	else if (!(m_p1 & P1_VDC_ENABLE))
	{
		m_i8244->write(space, offset, data);
	}
}


READ8_MEMBER(g7400_state::io_read)
{
	if ((m_p1 & (P1_VDC_COPY_MODE_ENABLE | P1_VDC_ENABLE)) == 0)
	{
		return m_i8244->read(space, offset);
	}
	else if (!(m_p1 & P1_EXT_RAM_ENABLE))
	{
		return m_ram[offset];
	}
	else if (!(m_p1 & P1_VPP_ENABLE))
	{
		return m_ef9340_1->ef9341_read( offset & 0x02, offset & 0x01 );
	}

	return 0;
}


WRITE8_MEMBER(g7400_state::io_write)
{
	if ((m_p1 & (P1_EXT_RAM_ENABLE | P1_VDC_COPY_MODE_ENABLE)) == 0x00)
	{
		m_ram[offset] = data;
		if (offset & 0x80)
		{
			logerror("voice write %02X, data = %02X (p1 = %02X)\n", offset, data, m_p1);
			m_cart->io_write(space, offset, data);
		}
	}
	else if (!(m_p1 & P1_VDC_ENABLE))
	{
		m_i8244->write(space, offset, data);
	}
	else if (!(m_p1 & P1_VPP_ENABLE))
	{
		m_ef9340_1->ef9341_write( offset & 0x02, offset & 0x01, data );
	}
}


WRITE16_MEMBER(odyssey2_state::scanline_postprocess)
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


WRITE16_MEMBER(g7400_state::scanline_postprocess)
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
		UINT16 d = bitmap->pix16( vpos, x );

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


UINT32 odyssey2_state::screen_update_odyssey2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_i8244->screen_update(screen, bitmap, cliprect);
}


READ8_MEMBER(odyssey2_state::t1_read)
{
	if ( m_i8244->vblank() || m_i8244->hblank() )
	{
		return 1;
	}
	return 0;
}


READ8_MEMBER(odyssey2_state::p1_read)
{
	UINT8 data = m_p1;

	return data;
}


WRITE8_MEMBER(odyssey2_state::p1_write)
{
	m_p1 = data;
	m_lum = ( data & 0x80 ) >> 4;
	m_cart->write_bank(m_p1);
}


READ8_MEMBER(odyssey2_state::p2_read)
{
	UINT8 h = 0xFF;
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


WRITE8_MEMBER(odyssey2_state::p2_write)
{
	m_p2 = data;
}


WRITE8_MEMBER(g7400_state::p2_write)
{
	m_p2 = data;
	m_i8243->i8243_p2_w( space, 0, m_p2 & 0x0f );
}


READ8_MEMBER(odyssey2_state::bus_read)
{
	UINT8 data = 0xff;

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


WRITE8_MEMBER(odyssey2_state::bus_write)
{
	logerror("%.6f bus written %.2x\n", machine().time().as_double(), data);
}


/*
    i8243 in the g7400
*/

WRITE8_MEMBER(g7400_state::i8243_port_w)
{
	switch ( offset & 3 )
	{
		case 0: // "port 4"
logerror("setting ef-port4 to %02x\n", data);
			m_ic674_decode[4] = BIT(data,0);
			m_ic674_decode[5] = BIT(data,1);
			m_ic674_decode[6] = BIT(data,2);
			m_ic674_decode[7] = BIT(data,3);
			break;

		case 1: // "port 5"
logerror("setting ef-port5 to %02x\n", data);
			m_ic674_decode[0] = BIT(data,0);
			m_ic674_decode[1] = BIT(data,1);
			m_ic674_decode[2] = BIT(data,2);
			m_ic674_decode[3] = BIT(data,3);
			break;

		case 2: // "port 6"
logerror("setting vdc-port6 to %02x\n", data);
			m_ic678_decode[4] = BIT(data,0);
			m_ic678_decode[5] = BIT(data,1);
			m_ic678_decode[6] = BIT(data,2);
			m_ic678_decode[7] = BIT(data,3);
			break;

		case 3: // "port 7"
logerror("setting vdc-port7 to %02x\n", data);
			m_ic678_decode[0] = BIT(data,0);
			m_ic678_decode[1] = BIT(data,1);
			m_ic678_decode[2] = BIT(data,2);
			m_ic678_decode[3] = BIT(data,3);
			break;

	}
}


static const gfx_layout odyssey2_graphicslayout =
{
	8,1,
	256,                                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	},
	/* y offsets */
	{ 0 },
	1*8
};


static const gfx_layout odyssey2_spritelayout =
{
	8,1,
	256,                                    /* 256 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes; 1 bit per pixel */
	/* x offsets */
	{
	7,6,5,4,3,2,1,0
	},
	/* y offsets */
	{ 0 },
	1*8
};


static GFXDECODE_START( odyssey2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, odyssey2_graphicslayout, 0, 2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, odyssey2_spritelayout, 0, 2 )
GFXDECODE_END



static MACHINE_CONFIG_FRAGMENT( odyssey2_cartslot )
	MCFG_O2_CARTRIDGE_ADD("cartslot", o2_cart, nullptr)

	MCFG_SOFTWARE_LIST_ADD("cart_list","odyssey2")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( odyssey2, odyssey2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8048, ( ( XTAL_7_15909MHz * 3 ) / 4 ) )
	MCFG_CPU_PROGRAM_MAP(odyssey2_mem)
	MCFG_CPU_IO_MAP(odyssey2_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( XTAL_7_15909MHz/2 * 2, i8244_device::LINE_CLOCKS, i8244_device::START_ACTIVE_SCAN, i8244_device::END_ACTIVE_SCAN, i8244_device::LINES, i8244_device::START_Y, i8244_device::START_Y + i8244_device::SCREEN_HEIGHT )
	MCFG_SCREEN_UPDATE_DRIVER(odyssey2_state, screen_update_odyssey2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", odyssey2 )
	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(odyssey2_state, odyssey2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_I8244_ADD( "i8244", XTAL_7_15909MHz/2 * 2, "screen", WRITELINE( odyssey2_state, irq_callback ), WRITE16( odyssey2_state, scanline_postprocess ) )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_FRAGMENT_ADD(odyssey2_cartslot)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( videopac, odyssey2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8048, ( XTAL_17_73447MHz / 3 ) )
	MCFG_CPU_PROGRAM_MAP(odyssey2_mem)
	MCFG_CPU_IO_MAP(odyssey2_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( XTAL_17_73447MHz/5 * 2, i8244_device::LINE_CLOCKS, i8244_device::START_ACTIVE_SCAN, i8244_device::END_ACTIVE_SCAN, i8245_device::LINES, i8244_device::START_Y, i8244_device::START_Y + i8244_device::SCREEN_HEIGHT )
	MCFG_SCREEN_UPDATE_DRIVER(odyssey2_state, screen_update_odyssey2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", odyssey2 )
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(odyssey2_state, odyssey2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_I8245_ADD( "i8244", XTAL_17_73447MHz/5 * 2, "screen", WRITELINE( odyssey2_state, irq_callback ), WRITE16( odyssey2_state, scanline_postprocess ) )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_FRAGMENT_ADD(odyssey2_cartslot)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( g7400, g7400_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8048, XTAL_5_911MHz )
	MCFG_CPU_PROGRAM_MAP(odyssey2_mem)
	MCFG_CPU_IO_MAP(g7400_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( 3540000 * 2, i8244_device::LINE_CLOCKS, i8244_device::START_ACTIVE_SCAN, i8244_device::END_ACTIVE_SCAN, i8245_device::LINES, i8244_device::START_Y, i8244_device::START_Y + i8244_device::SCREEN_HEIGHT )
	MCFG_SCREEN_UPDATE_DRIVER(odyssey2_state, screen_update_odyssey2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", odyssey2 )
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(g7400_state, g7400)

	MCFG_I8243_ADD( "i8243", NOOP, WRITE8(g7400_state,i8243_port_w))

	MCFG_EF9340_1_ADD( "ef9340_1", 3540000, "screen" )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_I8245_ADD( "i8244", 3540000 * 2, "screen", WRITELINE( odyssey2_state, irq_callback ), WRITE16( g7400_state, scanline_postprocess ) )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_FRAGMENT_ADD(odyssey2_cartslot)
	MCFG_DEVICE_REMOVE("cart_list")
	MCFG_SOFTWARE_LIST_ADD("cart_list","g7400")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("ody2_list","odyssey2")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( odyssey3, g7400_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8048, XTAL_5_911MHz )
	MCFG_CPU_PROGRAM_MAP(odyssey2_mem)
	MCFG_CPU_IO_MAP(g7400_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( 3540000 * 2, i8244_device::LINE_CLOCKS, i8244_device::START_ACTIVE_SCAN, i8244_device::END_ACTIVE_SCAN, i8244_device::LINES, i8244_device::START_Y, i8244_device::START_Y + i8244_device::SCREEN_HEIGHT )
	MCFG_SCREEN_UPDATE_DRIVER(odyssey2_state, screen_update_odyssey2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", odyssey2 )
	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(g7400_state, g7400)

	MCFG_I8243_ADD( "i8243", NOOP, WRITE8(g7400_state,i8243_port_w))

	MCFG_EF9340_1_ADD( "ef9340_1", 3540000, "screen" )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_I8244_ADD( "i8244", 3540000 * 2, "screen", WRITELINE( odyssey2_state, irq_callback ), WRITE16( g7400_state, scanline_postprocess ) )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_FRAGMENT_ADD(odyssey2_cartslot)
	MCFG_DEVICE_REMOVE("cart_list")
	MCFG_SOFTWARE_LIST_ADD("cart_list","g7400")
	MCFG_SOFTWARE_LIST_COMPATIBLE_ADD("ody2_list","odyssey2")
MACHINE_CONFIG_END


ROM_START (odyssey2)
	ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
	ROM_LOAD ("o2bios.rom", 0x0000, 0x0400, CRC(8016a315) SHA1(b2e1955d957a475de2411770452eff4ea19f4cee))
	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)
ROM_END


ROM_START (videopac)
	ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
	ROM_SYSTEM_BIOS( 0, "g7000", "g7000" )
	ROMX_LOAD ("o2bios.rom", 0x0000, 0x0400, CRC(8016a315) SHA1(b2e1955d957a475de2411770452eff4ea19f4cee), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "c52", "c52" )
	ROMX_LOAD ("c52.bin", 0x0000, 0x0400, CRC(a318e8d6) SHA1(a6120aed50831c9c0d95dbdf707820f601d9452e), ROM_BIOS(2))
	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)
ROM_END


ROM_START (g7400)
	ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
	ROM_LOAD ("g7400.bin", 0x0000, 0x0400, CRC(e20a9f41) SHA1(5130243429b40b01a14e1304d0394b8459a6fbae))
	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)
ROM_END


ROM_START (jopac)
	ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
	ROM_LOAD ("jopac.bin", 0x0000, 0x0400, CRC(11647ca5) SHA1(54b8d2c1317628de51a85fc1c424423a986775e4))
	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)
ROM_END


ROM_START (odyssey3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD ("odyssey3.bin", 0x0000, 0x0400, CRC(e2b23324) SHA1(0a38c5f2cea929d2fe0a23e5e1a60de9155815dc))

	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)
ROM_END

/*     YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     INIT      COMPANY     FULLNAME     FLAGS */
COMP( 1978, odyssey2, 0,        0,      odyssey2, odyssey2, odyssey2_state, odyssey2, "Magnavox", "Odyssey 2", 0 )
COMP( 1979, videopac, odyssey2, 0,      videopac, odyssey2, odyssey2_state, odyssey2, "Philips", "Videopac G7000/C52", 0 )
COMP( 1983, g7400, odyssey2, 0,         g7400,    odyssey2, odyssey2_state, odyssey2, "Philips", "Videopac Plus G7400", MACHINE_IMPERFECT_GRAPHICS )
COMP( 1983, jopac, odyssey2, 0,         g7400,    odyssey2, odyssey2_state, odyssey2, "Brandt", "Jopac JO7400", MACHINE_IMPERFECT_GRAPHICS )
COMP( 1983, odyssey3, odyssey2, 0,      odyssey3, odyssey2, odyssey2_state, odyssey2, "Magnavox", "Odyssey 3 Command Center (prototype)", MACHINE_IMPERFECT_GRAPHICS )
