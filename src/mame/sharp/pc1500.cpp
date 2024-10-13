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
#include "emupal.h"
#include "screen.h"

#include "pc1500.lh"


namespace {

class pc1500_state : public driver_device
{
public:
	pc1500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "upd1990a")
		, m_lcd_data(*this, "lcd_data")
		, m_keyboard(*this, "KEY.%u", 0)
		, m_io_on(*this, "ON")
		, m_busy(*this, "BUSY")
		, m_shift(*this, "SHIFT")
		, m_sml(*this, "SML")
		, m_small(*this, "SMALL")
		, m_iii(*this, "III")
		, m_ii(*this, "II")
		, m_i(*this, "I")
		, m_def(*this, "DEF")
		, m_de(*this, "DE")
		, m_g(*this, "G")
		, m_rad(*this, "RAD")
		, m_reserve(*this, "RESERVE")
		, m_pro(*this, "PRO")
		, m_run(*this, "RUN")
	{
	}

	void pc1500(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<lh5801_cpu_device> m_maincpu;
	required_device<upd1990a_device> m_rtc;

	required_shared_ptr<uint8_t> m_lcd_data;
	required_ioport_array<8> m_keyboard;
	required_ioport m_io_on;

	output_finder<> m_busy;
	output_finder<> m_shift;
	output_finder<> m_sml;
	output_finder<> m_small;
	output_finder<> m_iii;
	output_finder<> m_ii;
	output_finder<> m_i;
	output_finder<> m_def;
	output_finder<> m_de;
	output_finder<> m_g;
	output_finder<> m_rad;
	output_finder<> m_reserve;
	output_finder<> m_pro;
	output_finder<> m_run;

	uint8_t m_kb_matrix;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void kb_matrix_w(uint8_t data);
	uint8_t port_a_r();
	uint8_t port_b_r();
	void port_c_w(uint8_t data);

	uint8_t pc1500_kb_r();
	void pc1500_palette(palette_device &palette) const;
	void pc1500_mem(address_map &map) ATTR_COLD;
	void pc1500_mem_io(address_map &map) ATTR_COLD;
};

void pc1500_state::pc1500_mem(address_map &map)
{
	map.unmap_value_high();
	//  map(0x0000, 0x3fff).rom();    //module ROM/RAM
	map(0x4000, 0x47ff).ram();    //user RAM
	map(0x4800, 0x6fff).ram();    //expansion RAM
	map(0x7000, 0x71ff).ram().mirror(0x0600).share("lcd_data");
	map(0x7800, 0x7bff).ram().mirror(0x0400);
	//  map(0xa000, 0xbfff).rom();    //expansion ROM
	map(0xc000, 0xffff).rom().region("maincpu", 0);    //system ROM
}

void pc1500_state::pc1500_mem_io(address_map &map)
{
	map.unmap_value_high();
	map(0xf000, 0xf00f).rw("lh5810", FUNC(lh5810_device::data_r), FUNC(lh5810_device::data_w));
}

uint8_t pc1500_state::pc1500_kb_r()
{
	uint8_t data = 0xff;

	if (!started()) return 0;

	for (int i = 0; i < 8; i++)
	{
		if (!BIT(m_kb_matrix, i))
			data &= m_keyboard[i]->read();
	}

	return data;
}

uint32_t pc1500_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	for (int p=0; p<=1; p++)
		for (int a=0; a<0x4e; a++)
		{
			uint8_t data = m_lcd_data[a + (p<<8)];
			for (int b=0; b<8; b++)
			{
				if(b<4)
					bitmap.pix(b + 4 * (BIT( a, 0)), (a>>1) + 0x00 + 0x27*p) = BIT(data, b);
				else
					bitmap.pix(b - 4 * (BIT(~a, 0)), (a>>1) + 0x4e + 0x27*p) = BIT(data, b);
			}
		}

