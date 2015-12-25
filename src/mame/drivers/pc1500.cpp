// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Sharp pocket computers 1500

        Driver by Sandro Ronco

        More info:
            http://www.pc1500.com/

****************************************************************************/


#include "emu.h"
#include "cpu/lh5801/lh5801.h"
#include "machine/lh5810.h"
#include "machine/upd1990a.h"
#include "pc1500.lh"


class pc1500_state : public driver_device
{
public:
	pc1500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_rtc(*this, "upd1990a"),
			m_lcd_data(*this, "lcd_data"),
			m_keyboard(*this, "KEY"),
			m_io_on(*this, "ON") { }

	required_device<cpu_device> m_maincpu;
	required_device<upd1990a_device> m_rtc;

	required_shared_ptr<UINT8> m_lcd_data;
	required_ioport_array<8> m_keyboard;
	required_ioport m_io_on;

	UINT8 m_kb_matrix;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void machine_reset() override;

	DECLARE_WRITE8_MEMBER( kb_matrix_w );
	DECLARE_READ8_MEMBER( port_a_r );
	DECLARE_READ8_MEMBER( port_b_r );
	DECLARE_WRITE8_MEMBER( port_c_w );

	DECLARE_READ8_MEMBER( pc1500_kb_r );
	DECLARE_PALETTE_INIT(pc1500);
};

static ADDRESS_MAP_START( pc1500_mem , AS_PROGRAM, 8, pc1500_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x3fff) AM_ROM    //module ROM/RAM
	AM_RANGE( 0x4000, 0x47ff) AM_RAM    //user RAM
	AM_RANGE( 0x4800, 0x6fff) AM_RAM    //expansion RAM
	AM_RANGE( 0x7000, 0x71ff) AM_RAM    AM_MIRROR(0x0600)   AM_SHARE("lcd_data")
	AM_RANGE( 0x7800, 0x7bff) AM_RAM    AM_REGION("maincpu", 0x7800)    AM_MIRROR(0x0400)
	AM_RANGE( 0xa000, 0xbfff) AM_ROM    //expansion ROM
	AM_RANGE( 0xc000, 0xffff) AM_ROM    //system ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc1500_mem_io , AS_IO, 8, pc1500_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0xf000, 0xf00f) AM_DEVREADWRITE("lh5810", lh5810_device, data_r, data_w)
ADDRESS_MAP_END

READ8_MEMBER( pc1500_state::pc1500_kb_r )
{
	UINT8 data = 0xff;

	if (!started()) return 0;

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(m_kb_matrix, i))
			data &= m_keyboard[i]->read();
	}

	return data;
}

UINT32 pc1500_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for (int p=0; p<=1; p++)
		for (int a=0; a<0x4e; a++)
		{
			UINT8 data = m_lcd_data[a + (p<<8)];
			for (int b=0; b<8; b++)
			{
				if(b<4)
					bitmap.pix16(b + 4 * (BIT( a, 0)), (a>>1) + 0x00 + 0x27*p) = BIT(data, b);
				else
					bitmap.pix16(b - 4 * (BIT(~a, 0)), (a>>1) + 0x4e + 0x27*p) = BIT(data, b);
			}
		}

	output_set_value("BUSY",  BIT(m_lcd_data[0x4e], 0));
	output_set_value("SHIFT", BIT(m_lcd_data[0x4e], 1));
	output_set_value("SML",   BIT(m_lcd_data[0x4e], 2));
	output_set_value("SMALL", BIT(m_lcd_data[0x4e], 3));
	output_set_value("III",   BIT(m_lcd_data[0x4e], 4));
	output_set_value("II",    BIT(m_lcd_data[0x4e], 5));
	output_set_value("I",     BIT(m_lcd_data[0x4e], 6));
	output_set_value("DEF",   BIT(m_lcd_data[0x4e], 7));
	output_set_value("DE",    BIT(m_lcd_data[0x4f], 0));
	output_set_value("G",     BIT(m_lcd_data[0x4f], 1));
	output_set_value("RAD",   BIT(m_lcd_data[0x4f], 2));
	output_set_value("RESERVE", BIT(m_lcd_data[0x4f], 4));
	output_set_value("PRO",   BIT(m_lcd_data[0x4f], 5));
	output_set_value("RUN",   BIT(m_lcd_data[0x4f], 6));

	return 0;
}

void pc1500_state::machine_reset()
{
	m_kb_matrix = 0xff;
	m_rtc->cs_w(1);
}

