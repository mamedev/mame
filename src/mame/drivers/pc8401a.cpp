// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "includes/pc8401a.h"

/*

    NEC PC-8401A-LS "Starlet"
    NEC PC-8500 "Studley"

    TODO:

    - keyboard interrupt
    - RTC TP pulse
    - disassembler for NEC uPD70008C (RST mnemonics are different from Z80)
    - clock does not advance in menu
    - mirror e800-ffff to 6800-7fff
    - soft power on/off
    - NVRAM
    - 8251 USART
    - 8255 ports
    - MC6845 palette
    - MC6845 chargen ROM
    - MC6845 screen update

    - peripherals
        * PC-8431A Dual Floppy Drive
        * PC-8441A CRT / Disk Interface
        * PC-8461A 1200 Baud Modem
        * PC-8407A 128KB RAM Expansion
        * PC-8508A ROM/RAM Cartridge

    - Use the 600 baud save rate (PIP CAS2:=A:<filename.ext> this is more reliable than the 1200 baud (PIP CAS:=A:<filename.ext> rate.

*/

#include "bus/rs232/rs232.h"

/* Fake Keyboard */

void pc8401a_state::scan_keyboard()
{
	int strobe = 0;

	/* scan keyboard */
	for (int row = 0; row < 10; row++)
	{
		uint8_t data = m_io_y[row]->read();

		if (data != 0xff)
		{
			strobe = 1;
			m_key_latch = data;
		}
	}

	if (!m_key_strobe && strobe)
	{
		/* trigger interrupt */
		m_maincpu->set_input_line_and_vector(INPUT_LINE_IRQ0, ASSERT_LINE, 0x28);
		logerror("INTERRUPT\n");
	}

	if (strobe) m_key_strobe = strobe;
}

TIMER_DEVICE_CALLBACK_MEMBER(pc8401a_state::pc8401a_keyboard_tick)
{
	scan_keyboard();
}

/* Read/Write Handlers */

void pc8401a_state::bankswitch(uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	int rombank = data & 0x03;
	int ram0000 = (data >> 2) & 0x03;
	int ram8000 = (data >> 4) & 0x03;

	switch (ram0000)
	{
	case 0: /* ROM 0000H to 7FFFH */
		if (rombank < 3)
		{
			/* internal ROM */
			program.install_read_bank(0x0000, 0x7fff, "bank1");
			program.unmap_write(0x0000, 0x7fff);
			membank("bank1")->set_entry(rombank);
		}
		else if (m_cart_rom)
		{
			/* ROM cartridge */
			program.install_read_bank(0x0000, 0x7fff, "bank1");
			program.unmap_write(0x0000, 0x7fff);
			membank("bank1")->set_entry(6);
		}
		else
			program.unmap_readwrite(0x0000, 0x7fff);
		//logerror("0x0000-0x7fff = ROM %u\n", rombank);
		break;

	case 1: /* RAM 0000H to 7FFFH */
		program.install_readwrite_bank(0x0000, 0x7fff, "bank1");
		membank("bank1")->set_entry(4);
		//logerror("0x0000-0x7fff = RAM 0-7fff\n");
		break;

	case 2: /* RAM 8000H to FFFFH */
		program.install_readwrite_bank(0x0000, 0x7fff, "bank1");
		membank("bank1")->set_entry(5);
		//logerror("0x0000-0x7fff = RAM 8000-ffff\n");
		break;

	case 3: /* invalid */
		logerror("0x0000-0x7fff = invalid\n");
		break;
	}

	switch (ram8000)
	{
	case 0: /* cell addresses 0000H to 3FFFH */
		program.install_readwrite_bank(0x8000, 0xbfff, "bank3");
		membank("bank3")->set_entry(0);
		//logerror("0x8000-0xbfff = RAM 0-3fff\n");
		break;

	case 1: /* cell addresses 4000H to 7FFFH */
		program.install_readwrite_bank(0x8000, 0xbfff, "bank3");
		membank("bank3")->set_entry(1);
		//logerror("0x8000-0xbfff = RAM 4000-7fff\n");
		break;

	case 2: /* cell addresses 8000H to BFFFH */
		program.install_readwrite_bank(0x8000, 0xbfff, "bank3");
		membank("bank3")->set_entry(2);
		//logerror("0x8000-0xbfff = RAM 8000-bfff\n");
		break;

	case 3: /* RAM cartridge */
		if (m_ram->size() > 64)
		{
			program.install_readwrite_bank(0x8000, 0xbfff, "bank3");
			membank("bank3")->set_entry(3); // TODO or 4
		}
		else
		{
			program.unmap_readwrite(0x8000, 0xbfff);
		}
		//logerror("0x8000-0xbfff = RAM cartridge\n");
		break;
	}

	if (BIT(data, 6))
	{
		/* CRT video RAM */
		program.install_readwrite_bank(0xc000, 0xdfff, "bank4");
		program.unmap_readwrite(0xe000, 0xe7ff);
		membank("bank4")->set_entry(1);
		//logerror("0xc000-0xdfff = video RAM\n");
	}
	else
	{
		/* RAM */
		program.install_readwrite_bank(0xc000, 0xe7ff, "bank4");
		membank("bank4")->set_entry(0);
		//logerror("0xc000-0e7fff = RAM c000-e7fff\n");
	}
}

