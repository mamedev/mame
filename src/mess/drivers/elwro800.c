// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/***************************************************************************

        Elwro 800 Junior

        Driver by Mariusz Wojcieszek

        ToDo:
        - 8251 DTR and DTS signals are connected (with some additional logic) to NMI of Z80, this
          is not emulated
        - 8251 is used for JUNET network (a network of Elwro 800 Junior computers, allows sharing
          floppy disc drives and printers) - network is not emulated

****************************************************************************/

#include "emu.h"
#include "includes/spectrum.h"

/* Components */
#include "cpu/z80/z80.h"
#include "machine/upd765.h" /* for floppy disc controller */
#include "machine/i8255.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8251.h"

/* Devices */
#include "imagedev/cassette.h"
#include "formats/tzx_cas.h"
#include "machine/ram.h"


class elwro800_state : public spectrum_state
{
public:
	elwro800_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_state(mconfig, type, tag),
		m_i8251(*this, "i8251"),
		m_i8255(*this, "ppi8255"),
		m_centronics(*this, "centronics"),
		m_io_line8(*this, "LINE8"),
		m_io_line9(*this, "LINE9"),
		m_io_network_id(*this, "NETWORK ID")
	{
	}

	/* for elwro800 */
	/* RAM mapped at 0 */
	UINT8 m_ram_at_0000;

	/* NR signal */
	UINT8 m_NR;
	UINT8 m_df_on_databus;

	DECLARE_DIRECT_UPDATE_MEMBER(elwro800_direct_handler);
	DECLARE_WRITE8_MEMBER(elwro800jr_fdc_control_w);
	DECLARE_READ8_MEMBER(elwro800jr_io_r);
	DECLARE_WRITE8_MEMBER(elwro800jr_io_w);
	DECLARE_MACHINE_RESET(elwro800);
	INTERRUPT_GEN_MEMBER(elwro800jr_interrupt);
	DECLARE_READ8_MEMBER(i8255_port_c_r);
	DECLARE_WRITE8_MEMBER(i8255_port_c_w);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_ack);

protected:
	required_device<i8251_device> m_i8251;
	required_device<i8255_device> m_i8255;
	required_device<centronics_device> m_centronics;
	required_ioport m_io_line8;
	required_ioport m_io_line9;
	required_ioport m_io_network_id;

	void elwro800jr_mmu_w(UINT8 data);

	int m_centronics_ack;
};


/*************************************
 *
 * When RAM is mapped at 0x0000 - 0x1fff (in CP/J mode), reading a location 66 with /M1=0
 * (effectively reading NMI vector) is hardwired to return 0xDF (RST #18)
 * (note that in CP/J mode address 66 is used for FCB)
 *
 *************************************/
DIRECT_UPDATE_MEMBER(elwro800_state::elwro800_direct_handler)
{
	if (m_ram_at_0000 && address == 0x66)
	{
		direct.explicit_configure(0x66, 0x66, 0, &m_df_on_databus);
		return ~0;
	}
	return address;
}

/*************************************
 *
 *  UPD765/Floppy drive
 *
 *************************************/

WRITE8_MEMBER(elwro800_state::elwro800jr_fdc_control_w)
{
	m_upd765_0->get_device()->mon_w(!BIT(data, 0));
	m_upd765_1->get_device()->mon_w(!BIT(data, 1));

	m_upd765->tc_w(data & 0x04);

	if(!(data & 8))
		m_upd765->reset();
}

/*************************************
 *
 *  I/O port F7: memory mapping
 *
 *************************************/

void elwro800_state::elwro800jr_mmu_w(UINT8 data)
{
	UINT8 *prom = memregion("proms")->base() + 0x200;
	UINT8 *messram = m_ram->pointer();
	UINT8 cs;
	UINT8 ls175;

	ls175 = BITSWAP8(data, 7, 6, 5, 4, 4, 5, 7, 6) & 0x0f;

	cs = prom[((0x0000 >> 10) | (ls175 << 6)) & 0x1ff];
	if (!BIT(cs,0))
	{
		// rom BAS0
		membank("bank1")->set_base(memregion("maincpu")->base() + 0x0000); /* BAS0 ROM */
		m_maincpu->space(AS_PROGRAM).nop_write(0x0000, 0x1fff);
		m_ram_at_0000 = 0;
	}
	else if (!BIT(cs,4))
	{
		// rom BOOT
		membank("bank1")->set_base(memregion("maincpu")->base() + 0x4000); /* BOOT ROM */
		m_maincpu->space(AS_PROGRAM).nop_write(0x0000, 0x1fff);
		m_ram_at_0000 = 0;
	}
	else
	{
		// RAM
		membank("bank1")->set_base(messram);
		m_maincpu->space(AS_PROGRAM).install_write_bank(0x0000, 0x1fff, "bank1");
		m_ram_at_0000 = 1;
	}

	cs = prom[((0x2000 >> 10) | (ls175 << 6)) & 0x1ff];
	if (!BIT(cs,1))
	{
		membank("bank2")->set_base(memregion("maincpu")->base() + 0x2000); /* BAS1 ROM */
		m_maincpu->space(AS_PROGRAM).nop_write(0x2000, 0x3fff);
	}
	else
	{
		membank("bank2")->set_base(messram + 0x2000); /* RAM */
		m_maincpu->space(AS_PROGRAM).install_write_bank(0x2000, 0x3fff, "bank2");
	}

	if (BIT(ls175,2))
	{
		// relok
		m_screen_location = messram + 0xe000;
	}
	else
	{
		m_screen_location = messram + 0x4000;
	}

	m_NR = BIT(ls175,3);
	if (BIT(ls175,3))
	{
		logerror("Reading network number\n");
	}
}

