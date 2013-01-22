/***************************************************************************

  /drivers/odyssey2.c

  Driver file to handle emulation of the Odyssey2.

  Minor update to "the voice" rom names, and add comment about
  the older revision of "the voice" - LN, 10/03/08

  TODO:
  - Reimplement the cartridge slot, and thus also the voice, as a slot device

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/cartslot.h"
#include "sound/sp0256.h"
#include "video/i8244.h"
#include "machine/i8243.h"
#include "video/ef9340_1.h"


class odyssey2_state : public driver_device
{
public:
	odyssey2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_i8243(*this, "i8243")
		, m_i8244(*this, "i8244")
		, m_ef9340_1(*this, "ef9340_1")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<i8243_device> m_i8243;
	required_device<i8244_device> m_i8244;
	optional_device<ef9340_1_device> m_ef9340_1;

	int m_the_voice_lrq_state;
	UINT8 *m_ram;
	UINT8 m_p1;
	UINT8 m_p2;
	size_t m_cart_size;
	UINT8 m_lum;
	DECLARE_READ8_MEMBER(t0_read);
	DECLARE_READ8_MEMBER(io_read);
	DECLARE_WRITE8_MEMBER(io_write);
	DECLARE_READ8_MEMBER(bus_read);
	DECLARE_WRITE8_MEMBER(bus_write);
	DECLARE_READ8_MEMBER(g7400_io_read);
	DECLARE_WRITE8_MEMBER(g7400_io_write);
	DECLARE_READ8_MEMBER(p1_read);
	DECLARE_WRITE8_MEMBER(p1_write);
	DECLARE_READ8_MEMBER(p2_read);
	DECLARE_WRITE8_MEMBER(p2_write);
	DECLARE_READ8_MEMBER(t1_read);
	DECLARE_DRIVER_INIT(odyssey2);
	virtual void machine_reset();
	virtual void palette_init();
	UINT32 screen_update_odyssey2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(the_voice_lrq_callback);
	DECLARE_WRITE8_MEMBER(i8243_port_w);
	DECLARE_WRITE_LINE_MEMBER(irq_callback);

	DECLARE_WRITE16_MEMBER(scanline_postprocess);
	DECLARE_WRITE16_MEMBER(scanline_postprocess_g7400);

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

	UINT8   m_g7400_ic674_decode[8];
	UINT8   m_g7400_ic678_decode[8];

	void switch_banks();
};


static ADDRESS_MAP_START( odyssey2_mem , AS_PROGRAM, 8, odyssey2_state )
	AM_RANGE(0x0000, 0x03FF) AM_ROM
	AM_RANGE(0x0400, 0x0BFF) AM_RAMBANK("bank1")
	AM_RANGE(0x0C00, 0x0FFF) AM_RAMBANK("bank2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( odyssey2_io , AS_IO, 8, odyssey2_state )
	AM_RANGE(0x00,           0xff)           AM_READWRITE(io_read, io_write)
	AM_RANGE(MCS48_PORT_P1,  MCS48_PORT_P1)  AM_READWRITE(p1_read, p1_write)
	AM_RANGE(MCS48_PORT_P2,  MCS48_PORT_P2)  AM_READWRITE(p2_read, p2_write)
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_READWRITE(bus_read, bus_write)
	AM_RANGE(MCS48_PORT_T0,  MCS48_PORT_T0)  AM_READ(t0_read)
	AM_RANGE(MCS48_PORT_T1,  MCS48_PORT_T1)  AM_READ(t1_read)
ADDRESS_MAP_END


static ADDRESS_MAP_START( g7400_io , AS_IO, 8, odyssey2_state )
	AM_RANGE(0x00,            0xff)            AM_READWRITE(g7400_io_read, g7400_io_write)
	AM_RANGE(MCS48_PORT_P1,   MCS48_PORT_P1)   AM_READWRITE(p1_read, p1_write)
	AM_RANGE(MCS48_PORT_P2,   MCS48_PORT_P2)   AM_READWRITE(p2_read, p2_write)
	AM_RANGE(MCS48_PORT_BUS,  MCS48_PORT_BUS)  AM_READWRITE(bus_read, bus_write)
	AM_RANGE(MCS48_PORT_T0,   MCS48_PORT_T0)   AM_READ(t0_read)
	AM_RANGE(MCS48_PORT_T1,   MCS48_PORT_T1)   AM_READ(t1_read)
	AM_RANGE(MCS48_PORT_PROG, MCS48_PORT_PROG) AM_DEVWRITE("i8243", i8243_device, i8243_prog_w);
ADDRESS_MAP_END


static INPUT_PORTS_START( odyssey2 )
	PORT_START("KEY0")      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("KEY1")      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("?? :") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("?? $") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')

	PORT_START("KEY2")      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')

	PORT_START("KEY3")      /* IN3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')

	PORT_START("KEY4")      /* IN4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')

	PORT_START("KEY5")      /* IN5 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('/')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(DEF_STR( Yes )) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(DEF_STR( No )) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\r')

	PORT_START("JOY0")      /* IN6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)     PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)  PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)   PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)   PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1)         PORT_PLAYER(1)
	PORT_BIT( 0xe0, 0xe0,    IPT_UNUSED )

	PORT_START("JOY1")      /* IN7 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)     PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)  PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)   PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)   PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1)         PORT_PLAYER(2)
	PORT_BIT( 0xe0, 0xe0,    IPT_UNUSED )
INPUT_PORTS_END


/* character sprite colors
   dark grey, red, green, orange, blue, violet, light grey, white
   dark back / grid colors
   black, dark blue, dark green, light green, red, violet, orange, light grey
   light back / grid colors
   black, blue, green, light green, red, violet, orange, light grey */

