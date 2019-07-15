// license:BSD-3-Clause
// copyright-holders:Curt Coder
/******************************************************************************************

    Kyocera Kyotronics 85 (and similar laptop computers)

    2009/04 Very Preliminary Driver (video emulation courtesy of very old code by
            Hamish Coleman)

    Comments about bit usage from Tech References and Virtual T source.

    Supported systems:
      - Kyosei Kyotronic 85
      - Olivetti M10 (slightly diff hw, BIOS is shifted by 2 words)
      - NEC PC-8201A (slightly diff hw)
      - TRS-80 Model 100
      - Tandy Model 102 (slightly diff hw)
      - Tandy Model 200 (diff video & rtc)

    To Do:
      - Find dumps of systems which could easily be added:
        * Olivetti M10 Modem (US) (diff BIOS than the European version)
        * NEC PC-8201 (original Japanese version of PC-8201A)
        * NEC PC-8300 (similar hardware to PC-8201)
        * NEC PC-8300 w/BradyWriter II ROMs

    - bar code reader (!RxDB -> RST5.5, Hewlett-Packard HREDS-3050 interface)
    - un-Y2K-hack tandy200
    - keyboard is unresponsive for couple of seconds after boot
    - soft power on/off
    - pc8201 48K RAM option
    - pc8201 NEC PC-8241A video interface (TMS9918, 16K videoRAM, 8K ROM)
    - pc8201 NEC PC-8233 floppy controller
    - pc8201 NEC floppy disc drives (PC-8031-1W, PC-8031-2W, PC-80S31)
    - trsm100 Tandy Portable Disk Drive (TPDD: 100k 3?", TPDD2: 200k 3?") (undumped HD63A01V1 MCU + full custom uPD65002, serial comms via IM6042)
    - trsm100 Chipmunk disk drive (384k 3?") (full custom logic, not going to happen)
    - trsm100 RS232/modem select
    - tandy200 RTC alarm
    - tandy200 TCM5089 DTMF sound
    - international keyboard option ROMs
    - cassette is not working on pc8201, pc8201a, npc8300

    10 FOR A=0 TO 255
    20 PRINT CHR$(A);
    30 NEXT A


                          * PC-8201/8300 HARDWARE PORT DEFINITIONS *

                -Port-
    Name       Hex  Dec   Notes
    --------   ---  ---   -----------------------------------------
    A8255      070  112   Video interface port A (8255)
    B8255      071  113   Video interface port B (8255)
    C8255      072  114   Video interface port C (8255)
    CW8255     073  115   Video interface command/mode port (8255)

******************************************************************************************/


#include "emu.h"
#include "includes/kyocera.h"
#include "softlist.h"
#include "speaker.h"

/* Read/Write Handlers */

READ8_MEMBER( pc8201_state::bank_r )
{
	/*

	    bit     signal      description

	    0       LADR1       select address 0 to 7fff
	    1       LADR2       select address 0 to 7fff
	    2       HADR1       select address 8000 to ffff
	    3       HADR2       select address 8000 to ffff
	    4
	    5
	    6       SELB        serial interface status bit 1
	    7       SELA        serial interface status bit 0

	*/

	return (m_iosel << 5) | m_bank;
}

void pc8201_state::bankswitch(uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	int rom_bank = data & 0x03;
	int ram_bank = (data >> 2) & 0x03;

	m_bank = data & 0x0f;

	if (rom_bank > 1)
	{
		/* RAM */
		program.install_readwrite_bank(0x0000, 0x7fff, "bank1");
	}
	else
	{
		/* ROM */
		program.install_read_bank(0x0000, 0x7fff, "bank1");
		program.unmap_write(0x0000, 0x7fff);
	}

	membank("bank1")->set_entry(rom_bank);

	switch (ram_bank)
	{
	case 0:
		if (m_ram->size() > 16 * 1024)
		{
			program.install_readwrite_bank(0x8000, 0xffff, "bank2");
		}
		else
		{
			program.unmap_readwrite(0x8000, 0xbfff);
			program.install_readwrite_bank(0xc000, 0xffff, "bank2");
		}
		break;

	case 1:
		program.unmap_readwrite(0x8000, 0xffff);
		break;

	case 2:
		if (m_ram->size() > 32 * 1024)
			program.install_readwrite_bank(0x8000, 0xffff, "bank2");
		else
			program.unmap_readwrite(0x8000, 0xffff);
		break;

	case 3:
		if (m_ram->size() > 64 * 1024)
			program.install_readwrite_bank(0x8000, 0xffff, "bank2");
		else
			program.unmap_readwrite(0x8000, 0xffff);
		break;
	}

	membank("bank2")->set_entry(ram_bank);
}

WRITE8_MEMBER( pc8201_state::bank_w )
{
	/*

	    bit     signal      description

	    0       LADR1       select address 0 to 7fff
	    1       LADR2       select address 0 to 7fff
	    2       HADR1       select address 8000 to ffff
	    3       HADR2       select address 8000 to ffff
	    4
	    5
	    6
	    7

	*/
printf("bank %02x\n",data);
	bankswitch(data);
}

