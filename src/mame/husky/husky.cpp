// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
// thanks-to:Phill Harvey-Smith
/***************************************************************************

    DVW Husky

    Known RAM configurations:
    Husky - 32K, 48K, 96K (manual suggests 16K, 32K, 48K, 64K, 80K, 96K, 112K, 128K, 144K)

    Usage:
    On first use of BASIC enter NEW to initialise RAM.

    TODO:
    - implement power button
    - test rs232 port
    - Break key doesn't work?

****************************************************************************/

#include "emu.h"
#include "cpu/z80/nsc800.h"
#include "machine/mm58274c.h"
#include "machine/nsc810.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "sound/beep.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class husky_state : public driver_device
{
public:
	husky_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kbd(*this, "X%u", 0)
		, m_rs232(*this, "serial")
		, m_rom(*this, "rom")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_rombank0(*this, "rombank0")
		, m_rombank1(*this, "rombank1")
		, m_rombank2(*this, "rombank2")
		, m_rombank3(*this, "rombank3")
		, m_rambank0(*this, "rambank0")
		, m_rambank1(*this, "rambank1")
		, m_rambank2(*this, "rambank2")
		, m_beeper(*this, "beeper")
		, m_palette(*this, "palette")
		, m_battery(*this, "BATTERY")
	{ }

	void husky(machine_config &config);

	void init_husky();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t porta_r();
	void porta_w(uint8_t data);
	uint8_t portb_r();
	uint8_t portc_r();
	void portc_w(uint8_t data);
	void serial_tx_w(uint8_t data);
	void cursor_w(uint8_t data);
	void curinh_w(uint8_t data);
	void irqctrl_w(uint8_t data);
	void page_w(offs_t offset, uint8_t data);
	void husky_palette(palette_device &palette) const;

	void timer0_out(int state);
	void timer1_out(int state);
	void cts_w(int state);
	void rxd_w(int state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void husky_io(address_map &map) ATTR_COLD;
	void husky_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_kbd;
	required_device<rs232_port_device> m_rs232;
	required_memory_region m_rom;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_memory_bank m_rombank0;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_memory_bank m_rombank3;
	required_memory_bank m_rambank0;
	required_memory_bank m_rambank1;
	required_memory_bank m_rambank2;
	required_device<beep_device> m_beeper;
	required_device<palette_device> m_palette;
	required_ioport m_battery;

	uint8_t m_cursor;
	uint8_t m_curinh;
	uint16_t m_keydata;
	uint8_t m_irq_mask;
	uint8_t m_page;
};

void husky_state::husky_mem(address_map &map)
{
	map(0x0000, 0x0fff).bankr("rombank0");
	map(0x1000, 0x1fff).bankr("rombank1");
	map(0x2000, 0x2fff).bankr("rombank2");
	map(0x3000, 0x3fff).bankr("rombank3");
	map(0x4000, 0x7fff).bankrw("rambank0");
	map(0x8000, 0xbfff).bankrw("rambank1");
	map(0xc000, 0xffff).bankrw("rambank2");
}

void husky_state::husky_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x1f).rw("iotimer", FUNC(nsc810_device::read), FUNC(nsc810_device::write));
	map(0x20, 0x20).mirror(0x1f).w(FUNC(husky_state::cursor_w));
	map(0x40, 0x4f).rw("rtc", FUNC(mm58274c_device::read), FUNC(mm58274c_device::write));
	map(0x80, 0x80).mirror(0x18).noprw(); /* POWSAV - Power save control */
	map(0x81, 0x81).mirror(0x18).w(FUNC(husky_state::serial_tx_w));
	map(0x82, 0x82).mirror(0x18).w(FUNC(husky_state::curinh_w));
	map(0x83, 0x83).mirror(0x18).nopw(); /* POWHLD - Power control */
	map(0x84, 0x84).mirror(0x18).nopw(); /* INVCON - V24 inverter control */
	map(0x85, 0x87).mirror(0x18).w(FUNC(husky_state::page_w));
	map(0xbb, 0xbb).w(FUNC(husky_state::irqctrl_w));
	map(0xc0, 0xc0).noprw(); /* PPORT0 - Parallel port 0 */
	map(0xc1, 0xc1).noprw(); /* PPORT1 - Parallel port 1 */
	map(0xc2, 0xc2).noprw(); /* ADIN - Analogue port */
}


