// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller, Robbbert
/******************************************************************************
    Motorola Evaluation Kit 6800 D2
    MEK6800D2

    system driver

    Juergen Buchmueller, Jan 2000
    2013-06-16 Working driver [Robbbert]

    memory map

    range       short   description
    0000-00ff   RAM     256 bytes RAM
    0100-01ff   RAM     optional 256 bytes RAM
    6000-63ff   PROM    optional PROM
    or
    6000-67ff   ROM     optional ROM
    8004-8007   PIA     expansion port
    8008-8009   ACIA    cassette interface
    8020-8023   PIA     keyboard interface
    a000-a07f   RAM     128 bytes RAM (JBUG scratch)
    c000-c3ff   PROM    optional PROM
    or
    c000-c7ff   ROM     optional ROM
    e000-e3ff   ROM     JBUG monitor program
    e400-ffff   -/-     mirrors of monitor rom


Enter the 4 digit address then the command key:

  - M : Examine and Change Memory (example: E000M, then G to skip to next, ESC to exit)
  - E : Escape (abort) operation (ESC key in our emulation)
  - R : Examine Registers
  - G : Begin execution at specified address
  - P : Punch data from memory to magnetic tape
  - L : Load memory from magnetic tape
  - N : Trace one instruction
  - V : Set (and remove) breakpoints

The keys are laid out as:

  P L N V

  7 8 9 A  M
  4 5 6 B  E
  1 2 3 C  R
  0 F E D  G


Pasting:
        0-F : as is
        NEXT : ^
        MEM : =
        GO : ^

Test Paste:
        HA030=11^22^33^44^55^66^77^88^99^HA030=
        Now press up-arrow to confirm the data has been entered.

        If you wish to follow the tutorial in the manual, here is the test
        program that you need to enter in step 1:
        H0020=8E^00^FF^4F^C6^04^CE^00^10^AB^00^08^5A^26^FA^97^15^3F^H

        Save the above program to tape:
        HA002=00^20^00^32^HP (A002 has start address, A004 has end address, big endian)

TODO
        Display should go blank during cassette operations


******************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "sound/wave.h"
#include "mekd2.lh"

#define XTAL_MEKD2 1228800

class mekd2_state : public driver_device
{
public:
	enum
	{
		TIMER_TRACE
	};

	mekd2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia_s(*this, "pia_s"),
		m_pia_u(*this, "pia_u"),
		m_acia(*this, "acia"),
		m_cass(*this, "cassette")
	{ }

	DECLARE_READ_LINE_MEMBER(mekd2_key40_r);
	DECLARE_READ8_MEMBER(mekd2_key_r);
	DECLARE_WRITE_LINE_MEMBER(mekd2_nmi_w);
	DECLARE_WRITE8_MEMBER(mekd2_digit_w);
	DECLARE_WRITE8_MEMBER(mekd2_segment_w);
	DECLARE_QUICKLOAD_LOAD_MEMBER(mekd2_quik);
	DECLARE_WRITE_LINE_MEMBER(cass_w);
	TIMER_DEVICE_CALLBACK_MEMBER(mekd2_c);
	TIMER_DEVICE_CALLBACK_MEMBER(mekd2_p);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	UINT8 m_cass_data[4];
	UINT8 m_segment;
	UINT8 m_digit;
	UINT8 m_keydata;
	bool m_cass_state;
	bool m_cassold;
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia_s;
	required_device<pia6821_device> m_pia_u;
	required_device<acia6850_device> m_acia;
	required_device<cassette_image_device> m_cass;
};



/***********************************************************

    Address Map

************************************************************/

static ADDRESS_MAP_START( mekd2_mem , AS_PROGRAM, 8, mekd2_state)
	AM_RANGE(0x0000, 0x00ff) AM_RAM // user ram
	AM_RANGE(0x8004, 0x8007) AM_DEVREADWRITE("pia_u", pia6821_device, read, write)
	AM_RANGE(0x8008, 0x8008) AM_DEVREADWRITE("acia", acia6850_device, status_r, control_w)
	AM_RANGE(0x8009, 0x8009) AM_DEVREADWRITE("acia", acia6850_device, data_r, data_w)
	AM_RANGE(0x8020, 0x8023) AM_DEVREADWRITE("pia_s", pia6821_device, read, write)
	AM_RANGE(0xa000, 0xa07f) AM_RAM // system ram
	AM_RANGE(0xe000, 0xe3ff) AM_ROM AM_MIRROR(0x1c00)   /* JBUG ROM */
ADDRESS_MAP_END

/***********************************************************

    Keys

************************************************************/

static INPUT_PORTS_START( mekd2 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E (hex)") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') // save tape
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') // load tape
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') // trace (step)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') // breakpoint

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('=') // memory
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E (escape)") PORT_CODE(KEYCODE_ESC) PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') // regs
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_UP) PORT_CHAR('^') // go, next
INPUT_PORTS_END


/***********************************************************

    Trace hardware (what happens when N is pressed)

************************************************************/

void mekd2_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_TRACE:
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		break;
	default:
		assert_always(FALSE, "Unknown id in mekd2_state::device_timer");
	}
}