static INPUT_PORTS_START( pc1500 )

	PORT_START("ON")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)

	PORT_START("KEY.0")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP)

	PORT_START("KEY.1")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("OFF") PORT_CODE(KEYCODE_F11)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ST") PORT_CODE(KEYCODE_F8)

	PORT_START("KEY.2")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)

	PORT_START("KEY.3")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(")") PORT_CODE(KEYCODE_CLOSEBRACE)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(") PORT_CODE(KEYCODE_OPENBRACE)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)

	PORT_START("KEY.4")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH_PAD)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RCL") PORT_CODE(KEYCODE_F7)

	PORT_START("KEY.5")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE)

	PORT_START("KEY.6")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MODE") PORT_CODE(KEYCODE_HOME)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEF") PORT_CODE(KEYCODE_PGUP)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SML") PORT_CODE(KEYCODE_PGDN)

	PORT_START("KEY.7")
		PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
		PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN)
INPUT_PORTS_END


WRITE8_MEMBER( pc1500_state::kb_matrix_w )
{
	m_kb_matrix = data;
}

WRITE8_MEMBER( pc1500_state::port_c_w )
{
	m_rtc->data_in_w(BIT(data, 0));
	m_rtc->stb_w(BIT(data, 1));
	m_rtc->clk_w(BIT(data, 2));
	m_rtc->oe_w(BIT(data, 3));
	m_rtc->c0_w(BIT(data, 3));
	m_rtc->c1_w(BIT(data, 4));
	m_rtc->c2_w(BIT(data, 5));
}

READ8_MEMBER( pc1500_state::port_b_r )
{
	/*
	x--- ---- ON/Break key
	-xx- ---- upd1990ac
	---x ---- GND
	---- x--- Japan: GND, export: VCC
	---- -x-- cassette in
	---- --xx connector
	*/
	UINT8 data = 0;

	data |= 0x08;

	data |= (m_rtc->tp_r()<<5);
	data |= (m_rtc->data_out_r()<<6);
	data |= (m_io_on->read()<<7);

	return data;
}

READ8_MEMBER( pc1500_state::port_a_r )
{
	return 0xff;
}

PALETTE_INIT_MEMBER(pc1500_state, pc1500)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

static MACHINE_CONFIG_START( pc1500, pc1500_state )
	MCFG_CPU_ADD("maincpu", LH5801, 1300000)            //1.3 MHz
	MCFG_CPU_PROGRAM_MAP( pc1500_mem )
	MCFG_CPU_IO_MAP( pc1500_mem_io )
	MCFG_LH5801_IN(READ8(pc1500_state,pc1500_kb_r))

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))  // not accurate
	MCFG_SCREEN_UPDATE_DRIVER(pc1500_state, screen_update)
	MCFG_SCREEN_SIZE(156, 8)
	MCFG_SCREEN_VISIBLE_AREA(0, 156-1, 0, 7-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_pc1500)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(pc1500_state, pc1500)

	MCFG_DEVICE_ADD("lh5810", LH5810, 0)
	MCFG_LH5810_PORTA_R_CB(READ8(pc1500_state, port_a_r))
	MCFG_LH5810_PORTA_W_CB(WRITE8(pc1500_state, kb_matrix_w))
	MCFG_LH5810_PORTB_R_CB(READ8(pc1500_state, port_b_r))
	MCFG_LH5810_PORTC_W_CB(WRITE8(pc1500_state, port_c_w))
	MCFG_LH5810_OUT_INT_CB(INPUTLINE("maincpu", LH5801_LINE_MI))

	MCFG_UPD1990A_ADD("upd1990a", XTAL_32_768kHz, NULL, NULL)
MACHINE_CONFIG_END


ROM_START( pc1500 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sys1500.rom", 0xc000, 0x4000, CRC(d480b50d) SHA1(4bf748ba4d7c2b7cd7da7f3fdefcdd2e4cd41c4e))
	ROM_REGION( 0x10000, "ce150", ROMREGION_ERASEFF )
	ROM_LOAD( "ce-150.rom", 0x0000, 0x2000, CRC(8fa1df6d) SHA1(a3aa02a641a46c27c0d4c0dc025b0dbe9b5b79c8))
ROM_END

/*    YEAR  NAME    PARENT  COMPAT   MACHINE INPUT   INIT         COMPANY     FULLNAME */
COMP( 198?, pc1500, 0,  0,  pc1500, pc1500, driver_device,  0,   "Sharp", "Pocket Computer 1500", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
