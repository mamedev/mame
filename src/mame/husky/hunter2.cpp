// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/***************************************************************************

    Husky Hunter, Hunter 2

    http://www.seasip.info/VintagePC/husky2.html

    The main hardware difference between the original Hunter and Hunter 2 is
    the LCD panel, in the Hunter 2 it is physically larger. It is also driven
    differently. The Hunter sets up the HD61830 number of time divisions to
    be 32 and uses both address counters RAC1/RAC2, whereas the Hunter 2 sets
    number of time divisions to 64 and only uses RAC1 to address the upper half
    of the screen.
    Early models would use all 6 ROM sockets to provide 48K ROM (6 x 2764),
    whereas later models would populate 2 sockets with 2 x 32K ROM (TC57256),
    though only 48K of this is ever paged in.

    Usage:
    The RAMdisk must first be formatted using 'format' (but you'll lose the inbuilt apps).
    The function keys labels at the bottom of the screen are accessed by Ctrl 1 through 8.

    TODO:
    - Add ports 83 and 85
    - Need software
    - Add whatever image devices existed
    - Probably lots of other stuff
    - DIR command can go crazy
    - Clock doesn't advance

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/nsc800.h"
#include "machine/mm58274c.h"
#include "machine/nsc810.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "sound/spkrdev.h"
#include "video/hd61830.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class hunter2_state : public driver_device
{
public:
	hunter2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_kbd(*this, "X%u", 0)
		, m_rs232(*this, "serial")
		, m_rom(*this, "rom")
		, m_ram(*this, RAM_TAG)
		, m_nvram(*this, "nvram")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_battery(*this, "BATTERY")
	{ }

	void hunter2(machine_config &config);
	void hunter(machine_config &config);

	void init_hunter2();

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t keyboard_r();
	uint8_t portb_r();
	void keyboard_w(uint8_t data);
	uint8_t portc_r();
	void display_ctrl_w(uint8_t data);
	void serial_tx_w(uint8_t data);
	void serial_dtr_w(uint8_t data);
	void serial_rts_w(uint8_t data);
	void speaker_w(uint8_t data);
	void irqctrl_w(uint8_t data);
	void memmap_w(uint8_t data);
	void hunter2_palette(palette_device &palette) const;

	void timer0_out(int state);
	void timer1_out(int state);
	void cts_w(int state);
	void rxd_w(int state);

	void hunter2_io(address_map &map) ATTR_COLD;
	void hunter2_mem(address_map &map) ATTR_COLD;

	uint8_t m_keydata = 0;
	uint8_t m_irq_mask = 0;
	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<7> m_kbd;
	required_device<rs232_port_device> m_rs232;
	required_memory_region m_rom;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_ioport m_battery;
};

void hunter2_state::hunter2_mem(address_map &map)
{
	map(0x0000, 0x7fff).bankr("bank1");
	map(0x8000, 0xbfff).bankr("bank2");
	map(0xc000, 0xffff).bankrw("bank3");
}

void hunter2_state::hunter2_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x1f).rw("iotimer", FUNC(nsc810_device::read), FUNC(nsc810_device::write));
	map(0x20, 0x20).w("lcdc", FUNC(hd61830_device::data_w));
	map(0x21, 0x21).rw("lcdc", FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w));
	map(0x3e, 0x3e).r("lcdc", FUNC(hd61830_device::data_r));
	map(0x40, 0x4f).mirror(0x10).rw("rtc", FUNC(mm58274c_device::read), FUNC(mm58274c_device::write));
	map(0x60, 0x60).mirror(0x1f).w(FUNC(hunter2_state::display_ctrl_w));
	map(0x80, 0x80).mirror(0x18).noprw(); /* POWSAV - Power save control */
	map(0x81, 0x81).mirror(0x18).w(FUNC(hunter2_state::serial_tx_w));
	map(0x82, 0x82).mirror(0x18).w(FUNC(hunter2_state::serial_rts_w));
	map(0x83, 0x83).mirror(0x18).noprw(); /* POWHLD - Power control */
	map(0x84, 0x84).mirror(0x18).noprw(); /* INVCON - V24 inverter control */
	map(0x85, 0x85).mirror(0x18).w(FUNC(hunter2_state::serial_dtr_w));
	map(0x86, 0x86).mirror(0x18).w(FUNC(hunter2_state::speaker_w));
	map(0x87, 0x87).mirror(0x18).noprw(); /* AUXPWR - Turns on the auxiliary power */
	map(0xbb, 0xbb).w(FUNC(hunter2_state::irqctrl_w));
	map(0xe0, 0xe0).w(FUNC(hunter2_state::memmap_w));
}


