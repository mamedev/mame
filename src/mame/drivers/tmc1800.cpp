// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

Telmac 2000

PCB Layout
----------

|-----------------------------------------------------------|
|                                                           |
|    |                  4051    4042        2114    2114    |
|   CN1                                                     |
|    |                  4051    4042        2114    2114    |
|                                                           |
|   4011                4013    |--------|  2114    2114    |
|                               |  4515  |                  |
|                       4013    |--------|  2114    2114    |
|                               4042                        |
|                       4011                2114    2114    |
|                               4011                        |
|                       4011                2114    2114    |
|SW1                                                        |
|                       4011    |--------|  2114    2114    |
|                               |  PROM  |                  |
|                       40107   |--------|  2114    2114    |
|   4050    1.75MHz                                         |
|                       |-------------|     2114    2114    |
|                       |   CDP1802   |                     |
|                       |-------------|     2114    2114    |
|           4049                                            |
|                       |-------------|     2114    2114    |
|       4.43MHz         |   CDP1864   |                     |
|                       |-------------|     2114    2114    |
|                                                           |
|               741             4502        2114    2114    |
|  -                                                        |
|  |                            2114        2114    2114    |
| CN2                                                       |
|  |            4001            4051        2114    2114    |
|  -                                                        |
|                                           2114    2114    |
|-----------------------------------------------------------|

Notes:
    All IC's shown.

    PROM    - MMI6341
    2114    - 2114 4096 Bit (1024x4) NMOS Static RAM
    CDP1802 - RCA CDP1802 CMOS 8-Bit Microprocessor @ 1.75 MHz
    CDP1864 - RCA CDP1864CE COS/MOS PAL Compatible Color TV Interface @ 1.75 MHz
    CN1     - keyboard connector
    CN2     - ASTEC RF modulator connector
    SW1     - Run/Reset switch

*/

/*

OSCOM Nano

PCB Layout
----------

OK 30379

|-------------------------------------------------|
|   CN1     CN2     CN3                 7805      |
|                       1.75MHz                   |
|                               741               |
|                       |-------------|         - |
|   741     741   4011  |   CDP1864   |         | |
|                       |-------------|         | |
|                 4013  |-------------|         | |
|                       |   CDP1802   |         | |
|                 4093  |-------------|         | |
|                                               C |
|            4051   4042    4017        4042    N |
|                           |-------|           4 |
|                           |  ROM  |   14556   | |
|                           |-------|           | |
|                           2114    2114        | |
|                                               | |
|                           2114    2114        | |
|                                               - |
|                           2114    2114          |
|                                                 |
|                           2114    2114          |
|-------------------------------------------------|

Notes:
    All IC's shown.

    ROM     - Intersil 5504?
    2114    - 2114UCB 4096 Bit (1024x4) NMOS Static RAM
    CDP1802 - RCA CDP1802E CMOS 8-Bit Microprocessor @ 1.75 MHz
    CDP1864 - RCA CDP1864CE COS/MOS PAL Compatible Color TV Interface @ 1.75 MHz
    CN1     - tape connector
    CN2     - video connector
    CN3     - power connector
    CN4     - expansion connector

*/

/*

    TODO:

    - tape input/output
    - tmc2000: add missing keys
    - tmc2000: TOOL-2000 rom banking
    - nano: correct time constant for EF4 RC circuit

*/

#include "includes/tmc1800.h"
#include "sound/beep.h"

/* Read/Write Handlers */

WRITE8_MEMBER( tmc1800_state::keylatch_w )
{
	m_keylatch = data;
}

WRITE8_MEMBER( osc1000b_state::keylatch_w )
{
	m_keylatch = data;
}

WRITE8_MEMBER( tmc2000_state::keylatch_w )
{
	/*

	    bit     description

	    0       X0
	    1       X1
	    2       X2
	    3       Y0
	    4       Y1
	    5       Y2
	    6       EXP1
	    7       EXP2

	*/

	m_keylatch = data & 0x3f;
}

WRITE8_MEMBER( nano_state::keylatch_w )
{
	/*

	    bit     description

	    0       A
	    1       B
	    2       C
	    3       NY0
	    4       NY1
	    5
	    6
	    7

	*/

	m_keylatch = data & 0x1f;
}

