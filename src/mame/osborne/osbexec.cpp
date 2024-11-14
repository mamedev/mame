// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Osborne Executive driver file

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/z80daisy.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/ripple_counter.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

#define MAIN_CLOCK  XTAL(23'961'600)

#define MODEM_PORT_TAG "modem"
#define PRINTER_PORT_TAG "printer"


class osbexec_state : public driver_device
{
public:
	osbexec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_mb8877(*this, "mb8877")
		, m_messram( *this, RAM_TAG)
		, m_pia(*this, "pia_%u", 0U)
		, m_rtc(*this, "rtc")
		, m_sio(*this, "sio")
		, m_speaker(*this, "speaker")
		, m_floppy(*this, "mb8877:%u:525ssdd", 0U)
		, m_kbd_row(*this, "ROW%u", 0U)
	{ }

	void osbexec(machine_config &config);

	void init_osbexec();

private:
	required_device<z80_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<mb8877_device>  m_mb8877;
	required_device<ram_device> m_messram;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<ripple_counter_device> m_rtc;
	required_device<z80sio_device> m_sio;
	required_device<speaker_sound_device>   m_speaker;
	required_device_array<floppy_image_device, 2> m_floppy;
	required_ioport_array<8> m_kbd_row;

	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bitmap_ind16 m_bitmap;

	std::unique_ptr<uint8_t[]> m_fontram;
	std::unique_ptr<uint8_t[]> m_vram;
	uint8_t   *m_ram_0000 = nullptr;
	uint8_t   *m_ram_c000 = nullptr;
	uint8_t   m_temp_attr = 0;
	emu_timer *m_video_timer = nullptr;

	/* PIA 0 (UD12) */
	uint8_t   m_pia0_porta = 0;
	uint8_t   m_pia0_portb = 0;
	int     m_pia0_cb2 = 0;         /* 60/50 */

	/* PIA 1 (UD8) */

	void set_banks()
	{
		uint8_t *ram_ptr = m_messram->pointer();

		m_ram_0000 = ram_ptr;

		if ( m_pia0_porta & 0x01 )
			m_ram_0000 += 0x10000;

		membank( "0000" )->set_base( m_ram_0000 + 0x0000 );
		membank( "2000" )->set_base( m_ram_0000 + 0x2000 );
		membank( "4000" )->set_base( m_ram_0000 + 0x4000 );
		m_ram_c000 = m_ram_0000 + 0xc000;
		membank( "e000" )->set_base( m_ram_0000 + 0xe000 );

		if ( m_pia0_porta & 0x80 )
		{
			membank( "0000" )->set_base( memregion("maincpu")->base());
			/* When BIOS is enabled 2000-3FFF is set to the "ROM RAM" */
			membank( "2000" )->set_base( ram_ptr + 0x20000 );
		}

		if ( m_pia0_porta & 0x40 )
			m_ram_c000 = m_vram.get();
	}

	void osbexec_0000_w(offs_t offset, uint8_t data);
	uint8_t osbexec_c000_r(offs_t offset);
	void osbexec_c000_w(offs_t offset, uint8_t data);
	uint8_t osbexec_kbd_r(offs_t offset);
	uint8_t osbexec_rtc_r();
	virtual void machine_reset() override ATTR_COLD;
	TIMER_CALLBACK_MEMBER(osbexec_video_callback);
	uint8_t osbexec_pia0_a_r();
	void osbexec_pia0_a_w(uint8_t data);
	uint8_t osbexec_pia0_b_r();
	void osbexec_pia0_b_w(uint8_t data);
	void osbexec_pia0_ca2_w(int state);
	void osbexec_pia0_cb2_w(int state);
	void modem_txclk_w(int state);
	void modem_rxclk_w(int state);
	void modem_dsr_w(int state);
	void modem_ri_w(int state);
	void comm_clk_a_w(int state);
	void osbexec_io(address_map &map) ATTR_COLD;
	void osbexec_mem(address_map &map) ATTR_COLD;
};


