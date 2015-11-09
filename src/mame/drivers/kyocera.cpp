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

******************************************************************************************/

/*

    TODO:

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

    10 FOR A=0 TO 255
    20 PRINT CHR$(A);
    30 NEXT A

*/

/*

                          * PC-8201/8300 HARDWARE PORT DEFINITIONS *

                -Port-
    Name       Hex  Dec   Notes
    --------   ---  ---   -----------------------------------------
    A8255      070  112   Video interface port A (8255)
    B8255      071  113   Video interface port B (8255)
    C8255      072  114   Video interface port C (8255)
    CW8255     073  115   Video interface command/mode port (8255)

*/


#include "includes/kyocera.h"
#include "softlist.h"

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

void pc8201_state::bankswitch(UINT8 data)
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

	UINT8 data = 0x40;

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

	UINT8 data = 0x40;

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
	UINT8 data = 0xff;

	if (m_rom_sel)
		data = m_cas_cart->read_rom(space, m_rom_addr & 0x1ffff);

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
	UINT8 data = 0xff;

	if (!BIT(m_keylatch, 0)) data &= m_y0->read();
	if (!BIT(m_keylatch, 1)) data &= m_y1->read();
	if (!BIT(m_keylatch, 2)) data &= m_y2->read();
	if (!BIT(m_keylatch, 3)) data &= m_y3->read();
	if (!BIT(m_keylatch, 4)) data &= m_y4->read();
	if (!BIT(m_keylatch, 5)) data &= m_y5->read();
	if (!BIT(m_keylatch, 6)) data &= m_y6->read();
	if (!BIT(m_keylatch, 7)) data &= m_y7->read();
	if (!BIT(m_keylatch, 8)) data &= m_y8->read();

	return data;
}

void tandy200_state::bankswitch(UINT8 data)
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
	UINT8 data = 0xff;

	if (!BIT(m_keylatch, 0)) data &= m_y0->read();
	if (!BIT(m_keylatch, 1)) data &= m_y1->read();
	if (!BIT(m_keylatch, 2)) data &= m_y2->read();
	if (!BIT(m_keylatch, 3)) data &= m_y3->read();
	if (!BIT(m_keylatch, 4)) data &= m_y4->read();
	if (!BIT(m_keylatch, 5)) data &= m_y5->read();
	if (!BIT(m_keylatch, 6)) data &= m_y6->read();
	if (!BIT(m_keylatch, 7)) data &= m_y7->read();
	if (!BIT(m_keylatch, 8)) data &= m_y8->read();

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
	UINT8 data = 0;

	data |= m_lcdc0->read(space, offset);
	data |= m_lcdc1->read(space, offset);
	data |= m_lcdc2->read(space, offset);
	data |= m_lcdc3->read(space, offset);
	data |= m_lcdc4->read(space, offset);
	data |= m_lcdc5->read(space, offset);
	data |= m_lcdc6->read(space, offset);
	data |= m_lcdc7->read(space, offset);
	data |= m_lcdc8->read(space, offset);
	data |= m_lcdc9->read(space, offset);

	return data;
}

WRITE8_MEMBER( kc85_state::lcd_w )
{
	m_lcdc0->write(space, offset, data);
	m_lcdc1->write(space, offset, data);
	m_lcdc2->write(space, offset, data);
	m_lcdc3->write(space, offset, data);
	m_lcdc4->write(space, offset, data);
	m_lcdc5->write(space, offset, data);
	m_lcdc6->write(space, offset, data);
	m_lcdc7->write(space, offset, data);
	m_lcdc8->write(space, offset, data);
	m_lcdc9->write(space, offset, data);
}

/* Memory Maps */

static ADDRESS_MAP_START( kc85_mem, AS_PROGRAM, 8, kc85_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_RAMBANK("bank2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc8201_mem, AS_PROGRAM, 8, pc8201_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_RAMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_RAMBANK("bank2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tandy200_mem, AS_PROGRAM, 8, tandy200_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xffff) AM_RAMBANK("bank2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( kc85_io, AS_IO, 8, kc85_state )
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(0x70, 0x70) AM_MIRROR(0x0f) optional RAM unit
//  AM_RANGE(0x80, 0x80) AM_MIRROR(0x0f) optional I/O controller unit
//  AM_RANGE(0x90, 0x90) AM_MIRROR(0x0f) optional answering telephone unit
//  AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0f) optional modem
	AM_RANGE(0xb0, 0xb7) AM_MIRROR(0x08) AM_DEVREADWRITE(I8155_TAG, i8155_device, io_r, io_w)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x0f) AM_DEVREADWRITE(IM6402_TAG, im6402_device, read, write)
	AM_RANGE(0xd0, 0xd0) AM_MIRROR(0x0f) AM_READWRITE(uart_status_r, uart_ctrl_w)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x0f) AM_READWRITE(keyboard_r, ctrl_w)
	AM_RANGE(0xf0, 0xf1) AM_MIRROR(0x0e) AM_READWRITE(lcd_r, lcd_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( trsm100_io, AS_IO, 8, kc85_state )
	AM_IMPORT_FROM(kc85_io)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0f) AM_WRITE(modem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc8201_io, AS_IO, 8, pc8201_state )
	ADDRESS_MAP_UNMAP_HIGH