void tmc2000_state::bankswitch()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();
	UINT8 *rom = m_rom->base();

	if (m_roc)
	{
		// monitor ROM
		program.install_rom(0x0000, 0x01ff, 0, 0x7e00, rom);
	}
	else
	{
		// RAM
		switch (m_ram->size())
		{
		case 4 * 1024:
			program.install_ram(0x0000, 0x0fff, 0, 0x7000, ram);
			break;

		case 16 * 1024:
			program.install_ram(0x0000, 0x3fff, 0, 0x4000, ram);
			break;

		case 32 * 1024:
			program.install_ram(0x0000, 0x7fff, ram);
			break;
		}
	}

	if (m_rac)
	{
		// color RAM
		program.install_ram(0x8000, 0x81ff, 0, 0x7e00, m_colorram);
		program.unmap_read(0x8000, 0x81ff, 0, 0x7e00);
	}
	else
	{
		// monitor ROM
		program.install_rom(0x8000, 0x81ff, 0, 0x7e00, rom);
	}
}

WRITE8_MEMBER( tmc2000_state::bankswitch_w )
{
	m_roc = 0;
	m_rac = BIT(data, 0);
	bankswitch();

	m_cti->tone_latch_w(space, 0, data);
}

WRITE8_MEMBER( nano_state::bankswitch_w )
{
	/* enable RAM */
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();
	program.install_ram(0x0000, 0x0fff, 0, 0x7000, ram);

	/* write to CDP1864 tone latch */
	m_cti->tone_latch_w(space, 0, data);
}

READ8_MEMBER( tmc1800_state::dispon_r )
{
	m_vdc->disp_on_w(1);
	m_vdc->disp_on_w(0);

	return 0xff;
}

WRITE8_MEMBER( tmc1800_state::dispoff_w )
{
	m_vdc->disp_off_w(1);
	m_vdc->disp_off_w(0);
}

/* Memory Maps */

// Telmac 1800

