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
#include "sound/wave.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8251.h"

/* Devices */
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "formats/tzx_cas.h"
#include "machine/bankdev.h"

#include "screen.h"
#include "speaker.h"


class elwro800_state : public spectrum_state
{
public:
	elwro800_state(const machine_config &mconfig, device_type type, const char *tag) :
		spectrum_state(mconfig, type, tag),
		m_i8251(*this, "i8251"),
		m_i8255(*this, "ppi8255"),
		m_centronics(*this, "centronics"),
		m_upd765(*this, "upd765"),
		m_flop(*this, "upd765:%u", 0U),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_io_ports(*this, {"LINE7", "LINE6", "LINE5", "LINE4", "LINE3", "LINE2", "LINE1", "LINE0", "LINE8"}),
		m_io_line9(*this, "LINE9"),
		m_io_network_id(*this, "NETWORK ID")
	{
	}

	void elwro800(machine_config &config);

private:
	/* for elwro800 */
	/* RAM mapped at 0 */
	uint8_t m_ram_at_0000;

	/* NR signal */
	uint8_t m_NR;

	uint8_t nmi_r();
	DECLARE_WRITE8_MEMBER(elwro800jr_fdc_control_w);
	DECLARE_READ8_MEMBER(elwro800jr_io_r);
	DECLARE_WRITE8_MEMBER(elwro800jr_io_w);
	DECLARE_MACHINE_RESET(elwro800);
	INTERRUPT_GEN_MEMBER(elwro800jr_interrupt);
	DECLARE_READ8_MEMBER(i8255_port_c_r);
	DECLARE_WRITE8_MEMBER(i8255_port_c_w);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_ack);

	void elwro800_bank1(address_map &map);
	void elwro800_bank2(address_map &map);
	void elwro800_io(address_map &map);
	void elwro800_m1(address_map &map);
	void elwro800_mem(address_map &map);

	required_device<i8251_device> m_i8251;
	required_device<i8255_device> m_i8255;
	required_device<centronics_device> m_centronics;
	required_device<upd765a_device> m_upd765;
	required_device_array<floppy_connector, 2> m_flop;
	required_device<address_map_bank_device> m_bank1;
	required_device<address_map_bank_device> m_bank2;
	required_ioport_array<9> m_io_ports;
	required_ioport m_io_line9;
	required_ioport m_io_network_id;

	void elwro800jr_mmu_w(uint8_t data);

	int m_centronics_ack;
};


/*************************************
 *
 * When RAM is mapped at 0x0000 - 0x1fff (in CP/J mode), reading a location 66 with /M1=0
 * (effectively reading NMI vector) is hardwired to return 0xDF (RST #18)
 * (note that in CP/J mode address 66 is used for FCB)
 *
 *************************************/

uint8_t elwro800_state::nmi_r()
{
	if (m_ram_at_0000)
		return 0xdf;
	else
		return m_bank1->read8(0x66);
}

/*************************************
 *
 *  UPD765/Floppy drive
 *
 *************************************/

WRITE8_MEMBER(elwro800_state::elwro800jr_fdc_control_w)
{
	for (int i = 0; i < 2; i++)
		if (m_flop[i]->get_device())
			m_flop[i]->get_device()->mon_w(!BIT(data, i));

	m_upd765->tc_w(data & 0x04);

	if(!(data & 8))
		m_upd765->reset();
}

/*************************************
 *
 *  I/O port F7: memory mapping
 *
 *************************************/

