/***************************************************************************

    C-80

    12/05/2009 Skeleton driver.

    Pasting:
        0-F : as is
        + (inc) : ^
        - (dec) : V
        M : -
        GO : X

    Test Paste:
        -800^11^22^33^44^55^66^77^88^99^-800
        Now press up-arrow to confirm the data has been entered.

****************************************************************************/

#include "includes/c80.h"
#include "c80.lh"

/* Memory Maps */

static ADDRESS_MAP_START( c80_mem, AS_PROGRAM, 8, c80_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x0bff) AM_MIRROR(0x400) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( c80_io, AS_IO, 8, c80_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x7c, 0x7f) AM_DEVREADWRITE(Z80PIO2_TAG, z80pio_device, read, write)
	AM_RANGE(0xbc, 0xbf) AM_DEVREADWRITE(Z80PIO1_TAG, z80pio_device, read, write)
ADDRESS_MAP_END

/* Input Ports */

INPUT_CHANGED_MEMBER( c80_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( c80_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( c80 )
	PORT_START("ROW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FCN") PORT_CODE(KEYCODE_F1)

	PORT_START("ROW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')

	PORT_START("ROW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_M) PORT_CHAR('-')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RES") PORT_CODE(KEYCODE_F10) PORT_CHANGED_MEMBER(DEVICE_SELF, c80_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BRK") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, c80_state, trigger_nmi, 0)
INPUT_PORTS_END

/* Z80-PIO Interface */

READ8_MEMBER( c80_state::pio1_pa_r )
{
	/*

        bit     description

        PA0     keyboard row 0 input
        PA1     keyboard row 1 input
        PA2     keyboard row 2 input
        PA3
        PA4     _BSTB input
        PA5     display enable output (0=enabled, 1=disabled)
        PA6     tape output
        PA7     tape input

    */

	UINT8 data = !m_pio1_brdy << 4 | 0x07;

	int i;

	for (i = 0; i < 8; i++)
	{
		if (!BIT(m_keylatch, i))
		{
			if (!BIT(ioport("ROW0")->read(), i)) data &= ~0x01;
			if (!BIT(ioport("ROW1")->read(), i)) data &= ~0x02;
			if (!BIT(ioport("ROW2")->read(), i)) data &= ~0x04;
		}
	}

	data |= (m_cassette->input() < +0.0) << 7;

	return data;
}

WRITE8_MEMBER( c80_state::pio1_pa_w )
{
	/*

        bit     description

        PA0     keyboard row 0 input
        PA1     keyboard row 1 input
        PA2     keyboard row 2 input
        PA3
        PA4     _BSTB input
        PA5     display enable output (0=enabled, 1=disabled)
        PA6     tape output
        PA7     tape input

    */

	m_pio1_a5 = BIT(data, 5);

	if (!BIT(data, 5))
	{
		m_digit = 0;
	}

	m_cassette->output(BIT(data, 6) ? +1.0 : -1.0);
}

WRITE8_MEMBER( c80_state::pio1_pb_w )
{
	/*

        bit     description

        PB0     VQD30 segment A
        PB1     VQD30 segment B
        PB2     VQD30 segment C
        PB3     VQD30 segment D
        PB4     VQD30 segment E
        PB5     VQD30 segment F
        PB6     VQD30 segment G
        PB7     VQD30 segment P

    */

	if (!m_pio1_a5)
	{
		output_set_digit_value(m_digit, data);
	}

	m_keylatch = data;
}

WRITE_LINE_MEMBER( c80_state::pio1_brdy_w )
{
	m_pio1_brdy = state;

	if (state)
	{
		if (!m_pio1_a5)
		{
			m_digit++;
		}

		m_pio1->strobe_b(1);
		m_pio1->strobe_b(0);
	}
}

static Z80PIO_INTERFACE( pio1_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),	/* callback when change interrupt status */
	DEVCB_DRIVER_MEMBER(c80_state, pio1_pa_r),	/* port A read callback */
	DEVCB_DRIVER_MEMBER(c80_state, pio1_pa_w),	/* port A write callback */
	DEVCB_NULL,						/* portA ready active callback */
	DEVCB_NULL,						/* port B read callback */
	DEVCB_DRIVER_MEMBER(c80_state, pio1_pb_w),	/* port B write callback */
	DEVCB_DRIVER_LINE_MEMBER(c80_state, pio1_brdy_w)			/* portB ready active callback */
};

static Z80PIO_INTERFACE( pio2_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),	/* callback when change interrupt status */
	DEVCB_NULL,						/* port A read callback */
	DEVCB_NULL,						/* port A write callback */
	DEVCB_NULL,						/* portA ready active callback */
	DEVCB_NULL,						/* port B read callback */
	DEVCB_NULL,						/* port B write callback */
	DEVCB_NULL						/* portB ready active callback */
};

/* Z80 Daisy Chain */

static const z80_daisy_config c80_daisy_chain[] =
{
	{ Z80PIO1_TAG },
	{ Z80PIO2_TAG },
	{ NULL }
};

/* Machine Initialization */

void c80_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_keylatch));
	save_item(NAME(m_digit));
	save_item(NAME(m_pio1_a5));
	save_item(NAME(m_pio1_brdy));
}

/* Machine Driver */

static const cassette_interface c80_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};

static MACHINE_CONFIG_START( c80, c80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, 2500000) /* U880D */
	MCFG_CPU_PROGRAM_MAP(c80_mem)
	MCFG_CPU_IO_MAP(c80_io)
	MCFG_CPU_CONFIG(c80_daisy_chain)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_c80 )

	/* devices */
	MCFG_Z80PIO_ADD(Z80PIO1_TAG, 2500000, pio1_intf)
	MCFG_Z80PIO_ADD(Z80PIO2_TAG, 2500000, pio2_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, c80_cassette_interface)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( c80 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "c80.d3", 0x0000, 0x0400, CRC(ad2b3296) SHA1(14f72cb73a4068b7a5d763cc0e254639c251ce2e) )
ROM_END


/* System Drivers */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT  COMPANY      FULLNAME    FLAGS */
COMP( 1986, c80,    0,      0,      c80,    c80, driver_device,    0, "Joachim Czepa", "C-80", GAME_SUPPORTS_SAVE | GAME_NO_SOUND)