static ADDRESS_MAP_START( tmc1800_map, AS_PROGRAM, 8, tmc1800_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x7800) AM_RAM
	AM_RANGE(0x8000, 0x81ff) AM_MIRROR(0x7e00) AM_ROM AM_REGION(CDP1802_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( tmc1800_io_map, AS_IO, 8, tmc1800_state )
	AM_RANGE(0x01, 0x01) AM_READWRITE(dispon_r, dispoff_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(keylatch_w)
ADDRESS_MAP_END

// OSCOM 1000B

static ADDRESS_MAP_START( osc1000b_map, AS_PROGRAM, 8, osc1000b_state )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x7800) AM_RAM
	AM_RANGE(0x8000, 0x81ff) AM_MIRROR(0x7e00) AM_ROM AM_REGION(CDP1802_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( osc1000b_io_map, AS_IO, 8, osc1000b_state )
	AM_RANGE(0x02, 0x02) AM_WRITE(keylatch_w)
ADDRESS_MAP_END

// Telmac 2000

static ADDRESS_MAP_START( tmc2000_map, AS_PROGRAM, 8, tmc2000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) // RAM / monitor ROM
	AM_RANGE(0x8000, 0xffff) // color RAM / monitor ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tmc2000_io_map, AS_IO, 8, tmc2000_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE(CDP1864_TAG, cdp1864_device, dispon_r, step_bgcolor_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(keylatch_w)
	AM_RANGE(0x04, 0x04) AM_DEVREAD(CDP1864_TAG, cdp1864_device, dispoff_r) AM_WRITE(bankswitch_w)
ADDRESS_MAP_END

// OSCOM Nano

static ADDRESS_MAP_START( nano_map, AS_PROGRAM, 8, nano_state )
	AM_RANGE(0x0000, 0x7fff) // RAM / monitor ROM
	AM_RANGE(0x8000, 0x81ff) AM_MIRROR(0x7e00) AM_ROM AM_REGION(CDP1802_TAG, 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nano_io_map, AS_IO, 8, nano_state )
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE(CDP1864_TAG, cdp1864_device, dispon_r, step_bgcolor_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(keylatch_w)
	AM_RANGE(0x04, 0x04) AM_DEVREAD(CDP1864_TAG, cdp1864_device, dispoff_r) AM_WRITE(bankswitch_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( tmc1800 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Run/Reset") PORT_CODE(KEYCODE_R) PORT_TOGGLE
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( tmc2000_state::run_pressed )
{
	if (oldval && !newval)
	{
		machine_reset();
	}
}

static INPUT_PORTS_START( tmc2000 )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Run/Reset") PORT_CODE(KEYCODE_R) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, tmc2000_state, run_pressed, 0)

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V')

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( nano_state::run_pressed )
{
	if (oldval && !newval)
	{
		machine_reset();
	}
}

INPUT_CHANGED_MEMBER( nano_state::monitor_pressed )
{
	if (oldval && !newval)
	{
		machine_reset();

		m_maincpu->set_input_line(COSMAC_INPUT_LINE_EF4, CLEAR_LINE);
	}
	else if (!oldval && newval)
	{
		// TODO: what are the correct values?
		int t = RES_K(27) * CAP_U(1) * 1000; // t = R26 * C1
		timer_set(attotime::from_msec(t), TIMER_ID_EF4);
	}
}

static INPUT_PORTS_START( nano )
	PORT_START("NY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7')

	PORT_START("NY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RUN") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, nano_state, run_pressed, 0)

	PORT_START("MONITOR")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("MONITOR") PORT_CODE(KEYCODE_M) PORT_CHANGED_MEMBER(DEVICE_SELF, nano_state, monitor_pressed, 0)
INPUT_PORTS_END

/* CDP1802 Interfaces */

// Telmac 1800

READ_LINE_MEMBER( tmc1800_state::clear_r )
{
	return BIT(m_run->read(), 0);
}

READ_LINE_MEMBER( tmc1800_state::ef2_r )
{
	return m_cassette->input() < 0;
}

READ_LINE_MEMBER( tmc1800_state::ef3_r )
{
	return CLEAR_LINE; // TODO
}

WRITE_LINE_MEMBER( tmc1800_state::q_w )
{
	m_cassette->output(state ? 1.0 : -1.0);
}

// Oscom 1000B

READ_LINE_MEMBER( osc1000b_state::clear_r )
{
	return BIT(m_run->read(), 0);
}

READ_LINE_MEMBER( osc1000b_state::ef2_r )
{
	return m_cassette->input() < 0;
}

READ_LINE_MEMBER( osc1000b_state::ef3_r )
{
	return CLEAR_LINE; // TODO
}

WRITE_LINE_MEMBER( osc1000b_state::q_w )
{
	m_cassette->output(state ? 1.0 : -1.0);
}

// Telmac 2000

READ_LINE_MEMBER( tmc2000_state::clear_r )
{
	return BIT(m_run->read(), 0);
}

READ_LINE_MEMBER( tmc2000_state::ef2_r )
{
	return (m_cassette)->input() < 0;
}

READ_LINE_MEMBER( tmc2000_state::ef3_r )
{
	UINT8 data = ~m_key_row[m_keylatch / 8]->read();

	return BIT(data, m_keylatch % 8);
}

WRITE_LINE_MEMBER( tmc2000_state::q_w )
{
	/* CDP1864 audio output enable */
	m_cti->aoe_w(state);

	/* set Q led status */
	output().set_led_value(1, state);

	/* tape output */
	m_cassette->output(state ? 1.0 : -1.0);
}

WRITE8_MEMBER( tmc2000_state::dma_w )
{
	m_color = ~(m_colorram[offset & 0x1ff]) & 0x07;

	m_cti->con_w(0); // HACK
	m_cti->dma_w(space, offset, data);
}

// OSCOM Nano

READ_LINE_MEMBER( nano_state::clear_r )
{
	int run = BIT(m_run->read(), 0);
	int monitor = BIT(m_monitor->read(), 0);

	return run && monitor;
}

READ_LINE_MEMBER( nano_state::ef2_r )
{
	return m_cassette->input() < 0;
}

READ_LINE_MEMBER( nano_state::ef3_r )
{
	UINT8 data = 0xff;

	if (!BIT(m_keylatch, 3)) data &= m_ny0->read();
	if (!BIT(m_keylatch, 4)) data &= m_ny1->read();

	return !BIT(data, m_keylatch & 0x07);
}

WRITE_LINE_MEMBER( nano_state::q_w )
{
	/* CDP1864 audio output enable */
	m_cti->aoe_w(state);

	/* set Q led status */
	output().set_led_value(1, state);

	/* tape output */
	m_cassette->output(state ? 1.0 : -1.0);
}

/* Machine Initialization */

// Telmac 1800

void tmc1800_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_keylatch));
}

void tmc1800_state::machine_reset()
{
	/* reset CDP1861 */
	m_vdc->reset();
}

// OSCOM 1000B

void osc1000b_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_keylatch));
}