//  AM_RANGE(0x70, 0x70) AM_MIRROR(0x0f) optional video interface 8255
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x03) AM_WRITE(romah_w)
	AM_RANGE(0x84, 0x84) AM_MIRROR(0x03) AM_WRITE(romal_w)
	AM_RANGE(0x88, 0x88) AM_MIRROR(0x03) AM_WRITE(romam_w)
	AM_RANGE(0x8c, 0x8c) AM_MIRROR(0x03) AM_READ(romrd_r)
	AM_RANGE(0x90, 0x90) AM_MIRROR(0x0f) AM_WRITE(scp_w)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0f) AM_READWRITE(bank_r, bank_w)
	AM_RANGE(0xb0, 0xb7) AM_MIRROR(0x08) AM_DEVREADWRITE(I8155_TAG, i8155_device, io_r, io_w)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x0f) AM_DEVREADWRITE(IM6402_TAG, im6402_device, read, write)
	AM_RANGE(0xd0, 0xd0) AM_MIRROR(0x0f) AM_READ(uart_status_r) AM_WRITE(uart_ctrl_w)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x0f) AM_READ(keyboard_r)
	AM_RANGE(0xf0, 0xf1) AM_MIRROR(0x0e) AM_READWRITE(lcd_r, lcd_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tandy200_io, AS_IO, 8, tandy200_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x90, 0x9f) AM_DEVREADWRITE(RP5C01A_TAG, rp5c01_device, read, write)
//  AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0f) AM_DEVWRITE(TCM5089_TAG, write)
	AM_RANGE(0xb0, 0xb7) AM_MIRROR(0x08) AM_DEVREADWRITE(I8155_TAG, i8155_device, io_r, io_w)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x0e) AM_DEVREADWRITE(I8251_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0xc1, 0xc1) AM_MIRROR(0x0e) AM_DEVREADWRITE(I8251_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0xd0, 0xd0) AM_MIRROR(0x0f) AM_READWRITE(bank_r, bank_w)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x0f) AM_READWRITE(stbk_r, stbk_w)
	AM_RANGE(0xf0, 0xf0) AM_MIRROR(0x0e) AM_DEVREADWRITE(HD61830_TAG, hd61830_device, data_r, data_w)
	AM_RANGE(0xf1, 0xf1) AM_MIRROR(0x0e) AM_DEVREADWRITE(HD61830_TAG, hd61830_device, status_r, control_w)
ADDRESS_MAP_END

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
	m_lcdc0->cs2_w(BIT(data, 0));
	m_lcdc1->cs2_w(BIT(data, 1));
	m_lcdc2->cs2_w(BIT(data, 2));
	m_lcdc3->cs2_w(BIT(data, 3));
	m_lcdc4->cs2_w(BIT(data, 4));
	m_lcdc5->cs2_w(BIT(data, 5));
	m_lcdc6->cs2_w(BIT(data, 6));
	m_lcdc7->cs2_w(BIT(data, 7));

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
	m_lcdc8->cs2_w(BIT(data, 0));
	m_lcdc9->cs2_w(BIT(data, 1));

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

	UINT8 data = 0;

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

	m_cent_data_out->write(space, 0, data);

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

	UINT8 data = 0x01;

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
	UINT8 *ram = m_ram->pointer();

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
	return m_cassette->input() > 0.0;
}

WRITE_LINE_MEMBER( tandy200_state::kc85_sod_w )
{
	m_cassette->output(state ? +1.0 : -1.0);
}

READ_LINE_MEMBER( tandy200_state::kc85_sid_r )
{
	return m_cassette->input() > 0.0;
}

TIMER_DEVICE_CALLBACK_MEMBER(tandy200_state::tandy200_tp_tick)
{
	m_maincpu->set_input_line(I8085_RST75_LINE, m_tp);

	m_tp = !m_tp;
}