WRITE8_MEMBER( pc8201_state::scp_w )
{
	/*

	    bit     signal      description

	    0
	    1
	    2
	    3       REMOTE      cassette motor
	    4       TSTB        RTC strobe
	    5       PSTB        printer strobe
	    6       SELB        serial interface select bit 1
	    7       SELA        serial interface select bit 0

	*/

	/* cassette motor */
	m_cassette->change_state(BIT(data,3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	/* RTC strobe */
	m_rtc->stb_w(BIT(data, 4));

	/* printer strobe */
	m_centronics->write_strobe(BIT(data, 5));

	/* serial interface select */
	m_iosel = data >> 5;
}

WRITE8_MEMBER( kc85_state::uart_ctrl_w )
{
	/*

	    bit     signal      description

	    0       SBS         stop bit select
	    1       EPE         even parity enable
	    2       PI          parity inhibit
	    3       CLS1        character length select bit 1
	    4       CLS2        character length select bit 2
	    5
	    6
	    7

	*/

	m_uart->sbs_w(BIT(data, 0));
	m_uart->epe_w(BIT(data, 1));
	m_uart->pi_w(BIT(data, 2));
	m_uart->cls1_w(BIT(data, 3));
	m_uart->cls2_w(BIT(data, 4));

	m_uart->crl_w(1);
}

READ8_MEMBER( kc85_state::uart_status_r )
{
	/*

	    bit     signal      description

	    0       CD          carrier detect
	    1       OE          overrun error
	    2       FE          framing error
	    3       PE          parity error
	    4       TBRE        transmit buffer register empty
	    5       RP
	    6       +5V
	    7       _LPS        low power sensor

	*/

	uint8_t data = 0x40;

	// carrier detect
	data |= m_rs232->dcd_r();

	// overrun error
	data |= m_uart->oe_r() << 1;

	// framing error
	data |= m_uart->fe_r() << 2;

	// parity error
	data |= m_uart->pe_r() << 3;

	// transmit buffer register empty
	data |= m_uart->tbre_r() << 4;

	// rp TODO
	data |= 0x20;

	// low power sensor
	data |= BIT(m_battery->read(), 0) << 7;

	return data;
}

READ8_MEMBER( pc8201_state::uart_status_r )
{
	/*

	    bit     signal      description

	    0       _DCD/_RD    data carrier detect / ring detect
	    1       OE          overrun error
	    2       FE          framing error
	    3       PE          parity error
	    4       TBRE        transmit buffer register empty
	    5       RP
	    6       +5V
	    7       _LPS        low power signal

	*/

	uint8_t data = 0x40;

	// data carrier detect / ring detect
	data |= m_rs232->dcd_r();

	// overrun error
	data |= m_uart->oe_r() << 1;

	// framing error
	data |= m_uart->fe_r() << 2;

	// parity error
	data |= m_uart->pe_r() << 3;

	// transmit buffer register empty
	data |= m_uart->tbre_r() << 4;

	// rp TODO
	data |= 0x20;

	// low power sensor
	data |= BIT(m_battery->read(), 0) << 7;

	return data;
}

WRITE8_MEMBER( pc8201_state::romah_w )
{
	/*

	    bit     signal

	    0       A16
	    1       ROM SEL
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	// ROM address bit 16
	m_rom_addr = (BIT(data, 0) << 16) | (m_rom_addr & 0xffff);

	// ROM select
	m_rom_sel = BIT(data, 1);
}

WRITE8_MEMBER( pc8201_state::romal_w )
{
	/*

	    bit     signal

	    0       A0
	    1       A1
	    2       A2
	    3       A3
	    4       A4
	    5       A5
	    6       A6
	    7       A7

	*/

	m_rom_addr = (m_rom_addr & 0x1ff00) | data;
}

WRITE8_MEMBER( pc8201_state::romam_w )
{
	/*

	    bit     signal

	    0       A8
	    1       A9
	    2       A10
	    3       A11
	    4       A12
	    5       A13
	    6       A14
	    7       A15

	*/

	m_rom_addr = (m_rom_addr & 0x100ff) | (data << 8);
}

READ8_MEMBER( pc8201_state::romrd_r )
{
	uint8_t data = 0xff;

	if (m_rom_sel)
		data = m_cas_cart->read_rom(m_rom_addr & 0x1ffff);

	return data;
}

WRITE8_MEMBER( kc85_state::modem_w )
{
	/*

	    bit     signal      description

	    0                   telephone line signal selection relay output
	    1       EN          MC14412 enable output
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	//m_modem->en_w(BIT(data, 1));
}

WRITE8_MEMBER( kc85_state::ctrl_w )
{
	/*

	    bit     signal      description

	    0       _STROM      ROM selection (0=standard, 1=option)
	    1       _STROBE     printer strobe output
	    2       STB         RTC strobe output
	    3       _REMOTE     cassette motor
	    4
	    5
	    6
	    7

	*/

	/* ROM bank selection */
	membank("bank1")->set_entry(BIT(data, 0));

	/* printer strobe */
	m_centronics->write_strobe(BIT(data, 1));

	/* RTC strobe */
	m_rtc->stb_w(BIT(data, 2));

	/* cassette motor */
	m_cassette->change_state(BIT(data,3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

READ8_MEMBER( kc85_state::keyboard_r )
{
	uint8_t data = 0xff;

	for (int i = 0; i < 9; i++)
	{
		if (!BIT(m_keylatch, i)) data &= m_y[i]->read();
	}

	return data;
}

void tandy200_state::bankswitch(uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	int rom_bank = data & 0x03;
	int ram_bank = (data >> 2) & 0x03;

	m_bank = data & 0x0f;

	if (rom_bank == 3)
	{
		/* invalid ROM bank */
		program.unmap_readwrite(0x0000, 0x7fff);
	}
	else
	{
		program.install_read_bank(0x0000, 0x7fff, "bank1");
		program.unmap_write(0x0000, 0x7fff);
		membank("bank1")->set_entry(rom_bank);
	}

	if (m_ram->size() < ((ram_bank + 1) * 24 * 1024))
	{
		/* invalid RAM bank */
		program.unmap_readwrite(0xa000, 0xffff);
	}
	else
	{
		program.install_readwrite_bank(0xa000, 0xffff, "bank2");
		membank("bank2")->set_entry(ram_bank);
	}
}

READ8_MEMBER( tandy200_state::bank_r )
{
	return m_bank;
}

WRITE8_MEMBER( tandy200_state::bank_w )
{
	bankswitch(data);
}

READ8_MEMBER( tandy200_state::stbk_r )
{
	uint8_t data = 0xff;

	for (int i = 0; i < 9; i++)
	{
		if (!BIT(m_keylatch, i)) data &= m_y[i]->read();
	}

	return data;
}

WRITE8_MEMBER( tandy200_state::stbk_w )
{
	/*

	    bit     signal  description

	    0       _PSTB   printer strobe output
	    1       REMOTE  cassette motor
	    2
	    3
	    4
	    5
	    6
	    7

	*/

	/* printer strobe */
	m_centronics->write_strobe(BIT(data, 0));

	/* cassette motor */
	m_cassette->change_state(BIT(data,1) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

READ8_MEMBER( kc85_state::lcd_r )
{
	uint8_t data = 0;

	for (uint8_t i = 0; i < 10; i++)
		data |= m_lcdc[i]->read(space, offset);

	return data;
}

WRITE8_MEMBER( kc85_state::lcd_w )
{
	for (uint8_t i = 0; i < 10; i++)
		m_lcdc[i]->write(space, offset, data);
}

/* Memory Maps */

void kc85_state::kc85_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).bankr("bank1");
	map(0x8000, 0xffff).bankrw("bank2");
}

void pc8201_state::pc8201_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).bankrw("bank1");
	map(0x8000, 0xffff).bankrw("bank2");
}

void tandy200_state::tandy200_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).bankr("bank1");
	map(0x8000, 0x9fff).rom();
	map(0xa000, 0xffff).bankrw("bank2");
}

void kc85_state::kc85_io(address_map &map)
{
	map.unmap_value_high();
//  AM_RANGE(0x70, 0x70) AM_MIRROR(0x0f) optional RAM unit
//  AM_RANGE(0x80, 0x80) AM_MIRROR(0x0f) optional I/O controller unit
//  AM_RANGE(0x90, 0x90) AM_MIRROR(0x0f) optional answering telephone unit
//  AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0f) optional modem
	map(0xb0, 0xb7).mirror(0x08).rw(I8155_TAG, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xc0, 0xc0).mirror(0x0f).rw(m_uart, FUNC(im6402_device::read), FUNC(im6402_device::write));
	map(0xd0, 0xd0).mirror(0x0f).rw(FUNC(kc85_state::uart_status_r), FUNC(kc85_state::uart_ctrl_w));
	map(0xe0, 0xe0).mirror(0x0f).rw(FUNC(kc85_state::keyboard_r), FUNC(kc85_state::ctrl_w));
	map(0xf0, 0xf1).mirror(0x0e).rw(FUNC(kc85_state::lcd_r), FUNC(kc85_state::lcd_w));
}

void kc85_state::trsm100_io(address_map &map)
{
	kc85_io(map);
	map(0xa0, 0xa0).mirror(0x0f).w(FUNC(kc85_state::modem_w));
}

void pc8201_state::pc8201_io(address_map &map)
{
	map.unmap_value_high();
//  AM_RANGE(0x70, 0x70) AM_MIRROR(0x0f) optional video interface 8255
	map(0x80, 0x80).mirror(0x03).w(FUNC(pc8201_state::romah_w));
	map(0x84, 0x84).mirror(0x03).w(FUNC(pc8201_state::romal_w));
	map(0x88, 0x88).mirror(0x03).w(FUNC(pc8201_state::romam_w));
	map(0x8c, 0x8c).mirror(0x03).r(FUNC(pc8201_state::romrd_r));
	map(0x90, 0x90).mirror(0x0f).w(FUNC(pc8201_state::scp_w));
	map(0xa0, 0xa0).mirror(0x0f).rw(FUNC(pc8201_state::bank_r), FUNC(pc8201_state::bank_w));
	map(0xb0, 0xb7).mirror(0x08).rw(I8155_TAG, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xc0, 0xc0).mirror(0x0f).rw(m_uart, FUNC(im6402_device::read), FUNC(im6402_device::write));
	map(0xd0, 0xd0).mirror(0x0f).r(FUNC(pc8201_state::uart_status_r)).w(FUNC(pc8201_state::uart_ctrl_w));
	map(0xe0, 0xe0).mirror(0x0f).r(FUNC(pc8201_state::keyboard_r));
	map(0xf0, 0xf1).mirror(0x0e).rw(FUNC(pc8201_state::lcd_r), FUNC(pc8201_state::lcd_w));
}

void tandy200_state::tandy200_io(address_map &map)
{
	map.unmap_value_high();
	map(0x90, 0x9f).rw(m_rtc, FUNC(rp5c01_device::read), FUNC(rp5c01_device::write));
//  AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0f) AM_DEVWRITE(TCM5089_TAG, write)
	map(0xb0, 0xb7).mirror(0x08).rw(I8155_TAG, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xc0, 0xc1).mirror(0x0e).rw(I8251_TAG, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xd0, 0xd0).mirror(0x0f).rw(FUNC(tandy200_state::bank_r), FUNC(tandy200_state::bank_w));
	map(0xe0, 0xe0).mirror(0x0f).rw(FUNC(tandy200_state::stbk_r), FUNC(tandy200_state::stbk_w));
	map(0xf0, 0xf0).mirror(0x0e).rw(m_lcdc, FUNC(hd61830_device::data_r), FUNC(hd61830_device::data_w));
	map(0xf1, 0xf1).mirror(0x0e).rw(m_lcdc, FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w));
}