WRITE8_MEMBER( pc8401a_state::mmr_w )
{
	/*

	    bit     description

	    0       ROM section bit 0
	    1       ROM section bit 1
	    2       mapping for CPU addresses 0000H to 7FFFH bit 0
	    3       mapping for CPU addresses 0000H to 7FFFH bit 1
	    4       mapping for CPU addresses 8000H to BFFFH bit 0
	    5       mapping for CPU addresses 8000H to BFFFH bit 1
	    6       mapping for CPU addresses C000H to E7FFH
	    7

	*/

	if (data != m_mmr)
	{
		bankswitch(data);
	}

	m_mmr = data;
}

READ8_MEMBER( pc8401a_state::mmr_r )
{
	return m_mmr;
}

READ8_MEMBER( pc8401a_state::rtc_r )
{
	/*

	    bit     description

	    0       RTC TP?
	    1       RTC DATA OUT
	    2       ?
	    3
	    4
	    5
	    6
	    7

	*/

	return (m_rtc->data_out_r() << 1) | (m_rtc->tp_r() << 2);
}

WRITE8_MEMBER( pc8401a_state::rtc_cmd_w )
{
	/*

	    bit     description

	    0       RTC C0
	    1       RTC C1
	    2       RTC C2
	    3       RTC DATA IN?
	    4
	    5
	    6
	    7

	*/

	m_rtc->c0_w(BIT(data, 0));
	m_rtc->c1_w(BIT(data, 1));
	m_rtc->c2_w(BIT(data, 2));
	m_rtc->data_in_w(BIT(data, 3));
}

WRITE8_MEMBER( pc8401a_state::rtc_ctrl_w )
{
	/*

	    bit     description

	    0       RTC OE or CS?
	    1       RTC STB
	    2       RTC CLK
	    3
	    4
	    5
	    6
	    7

	*/

	m_rtc->oe_w(BIT(data, 0));
	m_rtc->stb_w(BIT(data, 1));
	m_rtc->clk_w(BIT(data, 2));
}

READ8_MEMBER( pc8401a_state::io_rom_data_r )
{
	//logerror("I/O ROM read from %05x\n", m_io_addr);
	return m_io_cart->read_rom(m_io_addr);
}

WRITE8_MEMBER( pc8401a_state::io_rom_addr_w )
{
	switch (offset)
	{
	case 0: /* A17..A16 */
		m_io_addr = ((data & 0x03) << 16) | (m_io_addr & 0xffff);
		break;

	case 1: /* A15..A8 */
		m_io_addr = (m_io_addr & 0x300ff) | (data << 8);
		break;

	case 2: /* A7..A0 */
		m_io_addr = (m_io_addr & 0x3ff00) | data;
		break;

	case 3:
		/* the same data is written here as to 0xb2, maybe this latches the address value? */
		break;
	}
}

READ8_MEMBER( pc8401a_state::port70_r )
{
	/*

	    bit     description

	    0       key pressed
	    1
	    2
	    3
	    4       must be 1 or CPU goes to HALT
	    5
	    6
	    7

	*/

	return 0x10 | m_key_strobe;
}

READ8_MEMBER( pc8401a_state::port71_r )
{
	return m_key_latch;
}

WRITE8_MEMBER( pc8401a_state::port70_w )
{
	m_key_strobe = 0;
}

WRITE8_MEMBER( pc8401a_state::port71_w )
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);

	m_key_latch = data;
}

/* Memory Maps */