const UINT8 odyssey2_colors[] =
{
	/* Background,Grid Dim */
	0x00,0x00,0x00,						// i r g b
	0x00,0x00,0xFF,   /* Blue */		// i r g B
	0x00,0x80,0x00,   /* DK Green */	// i r G b
	0xff,0x9b,0x60,						// i r G B
	0xCC,0x00,0x00,   /* Red */			// i R g b
	0xa9,0x80,0xff,						// i R g B
	0x82,0xfd,0xdb,						// i R G b
	0xcc,0xcc,0xcc,						// i R G B

	/* Background,Grid Bright */
	0x80,0x80,0x80,						// I r g b
	0x50,0xAE,0xFF,   /* Blue */		// I r g B
	0x00,0xFF,0x00,   /* Dk Green */	// I r G b
	0x82,0xfb,0xdb,   /* Lt Grey */		// I r G B
	0xff,0x80,0x80,   /* Red */			// I R g b
	0xa9,0x80,0xff,   /* Violet */		// I R g B
	0xff,0x9b,0x60,   /* Orange */		// I R G b
	0xFF,0xFF,0xFF,						// I R G B

	/* Character,Sprite colors */
	0x80,0x80,0x80,   /* Dark Grey */	// I r g b 
	0xFF,0x80,0x80,   /* Red */			// I R g b
	0x00,0xC0,0x00,   /* Green */		// I r G b
	0xff,0x9b,0x60,   /* Orange */		// I R G b
	0x50,0xAE,0xFF,   /* Blue */		// I r g B
	0xa9,0x80,0xff,   /* Violet */		// I R g B
	0x82,0xfb,0xdb,   /* Lt Grey */		// I r G B
	0xff,0xff,0xff,   /* White */		// I R G B

	/* EF9340/EF9341 colors */
	0x00, 0x00, 0x00,
	0x00, 0x00, 0xFF,
	0x00, 0xFF, 0x00,
	0x00, 0xFF, 0xFF,
	0xFF, 0x00, 0x00,
	0xFF, 0x00, 0xFF,
	0xFF, 0xFF, 0x00,
	0xFF, 0xFF, 0xFF
};


void odyssey2_state::palette_init()
{
	int i;

	for ( i = 0; i < 32; i++ )
	{
		palette_set_color_rgb( machine(), i, odyssey2_colors[i*3], odyssey2_colors[i*3+1], odyssey2_colors[i*3+2] );
	}
}


WRITE_LINE_MEMBER(odyssey2_state::irq_callback)
{
	m_maincpu->set_input_line(0, state);
}


void odyssey2_state::switch_banks()
{
	switch ( m_cart_size )
	{
		case 12288:
			/* 12KB cart support (for instance, KTAA as released) */
			membank( "bank1" )->set_base( memregion("user1")->base() + (m_p1 & 0x03) * 0xC00 );
			membank( "bank2" )->set_base( memregion("user1")->base() + (m_p1 & 0x03) * 0xC00 + 0x800 );
			break;

		case 16384:
			/* 16KB cart support (for instance, full sized version KTAA) */
			membank( "bank1" )->set_base( memregion("user1")->base() + (m_p1 & 0x03) * 0x1000 + 0x400 );
			membank( "bank2" )->set_base( memregion("user1")->base() + (m_p1 & 0x03) * 0x1000 + 0xC00 );
			break;

		default:
			membank("bank1")->set_base(memregion("user1")->base() + (m_p1 & 0x03) * 0x800);
			membank("bank2")->set_base(memregion("user1")->base() + (m_p1 & 0x03) * 0x800 );
			break;
	}
}