/* Input Ports */

static INPUT_PORTS_START( kc85 )
	PORT_START("Y0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("Y1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("Y2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("Y3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("Y4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("Y5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')

	PORT_START("Y6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PRINT") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LABEL") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PASTE") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x92|") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL BKSP") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("Y7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("Y8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PAUSE BREAK") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NUM") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CODE") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING( 0x01, DEF_STR( Normal ) )
	PORT_CONFSETTING( 0x00, "Low Battery" )
INPUT_PORTS_END

static INPUT_PORTS_START( pc8201 )
	PORT_INCLUDE( kc85 )

	PORT_MODIFY("Y3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('^')

	PORT_MODIFY("Y4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')

	PORT_MODIFY("Y5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PAST INS") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RALT) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')

	PORT_MODIFY("Y6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x92|") PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL BKSP") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_MODIFY("Y7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STOP") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("f.5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("f.4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("f.3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("f.2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("f.1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_MODIFY("Y8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( pc8201a )
	PORT_INCLUDE( kc85 )

	PORT_MODIFY("Y3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('^')

	PORT_MODIFY("Y4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')

	PORT_MODIFY("Y5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PAST INS") PORT_CODE(KEYCODE_INSERT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RALT) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')

	PORT_MODIFY("Y6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x92|") PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL BKSP") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_MODIFY("Y7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STOP") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("f.5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("f.4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("f.3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("f.2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("f.1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_MODIFY("Y8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( olivm10 )
	PORT_START("Y0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("Y1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("Y2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("Y3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("Y4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')

	PORT_START("Y5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')

	PORT_START("Y6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PASTE") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x92|") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("DEL BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("Y7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("Y8")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PAUSE BREAK") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NUM") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GRAPH") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PRINT") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("LABEL") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x01, "Battery Status" )
	PORT_CONFSETTING( 0x01, DEF_STR( Normal ) )
	PORT_CONFSETTING( 0x00, "Low Battery" )
INPUT_PORTS_END


/* 8155 Interface */

WRITE8_MEMBER( kc85_state::i8155_pa_w )
{
	/*

	    bit     description

	    0       LCD chip select 0, key scan 0, RTC C0
	    1       LCD chip select 1, key scan 1, RTC C1
	    2       LCD chip select 2, key scan 2, RTC C2
	    3       LCD chip select 3, key scan 3, RTC CLK
	    4       LCD chip select 4, key scan 4, RTC DATA IN
	    5       LCD chip select 5, key scan 5
	    6       LCD chip select 6, key scan 6
	    7       LCD chip select 7, key scan 7

	*/

	/* keyboard */
	m_keylatch = (m_keylatch & 0x100) | data;

	/* LCD */
	for (uint8_t i = 0; i < 8; i++)
		m_lcdc[i]->cs2_w(BIT(data, i));

	/* RTC */
	m_rtc->c0_w(BIT(data, 0));
	m_rtc->c1_w(BIT(data, 1));
	m_rtc->c2_w(BIT(data, 2));
	m_rtc->clk_w(BIT(data, 3));
	m_rtc->data_in_w(BIT(data, 4));
}

WRITE8_MEMBER( kc85_state::i8155_pb_w )
{
	/*

	    bit     signal      description

	    0                   LCD chip select 8, key scan 8
	    1                   LCD chip select 9
	    2       _MC         melody control output
	    3       DCD/_RD     RS232 DCD/_RD select (0=RS232, 1=modem)
	    4       APO         auto power off output
	    5       BELL        buzzer output (0=ring, 1=not ring)
	    6       _DTR        RS232 data terminal ready output
	    7       _RTS        RS232 request to send output

	*/

	/* keyboard */
	m_keylatch = (BIT(data, 0) << 8) | (m_keylatch & 0xff);

	/* LCD */
	m_lcdc[8]->cs2_w(BIT(data, 0));
	m_lcdc[9]->cs2_w(BIT(data, 1));

	/* beeper */
	m_buzzer = BIT(data, 2);
	m_bell = BIT(data, 5);

	if (m_buzzer) m_speaker->level_w(m_bell);

	// RS-232
	m_rs232->write_dtr(BIT(data, 6));
	m_rs232->write_rts(BIT(data, 7));
}

WRITE_LINE_MEMBER( kc85_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( kc85_state::write_centronics_select )
{
	m_centronics_select = state;
}

READ8_MEMBER( kc85_state::i8155_pc_r )
{
	/*

	    bit     description

	    0       CDI         clock data input
	    1       SLCT        _BUSY signal from printer
	    2       BUSY        BUSY signal from printer
	    3       BCR         bar code reader data input
	    4       _CTS        RS232 clear to send input
	    5       _DSR        RS232 DSR input

	*/

	uint8_t data = 0;

	// clock data input
	data |= m_rtc->data_out_r();

	// centronics busy
	data |= m_centronics_select << 1;
	data |= m_centronics_busy << 2;

	// RS-232
	data |= m_rs232->cts_r() << 4;
	data |= m_rs232->dsr_r() << 5;

	return data;
}

WRITE_LINE_MEMBER( kc85_state::i8155_to_w )
{
	if (!m_buzzer && m_bell)
	{
		m_speaker->level_w(state);
	}

	m_uart->trc_w(state);
	m_uart->rrc_w(state);
}

WRITE8_MEMBER( tandy200_state::i8155_pa_w )
{
	/*

	    bit     description

	    0       print data 0, key scan 0
	    1       print data 1, key scan 1
	    2       print data 2, key scan 2
	    3       print data 3, key scan 3
	    4       print data 4, key scan 4
	    5       print data 5, key scan 5
	    6       print data 6, key scan 6
	    7       print data 7, key scan 7

	*/

	m_cent_data_out->write(data);

	m_keylatch = (m_keylatch & 0x100) | data;
}

WRITE8_MEMBER( tandy200_state::i8155_pb_w )
{
	/*

	    bit     signal      description

	    0                   key scan 8
	    1       ORIG/ANS    (1=ORIG, 0=ANS)
	    2       _BUZZER     (0=data from 8155 TO, 1=data from PB2)
	    3       _RS232C     (1=modem, 0=RS-232)
	    4       PCS         power cut signal
	    5       BELL        buzzer data output
	    6       MEN         modem enable output
	    7       CALL        connects and disconnects the phone line

	*/

	/* keyboard */
	m_keylatch = (BIT(data, 0) << 8) | (m_keylatch & 0xff);

	/* beeper */
	m_buzzer = BIT(data, 2);
	m_bell = BIT(data, 5);

	if (m_buzzer) m_speaker->level_w(m_bell);
}

WRITE_LINE_MEMBER( tandy200_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( tandy200_state::write_centronics_select )
{
	m_centronics_select = state;
}

READ8_MEMBER( tandy200_state::i8155_pc_r )
{
	/*

	    bit     signal  description

	    0       _LPS    low power sense input
	    1       _BUSY   not busy input
	    2       BUSY    busy input
	    3       BCR     bar code reader data input
	    4       CD      carrier detect input
	    5       CDBD    carrier detect break down input

	*/

	uint8_t data = 0x01;

	// centronics
	data |= m_centronics_select << 1;
	data |= m_centronics_busy << 2;

	// RS-232
	data |= m_rs232->dcd_r() << 4;

	return data;
}

WRITE_LINE_MEMBER( tandy200_state::i8155_to_w )
{
	if (!m_buzzer && m_bell)
	{
		m_speaker->level_w(state);
	}
}

/* Machine Drivers */

void kc85_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	std::string region_tag;
	m_opt_region = memregion(region_tag.assign(m_opt_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	/* initialize RTC */
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	/* configure ROM banking */
	program.install_read_bank(0x0000, 0x7fff, "bank1");
	program.unmap_write(0x0000, 0x7fff);
	membank("bank1")->configure_entry(0, m_rom->base());
	membank("bank1")->configure_entry(1, m_opt_region ? m_opt_region->base() : m_rom->base());
	membank("bank1")->set_entry(0);

	/* configure RAM banking */
	switch (m_ram->size())
	{
	case 16 * 1024:
		program.unmap_readwrite(0x8000, 0xbfff);
		program.install_readwrite_bank(0xc000, 0xffff, "bank2");
		break;

	case 32 * 1024:
		program.install_readwrite_bank(0x8000, 0xffff,"bank2");
		break;
	}

	membank("bank2")->configure_entry(0, m_ram->pointer());
	membank("bank2")->set_entry(0);

	/* register for state saving */
	save_item(NAME(m_bank));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_buzzer));
	save_item(NAME(m_bell));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_select));
}

void pc8201_state::machine_start()
{
	uint8_t *ram = m_ram->pointer();

	std::string region_tag;
	m_opt_region = memregion(region_tag.assign(m_opt_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	/* initialize RTC */
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	/* configure ROM banking */
	membank("bank1")->configure_entry(0, m_rom->base());
	membank("bank1")->configure_entry(1, m_opt_region ? m_opt_region->base() : m_rom->base());
	membank("bank1")->configure_entries(2, 2, ram + 0x8000, 0x8000);
	membank("bank1")->set_entry(0);

	/* configure RAM banking */
	membank("bank2")->configure_entry(0, ram);
	membank("bank2")->configure_entries(2, 2, ram + 0x8000, 0x8000);
	membank("bank2")->set_entry(0);

	bankswitch(0);

	/* register for state saving */
	save_item(NAME(m_bank));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_buzzer));
	save_item(NAME(m_bell));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_select));
	save_item(NAME(m_iosel));
}

void trsm100_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	std::string region_tag;
	m_opt_region = memregion(region_tag.assign(m_opt_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	/* initialize RTC */
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	/* configure ROM banking */
	program.install_read_bank(0x0000, 0x7fff, "bank1");
	program.unmap_write(0x0000, 0x7fff);
	membank("bank1")->configure_entry(0, m_rom->base());
	membank("bank1")->configure_entry(1, m_opt_region ? m_opt_region->base() : m_rom->base());
	membank("bank1")->set_entry(0);

	/* configure RAM banking */
	switch (m_ram->size())
	{
	case 8 * 1024:
		program.unmap_readwrite(0x8000, 0xcfff);
		program.install_readwrite_bank(0xe000, 0xffff, "bank2");
		break;

	case 16 * 1024:
		program.unmap_readwrite(0x8000, 0xbfff);
		program.install_readwrite_bank(0xc000, 0xffff, "bank2");
		break;

	case 24 * 1024:
		program.unmap_readwrite(0x8000, 0x9fff);
		program.install_readwrite_bank(0xa000, 0xffff, "bank2");
		break;

	case 32 * 1024:
		program.install_readwrite_bank(0x8000, 0xffff, "bank2");
		break;
	}

	membank("bank2")->configure_entry(0, m_ram->pointer());
	membank("bank2")->set_entry(0);

	/* register for state saving */
	save_item(NAME(m_bank));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_buzzer));
	save_item(NAME(m_bell));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_select));
}

void tandy200_state::machine_start()
{
	std::string region_tag;
	m_opt_region = memregion(region_tag.assign(m_opt_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	/* configure ROM banking */
	membank("bank1")->configure_entry(0, m_rom->base());
	membank("bank1")->configure_entry(1, m_rom->base() + 0x10000);
	membank("bank1")->configure_entry(2, m_opt_region ? m_opt_region->base() : m_rom->base());
	membank("bank1")->set_entry(0);

	/* configure RAM banking */
	membank("bank2")->configure_entries(0, 3, m_ram->pointer(), 0x6000);
	membank("bank2")->set_entry(0);

	/* register for state saving */
	save_item(NAME(m_bank));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_buzzer));
	save_item(NAME(m_bell));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_select));
	save_item(NAME(m_tp));
}

WRITE_LINE_MEMBER( kc85_state::kc85_sod_w )
{
	m_cassette->output(state ? +1.0 : -1.0);
}

READ_LINE_MEMBER( kc85_state::kc85_sid_r )
{
	return (m_cassette->input() > 0.04) ? 0 : 1;
}

WRITE_LINE_MEMBER( tandy200_state::kc85_sod_w )
{
	m_cassette->output(state ? +1.0 : -1.0);
}

READ_LINE_MEMBER( tandy200_state::kc85_sid_r )
{
	return (m_cassette->input() > 0.04) ? 0 : 1;
}

TIMER_DEVICE_CALLBACK_MEMBER(tandy200_state::tandy200_tp_tick)
{
	m_maincpu->set_input_line(I8085_RST75_LINE, m_tp);

	m_tp = !m_tp;
}

void kc85_state::kc85(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(4'915'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &kc85_state::kc85_mem);
	m_maincpu->set_addrmap(AS_IO, &kc85_state::kc85_io);
	m_maincpu->in_sid_func().set(FUNC(kc85_state::kc85_sid_r));
	m_maincpu->out_sod_func().set(FUNC(kc85_state::kc85_sod_w));

	/* video hardware */
	kc85_video(config);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	i8155_device &i8155(I8155(config, I8155_TAG, XTAL(4'915'200)/2));
	i8155.out_pa_callback().set(FUNC(kc85_state::i8155_pa_w));
	i8155.out_pb_callback().set(FUNC(kc85_state::i8155_pb_w));
	i8155.in_pc_callback().set(FUNC(kc85_state::i8155_pc_r));
	i8155.out_to_callback().set(FUNC(kc85_state::i8155_to_w));

	UPD1990A(config, m_rtc);
	m_rtc->tp_callback().set_inputline(m_maincpu, I8085_RST75_LINE);

	IM6402(config, m_uart, 0, 0);
	m_uart->tro_callback().set(RS232_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(im6402_device::write_rri));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(kc85_state::write_centronics_busy));
	m_centronics->select_handler().set(FUNC(kc85_state::write_centronics_select));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);

	/* option ROM cartridge */
	GENERIC_CARTSLOT(config, m_opt_cart, generic_linear_slot, "trsm100_cart", "bin,rom");

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("trsm100");

	/* internal ram */
	RAM(config, m_ram).set_default_size("16K").set_extra_options("32K");
}

void pc8201_state::pc8201(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(4'915'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &pc8201_state::pc8201_mem);
	m_maincpu->set_addrmap(AS_IO, &pc8201_state::pc8201_io);
	m_maincpu->in_sid_func().set(FUNC(kc85_state::kc85_sid_r));
	m_maincpu->out_sod_func().set(FUNC(kc85_state::kc85_sod_w));

	/* video hardware */
	kc85_video(config);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	i8155_device &i8155(I8155(config, I8155_TAG, XTAL(4'915'200)/2));
	i8155.out_pa_callback().set(FUNC(kc85_state::i8155_pa_w));
	i8155.out_pb_callback().set(FUNC(kc85_state::i8155_pb_w));
	i8155.in_pc_callback().set(FUNC(kc85_state::i8155_pc_r));
	i8155.out_to_callback().set(FUNC(kc85_state::i8155_to_w));

	UPD1990A(config, m_rtc);
	m_rtc->tp_callback().set_inputline(m_maincpu, I8085_RST75_LINE);

	IM6402(config, m_uart, 0, 0);
	m_uart->tro_callback().set(RS232_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(im6402_device::write_rri));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(kc85_state::write_centronics_busy));
	m_centronics->select_handler().set(FUNC(kc85_state::write_centronics_select));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);

	/* option ROM cartridge */
	GENERIC_CARTSLOT(config, m_opt_cart, generic_linear_slot, "pc8201_cart", "bin,rom");

	/* 128KB ROM cassette */
	GENERIC_CARTSLOT(config, "cas_cartslot", generic_linear_slot, "pc8201_cart2", "bin,rom");

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("pc8201");

	/* internal ram */
	RAM(config, m_ram).set_default_size("16K").set_extra_options("32K,64K,96K");
}

void pc8201_state::pc8300(machine_config &config)
{
	pc8201(config);
	m_ram->set_default_size("32K").set_extra_options("64K,96K");
}

void trsm100_state::trsm100(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(4'915'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &trsm100_state::kc85_mem);
	m_maincpu->set_addrmap(AS_IO, &trsm100_state::trsm100_io);
	m_maincpu->in_sid_func().set(FUNC(kc85_state::kc85_sid_r));
	m_maincpu->out_sod_func().set(FUNC(kc85_state::kc85_sod_w));

	/* video hardware */
	kc85_video(config);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	i8155_device &i8155(I8155(config, I8155_TAG, XTAL(4'915'200)/2));
	i8155.out_pa_callback().set(FUNC(kc85_state::i8155_pa_w));
	i8155.out_pb_callback().set(FUNC(kc85_state::i8155_pb_w));
	i8155.in_pc_callback().set(FUNC(kc85_state::i8155_pc_r));
	i8155.out_to_callback().set(FUNC(kc85_state::i8155_to_w));

	UPD1990A(config, m_rtc);
	m_rtc->tp_callback().set_inputline(m_maincpu, I8085_RST75_LINE);

	IM6402(config, m_uart, 0, 0);
	m_uart->tro_callback().set(RS232_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_uart, FUNC(im6402_device::write_rri));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);

//  MCFG_MC14412_ADD(MC14412_TAG, XTAL(1'000'000))

	/* option ROM cartridge */
	GENERIC_CARTSLOT(config, m_opt_cart, generic_linear_slot, "trsm100_cart", "bin,rom");

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("trsm100");

	/* internal ram */
	RAM(config, m_ram).set_default_size("8K").set_extra_options("16K,24K,32K");
}