/* Input ports */
static INPUT_PORTS_START( hunter2 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc/Brk") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Lbl/Ins") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctl/Fn")  PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("BATTERY")
	PORT_CONFNAME(0x04, 0x00, "Battery")
	PORT_CONFSETTING(0x00, "OK")
	PORT_CONFSETTING(0x04, "Low")
INPUT_PORTS_END

uint8_t hunter2_state::keyboard_r()
{
	uint8_t data = 0xff;
	for (int i = 0; i < 7; i++)
	{
		if (!BIT(m_keydata, i))
		{
			data &= m_kbd[i]->read();
		}
	}
	return data;
}

uint8_t hunter2_state::portb_r()
{
	uint8_t res = 0x00;

	if(m_rs232->dsr_r())
		res |= 0x80;

	return res;
}

void hunter2_state::keyboard_w(uint8_t data)
{
	m_keydata = data;
}

/*
INPUTS - This port has a number of input functions:
Bit 0 = Data in (inverted from RS-232 line)
Bit 1 = DCD (inverted from RS-232 line)
Bit 2 = Power low warning (0 = Power OK, 1 = Power low)
Bit 3 = TXCLK (inverted from RS-232 line)
*/
uint8_t hunter2_state::portc_r()
{
	uint8_t res = 0x28;

	if(m_rs232->rxd_r())
		res |= 0x01;
	if(m_rs232->dcd_r())
		res |= 0x02;
	res |= m_battery->read();

	return res;
}

/*
ANGLE - Controls display viewing angle. Input value in the range 0-0x1F.
*/
void hunter2_state::display_ctrl_w(uint8_t data)
{
/* according to the website,
Bit 2: Backlight toggle
Bits 1,0,7,6: Contrast level.
00 is being written here
*/
}

/*
V24OUT - Directly outputs to the V24 data line signal on bit 0.
The output is voltage inverted ie. 0 = +ve, 1 = -ve
*/
void hunter2_state::serial_tx_w(uint8_t data)
{
	m_rs232->write_txd(data & 0x01);
}

/*
DTR output bit
*/
void hunter2_state::serial_dtr_w(uint8_t data)
{
	m_rs232->write_dtr(data & 0x01);
}

/*
RTS output bit
*/
void hunter2_state::serial_rts_w(uint8_t data)
{
	m_rs232->write_rts(data & 0x01);
}

void hunter2_state::speaker_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 0));
}

/*
ICRREG - NSC800 internal interrupt mask register
Bit 0 = Enable normal interrupts
Bit 1 = Enable RSTC interrupts
Bit 2 = Enable RSTB interrupts
Bit 3 = Enable RSTA interrupts
*/
void hunter2_state::irqctrl_w(uint8_t data)
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
PAGE - Memory paging register
*/
void hunter2_state::memmap_w(uint8_t data)
{
	address_space &prog_space = m_maincpu->space(AS_PROGRAM);
	uint8_t bank = data & 0x0f;

	switch (BIT(data, 7))
	{
	case 0: /* ROM */
		prog_space.install_read_bank(0x0000, 0x7fff, m_bank1);
		m_bank1->set_base(m_rom->base());
		/* select ROM bank only if present */
		if (m_rom->bytes() > (0x8000 + (0x4000 * bank)))
		{
			prog_space.install_read_bank(0x8000, 0xbfff, m_bank2);
			m_bank2->set_base(m_rom->base() + (0x8000 + (0x4000 * bank)));
		}
		else
		{
			prog_space.unmap_readwrite(0x8000, 0xbfff);
		}
		break;
	case 1: /* RAM */
		/* select RAM bank only if present */
		if (m_ram->size() > (0x4000 + (0xc000 * bank)))
		{
			prog_space.install_readwrite_bank(0x0000, 0xbfff, m_bank1);
			m_bank1->set_base(m_ram->pointer() + (0x4000 + (0xc000 * bank)));
		}
		else
		{
			prog_space.unmap_readwrite(0x0000, 0xbfff);
		}
		break;
	}
}