void osbexec_state::osbexec_0000_w(offs_t offset, uint8_t data)
{
	/* Font RAM writing is enabled when ROM bank is enabled */
	if ( m_pia0_porta & 0x80 )
	{
		if ( offset < 0x1000 )
			m_fontram[ offset ] = data;
	}
	else
	{
		m_ram_0000[ offset ] = data;
	}
}


uint8_t osbexec_state::osbexec_c000_r(offs_t offset)
{
	uint8_t   data = m_ram_c000[offset];

	if ( ( m_pia0_porta & 0x40 ) && offset < 0x1000 )
	{
		m_temp_attr = m_ram_c000[ 0x1000 + offset ];
	}

	return data;
}


void osbexec_state::osbexec_c000_w(offs_t offset, uint8_t data)
{
	m_ram_c000[offset] = data;

	if ( ( m_pia0_porta & 0x40 ) && offset < 0x1000 )
	{
		m_ram_c000[ 0x1000 + offset ] = m_temp_attr;
	}
}


uint8_t osbexec_state::osbexec_kbd_r(offs_t offset)
{
	uint8_t data = 0xFF;

	for (int j = 0; j < 8; j++)
		if (BIT(offset, j + 8))
			data &= m_kbd_row[j]->read();

	return data;
}


uint8_t osbexec_state::osbexec_rtc_r()
{
	// 74LS244 buffer @ UF13
	return m_rtc->count();
}


void osbexec_state::osbexec_mem(address_map &map)
{
	map(0x0000, 0x1FFF).bankr("0000").w(FUNC(osbexec_state::osbexec_0000_w));   /* ROM and maybe also banked ram */
	map(0x2000, 0x3FFF).bankrw("2000");                               /* Banked RAM */
	map(0x4000, 0xBFFF).bankrw("4000");                               /* Banked RAM */
	map(0xC000, 0xDFFF).rw(FUNC(osbexec_state::osbexec_c000_r), FUNC(osbexec_state::osbexec_c000_w));    /* Video ram / Banked RAM */
	map(0xE000, 0xEFFF).bankrw("e000");                               /* Banked RAM */
	map(0xF000, 0xFFFF).ram();                                           /* 4KB of non-banked RAM for system stack etc */
}


void osbexec_state::osbexec_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x03).mirror(0xff00).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));       /* 6821 PIA @ UD12 */
	map(0x04, 0x07).mirror(0xff00).rw("ctc", FUNC(pit8253_device::read), FUNC(pit8253_device::write));          /* 8253 @UD1 */
	map(0x08, 0x0B).mirror(0xff00).rw(m_mb8877, FUNC(wd_fdc_device_base::read), FUNC(wd_fdc_device_base::write));  /* MB8877 @ UB17 input clock = 1MHz */
	map(0x0C, 0x0F).mirror(0xff00).rw(m_sio, FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));    /* SIO @ UD4 */
	map(0x10, 0x13).mirror(0xff00).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));       /* 6821 PIA @ UD8 */
	map(0x14, 0x17).select(0xff00).r(FUNC(osbexec_state::osbexec_kbd_r));                                      /* KBD */
	map(0x18, 0x1b).mirror(0xff00).r(FUNC(osbexec_state::osbexec_rtc_r));                                      /* "RTC" @ UE13/UF13 */
	/* ?? - vid ? */
}


static INPUT_PORTS_START( osbexec )
	PORT_START("ROW0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)   PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)       PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)     PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)          PORT_CHAR('\t')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)          PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("ROW1")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)            PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)            PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)            PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)            PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)            PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)            PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)            PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)            PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)            PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)            PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(21)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)            PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR(25)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)            PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(20)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)            PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR(18)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)            PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(5)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)            PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR(23)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)            PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR(17)

	PORT_START("ROW3")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)            PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR(11)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)            PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR(10)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)            PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR(8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)            PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR(7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)            PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR(6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)            PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR(4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)            PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR(19)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)            PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR(1)

	PORT_START("ROW4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)            PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)            PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(14)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)            PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)            PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR(22)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)            PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR(3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)            PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR(24)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)            PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(26)

	PORT_START("ROW5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)            PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)            PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(15)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)            PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR(16)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)        PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)            PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)         PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)           PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("ROW6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)       PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)            PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(12)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)         PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)        PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("ROW7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_DIPNAME( 0x08, 0, "Alpha Lock" ) PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void osbexec_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
}