void trsm100_state::tandy102(machine_config &config)
{
	trsm100(config);
	m_ram->set_default_size("24K").set_extra_options("32K");
}

void tandy200_state::tandy200(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, XTAL(4'915'200));
	m_maincpu->set_addrmap(AS_PROGRAM, &tandy200_state::tandy200_mem);
	m_maincpu->set_addrmap(AS_IO, &tandy200_state::tandy200_io);
	m_maincpu->in_sid_func().set(FUNC(tandy200_state::kc85_sid_r));
	m_maincpu->out_sod_func().set(FUNC(tandy200_state::kc85_sod_w));

	/* video hardware */
	tandy200_video(config);

	/* TP timer */
	TIMER(config, "tp").configure_periodic(FUNC(tandy200_state::tandy200_tp_tick), attotime::from_hz(XTAL(4'915'200)/2/8192));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.25);

//  MCFG_TCM5089_ADD(TCM5089_TAG, XTAL(3'579'545))

	/* devices */
	i8155_device &i8155(I8155(config, I8155_TAG, XTAL(4'915'200)/2));
	i8155.out_pa_callback().set(FUNC(tandy200_state::i8155_pa_w));
	i8155.out_pb_callback().set(FUNC(tandy200_state::i8155_pb_w));
	i8155.in_pc_callback().set(FUNC(tandy200_state::i8155_pc_r));
	i8155.out_to_callback().set(FUNC(tandy200_state::i8155_to_w));

	RP5C01(config, m_rtc, XTAL(32'768));

	i8251_device &i8251(I8251(config, I8251_TAG, 0)); /*XTAL(4'915'200)/2,*/
	i8251.txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	i8251.dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	i8251.rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, nullptr));
	rs232.rxd_handler().set(I8251_TAG, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(I8251_TAG, FUNC(i8251_device::write_dsr));

//  MCFG_MC14412_ADD(MC14412_TAG, XTAL(1'000'000))
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(tandy200_state::write_centronics_busy));
	m_centronics->select_handler().set(FUNC(tandy200_state::write_centronics_select));

	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);

	/* option ROM cartridge */
	GENERIC_CARTSLOT(config, m_opt_cart, generic_linear_slot, "tandy200_cart", "bin,rom");

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("tandy200");

	/* internal ram */
	RAM(config, m_ram).set_default_size("24K").set_extra_options("48K,72K");
}

