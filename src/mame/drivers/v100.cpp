// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    The Visual 100 was the first of the second series of display terminals released by Visual Technology. (The Visual 200 and
    Visual 210 made up the first series.) It was followed by the Visual 110 (which emulated the Data General Dasher series rather
    than the DEC VT-100) and the "top of the line" Visual 400.

    The second 8251 and 8116T seem to have been typically unpopulated. However, the program does include routines to drive these
    components, which probably would be installed to provide the optional serial printer interface.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/com8116.h"
#include "machine/er1400.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "video/tms9927.h"
#include "screen.h"

// character matrix is supposed to be only 7x7, but 15 produces correct timings
#define CHAR_WIDTH 15

class v100_state : public driver_device
{
public:
	v100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_vtac(*this, "vtac")
		, m_brg(*this, "brg%u", 1)
		, m_usart(*this, "usart%u", 1)
		, m_earom(*this, "earom")
		, m_picu(*this, "picu")
		, m_p_chargen(*this, "chargen")
		, m_videoram(*this, "videoram")
		, m_key_row(*this, "ROW%u", 0)
	{ }

	template<int N> DECLARE_WRITE8_MEMBER(brg_w);
	DECLARE_READ8_MEMBER(earom_r);
	DECLARE_WRITE8_MEMBER(port30_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(key_row_w);
	DECLARE_WRITE8_MEMBER(port48_w);
	DECLARE_WRITE8_MEMBER(picu_w);
	template<int N> DECLARE_WRITE_LINE_MEMBER(picu_r_w);
	IRQ_CALLBACK_MEMBER(irq_ack);
	DECLARE_WRITE8_MEMBER(ppi_porta_w);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void v100(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<crt5037_device> m_vtac;
	required_device_array<com8116_device, 2> m_brg;
	required_device_array<i8251_device, 2> m_usart;
	required_device<er1400_device> m_earom;
	required_device<i8214_device> m_picu;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_videoram;
	optional_ioport_array<16> m_key_row;

	u8 m_active_row;
};

u32 v100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_vtac->screen_reset())
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	unsigned row0 = cliprect.top() / 10;
	unsigned x0 = cliprect.left();
	unsigned px0 = x0 % CHAR_WIDTH;

	u16 start = 0;
	unsigned y = 0;
	for (unsigned row = 0; y <= cliprect.bottom(); row++)
	{
		start = m_videoram[start] | (m_videoram[(start + 1) & 0xfff] << 8);
		u16 end = start + 0x50;
		switch (start & 0xf000)
		{
		case 0x1000:
			end = start + 0x29;
			break;
		case 0x3000:
			end = start + 0x6e;
			break;
		case 0xc000:
			end = start + 0x50;
			break;
		default:
			break;
		}
		start &= 0xfff;
		end &= 0xfff;

		if (row < row0)
			y += 10;
		else
		{
			unsigned scan = 0;
			if (row == row0)
			{
				scan += cliprect.top() - y;
				y = cliprect.top();
			}
			while (scan < 10 && y <= cliprect.bottom())
			{
				unsigned x = x0, px = px0, addr = start;
				u8 gfxdata = m_p_chargen[((m_videoram[addr] & 0x7f) << 4) | scan] << (px0 / 2);
				while (x <= cliprect.right())
				{
					bitmap.pix(y, x) = BIT(gfxdata, 7) ? rgb_t::white() : rgb_t::black();
					x++;
					px++;
					if ((px & 1) == 0)
						gfxdata <<= 1;
					if (px >= CHAR_WIDTH)
					{
						addr = (addr + 1) & 0xfff;
						gfxdata = m_p_chargen[((m_videoram[addr] & 0x7f) << 4) | scan];
						px = 0;

						if (addr == end)
						{
							while (x <= cliprect.right())
								bitmap.pix(y, x++) = rgb_t::black();
							break;
						}
					}
				}
				scan++;
				y++;
			}
		}

		start = end;
	}

	return 0;
}

void v100_state::machine_start()
{
	m_picu->inte_w(1);
	m_picu->etlg_w(1);

	m_usart[0]->write_cts(0);
	m_usart[1]->write_cts(0);

	m_active_row = 0;
	save_item(NAME(m_active_row));
}

template<int N>
WRITE8_MEMBER(v100_state::brg_w)
{
	m_brg[N]->str_w(data & 0x0f);
	m_brg[N]->stt_w(data >> 4);
}

READ8_MEMBER(v100_state::earom_r)
{
	return m_earom->data_r();
}

WRITE8_MEMBER(v100_state::port30_w)
{
	// D6 = cursor/text blinking?

	//logerror("Writing %02X to port 30\n", data);
}

READ8_MEMBER(v100_state::keyboard_r)
{
	return m_key_row[m_active_row & 15].read_safe(0xff);
}

WRITE8_MEMBER(v100_state::key_row_w)
{
	m_active_row = data;
}

WRITE8_MEMBER(v100_state::port48_w)
{
	//logerror("Writing %02X to port 48\n", data);
}

WRITE8_MEMBER(v100_state::picu_w)
{
	m_picu->b_w((data & 0x0e) >> 1);
	m_picu->sgs_w(BIT(data, 4));
}

template<int N>
WRITE_LINE_MEMBER(v100_state::picu_r_w)
{
	m_picu->r_w(N, state);
}

IRQ_CALLBACK_MEMBER(v100_state::irq_ack)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return (m_picu->a_r() << 1) | 0xf0;
}

WRITE8_MEMBER(v100_state::ppi_porta_w)
{
	m_vtac->set_clock_scale(BIT(data, 5) ? 0.5 : 1.0);
	m_screen->set_clock_scale(BIT(data, 5) ? 0.5 : 1.0);

	//logerror("Writing %02X to PPI port A\n", data);
}

