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
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/com8116.h"
#include "machine/er1400.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "video/tms9927.h"
#include "screen.h"


namespace {

// character matrix is supposed to be only 7x7, but 15 produces correct timings
#define V100_CH_WIDTH 15

class v100_state : public driver_device
{
public:
	v100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_vtac(*this, "vtac")
		, m_usart(*this, "usart%u", 1)
		, m_earom(*this, "earom")
		, m_picu(*this, "picu")
		, m_modem(*this, "modem")
		, m_p_chargen(*this, "chargen")
		, m_videoram(*this, "videoram")
		, m_key_row(*this, "ROW%u", 0)
	{ }

	void v100(machine_config &config);

private:
	u8 status_r();
	void port30_w(u8 data);
	u8 keyboard_r();
	void key_row_w(u8 data);
	void port48_w(u8 data);
	void picu_w(u8 data);
	template<int N> void picu_r_w(int state);
	IRQ_CALLBACK_MEMBER(irq_ack);
	void ppi_porta_w(u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<crt5037_device> m_vtac;
	required_device_array<i8251_device, 2> m_usart;
	required_device<er1400_device> m_earom;
	required_device<i8214_device> m_picu;
	required_device<rs232_port_device> m_modem;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_videoram;
	optional_ioport_array<16> m_key_row;

	u8 m_active_row = 0;
	bool m_video_enable = false;
};

u32 v100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_vtac->screen_reset() || !m_video_enable)
	{
		bitmap.fill(rgb_t::black(), cliprect);
		return 0;
	}

	unsigned row0 = cliprect.top() / 10;
	unsigned x0 = cliprect.left();
	unsigned px0 = x0 % V100_CH_WIDTH;
	unsigned columns = screen.visible_area().width() / V100_CH_WIDTH;

	u16 start = 0;
	unsigned y = 0;
	for (unsigned row = 0; y <= cliprect.bottom(); row++)
	{
		start = m_videoram[start] | (m_videoram[(start + 1) & 0xfff] << 8);
		u16 end = start + ((start & 0x3000) != 0 ? (columns / 2) + 1 : columns);
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
					if (px >= V100_CH_WIDTH)
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
	m_usart[1]->write_cts(0);

	m_active_row = 0;
	m_video_enable = false;
	save_item(NAME(m_active_row));
	save_item(NAME(m_video_enable));
}

u8 v100_state::status_r()
{
	u8 status = 0xc0;
	status |= m_earom->data_r();
	status |= m_modem->dcd_r() << 1;
	status |= m_modem->si_r() << 2; // SCCD (pin 12)
	status |= m_modem->ri_r() << 3;
	return status;
}

void v100_state::port30_w(u8 data)
{
	// D6 = cursor/text blinking?

	//logerror("Writing %02X to port 30\n", data);
}

u8 v100_state::keyboard_r()
{
	return m_key_row[m_active_row & 15].read_safe(0xff);
}

void v100_state::key_row_w(u8 data)
{
	m_active_row = data;
}

void v100_state::port48_w(u8 data)
{
	//logerror("Writing %02X to port 48\n", data);
}

void v100_state::picu_w(u8 data)
{
	m_picu->b_w((data & 0x0e) >> 1);
	m_picu->sgs_w(BIT(data, 4));
}

template<int N>
void v100_state::picu_r_w(int state)
{
	m_picu->r_w(N, state);
}

IRQ_CALLBACK_MEMBER(v100_state::irq_ack)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return (m_picu->a_r() << 1) | 0xf0;
}

void v100_state::ppi_porta_w(u8 data)
{
	m_vtac->set_clock_scale(BIT(data, 5) ? 0.5 : 1.0);
	m_screen->set_clock_scale(BIT(data, 5) ? 0.5 : 1.0);

	m_video_enable = !BIT(data, 7);

	//logerror("Writing %02X to PPI port A\n", data);
}

void v100_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x4000, 0x4fff).ram().share("videoram");
	map(0x5000, 0x5fff).ram();
}

void v100_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).w(m_vtac, FUNC(crt5037_device::write));
	map(0x10, 0x10).w("brg1", FUNC(com8116_device::stt_str_w));
	map(0x12, 0x13).rw("usart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x14, 0x15).rw("usart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x16, 0x16).w("brg2", FUNC(com8116_device::stt_str_w));
	map(0x20, 0x20).r(FUNC(v100_state::status_r));
	map(0x30, 0x30).w(FUNC(v100_state::port30_w));
	map(0x40, 0x40).rw(FUNC(v100_state::keyboard_r), FUNC(v100_state::key_row_w));
	map(0x48, 0x48).w(FUNC(v100_state::port48_w));
	map(0x60, 0x60).w(FUNC(v100_state::picu_w));
	map(0x70, 0x73).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


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


