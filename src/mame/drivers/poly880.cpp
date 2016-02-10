// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Poly-Computer 880

    12/05/2009 Skeleton driver.

    http://www.kc85-museum.de/books/poly880/index.html

After starting this driver, the screen may be blank. Press F2 until
something appears (most likely 'Go'). Then it can be used, or pasted to.

To see it say POLY-880, start the system, press F2, F1, F2.

Pasting:
        0-F : as is
        EXEC : ^
        BACK : V
        MEM : -
        GO : X

Test Paste:
        -4000^11^22^33^44^55^66^77^88^99^-4000
        Now press up-arrow to confirm the data has been entered.

****************************************************************************/

#include "includes/poly880.h"
#include "poly880.lh"

/*

    TODO:

    - SEND/SCON
    - MCYCL (activate single stepping)
    - CYCL (single step)
    - layout LEDs (address bus, data bus, command bus, MCYCL)
    - RAM expansion

*/

/* Read/Write Handlers */

void poly880_state::update_display()
{
	int i;

	for (i = 0; i < 8; i++)
	{
		if (BIT(m_digit, i)) output().set_digit_value(7 - i, m_segment);
	}
}

WRITE8_MEMBER( poly880_state::cldig_w )
{
	m_digit = data;

	update_display();
}

/* Memory Maps */

static ADDRESS_MAP_START( poly880_mem, AS_PROGRAM, 8, poly880_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x0c00) AM_ROM
	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0x0c00) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_MIRROR(0x0c00) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0x0c00) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x3c00) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_RAMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( poly880_io, AS_IO, 8, poly880_state )
	ADDRESS_MAP_GLOBAL_MASK(0xaf)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE(Z80PIO1_TAG, z80pio_device, read_alt, write_alt)
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE(Z80PIO2_TAG, z80pio_device, read_alt, write_alt)
	AM_RANGE(0x88, 0x8b) AM_DEVREADWRITE(Z80CTC_TAG, z80ctc_device, read, write)
	AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0f) AM_WRITE(cldig_w)
ADDRESS_MAP_END

/* Input Ports */

INPUT_CHANGED_MEMBER( poly880_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( poly880_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( poly880 )
	PORT_START("KI1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("EXEC") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BACK") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("FCT") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_M) PORT_CHAR('-')

	PORT_START("KI2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')

	PORT_START("KI3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RES") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, poly880_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MON") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, poly880_state, trigger_nmi, 0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MCYCL") PORT_CODE(KEYCODE_F3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CYCL") PORT_CODE(KEYCODE_F4)
INPUT_PORTS_END

/* Z80-CTC Interface */

WRITE_LINE_MEMBER( poly880_state::ctc_z0_w )
{
	// SEND
}

WRITE_LINE_MEMBER( poly880_state::ctc_z1_w )
{
}

/* Z80-PIO Interface */

WRITE8_MEMBER( poly880_state::pio1_pa_w )
{
	/*

	    bit     signal  description

	    PA0     SD0     segment E
	    PA1     SD1     segment D
	    PA2     SD2     segment C
	    PA3     SD3     segment P
	    PA4     SD4     segment G
	    PA5     SD5     segment A
	    PA6     SD6     segment F
	    PA7     SD7     segment B

	*/

	m_segment = BITSWAP8(data, 3, 4, 6, 0, 1, 2, 7, 5);

	update_display();
}

READ8_MEMBER( poly880_state::pio1_pb_r )
{
	/*

	    bit     signal  description

	    PB0     TTY
	    PB1     MIN     tape input
	    PB2     MOUT    tape output
	    PB3
	    PB4     KI1     key row 1 input
	    PB5     KI2     key row 2 input
	    PB6     SCON
	    PB7     KI3     key row 3 input

	*/

	UINT8 data = 0xf0 | ((m_cassette->input() < +0.0) << 1);
	int i;

	for (i = 0; i < 8; i++)
	{
		if (BIT(m_digit, i))
		{
			if (!BIT(m_ki1->read(), i)) data &= ~0x10;
			if (!BIT(m_ki2->read(), i)) data &= ~0x20;
			if (!BIT(m_ki3->read(), i)) data &= ~0x80;
		}
	}

	return data;
}

WRITE8_MEMBER( poly880_state::pio1_pb_w )
{
	/*

	    bit     signal  description

	    PB0     TTY     teletype serial output
	    PB1     MIN
	    PB2     MOUT    tape output
	    PB3
	    PB4     KI1     key row 1 input
	    PB5     KI2     key row 2 input
	    PB6     SCON
	    PB7     KI3     key row 3 input

	*/

	/* tape output */
	m_cassette->output( BIT(data, 2) ? +1.0 : -1.0);
}

#if 0
/* Z80 Daisy Chain */

static const z80_daisy_config poly880_daisy_chain[] =
{
	{ Z80PIO1_TAG },
	{ Z80PIO2_TAG },
	{ Z80CTC_TAG },
	{ NULL }
};
#endif

/* Machine Initialization */

void poly880_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_digit));
	save_item(NAME(m_segment));
}

/* Machine Driver */

static MACHINE_CONFIG_START( poly880, poly880_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_7_3728MHz/8)
	MCFG_CPU_PROGRAM_MAP(poly880_mem)
	MCFG_CPU_IO_MAP(poly880_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_poly880 )

	/* devices */
	MCFG_DEVICE_ADD(Z80CTC_TAG, Z80CTC, XTAL_7_3728MHz/16)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(poly880_state, ctc_z0_w))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(poly880_state, ctc_z1_w))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE(Z80CTC_TAG, z80ctc_device, trg3))

	MCFG_DEVICE_ADD(Z80PIO1_TAG, Z80PIO, XTAL_7_3728MHz/16)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(poly880_state, pio1_pa_w))
	MCFG_Z80PIO_IN_PB_CB(READ8(poly880_state, pio1_pb_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(poly880_state, pio1_pb_w))

	MCFG_DEVICE_ADD(Z80PIO2_TAG, Z80PIO, XTAL_7_3728MHz/16)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( poly880 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "poly880.i5", 0x0000, 0x0400, CRC(b1c571e8) SHA1(85bfe53d39d6690e79999a1e1240789497e72db0) )
	ROM_LOAD( "poly880.i6", 0x1000, 0x0400, CRC(9efddf5b) SHA1(6ffa2f80b2c6f8ec9e22834f739c82f9754272b8) )
ROM_END

/* System Drivers */

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT    COMPANY             FULLNAME                FLAGS */
COMP( 1983, poly880,    0,      0,      poly880,    poly880, driver_device,    0,   "VEB Polytechnik", "Poly-Computer 880", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW)
