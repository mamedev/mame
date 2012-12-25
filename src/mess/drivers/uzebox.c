/***************************************************************************

    Belogic Uzebox

    driver by Sandro Ronco

    TODO:
    - Sound
    - SDCard
    - Mouse

****************************************************************************/

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "sound/dac.h"
#include "imagedev/cartslot.h"

// overclocked to 8 * NTSC burst frequency
#define MASTER_CLOCK 28618180

#define INTERLACED		0

class uzebox_state : public driver_device
{
public:
	uzebox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
        m_maincpu(*this, "maincpu")
	{
	}

	required_device<avr8_device> m_maincpu;

	DECLARE_READ8_MEMBER(port_a_r);
	DECLARE_WRITE8_MEMBER(port_a_w);
	DECLARE_READ8_MEMBER(port_b_r);
	DECLARE_WRITE8_MEMBER(port_b_w);
	DECLARE_READ8_MEMBER(port_c_r);
	DECLARE_WRITE8_MEMBER(port_c_w);
	DECLARE_READ8_MEMBER(port_d_r);
	DECLARE_WRITE8_MEMBER(port_d_w);

	virtual void machine_start();
	virtual void machine_reset();
	void line_update();
	UINT32 screen_update_uzebox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	int 			m_vpos;
	UINT64			m_line_start_cycles;
	UINT32			m_line_pos_cycles;
	UINT8			m_port_a;
	UINT8			m_port_b;
	UINT8			m_port_c;
	UINT8			m_port_d;
	UINT16			m_joy_data[2];
	bitmap_rgb32	m_bitmap;
};

void uzebox_state::machine_start()
{
	machine().primary_screen->register_screen_bitmap(m_bitmap);
}

void uzebox_state::machine_reset()
{
	m_vpos = 0;
	m_line_start_cycles = 0;
	m_line_pos_cycles = 0;
	m_port_a = 0;
	m_port_b = 0;
	m_port_c = 0;
	m_port_d = 0;
	m_joy_data[0] = m_joy_data[1] = 0;
}


WRITE8_MEMBER(uzebox_state::port_a_w)
{
	//  xxxx ----   NC
	//  ---- x---   SNES controller clk
	//  ---- -x--   SNES controller latch
	//  ---- --x-   SNES controller P2 data
	//  ---- ---x   SNES controller P1 data

	UINT8 changed = m_port_a ^ data;

	if (changed & data & 0x04)
	{
		m_joy_data[0] = ioport("P1")->read();
		m_joy_data[1] = ioport("P2")->read();
	}
	else if (changed & 0x08)
	{
		if (changed & data & 0x08)
		{
			m_joy_data[0] >>= 1;
			m_joy_data[1] >>= 1;
		}

		m_port_a = (m_joy_data[0] & 0x01) | ((m_joy_data[1] & 0x01) << 1);
	}

	m_port_a = (data & 0x0c) | (m_port_a & 0x03);
}

READ8_MEMBER(uzebox_state::port_a_r)
{
	return  m_port_a | 0xf0;
}

WRITE8_MEMBER(uzebox_state::port_b_w)
{
	//  xxx- ----   SDCard
	//  ---x ----   AD725 CE
	//  ---- x---   AD725 4FSC
	//  ---- -xx-   NC
	//  ---- ---x   AD725 HSYNC

	if (m_port_b & 0x10)
		if ((m_port_b ^ data) & m_port_b & 0x01)
		{
			line_update();

			UINT32 cycles = (UINT32)(m_maincpu->get_elapsed_cycles() - m_line_start_cycles);
			if (cycles < 1000 && m_vpos >= 448)
				m_vpos = INTERLACED ? ((m_vpos ^ 0x01) & 0x01) : 0;
			else if (cycles > 1000)
				m_vpos += 2;

			m_line_start_cycles = m_maincpu->get_elapsed_cycles();
			m_line_pos_cycles = 0;
		}

	m_port_b = data;
}

READ8_MEMBER(uzebox_state::port_b_r)
{
	return m_port_b;
}

WRITE8_MEMBER(uzebox_state::port_c_w)
{
	//  xx-- ----   blue
	//  --xx x---   green
	//  ---- -xxx   red

	line_update();
	m_port_c = data;
}

READ8_MEMBER(uzebox_state::port_c_r)
{
	return m_port_c;
}