void osc1000b_state::machine_reset()
{
}

// Telmac 2000

void tmc2000_state::machine_start()
{
	UINT16 addr;

	m_colorram.allocate(TMC2000_COLORRAM_SIZE);

	// randomize color RAM contents
	for (addr = 0; addr < TMC2000_COLORRAM_SIZE; addr++)
	{
		m_colorram[addr] = machine().rand() & 0xff;
	}

	// find keyboard rows
	m_key_row[0] = m_y0;
	m_key_row[1] = m_y1;
	m_key_row[2] = m_y2;
	m_key_row[3] = m_y3;
	m_key_row[4] = m_y4;
	m_key_row[5] = m_y5;
	m_key_row[6] = m_y6;
	m_key_row[7] = m_y7;

	// state saving
	save_item(NAME(m_keylatch));
	save_item(NAME(m_rac));
	save_item(NAME(m_roc));
}

void tmc2000_state::machine_reset()
{
	// reset CDP1864
	m_cti->reset();

	// banking
	m_roc = 1;
	m_rac = 0;
	bankswitch();
}

// OSCOM Nano

void nano_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_ID_EF4:
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_EF4, ASSERT_LINE);
		break;
	}
}

void nano_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_keylatch));
}

void nano_state::machine_reset()
{
	/* assert EF4 */
	m_maincpu->set_input_line(COSMAC_INPUT_LINE_EF4, ASSERT_LINE);

	/* reset CDP1864 */
	m_cti->reset();

	/* enable ROM */
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *rom = m_rom->base();
	program.install_rom(0x0000, 0x01ff, 0, 0x7e00, rom);
}

/* Machine Drivers */