void elwro800_state::elwro800jr_mmu_w(uint8_t data)
{
	uint8_t *prom = memregion("proms")->base() + 0x200;
	uint8_t *messram = m_ram->pointer();
	uint8_t cs;
	uint8_t ls175;

	ls175 = bitswap<8>(data, 7, 6, 5, 4, 4, 5, 7, 6) & 0x0f;

	cs = prom[((0x0000 >> 10) | (ls175 << 6)) & 0x1ff];
	if (!BIT(cs,0))
	{
		// rom BAS0
		m_bank1->set_bank(1);
		m_ram_at_0000 = 0;
	}
	else if (!BIT(cs,4))
	{
		// rom BOOT
		m_bank1->set_bank(2);
		m_ram_at_0000 = 0;
	}
	else
	{
		// RAM
		m_bank1->set_bank(0);
		m_ram_at_0000 = 1;
	}

	cs = prom[((0x2000 >> 10) | (ls175 << 6)) & 0x1ff];
	if (!BIT(cs,1))
	{
		m_bank2->set_bank(1); // BAS1 ROM
	}
	else
	{
		m_bank2->set_bank(0); // RAM
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
	uint8_t *prom = memregion("proms")->base();
	uint8_t cs = prom[offset & 0x1ff];

	if (!BIT(cs,0))
	{
		// CFE
		int mask = 0x8000;
		int data = 0xff;
		int i;

		if ( !m_NR )
		{
			for (i = 0; i < 9; mask >>= 1, i++)
			{
				if (!(offset & mask))
				{
					data &= m_io_ports[i]->read();
				}
			}

			if ((offset & 0xff) == 0xfb)
			{
				data &= m_io_line9->read();
			}

			/* cassette input from wav */
			if (m_cassette->input() > 0.0038 )
			{
				data |= 0x40;
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
		return m_i8255->read((offset & 0x03) ^ 0x03);
	}
	else if (!BIT(cs,3))
	{
		// CSFDC
		if (offset & 1)
		{
			return m_upd765->fifo_r();
		}
		else
		{
			return m_upd765->msr_r();
		}
	}
	else if (!BIT(cs,4))
	{
		// CS51
		return m_i8251->read(offset & 1);
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
	uint8_t *prom = memregion("proms")->base();
	uint8_t cs = prom[offset & 0x1ff];

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
		m_i8255->write((offset & 0x03) ^ 0x03, data);
	}
	else if (!BIT(cs,3))
	{
		// CSFDC
		if (offset & 1)
		{
			m_upd765->fifo_w(data);
		}
	}
	else if (!BIT(cs,4))
	{
		// CS51
		m_i8251->write(offset & 1, data);
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

void elwro800_state::elwro800_mem(address_map &map)
{
	map(0x0000, 0x1fff).m(m_bank1, FUNC(address_map_bank_device::amap8));
	map(0x2000, 0x3fff).m(m_bank2, FUNC(address_map_bank_device::amap8));
	map(0x4000, 0xffff).bankrw("rambank3");
}

void elwro800_state::elwro800_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(elwro800_state::elwro800jr_io_r), FUNC(elwro800_state::elwro800jr_io_w));
}

void elwro800_state::elwro800_m1(address_map &map)
{
	map(0x0000, 0x1fff).m(m_bank1, FUNC(address_map_bank_device::amap8));
	map(0x0066, 0x0066).r(FUNC(elwro800_state::nmi_r));
	map(0x2000, 0x3fff).m(m_bank2, FUNC(address_map_bank_device::amap8));
	map(0x4000, 0xffff).bankrw("rambank3");
}

void elwro800_state::elwro800_bank1(address_map &map)
{
	map(0x0000, 0x1fff).bankrw("rambank1");
	map(0x2000, 0x3fff).rom().region("maincpu", 0x0000).nopw(); // BAS0 ROM
	map(0x4000, 0x5fff).rom().region("maincpu", 0x4000).nopw(); // BOOT ROM
}

void elwro800_state::elwro800_bank2(address_map &map)
{
	map(0x0000, 0x1fff).bankrw("rambank2");
	map(0x2000, 0x3fff).rom().region("maincpu", 0x2000).nopw(); // BAS1 ROM
}

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
	uint8_t *messram = m_ram->pointer();

	memset(messram, 0, 64*1024);

	membank("rambank1")->set_base(messram + 0x0000);
	membank("rambank2")->set_base(messram + 0x2000);
	membank("rambank3")->set_base(messram + 0x4000);

	m_port_7ffd_data = 0;
	m_port_1ffd_data = -1;

	// this is a reset of ls175 in mmu
	elwro800jr_mmu_w(0);
}

INTERRUPT_GEN_MEMBER(elwro800_state::elwro800jr_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

static void elwro800jr_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
}

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

static GFXDECODE_START( gfx_elwro800 )
	GFXDECODE_ENTRY( "maincpu", 0x3c00, elwro800_charlayout, 0, 8 )
GFXDECODE_END


void elwro800_state::elwro800(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 14_MHz_XTAL / 4);    /* 3.5 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &elwro800_state::elwro800_mem);
	m_maincpu->set_addrmap(AS_IO, &elwro800_state::elwro800_io);
	m_maincpu->set_addrmap(AS_OPCODES, &elwro800_state::elwro800_m1);
	m_maincpu->set_vblank_int("screen", FUNC(elwro800_state::elwro800jr_interrupt));

	MCFG_MACHINE_RESET_OVERRIDE(elwro800_state,elwro800)

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14_MHz_XTAL / 2, 448, 0, SPEC_SCREEN_WIDTH, 312, 0, SPEC_SCREEN_HEIGHT);
	// Sync and interrupt timings determined by 2716 EPROM
	screen.set_screen_update(FUNC(elwro800_state::screen_update_spectrum));
	screen.screen_vblank().set(FUNC(elwro800_state::screen_vblank_spectrum));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(elwro800_state::spectrum_palette), 16);
	GFXDECODE(config, "gfxdecode", "palette", gfx_elwro800);

	MCFG_VIDEO_START_OVERRIDE(elwro800_state, spectrum)

	UPD765A(config, "upd765", 8_MHz_XTAL / 2, true, true);

	I8255A(config, m_i8255, 0);
	m_i8255->in_pa_callback().set_ioport("JOY");
	m_i8255->in_pb_callback().set("cent_data_in", FUNC(input_buffer_device::bus_r));
	m_i8255->out_pb_callback().set("cent_data_out", FUNC(output_latch_device::bus_w));
	m_i8255->in_pc_callback().set(FUNC(elwro800_state::i8255_port_c_r));
	m_i8255->out_pc_callback().set(FUNC(elwro800_state::i8255_port_c_w));

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->set_data_input_buffer("cent_data_in");
	m_centronics->ack_handler().set(FUNC(elwro800_state::write_centronics_ack));

	INPUT_BUFFER(config, "cent_data_in");
	output_latch_device &cent_data_out(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(cent_data_out);

	I8251(config, m_i8251, 14_MHz_XTAL / 4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	WAVE(config, "wave", m_cassette).add_route(ALL_OUTPUTS, "mono", 0.25);
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	CASSETTE(config, m_cassette);
	m_cassette->set_formats(tzx_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);

	FLOPPY_CONNECTOR(config, "upd765:0", elwro800jr_floppies, "525hd", floppy_image_device::default_floppy_formats);
	FLOPPY_CONNECTOR(config, "upd765:1", elwro800jr_floppies, "525hd", floppy_image_device::default_floppy_formats);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");

	ADDRESS_MAP_BANK(config, "bank1").set_map(&elwro800_state::elwro800_bank1).set_data_width(8).set_stride(0x2000);
	ADDRESS_MAP_BANK(config, "bank2").set_map(&elwro800_state::elwro800_bank2).set_data_width(8).set_stride(0x2000);
}

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

	ROM_REGION(0x0c00, "proms", 0 )
	ROM_LOAD( "junior_io_prom.bin",  0x0000, 0x0200, CRC(c6a777c4) SHA1(41debc1b4c3bd4eef7e0e572327c759e0399a49c) )
	ROM_LOAD( "junior_mem_prom.bin", 0x0200, 0x0200, CRC(0f745f42) SHA1(360ec23887fb6d7e19ee85d2bb30d9fa57f4936e) )
	ROM_LOAD( "tv_2716.e11",         0x0400, 0x0800, CRC(6093e80e) SHA1(a4972f336490d15222f4f24369f1f3253cfb9516) )
ROM_END

/* Driver */

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME      FLAGS
COMP( 1986, elwro800, 0,      0,      elwro800, elwro800, elwro800_state, empty_init, "Elwro", "800 Junior", 0 )