	m_busy =  BIT(m_lcd_data[0x4e], 0);
	m_shift = BIT(m_lcd_data[0x4e], 1);
	m_sml =   BIT(m_lcd_data[0x4e], 2);
	m_small = BIT(m_lcd_data[0x4e], 3);
	m_iii =   BIT(m_lcd_data[0x4e], 4);
	m_ii =    BIT(m_lcd_data[0x4e], 5);
	m_i =     BIT(m_lcd_data[0x4e], 6);
	m_def =   BIT(m_lcd_data[0x4e], 7);
	m_de =    BIT(m_lcd_data[0x4f], 0);
	m_g =     BIT(m_lcd_data[0x4f], 1);
	m_rad =   BIT(m_lcd_data[0x4f], 2);
	m_reserve = BIT(m_lcd_data[0x4f], 4);
	m_pro =   BIT(m_lcd_data[0x4f], 5);
	m_run =   BIT(m_lcd_data[0x4f], 6);

	return 0;
}

void pc1500_state::machine_start()
{
	m_busy.resolve();
	m_shift.resolve();
	m_sml.resolve();
	m_small.resolve();
	m_iii.resolve();
	m_ii.resolve();
	m_i.resolve();
	m_def.resolve();
	m_de.resolve();
	m_g.resolve();
	m_rad.resolve();
	m_reserve.resolve();
	m_pro.resolve();
	m_run.resolve();
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


void pc1500_state::kb_matrix_w(uint8_t data)
{
	m_kb_matrix = data;
}

void pc1500_state::port_c_w(uint8_t data)
{
	m_rtc->data_in_w(BIT(data, 0));
	m_rtc->stb_w(BIT(data, 1));
	m_rtc->clk_w(BIT(data, 2));
	m_rtc->oe_w(BIT(data, 3));
	m_rtc->c0_w(BIT(data, 3));
	m_rtc->c1_w(BIT(data, 4));
	m_rtc->c2_w(BIT(data, 5));
}

uint8_t pc1500_state::port_b_r()
{
	/*
	x--- ---- ON/Break key
	-xx- ---- upd1990ac
	---x ---- GND
	---- x--- Japan: GND, export: VCC
	---- -x-- cassette in
	---- --xx connector
	*/
	uint8_t data = 0;

	data |= 0x08;

	data |= (m_rtc->tp_r()<<5);
	data |= (m_rtc->data_out_r()<<6);
	data |= (m_io_on->read()<<7);

	return data;
}

uint8_t pc1500_state::port_a_r()
{
	return 0xff;
}

void pc1500_state::pc1500_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void pc1500_state::pc1500(machine_config &config)
{
	LH5801(config, m_maincpu, 2.6_MHz_XTAL); // 1.3 MHz internally
	m_maincpu->set_addrmap(AS_PROGRAM, &pc1500_state::pc1500_mem);
	m_maincpu->set_addrmap(AS_IO, &pc1500_state::pc1500_mem_io);
	m_maincpu->in_func().set(FUNC(pc1500_state::pc1500_kb_r));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));  // not accurate
	screen.set_screen_update(FUNC(pc1500_state::screen_update));
	screen.set_size(156, 8);
	screen.set_visarea(0, 156-1, 0, 7-1);
	screen.set_palette("palette");

	config.set_default_layout(layout_pc1500);
	PALETTE(config, "palette", FUNC(pc1500_state::pc1500_palette), 2);

	lh5810_device &ioports(LH5810(config, "lh5810"));
	ioports.porta_r().set(FUNC(pc1500_state::port_a_r));
	ioports.porta_w().set(FUNC(pc1500_state::kb_matrix_w));
	ioports.portb_r().set(FUNC(pc1500_state::port_b_r));
	ioports.portc_w().set(FUNC(pc1500_state::port_c_w));
	ioports.out_int().set_inputline("maincpu", LH5801_LINE_MI);

	UPD1990A(config, m_rtc, 32.768_kHz_XTAL);
}


ROM_START( pc1500 )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sys1500.rom", 0x0000, 0x4000, CRC(d480b50d) SHA1(4bf748ba4d7c2b7cd7da7f3fdefcdd2e4cd41c4e))
	ROM_REGION( 0x2000, "ce150", ROMREGION_ERASEFF )
	ROM_LOAD( "ce-150.rom", 0x0000, 0x2000, CRC(8fa1df6d) SHA1(a3aa02a641a46c27c0d4c0dc025b0dbe9b5b79c8))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   CLASS         INIT        COMPANY  FULLNAME                FLAGS
COMP( 198?, pc1500, 0,      0,      pc1500, pc1500, pc1500_state, empty_init, "Sharp", "Pocket Computer 1500", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