void hunter2_state::machine_reset()
{
	m_keydata = 0xff;
	m_irq_mask = 0;

	/* select ROM/RAM banks */
	m_bank1->set_base(m_rom->base());
	m_bank2->set_base(m_rom->base() + 0x4000);
	m_bank3->set_base(m_ram->pointer());
}

void hunter2_state::init_hunter2()
{
	m_nvram->set_base(m_ram->pointer(), m_ram->size());
}

void hunter2_state::hunter2_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void hunter2_state::timer0_out(int state)
{
	if(state == ASSERT_LINE)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void hunter2_state::timer1_out(int state)
{
	if(m_irq_mask & 0x08)
		m_maincpu->set_input_line(NSC800_RSTA, state);
}

void hunter2_state::cts_w(int state)
{
	if(BIT(m_irq_mask, 1))
	{
		m_maincpu->set_input_line(NSC800_RSTC, state);
		popmessage("CTS: RSTC set %i\n",state);
	}
}

void hunter2_state::rxd_w(int state)
{
	if(BIT(m_irq_mask, 2))
		m_maincpu->set_input_line(NSC800_RSTB, ASSERT_LINE);
}

void hunter2_state::hunter2(machine_config &config)
{
	/* basic machine hardware */
	NSC800(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hunter2_state::hunter2_mem);
	m_maincpu->set_addrmap(AS_IO, &hunter2_state::hunter2_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(80);
	screen.set_screen_update("lcdc", FUNC(hd61830_device::screen_update));
	screen.set_size(240, 128);
	screen.set_visarea(0, 239, 0, 63);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(hunter2_state::hunter2_palette), 2);
	hd61830_device &hd61830(HD61830(config, "lcdc", XTAL(4'915'200)/2/2)); // unknown clock
	hd61830.set_screen("screen");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Devices */
	mm58274c_device &rtc(MM58274C(config, "rtc", 32.768_kHz_XTAL));
	// this is all guess
	rtc.set_mode24(0); // 12 hour
	rtc.set_day1(1);   // monday

	nsc810_device &iotimer(NSC810(config, "iotimer", 0, 8_MHz_XTAL / 2, 8_MHz_XTAL / 2));
	iotimer.portA_read_callback().set(FUNC(hunter2_state::keyboard_r));
	iotimer.portB_read_callback().set(FUNC(hunter2_state::portb_r));
	iotimer.portB_write_callback().set(FUNC(hunter2_state::keyboard_w));
	iotimer.portC_read_callback().set(FUNC(hunter2_state::portc_r));
	iotimer.timer0_callback().set(FUNC(hunter2_state::timer0_out));
	iotimer.timer1_callback().set(FUNC(hunter2_state::timer1_out));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->cts_handler().set(FUNC(hunter2_state::cts_w));
	m_rs232->rxd_handler().set(FUNC(hunter2_state::rxd_w));

	/* internal ram */
	RAM(config, m_ram).set_default_size("352K").set_extra_options("80K,144K,208K,496K").set_default_value(0xff);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void hunter2_state::hunter(machine_config &config)
{
	hunter2(config);

	/* video hardware */
	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(240, 64);
	screen.set_visarea(0, 239, 0, 63);
}

/* ROM definition */
ROM_START( hunter )
	ROM_REGION(0x10000, "rom", ROMREGION_ERASEFF) // board has space for 6 roms, but only 2 are populated normally
	ROM_SYSTEM_BIOS(0, "1", "DEMOS 2.2 9G06h")
	ROMX_LOAD( "99-06h-00.ic50", 0x0000, 0x8000, CRC(ccc57737) SHA1(917d0d7983ab735b24e36b1e75b948c444105f28), ROM_BIOS(0) )
	ROMX_LOAD( "99-06h-01.ic51", 0x8000, 0x8000, CRC(8b1b18c7) SHA1(734d37505cf4c3c8effc75dedd129311a9b20a38), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "2", "DEMOS 2.2 9G+")
	ROMX_LOAD( "husky0iss9.ic50", 0x0000, 0x8000, CRC(77f80f33) SHA1(b0240ecfc95d32e960179af2bfa97fe5b38f7440), ROM_BIOS(1) )
	ROMX_LOAD( "husky1iss9.ic51", 0x8000, 0x8000, CRC(2c8f05fb) SHA1(5debc00037259be0d017c34e54ef4992cd6c0b38), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(2, "3", "DEMOS Ver 2.2") // not working, maybe auto-run software from RAM
	ROMX_LOAD( "v08c-0.ic50", 0x0000, 0x2000, CRC(e32687f5) SHA1(1679e1aa78d822da57f59f1eb5560c96794a199f), ROM_BIOS(2) )
	ROMX_LOAD( "v08c-1.ic51", 0x2000, 0x2000, CRC(cc669b10) SHA1(f9a197bfc9e5800910ba71b5c4e8d810320a7a43), ROM_BIOS(2) )
	ROMX_LOAD( "v08c-2.ic52", 0x4000, 0x2000, CRC(df0a4dcb) SHA1(79e49d37ae710790c29bc49e2ec7c9c113a36561), ROM_BIOS(2) )
	ROMX_LOAD( "v08c-3.ic53", 0x6000, 0x2000, CRC(d6a7869e) SHA1(e3d3364b4083f312031e96b340fbda763b310d28), ROM_BIOS(2) )
	ROMX_LOAD( "v08c-4.ic54", 0x8000, 0x2000, CRC(0339b049) SHA1(7d5d5297ee9db348566b8f717942bde0eb67d058), ROM_BIOS(2) )
	ROMX_LOAD( "v08c-5.ic55", 0xa000, 0x2000, CRC(98fcde65) SHA1(b9f3ce9a172ab29672026912e96c4a838cbddb46), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(3, "4", "DEMOS 2.2 9G06h (6 ROMs)")
	ROMX_LOAD( "eprom0.ic50", 0x0000, 0x2000, CRC(ad861c35) SHA1(fe8bcad60d4a0dc94cc6756d9132743f27fbb65d), ROM_BIOS(3) )
	ROMX_LOAD( "eprom1.ic51", 0x2000, 0x2000, CRC(c8c2fae8) SHA1(4235b4483ea8a091d332248074f72f220692c05e), ROM_BIOS(3) )
	ROMX_LOAD( "eprom2.ic52", 0x4000, 0x2000, CRC(b301621f) SHA1(02a0ce58405e95b48f7ac8aa291cb836a6927375), ROM_BIOS(3) )
	ROMX_LOAD( "eprom3.ic53", 0x6000, 0x2000, CRC(be6f196a) SHA1(73fc7f953b56d491ad4fcfe80636db0150a718dc), ROM_BIOS(3) )
	ROMX_LOAD( "eprom4.ic54", 0x8000, 0x2000, CRC(7a0d2228) SHA1(6667fbd5583217ea7ba9b4b4fc9dc56240b0e14e), ROM_BIOS(3) )
	ROMX_LOAD( "eprom5.ic55", 0xa000, 0x2000, CRC(48526702) SHA1(bd8f5a37e5f16511623dc128e2b95b4866b4297a), ROM_BIOS(3) )
ROM_END

ROM_START( hunter2 )
	ROM_REGION(0x10000, "rom", ROMREGION_ERASEFF) // board has space for 6 roms, but only 2 are populated normally
	ROM_SYSTEM_BIOS(0, "1", "DEMOS 2.21 9G08h V4")
	ROMX_LOAD( "tr032kx8mrom0.ic50", 0x0000, 0x8000, CRC(694d252c) SHA1(b11dbf24faf648596d92b1823e25a8e4fb7f542c), ROM_BIOS(0) )
	ROMX_LOAD( "tr032kx8mrom1.ic51", 0x8000, 0x8000, CRC(82901642) SHA1(d84f2bbd2e9e052bd161a313c240a67918f774ad), ROM_BIOS(0) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY                FULLNAME          FLAGS
COMP( 1983, hunter,  hunter2, 0,      hunter,  hunter2, hunter2_state, init_hunter2, "Husky Computers Ltd", "Husky Hunter",   MACHINE_NOT_WORKING )
COMP( 1984, hunter2, 0,       0,      hunter2, hunter2, hunter2_state, init_hunter2, "Husky Computers Ltd", "Husky Hunter 2", MACHINE_NOT_WORKING )