void v100_state::v100(machine_config &config)
{
	Z80(config, m_maincpu, 47.736_MHz_XTAL / 20); // 2.387 MHz PCLOCK
	m_maincpu->set_addrmap(AS_PROGRAM, &v100_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &v100_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(v100_state::irq_ack));

	I8251(config, m_usart[0], 47.736_MHz_XTAL / 20);
	m_usart[0]->txd_handler().set(m_modem, FUNC(rs232_port_device::write_txd));
	m_usart[0]->dtr_handler().set(m_modem, FUNC(rs232_port_device::write_dtr));
	m_usart[0]->rts_handler().set(m_modem, FUNC(rs232_port_device::write_rts));
	//m_usart[0]->rxrdy_handler().set(FUNC(v100_state::picu_r_w<4>)).invert();

	input_merger_device &acts(INPUT_MERGER_ALL_HIGH(config, "acts"));
	acts.output_handler().set(m_usart[0], FUNC(i8251_device::write_cts));

	com8116_device &brg1(COM8116_020(config, "brg1", 1.8432_MHz_XTAL));
	brg1.fr_handler().set(m_usart[0], FUNC(i8251_device::write_rxc));
	brg1.ft_handler().set(m_usart[0], FUNC(i8251_device::write_txc));

	I8251(config, m_usart[1], 47.736_MHz_XTAL / 20);
	m_usart[1]->txd_handler().set("aux", FUNC(rs232_port_device::write_txd));
	m_usart[1]->dtr_handler().set("aux", FUNC(rs232_port_device::write_dtr));
	//m_usart[1]->txrdy_handler().set(FUNC(v100_state::picu_r_w<2>)).invert();

	com8116_device &brg2(COM8116_020(config, "brg2", 1.8432_MHz_XTAL));
	brg2.fr_handler().set(m_usart[1], FUNC(i8251_device::write_rxc));
	brg2.ft_handler().set(m_usart[1], FUNC(i8251_device::write_txc));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	//m_screen->set_raw(47.736_MHz_XTAL / 2, 102 * V100_CH_WIDTH, 0, 80 * V100_CH_WIDTH, 260, 0, 240);
	m_screen->set_raw(47.736_MHz_XTAL, 170 * V100_CH_WIDTH, 0, 132 * V100_CH_WIDTH, 312, 0, 240);
	m_screen->set_screen_update(FUNC(v100_state::screen_update));

	CRT5037(config, m_vtac, 47.736_MHz_XTAL / V100_CH_WIDTH);
	m_vtac->set_char_width(V100_CH_WIDTH);
	m_vtac->set_screen("screen");
	m_vtac->hsyn_callback().set(FUNC(v100_state::picu_r_w<7>)).invert();
	m_vtac->vsyn_callback().set(FUNC(v100_state::picu_r_w<6>)).invert();

	I8214(config, m_picu, 47.736_MHz_XTAL / 20);
	m_picu->int_wr_callback().set_inputline(m_maincpu, 0, ASSERT_LINE);

	i8255_device &ppi(I8255(config, "ppi", 0));
	ppi.out_pa_callback().set(FUNC(v100_state::ppi_porta_w));
	ppi.out_pb_callback().set(m_earom, FUNC(er1400_device::c3_w)).bit(6).invert();
	ppi.out_pb_callback().append(m_earom, FUNC(er1400_device::c2_w)).bit(5).invert();
	ppi.out_pb_callback().append(m_earom, FUNC(er1400_device::c1_w)).bit(4).invert();
	ppi.out_pc_callback().set(m_earom, FUNC(er1400_device::data_w)).bit(6).invert();
	ppi.out_pc_callback().append(m_earom, FUNC(er1400_device::clock_w)).bit(0).invert();
	ppi.out_pc_callback().append(m_modem, FUNC(rs232_port_device::write_spds)).bit(4);
	ppi.out_pc_callback().append("acts", FUNC(input_merger_device::in_w<1>)).bit(7);

	ER1400(config, m_earom);

	RS232_PORT(config, m_modem, default_rs232_devices, "loopback"); // EIA port
	m_modem->rxd_handler().set(m_usart[0], FUNC(i8251_device::write_rxd));
	m_modem->cts_handler().set("acts", FUNC(input_merger_device::in_w<0>));
	m_modem->dcd_handler().set(m_usart[0], FUNC(i8251_device::write_dsr));

	rs232_port_device &aux(RS232_PORT(config, "aux", default_rs232_devices, nullptr)); // optional printer port
	aux.rxd_handler().set(m_usart[1], FUNC(i8251_device::write_rxd));
	aux.dcd_handler().set(m_usart[1], FUNC(i8251_device::write_dsr)); // printer busy
}



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

} // anonymous namespace


COMP( 1980, v100, 0, 0, v100, v100, v100_state, empty_init, "Visual Technology", "Visual 100", MACHINE_IS_SKELETON )