QUICKLOAD_LOAD_MEMBER( tmc1800_base_state, tmc1800 )
{
	UINT8 *ptr = m_rom->base();
	int size = image.length();

	if (size > m_ram->size())
	{
		return IMAGE_INIT_FAIL;
	}

	image.fread( ptr, size);

	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( tmc1800, tmc1800_state )
	// basic system hardware
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_1_75MHz)
	MCFG_CPU_PROGRAM_MAP(tmc1800_map)
	MCFG_CPU_IO_MAP(tmc1800_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(tmc1800_state, clear_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(tmc1800_state, ef2_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(tmc1800_state, ef3_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(tmc1800_state, q_w))
	MCFG_COSMAC_DMAW_CALLBACK(DEVWRITE8(CDP1861_TAG, cdp1861_device, dma_w))

	// video hardware
	MCFG_FRAGMENT_ADD(tmc1800_video)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_QUICKLOAD_ADD("quickload", tmc1800_base_state, tmc1800, "bin", 0)
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2K")
	MCFG_RAM_EXTRA_OPTIONS("4K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( osc1000b, osc1000b_state )
	// basic system hardware
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_1_75MHz)
	MCFG_CPU_PROGRAM_MAP(osc1000b_map)
	MCFG_CPU_IO_MAP(osc1000b_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(osc1000b_state, clear_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(osc1000b_state, ef2_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(osc1000b_state, ef3_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(osc1000b_state, q_w))

	// video hardware
	MCFG_FRAGMENT_ADD(osc1000b_video)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	// devices
	MCFG_QUICKLOAD_ADD("quickload", tmc1800_base_state, tmc1800, "bin", 0)
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2K")
	MCFG_RAM_EXTRA_OPTIONS("4K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( tmc2000, tmc2000_state )
	// basic system hardware
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_1_75MHz)
	MCFG_CPU_PROGRAM_MAP(tmc2000_map)
	MCFG_CPU_IO_MAP(tmc2000_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(tmc2000_state, clear_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(tmc2000_state, ef2_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(tmc2000_state, ef3_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(tmc2000_state, q_w))
	MCFG_COSMAC_DMAW_CALLBACK(WRITE8(tmc2000_state, dma_w))

	// video hardware
	MCFG_FRAGMENT_ADD(tmc2000_video)

	// devices
	MCFG_QUICKLOAD_ADD("quickload", tmc1800_base_state, tmc1800, "bin", 0)
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4K")
	MCFG_RAM_EXTRA_OPTIONS("16K,32K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( nano, nano_state )
	// basic system hardware
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_1_75MHz)
	MCFG_CPU_PROGRAM_MAP(nano_map)
	MCFG_CPU_IO_MAP(nano_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(nano_state, clear_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(nano_state, ef2_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(nano_state, ef3_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(nano_state, q_w))
	MCFG_COSMAC_DMAW_CALLBACK(DEVWRITE8(CDP1864_TAG, cdp1864_device, dma_w))

	// video hardware
	MCFG_FRAGMENT_ADD(nano_video)

	// devices
	MCFG_QUICKLOAD_ADD("quickload", tmc1800_base_state, tmc1800, "bin", 0)
	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( tmc1800 )
	ROM_REGION( 0x200, CDP1802_TAG, 0 )
	ROM_LOAD( "mmi6341-1.ic2", 0x000, 0x200, NO_DUMP ) // equivalent to 82S141
ROM_END

ROM_START( osc1000b )
	ROM_REGION( 0x200, CDP1802_TAG, 0 )
	ROM_LOAD( "mmi6341-1.ic2", 0x000, 0x0200, NO_DUMP ) // equivalent to 82S141

	ROM_REGION( 0x400, "gfx1", 0 )
	ROM_LOAD( "mmi6349.5d", 0x000, 0x200, NO_DUMP ) // equivalent to 82S147
	ROM_LOAD( "mmi6349.5c", 0x200, 0x200, NO_DUMP ) // equivalent to 82S147
ROM_END

ROM_START( tmc2000 )
	ROM_REGION( 0x800, CDP1802_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "prom200", "PROM N:o 200" )
	ROMX_LOAD( "200.m5",    0x000, 0x200, BAD_DUMP CRC(79da3221) SHA1(008da3ef4f69ab1a493362dfca856375b19c94bd), ROM_BIOS(1) ) // typed in from the manual
	ROM_SYSTEM_BIOS( 1, "prom202", "PROM N:o 202" )
	ROMX_LOAD( "202.m5",    0x000, 0x200, NO_DUMP, ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "tool2000", "TOOL-2000" )
	ROMX_LOAD( "tool2000",  0x000, 0x800, NO_DUMP, ROM_BIOS(3) )
ROM_END

ROM_START( nano )
	ROM_REGION( 0x200, CDP1802_TAG, 0 )
	ROM_LOAD( "mmi6349.ic", 0x000, 0x200, BAD_DUMP CRC(1ec1b432) SHA1(ac41f5e38bcd4b80bd7a5b277a2c600899fd5fb8) ) // equivalent to 82S141
ROM_END

/* Driver Initialization */

void tmc1800_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_SETUP_BEEP:
		m_beeper->set_state(0);
		m_beeper->set_clock(0);
		break;
	default:
		assert_always(FALSE, "Unknown id in tmc1800_state::device_timer");
	}
}

DRIVER_INIT_MEMBER(tmc1800_state,tmc1800)
{
	timer_set(attotime::zero, TIMER_SETUP_BEEP);
}

/* System Drivers */

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT        COMPANY         FULLNAME        FLAGS */
COMP( 1977, tmc1800,    0,      0,      tmc1800,    tmc1800, tmc1800_state,    tmc1800, "Telercas Oy",  "Telmac 1800",  MACHINE_NOT_WORKING )
COMP( 1977, osc1000b,   tmc1800,0,      osc1000b,   tmc1800, driver_device,    0,       "OSCOM Oy",     "OSCOM 1000B",  MACHINE_NOT_WORKING )
COMP( 1980, tmc2000,    0,      0,      tmc2000,    tmc2000, driver_device,    0,       "Telercas Oy",  "Telmac 2000",  MACHINE_SUPPORTS_SAVE )
COMP( 1980, nano,       tmc2000,0,      nano,       nano,    driver_device,    0,       "OSCOM Oy",     "OSCOM Nano",   MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