void pc8401a_state::pc8401a_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).bankrw("bank1");
	map(0x8000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xe7ff).bankrw("bank4");
	map(0xe800, 0xffff).bankrw("bank5");
}

void pc8401a_state::pc8401a_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
}

void pc8401a_state::pc8500_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).portr("Y.0");
	map(0x01, 0x01).portr("Y.1");
	map(0x02, 0x02).portr("Y.2");
	map(0x03, 0x03).portr("Y.3");
	map(0x04, 0x04).portr("Y.4");
	map(0x05, 0x05).portr("Y.5");
	map(0x06, 0x06).portr("Y.6");
	map(0x07, 0x07).portr("Y.7");
	map(0x08, 0x08).portr("Y.8");
	map(0x09, 0x09).portr("Y.9");
	map(0x10, 0x10).w(FUNC(pc8401a_state::rtc_cmd_w));
	map(0x20, 0x21).rw(I8251_TAG, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x30, 0x30).rw(FUNC(pc8401a_state::mmr_r), FUNC(pc8401a_state::mmr_w));
//  AM_RANGE(0x31, 0x31)
	map(0x40, 0x40).rw(FUNC(pc8401a_state::rtc_r), FUNC(pc8401a_state::rtc_ctrl_w));
//  AM_RANGE(0x41, 0x41)
//  AM_RANGE(0x50, 0x51)
	map(0x60, 0x60).rw(m_lcdc, FUNC(sed1330_device::status_r), FUNC(sed1330_device::data_w));
	map(0x61, 0x61).rw(m_lcdc, FUNC(sed1330_device::data_r), FUNC(sed1330_device::command_w));
	map(0x70, 0x70).rw(FUNC(pc8401a_state::port70_r), FUNC(pc8401a_state::port70_w));
	map(0x71, 0x71).rw(FUNC(pc8401a_state::port71_r), FUNC(pc8401a_state::port71_w));
//  AM_RANGE(0x80, 0x80) modem status, set to 0xff to boot
//  AM_RANGE(0x8b, 0x8b)
//  AM_RANGE(0x90, 0x93)
//  AM_RANGE(0xa0, 0xa1)
	map(0x98, 0x98).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0x99, 0x99).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xb0, 0xb3).w(FUNC(pc8401a_state::io_rom_addr_w));
	map(0xb3, 0xb3).r(FUNC(pc8401a_state::io_rom_data_r));
//  AM_RANGE(0xc8, 0xc8)
	map(0xfc, 0xff).rw(I8255A_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

/* Input Ports */

static INPUT_PORTS_START( pc8401a )
	PORT_START("Y.0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STOP")// PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y.1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("Y.2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("Y.3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')

	PORT_START("Y.4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("Y.5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('*')

	PORT_START("Y.6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('*')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('*')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) // ?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('*')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('*')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("Y.7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) // ^I
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) // ^C

	PORT_START("Y.8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("Y.9")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F11)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F12)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL BKSP") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
INPUT_PORTS_END

/* Machine Initialization */

void pc8401a_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	/* initialize RTC */
	m_rtc->cs_w(1);

	/* allocate CRT video RAM */
	m_crt_ram.allocate(PC8401A_CRT_VIDEORAM_SIZE);

	uint8_t *ram = m_ram->pointer();

	/* set up A0/A1 memory banking */
	membank("bank1")->configure_entries(0, 4, m_rom->base(), 0x8000);
	membank("bank1")->configure_entries(4, 2, ram, 0x8000);
	if (m_cart_rom)
		membank("bank1")->configure_entries(6, 1, m_cart_rom->base(), 0x8000);
	membank("bank1")->set_entry(0);

	/* set up A2 memory banking */
	membank("bank3")->configure_entries(0, 5, ram, 0x4000);
	membank("bank3")->set_entry(0);

	/* set up A3 memory banking */
	membank("bank4")->configure_entry(0, ram + 0xc000);
	membank("bank4")->configure_entry(1, m_crt_ram);
	membank("bank4")->set_entry(0);

	/* set up A4 memory banking */
	membank("bank5")->configure_entry(0, ram + 0xe800);
	membank("bank5")->set_entry(0);

	/* bank switch */
	bankswitch(0);

	/* register for state saving */
	save_item(NAME(m_mmr));
	save_item(NAME(m_io_addr));
}