WRITE_LINE_MEMBER(odyssey2_state::the_voice_lrq_callback)
{
	m_the_voice_lrq_state = state;
}


READ8_MEMBER(odyssey2_state::t0_read)
{
	return ( m_the_voice_lrq_state == ASSERT_LINE ) ? 0 : 1;
}


DRIVER_INIT_MEMBER(odyssey2_state,odyssey2)
{
	int i;
	int size = 0;
	UINT8 *gfx = memregion("gfx1")->base();
	device_image_interface *image = dynamic_cast<device_image_interface *>(machine().device("cart"));

	m_ram        = auto_alloc_array(machine(), UINT8, 256);

	for (i = 0; i < 256; i++)
	{
		gfx[i] = i;     /* TODO: Why i and not 0? */
		m_ram[i] = 0;
	}

	if (image->exists())
	{
		if (image->software_entry() == NULL)
		{
			size = image->length();
		}
		else
		{
			size = image->get_software_region_length("rom");
		}
	}
	m_cart_size = size;
}


void odyssey2_state::machine_reset()
{
	m_lum = 0;

	/* jump to "last" bank, will work for all sizes due to being mirrored */
	m_p1 = 0xFF;
	m_p2 = 0xFF;
	switch_banks();

	for ( int i = 0; i < 8; i++ )
	{
		m_g7400_ic674_decode[i] = 0;
		m_g7400_ic678_decode[i] = 0;
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
		if ( offset & 0x80 )
		{
			if ( data & 0x20 )
			{
				logerror("voice write %02X, data = %02X (p1 = %02X)\n", offset, data, m_p1 );
				sp0256_ALD_w( machine().device("sp0256_speech"), space, 0, offset & 0x7F );
			}
			else
			{
				/* TODO: Reset sp0256 in this case */
			}
		}
	}
	else if (!(m_p1 & P1_VDC_ENABLE))
	{
		m_i8244->write(space, offset, data);
	}
}


READ8_MEMBER(odyssey2_state::g7400_io_read)
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