static INPUT_PORTS_START(husky)
	PORT_START("X0")
	PORT_BIT(0x000f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('!')
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('"')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR(';')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('[')
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR(']')
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('\\')
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('/')
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('(')
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR(')')
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_NAME("P Break")

	PORT_START("X1")
	PORT_BIT(0x000f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('?')
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('>')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR(':')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('\'')
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('`')
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('#')
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('*')
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('=')
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR(27) PORT_NAME("L Esc")
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Power on/off")

	PORT_START("X2")
	PORT_BIT(0x000f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('<')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('$')
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('%')
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_NAME("V Insert")
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('-')
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME("N Left")
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME("M Right")
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)

	PORT_START("X3")
	PORT_BIT(0x000f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_NAME("1 Help")
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR(',')
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('&')
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('@')
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_NAME("5 Control")
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('.')
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('+')
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME("8 Down")
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_NAME("9 Up")
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_NAME("0 Del")

	PORT_START("BATTERY")
	PORT_CONFNAME(0x04, 0x04, "Battery")
	PORT_CONFSETTING(0x00, "Low")
	PORT_CONFSETTING(0x04, "OK")
INPUT_PORTS_END


uint8_t husky_state::porta_r()
{
	uint8_t data = 0xff;

	if (!BIT(m_keydata, 0)) data &= m_kbd[0]->read() & 0x00ff;
	if (!BIT(m_keydata, 1)) data &= m_kbd[1]->read() & 0x00ff;
	if (!BIT(m_keydata, 2)) data &= m_kbd[2]->read() & 0x00ff;
	if (!BIT(m_keydata, 3)) data &= m_kbd[3]->read() & 0x00ff;

	return data;
}

void husky_state::porta_w(uint8_t data)
{
	m_keydata = data;
}

uint8_t husky_state::portb_r()
{
	uint8_t data = 0xff;

	if (!BIT(m_keydata, 0)) data &= (m_kbd[0]->read() & 0xff00) >> 8;
	if (!BIT(m_keydata, 1)) data &= (m_kbd[1]->read() & 0xff00) >> 8;
	if (!BIT(m_keydata, 2)) data &= (m_kbd[2]->read() & 0xff00) >> 8;
	if (!BIT(m_keydata, 3)) data &= (m_kbd[3]->read() & 0xff00) >> 8;

	return data;
}

/*
Bit 2 = Power low warning (1 = Power OK, 0 = Power low)
Input bits 7,6,4,3,2
*/
uint8_t husky_state::portc_r()
{
	uint8_t data = 0x23;

	data |= m_battery->read();
	data |= m_rs232->cts_r() << 3;

	return data;
}

/*
Output bits 5,1,0
*/
void husky_state::portc_w(uint8_t data)
{
	m_beeper->set_state(!BIT(data, 0));
	m_rs232->write_rts(BIT(data, 1));
}

/*
CURSOR - Output with a number 0-7F for the cursor position on the display
*/
void husky_state::cursor_w(uint8_t data)
{
	m_cursor = data & 0x7f;
}

/*
CURINH - Inhibit cursor. BIT0 = 1 : Cursor off, BIT0 = 0 : Cursor on
*/
void husky_state::curinh_w(uint8_t data)
{
	m_curinh = BIT(data, 0);
}

/*
V24OUT - Directly outputs to the V24 data line signal on bit 0.
The output is voltage inverted ie. 0 = +ve, 1 = -ve
*/
void husky_state::serial_tx_w(uint8_t data)
{
	m_rs232->write_txd(data & 0x01);
}

/*
ICRREG - NSC800 internal interrupt mask register
Bit 0 = Enable normal interrupts
Bit 1 = Enable RSTC interrupts
Bit 2 = Enable RSTB interrupts
Bit 3 = Enable RSTA interrupts
*/
void husky_state::irqctrl_w(uint8_t data)
{
	m_irq_mask = data;
	if(!(data & 0x08))
		m_maincpu->set_input_line(NSC800_RSTA, CLEAR_LINE);
	if(!(data & 0x04))
		m_maincpu->set_input_line(NSC800_RSTB, CLEAR_LINE);
	if(!(data & 0x02))
		m_maincpu->set_input_line(NSC800_RSTC, CLEAR_LINE);
}

/*
PAGA 16, 17, 18 - Memory paging addresses
*/
void husky_state::page_w(offs_t offset, uint8_t data)
{
	if (offset == 0x02)
	{
		logerror("PAGA18 not implemented!");
	}
	else
	{
		if (BIT(data, 0))
			m_page |= 1 << offset;
		else
			m_page &= ~(1 << offset);

		/* ROM banks */
		m_rombank0->set_entry(m_page & 0x03);
		m_rombank1->set_entry(m_page & 0x03);
		m_rombank2->set_entry(m_page & 0x03);
		m_rombank3->set_entry(m_page & 0x03);

		/* RAM banks */
		m_rambank1->set_entry(m_page & 0x03);
		m_rambank2->set_entry(m_page & 0x03);

		address_space &prog_space = m_maincpu->space(AS_PROGRAM);

		if (m_ram->size() > 0x4000 + (0x8000 * (m_page & 0x03)))
			prog_space.install_readwrite_bank(0x8000, 0xbfff, m_rambank1);
		else
			prog_space.nop_readwrite(0x8000, 0xbfff);

		if (m_ram->size() > 0x8000 + (0x8000 * (m_page & 0x03)))
			prog_space.install_readwrite_bank(0xc000, 0xffff, m_rambank2);
		else
			prog_space.nop_readwrite(0xc000, 0xffff);
	}
}

void husky_state::machine_start()
{
	/* configure ROM/RAM banks */
	m_rombank0->configure_entries(0, 4, m_rom->base() + 0x0000, 0x1000);
	m_rombank1->configure_entries(0, 4, m_rom->base() + 0x4000, 0x1000);
	m_rombank2->configure_entries(0, 4, m_rom->base() + 0x8000, 0x1000);
	m_rombank3->configure_entries(0, 4, m_rom->base() + 0xc000, 0x1000);
	m_rambank0->configure_entry(0, m_ram->pointer());
	m_rambank1->configure_entries(0, 4, m_ram->pointer() + 0x4000, 0x8000);
	m_rambank2->configure_entries(0, 4, m_ram->pointer() + 0x8000, 0x8000);

	/* register for save states */
	save_item(NAME(m_cursor));
	save_item(NAME(m_curinh));
	save_item(NAME(m_page));
}

void husky_state::machine_reset()
{
	m_keydata = 0xff;
	m_irq_mask = 0;
	m_page = 0x00;

	/* select ROM/RAM banks */
	m_rombank0->set_entry(0);
	m_rombank1->set_entry(0);
	m_rombank2->set_entry(0);
	m_rombank3->set_entry(0);
	m_rambank0->set_entry(0);
	m_rambank1->set_entry(0);
	m_rambank2->set_entry(0);
}

void husky_state::init_husky()
{
	m_nvram->set_base(m_ram->pointer(), m_ram->size());
}

uint32_t husky_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/*
	The LCD panel contains 4 rows of 32 characters, each character is a 5x7 dot-matrix,
	separated by a single dot space. Beneath each character is an empty line followed by
	a 5x1 dot-matrix used for the cursor. The character rows are separated by 3 (maybe 4)
	empty lines.
	Each character is stored as 8 bytes, only 7 are used. To include cursor and spacing
	I'm drawing each character as 6x12 to form an overall screen of 192x48.
	*/

	pen_t const *const pen = m_palette->pens();

	for (int y = 0; y < 48; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			uint8_t data;
			switch (y % 12)
			{
			case 0: case 1: case 2: case 3: case 4: case 5: case 6:
				/* character */
				data = m_ram->pointer()[(x * 8) + ((y / 12) * 32 * 8) + (y % 12)];

				for (int b = 0; b < 5; b++)
				{
					bitmap.pix(y, (x * 6) + b) = BIT(data, 0) ? pen[1] : pen[2];
					data >>= 1;
				}
				bitmap.pix(y, (x * 6) + 5) = pen[0];
				break;
			case 8:
				/* cursor */
				for (int b = 0; b < 5; b++)
				{
					bitmap.pix(y, (x * 6) + b) = (!m_curinh && m_cursor == (y / 12) * 32 + x) ? pen[1] : pen[2];
				}
				bitmap.pix(y, (x * 6) + 5) = pen[0];
				break;
			default:
				/* blank */
				for (int b = 0; b < 6; b++)
				{
					bitmap.pix(y, (x * 6) + b) = pen[0];
				}
				break;
			}
		}
	}
	return 0;
}

void husky_state::husky_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t(92, 83, 88));    // LCD pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // LCD pixel off
}