uint32_t osbexec_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

/*
  UD12 - 6821 PIA

  Port A:
  PA7 - ROM BANK ENA
  PA6 - VRAM BANK ENA
  PA5 - BANK6ENA
  PA4 - BANK5ENA
  PA3 - BANK4ENA
  PA2 - BANK3ENA
  PA1 - BANK2ENA
  PA0 - BANK1ENA
  CA1 - DMA IRQ
  CA2 - KBD STB (i/o)

  Port B:
  PB7 - MODEM RI (input)
  PB6 - MODEM DSR (input)
  PB5 - TXC SEL
  PB4 - RXC SEL
  PB3 - speaker
  PB2 - DSEL2
  PB1 - DSEL1
  PB0 - DDEN
  CB1 - VBlank (input)
  CB2 - 60/50 (?)
*/

uint8_t osbexec_state::osbexec_pia0_a_r()
{
	return m_pia0_porta;
}


void osbexec_state::osbexec_pia0_a_w(uint8_t data)
{
	//logerror("osbexec_pia0_a_w: %02x\n", data );

	m_pia0_porta = data;

	set_banks();
}


uint8_t osbexec_state::osbexec_pia0_b_r()
{
	return m_pia0_portb;
}


void osbexec_state::osbexec_pia0_b_w(uint8_t data)
{
	m_pia0_portb = (m_pia0_portb & 0xc0) | (data & 0x3f);

	m_speaker->level_w(!BIT(data, 3));

	switch ( data & 0x06 )
	{
	case 0x02:
		m_mb8877->set_floppy(m_floppy[1].target());
		m_floppy[1]->mon_w(0);
		break;
	case 0x04:
		m_mb8877->set_floppy(m_floppy[0].target());
		m_floppy[0]->mon_w(0);
		break;
	default:
		m_mb8877->set_floppy(nullptr);
		break;
	}

	m_mb8877->dden_w(( data & 0x01 ) ? 1 : 0 );
}


void osbexec_state::osbexec_pia0_ca2_w(int state)
{
	logerror("osbexec_pia0_ca2_w: state = %d\n", state);
}


void osbexec_state::osbexec_pia0_cb2_w(int state)
{
	m_pia0_cb2 = state;
}


void osbexec_state::modem_txclk_w(int state)
{
	if (BIT(m_pia0_portb, 5))
		m_sio->txca_w(state);
}


void osbexec_state::modem_rxclk_w(int state)
{
	if (BIT(m_pia0_portb, 4))
		m_sio->rxca_w(state);
}


void osbexec_state::modem_dsr_w(int state)
{
	if (state)
		m_pia0_portb |= 0x40;
	else
		m_pia0_portb &= 0xbf;
}


void osbexec_state::modem_ri_w(int state)
{
	if (state)
		m_pia0_portb |= 0x80;
	else
		m_pia0_portb &= 0x7f;
}


void osbexec_state::comm_clk_a_w(int state)
{
	if (!BIT(m_pia0_portb, 5))
		m_sio->txca_w(state);
	if (!BIT(m_pia0_portb, 4))
		m_sio->rxca_w(state);
}


/*
 * The Osborne Executive supports the following disc formats: (TODO: Verify)
 * - Osborne single density: 40 tracks, 10 sectors per track, 256-byte sectors (100 KByte)
 * - Osborne double density: 40 tracks, 5 sectors per track, 1024-byte sectors (200 KByte)
 * - IBM Personal Computer: 40 tracks, 8 sectors per track, 512-byte sectors (160 KByte)
 * - Xerox 820 Computer: 40 tracks, 18 sectors per track, 128-byte sectors (90 KByte)
 * - DEC 1820 double density: 40 tracks, 9 sectors per track, 512-byte sectors (180 KByte)
 *
 */