WRITE_LINE_MEMBER( mekd2_state::mekd2_nmi_w )
{
	if (state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	else
		timer_set(attotime::from_usec(18), TIMER_TRACE);
}



/***********************************************************

    Keyboard

************************************************************/

READ_LINE_MEMBER( mekd2_state::mekd2_key40_r )
{
	return BIT(m_keydata, 6);
}

READ8_MEMBER( mekd2_state::mekd2_key_r )
{
	char kbdrow[4];
	UINT8 i;
	m_keydata = 0xff;

	for (i = 0; i < 6; i++)
	{
		if (BIT(m_digit, i))
		{
			sprintf(kbdrow,"X%d",i);
			m_keydata &= ioport(kbdrow)->read();
		}
	}

	i = 0x80;
	if (m_digit < 0x40)
		i = BIT(m_keydata, 0) ? 0x80 : 0;
	else
	if (m_digit < 0x80)
		i = BIT(m_keydata, 1) ? 0x80 : 0;
	else
	if (m_digit < 0xc0)
		i = BIT(m_keydata, 2) ? 0x80 : 0;
	else
		i = BIT(m_keydata, 3) ? 0x80 : 0;

	return i | m_segment;
}



/***********************************************************

    LED display

************************************************************/

WRITE8_MEMBER( mekd2_state::mekd2_segment_w )
{
	m_segment = data & 0x7f;
}

WRITE8_MEMBER( mekd2_state::mekd2_digit_w )
{
	UINT8 i;
	if (data < 0x3f)
	{
		for (i = 0; i < 6; i++)
		{
			if (BIT(data, i))
				output().set_digit_value(i, ~m_segment & 0x7f);
		}
	}
	m_digit = data;
}



/***********************************************************

    Interfaces

************************************************************/

WRITE_LINE_MEMBER( mekd2_state::cass_w )
{
	m_cass_state = state;
}

QUICKLOAD_LOAD_MEMBER( mekd2_state, mekd2_quik )
{
	static const char magic[] = "MEK6800D2";
	char buff[9];
	UINT16 addr, size;
	UINT8 ident, *RAM = memregion("maincpu")->base();

	image.fread(buff, sizeof (buff));
	if (memcmp(buff, magic, sizeof (buff)))
	{
		logerror("mekd2 rom load: magic '%s' not found\n", magic);
		return IMAGE_INIT_FAIL;
	}
	image.fread(&addr, 2);
	addr = LITTLE_ENDIANIZE_INT16(addr);
	image.fread(&size, 2);
	size = LITTLE_ENDIANIZE_INT16(size);
	image.fread(&ident, 1);
	logerror("mekd2 rom load: $%04X $%04X $%02X\n", addr, size, ident);
	while (size-- > 0)
		image.fread(&RAM[addr++], 1);

	return IMAGE_INIT_PASS;
}

TIMER_DEVICE_CALLBACK_MEMBER(mekd2_state::mekd2_c)
{
	m_cass_data[3]++;

	if (m_cass_state != m_cassold)
	{
		m_cass_data[3] = 0;
		m_cassold = m_cass_state;
	}

	if (m_cass_state)
		m_cass->output(BIT(m_cass_data[3], 0) ? -1.0 : +1.0); // 2400Hz
	else
		m_cass->output(BIT(m_cass_data[3], 1) ? -1.0 : +1.0); // 1200Hz
}

TIMER_DEVICE_CALLBACK_MEMBER(mekd2_state::mekd2_p)
{
	/* cassette - turn 1200/2400Hz to a bit */
	m_cass_data[1]++;
	UINT8 cass_ws = (m_cass->input() > +0.03) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		m_acia->write_rxd((m_cass_data[1] < 12) ? 1 : 0);
		m_cass_data[1] = 0;
	}
}

/***********************************************************

    Machine

************************************************************/

static MACHINE_CONFIG_START( mekd2, mekd2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_MEKD2 / 2)        /* 614.4 kHz */
	MCFG_CPU_PROGRAM_MAP(mekd2_mem)

	MCFG_DEFAULT_LAYOUT(layout_mekd2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_CASSETTE_ADD("cassette")

	/* Devices */
	MCFG_DEVICE_ADD("pia_s", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(mekd2_state, mekd2_key_r))
	MCFG_PIA_READCB1_HANDLER(READLINE(mekd2_state, mekd2_key40_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(mekd2_state, mekd2_segment_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mekd2_state, mekd2_digit_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(mekd2_state, mekd2_nmi_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, nmi_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, nmi_line))

	MCFG_DEVICE_ADD("pia_u", PIA6821, 0)
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6800_cpu_device, irq_line))

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(mekd2_state, cass_w))

	MCFG_DEVICE_ADD("acia_tx_clock", CLOCK, XTAL_MEKD2 / 256) // 4800Hz
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("acia", acia6850_device, write_txc))

	MCFG_DEVICE_ADD("acia_rx_clock", CLOCK, 300) // toggled by cassette circuit
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxc))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("mekd2_c", mekd2_state, mekd2_c, attotime::from_hz(4800))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("mekd2_p", mekd2_state, mekd2_p, attotime::from_hz(40000))

	MCFG_QUICKLOAD_ADD("quickload", mekd2_state, mekd2_quik, "d2", 1)
MACHINE_CONFIG_END

/***********************************************************

    ROMS

************************************************************/

ROM_START(mekd2)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("jbug.rom", 0xe000, 0x0400, CRC(5ed08792) SHA1(b06e74652a4c4e67c4a12ddc191ffb8c07f3332e) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT  CLASS        INIT    COMPANY     FULLNAME   FLAGS */
COMP( 1977, mekd2,  0,      0,      mekd2,    mekd2, driver_device, 0,  "Motorola", "MEK6800D2" , 0 )