void husky_state::timer0_out(int state)
{
	if(state == ASSERT_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void husky_state::timer1_out(int state)
{
	if(BIT(m_irq_mask, 7))
		m_maincpu->set_input_line(NSC800_RSTA, state);
}

void husky_state::cts_w(int state)
{
	if(BIT(m_irq_mask, 1))
		m_maincpu->set_input_line(NSC800_RSTC, state);
}

void husky_state::rxd_w(int state)
{
	if(BIT(m_irq_mask, 2))
		m_maincpu->set_input_line(NSC800_RSTB, ASSERT_LINE);
}

void husky_state::husky(machine_config &config)
{
	/* basic machine hardware */
	NSC800(config, m_maincpu, 2_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &husky_state::husky_mem);
	m_maincpu->set_addrmap(AS_IO, &husky_state::husky_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(FUNC(husky_state::screen_update));
	screen.set_size(192, 48);
	screen.set_visarea(0, 191, 0, 47);
	PALETTE(config, m_palette, FUNC(husky_state::husky_palette), 3);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 3200).add_route(ALL_OUTPUTS, "mono", 1.0); // TODO: unknown frequency

	/* internal ram */
	RAM(config, m_ram).set_default_size("48K").set_extra_options("16K,32K,64K,80K,96K,112K,128K,144K");
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Devices */
	mm58274c_device &rtc(MM58274C(config, "rtc", 32.768_kHz_XTAL)); /* MM58174A */
	rtc.set_mode24(1); // 24 hour
	rtc.set_day1(1);   // monday

	nsc810_device &iotimer(NSC810(config, "iotimer", 0, 2_MHz_XTAL / 2, 2_MHz_XTAL / 2));
	iotimer.portA_read_callback().set(FUNC(husky_state::porta_r));
	iotimer.portA_write_callback().set(FUNC(husky_state::porta_w));
	iotimer.portB_read_callback().set(FUNC(husky_state::portb_r));
	iotimer.portC_read_callback().set(FUNC(husky_state::portc_r));
	iotimer.portC_write_callback().set(FUNC(husky_state::portc_w));
	iotimer.timer0_callback().set(FUNC(husky_state::timer0_out));
	iotimer.timer1_callback().set(FUNC(husky_state::timer1_out));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->cts_handler().set(FUNC(husky_state::cts_w));
	m_rs232->rxd_handler().set(FUNC(husky_state::rxd_w));

	/* optional ADC ICL7109CPL @ 6Mhz */
}


ROM_START( husky )
	/* EPROM's are any of 2732, 2764 or 27128 varieties */
	ROM_REGION(0x10000, "rom", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "hc19jul", "HC19JUL")
	ROMX_LOAD( "dvweprom0.ic27", 0x0000, 0x2000, CRC(4657cf89) SHA1(7f3d5a581a8f4a7ea6ef1eb93881f64dbb6af65b), ROM_BIOS(0) )
	ROM_RELOAD(                  0x2000, 0x2000)
	ROMX_LOAD( "dvweprom1.ic26", 0x4000, 0x2000, CRC(1cf2553f) SHA1(5a594820753a1389999d5c21dd7c9c01672d92f2), ROM_BIOS(0) )
	ROM_RELOAD(                  0x6000, 0x2000)
	ROMX_LOAD( "dvweprom2.ic25", 0x8000, 0x2000, CRC(6e91e6a5) SHA1(fef28f00767cb4d52e56fe7ee993fa92a2c77a1f), ROM_BIOS(0) )
	ROM_RELOAD(                  0xa000, 0x2000)
	ROMX_LOAD( "dvweprom3.ic24", 0xc000, 0x2000, CRC(595d36f4) SHA1(252d4cdff1ac19c4b1dc2d6dcf13f108d485bd4c), ROM_BIOS(0) )
	ROM_RELOAD(                  0xe000, 0x2000)
	ROM_SYSTEM_BIOS(1, "hs19jul", "HS19JUL")
	ROMX_LOAD( "hs-19-jul-0.ic27", 0x0000, 0x2000, CRC(c42ab5d5) SHA1(36dff5a25212c19fd8b88fe7288012ba6015fd31), ROM_BIOS(1) )
	ROM_RELOAD(                    0x2000, 0x2000)
	ROMX_LOAD( "hs-19-jul-1.ic26", 0x4000, 0x2000, CRC(b753891f) SHA1(fd29ea1d2e9de094d2974bbc0fe47c4f2e99e43f), ROM_BIOS(1) )
	ROM_RELOAD(                    0x6000, 0x2000)
	ROMX_LOAD( "hs-19-jul-2.ic25", 0x8000, 0x2000, CRC(a75a5eef) SHA1(26e79a1382fc051f84c46f6add223ffe46a22036), ROM_BIOS(1) )
	ROM_RELOAD(                    0xa000, 0x2000)
	ROMX_LOAD( "hs-19-jul-3.ic24", 0xc000, 0x2000, CRC(ea430f24) SHA1(dd445c8af2315e6fe02e7e7a4a0dcc0f22778b7c), ROM_BIOS(1) )
	ROM_RELOAD(                    0xe000, 0x2000)
	ROM_SYSTEM_BIOS(2, "hc03jun", "HC03JUN")
	ROMX_LOAD( "03-jun-00.ic27", 0x0000, 0x2000, CRC(e08096e5) SHA1(5c34da226a93b414fadadc7fa3fadaa2c11bb34a), ROM_BIOS(2) )
	ROM_RELOAD(                  0x2000, 0x2000)
	ROMX_LOAD( "03-jun-01.ic26", 0x4000, 0x2000, CRC(ff961693) SHA1(2f51bd07245118d03524684ceb63df97f11bffcb), ROM_BIOS(2) )
	ROM_RELOAD(                  0x6000, 0x2000)
	ROMX_LOAD( "03-jun-02.ic25", 0x8000, 0x2000, CRC(be6cf4d3) SHA1(3a7b49ec83e6612ac23149ea1f0ae4ff7a90a258), ROM_BIOS(2) )
	ROM_RELOAD(                  0xa000, 0x2000)
	ROMX_LOAD( "03-jun-03.ic24", 0xc000, 0x2000, CRC(8fd629d1) SHA1(ef2821c9ce1c1375326d80cad3bfc74e75548172), ROM_BIOS(2) )
	ROM_RELOAD(                  0xe000, 0x2000)
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY                 FULLNAME  FLAGS
COMP( 1981, husky, 0,      0,      husky,   husky, husky_state, init_husky, "DVW Microelectronics", "Husky",  0 )
