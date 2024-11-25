// license:BSD-3-Clause
// copyright-holders: Jean-Francois DEL NERO
/***************************************************************************

    Minitel 2

    The Minitel is a small, on-line computer/Videotex terminal with multi-services that
    can be connected to any French telephone line. This terminal was widely used in France
    during the 80's and 90's.

    There are several models and versions. Most of them are based on 8051 compatible MCUs
    and EF9345 semi graphic video chip.

    The current implementation is a Minitel 2 from "La RADIOTECHNIQUE PORTENSEIGNE" / RPIC (Philips)
    More Minitel hardware related informations are available on this page :
    http://hxc2001.free.fr/minitel

    What is implemented and working :

    - Main MCU
    - Video output
    - Keyboard
    - 24C02 EPROM
    - Modem serial interface.

    What is implemented but not working :

    - The rear serial port.(Prise péri-informatique)
     (Internal 8051 serial port emulation missing).

    What is not yet implemented :

    - Sound output.
    - Screen should go blank when switched off

    The original firmware and the experimental demo ROM are currently both working.

    Please note the current special function keys assignation :

    F1 -> Suite
    F2 -> Retour
    F3 -> Envoi
    F4 -> Repetition
    F5 -> TEL
    F6 -> Guide
    F7 -> Sommaire
    F8 -> Connexion/Fin
    F10-> ON / OFF
    Alt Gr -> Fonction

    With the official ROM you need to press F10 to switch on the CRT.

    Modem and external port can be exported to the "modem" and "periinfo"
    interfaces.
    Example : Command line options to use to create a TCP socket linked to
    the modem port : "-modem null_modem -bitb socket.127.0.0.1:20000"
    Once mame started you can then send vdt files with netcat to this socket.

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/clock.h"
#include "machine/i2cmem.h"
#include "machine/timer.h"
#include "video/ef9345.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"

#include "logmacro.h"

namespace {

// 14174 Control register bits definition
enum
{
	CTRL_REG_MCBC      = 1 << 0,
	CTRL_REG_DTMF      = 1 << 1,
	CTRL_REG_CRTON     = 1 << 3,
	CTRL_REG_OPTO      = 1 << 4,
	CTRL_REG_LINERELAY = 1 << 5,
};

// 80C32 Port IO usage definitions
enum
{
	PORT_1_KBSERIN  = 1 << 0,
	PORT_1_MDM_DCDn = 1 << 1,
	PORT_1_MDM_PRD  = 1 << 2,
	PORT_1_MDM_TXD  = 1 << 3,
	PORT_1_MDM_RTS  = 1 << 4,
	PORT_1_KBLOAD   = 1 << 5,
	PORT_1_SCL      = 1 << 6,
	PORT_1_SDA      = 1 << 7,
};

enum
{
	PORT_3_SER_RXD = 1 << 0,
	PORT_3_SER_TXD = 1 << 1,
	PORT_3_SER_ZCO = 1 << 2,
	PORT_3_MDM_RXD = 1 << 3,
	PORT_3_SER_RDY = 1 << 5,
};

class minitel_state : public driver_device
{
public:
	minitel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ts9347(*this, "ts9347")
		, m_palette(*this, "palette")
		, m_i2cmem(*this, "i2cmem")
		, m_modem(*this, "modem")
		, m_serport(*this, "periinfo")
		, m_io_kbd(*this, "Y%u", 0)
	{
	}

	void minitel2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i80c32_device> m_maincpu;
	required_device<ts9347_device> m_ts9347;
	required_device<palette_device> m_palette;
	required_device<i2cmem_device> m_i2cmem;
	required_device<rs232_port_device> m_modem;
	required_device<rs232_port_device> m_serport;

	required_ioport_array<16> m_io_kbd;

	uint8_t port1 = 0, port3 = 0;

	int keyboard_para_ser = 0;
	uint8_t keyboard_x_row_reg = 0;

	uint8_t last_ctrl_reg = 0;

	int lineconnected = 0;
	int tonedetect = 0;

	TIMER_DEVICE_CALLBACK_MEMBER(minitel_scanline);

	void update_modem_state();

	void port1_w(uint8_t data);
	void port3_w(uint8_t data);
	uint8_t port1_r();
	uint8_t port3_r();

	void serial_rxd(int state);

	void dev_ctrl_reg_w(offs_t offset, uint8_t data);
	uint8_t dev_keyb_ser_r(offs_t offset);

	uint8_t ts9347_io_r(offs_t offset);
	void ts9347_io_w(offs_t offset, uint8_t data);

	void mem_prg(address_map &map) ATTR_COLD;
	void mem_io(address_map &map) ATTR_COLD;
};

void minitel_state::machine_start()
{
	m_palette->set_pen_color( 0, 0, 0, 0);
	m_palette->set_pen_color( 1, 86, 86, 86);
	m_palette->set_pen_color( 2, 172, 172, 172);
	m_palette->set_pen_color( 3, 255, 255, 255);
	m_palette->set_pen_color( 4, 44, 44, 44);
	m_palette->set_pen_color( 5, 86, 86, 86);
	m_palette->set_pen_color( 6, 172, 172, 172);
	m_palette->set_pen_color( 7, 255, 255, 255);
}

void minitel_state::port1_w(uint8_t data)
{
	LOG("port_w: write %02X to PORT1\n", data);

	if( (port1 ^ data) & PORT_1_KBSERIN )
	{
		LOG("PORT_1_KBSERIN : %d \n", data & PORT_1_KBSERIN );
	}

	if( (port1 ^ data) & PORT_1_MDM_DCDn )
	{
		LOG("PORT_1_MDM_DCD : %d \n", data & PORT_1_MDM_DCDn );
	}

	if( (port1 ^ data) & PORT_1_MDM_PRD )
	{
		LOG("PORT_1_MDM_PRD : %d \n", data & PORT_1_MDM_PRD );
	}

	if( (port1 ^ data) & PORT_1_MDM_TXD )
	{
		LOG("PORT_1_MDM_TXD : %d \n", data & PORT_1_MDM_TXD );
	}

	if(lineconnected)
	{
		m_modem->write_txd(!!(data & PORT_1_MDM_TXD));
	}

	if( (port1 ^ data) & PORT_1_MDM_RTS )
	{
		LOG("PORT_1_MDM_RTS : %d \n", data & PORT_1_MDM_RTS );
	}

	if( (port1 ^ data) & PORT_1_KBLOAD )
	{
		LOG("PORT_1_KBLOAD : %d PC:0x%x\n", data & PORT_1_KBLOAD, m_maincpu->pc() );

		if(data & PORT_1_KBLOAD)
			keyboard_para_ser = 1;
		else
			keyboard_para_ser = 0;
	}

	if( (port1 ^ data) & PORT_1_SCL )
	{
		LOG("PORT_1_SCL : %d \n", data & PORT_1_SCL );
		m_i2cmem->write_scl( (data & PORT_1_SCL) ? 1 : 0);
	}

	if( (port1 ^ data) & PORT_1_SDA )
	{
		LOG("PORT_1_SDA : %d \n", data & PORT_1_SDA );
		m_i2cmem->write_sda( (data & PORT_1_SDA) ? 1 : 0);
	}

	port1 = data;
}

void minitel_state::port3_w(uint8_t data)
{
	LOG("port_w: write %02X to PORT3\n", data);

	m_serport->write_txd( !!(data & PORT_3_SER_TXD) );

	port3 = (port3 & PORT_3_SER_RXD) | (data & ~PORT_3_SER_RXD);
}

void minitel_state::serial_rxd(int state)
{
	if (state)
		port3 |= PORT_3_SER_RXD;
	else
		port3 &= ~PORT_3_SER_RXD;
}


void minitel_state::update_modem_state()
{
	// 1300 Hz tone detection :  PORT_1_MDM_RTS = 1, CTRL_REG_DTMF = 0, CTRL_REG_MCBC = 0
	if(  ( last_ctrl_reg & CTRL_REG_LINERELAY ) &&
		 ( port1 & PORT_1_MDM_RTS ) &&
		!( last_ctrl_reg & CTRL_REG_DTMF ) &&
		!( last_ctrl_reg & CTRL_REG_MCBC ) )
	{
		tonedetect = 1;
	}
	else
	{
		tonedetect = 0;
	}

	// Main transmission mode :  PORT_1_MDM_RTS = 0, CTRL_REG_DTMF = 1, CTRL_REG_MCBC = 0
	if( last_ctrl_reg & CTRL_REG_LINERELAY )
		lineconnected = 1;
	else
		lineconnected = 0;
};

uint8_t minitel_state::port1_r()
{
	uint8_t data;

	LOG("port_r: read %02X from PORT1 - Keyboard -> %x\n", port1,((keyboard_x_row_reg>>7)&1));

	data = ( ( (port1 & (0xFE & ~PORT_1_SDA) ) | ( (keyboard_x_row_reg>>7)&1) ) );
	data |= (m_i2cmem->read_sda() ? PORT_1_SDA : 0);

	update_modem_state();

	if( lineconnected )
		data &= ~PORT_1_MDM_DCDn;
	else
		data |=  PORT_1_MDM_DCDn;

	return data;
}

uint8_t minitel_state::port3_r()
{
	uint8_t data;

	LOG("port_r: read %02X from PORT3\n", port3);

	update_modem_state();

	data = (port3 & ~(PORT_3_SER_RDY)); // External port ready state

	return data;
}

void minitel_state::dev_ctrl_reg_w(offs_t offset, uint8_t data)
{
	if( last_ctrl_reg != data)
	{
		LOG("minitel_state::hw_ctrl_reg : %x %x\n",offset, data);

		if( (last_ctrl_reg ^ data) & CTRL_REG_DTMF )
		{
			LOG("CTRL_REG_DTMF : %d \n", data & CTRL_REG_DTMF );
		}

		if( (last_ctrl_reg ^ data) & CTRL_REG_MCBC )
		{
			LOG("CTRL_REG_MCBC : %d \n", data & CTRL_REG_MCBC );
		}

		if( (last_ctrl_reg ^ data) & CTRL_REG_OPTO )
		{
			LOG("CTRL_REG_OPTO : %d \n", data & CTRL_REG_OPTO );
		}

		if( (last_ctrl_reg ^ data) & CTRL_REG_LINERELAY )
		{
			LOG("CTRL_REG_RELAY : %d \n", data & CTRL_REG_LINERELAY );
		}

		if( (last_ctrl_reg ^ data) & CTRL_REG_CRTON )
		{
			LOG("CTRL_REG_CRTON : %d \n", data & CTRL_REG_CRTON );
		}
	}

	last_ctrl_reg = data;
}

uint8_t minitel_state::dev_keyb_ser_r(offs_t offset)
{
	LOG("minitel_state::keyb read : %x\n",offset);

	if ( keyboard_para_ser )
	{
		// load the 4014 with the keyboard row state
		keyboard_x_row_reg = m_io_kbd[(offset>>8)&0xF]->read();
		LOG("4014 Load : 0x%.2X 0x%.2X\n",(offset>>8)&0xF,keyboard_x_row_reg);
	}
	else
	{
		//shift the keyboard register...
		keyboard_x_row_reg = keyboard_x_row_reg << 1;
	}

	return 0xFF;
}

uint8_t minitel_state::ts9347_io_r(offs_t offset)
{
	return m_ts9347->data_r(offset);
}

void minitel_state::ts9347_io_w(offs_t offset, uint8_t data)
{
	LOG("minitel_state::ts9347_io_w : %x %x\n",offset, data);

	m_ts9347->data_w(offset, data);
}

TIMER_DEVICE_CALLBACK_MEMBER(minitel_state::minitel_scanline)
{
	m_ts9347->update_scanline((uint16_t)param);
}

void minitel_state::mem_prg(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void minitel_state::mem_io(address_map &map)
{
	map(0x2000, 0x3fff).rw(FUNC(minitel_state::dev_keyb_ser_r), FUNC(minitel_state::dev_ctrl_reg_w));
	/* ts9347 */
	map(0x4000, 0x5ffF).rw(FUNC(minitel_state::ts9347_io_r), FUNC(minitel_state::ts9347_io_w));
}

