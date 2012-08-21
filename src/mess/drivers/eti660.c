/*

    TODO:

    - does not boot!
    - reset key
    - allocate color ram
    - quickload
    - color on

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

static CDP1864_INTERFACE( eti660_cdp1864_intf )
{
	CDP1802_TAG,
	SCREEN_TAG,
	CDP1864_INTERLACED,
	DEVCB_DRIVER_LINE_MEMBER(eti660_state, rdata_r),
	DEVCB_DRIVER_LINE_MEMBER(eti660_state, bdata_r),
	DEVCB_DRIVER_LINE_MEMBER(eti660_state, gdata_r),
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT),
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT),
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1),
	DEVCB_NULL,
	RES_K(2.2), /* R7 */
	RES_K(1),	/* R5 */
	RES_K(4.7), /* R6 */
	RES_K(4.7)	/* R4 */
};

/* CDP1802 Interface */

READ_LINE_MEMBER( eti660_state::clear_r )
{
	return BIT(ioport("SPECIAL")->read(), 0);
}

READ_LINE_MEMBER( eti660_state::ef2_r )
{
	return (m_cassette)->input() < 0;
}

READ_LINE_MEMBER( eti660_state::ef4_r )
{
	return BIT(ioport("SPECIAL")->read(), 1);
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
	UINT8 colorram_offset = ((offset & 0xf8) >> 1) || (offset & 0x03);

	m_color = m_color_ram[colorram_offset];

	m_cti->con_w(0); // HACK
	m_cti->dma_w(space, offset, data);
}

static COSMAC_INTERFACE( eti660_config )
{
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_LINE_MEMBER(eti660_state, clear_r),
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(eti660_state, ef2_r),
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(eti660_state, ef4_r),
	DEVCB_DRIVER_LINE_MEMBER(eti660_state, q_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(eti660_state, dma_w),
	NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

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

	if (!BIT(m_keylatch, 0)) data &= ioport("PA0")->read();
	if (!BIT(m_keylatch, 1)) data &= ioport("PA1")->read();
	if (!BIT(m_keylatch, 2)) data &= ioport("PA2")->read();
	if (!BIT(m_keylatch, 3)) data &= ioport("PA3")->read();

	return data;
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

static const pia6821_interface eti660_mc6821_intf =
{
	DEVCB_DRIVER_MEMBER(eti660_state, pia_pa_r),								/* port A input */
	DEVCB_NULL,													/* port B input */
	DEVCB_NULL,													/* CA1 input */
	DEVCB_NULL,													/* CB1 input */
	DEVCB_NULL,													/* CA2 input */
	DEVCB_NULL,													/* CB2 input */
	DEVCB_DRIVER_MEMBER(eti660_state, pia_pa_w),								/* port A output */
	DEVCB_NULL,													/* port B output */
	DEVCB_NULL,													/* CA2 output */
	DEVCB_NULL,													/* CB2 output */
	DEVCB_NULL,													/* IRQA output */
	DEVCB_NULL													/* IRQB output */
};

/* Machine Drivers */

static const cassette_interface eti660_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};

static MACHINE_CONFIG_START( eti660, eti660_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, COSMAC, XTAL_8_867238MHz/5)
	MCFG_CPU_PROGRAM_MAP(eti660_map)
	MCFG_CPU_IO_MAP(eti660_io_map)
	MCFG_CPU_CONFIG(eti660_config)

    /* video hardware */
	MCFG_CDP1864_SCREEN_ADD(SCREEN_TAG, XTAL_8_867238MHz/5)
	MCFG_SCREEN_UPDATE_DEVICE(CDP1864_TAG, cdp1864_device, screen_update)

	MCFG_PALETTE_LENGTH(8+8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_CDP1864_ADD(CDP1864_TAG, XTAL_8_867238MHz/5, eti660_cdp1864_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_PIA6821_ADD(MC6821_TAG, eti660_mc6821_intf)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, eti660_cassette_interface)

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
COMP( 1981, eti660,		0,		0,		eti660,		eti660, driver_device,		0,			"Electronics Today International",	"ETI-660 (Australia)",	GAME_NOT_WORKING )