static MACHINE_CONFIG_START( kc85, kc85_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(I8085_TAG, I8085A, XTAL_4_9152MHz)
	MCFG_CPU_PROGRAM_MAP(kc85_mem)
	MCFG_CPU_IO_MAP(kc85_io)
	MCFG_I8085A_SID(READLINE(kc85_state,kc85_sid_r))
	MCFG_I8085A_SOD(WRITELINE(kc85_state,kc85_sod_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD(kc85_video)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_DEVICE_ADD(I8155_TAG, I8155, XTAL_4_9152MHz/2)
	MCFG_I8155_OUT_PORTA_CB(WRITE8(kc85_state, i8155_pa_w))
	MCFG_I8155_OUT_PORTB_CB(WRITE8(kc85_state, i8155_pb_w))
	MCFG_I8155_IN_PORTC_CB(READ8(kc85_state, i8155_pc_r))
	MCFG_I8155_OUT_TIMEROUT_CB(WRITELINE(kc85_state, i8155_to_w))

	MCFG_UPD1990A_ADD(UPD1990A_TAG, XTAL_32_768kHz, NULL, INPUTLINE(I8085_TAG, I8085_RST75_LINE))

	MCFG_IM6402_ADD(IM6402_TAG, 0, 0)
	MCFG_IM6402_TRO_CALLBACK(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(IM6402_TAG, im6402_device, write_rri))

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(kc85_state, write_centronics_busy))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(kc85_state, write_centronics_select))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)

	/* option ROM cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("opt_cartslot", generic_linear_slot, "trsm100_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "trsm100")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pc8201, pc8201_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(I8085_TAG, I8085A, XTAL_4_9152MHz)
	MCFG_CPU_PROGRAM_MAP(pc8201_mem)
	MCFG_CPU_IO_MAP(pc8201_io)
	MCFG_I8085A_SID(READLINE(kc85_state,kc85_sid_r))
	MCFG_I8085A_SOD(WRITELINE(kc85_state,kc85_sod_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD(kc85_video)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_DEVICE_ADD(I8155_TAG, I8155, XTAL_4_9152MHz/2)
	MCFG_I8155_OUT_PORTA_CB(WRITE8(kc85_state, i8155_pa_w))
	MCFG_I8155_OUT_PORTB_CB(WRITE8(kc85_state, i8155_pb_w))
	MCFG_I8155_IN_PORTC_CB(READ8(kc85_state, i8155_pc_r))
	MCFG_I8155_OUT_TIMEROUT_CB(WRITELINE(kc85_state, i8155_to_w))

	MCFG_UPD1990A_ADD(UPD1990A_TAG, XTAL_32_768kHz, NULL, INPUTLINE(I8085_TAG, I8085_RST75_LINE))

	MCFG_IM6402_ADD(IM6402_TAG, 0, 0)
	MCFG_IM6402_TRO_CALLBACK(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(IM6402_TAG, im6402_device, write_rri))

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(kc85_state, write_centronics_busy))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(kc85_state, write_centronics_select))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)

	/* option ROM cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("opt_cartslot", generic_linear_slot, "pc8201_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	/* 128KB ROM cassette */
	MCFG_GENERIC_CARTSLOT_ADD("cas_cartslot", generic_linear_slot, "pc8201_cart2")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "pc8201")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K,64K,96K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pc8300, pc8201 )
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("32K")
	MCFG_RAM_EXTRA_OPTIONS("64K,96K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( trsm100, trsm100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(I8085_TAG, I8085A, XTAL_4_9152MHz)
	MCFG_CPU_PROGRAM_MAP(kc85_mem)
	MCFG_CPU_IO_MAP(trsm100_io)
	MCFG_I8085A_SID(READLINE(kc85_state,kc85_sid_r))
	MCFG_I8085A_SOD(WRITELINE(kc85_state,kc85_sod_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD(kc85_video)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_DEVICE_ADD(I8155_TAG, I8155, XTAL_4_9152MHz/2)
	MCFG_I8155_OUT_PORTA_CB(WRITE8(kc85_state, i8155_pa_w))
	MCFG_I8155_OUT_PORTB_CB(WRITE8(kc85_state, i8155_pb_w))
	MCFG_I8155_IN_PORTC_CB(READ8(kc85_state, i8155_pc_r))
	MCFG_I8155_OUT_TIMEROUT_CB(WRITELINE(kc85_state, i8155_to_w))

	MCFG_UPD1990A_ADD(UPD1990A_TAG, XTAL_32_768kHz, NULL, INPUTLINE(I8085_TAG, I8085_RST75_LINE))

	MCFG_IM6402_ADD(IM6402_TAG, 0, 0)
	MCFG_IM6402_TRO_CALLBACK(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(IM6402_TAG, im6402_device, write_rri))

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)

//  MCFG_MC14412_ADD(MC14412_TAG, XTAL_1MHz)

	/* option ROM cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("opt_cartslot", generic_linear_slot, "trsm100_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "trsm100")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8K")
	MCFG_RAM_EXTRA_OPTIONS("16K,24K,32K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tandy102, trsm100 )
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("24K")
	MCFG_RAM_EXTRA_OPTIONS("32K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( tandy200, tandy200_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(I8085_TAG, I8085A, XTAL_4_9152MHz)
	MCFG_CPU_PROGRAM_MAP(tandy200_mem)
	MCFG_CPU_IO_MAP(tandy200_io)
	MCFG_I8085A_SID(READLINE(tandy200_state,kc85_sid_r))
	MCFG_I8085A_SOD(WRITELINE(tandy200_state,kc85_sod_w))

	/* video hardware */
	MCFG_FRAGMENT_ADD(tandy200_video)

	/* TP timer */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("tp", tandy200_state, tandy200_tp_tick, attotime::from_hz(XTAL_4_9152MHz/2/8192))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
//  MCFG_TCM5089_ADD(TCM5089_TAG, XTAL_3_579545MHz)

	/* devices */
	MCFG_DEVICE_ADD(I8155_TAG, I8155, XTAL_4_9152MHz/2)
	MCFG_I8155_OUT_PORTA_CB(WRITE8(tandy200_state, i8155_pa_w))
	MCFG_I8155_OUT_PORTB_CB(WRITE8(tandy200_state, i8155_pb_w))
	MCFG_I8155_IN_PORTC_CB(READ8(tandy200_state, i8155_pc_r))
	MCFG_I8155_OUT_TIMEROUT_CB(WRITELINE(tandy200_state, i8155_to_w))

	MCFG_DEVICE_ADD(RP5C01A_TAG, RP5C01, XTAL_32_768kHz)

	MCFG_DEVICE_ADD(I8251_TAG, I8251, 0) /*XTAL_4_9152MHz/2,*/
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_dsr))