WRITE8_MEMBER(uzebox_state::port_d_w)
{
	//  x--- ----   sound
	//  -x-- ----   SDCard CS
	//  ---x ----   LED
	//  --x- x---   NC
	//  ---- -x--   power
	//  ---- --xx   UART MIDI

	m_port_d = data;
}

READ8_MEMBER(uzebox_state::port_d_r)
{
	return m_port_d;
}


/****************************************************\
* Address maps                                       *
\****************************************************/

static ADDRESS_MAP_START( uzebox_prg_map, AS_PROGRAM, 8, uzebox_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM // 64 KB internal eprom  ATmega644
ADDRESS_MAP_END

static ADDRESS_MAP_START( uzebox_data_map, AS_DATA, 8, uzebox_state )
	AM_RANGE(0x0100, 0x10ff) AM_RAM //  4KB RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( uzebox_io_map, AS_IO, 8, uzebox_state )
	AM_RANGE(AVR8_REG_A, AVR8_REG_A) AM_READWRITE( port_a_r, port_a_w )
	AM_RANGE(AVR8_REG_B, AVR8_REG_B) AM_READWRITE( port_b_r, port_b_w )
	AM_RANGE(AVR8_REG_C, AVR8_REG_C) AM_READWRITE( port_c_r, port_c_w )
	AM_RANGE(AVR8_REG_D, AVR8_REG_D) AM_READWRITE( port_d_r, port_d_w )
ADDRESS_MAP_END

/****************************************************\
* Input ports                                        *
\****************************************************/

static INPUT_PORTS_START( uzebox )
	PORT_START( "P1" )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Button Y") PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SELECT ) PORT_NAME("P1 Select") PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start") PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Button A") PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Button X") PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Button L") PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Button R") PORT_PLAYER(1)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "P2" )
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button B") PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Button Y") PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SELECT ) PORT_NAME("P1 Select") PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("P1 Start") PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Button A") PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Button X") PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Button L") PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Button R") PORT_PLAYER(2)
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/****************************************************\
* Video hardware                                     *
\****************************************************/

void uzebox_state::line_update()
{
	UINT32 cycles = (UINT32)(m_maincpu->get_elapsed_cycles() - m_line_start_cycles) / 2;

	for (UINT32 x = m_line_pos_cycles; x < cycles; x++)
	{
		if (m_bitmap.cliprect().contains(x, m_vpos))
			m_bitmap.pix32(m_vpos, x) = MAKE_RGB(pal3bit(m_port_c >> 0), pal3bit(m_port_c >> 3), pal2bit(m_port_c >> 6));
		if (!INTERLACED)
			if (m_bitmap.cliprect().contains(x, m_vpos + 1))
				m_bitmap.pix32(m_vpos + 1, x) = MAKE_RGB(pal3bit(m_port_c >> 0), pal3bit(m_port_c >> 3), pal2bit(m_port_c >> 6));
	}

	m_line_pos_cycles = cycles;
}

UINT32 uzebox_state::screen_update_uzebox(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

/****************************************************\
* Machine definition                                 *
\****************************************************/

const avr8_config atmega644_config =
{
	"eeprom"
};

static MACHINE_CONFIG_START( uzebox, uzebox_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ATMEGA644, MASTER_CLOCK)
	MCFG_CPU_AVR8_CONFIG(atmega644_config)
	MCFG_CPU_PROGRAM_MAP(uzebox_prg_map)
	MCFG_CPU_DATA_MAP(uzebox_data_map)
	MCFG_CPU_IO_MAP(uzebox_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.99)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1395))
	MCFG_SCREEN_SIZE(870, 525)
	MCFG_SCREEN_VISIBLE_AREA(150, 870-1, 40, 488-1)
	MCFG_SCREEN_UPDATE_DRIVER(uzebox_state, screen_update_uzebox)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("avr8")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(0, "avr8", 1.00)

	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("uzebox")
	MCFG_SOFTWARE_LIST_ADD("eprom_list","uzebox")
MACHINE_CONFIG_END

ROM_START( uzebox )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )  /* Main program store */
	ROM_CART_LOAD("cart1", 0x0000, 0x10000, ROM_OPTIONAL | ROM_FILL_FF)

	ROM_REGION( 0x200, "eeprom", ROMREGION_ERASE00 )  /* on-die eeprom */
ROM_END

/*   YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT      COMPANY   FULLNAME */
CONS(2010, uzebox,   0,        0,        uzebox,   uzebox, driver_device,   0,  "Belogic", "Uzebox", GAME_NO_SOUND | GAME_NOT_WORKING)