static void osborne2_floppies(device_slot_interface &device)
{
	device.option_add("525ssdd", FLOPPY_525_SSDD);
}


TIMER_CALLBACK_MEMBER(osbexec_state::osbexec_video_callback)
{
	int y = m_screen->vpos();

	if ( y < 240 )
	{
		uint16_t row_addr = ( y / 10 ) * 128;
		uint16_t *const p = &m_bitmap.pix(y);
		uint8_t char_line = y % 10;

		for ( int x = 0; x < 80; x++ )
		{
			uint8_t ch = m_vram[ row_addr + x ];
			uint8_t attr = m_vram[ 0x1000 + row_addr + x ];
			uint8_t fg_col = ( attr & 0x80 ) ? 2 : 1;
			uint8_t font_bits = m_fontram[ ( ( attr & 0x10 ) ? 0x800 : 0 ) + ( ch & 0x7f ) * 16 + char_line ];

			/* Check for underline */
			if (BIT(attr, 6) && char_line == 9)
				font_bits = 0xFF;

			/* Check for blink */
			if (BIT(attr, 5) && BIT(m_rtc->count(), 4))
				font_bits = 0;

			/* Check for inverse video */
			if (BIT(ch, 7) && !BIT(attr, 4))
				font_bits ^= 0xFF;

			for ( int b = 0; b < 8; b++ )
			{
				p[ x * 8 + b ] = ( font_bits & 0x80 ) ? fg_col : 0;
				font_bits <<= 1;
			}
		}
	}

	m_video_timer->adjust( m_screen->time_until_pos( y + 1, 0 ) );
}


void osbexec_state::init_osbexec()
{
	m_vram = make_unique_clear<uint8_t[]>(0x2000);
	m_fontram = make_unique_clear<uint8_t[]>(0x1000);

	m_video_timer = timer_alloc(FUNC(osbexec_state::osbexec_video_callback), this);
}


void osbexec_state::machine_reset()
{
	m_pia0_porta = 0xC0;        /* Enable ROM and VRAM on reset */

	set_banks();

	m_video_timer->adjust( m_screen->time_until_pos( 0, 0 ) );

	// D0 cleared on interrupt acknowledge cycle by TTL gates at UC21 and UA18
	m_maincpu->set_input_line_vector(0, 0xfe); // Z80
}


static const z80_daisy_config osbexec_daisy_config[] =
{
	{ "sio" },
	{ nullptr }
};