/* ROMs */

ROM_START( kc85 )
	ROM_REGION( 0x8000, I8085_TAG, 0 )
	ROM_LOAD( "kc85rom.bin", 0x0000, 0x8000, CRC(8a9ddd6b) SHA1(9d18cb525580c9e071e23bc3c472380aa46356c0) )
ROM_END

ROM_START( pc8201 )
	ROM_REGION( 0x10000, I8085_TAG, 0 )
	ROM_LOAD( "3256a41-3b1 n 82 basic.rom0", 0x0000, 0x8000, CRC(3dbaa484) SHA1(9886a973faa639ca9e0ba478790bab20e5163495) )
ROM_END

ROM_START( pc8201a )
	ROM_REGION( 0x10000, I8085_TAG, 0 )
	ROM_LOAD( "pc8201rom.rom", 0x0000, 0x8000, CRC(30555035) SHA1(96f33ff235db3028bf5296052acedbc94437c596) )
ROM_END

ROM_START( npc8300 )
	ROM_REGION( 0x10000, I8085_TAG, 0 )
	ROM_LOAD( "831000-438_n83a_basic_1986_microsoft_8716_z01.bin", 0x0000, 0x8000, CRC(a3c15dcb) SHA1(f0322dfe3f2e951de043bf6d0973e6ffc2c87181))