WRITE8_MEMBER(odyssey2_state::g7400_io_write)
{
	if ((m_p1 & (P1_EXT_RAM_ENABLE | P1_VDC_COPY_MODE_ENABLE)) == 0x00)
	{
		m_ram[offset] = data;
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


WRITE16_MEMBER(odyssey2_state::scanline_postprocess_g7400)
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
	int x_real_start = i8244_device::START_ACTIVE_SCAN + i8244_device::BORDER_SIZE + 2;
	int x_real_end = i8244_device::END_ACTIVE_SCAN - i8244_device::BORDER_SIZE + 2;
	for ( int x = i8244_device::START_ACTIVE_SCAN; x < i8244_device::END_ACTIVE_SCAN; x++ )
	{
		UINT16 d = bitmap->pix16( vpos, x );

		if ( ( ! m_g7400_ic678_decode[ d & 0x07 ] ) && x >= x_real_start && x < x_real_end && y < 240 )
		{
			// Use EF934x input
			d = ef934x_bitmap->pix16( y, x - x_real_start ) & 0x07;

			if ( ! m_g7400_ic674_decode[ d & 0x07 ] )
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

	switch_banks();
}


READ8_MEMBER(odyssey2_state::p2_read)
{
	UINT8 h = 0xFF;
	int i, j;
	static const char *const keynames[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5" };

	if (!(m_p1 & P1_KEYBOARD_SCAN_ENABLE))
	{
		if ((m_p2 & P2_KEYBOARD_SELECT_MASK) <= 5)  /* read keyboard */
		{
			h &= ioport(keynames[m_p2 & P2_KEYBOARD_SELECT_MASK])->read();
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

	if ( m_i8243 )
	{
		m_i8243->i8243_p2_w( space, 0, m_p2 & 0x0f );
	}
}


READ8_MEMBER(odyssey2_state::bus_read)
{
	UINT8 data = 0xff;

	if ((m_p2 & P2_KEYBOARD_SELECT_MASK) == 1)
	{
		data &= ioport("JOY0")->read();       /* read joystick 1 */
	}

	if ((m_p2 & P2_KEYBOARD_SELECT_MASK) == 0)
	{
		data &= ioport("JOY1")->read();       /* read joystick 2 */
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

WRITE8_MEMBER(odyssey2_state::i8243_port_w)
{
	switch ( offset & 3 )
	{
		case 0: // "port 4"
logerror("setting ef-port4 to %02x\n", data);
			m_g7400_ic674_decode[4] = BIT(data,0);
			m_g7400_ic674_decode[5] = BIT(data,1);
			m_g7400_ic674_decode[6] = BIT(data,2);
			m_g7400_ic674_decode[7] = BIT(data,3);
			break;

		case 1: // "port 5"
logerror("setting ef-port5 to %02x\n", data);
			m_g7400_ic674_decode[0] = BIT(data,0);
			m_g7400_ic674_decode[1] = BIT(data,1);
			m_g7400_ic674_decode[2] = BIT(data,2);
			m_g7400_ic674_decode[3] = BIT(data,3);
			break;

		case 2: // "port 6"
logerror("setting vdc-port6 to %02x\n", data);
			m_g7400_ic678_decode[4] = BIT(data,0);
			m_g7400_ic678_decode[5] = BIT(data,1);
			m_g7400_ic678_decode[6] = BIT(data,2);
			m_g7400_ic678_decode[7] = BIT(data,3);
			break;

		case 3: // "port 7"
logerror("setting vdc-port7 to %02x\n", data);
			m_g7400_ic678_decode[0] = BIT(data,0);
			m_g7400_ic678_decode[1] = BIT(data,1);
			m_g7400_ic678_decode[2] = BIT(data,2);
			m_g7400_ic678_decode[3] = BIT(data,3);
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


static const sp0256_interface the_voice_sp0256 =
{
	DEVCB_DRIVER_LINE_MEMBER(odyssey2_state,the_voice_lrq_callback),
	DEVCB_NULL
};


static MACHINE_CONFIG_FRAGMENT( odyssey2_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,rom")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("odyssey_cart")
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

	MCFG_GFXDECODE( odyssey2 )
	MCFG_PALETTE_LENGTH(32)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_I8244_ADD( "i8244", XTAL_7_15909MHz/2 * 2, "screen", WRITELINE( odyssey2_state, irq_callback ), WRITE16( odyssey2_state, scanline_postprocess ) )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_SOUND_ADD("sp0256_speech", SP0256, 3120000)
	MCFG_SOUND_CONFIG(the_voice_sp0256)
	/* The Voice uses a speaker with its own volume control so the relative volumes to use are subjective, these sound good */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

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

	MCFG_GFXDECODE( odyssey2 )
	MCFG_PALETTE_LENGTH(32)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_I8245_ADD( "i8244", XTAL_17_73447MHz/5 * 2, "screen", WRITELINE( odyssey2_state, irq_callback ), WRITE16( odyssey2_state, scanline_postprocess ) )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_SOUND_ADD("sp0256_speech", SP0256, 3120000)
	MCFG_SOUND_CONFIG(the_voice_sp0256)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FRAGMENT_ADD(odyssey2_cartslot)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( g7400, odyssey2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8048, XTAL_5_911MHz )
	MCFG_CPU_PROGRAM_MAP(odyssey2_mem)
	MCFG_CPU_IO_MAP(g7400_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS( 3540000 * 2, i8244_device::LINE_CLOCKS, i8244_device::START_ACTIVE_SCAN, i8244_device::END_ACTIVE_SCAN, i8245_device::LINES, i8244_device::START_Y, i8244_device::START_Y + i8244_device::SCREEN_HEIGHT )
	MCFG_SCREEN_UPDATE_DRIVER(odyssey2_state, screen_update_odyssey2)

	MCFG_GFXDECODE( odyssey2 )
	MCFG_PALETTE_LENGTH(32)

	MCFG_I8243_ADD( "i8243", NOOP, WRITE8(odyssey2_state,i8243_port_w))

	MCFG_EF9340_1_ADD( "ef9340_1", 3540000, "screen" )

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_I8245_ADD( "i8244", 3540000 * 2, "screen", WRITELINE( odyssey2_state, irq_callback ), WRITE16( odyssey2_state, scanline_postprocess_g7400 ) )
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

	ROM_REGION(0x4000, "user1", 0)
	ROM_CART_LOAD("cart", 0x0000, 0x4000, ROM_MIRROR)

	ROM_REGION( 0x10000, "sp0256_speech", 0 )
	/* SP0256B-019 Speech chip w/2KiB mask rom */
	ROM_LOAD( "sp0256b-019.bin",   0x1000, 0x0800, CRC(4bb43724) SHA1(49f5326ad45392dc96c89d1d4e089a20bd21e609) )

	/* A note about "The Voice": Two versions of "The Voice" exist:
	   * An earlier version with eight 2KiB speech roms, spr016-??? through spr016-??? on a small daughterboard
	   <note to self: fill in numbers later>
	   * A later version with one 16KiB speech rom, spr128-003, mounted directly on the mainboard
	   The rom contents of these two versions are EXACTLY the same.
	   Both versions have an sp0256b-019 speech chip, which has 2KiB of its own internal speech data
	   Thanks to kevtris for this info. - LN
	*/
	/* External 16KiB speech ROM (spr128-003) from "The Voice" */
	ROM_LOAD( "spr128-003.bin",   0x4000, 0x4000, CRC(509367b5) SHA1(0f31f46bc02e9272885779a6dd7102c78b18895b) )
	/* Additional External 16KiB ROM (spr128-004) from S.I.D. the Spellbinder */
	ROM_LOAD( "spr128-004.bin",   0x8000, 0x4000, CRC(e79dfb75) SHA1(37f33d79ffd1739d7c2f226b010a1eac28d74ca0) )
ROM_END


ROM_START (videopac)
	ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
	ROM_SYSTEM_BIOS( 0, "g7000", "g7000" )
	ROMX_LOAD ("o2bios.rom", 0x0000, 0x0400, CRC(8016a315) SHA1(b2e1955d957a475de2411770452eff4ea19f4cee), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "c52", "c52" )
	ROMX_LOAD ("c52.bin", 0x0000, 0x0400, CRC(a318e8d6) SHA1(a6120aed50831c9c0d95dbdf707820f601d9452e), ROM_BIOS(2))
	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)

	ROM_REGION(0x4000, "user1", 0)
	ROM_CART_LOAD("cart", 0x0000, 0x4000, ROM_MIRROR)

	ROM_REGION( 0x10000, "sp0256_speech", 0 )
	/* SP0256B-019 Speech chip w/2KiB mask rom */
	ROM_LOAD( "sp0256b-019.bin",   0x1000, 0x0800, CRC(4bb43724) SHA1(49f5326ad45392dc96c89d1d4e089a20bd21e609) )
	/* External 16KiB speech ROM (spr128-003) from "The Voice" */
	ROM_LOAD( "spr128-003.bin",   0x4000, 0x4000, CRC(509367b5) SHA1(0f31f46bc02e9272885779a6dd7102c78b18895b) )
	/* Additional External 16KiB speech ROM (spr128-004) from S.I.D. the Spellbinder */
	ROM_LOAD( "spr128-004.bin",   0x8000, 0x4000, CRC(e79dfb75) SHA1(37f33d79ffd1739d7c2f226b010a1eac28d74ca0) )
ROM_END


ROM_START (g7400)
	ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
	ROM_LOAD ("g7400.bin", 0x0000, 0x0400, CRC(e20a9f41) SHA1(5130243429b40b01a14e1304d0394b8459a6fbae))
	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)

	ROM_REGION(0x4000, "user1", 0)
	ROM_CART_LOAD("cart", 0x0000, 0x4000, ROM_MIRROR)
ROM_END


ROM_START (jopac)
	ROM_REGION(0x10000,"maincpu",0)    /* safer for the memory handler/bankswitching??? */
	ROM_LOAD ("jopac.bin", 0x0000, 0x0400, CRC(11647ca5) SHA1(54b8d2c1317628de51a85fc1c424423a986775e4))
	ROM_REGION(0x100, "gfx1", ROMREGION_ERASEFF)

	ROM_REGION(0x4000, "user1", 0)
	ROM_CART_LOAD("cart", 0x0000, 0x4000, ROM_MIRROR)
ROM_END


/*     YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT     INIT      COMPANY     FULLNAME     FLAGS */
COMP( 1978, odyssey2, 0,        0,      odyssey2, odyssey2, odyssey2_state, odyssey2, "Magnavox", "Odyssey 2", 0 )
COMP( 1979, videopac, odyssey2, 0,      videopac, odyssey2, odyssey2_state, odyssey2, "Philips", "Videopac G7000/C52", 0 )
COMP( 1983, g7400, odyssey2, 0,         g7400,    odyssey2, odyssey2_state, odyssey2, "Philips", "Videopac Plus G7400", GAME_NOT_WORKING )
COMP( 1983, jopac, odyssey2, 0,         g7400,    odyssey2, odyssey2_state, odyssey2, "Brandt", "Jopac JO7400", GAME_NOT_WORKING )