/* Input ports */
static INPUT_PORTS_START( minitel2 )
	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('-')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Suite") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Retour") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Connexion/Fin") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Fonction") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT) // Right maj
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) // Left maj

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("Y3")

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Envoi") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Repetition") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("Y5")

	PORT_START("Y6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tel") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("Y7")

	PORT_START("Y8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(';')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(']')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BackS  Delete") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("Y9")

	PORT_START("Y10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('*')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('#')

	PORT_START("Y11")

	PORT_START("Y12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("On/Off") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("Y13")

	PORT_START("Y14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('[')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Guide") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sommaire") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("Y15")

INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( m_modem )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_75 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( m_serport )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void minitel_state::minitel2(machine_config &config)
{
	/* basic machine hardware */
	I80C32(config, m_maincpu, XTAL(14'318'181)); //verified on pcb
	m_maincpu->set_addrmap(AS_PROGRAM, &minitel_state::mem_prg);
	m_maincpu->set_addrmap(AS_IO, &minitel_state::mem_io);
	m_maincpu->port_in_cb<1>().set(FUNC(minitel_state::port1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(minitel_state::port1_w));
	m_maincpu->port_in_cb<3>().set(FUNC(minitel_state::port3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(minitel_state::port3_w));

	I2C_24C02(config, m_i2cmem);

	TS9347(config, m_ts9347, 0);
	m_ts9347->set_palette_tag(m_palette);

	TIMER(config, "minitel_sl", 0).configure_scanline(FUNC(minitel_state::minitel_scanline), "screen", 0, 10);

	RS232_PORT(config, m_modem, default_rs232_devices, nullptr);
	m_modem->rxd_handler().set_inputline(m_maincpu, MCS51_INT1_LINE).invert();
	m_modem->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(m_modem));

	RS232_PORT(config, m_serport, default_rs232_devices, nullptr);
	m_serport->rxd_handler().set(FUNC(minitel_state::serial_rxd));
	m_serport->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(m_serport));

	lineconnected = 0;

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update("ts9347", FUNC(ts9347_device::screen_update));
	screen.set_size(512, 312);
	screen.set_visarea(2, 512-10, 0, 278-1);

	PALETTE(config, m_palette).set_entries(8+1);

	// Send a fake 1300 Hz carrier (emulate the modem ZCO output)
	auto &fake_1300hz_clock(CLOCK(config, "fake_1300hz_clock", 1300));
	fake_1300hz_clock.set_pulse_width(attotime::from_usec(384));
	fake_1300hz_clock.signal_handler().set_inputline(m_maincpu, MCS51_INT0_LINE);
}

ROM_START( minitel2 )

	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("ft_bv4")

	ROM_SYSTEM_BIOS(0, "ft_bv4", "Minitel 2 ROM BV4")
	ROMX_LOAD( "minitel2_bv4.bin",   0x0000, 0x8000, CRC(8844a0a7) SHA1(d3e9079b080dbcee27ad870ec6c39ac42e7deacf), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "demov1", "Minitel 2 Demo")
	ROMX_LOAD( "demo_minitel.bin",   0x0000, 0x8000, CRC(607f2482) SHA1(7965edbef68e45d09dc67a4684da56003eff6328), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "ft_bv9", "Minitel 2 ROM Bv9")
	ROMX_LOAD( "bv9.1402",           0x0000, 0x8000, CRC(ace5d65e) SHA1(c8d589f8af6bd7d339964fdece937a76db972115), ROM_BIOS(2) )

	ROM_REGION( 0x4000, "ts9347", 0 )
	ROM_LOAD( "charset.rom", 0x0000, 0x2000, BAD_DUMP CRC(b2f49eb3) SHA1(d0ef530be33bfc296314e7152302d95fdf9520fc) )            // from dcvg5k
ROM_END

} // anonymous namespace

COMP( 1989, minitel2, 0, 0, minitel2, minitel2, minitel_state, empty_init, "Philips", "Minitel 2", MACHINE_NO_SOUND )