ROM_END

ROM_START( trsm100 )
	/*
	    Board Code  ROM type            ROM Code            Comment
	    -------------------------------------------------------------------
	    PLX110CH1X  custom              LH535618            early North America
	    PLX110EH1X  27C256 compatible   3256C07-3J1/11US    late North America
	    PLX120CH1X  27C256 compatible   3256C05-3E1/11EP    European/Italian
	*/
	ROM_REGION( 0x8000, I8085_TAG, 0 )
	ROM_LOAD( "m100rom.m12",  0x0000, 0x8000, CRC(730a3611) SHA1(094dbc4ac5a4ea5cdf51a1ac581a40a9622bb25d) )
ROM_END

ROM_START( m10 )
	// 3256C02-4B3/I        Italian
	ROM_REGION( 0x8010, I8085_TAG, 0 )
	ROM_LOAD( "m10rom.m12", 0x0000, 0x8000, CRC(f0e8447a) SHA1(d58867276213116a79f7074109b7d7ce02e8a3af) )
ROM_END

ROM_START( tandy102 )
	ROM_REGION( 0x8000, I8085_TAG, 0 )
	ROM_LOAD( "m102rom.m12", 0x0000, 0x8000, CRC(08e9f89c) SHA1(b6ede7735a361c80419f4c9c0e36e7d480c36d11) )