//  MCFG_MC14412_ADD(MC14412_TAG, XTAL_1MHz)
	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(tandy200_state, write_centronics_busy))
	MCFG_CENTRONICS_SELECT_HANDLER(WRITELINE(tandy200_state, write_centronics_select))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)

	/* option ROM cartridge */
	MCFG_GENERIC_CARTSLOT_ADD("opt_cartslot", generic_linear_slot, "tandy200_cart")
	MCFG_GENERIC_EXTENSIONS("bin,rom")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list", "tandy200")

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("24K")
	MCFG_RAM_EXTRA_OPTIONS("48K,72K")
MACHINE_CONFIG_END

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
	ROM_LOAD( "rom #1-1.m15", 0x00000, 0x8000, NO_DUMP )
	ROM_LOAD( "rom #1-2.m13", 0x08000, 0x2000, NO_DUMP )
	ROM_LOAD( "rom #2.m14",   0x10000, 0x8000, NO_DUMP )
	ROM_LOAD( "t200rom.bin", 0x0000, 0xa000, BAD_DUMP CRC(e3358b38) SHA1(35d4e6a5fb8fc584419f57ec12b423f6021c0991) ) /* Y2K hacked */
	ROM_CONTINUE(           0x10000, 0x8000 )
ROM_END

/* System Drivers */

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT    COMPANY                 FULLNAME */
COMP( 1983, kc85,       0,      0,      kc85,       kc85,       driver_device, 0,      "Kyosei",                    "Kyotronic 85 (Japan)",     0 )
COMP( 1983, m10,        kc85,   0,      kc85,       olivm10,    driver_device, 0,      "Olivetti",                  "M-10",                     0 )
//COMP( 1983, m10m,     kc85,   0,      kc85,       olivm10,    driver_device, 0,      "Olivetti",                  "M-10 Modem (US)",          0 )
COMP( 1983, trsm100,    0,      0,      trsm100,    kc85,       driver_device, 0,      "Tandy Radio Shack",         "TRS-80 Model 100",         0 )
COMP( 1986, tandy102,   trsm100,0,      tandy102,   kc85,       driver_device, 0,      "Tandy Radio Shack",         "Tandy 102",                0 )
COMP( 1983, pc8201,     0,      0,      pc8201,     pc8201,     driver_device, 0,      "Nippon Electronic Company", "PC-8201 (Japan)",          MACHINE_NOT_WORKING ) // keyboard layout wrong
COMP( 1983, pc8201a,    pc8201, 0,      pc8201,     pc8201a,    driver_device, 0,      "Nippon Electronic Company", "PC-8201A",                 0 )
COMP( 1987, npc8300,    pc8201, 0,      pc8300,     pc8201a,    driver_device, 0,      "Nippon Electronic Company", "PC-8300",                  MACHINE_NOT_WORKING )
COMP( 1984, tandy200,   0,      0,      tandy200,   kc85,       driver_device, 0,      "Tandy Radio Shack",         "Tandy 200",                0 )
