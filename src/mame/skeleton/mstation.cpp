// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    CIDCO MailStation

    22/10/2011 Preliminary driver by Sandro Ronco

    Hardware:
        - Z80 CPU
        - 29f080 8 Mbit flash
        - 28SF040 4 Mbit flash (for user data)
        - 128kb RAM
        - 320x128 LCD
        - RCV336ACFW 33.6kbps modem

    TODO:
    - RCV336ACFW modem
    - Add similar models (Mivo 100/150/200/250/350)
    - New Mail led
    - NVRAM

    More info:
      http://www.fybertech.net/mailstation/info.php

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/intelfsh.h"
#include "machine/ram.h"
#include "machine/rp5c01.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"


namespace {

class mstation_state : public driver_device
{
public:
	mstation_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_bankdev1(*this, "bank0")
		, m_bankdev2(*this, "bank1")
		, m_keyboard(*this, "LINE.%u", 0)
		, m_nvram(*this, "nvram")
	{
	}

	void mstation(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_bankdev1;
	required_device<address_map_bank_device> m_bankdev2;
	required_ioport_array<10> m_keyboard;
	required_shared_ptr<uint8_t> m_nvram;

	uint8_t m_bank1[2];
	uint8_t m_bank2[2];
	std::unique_ptr<uint8_t[]> m_vram;
	uint8_t m_screen_column = 0;
	uint8_t m_port2 = 0;
	uint8_t m_irq = 0;
	uint16_t m_kb_matrix = 0;

	uint8_t modem_r();
	void modem_w(uint8_t data);

	void lcd_w(uint16_t offset, int column, uint8_t data);
	uint8_t lcd_r(uint16_t offset, int column);
	uint8_t lcd_right_r(offs_t offset);
	void lcd_right_w(offs_t offset, uint8_t data);
	uint8_t lcd_left_r(offs_t offset);
	void lcd_left_w(offs_t offset, uint8_t data);

	uint8_t bank1_r(offs_t offset);
	void bank1_w(offs_t offset, uint8_t data);
	uint8_t bank2_r(offs_t offset);
	void bank2_w(offs_t offset, uint8_t data);

	uint8_t battery_status_r();
	void port2_w(uint8_t data);
	uint8_t kb_r();
	void kb_w(uint8_t data);
	uint8_t irq_r();
	void irq_w(uint8_t data);
	void refresh_ints();

	void rtc_irq(int state);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mstation_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(mstation_1hz_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(mstation_kb_timer);
	void mstation_banked_map(address_map &map) ATTR_COLD;
	void mstation_io(address_map &map) ATTR_COLD;
	void mstation_mem(address_map &map) ATTR_COLD;
};


uint8_t mstation_state::modem_r()
{
	return 0xff;
}

void mstation_state::modem_w(uint8_t data)
{
}


//***************************************************************************
//  video hardware emulation
//***************************************************************************/

void mstation_state::lcd_w(uint16_t offset, int column, uint8_t data)
{
	if (m_port2 & 0x08)
		m_vram[(column * 240) + (offset % 240)] = data;
	else
		m_screen_column = data % 20;
}

uint8_t mstation_state::lcd_r(uint16_t offset, int column)
{
	if (m_port2 & 0x08)
		return m_vram[(column * 240) + (offset % 240)];
	else
		return m_screen_column % 20;
}

uint8_t mstation_state::lcd_left_r(offs_t offset) {  return lcd_r(offset, m_screen_column);  }
void mstation_state::lcd_left_w(offs_t offset, uint8_t data) {  lcd_w(offset, m_screen_column, data);   }
uint8_t mstation_state::lcd_right_r(offs_t offset) {  return lcd_r(offset, m_screen_column + 20); }
void mstation_state::lcd_right_w(offs_t offset, uint8_t data) {  lcd_w(offset, m_screen_column + 20, data);  }

uint32_t mstation_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int x=0; x<40; x++)
		for (int y=0; y<128; y++)
		{
			uint8_t data = m_vram[56 + y + x * 240];

			for (int b=0; b<8; b++)
			{
				// columns are inverted (right to left)
				int col = ((x < 20) ? 19 : 59) - x;

				bitmap.pix(y, col*8 + b)= BIT(data, 0);
				data >>= 1;
			}
		}
	return 0;
}

//***************************************************************************
//  Bankswitch
//***************************************************************************

uint8_t mstation_state::bank1_r(offs_t offset)
{
	return m_bank1[offset];
}

uint8_t mstation_state::bank2_r(offs_t offset)
{
	return m_bank2[offset];
}

void mstation_state::bank1_w(offs_t offset, uint8_t data)
{
	m_bank1[offset] = data;

	m_bankdev1->set_bank(((m_bank1[1] & 0x07) << 8) | m_bank1[0]);
}

void mstation_state::bank2_w(offs_t offset, uint8_t data)
{
	m_bank2[offset] = data;

	m_bankdev2->set_bank(((m_bank2[1] & 0x07) << 8) | m_bank2[0]);
}



void mstation_state::port2_w(uint8_t data)
{
	m_port2 = data;

	m_kb_matrix = (m_kb_matrix & 0xff) | ((data & 0x03)<<8);
}

/*
    IRQ bits (port 3) ordered by priority:

    bit 7: power down request
    bit 5: real time clock
    bit 6: modem
    bit 4: 1 sec int
    bit 3: unknown
    bit 0: unknown
    bit 1: keyboard int
    bit 2: unknown
*/

void mstation_state::refresh_ints()
{
	if (m_irq != 0)
		m_maincpu->set_input_line(0, HOLD_LINE);
	else
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

uint8_t mstation_state::irq_r()
{
	return m_irq;
}

void mstation_state::irq_w(uint8_t data)
{
	m_irq &= data;

	refresh_ints();
}

uint8_t mstation_state::battery_status_r()
{
	/*
	  bit 0-3 - unknown
	  bit 4-7 - battery status
	*/
	return 0xf0;
}

void mstation_state::kb_w(uint8_t data)
{
	m_kb_matrix = (m_kb_matrix & 0x300) | data;
}

uint8_t mstation_state::kb_r()
{
	uint8_t data = 0xff;

	for (int i=0; i<10; i++)
	{
		if (!(m_kb_matrix & (1<<i)))
			data &= m_keyboard[i]->read();
	}

	return data;
}


void mstation_state::mstation_banked_map(address_map &map)
{
	map(0x0000000, 0x00fffff).mirror(0x0300000).rw("flash0", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x0400000, 0x041ffff).mirror(0x03e0000).ram().share("nvram");
	map(0x0c00000, 0x0c7ffff).mirror(0x0380000).rw("flash1", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x0800000, 0x0803fff).mirror(0x03fc000).rw(FUNC(mstation_state::lcd_left_r), FUNC(mstation_state::lcd_left_w));
	map(0x1000000, 0x1003fff).mirror(0x03fc000).rw(FUNC(mstation_state::lcd_right_r), FUNC(mstation_state::lcd_right_w));
	map(0x1400000, 0x1403fff).mirror(0x03fc000).rw(FUNC(mstation_state::modem_r), FUNC(mstation_state::modem_w));
}

void mstation_state::mstation_mem(address_map &map)
{
	map(0x0000, 0x3fff).rw("flash0", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x4000, 0x7fff).rw(m_bankdev1, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0x8000, 0xbfff).rw(m_bankdev2, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xc000, 0xffff).bankrw("sysram");    // system ram always first RAM bank
}

void mstation_state::mstation_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).rw(FUNC(mstation_state::kb_r), FUNC(mstation_state::kb_w));
	map(0x02, 0x02).w(FUNC(mstation_state::port2_w));
	map(0x03, 0x03).rw(FUNC(mstation_state::irq_r), FUNC(mstation_state::irq_w));
	map(0x05, 0x06).rw(FUNC(mstation_state::bank1_r), FUNC(mstation_state::bank1_w));
	map(0x07, 0x08).rw(FUNC(mstation_state::bank2_r), FUNC(mstation_state::bank2_w));
	map(0x09, 0x09).r(FUNC(mstation_state::battery_status_r));
	map(0x10, 0x1f).rw("rtc", FUNC(rp5c01_device::read), FUNC(rp5c01_device::write));
	//map(0x2c, 0x2c) printer
}