/*************************************
 *
 *  8255: joystick and Centronics printer connections
 *
 *************************************/

WRITE_LINE_MEMBER(elwro800_state::write_centronics_ack)
{
	m_centronics_ack = state;
	m_i8255->pc2_w(state);
}

READ8_MEMBER(elwro800_state::i8255_port_c_r)
{
	return m_centronics_ack << 2;
}

WRITE8_MEMBER(elwro800_state::i8255_port_c_w)
{
	m_centronics->write_strobe((data >> 7) & 0x01);
}

/*************************************
 *
 *  I/O reads and writes
 *
 *  I/O accesses are decoded by prom which uses 8 low address lines (A0-A8) as input
 *  and outputs chip select signals for system components. Standard addresses are:
 *
 *  0x1F: 8255 port A (joystick)
 *  0xBE: 8251 data (Junet network)
 *  0xBF: 8251 control/status (Junet network)
 *  0xDC: 8255 control
 *  0xDD: 8255 port C (centronics 7-strobe, 2-ack)
 *  0xDE: 8255 port B (centronics data)
 *  0xDF: 8255 port A (joystick)
 *  0xEE: FDC 765A status
 *  0xEF: FDC 765A command and data
 *  0xF1: FDC control (motor on/off)
 *  0xF7: memory banking
 *  0xFE (write): border color, speaker and tape (as in Spectrum)
 *  0x??FE, 0x??7F, 0x??7B (read): keyboard reading
 *************************************/

READ8_MEMBER(elwro800_state::elwro800jr_io_r)
{
	UINT8 *prom = memregion("proms")->base();
	UINT8 cs = prom[offset & 0x1ff];

	if (!BIT(cs,0))
	{
		// CFE
		int mask = 0x8000;
		int data = 0xff;
		int i;
		ioport_port *io_ports[9] = { m_io_line7, m_io_line6, m_io_line5, m_io_line4, m_io_line3, m_io_line2, m_io_line1, m_io_line0, m_io_line8 };

		if ( !m_NR )
		{
			for (i = 0; i < 9; mask >>= 1, i++)
			{
				if (!(offset & mask))
				{
					data &= io_ports[i]->read();
				}
			}

			if ((offset & 0xff) == 0xfb)
			{
				data &= m_io_line9->read();
			}

			/* cassette input from wav */
			if (m_cassette->input() > 0.0038 )
			{
				data &= ~0x40;
			}
		}
		else
		{
			data = m_io_network_id->read();
		}

		return data;
	}
	else if (!BIT(cs,1))
	{
		// CF7
	}
	else if (!BIT(cs,2))
	{
		// CS55
		return m_i8255->read(space, (offset & 0x03) ^ 0x03);
	}
	else if (!BIT(cs,3))
	{
		// CSFDC
		if (offset & 1)
		{
			return m_upd765->fifo_r(space, 0, 0xff);
		}
		else
		{
			return m_upd765->msr_r(space, 0, 0xff);
		}
	}
	else if (!BIT(cs,4))
	{
		// CS51
		if (offset & 1)
		{
			return m_i8251->status_r(space, 0);
		}
		else
		{
			return m_i8251->data_r(space, 0);
		}
	}
	else if (!BIT(cs,5))
	{
		// CF1
	}
	else
	{
		logerror("Unmapped I/O read: %04x\n", offset);
	}
	return 0x00;
}

