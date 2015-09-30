// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    TODO:

    - does not boot!
    - reset key
    - allocate color ram
    - quickload
    - color on

    - keyboard not working
    - foreground is black, according to the construction article it should be white.
    - it is supposed to reset itself at boot, but that isn't working. You must press R.
    - Pressing F3 causes it to jump into the weeds.

*/

#include "includes/eti660.h"

/* Read/Write Handlers */

READ8_MEMBER( eti660_state::pia_r )
{
	int pia_offset = m_maincpu->get_memory_address() & 0x03;

	return m_pia->read(space, pia_offset);
}

WRITE8_MEMBER( eti660_state::pia_w )
{
	int pia_offset = m_maincpu->get_memory_address() & 0x03;

	m_pia->write(space, pia_offset, data);
}

WRITE8_MEMBER( eti660_state::colorram_w )
{
	int colorram_offset = m_maincpu->get_memory_address() & 0xff;

	colorram_offset = ((colorram_offset & 0xf8) >> 1) || (colorram_offset & 0x03);

	m_color_ram[colorram_offset] = data;
}

/* Memory Maps */

static ADDRESS_MAP_START( eti660_map, AS_PROGRAM, 8, eti660_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
	AM_RANGE(0x0400, 0x0fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( eti660_io_map, AS_IO, 8, eti660_state )
	AM_RANGE(0x01, 0x01) AM_DEVREADWRITE(CDP1864_TAG, cdp1864_device, dispon_r, step_bgcolor_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(pia_r, pia_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(colorram_w)
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE(CDP1864_TAG, cdp1864_device, dispoff_r, tone_latch_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( eti660 )
	PORT_START("PA0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')

	PORT_START("PA1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')

	PORT_START("PA2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')

	PORT_START("PA3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_S)
INPUT_PORTS_END

/* Video */

READ_LINE_MEMBER( eti660_state::rdata_r )
{
	return BIT(m_color, 0);
}

READ_LINE_MEMBER( eti660_state::bdata_r )
{
	return BIT(m_color, 1);
}

READ_LINE_MEMBER( eti660_state::gdata_r )
{
	return BIT(m_color, 2);
}

/* CDP1802 Interface */

READ_LINE_MEMBER( eti660_state::clear_r )
{
	return BIT(m_special->read(), 0);
}

READ_LINE_MEMBER( eti660_state::ef2_r )
{
	return m_cassette->input() < 0;
}

READ_LINE_MEMBER( eti660_state::ef4_r )
{
	return BIT(m_special->read(), 1);
}

WRITE_LINE_MEMBER( eti660_state::q_w )
{
	/* CDP1864 audio output enable */
	m_cti->aoe_w(state);

	/* PULSE led */
	set_led_status(machine(), LED_PULSE, state);

	/* tape output */
	m_cassette->output(state ? 1.0 : -1.0);
}

WRITE8_MEMBER( eti660_state::dma_w )
{
	UINT8 colorram_offset = ((offset & 0xf8) >> 1) | (offset & 0x03);

	m_color = m_color_ram[colorram_offset];

	m_cti->con_w(0); // HACK
	m_cti->dma_w(space, offset, data);
}

/* PIA6821 Interface */

READ8_MEMBER( eti660_state::pia_pa_r )
{
	/*

	    bit     description

	    PA0     keyboard row 0
	    PA1     keyboard row 1
	    PA2     keyboard row 2
	    PA3     keyboard row 3
	    PA4     keyboard column 0
	    PA5     keyboard column 1
	    PA6     keyboard column 2
	    PA7     keyboard column 3

	*/

	UINT8 data = 0xf0;

	if (!BIT(m_keylatch, 0)) data &= m_pa0->read();
	if (!BIT(m_keylatch, 1)) data &= m_pa1->read();
	if (!BIT(m_keylatch, 2)) data &= m_pa2->read();
	if (!BIT(m_keylatch, 3)) data &= m_pa3->read();

	return data | m_keylatch;
}

WRITE8_MEMBER( eti660_state::pia_pa_w )
{
	/*

	    bit     description

	    PA0     keyboard row 0
	    PA1     keyboard row 1
	    PA2     keyboard row 2
	    PA3     keyboard row 3
	    PA4     keyboard column 0
	    PA5     keyboard column 1
	    PA6     keyboard column 2
	    PA7     keyboard column 3

	*/

	m_keylatch = data & 0x0f;
}

/* Machine Drivers */

static MACHINE_CONFIG_START( eti660, eti660_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_8_867238MHz/5)
	MCFG_CPU_PROGRAM_MAP(eti660_map)
	MCFG_CPU_IO_MAP(eti660_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(eti660_state, clear_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(eti660_state, ef2_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(eti660_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(eti660_state, q_w))
	MCFG_COSMAC_DMAW_CALLBACK(WRITE8(eti660_state, dma_w))

	/* video hardware */
	MCFG_CDP1864_SCREEN_ADD(SCREEN_TAG, XTAL_8_867238MHz/5)
	MCFG_SCREEN_UPDATE_DEVICE(CDP1864_TAG, cdp1864_device, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_CDP1864_ADD(CDP1864_TAG, SCREEN_TAG, XTAL_8_867238MHz/5, GND, INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT), INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT), INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1), NULL, READLINE(eti660_state, rdata_r), READLINE(eti660_state, bdata_r), READLINE(eti660_state, gdata_r))
	MCFG_CDP1864_CHROMINANCE(RES_K(2.2), RES_K(1), RES_K(4.7), RES_K(4.7)) // R7, R5, R6, R4
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_DEVICE_ADD(MC6821_TAG, PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(eti660_state, pia_pa_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(eti660_state, pia_pa_w))

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("3K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( eti660 )
	ROM_REGION( 0x10000, CDP1802_TAG, 0 )
	ROM_LOAD( "eti660.bin", 0x0000, 0x0400, CRC(811dfa62) SHA1(c0c4951e02f873f15560bdc3f35cdf3f99653922) )
ROM_END

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       INIT        COMPANY                             FULLNAME                FLAGS */
COMP( 1981, eti660,     0,      0,      eti660,     eti660, driver_device,      0,          "Electronics Today International",  "ETI-660",  MACHINE_NOT_WORKING )