ROM_END

ROM_START( tandy200 )
	ROM_REGION( 0x18000, I8085_TAG, 0 )
	ROM_LOAD( "rom 1-1.m15", 0x00000, 0x8000, NO_DUMP )
	ROM_LOAD( "rom 1-2.m13", 0x08000, 0x2000, NO_DUMP )
	ROM_LOAD( "rom 2.m14",   0x10000, 0x8000, NO_DUMP )
	ROM_LOAD( "t200rom.bin", 0x0000, 0xa000, BAD_DUMP CRC(e3358b38) SHA1(35d4e6a5fb8fc584419f57ec12b423f6021c0991) ) /* Y2K hacked */
	ROM_CONTINUE(           0x10000, 0x8000 )
ROM_END

/* System Drivers */

/*    YEAR  NAME      PARENT   COMPAT  MACHINE   INPUT    CLASS           INIT        COMPANY              FULLNAME */
COMP( 1983, kc85,     0,       0,      kc85,     kc85,    kc85_state,     empty_init, "Kyosei",            "Kyotronic 85 (Japan)", 0 )
COMP( 1983, m10,      kc85,    0,      kc85,     olivm10, kc85_state,     empty_init, "Olivetti",          "M-10",                 0 )
//COMP( 1983, m10m,     kc85,    0,      kc85,     olivm10, kc85_state,     empty_init, "Olivetti",          "M-10 Modem (US)",      0 )
COMP( 1983, trsm100,  0,       0,      trsm100,  kc85,    trsm100_state,  empty_init, "Tandy Radio Shack", "TRS-80 Model 100",     0 )
COMP( 1986, tandy102, trsm100, 0,      tandy102, kc85,    trsm100_state,  empty_init, "Tandy Radio Shack", "Tandy 102",            0 )
COMP( 1983, pc8201,   0,       0,      pc8201,   pc8201,  pc8201_state,   empty_init, "NEC",               "PC-8201 (Japan)",      MACHINE_NOT_WORKING ) // keyboard layout wrong
COMP( 1983, pc8201a,  pc8201,  0,      pc8201,   pc8201a, pc8201_state,   empty_init, "NEC",               "PC-8201A",             0 )
COMP( 1987, npc8300,  pc8201,  0,      pc8300,   pc8201a, pc8201_state,   empty_init, "NEC",               "PC-8300",              MACHINE_NOT_WORKING )
COMP( 1984, tandy200, 0,       0,      tandy200, kc85,    tandy200_state, empty_init, "Tandy Radio Shack", "Tandy 200",            0 )
