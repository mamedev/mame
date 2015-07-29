// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Kosmos CP-1

        06/03/2012 Skeleton driver.

        on board there is also 8155
        KEYBOARD Membrane keyboard, 57 keys
        6 * 7 seg led display

****************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/i8155.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "cp1.lh"

class cp1_state : public driver_device
{
public:
	cp1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_i8155(*this, "i8155"),
		m_i8155_cp3(*this, "i8155_cp3"),
		m_cassette(*this, "cassette"),
		m_io_line0(*this, "LINE0"),
		m_io_line1(*this, "LINE1"),
		m_io_line2(*this, "LINE2"),
		m_io_line3(*this, "LINE3"),
		m_io_line4(*this, "LINE4"),
		m_io_config(*this, "CONFIG")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<i8155_device> m_i8155;
	required_device<i8155_device> m_i8155_cp3;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_io_line0;
	required_ioport m_io_line1;
	required_ioport m_io_line2;
	required_ioport m_io_line3;
	required_ioport m_io_line4;
	required_ioport m_io_config;

	virtual void machine_reset();
	DECLARE_READ8_MEMBER(port1_r);
	DECLARE_READ8_MEMBER(port2_r);
	DECLARE_READ8_MEMBER(getbus);
	DECLARE_READ8_MEMBER(t0_r);
	DECLARE_READ8_MEMBER(t1_r);
	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_WRITE8_MEMBER(port2_w);
	DECLARE_WRITE8_MEMBER(putbus);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload);

	DECLARE_READ8_MEMBER(i8155_read);
	DECLARE_WRITE8_MEMBER(i8155_write);
	DECLARE_WRITE8_MEMBER(i8155_porta_w);
	DECLARE_READ8_MEMBER(i8155_portb_r);
	DECLARE_WRITE8_MEMBER(i8155_portb_w);
	DECLARE_WRITE8_MEMBER(i8155_portc_w);

private:
	UINT8   m_7seg;
	UINT8   m_port2;
	UINT8   m_matrix;
};

READ8_MEMBER(cp1_state::port1_r)
{
	logerror("Read from expansion port 1\n");

	UINT8 data = 0;

	if (m_io_config->read() & 0x01)
		data |= (m_cassette->input() > 0.03) ? 0x80 : 0x00;

	return data;
}

WRITE8_MEMBER(cp1_state::port1_w)
{
	logerror("Write to expansion port 1 %x\n", data);

	if (m_io_config->read() & 0x01)
		m_cassette->output(data & 0x80 ? +1.0 : -1.0);
}

READ8_MEMBER(cp1_state::port2_r)
{
	// x--- ----   I8155 IO/M
	// -x-- ----   I8155 RESET
	// --x- ----   expansion port CE
	// ---x ----   I8155 CE
	// ---- xxxx   keyboard input

	ioport_port* portnames[] = { m_io_line0, m_io_line1, m_io_line2, m_io_line3, m_io_line4 };
	UINT8 data = 0;

	for(int i=0; i<5; i++)
		if (!(m_matrix & (1<<i)))
			data |= portnames[i]->read();

	return (data & 0x0f) | (m_port2 & 0xf0);
}

WRITE8_MEMBER(cp1_state::port2_w)
{
	if (data & 0x40)
	{
		m_i8155->reset();

		if (m_io_config->read() & 0x02)
			m_i8155_cp3->reset();
	}

	m_port2 = data;
}

READ8_MEMBER(cp1_state::t0_r)
{
	logerror("t0_r\n");
	return 0;
}

READ8_MEMBER(cp1_state::t1_r)
{
	logerror("t1_r\n");
	return 0;
}

READ8_MEMBER(cp1_state::getbus)
{
	logerror("getbus\n");
	return 0;
}

WRITE8_MEMBER(cp1_state::putbus)
{
	logerror("putbus\n");
}

READ8_MEMBER(cp1_state::i8155_read)
{
	UINT8 data = 0;

	if (!(m_port2 & 0x10))
	{
		m_i8155->ale_w(space, BIT(m_port2, 7), offset);
		data |= m_i8155->read(space, offset);
	}
	if ((m_io_config->read() & 0x02) && !(m_port2 & 0x20))
	{
		// CP3 RAM expansion
		m_i8155_cp3->ale_w(space, BIT(m_port2, 7), offset);
		data |= m_i8155_cp3->read(space, offset);
	}

	return data;
}

WRITE8_MEMBER(cp1_state::i8155_write)
{
	if (!(m_port2 & 0x10))
	{
		m_i8155->ale_w(space, BIT(m_port2, 7), offset);
		m_i8155->write(space, offset, data);
	}
	if ((m_io_config->read() & 0x02) && !(m_port2 & 0x20))
	{
		// CP3 RAM expansion
		m_i8155_cp3->ale_w(space, BIT(m_port2, 7), offset);
		m_i8155_cp3->write(space, offset, data);
	}
}

WRITE8_MEMBER(cp1_state::i8155_porta_w)
{
	data &= 0x7f;   // PA7 is not connected

	if (m_7seg)
	{
		if (!(m_matrix & 0x01))     output_set_digit_value(5, data);
		if (!(m_matrix & 0x02))     output_set_digit_value(4, data);
		if (!(m_matrix & 0x04))     output_set_digit_value(3, data);
		if (!(m_matrix & 0x08))     output_set_digit_value(2, data | 0x80);     // this digit has always the dot active
		if (!(m_matrix & 0x10))     output_set_digit_value(1, data);
		if (!(m_matrix & 0x20))     output_set_digit_value(0, data);
	}

	m_7seg ^= 0x01;
}