WRITE8_MEMBER(elwro800_state::elwro800jr_io_w)
{
	UINT8 *prom = memregion("proms")->base();
	UINT8 cs = prom[offset & 0x1ff];

	if (!BIT(cs,0))
	{
		// CFE
		spectrum_port_fe_w(space, 0, data);
	}
	else if (!BIT(cs,1))
	{
		// CF7
		elwro800jr_mmu_w(data);
	}
	else if (!BIT(cs,2))
	{
		// CS55
		m_i8255->write(space, (offset & 0x03) ^ 0x03, data);
	}
	else if (!BIT(cs,3))
	{
		// CSFDC
		if (offset & 1)
		{
			m_upd765->fifo_w(space, 0, data, 0xff);
		}
	}
	else if (!BIT(cs,4))
	{
		// CS51
		if (offset & 1)
		{
			m_i8251->control_w(space, 0, data);
		}
		else
		{
			m_i8251->data_w(space, 0, data);
		}
	}
	else if (!BIT(cs,5))
	{
		// CF1
		elwro800jr_fdc_control_w(space, 0, data);
	}
	else
	{
		logerror("Unmapped I/O write: %04x %02x\n", offset, data);
	}
}

/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START(elwro800_mem, AS_PROGRAM, 8, elwro800_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAMBANK("bank1")
	AM_RANGE(0x2000, 0x3fff) AM_RAMBANK("bank2")
	AM_RANGE(0x4000, 0xffff) AM_RAMBANK("bank3")
ADDRESS_MAP_END

static ADDRESS_MAP_START(elwro800_io, AS_IO, 8, elwro800_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(elwro800jr_io_r, elwro800jr_io_w)
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( elwro800 )
	PORT_START("LINE0") /* 0xFEFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS SHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)     PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)     PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)     PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(":    *") PORT_CODE(KEYCODE_ASTERISK)     PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(";    +") PORT_CODE(KEYCODE_COLON)        PORT_CHAR(';')

	PORT_START("LINE1") /* 0xFDFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("-    =") PORT_CODE(KEYCODE_MINUS)        PORT_CHAR('-')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("[    {") PORT_CODE(KEYCODE_OPENBRACE)    PORT_CHAR('[')

	PORT_START("LINE2") /* 0xFBFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(".    >") PORT_CODE(KEYCODE_STOP)         PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(",    <") PORT_CODE(KEYCODE_COMMA)        PORT_CHAR(',')

	PORT_START("LINE3") /* 0xF7FE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1    !") PORT_CODE(KEYCODE_1)    PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2    @") PORT_CODE(KEYCODE_2)    PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3    #") PORT_CODE(KEYCODE_3)    PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4    $") PORT_CODE(KEYCODE_4)    PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5    %") PORT_CODE(KEYCODE_5)    PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("/    ?") PORT_CODE(KEYCODE_SLASH)        PORT_CHAR('/')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("@    \\") PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('@')

	PORT_START("LINE4") /* 0xEFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0    _") PORT_CODE(KEYCODE_0)    PORT_CHAR('0') PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9    )") PORT_CODE(KEYCODE_9)    PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8    (") PORT_CODE(KEYCODE_8)    PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7    '") PORT_CODE(KEYCODE_7)    PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6    &") PORT_CODE(KEYCODE_6)    PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_DEL)     PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("]    }") PORT_CODE(KEYCODE_CLOSEBRACE)   PORT_CHAR(']')

	PORT_START("LINE5") /* 0xDFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC)             PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE)        PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))

	PORT_START("LINE6") /* 0xBFFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB)                 PORT_CHAR('\t')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT LOCK") PORT_CODE(KEYCODE_CAPSLOCK)     PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("LINE7") /* 0x7FFE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SYMBOL SHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("^    -") PORT_CODE(KEYCODE_TILDE)    PORT_CHAR('^') PORT_CHAR('-')

	PORT_START("LINE8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC4\x98") PORT_CODE(KEYCODE_0_PAD) // LATIN CAPITAL LETTER E WITH OGONEK
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC3\x93") PORT_CODE(KEYCODE_1_PAD) // LATIN CAPITAL LETTER O WITH ACUTE
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC5\x9A") PORT_CODE(KEYCODE_2_PAD) // LATIN CAPITAL LETTER S WITH ACUTE
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC5\x81") PORT_CODE(KEYCODE_3_PAD) // LATIN CAPITAL LETTER L WITH STROKE
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC5\xB9") PORT_CODE(KEYCODE_4_PAD) // LATIN CAPITAL LETTER Z WITH ACUTE
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC5\x83") PORT_CODE(KEYCODE_5_PAD) // LATIN CAPITAL LETTER N WITH ACUTE
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC4\x84") PORT_CODE(KEYCODE_6_PAD) // LATIN CAPITAL LETTER A WITH OGONEK

	PORT_START("LINE9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DIR") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Down") PORT_CODE(KEYCODE_DOWN)    PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Up") PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT)  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT)    PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC4\x86") PORT_CODE(KEYCODE_7_PAD) // LATIN CAPITAL LETTER C WITH ACUTE
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xC5\xBB") PORT_CODE(KEYCODE_8_PAD) // LATIN CAPITAL LETTER Z WITH DOT ABOVE

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1 Joystick Right") PORT_CODE(JOYCODE_X_RIGHT_SWITCH)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1 Joystick Left") PORT_CODE(JOYCODE_X_LEFT_SWITCH)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1 Joystick Down") PORT_CODE(JOYCODE_Y_DOWN_SWITCH)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1 Joystick Up") PORT_CODE(JOYCODE_Y_UP_SWITCH)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P1 Joystick Fire") PORT_CODE(JOYCODE_BUTTON1)

	PORT_START("NETWORK ID")
	PORT_DIPNAME( 0x3f, 0x01, "Computer network ID" )
	PORT_DIPSETTING( 0x01, "1" )
	PORT_DIPSETTING( 0x10, "16" )
	PORT_DIPSETTING( 0x11, "17" )

INPUT_PORTS_END

/*************************************
 *
 *  Machine
 *
 *************************************/

MACHINE_RESET_MEMBER(elwro800_state,elwro800)
{
	UINT8 *messram = m_ram->pointer();

	m_df_on_databus = 0xdf;
	memset(messram, 0, 64*1024);

	membank("bank3")->set_base(messram + 0x4000);

	m_port_7ffd_data = 0;
	m_port_1ffd_data = -1;

	// this is a reset of ls175 in mmu
	elwro800jr_mmu_w(0);

	m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(elwro800_state::elwro800_direct_handler), this));
}