ADDRESS_MAP_START(v100_state::mem_map)
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x4000, 0x4fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x5000, 0x5fff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(v100_state::io_map)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x0f) AM_DEVWRITE("vtac", crt5037_device, write)
	AM_RANGE(0x10, 0x10) AM_WRITE(brg_w<0>)
	AM_RANGE(0x12, 0x12) AM_DEVREADWRITE("usart1", i8251_device, data_r, data_w)
	AM_RANGE(0x13, 0x13) AM_DEVREADWRITE("usart1", i8251_device, status_r, control_w)
	AM_RANGE(0x14, 0x14) AM_DEVREADWRITE("usart2", i8251_device, data_r, data_w)
	AM_RANGE(0x15, 0x15) AM_DEVREADWRITE("usart2", i8251_device, status_r, control_w)
	AM_RANGE(0x16, 0x16) AM_WRITE(brg_w<1>)
	AM_RANGE(0x20, 0x20) AM_READ(earom_r)
	AM_RANGE(0x30, 0x30) AM_WRITE(port30_w)
	AM_RANGE(0x40, 0x40) AM_READWRITE(keyboard_r, key_row_w)
	AM_RANGE(0x48, 0x48) AM_WRITE(port48_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(picu_w)
	AM_RANGE(0x70, 0x73) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END


static INPUT_PORTS_START( v100 )
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Set-Up") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line Feed") PORT_CHAR(0x0a)

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8) 
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x09) PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("ROW8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x08) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("ROW9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("ROW10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END


MACHINE_CONFIG_START(v100_state::v100)
	MCFG_CPU_ADD("maincpu", Z80, XTAL(47'736'000) / 12) // divider not verified
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(v100_state, irq_ack)

	MCFG_DEVICE_ADD("usart1", I8251, XTAL(47'736'000) / 12) // divider not verified

	MCFG_DEVICE_ADD("brg1", COM8116, 5068800) // TODO: clock and divisors for this customized variant
	MCFG_COM8116_FR_HANDLER(DEVWRITELINE("usart1", i8251_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE("usart1", i8251_device, write_txc))

	MCFG_DEVICE_ADD("usart2", I8251, XTAL(47'736'000) / 12)

	MCFG_DEVICE_ADD("brg2", COM8116, 5068800)
	MCFG_COM8116_FR_HANDLER(DEVWRITELINE("usart2", i8251_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE("usart2", i8251_device, write_txc))

	MCFG_SCREEN_ADD("screen", RASTER)
	//MCFG_SCREEN_RAW_PARAMS(XTAL(47'736'000) / 2, 102 * CHAR_WIDTH, 0, 80 * CHAR_WIDTH, 260, 0, 240)
	MCFG_SCREEN_RAW_PARAMS(XTAL(47'736'000), 170 * CHAR_WIDTH, 0, 132 * CHAR_WIDTH, 312, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(v100_state, screen_update)

	// FIXME: dot clock should be divided by char width
	MCFG_DEVICE_ADD("vtac", CRT5037, XTAL(47'736'000))
	MCFG_TMS9927_CHAR_WIDTH(CHAR_WIDTH)
	MCFG_VIDEO_SET_SCREEN("screen")
	MCFG_TMS9927_HSYN_CALLBACK(WRITELINE(v100_state, picu_r_w<7>)) MCFG_DEVCB_INVERT
	MCFG_TMS9927_VSYN_CALLBACK(WRITELINE(v100_state, picu_r_w<6>)) MCFG_DEVCB_INVERT

	MCFG_DEVICE_ADD("picu", I8214, XTAL(47'736'000) / 12)
	MCFG_I8214_INT_CALLBACK(ASSERTLINE("maincpu", 0))

	MCFG_DEVICE_ADD("ppi", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(v100_state, ppi_porta_w))
	MCFG_I8255_OUT_PORTB_CB(DEVWRITELINE("earom", er1400_device, c3_w)) MCFG_DEVCB_BIT(6) MCFG_DEVCB_INVERT
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("earom", er1400_device, c2_w)) MCFG_DEVCB_BIT(5) MCFG_DEVCB_INVERT
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("earom", er1400_device, c1_w)) MCFG_DEVCB_BIT(4) MCFG_DEVCB_INVERT
	MCFG_I8255_OUT_PORTC_CB(DEVWRITELINE("earom", er1400_device, data_w)) MCFG_DEVCB_BIT(6) MCFG_DEVCB_INVERT
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("earom", er1400_device, clock_w)) MCFG_DEVCB_BIT(0) MCFG_DEVCB_INVERT

	MCFG_DEVICE_ADD("earom", ER1400, 0)
MACHINE_CONFIG_END



/**************************************************************************************************************

Visual 100. (VT-100 clone)
Chips: D780C-1 (Z80), CRT5037, D8255AC-5, uPB8214C, COM8116T-020, D8251AC, ER1400, 8-sw dip
Crystal: 47.736

***************************************************************************************************************/

ROM_START( v100 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "262-047.u108",  0x0000, 0x1000, CRC(e82f708c) SHA1(20ed83a41fd0703d72a20e170af971181cfbd575) )
	ROM_LOAD( "262-048.u110",  0x1000, 0x1000, CRC(830923d3) SHA1(108590234ff84b5856cc2784d738a2a625305953) )

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD( "241-001.u29",   0x0000, 0x0800, CRC(ef807141) SHA1(cbf3fed001811c5840b9a131d2d3133843cb3b6a) )
ROM_END

COMP( 1980, v100, 0, 0, v100, v100, v100_state, 0, "Visual Technology", "Visual 100", MACHINE_IS_SKELETON )
