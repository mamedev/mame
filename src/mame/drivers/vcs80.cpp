// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

    VCS-80

    12/05/2009 Skeleton driver.

    http://hc-ddr.hucki.net/entwicklungssysteme.htm#VCS_80_von_Eckhard_Schiller

This system is heavily based on the tk80. The display and keyboard matrix
are very similar, while the operation is easier in the vcs80. The hardware
is completely different however.

Pasting:
        0-F : as is
        A+ : ^
        A- : V
        MA : -
        GO : X

When booted, the system begins at 0000 which is ROM. You need to change the
address to 0400 before entering a program. Here is a test to paste in:
0400-11^22^33^44^55^66^77^88^99^0400-
Press the up-arrow to confirm data has been entered.

Operation:
4 digits at left is the address; 2 digits at right is the data.
As you increment addresses, the middle 2 digits show the previous byte.
You can enter 4 digits, and pressing 'MA' will transfer this info
to the left, thus setting the address to this value. Press 'A+' to
store new data and increment the address.

****************************************************************************/

#include "includes/vcs80.h"
#include "vcs80.lh"

/* Read/Write Handlers */

READ8_MEMBER( vcs80_state::pio_r )
{
	switch (offset)
	{
	case 0: return m_pio->control_read();
	case 1: return m_pio->control_read();
	case 2: return m_pio->data_read(1);
	case 3: return m_pio->data_read(0);
	}

	return 0;
}

WRITE8_MEMBER( vcs80_state::pio_w )
{
	switch (offset)
	{
	case 0: m_pio->control_write(1, data); break;
	case 1: m_pio->control_write(0, data); break;
	case 2: m_pio->data_write(1, data); break;
	case 3: m_pio->data_write(0, data); break;
	}
}

/* Memory Maps */

static ADDRESS_MAP_START( vcs80_mem, AS_PROGRAM, 8, vcs80_state )
	AM_RANGE(0x0000, 0x01ff) AM_ROM
	AM_RANGE(0x0400, 0x07ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( vcs80_io, AS_IO, 8, vcs80_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x07) AM_READWRITE(pio_r, pio_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( vcs80 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A+") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("A-") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MA") PORT_CODE(KEYCODE_M) PORT_CHAR('-')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("TR") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ST") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("PE") PORT_CODE(KEYCODE_P)
INPUT_PORTS_END

/* Z80-PIO Interface */

TIMER_DEVICE_CALLBACK_MEMBER(vcs80_state::vcs80_keyboard_tick)
{
	if (m_keyclk)
	{
		m_keylatch++;
		m_keylatch &= 7;
	}

	m_pio->port_a_write(m_keyclk << 7);

	m_keyclk = !m_keyclk;
}

READ8_MEMBER( vcs80_state::pio_pa_r )
{
	/*

	    bit     description

	    PA0     keyboard and led latch bit 0
	    PA1     keyboard and led latch bit 1
	    PA2     keyboard and led latch bit 2
	    PA3     GND
	    PA4     keyboard row input 0
	    PA5     keyboard row input 1
	    PA6     keyboard row input 2
	    PA7     demultiplexer clock input

	*/

	UINT8 data = 0;

	/* keyboard and led latch */
	data |= m_keylatch;

	/* keyboard rows */
	data |= BIT(m_y0->read(), m_keylatch) << 4;
	data |= BIT(m_y1->read(), m_keylatch) << 5;
	data |= BIT(m_y2->read(), m_keylatch) << 6;

	/* demultiplexer clock */
	data |= (m_keyclk << 7);

	return data;
}

WRITE8_MEMBER( vcs80_state::pio_pb_w )
{
	/*

	    bit     description

	    PB0     VQD30 segment A
	    PB1     VQD30 segment B
	    PB2     VQD30 segment C
	    PB3     VQD30 segment D
	    PB4     VQD30 segment E
	    PB5     VQD30 segment G
	    PB6     VQD30 segment F
	    PB7     _A0

	*/

	UINT8 led_data = BITSWAP8(data & 0x7f, 7, 5, 6, 4, 3, 2, 1, 0);
	int digit = m_keylatch;

	/* skip middle digit */
	if (digit > 3) digit++;

	output().set_digit_value(8 - digit, led_data);
}

/* Z80 Daisy Chain */

static const z80_daisy_config vcs80_daisy_chain[] =
{
	{ Z80PIO_TAG },
	{ nullptr }
};

/* Machine Initialization */

void vcs80_state::machine_start()
{
	m_pio->strobe_a(1);
	m_pio->strobe_b(1);

	/* register for state saving */
	save_item(NAME(m_keylatch));
	save_item(NAME(m_keyclk));
}

/* Machine Driver */

static MACHINE_CONFIG_START( vcs80, vcs80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_5MHz/2) /* U880D */
	MCFG_CPU_PROGRAM_MAP(vcs80_mem)
	MCFG_CPU_IO_MAP(vcs80_io)
	MCFG_CPU_CONFIG(vcs80_daisy_chain)

	/* keyboard timer */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", vcs80_state, vcs80_keyboard_tick, attotime::from_hz(1000))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_vcs80 )

	/* devices */
	MCFG_DEVICE_ADD(Z80PIO_TAG, Z80PIO, XTAL_5MHz/2)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(vcs80_state, pio_pa_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(vcs80_state, pio_pb_w))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( vcs80 )
	ROM_REGION( 0x10000, Z80_TAG, 0 )
	ROM_LOAD( "monitor.rom", 0x0000, 0x0200, CRC(44aff4e9) SHA1(3472e5a9357eaba3ed6de65dee2b1c6b29349dd2) )
ROM_END

/* Driver Initialization */

DIRECT_UPDATE_MEMBER(vcs80_state::vcs80_direct_update_handler)
{
	/* _A0 is connected to PIO PB7 */
	m_pio->port_b_write((!BIT(address, 0)) << 7);

	return address;
}

DRIVER_INIT_MEMBER(vcs80_state,vcs80)
{
	m_maincpu->space(AS_PROGRAM).set_direct_update_handler(direct_update_delegate(FUNC(vcs80_state::vcs80_direct_update_handler), this));
	m_maincpu->space(AS_IO).set_direct_update_handler(direct_update_delegate(FUNC(vcs80_state::vcs80_direct_update_handler), this));
}

/* System Drivers */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY             FULLNAME    FLAGS */
COMP( 1983, vcs80,  0,      0,      vcs80,  vcs80, vcs80_state,  vcs80, "Eckhard Schiller", "VCS-80", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND)