/* Input ports */
static INPUT_PORTS_START( mstation )
	PORT_START( "LINE.0" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Main Menu")  PORT_CODE( KEYCODE_HOME )   PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Back")   PORT_CODE( KEYCODE_DEL )    PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Print")  PORT_CODE( KEYCODE_F6 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("F1")     PORT_CODE( KEYCODE_F1 )     PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("F2")     PORT_CODE( KEYCODE_F2 )     PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("F3")     PORT_CODE( KEYCODE_F3 )     PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("F4")     PORT_CODE( KEYCODE_F4 )     PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("F5")     PORT_CODE( KEYCODE_F5 )     PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START( "LINE.1" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_END )    PORT_CHAR('@')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("A-A Size")       PORT_CODE( KEYCODE_F7 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Check Spelling") PORT_CODE( KEYCODE_F8 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Get E-Mail")     PORT_CODE( KEYCODE_F9 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("PG Up")      PORT_CODE( KEYCODE_PGUP )   PORT_CHAR(UCHAR_MAMEKEY(PGUP))

	PORT_START( "LINE.2" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_0_PAD )  PORT_CHAR(U'´')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_1 )      PORT_CHAR('1')      PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_2 )      PORT_CHAR('2')      PORT_CHAR('@')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_3 )      PORT_CHAR('3')      PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_4 )      PORT_CHAR('4')      PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_5 )      PORT_CHAR('5')      PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_6 )      PORT_CHAR('6')      PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_7 )      PORT_CHAR('7')      PORT_CHAR('&')

	PORT_START( "LINE.3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_8 )      PORT_CHAR('8')      PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_9 )      PORT_CHAR('9')      PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_0 )      PORT_CHAR('0')      PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_MINUS )  PORT_CHAR('-')      PORT_CHAR('_')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_EQUALS ) PORT_CHAR('=')      PORT_CHAR('+')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Backspace")      PORT_CODE( KEYCODE_BACKSPACE )      PORT_CHAR(8)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_BACKSLASH )  PORT_CHAR('\\')     PORT_CHAR('|')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("PG Down")        PORT_CODE( KEYCODE_PGDN )       PORT_CHAR(UCHAR_MAMEKEY(PGDN))

	PORT_START( "LINE.4" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Tab")    PORT_CODE( KEYCODE_TAB )        PORT_CHAR(9)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_Q )      PORT_CHAR('q')      PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_W )      PORT_CHAR('w')      PORT_CHAR('W')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_E )      PORT_CHAR('e')      PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_R )      PORT_CHAR('r')      PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_T )      PORT_CHAR('t')      PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_Y )      PORT_CHAR('y')      PORT_CHAR('Y')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_U )      PORT_CHAR('u')      PORT_CHAR('U')

	PORT_START( "LINE.5" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_I )      PORT_CHAR('i')      PORT_CHAR('I')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_O )      PORT_CHAR('o')      PORT_CHAR('O')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_P )      PORT_CHAR('p')      PORT_CHAR('P')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_OPENBRACE )  PORT_CHAR('[')      PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_CLOSEBRACE ) PORT_CHAR(']')      PORT_CHAR('}')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_COLON )      PORT_CHAR(';')      PORT_CHAR(':')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_QUOTE )      PORT_CHAR('\'')     PORT_CHAR('\"')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Enter")      PORT_CODE( KEYCODE_ENTER )  PORT_CHAR(13)

	PORT_START( "LINE.6" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("CapsLock")       PORT_CODE( KEYCODE_CAPSLOCK )       PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_A )      PORT_CHAR('a')      PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_S )      PORT_CHAR('s')      PORT_CHAR('S')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_D )      PORT_CHAR('d')      PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_F )      PORT_CHAR('f')      PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_G )      PORT_CHAR('g')      PORT_CHAR('G')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_H )      PORT_CHAR('h')      PORT_CHAR('H')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_J )      PORT_CHAR('j')      PORT_CHAR('J')

	PORT_START( "LINE.7" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_K )      PORT_CHAR('K')      PORT_CHAR('K')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_L )      PORT_CHAR('l')      PORT_CHAR('L')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_COMMA )  PORT_CHAR(',')      PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_STOP )   PORT_CHAR('.')      PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_SLASH )  PORT_CHAR('/')      PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Up")         PORT_CODE( KEYCODE_UP )     PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Down")       PORT_CODE( KEYCODE_DOWN )   PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Right")      PORT_CODE( KEYCODE_RIGHT )  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START( "LINE.8" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Left Shift")     PORT_CODE( KEYCODE_LSHIFT )         PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_Z )      PORT_CHAR('z')      PORT_CHAR('Z')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_X )      PORT_CHAR('x')      PORT_CHAR('X')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_C )      PORT_CHAR('c')      PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_V )      PORT_CHAR('v')      PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_B )      PORT_CHAR('b')      PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_N )      PORT_CHAR('n')      PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_CODE( KEYCODE_M )      PORT_CHAR('m')      PORT_CHAR('M')

	PORT_START( "LINE.9" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Function")       PORT_CODE( KEYCODE_LCONTROL )   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Space")          PORT_CODE( KEYCODE_SPACE )      PORT_CHAR(' ')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Right Shift")    PORT_CODE( KEYCODE_RSHIFT ) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )   PORT_NAME("Left")           PORT_CODE( KEYCODE_LEFT )   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