INTERRUPT_GEN_MEMBER(elwro800_state::elwro800jr_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

static SLOT_INTERFACE_START( elwro800jr_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END

/* F4 Character Displayer */
static const gfx_layout elwro800_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( elwro800 )
	GFXDECODE_ENTRY( "maincpu", 0x3c00, elwro800_charlayout, 0, 8 )
GFXDECODE_END


static MACHINE_CONFIG_START( elwro800, elwro800_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, 3500000)    /* 3.5 MHz */
	MCFG_CPU_PROGRAM_MAP(elwro800_mem)
	MCFG_CPU_IO_MAP(elwro800_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", elwro800_state,  elwro800jr_interrupt)

	MCFG_MACHINE_RESET_OVERRIDE(elwro800_state,elwro800)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50.08)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(SPEC_SCREEN_WIDTH, SPEC_SCREEN_HEIGHT)
	MCFG_SCREEN_VISIBLE_AREA(0, SPEC_SCREEN_WIDTH-1, 0, SPEC_SCREEN_HEIGHT-1)
	MCFG_SCREEN_UPDATE_DRIVER(elwro800_state, screen_update_spectrum )
	MCFG_SCREEN_VBLANK_DRIVER(elwro800_state, screen_eof_spectrum)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(elwro800_state, spectrum )
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", elwro800)

	MCFG_VIDEO_START_OVERRIDE(elwro800_state, spectrum )

	MCFG_UPD765A_ADD("upd765", true, true)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("JOY"))
	MCFG_I8255_IN_PORTB_CB(DEVREAD8("cent_data_in", input_buffer_device, read))
	MCFG_I8255_OUT_PORTB_CB(DEVWRITE8("cent_data_out", output_latch_device, write))
	MCFG_I8255_IN_PORTC_CB(READ8(elwro800_state, i8255_port_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(elwro800_state, i8255_port_c_w))

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_DATA_INPUT_BUFFER("cent_data_in")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(elwro800_state, write_centronics_ack))

	MCFG_DEVICE_ADD("cent_data_in", INPUT_BUFFER, 0)
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_DEVICE_ADD("i8251", I8251, 0)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_FORMATS(tzx_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)

	MCFG_FLOPPY_DRIVE_ADD("upd765:0", elwro800jr_floppies, "525hd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", elwro800jr_floppies, "525hd", floppy_image_device::default_floppy_formats)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( elwro800 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bas04.epr", 0x0000, 0x2000, CRC(6ab16f36) SHA1(49a19b279f311279c7fed3d2b3f207732d674c26) )
	ROM_LOAD( "bas14.epr", 0x2000, 0x2000, CRC(a743eb80) SHA1(3a300550838535b4adfe6d05c05fe0b39c47df16) )
	ROM_LOAD( "bootv.epr", 0x4000, 0x2000, CRC(de5fa37d) SHA1(4f203efe53524d84f69459c54b1a0296faa83fd9) )

	ROM_REGION(0x0400, "proms", 0 )
	ROM_LOAD( "junior_io_prom.bin", 0x0000, 0x0200,  CRC(c6a777c4) SHA1(41debc1b4c3bd4eef7e0e572327c759e0399a49c))
	ROM_LOAD( "junior_mem_prom.bin", 0x0200, 0x0200, CRC(0f745f42) SHA1(360ec23887fb6d7e19ee85d2bb30d9fa57f4936e))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1986, elwro800,  0,       0,  elwro800,   elwro800, driver_device,     0,  "Elwro",   "800 Junior",       0)