void osbexec_state::osbexec(machine_config &config)
{
	Z80(config, m_maincpu, MAIN_CLOCK/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &osbexec_state::osbexec_mem);
	m_maincpu->set_addrmap(AS_IO, &osbexec_state::osbexec_io);
	m_maincpu->set_daisy_config(osbexec_daisy_config);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_color(rgb_t::green());
	m_screen->set_screen_update(FUNC(osbexec_state::screen_update));
	m_screen->set_raw(MAIN_CLOCK/2, 768, 0, 640, 260, 0, 240);    // May not be correct
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set(m_pia[0], FUNC(pia6821_device::cb1_w));
	m_screen->screen_vblank().append(m_rtc, FUNC(ripple_counter_device::clock_w)).invert();

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	PIA6821(config, m_pia[0]);
	m_pia[0]->readpa_handler().set(FUNC(osbexec_state::osbexec_pia0_a_r));
	m_pia[0]->readpb_handler().set(FUNC(osbexec_state::osbexec_pia0_b_r));
	m_pia[0]->writepa_handler().set(FUNC(osbexec_state::osbexec_pia0_a_w));
	m_pia[0]->writepb_handler().set(FUNC(osbexec_state::osbexec_pia0_b_w));
	m_pia[0]->ca2_handler().set(FUNC(osbexec_state::osbexec_pia0_ca2_w));
	m_pia[0]->cb2_handler().set(FUNC(osbexec_state::osbexec_pia0_cb2_w));
	m_pia[0]->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_pia[0]->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	PIA6821(config, m_pia[1]);
	m_pia[1]->irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));
	m_pia[1]->irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<3>));

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, 0);

	RIPPLE_COUNTER(config, m_rtc); // 74LS393 @ UE13
	m_rtc->set_stages(8); // two halves cascaded

	Z80SIO(config, m_sio, MAIN_CLOCK/6);
	m_sio->out_txda_callback().set(MODEM_PORT_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(MODEM_PORT_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(MODEM_PORT_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_txdb_callback().set(PRINTER_PORT_TAG, FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(PRINTER_PORT_TAG, FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(PRINTER_PORT_TAG, FUNC(rs232_port_device::write_rts));
	m_sio->out_int_callback().set("mainirq", FUNC(input_merger_device::in_w<4>));

	pit8253_device &ctc(PIT8253(config, "ctc", 0));
	ctc.set_clk<0>(MAIN_CLOCK / 13); // divided by 74S161 @ UC25
	ctc.set_clk<1>(MAIN_CLOCK / 13); // divided by 74S161 @ UC25
	ctc.set_clk<2>(MAIN_CLOCK / 12);
	ctc.out_handler<0>().set(FUNC(osbexec_state::comm_clk_a_w));
	ctc.out_handler<0>().append(MODEM_PORT_TAG, FUNC(rs232_port_device::write_etc));
	ctc.out_handler<1>().set(m_sio, FUNC(z80sio_device::rxtxcb_w));
	//ctc.out_handler<2>().set(FUNC(osbexec_state::spindle_clk_w));

	rs232_port_device &modem_port(RS232_PORT(config, MODEM_PORT_TAG, default_rs232_devices, nullptr));
	modem_port.rxd_handler().set(m_sio, FUNC(z80sio_device::rxa_w));
	modem_port.dcd_handler().set(m_sio, FUNC(z80sio_device::dcda_w));
	modem_port.dsr_handler().set(FUNC(osbexec_state::modem_dsr_w));
	modem_port.ri_handler().set(FUNC(osbexec_state::modem_ri_w));
	modem_port.cts_handler().set(m_sio, FUNC(z80sio_device::ctsa_w));
	modem_port.txc_handler().set(FUNC(osbexec_state::modem_txclk_w));
	modem_port.rxc_handler().set(FUNC(osbexec_state::modem_rxclk_w));

	rs232_port_device &printer_port(RS232_PORT(config, PRINTER_PORT_TAG, default_rs232_devices, nullptr));
	printer_port.rxd_handler().set(m_sio, FUNC(z80sio_device::rxb_w));
	printer_port.dcd_handler().set(m_sio, FUNC(z80sio_device::dcdb_w));
	printer_port.cts_handler().set(m_sio, FUNC(z80sio_device::ctsb_w));

	MB8877(config, m_mb8877, MAIN_CLOCK/24);
	m_mb8877->intrq_wr_callback().set(m_pia[1], FUNC(pia6821_device::cb1_w));
	FLOPPY_CONNECTOR(config, "mb8877:0", osborne2_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, "mb8877:1", osborne2_floppies, "525ssdd", floppy_image_device::default_mfm_floppy_formats);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("136K"); /* 128KB Main RAM + RAM in ROM bank (8) */

	/* software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("osborne2");
}


ROM_START( osbexec )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "execv12.ud18", 0x0000, 0x2000, CRC(70798c2f) SHA1(2145a72da563bed1d6d455c77e48cc011a5f1153) )    /* Checksum C6B2 */
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY    FULLNAME     FLAGS
COMP( 1982, osbexec, 0,      0,      osbexec, osbexec, osbexec_state, init_osbexec, "Osborne", "Executive", MACHINE_NOT_WORKING )