INPUT_PORTS_END

void mstation_state::machine_start()
{
	// allocate the videoram
	m_vram = make_unique_clear<uint8_t[]>(9600);

	// map firsh RAM bank at 0xc000-0xffff
	membank("sysram")->set_base(m_nvram);
}


void mstation_state::machine_reset()
{
	m_bank1[0] =  m_bank1[1] = 0;
	m_bank2[0] =  m_bank2[1] = 0;

	// reset banks
	m_bankdev1->set_bank(0);
	m_bankdev2->set_bank(0);
}

void mstation_state::rtc_irq(int state)
{
	if (state)
		m_irq |= (1<<5);
	else
		m_irq &=~(1<<5);

	refresh_ints();
}

TIMER_DEVICE_CALLBACK_MEMBER(mstation_state::mstation_1hz_timer)
{
	m_irq |= (1<<4);

	refresh_ints();
}

TIMER_DEVICE_CALLBACK_MEMBER(mstation_state::mstation_kb_timer)
{
	m_irq |= (1<<1);

	refresh_ints();
}

void mstation_state::mstation_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}


void mstation_state::mstation(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000));      //unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &mstation_state::mstation_mem);
	m_maincpu->set_addrmap(AS_IO, &mstation_state::mstation_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(mstation_state::screen_update));
	screen.set_size(320, 128);
	screen.set_visarea(0, 320-1, 0, 128-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(mstation_state::mstation_palette), 2);

	AMD_29F080(config, "flash0");
	SST_28SF040(config, "flash1");

	// IRQ 4 is generated every second, used for auto power off
	TIMER(config, "1hz_timer").configure_periodic(FUNC(mstation_state::mstation_1hz_timer), attotime::from_hz(1));

	// IRQ 1 is used for scan the kb and for cursor blinking
	TIMER(config, "kb_timer").configure_periodic(FUNC(mstation_state::mstation_kb_timer), attotime::from_hz(50));

	rp5c01_device &rtc(RP5C01(config, "rtc", XTAL(32'768)));
	rtc.out_alarm_callback().set(FUNC(mstation_state::rtc_irq));

	ADDRESS_MAP_BANK(config, "bank0").set_map(&mstation_state::mstation_banked_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, "bank1").set_map(&mstation_state::mstation_banked_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("128K");
}

/* ROM definition */
ROM_START( mstation )
	ROM_REGION( 0x100000, "flash0", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v303a", "v3.03a" )
	ROMX_LOAD("ms303a.bin", 0x000000, 0x100000, CRC(7a5cf752) SHA1(15629ccaecd8094dd883987bed94c16eee6de7c2), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v253", "v2.53" )
	ROMX_LOAD("ms253.bin",  0x000000, 0x0fc000, BAD_DUMP CRC(a27e7f8b) SHA1(ae5a0aa0f1e23f3b183c5c0bcf4d4c1ae54b1798), ROM_BIOS(1))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME       FLAGS
COMP( 1999, mstation, 0,      0,      mstation, mstation, mstation_state, empty_init, "CIDCO", "MailStation", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