READ8_MEMBER(cp1_state::i8155_portb_r)
{
	logerror("read from expansion port 2\n");
	return 0;
}

WRITE8_MEMBER(cp1_state::i8155_portb_w)
{
	logerror("Write to expansion port 2 %x\n", data);
}

WRITE8_MEMBER(cp1_state::i8155_portc_w)
{
	// --xx xxxx   keyboard matrix

	m_matrix = data & 0x3f;
}


static ADDRESS_MAP_START( cp1_io , AS_IO, 8, cp1_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x00,             0xff )          AM_READWRITE( i8155_read, i8155_write)
	AM_RANGE( MCS48_PORT_P1,    MCS48_PORT_P1)  AM_READWRITE( port1_r, port1_w )
	AM_RANGE( MCS48_PORT_P2,    MCS48_PORT_P2)  AM_READWRITE( port2_r, port2_w )
	AM_RANGE( MCS48_PORT_BUS,   MCS48_PORT_BUS) AM_READWRITE( getbus, putbus )
	AM_RANGE( MCS48_PORT_T0,    MCS48_PORT_T0)  AM_READ( t0_r )
	AM_RANGE( MCS48_PORT_T1,    MCS48_PORT_T1)  AM_READ( t1_r )
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( cp1 )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K)   PORT_NAME("CAS [Cass. speichern]")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CLR [Irrtum]")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C)   PORT_NAME("PC [Programmzahler]")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A)   PORT_NAME("ACC [Akku]")
	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L)   PORT_NAME("CAL [Cass. lesen]")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S)   PORT_NAME("STEP [Schritt]")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B)   PORT_NAME("STP [Stopp]")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)   PORT_NAME("RUN [Lauf]")
	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)   PORT_NAME("8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9)   PORT_NAME("9")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O)   PORT_NAME("OUT [Auslesen]")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)   PORT_NAME("INP [Eingeben]")
	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)   PORT_NAME("4")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)   PORT_NAME("5")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)   PORT_NAME("6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)   PORT_NAME("7")
	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0)   PORT_NAME("0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)   PORT_NAME("1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)   PORT_NAME("2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)   PORT_NAME("3")

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "CP2 Cassette Interface" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x01, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x02, 0x00, "CP3 RAM Expansion" )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
	PORT_CONFSETTING( 0x02, DEF_STR( Yes ) )
INPUT_PORTS_END

void cp1_state::machine_reset()
{
	m_port2 = 0;
	m_matrix = 0;
	m_7seg = 0;
	m_cassette->change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
}

QUICKLOAD_LOAD_MEMBER( cp1_state, quickload )
{
	UINT8 *dest = (UINT8*)m_i8155->space().get_read_ptr(0);
	char line[0x10];
	int addr = 0;
	while (image.fgets(line, 10) && addr < 0x100)
	{
		int op = 0, arg = 0;
		if (sscanf(line, "%d.%d", &op, &arg) == 2)
		{
			dest[addr++] = op;
			dest[addr++] = arg;
		}
		else
		{
			return IMAGE_INIT_FAIL;
		}
	}

	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( cp1, cp1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8049, XTAL_6MHz)
	MCFG_CPU_IO_MAP(cp1_io)

	MCFG_DEVICE_ADD("i8155", I8155, 0)
	MCFG_I8155_OUT_PORTA_CB(WRITE8(cp1_state, i8155_porta_w))
	MCFG_I8155_IN_PORTB_CB(READ8(cp1_state, i8155_portb_r))
	MCFG_I8155_OUT_PORTB_CB(WRITE8(cp1_state, i8155_portb_w))
	MCFG_I8155_OUT_PORTC_CB(WRITE8(cp1_state, i8155_portc_w))

	MCFG_DEVICE_ADD("i8155_cp3", I8155, 0)

	MCFG_DEFAULT_LAYOUT(layout_cp1)

	MCFG_CASSETTE_ADD("cassette")

	MCFG_QUICKLOAD_ADD("quickload", cp1_state, quickload, "obj", 1)
MACHINE_CONFIG_END

/* ROM definition */
/*
  KOSMOS B
  <Mitsubishi Logo> M5L8049-136P-6
  JAPAN 83F301
*/

ROM_START( cp1 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "b", "b" )
	ROMX_LOAD( "cp1-kosmos-b.rom", 0x0000, 0x0800, CRC(fea8a2b2) SHA1(c987b79a7b90fcbd58b66a69e95913f2655a1f0d), ROM_BIOS(1))
	// This is from 2716 eprom that was on board with I8039 instead of I8049
	ROM_SYSTEM_BIOS( 1, "2716", "2716" )
	ROMX_LOAD( "cp1-2716.bin",     0x0000, 0x0800, CRC(3a2caf0e) SHA1(ff4befcf82a664950186d3af1843fdef70d2209f), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1980, cp1,  0,       0,   cp1,    cp1, driver_device,  0,   "Kosmos",   "CP1 / Computer Praxis",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