READ8_MEMBER( pc8401a_state::ppi_pc_r )
{
	/*

	    bit     signal          description

	    PC0
	    PC1
	    PC2
	    PC3
	    PC4     PC-8431A DAV    data valid
	    PC5     PC-8431A RFD    ready for data
	    PC6     PC-8431A DAC    data accepted
	    PC7     PC-8431A ATN    attention

	*/

	return 0;
}

WRITE8_MEMBER( pc8401a_state::ppi_pc_w )
{
	/*

	    bit     signal          description

	    PC0
	    PC1
	    PC2
	    PC3
	    PC4     PC-8431A DAV    data valid
	    PC5     PC-8431A RFD    ready for data
	    PC6     PC-8431A DAC    data accepted
	    PC7     PC-8431A ATN    attention

	*/
}

/* Machine Drivers */

void pc8401a_state::pc8401a(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000); // NEC uPD70008C
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8401a_state::pc8401a_mem);
	m_maincpu->set_addrmap(AS_IO, &pc8401a_state::pc8401a_io);

	/* fake keyboard */
	TIMER(config, "keyboard").configure_periodic(FUNC(pc8401a_state::pc8401a_keyboard_tick), attotime::from_hz(64));

	/* devices */
	UPD1990A(config, m_rtc);

	i8255_device &ppi(I8255A(config, I8255A_TAG));
	ppi.in_pc_callback().set(FUNC(pc8401a_state::ppi_pc_r));
	ppi.out_pc_callback().set(FUNC(pc8401a_state::ppi_pc_w));

	i8251_device &uart(I8251(config, I8251_TAG, 0));
	uart.txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(I8251_TAG, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(I8251_TAG, FUNC(i8251_device::write_dsr));

	/* video hardware */
	pc8401a_video(config);

	/* option ROM cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, nullptr, "bin,rom");

	/* I/O ROM cartridge */
	GENERIC_CARTSLOT(config, m_io_cart, generic_linear_slot, nullptr, "bin,rom");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K").set_extra_options("96K");
}

void pc8500_state::pc8500(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000); // NEC uPD70008C
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8500_state::pc8401a_mem);
	m_maincpu->set_addrmap(AS_IO, &pc8500_state::pc8500_io);

	/* fake keyboard */
	TIMER(config, "keyboard").configure_periodic(FUNC(pc8401a_state::pc8401a_keyboard_tick), attotime::from_hz(64));

	/* devices */
	UPD1990A(config, m_rtc);

	i8255_device &ppi(I8255A(config, I8255A_TAG));
	ppi.in_pc_callback().set(FUNC(pc8401a_state::ppi_pc_r));
	ppi.out_pc_callback().set(FUNC(pc8401a_state::ppi_pc_w));

	i8251_device &uart(I8251(config, I8251_TAG, 0));
	uart.txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	uart.dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	uart.rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(I8251_TAG, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(I8251_TAG, FUNC(i8251_device::write_dsr));

	/* video hardware */
	pc8500_video(config);

	/* option ROM cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, nullptr, "bin,rom");

	/* I/O ROM cartridge */
	GENERIC_CARTSLOT(config, m_io_cart, generic_linear_slot, nullptr, "bin,rom");

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K").set_extra_options("96K");
}

/* ROMs */

ROM_START( pc8401a )
	ROM_REGION( 0x20000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "pc8401a.bin", 0x0000, 0x18000, NO_DUMP )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "pc8441a.bin", 0x0000, 0x1000, NO_DUMP )
ROM_END

ROM_START( pc8500 )
	ROM_REGION( 0x20000, Z80_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "pc8500.bin", 0x0000, 0x10000, CRC(c2749ef0) SHA1(f766afce9fda9ec84ed5b39ebec334806798afb3) )

	ROM_REGION( 0x1000, "chargen", 0 )
	ROM_LOAD( "pc8441a.bin", 0x0000, 0x1000, NO_DUMP )
ROM_END

/* System Drivers */

/*    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY  FULLNAME */
COMP( 1984, pc8401a,  0,       0,      pc8401a, pc8401a, pc8401a_state, empty_init, "NEC",   "PC-8401A-LS", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
//COMP( 1984, pc8401bd, pc8401a, 0,      pc8401a, pc8401a, pc8401a_state, empty_init, "NEC",   "PC-8401BD", MACHINE_NOT_WORKING)
COMP( 1985, pc8500,   0,       0,      pc8500,  pc8401a, pc8500_state,  empty_init, "NEC",   "PC-8500", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
